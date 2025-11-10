/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#ifndef MKI_UTILS_RT_MODULE_MODULE_H
#define MKI_UTILS_RT_MODULE_MODULE_H

#include "mki/utils/rt/base/types.h"

#ifdef __cplusplus
extern "C" {
#endif
namespace Mki {
int MkiRtModuleCreate(MkiRtModuleInfo *moduleInfo, MkiRtModule *module);
int MkiRtModuleCreateFromFile(const char *moduleFilePath, MkiRtModuleType type, int version, MkiRtModule *module);
int MkiRtModuleDestory(MkiRtModule *module);
int MkiRtModuleBindFunction(MkiRtModule module, const char *funcName, void *func);
int MkiRtRegisterAllFunction(MkiRtModuleInfo *moduleInfo, void **handle);
int MkiRtFunctionLaunch(const void *func, const MkiRtKernelParam *launchParam, MkiRtStream stream);
int MkiRtFunctionLaunchWithHandle(void *handle, const MkiRtKernelParam *launchParam, MkiRtStream stream,
    const RtTaskCfgInfoT *cfgInfo);
int MkiRtFunctionLaunchWithFlag(const void *func, const MkiRtKernelParam *launchParam, MkiRtStream stream,
    const RtTaskCfgInfoT *cfgInfo);
int MkiRtAicpuFunctionLaunchWithFlag(const RtKernelLaunchNamesT *launchNames, const MkiRtAicpuKernelParam *param,
    MkiRtStream stream);
int MkiRtAicpuFunctionLaunchExWithArgs(const char * const opName, const MkiRtAicpuKernelParam *param,
    MkiRtStream stream);
}
#ifdef __cplusplus
}
#endif
#endif
