/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/base/bishengir_kernel_base.h"
#include <securec.h>
#include "mki/utils/assert/assert.h"
#include "mki/utils/checktensor/check_tensor.h"
#include "mki/utils/file_system/file_system.h"
#include "mki/utils/log/log.h"
#include "mki/utils/rt/rt.h"
#include "mki/utils/math/tensor_utils.h"
#include "mki/utils/memset/clear_tensors.h"

namespace Mki {
const uint64_t SPACE_IDX_ZERO = 0;
const uint64_t SPACE_IDX_ONE = 1;
const uint64_t SPACE_IDX_TWO = 2;
const uint64_t SPACE_IDX_THREE = 3;
const uint64_t SPACE_IDX_FOUR = 4;
const uint64_t BISHENRIR_INPUT_NUM = 3;
class BishengIRParamBuilder {
public:
    Status Init(const LaunchParam &launchParam, const RunInfo &runInfo, KernelInfo &kernelInfo)
    {
        const auto &kernelArgsIndex = kernelInfo.GetKernelArgsIndex();
        uint64_t argsNum = kernelInfo.GetKernelArgsIndex().size();
        uint8_t *argsPtr = kernelInfo.GetArgs();
        uint64_t argsSize = kernelInfo.GetArgsSize();
        MKI_CHECK(argsPtr != nullptr,
            "kernel info args is nullptr, please check first error before.",
            return Status::FailStatus(ERROR_INVALID_VALUE));

        auto ret = memset_s(argsPtr, argsSize, 0, argsSize);
        MKI_CHECK(ret == EOK, "memory set failed", return Status::FailStatus(ERROR_MEMERY_COPY_ERROR));
        void **args = reinterpret_cast<void **>(static_cast<void *>(argsPtr));
        int64_t hwsyncIdx = kernelArgsIndex[0];
        auto status = UpdateHwsyncArgs(args, argsNum, hwsyncIdx);
        MKI_CHECK(status.Ok(), "failed to update hwsync args", return status);

        status = UpdateInOutArgs(args, launchParam, kernelArgsIndex);
        MKI_CHECK(status.Ok(), "failed to update in out args", return status);

        status = UpdateWSArgs(args, launchParam, runInfo, kernelInfo, kernelArgsIndex);

        MKI_CHECK(status.Ok(), "failed to update workspace args", return status);

        status = UpdateTilingArgs(args, launchParam, runInfo, kernelInfo, kernelArgsIndex);
        MKI_CHECK(status.Ok(), "failed to update tiling args", return status);

        kernelParam_.tilingId = kernelInfo.GetTilingId();
        kernelParam_.blockDim = kernelInfo.GetBlockDim();
        kernelParam_.argSize = argsSize;
        kernelParam_.args = argsPtr;
        return status;
    }

    const MkiRtKernelParam &GetKernelParam() const
    {
        return kernelParam_;
    }

private:
    Status UpdateHwsyncArgs(void **args, uint64_t argsNum, int64_t hwsyncIdx) const
    {
        if (hwsyncIdx >= 0 && static_cast<uint64_t>(hwsyncIdx) < argsNum) {
            uint64_t *addr = nullptr;
            uint32_t len = 0;
            int st = MkiRtGetC2cCtrlAddr(reinterpret_cast<uint64_t *>(&addr), &len);
            MKI_CHECK(
                st == MKIRT_SUCCESS, "Mki Get RtC2cCtrlAddr fail", return Status::FailStatus(ERROR_RUN_TIME_ERROR));
            MKI_LOG(INFO) << "args info: hwsync " << hwsyncIdx;
            *(args + static_cast<uint64_t>(hwsyncIdx)) = addr;
        }
        return Status::OkStatus();
    }

    Status UpdateInOutArgs(
        void **args, const LaunchParam &launchParam, const MiniVector<uint64_t> &kernelArgsIndex) const
    {
        uint64_t argsNum = kernelArgsIndex.size();
        size_t inputNum = launchParam.GetInTensorCount();
        uint64_t idx = 1;
        for (size_t i = 0; i < inputNum && idx < argsNum; idx++) {
            auto idxArgs = kernelArgsIndex[idx];
            args[idxArgs] = launchParam.GetInTensor(i).data;

            args[idxArgs + 1] = args[idxArgs];
            uint64_t *current = reinterpret_cast<uint64_t *>(&args[idxArgs + 2]);
            current[SPACE_IDX_ONE] = launchParam.GetInTensor(i).desc.dims[0];
            current[SPACE_IDX_TWO] = launchParam.GetInTensor(i).desc.dims[1];
            current[SPACE_IDX_THREE] = launchParam.GetInTensor(i).desc.dims[1];
            current[SPACE_IDX_FOUR] = 1;
            i++;
        }
        if (inputNum < BISHENRIR_INPUT_NUM) {
            idx += 1;
        }

        size_t outputNum = launchParam.GetOutTensorCount();
        for (size_t i = 0; i < outputNum && idx < argsNum; idx++) {
            auto idxArgs = kernelArgsIndex[idx];

            args[idxArgs] = launchParam.GetOutTensor(i).data;

            args[idxArgs + 1] = args[idxArgs];

            uint64_t *current = reinterpret_cast<uint64_t *>(&args[idxArgs + 2]);
            current[SPACE_IDX_ZERO] = 0;
            current[SPACE_IDX_ONE] = launchParam.GetOutTensor(i).desc.dims[0];
            current[SPACE_IDX_TWO] = launchParam.GetOutTensor(i).desc.dims[1];
            current[SPACE_IDX_THREE] = launchParam.GetOutTensor(i).desc.dims[1];
            current[SPACE_IDX_FOUR] = 1;
            i++;
        }
        return Status::OkStatus();
    }

    Status UpdateWSArgs(void **args, const LaunchParam &launchParam, const RunInfo &runInfo,
        const KernelInfo &kernelInfo, const MiniVector<uint64_t> &kernelArgsIndex) const
    {
        auto wsIndex =
            launchParam.GetInTensorCount() + launchParam.GetOutTensorCount() + (kernelInfo.GetHwsyncIdx() < 0 ? 0 : 1);
        if (launchParam.GetInTensorCount() != BISHENRIR_INPUT_NUM) {
            wsIndex += 1;
        }
        auto argsNum = kernelArgsIndex.size();
        for (size_t i = 0; i < kernelInfo.GetScratchSizes().size() && wsIndex < argsNum; i++) {
            auto idxArgs = kernelArgsIndex[wsIndex];
            size_t workspaceSize = kernelInfo.GetScratchSizes()[i];
            args[idxArgs] = runInfo.GetScratchDeviceAddr();
            args[idxArgs + 1] = args[idxArgs];
            uint64_t *current = reinterpret_cast<uint64_t *>(&args[idxArgs + 2]);
            current[SPACE_IDX_ZERO] = 0;
            current[SPACE_IDX_ONE] = workspaceSize;
            current[SPACE_IDX_TWO] = 1;
        }
        return Status::OkStatus();
    }

    Status UpdateTilingArgs(void **args, const LaunchParam &launchParam, const RunInfo &runInfo,
        const KernelInfo &kernelInfo, const MiniVector<uint64_t> &kernelArgsIndex) const
    {
        auto tilingIndex = launchParam.GetInTensorCount() + launchParam.GetOutTensorCount() +
                           kernelInfo.GetScratchSizes().size() + (kernelInfo.GetHwsyncIdx() < 0 ? 0 : 1);
        if (launchParam.GetInTensorCount() != BISHENRIR_INPUT_NUM) {
            tilingIndex += 1;
        }
        auto idxArgs = kernelArgsIndex[tilingIndex];

        auto tilingByteSize = kernelInfo.GetTilingSize();
        args[idxArgs] = runInfo.GetTilingDeviceAddr();
        uint64_t *current = reinterpret_cast<uint64_t *>(&args[idxArgs + 2]);
        // uint64_t
        current[SPACE_IDX_ZERO] = 0;
        current[SPACE_IDX_ONE] = tilingByteSize / sizeof(uint64_t);
        current[SPACE_IDX_TWO] = 1;
        return Status::OkStatus();
    }

private:
    RtArgsExT argsEx_;
    std::unique_ptr<RtHostInputInfoT[]> hostInfo_{nullptr};
    MkiRtKernelParam kernelParam_;
};

BishengIRKernelBase::BishengIRKernelBase(const std::string &opName, const BinHandle *handle)
    : kernelName_(opName), handle_(handle)
{
    if (handle_ != nullptr) {
        launchBufferSize_ = handle_->GetKernelTilingSize();
        int32_t coreType = handle_->GetKernelCoreType();
        switch (coreType) {
            case -1:
                MKI_LOG(ERROR) << "Failed to get core type!";
                break;  // -1: get core type fail
            case 0:
                kernelType_ = KernelType::KERNEL_TYPE_AI_CORE;
                break;  // 0: AI_CORE
            case 2:
                kernelType_ = KernelType::KERNEL_TYPE_AIV;
                break;  // 2: AIV
            case 4:
                kernelType_ = KernelType::KERNEL_TYPE_MIX_AIC;
                break;  // 4: AIC
            default:
                MKI_LOG(WARN) << "Unexpected core type, use AIC as default!";
                kernelType_ = KernelType::KERNEL_TYPE_MIX_AIC;
        }
        MKI_LOG(DEBUG) << "Create kernel " << kernelName_ << ", launch buffer size " << launchBufferSize_
                       << ", coreType: " << kernelType_;
    } else {
        MKI_LOG(ERROR) << "Kernel " << kernelName_ << " handle is nullptr";
    }
}

BishengIRKernelBase::~BishengIRKernelBase()
{}

std::string BishengIRKernelBase::GetName() const
{
    return kernelName_;
}

KernelType BishengIRKernelBase::GetType() const
{
    return kernelType_;
}

const BinHandle *BishengIRKernelBase::GetBinHandle() const
{
    return handle_;
}

const KernelInfo &BishengIRKernelBase::GetKernelInfo() const
{
    return kernelInfo_;
}

void BishengIRKernelBase::SetLaunchWithTiling(bool flag)
{
    kernelInfo_.SetLaunchWithTiling(flag);
}

void BishengIRKernelBase::SetTilingHostAddr(uint8_t *addr, uint64_t len)
{
    kernelInfo_.SetTilingHostAddr(addr, len);
}

void BishengIRKernelBase::Reset()
{
    kernelInfo_.Reset();
}

Status BishengIRKernelBase::Init(const LaunchParam &launchParam)
{
    MKI_CHECK(CheckInTensors(launchParam), "Not supported in tensors", return Status::FailStatus(ERROR_INVALID_VALUE));
    MKI_CHECK(CanSupport(launchParam), "Not supported op", return Status::FailStatus(ERROR_INVALID_VALUE));

    kernelInfo_.Reset();

    auto status = InitImpl(launchParam);
    MKI_CHECK(
        status.Ok(), "Failed to init run info " << status.ToString(), return Status::FailStatus(ERROR_INVALID_VALUE));
    uint64_t argsSize = GetKernelArgsTotalSize(launchParam);
    UpdateKernelArgsIndex(launchParam);
    status = kernelInfo_.InitArgs(argsSize);
    return status;
}

Status BishengIRKernelBase::Run(const LaunchParam &launchParam, RunInfo &runInfo)
{
    BishengIRParamBuilder bishengIRParamBuilder;
    Status status = bishengIRParamBuilder.Init(launchParam, runInfo, kernelInfo_);
    MKI_CHECK(status.Ok(), "failed to build kernel params", return status);
    const MkiRtKernelParam &kernelParam = bishengIRParamBuilder.GetKernelParam();
    int st = MkiRtFunctionLaunch(handle_->GetHandle(), &kernelParam, runInfo.GetStream());
    MKI_CHECK(st == MKIRT_SUCCESS,
        "Mki RtFunction LaunchWithFlag fail",
        return Status::FailStatus(ERROR_LAUNCH_KERNEL_ERROR, "Mki RtFunction Launch fail"));
    return Status::OkStatus();
}

Status BishengIRKernelBase::BuildArgs(const LaunchParam &launchParam, RunInfo &runinfo, void *hostBuffer)
{
    (void)launchParam;
    (void)runinfo;
    (void)hostBuffer;
    MKI_LOG(ERROR) << "BuildArgs in BishengIRKernel is not implemented!";
    return Status::FailStatus(ERROR_INVALID_VALUE, "Mki BuildArgs fail");
}

Status BishengIRKernelBase::RunWithArgs(void *args, void *stream, bool isDeviceAddr)
{
    (void)args;
    (void)stream;
    (void)isDeviceAddr;
    MKI_LOG(ERROR) << "RunWithArgs in BishengIRKernel is not implemented!";
    return Status::FailStatus(ERROR_INVALID_VALUE, "Mki RunWithArgs fail");
}

void BishengIRKernelBase::UpdateKernelArgsIndex(const LaunchParam &launchParam)
{
    // 按照char数据大小更新KernelArgs的起始位置
    auto &kernelArgsIndex = kernelInfo_.GetKernelArgsIndex();
    uint64_t hwsyncNum = kernelInfo_.GetHwsyncIdx() < 0 ? 0 : 1;
    uint64_t inputOutputNum = launchParam.GetInTensorCount() + launchParam.GetOutTensorCount();
    if (launchParam.GetInTensorCount() != BISHENRIR_INPUT_NUM) {
        inputOutputNum = BISHENRIR_INPUT_NUM + launchParam.GetOutTensorCount();
    }
    uint64_t workspaceNum = kernelInfo_.GetScratchSizes().size();
    if (hwsyncNum == 1) {
        kernelArgsIndex.push_back(0);
    }
    kernelArgsIndex.push_back(1);

    for (size_t i = 1; i <= inputOutputNum; i++) {
        kernelArgsIndex.push_back((kernelArgsIndex.empty() ? 0 : kernelArgsIndex[kernelArgsIndex.size() - 1]) +
                                  (getStrideInOutput() * sizeof(uint64_t) / sizeof(void *) + getNumPtr()));
    }
    // 包含worlspace（workspaceNum.size()） 和 tiling (+1)
    for (size_t i = 0; i < workspaceNum; i++) {
        kernelArgsIndex.push_back((kernelArgsIndex.empty() ? 0 : kernelArgsIndex[kernelArgsIndex.size() - 1]) +
                                  (getStrideTilingWs() * sizeof(uint64_t) / sizeof(void *) + getNumPtr()));
    }
    return;
}

uint64_t BishengIRKernelBase::GetKernelArgsTotalSize(const LaunchParam &launchParam)
{
    uint64_t inputOutputNum = BISHENRIR_INPUT_NUM + launchParam.GetOutTensorCount();
    uint64_t workspaceNum = kernelInfo_.GetScratchSizes().size();
    uint64_t hwsyncNum = kernelInfo_.GetHwsyncIdx() < 0 ? 0 : 1;
    MKI_LOG(DEBUG) << "kernel param: " << inputOutputNum << " in/out, " << workspaceNum << " workspaces, " << hwsyncNum
                   << " hwsync";
    uint64_t totalSize = 0;
    totalSize +=
        inputOutputNum * sizeof(void *) * getNumPtr() + inputOutputNum * sizeof(uint64_t) * getStrideInOutput();
    totalSize += hwsyncNum * sizeof(void *);
    totalSize += workspaceNum * sizeof(void *) * getNumPtr() + workspaceNum * sizeof(uint64_t) * getStrideTilingWs();

    totalSize += sizeof(void *) * getNumPtr() + workspaceNum * sizeof(uint64_t) * getStrideTilingWs();
    return totalSize;
}

const uint64_t BishengIRKernelBase::getNumPtr() const
{
    return numPtr_;
}

void BishengIRKernelBase::setStrideInOutput(const uint64_t numPtr)
{
    strideInOutput_ = numPtr;
}
const uint64_t BishengIRKernelBase::getStrideInOutput() const
{
    return strideInOutput_;
}
void BishengIRKernelBase::setStrideTilingWs(const uint64_t numPtr)
{
    strideTilingWs_ = numPtr;
    return;
}
const uint64_t BishengIRKernelBase::getStrideTilingWs() const
{
    return strideTilingWs_;
}

bool BishengIRKernelBase::CanSupport(const LaunchParam &launchParam) const
{
    UNUSED_VALUE(launchParam);
    MKI_CHECK(false, "CanSupport has to be overriden!", return false);
}

Status BishengIRKernelBase::Copy(const Kernel &other)
{
    auto const* kernelbase = dynamic_cast<const BishengIRKernelBase*>(&other);
    MKI_CHECK(kernelbase != nullptr, "failed to convert kernel type", return Status::FailStatus(ERROR_KERNEL_NOT_EXIST));
    kernelName_ = kernelbase->kernelName_;
    launchBufferSize_ = kernelbase->launchBufferSize_;
    handle_ = kernelbase->handle_;
    kernelType_ = kernelbase->kernelType_;
    creator_ = kernelbase->creator_;
    kernelInfo_.Copy(kernelbase->kernelInfo_);
    return Status::OkStatus();
}

uint64_t BishengIRKernelBase::GetTilingSize(const LaunchParam &launchParam) const
{
    UNUSED_VALUE(launchParam);
    return launchBufferSize_;
}

Status BishengIRKernelBase::InitImpl(const LaunchParam &launchParam)
{
    UNUSED_VALUE(launchParam);
    MKI_CHECK(false, "InitImpl has to be overriden!", return Status::FailStatus(ERROR_INVALID_VALUE));
}

Kernel *BishengIRKernelBase::Clone() const
{
    MKI_CHECK(creator_ != nullptr, kernelName_ << " creator is nullptr", return nullptr);
    BishengIRKernelBase *kernel = creator_();
    MKI_CHECK(kernel != nullptr, kernelName_ << " create kernel failed", return nullptr);
    kernel->Copy(*this);
    return kernel;
}

void SetKernelSelfCreator(BishengIRKernelBase &kernel, BishengIRKernelBase::KernelSelfCreator func)
{
    MKI_CHECK(func != nullptr, "creator function is nullptr", return);
    kernel.creator_ = func;
}
}  // namespace Mki