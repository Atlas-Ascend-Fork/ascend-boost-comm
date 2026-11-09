/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/statvfs.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <filesystem>

#include "mki/utils/log/log_sink_file.h"

namespace Mki
{
TEST(LogSinkFileTest, ClosesDescriptorZero)
{
    char dirTemplate[] = "/tmp/mki_logfd_XXXXXX";
    char *dir = mkdtemp(dirTemplate);
    ASSERT_NE(dir, nullptr);
    struct statvfs stats
    {
    };
    ASSERT_EQ(statvfs(dir, &stats), 0);
    constexpr uint64_t limit = 10ULL * 1024 * 1024 * 1024;
    if (static_cast<uint64_t>(stats.f_bsize) * stats.f_bfree <= limit)
    {
        (void)rmdir(dir);
        GTEST_SKIP() << "LogSinkFile requires more than 10 GiB free";
    }
    pid_t child = fork();
    ASSERT_GE(child, 0);
    if (child == 0)
    {
        if (setenv("ASCEND_PROCESS_LOG_PATH", dir, 1) != 0) _exit(2);
        bool opened = false;
        {
            LogSinkFile logger;
            close(STDIN_FILENO);
            logger.Log("probe\n", 6);
            opened = fcntl(STDIN_FILENO, F_GETFD) != -1;
        }
        errno = 0;
        bool closed = fcntl(STDIN_FILENO, F_GETFD) == -1 && errno == EBADF;
        _exit(opened && closed ? 0 : 1);
    }
    int status = 0;
    pid_t waited = waitpid(child, &status, 0);
    std::error_code error;
    std::filesystem::remove_all(dir, error);
    EXPECT_EQ(waited, child);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0);
    EXPECT_FALSE(error);
}
}  // namespace Mki
