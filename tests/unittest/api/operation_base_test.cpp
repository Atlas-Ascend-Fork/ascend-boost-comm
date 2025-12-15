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
#include "mki/base/operation_base.h"
#include "mki/base/kernel_base.h"

namespace Mki {
class OperationBaseTest : public OperationBase {
public:
    OperationBaseTest() : OperationBase("") {}
    Mki::Status InferShape(LaunchParam &launchParam) const override { return Status::OkStatus(); };
    int64_t GetInputNum(const Any &specificParam) const override { return 1; };
    int64_t GetOutputNum(const Any &specificParam) const override { return 1; };
    Kernel *GetBestKernel(const LaunchParam &launchParam) const override { return nullptr; }

protected:
    Status InferShapeImpl(const LaunchParam &launchParam, SVector<Tensor> &outTensors) const override
    {
        outTensors[0].desc = launchParam.GetOutTensor(0).desc;
        return Status::OkStatus();
    }
};

TEST(OperationBaseTest, all)
{
    Mki::Operation *op = new (std::nothrow) OperationBaseTest();
    ASSERT_NE(op, nullptr);
    std::unique_ptr<Kernel> kernel{nullptr};
    LaunchParam launchParam;
    Tensor inTensor;
    Tensor outTensor = inTensor;
    launchParam.AddInTensor(inTensor);
    launchParam.AddOutTensor(outTensor);

    kernel.reset(op->GetBestKernel(launchParam));
    EXPECT_EQ(kernel, nullptr);
    EXPECT_EQ(op->IsConsistent(launchParam), true);
    delete op;
    op = nullptr;
}

TEST(OperationBaseTest, all0)
{
    Mki::Operation *op = new (std::nothrow) OperationBaseTest();
    ASSERT_NE(op, nullptr);
    const KernelList &kernelList = op->GetKernelList();
    EXPECT_EQ(kernelList.size(), 0);
    delete op;
    op = nullptr;
}
} // namespace Mki
