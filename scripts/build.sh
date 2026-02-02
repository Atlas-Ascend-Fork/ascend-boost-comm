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
    cann_install_path="/home/slave1/Ascend/ascend-toolkit/"
    # Record the start time
    time_before=$(date +%s)
    if [[ -d "$cann_install_path" ]]; then
        export ASCEND_HOME_PATH=${cann_install_path}/latest
        export LD_LIBRARY_PATH=${ASCEND_HOME_PATH}/lib64:${LD_LIBRARY_PATH}
        return 0
    fi
    mkdir -p ${cann_install_path}

    cd $CANN_DIR
    chmod +x *.run

    TOOLKIT_PKG_LIST=(cann-ge-executor_*.run cann-ge-compiler_*.run cann-npu-runtime_*.run cann-asc-devkit_*.run cann-metadef_*.run cann-opbase_*.run cann-oam-tools_*.run cann-asc-tools_*.run cann-aoe_*.run cann-ncs_*.run cann-graph-autofusion_*.run cann-bisheng-compiler_*.run cann-tbe-tik_*.run cann-dflow-executor_*.run cann-acl-extend_*.run cann-hcomm_*.run Ascend-mindstudio-toolkit_*.run Ascend-BiSheng-toolkit_*.run Ascend-pyACL_*.run Ascend-test-ops*.run cann-hccl_*.run)

    for ((i = 0; i < ${#TOOLKIT_PKG_LIST[@]}; i++)); do
        if ls ${TOOLKIT_PKG_LIST[i]} &> /dev/null; then
            echo "Start to install: ${TOOLKIT_PKG_LIST[i]}"
            ./${TOOLKIT_PKG_LIST[i]} --full --quiet --nox11 --install-path=${cann_install_path}
            chmod -R 777 ${cann_install_path}
        else
            echo "File ${TOOLKIT_PKG_LIST[i]} does not exist!"
        fi
    done
    wait
    echo "INSTALL CANN TOOLKIT finished"

    OPS_PKG_LIST=(cann-910b-ops-legacy_*.run cann-hixl_*.run cann-dvpp_*.run cann-910b-ops-math_*.run cann-910b-ops-nn_*.run cann-910b-ops-cv_*.run cann-910b-ops-transformer_*.run)
 
    for ((i = 0; i < ${#OPS_PKG_LIST[@]}; i++)); do
        if ls ${OPS_PKG_LIST[i]} &> /dev/null; then
            echo "Start to install: ${OPS_PKG_LIST[i]}"
            set +e
            ./${OPS_PKG_LIST[i]} --full --quiet --nox11 --install-path=${cann_install_path}
            chmod -R 777 ${cann_install_path}
            set -e
        else
            echo "File ${OPS_PKG_LIST[i]} does not exist!"
        fi
    done
    set +e
    ln -s ${cann_install_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend910b/ops_legacy/*  ${cann_install_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend910b/
    ln -s ${cann_install_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend910b/ops_math/*  ${cann_install_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend910b/
    ln -s ${cann_install_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend910b/ops_nn/*  ${cann_install_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend910b/
    ln -s ${cann_install_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend910b/ops_transformer/*  ${cann_install_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend910b/
    set -e

    ln -s ${cann_install_path}/cann ${cann_install_path}/latest
    chmod -R 777 ${cann_install_path}
    mkdir -p ${cann_install_path}/latest/opp/built-in/op_impl/ai_core/tbe/kernel/
    chmod -R 777 ${cann_install_path}

    SOC_ARCH_LIST=(310p 910 310b)
    for ((i = 0; i < ${#SOC_ARCH_LIST[@]}; i++)); do
        soc_arch=${SOC_ARCH_LIST[i]}
        SUB_OPS_PKG_LIST=(cann-${soc_arch}-ops-legacy_*.run cann-${soc_arch}-ops-math_*.run cann-${soc_arch}-ops-nn_*.run cann-${soc_arch}-ops-cv_*.run cann-${soc_arch}-ops-transformer_*.run)
        tmp_kernel_path=/home/slave1/Ascend/ascend${soc_arch}
        mkdir -p ${tmp_kernel_path}
        for ((j = 0; j < ${#SUB_OPS_PKG_LIST[@]}; j++)); do
            if ls ${SUB_OPS_PKG_LIST[j]} &> /dev/null; then
                echo "Start to install: ${SUB_OPS_PKG_LIST[j]}"
                set +e
                ./${SUB_OPS_PKG_LIST[j]} --full --quiet --nox11 --install-path=${tmp_kernel_path}
                chmod -R 777 ${tmp_kernel_path}
                set -e
            else
                echo "File ${SUB_OPS_PKG_LIST[j]} does not exist!"
            fi
        done
        set +e
        ln -s ${tmp_kernel_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend${soc_arch}/ops_legacy/*  ${tmp_kernel_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend${soc_arch}/
        ln -s ${tmp_kernel_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend${soc_arch}/ops_math/*  ${tmp_kernel_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend${soc_arch}/
        ln -s ${tmp_kernel_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend${soc_arch}/ops_nn/*  ${tmp_kernel_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend${soc_arch}/
        ln -s ${tmp_kernel_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend${soc_arch}/ops_transformer/*  ${tmp_kernel_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend${soc_arch}/
        ln -s ${tmp_kernel_path}/cann/opp/built-in/op_impl/ai_core/tbe/kernel/ascend${soc_arch} ${cann_install_path}/latest/opp/built-in/op_impl/ai_core/tbe/kernel/
        if ! [ -f "${cann_install_path}/latest/lib64/libopapi.so" ]; then
            ln -s ${tmp_kernel_path}/cann/opp/built-in/op_impl/ai_core/tbe/op_api/lib ${cann_install_path}/latest/opp/built-in/op_impl/ai_core/tbe/op_api/
            ln -s ${tmp_kernel_path}/cann/opp/built-in/op_impl/ai_core/tbe/op_api/lib/linux/$(arch)/* ${cann_install_path}/latest/lib64/
            ln -s ${tmp_kernel_path}/cann/opp/built-in/op_impl/ai_core/tbe/op_api/include/aclnnop/* ${cann_install_path}/latest/opp/built-in/op_impl/ai_core/tbe/op_api/include/aclnnop/
            ln -s ${tmp_kernel_path}/cann/opp/built-in/op_impl/ai_core/tbe/op_api/include/aclnnop/level2/* ${cann_install_path}/latest/opp/built-in/op_impl/ai_core/tbe/op_api/include/aclnnop/level2/
        fi
        set -e
    done
    wait
    echo "INSTALL CANN OPS finished"

    set +e
    source ${cann_install_path}/latest/$(arch)-linux/bin/setenv.bash
    export ASCEND_HOME_PATH=${cann_install_path}/latest
    chmod -R 777 ${ASCEND_HOME_PATH}
    mkdir -p ${ASCEND_HOME_PATH}/include/experiment/msprof
    mkdir -p ${ASCEND_HOME_PATH}/compiler/
    ln -s "${ASCEND_HOME_PATH}/$(arch)-linux/tikcpp/" "${ASCEND_HOME_PATH}/compiler/"
    ln -s "${ASCEND_HOME_PATH}/tools/ccec_compiler/" "${ASCEND_HOME_PATH}/compiler/"
    ln -s "${ASCEND_HOME_PATH}/$(arch)-linux/pkg_inc/runtime" "${ASCEND_HOME_PATH}/include/experiment/"
    ln -s "${ASCEND_HOME_PATH}/$(arch)-linux/pkg_inc/toolchain" "${ASCEND_HOME_PATH}/include/"
    ln -s "${ASCEND_HOME_PATH}/$(arch)-linux/pkg_inc/base/"* "${ASCEND_HOME_PATH}/include/base/"
    ln -s "${ASCEND_HOME_PATH}/$(arch)-linux/pkg_inc/base/"* "${ASCEND_HOME_PATH}/include/toolchain/"
    ln -s "${ASCEND_HOME_PATH}/$(arch)-linux/pkg_inc/profiling" "${ASCEND_HOME_PATH}/include/experiment/msprof/toolchain"
    set -e

    # atb
    cp -rf ${CODE_ROOT}/../mstx ${ASCEND_HOME_PATH}/tools/
    cp -rf ${CODE_ROOT}/../mstx/include/mstx ${ASCEND_HOME_PATH}/include/
}

function fn_build_googletest()
{
    GTEST_DIR=$THIRD_PARTY_DIR/googletest
    if [ -d "$GTEST_DIR" ]; then
        return $?
    fi
    [[ ! -d "$THIRD_PARTY_DIR" ]] && mkdir -p $THIRD_PARTY_DIR
    cd $THIRD_PARTY_DIR
    wget --no-check-certificate https://gitcode.com/cann-src-third-party/googletest/releases/download/v1.14.0/googletest-1.14.0.tar.gz \
        -O googletest-1.14.0.tar.gz
    tar -xf googletest-1.14.0.tar.gz
    mv googletest-1.14.0 googletest
    rm googletest-1.14.0.tar.gz
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
    if [ -f "$ASCEND_HOME_PATH/compiler/ccec_compiler/bin/ccec" ]; then
        ln -s $ASCEND_HOME_PATH/compiler/ccec_compiler $CCEC_COMPILER_DIR
        ln -s $ASCEND_HOME_PATH/compiler/tikcpp $TIKCPP_DIR
    else
        ln -s $ASCEND_HOME_PATH/tools/ccec_compiler $CCEC_COMPILER_DIR
        ln -s $ASCEND_HOME_PATH/tools/tikcpp $TIKCPP_DIR
    fi
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
