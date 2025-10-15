/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#include "kernel_operator.h"

namespace {
constexpr uint32_t TILING_LEN = 4 * 8; // 4 * 8 uint32 list

class Memset {
public:
    __aicore__ inline Memset() {}

    __aicore__ inline void Init(__gm__ uint8_t *tiling)
    {
        AscendC::GlobalTensor<uint32_t> tensorTilingGm;
        tensorTilingGm.SetGlobalBuffer((__gm__ uint32_t *)tiling);
        pipe_.InitBuffer(tilingBuf_, sizeof(uint32_t) * TILING_LEN);
        tilingTensor_ = tilingBuf_.Get<uint32_t>();
        DataCopy(tilingTensor_, tensorTilingGm, TILING_LEN);
        AscendC::PipeBarrier<PIPE_ALL>();

        maxUbSize_ = tilingTensor_.GetValue(3 * 8); // ub size offset, 3 list, each has 8 uint32
        AscendC::PipeBarrier<PIPE_ALL>();
        pipe_.InitBuffer(zeroBuf_, maxUbSize_);
        zeroTensor_ = zeroBuf_.Get<uint16_t>();
        Duplicate(zeroTensor_, (uint16_t)0, maxUbSize_ / sizeof(uint16_t));
        AscendC::SetFlag<AscendC::HardEvent::MTE3_V>(EVENT_ID0);
        AscendC::WaitFlag<AscendC::HardEvent::MTE3_V>(EVENT_ID0);
    }

    __aicore__ inline void CleanTensor(uint32_t idx, __gm__ uint8_t *tensor)
    {
        if (tensor == nullptr) {
            return;
        }
        uint32_t size = tilingTensor_.GetValue(idx);
        uint32_t sizeBlock = tilingTensor_.GetValue(8 + idx); // 8: offset of block
        uint32_t sizeLoop = tilingTensor_.GetValue(8 * 2 + idx); // 8 * 2: offset of loop

        uint32_t base = sizeBlock * AscendC::GetBlockIdx();
        AscendC::GlobalTensor<uint16_t> tensorGm;
        for (uint32_t i = 0; i < sizeBlock && (base + i) < size; i += sizeLoop) {
            uint32_t leftCount = size - (base + i);
            uint32_t handleCount = leftCount < sizeLoop ? leftCount : sizeLoop;
            tensorGm.SetGlobalBuffer((__gm__ uint16_t *)(tensor + base + i));
            DataCopy(tensorGm, zeroTensor_, handleCount / sizeof(uint16_t));
        }
    }

private:
    uint32_t maxUbSize_{0};
    AscendC::TPipe pipe_;
    AscendC::TBuf<AscendC::QuePosition::VECCALC> tilingBuf_;
    AscendC::TBuf<AscendC::QuePosition::VECCALC> zeroBuf_;
    AscendC::LocalTensor<uint32_t> tilingTensor_;
    AscendC::LocalTensor<uint16_t> zeroTensor_;
};
}

extern "C" __global__ __aicore__ void memory_set(GM_ADDR tensor0, GM_ADDR tensor1, GM_ADDR tensor2, GM_ADDR tensor3,
                                             GM_ADDR tensor4, GM_ADDR tensor5, GM_ADDR tensor6, GM_ADDR tensor7,
                                             GM_ADDR tiling)
{
    Memset kernel;
    kernel.Init(tiling);
    uint32_t idx = 0;
    kernel.CleanTensor(idx++, tensor0);
    kernel.CleanTensor(idx++, tensor1);
    kernel.CleanTensor(idx++, tensor2);
    kernel.CleanTensor(idx++, tensor3);
    kernel.CleanTensor(idx++, tensor4);
    kernel.CleanTensor(idx++, tensor5);
    kernel.CleanTensor(idx++, tensor6);
    kernel.CleanTensor(idx++, tensor7);
    AscendC::PipeBarrier<PIPE_ALL>();
}
