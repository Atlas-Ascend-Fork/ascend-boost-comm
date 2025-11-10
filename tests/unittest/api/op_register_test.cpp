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
#include "mki_loader/op_register.h"

namespace Mki {
TEST(OpRegisterTest, Base)
{
    std::vector<std::string> opList;
    auto &operationCreators = OpSpace::OperationRegister::GetOperationCreators();
    auto &kernelCreators = OpSpace::KernelRegister::GetKernelCreators();
    (void)operationCreators;
    (void)kernelCreators;
}
} // namespace Mki
