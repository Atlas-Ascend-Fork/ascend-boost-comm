#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# Copyright (c) 2024 Huawei Technologies Co., Ltd.
# This file is a part of the CANN Open Software.
# Licensed under CANN Open Software License Agreement Version 1.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
import yaml
import os
import re
import logging
import argparse
import configparser
import build_util

pattern_operation = r"add_operation\(.+?\)"
pattern_kernel = r"add_kernel\(.+?\)"


def parse_operation(add_operation_text):
    param_list = add_operation_text.split('(')[1].strip(')').split(' ')
    return param_list[0]


def parse_kernel(add_kernel_text):
    param_list = add_kernel_text.split('(')[1].strip(')').split(' ')
    param_list = [x for x in param_list if x]
    return param_list[4], param_list[1]


def build_op_list(ops_src_root_dir, dst_yaml_path):
    logging.basicConfig(level=logging.INFO,
                        format='%(filename)s [line:%(lineno)d] - %(levelname)s: %(message)s')
    if os.path.exists(dst_yaml_path):
        os.remove(dst_yaml_path)
    op_kernel_list = {}
    for path, _, file_list in os.walk(ops_src_root_dir):
        for file in file_list:
            if file == 'CMakeLists.txt':
                file_path = os.path.join(path, file)
                with open (file_path) as fd:
                    context = fd.read().replace('\n', ' ')
                    match_operation = re.search(pattern_operation, context)
                    if not match_operation:
                        break
                    add_operation_text = match_operation.group()
                    match_kernel_text_list = re.findall(pattern_kernel, context)
                    op_name = parse_operation(add_operation_text)
                    if op_name in op_kernel_list:
                        raise ValueError(f"operation {op_name} is duplicate")
                    kernel_list = {}
                    for add_kernel_text in match_kernel_text_list:
                        kernel_name, soc = parse_kernel(add_kernel_text)
                        if kernel_name in kernel_list:
                            if soc in kernel_list[kernel_name]:
                                raise ValueError(f"{op_name}: kernel {kernel_name}-{soc} is duplicate")
                            kernel_list[kernel_name][soc] = True
                        else:
                            kernel_list[kernel_name] = {soc: True}
                    op_kernel_list[op_name] = kernel_list if kernel_list else 'host-only'

    with open(dst_yaml_path, 'w') as yf:
        yaml_context = yaml.dump(op_kernel_list, indent=4)
        yf.write(yaml_context)
        logging.debug(yaml_context)
        logging.info(f'op list is build at {dst_yaml_path}')


def input_args_check(src_root_dir, dst_yaml_dir):
    if not os.path.exists(os.path.join(src_root_dir, 'CMakeLists.txt')):
        logging.error(f'input dir {src_root_dir} is not root directory')
        exit(1)
    if not os.path.exists(dst_yaml_dir):
        logging.error(f'dest dir {dst_yaml_dir} do not exist')
        exit(1)


def build_cmake_options(yaml_file_path, cmake_option_path, json_ini_path=''):
    logging.basicConfig(level=logging.INFO,
                        format='%(filename)s [line:%(lineno)d] - %(levelname)s: %(message)s')
    if os.path.basename(yaml_file_path) != 'op_list.yaml':
        logging.error(f'{yaml_file_path}: file name error')
        exit(1)
    if not os.path.exists(os.path.dirname(cmake_option_path)):
        logging.error(f'{os.path.dirname(cmake_option_path)} do not exist')
        exit(1)

    tbe_ini = None
    if not os.path.exists(os.path.dirname(json_ini_path)):
        logging.warning(f'{os.path.dirname(json_ini_path)} do not exist')
    else:
        tbe_ini = configparser.RawConfigParser()
        tbe_ini.optionxform = lambda option: option
        try:
            tbe_ini.read(json_ini_path)
        except configparser.MissingSectionHeaderError:
            logging.error("ini file: %s format error!", json_ini_path)
            exit(1)
        except configparser.ParsingError:
            logging.error("ini file: %s format error!", json_ini_path)
            exit(1)


    option_list = []
    built_device_list, all_device_list = build_util.get_build_target_list(with_all=True)
    try:
        with open(yaml_file_path) as f:
            op_kernel_list = yaml.safe_load(f)
            for op_name in op_kernel_list:
                if not op_name.endswith('Operation'):
                    raise ValueError(f'{op_name} is an invalid operation name')
                option_list.append(f'set(BUILD_{op_name} ON)')
                kernel_list = op_kernel_list[op_name]
                if isinstance(kernel_list, str):
                    if kernel_list != 'host-only':
                        raise ValueError(f'{op_name} kernel parse fail, invalid string: {kernel_list}')
                    continue
                kernel_built_count = 0
                reuse_kernels = set()
                if tbe_ini is not None and op_name in tbe_ini.sections():
                    reuse_kernels = dict(tbe_ini.items(op_name)).keys()
                for kernel_name in kernel_list:
                    if not kernel_name.endswith('Kernel'):
                        raise ValueError(f'{kernel_name} is an invalid kernel name')
                    soc_list = kernel_list[kernel_name]
                    for soc in soc_list:
                        if soc not in all_device_list:
                            raise ValueError(f'{soc} is an invalid soc type')
                        if not isinstance(soc_list[soc], bool):
                            raise TypeError(f'{soc_list[soc]} is not bool type')
                        full_name = f'{kernel_name}.{soc}'
                        op_switch = 'OFF'
                        if soc_list[soc] and soc in built_device_list and full_name not in reuse_kernels:
                            op_switch = 'ON'

                        option_name = f'BUILD_{op_name}_{kernel_name}_{soc}'
                        option_list.append(f'set({option_name} {op_switch})')
                        if soc_list[soc]:
                            kernel_built_count += 1
                logging.debug(f'{op_name} has {kernel_built_count} ascendc kernels built')

    except FileExistsError:
        logging.error(f'can not open yaml file, {yaml_file_path} do not exist')
        exit(1)
    except yaml.YAMLError as e:
        if isinstance(e, yaml.scanner.ScannerError) and hasattr(e, 'problem_mark'):
            mark = e.problem_mark
            logging.error(f"{yaml_file_path} parsed failed at line {mark.line}, col {mark.column + 1}")
        else:
            logging.error(f'an error occurred when parse {yaml_file_path}')
        exit(1)
    except ValueError as e:
        logging.error(e.args[0])
        exit(1)
    except TypeError as e:
        logging.error(f'{op_name} get invalid value type, error info: {e.args[0]}')
        exit(1)

    with open(cmake_option_path, 'w') as cmake_file:
        cmake_file.write('\n'.join(option_list))
        logging.info(f'cmake file is build at {cmake_option_path}')


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO,
                        format='%(filename)s [line:%(lineno)d] - %(levelname)s: %(message)s')
    scripts_dir = os.path.dirname(__file__)
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--src_root_dir', type=str, required=False,
                        default=f'{scripts_dir}/../../..',
                        help='root directory that contains a CMakeLists.txt file')
    parser.add_argument('-d', '--dst_yaml_dir', type=str, required=False,
                        default=f'{scripts_dir}/../../../configs',
                        help='destination path of op list yaml file')
    input_args = parser.parse_args()
    src_root_dir = os.path.abspath(input_args.src_root_dir)
    dst_yaml_dir = os.path.abspath(input_args.dst_yaml_dir)
    input_args_check(src_root_dir, dst_yaml_dir)
    dst_yaml_path = os.path.join(dst_yaml_dir, 'op_list.yaml')
    build_op_list(src_root_dir, dst_yaml_path)
