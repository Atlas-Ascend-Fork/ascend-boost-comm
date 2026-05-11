#!/bin/bash
# Copyright (c) 2024 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.

cmake_minimum_required(VERSION 3.20)
project("mki")
set(CMAKE_CXX_STANDARD 17)

option(BUILD_TEST_FRAMEWORK "BUILD_TEST_FRAMEWORK" OFF)
option(BUILD_UNIT_TEST "BUILD_UNIT_TEST" OFF)
option(USE_CXX11_ABI "USE_CXX11_ABI" ON)
option(USE_MSDEBUG "USE_MSDEBUG" OFF)
option(USE_MSSANITIZER "USE_MSSANITIZER" OFF)
option(USE_ASCENDC_DUMP "USE_ASCENDC_DUMP" OFF)
message(STATUS "BUILD_TEST_FRAMEWORK:${BUILD_TEST_FRAMEWORK}")
message(STATUS "BUILD_UNIT_TEST:${BUILD_UNIT_TEST}")
message(STATUS "USE_CXX11_ABI:${USE_CXX11_ABI}")
message(STATUS "USE_MSDEBUG:${USE_MSDEBUG}")
message(STATUS "USE_MSSANITIZER:${USE_MSSANITIZER}")
message(STATUS "USE_ASCENDC_DUMP:${USE_ASCENDC_DUMP}")

include(${PROJECT_SOURCE_DIR}/cmake/host_config.cmake)

include_directories(
    ${PROJECT_SOURCE_DIR}/src/include
    $ENV{ASCEND_HOME_PATH}/include
    $ENV{ASCEND_HOME_PATH}/pkg_inc
    $ENV{ASCEND_HOME_PATH}/include/aclnn
)

link_directories(
    $ENV{ASCEND_HOME_PATH}/lib64
)

if(USE_ASCENDC_DUMP)
    add_definitions(-DUSE_ASCENDC_DUMP)
endif()

add_subdirectory(src)

if(BUILD_TEST_FRAMEWORK)
    add_subdirectory(${PROJECT_SOURCE_DIR}/tests/framework)
endif()

if(BUILD_EXAMPLE)
    add_subdirectory(${PROJECT_SOURCE_DIR}/example)
endif()

if(BUILD_UNIT_TEST)
    add_subdirectory(${PROJECT_SOURCE_DIR}/tests/unittest)
endif()

# cmake install stage
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/output/mki" CACHE PATH "..." FORCE)
endif()

# cp files
install(FILES ${PROJECT_SOURCE_DIR}/scripts/set_env.sh DESTINATION ./)
file(GLOB_RECURSE SCRIPTS_FILES "${PROJECT_SOURCE_DIR}/scripts/*.py")
list(APPEND SCRIPTS_FILES
    ${PROJECT_SOURCE_DIR}/scripts/build_env_setup.sh
)
install(FILES ${SCRIPTS_FILES} DESTINATION ./scripts)

# --- ASB 2.1 自动化诊断挂钩 (由 SOP 2.0 强制注入) ---
# 1. 寻找 Python 环境
find_package(Python3 REQUIRED)

# 2. 定义诊断动作：运行 debug_tiling.py，目标指向当前的编译输出目录
add_custom_target(check_tiling ALL
    COMMAND ${Python3_EXECUTABLE} /home/atomgit/asbtoolkit/debug_tiling.py ${CMAKE_BINARY_DIR}
    COMMENT ">>> ASB 正在自动审计算子 Tiling 资产安全性..."
    VERBATIM
)

# 3. 建立依赖：确保编译前必须通过诊断
# 注意：这里的 atb 需要替换为你工程里 project() 定义的名字，通常就是 atb
add_dependencies(atb check_tiling)