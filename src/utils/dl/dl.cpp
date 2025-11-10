/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/utils/dl/dl.h"
#include <atomic>
#include <unistd.h>
#include <dlfcn.h>
#include "mki/utils/assert/assert.h"
#include "mki/utils/log/log.h"
#include "mki/utils/file_system/file_system.h"
namespace Mki {
std::atomic_int g_memfdNextId(0);

Dl::Dl(const std::string &dlFilePath, bool checkSymlinks) : memFd_(0), dlHandle_(nullptr)
{
    dlSuccess_ = false;
    std::string realPath = FileSystem::PathCheckAndRegular(dlFilePath, checkSymlinks);
    MKI_CHECK(!realPath.empty(), "file path is invalid", return);
    MKI_CHECK((FileSystem::FileSize(realPath) > 0 && FileSystem::FileSize(realPath) <= MAX_FILE_SIZE / 10),
        "File size is invalid", return);   // MAX_FILE_SIZE / 10 = 1GB
    dlHandle_ = dlopen(realPath.c_str(), RTLD_LAZY);
    MKI_LOG_IF(dlHandle_ == nullptr, ERROR) << "dlopen " << dlFilePath << " fail, error:" << dlerror();
    dlSuccess_ = true;
}

Dl::~Dl()
{
    if (dlHandle_) {
        dlclose(dlHandle_);
        dlHandle_ = nullptr;
    }

    if (memFd_ > 0) {
        close(memFd_);
        memFd_ = 0;
    }
}

bool Dl::IsValid() const
{
    return dlSuccess_;
}

void *Dl::GetSymbol(const std::string &symbolName) const
{
    if (dlHandle_ == nullptr) {
        return nullptr;
    }

    return dlsym(dlHandle_, symbolName.c_str());
}

std::string Dl::GetNewMemFdFileName() const
{
    std::string fileName;
    int id = g_memfdNextId++;
    fileName = "dl_" + std::to_string(id);
    return fileName;
}
} // namespace Mki
