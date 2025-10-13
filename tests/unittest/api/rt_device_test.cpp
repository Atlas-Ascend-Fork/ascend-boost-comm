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
#include "mki/utils/rt/device/device.h"
#include "mki/utils/log/log.h"

namespace Mki {
TEST(RtDeviceTest, RtDeviceSetSocVersionTest)
{
    const char *setVersion = "Ascend910A";
    int status = MkiRtDeviceSetSocVersion(setVersion);
    EXPECT_EQ(status, MKIRT_SUCCESS);
}

TEST(RtDeviceTest, RtDeviceGetSocVersionTest)
{
    std::string getVersion;
    uint32_t maxLen = 100;
    int status = MkiRtDeviceGetSocVersion(getVersion, maxLen);
    EXPECT_EQ(status, MKIRT_SUCCESS);
    if (!getVersion.empty()) {
        MKI_LOG(INFO) << "DeviceVersion: " << getVersion;
    }
}

} // namespace Mki
