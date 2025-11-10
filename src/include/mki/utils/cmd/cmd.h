/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#ifndef MKI_UTILS_CMD_CMD_H
#define MKI_UTILS_CMD_CMD_H
#include <vector>
#include <functional>
#include <map>
#include <string>
#include "mki/utils/status/status.h"

namespace Mki {
using CmdFunc = std::function<Status(const std::vector<std::string> &args)>;

class Cmd {
public:
    Cmd();
    ~Cmd() = default;
    void Register(const std::string &cmd, CmdFunc func, const std::string &desc);
    Status Run(int argc, const char *argv[]);

private:
    Status DoHelp(const std::vector<std::string> &args) const;

private:
    struct CmdItem {
        CmdFunc func;
        std::string desc;
    };
    std::map<std::string, CmdItem> cmdMap_;
};
} // namespace Mki
#endif
