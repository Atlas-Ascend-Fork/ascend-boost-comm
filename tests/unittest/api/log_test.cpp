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
#include <cstdlib>
#include "mki/utils/SVector/SVector.h"
#include "mki/utils/log/log.h"
#include "mki/utils/log/log_entity.h"
#include "mki/utils/log/log_sink_file.h"
#include "mki/utils/log/log_sink_stdout.h"
#include "mki/utils/log/log_core.h"

namespace Mki {
TEST(LogCore, InitTest1)
{
    char envGlobalLogLevel[] = "ASCEND_GLOBAL_LOG_LEVEL=4";
    char envModuleLogLevel[] = "ASCEND_MODULE_LOG_LEVEL=ATB=:$ASCEND_MODULE_LOG_LEVEL";
    char envLogToStdout[] = "ASCEND_SLOG_PRINT_TO_STDOUT=0";
    putenv(envGlobalLogLevel);
    putenv(envModuleLogLevel);
    putenv(envLogToStdout);
    Mki::LogCore logCore;
    Mki::LogLevel logLevel = logCore.GetLogLevel();
    std::vector<std::shared_ptr<Mki::LogSink>> sinks = logCore.GetAllSinks();
    EXPECT_EQ(logLevel, Mki::LogLevel::ERROR);
    EXPECT_EQ(sinks.size(), 0);
}

TEST(LogCore, InitTest2)
{
    char envGlobalLogLevel[] = "ASCEND_GLOBAL_LOG_LEVEL=1";
    char envModuleLogLevel[] = "ASCEND_MODULE_LOG_LEVEL=ATB=4:$ASCEND_MODULE_LOG_LEVEL";
    char envLogToStdout[] = "ASCEND_SLOG_PRINT_TO_STDOUT=1";
    putenv(envGlobalLogLevel);
    putenv(envModuleLogLevel);
    putenv(envLogToStdout);
    Mki::LogCore logCore;
    Mki::LogLevel logLevel = logCore.GetLogLevel();
    std::vector<std::shared_ptr<Mki::LogSink>> sinks = logCore.GetAllSinks();
    EXPECT_EQ(logLevel, Mki::LogLevel::ERROR);
    EXPECT_EQ(sinks.size(), 0);
}

TEST(LogCore, InitTest3)
{
    char envGlobalLogLevel[] = "ASCEND_GLOBAL_LOG_LEVEL=";
    char envModuleLogLevel[] = "ASCEND_MODULE_LOG_LEVEL=ATB=:$ASCEND_MODULE_LOG_LEVEL";
    char envLogToStdout[] = "ASCEND_SLOG_PRINT_TO_STDOUT=1";
    putenv(envGlobalLogLevel);
    putenv(envModuleLogLevel);
    putenv(envLogToStdout);
    Mki::LogCore logCore;
    Mki::LogLevel logLevel = logCore.GetLogLevel();
    std::vector<std::shared_ptr<Mki::LogSink>> sinks = logCore.GetAllSinks();
    EXPECT_EQ(logLevel, Mki::LogLevel::ERROR);
    EXPECT_EQ(sinks.size(), 2);
}

TEST(LogCore, InitTest4)
{
    char envGlobalLogLevel[] = "ASCEND_GLOBAL_LOG_LEVEL=1";
    char envModuleLogLevel[] = "ASCEND_MODULE_LOG_LEVEL=ATB=2:$ASCEND_MODULE_LOG_LEVEL";
    char envLogToStdout[] = "ASCEND_SLOG_PRINT_TO_STDOUT=0";
    putenv(envGlobalLogLevel);
    putenv(envModuleLogLevel);
    putenv(envLogToStdout);
    Mki::LogCore logCore;
    Mki::LogLevel logLevel = logCore.GetLogLevel();
    std::vector<std::shared_ptr<Mki::LogSink>> sinks = logCore.GetAllSinks();
    EXPECT_EQ(logLevel, Mki::LogLevel::WARN);
    EXPECT_EQ(sinks.size(), 1);
}

TEST(LogCore, InitTest5)
{
    char envGlobalLogLevel[] = "ASCEND_GLOBAL_LOG_LEVEL=0";
    char envModuleLogLevel[] = "ASCEND_MODULE_LOG_LEVEL=ATB=:$ASCEND_MODULE_LOG_LEVEL";
    char envLogToStdout[] = "ASCEND_SLOG_PRINT_TO_STDOUT=1";
    putenv(envGlobalLogLevel);
    putenv(envModuleLogLevel);
    putenv(envLogToStdout);
    Mki::LogCore logCore;
    Mki::LogLevel logLevel = logCore.GetLogLevel();
    std::vector<std::shared_ptr<Mki::LogSink>> sinks = logCore.GetAllSinks();
    EXPECT_EQ(logLevel, Mki::LogLevel::TRACE);
    EXPECT_EQ(sinks.size(), 2);
}

TEST(LogCore, InitTest6)
{
    unsetenv("ASCEND_SLOG_PRINT_TO_STDOUT");
    unsetenv("ASCEND_MODULE_LOG_LEVEL");
    unsetenv("ASCEND_GLOBAL_LOG_LEVEL");
    Mki::LogCore logCore;
    Mki::LogLevel logLevel = logCore.GetLogLevel();
    std::vector<std::shared_ptr<Mki::LogSink>> sinks = logCore.GetAllSinks();
    EXPECT_EQ(logLevel, Mki::LogLevel::ERROR);
    EXPECT_EQ(sinks.size(), 1);
}

TEST(LogCore, LogTest1)
{
    char envGlobalLogLevel[] = "ASCEND_GLOBAL_LOG_LEVEL=1";
    char envModuleLogLevel[] = "ASCEND_MODULE_LOG_LEVEL=ATB=:$ASCEND_MODULE_LOG_LEVEL";
    char envLogToStdout[] = "ASCEND_SLOG_PRINT_TO_STDOUT=1";
    putenv(envGlobalLogLevel);
    putenv(envModuleLogLevel);
    putenv(envLogToStdout);
    Mki::LogCore logCore;
    Mki::LogLevel logLevel = logCore.GetLogLevel();
    EXPECT_EQ(logLevel, Mki::LogLevel::INFO);
    constexpr int a = 111;
    constexpr int b = 222;
    MKI_FLOG(INFO, "value of number a is %d,value of number b is %d\n", a, b);
    MKI_FLOG(DEBUG, "value of number b is %d,value of number a is %d\n", b, a);
}

TEST(LogCore, LogTest2)
{
    char envGlobalLogLevel[] = "ASCEND_GLOBAL_LOG_LEVEL=1";
    char envModuleLogLevel[] = "ASCEND_MODULE_LOG_LEVEL=ATB=:$ASCEND_MODULE_LOG_LEVEL";
    char envLogToStdout[] = "ASCEND_SLOG_PRINT_TO_STDOUT=1";
    putenv(envGlobalLogLevel);
    putenv(envModuleLogLevel);
    putenv(envLogToStdout);
    Mki::LogCore logCore;
    Mki::LogLevel logLevel = logCore.GetLogLevel();
    EXPECT_EQ(logLevel, Mki::LogLevel::INFO);
    constexpr int a = 111;
    constexpr int b = 222;
    MKI_FLOG(INFO, "value of number a is %d,value of number b is %d", a, b);
    MKI_FLOG(DEBUG, "value of number b is %d,value of number a is %d", b, a);
    MKI_LOG(INFO) << "AAAAAAAAA";
}

TEST(LogCore, LogTest3)
{
    Mki::LogCore logCore;
    logCore.SetLogLevel(Mki::LogLevel::DEBUG);
    Mki::LogLevel logLevel = logCore.GetLogLevel();
    EXPECT_EQ(logLevel, Mki::LogLevel::DEBUG);
}

TEST(LogEntity, Trace) {
    EXPECT_EQ(std::string("trace"), Mki::LogLevelToString(Mki::LogLevel::TRACE));
}

TEST(LogEntity, Debug) {
    EXPECT_EQ(std::string("debug"), Mki::LogLevelToString(Mki::LogLevel::DEBUG));
}

TEST(LogEntity, Info) {
    EXPECT_EQ(std::string("info"), Mki::LogLevelToString(Mki::LogLevel::INFO));
}

TEST(LogEntity, Warn) {
    EXPECT_EQ(std::string("warn"), Mki::LogLevelToString(Mki::LogLevel::WARN));
}

TEST(LogEntity, Error) {
    EXPECT_EQ(std::string("error"), Mki::LogLevelToString(Mki::LogLevel::ERROR));
}

TEST(LogEntity, Fatal) {
    EXPECT_EQ(std::string("fatal"), Mki::LogLevelToString(Mki::LogLevel::FATAL));
}

TEST(LogEntity, Unknown) {
    EXPECT_EQ(std::string("unknown"), Mki::LogLevelToString(static_cast<Mki::LogLevel>(100)));
}

TEST(LogEntity, AddSink) {
    Mki::LogCore logCore;
    logCore.AddSink(std::make_shared<Mki::LogSinkStdout>());
}
} // namespace Mki
