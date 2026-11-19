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
#ifndef ATBOPS_ADDCUSTOM_TILING_DATA
#define ATBOPS_ADDCUSTOM_TILING_DATA

#include <cstdint>

namespace Mki
{
// 每个核至少处理的元素数。half 按 32 字节对齐时为 16 个元素，这里取更大的切分粒度。
constexpr uint32_t ADDCUSTOM_MIN_BLOCK_LENGTH = 32;
// DataCopy 搬运 half 时，元素个数需要 32 字节对齐。
constexpr uint32_t ADDCUSTOM_ELEM_ALIGN = 16;
// 核间切分按 64B cache line 对齐（32 个 half），保证不同核不会写同一条 cache line：
// 尾核可能整段走标量路径，若与相邻核的 DataCopy 结果共用一行，整行回写会互相覆盖。
constexpr uint32_t ADDCUSTOM_BLOCK_ALIGN = 32;
// 三块 half UB（x/y/z）共用约 96KB，单 tile 取 16384 个元素并保持对齐。
constexpr uint32_t ADDCUSTOM_MAX_TILE_LENGTH = 16384;

struct AddcustomTilingData
{
    uint32_t totalLength;   // 总数据长度
    uint32_t formerNum;     // 前 formerNum 个核对齐切分核数（blockDim>1 时为 blockDim-1）
    uint32_t formerLength;  // 前 formerNum 个核各自的元素个数，为 ADDCUSTOM_BLOCK_ALIGN 的倍数
    uint32_t tailLength;    // 最后一个核的元素个数，承接剩余尾部
};

struct AddcustomBlockRange
{
    uint32_t start;
    uint32_t length;
};

struct AddcustomTilePlan
{
    uint32_t tileLength;
    uint32_t alignLength;
};

#if defined(__CCE_AICORE__) || defined(__CCE_KT_TEST__)
#define ADDCUSTOM_FN __aicore__ inline
#else
#define ADDCUSTOM_FN inline
#endif

// 前 formerNum 个核各处理 formerLength 个元素（起点与长度均按 64B 对齐），其余核处理
// tailLength 个元素。
ADDCUSTOM_FN AddcustomBlockRange GetAddcustomBlockRange(const AddcustomTilingData &tiling, uint32_t blockIdx)
{
    AddcustomBlockRange range{0, 0};
    if (blockIdx < tiling.formerNum)
    {
        range.length = tiling.formerLength;
        range.start = static_cast<uint32_t>(static_cast<uint64_t>(blockIdx) * tiling.formerLength);
    }
    else
    {
        range.length = tiling.tailLength;
        uint64_t start = static_cast<uint64_t>(tiling.formerNum) * tiling.formerLength +
                         static_cast<uint64_t>(blockIdx - tiling.formerNum) * tiling.tailLength;
        range.start = static_cast<uint32_t>(start);
    }
    return range;
}

// 对齐部分走向量 DataCopy；不足对齐的尾元素由 kernel 经 UB 尾块搬运（910B 用
// DataCopyPad），不与对齐段共用同一 cache line 标量写回。
ADDCUSTOM_FN AddcustomTilePlan MakeAddcustomTilePlan(uint32_t blockLength)
{
    AddcustomTilePlan plan{0, 0};
    plan.alignLength = blockLength / ADDCUSTOM_ELEM_ALIGN * ADDCUSTOM_ELEM_ALIGN;
    if (plan.alignLength == 0)
    {
        return plan;
    }
    plan.tileLength = plan.alignLength < ADDCUSTOM_MAX_TILE_LENGTH ? plan.alignLength : ADDCUSTOM_MAX_TILE_LENGTH;
    return plan;
}

#undef ADDCUSTOM_FN
}  // namespace Mki
#endif
