# Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.

import argparse
import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

SCRIPT = Path(__file__).resolve().parents[2] / 'scripts' / 'compile_ascendc.py'
spec = importlib.util.spec_from_file_location('compile_ascendc', str(SCRIPT))
compiler = importlib.util.module_from_spec(spec)
spec.loader.exec_module(compiler)


class TilingKeyTest(unittest.TestCase):
    def test_key_selection_keeps_first_occurrence_and_default(self):
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / 'kernel.cpp'
            source.write_text('TILING_KEY_IS(2) TILING_KEY_IS(1) TILING_KEY_IS(2)', encoding='utf-8')
            self.assertEqual(compiler.get_tiling_key_ids(str(source)), ['2', '1'])
            source.write_text('no tiling key', encoding='utf-8')
            self.assertEqual(compiler.get_tiling_key_ids(str(source)), [0])

    def test_duplicate_keys_are_compiled_and_linked_once(self):
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / 'kernel.cpp'
            source.write_text(
                'if (TILING_KEY_IS(1)) {}\nif (TILING_KEY_IS(2)) {}\nif (TILING_KEY_IS(1)) {}\n', encoding='utf-8'
            )
            args = argparse.Namespace(
                srcs=str(source),
                soc='ascend910b',
                channel='vector',
                code_root=tmp,
                include_directories=None,
                no_warning=False,
                dst=str(Path(tmp) / 'kernel.o'),
                kernel='kernel',
                use_mssanitizer='OFF',
                use_msdebug='OFF',
                use_ascendc_dump=False,
            )

            def fake_compile(_args, key, _arch, _options, _sanitizer):
                return key, [str(Path(tmp) / ('kernel_' + str(key) + '.o'))], True

            with patch.object(compiler, '_compile_one_tiling_key', side_effect=fake_compile) as compile_key:
                with patch.object(compiler, 'exe_cmd', return_value=0) as link:
                    with patch.object(compiler, 'gen_json') as metadata:
                        self.assertEqual(compiler.compile_ascendc_operation(args), 0)
            self.assertCountEqual([call.args[1] for call in compile_key.call_args_list], ['1', '2'])
            link_cmd = link.call_args.args[0]
            self.assertEqual(link_cmd.count(str(Path(tmp) / 'kernel_1.o')), 1)
            self.assertEqual(metadata.call_args.args[1], ['kernel_1', 'kernel_2'])


if __name__ == '__main__':
    unittest.main()
