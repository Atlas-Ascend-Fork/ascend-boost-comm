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

#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#include "mki/utils/bin_file/bin_file.h"

namespace Mki
{
namespace
{
constexpr size_t PAYLOAD_SIZE = 4096;

class BinFileWriteTest : public testing::Test
{
   protected:
    void SetUp() override
    {
        char directory[] = "/tmp/mki_binfile_write_XXXXXX";
        const char *created = mkdtemp(directory);
        ASSERT_NE(created, nullptr);
        directory_ = created;
    }

    void TearDown() override
    {
        if (!directory_.empty())
        {
            std::error_code error;
            std::filesystem::remove_all(directory_, error);
            EXPECT_FALSE(error);
        }
    }

    void CheckLimitedWrite(size_t payloadBytes)
    {
        BinFile file;
        std::vector<char> payload(PAYLOAD_SIZE, 'x');
        ASSERT_TRUE(file.AddObject("payload", payload.data(), payload.size()).Ok());
        const std::string completePath = directory_ + "/complete.bin";
        ASSERT_TRUE(file.Write(completePath).Ok());
        const auto headerSize = std::filesystem::file_size(completePath) - payload.size();
        const rlim_t fileLimit = headerSize + payloadBytes;
        const std::string limitedPath = directory_ + "/limited.bin";

        // Change process-wide limits only in the child; use the real write(2).
        EXPECT_EXIT(
            {
                if (std::signal(SIGXFSZ, SIG_IGN) == SIG_ERR) _exit(2);
                struct rlimit limit;
                if (getrlimit(RLIMIT_FSIZE, &limit) != 0) _exit(3);
                limit.rlim_cur = fileLimit;
                if (setrlimit(RLIMIT_FSIZE, &limit) != 0) _exit(4);
                const auto result = file.Write(limitedPath);
                const bool failed = !result.Ok() && std::filesystem::file_size(limitedPath) == fileLimit;
                // Also clean up when GoogleTest re-executes this fixture for a death test.
                const bool cleaned = unlink(limitedPath.c_str()) == 0 && unlink(completePath.c_str()) == 0 &&
                                     rmdir(directory_.c_str()) == 0;
                _exit(failed && cleaned ? 0 : 1);
            },
            testing::ExitedWithCode(0), "");
    }

    std::string directory_;
};

TEST_F(BinFileWriteTest, ReportsPartialPayloadWrite) { CheckLimitedWrite(PAYLOAD_SIZE / 2); }

TEST_F(BinFileWriteTest, ReportsFailedPayloadWrite) { CheckLimitedWrite(0); }

TEST_F(BinFileWriteTest, CompletePayloadRoundTrips)
{
    BinFile writer;
    std::vector<char> payload(PAYLOAD_SIZE, 'x');
    ASSERT_TRUE(writer.AddObject("payload", payload.data(), payload.size()).Ok());
    const std::string path = directory_ + "/complete.bin";
    ASSERT_TRUE(writer.Write(path).Ok());

    BinFile reader;
    ASSERT_TRUE(reader.Read(path).Ok());
    std::vector<std::pair<std::string, std::pair<char *, uint64_t>>> objects;
    reader.GetAllObjects(objects);
    ASSERT_EQ(objects.size(), 1U);
    EXPECT_EQ(objects[0].first, "payload");
    ASSERT_EQ(objects[0].second.second, payload.size());
    EXPECT_EQ(std::vector<char>(objects[0].second.first, objects[0].second.first + payload.size()), payload);
}
}  // namespace
}  // namespace Mki
