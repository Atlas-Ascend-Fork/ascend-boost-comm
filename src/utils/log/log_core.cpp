/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <cstdlib>
#include <string>
#include <cstring>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include "mki/utils/env/env.h"
#include "mki/utils/log/log_sink_stdout.h"
#include "mki/utils/log/log_sink_file.h"
#include "mki/utils/log/log.h"
#include "mki/utils/log/log_core.h"

namespace Mki {
static bool GetLogToStdoutFromEnvCfg()
{
    const char *envLogToStdout = std::getenv("ASCEND_SLOG_PRINT_TO_STDOUT");
    return envLogToStdout != nullptr && strlen(envLogToStdout) <= MAX_ENV_STRING_LEN &&
           strcmp(envLogToStdout, "1") == 0;
}

static void GetGlobalLogLevelFromEnvCfg(bool &isOpen, LogLevel &logLevel)
{
    const char *env = std::getenv("ASCEND_GLOBAL_LOG_LEVEL");
    if (env == nullptr || strlen(env) > MAX_ENV_STRING_LEN) {
        isOpen = true;
        logLevel = LogLevel::ERROR;
        return;
    }
    std::string envLogLevel(env);
    if (envLogLevel == "4") {
        isOpen = false;
        logLevel = LogLevel::ERROR;
        return;
    }
    static std::unordered_map<std::string, LogLevel> levelMap{{"0", LogLevel::TRACE},
                                                              {"1", LogLevel::INFO},
                                                              {"2", LogLevel::WARN},
                                                              {"3", LogLevel::ERROR}};
    auto levelIt = levelMap.find(envLogLevel);
    if (levelIt != levelMap.end()) {
        isOpen = true;
        logLevel = levelIt->second;
    } else {
        isOpen = true;
        logLevel = LogLevel::ERROR;
    };
}

static std::vector<std::string> splitString(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

static void GetModuleLogLevelFromEnvCfg(bool &isSet, bool &isOpen, LogLevel &logLevel)
{
    const char *env = std::getenv("ASCEND_MODULE_LOG_LEVEL");
    if (env == nullptr || strlen(env) > MAX_ENV_STRING_LEN) {
        isSet = false;
        isOpen = true;
        logLevel = LogLevel::ERROR;
        return;
    }
    std::string envs(env);
    std::vector<std::string> envLogLevels = splitString(envs, ':');
    std::string envLogLevel = "-1";

    for (const auto &i : envLogLevels) {
        std::vector<std::string> key_value = splitString(i, '=');
        if (key_value.size() >= 1) {
            if (key_value[0] == "ATB") {
                if (key_value.size() == 2) { // 2: correct key-value pair
                    isSet = true;
                    envLogLevel = key_value[1];
                }
                break;
            }
        }
    }
    if (!isSet) {
        isOpen = true;
        logLevel = LogLevel::ERROR;
        return;
    }
    if (envLogLevel == "4") {
        isOpen = false;
        logLevel = LogLevel::ERROR;
        return;
    }
    static std::unordered_map<std::string, LogLevel> levelMap{{"0", LogLevel::TRACE},
                                                              {"1", LogLevel::INFO},
                                                              {"2", LogLevel::WARN},
                                                              {"3", LogLevel::ERROR}};
    auto levelIt = levelMap.find(envLogLevel);
    if (levelIt != levelMap.end()) {
        isOpen = true;
        logLevel = levelIt->second;
    } else {
        isSet = false;
        isOpen = true;
        logLevel = LogLevel::ERROR;
    };
}

LogCore::LogCore()
{
    bool isModuleSet = false;
    bool isGlobalLogOpen = true;
    bool isModuleLogOpen = true;
    LogLevel globalLogLevel = LogLevel::ERROR;
    LogLevel moduleLogLevel = LogLevel::ERROR;

    GetGlobalLogLevelFromEnvCfg(isGlobalLogOpen, globalLogLevel);
    GetModuleLogLevelFromEnvCfg(isModuleSet, isModuleLogOpen, moduleLogLevel);

    bool isLogOpen = true;
    isLogOpen = isModuleSet ? isModuleLogOpen : isGlobalLogOpen;
    level_ = isModuleSet ? moduleLogLevel : globalLogLevel;

    if (isLogOpen && GetLogToStdoutFromEnvCfg()) {
        AddSink(std::make_shared<LogSinkStdout>());
    }
    if (isLogOpen) {
        AddSink(std::make_shared<LogSinkFile>());
    }
}

LogCore &LogCore::Instance()
{
    static LogCore logCore;
    return logCore;
}

LogLevel LogCore::GetLogLevel() const { return level_; }

void LogCore::SetLogLevel(LogLevel level) { level_ = level; }

void LogCore::Log(const char *log, uint64_t logLen)
{
    for (auto &sink : sinks_) {
        sink->Log(log, logLen);
    }
}

void LogCore::DeleteLogFileSink()
{
    sinks_.pop_back();
}

void LogCore::AddSink(std::shared_ptr<LogSink> sink)
{
    if (sink == nullptr) {
        MKI_LOG(ERROR) << "sink is NULL";
        return;
    }
    sinks_.push_back(sink);
}

const std::vector<std::shared_ptr<LogSink>> &LogCore::GetAllSinks() const { return sinks_; }
} // namespace Mki
