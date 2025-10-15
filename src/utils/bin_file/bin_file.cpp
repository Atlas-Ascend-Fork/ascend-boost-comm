/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/utils/bin_file/bin_file.h"
#include <securec.h>
#include <cstring>
#include <climits>
#include <fcntl.h>
#include <unistd.h>
#include "mki/utils/assert/assert.h"
#include "mki/utils/log/log.h"
#include "mki/utils/strings/str_split.h"
#include "mki/utils/strings/str_replace.h"
#include "mki/utils/strings/match.h"
#include "mki/utils/file_system/file_system.h"
#include "mki/utils/strings/str_checker.h"

namespace Mki {
constexpr uint32_t MAX_FILE_OPERATE_NUM = 1000;
constexpr uint32_t PER_OPERATE_LINE_NUM = 8;
constexpr uint32_t MAX_FILE_LINE_NUM = MAX_FILE_OPERATE_NUM * PER_OPERATE_LINE_NUM;
constexpr uint64_t MAX_SINGLE_MEMCPY_SIZE = 1073741824; // copy 1G each loop

BinFile::BinFile() {}

BinFile::~BinFile() {}

Status BinFile::AddAttr(const std::string &name, const std::string &value)
{
    if (!CheckNameValid(name)) {
        return Status::FailStatus(1, "invalid name");
    }

    if (attrNames_.find(name) != attrNames_.end()) {
        return Status::FailStatus(1, "attr:" + name + " already exists");
    }

    attrNames_.insert(name);
    attrs_.push_back({name, value});

    return Status::OkStatus();
}

Status BinFile::AddObject(const std::string &name, char *binaryBuffer, uint64_t binaryLen)
{
    if (!CheckNameValid(name)) {
        return Status::FailStatus(1, "invalid name");
    }

    if (binaryBuffer == nullptr) {
        return Status::FailStatus(1, "binaryBuffer is nullptr");
    }

    size_t needLen = binariesBuffer_.size() + binaryLen;
    if (binaryLen > static_cast<uint64_t>(MAX_FILE_SIZE) || needLen > static_cast<uint64_t>(MAX_FILE_SIZE)) {
        return Status::FailStatus(1, "MAX file size exceeded");
    }

    if (binaryNames_.find(name) != binaryNames_.end()) {
        return Status::FailStatus(1, "object:" + name + " already exists");
    }
    binaryNames_.insert(name);

    size_t currentLen = binariesBuffer_.size();
    BinFile::Binary binary = {currentLen, binaryLen};
    binaries_.push_back({name, binary});
    binariesBuffer_.resize(needLen);

    uint64_t offset = 0;
    uint64_t copyLen = binaryLen;
    while (copyLen > 0) {
        uint64_t curCopySize = copyLen > MAX_SINGLE_MEMCPY_SIZE ? MAX_SINGLE_MEMCPY_SIZE : copyLen;
        auto ret = memcpy_s(binariesBuffer_.data() + currentLen + offset, binariesBuffer_.size() - currentLen - offset,
                            binaryBuffer + offset, curCopySize);
        if (ret != EOK) {
            return Status::FailStatus(1, "Memcpy fail");
        }
        offset += curCopySize;
        copyLen -= curCopySize;
    }
    return Status::OkStatus();
}

bool BinFile::WriteImpl(int &fd)
{
    bool ret = WriteAttr(fd, ATTR_VERSION, version_);
    MKI_CHECK(ret, "write attr version fail", return ret);

    ret = WriteAttr(fd, ATTR_OBJECT_COUNT, std::to_string(binaries_.size()));
    MKI_CHECK(ret, "write attr object count fail", return ret);

    ret = WriteAttr(fd, ATTR_OBJECT_LENGTH, std::to_string(binariesBuffer_.size()));
    MKI_CHECK(ret, "write attr object length fail", return ret);

    for (const auto &attrIt : attrs_) {
        ret = WriteAttr(fd, attrIt.first, attrIt.second);
        MKI_CHECK(ret, "write attrIt fail", return ret);
    }

    for (const auto &objIt : binaries_) {
        ret = WriteAttr(fd, ATTR_OBJECT_PREFIX + objIt.first,
                        std::to_string(objIt.second.offset) + "," + std::to_string(objIt.second.length));
        MKI_CHECK(ret, "write offset and length fail", return ret);
    }

    ret = WriteAttr(fd, ATTR_END, "1");
    MKI_CHECK(ret, "write ATTR_END fail", return ret);

    if (binariesBuffer_.size() > 0) {
        auto writeSize = write(fd, binariesBuffer_.data(), binariesBuffer_.size());
        if (writeSize != static_cast<ssize_t>(binariesBuffer_.size())) {
            MKI_LOG(ERROR) << "write binaries buffer fail";
            return ret;
        }
    }
    return ret;
}

Status BinFile::Write(const std::string &filePath, const mode_t mode)
{
    char resolvedDir[PATH_MAX] = {0};
    MKI_CHECK(realpath(FileSystem::DirName(filePath).c_str(), resolvedDir) != nullptr, filePath <<
              " realpath resolved fail", return Status::FailStatus(1, "open file fail"));

    int fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_TRUNC, mode);
    if (fd < 0) {
        return Status::FailStatus(1, "open file fail");
    }
    if (!WriteImpl(fd)) {
        close(fd);
        return Status::FailStatus(1, "write bin file fail");
    }

    close(fd);
    return Status::OkStatus();
}

bool BinFile::WriteAttr(int &fd, const std::string &name, const std::string &value) const
{
    std::string line = name + "=" + value + "\n";
    auto writeSize = write(fd, line.data(), line.size());
    if (writeSize != static_cast<ssize_t>(line.size())) {
        MKI_LOG(ERROR) << "write attr failed";
        return false;
    }
    return true;
}

Status BinFile::Read(const std::string &filePath)
{
    char resolvedPath[PATH_MAX] = {0};
    MKI_CHECK(realpath(filePath.c_str(), resolvedPath) != nullptr, "realpath resolved fail",
              return Status::FailStatus(1, "realpath resolved fail"));

    std::string realPath = FileSystem::PathCheckAndRegular(resolvedPath);
    MKI_CHECK(!realPath.empty(), "bin file path invalid", return Status::FailStatus(1, "file path is invalid"));
    int64_t fileSize = FileSystem::FileSize(realPath);
    if (fileSize < 0 || fileSize > MAX_FILE_SIZE) {
        return Status::FailStatus(1, "File size is invalid");
    }

    std::ifstream fd(realPath.c_str());
    if (!fd) {
        return Status::FailStatus(1, "open " + FileSystem::BaseName(filePath) + " for read fail");
    }

    return ParseBinFile(fd, static_cast<size_t>(fileSize));
}

Status BinFile::ParseBinFile(std::ifstream &fd, size_t fileSize)
{
    MKI_LOG(DEBUG) << "Begin to parse bin file";
    bool foundAttrObjectLength = false;
    bool matchAttrEnd = false;
    std::string line;
    uint32_t lineNum = 0;
    while (getline(fd, line)) {
        MKI_CHECK(lineNum <= MAX_FILE_LINE_NUM, "file lineNum is out of range, lineNum: " << lineNum, break);
        lineNum++;

        std::vector<std::string> fields;
        StrSplit(line, '=', fields);
        const int needFieldNum = 2;
        MKI_CHECK(fields.size() == needFieldNum, "Invalid binary file line!!!", break);

        std::string attrName = fields[0];
        std::string attrValue = fields[1];
        MKI_CHECK(!attrName.empty() && !attrValue.empty(), "attrName or attrValue is empty!!!", break);
        MKI_LOG(DEBUG) << "attrName:" << attrName << ", attrValue:" << attrValue;
        if (attrName == ATTR_END) {
            matchAttrEnd = true;
            break;
        } else if (attrName == ATTR_OBJECT_COUNT) {
            continue; // this attr is not used now
        } else if (attrName == ATTR_OBJECT_LENGTH) {
            MKI_CHECK(!foundAttrObjectLength, "object length is repeated", break);
            foundAttrObjectLength = true;
            uint64_t totalBinarySize = std::strtoull(attrValue.c_str(), nullptr, 10); // 10进制
            MKI_CHECK(totalBinarySize <= static_cast<uint64_t>(MAX_FILE_SIZE), "totalBinarySize exceeded", break);
            MKI_LOG(DEBUG) << "Parsed object length " << totalBinarySize;
            binariesBuffer_.resize(totalBinarySize);
        } else if (attrName.at(0) == '$') {
            MKI_CHECK(ParseSystemAttr(attrName, attrValue).Ok(), "failed to parse attr", break);
        } else {
            MKI_CHECK(attrNames_.find(attrName) == attrNames_.end(), "attrName is repeated!!!", break);
            attrNames_.insert(attrName);
            attrs_.push_back({attrName, attrValue});
        }
    }
    MKI_CHECK(matchAttrEnd, "failed to parse bin file", return Status::FailStatus(1, "Failed to parse bin file"));

    size_t position = static_cast<size_t>(fd.tellg());
    MKI_LOG(DEBUG) << "Read binary buffer, size: " << binariesBuffer_.size() << ", file size: " << fileSize
                  << ", current posotion: " << position;
    MKI_CHECK(position <= fileSize && binariesBuffer_.size() <= fileSize - position,
        "Not enough file for binary buffer", return Status::FailStatus(1, "Failed to parse binary from bin file"));

    fd.read(binariesBuffer_.data(), binariesBuffer_.size());

    return Status::OkStatus();
}

Status BinFile::ParseSystemAttr(const std::string &attrName, const std::string &value)
{
    if (StartsWith(attrName, ATTR_OBJECT_PREFIX)) {
        std::string objName = attrName;
        StrErase(objName, ATTR_OBJECT_PREFIX);
        std::vector<std::string> fields;
        StrSplit(value, ',', fields);
        const size_t needFieldNum = 2;
        MKI_CHECK(fields.size() == needFieldNum && binaryNames_.find(objName) == binaryNames_.end(),
            "Invalid object field " << value, return Status::FailStatus(1, "Invalid object field"));
        binaryNames_.insert(objName);

        uint64_t offset = std::strtoull(fields[0].c_str(), nullptr, 10); // 10 进制
        uint64_t length = std::strtoull(fields[1].c_str(), nullptr, 10); // 10 进制
        MKI_CHECK(offset <= binariesBuffer_.size() &&
            length <= binariesBuffer_.size() && offset + length <= binariesBuffer_.size(),
            "offset " << offset << " or length " << length << " exceeded",
            return Status::FailStatus(1, "Invalid object offset/length")
        );
        binaries_.push_back({objName, BinFile::Binary({offset, length})});
    } else {
        MKI_LOG(WARN) << "not process " << attrName;
    }
    return Status::OkStatus();
}

void BinFile::GetAllAttrs(std::vector<std::pair<std::string, std::string>> &attrs) { attrs = attrs_; }

void BinFile::GetAllObjects(std::vector<std::pair<std::string, std::pair<char *, uint64_t>>> &binaries)
{
    for (auto it : binaries_) {
        binaries.push_back({it.first, {binariesBuffer_.data() + it.second.offset, it.second.length}});
    }
}
} // namespace Mki
