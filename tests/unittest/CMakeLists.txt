#!/bin/bash
# Copyright (c) 2024 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/api TEST_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/operationir TEST_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/utils TEST_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/common TEST_SRC)
add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/googletest googletest EXCLUDE_FROM_ALL)
add_executable(mki_unittest ${TEST_SRC})
target_include_directories(mki_unittest PRIVATE ${PROJECT_SOURCE_DIR}/3rdparty/googletest/include)
target_include_directories(mki_unittest PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_directories(mki_unittest PRIVATE ${PROJECT_SOURCE_DIR}/3rdparty/googletest/lib)
target_link_libraries(mki_unittest PRIVATE mki gtest gtest_main)
target_compile_options(mki_unittest PRIVATE
    -Wno-sign-compare
    -Wno-narrowing
)

install(TARGETS mki_unittest DESTINATION bin)
