/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/utils/operationir/operation_ir.h"
#include <string>
#include "mki/utils/log/log.h"
#include "mki/utils/assert/assert.h"

namespace Mki {
static const int MAX_TENSOR_INDEX = 48;
static const std::string INPUT = "input";
static const std::string OUTPUT = "output";
static const std::string TENSOR_KEY_NAME = "name";
static const std::string TENSOR_KEY_D_TYPE = "dtype";
static const std::string TENSOR_KEY_FORMAT = "format";
static const std::string TENSOR_KEY_OPTIONAL = "optional";

std::string TensorInfoIr::ToString() const
{
    std::stringstream ss;
    ss << "{supportedDtypes=[";
    for (size_t i = 0; i < supportedDtypes.size(); ++i) {
        if (i > 0) {
            ss << ", ";
        }
        ss << GetStrWithDType(supportedDtypes[i]);
    }
    ss << "], supportedFormats=[";
    for (size_t i = 0; i < supportedFormats.size(); ++i) {
        if (i > 0) {
            ss << ", ";
        }
        ss << GetStrWithFormat(supportedFormats[i]);
    }
    ss << "], isOptional=" << isOptional;
    ss << "}";
    return ss.str();
}

OperationIr::OperationIr() {}

OperationIr::~OperationIr() {}

std::string OperationIr::ToString()
{
    std::stringstream ss;
    for (size_t i = 0; i < inTensorInfoIrs_.size(); ++i) {
        ss << "\ninTensorInfoIrs[" << i << "]:" << inTensorInfoIrs_[i].ToString() << std::endl;
    }
    for (size_t i = 0; i < outTensorInfoIrs_.size(); ++i) {
        ss << "outTensorInfoIrs[" << i << "]:" << outTensorInfoIrs_[i].ToString() << std::endl;
    }
    return ss.str();
}

static std::string GetCombStringByIndex(const std::string prefix,
    const Mki::SVector<Mki::TensorInfoIr> &tensorInfoIrs, const size_t supportIdx)
{
    std::stringstream ss;
    for (size_t tensorIdx = 0; tensorIdx < tensorInfoIrs.size(); tensorIdx++) {
        if (tensorIdx > 0) {
            ss << ", ";
        }
        ss << prefix << tensorIdx << "(" << GetStrWithDType(tensorInfoIrs[tensorIdx].supportedDtypes[supportIdx]) <<
            "," << GetStrWithFormat(tensorInfoIrs[tensorIdx].supportedFormats[supportIdx]);
        if (tensorInfoIrs[tensorIdx].isOptional) {
            ss << ", isOptional";
        }
        ss << ")";
    }
    return ss.str();
}

std::string OperationIr::GetCombString() const
{
    MKI_CHECK(isValid_, "OperationIr is invalid", return "");
    std::stringstream ss;
    for (size_t supportIdx = 0; supportIdx < supportSize_; supportIdx++) {
        if (supportIdx > 0) {
            ss << "\n";
        }
        ss << "[comb" << supportIdx << "]:" << GetCombStringByIndex("inTensor", inTensorInfoIrs_, supportIdx) << "," <<
            GetCombStringByIndex("outTensor", outTensorInfoIrs_, supportIdx);
    }
    return ss.str();
}

bool OperationIr::Parse(const std::map<std::string, std::string> &contentMap)
{
    std::map<std::string, std::string>::const_iterator it;
    for (it = contentMap.begin(); it != contentMap.end(); it++) {
        MKI_CHECK(ParseKeyValue(it->first, it->second), "ParseKeyValue failed.", return false);
    }
    InitIsValid();
    return true;
}

void OperationIr::InitIsValid()
{
    isValid_ = false;
    supportSize_ = 0;
    if (inTensorInfoIrs_.size() > 0) {
        supportSize_ = inTensorInfoIrs_[0].supportedDtypes.size();
    } else if (outTensorInfoIrs_.size() > 0) {
        supportSize_ = outTensorInfoIrs_[0].supportedDtypes.size();
    }
    MKI_CHECK(supportSize_ != 0, "supportSize_ is 0.", return);
    for (size_t i = 0; i < inTensorInfoIrs_.size(); ++i) {
        MKI_CHECK(inTensorInfoIrs_[i].supportedDtypes.size() == supportSize_ &&
            inTensorInfoIrs_[i].supportedFormats.size() == supportSize_,
            "inTensorInfoIrs_ " << i << " is invalid", return);
    }
    for (size_t i = 0; i < outTensorInfoIrs_.size(); ++i) {
        MKI_CHECK(outTensorInfoIrs_[i].supportedDtypes.size() == supportSize_ &&
            outTensorInfoIrs_[i].supportedFormats.size() == supportSize_,
            "outTensorInfoIrs_ " << i << " is invalid", return);
    }
    isValid_ = true;
}

void OperationIr::ConvertStrToInt(const std::string &str, int &num) const
{
    try {
        num = std::stoi(str);
    } catch (const std::invalid_argument &e) {
        num = -1;
    } catch (const std::out_of_range &e) {
        num = -1;
    }
}

bool OperationIr::ParseKeyValue(const std::string &key, const std::string &value)
{
    size_t dotPos = key.find('.');
    MKI_CHECK(dotPos != std::string::npos, "Parse failed, Exclude '.' ", return false);
    std::string tensorId = key.substr(0, dotPos);
    int index = -1;
    bool isInTensor = false;
    if (tensorId.substr(0, INPUT.size()).compare(INPUT) == 0) {
        isInTensor = true;
        ConvertStrToInt(tensorId.substr(INPUT.size()), index);
    } else if (tensorId.substr(0, OUTPUT.size()).compare(OUTPUT) == 0) {
        ConvertStrToInt(tensorId.substr(OUTPUT.size()), index);
    } else {
        return false;
    }
    MKI_CHECK(index >= 0 && index < MAX_TENSOR_INDEX, "invalid index " << tensorId << ", should >=0 & <48.",
        return false);
    std::string tensorKey = key.substr(dotPos + 1);
    if (isInTensor) {
        return ParseValueToInfoIrs(tensorKey, inTensorInfoIrs_, static_cast<size_t>(index), value);
    }
    return ParseValueToInfoIrs(tensorKey, outTensorInfoIrs_, static_cast<size_t>(index), value);
}

bool OperationIr::ParseValueToInfoIrs(const std::string &tensorKey, SVector<TensorInfoIr> &tensorInfoIrs,
    const size_t &tensorIndex, const std::string &value) const
{
    if (tensorKey == TENSOR_KEY_D_TYPE) {
        return SetSupportedDtypes(tensorInfoIrs, tensorIndex, GetParsedDTypes(value));
    } else if (tensorKey == TENSOR_KEY_FORMAT) {
        return SetSupportedFormats(tensorInfoIrs, tensorIndex, GetParseFormats(value));
    } else if (tensorKey == TENSOR_KEY_OPTIONAL) {
        return SetIsOptional(tensorInfoIrs, tensorIndex, value == "true");
    } else if ((tensorKey == TENSOR_KEY_NAME)) {
        return true;
    }
    MKI_LOG(ERROR) << "Not support tensorKey: " << tensorKey;
    return false;
}

const SVector<TensorInfoIr> &OperationIr::GetInTensorInfoIrs() const
{
    return inTensorInfoIrs_;
}

const SVector<TensorInfoIr> &OperationIr::GetOutTensorInfoIrs() const
{
    return outTensorInfoIrs_;
}

bool OperationIr::IsValid() const
{
    return isValid_;
}

size_t OperationIr::GetSupportSize() const
{
    return supportSize_;
}

std::vector<TensorDType> OperationIr::GetParsedDTypes(const std::string &dTypeStrLine) const
{
    std::stringstream ss(dTypeStrLine);
    std::string dTypeStr;
    std::vector<TensorDType> supportedDtypes;
    TensorDType dType;
    while (getline(ss, dTypeStr, ',')) {
        dType = GetDTypeWithStr(dTypeStr);
        MKI_CHECK(dType != TENSOR_DTYPE_UNDEFINED, "GetDTypeWithStr failed: " << dTypeStr, return {});
        supportedDtypes.push_back(dType);
    }
    return supportedDtypes;
}

std::vector<TensorFormat> OperationIr::GetParseFormats(const std::string &formatStrLine) const
{
    std::stringstream ss(formatStrLine);
    std::string formatStr;
    TensorFormat format;
    std::vector<TensorFormat> supportedFormats;
    while (getline(ss, formatStr, ',')) {
        format = GetFormatWithStr(formatStr);
        MKI_CHECK(format != TENSOR_FORMAT_UNDEFINED, "GetFormatWithStr failed: " << formatStr, return {});
        supportedFormats.push_back(format);
    }
    return supportedFormats;
}


bool OperationIr::SetSupportedDtypes(SVector<TensorInfoIr> &tensorInfoIrs, const size_t &index,
    const std::vector<TensorDType> &supportedDtypes) const
{
    MKI_CHECK(supportedDtypes.size() > 0, "supportedDtypes invalid.", return false);

    if (index >= tensorInfoIrs.size()) {
        tensorInfoIrs.resize(index + 1);
    }
    tensorInfoIrs[index].supportedDtypes = supportedDtypes;
    return true;
}

bool OperationIr::SetSupportedFormats(SVector<TensorInfoIr> &tensorInfoIrs, const size_t &index,
    const std::vector<TensorFormat> &supportedFormats) const
{
    MKI_CHECK(supportedFormats.size() > 0, "supportedFormats invalid.", return false);

    if (index >= tensorInfoIrs.size()) {
        tensorInfoIrs.resize(index + 1);
    }
    tensorInfoIrs[index].supportedFormats = supportedFormats;
    return true;
}

bool OperationIr::SetIsOptional(SVector<TensorInfoIr> &tensorInfoIrs, const size_t &index,
                                const bool &isOptional) const
{
    if (index >= tensorInfoIrs.size()) {
        tensorInfoIrs.resize(index + 1);
    }
    tensorInfoIrs[index].isOptional = isOptional;
    return true;
}
}
