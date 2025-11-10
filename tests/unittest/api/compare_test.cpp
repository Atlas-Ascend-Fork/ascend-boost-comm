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
#include "mki/utils/compare/compare.h"

namespace Mki {
TEST(CompareTest, IsEqual) {
    int a = 5;
    int b = 5;
    EXPECT_TRUE(Mki::Utils::Compare<int>::IsEqual(a, b));
    double c = 3.14;
    double d = 3.14;
    EXPECT_TRUE(Mki::Utils::Compare<double>::IsEqual(c, d));
    std::string e = "hello";
    std::string f = "world";
    EXPECT_FALSE(Mki::Utils::Compare<std::string>::IsEqual(e, f));
}
} // namespace Mki
