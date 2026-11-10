# Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.

import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

import yaml

SCRIPT = Path(__file__).resolve().parents[2] / 'scripts' / 'op_list_utils.py'


class OpListWhitespaceTest(unittest.TestCase):
    def test_whitespace_variants_generate_same_operation_list(self):
        fixtures = (
            'add_operation(AddcustomOperation source.cpp)\nadd_kernel(addcustom ascend910b vector kernel.cpp AddcustomKernel)\n',
            'add_operation( AddcustomOperation source.cpp )\nadd_kernel(addcustom ascend910b vector kernel.cpp AddcustomKernel)\n',
            'add_operation(AddcustomOperation source.cpp)\nadd_kernel(addcustom\tascend910b\tvector\tkernel.cpp\tAddcustomKernel)\n',
        )
        for content in fixtures:
            with self.subTest(content=content), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                (root / 'CMakeLists.txt').write_text('# root\n', encoding='utf-8')
                op = root / 'op'
                op.mkdir()
                (op / 'CMakeLists.txt').write_text(content, encoding='utf-8')
                dest = root / 'config'
                dest.mkdir()
                result = subprocess.run(
                    [sys.executable, str(SCRIPT), '-s', str(root), '-d', str(dest)],
                    capture_output=True,
                    text=True,
                    env=os.environ.copy(),
                    check=False,
                )
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                data = yaml.safe_load((dest / 'op_list.yaml').read_text(encoding='utf-8'))
                self.assertEqual(data, {'AddcustomOperation': {'AddcustomKernel': {'ascend910b': True}}})
                options = dest / 'op_build.cmake'
                code = (
                    "import sys; sys.path.insert(0, sys.argv[1]); "
                    "import op_list_utils; op_list_utils.build_cmake_options(sys.argv[2], sys.argv[3])"
                )
                generated = subprocess.run(
                    [sys.executable, '-c', code, str(SCRIPT.parent), str(dest / 'op_list.yaml'), str(options)],
                    capture_output=True,
                    text=True,
                    env=os.environ.copy(),
                    check=False,
                )
                self.assertEqual(generated.returncode, 0, generated.stdout + generated.stderr)
                self.assertIn(
                    'set(BUILD_AddcustomOperation_AddcustomKernel_ascend910b ON)', options.read_text(encoding='utf-8')
                )


if __name__ == '__main__':
    unittest.main()
