/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software; you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for the specific language governing permissions and limitations under the License.
 */
#include "mki/kernel_info.h"

#include <gtest/gtest.h>

namespace
{
TEST(KernelInfoTest, CopyReallocatesTilingHostAfterSizeChange)
{
    Mki::KernelInfo source;
    Mki::KernelInfo destination;
    ASSERT_TRUE(source.InitArgs(8).Ok());
    ASSERT_TRUE(destination.InitArgs(8).Ok());
    ASSERT_TRUE(source.AllocTilingHost(64).Ok());
    ASSERT_TRUE(destination.AllocTilingHost(32).Ok());
    for (int i = 0; i < 64; ++i)
    {
        source.GetTilingHostAddr()[i] = static_cast<uint8_t>(i + 1);
    }

    destination.Copy(source);

    EXPECT_EQ(destination.GetTilingSize(), 64);
    EXPECT_EQ(destination.GetTilingHostAddr()[63], 64);
}
}  // namespace
