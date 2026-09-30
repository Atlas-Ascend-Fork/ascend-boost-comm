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
#include "mki/utils/status/status.h"

namespace Mki {
TEST(StatusTest, Base)
{
    Mki::Status st = Mki::Status::FailStatus(3, "dd");
    std::cout << st.ToString() << std::endl;
    Mki::Status st1 = st;
    Mki::Status st2;
    st2 = st1;
}
TEST(StatusTest, Base1)
{
    Mki::Status st = Mki::Status::OkStatus();
    EXPECT_EQ(st.Ok(), true);
    EXPECT_EQ(st.ToString(), "ok");
    EXPECT_EQ(st.Code(), 0);
    EXPECT_EQ(st.Message(), "");
}

TEST(StatusTest, Base2)
{
    Mki::Status status;
    Mki::Status status_copy = status;
}

TEST(StatusTest, Code)
{
    Mki::Status st = Mki::Status::FailStatus(3, "dd");
    EXPECT_EQ(st.Code(), 3);
}

TEST(StatusTest, CopyAndAssignmentPreserveEmptyAndLongErrorMessages)
{
    for (const size_t length : {0, 1015, 1016, 1024})
    {
        const std::string message(length, 'x');
        const Mki::Status original = Mki::Status::FailStatus(7, message);

        ASSERT_FALSE(original.Ok());
        ASSERT_EQ(original.Code(), 7);
        ASSERT_EQ(original.Message(), message);

        const Mki::Status copied(original);
        EXPECT_FALSE(copied.Ok());
        EXPECT_EQ(copied.Code(), 7);
        EXPECT_EQ(copied.Message(), message);

        Mki::Status assigned;
        assigned = original;
        EXPECT_FALSE(assigned.Ok());
        EXPECT_EQ(assigned.Code(), 7);
        EXPECT_EQ(assigned.Message(), message);
    }
}

TEST(StatusTest, OversizedErrorMessageKeepsExistingContract)
{
    const std::string message(1025, 'x');
    const Mki::Status original = Mki::Status::FailStatus(7, message);

    EXPECT_TRUE(original.Ok());
    EXPECT_EQ(original.Code(), 0);
    EXPECT_EQ(original.Message(), "");

    const Mki::Status copied(original);
    EXPECT_TRUE(copied.Ok());
    EXPECT_EQ(copied.Code(), 0);
    EXPECT_EQ(copied.Message(), "");

    Mki::Status assigned = Mki::Status::FailStatus(8, "seed");
    assigned = original;
    EXPECT_TRUE(assigned.Ok());
    EXPECT_EQ(assigned.Code(), 0);
    EXPECT_EQ(assigned.Message(), "");
}
} // namespace Mki
