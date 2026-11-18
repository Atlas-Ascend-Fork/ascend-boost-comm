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

#include "mki/utils/log/log.h"
#include "mki/utils/rt/device/device.h"

namespace Mki
{
TEST(RtDeviceTest, RtDeviceSetSocVersionTest)
{
    // 使用当前硬件真实的 soc 版本，不写死具体型号。rtSetSocVersion 改写的是进程级全局设定且没有
    // 复位接口，写入与硬件不符的值会让同进程内后续所有 aclrtSetDevice 返回
    // ACL_ERROR_RT_DEV_SETUP_ERROR(507033)；写回原值则不会改变任何设备状态。
    std::string realVersion;
    ASSERT_EQ(MkiRtDeviceGetSocVersion(realVersion, 100), MKIRT_SUCCESS);
    ASSERT_FALSE(realVersion.empty());

    int status = MkiRtDeviceSetSocVersion(realVersion.c_str());
    EXPECT_EQ(status, MKIRT_SUCCESS);
}

TEST(RtDeviceTest, RtDeviceGetSocVersionTest)
{
    std::string getVersion;
    uint32_t maxLen = 100;
    int status = MkiRtDeviceGetSocVersion(getVersion, maxLen);
    EXPECT_EQ(status, MKIRT_SUCCESS);
    if (!getVersion.empty())
    {
        MKI_LOG(INFO) << "DeviceVersion: " << getVersion;
    }
}

}  // namespace Mki
