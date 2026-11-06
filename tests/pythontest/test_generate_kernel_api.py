# Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.

import importlib.util
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[2] / 'scripts' / 'generate_kernel_api.py'
spec = importlib.util.spec_from_file_location('generate_kernel_api', str(SCRIPT))
generator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(generator)


class GenerateKernelApiTest(unittest.TestCase):
    def test_dump_insertion_preserves_inline_and_multiline_body(self):
        for body in ('{int value = 1;}', '{\nint value = 1;}', '{\r\nint value = 1;}'):
            with self.subTest(body=body):
                result = generator.add_initDump_in_kernel_function(body)
                self.assertEqual(result.count(generator.INITDUMP_CONTENT), 1)
                self.assertEqual(result.replace('\n' + generator.INITDUMP_CONTENT, '', 1), body)

    def test_cli_keeps_inline_first_statement_intact(self):
        source = 'extern "C" __global__ __aicore__ void my_kernel(GM_ADDR x, GM_ADDR tiling) {int value = 1;}\n'
        with tempfile.TemporaryDirectory() as temp:
            src = Path(temp) / 'kernel.cpp'
            dst = Path(temp) / 'decorated.cpp'
            src.write_text(source, encoding='utf-8')
            subprocess.run([sys.executable, str(SCRIPT), '--srcs', str(src), '--dst', str(dst)], check=True)
            output = dst.read_text(encoding='utf-8')
            self.assertIn('int value = 1;', output)
            self.assertIn(generator.INITDUMP_CONTENT, output)
            self.assertEqual(src.read_text(encoding='utf-8'), source)


if __name__ == '__main__':
    unittest.main()
