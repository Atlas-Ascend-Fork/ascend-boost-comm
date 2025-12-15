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
#include <thread>
#include "mki/utils/time/timer.h"

namespace Mki {
TEST(TimerTest, ElapsedMicroSecond)
{
    Mki::Timer timer;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << timer.ElapsedMicroSecond() << std::endl;

    timer.Reset();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << timer.ElapsedMicroSecond() << std::endl;

    timer.Reset();
    {
        std::vector<int> data(10);
    }
    std::cout << timer.ElapsedMicroSecond() << std::endl;

    timer.Reset();
    std::cout << timer.ElapsedMicroSecond() << std::endl;
}
} // namespace Mki
