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
#include <thread>
#include "mki/utils/SVector/SVector.h"
#include "mki/utils/log/log.h"

namespace Mki {
TEST(SVectorTest, all)
{
    Mki::SVector<int, 32> ints;
    Mki::SVector<int, 48> ints1;
    Mki::SVector<int> ints2;
    Mki::SVector<int, 48> ints3;
    ASSERT_EQ(ints.size(), 0);
    ints.push_back(4);
    ints1.push_back(3);
    ints2.push_back(2);
    ints3.push_back(1);
    ASSERT_EQ(ints.size(), 1);
    ints.emplace_back(4);
    ASSERT_EQ(ints.size(), 2);
    ints.resize(5);
    ASSERT_EQ(ints.size(), 5);
    try {
        ints.resize(64);
    } catch (const std::exception &e) {
    }
}

TEST(SVectorTest, InitializerList)
{
    Mki::SVector<int> ints = {3, 4, 5};
    ASSERT_EQ(ints.size(), 3);
    ASSERT_EQ(ints[0], 3);
    ASSERT_EQ(ints[1], 4);
    ASSERT_EQ(ints[2], 5);
}

TEST(SVectorTest, InitializerListConstructor)
{
    Mki::SVector<int, 5> v1{1, 2, 3};
    EXPECT_EQ(v1.size(), 3);
    EXPECT_EQ(v1[0], 1);
    EXPECT_EQ(v1[1], 2);
    EXPECT_EQ(v1[2], 3);
    Mki::SVector<std::string, 3> v2{"hello", "world"};
    EXPECT_EQ(v2.size(), 2);
    EXPECT_EQ(v2[0], "hello");
    EXPECT_EQ(v2[1], "world");
    try {
        Mki::SVector<int, 2>{1, 2, 3};
    } catch (const std::exception &e) {
    }
}

TEST(SVectorTest, Constructor)
{
    Mki::SVector<int, 5> v1(3, 42);
    EXPECT_EQ(v1.size(), 3);
    EXPECT_EQ(v1[0], 42);
    EXPECT_EQ(v1[1], 42);
    EXPECT_EQ(v1[2], 42);
    Mki::SVector<int, 5> v2(4);
    EXPECT_EQ(v2.size(), 4);
    EXPECT_EQ(v2[0], 0);
    EXPECT_EQ(v2[1], 0);
    EXPECT_EQ(v2[2], 0);
    EXPECT_EQ(v2[3], 0);
    try {
        Mki::SVector<int, 5>(6);
    } catch (const std::exception &e) {
    }
}

TEST(PushBackTest, push_back)
{
    Mki::SVector<int, 1> ints;
    ints.push_back(10);
    EXPECT_EQ(ints.size(), 1);
    int i;
    try {
        for (i = 0; i < 47; i++) {
            ints.push_back(i);
        }
    } catch (const std::exception &e) {
    }
}

TEST(PushBackTest, emplace_back)
{
    Mki::SVector<int, 48> ints;
    ints.resize(48);
    try {
        ints.emplace_back(5);
    } catch (const std::exception &e) {
    }
}

TEST(PushBackTest, EraseOutOfrange)
{
    Mki::SVector<int, 48> ints;
    ints.push_back(4);
    try {
        ints.erase(ints.begin() + 1);
    } catch (const std::exception &e) {
    }
}

TEST(OperatorTest, AccessElementOutOfBounds)
{
    Mki::SVector<int, 5> v1(3, 42);
    v1.resize(0);
    EXPECT_THROW(v1[3], std::out_of_range);
    EXPECT_THROW(v1[10], std::out_of_range);
}
} // namespace Mki
