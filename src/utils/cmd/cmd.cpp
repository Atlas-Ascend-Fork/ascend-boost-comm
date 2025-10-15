/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/utils/cmd/cmd.h"
#include <iostream>
#include "mki/utils/assert/assert.h"

namespace Mki {
constexpr int ERROR_INVALID_ARG = 1;
constexpr int ARG_LEN_MIN = 2;
const std::string CMD_HELP = "help";

Cmd::Cmd() { cmdMap_[CMD_HELP] = {std::bind(&Cmd::DoHelp, this, std::placeholders::_1), ""}; }

void Cmd::Register(const std::string &cmd, CmdFunc func, const std::string &desc) { cmdMap_[cmd] = {func, desc}; }

Status Cmd::Run(int argc, const char *argv[])
{
    if (argc < ARG_LEN_MIN) {
        return Status::FailStatus(ERROR_INVALID_ARG, "invalid args");
    }
    if (argv == nullptr) {
        return Status::FailStatus(ERROR_INVALID_ARG, "argv is nullptr");
    }
    std::string cmd = argv[1];
    const auto it = cmdMap_.find(cmd);
    if (it == cmdMap_.cend()) {
        return Status::FailStatus(ERROR_INVALID_ARG, "not support cmd:" + cmd);
    }

    std::vector<std::string> args;
    for (int i = 2; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    return it->second.func(args);
}

Status Cmd::DoHelp(const std::vector<std::string> &args) const
{
    UNUSED_VALUE(args);
    for (const auto &it : cmdMap_) {
        if (it.first == CMD_HELP) {
            continue;
        }
        std::cout << it.second.desc << std::endl;
    }
    return Status::OkStatus();
}
} // namespace Mki
