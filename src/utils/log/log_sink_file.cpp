/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <syscall.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sys/stat.h>
#include <pwd.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/statvfs.h>
#include <securec.h>
#include "mki/utils/log/log_core.h"
#include "mki/utils/env/env.h"
#include "mki/utils/strings/match.h"
#include "mki/utils/strings/str_checker.h"
#include "mki/utils/strings/str_split.h"
#include "mki/utils/log/log_sink_file.h"

namespace Mki {
constexpr size_t MAX_LOG_FILE_COUNT = 50;                               // 50 回滚管理50个日志文件
constexpr size_t MAX_FILE_NAME_LEN = 127;                               // 127: max file length without '\0' = 128 - 1
constexpr uint64_t MAX_FILE_SIZE_THRESHOLD = 1073741824;                // 1073741824 当前单个日志文件最大1G
constexpr uint64_t DISK_AVAILABEL_LIMIT = 10 * MAX_FILE_SIZE_THRESHOLD; // 磁盘剩余空间门限10G

// A symlink with a trailing slash (/) is not recognized as a symlink, which is consistent with the operating system.
static bool IsSymlink(const std::string &filePath)
{
    struct stat buf {};
    if (lstat(filePath.c_str(), &buf) != 0) {
        return false;
    }
    return S_ISLNK(buf.st_mode);
}

std::string RemoveTrailingSlash(const std::string &path)
{
    std::string res;
    size_t lastNonSlash = path.find_last_not_of("/");
    if (lastNonSlash == std::string::npos && !path.empty()) {
        res = "/";
    } else if (lastNonSlash != std::string::npos && lastNonSlash != path.size() - 1) {
        res = path.substr(0, lastNonSlash + 1);
    } else {
        res = path;
    }
    return res;
}

static std::string PathCheckAndRegular(const std::string &path)
{
    if (path.empty()) {
        std::cout << "path string is NULL";
        return "";
    }

    if (path.size() >= PATH_MAX) {
        std::cout << "file path " << path.c_str() << " is too long!";
        return "";
    }

    if (path.find("..") != std::string::npos) {
        std::cout << "file path " << path.c_str() << " contains parent directory reference!";
        return "";
    }

    std::string regularPath = RemoveTrailingSlash(path);
    if (IsSymlink(regularPath)) {
        std::cout << "The 'filepath' " << path.c_str() << " is symbolic link";
        return "";
    }

    char resolvedPath[PATH_MAX] = {0};
    std::string res;

    if (realpath(path.c_str(), resolvedPath) != nullptr) {
        res = resolvedPath;
    }
    return res;
}

LogSinkFile::LogSinkFile() { Init(); }

LogSinkFile::~LogSinkFile() { CloseFile(); }

void LogSinkFile::Log(const char *log, uint64_t logLen)
{
    std::lock_guard<std::mutex> guard(mutex_);
    if (currentFileSize_ + logLen >= MAX_FILE_SIZE_THRESHOLD) {
        CloseFile();
    }

    if (currentFd_ < 0) {
        OpenFile();
    }

    if (currentFd_ < 0) {
        return;
    }

    ssize_t writeSize = write(currentFd_, log, logLen);
    if (writeSize != static_cast<ssize_t>(logLen)) {
        std::cout << "mki_log write file fail, want to write size: " << logLen
                  << ", success write size:" << writeSize;
        CloseFile();
        return;
    }

    currentFileSize_ += writeSize;
}

static bool IsValidFileName(const std::string &name)
{
    size_t len = name.size();
    if (len == 0 || len > MAX_FILE_NAME_LEN) {
        return false;
    }

    for (size_t i = 0; i < len; ++i) {
        char c = name[i];
        if (!isalnum(c) && c != '_' && c != '/') {
            return false;
        }
    }

    return true;
}

void LogSinkFile::Init()
{
    boostType_ = "atb";

    std::string logRootDir = GetHomeDir();

    std::string env = std::string(std::getenv("HOME") != nullptr ? std::getenv("HOME") : "");
    logRootDir = IsValidFileName(env) ? env + "/ascend/log" : logRootDir;
    logRootDir = PathCheckAndRegular(logRootDir);

    env = std::string(std::getenv("ASCEND_WORK_PATH") != nullptr ? std::getenv("ASCEND_WORK_PATH") : "");
    logRootDir = IsValidFileName(env) ? env + "/log" : logRootDir;
    logRootDir = PathCheckAndRegular(logRootDir);

    env = std::string(std::getenv("ASCEND_PROCESS_LOG_PATH") != nullptr ? std::getenv("ASCEND_PROCESS_LOG_PATH") : "");
    logRootDir = IsValidFileName(env) ? env : logRootDir;
    logRootDir = PathCheckAndRegular(logRootDir);

    logDir_ = logRootDir + "/" + boostType_;

    env = std::string(std::getenv("ASCEND_LOG_SYNC_SAVE") != nullptr ? std::getenv("ASCEND_LOG_SYNC_SAVE") : "");
    isFlush_ = env.size() > 0 && env.size() <= MAX_ENV_STRING_LEN - 1 ? env == "1" : false;
}

bool LogSinkFile::IsFileNameMatched(const std::string &fileName, std::string &createTime)
{
    std::string prefix = boostType_ + '_';
    bool match = StartsWith(fileName, prefix);
    if (!match) {
        return false;
    }
    match = EndsWith(fileName, ".log");
    if (!match) {
        return false;
    }
    size_t subStrLen = fileName.length() - prefix.length() - 4;     // 4: length of ".log" postfix
    std::string subStr = fileName.substr(prefix.length(), subStrLen);
    std::vector<std::string> splitResult;
    StrSplit(subStr, '_', splitResult);
    if (splitResult.size() != 2) {  // 2: time & pid
        return false;
    }
    for (auto &str : splitResult) {
        if (str.empty() || !std::all_of(str.begin(), str.end(), ::isdigit)) {
            return false;
        }
    }
    createTime = splitResult.at(1);
    return true;
}

void LogSinkFile::DeleteOldestFile()
{
    std::vector<std::pair<std::string, std::string>> logFiles;
    DIR *dir = opendir(logDir_.c_str());
    if (!dir) {
        return;
    }
    struct dirent *ptr = nullptr;
    while ((ptr = readdir(dir)) != nullptr) {
        if (ptr->d_name[0] != '.') {
            std::string fileName = ptr->d_name;
            std::string createTime;
            if (IsFileNameMatched(fileName, createTime)) {
                logFiles.emplace_back(logDir_ + "/" + fileName, createTime);
            }
        }
    }
    closedir(dir);

    std::sort(logFiles.begin(), logFiles.end(),
              [](std::pair<std::string, std::string> &a, std::pair<std::string, std::string> &b) {
                  return a.second < b.second;
              });

    if (logFiles.size() > MAX_LOG_FILE_COUNT) {
        size_t deleteCount = logFiles.size() - MAX_LOG_FILE_COUNT;
        for (size_t i = 0; i < deleteCount; ++i) {
            std::cout << "mki_log delete old file:" << logFiles[i].first << std::endl;
            remove(logFiles[i].first.c_str());
        }
    }
}

void LogSinkFile::OpenFile()
{
    MakeLogDir();

    DeleteOldestFile();

    if (!IsDiskAvailable()) {
        return;
    }

    std::string logFilePath = GetNewLogFilePath();
    currentFd_ = open(logFilePath.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
    if (currentFd_ < 0) {
        std::cout << "mki_log open " << logFilePath << " fail" << std::endl;
    }
}

std::string LogSinkFile::GetNewLogFilePath()
{
    std::time_t tmpTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *nowTime = std::localtime(&tmpTime);

    std::stringstream filePath;
    filePath << logDir_ << "/" << boostType_ << "_" << std::to_string(syscall(SYS_getpid)) << "_"
             << std::put_time(nowTime, "%Y%m%d%H%M%S") << ".log";
    return filePath.str();
}

bool LogSinkFile::IsDiskAvailable()
{
    struct statvfs vfs;
    if (statvfs(logDir_.c_str(), &vfs) == -1) {
        std::cout << "mki_log get current disk stats fail" << std::endl;
        return false;
    }

    uint64_t availableSize = vfs.f_bsize * vfs.f_bfree;
    if (availableSize <= DISK_AVAILABEL_LIMIT) {
        std::cout << "mki_log disk available space it too low, available size:" << availableSize
                  << ", limit size:" << DISK_AVAILABEL_LIMIT << std::endl;
        return false;
    }

    return true;
}

void LogSinkFile::MakeLogDir()
{
    struct stat st;
    if (stat(logDir_.c_str(), &st) == 0) { // 目录已经存在，就不创建
        return;
    }

    mode_t mode = S_IRWXU | S_IRGRP |  S_IXGRP;
    uint32_t offset = 0;
    uint32_t pathLen = logDir_.size();
    do {
        const char *str = strchr(logDir_.c_str() + offset, '/');
        offset = (str == nullptr) ? pathLen : str - logDir_.c_str() + 1;
        std::string childDir = logDir_.substr(0, offset);
        if (stat(childDir.c_str(), &st) < 0) {
            std::cout << "mki_log mkdir " << childDir << std::endl;
            if (mkdir(childDir.c_str(), mode) < 0) {
                return;
            }
        }
    } while (offset != pathLen);
}

void LogSinkFile::CloseFile()
{
    if (currentFd_ > 0) {
        (void)fchmod(currentFd_, S_IRUSR | S_IRGRP);
        close(currentFd_);
        currentFd_ = -1;
        currentFileSize_ = 0;
    }
}

std::string LogSinkFile::GetHomeDir()
{
    int bufsize;
    if ((bufsize = sysconf(_SC_GETPW_R_SIZE_MAX)) == -1) {
        return "";
    }

    char buffer[bufsize] = {0};
    struct passwd pwd;
    struct passwd *result = nullptr;
    if (getpwuid_r(getuid(), &pwd, buffer, bufsize, &result) != 0 || !result) {
        return "";
    }

    return std::string(pwd.pw_dir);
}
} // namespace Mki
