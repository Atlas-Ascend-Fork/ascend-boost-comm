/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#ifndef MKI_UTILS_LOG_LOG_H
#define MKI_UTILS_LOG_LOG_H
#include "mki/utils/log/log_stream.h"
#include "mki/utils/log/log_core.h"
#include "mki/utils/log/log_sink.h"
#include "mki/utils/log/log_entity.h"

#define MKI_LOG(level) MKI_LOG_##level

#define MKI_FLOG(level, format, ...) MKI_FLOG_##level(format, ##__VA_ARGS__)

#define MKI_LOG_IF(condition, level)                                                                             \
    if (condition)                                                                                               \
    MKI_LOG(level)

#define MKI_LOG_TRACE                                                                                            \
    if (Mki::LogLevel::TRACE >= Mki::LogCore::Instance().GetLogLevel())                                          \
    Mki::LogStream(__FILE__, __LINE__, __FUNCTION__, Mki::LogLevel::TRACE)
#define MKI_LOG_DEBUG                                                                                            \
    if (Mki::LogLevel::DEBUG >= Mki::LogCore::Instance().GetLogLevel())                                          \
    Mki::LogStream(__FILE__, __LINE__, __FUNCTION__, Mki::LogLevel::DEBUG)
#define MKI_LOG_INFO                                                                                             \
    if (Mki::LogLevel::INFO >= Mki::LogCore::Instance().GetLogLevel())                                           \
    Mki::LogStream(__FILE__, __LINE__, __FUNCTION__, Mki::LogLevel::INFO)
#define MKI_LOG_WARN                                                                                             \
    if (Mki::LogLevel::WARN >= Mki::LogCore::Instance().GetLogLevel())                                           \
    Mki::LogStream(__FILE__, __LINE__, __FUNCTION__, Mki::LogLevel::WARN)
#define MKI_LOG_ERROR                                                                                            \
    if (Mki::LogLevel::ERROR >= Mki::LogCore::Instance().GetLogLevel())                                          \
    Mki::LogStream(__FILE__, __LINE__, __FUNCTION__, Mki::LogLevel::ERROR)
#define MKI_LOG_FATAL                                                                                            \
    if (Mki::LogLevel::FATAL >= Mki::LogCore::Instance().GetLogLevel())                                          \
    Mki::LogStream(__FILE__, __LINE__, __FUNCTION__, Mki::LogLevel::FATAL)

#define MKI_FLOG_TRACE(format, ...)                                                                              \
    if (Mki::LogLevel::TRACE >= Mki::LogCore::Instance().GetLogLevel())                                          \
    Mki::LogStream(__FILE__, __LINE__, __FUNCTION__, Mki::LogLevel::TRACE).Format(format, ##__VA_ARGS__)
#define MKI_FLOG_DEBUG(format, ...)                                                                              \
    if (Mki::LogLevel::DEBUG >= Mki::LogCore::Instance().GetLogLevel())                                          \
    Mki::LogStream(__FILE__, __LINE__, __FUNCTION__, Mki::LogLevel::DEBUG).Format(format, ##__VA_ARGS__)
#define MKI_FLOG_INFO(format, ...)                                                                               \
    if (Mki::LogLevel::INFO >= Mki::LogCore::Instance().GetLogLevel())                                           \
    Mki::LogStream(__FILE__, __LINE__, __FUNCTION__, Mki::LogLevel::INFO).Format(format, ##__VA_ARGS__)
#define MKI_FLOG_WARN(format, ...)                                                                               \
    if (Mki::LogLevel::WARN >= Mki::LogCore::Instance().GetLogLevel())                                           \
    Mki::LogStream(__FILE__, __LINE__, __FUNCTION__, Mki::LogLevel::WARN).Format(format, ##__VA_ARGS__)
#define MKI_FLOG_ERROR(format, ...)                                                                              \
    if (Mki::LogLevel::ERROR >= Mki::LogCore::Instance().GetLogLevel())                                          \
    Mki::LogStream(__FILE__, __LINE__, __FUNCTION__, Mki::LogLevel::ERROR).Format(format, ##__VA_ARGS__)
#define MKI_FLOG_FATAL(format, ...)                                                                              \
    if (Mki::LogLevel::FATAL >= Mki::LogCore::Instance().GetLogLevel())                                          \
    Mki::LogStream(__FILE__, __LINE__, __FUNCTION__, Mki::LogLevel::FATAL).Format(format, ##__VA_ARGS__)

#endif
