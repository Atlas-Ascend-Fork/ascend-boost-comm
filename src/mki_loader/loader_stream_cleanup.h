/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This program is free software; you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for the specific language governing permissions and limitations under the License.
 */
#ifndef MKI_LOADER_LOADER_STREAM_CLEANUP_H
#define MKI_LOADER_LOADER_STREAM_CLEANUP_H

namespace Mki
{
namespace LoaderDetail
{
template <typename Stream, typename Destroy>
int DestroyStreamIfPresent(Stream& stream, int success, Destroy&& destroy)
{
    if (stream == nullptr)
    {
        return success;
    }
    auto destroyStatus = destroy(stream);
    stream = nullptr;
    return destroyStatus;
}

template <typename Stream, typename Destroy>
int CleanupStreamAfterCreateFailure(int createStatus, Stream& stream, int success, Destroy&& destroy)
{
    if (createStatus == success && stream != nullptr)
    {
        return success;
    }
    return DestroyStreamIfPresent(stream, success, static_cast<Destroy&&>(destroy));
}
}  // namespace LoaderDetail
}  // namespace Mki

#endif
