/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/types.h"
#include <map>
#include "mki/utils/log/log.h"

namespace Mki {
constexpr size_t HALF_DATA_SIZE = 2;
constexpr size_t BITWIDTH128_DATA_SIZE = 16;
static const std::string UNDEFINED_STR = "undefined";

static const std::map<TensorDType, size_t> MAP_OF_DTYPE_SIZE = {
    {TensorDType::TENSOR_DTYPE_UNDEFINED, 0},
    {TensorDType::TENSOR_DTYPE_FLOAT, sizeof(float)},
    {TensorDType::TENSOR_DTYPE_FLOAT16, HALF_DATA_SIZE},
    {TensorDType::TENSOR_DTYPE_INT8, sizeof(int8_t)},
    {TensorDType::TENSOR_DTYPE_INT32, sizeof(int32_t)},
    {TensorDType::TENSOR_DTYPE_UINT8, sizeof(uint8_t)},
    {TensorDType::TENSOR_DTYPE_INT16, sizeof(int16_t)},
    {TensorDType::TENSOR_DTYPE_UINT16, sizeof(uint16_t)},
    {TensorDType::TENSOR_DTYPE_UINT32, sizeof(uint32_t)},
    {TensorDType::TENSOR_DTYPE_INT64, sizeof(int64_t)},
    {TensorDType::TENSOR_DTYPE_UINT64, sizeof(uint64_t)},
    {TensorDType::TENSOR_DTYPE_DOUBLE, sizeof(double)},
    {TensorDType::TENSOR_DTYPE_BOOL, sizeof(bool)},
    {TensorDType::TENSOR_DTYPE_BF16, HALF_DATA_SIZE},
    {TensorDType::TENSOR_DTYPE_COMPLEX64, sizeof(double)},
    {TensorDType::TENSOR_DTYPE_COMPLEX128, BITWIDTH128_DATA_SIZE},
    {TensorDType::TENSOR_DTYPE_BF16, HALF_DATA_SIZE},
    {TensorDType::TENSOR_DTYPE_INT4, sizeof(bool)},
    {TensorDType::TENSOR_DTYPE_UINT1, sizeof(bool)},
    {TensorDType::TENSOR_DTYPE_COMPLEX32, sizeof(int32_t)},
    {TensorDType::TENSOR_DTYPE_HIFLOAT8, sizeof(int8_t)},
    {TensorDType::TENSOR_DTYPE_FLOAT8_E5M2, sizeof(int8_t)},
    {TensorDType::TENSOR_DTYPE_FLOAT8_E4M3FN, sizeof(int8_t)},
    {TensorDType::TENSOR_DTYPE_FLOAT8_E8M0, sizeof(int8_t)},
    {TensorDType::TENSOR_DTYPE_FLOAT6_E3M2, sizeof(int8_t)},
    {TensorDType::TENSOR_DTYPE_FLOAT6_E2M3, sizeof(int8_t)},
    {TensorDType::TENSOR_DTYPE_FLOAT4_E2M1, sizeof(int8_t)},
    {TensorDType::TENSOR_DTYPE_FLOAT4_E1M2, sizeof(int8_t)},
};

size_t GetTensorElementSize(const TensorDType dtype)
{
    auto iter = MAP_OF_DTYPE_SIZE.find(dtype);
    if (iter == MAP_OF_DTYPE_SIZE.end()) {
        MKI_LOG(ERROR) << "Get Tensor ElementSize:dtype not found!";
        return 0;
    }
    return iter->second;
}

static const std::map<std::string, TensorDType> MAP_STRING_TO_DTYPE = {
    { "float", TENSOR_DTYPE_FLOAT },
    { "float16", TENSOR_DTYPE_FLOAT16 },
    { "int8", TENSOR_DTYPE_INT8 },
    { "int32", TENSOR_DTYPE_INT32 },
    { "uint8", TENSOR_DTYPE_UINT8 },
    { "int16", TENSOR_DTYPE_INT16 },
    { "uint16", TENSOR_DTYPE_UINT16 },
    { "uint32", TENSOR_DTYPE_UINT32 },
    { "int64", TENSOR_DTYPE_INT64 },
    { "uint64", TENSOR_DTYPE_UINT64 },
    { "double", TENSOR_DTYPE_DOUBLE },
    { "bool", TENSOR_DTYPE_BOOL },
    { "string", TENSOR_DTYPE_STRING },
    { "complex64", TENSOR_DTYPE_COMPLEX64 },
    { "complex128", TENSOR_DTYPE_COMPLEX128 },
    { "bf16", TENSOR_DTYPE_BF16 },
    { "int4", TENSOR_DTYPE_INT4 },
    { "uint1", TENSOR_DTYPE_UINT1 },
    { "complex32", TENSOR_DTYPE_COMPLEX32 },
    { "hifloat8", TENSOR_DTYPE_HIFLOAT8 },
    { "float8_e5m2", TENSOR_DTYPE_FLOAT8_E5M2 },
    { "float8_e4m3fn", TENSOR_DTYPE_FLOAT8_E4M3FN },
    { "float8_e8m0", TENSOR_DTYPE_FLOAT8_E8M0 },
    { "float6_e3m2", TENSOR_DTYPE_FLOAT6_E3M2 },
    { "float6_e2m3", TENSOR_DTYPE_FLOAT6_E2M3 },
    { "float4_e2m1", TENSOR_DTYPE_FLOAT4_E2M1 },
    { "float4_e1m2", TENSOR_DTYPE_FLOAT4_E1M2 },
};

TensorDType GetDTypeWithStr(const std::string &typeStr)
{
    std::map<std::string, TensorDType>::const_iterator it = MAP_STRING_TO_DTYPE.find(typeStr);
    if (it != MAP_STRING_TO_DTYPE.end()) {
        return it->second;
    }
    return TENSOR_DTYPE_UNDEFINED;
}

static const std::map<int, std::string> MAP_DTYPE_TO_STRING = {
    { TENSOR_DTYPE_FLOAT, "float" },
    { TENSOR_DTYPE_FLOAT16, "float16" },
    { TENSOR_DTYPE_INT8, "int8" },
    { TENSOR_DTYPE_INT32, "int32" },
    { TENSOR_DTYPE_UINT8, "uint8" },
    { TENSOR_DTYPE_INT16, "int16" },
    { TENSOR_DTYPE_UINT16, "uint16" },
    { TENSOR_DTYPE_UINT32, "uint32" },
    { TENSOR_DTYPE_INT64, "int64" },
    { TENSOR_DTYPE_UINT64, "uint64" },
    { TENSOR_DTYPE_DOUBLE, "double" },
    { TENSOR_DTYPE_BOOL, "bool" },
    { TENSOR_DTYPE_STRING, "string" },
    { TENSOR_DTYPE_COMPLEX64, "complex64" },
    { TENSOR_DTYPE_COMPLEX128, "complex128" },
    { TENSOR_DTYPE_BF16, "bf16" },
    { TENSOR_DTYPE_INT4, "int4" },
    { TENSOR_DTYPE_UINT1, "uint1" },
    { TENSOR_DTYPE_COMPLEX32, "complex32" },
    { TENSOR_DTYPE_HIFLOAT8, "hifloat8" },
    { TENSOR_DTYPE_FLOAT8_E5M2, "float8_e5m2" },
    { TENSOR_DTYPE_FLOAT8_E4M3FN, "float8_e4m3fn" },
    { TENSOR_DTYPE_FLOAT8_E8M0, "float8_e8m0" },
    { TENSOR_DTYPE_FLOAT6_E3M2, "float6_e3m2" },
    { TENSOR_DTYPE_FLOAT6_E2M3, "float6_e2m3" },
    { TENSOR_DTYPE_FLOAT4_E2M1, "float4_e2m1" },
    { TENSOR_DTYPE_FLOAT4_E1M2, "float4_e1m2" },
};

const std::string &GetStrWithDType(int dType)
{
    auto it = MAP_DTYPE_TO_STRING.find(dType);
    if (it != MAP_DTYPE_TO_STRING.end()) {
        return it->second;
    }
    return UNDEFINED_STR;
}

static const std::map<std::string, TensorFormat> MAP_STRING_TO_FORMAT = {
    { "nchw", TENSOR_FORMAT_NCHW },
    { "nhwc", TENSOR_FORMAT_NHWC },
    { "nd", TENSOR_FORMAT_ND },
    { "nc1hwc0", TENSOR_FORMAT_NC1HWC0 },
    { "fractal_z", TENSOR_FORMAT_FRACTAL_Z },
    { "nc1hwc0_c04", TENSOR_FORMAT_NC1HWC0_C04 },
    { "hwcn", TENSOR_FORMAT_HWCN },
    { "ndhwc", TENSOR_FORMAT_NDHWC },
    { "fractal_nz", TENSOR_FORMAT_FRACTAL_NZ },
    { "ncdhw", TENSOR_FORMAT_NCDHW },
    { "ndc1hwc0", TENSOR_FORMAT_NDC1HWC0 },
    { "fractal_z_3d", TENSOR_FORMAT_FRACTAL_Z_3D },
    { "nc", TENSOR_FORMAT_NC},
    { "ncl", TENSOR_FORMAT_NCL},
    { "fractal_nz_c0_16", TENSOR_FORMAT_FRACTAL_NZ_C0_16},
    { "fractal_nz_c0_32", TENSOR_FORMAT_FRACTAL_NZ_C0_32},
};

TensorFormat GetFormatWithStr(const std::string &formatStr)
{
    std::map<std::string, TensorFormat>::const_iterator it = MAP_STRING_TO_FORMAT.find(formatStr);
    if (it != MAP_STRING_TO_FORMAT.end()) {
        return it->second;
    }
    return TENSOR_FORMAT_UNDEFINED;
}

static const std::map<int, std::string> MAP_FORMAT_TO_STRING = {
    { TENSOR_FORMAT_NCHW, "nchw" },
    { TENSOR_FORMAT_NHWC, "nhwc" },
    { TENSOR_FORMAT_ND, "nd" },
    { TENSOR_FORMAT_NC1HWC0, "nc1hwc0" },
    { TENSOR_FORMAT_FRACTAL_Z, "fractal_z" },
    { TENSOR_FORMAT_NC1HWC0_C04, "nc1hwc0_c04" },
    { TENSOR_FORMAT_HWCN, "hwcn" },
    { TENSOR_FORMAT_NDHWC, "ndhwc" },
    { TENSOR_FORMAT_FRACTAL_NZ, "fractal_nz" },
    { TENSOR_FORMAT_NCDHW, "ncdhw" },
    { TENSOR_FORMAT_NDC1HWC0, "ndc1hwc0" },
    { TENSOR_FORMAT_FRACTAL_Z_3D, "fractal_z_3d" },
    { TENSOR_FORMAT_NC, "nc" },
    { TENSOR_FORMAT_NCL, "ncl" },
    { TENSOR_FORMAT_FRACTAL_NZ_C0_16, "fractal_nz_c0_16" },
    { TENSOR_FORMAT_FRACTAL_NZ_C0_32, "fractal_nz_c0_32" },
};

const std::string &GetStrWithFormat(int format)
{
    auto it = MAP_FORMAT_TO_STRING.find(format);
    if (it != MAP_FORMAT_TO_STRING.end()) {
        return it->second;
    }
    return UNDEFINED_STR;
}
} // namespace Mki
