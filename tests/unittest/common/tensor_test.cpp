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
#include "mki/tensor.h"
#include "../schedule/common/tensor.cpp"
namespace {
TEST(CalNumelTest, HandleValidInput) {
    Mki::SVector<int64_t> v = {2, 3, 4};
    EXPECT_EQ(Mki::CalNumel(v, 0, 3), 24);

    v = {1, 2, 3};
    EXPECT_EQ(Mki::CalNumel(v, 0, 3), 6);

    v = {1000, 1000, 1000};
    EXPECT_EQ(Mki::CalNumel(v, 0, 3), 1000000000);
}

TEST(CalNumelTest, HandleInvalidInput) {
    Mki::SVector<int64_t> v = {-1, 3, 4};
    EXPECT_EQ(Mki::CalNumel(v, 0, 3), 0);

    v = {2, 0, 4};
    EXPECT_EQ(Mki::CalNumel(v, 0, 3), 0);

    v = {2, std::numeric_limits<int64_t>::max(), 4};
    EXPECT_EQ(Mki::CalNumel(v, 0, 3), 0);
}

TEST(CalNumelTest, HandleSubrangeInput) {
    Mki::SVector<int64_t> v = {2, 3, 4};
    EXPECT_EQ(Mki::CalNumel(v, 0, 2), 6);

    v = {2, 3, 4};
    EXPECT_EQ(Mki::CalNumel(v, 1, 3), 12);
}

TEST(ViewTest, HandleValidInput) {
    Mki::TensorDesc td;
    td.dims = {3, 2, 4};
    Mki::SVector<int64_t> ms = {2, 3, 4};
    td.View(ms);
    EXPECT_EQ(td.dims, ms);

    Mki::SVector<int64_t> ms2 = {1, 2, 3};
    td.View(ms2);
    EXPECT_NE(td.dims, ms2);
}

TEST(ViewTest, HandleInvalidInput) {
    Mki::TensorDesc td;
    Mki::SVector<int64_t> ms = {2, 3, 4};
    td.View(ms);
    EXPECT_TRUE(td.dims.empty());
}

TEST(ToStringTest, HandleEmptyDims) {
    Mki::TensorDesc td;
    td.dims = {};
    EXPECT_EQ(td.ToString(), "dtype:undefined, format:undefined, dims:[], strides:[], offset:0");
}

TEST(ToStringTest, HandleSingleElementDims) {
    Mki::TensorDesc td;
    td.dims = {10};
    EXPECT_EQ(td.ToString(), "dtype:undefined, format:undefined, dims:[10], strides:[], offset:0");
}

TEST(ToStringTest, HandleMultipleElementsDims) {
    Mki::TensorDesc td;
    td.dims = {1, 2, 3};
    EXPECT_EQ(td.ToString(), "dtype:undefined, format:undefined, dims:[1, 2, 3], strides:[], offset:0");
}

TEST(ToStringTest, HandleMultipleElementsDimsWithStrideAndOffset) {
    Mki::TensorDesc td;
    td.dims = {1, 2, 3};
    td.strides = {8, 4, 1};
    td.offset = 10;
    EXPECT_EQ(td.ToString(), "dtype:undefined, format:undefined, dims:[1, 2, 3], strides:[8, 4, 1], offset:10");
}

TEST(IsContiguousTest, EmptyStridesTest) {
    Mki::TensorDesc td;
    td.dims = {1, 2, 3};
    td.strides = {};
    EXPECT_TRUE(td.IsContiguous());
}
 
TEST(IsContiguousTest, ContiguousStridesTest) {
    Mki::TensorDesc td;
    td.dims = {1, 2, 3};
    td.strides = {6, 3, 1};
    EXPECT_TRUE(td.IsContiguous());
}
 
TEST(IsContiguousTest, NotContiguousStridesTest) {
    Mki::TensorDesc td;
    td.dims = {1, 2, 3};
    td.strides = {8, 4, 1};
    EXPECT_TRUE(!td.IsContiguous());
}

TEST(IsContiguousTest, NotContiguousOffsetTest) {
    Mki::TensorDesc td;
    td.dims = {1, 2, 3};
    td.strides = {6, 3, 1};
    td.offset = 10;
    EXPECT_TRUE(!td.IsContiguous());
}
} // namespace
