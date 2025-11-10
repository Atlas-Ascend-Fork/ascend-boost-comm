#!/bin/bash
# Copyright (c) 2024 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.

include_directories(
    $ENV{THIRD_PARTY_DIR}/nlohmannJson/include
)

execute_process(COMMAND python3 ${CMAKE_CURRENT_LIST_DIR}/op_desc_json_auto.py
                                --namespace ${NAMESPACE}
                                --params_path ${OPS_PROJECT_ROOT_DIR}/${CMAKE_PROJECT_NAME}/include)
add_library(mki_test_autogen SHARED ${CMAKE_CURRENT_LIST_DIR}/op_desc_json.cpp)
target_link_libraries(mki_test_autogen PRIVATE simple_ops)
target_compile_definitions(mki_test_autogen PRIVATE OpSpace=${NAMESPACE})

install(TARGETS mki_test_autogen DESTINATION tests)
