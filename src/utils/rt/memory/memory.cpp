/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/utils/rt/memory/memory.h"
#include "mki/utils/rt/backend/backend_factory.h"

namespace Mki {
int MkiRtMemMallocDevice(void **devPtr, uint64_t size, MkiRtMemType memType)
{
    return BackendFactory::GetBackend()->MemMallocDevice(devPtr, size, memType);
}

int MkiRtMemFreeDevice(void *devPtr) { return BackendFactory::GetBackend()->MemFreeDevice(devPtr); }

int MkiRtMemMallocHost(void **hostPtr, uint64_t size)
{
    return BackendFactory::GetBackend()->MemMallocHost(hostPtr, size);
}

int MkiRtMemFreeHost(void *hostPtr) { return BackendFactory::GetBackend()->MemFreeHost(hostPtr); }

int MkiRtMemCopy(void *dstPtr, uint64_t dstLen, const void *srcPtr, uint64_t srcLen, MkiRtMemCopyType copyType)
{
    return BackendFactory::GetBackend()->MemCopy(dstPtr, dstLen, srcPtr, srcLen, copyType);
}

int MkiRtMemCopyAsync(void *dstPtr, uint64_t dstLen, const void *srcPtr, uint64_t srcLen, MkiRtMemCopyType copyType,
                      void *stream)
{
    return BackendFactory::GetBackend()->MemCopyAsync(dstPtr, dstLen, srcPtr, srcLen, copyType, stream);
}

int MkiRtMemSetAsync(void *dstPtr, uint64_t destMax, uint32_t value, uint64_t count, void *stream)
{
    return BackendFactory::GetBackend()->MemSetAsync(dstPtr, destMax, value, count, stream);
}

int MkiRtIpcSetMemoryName(const void *ptr, uint64_t byteCount, const char *name, uint32_t len)
{
    return BackendFactory::GetBackend()->IpcSetMemoryName(ptr, byteCount, name, len);
}

int MkiRtIpcOpenMemory(void **ptr, const char *name)
{
    return BackendFactory::GetBackend()->IpcOpenMemory(ptr, name);
}

int MkiRtSetIpcMemPid(const char *name, int32_t pid[], int num)
{
    return BackendFactory::GetBackend()->SetIpcMemPid(name, pid, num);
}
}
