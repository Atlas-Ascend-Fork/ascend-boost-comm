/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under
 * the terms and conditions of CANN Open Software License Agreement Version 2.0
 * (the "License"). Please refer to the License for details. You may not use
 * this file except in compliance with the License. THIS SOFTWARE IS PROVIDED ON
 * AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS
 * FOR A PARTICULAR PURPOSE. See the License in the root of the software
 * repository for the full text of the License.
 */
#include "mki/utils/rt/stream/stream.h"
#include <acl/acl.h>
#include <cstdint>
#include <dlfcn.h>
#include <gtest/gtest.h>

namespace {
constexpr uintptr_t TEST_STREAM_VALUE = 0x1234;
constexpr aclError TEST_ACL_ERROR = static_cast<aclError>(-1234);

bool g_captureAclCreate = false;
uint32_t g_capturedPriority = 0;
uint32_t g_capturedFlags = 0;
int g_aclCreateCallCount = 0;
aclError g_aclCreateResult = ACL_ERROR_NONE;

using AclCreateStreamWithConfig = aclError (*)(aclrtStream *, uint32_t,
                                               uint32_t);

void ResetAclCreateCapture() {
  g_captureAclCreate = false;
  g_capturedPriority = 0;
  g_capturedFlags = 0;
  g_aclCreateCallCount = 0;
  g_aclCreateResult = ACL_ERROR_NONE;
}
} // namespace

extern "C" aclError aclrtCreateStreamWithConfig(aclrtStream *stream,
                                                uint32_t priority,
                                                uint32_t flags) {
  if (g_captureAclCreate) {
    ++g_aclCreateCallCount;
    g_capturedPriority = priority;
    g_capturedFlags = flags;
    if (g_aclCreateResult == ACL_ERROR_NONE && stream != nullptr) {
      *stream = reinterpret_cast<aclrtStream>(TEST_STREAM_VALUE);
    }
    return g_aclCreateResult;
  }

  static const auto realAclCreate = reinterpret_cast<AclCreateStreamWithConfig>(
      dlsym(RTLD_NEXT, "aclrtCreateStreamWithConfig"));
  if (realAclCreate == nullptr) {
    return static_cast<aclError>(-1);
  }
  return realAclCreate(stream, priority, flags);
}

namespace Mki {
class StreamPriorityTest : public ::testing::Test {
protected:
  void SetUp() override {
    ResetAclCreateCapture();
    g_captureAclCreate = true;
  }

  void TearDown() override { g_captureAclCreate = false; }
};

TEST_F(StreamPriorityTest, DefaultPriorityIsForwarded) {
  MkiRtStream stream = nullptr;
  EXPECT_EQ(MkiRtStreamCreate(&stream), MKIRT_SUCCESS);
  EXPECT_EQ(stream, reinterpret_cast<MkiRtStream>(TEST_STREAM_VALUE));
  EXPECT_EQ(g_aclCreateCallCount, 1);
  EXPECT_EQ(g_capturedPriority, 0U);
  EXPECT_EQ(g_capturedFlags, 0U);
}

TEST_F(StreamPriorityTest, PositiveBoundaryPriorityIsForwarded) {
  MkiRtStream stream = nullptr;
  EXPECT_EQ(MkiRtStreamCreate(&stream, 7), MKIRT_SUCCESS);
  EXPECT_EQ(g_aclCreateCallCount, 1);
  EXPECT_EQ(g_capturedPriority, 7U);
  EXPECT_EQ(g_capturedFlags, 0U);
}

TEST_F(StreamPriorityTest, NegativeBoundaryPriorityIsForwarded) {
  constexpr int32_t priority = -1;
  MkiRtStream stream = nullptr;
  EXPECT_EQ(MkiRtStreamCreate(&stream, priority), MKIRT_SUCCESS);
  EXPECT_EQ(g_aclCreateCallCount, 1);
  EXPECT_EQ(g_capturedPriority, static_cast<uint32_t>(priority));
  EXPECT_EQ(g_capturedFlags, 0U);
}

TEST_F(StreamPriorityTest, RuntimeErrorCodeIsPreserved) {
  g_aclCreateResult = TEST_ACL_ERROR;
  MkiRtStream stream = nullptr;
  EXPECT_EQ(MkiRtStreamCreate(&stream, 1), static_cast<int>(TEST_ACL_ERROR));
  EXPECT_EQ(stream, nullptr);
  EXPECT_EQ(g_aclCreateCallCount, 1);
  EXPECT_EQ(g_capturedPriority, 1U);
  EXPECT_EQ(g_capturedFlags, 0U);
}
} // namespace Mki
