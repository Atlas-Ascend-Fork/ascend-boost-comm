/*
 * Copyright (c) lzc12138. 2026. All rights reserved.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <dlfcn.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include "mki/utils/dl/dl.h"

namespace Mki
{
namespace
{
class DlValidityTest : public testing::Test
{
   protected:
    void SetUp() override
    {
        char directory[] = "/tmp/mki_dl_validity_XXXXXX";
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

    std::string directory_;
};

TEST_F(DlValidityTest, RejectsNonLibraryFile)
{
    const std::string path = directory_ + "/invalid.so";
    {
        std::ofstream output(path, std::ios::binary);
        ASSERT_TRUE(output.is_open());
        output << "This file is not an ELF shared library.\n";
        output.close();
        ASSERT_FALSE(output.fail());
    }
    Dl library(path);
    EXPECT_FALSE(library.IsValid());
    EXPECT_EQ(library.GetSymbol("malloc"), nullptr);
}

TEST_F(DlValidityTest, RejectsMissingFile)
{
    Dl library(directory_ + "/missing.so");
    EXPECT_FALSE(library.IsValid());
    EXPECT_EQ(library.GetSymbol("malloc"), nullptr);
}

TEST_F(DlValidityTest, LoadsExistingLibraryAndResolvesSymbol)
{
    // Discover the loaded C library without assuming a distribution-specific path.
    void *symbol = dlsym(RTLD_DEFAULT, "malloc");
    ASSERT_NE(symbol, nullptr);
    Dl_info info{};
    ASSERT_NE(dladdr(symbol, &info), 0);
    ASSERT_NE(info.dli_fname, nullptr);
    const auto path = std::filesystem::canonical(info.dli_fname);
    Dl library(path.string());
    EXPECT_TRUE(library.IsValid());
    EXPECT_NE(library.GetSymbol("malloc"), nullptr);
}
}  // namespace
}  // namespace Mki
