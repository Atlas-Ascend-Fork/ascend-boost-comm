/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <gtest/gtest.h>
#include <unistd.h>

#include <cstdlib>
#include <fstream>

#include "mki/utils/file_system/file_system.h"

namespace Mki
{
TEST(FileSystemTest, ReadFileReportsReadFailure)
{
    char dirTemplate[] = "/tmp/mki_read_file_XXXXXX";
    char *dir = mkdtemp(dirTemplate);
    ASSERT_NE(dir, nullptr);
    std::string path = std::string(dir) + "/data";
    {
        std::ofstream out(path, std::ios::binary);
        ASSERT_TRUE(out.is_open());
        out << "hello";
        ASSERT_TRUE(out.good());
    }
    uint8_t buffer[6] = {};
    bool valid = FileSystem::ReadFile(path, buffer, 5);
    std::string contents(reinterpret_cast<char *>(buffer), 5);
    bool tooLarge = FileSystem::ReadFile(path, buffer, 6);
    bool directory = FileSystem::ReadFile(dir, buffer, 1);
    (void)unlink(path.c_str());
    (void)rmdir(dir);
    EXPECT_TRUE(valid);
    EXPECT_EQ(contents, "hello");
    EXPECT_FALSE(tooLarge);
    EXPECT_FALSE(directory);
}

}  // namespace Mki
