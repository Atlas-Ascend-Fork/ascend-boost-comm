/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <securec.h>
#include "mki/utils/assert/assert.h"
#include "mki/utils/log/log.h"
#include "mki/utils/status/status.h"

namespace Mki {
constexpr int MAX_LOG_STRING_SIZE = 1024;
Status::Status() noexcept : rep_(nullptr){};

Status::~Status()
{
    RemoveMsg();
};

Status::Status(const Status &other)
{
    if (other.rep_ == nullptr) {
        rep_ = nullptr;
        return;
    }

    int len = other.GetLen();
    if (len > MAX_LOG_STRING_SIZE) {
        return;
    }
    rep_ = new (std::nothrow) uint8_t[len];
    MKI_CHECK(rep_ != nullptr, "create msg holder failed", return);
    auto ret = memcpy_s(rep_, len, other.rep_, len);
    MKI_LOG_IF(ret != EOK, ERROR) << "memcpy failed";
}

Status &Status::operator=(const Status &other)
{
    if (this == &other) {
        return *this;
    }

    RemoveMsg();
    if (other.rep_ != nullptr) {
        int len = other.GetLen();
        if (len > MAX_LOG_STRING_SIZE) {
            return *this;
        }
        rep_ = new (std::nothrow) uint8_t[len];
        MKI_CHECK(rep_ != nullptr, "create msg holder failed", return *this);
        auto ret = memcpy_s(rep_, len, other.rep_, len);
        MKI_LOG_IF(ret != EOK, ERROR) << "memcpy failed";
    }

    return *this;
}

Status::Status(Status &&other) noexcept
{
    rep_ = other.rep_;
    other.rep_ = nullptr;
}

Status &Status::operator=(Status &&other)
{
    std::swap(rep_, other.rep_);
    return *this;
}

bool Status::Ok() const { return rep_ == nullptr; }

std::string Status::ToString() const
{
    if (rep_ == nullptr) {
        return "ok";
    }

    return "code:" + std::to_string(GetCode()) + ", msg:" + GetMsg();
}

int Status::Code() const
{
    if (rep_ == nullptr) {
        return 0;
    }
    return GetCode();
}

std::string Status::Message() const
{
    if (rep_ == nullptr) {
        return "";
    }
    return GetMsg();
}

Status::Status(int code, const std::string &msg)
{
    if (msg.size() > MAX_LOG_STRING_SIZE) {
        return;
    }
    int len = static_cast<int>(sizeof(int) + sizeof(int) + msg.size() + 1);
    rep_ = new (std::nothrow) uint8_t[len];
    MKI_CHECK(rep_ != nullptr, "create msg holder failed", return);
    int *lenPtr = reinterpret_cast<int *>(rep_);
    *lenPtr = len;
    int *codePtr = reinterpret_cast<int *>(rep_ + sizeof(int));
    *codePtr = code;
    char *msgPtr = reinterpret_cast<char *>(rep_ + sizeof(int) + sizeof(int));
    auto ret = memcpy_s(msgPtr, len - sizeof(int) - sizeof(int), msg.data(), msg.size());
    MKI_LOG_IF(ret != EOK, ERROR) << "memcpy failed";
    rep_[len - 1] = 0;
}

Status Status::OkStatus()
{
    Status st;
    return st;
}

Status Status::FailStatus(int code, const std::string &msg)
{
    Status st(code, msg);
    return st;
}

int Status::GetLen() const
{
    int *lenPtr = reinterpret_cast<int *>(rep_);
    return *lenPtr;
}

int Status::GetCode() const
{
    int *codePtr = reinterpret_cast<int *>(rep_ + sizeof(int));
    return *codePtr;
}

const char *Status::GetMsg() const
{
    char *msgPtr = reinterpret_cast<char *>(rep_ + sizeof(int) + sizeof(int));
    return msgPtr;
}

void Status::RemoveMsg()
{
    if (rep_ != nullptr) {
        delete[] rep_;
        rep_ = nullptr;
    }
}
} // namespace Mki
