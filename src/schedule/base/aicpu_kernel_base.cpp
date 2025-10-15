/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/base/aicpu_kernel_base.h"
#include <securec.h>
#include "mki/utils/assert/assert.h"
#include "mki/utils/checktensor/check_tensor.h"
#include "mki/utils/file_system/file_system.h"
#include "mki/utils/log/log.h"
#include "mki/utils/rt/rt.h"
#include "mki/utils/math/tensor_utils.h"

namespace Mki {
class AicpuKernelParamBuilder {
public:
    AicpuKernelParamBuilder(const std::string &soName, const std::string &deviceKernelName)
        : soName_(soName), deviceKernelName_(deviceKernelName) {}

    Status Init(const LaunchParam &launchParam, uint64_t argsNum, const KernelInfo &kernelInfo)
    {
        uint8_t *argsPtr = kernelInfo.GetArgs();
        uint64_t argsSize = kernelInfo.GetArgsSize();
        MKI_CHECK(argsPtr != nullptr, "args size invalid", return Status::FailStatus(-1));
        MKI_CHECK(argsNum * sizeof(void *) <= argsSize, "args size invalid", return Status::FailStatus(-1));
        auto ret = memset_s(argsPtr, argsSize, 0, argsNum * sizeof(void *));
        MKI_CHECK(ret == EOK, "memory set failed", return Status::FailStatus(ERROR_INVALID_VALUE));
        void **args = reinterpret_cast<void **>(static_cast<void *>(argsPtr));

        // set input / output
        auto status = UpdateInOutArgs(args, argsNum, launchParam);
        MKI_CHECK(status.Ok(), "failed to update input output args", return status);

        // Set soName and kernelName
        uint32_t tensorOffset = argsNum * sizeof(void *);
        uint32_t soNameSize = soName_.length();
        uint32_t kernelNameSize = deviceKernelName_.length();
        ret = memcpy_s(argsPtr + tensorOffset, soNameSize, soName_.c_str(), soNameSize);
        MKI_CHECK(ret == EOK, "memcpy for soName fail, errCode:" << ret,
                  return Status::FailStatus(ERROR_INVALID_VALUE));
        ret = memcpy_s(argsPtr + tensorOffset + soNameSize + 1, kernelNameSize, deviceKernelName_.c_str(),
                       kernelNameSize);
        MKI_CHECK(ret == EOK, "memcpy for kernelName fail, errCode:" << ret,
                  return Status::FailStatus(ERROR_INVALID_VALUE));

        // launch
        kernelParam_.blockDim = kernelInfo.GetBlockDim();
        kernelParam_.aicpuArgsEx = &argsEx_;
        argsEx_.args = argsPtr;
        argsEx_.argsSize = argsSize;
        argsEx_.soNameAddrOffset = tensorOffset;
        argsEx_.kernelNameAddrOffset = tensorOffset + soNameSize + 1;
        return status;
    }

    const MkiRtAicpuKernelParam &GetKernelParam() const
    {
        return kernelParam_;
    }

private:
    Status UpdateInOutArgs(void **args, uint64_t argsNum, const LaunchParam &launchParam) const
    {
        size_t inputNum = launchParam.GetInTensorCount();
        uint64_t idx = 0;
        for (size_t i = 0; i < inputNum && idx < argsNum; idx++) {
            if (args[idx] != nullptr) {
                continue;
            }
            MKI_LOG(DEBUG) << "args info: input tensor " << idx;
            args[idx] = launchParam.GetInTensor(i++).data;
        }
        size_t outputNum = launchParam.GetOutTensorCount();
        for (size_t i = 0; i < outputNum && idx < argsNum; idx++) {
            if (args[idx] != nullptr) {
                continue;
            }
            MKI_LOG(DEBUG) << "args info: output tensor " << idx;
            args[idx] = launchParam.GetOutTensor(i++).data;
        }
        return Status::OkStatus();
    }

private:
    const std::string soName_;
    const std::string deviceKernelName_;
    RtAicpuArgsExT argsEx_;
    MkiRtAicpuKernelParam kernelParam_;
};

AicpuKernelBase::AicpuKernelBase(const std::string &opName, const char *deviceKernelName)
    : kernelName_(opName), deviceKernelName_(deviceKernelName)
{
    if (deviceKernelName == nullptr) {
        MKI_LOG(ERROR) << "AicpuKernelBase get null deviceKernelName!";
    }
    MKI_LOG(INFO) << "Create AicpuKernel " << kernelName_ << ", deviceKernelName: " << deviceKernelName_
                   << ", coreType (const): " << kernelType_;
}

AicpuKernelBase::~AicpuKernelBase() {}

std::string AicpuKernelBase::GetName() const { return kernelName_; }

KernelType AicpuKernelBase::GetType() const { return kernelType_; }

const std::string AicpuKernelBase::GetDeviceKernelName() const { return deviceKernelName_; }

const KernelInfo &AicpuKernelBase::GetKernelInfo() const { return kernelInfo_; }

void AicpuKernelBase::SetLaunchWithTiling(bool flag) { kernelInfo_.SetLaunchWithTiling(flag); }

void AicpuKernelBase::SetTilingHostAddr(uint8_t *addr, uint64_t len) { kernelInfo_.SetTilingHostAddr(addr, len); }

void AicpuKernelBase::Reset()
{
    kernelInfo_.Reset();
}

Status AicpuKernelBase::Init(const LaunchParam &launchParam)
{
    MKI_CHECK(CheckInTensors(launchParam), "Not supported in tensors", return Status::FailStatus(1));
    MKI_CHECK(CanSupport(launchParam), "Not supported op", return Status::FailStatus(1));

    kernelInfo_.Reset();

    auto tilingSize = GetTilingSize(launchParam);
    auto status = kernelInfo_.AllocTilingHost(tilingSize);
    MKI_CHECK(status.Ok(), "Failed to alloc host tensor buffer " << status.ToString(), return status);

    status = InitImpl(launchParam);
    MKI_CHECK(status.Ok(), "Failed to init kernel " << status.ToString(), return status);

    auto kernelParamNum = GetKernelArgsNum(launchParam);
    MKI_LOG(INFO) << "args num " << kernelParamNum;

    uint64_t soNameSize = static_cast<uint64_t>(strlen(soName_.c_str())) + 1;
    uint64_t kernelNameSize = static_cast<uint64_t>(strlen(deviceKernelName_.c_str())) + 1;
    uint64_t argsSize = kernelParamNum * sizeof(void *);
    argsSize += soNameSize + kernelNameSize;

    // Args mem mapping
    // [0..num_in_tensors(n)] inTensors
    // [n..num_out_tensors(m)] outTensors
    // [m..so_name_size(o)] soName
    // [o..kernel_name_size(p)] kernelName
    status = kernelInfo_.InitArgs(argsSize);

    MKI_CHECK(status.Ok(), "failed to init args", return status);
    return Status::OkStatus();
}

uint64_t AicpuKernelBase::GetKernelArgsNum(const LaunchParam &launchParam) const
{
    uint64_t inputOutputNum = launchParam.GetInTensorCount() + launchParam.GetOutTensorCount();
    MKI_LOG(DEBUG) << "aicpu kernel param: " << inputOutputNum << " in/out";
    return inputOutputNum;
}

Status AicpuKernelBase::Run(const LaunchParam &launchParam, RunInfo &runInfo)
{
    AicpuKernelParamBuilder paramBuilder(soName_, deviceKernelName_);
    uint64_t argsNum = GetKernelArgsNum(launchParam);
    Status status = paramBuilder.Init(launchParam, argsNum, kernelInfo_);
    MKI_CHECK(status.Ok(), "failed to build kernel params", return status);
    const MkiRtAicpuKernelParam &kernelParam = paramBuilder.GetKernelParam();
    MKI_LOG(INFO) << "Ready to run, KernelInfo:\n" << kernelInfo_.ToString();

    MKI_LOG(DEBUG) << "launch aicpu function ex with args";
    int ret = Mki::MkiRtAicpuFunctionLaunchExWithArgs("", &kernelParam, runInfo.GetStream());
    MKI_CHECK(ret == MKIRT_SUCCESS, "Mki RtAicpuFunction LaunchExWithArgs fail, errCode:" << ret,
              return Status::FailStatus(ERROR_LAUNCH_KERNEL_ERROR, "Mki RtAicpuFunction LaunchExWithArgs fail"));

    return Status::OkStatus();
}

Status AicpuKernelBase::BuildArgs(const LaunchParam &launchParam, RunInfo &runinfo, void *hostBuffer)
{
    (void)launchParam;
    (void)runinfo;
    (void)hostBuffer;
    MKI_LOG(ERROR) << "BuildArgs in AicpuKernel is not implemented!";
    return Status::FailStatus(ERROR_INVALID_VALUE, "Mki BuildArgs fail");
}

Status AicpuKernelBase::RunWithArgs(void *args, void *stream, bool isDeviceAddr)
{
    (void)args;
    (void)stream;
    (void)isDeviceAddr;
    MKI_LOG(ERROR) << "RunWithArgs in AicpuKernel is not implemented!";
    return Status::FailStatus(ERROR_INVALID_VALUE, "Mki RunWithArgs fail");
}

bool AicpuKernelBase::CanSupport(const LaunchParam &launchParam) const
{
    UNUSED_VALUE(launchParam);
    MKI_CHECK(false, "CanSupport has to be overriden!", return false);
}

void AicpuKernelBase::Copy(const AicpuKernelBase &other)
{
    kernelName_ = other.kernelName_;
    launchBufferSize_ = other.launchBufferSize_;
    creator_ = other.creator_;
    kernelInfo_.Copy(other.kernelInfo_);

    deviceKernelName_ = other.deviceKernelName_;
    soName_ = other.soName_;
}

uint64_t AicpuKernelBase::GetTilingSize(const LaunchParam &launchParam) const
{
    UNUSED_VALUE(launchParam);
    return launchBufferSize_;
}

Status AicpuKernelBase::InitImpl(const LaunchParam &launchParam)
{
    UNUSED_VALUE(launchParam);
    MKI_CHECK(false, "InitImpl has to be overriden!", return Status::FailStatus(-1));
}

Kernel *AicpuKernelBase::Clone() const
{
    MKI_CHECK(creator_ != nullptr, kernelName_ << " creator is nullptr", return nullptr);
    AicpuKernelBase *kernel = creator_();
    kernel->Copy(*this);
    return kernel;
}

void SetAicpuKernelSelfCreator(AicpuKernelBase &kernel, AicpuKernelBase::AicpuKernelSelfCreator func)
{
    MKI_CHECK(func != nullptr, "creator function is nullptr", return);
    kernel.creator_ = func;
}
} // namespace Mki
