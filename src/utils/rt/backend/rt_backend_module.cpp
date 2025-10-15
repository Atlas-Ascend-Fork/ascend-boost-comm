/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <securec.h>
#include "mki/utils/assert/assert.h"
#include "mki/utils/file_system/file_system.h"
#include "mki/utils/log/log.h"
#include "mki/utils/rt/backend/rt_backend.h"
#include "mki/utils/rt/backend/help_macro.h"
#include "mki/utils/rt/module/module.h"
#include "mki/utils/rt/rt.h"

namespace {
const std::vector<uint32_t> MAGIC_LIST = {
    MKIRT_DEV_BINARY_MAGIC_ELF,
    MKIRT_DEV_BINARY_MAGIC_ELF_AIVEC,
    MKIRT_DEV_BINARY_MAGIC_ELF_AICUBE
};
}
namespace Mki {
struct MkiRtModuleProxy {
    MkiRtModuleType type = MKIRT_MODULE_OBJECT;
    uint32_t version = 0;
    void *rtModule = nullptr;
};

int RtBackend::ModuleCreate(MkiRtModuleInfo *moduleInfo, MkiRtModule *module)
{
    if (moduleInfo == nullptr) {
        MKI_LOG(ERROR) << "moduleInfo is nullptr";
        return MKIRT_ERROR_NOT_INITIALIZED;
    }
    if (module == nullptr) {
        MKI_LOG(ERROR) << "module is nullptr";
        return MKIRT_ERROR_NOT_INITIALIZED;
    }
    MkiRtModuleProxy *moduleProxy = new MkiRtModuleProxy;
    moduleProxy->type = moduleInfo->type;
    moduleProxy->version = moduleInfo->version;
    *module = moduleProxy;

    const uint32_t maxLen = 100;
    std::string getVersion;
    auto ret = MkiRtDeviceGetSocVersion(getVersion, maxLen);
    if (ret != MKIRT_SUCCESS) {
        MKI_LOG(ERROR) << "Failed to get soc version";
        return ret;
    }
    std::string socVersion(getVersion);
    MKI_LOG(DEBUG) << "Soc version: " << socVersion;

    if (moduleInfo->type == MKIRT_MODULE_OBJECT) {
        RtDevBinaryT devBin;
        if (std::find(MAGIC_LIST.begin(), MAGIC_LIST.end(), moduleInfo->magic) == MAGIC_LIST.end()) {
            MKI_LOG(ERROR) << "Invalid magic";
            return MKIRT_ERROR_PARA_CHECK_FAIL;
        }
        devBin.magic = moduleInfo->magic;
        devBin.version = moduleInfo->version;
        devBin.data = moduleInfo->data;
        devBin.length = moduleInfo->dataLen;
        MKI_LOG(DEBUG) << "RtDev BinaryRegister start, len:" << moduleInfo->dataLen;
        CHECK_STATUS_WITH_DESC_RETURN(rtDevBinaryRegister(&devBin, &moduleProxy->rtModule), "RtDev BinaryRegister");
    } else {
        return MKIRT_ERROR_NOT_IMPLMENT;
    }
    return MKIRT_SUCCESS;
}

int RtBackend::ModuleCreateFromFile(const char *moduleFilePath, MkiRtModuleType type, int version, MkiRtModule *module)
{
    CHECK_FUN_PARA_RETURN(module);
    std::string realPath = FileSystem::PathCheckAndRegular(moduleFilePath);
    MKI_CHECK(!realPath.empty(), "moduleFilePath is null", return MKIRT_ERROR_PARA_CHECK_FAIL);
    std::ifstream fd(realPath, std::ios::binary);
    if (!fd.is_open()) {
        return MKIRT_ERROR_OPEN_BIN_FILE_FAIL;
    }

    fd.seekg(0, std::ios::end);
    size_t fileSize = static_cast<size_t>(fd.tellg());
    if (fileSize > static_cast<size_t>(MAX_FILE_SIZE)) {
        MKI_LOG(ERROR) << "max file size exceeded";
        return MKIRT_ERROR_NOT_IMPLMENT;
    }
    fd.seekg(0, std::ios::beg);
    std::vector<char> fileBuf(fileSize, 0);
    fd.read(fileBuf.data(), fileSize);

    MkiRtModuleInfo moduleInfo;
    moduleInfo.type = type;
    moduleInfo.version = static_cast<uint32_t>(version);
    moduleInfo.data = fileBuf.data();
    moduleInfo.dataLen = fileSize;

    return ModuleCreate(&moduleInfo, module);
}

int RtBackend::ModuleDestory(MkiRtModule *module)
{
    int st = MKIRT_SUCCESS;
    if (module == nullptr) {
        return st;
    }

    MkiRtModuleProxy *moduleProxy = static_cast<MkiRtModuleProxy *>(*module);
    if (moduleProxy != nullptr) {
        if (moduleProxy->rtModule != nullptr) {
            st = ModuleDestoryRtModule(moduleProxy->rtModule);
        }

        delete moduleProxy;
        moduleProxy = nullptr;
        *module = nullptr;
    }

    return st;
}

int RtBackend::ModuleBindFunction(MkiRtModule module, const char *funcName, void *func)
{
    CHECK_FUN_PARA_RETURN(module);
    if (funcName == nullptr) {
        MKI_LOG(ERROR) << "funcName is nullptr:";
        return MKIRT_ERROR_NOT_INITIALIZED;
    }
    const char *stubName = funcName;
    const void *kernelInfoExit = funcName;
    uint32_t funcMode = 0;
    MkiRtModuleProxy *moduleProxy = static_cast<MkiRtModuleProxy *>(module);
    MKI_LOG(DEBUG) << "RtFunction Register start, module:" << moduleProxy->rtModule << ", stubFunc:" << func
                  << ", subName:" << funcName;
    CHECK_STATUS_WITH_DESC_RETURN(rtFunctionRegister(moduleProxy->rtModule, func, stubName, kernelInfoExit, funcMode),
                                  "RtFunction Register");
}

int RtBackend::RegisterAllFunction(MkiRtModuleInfo *moduleInfo, void **handle)
{
    CHECK_FUN_PARA_RETURN(moduleInfo);
    RtDevBinaryT devBin;
    devBin.magic = moduleInfo->magic;
    devBin.version = moduleInfo->version;
    devBin.data = moduleInfo->data;
    devBin.length = moduleInfo->dataLen;
    MKI_LOG(DEBUG) << "Rt Register AllKernel start, len: " << devBin.length << ", magic: " << devBin.magic;
    CHECK_STATUS_WITH_DESC_RETURN(rtRegisterAllKernel(&devBin, handle), "Rt Register AllKernel");
}

int RtBackend::FunctionLaunch(const void *func, const MkiRtKernelParam *param, MkiRtStream stream)
{
    CHECK_FUN_PARA_RETURN(param);
    CHECK_STATUS_WITH_DESC_RETURN(rtKernelLaunch(func, param->blockDim, param->args, param->argSize, nullptr, stream),
                                  "rt KernelLaunch");
}

int RtBackend::FunctionLaunchWithHandle(void *handle, const MkiRtKernelParam *param, MkiRtStream stream,
    const RtTaskCfgInfoT *cfgInfo)
{
    /* runtime允许cfgInfo为nullptr，此处不校验 */
    CHECK_FUN_PARA_RETURN(param);
    CHECK_FUN_PARA_RETURN(param->argsEx);

    CHECK_STATUS_WITH_DESC_RETURN(rtKernelLaunchWithHandleV2(handle, param->tilingId,
                                                             param->blockDim, param->argsEx,
                                                             nullptr, stream, cfgInfo),
                                  "rt KernelLaunch With Handle");
}

int RtBackend::FunctionLaunchWithFlag(const void *func, const MkiRtKernelParam *param, MkiRtStream stream,
    const RtTaskCfgInfoT *cfgInfo)
{
    /* runtime允许cfgInfo为nullptr，此处不校验 */
    CHECK_FUN_PARA_RETURN(param);
    CHECK_FUN_PARA_RETURN(param->argsEx);
    CHECK_STATUS_WITH_DESC_RETURN(rtKernelLaunchWithFlagV2(func, param->blockDim,
                                                           param->argsEx, nullptr,
                                                           stream, 0, cfgInfo),
                                  "rt KernelLaunch With Flag");
}

int RtBackend::AicpuFunctionLaunchWithFlag(const RtKernelLaunchNamesT *launchNames, const MkiRtAicpuKernelParam *param,
    MkiRtStream stream)
{
    CHECK_FUN_PARA_RETURN(launchNames);
    CHECK_FUN_PARA_RETURN(param);
    CHECK_FUN_PARA_RETURN(param->argsEx);
    CHECK_STATUS_WITH_DESC_RETURN(rtAicpuKernelLaunchWithFlag(launchNames, param->blockDim,
                                                              param->argsEx, nullptr,
                                                              stream, 0U),
                                  "rt Aicpu KernelLaunch With Flag");
}

int RtBackend::AicpuFunctionLaunchExWithArgs(const char * const opName, const MkiRtAicpuKernelParam *param,
    MkiRtStream stream)
{
    CHECK_FUN_PARA_RETURN(opName);
    CHECK_FUN_PARA_RETURN(param);
    CHECK_FUN_PARA_RETURN(param->aicpuArgsEx);
    constexpr uint32_t kernelTypeAicpuCustom = 4;
    constexpr uint32_t rtKernelCustomAicpu = 8;
    CHECK_STATUS_WITH_DESC_RETURN(rtAicpuKernelLaunchExWithArgs(kernelTypeAicpuCustom, opName, param->blockDim,
                                                                param->aicpuArgsEx, nullptr,
                                                                stream, rtKernelCustomAicpu),
                                  "rt Aicpu KernelLaunch Ex With Args");
}

int RtBackend::GetC2cCtrlAddr(uint64_t *addr, uint32_t *len)
{
    CHECK_STATUS_WITH_DESC_RETURN(rtGetC2cCtrlAddr(addr, len), "rt Get C2cCtrl Addr");
}

int RtBackend::CtxGetOverflowAddr(void **addr)
{
    CHECK_STATUS_WITH_DESC_RETURN(rtCtxGetOverflowAddr(addr), "rt Get Overflow Addr");
}

int RtBackend::ModuleDestoryRtModule(void *rtModule) const
{
    if (rtModule == nullptr) {
        return MKIRT_SUCCESS;
    }
    CHECK_STATUS_RETURN(rtDevBinaryUnRegister(rtModule));
}
}
