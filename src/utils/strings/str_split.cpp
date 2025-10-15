/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <sstream>
#include "mki/utils/log/log.h"
#include "mki/utils/strings/str_split.h"

namespace Mki {
constexpr int64_t MAX_USER_STRING_LENGTH = 10000;

void StrSplit(const std::string &text, const char delimiter, std::vector<std::string> &result)
{
    std::istringstream iss(text);
    std::string subStr;
    uint32_t lineNum = 0;
    while (getline(iss, subStr, delimiter)) {
        if (lineNum > MAX_USER_STRING_LENGTH) {
            MKI_LOG(ERROR) << "file lineNum is out of range, lineNum : " << lineNum;
            break;
        }
        lineNum++;
        result.push_back(subStr);
    }
}
} // namespace Mki
