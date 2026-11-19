/*
 * Copyright (c) 2025 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under
 * the terms and conditions of CANN Open Software License Agreement Version 2.0
 * (the "License"). Please refer to the License for details. You may not use
 * this file except in compliance with the License. THIS SOFTWARE IS PROVIDED ON
 * AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS
 * FOR A PARTICULAR PURPOSE. See LICENSE in the root of the software repository
 * for the full text of the License.
 */

#include "addcustom_tiling.h"

#include <mki/utils/assert/assert.h>
#include <mki/utils/log/log.h>
#include <mki/utils/platform/platform_info.h>

#include <algorithm>
#include <cstdint>
#include <limits>

#include "atbops/params/addcustom.h"
#include "tiling_data.h"

namespace Mki
{
Status BuildAddcustomTiling(uint32_t totalLength, uint32_t coreNum, AddcustomTilingData &tilingData, uint32_t &blockDim)
{
    MKI_CHECK(totalLength > 0, "addcustom totalLength must be positive",
              return Status::FailStatus(ERROR_INVALID_VALUE, "addcustom totalLength must be positive"));
    MKI_CHECK(coreNum > 0, "addcustom coreNum must be positive",
              return Status::FailStatus(ERROR_INVALID_VALUE, "addcustom coreNum must be positive"));

    if (totalLength < ADDCUSTOM_MIN_BLOCK_LENGTH)
    {
        blockDim = 1;
    }
    else
    {
        // 下取整保证平均每核至少 ADDCUSTOM_MIN_BLOCK_LENGTH，避免对齐损失全部压到尾核。
        uint32_t needCore = totalLength / ADDCUSTOM_MIN_BLOCK_LENGTH;
        blockDim = std::min(needCore, coreNum);
    }
    MKI_CHECK(blockDim > 0 && blockDim <= coreNum, "addcustom blockDim is invalid",
              return Status::FailStatus(ERROR_INVALID_VALUE, "addcustom blockDim is invalid"));

    tilingData.totalLength = totalLength;
    // 前若干核按 ADDCUSTOM_BLOCK_ALIGN（64B = 32 个 half）对齐切分：既保证 DataCopy 的
    // GM 基址 32B 对齐，也保证各核不共用同一条 cache line（尾核可能整段走标量路径）。
    // 先按平均值向上对齐确定前核长度，再由前核长度反推实际前核数，最后收缩 blockDim
    // 以不启动空核。
    // 尾核长度的保证分两种情况：
    //   1) 前核数未受核数上限约束（frontCores <= blockDim-1）：1 <= tailLength <= formerLength；
    //   2) 前核数被核数上限截断（frontCores = blockDim-1）：64B 对齐可能使尾核大于前核，
    //      但始终满足 tailLength <= avgLength + blockDim - 1（多余量来自对齐取整，
    //      例如 totalLength=1583、48 核时前核 32、尾核 79）。
    if (blockDim == 1)
    {
        tilingData.formerNum = 1;
        tilingData.formerLength = totalLength;
        tilingData.tailLength = totalLength;
    }
    else
    {
        const uint32_t avgLength = totalLength / blockDim;
        uint32_t frontLength = (avgLength + ADDCUSTOM_BLOCK_ALIGN - 1) / ADDCUSTOM_BLOCK_ALIGN * ADDCUSTOM_BLOCK_ALIGN;
        if (frontLength == 0)
        {
            frontLength = ADDCUSTOM_BLOCK_ALIGN;
        }
        // frontCores * frontLength < totalLength，保证 1 <= tailLength；不超过 blockDim-1。
        uint32_t frontCores = (totalLength - 1) / frontLength;
        const uint32_t maxFrontCores = blockDim - 1;
        if (frontCores > maxFrontCores)
        {
            frontCores = maxFrontCores;
        }
        if (frontCores == 0)
        {
            blockDim = 1;
            tilingData.formerNum = 1;
            tilingData.formerLength = totalLength;
            tilingData.tailLength = totalLength;
        }
        else
        {
            // 按实际前核数收缩 blockDim，避免启动多余的空核。
            blockDim = frontCores + 1;
            tilingData.formerNum = frontCores;
            tilingData.formerLength = frontLength;
            tilingData.tailLength = totalLength - frontCores * frontLength;
        }
    }
    MKI_CHECK(blockDim > 0 && blockDim <= coreNum, "addcustom blockDim is invalid",
              return Status::FailStatus(ERROR_INVALID_VALUE, "addcustom blockDim is invalid"));

    const uint64_t covered = static_cast<uint64_t>(tilingData.formerNum) * tilingData.formerLength +
                             static_cast<uint64_t>(blockDim - tilingData.formerNum) * tilingData.tailLength;
    MKI_CHECK(covered == totalLength, "addcustom tiling does not cover totalLength",
              return Status::FailStatus(ERROR_INVALID_VALUE, "addcustom tiling does not cover totalLength"));
    MKI_CHECK(tilingData.formerLength > 0 && tilingData.tailLength > 0, "addcustom block length is invalid",
              return Status::FailStatus(ERROR_INVALID_VALUE, "addcustom block length is invalid"));
    if (blockDim > 1)
    {
        MKI_CHECK(tilingData.formerLength % ADDCUSTOM_BLOCK_ALIGN == 0,
                  "addcustom formerLength must be cache-line aligned",
                  return Status::FailStatus(ERROR_INVALID_VALUE, "addcustom formerLength must be cache-line aligned"));
    }

    return Status::OkStatus();
}

Status AddcustomTiling(const LaunchParam &launchParam, KernelInfo &kernelInfo)
{
    AddcustomTilingData *tilingDataPointer = reinterpret_cast<AddcustomTilingData *>(kernelInfo.GetTilingHostAddr());
    MKI_CHECK(tilingDataPointer != nullptr, "tilingDataPtr should not be empty",
              return Status::FailStatus(ERROR_INVALID_VALUE, "tilingDataPtr should not be empty"));

    MKI_CHECK(launchParam.GetParam().Type() == typeid(OpParam::Addcustom),
              "Failed to check addcustom param, type of specificParam is not "
              "equals to OpParam::Addcustom",
              return Status::FailStatus(ERROR_ATTR_INVALID_TYPE,
                                        "Failed to check addcustom param, type of specificParam is not "
                                        "equals to OpParam::Addcustom"));

    MKI_CHECK(launchParam.GetInTensorCount() > 0, "addcustom input tensor is empty",
              return Status::FailStatus(ERROR_INVALID_VALUE, "addcustom input tensor is empty"));
    const auto &inTensor = launchParam.GetInTensor(0);
    MKI_CHECK(inTensor.desc.dims.size() >= 1, "addcustom input rank must be at least 1",
              return Status::FailStatus(ERROR_INVALID_VALUE, "addcustom input rank must be at least 1"));
    const int64_t length = static_cast<int64_t>(inTensor.desc.Numel());
    MKI_CHECK(length > 0 && static_cast<uint64_t>(length) <= std::numeric_limits<uint32_t>::max(),
              "addcustom totalLength is invalid",
              return Status::FailStatus(ERROR_INVALID_VALUE, "addcustom totalLength is invalid"));

    uint32_t coreNum = PlatformInfo::Instance().GetCoreNum(CoreType::CORE_TYPE_VECTOR);
    MKI_LOG(INFO) << "Total length is " << length << ", core number is " << coreNum;

    uint32_t blockDim = 0;
    Status status = BuildAddcustomTiling(static_cast<uint32_t>(length), coreNum, *tilingDataPointer, blockDim);
    MKI_CHECK(status.Ok(), "failed to build addcustom tiling", return status);

    MKI_LOG(INFO) << "BlockDim is " << blockDim;
    MKI_LOG(INFO) << "Former number is " << tilingDataPointer->formerNum << ", former length is "
                  << tilingDataPointer->formerLength << ", tail length is " << tilingDataPointer->tailLength;
    kernelInfo.SetBlockDim(blockDim);
    return Status::OkStatus();
}
}  // namespace Mki
