#!/bin/bash
# Copyright (c) 2024 Huawei Technologies Co., Ltd.
# This file is a part of the CANN Open Software.
# Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
if(NOT EXISTS ${OP_LIST_YAML_DIR}/op_list.yaml)
    execute_process(COMMAND python3 ${MKI_PACKAGE_DIR}/scripts/op_list_utils.py -s ${PROJECT_SOURCE_DIR} -d ${OP_LIST_YAML_DIR}
                    ERROR_VARIABLE RESULT_INFO
                    RESULT_VARIABLE RESULT)
    if(NOT RESULT EQUAL 0)
        message(FATAL_ERROR "create op_build.yaml failed, error code: ${RESULT}, error info:\n${RESULT_INFO}\nPlease build testframework first! ")
    endif()
endif()

set(OP_LIST_CMD_PARAMS "'${OP_LIST_YAML_DIR}/op_list.yaml', '${MKI_PACKAGE_DIR}/cmake/op_build.cmake'")
execute_process(COMMAND python3 -c "import op_list_utils\nop_list_utils.build_cmake_options(${OP_LIST_CMD_PARAMS})"
                WORKING_DIRECTORY ${MKI_PACKAGE_DIR}/scripts
                ERROR_VARIABLE RESULT_INFO
                RESULT_VARIABLE RESULT)
if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR "create op_build.cmake failed, error code: ${RESULT}, error info:\n${RESULT_INFO}\nPlease build testframework first! ")
endif()

include(${MKI_PACKAGE_DIR}/cmake/op_build.cmake)
 
file(GLOB_RECURSE CMAKELISTS_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/*/CMakeLists.txt")
string(REPLACE "/CMakeLists.txt" "" CMAKELISTS_PATHS "${CMAKELISTS_PATHS}")
foreach(SUB_DIR ${CMAKELISTS_PATHS})
    message(STATUS "add subdirectory: ${SUB_DIR}")
    add_subdirectory(${SUB_DIR})
endforeach()

set_source_files_properties(${BINARY_SRC_LIST} PROPERTIES GENERATED TRUE)
message("BINARY_SRC_LIST: ${BINARY_SRC_LIST}")
message("BINARY_TARGET_LIST: ${BINARY_TARGET_LIST}")

add_custom_command(
    OUTPUT ${BINARY_SRC_LIST}
    DEPENDS ${BINARY_TARGET_LIST}
    COMMAND python3 ${MKI_PACKAGE_DIR}/scripts/build_util.py --binary_dir ${CMAKE_BINARY_DIR} --op_type "ascendc"
)

add_custom_target(BINARY_SRC_TARGET ALL
    DEPENDS ${BINARY_SRC_LIST}
)

#########################
# 设置target名称
#########################
add_library(atbops SHARED ${BINARY_SRC_LIST} ops.cpp)
add_dependencies(atbops BINARY_SRC_TARGET)
target_link_libraries(atbops PRIVATE ${ops_objects} mki)

add_library(atbops_static STATIC ${BINARY_SRC_LIST})
add_dependencies(atbops_static BINARY_SRC_TARGET)
target_link_libraries(atbops_static PRIVATE ${ops_objects} mki)

install(TARGETS atbops DESTINATION lib)
install(TARGETS atbops_static DESTINATION lib)