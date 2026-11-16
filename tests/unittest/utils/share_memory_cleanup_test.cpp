/*
 * Copyright (c) lzc12138. 2026. All rights reserved.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <gtest/gtest.h>
#include <sys/resource.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <string>

#include "mki/utils/share_memory/share_memory.h"

namespace Mki
{
namespace
{
constexpr uint32_t SHARED_BYTES = 64;

TEST(ShareMemoryCleanupTest, InvalidSemaphoreNameCanBeDestroyed)
{
    EXPECT_EXIT(
        {
            struct rlimit noCore;
            noCore.rlim_cur = 0;
            noCore.rlim_max = 0;
            if (setrlimit(RLIMIT_CORE, &noCore) != 0) _exit(2);
            if (unsetenv("ATB_SHARE_MEMORY_NAME_SUFFIX") != 0) _exit(3);
            {
                ShareMemory memory("/mki/invalid", SHARED_BYTES);
                if (memory.GetShm() != nullptr) _exit(4);
            }
            _exit(0);
        },
        testing::ExitedWithCode(0), "");
}

TEST(ShareMemoryCleanupTest, ExhaustedFileDescriptorsCanBeDestroyed)
{
    EXPECT_EXIT(
        {
            struct rlimit noCore;
            noCore.rlim_cur = 0;
            noCore.rlim_max = 0;
            if (setrlimit(RLIMIT_CORE, &noCore) != 0) _exit(2);
            if (unsetenv("ATB_SHARE_MEMORY_NAME_SUFFIX") != 0) _exit(3);
            const std::string name = "/mki_fd_failure_" + std::to_string(getpid());
            struct rlimit previous;
            if (getrlimit(RLIMIT_NOFILE, &previous) != 0) _exit(4);
            struct rlimit limited = previous;
            limited.rlim_cur = 0;
            if (setrlimit(RLIMIT_NOFILE, &limited) != 0) _exit(5);
            {
                ShareMemory memory(name, SHARED_BYTES);
                if (setrlimit(RLIMIT_NOFILE, &previous) != 0) _exit(6);
                if (memory.GetShm() != nullptr) _exit(7);
            }
            _exit(0);
        },
        testing::ExitedWithCode(0), "");
}

TEST(ShareMemoryCleanupTest, SuccessfulConstructionStillReleasesSemaphore)
{
    EXPECT_EXIT(
        {
            if (unsetenv("ATB_SHARE_MEMORY_NAME_SUFFIX") != 0) _exit(2);
            const std::string name = "/mki_cleanup_" + std::to_string(getpid());
            bool initialized = false;
            {
                ShareMemory memory(name, SHARED_BYTES);
                initialized = memory.GetShm() != nullptr;
            }
            errno = 0;
            sem_t *semaphore = sem_open(name.c_str(), 0);
            const bool removed = semaphore == SEM_FAILED && errno == ENOENT;
            if (semaphore != SEM_FAILED)
            {
                sem_close(semaphore);
                sem_unlink(name.c_str());
            }
            _exit(initialized && removed ? 0 : 1);
        },
        testing::ExitedWithCode(0), "");
}
}  // namespace
}  // namespace Mki
