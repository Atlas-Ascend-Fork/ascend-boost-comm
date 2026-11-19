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

#include "kernel_operator.h"
#include "ops/addcustom/tiling/tiling_data.h"
#include "ops/utils/common/kernel/kernel_utils.h"

static constexpr uint32_t BUFFER_NUM = 1;

class Addcustom
{
   public:
    __aicore__ inline Addcustom() {}
    __aicore__ inline void Init(GM_ADDR x, GM_ADDR y, GM_ADDR z, const Mki::AddcustomTilingData &tiling)
    {
        Mki::AddcustomBlockRange range = Mki::GetAddcustomBlockRange(tiling, AscendC::GetBlockIdx());
        this->blockLength = range.length;
        xGm.SetGlobalBuffer((__gm__ half *)x + range.start, this->blockLength);
        yGm.SetGlobalBuffer((__gm__ half *)y + range.start, this->blockLength);
        zGm.SetGlobalBuffer((__gm__ half *)z + range.start, this->blockLength);

        Mki::AddcustomTilePlan plan = Mki::MakeAddcustomTilePlan(this->blockLength);
        this->tileLength = plan.tileLength;
        this->alignLength = plan.alignLength;
        // 纯尾块（blockLength < ADDCUSTOM_ELEM_ALIGN）时 tileLength 为 0，
        // 但尾块搬运仍需一段至少 ELEM_ALIGN 的 UB，这里统一按需申请。
        uint32_t ubLength = this->tileLength;
        if (ubLength == 0 && this->blockLength > 0)
        {
            ubLength = Mki::ADDCUSTOM_ELEM_ALIGN;
        }
        if (ubLength > 0)
        {
            pipe.InitBuffer(inQueueX, BUFFER_NUM, ubLength * sizeof(half));
            pipe.InitBuffer(inQueueY, BUFFER_NUM, ubLength * sizeof(half));
            pipe.InitBuffer(outQueueZ, BUFFER_NUM, ubLength * sizeof(half));
        }
    }
    __aicore__ inline void Process()
    {
        if (this->blockLength == 0)
        {
            return;
        }
#if defined(__CCE_KT_TEST__) || (__CCE_AICORE__ == 220)
        ProcessVector();
        ProcessTailPad();
#else
        // Ascend910 不支持 DataCopyPad。存在非对齐尾元素时整段走标量，避免对齐段
        // DataCopy(MTE3) 与尾段标量写混用同一条 64B cache line（DataCopy 完成后
        // FreeTensor 不等待 MTE3 落地，标量写回整行会覆盖已算好的前半行）。
        if (this->alignLength < this->blockLength)
        {
            ProcessScalar(0, this->blockLength);
            return;
        }
        ProcessVector();
#endif
    }

   private:
    // 对齐段：GM -> UB -> 向量 Add -> GM，长度均按 ADDCUSTOM_ELEM_ALIGN 对齐。
    __aicore__ inline void ProcessVector()
    {
        uint32_t offset = 0;
        while (this->tileLength > 0 && offset < this->alignLength)
        {
            uint32_t len = this->tileLength;
            if (len > this->alignLength - offset)
            {
                len = this->alignLength - offset;
            }
            CopyIn(offset, len);
            Compute(len);
            CopyOut(offset, len);
            offset += len;
        }
    }
#if defined(__CCE_KT_TEST__) || (__CCE_AICORE__ == 220)
    // 尾块同样经 UB 搬运，只是 GM 侧长度按真实尾长（< ADDCUSTOM_ELEM_ALIGN）搬入搬出，
    // 因此不会和前面 DataCopy 写入的区域共用 cache line 造成覆盖。
    __aicore__ inline void ProcessTailPad()
    {
        if (this->alignLength >= this->blockLength)
        {
            return;
        }
        const uint32_t offset = this->alignLength;
        const uint32_t tailLength = this->blockLength - this->alignLength;
        AscendC::DataCopyExtParams copyParams{1, static_cast<uint32_t>(tailLength * sizeof(half)), 0, 0, 0};
        // 尾块不足一个对齐长度，右侧补 0，保证向量 Add 读到的 UB 数据确定。
        AscendC::DataCopyPadExtParams<half> padParams{
            true, 0, static_cast<uint8_t>(Mki::ADDCUSTOM_ELEM_ALIGN - tailLength), static_cast<half>(0)};

        AscendC::LocalTensor<half> xLocal = inQueueX.AllocTensor<half>();
        AscendC::LocalTensor<half> yLocal = inQueueY.AllocTensor<half>();
        AscendC::DataCopyPad(xLocal, xGm[offset], copyParams, padParams);
        AscendC::DataCopyPad(yLocal, yGm[offset], copyParams, padParams);
        inQueueX.EnQue(xLocal);
        inQueueY.EnQue(yLocal);

        xLocal = inQueueX.DeQue<half>();
        yLocal = inQueueY.DeQue<half>();
        AscendC::LocalTensor<half> zLocal = outQueueZ.AllocTensor<half>();
        AscendC::Add(zLocal, xLocal, yLocal, Mki::ADDCUSTOM_ELEM_ALIGN);
        outQueueZ.EnQue<half>(zLocal);
        inQueueX.FreeTensor(xLocal);
        inQueueY.FreeTensor(yLocal);

        zLocal = outQueueZ.DeQue<half>();
        // 搬出只写真实尾长字节，补 0 部分不会落到 GM。
        AscendC::DataCopyPad(zGm[offset], zLocal, copyParams);
        outQueueZ.FreeTensor(zLocal);
    }
#else
    // Ascend910 兜底：标量逐元素计算。调用方保证此时该核没有 DataCopy 写过同一段数据。
    __aicore__ inline void ProcessScalar(uint32_t offset, uint32_t len)
    {
        for (uint32_t i = offset; i < offset + len; ++i)
        {
            float xVal = static_cast<float>(xGm.GetValue(i));
            float yVal = static_cast<float>(yGm.GetValue(i));
            zGm.SetValue(i, static_cast<half>(xVal + yVal));
        }
    }
#endif
    __aicore__ inline void CopyIn(uint32_t offset, uint32_t len)
    {
        AscendC::LocalTensor<half> xLocal = inQueueX.AllocTensor<half>();
        AscendC::LocalTensor<half> yLocal = inQueueY.AllocTensor<half>();
        AscendC::DataCopy(xLocal, xGm[offset], len);
        AscendC::DataCopy(yLocal, yGm[offset], len);
        inQueueX.EnQue(xLocal);
        inQueueY.EnQue(yLocal);
    }
    __aicore__ inline void Compute(uint32_t len)
    {
        AscendC::LocalTensor<half> xLocal = inQueueX.DeQue<half>();
        AscendC::LocalTensor<half> yLocal = inQueueY.DeQue<half>();
        AscendC::LocalTensor<half> zLocal = outQueueZ.AllocTensor<half>();
        AscendC::Add(zLocal, xLocal, yLocal, len);
        outQueueZ.EnQue<half>(zLocal);
        inQueueX.FreeTensor(xLocal);
        inQueueY.FreeTensor(yLocal);
    }
    __aicore__ inline void CopyOut(uint32_t offset, uint32_t len)
    {
        AscendC::LocalTensor<half> zLocal = outQueueZ.DeQue<half>();
        AscendC::DataCopy(zGm[offset], zLocal, len);
        outQueueZ.FreeTensor(zLocal);
    }

   private:
    AscendC::TPipe pipe;
    AscendC::TQue<AscendC::QuePosition::VECIN, BUFFER_NUM> inQueueX, inQueueY;
    AscendC::TQue<AscendC::QuePosition::VECOUT, BUFFER_NUM> outQueueZ;
    AscendC::GlobalTensor<half> xGm;
    AscendC::GlobalTensor<half> yGm;
    AscendC::GlobalTensor<half> zGm;
    uint32_t blockLength{0};
    uint32_t alignLength{0};
    uint32_t tileLength{0};
};

inline __aicore__ void InitTilingData(const __gm__ uint8_t *p_tilingdata, Mki::AddcustomTilingData *tilingdata)
{
    const __gm__ uint32_t *data = reinterpret_cast<const __gm__ uint32_t *>(p_tilingdata);
    tilingdata->totalLength = data[0];
    tilingdata->formerNum = data[1];
    tilingdata->formerLength = data[2];
    tilingdata->tailLength = data[3];
}

extern "C" __global__ __aicore__ void addcustom(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR tiling)
{
    Addcustom op;
    Mki::AddcustomTilingData tdata;
    InitTilingData(tiling, &tdata);
    op.Init(x, y, z, tdata);
    op.Process();
}
