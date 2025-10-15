#!/bin/bash
# Copyright (c) 2024 Huawei Technologies Co., Ltd.
# This file is a part of the CANN Open Software.
# Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
cmake_minimum_required(VERSION 3.8)
project("atbops")
set(CMAKE_CXX_STANDARD 17)

option(BUILD_TEST_FRAMEWORK "BUILD_TEST_FRAMEWORK" OFF)
option(USE_UNIT_TEST "USE_UNIT_TEST" OFF)
option(USE_PYTHON_TEST "USE_PYTHON_TEST" OFF)
option(USE_CXX11_ABI "USE_CXX11_ABI" ON)
if(USE_UNIT_TEST OR USE_PYTHON_TEST)
    set(BUILD_TEST_FRAMEWORK ON)
endif()
message(STATUS "BUILD_TEST_FRAMEWORK:${BUILD_TEST_FRAMEWORK}")
message(STATUS "USE_UNIT_TEST:${USE_UNIT_TEST}")
message(STATUS "USE_PYTHON_TEST:${USE_PYTHON_TEST}")
message(STATUS "USE_CXX11_ABI:${USE_CXX11_ABI}")

set(NAMESPACE "AtbOps")
set(ASCEND_DRIVER_DIR /usr/local/Ascend/driver)
set(OPS_PROJECT_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(OPS_THIRD_PARTY_DIR "${CMAKE_CURRENT_LIST_DIR}/../3rdparty")
set(MKI_PACKAGE_DIR "$ENV{CODE_ROOT}/output/mki")
set(OP_LIST_YAML_DIR "${CMAKE_CURRENT_LIST_DIR}/../configs")
set(MKI_SCRIPT_DIR "${MKI_PACKAGE_DIR}/scripts")
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX
        "${CMAKE_SOURCE_DIR}/../output/asdops" CACHE PATH "..." FORCE)
endif()

include($ENV{CODE_ROOT}/cmake/host_config.cmake)
include($ENV{CODE_ROOT}/cmake/kernel_config.cmake)

include_directories(
    ${MKI_PACKAGE_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${OPS_THIRD_PARTY_DIR}/metadef/inc/external
    ${OPS_THIRD_PARTY_DIR}/nlohmannJson/include
    ${ASCEND_DRIVER_DIR}/kernel/libc_sec/include
    $ENV{ASCEND_HOME_PATH}/include
)

link_directories(
    ${MKI_PACKAGE_DIR}/lib
    ${ASCEND_DRIVER_DIR}/lib64/common
    $ENV{ASCEND_HOME_PATH}/lib64
    $ENV{PYTHON_LIB_PATH}
    $ENV{PYTORCH_INSTALL_PATH}/lib
    $ENV{PYTORCH_NPU_INSTALL_PATH}/lib
)

add_subdirectory(ops)
add_subdirectory(tests)

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include DESTINATION .)
install(DIRECTORY
    ${MKI_PACKAGE_DIR}/include DESTINATION .)
install(DIRECTORY
    ${MKI_PACKAGE_DIR}/lib DESTINATION .)