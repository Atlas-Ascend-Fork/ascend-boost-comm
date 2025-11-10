/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#include "mki/utils/strings/str_checker.h"
#include "mki/utils/log/log.h"

namespace Mki {
bool CheckNameValid(const std::string &name, const size_t maxLen)
{
    // 1. check the name not empty
    if (name.empty()) {
        MKI_LOG(ERROR) << "The name is empty";
        return false;
    }

    // 2. check the length of the name
    if (name.size() > maxLen) {
        MKI_LOG(ERROR) << "The length of the name " << name.size() << " > " << maxLen;
        return false;
    }

    // 3. check the name has invalid characters
    const std::vector<char> invalidChars = { '\n', '\f', '\r', '\v', '\t', '\b', '\u007f', ' ', '$'};
    for (auto c : invalidChars) {
        if (name.find(c) != std::string::npos) {
            MKI_LOG(ERROR) << "Name contains invalid characters: " << c;
            return false;
        }
    }
    return true;
}
}
