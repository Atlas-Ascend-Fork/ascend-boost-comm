/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 1.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#include "mki/utils/platform/platform_configs.h"
#include <mutex>
#include "mki/utils/dl/dl.h"
#include "mki/utils/env/env.h"
#include "mki/utils/log/log.h"
#include "mki/types.h"
#include "mki/utils/rt/resource/resource.h"

namespace Mki {
constexpr uint32_t MAX_CORE_NUM = 128;
void PlatformConfigs::SetPlatformSpec(const std::string &label, std::map<std::string, std::string> &res)
{
    platformSpecMap_[label] = res;
}

bool PlatformConfigs::GetPlatformSpec(const std::string &label, const std::string &key, std::string &value)
{
    const auto itLabel = platformSpecMap_.find(label);
    if (itLabel == platformSpecMap_.cend()) {
        return false;
    }

    auto itKey = itLabel->second.find(key);
    if (itKey == itLabel->second.end()) {
        return false;
    }

    value = itKey->second;
    return true;
}

bool PlatformConfigs::GetPlatformSpec(const std::string &label, std::map<std::string, std::string> &res)
{
    auto itLabel = platformSpecMap_.find(label);
    if (itLabel == platformSpecMap_.end()) {
        return false;
    }

    res = itLabel->second;
    return true;
}

uint32_t PlatformConfigs::GetCoreNumByType(const std::string &coreType)
{
    uint32_t coreNum = 0;
    int8_t resType = coreType == "VectorCore" ? 1 : 0;
    int getResRet = GetResInCurrentThread(resType, coreNum);
    
    if (getResRet == NO_ERROR) {
        if (coreNum == 0 || coreNum > MAX_CORE_NUM) {
            MKI_LOG(ERROR) << "core_num is out of range : " << coreNum;
            return 1;
        } else {
            return coreNum;
        }
    }

    std::string coreNumStr;
    std::string coreTypeStr = coreType == "VectorCore" ? "vector_core_cnt" : "ai_core_cnt";
    (void)GetPlatformSpec("SoCInfo", coreTypeStr, coreNumStr);
    MKI_LOG(DEBUG) << "Get PlatformConfigs::core_num_ to " << coreTypeStr << ": " << coreNumStr;
    if (coreNumStr.empty()) {
        MKI_LOG(ERROR) << "CoreNumStr is empty!";
        return 1;
    } else {
        coreNum = std::strtoul(coreNumStr.c_str(), nullptr, 10); // 10 进制
    }
    if (coreNum == 0 || coreNum > MAX_CORE_NUM) {
        MKI_LOG(ERROR) << "core_num is out of range : " << coreNum;
        return 1;
    }
    return coreNum;
}

void PlatformConfigs::SetFixPipeDtypeMap(const std::map<std::string, std::vector<std::string>> &fixpipeDtypeMap)
{
    fixpipeDtypeMap_ = fixpipeDtypeMap;
}

void PlatformConfigs::SetAICoreIntrinsicDtype(std::map<std::string, std::vector<std::string>> &intrinsicDtypes)
{
    aiCoreIntrinsicDtypeMap_ = intrinsicDtypes;
}

void PlatformConfigs::SetVectorCoreIntrinsicDtype(std::map<std::string, std::vector<std::string>> &intrinsicDtypes)
{
    vectorCoreIntrinsicDtypeMap_ = intrinsicDtypes;
}

const std::map<std::string, std::vector<std::string>> &PlatformConfigs::GetFixPipeDtypeMap()
{
    return fixpipeDtypeMap_;
}

std::map<std::string, std::vector<std::string>> &PlatformConfigs::GetAICoreIntrinsicDtype()
{
    return aiCoreIntrinsicDtypeMap_;
}

std::map<std::string, std::vector<std::string>> &PlatformConfigs::GetVectorCoreIntrinsicDtype()
{
    return vectorCoreIntrinsicDtypeMap_;
}

const std::map<std::string, std::map<std::string, std::string>> &PlatformConfigs::GetPlatformSpecMap()
{
    return platformSpecMap_;
}

void PlatformConfigs::GetLocalMemSize(const LocalMemType &memType, uint64_t &size)
{
    std::string sizeStr;
    switch (memType) {
        case LocalMemType::L0_A: {
            (void)GetPlatformSpec("AICoreSpec", "l0_a_size", sizeStr);
            break;
        }
        case LocalMemType::L0_B: {
            (void)GetPlatformSpec("AICoreSpec", "l0_b_size", sizeStr);
            break;
        }
        case LocalMemType::L0_C: {
            (void)GetPlatformSpec("AICoreSpec", "l0_c_size", sizeStr);
            break;
        }
        case LocalMemType::L1: {
            (void)GetPlatformSpec("AICoreSpec", "l1_size", sizeStr);
            break;
        }
        case LocalMemType::L2: {
            (void)GetPlatformSpec("SoCInfo", "l2_size", sizeStr);
            break;
        }
        case LocalMemType::UB: {
            (void)GetPlatformSpec("AICoreSpec", "ub_size", sizeStr);
            break;
        }
        case LocalMemType::HBM: {
            (void)GetPlatformSpec("SoCInfo", "memory_size", sizeStr);
            break;
        }
        default: {
            break;
        }
    }

    if (sizeStr.empty()) {
        size = 0;
    } else {
        try {
            size = static_cast<uint64_t>(std::stoll(sizeStr.c_str()));
        } catch (const std::invalid_argument &e) {
            size = 0;
        } catch (const std::out_of_range &e) {
            size = 0;
        }
    }
}
} // namespace Mki