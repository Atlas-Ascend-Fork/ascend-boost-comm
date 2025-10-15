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
#include "mki/types.h"
namespace {
using namespace Mki;
TEST(GetTensorElementSizeTest, HandleValidInput) {
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_UNDEFINED), 0);
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_FLOAT), sizeof(float));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_FLOAT16), 2);
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_INT8), sizeof(int8_t));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_INT32), sizeof(int32_t));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_UINT8), sizeof(uint8_t));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_INT16), sizeof(int16_t));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_UINT16), sizeof(uint16_t));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_UINT32), sizeof(uint32_t));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_INT64), sizeof(int64_t));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_UINT64), sizeof(uint64_t));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_DOUBLE), sizeof(double));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_BOOL), sizeof(bool));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_BF16), 2);
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_COMPLEX64), sizeof(double));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_COMPLEX128), 16);
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_INT4), sizeof(bool));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_UINT1), sizeof(bool));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_COMPLEX32), sizeof(int32_t));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_HIFLOAT8), sizeof(int8_t));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_FLOAT8_E5M2), sizeof(int8_t));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_FLOAT8_E4M3FN), sizeof(int8_t));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_FLOAT8_E8M0), sizeof(int8_t));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_FLOAT6_E3M2), sizeof(int8_t));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_FLOAT6_E2M3), sizeof(int8_t));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_FLOAT4_E2M1), sizeof(int8_t));
    EXPECT_EQ(GetTensorElementSize(TensorDType::TENSOR_DTYPE_FLOAT4_E1M2), sizeof(int8_t));
}

TEST(GetTensorElementSizeTest, HandleInvalidInput) {
    TensorDType invalid_value = static_cast<TensorDType>(999);
    EXPECT_EQ(GetTensorElementSize(invalid_value), 0);
}

TEST(GetDTypeWithStrTest, HandleValidInput) {
    EXPECT_EQ(GetDTypeWithStr("float"), TensorDType::TENSOR_DTYPE_FLOAT);
    EXPECT_EQ(GetDTypeWithStr("float16"), TensorDType::TENSOR_DTYPE_FLOAT16);
    EXPECT_EQ(GetDTypeWithStr("int8"), TensorDType::TENSOR_DTYPE_INT8);
    EXPECT_EQ(GetDTypeWithStr("int32"), TensorDType::TENSOR_DTYPE_INT32);
    EXPECT_EQ(GetDTypeWithStr("uint8"), TensorDType::TENSOR_DTYPE_UINT8);
    EXPECT_EQ(GetDTypeWithStr("int16"), TensorDType::TENSOR_DTYPE_INT16);
    EXPECT_EQ(GetDTypeWithStr("uint16"), TensorDType::TENSOR_DTYPE_UINT16);
    EXPECT_EQ(GetDTypeWithStr("uint32"), TensorDType::TENSOR_DTYPE_UINT32);
    EXPECT_EQ(GetDTypeWithStr("int64"), TensorDType::TENSOR_DTYPE_INT64);
    EXPECT_EQ(GetDTypeWithStr("uint64"), TensorDType::TENSOR_DTYPE_UINT64);
    EXPECT_EQ(GetDTypeWithStr("double"), TensorDType::TENSOR_DTYPE_DOUBLE);
    EXPECT_EQ(GetDTypeWithStr("bool"), TensorDType::TENSOR_DTYPE_BOOL);
    EXPECT_EQ(GetDTypeWithStr("string"), TensorDType::TENSOR_DTYPE_STRING);
    EXPECT_EQ(GetDTypeWithStr("int4"), TensorDType::TENSOR_DTYPE_INT4);
    EXPECT_EQ(GetDTypeWithStr("uint1"), TensorDType::TENSOR_DTYPE_UINT1);
    EXPECT_EQ(GetDTypeWithStr("complex32"), TensorDType::TENSOR_DTYPE_COMPLEX32);
    EXPECT_EQ(GetDTypeWithStr("hifloat8"), TensorDType::TENSOR_DTYPE_HIFLOAT8);
    EXPECT_EQ(GetDTypeWithStr("float8_e5m2"), TensorDType::TENSOR_DTYPE_FLOAT8_E5M2);
    EXPECT_EQ(GetDTypeWithStr("float8_e4m3fn"), TensorDType::TENSOR_DTYPE_FLOAT8_E4M3FN);
    EXPECT_EQ(GetDTypeWithStr("float8_e8m0"), TensorDType::TENSOR_DTYPE_FLOAT8_E8M0);
    EXPECT_EQ(GetDTypeWithStr("float6_e3m2"), TensorDType::TENSOR_DTYPE_FLOAT6_E3M2);
    EXPECT_EQ(GetDTypeWithStr("float6_e2m3"), TensorDType::TENSOR_DTYPE_FLOAT6_E2M3);
    EXPECT_EQ(GetDTypeWithStr("float4_e2m1"), TensorDType::TENSOR_DTYPE_FLOAT4_E2M1);
    EXPECT_EQ(GetDTypeWithStr("float4_e1m2"), TensorDType::TENSOR_DTYPE_FLOAT4_E1M2);
}

TEST(GetDTypeWithStrTest, HandleInvalidInput) {
    std::string invalid_value = "999";
    EXPECT_EQ(GetDTypeWithStr(invalid_value), TensorDType::TENSOR_DTYPE_UNDEFINED);
}

TEST(GetStrWithDTypeTest, HandleValidInput) {
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_FLOAT), "float");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_FLOAT16), "float16");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_INT8), "int8");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_INT32), "int32");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_UINT8), "uint8");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_INT16), "int16");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_UINT16), "uint16");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_UINT32), "uint32");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_INT64), "int64");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_UINT64), "uint64");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_DOUBLE), "double");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_BOOL), "bool");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_STRING), "string");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_COMPLEX64), "complex64");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_COMPLEX128), "complex128");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_BF16), "bf16");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_INT4), "int4");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_UINT1), "uint1");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_COMPLEX32), "complex32");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_HIFLOAT8), "hifloat8");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_FLOAT8_E5M2), "float8_e5m2");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_FLOAT8_E4M3FN), "float8_e4m3fn");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_FLOAT8_E8M0), "float8_e8m0");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_FLOAT6_E3M2), "float6_e3m2");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_FLOAT6_E2M3), "float6_e2m3");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_FLOAT4_E2M1), "float4_e2m1");
    EXPECT_EQ(GetStrWithDType(TensorDType::TENSOR_DTYPE_FLOAT4_E1M2), "float4_e1m2");
}

TEST(GetStrWithDTypeTest, HandleInvalidInput) {
    int invalidValue = 999;
    EXPECT_EQ(GetStrWithDType(invalidValue), "undefined");
}

TEST(GetFormatWithStrTest, HandleValidInput) {
    EXPECT_EQ(GetFormatWithStr("nchw"), TensorFormat::TENSOR_FORMAT_NCHW);
    EXPECT_EQ(GetFormatWithStr("nhwc"), TensorFormat::TENSOR_FORMAT_NHWC);
    EXPECT_EQ(GetFormatWithStr("nd"), TensorFormat::TENSOR_FORMAT_ND);
    EXPECT_EQ(GetFormatWithStr("nc1hwc0"), TensorFormat::TENSOR_FORMAT_NC1HWC0);
    EXPECT_EQ(GetFormatWithStr("fractal_z"), TensorFormat::TENSOR_FORMAT_FRACTAL_Z);
    EXPECT_EQ(GetFormatWithStr("nc1hwc0_c04"), TensorFormat::TENSOR_FORMAT_NC1HWC0_C04);
    EXPECT_EQ(GetFormatWithStr("hwcn"), TensorFormat::TENSOR_FORMAT_HWCN);
    EXPECT_EQ(GetFormatWithStr("ndhwc"), TensorFormat::TENSOR_FORMAT_NDHWC);
    EXPECT_EQ(GetFormatWithStr("fractal_nz"), TensorFormat::TENSOR_FORMAT_FRACTAL_NZ);
    EXPECT_EQ(GetFormatWithStr("ncdhw"), TensorFormat::TENSOR_FORMAT_NCDHW);
    EXPECT_EQ(GetFormatWithStr("ndc1hwc0"), TensorFormat::TENSOR_FORMAT_NDC1HWC0);
    EXPECT_EQ(GetFormatWithStr("fractal_z_3d"), TensorFormat::TENSOR_FORMAT_FRACTAL_Z_3D);
    EXPECT_EQ(GetFormatWithStr("nc"), TensorFormat::TENSOR_FORMAT_NC);
    EXPECT_EQ(GetFormatWithStr("ncl"), TensorFormat::TENSOR_FORMAT_NCL);
    EXPECT_EQ(GetFormatWithStr("fractal_nz_c0_16"), TensorFormat::TENSOR_FORMAT_FRACTAL_NZ_C0_16);
    EXPECT_EQ(GetFormatWithStr("fractal_nz_c0_32"), TensorFormat::TENSOR_FORMAT_FRACTAL_NZ_C0_32);
}

TEST(GetFormatWithStrTest, HandleInvalidInput) {
    std::string invalid_value = "999";
    EXPECT_EQ(GetFormatWithStr(invalid_value), TensorFormat::TENSOR_FORMAT_UNDEFINED);
}

TEST(GetStrWithFormatTest, HandleValidInput) {
    EXPECT_EQ(GetStrWithFormat(TensorFormat::TENSOR_FORMAT_NCHW), "nchw");
    EXPECT_EQ(GetStrWithFormat(TensorFormat::TENSOR_FORMAT_NHWC), "nhwc");
    EXPECT_EQ(GetStrWithFormat(TensorFormat::TENSOR_FORMAT_ND), "nd");
    EXPECT_EQ(GetStrWithFormat(TensorFormat::TENSOR_FORMAT_NC1HWC0), "nc1hwc0");
    EXPECT_EQ(GetStrWithFormat(TensorFormat::TENSOR_FORMAT_FRACTAL_Z), "fractal_z");
    EXPECT_EQ(GetStrWithFormat(TensorFormat::TENSOR_FORMAT_NC1HWC0_C04), "nc1hwc0_c04");
    EXPECT_EQ(GetStrWithFormat(TensorFormat::TENSOR_FORMAT_HWCN), "hwcn");
    EXPECT_EQ(GetStrWithFormat(TensorFormat::TENSOR_FORMAT_NDHWC), "ndhwc");
    EXPECT_EQ(GetStrWithFormat(TensorFormat::TENSOR_FORMAT_FRACTAL_NZ), "fractal_nz");
    EXPECT_EQ(GetStrWithFormat(TensorFormat::TENSOR_FORMAT_NCDHW), "ncdhw");
    EXPECT_EQ(GetStrWithFormat(TensorFormat::TENSOR_FORMAT_NDC1HWC0), "ndc1hwc0");
    EXPECT_EQ(GetStrWithFormat(TensorFormat::TENSOR_FORMAT_FRACTAL_Z_3D), "fractal_z_3d");
    EXPECT_EQ(GetStrWithFormat(TensorFormat::TENSOR_FORMAT_NC), "nc");
    EXPECT_EQ(GetStrWithFormat(TensorFormat::TENSOR_FORMAT_NCL), "ncl");
    EXPECT_EQ(GetStrWithFormat(TensorFormat::TENSOR_FORMAT_FRACTAL_NZ_C0_16), "fractal_nz_c0_16");
    EXPECT_EQ(GetStrWithFormat(TensorFormat::TENSOR_FORMAT_FRACTAL_NZ_C0_32), "fractal_nz_c0_32");
}

TEST(GetStrWithFormatTest, HandleInvalidInput) {
    int invalidValue = 999;
    EXPECT_EQ(GetStrWithFormat(invalidValue), "undefined");
}
} // namespace
