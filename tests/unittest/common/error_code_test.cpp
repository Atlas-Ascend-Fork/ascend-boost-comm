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
#include "mki/launch_param.h"
#include "mki/base/kernel_base.h"
#include "mki/utils/SVector/SVector.h"
#include "mki/utils/checktensor/check_tensor.h"
#include "mki/utils/rt/other/other.h"

namespace {
class TestKernelBase : public Mki::KernelBase {
public:
    explicit TestKernelBase(const std::string &kernelName, const Mki::BinHandle *handle) noexcept
        : KernelBase(kernelName, handle) {}
    Mki::Status InlineCheckInit(const Mki::LaunchParam &launchParam)
    {
        return Init(launchParam);
    }

    Mki::Status InlineCheckInitImpl(const Mki::LaunchParam &launchParam)
    {
        return InitImpl(launchParam);
    }
};

class MKIErrorCodeTest : public ::testing::Test {
protected:
    
    Mki::LaunchParam launchParam;
    Mki::Tensor tensor1, tensor2;
    Mki::KernelInfo kernelInfo;
    const Mki::BinaryBasicInfo *binInfo = nullptr;
    TestKernelBase *testKernelBase = nullptr;
    const Mki::BinHandle* binHandle = nullptr;
    const std::string opName = "test";
    Mki::MiniVector<Mki::KernelInfo::MemsetInfo> memsetInfo;
    
    void SetUp() override
    {
        binInfo = new Mki::BinaryBasicInfo;
        binHandle = new Mki::BinHandle(binInfo);
        testKernelBase =  new TestKernelBase(opName, binHandle);
    }

    void TearDown() override
    {
        delete binInfo;
        binInfo = nullptr;
        delete binHandle;
        binHandle = nullptr;
        delete testKernelBase;
        testKernelBase = nullptr;
    }
};

TEST_F(MKIErrorCodeTest, CheckLaunchParam)
{
    EXPECT_EQ(testKernelBase->InlineCheckInit(launchParam).Code(), Mki::ErrorType::ERROR_INVALID_VALUE);
    EXPECT_EQ(testKernelBase->CanSupport(launchParam) ?
              Mki::ErrorType::NO_ERROR : Mki::ErrorType::ERROR_INVALID_VALUE,
              Mki::ErrorType::ERROR_INVALID_VALUE);
}

TEST_F(MKIErrorCodeTest, CheckAllocTilingHost)
{
    constexpr uint64_t INVALID_LEN = -1;
    uint64_t len = INVALID_LEN;
    EXPECT_EQ(kernelInfo.AllocTilingHost(-1).Code(), Mki::ErrorType::ERROR_ALLOC_HOST);
    constexpr uint64_t LARGE_SIZE = 1024 * 1024 + 1;
    len = LARGE_SIZE;
    EXPECT_EQ(kernelInfo.AllocTilingHost(len).Code(), Mki::ErrorType::ERROR_ALLOC_HOST);
    constexpr uint64_t VALID_LEN = 32;
    len = VALID_LEN;
    EXPECT_EQ(kernelInfo.AllocTilingHost(len).Code(), Mki::ErrorType::NO_ERROR);
}

TEST_F(MKIErrorCodeTest, CheckInitImpl)
{
    EXPECT_EQ(testKernelBase->InlineCheckInitImpl(launchParam).Code(), Mki::ErrorType::ERROR_INVALID_VALUE);
}

TEST_F(MKIErrorCodeTest, CheckInitArgs)
{
    constexpr int minInvalidValue = -1;
    EXPECT_EQ(kernelInfo.InitArgs(minInvalidValue).Code(), Mki::ErrorType::ERROR_INVALID_VALUE);
    constexpr int maxInvalidValue = 1024 * 1024 + 1;
    EXPECT_EQ(kernelInfo.InitArgs(maxInvalidValue).Code(), Mki::ErrorType::ERROR_INVALID_VALUE);
}

TEST_F(MKIErrorCodeTest, CheckUpdateHwsyncArgs)
{
    uint32_t *len = nullptr;
    int st = Mki::MkiRtGetC2cCtrlAddr(nullptr, len);
    EXPECT_EQ((st == 0) ? Mki::ErrorType::NO_ERROR : Mki::ErrorType::ERROR_RUN_TIME_ERROR,
              Mki::ErrorType::ERROR_RUN_TIME_ERROR);
}

} // namespace