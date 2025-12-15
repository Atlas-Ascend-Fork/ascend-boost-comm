/*
 * Copyright (c) 2025 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
 
#include <mki/base/operation_base.h>
#include <mki_loader/op_register.h>
#include <mki/utils/log/log.h>
#include "atbops/params/params.h"

namespace Mki {
using namespace Mki;

static constexpr int32_t INPUT_NUM  = 2;
static constexpr int32_t OUTPUT_NUM = 1;

class AddcustomOperation : public OperationBase {
public:
    explicit AddcustomOperation(const std::string &opName) noexcept : OperationBase(opName) {}

    /* ---------- 选择内核 ---------- */
    Kernel *GetBestKernel(const LaunchParam &launchParam) const override
    {
        MKI_CHECK(IsConsistent(launchParam), "Fail to check consistent", return nullptr);

        MKI_CHECK(launchParam.GetParam().Type() == typeid(OpParam::Addcustom),
                     "OpParam invalid", return nullptr);

        auto dtype = launchParam.GetInTensor(0).desc.dtype;
        MKI_CHECK(dtype == TENSOR_DTYPE_FLOAT16,
                     "Only Float16 supported", return nullptr);

        return GetKernelByName("AddcustomKernel");   // 内核名字必须与注册一致
    }

    /* ---------- 张量数量 ---------- */
    int64_t GetInputNum(const Any &specificParam) const override
    {
        (void)specificParam;
        return INPUT_NUM;
    }

    int64_t GetOutputNum(const Any &specificParam) const override
    {
        (void)specificParam;
        return OUTPUT_NUM;
    }

protected:
    /* ---------- 形状推导 ---------- */
    Status InferShapeImpl(const LaunchParam &launchParam,
                          SVector<Tensor> &outTensors) const override
    {
        MKI_CHECK(launchParam.GetParam().Type() == typeid(OpParam::Addcustom),
                     "no match param type",
                     return Status::FailStatus(ERROR_INFERSHAPE_ERROR, "OpParam invalid"));

        /* 输出形状 = 输入形状 */
        outTensors[0].desc = launchParam.GetInTensor(0).desc;
        return Status::OkStatus();
    }
};

/* ---------- 注册 ---------- */
REG_OPERATION(AddcustomOperation);

}  // namespace Mki