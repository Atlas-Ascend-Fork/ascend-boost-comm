/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#ifndef MKI_LOADER_LOADER_H
#define MKI_LOADER_LOADER_H
#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>
#include "mki/operation.h"
#include "mki/kernel.h"
#include "mki/bin_handle.h"
#include "mki_loader/creator.h"

namespace Mki {
class Loader {
using RtPtrDeleter = std::function<void(uint8_t *)>;
public:
    Loader(const OperationCreators &operationCreators, const KernelCreators &kernelCreators,
           const AicpuKernelCreators &aicpuKernelCreators, const BinaryBasicInfoMap &binaryMap);
    ~Loader();
    Loader() = delete;
    Loader(const Loader &) = delete;
    Loader &operator=(const Loader &other) = delete;
    const std::unordered_map<std::string, Operation *> &GetAllOperations() const;
    void GetOpKernels(const std::string &opName, Mki::KernelMap &kernels) const;
    bool IsValid() const;
    void ReLoad();

private:
    void Load();
    bool LoadKernelBinarys();
    bool CreateOperations();
    bool CreateKernels();

    bool GetAicpuDeviceKernelSo(uint32_t &fileSize);
    bool LoadAicpuKernelBinarys();
    bool CreateAicpuKernels();

    bool OpBaseAddKernels() const;

private:
    const OperationCreators &operationCreators_;
    const KernelCreators &kernelCreators_;
    const AicpuKernelCreators &aicpuKernelCreators_;
    const BinaryBasicInfoMap &binaryMap_;

    std::mutex mtx;
    std::atomic_bool loadSuccess_{false};
    std::unordered_map<std::string, Mki::Operation *> opMap_;
    std::unordered_map<std::string, Mki::KernelMap> opKernelMap_;
    std::unordered_map<std::string, Mki::BinHandle> binHandles_;
    std::unique_ptr<uint8_t[], RtPtrDeleter> aicpuSoHandle_;
    std::unique_ptr<uint8_t[], RtPtrDeleter> aicpuSoNameHandle_;
};
} // namespace Mki
#endif
