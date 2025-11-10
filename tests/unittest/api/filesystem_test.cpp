/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <unistd.h>
#include <gtest/gtest.h>
#include "mki/utils/strings/match.h"
#include "mki/utils/file_system/file_system.h"
#include "mki/utils/log/log.h"

namespace Mki {
TEST(FileSystemTest, GetDirChildFiles)
{
    const char *mkiHome = std::getenv("MKI_HOME_PATH");
    if (!mkiHome) {
        return;
    }

    std::vector<std::string> filePaths;
    Mki::FileSystem::GetDirChildFiles(mkiHome, filePaths);
    for (const auto &filePath : filePaths) {
        MKI_LOG(INFO) << filePath;
    }
    ASSERT_TRUE(filePaths.size() > 0);
}

TEST(FileSystemTest, DirName)
{
    EXPECT_EQ(Mki::FileSystem::DirName("/usr/lib"), "/usr");
    EXPECT_EQ(Mki::FileSystem::DirName("/usr/"), "/");
    EXPECT_EQ(Mki::FileSystem::DirName("usr"), ".");
    EXPECT_EQ(Mki::FileSystem::DirName("/"), "/");
    EXPECT_EQ(Mki::FileSystem::DirName("."), ".");
    EXPECT_EQ(Mki::FileSystem::DirName(".."), ".");
    EXPECT_EQ(Mki::FileSystem::DirName("///abc//"), "/");
    EXPECT_EQ(Mki::FileSystem::DirName("///abc//ab"), "///abc");
    EXPECT_EQ(Mki::FileSystem::DirName("..///abc/"), "..");
}

TEST(FileSystemTest, GetDirChildItems)
{
    const char *mkiHome = std::getenv("MKI_HOME_PATH");
    if (!mkiHome) {
        return;
    }
    std::vector<std::string> itemPaths;
    std::vector<std::string> longPath(10000, std::string('a', 1));
    Mki::FileSystem::GetDirChildFiles(mkiHome, itemPaths);
    const std::string directoryPath = "/path/to/directory";
    std::vector<std::string> fileExtension;
    fileExtension.push_back(".txt");
    Mki::FileSystem::GetDirChildItems(directoryPath, fileExtension);
    Mki::FileSystem::GetDirChildItems(mkiHome, longPath);
    std::string str1 = Mki::FileSystem::PathCheckAndRegular("");
    const std::string longPath1(10000, 'a');
    EXPECT_EQ(str1, "");
    std::string str2 = Mki::FileSystem::PathCheckAndRegular(longPath1);
    EXPECT_EQ(str2, "");
    for (const auto &itemPath : itemPaths) {
        MKI_LOG(INFO) << itemPath;
    }
    ASSERT_TRUE(itemPaths.size() > 0);
}

TEST(FileSystemTest, GetDirChildDirs)
{
    const char *mkiHome = std::getenv("MKI_HOME_PATH");
    if (!mkiHome) {
        return;
    }
    std::vector<std::string> dirPaths;
    Mki::FileSystem::GetDirChildDirs(mkiHome, dirPaths);
    for (const auto &dirPath : dirPaths) {
        MKI_LOG(INFO) << dirPath;
    }
    ASSERT_TRUE(dirPaths.size() > 0);
}

TEST(FileSystemTest, RenameTest)
{
    Mki::FileSystem fs;
    std::string dirPath = "testdir";
    fs.MakeDir(dirPath, 0777);
    std::string newDirPath = "new_file";
    bool result = fs.Rename(dirPath, newDirPath);
    EXPECT_TRUE(result);
    fs.DeleteFile(dirPath);
}

TEST(FileSystemTest, MakeDirTest) {
    Mki::FileSystem fs;
    std::string dirPath = "testdir";
    bool success = fs.MakeDir(dirPath, 0777);
    EXPECT_TRUE(success);
    fs.DeleteFile(dirPath);
}

TEST(FileSystemTest, MakedirsTest)
{
    Mki::FileSystem fs;
    std::string tempDir = "testdir1";
    ASSERT_TRUE(fs.MakeDir(tempDir, 1));

    bool result = Mki::FileSystem::Makedirs(tempDir + "/subdir1/subdir2", 1);

    ASSERT_TRUE(result);
    ASSERT_TRUE(fs.Exists(tempDir + "/subdir1"));
    ASSERT_TRUE(fs.Exists(tempDir + "/subdir1/subdir2"));
    bool result1 = Mki::FileSystem::Makedirs("", 0);
    ASSERT_FALSE(result1);
    fs.DeleteFile(tempDir + "/subdir1/subdir2");
    fs.DeleteFile(tempDir + "/subdir1");
    fs.DeleteFile(tempDir);
}

TEST(FileSystemTest, PathCheckAndRegular)
{
    Mki::FileSystem fs;
    EXPECT_EQ(fs.PathCheckAndRegular(""), "");
    std::string longPath(10000, 'a');
    EXPECT_EQ(fs.PathCheckAndRegular(longPath), "");
    ASSERT_TRUE(fs.MakeDir("testdir1", 1));
    ASSERT_TRUE(fs.MakeDir("testdir1/subdir1", 1));
    ASSERT_TRUE(fs.MakeDir("testdir1/subdir1/subdir11", 1));
    ASSERT_TRUE(symlink("testdir1/subdir1/subdir11", "testdir2") != -1);
    ASSERT_TRUE(Mki::EndsWith(fs.PathCheckAndRegular("testdir1/./subdir1"), "testdir1/subdir1"));
    EXPECT_EQ(fs.PathCheckAndRegular("testdir1/subdir1/../", true, true), "");
    ASSERT_TRUE(Mki::EndsWith(fs.PathCheckAndRegular("testdir1/subdir1/../", true, false), "testdir1"));
    EXPECT_EQ(fs.PathCheckAndRegular("testdir2/", true), "");
    ASSERT_TRUE(Mki::EndsWith(fs.PathCheckAndRegular("testdir2/", false), "testdir1/subdir1/subdir11"));
    EXPECT_EQ(fs.PathCheckAndRegular("testdir2", true), "");
    ASSERT_TRUE(Mki::EndsWith(fs.PathCheckAndRegular("testdir2", false), "testdir1/subdir1/subdir11"));
    fs.DeleteFile("testdir2");
    fs.DeleteFile("testdir1/subdir1/subdir11");
    fs.DeleteFile("testdir1/subdir1");
    fs.DeleteFile("testdir1");
}
} // namespace Mki
