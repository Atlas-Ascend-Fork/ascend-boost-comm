#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# Copyright (c) 2024 Huawei Technologies Co., Ltd.
# This file is a part of the CANN Open Software.
# Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.


import argparse
import json
import os
import re
import stat
import logging

ascend_home_path = os.getenv("ASCEND_HOME_PATH")
if not ascend_home_path:
    logging.error("ASCEND_HOME_PATH does not exist, aicpu kernels compilation failed.")
    exit(1)

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--srcs', nargs='+', type=str, required=True)
    parser.add_argument('--dst', type=str, required=True)
    return parser.parse_args()


def gen_compile_cmd(args, src: str, dst: str, compile_options):
    compile_cmd = [os.path.join(ascend_home_path, 'toolkit', 'toolchain',
                                'hcc', 'bin', 'aarch64-target-linux-gnu-g++')]
    compile_cmd += compile_options
    compile_cmd += ['-I/usr/local/Ascend/ascend-toolkit/latest/opp/built-in/op_impl/aicpu/aicpu_kernel/inc',
                    '-I/usr/local/Ascend/ascend-toolkit/latest/compiler/include',
                    '-I/usr/local/Ascend/ascend-toolkit/latest/include']
    compile_cmd += ['-D_GLIBCXX_USE_CXX11_ABI=1',
                    '-Dasdops_aicpu_kernels_EXPORTS',
                    '-std=gnu++11',
                    '-MD', '-MT', dst,
                    '-MF', dst + '.d',
                    '-o', dst,
                    '-c', src
                   ]
    return compile_cmd

def gen_fatbin_cmd(args, obj_file: list, target_so, compile_options):
    compile_cmd = [os.path.join(ascend_home_path, 'toolkit', 'toolchain',
                                'hcc', 'bin', 'aarch64-target-linux-gnu-g++')]
    compile_cmd += compile_options
    compile_cmd += ['-Wl,-z,relro,-z,now', '-Wl,-z,noexecstack', '-shared',
                    f'-Wl,-soname,{target_so}',
                    '-fvisibility=hidden', '-fvisibility-inlines-hidden']
    compile_cmd += obj_file
    compile_cmd += ['-o', f'{target_so}']
    compile_cmd += [f'-L{ascend_home_path}/lib64',
                    '-L/usr/local/Ascend/driver/lib64/common']
    return compile_cmd


def get_common_options():
    options = ['-O2']
    options += ['-D_FORTIFY_SOURCE=2', '-fstack-protector-all']
    options += ['-fpic', '-fpie', '-fPIC']

    return options


def exe_cmd(cmd):
    if subprocess.run(cmd) != 0:
        logging.error("execute command failed")
        logging.debug("command: %s", cmd)
        return -1
    return 0

def compile_aicpu_kernels(args, output_dir):
    dsts = []
    options = get_common_options()
    compile_cmd = ""
    link_cmd = ""

    intermediate_dir = os.path.join(output_dir, "aicpu_kernels")
    if not os.path.exists(intermediate_dir):
        os.makedirs(intermediate_dir)

    target_so = input_args.dst.split("/")[-1]

    # Compiling
    for src in args.srcs:
        dst = f"{os.path.join(intermediate_dir, src.split("/")[-1])}.o"
        compile_cmd = ' '.join(gen_compile_cmd(args, src, dst, options))
        if(exe_cmd(compile_cmd)) != 0:
            return -1
        dsts.append(dst)

    # Linking
    link_cmd = ' '.join(gen_fatbin_cmd(args, dsts, target_so, options))
    if(exe_cmd(link_cmd)) != 0:
        return -1

    return 0

if __name__ == '__main__':
    input_args = parse_args()
    output_dir = os.path.dirname(input_args.dst)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    res = compile_aicpu_kernels(input_args, output_dir)
    if res != 0:
        logging.error("compile aicpu kernels failed!")
        exit(1)
