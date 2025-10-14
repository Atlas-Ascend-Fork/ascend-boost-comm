/*
 * Copyright (c) 2025 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/utils/rt/resource/resource.h"
#include <mutex>
#include "mki/utils/dl/dl.h"
#include "mki/utils/env/env.h"
#include "mki/types.h"

namespace Mki {
using AclrtGetResInCurrentThreadFunc = int(*)(int, uint32_t*);

int GetResInCurrentThread(int type, uint32_t &resource)
{
    static std::once_flag onceFlag;
    static std::atomic<int> initFlag{ERROR_FUNC_NOT_INITIALIZED};  
    static std::unique_ptr<Dl> mkiDl; // 持久保存，避免库被卸载
    static AclrtGetResInCurrentThreadFunc aclFn = nullptr;
    std::call_once(onceFlag, []() {
        std::string p;
        const char *c = GetEnv("ASCEND_HOME_PATH");
        if (c) {
            p = std::string(c) + "/runtime/lib64/libascendcl.so";
        } else {
            p = "libascendcl.so";
        }
        auto dl = std::make_unique<Mki::Dl>(p, false);
        if (!dl->IsValid()) {
            MKI_LOG(ERROR) << "Try load libascendcl.so failed: " << p;
            initFlag.store(ERROR_FUNC_NOT_FOUND, std::memory_order_release);
            return;
        }
        auto sym = dl->GetSymbol("aclrtGetResInCurrentThread");
        if (sym == nullptr) {
            MKI_LOG(WARN) << "Symbol aclrtGetResInCurrentThread not found in: " << p;
            initFlag.store(ERROR_FUNC_NOT_FOUND, std::memory_order_release);
            return;
        }
        mkiDl = std::move(dl); // 保留句柄，防止卸载
        aclFn = reinterpret_cast<AclrtGetResInCurrentThreadFunc>(sym);
        initFlag.store(NO_ERROR, std::memory_order_release);
        MKI_LOG(DEBUG) << "Loaded libascendcl.so and resolved aclrtGetResInCurrentThread from: " << p;
    });

    // 初始化结果判定
    int rc = initFlag.load(std::memory_order_acquire);
    if (rc != NO_ERROR) {
        return rc;
    }

    if (type != 0 && type != 1) {
        MKI_LOG(ERROR) << "aclrtGetResInCurrentThread not support resource type: " << type;
        return ERROR_INVALID_VALUE;
    }

    // 调用前检查函数指针有效性
    if (aclFn == nullptr) {
        MKI_LOG(ERROR) << "aclrtGetResInCurrentThread function pointer is null.";
        return ERROR_FUNC_NOT_FOUND;
    }

    // 调用底层函数
    const int ret = aclFn(type, &resource);
    if (ret != 0) {
        MKI_LOG(ERROR) << "aclrtGetResInCurrentThread failed. type: " << type << " err: " << ret;
        return ERROR_RUN_TIME_ERROR;
    }

    MKI_LOG(INFO) << "Got resource in current thread. type: " << type << " resource: " << resource;
    return NO_ERROR;
}
}