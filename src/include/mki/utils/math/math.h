/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#ifndef MKI_UTILS_MATH_MATH_H
#define MKI_UTILS_MATH_MATH_H

#include <cstdint>
#include <limits>
#include <type_traits>
#include "mki/utils/log/log.h"

namespace Mki {
namespace Utils {
template <typename T>
T CeilDiv(T dividend, T divisor)
{
    if (divisor == 0) {
        return 0;
    }
    if constexpr (std::is_signed<T>::value) {
        if (dividend == std::numeric_limits<T>::min() && divisor == -1) {
            return std::numeric_limits<T>::max();
        }
    }
    const T quotient = dividend / divisor;
    const T remainder = dividend % divisor;
    return quotient + ((remainder != 0 && ((dividend < 0) == (divisor < 0))) ? 1 : 0);
}
size_t RoundUp(size_t size, size_t divisor = 32);
size_t RoundDown(size_t size, size_t divisor = 32);
void SetRandseeds(uint32_t &randseed);
}
}

#endif // MKI_UTILS_MATH_MATH_H
