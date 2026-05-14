# # include(${PROJECT_SOURCE_DIR}/cmake/kernel_config.cmake)

set(OPS_PROJECT_ROOT_DIR "$ENV{CODE_ROOT}")
set(MKI_SCRIPT_DIR "$ENV{SCRIPT_DIR}")
set(OPS_THIRD_PARTY_DIR "$ENV{THIRD_PARTY_DIR}")

include_directories(
    ${PROJECT_SOURCE_DIR}/3rdparty/securec/include
)

link_directories(
    $ENV{CODE_ROOT}/lib
    ${ASCEND_DRIVER_DIR}/lib64/common
    $ENV{ASCEND_HOME_PATH}/lib64
)

set(BUILD_MemsetOperation_MemsetKernel_ascend310p ON)
set(BUILD_MemsetOperation_MemsetKernel_ascend910b ON)

set(op_name MemsetOperation)

# add_kernel(memory_set ascend310p vector memset.cc MemsetKernel)

# add_kernel(memory_set ascend910b vector memset.cc MemsetKernel)
