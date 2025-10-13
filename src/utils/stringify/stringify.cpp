/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/utils/log/log.h"
#include "mki/utils/stringify/stringify.h"

namespace Mki {
std::map<size_t, StringifyFunc> Stringify::converterMap_;

Stringify::Stringify(size_t typeHashCode, StringifyFunc func) noexcept
{
    auto it = converterMap_.find(typeHashCode);
    if (it == converterMap_.end()) {
        converterMap_[typeHashCode] = func;
    } else {
        MKI_LOG(WARN) << typeHashCode << " has been registered";
    }
}

std::string Stringify::ToString(const Any &param)
{
    auto it = converterMap_.find(param.Type().hash_code());
    if (it != converterMap_.end()) {
        return it->second(param);
    }
    return std::string(param.Type().name()) + std::string(" can not be printed");
}
} // namespace Mki
