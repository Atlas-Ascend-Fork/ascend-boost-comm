/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/utils/time/timer.h"
#include <sys/time.h>

namespace Mki {
const uint64_t MICRSECOND_PER_SECOND = 1000000;

Timer::Timer() { startTimepoint_ = GetCurrentTimepoint(); }

Timer::~Timer() {}

uint64_t Timer::ElapsedMicroSecond()
{
    uint64_t now = GetCurrentTimepoint();
    uint64_t use = now - startTimepoint_;
    startTimepoint_ = now;
    return use;
}

void Timer::Reset() { startTimepoint_ = GetCurrentTimepoint(); }

uint64_t Timer::GetCurrentTimepoint() const
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    uint64_t ret = static_cast<uint64_t>(tv.tv_sec * MICRSECOND_PER_SECOND + tv.tv_usec);
    return ret;
}
} // namespace Mki
