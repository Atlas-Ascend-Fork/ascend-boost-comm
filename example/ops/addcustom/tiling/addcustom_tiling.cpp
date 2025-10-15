/*
 * Copyright (c) 2025 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
 
#include "addcustom_tiling.h"
#include <mki/utils/assert/assert.h>
#include <mki/utils/log/log.h>
#include <mki/utils/platform/platform_info.h>
#include <mki/utils/math/math.h>
#include <mki/utils/SVector/SVector.h>
#include "atbops/params/addcustom.h"
#include "tiling_data.h"

// 定义最小的块长度
constexpr uint32_t MIN_BLOCK_LENGTH = 32;

namespace Mki {
Status AddcustomTiling(const LaunchParam &launchParam, KernelInfo &kernelInfo)
{
    AddcustomTilingData *tilingDataPointer =
        reinterpret_cast<AddcustomTilingData *>(kernelInfo.GetTilingHostAddr());
    MKI_CHECK(tilingDataPointer != nullptr, "tilingDataPtr should not be empty",
              return Status::FailStatus(ERROR_INVALID_VALUE, "tilingDataPtr should not be empty"));

    if (launchParam.GetParam().Type() != typeid(OpParam::Addcustom)) {
        return Status::FailStatus(
            ERROR_ATTR_INVALID_TYPE,
            "Failed to check addcustom param, type of specificParam is not equals to OpParam::Addcustom");
    }

    // 获取输入张量的维度
    const uint32_t totalLength = launchParam.GetInTensor(0).desc.dims.at(0);
    MKI_LOG(INFO) << "Total length is " << totalLength;

    // 计算核心数
    uint32_t coreNum = PlatformInfo::Instance().GetCoreNum(CoreType::CORE_TYPE_VECTOR);
    MKI_LOG(INFO) << "Core number is " << coreNum;

    // 计算每个核心的块数
    uint32_t blockDims = 2;
    // uint32_t blockDims = std::min<uint32_t>((totalLength + MIN_BLOCK_LENGTH - 1) / MIN_BLOCK_LENGTH, coreNum);
    tilingDataPointer->tileNum = blockDims;
    tilingDataPointer->totalLength = totalLength;

    MKI_LOG(INFO) << "BlockDims is " << blockDims;
    MKI_LOG(INFO) << "Total length is " << tilingDataPointer->totalLength;
    MKI_LOG(INFO) << "Tile number is " << tilingDataPointer->tileNum;

    kernelInfo.SetBlockDim(blockDims);
    return Status::OkStatus();
}
} // namespace Mki