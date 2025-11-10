/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <gtest/gtest.h>
#include "mki/utils/rt/backend/rt_backend.h"

namespace Mki {
TEST(RtBackendTest, ModuleCreate) {
    Mki::RtBackend rtBackend;
    MkiRtModuleInfo moduleInfo;
    MkiRtModule module;
    moduleInfo.type = MKIRT_MODULE_OBJECT;
    moduleInfo.version = 0;
    moduleInfo.data = 0;
    moduleInfo.dataLen = 0;
    moduleInfo.magic = 0x41494343U;
    ASSERT_EQ(rtBackend.ModuleCreate(nullptr, &module), MKIRT_ERROR_NOT_INITIALIZED);
    ASSERT_EQ(rtBackend.ModuleCreate(&moduleInfo, nullptr), MKIRT_ERROR_NOT_INITIALIZED);
}
} // namespace Mki
