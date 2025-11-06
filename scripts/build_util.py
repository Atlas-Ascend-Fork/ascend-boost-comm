#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# Copyright (c) 2024 Huawei Technologies Co., Ltd.
# This file is a part of the CANN Open Software.
# Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.

import os
import configparser
import json
import logging
import shutil
import stat
import struct
import re
import argparse
import subprocess


# sycl-target --show-targets
def get_build_target_list(with_all: bool=False):
    usr_config_file_dir = os.getenv("BUILD_CONFIG_DIR", '')
    if usr_config_file_dir == '':
        script_file_path = os.path.realpath(__file__)
        build_config_json_file_path = os.path.join(os.path.dirname(
            script_file_path), "../configs/build_config.json")
    else:
        build_config_json_file_path = os.path.join(usr_config_file_dir, "build_config.json")
        build_config_json_file_path = os.path.realpath(build_config_json_file_path)
    built_device_set = set()
    all_device_set = set()
    try:
        with open(build_config_json_file_path) as conf_file:
            conf = json.load(conf_file)
            target_option = conf['targets']
            for target, switch in target_option.items():
                all_device_set.add(target)
                if switch:
                    built_device_set.add(target)
    except FileNotFoundError:
        logging.error("file %s is not found!", build_config_json_file_path)
        exit(1)
    except json.decoder.JSONDecodeError:
        logging.error("file %s is not json file!", build_config_json_file_path)
        exit(1)
    except KeyError:
        logging.error("key 'targets' is not found in %s!", build_config_json_file_path)
        exit(1)

    if len(built_device_set) == 0:
        logging.error("no target device is set")
        exit(1)

    if with_all:
        return list(built_device_set), list(all_device_set)
    else:
        return list(built_device_set)


def aligned_string(s:str, align:int) -> str:
    width = (len(s) // align + 1) * align
    return s.ljust(width, '\0')


def get_header_from_file(file_path):
    result = True
    magic_dict = {"RT_DEV_BINARY_MAGIC_ELF": 0x43554245,
                  "RT_DEV_BINARY_MAGIC_ELF_AIVEC": 0x41415246,
                  "RT_DEV_BINARY_MAGIC_ELF_AICUBE": 0x41494343}
    core_type_dict = {
        "AiCore": 0,
        "VectorCore": 2,
        "MIX": 4
    }
    aling_bytes = struct.calcsize('I')
    fixed_header_len = 128
    header = b''
    try:
        with open(file_path) as f:
            text = json.load(f)
            version = 0
            crc = 0
            compile_info_str = aligned_string(json.dumps(text["compileInfo"]), aling_bytes)
            op_para_size = text["opParaSize"]
            core_type = core_type_dict.get(text["coreType"], 0)
            magic_type = text["magic"]
            if magic_type not in magic_dict:
                logging.error("magic %s is invalid", magic_type)
                result = False
            else:
                magic = magic_dict[magic_type]
            kernel_list = []
            if "kernelList" in text:
                for kernel_item in text["kernelList"]:
                    kernel_list.append(aligned_string(kernel_item["kernelName"], aling_bytes))
            else:
                kernel_list.append(aligned_string(text["kernelName"], aling_bytes))
            kernel_num = len(kernel_list)
            if kernel_num == 0:
                result = False
            header = struct.pack('I', version) + struct.pack('I', magic) + struct.pack('I', op_para_size) + \
                     struct.pack('I', core_type) + struct.pack('I', kernel_num)
            offset = 0
            kernel_name_offset = offset
            for kernel_name in kernel_list:
                offset += (aling_bytes + len(kernel_name))
            compile_info_offset = offset
            offset += (aling_bytes + len(compile_info_str))
            binary_offset = offset
            header = header + struct.pack('I', kernel_name_offset) + struct.pack('I', compile_info_offset) + struct.pack('I', binary_offset)

            # Get ext info for reusing MIX type kernels
            intercore_sync = text.get("intercoreSync", 0)
            task_ration_type = text.get("taskRation", "tilingKey")
            if task_ration_type == "tilingKey":
                task_ration = 0
            else:
                ration = [int(r) for r in task_ration_type.split(":")]
                if len(ration) != 2:
                    logging.error(f"ration is invalid: {task_ration_type}")
                    result = False
                task_ration = (ration[0] << 16) + ration[1]
            header = header + struct.pack('I', intercore_sync) + struct.pack('I', task_ration)

            header = header.ljust(fixed_header_len - aling_bytes, b'\x00')
            header += struct.pack('I', crc)
            for kernel_name in kernel_list:
                header += struct.pack('I', len(kernel_name))
                header += kernel_name.encode('utf-8')
            header += struct.pack('I', len(compile_info_str))
            header += compile_info_str.encode('utf-8')
    except FileNotFoundError:
        logging.error("file %s is not found!", file_path)
        result = False
    except json.decoder.JSONDecodeError:
        logging.error("file %s is not json file!", file_path)
        result = False
    except KeyError:
        logging.error("keyerror in file %s!", file_path)
        result = False
    return header, result


def write_to_cpp(binary_path, header, dst_cpp_path, kernel, target_version):
    try:
        with open(binary_path, 'rb') as f:
            data = f.read()
            binary_size = len(data)
            header += struct.pack('I', binary_size)
            data = header + data
    except FileNotFoundError:
        logging.error("file %s is not found!", binary_path)
        return False
    shell_result = subprocess.run(['strings', os.path.realpath(binary_path)], capture_output=True, text=True)
    # 将数据写入到cpp文件中
    name = f'KERNELBIN_{kernel.upper()}_{target_version.upper()}'
    data_type = 'uint8_t' if 'g_opSystemRunCfg' in shell_result.stdout else 'const uint8_t'
    with open(dst_cpp_path, 'w') as f:
        f.write('#include <cstdint>\n#include "mki_loader/op_register.h"\n')
        f.write('namespace OpSpace {\n')
        f.write(f'static {data_type} {name}')
        f.write('[] = {')
        for i in range(0, len(data), 16):
            f.write(', '.join('0x{:02x}'.format(b) for b in data[i:i+16]))
            if i + 16 < len(data):
                f.write(', ')
        f.write('};\n\n')
        f.write(f'REG_KERNEL({target_version}, {kernel}, {name});\n')
        f.write('}\n')
    return True


# 目前只支持一个kernel文件夹下一个.o和.json文件
def copy_ascendc_code(binary_dir, target_version, output_path):
    op_kernels_version_dir = os.path.join(
        binary_dir, "op_kernels", target_version)
    if not os.path.exists(op_kernels_version_dir):
        return 0
    code_file_count = 0
    for operation in os.listdir(op_kernels_version_dir):
        operation_dir = os.path.join(op_kernels_version_dir, operation)
        output_operation_dir = os.path.join(output_path, operation)
        if not os.path.exists(output_operation_dir):
            os.makedirs(output_operation_dir)
        for kernel in os.listdir(operation_dir):
            kernel_dir = os.path.join(operation_dir, kernel)
            for file in os.listdir(kernel_dir):
                if not file.endswith('.json'):
                    continue
                code_file = os.path.join(kernel_dir, f"{file[:-4]}o")
                if not os.path.exists(code_file):
                    logging.error("file %s has no object file.", file)
                    exit(1)

                json_file = os.path.join(kernel_dir, file)
                header, result = get_header_from_file(json_file)
                if not result:
                    logging.error("failed to parse file %s.", json_file)
                    exit(1)

                dst_cpp_path = f"{os.path.join(output_operation_dir, file)[:-4]}cpp"
                result = write_to_cpp(code_file, header, dst_cpp_path, kernel, target_version)
                if not result:
                    logging.error("failed to write into file %s.", dst_cpp_path)
                    exit(1)

                code_file_count += 1
    return code_file_count


def compile_ascendc_code(obj_path, dst_cpp_path, is_const=True):
    if not obj_path.endswith('.o'):
        logging.error("%s is not an obj file.", obj_path)
        exit(1)
    json_file = obj_path.rsplit('.', 1)[0] + '.json'
    header, result = get_header_from_file(json_file)
    if not result:
        logging.error("failed to parse file %s.", json_file)
        exit(1)
    obj_realpath = os.path.realpath(obj_path)
    kernel = obj_path.split('/')[-2]
    target_version = obj_realpath.split('/')[-4]
    output_dir = os.path.dirname(dst_cpp_path)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir, exist_ok=True)
    result = write_to_cpp(obj_path, header, dst_cpp_path, kernel, target_version)
    if not result:
        logging.error("failed to write into file %s.", dst_cpp_path)
        exit(1)


def copy_tbe_code_all_version(input_paras):
    tbe_sections = input_paras["tbe_ini"].sections()
    for target_version in input_paras["target_version_list"]:
        binary_id = 0
        output_path = os.path.join(
            input_paras["env_cache_dir"], "obj", target_version)
        if not os.path.exists(output_path):
            os.makedirs(output_path, exist_ok=True)
        target_version_path = os.path.join(
            input_paras["tbe_kernel_path"], target_version)

        for op_name in tbe_sections:
            op_dir_path = os.path.join(output_path, op_name)
            if not os.path.exists(op_dir_path):
                os.makedirs(op_dir_path, exist_ok=True)
            items = dict(input_paras["tbe_ini"].items(op_name))
            for op_key, relative_op_path in items.items():
                if '.' in op_key:
                    op_key, version_op_key = op_key.split('.')
                    if version_op_key != target_version:
                        continue
                code_file = os.path.join(
                    target_version_path, f"{relative_op_path[:-4]}o")
                object_name = os.path.basename(code_file)
                dst_cpp_path = os.path.join(op_dir_path, f"{object_name[:-2]}_{op_key.lower()}.cpp")

                header, ret = get_header_from_file(os.path.join(
                    target_version_path, relative_op_path))
                if not ret:
                    logging.error("failed to parse json file %s", relative_op_path)
                    exit(1)

                result = write_to_cpp(code_file, header, dst_cpp_path, op_key, target_version)
                binary_id += 1
                if not result:
                    logging.error("failed to write into file %s.", dst_cpp_path)
                    exit(1)


def copy_ascendc_code_all_version(input_paras):
    for target_version in input_paras["target_version_list"]:
        output_path = os.path.join(
            input_paras["env_cache_dir"], "obj", target_version)
        if not os.path.exists(output_path):
            os.makedirs(output_path)

        ascendc_file_count = copy_ascendc_code(
            input_paras["binary_dir"], target_version, output_path)
        logging.info(
            f"{target_version} has {ascendc_file_count} AscendC kernels.")


def copy_tbe_device_code(args):
    env_cache_dir = os.getenv("CACHE_DIR")
    if not (env_cache_dir):
        logging.error(
            "env CACHE_DIR not exist!")
        exit(1)
    env_cache_dir = os.path.realpath(env_cache_dir)
    target_version_list = get_build_target_list()

    if (args.op_type == "tbe" or args.op_type is None):
        env_code_root = os.getenv("CODE_ROOT")
        tbe_kernel_path = os.getenv("ASCEND_KERNEL_PATH")
        if not (env_code_root and tbe_kernel_path):
            logging.error(
            "env CODE_ROOT | ASCEND_KERNEL_PATH not exist!")
            exit(1)
        logging.info(f"tbe_kernel_path: {tbe_kernel_path}")
        env_code_root = os.path.realpath(env_code_root)
        tbe_kernel_path = os.path.realpath(tbe_kernel_path)
        input_path = os.path.join(env_code_root, args.tbe_ini_path)
        if not os.path.exists(input_path):
            logging.error("ini file: %s not exist!", input_path)
            exit(1)
        tbe_ini = configparser.RawConfigParser()
        tbe_ini.optionxform = lambda option: option
        try:
            tbe_ini.read(input_path)
        except configparser.MissingSectionHeaderError:
            logging.error("ini file: %s format error!", input_path)
            exit(1)
        except configparser.ParsingError:
            logging.error("ini file: %s format error!", input_path)
            exit(1)
        copy_tbe_code_all_version({"target_version_list": target_version_list,
                                "env_cache_dir": env_cache_dir,
                                "tbe_kernel_path": tbe_kernel_path,
                                "binary_dir": args.binary_dir,
                                "tbe_ini": tbe_ini})

    if (args.op_type == "ascendc" or args.op_type is None):
        copy_ascendc_code_all_version({"target_version_list": target_version_list,
                                "env_cache_dir": env_cache_dir,
                                "binary_dir": args.binary_dir})


def get_build_target_list_for_shell():
    return "\n".join(get_build_target_list())

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary_dir', type=str, required=True)
    parser.add_argument('--op_type', type=str, required=False)
    parser.add_argument('--tbe_ini_path', type=str, default="configs/tbe_tactic_json.ini", required=False)
    input_args = parser.parse_args()
    copy_tbe_device_code(input_args)
