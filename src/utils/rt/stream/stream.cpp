/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "mki/utils/rt/stream/stream.h"
#include "mki/utils/rt/backend/backend_factory.h"

namespace Mki {
int MkiRtStreamCreate(MkiRtStream *stream, int32_t priority)
{
    return BackendFactory::GetBackend()->StreamCreate(stream, priority);
}

int MkiRtStreamDestroy(MkiRtStream stream) { return BackendFactory::GetBackend()->StreamDestroy(stream); }

int MkiRtStreamSynchronize(MkiRtStream stream)
{
    return BackendFactory::GetBackend()->StreamSynchronize(stream);
}

int MkiRtStreamGetId(MkiRtStream stream, int32_t *streamId)
{
    return BackendFactory::GetBackend()->StreamGetId(stream, streamId);
}
}
