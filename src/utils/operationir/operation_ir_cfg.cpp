/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/utils/operationir/operation_ir_cfg.h"
#include <fstream>
#include <algorithm>
#include <string>
#include "mki/types.h"
#include "mki/utils/log/log.h"
#include "mki/utils/assert/assert.h"
#include "mki/utils/file_system/file_system.h"
#include "mki/utils/inifile/ini_file.h"

namespace Mki {
constexpr size_t MAX_OP_NAME_LEN = 128;

OperationIrCfg::OperationIrCfg() {}

OperationIrCfg::~OperationIrCfg() {}

Status OperationIrCfg::Load(const std::string &fileName)
{
    std::map<std::string, std::map<std::string, std::string>> contentInfoMap;
    bool isParseSuccess = IniFile::ParseIniFileToMap(fileName, contentInfoMap);
    MKI_CHECK(isParseSuccess && !contentInfoMap.empty(), "ParseIniFileToMap failed!",
        return Status::FailStatus(ERROR_INVALID_VALUE));
    std::string currentOpKey;
    OperationIr opIr;
    std::map<std::string, std::map<std::string, std::string>>::iterator it;
    for (it = contentInfoMap.begin(); it != contentInfoMap.end(); it++) {
        currentOpKey = it->first;
        MKI_CHECK(IsValidOpKey(currentOpKey), "OpKey is invalid! key:" << currentOpKey,
            return Status::FailStatus(ERROR_INVALID_VALUE));
        opIr = OperationIr();
        isParseSuccess = opIr.Parse(it->second);

        MKI_CHECK(isParseSuccess, "parse failed!", return Status::FailStatus(ERROR_INVALID_VALUE));
        data_.emplace(make_pair(currentOpKey, opIr));
    }
    return Status::OkStatus();
}

bool OperationIrCfg::IsValidOpKey(const std::string &key) const
{
    MKI_CHECK(key.length() > 0 && key.length() <= MAX_OP_NAME_LEN,
        "The length of OpKey must be greater than 0 and less than or equal to 128.", return false);
    for (const char &c : key) {
        MKI_CHECK(isalnum(c), "opName only characters and digits are supported.", return false);
    }
    return true;
}

OperationIr *OperationIrCfg::GetOperationIr(const std::string &opKey)
{
    auto it = data_.find(opKey);
    if (it != data_.end()) {
        MKI_LOG(DEBUG) << opKey << ":\n" << it->second.ToString();
        return &it->second;
    }
    return nullptr;
}
} // namespace Mki
