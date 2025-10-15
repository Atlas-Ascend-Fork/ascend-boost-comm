/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <gtest/gtest.h>

#include <random>
#include <acl/acl.h>

#include "mki/utils/log/log.h"
#include "mki/utils/memset/clear_tensors.h"
#include "mki/utils/rt/rt.h"

namespace Mki {
constexpr float MEMSET_FACTOR = 0.7; // set 70% to zero
constexpr uint64_t BLOCK_LEN = 32;
constexpr int START = 1;
constexpr int END = 4096 * 1024; // 4mb

uint64_t GenRandomLen()
{
    return (rand() % (END - START)) + START;
}

void MallocDevice(uint8_t **p, uint64_t len)
{
    MKI_LOG(INFO) << "malloc " << len << " bytes for memset";
    std::vector<uint8_t> src(len, 0x78);
    void *vp = nullptr;
    int st = MkiRtMemMallocDevice(&vp, len, MKIRT_MEM_DEFAULT);
    ASSERT_EQ(st, MKIRT_SUCCESS);
    st = MkiRtMemCopy(vp, len, src.data(), len, MKIRT_MEMCOPY_HOST_TO_DEVICE);
    ASSERT_EQ(st, MKIRT_SUCCESS);
    *p = static_cast<uint8_t *>(vp);
}

void CompareResult(uint8_t *p, uint64_t len)
{
    MKI_LOG(INFO) << "compare result " << len;
    std::vector<uint8_t> dst(len, 0xb3);
    int st = MkiRtMemCopy(dst.data(), len, static_cast<void *>(p), len, MKIRT_MEMCOPY_DEVICE_TO_HOST);
    ASSERT_EQ(st, MKIRT_SUCCESS);
    uint64_t memsetLen = ((uint64_t)(len * MEMSET_FACTOR) + BLOCK_LEN - 1) / BLOCK_LEN * BLOCK_LEN;
    for (uint64_t i = 0; i < memsetLen && i < len; i++) {
        if (dst[i] != 0) {
            MKI_LOG(ERROR) << "dst[" << i << "] 0 error, actual " << (int)dst[i];
        }
        ASSERT_EQ(dst[i], 0);
    }
    for (uint64_t i = memsetLen; i < len; i++) {
        if (dst[i] != 0x78) {
            MKI_LOG(ERROR) << "dst[" << i << "] 78 error, actual " << (int)dst[i];
        }
        ASSERT_EQ(dst[i], 0x78);
    }
    MkiRtMemFreeDevice(static_cast<void *>(p));
}

#define GEN_MEMSET_TENSOR(idx) \
    uint64_t len##idx = GenRandomLen(); \
    uint64_t lent##idx = len##idx * MEMSET_FACTOR; \
    uint8_t *tensor##idx = nullptr; \
    MallocDevice(&tensor##idx, len##idx); \
    data.push_back(static_cast<void *>(tensor##idx)); \
    memsetInfo[idx].argIdx = idx; \
    memsetInfo[idx].size = lent##idx

TEST(TestMemset, TestMemsetOp)
{
    int st = MkiRtDeviceSetCurrent(0);
    ASSERT_EQ(st, MKIRT_SUCCESS);
    MkiRtStream stream = nullptr;
    st = MkiRtStreamCreate(&stream, 0);
    ASSERT_EQ(st, MKIRT_SUCCESS);

    srand(0);
    std::vector<void *> data;
    MiniVector<KernelInfo::MemsetInfo> memsetInfo;
    memsetInfo.resize(8);
    GEN_MEMSET_TENSOR(0);
    GEN_MEMSET_TENSOR(1);
    GEN_MEMSET_TENSOR(2);
    GEN_MEMSET_TENSOR(3);
    GEN_MEMSET_TENSOR(4);
    GEN_MEMSET_TENSOR(5);
    GEN_MEMSET_TENSOR(6);
    GEN_MEMSET_TENSOR(7);

    auto status = ClearTensors(data.data(), data.size(), memsetInfo, stream);
    ASSERT_TRUE(status.Ok());
    st = MkiRtStreamSynchronize(stream);
    ASSERT_EQ(st, MKIRT_SUCCESS);

    CompareResult(tensor0, len0);
    CompareResult(tensor1, len1);
    CompareResult(tensor2, len2);
    CompareResult(tensor3, len3);
    CompareResult(tensor4, len4);
    CompareResult(tensor5, len5);
    CompareResult(tensor6, len6);
    CompareResult(tensor7, len7);
}
}
