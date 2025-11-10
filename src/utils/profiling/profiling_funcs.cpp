/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <thread>
#include <mutex>
#include <dlfcn.h>
#include <mki/utils/singleton/singleton.h>
#include <mki/utils/log/log.h>
#include "mki/utils/profiling/prof_api.h"
#include "mki/utils/profiling/profiling_funcs.h"

namespace Mki {

bool ProfilingFuncs::isProfilingLevel0Enable_ = false;
bool ProfilingFuncs::isProfilingLevel1Enable_ = false;

static ProfilingFuncs g_profiling = Mki::GetSingleton<ProfilingFuncs>();

void ProfilingFuncs::SetProfilingLevel0Status(bool status) const
{
    isProfilingLevel0Enable_ = status;
}

void ProfilingFuncs::SetProfilingLevel1Status(bool status) const
{
    isProfilingLevel1Enable_ = status;
}

bool ProfilingFuncs::GetProfilingLevel0Status() const
{
    return isProfilingLevel0Enable_;
};

bool ProfilingFuncs::GetProfilingLevel1Status() const
{
    return isProfilingLevel1Enable_;
}

ProfilingFuncs::ProfilingFuncs() noexcept
{
    MKI_LOG(INFO) << "MsprofRegisterCallback start!";
    if (MsprofRegisterCallback(0, ProfilingFuncs::MkiProfCommandHandle) != 0) {
        MKI_LOG(ERROR) << "MsprofRegisterCallback fail!";
    }
}

ProfilingFuncs::~ProfilingFuncs() {}

int32_t ProfilingFuncs::ProfReportApi(uint32_t agingFlag, const MsProfApi *api) const
{
    MKI_LOG(INFO) << "ProfReportApi start!";
    return MsprofReportApi(agingFlag, api);
}

int32_t ProfilingFuncs::ProfReportCompactInfo(uint32_t agingFlag, const void *data, uint32_t length) const
{
    MKI_LOG(INFO) << "ProfReportCompactInfo start!";
    if (isProfilingLevel1Enable_) {
        return MsprofReportCompactInfo(agingFlag, data, length);
    }
    return 0;
}

int32_t ProfilingFuncs::ProfReportAdditionalInfo(uint32_t agingFlag, const void *data, uint32_t length) const
{
    MKI_LOG(INFO) << "ProfReportAdditionalInfo start!";
    return MsprofReportAdditionalInfo(agingFlag, data, length);
}

int32_t ProfilingFuncs::ProfReportTypeInfo(uint16_t level, uint32_t typeId, std::string typeName) const
{
    MKI_LOG(INFO) << "ProfReportTypeInfo start!";
    return MsprofRegTypeInfo(level, typeId, typeName.c_str());
}

uint64_t ProfilingFuncs::ProfSysCycleTime() const
{
    return MsprofSysCycleTime();
}

uint64_t ProfilingFuncs::ProfGetHashId(const char *hashInfo, size_t length) const
{
    return MsprofGetHashId(hashInfo, length);
}

uint64_t ProfilingFuncs::ProfGetHashId(const char *hashInfo, size_t length, void const *key)
{
    auto it = kernelNameHashCache_.find(key);
    if (it == kernelNameHashCache_.end()) {
        auto hashId = MsprofGetHashId(hashInfo, length);
        kernelNameHashCache_.insert({key, hashId});
        return hashId;
    }

    return it->second;
}

int32_t ProfilingFuncs::MkiProfCommandHandle(uint32_t type, void *data, uint32_t len)
{
    MKI_LOG(DEBUG) << "Step in MkiProfCommandHandle!";
    if (data == nullptr) {
        MKI_LOG(ERROR) << "MkiProfCommandHandle failed! data is nullptr!";
        return PROFILING_REPORT_FAILED;
    }
    if (type != PROF_CTRL_SWITCH) {
        MKI_LOG(ERROR) << "MkiProfCommandHandle failed! ProfCtrlType is not correct!";
        return PROFILING_REPORT_FAILED;
    }

    if (len < sizeof(MsprofCommandHandle)) {
        MKI_LOG(ERROR) << "MkiProfCommandHandle failed! dataSize is not correct!";
        return PROFILING_REPORT_FAILED;
    }
    MsprofCommandHandle *profilerConfig = static_cast<MsprofCommandHandle*>(data);
    const uint64_t profSwitch = profilerConfig->profSwitch;
    const uint32_t profType = profilerConfig->type;

    if (profType == PROF_COMMANDHANDLE_TYPE_START) {
        MKI_LOG(INFO) << "Open Profiling Switch";
        if ((profSwitch & PROF_TASK_TIME_L0) != PROF_CTRL_INVALID) {
            isProfilingLevel0Enable_ = true;
            MKI_LOG(INFO) << "Profiling Level0 Enable";
        }
        if ((profSwitch & PROF_TASK_TIME_L1) != PROF_CTRL_INVALID) {
            isProfilingLevel1Enable_ = true;
            MKI_LOG(INFO) << "Profiling Level1 Enable";
        }
    }
    if (profType == PROF_COMMANDHANDLE_TYPE_STOP) {
        MKI_LOG(INFO) << "Close Profiling Switch";
        isProfilingLevel0Enable_ = false;
        isProfilingLevel1Enable_ = false;
    }

    return PROFILING_REPORT_SUCCESS;
}
} // namespace Mki
