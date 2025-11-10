/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/utils/share_memory/share_memory.h"
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <securec.h>
#include <functional>
#include "mki/utils/assert/assert.h"
#include "mki/utils/env/env.h"
#include "mki/utils/log/log.h"

namespace {
constexpr int SEM_TIMEOUT = 300;
const size_t MAX_ENV_STRING_LEN = 12800;
}

namespace Mki {
ShareMemory::ShareMemory(const std::string &name, uint32_t size) : memSize_(size)
{
    fullName_ = name;
    const char *shareMemoryNameSuffix = Mki::GetEnv("ATB_SHARE_MEMORY_NAME_SUFFIX");
    if (shareMemoryNameSuffix && strlen(shareMemoryNameSuffix) <= MAX_ENV_STRING_LEN) {
        MKI_LOG(INFO) << "ATB_SHARE_MEMORY_NAME_SUFFIX is validate, value: " << std::string(shareMemoryNameSuffix);
        fullName_ += std::string(shareMemoryNameSuffix);
    }
    sem_ = sem_open(fullName_.c_str(), O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (SEM_FAILED == sem_) {
        MKI_LOG(ERROR) << "share memory open fail, fullName:" << fullName_;
        return;
    }
    MKI_LOG(INFO) << "create share memory begin, fullName:" << fullName_;

    SemLock();
    shareMemory_ = (uint8_t *)CreateShareMemory(fullName_, memSize_);
    MKI_LOG(INFO) << "create share memory success";
    SemUnLock();
}

ShareMemory::~ShareMemory()
{
    SemLock();
    CleanUpShm();
    SemUnLock();
    CleanUpSem();
}

void *ShareMemory::GetShm()
{
    return shareMemory_;
}

void ShareMemory::SemLock() const
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += SEM_TIMEOUT;
    int ret = sem_timedwait(sem_, &ts);
    // 等待信号量超时
    MKI_LOG_IF(ret == -1 && errno == ETIMEDOUT, ERROR) << "The semaphore waiting duration exceeds 5 minutes. Run the "
        "rm -rf /dev/shm/sem." <<
        fullName_ << " command to clear the semaphore.";
}

void ShareMemory::SemUnLock() const
{
    sem_post(sem_);
}

void *ShareMemory::CreateShareMemory(const std::string &name, uint32_t size)
{
    void *memory = nullptr;
    struct shmid_ds buf;
    key_t key = static_cast<key_t>(std::hash<std::string>{}(name));
    shmid_ = shmget(key, size, IPC_CREAT | 0600); // 0600提供文件所有者有读和写的权限
    MKI_LOG(INFO) << "key: " << key << " shmid: " << shmid_;
    MKI_CHECK(shmid_ != -1, "shmget err, errno is: " << errno, return nullptr);

    memory = shmat(shmid_, nullptr, 0);
    MKI_CHECK(memory != reinterpret_cast<void *>(-1), "shmat err, errno is: " << errno, return nullptr);

    shmctl(shmid_, IPC_STAT, &buf);

    if (buf.shm_nattch == 1) {
        int ret = memset_s(memory, size, 0, size);
        MKI_LOG_IF(ret != EOK, ERROR) << "memset_s Error! Error Code: " << ret;
    }
    return memory;
}

void ShareMemory::CleanUpShm()
{
    int ret = shmdt(shareMemory_);
    MKI_LOG_IF(ret != 0, ERROR) << "shmdt failed, errno is: " << errno;
    shmid_ds buf{};
    shmctl(shmid_, IPC_STAT, &buf);
    if (shmid_ != -1 && buf.shm_nattch == 0) {
        ret = shmctl(shmid_, IPC_RMID, nullptr);
        MKI_LOG_IF(ret != 0, ERROR) << "shmid: " << shmid_ << " delete share memory fail(shmctl IPC_RMID failed.) ret: "
            << ret << " errno is: " << errno;
    }
}

void ShareMemory::CleanUpSem()
{
    int ret = sem_close(sem_);
    MKI_LOG_IF(ret != 0, ERROR) << "sem_close failed. ret: " << ret;
    ret = sem_unlink(fullName_.c_str());
    MKI_LOG_IF(ret != 0, INFO) << "Already unlink sem";
}
}
