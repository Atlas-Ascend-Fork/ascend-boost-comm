/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <gtest/gtest.h>
#include "mki/utils/cmd/cmd.h"

namespace Mki {
Status CmdPackage(const std::vector<std::string> &args) { return Status::OkStatus(); }

Status CmdInspect(const std::vector<std::string> &args) { return Status::OkStatus(); }

TEST(CmdTest, all)
{
    Cmd cmd;
    cmd.Register("package", std::bind(&CmdPackage, std::placeholders::_1), "package SrcOpDir DestBinFilePath");
    cmd.Register("inspect", std::bind(&CmdInspect, std::placeholders::_1), "inspect BinFilePath");
}

TEST(StatusCmdTest, Run)
{
    Cmd cmd;
    cmd.Register("package", std::bind(&CmdPackage, std::placeholders::_1), "package SrcOpDir DestBinFilePath");
    cmd.Register("inspect", std::bind(&CmdInspect, std::placeholders::_1), "inspect BinFilePath");
    const char* argv1[] = {"program"};
    EXPECT_EQ(cmd.Run(1, argv1).Message(), "invalid args");
    const char* argv2[] = {"program", "cmd3", "arg1", "arg2"};
    EXPECT_EQ(cmd.Run(4, argv2).Message(), "not support cmd:cmd3");
    const char* argv3[] = {"program", "package", "cd", "arg2"};
    Status result = cmd.Run(4, argv3);
    EXPECT_TRUE(result.Ok());
}

TEST(StatusCmdTest, DoHelp)
{
    Cmd cmd;
    const char* argv1[] = {"program", "help", "arg1", "arg2"};
    Status result = cmd.Run(4, argv1);
    EXPECT_TRUE(result.Ok());
}
} // namespace Mki
