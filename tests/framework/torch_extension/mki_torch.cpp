/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki_torch.h"
#include <ATen/ATen.h>
#include <torch/extension.h>
#include <torch_npu/csrc/core/npu/NPUStream.h>
#include <torch_npu/csrc/core/npu/NPUFormat.h>
#include "mki/kernel.h"
#include "mki/utils/assert/assert.h"
#include "mki/utils/log/log.h"
#include "mki/utils/status/status.h"
#include "mki/utils/rt/rt.h"
#include "op_desc_json.h"

static std::map<at::ScalarType, Mki::TensorDType> DTYPE_MAP = {
    {at::ScalarType::Byte, Mki::TENSOR_DTYPE_UINT8},   {at::ScalarType::Char, Mki::TENSOR_DTYPE_INT8},
    {at::ScalarType::Half, Mki::TENSOR_DTYPE_FLOAT16}, {at::ScalarType::Float, Mki::TENSOR_DTYPE_FLOAT},
    {at::ScalarType::Int, Mki::TENSOR_DTYPE_INT32},    {at::ScalarType::Long, Mki::TENSOR_DTYPE_INT64},
    {at::ScalarType::BFloat16, Mki::TENSOR_DTYPE_BF16},
};
namespace {
TORCH_LIBRARY(MkiTorch, m)
{
    m.class_<Mki::Test::MkiTorch>("MkiTorch")
        .def(torch::init<std::string>())
        .def("execute", &Mki::Test::MkiTorch::Execute)
        .def("execute_nct", &Mki::Test::MkiTorch::ExecuteNCT);
}
}

namespace Mki {
namespace Test {
void *MkiTorch::GetCurrentStream() const
{
    int32_t devId = 0;
    int st = Mki::MkiRtDeviceGetCurrent(&devId);
    MKI_CHECK(st == MKIRT_SUCCESS, "failed to get current device", return nullptr);
    void *stream = c10_npu::getCurrentNPUStream(devId).stream();
    return stream;
}

void MkiTorch::ContiguousAtTensor(std::vector<at::Tensor> &atTensors)
{
    for (size_t i = 0; i < atTensors.size(); i++) {
        if (!atTensors.at(i).is_contiguous()) {
            atTensors.at(i) = atTensors.at(i).contiguous();
        }
    }
}

void MkiTorch::SetUpTensors(const nlohmann::json &opDescJson, Mki::LaunchParam &launchParam,
                            std::vector<at::Tensor> &atInTensors, std::vector<at::Tensor> &atOutTensors)
{
    std::vector<Mki::Tensor> mkiInTensors;
    std::vector<Mki::Tensor> mkiOutTensors;
    if (opDescJson.find("input_formats") !=  opDescJson.end()) {
        for (uint32_t i = 0; i < atInTensors.size(); i++) {
            mkiInTensors.push_back(AtTensor2MkiTensor(atInTensors[i]));
            mkiInTensors[i].desc.format = static_cast<Mki::TensorFormat>(opDescJson["input_formats"][i]);
        }
    } else {
        MKI_LOG(INFO) << "use default format";
        for (at::Tensor &atTensor : atInTensors) {
            mkiInTensors.push_back(AtTensor2MkiTensor(atTensor));
        }
    }
    if (opDescJson.find("output_formats") !=  opDescJson.end()) {
        for (uint32_t i = 0; i < atOutTensors.size(); i++) {
            mkiOutTensors.push_back(AtTensor2MkiTensor(atOutTensors[i]));
            mkiOutTensors[i].desc.format = static_cast<Mki::TensorFormat>(opDescJson["output_formats"][i]);
        }
    } else {
        MKI_LOG(INFO) << "use default format";
        for (at::Tensor &atTensor : atOutTensors) {
            mkiOutTensors.push_back(AtTensor2MkiTensor(atTensor));
        }
    }

    for (auto iter : mkiInTensors) {
        launchParam.AddInTensor(iter);
    }
    for (auto iter : mkiOutTensors) {
        launchParam.AddOutTensor(iter);
    }
}

std::string MkiTorch::SaveTensorsToBuf(Mki::SVector<Mki::Tensor> &mkiTensors)
{
    int st;
    int inTensorIdx = 0;
    for (auto iter : mkiTensors) {
        tensorTempBufList_[inTensorIdx] = (uint8_t *)malloc(iter.dataSize);
        st = Mki::MkiRtMemCopy((void *)tensorTempBufList_[inTensorIdx], iter.dataSize, iter.data, iter.dataSize,
                               MKIRT_MEMCOPY_DEVICE_TO_HOST);
        if (st != MKIRT_SUCCESS) {
            MKI_LOG(ERROR) << "MkiRtMemCopy error";
            return "MkiRtMemCopy error";
        }
        inTensorIdx++;
    }
    return "ok";
}

std::string MkiTorch::GetTensorsFromBuf(Mki::SVector<Mki::Tensor> &mkiTensors)
{
    int st;
    int inTensorIdx = 0;
    for (auto iter : mkiTensors) {
        st = Mki::MkiRtMemCopy(iter.data, iter.dataSize, tensorTempBufList_[inTensorIdx], iter.dataSize,
                                MKIRT_MEMCOPY_HOST_TO_DEVICE);
        if (st != MKIRT_SUCCESS) {
            MKI_LOG(ERROR) << "MkiRtMemCopy error";
            return "MkiRtMemCopy error";
        }
        inTensorIdx++;
    }
    return "ok";
}

Mki::Kernel *MkiTorch::GetKernelInstance(Mki::LaunchParam &launchParam, const std::string &kernelName)
{
    Mki::Operation *op = Mki::AutoGen::GetOpByName(opName_);
    if (op == nullptr) {
        MKI_LOG(ERROR) << "get operation by name fail, opName:" << opName_;
        return nullptr;
    }

    MKI_LOG(INFO) << "before infershape, launchParam:\n" << launchParam.ToString();
    Mki::Status status = op->InferShape(launchParam);
    if (!status.Ok()) {
        MKI_LOG(ERROR) << opName_ << " infer shape fail, error:" << status.ToString();
        return nullptr;
    }
    MKI_LOG(INFO) << "after infershape, launchParam:\n" << launchParam.ToString();

    Mki::Kernel *kernel = nullptr;
    if (kernelName != "") {
        kernel = op->GetKernelByName(kernelName);
        MKI_CHECK(kernel != nullptr, opName_ << " get kernel by name " << kernelName << " fail", return nullptr);
    } else {
        kernel = op->GetBestKernel(launchParam);
        MKI_CHECK(kernel != nullptr, opName_ << " get best kernel fail", return nullptr);
    }

    return kernel;
}

std::string MkiTorch::AddWorkspace(const Mki::KernelInfo &kernelInfo, Mki::RunInfo &runInfo)
{
    size_t bufferSize = kernelInfo.GetTotalScratchSize();
    if (bufferSize == 0) {
        MKI_LOG(INFO) << "no workspace";
        return "ok";
    }
    MKI_LOG(INFO) << "Workspace size: " << bufferSize;
    uint8_t *deviceBuffer = nullptr;
    int ret = Mki::MkiRtMemMallocDevice(reinterpret_cast<void **>(&deviceBuffer), bufferSize, MKIRT_MEM_DEFAULT);
    if (ret != MKIRT_SUCCESS) {
        MKI_LOG(ERROR) << "MkiRtMemMallocDevice fail, errCode:" << ret << ", errName:" << Mki::MkiRtErrorName(ret)
                       << "errDesc:" << Mki::MkiRtErrorDesc(ret);
        return "error:MkiRtMemMallocDevice fail";
    }
    runInfo.SetScratchDeviceAddr(deviceBuffer);
    return "ok";
}

std::string MkiTorch::FreeWorkspace(const Mki::KernelInfo &kernelInfo, Mki::RunInfo &runInfo)
{
    uint8_t *deviceBuffer = runInfo.GetScratchDeviceAddr();
    size_t bufferSize = kernelInfo.GetTotalScratchSize();
    if (bufferSize == 0) {
        return "ok";
    }
    if (deviceBuffer != nullptr) {
        Mki::MkiRtMemFreeDevice(deviceBuffer);
    }
    return "ok";
}

std::string MkiTorch::InitTilingAtMkiTorch(std::shared_ptr<Mki::Kernel> kernel, const Mki::LaunchParam &launchParam,
                                           Mki::RunInfo &runInfo)
{
    kernel->SetLaunchWithTiling(false);
    uint32_t launchBufferSize = kernel->GetTilingSize(launchParam);
    MKI_CHECK(launchBufferSize > 0, "empty tiling size", return "empty tiling size");

    uint8_t hostLaunchBuffer[launchBufferSize];
    kernel->SetTilingHostAddr(hostLaunchBuffer, launchBufferSize);
    auto status = kernel->Init(launchParam);
    MKI_CHECK(status.Ok(), "failed to init op", return "failed to init op");

    int st = Mki::MkiRtMemMallocDevice(reinterpret_cast<void **>(&deviceLaunchBuffer_),
                                       launchBufferSize, MKIRT_MEM_DEFAULT);
    MKI_CHECK(st == MKIRT_SUCCESS, "MkiRtMemMallocDevice error", return "MkiRtMemMallocDevice error");

    st = Mki::MkiRtMemCopy(deviceLaunchBuffer_, launchBufferSize,
                           hostLaunchBuffer, launchBufferSize, MKIRT_MEMCOPY_HOST_TO_DEVICE);
    if (st != MKIRT_SUCCESS) {
        Mki::MkiRtMemFreeDevice(deviceLaunchBuffer_);
        deviceLaunchBuffer_ = nullptr;
        MKI_LOG(ERROR) << "MkiRtMemCopy error";
        return "MkiRtMemCopy error";
    }
    runInfo.SetTilingDeviceAddr(deviceLaunchBuffer_);
    return "ok";
}

std::string MkiTorch::RunOp(Mki::LaunchParam &launchParam)
{
    std::string retStr;
    Mki::Status status;

    std::shared_ptr<Mki::Kernel> kernel(GetKernelInstance(launchParam));
    MKI_CHECK(kernel != nullptr, "failed to get kernel instance", return "failed to init op");

    Mki::RunInfo runInfo;
    MkiRtStream stream = GetCurrentStream();
    MKI_LOG(INFO) << "stream:" << stream;
    runInfo.SetStream(stream);

    if (launchWithTiling_) {
        kernel->SetLaunchWithTiling(true);
        status = kernel->Init(launchParam);
        MKI_CHECK(status.Ok(), "failed to init op", return "failed to init op");
    } else {
        InitTilingAtMkiTorch(kernel, launchParam, runInfo);
        MKI_CHECK(retStr == "ok", "failed to init tiling in mkiTorch", return retStr);
    }

    const Mki::KernelInfo &kernelInfo = kernel->GetKernelInfo();
    retStr = AddWorkspace(kernelInfo, runInfo);
    MKI_CHECK(retStr == "ok", "failed to add workspace", return retStr);

    MKI_LOG(INFO) << kernel->GetName() << " run start, LaunchParam:\n" << launchParam.ToString();
    MKI_LOG(INFO) << "RunInfo:\n" << runInfo.ToString();

    status = kernel->Run(launchParam, runInfo);
    MKI_LOG_IF(!status.Ok(), ERROR) << kernel->GetName() << " run fail, error:" << status.ToString();
    if (!status.Ok()) {
        return "kernel run fail";
    }

    int ret = Mki::MkiRtStreamSynchronize(runInfo.GetStream());
    MKI_LOG_IF(ret != 0, ERROR) << "MkiRtStreamSynchronize fail";
    if (ret != 0) {
        return "MkiRtStreamSynchronize fail";
    }

    if (deviceLaunchBuffer_ != nullptr) {
        Mki::MkiRtMemFreeDevice(deviceLaunchBuffer_);
    }

    retStr = FreeWorkspace(kernelInfo, runInfo);
    return retStr;
}

std::string MkiTorch::RunOpPerf(Mki::LaunchParam &launchParam, int runTimes)
{
    std::string retStr;
    Mki::Status status;

    retStr = SaveTensorsToBuf(launchParam.GetInTensors());
    MKI_CHECK(retStr == "ok", "failed to save tensors to buffer", return retStr);

    std::shared_ptr<Mki::Kernel> kernel(GetKernelInstance(launchParam, kernelName_));
    MKI_CHECK(kernel != nullptr, "failed to get kernel instance", return "failed to init op");

    Mki::RunInfo runInfo;
    MkiRtStream stream = GetCurrentStream();
    MKI_LOG(INFO) << "stream:" << stream;
    runInfo.SetStream(stream);

    if (launchWithTiling_) {
        kernel->SetLaunchWithTiling(true);
        status = kernel->Init(launchParam);
        MKI_CHECK(status.Ok(), "failed to run tiling", return "failed to run tiling");
    } else {
        InitTilingAtMkiTorch(kernel, launchParam, runInfo);
        MKI_CHECK(retStr == "ok", "failed to init tiling in mkiTorch", return retStr);
    }

    const Mki::KernelInfo &kernelInfo = kernel->GetKernelInfo();
    retStr = AddWorkspace(kernelInfo, runInfo);
    MKI_CHECK(retStr == "ok", "failed to add workspace", return retStr);

    MKI_LOG(INFO) << kernel->GetName() << " run start, runInfo:\n" << runInfo.ToString();

    for (int runIdx = 0; runIdx < runTimes; runIdx++) {
        retStr = GetTensorsFromBuf(launchParam.GetInTensors());
        MKI_CHECK(retStr == "ok", "failed to save tensors to buffer", return retStr);

        status = kernel->Run(launchParam, runInfo);
        MKI_LOG_IF(!status.Ok(), ERROR) << kernel->GetName() << " run fail, error:" << status.ToString();
        if (!status.Ok()) {
            if (deviceLaunchBuffer_ != nullptr) {
                Mki::MkiRtMemFreeDevice(deviceLaunchBuffer_);
            }
            retStr = FreeWorkspace(kernelInfo, runInfo);
            return "kernel run fail";
        }
    }

    int ret = Mki::MkiRtStreamSynchronize(runInfo.GetStream());
    MKI_LOG_IF(ret != 0, ERROR) << "MkiRtStreamSynchronize fail";
    if (ret != 0) {
        return "MkiRtStreamSynchronize fail";
    }

    if (deviceLaunchBuffer_ != nullptr) {
        Mki::MkiRtMemFreeDevice(deviceLaunchBuffer_);
    }

    retStr = FreeWorkspace(kernelInfo, runInfo);
    return retStr;
}

Mki::Tensor MkiTorch::AtTensor2MkiTensor(const at::Tensor &atTensor)
{
    Mki::Tensor mkiTensor;
    mkiTensor.data = atTensor.data_ptr();
    mkiTensor.dataSize = atTensor.numel() * atTensor.element_size();

    mkiTensor.desc.dims.resize(atTensor.sizes().size());
    for (uint64_t i = 0; i < atTensor.sizes().size(); i++) {
        mkiTensor.desc.dims[i] = atTensor.sizes()[i];
    }

    mkiTensor.desc.format = static_cast<Mki::TensorFormat>(at_npu::native::get_npu_format(atTensor));

    mkiTensor.desc.strides.resize(atTensor.strides().size());
    for (uint64_t i = 0; i < atTensor.strides().size(); i++) {
        mkiTensor.desc.strides[i] = atTensor.stride(i);
    }
 
    mkiTensor.desc.offset = atTensor.storage_offset();

    MKI_LOG(INFO) << "At tensor dtype " << atTensor.scalar_type();

    auto it = DTYPE_MAP.find(atTensor.scalar_type());
    if (it != DTYPE_MAP.end()) {
        mkiTensor.desc.dtype = it->second;
    } else {
        MKI_LOG(ERROR) << "not support dtype:" << atTensor.scalar_type();
    }

    return mkiTensor;
}

std::string MkiTorch::ExecuteImpl(std::vector<at::Tensor> &atInTensors, std::vector<at::Tensor> &atOutTensors)
{
    MKI_LOG(INFO) << "Execute start, json:" << opDescJsonStr_ << ", compile time:" << __DATE__ << ":" << __TIME__;
    nlohmann::json opDescJson;
    std::string retStr;
    try {
        opDescJson = nlohmann::json::parse(opDescJsonStr_);
    } catch (nlohmann::json::parse_error &ex) {
        MKI_LOG(ERROR) << "json parse error, CallOp fail";
        return "error:json parse error, CallOp fail";
    }
    Mki::LaunchParam launchParam;
    int runTimes;
    int perfFlag = 0;

    opName_ = opDescJson["opName"];
    if (opDescJson.contains("kernelName")) {
        kernelName_ = opDescJson["kernelName"];
    }
    if (opDescJson.find("runTimes") != opDescJson.end()) {
        perfFlag = 1;
        runTimes = opDescJson["runTimes"];
    }
    Mki::AutoGen::JsonToOpParam(opDescJson, launchParam);

    SetUpTensors(opDescJson, launchParam, atInTensors, atOutTensors);

    retStr = perfFlag ? RunOpPerf(launchParam, runTimes) : RunOp(launchParam);

    return retStr;
}

std::string MkiTorch::Execute(std::vector<at::Tensor> atInTensors, std::vector<at::Tensor> atOutTensors)
{
    ContiguousAtTensor(atInTensors);
    ContiguousAtTensor(atOutTensors);
    return ExecuteImpl(atInTensors, atOutTensors);
}

std::string MkiTorch::ExecuteNCT(std::vector<at::Tensor> atInTensors, std::vector<at::Tensor> atOutTensors)
{
    return ExecuteImpl(atInTensors, atOutTensors);
}
} // namespace Test
} // namespace Mki
