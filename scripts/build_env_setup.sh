#!/bin/bash
# Copyright (c) 2024 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.

set -e
function fn_install_lcov()
{
    wget --no-check-certificate https://github.com/linux-test-project/lcov/archive/refs/tags/v1.16.tar.gz
    tar -xvf v1.16.tar.gz
    cd lcov-1.16
    make -j
    make PREFIX=/usr/local/lcov install
}

function fn_install_doxygen()
{
    wget --no-check-certificate https://github.com/doxygen/doxygen/archive/refs/heads/master.tar.gz
    tar -xvf master.tar.gz

    apt-get install flex
    apt-get install bison

    cd doxygen-master
    mkdir build
    cd build
    cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/usr/local/doxygen ../
    make -j
    make install
}

function fn_main()
{
    fn_install_lcov
    fn_install_doxygen
}

fn_main "$@"
