#!/bin/bash
# Copyright (c) 2024 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.

set -e
BUILD_DIR=$(dirname $(readlink -f $0))
CANN_DIR=$(readlink -f $BUILD_DIR/../CANN)
export SCRIPT_DIR=$(cd $(dirname $0); pwd)
CURRENT_DIR=$(pwd)
cd $SCRIPT_DIR
cd ..
export CODE_ROOT=$(pwd)
export CACHE_DIR=$CODE_ROOT/build
export OUTPUT_DIR=$CODE_ROOT/output
export BUILD_CONFIG_DIR=$BUILD_DIR/../configs

export THIRD_PARTY_DIR=$CODE_ROOT/3rdparty
BUILD_TEST_FRAMEWORK=OFF
COMPILE_OPTIONS=""
DEVICE_CODE_PACK_SWITCH=ON
USE_VERBOSE=OFF
ENABLE_COVERAGE=OFF
USE_ASCENDC_DUMP=OFF
USE_CXX11_ABI=""
IS_RELEASE=False
BUILD_OPTION_LIST="testframework release example dev debug unittest clean help"
BUILD_CONFIGURE_LIST=("--output=.*" "--use_cxx11_abi=0" "--use_cxx11_abi=1"
                      "--verbose" "--no_werror" "--coverage" "--namespace=.*" "--msdebug" "--ascendc_dump")

# install cann
function fn_install_cann_and_kernel()
{
    echo "start"
    echo "$(pwd)"
    # CANN package location
    cann_install_path="/home/slave1/Ascend/ascend-toolkit/latest/"
    # Record the start time
    time_before=$(date +%s)
    if [[ -d "$cann_install_path" ]]; then
        export ASCEND_HOME_PATH=${cann_install_path}
        export LD_LIBRARY_PATH=${ASCEND_HOME_PATH}/lib64:${LD_LIBRARY_PATH}
        return 0
    fi
    mkdir -p ${cann_install_path}
    mkdir -p ${cann_install_path}/opp/built-in/op_impl/ai_core/tbe/kernel/
    chmod 750 ${cann_install_path}
    export ASCEND_HOME_PATH=${cann_install_path}

    cd $CANN_DIR
    chmod +x *.run
    set +e
    for run_file in $(ls *.run);do
        if [[ "${run_file}" =~ "opp_kernel" ]];then
            kernel_path=$(echo ${run_file%%-*} | tr A-Z a-z)
            mkdir -p ${kernel_path}/opp_kernel
            chmod -R 750 ${cann_install_path}/opp/built-in/op_impl/ai_core/tbe
            ./${run_file} --tar xmf -C ${kernel_path} &
            ln -s $CANN_DIR/${kernel_path}/opp_kernel ${cann_install_path}/opp/built-in/op_impl/ai_core/tbe/kernel/${kernel_path}
        else
            ./${run_file} --tar xmf -C ${cann_install_path} --exclude='latest_manager/*' &
        fi
    done
    wait
    time_after=$(date +%s)
    echo "并行解压花费时间 $(($time_after - $time_before))秒"
    # link library
    export LD_LIBRARY_PATH=${ASCEND_HOME_PATH}/lib64:${LD_LIBRARY_PATH}
    echo "ASCEND_HOME_PATH is: ${ASCEND_HOME_PATH}"
    echo "LD LIBRARY PATH is: ${LD_LIBRARY_PATH}"
    # compiler
    mkdir ${ASCEND_HOME_PATH}/compiler/tikcpp/tikcfw
    find "${ASCEND_HOME_PATH}/toolkit/$(arch)-linux/ascendc/include/basic_api/" -mindepth 1 -maxdepth 1 -exec ln -s {} "${ASCEND_HOME_PATH}/compiler/tikcpp/tikcfw/" \; 2>/dev/null
    find "${ASCEND_HOME_PATH}/toolkit/$(arch)-linux/ascendc/include/highlevel_api/" -mindepth 1 -maxdepth 1 -exec ln -s {} "${ASCEND_HOME_PATH}/compiler/tikcpp/tikcfw/" \; 2>/dev/null
    ln -s "${ASCEND_HOME_PATH}/compiler/$(arch)-linux/asc/include" "${ASCEND_HOME_PATH}/compiler/tikcpp/tikcfw/"
    ln -s "${ASCEND_HOME_PATH}/compiler/$(arch)-linux/asc/include/adv_api" "${ASCEND_HOME_PATH}/compiler/tikcpp/tikcfw/lib"
    ln -s "${ASCEND_HOME_PATH}/compiler/$(arch)-linux/asc/include/interface" "${ASCEND_HOME_PATH}/compiler/tikcpp/tikcfw/"
    ln -s "${ASCEND_HOME_PATH}/compiler/$(arch)-linux/asc/include/kernel_operator.h" "${ASCEND_HOME_PATH}/compiler/tikcpp/tikcfw/"
    ln -s ${ASCEND_HOME_PATH}/compiler/$(arch)-linux/ascendc/include/highlevel_api/lib/ascendc_api_version.h ${ASCEND_HOME_PATH}/compiler/tikcpp/tikcfw/
    ## include
    rm -rf ${ASCEND_HOME_PATH}/include ${ASCEND_HOME_PATH}/$(arch)-linux/
    mkdir -p ${ASCEND_HOME_PATH}/include ${ASCEND_HOME_PATH}/$(arch)-linux/include
    # mki
    find "${ASCEND_HOME_PATH}/runtime/include" -mindepth 1 -maxdepth 1 -exec ln -s {} "${ASCEND_HOME_PATH}/include" \; 2>/dev/null
    find "${ASCEND_HOME_PATH}/opp/include" -mindepth 1 -maxdepth 1 -exec ln -s {} "${ASCEND_HOME_PATH}/include" \; 2>/dev/null
    find "${ASCEND_HOME_PATH}/compiler/include" -mindepth 1 -maxdepth 1 -exec ln -s {} "${ASCEND_HOME_PATH}/include" \; 2>/dev/null
    find "${ASCEND_HOME_PATH}/hccl/include" -mindepth 1 -maxdepth 1 -exec ln -s {} "${ASCEND_HOME_PATH}/include" \; 2>/dev/null
    ln -s ${ASCEND_HOME_PATH}/opp/built-in/op_impl/ai_core/tbe/op_api/include/aclnnop             ${ASCEND_HOME_PATH}/include/
    ln -s ${ASCEND_HOME_PATH}/compiler/$(arch)-linux/ascendc/                                     ${ASCEND_HOME_PATH}/include/
    ln -s ${ASCEND_HOME_PATH}/compiler/$(arch)-linux/ascendc/include/highlevel_api/kernel_tiling/ ${ASCEND_HOME_PATH}/include/ # kernel_tiling
    ln -s ${ASCEND_HOME_PATH}/compiler/$(arch)-linux/asc/include/tiling/                          ${ASCEND_HOME_PATH}/include/ # tiling

    ln -s ${ASCEND_HOME_PATH}/compiler/include/acl/acl_tdt.h                                      ${ASCEND_HOME_PATH}/runtime/include/acl/
    ln -s ${ASCEND_HOME_PATH}/toolkit/tools/custom_operator_sample/AICPU/Tensorflow/cpukernel/context/inc/cpu_kernel_utils.h ${ASCEND_HOME_PATH}/opp/include/aicpu/
    ln -s ${ASCEND_HOME_PATH}/toolkit/tools/custom_operator_sample/AICPU/Tensorflow/cpukernel/context/inc/cpu_node_def.h ${ASCEND_HOME_PATH}/opp/include/aicpu/
    ln -s ${ASCEND_HOME_PATH}/aoe/include/aoe/external ${ASCEND_HOME_PATH}/runtime/include/aoe/
    rm ${ASCEND_HOME_PATH}/include/experiment
    ln -s ${ASCEND_HOME_PATH}/toolkit/include/experiment/ ${ASCEND_HOME_PATH}/include/
    ln -s ${ASCEND_HOME_PATH}/runtime/include/experiment/register/ ${ASCEND_HOME_PATH}/toolkit/include/experiment/
    rm ${ASCEND_HOME_PATH}/include/ge ${ASCEND_HOME_PATH}/include/graph
    ln -s ${ASCEND_HOME_PATH}/compiler/include/ge/ ${ASCEND_HOME_PATH}/include/
    ln -s ${ASCEND_HOME_PATH}/compiler/include/graph/ ${ASCEND_HOME_PATH}/include/
    ln -s ${ASCEND_HOME_PATH}/compiler/include/proto/caffe.proto ${ASCEND_HOME_PATH}/runtime/include/proto/
    ln -s ${ASCEND_HOME_PATH}/compiler/include/proto/ge_onnx.proto ${ASCEND_HOME_PATH}/runtime/include/proto/
    ln -s ${ASCEND_HOME_PATH}/toolkit/toolkit/include/proto/ge_ir.proto ${ASCEND_HOME_PATH}/runtime/include/proto/
    rm ${ASCEND_HOME_PATH}/include/register
    ln -s ${ASCEND_HOME_PATH}/compiler/include/register/ ${ASCEND_HOME_PATH}/include/
    mkdir -p ${ASCEND_HOME_PATH}/toolchain/
    ln -s  ${ASCEND_HOME_PATH}/toolkit/include/experiment/slog/toolchain/ ${ASCEND_HOME_PATH}/include/
    mkdir -p ${ASCEND_HOME_PATH}/include/version/
    ln -s ${ASCEND_HOME_PATH}/runtime/cann_version.h ${ASCEND_HOME_PATH}/include/version/
    ln -s ${ASCEND_HOME_PATH}/compiler/compiler_version.h ${ASCEND_HOME_PATH}/include/version/
    ln -s ${ASCEND_HOME_PATH}/hccl/hccl_version.h ${ASCEND_HOME_PATH}/include/version/
    ln -s ${ASCEND_HOME_PATH}/opp/opp_version.h ${ASCEND_HOME_PATH}/include/version/
    ln -s ${ASCEND_HOME_PATH}/runtime/runtime_version.h ${ASCEND_HOME_PATH}/include/version/
    ln -s ${ASCEND_HOME_PATH}/toolkit/toolkit_version.h ${ASCEND_HOME_PATH}/include/version/
    # asdops
    ln -s ${ASCEND_HOME_PATH}/hccl/include/hccl             ${ASCEND_HOME_PATH}/$(arch)-linux/include/
    ln -s ${ASCEND_HOME_PATH}/opp/include/aicpu             ${ASCEND_HOME_PATH}/$(arch)-linux/include/
    ln -s ${ASCEND_HOME_PATH}/opp/include/aclnn_kernels     ${ASCEND_HOME_PATH}/$(arch)-linux/include/
    ln -s ${ASCEND_HOME_PATH}/compiler/include/parser       ${ASCEND_HOME_PATH}/$(arch)-linux/include/
    ln -s ${ASCEND_HOME_PATH}/compiler/include/amct         ${ASCEND_HOME_PATH}/$(arch)-linux/include/
    find "${ASCEND_HOME_PATH}/runtime/include" -mindepth 1 -maxdepth 1 -exec ln -s {} "${ASCEND_HOME_PATH}/$(arch)-linux/include/" \; 2>/dev/null
    find "${ASCEND_HOME_PATH}/hccl/include" -mindepth 1 -maxdepth 1 -exec ln -s {} "${ASCEND_HOME_PATH}/$(arch)-linux/include/" \; 2>/dev/null
    find "${ASCEND_HOME_PATH}/opp/include" -mindepth 1 -maxdepth 1 -exec ln -s {} "${ASCEND_HOME_PATH}/$(arch)-linux/include/" \; 2>/dev/null
    rm -rf ${ASCEND_HOME_PATH}/$(arch)-linux/include/experiment
    ln -s ${ASCEND_HOME_PATH}/opp/built-in/op_impl/ai_core/tbe/op_api/include/aclnnop             ${ASCEND_HOME_PATH}/$(arch)-linux/include/
    ln -s ${ASCEND_HOME_PATH}/compiler/$(arch)-linux/ascendc/                                     ${ASCEND_HOME_PATH}/$(arch)-linux/include/
    ln -s ${ASCEND_HOME_PATH}/compiler/include/flow_graph/                                        ${ASCEND_HOME_PATH}/$(arch)-linux/include/
    ln -s ${ASCEND_HOME_PATH}/compiler/$(arch)-linux/ascendc/include/highlevel_api/kernel_tiling/ ${ASCEND_HOME_PATH}/$(arch)-linux/include/ # kernel_tiling
    ln -s ${ASCEND_HOME_PATH}/compiler/$(arch)-linux/asc/include/tiling/                          ${ASCEND_HOME_PATH}/$(arch)-linux/include/ # tiling
    ln -s ${ASCEND_HOME_PATH}/toolkit/include/experiment                                          ${ASCEND_HOME_PATH}/$(arch)-linux/include/
    ln -s ${ASCEND_HOME_PATH}/toolkit/include/experiment/slog/toolchain/                          ${ASCEND_HOME_PATH}/$(arch)-linux/include/experiment/
    # atb
    mkdir -p ${ASCEND_HOME_PATH}/$(arch)-linux/ascendc/include/
    ln -s ${ASCEND_HOME_PATH}/compiler/$(arch)-linux/ascendc/include/highlevel_api/               ${ASCEND_HOME_PATH}/$(arch)-linux/ascendc/include/ # platform_ascendc.h
    ln -s ${ASCEND_HOME_PATH}/compiler/include/acl/acl_op_compiler.h                              ${ASCEND_HOME_PATH}/$(arch)-linux/include/acl/ # acl_op_compiler.h
    cp -rf /usr1/CANN/mstx ${ASCEND_HOME_PATH}/tools/
    cp -rf /usr1/CANN/mstx/include/mstx ${ASCEND_HOME_PATH}/include/
    ## lib64
    # mki
    rm -rf ${ASCEND_HOME_PATH}/lib64
    mkdir -p ${ASCEND_HOME_PATH}/lib64
    find "${ASCEND_HOME_PATH}/runtime/lib64"  -mindepth 1 -maxdepth 1 -exec ln -s {} "${ASCEND_HOME_PATH}/lib64" \; 2>/dev/null
    find "${ASCEND_HOME_PATH}/compiler/lib64" -mindepth 1 -maxdepth 1 -exec ln -s {} "${ASCEND_HOME_PATH}/lib64" \; 2>/dev/null
    find "${ASCEND_HOME_PATH}/aoe/lib64"      -mindepth 1 -maxdepth 1 -exec ln -s {} "${ASCEND_HOME_PATH}/lib64" \; 2>/dev/null
    find "${ASCEND_HOME_PATH}/hccl/lib64"     -mindepth 1 -maxdepth 1 -exec ln -s {} "${ASCEND_HOME_PATH}/lib64" \; 2>/dev/null
    find "${ASCEND_HOME_PATH}/toolkit/lib64"  -mindepth 1 -maxdepth 1 -exec ln -s {} "${ASCEND_HOME_PATH}/lib64" \; 2>/dev/null
    set -e
}

function fn_build_googletest()
{
    GTEST_DIR=$THIRD_PARTY_DIR/googletest
    if [ -d "$GTEST_DIR" ]; then
        return $?
    fi
    [[ ! -d "$THIRD_PARTY_DIR" ]] && mkdir -p $THIRD_PARTY_DIR
    cd $THIRD_PARTY_DIR
    wget --no-check-certificate https://github.com/google/googletest/archive/refs/tags/v1.13.0.tar.gz
    tar -xf v1.13.0.tar.gz
    mv googletest-1.13.0 googletest
    rm v1.13.0.tar.gz
}

function fn_build_nlohmann_json()
{
    if [ -d "$THIRD_PARTY_DIR/nlohmannJson" ]; then
        return $?
    fi
    cd $CACHE_DIR
    rm -rf nlohmann
    mkdir nlohmann
    cd nlohmann
    git clone -b v3.11.3 --depth 1 https://gitcode.com/sound/nlohmann-json.git
    mkdir -p $THIRD_PARTY_DIR/nlohmannJson
    mv nlohmann-json/include $THIRD_PARTY_DIR/nlohmannJson
    cd $CACHE_DIR
    rm -rf nlohmann
}

function fn_build_dependency()
{
    ARCH=$(uname -m)
    CCEC_COMPILER_DIR=$THIRD_PARTY_DIR/compiler/ccec_compiler
    TIKCPP_DIR=$THIRD_PARTY_DIR/compiler/tikcpp

    [[ -d "$THIRD_PARTY_DIR/compiler" ]] && rm -rf $THIRD_PARTY_DIR/compiler
    mkdir -p $THIRD_PARTY_DIR/compiler
    ln -s $ASCEND_HOME_PATH/compiler/ccec_compiler $CCEC_COMPILER_DIR
    ln -s $ASCEND_HOME_PATH/compiler/tikcpp $TIKCPP_DIR
}

function fn_build_release_3rdparty()
{
    fn_build_dependency
    fn_build_nlohmann_json
}

function fn_init_pytorch_env()
{
    export PYTHON_INCLUDE_PATH="$(python3 -c 'from sysconfig import get_paths; print(get_paths()["include"])')"
    export PYTHON_LIB_PATH="$(python3 -c 'from sysconfig import get_paths; print(get_paths()["include"])')"
    export PYTORCH_INSTALL_PATH="$(python3 -c 'import importlib.util; SPEC=importlib.util.find_spec("torch"); \
                                                   print(SPEC.submodule_search_locations[0])')"
    export PYTORCH_NPU_INSTALL_PATH="$(python3 -c 'import importlib.util; SPEC=importlib.util.find_spec("torch_npu"); \
                                                   print(SPEC.submodule_search_locations[0])')"
    echo "PYTHON_INCLUDE_PATH=$PYTHON_INCLUDE_PATH"
    echo "PYTHON_LIB_PATH=$PYTHON_LIB_PATH"
    echo "PYTORCH_INSTALL_PATH=$PYTORCH_INSTALL_PATH"
    echo "PYTORCH_NPU_INSTALL_PATH=$PYTORCH_NPU_INSTALL_PATH"
}

function fn_init_use_cxx11_abi()
{
    res=$(python3 -c "import torch" &> /dev/null || echo "torch_not_exist")
    if [ "$res" == "torch_not_exist" ]; then
        echo "Warning: Torch is not installed!"
        [[ "$USE_CXX11_ABI" == "" ]] && USE_CXX11_ABI=ON
        echo "USE_CXX11_ABI=$USE_CXX11_ABI"
        return 0
    fi

    if [ "$USE_CXX11_ABI" == "" ]; then
        if [ "$(python3 -c 'import torch; print(torch.compiled_with_cxx11_abi())')" == "True" ]; then
            USE_CXX11_ABI=ON
        else
            USE_CXX11_ABI=OFF
        fi
    fi
    echo "USE_CXX11_ABI=$USE_CXX11_ABI"
}

function fn_compile_and_install()
{
    cmake $1 $2
    if [ "$USE_VERBOSE" == "ON" ];then
        VERBOSE=1 make -j
    else
        make -j
    fi
    make install
}

function fn_config_json_copy()
{
    CONFIG_JSON_DIR=$CODE_ROOT/configs
    CONFIG_JSON_DEST=$OUTPUT_DIR/mki/configs
    if [ -d "$CONFIG_JSON_DEST" ];then
        rm -rf $CONFIG_JSON_DEST
    fi
    mkdir -p $CONFIG_JSON_DEST
    cp $CONFIG_JSON_DIR/build_config.json $CONFIG_JSON_DEST/build_config.json
}

function fn_cmake_configs_copy()
{
    CMAKE_CONFIGS_DIR=$CODE_ROOT/cmake
    CMAKE_CONFIGS_OUT_DIR=$OUTPUT_DIR/mki/cmake
    if [ -d "$CMAKE_CONFIGS_OUT_DIR" ];then
        rm -rf $CMAKE_CONFIGS_OUT_DIR
    fi
    mkdir -p $CMAKE_CONFIGS_OUT_DIR
    for cfg_name in $(ls $CMAKE_CONFIGS_DIR);do
        cp $CMAKE_CONFIGS_DIR/$cfg_name $CMAKE_CONFIGS_OUT_DIR/$cfg_name
    done
}

function fn_build()
{
    local_ascend_path="/usr/local/Ascend/ascend-toolkit"
    if [ "$IS_RELEASE" == "True" ];then
        if [ ! -d "$local_ascend_path" ];then
            fn_install_cann_and_kernel
        else
            . /usr/local/Ascend/ascend-toolkit/set_env.sh
        fi
    fi
    if [ -z "$ASCEND_HOME_PATH" ];then
        echo "env ASCEND_HOME_PATH not exists, build fail"
        exit 1
    fi

    [ -n "$CACHE_DIR" ] && rm -rf $CACHE_DIR
    [[ ! -d "$CACHE_DIR" ]] && mkdir $CACHE_DIR
    [[ ! -d "$OUTPUT_DIR" ]] && mkdir -p $OUTPUT_DIR
    [[ ! -d "$THIRD_PARTY_DIR" ]] && mkdir -p $THIRD_PARTY_DIR

    fn_build_release_3rdparty

    cd $CACHE_DIR
    COMPILE_OPTIONS="${COMPILE_OPTIONS} -DCMAKE_INSTALL_PREFIX=$OUTPUT_DIR/mki"
    if command -v ccache &> /dev/null; then
        COMPILE_OPTIONS="${COMPILE_OPTIONS} -DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
    fi
    echo "COMPILE_OPTIONS:$COMPILE_OPTIONS"

    echo "** Mki framework build and install!"
    fn_compile_and_install "$CODE_ROOT" "$COMPILE_OPTIONS"

    fn_config_json_copy
    fn_cmake_configs_copy
}

function fn_make_tar_package()
{
    cd $OUTPUT_DIR
    tar -czf mki.tar.gz mki
    rm -rf mki
}

function fn_main()
{
    if [[ "$BUILD_OPTION_LIST" =~ "$1" ]];then
        if [[ -z "$1" ]];then
            arg1="dev"
        else
            arg1=$1
            shift
        fi
    else
        cfg_flag=0
        for item in ${BUILD_CONFIGURE_LIST[*]};do
            if [[ "$1" =~ "$item" ]];then
                cfg_flag=1
                break 1
            fi
        done
        if [[ "$cfg_flag" == 1 ]];then
            arg1="dev"
        else
            echo "argument $1 is unknown, please type 'build.sh help' for more imformation"
            exit 1
        fi
    fi

    until [[ -z "$1" ]]
    do {
        arg2=$1
        case "${arg2}" in
        --output=*)
            arg2=${arg2#*=}
            if [ -z "$arg2" ];then
                echo "the output directory is not set. This should be set like --output=<outputDir>"
            else
                cd $CURRENT_DIR
                if [ ! -d "$arg2" ];then
                    mkdir -p $arg2
                fi
                export OUTPUT_DIR=$(cd $arg2; pwd)
            fi
            ;;
        "--use_cxx11_abi=1")
            USE_CXX11_ABI=ON
            ;;
        "--use_cxx11_abi=0")
            USE_CXX11_ABI=OFF
            ;;
        "--verbose")
            USE_VERBOSE=ON
            ;;
        "--coverage")
            ENABLE_COVERAGE=ON
            ;;
        --namespace=*)
            ;;
        "--no_werror")
            COMPILE_OPTIONS="${COMPILE_OPTIONS} -DNO_WERROR=ON"
            ;;
        "--msdebug")
            # In msdebug mode, ATB uses the same kernel configuration for 910c and 910b.
            # Map 910c to 910b to keep the CMake SOC-matching logic consistent.
            if npu-smi info -t board -i 0 -c 0 | grep -qE 'NPU Name\s*:\s*(9392|9382)'; then
                CHIP_TYPE='ascend910b'
            else
                CHIP_TYPE=$(npu-smi info -m | grep -oE 'Ascend\s*\S+' | head -n 1 | tr -d ' ' | tr '[:upper:]' '[:lower:]')
            fi
            COMPILE_OPTIONS="${COMPILE_OPTIONS} -DUSE_MSDEBUG=ON -DCHIP_TYPE=${CHIP_TYPE}"
            ;;
        "--ascendc_dump")
            USE_ASCENDC_DUMP=ON
            COMPILE_OPTIONS="${COMPILE_OPTIONS} -DUSE_ASCENDC_DUMP=ON"
            ;;
        esac
        shift
    }
    done

    fn_init_use_cxx11_abi
    COMPILE_OPTIONS="${COMPILE_OPTIONS} -DUSE_CXX11_ABI=$USE_CXX11_ABI -DENABLE_COVERAGE=$ENABLE_COVERAGE"
    case "${arg1}" in
        "testframework")
            COMPILE_OPTIONS="${COMPILE_OPTIONS} -DCMAKE_BUILD_TYPE=Release -DBUILD_TEST_FRAMEWORK=ON"
            fn_init_pytorch_env
            fn_build
            ;;
        "release")
            IS_RELEASE=True
            COMPILE_OPTIONS="${COMPILE_OPTIONS} -DCMAKE_BUILD_TYPE=Release"
            fn_build
            fn_make_tar_package
            ;;
        "example")
            COMPILE_OPTIONS="${COMPILE_OPTIONS} -DBUILD_EXAMPLE=ON"
            fn_init_pytorch_env
            fn_build
            ;;
        "dev")
            COMPILE_OPTIONS="${COMPILE_OPTIONS} -DCMAKE_BUILD_TYPE=Release"
            fn_build
            ;;
        "debug")
            COMPILE_OPTIONS="${COMPILE_OPTIONS} -DCMAKE_BUILD_TYPE=Debug"
            fn_build
            ;;
        "unittest")
            COMPILE_OPTIONS="${COMPILE_OPTIONS} -DCMAKE_BUILD_TYPE=Debug -DBUILD_UNIT_TEST=ON"
            fn_build_googletest
            fn_build
            ;;
        "clean")
            [[ -d "$CACHE_DIR" ]] && rm -rf $CACHE_DIR
            [[ -d "$OUTPUT_DIR" ]] && rm -rf $OUTPUT_DIR
            [[ -d "$THIRD_PARTY_DIR" ]] && rm -rf $THIRD_PARTY_DIR
            echo "clear all build history."
            ;;
        *)
            echo "build.sh testframework|release|example|dev|debug|unittest|clean"\
            "--output=<dir>|--force_clean|--use_cxx11_abi=0|--use_cxx11_abi=1"\
            "|--no_werror|--verbose|--coverage|--namespace=<namespace>|--msdebug|--ascendc_dump"
            ;;
    esac
}

fn_main "$@"
