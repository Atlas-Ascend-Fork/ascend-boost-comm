/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#include "mki/utils/platform/platform_info.h"
#include <mutex>
#include "mki/utils/assert/assert.h"
#include "mki/utils/file_system/file_system.h"
#include "mki/utils/log/log.h"
#include "mki/utils/rt/rt.h"
#include "mki/utils/platform/platform_manager.h"
#include "mki/utils/rt/resource/resource.h"
#include "mki/types.h"

namespace Mki {
constexpr uint32_t MAX_CORE_NUM = 128;
PlatformInfo::PlatformInfo() { Init(); }

PlatformInfo &PlatformInfo::Instance()
{
    static PlatformInfo platformInfo;
    return platformInfo;
}

void PlatformInfo::Init()
{
    if (inited_) {
        return;
    }

    const uint32_t maxLen = 100;
    std::string version;
    if (MkiRtDeviceGetSocVersion(version, maxLen) != MKIRT_SUCCESS) {
        MKI_LOG(ERROR) << "Failed to get soc version";
        return;
    }
    MKI_LOG(INFO) << "PlatformInfo get soc version: " << version;

    Mki::PlatformManager &platformManager = Mki::PlatformManager::Instance();
    MKI_CHECK(platformManager.InitializePlatformManager() == PLATFORM_SUCCESS,
                 "Initialize platform manager failed", return);
    MKI_CHECK(platformManager.GetPlatformConfigs(version, platformConfigs_) == PLATFORM_SUCCESS,
                 "Get platform Information failed", return);

    const std::unordered_map<std::string, std::pair<PlatformType, std::string>> supportedPlatform = {
        {"Ascend310P",   {PlatformType::ASCEND_310P, "ascend310p"}},
        {"Ascend310B",   {PlatformType::ASCEND_310B, "ascend310b"}},
        {"Ascend910",    {PlatformType::ASCEND_910A, "ascend910"}},
        {"Ascend910B",   {PlatformType::ASCEND_910B, "ascend910b"}},
        {"Ascend910_93", {PlatformType::ASCEND_910B, "ascend910b"}},
        {"Ascend910_95", {PlatformType::ASCEND_910_95, "ascend910_95"}}};

    (void)platformConfigs_.GetPlatformSpec("version", "Short_SoC_version", platformName_);
    const auto it = supportedPlatform.find(platformName_);
    if (it == supportedPlatform.cend()) {
        MKI_LOG(ERROR) << "Unsupport soc";
        platformName_ = "unrecognized";
        return;
    }

    platformType_ = it->second.first;
    platformName_ = it->second.second;
    inited_ = true;
}

bool PlatformInfo::Inited() const { return inited_; }

uint32_t PlatformInfo::GetCoreNum(CoreType type)
{
    uint32_t coreNum = 0;
    int8_t resType;
    if (type == CoreType::CORE_TYPE_VECTOR) {
        resType = 1;
    } else {
        resType = 0;
    }
    int getResRet = GetResInCurrentThread(resType, coreNum);
    if (getResRet == NO_ERROR) {
        if (coreNum == 0 || coreNum > MAX_CORE_NUM) {
            MKI_LOG(ERROR) << "core_num is out of range : " << coreNum;
            return 1;
        } else {
            return coreNum;
        }
    }

    if (platformType_ == PlatformType::ASCEND_910B || platformType_ == PlatformType::ASCEND_910_95) {
        switch (type) {
            case CoreType::CORE_TYPE_VECTOR:
                coreNum = platformConfigs_.GetCoreNumByType("VectorCore");
                break;
            default:
                coreNum = platformConfigs_.GetCoreNumByType("AiCore");
                break;
        }
    } else {
        coreNum = platformConfigs_.GetCoreNumByType("AiCore");
    }
    if (coreNum == 0 || coreNum > MAX_CORE_NUM) {
        MKI_LOG(ERROR) << "core_num is out of range : " << coreNum;
        return 1;
    }
    MKI_FLOG_INFO("Platform get core num %u, type %d", coreNum, type);
    return coreNum;
}

uint64_t PlatformInfo::GetL2Size()
{
    uint64_t memSize = 0;
    platformConfigs_.GetLocalMemSize(LocalMemType::L2, memSize);
    MKI_LOG(INFO) << "PlatformInfo get l2 size " << memSize;
    return memSize;
}

uint64_t PlatformInfo::GetL1Size()
{
    uint64_t memSize = 0;
    platformConfigs_.GetLocalMemSize(LocalMemType::L1, memSize);
    MKI_LOG(INFO) << "PlatformInfo get l1 size " << memSize;
    return memSize;
}

uint64_t PlatformInfo::GetL0ASize()
{
    uint64_t memSize = 0;
    platformConfigs_.GetLocalMemSize(LocalMemType::L0_A, memSize);
    MKI_LOG(INFO) << "PlatformInfo get l0a size " << memSize;
    return memSize;
}

uint64_t PlatformInfo::GetL0BSize()
{
    uint64_t memSize = 0;
    platformConfigs_.GetLocalMemSize(LocalMemType::L0_B, memSize);
    MKI_LOG(INFO) << "PlatformInfo get l0b size " << memSize;
    return memSize;
}

uint64_t PlatformInfo::GetL0CSize()
{
    uint64_t memSize = 0;
    platformConfigs_.GetLocalMemSize(LocalMemType::L0_C, memSize);
    MKI_LOG(INFO) << "PlatformInfo get l0c size " << memSize;
    return memSize;
}

uint64_t PlatformInfo::GetUbSize()
{
    uint64_t memSize = 0;
    platformConfigs_.GetLocalMemSize(LocalMemType::UB, memSize);
    MKI_LOG(INFO) << "PlatformInfo get ub size " << memSize;
    return memSize;
}

bool PlatformInfo::SupportL0c2out() { return GetAicoreIntrinsic("Intrinsic_fix_pipe_l0c2out"); }

bool PlatformInfo::GetAicoreIntrinsic(const std::string &intrinsicName)
{
    std::string val;
    (void)platformConfigs_.GetPlatformSpec("AICoreintrinsicDtypeMap", intrinsicName, val);
    if (!val.empty()) {
        MKI_LOG(INFO) << "PlatformInfo get aicore intrinsic " << intrinsicName << " = " << val;
        return true;
    } else {
        MKI_LOG(INFO) << "PlatformInfo get no aicore intrinsic " << intrinsicName;
        return false;
    }
}

PlatformType PlatformInfo::GetPlatformType() const { return platformType_; }
std::string PlatformInfo::GetPlatformName() const { return platformName_; }
} // namespace Mki
