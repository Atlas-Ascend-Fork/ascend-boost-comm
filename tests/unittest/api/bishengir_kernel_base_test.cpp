/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/base/bishengir_kernel_base.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>
#include <vector>

#include "mki/launch_param.h"
#include "mki/run_info.h"

namespace
{
constexpr size_t kInputCount = 3;
constexpr uint64_t kWorkspaceStrideOne = 1;

class WorkspaceKernel : public Mki::BishengIRKernelBase
{
   public:
    explicit WorkspaceKernel(std::initializer_list<uint64_t> sizes)
        : Mki::BishengIRKernelBase("bishengir_multi_workspace", nullptr), sizes_(sizes)
    {
    }

    bool CanSupport(const Mki::LaunchParam &) const override { return true; }

   protected:
    Mki::Status InitImpl(const Mki::LaunchParam &) override
    {
        auto &scratchSizes = kernelInfo_.GetScratchSizes();
        for (uint64_t size : sizes_)
        {
            scratchSizes.push_back(size);
        }
        return Mki::Status::OkStatus();
    }

   private:
    std::vector<uint64_t> sizes_;
};

struct WorkspaceDesc
{
    void *addr = nullptr;
    void *addrAlias = nullptr;
    uint64_t reserved = 0;
    uint64_t size = 0;
    uint64_t one = 0;
};

uint64_t FirstWorkspaceSlot(const Mki::LaunchParam &launchParam, const Mki::KernelInfo &info)
{
    uint64_t slot = launchParam.GetInTensorCount() + launchParam.GetOutTensorCount();
    if (info.GetHwsyncIdx() >= 0)
    {
        slot += 1;
    }
    if (launchParam.GetInTensorCount() != kInputCount || info.GetHwsyncIdx() < 0)
    {
        slot += 1;
    }
    return slot;
}

WorkspaceDesc ReadWorkspace(const Mki::KernelInfo &info, uint64_t slot, uint8_t *argsBase = nullptr)
{
    WorkspaceDesc desc;
    const auto &index = info.GetKernelArgsIndex();
    uint8_t *argsMem = argsBase != nullptr ? argsBase : info.GetArgs();
    if (slot >= index.size() || argsMem == nullptr)
    {
        ADD_FAILURE() << "workspace slot " << slot << " is outside the packed args";
        return desc;
    }
    uint64_t argIndex = index[slot];
    void **args = reinterpret_cast<void **>(static_cast<void *>(argsMem));
    desc.addr = args[argIndex];
    desc.addrAlias = args[argIndex + 1];
    uint64_t *fields = reinterpret_cast<uint64_t *>(&args[argIndex + 2]);
    desc.reserved = fields[0];
    desc.size = fields[1];
    desc.one = fields[2];
    return desc;
}

void ExpectWorkspace(const WorkspaceDesc &desc, void *addr, uint64_t size)
{
    EXPECT_EQ(desc.addr, addr);
    EXPECT_EQ(desc.addrAlias, addr);
    EXPECT_EQ(desc.reserved, 0u);
    EXPECT_EQ(desc.size, size);
    EXPECT_EQ(desc.one, kWorkspaceStrideOne);
}
}  // namespace

class BishengIRKernelBaseTest : public ::testing::Test
{
   protected:
    uint8_t inputStorage_[kInputCount][16] = {};
    uint8_t scratch_[256] = {};

    Mki::LaunchParam MakeLaunchParam()
    {
        Mki::LaunchParam launchParam;
        for (size_t i = 0; i < kInputCount; ++i)
        {
            Mki::Tensor tensor;
            tensor.desc.dtype = Mki::TENSOR_DTYPE_FLOAT;
            tensor.desc.format = Mki::TENSOR_FORMAT_ND;
            tensor.desc.dims = {2, 2};
            tensor.data = inputStorage_[i];
            tensor.dataSize = sizeof(inputStorage_[i]);
            launchParam.AddInTensor(tensor);
        }
        return launchParam;
    }

    Mki::Status Pack(WorkspaceKernel &kernel, Mki::LaunchParam &launchParam, Mki::RunInfo &runInfo)
    {
        Mki::Status status = kernel.Init(launchParam);
        if (!status.Ok())
        {
            return status;
        }
        return kernel.BuildArgs(launchParam, runInfo, nullptr);
    }
};

TEST_F(BishengIRKernelBaseTest, PlacesEachWorkspaceInItsOwnSlice)
{
    WorkspaceKernel kernel({16, 32});
    Mki::LaunchParam launchParam = MakeLaunchParam();
    Mki::RunInfo runInfo;
    runInfo.SetScratchDeviceAddr(scratch_);

    Mki::Status status = Pack(kernel, launchParam, runInfo);
    ASSERT_TRUE(status.Ok()) << status.ToString();

    const Mki::KernelInfo &info = kernel.GetKernelInfo();
    uint64_t slot = FirstWorkspaceSlot(launchParam, info);
    const auto &index = info.GetKernelArgsIndex();
    ASSERT_LT(slot + 1, index.size());
    EXPECT_NE(index[slot], index[slot + 1]);
    void **args = reinterpret_cast<void **>(static_cast<void *>(info.GetArgs()));
    for (size_t i = 0; i < kInputCount; ++i)
    {
        ASSERT_LT(i + 1, index.size());
        EXPECT_EQ(args[index[i + 1]], inputStorage_[i]);
    }

    ExpectWorkspace(ReadWorkspace(info, slot), scratch_, 16);
    ExpectWorkspace(ReadWorkspace(info, slot + 1), scratch_ + 16, 32);
}

TEST_F(BishengIRKernelBaseTest, KeepsSingleWorkspaceDescriptor)
{
    WorkspaceKernel kernel({16});
    Mki::LaunchParam launchParam = MakeLaunchParam();
    Mki::RunInfo runInfo;
    runInfo.SetScratchDeviceAddr(scratch_);

    Mki::Status status = Pack(kernel, launchParam, runInfo);
    ASSERT_TRUE(status.Ok()) << status.ToString();

    const Mki::KernelInfo &info = kernel.GetKernelInfo();
    ASSERT_EQ(info.GetScratchSizes().size(), 1u);
    ExpectWorkspace(ReadWorkspace(info, FirstWorkspaceSlot(launchParam, info)), scratch_, 16);
}

TEST_F(BishengIRKernelBaseTest, PlacesThreeOrMoreWorkspacesContiguously)
{
    constexpr uint64_t kSizes[] = {16, 32, 64};
    WorkspaceKernel kernel({kSizes[0], kSizes[1], kSizes[2]});
    Mki::LaunchParam launchParam = MakeLaunchParam();
    Mki::RunInfo runInfo;
    runInfo.SetScratchDeviceAddr(scratch_);

    Mki::Status status = Pack(kernel, launchParam, runInfo);
    ASSERT_TRUE(status.Ok()) << status.ToString();

    const Mki::KernelInfo &info = kernel.GetKernelInfo();
    uint64_t slot = FirstWorkspaceSlot(launchParam, info);
    uint64_t offset = 0;
    for (uint64_t size : kSizes)
    {
        ExpectWorkspace(ReadWorkspace(info, slot), scratch_ + offset, size);
        offset += size;
        slot += 1;
    }
}

TEST_F(BishengIRKernelBaseTest, EmptyWorkspaceBaseKeepsExistingAddress)
{
    uint8_t kept[16] = {};
    Mki::RunInfo keptInfo;
    keptInfo.SetScratchDeviceAddr(kept);
    keptInfo.SetScratchDeviceAddr(nullptr);
    EXPECT_EQ(keptInfo.GetScratchDeviceAddr(), kept);

    WorkspaceKernel kernel({16, 32});
    Mki::LaunchParam launchParam = MakeLaunchParam();
    Mki::Status status = Pack(kernel, launchParam, keptInfo);
    ASSERT_TRUE(status.Ok()) << status.ToString();

    const Mki::KernelInfo &info = kernel.GetKernelInfo();
    uint64_t slot = FirstWorkspaceSlot(launchParam, info);
    ExpectWorkspace(ReadWorkspace(info, slot), kept, 16);
    ExpectWorkspace(ReadWorkspace(info, slot + 1), kept + 16, 32);
}

TEST_F(BishengIRKernelBaseTest, ZeroWorkspaceSizeKeepsOffset)
{
    WorkspaceKernel kernel({16, 0, 32});
    Mki::LaunchParam launchParam = MakeLaunchParam();
    Mki::RunInfo runInfo;
    runInfo.SetScratchDeviceAddr(scratch_);

    Mki::Status status = Pack(kernel, launchParam, runInfo);
    ASSERT_TRUE(status.Ok()) << status.ToString();

    const Mki::KernelInfo &info = kernel.GetKernelInfo();
    uint64_t slot = FirstWorkspaceSlot(launchParam, info);
    ExpectWorkspace(ReadWorkspace(info, slot), scratch_, 16);
    ExpectWorkspace(ReadWorkspace(info, slot + 1), scratch_ + 16, 0);
    ExpectWorkspace(ReadWorkspace(info, slot + 2), scratch_ + 16, 32);
}

TEST_F(BishengIRKernelBaseTest, BuildArgsWritesCallerBuffer)
{
    WorkspaceKernel kernel({16, 32});
    Mki::LaunchParam launchParam = MakeLaunchParam();
    Mki::RunInfo runInfo;
    runInfo.SetScratchDeviceAddr(scratch_);
    ASSERT_TRUE(kernel.Init(launchParam).Ok());

    const Mki::KernelInfo &info = kernel.GetKernelInfo();
    info.GetArgs()[0] = 0x5a;
    std::vector<uint8_t> hostBuffer(info.GetArgsSize(), 0);
    Mki::Status status = kernel.BuildArgs(launchParam, runInfo, hostBuffer.data());
    ASSERT_TRUE(status.Ok()) << status.ToString();
    EXPECT_EQ(info.GetArgs()[0], 0x5a);

    uint64_t slot = FirstWorkspaceSlot(launchParam, info);
    ExpectWorkspace(ReadWorkspace(info, slot, hostBuffer.data()), scratch_, 16);
    ExpectWorkspace(ReadWorkspace(info, slot + 1, hostBuffer.data()), scratch_ + 16, 32);
}

TEST_F(BishengIRKernelBaseTest, EmptyScratchListReturnsOk)
{
    WorkspaceKernel kernel({});
    Mki::LaunchParam launchParam = MakeLaunchParam();
    Mki::RunInfo runInfo;

    Mki::Status status = Pack(kernel, launchParam, runInfo);
    EXPECT_TRUE(status.Ok()) << status.ToString();
    EXPECT_EQ(kernel.GetKernelInfo().GetScratchSizes().size(), 0u);
}
