/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/utils/inifile/ini_file.h"
#include <cstring>
#include <fstream>
#include <algorithm>
#include <iostream>
#include "mki/utils/assert/assert.h"
#include "mki/utils/log/log.h"
#include "mki/utils/file_system/file_system.h"

namespace Mki {
constexpr uint32_t MAX_FILE_LINE_NUM = 20000;
bool IniFile::ParseIniFileToMap(const std::string &iniFilePath,
    std::map<std::string, std::map<std::string, std::string>> &contentInfoMap, bool symCheck)
{
    std::string realPath = FileSystem::PathCheckAndRegular(iniFilePath, symCheck);
    MKI_CHECK((FileSystem::FileSize(realPath) > 0 && FileSystem::FileSize(realPath) <= MAX_FILE_SIZE),
        "File size is invalid", return false);
    std::ifstream ifs(realPath);
    MKI_CHECK(ifs.is_open(), "Open ini file failed", return false);

    std::map<std::string, std::string> contentMap;
    contentMap.clear();
    contentInfoMap.clear();
    std::string line;
    std::string mapKey;
    uint32_t lineNum = 0;
    while (std::getline(ifs, line)) {
        if (lineNum > MAX_FILE_LINE_NUM) {
            MKI_LOG(ERROR) << "file lineNum is out of range, lineNum : " << lineNum;
            break;
        }
        lineNum++;
        // 去除空格
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        if (line.empty() || line.find('#') == 0) {
            continue;
        }
        if (line.find('[') == 0) {
            if (!mapKey.empty() && !contentMap.empty()) {
                contentInfoMap.emplace(make_pair(mapKey, contentMap));
                contentMap.clear();
            }
            size_t pos = line.rfind(']');
            if (pos == std::string::npos) {
                continue;
            }
            mapKey = line.substr(1, pos - 1);
            continue;
        }
        size_t posOfEqual = line.find('=');
        if (posOfEqual == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, posOfEqual);
        std::string value = line.substr(posOfEqual + 1, line.length() - posOfEqual - 1);
        if (!key.empty() && !value.empty()) {
            contentMap.emplace(make_pair(key, value));
        }
    }
    if (!contentMap.empty() && !mapKey.empty()) {
        contentInfoMap.emplace(make_pair(mapKey, contentMap));
        contentMap.clear();
    }
    ifs.close();
    return true;
}

bool IniFile::ParseIniFileToMapNoLog(const std::string &iniFilePath,
    std::map<std::string, std::map<std::string, std::string>> &contentInfoMap, bool symCheck)
{
    std::string realPath = FileSystem::PathCheckAndRegularNoLog(iniFilePath, symCheck);
    if (FileSystem::FileSize(realPath) <= 0 || FileSystem::FileSize(realPath) > MAX_FILE_SIZE) {
        return false;
    }
    std::ifstream ifs(realPath);
    if (!ifs.is_open()) {
        return false;
    }

    std::map<std::string, std::string> contentMap;
    contentMap.clear();
    contentInfoMap.clear();
    std::string line;
    std::string mapKey;
    uint32_t lineNum = 0;
    while (std::getline(ifs, line)) {
        if (lineNum > MAX_FILE_LINE_NUM) {
            break;
        }
        lineNum++;
        // 去除空格
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        if (line.empty() || line.find('#') == 0) {
            continue;
        }
        if (line.find('[') == 0) {
            if (!mapKey.empty() && !contentMap.empty()) {
                contentInfoMap.emplace(make_pair(mapKey, contentMap));
                contentMap.clear();
            }
            size_t pos = line.rfind(']');
            if (pos == std::string::npos) {
                continue;
            }
            mapKey = line.substr(1, pos - 1);
            continue;
        }
        size_t posOfEqual = line.find('=');
        if (posOfEqual == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, posOfEqual);
        std::string value = line.substr(posOfEqual + 1, line.length() - posOfEqual - 1);
        if (!key.empty() && !value.empty()) {
            contentMap.emplace(make_pair(key, value));
        }
    }
    if (!contentMap.empty() && !mapKey.empty()) {
        contentInfoMap.emplace(make_pair(mapKey, contentMap));
        contentMap.clear();
    }
    ifs.close();
    return true;
}
} // namespace Mki
