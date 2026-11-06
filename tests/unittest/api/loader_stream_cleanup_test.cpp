/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software; you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for the specific language governing permissions and limitations under the License.
 */
#include "mki_loader/loader_stream_cleanup.h"

#include <gtest/gtest.h>

namespace Mki
{
TEST(LoaderStreamCleanupTest, DestroysHandleReturnedByFailedCreate)
{
    constexpr int createFailure = 1;
    constexpr int destroyFailure = 2;
    int fakeStream = 0;
    int destroyCalls = 0;
    auto stream = &fakeStream;

    int destroyStatus = LoaderDetail::CleanupStreamAfterCreateFailure(createFailure, stream, 0,
                                                                      [&](int* handle)
                                                                      {
                                                                          EXPECT_EQ(handle, &fakeStream);
                                                                          ++destroyCalls;
                                                                          return destroyFailure;
                                                                      });

    EXPECT_EQ(destroyCalls, 1);
    EXPECT_EQ(destroyStatus, destroyFailure);
    EXPECT_EQ(stream, nullptr);
}

TEST(LoaderStreamCleanupTest, DoesNotDestroyHandleAfterSuccessfulCreate)
{
    int fakeStream = 0;
    int destroyCalls = 0;
    auto stream = &fakeStream;

    int destroyStatus = LoaderDetail::CleanupStreamAfterCreateFailure(0, stream, 0,
                                                                      [&](int*)
                                                                      {
                                                                          ++destroyCalls;
                                                                          return 1;
                                                                      });

    EXPECT_EQ(destroyCalls, 0);
    EXPECT_EQ(destroyStatus, 0);
    EXPECT_EQ(stream, &fakeStream);
}
}  // namespace Mki
