/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#ifndef MKI_RUN_INFO_H
#define MKI_RUN_INFO_H

#include <cstdint>
#include <string>

namespace Mki {
class RunInfo {
public:
    RunInfo() = default;
    ~RunInfo();
    RunInfo(const RunInfo &) = delete;
    RunInfo &operator=(const RunInfo &other) = delete;

public:
    void Reset();

    void SetStream(void *stream);
    void *GetStream() const;

    void SetScratchDeviceAddr(uint8_t *addr);
    uint8_t *GetScratchDeviceAddr() const;

    void SetTilingDeviceAddr(uint8_t *addr);
    uint8_t *GetTilingDeviceAddr() const;

    std::string ToString() const;

    void Copy(const RunInfo &runInfo);

private:
    // used by User
    void *stream_ = nullptr;
    uint8_t *scratchAddr_ = nullptr;
    uint8_t *tilingDeviceAddr_ = nullptr;
};
} // namespace Mki

#endif
