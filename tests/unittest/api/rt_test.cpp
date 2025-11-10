/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <gtest/gtest.h>
#include "mki/utils/rt/rt.h"

namespace Mki {
constexpr int DEV_COUNT = 56;

TEST(RtTest, Base)
{
    int major = 0;
    int minor = 0;
    int patch = 0;
    int &major1 = major;
    int &minor1 = minor;
    int &patch1 = patch;
    MkiRtGetVersion(major1, minor1, patch1);
    EXPECT_NE(major1, 0);
    std::cout << MkiRtGetBuildInfo();
    std::cout << MkiRtErrorName(MKIRT_ERROR_NOT_INITIALIZED);
    std::cout << MkiRtErrorDesc(MKIRT_ERROR_NOT_INITIALIZED);
    std::cout << MkiRtErrorName(MKIRT_ERROR_NOT_IMPLMENT);
    std::cout << MkiRtErrorDesc(MKIRT_ERROR_NOT_IMPLMENT);
}

TEST(Rt, Device)
{
    int32_t devCount = 0;
    MkiRtDeviceGetCount(&devCount);
    int32_t devId = 0;
    MkiRtDeviceGetCurrent(&devId);
    int32_t devIds[DEV_COUNT];
    MkiRtDeviceGetIds(devIds, DEV_COUNT);
}

TEST(Rt, Mem)
{
    ASSERT_EQ(MkiRtDeviceSetCurrent(0), 0);
    void *hostPtr = nullptr;
    uint64_t size = 10;
    MkiRtMemMallocHost(&hostPtr, size);
    ASSERT_NE(hostPtr, nullptr);
    MkiRtMemFreeHost(hostPtr);
    ASSERT_EQ(MkiRtDeviceResetCurrent(0), 0);
}

TEST(Rt, Stream)
{
    ASSERT_EQ(MkiRtDeviceSetCurrent(0), 0);
    MkiRtStream stream = nullptr;
    MkiRtStreamCreate(&stream, 0);
    ASSERT_NE(stream, nullptr);
    int32_t streamId = 0;
    ASSERT_EQ(MkiRtStreamGetId(stream, &streamId), 0);
    std::cout << "streamId: " << streamId << std::endl;
    MkiRtStreamDestroy(stream);
    ASSERT_EQ(MkiRtDeviceResetCurrent(0), 0);
}
} // namespace Mki
