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
#include "mki/run_info.h"

namespace {
class RunInfoTest : public::testing::Test {
protected:
    Mki::RunInfo runInfo;
};

TEST_F(RunInfoTest, ResetTest)
{
    uint8_t addr = 1;
    runInfo.SetScratchDeviceAddr(&addr);
    runInfo.SetTilingDeviceAddr(&addr);
    ASSERT_NE(runInfo.GetScratchDeviceAddr(), nullptr);
    runInfo.Reset();
    EXPECT_EQ(runInfo.GetScratchDeviceAddr(), nullptr);
    EXPECT_EQ(runInfo.GetTilingDeviceAddr(), nullptr);
}

TEST_F(RunInfoTest, SetStreamTest)
{
    int stream = 42;
    runInfo.SetStream(&stream);
    EXPECT_EQ(runInfo.GetStream(), &stream);
    runInfo.SetStream(nullptr);
    EXPECT_EQ(runInfo.GetStream(), &stream);  // Should still be the same as before
}

TEST_F(RunInfoTest, SetScratchDeviceAddrTest)
{
    uint8_t addr = 1;
    runInfo.SetScratchDeviceAddr(&addr);
    EXPECT_EQ(runInfo.GetScratchDeviceAddr(), &addr);
    runInfo.SetScratchDeviceAddr(nullptr);
    EXPECT_EQ(runInfo.GetScratchDeviceAddr(), &addr);  // Should still be the same as before
}

TEST_F(RunInfoTest, SetTilingDeviceAddrTest)
{
    uint8_t addr = 1;
    runInfo.SetTilingDeviceAddr(&addr);
    EXPECT_EQ(runInfo.GetTilingDeviceAddr(), &addr);
    runInfo.SetTilingDeviceAddr(nullptr);
    EXPECT_EQ(runInfo.GetTilingDeviceAddr(), &addr);  // Should still be the same as before
}

TEST_F(RunInfoTest, ToStringTest)
{
    uint8_t sAddr = 1;
    uint8_t tAddr = 2;
    int stream = 42;
    runInfo.SetScratchDeviceAddr(&sAddr);
    runInfo.SetTilingDeviceAddr(&tAddr);
    runInfo.SetStream(&stream);
    std::string str = runInfo.ToString();
    EXPECT_NE(str.find("stream: "), std::string::npos);
    EXPECT_NE(str.find("workspaceAddr: "), std::string::npos);
    EXPECT_NE(str.find("tilingDeviceAddr: "), std::string::npos);
}

TEST_F(RunInfoTest, CopyTest)
{
    uint8_t sAddr = 1;
    uint8_t tAddr = 2;
    int stream = 42;
    runInfo.SetScratchDeviceAddr(&sAddr);
    runInfo.SetTilingDeviceAddr(&tAddr);
    runInfo.SetStream(&stream);
    Mki::RunInfo runInfo2;
    runInfo2.Copy(runInfo);

    EXPECT_EQ(runInfo2.GetStream(), runInfo.GetStream());
    EXPECT_EQ(runInfo2.GetScratchDeviceAddr(), runInfo.GetScratchDeviceAddr());
    EXPECT_EQ(runInfo2.GetTilingDeviceAddr(), runInfo.GetTilingDeviceAddr());
}
} // namespace
