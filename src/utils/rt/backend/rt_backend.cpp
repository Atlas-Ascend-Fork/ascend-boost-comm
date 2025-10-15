/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/utils/rt/backend/rt_backend.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <acl/acl.h>
#include "mki/utils/assert/assert.h"
#include "mki/utils/log/log.h"
#include "mki/utils/file_system/file_system.h"
#include "mki/utils/rt/backend/help_macro.h"

namespace Mki {
int RtBackend::DeviceGetCount(int32_t *devCount)
{
    MKI_LOG(INFO) << "aclrt GetDeviceCount Func start";

    CHECK_STATUS_RETURN(aclrtGetDeviceCount(reinterpret_cast<uint32_t*>(devCount)));
}

int RtBackend::DeviceGetIds(int32_t *devIds, int32_t devIdNum)
{
    CHECK_STATUS_RETURN(rtGetDeviceIDs(devIds, devIdNum));
}

int RtBackend::DeviceGetCurrent(int32_t *devId)
{
    CHECK_STATUS_RETURN(aclrtGetDevice(devId));
}

int RtBackend::DeviceSetCurrent(int32_t devId)
{
    MKI_LOG(INFO) << "rt SetDevice start, devId:" << devId;
    CHECK_STATUS_WITH_DESC_RETURN(aclrtSetDevice(devId), "aclrt SetDevice");
}

int RtBackend::DeviceResetCurrent(int32_t devId)
{
    MKI_LOG(INFO) << "rt ResetDevice start, devId:" << devId;
    CHECK_STATUS_WITH_DESC_RETURN(aclrtResetDevice(devId), "aclrt DeviceReset");
}

int RtBackend::DeviceSetSocVersion(const char *version)
{
    MKI_LOG(INFO) << "rt SetSocVersion start, version:" << version;
    CHECK_STATUS_WITH_DESC_RETURN(rtSetSocVersion(version), "rt SetSocVersion");
}

int RtBackend::DeviceGetSocVersion(char *version, uint32_t maxLen)
{
    CHECK_STATUS_RETURN(rtGetSocVersion(version, maxLen));
}

int RtBackend::DeviceGetBareTgid(uint32_t *pid)
{
    CHECK_STATUS_RETURN(aclrtDeviceGetBareTgid(reinterpret_cast<int32_t*>(pid)));
}

int RtBackend::DeviceGetPairDevicesInfo(uint32_t devId, uint32_t otherDevId, int32_t infoType, int64_t *val)
{
    CHECK_STATUS_RETURN(rtGetPairDevicesInfo(devId, otherDevId, infoType, val));
}

int RtBackend::StreamCreate(MkiRtStream *stream, int32_t priority)
{
    CHECK_STATUS_WITH_DESC_RETURN(aclrtCreateStream(stream), "aclrt Create Stream");
}

int RtBackend::StreamDestroy(MkiRtStream stream)
{
    CHECK_STATUS_RETURN(aclrtDestroyStream(stream));
}

int RtBackend::StreamSynchronize(MkiRtStream stream)
{
    CHECK_STATUS_RETURN(aclrtSynchronizeStream(stream));
}

int RtBackend::StreamGetId(MkiRtStream stream, int32_t *streamId)
{
    CHECK_STATUS_WITH_DESC_RETURN(rtGetStreamId(stream, streamId), "rt GetStreamId");
}

int RtBackend::MemMallocDevice(void **devPtr, uint64_t size, MkiRtMemType memType)
{
    MKI_LOG(INFO) << "aclrtMalloc start, size:" << size << ", memType:" << memType;
    CHECK_STATUS_WITH_DESC_RETURN(aclrtMalloc(devPtr, size, static_cast<aclrtMemMallocPolicy>(memType)), "aclrtMalloc");
}

int RtBackend::MemFreeDevice(void *devPtr)
{
    CHECK_STATUS_RETURN(aclrtFree(devPtr));
}

int RtBackend::MemMallocHost(void **hostPtr, uint64_t size)
{
    CHECK_STATUS_RETURN(aclrtMallocHost(hostPtr, size));
}

int RtBackend::MemFreeHost(void *hostPtr)
{
    CHECK_STATUS_RETURN(aclrtFreeHost(hostPtr));
}

int RtBackend::MemCopy(void *dst, uint64_t dstLen, const void *srcPtr, uint64_t srcLen, MkiRtMemCopyType copyType)
{
    CHECK_STATUS_RETURN(aclrtMemcpy(dst, dstLen, srcPtr, srcLen, static_cast<aclrtMemcpyKind>(copyType)));
}

int RtBackend::MemCopyAsync(void *dst, uint64_t dstLen, const void *srcPtr, uint64_t srcLen,
                            MkiRtMemCopyType copyType, void *stream)
{
    CHECK_STATUS_RETURN(aclrtMemcpyAsync(dst, dstLen, srcPtr, srcLen, static_cast<aclrtMemcpyKind>(copyType),
                                        static_cast<aclrtStream>(stream)));
}

int RtBackend::MemSetAsync(void *dst, uint64_t destMax, uint32_t value, uint64_t count, void *stream)
{
    CHECK_STATUS_RETURN(aclrtMemsetAsync(dst, destMax, value, count, stream));
}

int RtBackend::IpcSetMemoryName(const void *ptr, uint64_t byteCount, const char *name, uint32_t len)
{
    CHECK_STATUS_RETURN(rtIpcSetMemoryName(ptr, byteCount, name, len));
}

int RtBackend::IpcOpenMemory(void **ptr, const char *name)
{
    CHECK_STATUS_RETURN(rtIpcOpenMemory(ptr, name));
}

int RtBackend::SetIpcMemPid(const char *name, int32_t pid[], int num)
{
    CHECK_STATUS_RETURN(rtSetIpcMemPid(name, pid, num));
}
}
