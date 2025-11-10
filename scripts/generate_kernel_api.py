#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2025 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
import os
import argparse

# 核函数标志符
KERNEL_FUNCTION_KEY = "extern \"C\" __global__ __aicore__ void"
DUMP_WORKSPACE_CPP = " GM_ADDR dumpWorkspace"
DUMP_WORKSPACE_CCE = " __gm__ uint8_t *__restrict__ dumpWorkspace"
INITDUMP_CONTENT = "    AscendC::InitDump(false, dumpWorkspace, 75 * 1024 * 1024);\n"
INCLUDE_CONTENT = "#include \"kernel_operator.h\""


# 对于CCE算子文件，在开头添加
def add_include_file_in_kernel_file(src_content):
    include_start_idx = src_content.find("#include")
    if src_content.find(INCLUDE_CONTENT) == -1:
        return src_content[:include_start_idx] + INCLUDE_CONTENT + '\n' + src_content[include_start_idx:]
    return src_content


# 替换算子核函数接口，添加一个新的workspace参数用于存放dump数据
def add_param_in_kernel_function_definition(kernel_func_def, file_type):
    new_kernel_func_def = ""
    param_start_idx = kernel_func_def.find('(') + 1
    param_end_idx = kernel_func_def.find(')')
    param_parts = kernel_func_def[param_start_idx:param_end_idx].strip().split(',')
    # 找到函数定义中最后一个逗号，在此逗号后面添加DUMP_WORKSPACE_CCE或者DUMP_WORKSPACE_CPP
    if not param_parts:
        print(f"the kernel function has no param, please check the kernel function, kernel function definiton: {kernel_func_def}")
    else:
        # 这里一个正常的kernel核函数其param个数一定大于等于2(intensor、tiling)
        if file_type == "CCE":
            if "GM_ADDR" in kernel_func_def:
                param_parts.insert(-1, DUMP_WORKSPACE_CPP)
            else:
                param_parts.insert(-1, DUMP_WORKSPACE_CCE)
        else:
            param_parts.insert(-1, DUMP_WORKSPACE_CPP)
        new_kernel_func_def = kernel_func_def[:param_start_idx] + ','.join(param_parts) + kernel_func_def[param_end_idx:]
    return new_kernel_func_def


# 在算子核函数中，添加InitDump函数
def add_initDump_in_kernel_function(kernel_func_content):
    new_kernel_func_content = ""
    new_kernel_func_content_start_idx = kernel_func_content.find('{') + 2
    new_kernel_func_content = kernel_func_content[:new_kernel_func_content_start_idx] + INITDUMP_CONTENT + kernel_func_content[new_kernel_func_content_start_idx:]
    return new_kernel_func_content


# 生成新的算子核函数
def generate_new_kernel_function(kernel_func, file_type):
    # 生成新的核函数定义
    kernel_func_def_end_idx = kernel_func.find('{')
    kernel_func_def = kernel_func[:kernel_func_def_end_idx]
    new_kernel_func_def = add_param_in_kernel_function_definition(kernel_func_def, file_type)
    # 生成新的核函数内容
    kernel_func_content = kernel_func[kernel_func_def_end_idx:]
    new_kernel_func_content = add_initDump_in_kernel_function(kernel_func_content)
    return new_kernel_func_def + new_kernel_func_content


# 修改算子代码
def decorate_kernel_file(kernel_path):
    file_type = "CPP"
    new_src_content = ""
    try:
        with open(kernel_path, 'r', encoding='utf-8') as file:
            src_content = file.read()
        # 判断是否是memset.cc
        file_name = os.path.basename(input_args.srcs)
        if file_name == "memset.cc":
            return src_content
        # 判断算子代码是AscendC还是CCE
        if (kernel_path.endswith(".cce")):
            file_type = "CCE"
            src_content = add_include_file_in_kernel_file(src_content)
        # 获取核函数, 核函数可能有多个
        kernel_func_idxs = []
        content_start_idx = 0
        while True:
            kernel_func_start_idx = src_content.find(KERNEL_FUNCTION_KEY, content_start_idx)
            if kernel_func_start_idx == -1:
                break
            kernel_func_end_idx = src_content.find('}', kernel_func_start_idx) + 1
            kernel_func_idxs.append((kernel_func_start_idx, kernel_func_end_idx))
            content_start_idx = kernel_func_start_idx + 1
        content_start_idx = 0
        for start_idx, end_idx in kernel_func_idxs:
            kernel_func = src_content[start_idx:end_idx]
            new_kernel_func = generate_new_kernel_function(kernel_func, file_type)
            new_src_content += (src_content[content_start_idx:start_idx] + new_kernel_func)
            content_start_idx = end_idx
        new_src_content += src_content[content_start_idx:]

    except Exception as e:
        print(f"decorate cpp kernel function fail, kernel path: {kernel_path}, and error: {str(e)}")
    return new_src_content


# 使用生成的内容替换原先的算子内容，然后将原算子内容储存起来，编译完成之后从新复原原算子内容
def save_to_file(dist_path, new_src_content):
    try:
        if os.path.dirname(dist_path):
            os.makedirs(os.path.dirname(dist_path), exist_ok=True)
        with open(dist_path, 'w', encoding='utf-8') as file:
            file.write(new_src_content)
    except Exception as e:
        print(f"write file fail, write file path: {dist_path}, and error: {str(e)}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--srcs', type=str, required=True)
    parser.add_argument('--dst', type=str, required=True)
    input_args = parser.parse_args()

    new_src_content = ""
    new_src_content = decorate_kernel_file(input_args.srcs)
    save_to_file(input_args.dst, new_src_content)
