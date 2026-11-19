/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under
 * the terms and conditions of CANN Open Software License Agreement Version 2.0
 * (the "License"). Please refer to the License for details. You may not use
 * this file except in compliance with the License. THIS SOFTWARE IS PROVIDED ON
 * AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS
 * FOR A PARTICULAR PURPOSE. See LICENSE in the root of the software repository
 * for the full text of the License.
 */
#include "unpad_tiling.h"

#include <mki/utils/assert/assert.h>

#include <cstdint>
#include <limits>

#include "tiling_data.h"

namespace Mki
{
namespace
{
constexpr uint32_t UNPAD_MAX_BATCH_NUM = 64;
}

Status UnpadTiling(const LaunchParam &launchParam, KernelInfo &kernelInfo)
{
    UnpadTilingData *tilingDataPtr = reinterpret_cast<UnpadTilingData *>(kernelInfo.GetTilingHostAddr());
    MKI_CHECK(tilingDataPtr != nullptr, "tiling host addr is null",
              return Status::FailStatus(ERROR_INVALID_VALUE, "tiling host addr is null"));
    MKI_CHECK(launchParam.GetInTensorCount() > 0, "unpad input tensor is empty",
              return Status::FailStatus(ERROR_INVALID_VALUE, "unpad input tensor is empty"));

    const auto &dims = launchParam.GetInTensor(0).desc.dims;
    MKI_CHECK(dims.size() >= 2, "unpad input rank must be at least 2",
              return Status::FailStatus(ERROR_INVALID_VALUE, "unpad input rank must be at least 2"));
    const int64_t batch = dims[0];
    const int64_t padLength = dims[1];
    MKI_CHECK(batch > 0 && padLength > 0 && static_cast<uint64_t>(batch) <= UNPAD_MAX_BATCH_NUM &&
                  static_cast<uint64_t>(padLength) <= std::numeric_limits<uint32_t>::max(),
              "unpad batch or padLength is invalid",
              return Status::FailStatus(ERROR_INVALID_VALUE, "unpad batch or padLength is invalid"));

    tilingDataPtr->batch = static_cast<uint32_t>(batch);
    tilingDataPtr->padLength = static_cast<uint32_t>(padLength);
    kernelInfo.SetBlockDim(1);

    uint64_t sysWorkspaceSize = 16;
    kernelInfo.GetScratchSizes() = {sysWorkspaceSize};
    return Status::OkStatus();
}
}  // namespace Mki
