/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/base/kernel_base.h"
#include <acl/acl.h>
#include <securec.h>
#include "mki/utils/assert/assert.h"
#include "mki/utils/checktensor/check_tensor.h"
#include "mki/utils/file_system/file_system.h"
#include "mki/utils/log/log.h"
#include "mki/utils/rt/rt.h"
#include "mki/utils/math/tensor_utils.h"
#include "mki/utils/memset/clear_tensors.h"

namespace {
constexpr uint32_t TILING_ADDR_NEG_IDX = 2;
constexpr uint32_t OVERFLOW_ADDR_NEG_IDX = 1;
#ifdef USE_ASCENDC_DUMP
constexpr uint32_t ALL_DUMPSIZE = 75 * 1024 * 1024;
#endif
}
#ifdef USE_ASCENDC_DUMP
namespace Adx {
    void AdumpPrintWorkSpace(const void *dumpBufferAddr, const size_t dumpBufferSize,
                             aclrtStream stream, const char *opType);
}
#endif

namespace Mki {
class KernelParamBuilder {
public:
    Status Init(const LaunchParam &launchParam, const RunInfo &runInfo, uint64_t argsNum, const KernelInfo &kernelInfo,
                uint8_t *hostBuffer = nullptr)
    {
        uint8_t *argsPtr = kernelInfo.GetArgs();
        if (hostBuffer) {
            argsPtr = hostBuffer;
        }
        uint64_t argsSize = kernelInfo.GetArgsSize();
        MKI_CHECK(argsPtr != nullptr, "kernel info args is nullptr, please check first error before.",
                  return Status::FailStatus(ERROR_INVALID_VALUE));
        MKI_CHECK(argsNum * sizeof(void *) <= argsSize,
                  "args size invalid",
                  return Status::FailStatus(ERROR_INVALID_VALUE));
        auto ret = memset_s(argsPtr, argsSize, 0, argsNum * sizeof(void *));
        MKI_CHECK(ret == EOK, "memory set failed", return Status::FailStatus(ERROR_MEMERY_COPY_ERROR));
        void **args = reinterpret_cast<void **>(static_cast<void *>(argsPtr));
        // set hwsync
        int64_t hwsyncIdx = kernelInfo.GetHwsyncIdx();
        auto status = UpdateHwsyncArgs(args, argsNum, hwsyncIdx);
        MKI_CHECK(status.Ok(), "failed to update hwsync args", return status);
        // set const input
        bool launchWithTiling = kernelInfo.GetLaunchWithTiling();
        const auto &constTensorInfos = kernelInfo.GetConstTensorInfos();
        if (launchWithTiling) {
            constexpr size_t maxConstTensorCount = 1024;
            size_t constTensorCount = kernelInfo.GetConstTensorCount();
            if (constTensorCount > 0) {
                MKI_CHECK(constTensorCount < maxConstTensorCount,
                          "const tensor size check failed, is "<< constTensorCount,
                          return Status::FailStatus(ERROR_INVALID_VALUE));
                hostInfo_.reset(new (std::nothrow) RtHostInputInfoT[constTensorCount]);
                MKI_CHECK(hostInfo_ != nullptr, "hostInfo size nullptr",
                          return Status::FailStatus(ERROR_INVALID_VALUE));
                uint64_t constTensorOffset =
                    Utils::GetTensorAlignedSize(kernelInfo.GetTilingUsedSize()) + argsNum * sizeof(void *);
                status = UpdateConstTensorArgs(args, hostInfo_.get(), constTensorOffset, constTensorInfos);
                MKI_CHECK(status.Ok(), "failed to update const tensor args, tiling flag: " << launchWithTiling,
                          return status);
            }
            argsEx_.hostInputInfoPtr = hostInfo_.get();
            argsEx_.hostInputInfoNum = constTensorCount;
        } else {
            uint64_t constTensorOffset = kernelInfo.GetConstTensorOffset();
            status = UpdateConstTensorArgs(args, runInfo.GetTilingDeviceAddr(), constTensorOffset, constTensorInfos);
            MKI_CHECK(status.Ok(), "failed to update const tensor args, tiling flag: " << launchWithTiling,
                      return status);
        }
        // set input / output / workspace
        const auto &workspaces = kernelInfo.GetScratchSizes();
        status = UpdateInOutWkspArgs(args, argsNum, workspaces, launchParam, runInfo.GetScratchDeviceAddr());
        MKI_CHECK(status.Ok(), "failed to update input output wksp args", return status);
        // set tiling
        status = launchWithTiling ? UpdateTilingArgs(argsEx_, argsNum)
                                  : UpdateTilingArgs(args, argsNum, runInfo.GetTilingDeviceAddr());
        MKI_CHECK(status.Ok(), "failed to get launch with tiling", return status);
        // set overflow addr
        status = UpdateOverflowArgs(args, argsNum);
        MKI_CHECK(status.Ok(), "failed to set overflow args", return status);
        // Memset
        const auto &memsetInfo = kernelInfo.GetMemsetInfo();
        status = MemsetTensorArgs(args, argsNum, runInfo.GetStream(), memsetInfo);
        MKI_CHECK(status.Ok(), "failed to memset tensor args", return status);

        // launch
        kernelParam_.tilingId = kernelInfo.GetTilingId();
        kernelParam_.blockDim = kernelInfo.GetBlockDim();
        kernelParam_.argsEx = &argsEx_;
        argsEx_.args = argsPtr;
        argsEx_.argsSize = argsSize;
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
            MKI_CHECK(st == MKIRT_SUCCESS,
                "Mki Get RtC2cCtrlAddr fail", return Status::FailStatus(ERROR_RUN_TIME_ERROR));
            MKI_LOG(INFO) << "args info: hwsync " << hwsyncIdx;
            *(args + static_cast<uint64_t>(hwsyncIdx)) = addr;
        }
        return Status::OkStatus();
    }

    Status UpdateInOutWkspArgs(void **args, uint64_t argsNum, const MiniVector<uint64_t> &workspaces,
                               const LaunchParam &launchParam, uint8_t *workspaceAddr) const
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
        size_t workspaceNum = workspaces.size();
        uint64_t offset = 0;
        for (size_t i = 0; i < workspaceNum && idx < argsNum; idx++) {
            if (args[idx] != nullptr) {
                continue;
            }
            MKI_LOG(DEBUG) << "args info: workspace " << idx << " offset " << offset;
            args[idx] = workspaceAddr + offset;
            offset += workspaces.at(i++);
        }

        return Status::OkStatus();
    }

    Status UpdateOverflowArgs(void **args, uint64_t argsNum) const
    {
        void *overflowAddr = nullptr;
        int32_t st = MkiRtCtxGetOverflowAddr(&overflowAddr);
        MKI_CHECK(st == MKIRT_SUCCESS, "Mki Get RtC2cCtrlAddr failed: %d" << st,
                  return Status::FailStatus(ERROR_RUN_TIME_ERROR));
        args[argsNum - OVERFLOW_ADDR_NEG_IDX] = overflowAddr;
        MKI_LOG(DEBUG) << "args info: overflow addr " << (argsNum - OVERFLOW_ADDR_NEG_IDX);

        return Status::OkStatus();
    }

    Status MemsetTensorArgs(void **args, uint64_t argsNum, void *stream,
                            const MiniVector<KernelInfo::MemsetInfo> &memsetInfo) const
    {
        if (memsetInfo.size() != 0) {
            Status status = ClearTensors(args, argsNum, memsetInfo, stream);
            MKI_CHECK(status.Ok(), "failed to clear tensors", return status);
        }
        return Status::OkStatus();
    }

    Status UpdateConstTensorArgs(void **args, RtHostInputInfoT *info, uint64_t constTensorOffset,
                                 const MiniVector<KernelInfo::ConstTensorInfo> &constTensorInfos) const
    {
        uint64_t offset = constTensorOffset;
        for (uint64_t i = 0; i < constTensorInfos.size(); i++) {
            auto &constTensorInfo = constTensorInfos.at(i);
            args[constTensorInfo.argIdx] = info + i; // placeholder
            info[i].addrOffset = constTensorInfo.argIdx * sizeof(void *);
            info[i].dataOffset = offset;
            MKI_LOG(DEBUG) << "args info: const tensor " << constTensorInfo.argIdx << " offset in args " << offset;
            offset += constTensorInfo.size;
        }
        return Status::OkStatus();
    }

    Status UpdateConstTensorArgs(void **args, uint8_t *tilingDeviceAddr, uint64_t constTensorOffset,
                                 const MiniVector<KernelInfo::ConstTensorInfo> &constTensorInfos) const
    {
        uint64_t offset = constTensorOffset;
        for (size_t i = 0; i < constTensorInfos.size(); i++) {
            auto &constTensorInfo = constTensorInfos.at(i);
            args[constTensorInfo.argIdx] = tilingDeviceAddr + offset;
            MKI_LOG(DEBUG) << "args info: const tensor " << constTensorInfo.argIdx << " offset in tiling " << offset;
            offset += constTensorInfo.size;
        }
        return Status::OkStatus();
    }

    Status UpdateTilingArgs(RtArgsExT &argsEx, uint64_t argsNum) const
    {
        MKI_CHECK((argsNum > 2), "argsNum invalid : " << argsNum, return Status::FailStatus(ERROR_INVALID_VALUE));
        MKI_LOG(DEBUG) << "args info: tiling " << (argsNum - TILING_ADDR_NEG_IDX);
        argsEx.hasTiling = 1;
        argsEx.tilingAddrOffset = (argsNum - TILING_ADDR_NEG_IDX) * sizeof(void *);
        argsEx.tilingDataOffset = argsNum * sizeof(void *);
        return Status::OkStatus();
    }

    Status UpdateTilingArgs(void **args, uint64_t argsNum, uint8_t *tilingDeviceAddr) const
    {
        MKI_LOG(DEBUG) << "args info: tiling " << (argsNum - TILING_ADDR_NEG_IDX);
        args[argsNum - TILING_ADDR_NEG_IDX] = tilingDeviceAddr;
        return Status::OkStatus();
    }

private:
    RtArgsExT argsEx_;
    std::unique_ptr<RtHostInputInfoT[]> hostInfo_{nullptr};
    MkiRtKernelParam kernelParam_;
};

KernelBase::KernelBase(const std::string &opName, const BinHandle *handle) : kernelName_(opName), handle_(handle)
{
    if (handle_ != nullptr) {
        launchBufferSize_ = handle_->GetKernelTilingSize();
        int32_t coreType = handle_->GetKernelCoreType();
        switch (coreType) {
            case -1: MKI_LOG(ERROR) << "Failed to get core type!"; break;   // -1: get core type fail
            case 0: kernelType_ = KernelType::KERNEL_TYPE_AI_CORE; break;   // 0: AI_CORE
            case 2: kernelType_ = KernelType::KERNEL_TYPE_AIV; break;       // 2: AIV
            case 4: kernelType_ = KernelType::KERNEL_TYPE_MIX_AIC; break;   // 4: AIC
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

KernelHandle KernelBase::GetBinaryHandle() const { return handle_->GetHandle(); }

KernelBase::~KernelBase() {}

std::string KernelBase::GetName() const { return kernelName_; }

KernelType KernelBase::GetType() const { return kernelType_; }

const BinHandle *KernelBase::GetBinHandle() const { return handle_; }

const KernelInfo &KernelBase::GetKernelInfo() const { return kernelInfo_; }

void KernelBase::SetLaunchWithTiling(bool flag) { kernelInfo_.SetLaunchWithTiling(flag); }

void KernelBase::SetTilingHostAddr(uint8_t *addr, uint64_t len) { kernelInfo_.SetTilingHostAddr(addr, len); }

void KernelBase::Reset()
{
    kernelInfo_.Reset();
}

Status KernelBase::Init(const LaunchParam &launchParam)
{
    MKI_CHECK(CheckInTensors(launchParam), "Not supported in tensors", return Status::FailStatus(ERROR_INVALID_VALUE));
    MKI_CHECK(CanSupport(launchParam), "Not supported op", return Status::FailStatus(ERROR_INVALID_VALUE));

    kernelInfo_.Reset();

    auto tilingSize = GetTilingSize(launchParam);
    bool launchWithTiling = kernelInfo_.GetLaunchWithTiling();
    if (launchWithTiling) {
        auto st = kernelInfo_.AllocTilingHost(tilingSize);
        MKI_CHECK(st.Ok(), "Failed to alloc host tiling buffer " << st.ToString(), return st);
    }

    auto status = InitImpl(launchParam);
#ifdef USE_ASCENDC_DUMP
    // 由于Adump 功能需要多分配75MB内存在workspace最后
    kernelInfo_.GetScratchSizes().push_back(ALL_DUMPSIZE);
#endif
    MKI_CHECK(status.Ok(), "Failed to init run info " << status.ToString(),
              return Status::FailStatus(ERROR_INVALID_VALUE));

    auto kernelParamNum = GetKernelArgsNum(launchParam);
    uint64_t baseSize = kernelParamNum * sizeof(void *);
    uint64_t argsSize = baseSize;
    if (!launchWithTiling) {
        return kernelInfo_.InitArgs(argsSize);
    }

    uint64_t tilingUsedSize = Utils::GetTensorAlignedSize(kernelInfo_.GetTilingUsedSize());
    argsSize += tilingUsedSize;
    uint64_t constTensorSize = kernelInfo_.GetTilingSize() - kernelInfo_.GetConstTensorOffset();
    argsSize += constTensorSize;
    MKI_LOG(INFO) << "args num " << kernelParamNum << ", tiling used size " << tilingUsedSize
                  << ", const tensor size " << constTensorSize;
    status = kernelInfo_.InitArgs(argsSize);
    MKI_CHECK(status.Ok(), "failed to init args", return status);
    uint8_t *args = kernelInfo_.GetArgs();
    auto ret = memcpy_s(args + baseSize, argsSize - baseSize,
                        kernelInfo_.GetTilingHostAddr(), kernelInfo_.GetTilingUsedSize());
    MKI_CHECK(ret == EOK, "failed to copy tiling",
              return Status::FailStatus(ERROR_MEMERY_COPY_ERROR));
    MKI_LOG(INFO) << "copy tiling data " << tilingUsedSize << " to args offset " << baseSize;
    if (constTensorSize > 0) {
        ret = memcpy_s(args + baseSize + tilingUsedSize, argsSize - baseSize - tilingUsedSize,
                       kernelInfo_.GetTilingHostAddr() + kernelInfo_.GetConstTensorOffset(), constTensorSize);
        MKI_CHECK(ret == EOK, "failed to copy const tensor",
                  return Status::FailStatus(ERROR_MEMERY_COPY_ERROR));
        MKI_LOG(INFO) << "copy const data " << constTensorSize << " to args offset " << baseSize + tilingUsedSize;
    }
    return Status::OkStatus();
}

uint64_t KernelBase::GetKernelArgsNum(const LaunchParam &launchParam)
{
    uint64_t inputOutputNum = launchParam.GetInTensorCount() + launchParam.GetOutTensorCount();
    uint64_t constInputNum = kernelInfo_.GetConstTensorCount();
    uint64_t workspaceNum = kernelInfo_.GetScratchSizes().size();
    uint64_t hwsyncNum = kernelInfo_.GetHwsyncIdx() < 0 ? 0 : 1;
    MKI_LOG(DEBUG) << "kernel param: " << inputOutputNum << " in/out, " << constInputNum << " const in, "
                << workspaceNum << " workspaces, " << hwsyncNum << " hwsync";
    return inputOutputNum + constInputNum + workspaceNum + hwsyncNum + 2; // 2 for tiling addr and overflow addr
}

Status KernelBase::Run(const LaunchParam &launchParam, RunInfo &runInfo)
{
    Status st = BuildArgs(launchParam, runInfo, nullptr);
    MKI_CHECK(st.Ok(), "build args failed, abort running", return st);
    st = RunWithArgs(kernelInfo_.GetArgs(), runInfo.GetStream(), false);
#ifdef USE_ASCENDC_DUMP
    // Adapt Dump tensor and printf
    // 在原来的workspace后添加75MB
    int ret = aclrtSynchronizeStream(runInfo.GetStream());
    if (ret == 0) {
        uint8_t *dumpWorkspaceAddr = runInfo.GetScratchDeviceAddr() + kernelInfo_.GetTotalScratchSize() - ALL_DUMPSIZE;
        Adx::AdumpPrintWorkSpace(dumpWorkspaceAddr, ALL_DUMPSIZE, runInfo.GetStream(), "device_kernel");
    }
#endif
    return st;
}

bool KernelBase::CanSupport(const LaunchParam &launchParam) const
{
    UNUSED_VALUE(launchParam);
    MKI_CHECK(false, "CanSupport has to be overriden!", return false);
}

void KernelBase::Copy(const KernelBase &other)
{
    kernelName_ = other.kernelName_;
    launchBufferSize_ = other.launchBufferSize_;
    handle_ = other.handle_;
    kernelType_ = other.kernelType_;
    creator_ = other.creator_;
    kernelInfo_.Copy(other.kernelInfo_);
}

uint64_t KernelBase::GetTilingSize(const LaunchParam &launchParam) const
{
    UNUSED_VALUE(launchParam);
    return launchBufferSize_;
}

Status KernelBase::InitImpl(const LaunchParam &launchParam)
{
    UNUSED_VALUE(launchParam);
    MKI_CHECK(false, "InitImpl has to be overriden!",
              return Status::FailStatus(ERROR_INVALID_VALUE));
}

Kernel *KernelBase::Clone() const
{
    MKI_CHECK(creator_ != nullptr, kernelName_ << " creator is nullptr", return nullptr);
    KernelBase *kernel = creator_();
    MKI_CHECK(kernel != nullptr, kernelName_ << " create kernel failed", return nullptr);
    kernel->Copy(*this);
    return kernel;
}

Status KernelBase::BuildArgs(const LaunchParam &launchParam, RunInfo &runinfo, void *hostBuffer)
{
    builder_.reset(new(KernelParamBuilder));
    uint64_t argsNum = GetKernelArgsNum(launchParam);
    uint8_t *hostBufferPtr = reinterpret_cast<uint8_t*>(hostBuffer);
    Status status = builder_->Init(launchParam, runinfo, argsNum, kernelInfo_, hostBufferPtr);
    return status;
}

Status KernelBase::RunWithArgs(void *args, void *stream, bool isDeviceAddr)
{
    MKI_LOG(INFO) << "Ready to run, KernelInfo:\n" << kernelInfo_.ToString();
    MKI_CHECK(handle_ != nullptr, "handle is nullptr", return Status::FailStatus(ERROR_INVALID_VALUE));
    const MkiRtKernelParam &kernelParam = builder_->GetKernelParam();
    MKI_CHECK(kernelParam.argsEx != nullptr, "kernelParam's argsEx is nullptr",
              return Status::FailStatus(ERROR_INVALID_VALUE));
#ifdef _DEBUG
    MKI_LOG(DEBUG) << "ARGS CONTENT IS:";
    for (size_t i = 0; i < kernelParam.argsEx->argsSize / sizeof(void*); i++) {
        MKI_LOG(DEBUG) << ((void**)(kernelParam.argsEx->args))[i];
    }
#endif
    kernelParam.argsEx->args = args;
    if (isDeviceAddr) {
        kernelParam.argsEx->isNoNeedH2DCopy = 1;
    }
    if (*handle_->GetHandle() != nullptr) {
        MKI_LOG(DEBUG) << "launch function with handle";
        int st = MkiRtFunctionLaunchWithHandle(*handle_->GetHandle(), &kernelParam, stream, nullptr);
        MKI_CHECK(
            st == MKIRT_SUCCESS, "Mki RtFunction LaunchWithHandle fail",
            return Status::FailStatus(ERROR_LAUNCH_KERNEL_ERROR, "Mki RtFunction LaunchWithHandle fail"));
    } else {
        MKI_LOG(DEBUG) << "launch function with flag";
        int st = MkiRtFunctionLaunchWithFlag(handle_->GetHandle(), &kernelParam, stream, nullptr);
        MKI_CHECK(st == MKIRT_SUCCESS, "Mki RtFunction LaunchWithFlag fail",
                    return Status::FailStatus(ERROR_LAUNCH_KERNEL_ERROR, "Mki RtFunction Launch fail"));
    }
    return Status::OkStatus();
}

void SetKernelSelfCreator(KernelBase &kernel, KernelBase::KernelSelfCreator func)
{
    MKI_CHECK(func != nullptr, "creator function is nullptr", return);
    kernel.creator_ = func;
}
} // namespace Mki