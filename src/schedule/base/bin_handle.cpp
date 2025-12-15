/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#include "mki/bin_handle.h"
#include "mki/utils/assert/assert.h"
#include "mki/utils/rt/rt.h"

namespace Mki {
constexpr uint32_t BYTE_SIZE = 8;
constexpr uint32_t HEADER_LENGTH = 128;
constexpr uint32_t UINT32_TYPE_LENGTH = sizeof(uint32_t);
constexpr uint32_t MASK_LOWER16 = 0xFFFFU;
constexpr uint32_t MASK_HIGHER16 = 0xFFFF0000U;
constexpr uint32_t SHIFT16 = 16U;

struct KernelHeaderInfo {
    uint32_t version = 0;
    uint32_t magic = 0;
    uint32_t tilingSize = 0;
    uint32_t coreType = 0;
    uint32_t kernelNum = 0;
    uint32_t kernelNameOffset = 0;
    uint32_t compileInfoOffset = 0;
    uint32_t kernelBinOffset = 0;
    uint32_t intercoreSync = 0;
    uint32_t taskRation = 0;
};

BinHandle::BinHandle(const BinaryBasicInfo *binInfo) : basicInfo_(binInfo) {}

BinHandle::~BinHandle()
{
    int st = MkiRtModuleDestory(&moduleHandle_);
    if (st != MKIRT_SUCCESS) {
        MKI_LOG(ERROR) << "Module Destory failed";
    }
}

KernelHandle BinHandle::GetHandle() const { return &handle_; }

bool BinHandle::CheckBinaryValid() const
{
    MKI_CHECK(basicInfo_ != nullptr, "basicInfo is nullptr", return false);
    MKI_CHECK(basicInfo_->binaryBuf != nullptr, "binary info is empty!", return false);
    MKI_CHECK(basicInfo_->binaryLen > HEADER_LENGTH, "binary length is less than header length!", return false);
    return true;
}

bool BinHandle::CheckKernelInfo(const std::string &kernelName) const
{
    MKI_CHECK(!metaInfo_.kernelList.empty(), "Get Binary Kernel Num empty, kernel: " << kernelName, return false);
    MKI_CHECK(metaInfo_.magic != 0, "Get Magic empt, kernel: " << kernelName, return false);
#ifdef _DEBUG
    for (const std::string &kernel : metaInfo_.kernelList) {
        MKI_LOG(DEBUG) << kernelName << ".kernelName: " << kernel;
    }
    MKI_LOG(DEBUG) << kernelName << ".devicecodeBufLen: " << metaInfo_.codeBufLen;
#endif
    return true;
}

bool BinHandle::Init(const std::string &kernelName)
{
    codeLoadSuccess_ = false;

    MKI_CHECK(CheckBinaryValid(), "basicInfo is invalid", return false);

    const KernelHeaderInfo &header = *reinterpret_cast<const KernelHeaderInfo *>(basicInfo_->binaryBuf);
    const uint8_t *data = basicInfo_->binaryBuf + HEADER_LENGTH;
    const uint8_t *maxAddr = basicInfo_->binaryBuf + basicInfo_->binaryLen;
    metaInfo_.version = header.version;
    metaInfo_.magic = header.magic;
    metaInfo_.tilingSize = header.tilingSize;
    metaInfo_.coreType = header.coreType;
    metaInfo_.intercoreSync = header.intercoreSync;
    metaInfo_.cubeRatio = (header.taskRation & MASK_HIGHER16) >> SHIFT16;
    metaInfo_.vectorRatio = header.taskRation & MASK_LOWER16;
    uint32_t kernelNum = header.kernelNum;
    const uint8_t *kernelNameStart = data + header.kernelNameOffset;
    for (size_t count = 0; count < kernelNum; ++count) {
        MKI_CHECK(kernelNameStart + UINT32_TYPE_LENGTH <= maxAddr, "length error", return false);
        uint32_t kernelNameSize = *reinterpret_cast<const uint32_t *>(kernelNameStart);
        kernelNameStart += UINT32_TYPE_LENGTH;
        MKI_CHECK(kernelNameStart + kernelNameSize <= maxAddr, "length error", return false);
        std::string str = reinterpret_cast<const char *>(kernelNameStart);
        MKI_CHECK(str.length() < kernelNameSize, "length error", return false);
        metaInfo_.kernelList.push_back(str);
        kernelNameStart += kernelNameSize;
    }
    const uint8_t *compileInfoStart = data + header.compileInfoOffset;
    MKI_CHECK(compileInfoStart + UINT32_TYPE_LENGTH <= maxAddr, "length error", return false);
    uint32_t compileInfoSize = *reinterpret_cast<const uint32_t *>(compileInfoStart);
    compileInfoStart += UINT32_TYPE_LENGTH;
    metaInfo_.compileInfo = reinterpret_cast<const char *>(compileInfoStart);
    MKI_CHECK(metaInfo_.compileInfo.length() < compileInfoSize, "length error", return false);

    const uint8_t *kernelBinStart = data + header.kernelBinOffset;
    MKI_CHECK(kernelBinStart + UINT32_TYPE_LENGTH <= maxAddr, "length error", return false);
    uint32_t kernelBinSize = *reinterpret_cast<const uint32_t *>(kernelBinStart);
    MKI_CHECK(kernelBinStart + UINT32_TYPE_LENGTH + kernelBinSize == maxAddr, "length error", return false);
    metaInfo_.codeBuf = static_cast<const void *>(kernelBinStart + UINT32_TYPE_LENGTH);
    metaInfo_.codeBufLen = kernelBinSize;

    MKI_CHECK(CheckKernelInfo(kernelName), kernelName << " check kernel info error", return false);
    MKI_CHECK(RegisterBin(kernelName), kernelName << " register kernel fail", return false);

    codeLoadSuccess_ = true;
    return true;
}

uint32_t BinHandle::GetKernelTilingSize() const
{
    if (!codeLoadSuccess_) {
        return 0;
    }
    uint32_t tilingSize = metaInfo_.tilingSize;
    return ceil(static_cast<double>(tilingSize) / BYTE_SIZE) * BYTE_SIZE;
}

int32_t BinHandle::GetKernelCoreType() const
{
    if (!codeLoadSuccess_) {
        return -1;
    }
    return static_cast<int32_t>(metaInfo_.coreType);
}

uint32_t BinHandle::GetIntercoreSync() const
{
    if (!codeLoadSuccess_) {
        return 0;
    }
    return metaInfo_.intercoreSync;
}

uint32_t BinHandle::GetCubeRatio() const
{
    if (!codeLoadSuccess_) {
        return 0;
    }
    return metaInfo_.cubeRatio;
}

uint32_t BinHandle::GetVectorRatio() const
{
    if (!codeLoadSuccess_) {
        return 0;
    }
    return metaInfo_.vectorRatio;
}

const char *BinHandle::GetKernelCompileInfo() const
{
    if (!codeLoadSuccess_) {
        return nullptr;
    }
    return metaInfo_.compileInfo.c_str();
}

bool BinHandle::RegisterBin(const std::string &kernelName)
{
    size_t kernelNum = metaInfo_.kernelList.size();
    MKI_CHECK(kernelNum != 0, "Get Binary Kernel Num empty, op: " << kernelName, return false);

    MkiRtModuleInfo moduleInfo;
    moduleInfo.type = MKIRT_MODULE_OBJECT;
    moduleInfo.version = 0;
    moduleInfo.data = metaInfo_.codeBuf;
    moduleInfo.dataLen = metaInfo_.codeBufLen;
    moduleInfo.magic = metaInfo_.magic;

    if (metaInfo_.kernelList.size() == 1) {
        MKI_LOG(DEBUG) << "single kernel register bin start, opName:" << kernelName;
        int st = MkiRtModuleCreate(&moduleInfo, &moduleHandle_);
        MKI_CHECK(st == MKIRT_SUCCESS, kernelName << " Create RtModule fail, error:" << st, return false);

        MKI_CHECK(moduleHandle_ != nullptr, kernelName << " Create RtModule fail,"
                                                    << " because it return false null handle", return false);

        st = MkiRtModuleBindFunction(moduleHandle_, metaInfo_.kernelList[0].c_str(), &handle_);
        MKI_CHECK(st == MKIRT_SUCCESS, kernelName << " Mki RtModuleGetFunction fail, errCode:" << st
                                                << ", errName:" << MkiRtErrorName(st)
                                                << ", errDesc:" << MkiRtErrorDesc(st), return false);
    } else {
        MKI_LOG(DEBUG) << "multi kernel register bin start, opName:" << kernelName;
        int st = MkiRtRegisterAllFunction(&moduleInfo, &handle_);
        MKI_CHECK(st == MKIRT_SUCCESS, kernelName << " Mki RtRegister AllFunction fail, error:" << st, return false);

        MKI_CHECK(handle_ != nullptr, kernelName << " Mki RtRegister AllFunction fail,"
                                                << " because it return false null handle" << st, return false);
    }
    MKI_LOG(DEBUG) << "kernel register bin finish";
    return true;
}
}
