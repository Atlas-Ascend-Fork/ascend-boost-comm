#!/bin/bash
# Copyright (c) 2024 Huawei Technologies Co., Ltd.
# This file is a part of the CANN Open Software.
# Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/../autogen
    ${PROJECT_SOURCE_DIR}/3rdparty/nlohmannJson/include
    $ENV{PYTHON_INCLUDE_PATH}
    $ENV{PYTORCH_INSTALL_PATH}/include
    $ENV{PYTORCH_INSTALL_PATH}/include/torch/csrc/api/include
    $ENV{PYTORCH_NPU_INSTALL_PATH}/include
)

link_directories(
    $ENV{PYTHON_LIB_PATH}
    $ENV{PYTORCH_INSTALL_PATH}/lib
    $ENV{PYTORCH_NPU_INSTALL_PATH}/lib
)

# Pytorch testframework
## json->param lib
if (NOT BUILD_EXAMPLE)
    add_library(mki_test_autogen SHARED ${CMAKE_CURRENT_LIST_DIR}/../autogen/op_desc_json_ph.cpp)
    if(NOT EXISTS ${CMAKE_INSTALL_PREFIX}/tests/libmki_test_autogen.so)
        MESSAGE(STATUS "libmki_test_autogen.so does NOT exist. Generate placeholder.")
        install(TARGETS mki_test_autogen DESTINATION tests)
    endif()
endif()
## main framework
file(GLOB_RECURSE TORCH_EXTENSION_SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/torch_extension/mki_torch.cpp")
add_library(mki_torch SHARED ${TORCH_EXTENSION_SOURCE_FILES})
target_link_libraries(mki_torch PUBLIC mki_test_autogen mki torch torch_cpu c10 torch_python torch_npu)
target_compile_options(mki_torch PRIVATE
    -Wno-unused-function
    -Wno-attributes
    -Wno-maybe-uninitialized
)
install(TARGETS mki_torch DESTINATION tests)

# cp header files
install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../autogen/
    DESTINATION tests/autogen
    FILES_MATCHING
    PATTERN "*.txt"
    PATTERN "*.py"
    PATTERN "*.h"
)

install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../pythontest/
    DESTINATION tests/pythontest/
    FILES_MATCHING PATTERN "*.py"
)
