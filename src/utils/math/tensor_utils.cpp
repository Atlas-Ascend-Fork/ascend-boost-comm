/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/utils/math/tensor_utils.h"
#include "mki/utils/math/math.h"

namespace Mki {
namespace Utils {
uint64_t GetTensorAlignedSize(uint64_t len)
{
    constexpr uint64_t alignTo = 64;
    if (len == 0) {
        return 0;
    }
    return RoundUp(len, alignTo);
}
}
}
