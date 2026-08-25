/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/utils/math/math.h"

#include <gtest/gtest.h>

#include <limits>

namespace Mki
{
TEST(MathTest, DividendIsZero)
{
    int64_t dividend = 2147483648;
    int64_t divisor = 2147483648;
    int64_t divisor1 = 1;
    int64_t divisor2 = 0;
    EXPECT_EQ(Mki::Utils::CeilDiv(dividend, divisor), 1);
    EXPECT_EQ(Mki::Utils::CeilDiv(dividend, divisor1), 2147483648);
    EXPECT_EQ(Mki::Utils::CeilDiv(dividend, divisor2), 0);
}

TEST(MathTest, CeilDivDoesNotOverflowAtIntegerLimits)
{
    EXPECT_EQ(Mki::Utils::CeilDiv<uint64_t>(std::numeric_limits<uint64_t>::max(), 2), 9223372036854775808ULL);
    EXPECT_EQ(Mki::Utils::CeilDiv<int64_t>(std::numeric_limits<int64_t>::max(), 2), 4611686018427387904LL);
    EXPECT_EQ(Mki::Utils::CeilDiv<int64_t>(std::numeric_limits<int64_t>::min(), -1),
              std::numeric_limits<int64_t>::max());
}
}  // namespace Mki
