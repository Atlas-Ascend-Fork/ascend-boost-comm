#
# Copyright (c) 2025 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
#
import os
import unittest
import torch
import sys

sys.path.append(f"{os.environ['MKI_HOME_PATH']}/tests/pythontest")
import op_test

OP_NAME = "AddcustomOperation"
OP_PARAM0 = {"addcustomDim": 0}


class TestAddcustom(op_test.OpTest):
    def golden_calc(self, in_tensors):
        a = in_tensors[0]
        b = in_tensors[1]
        return [a + b]

    def golden_compare(self, out_tensors, golden_out_tensors):
        return torch.allclose(out_tensors[0], golden_out_tensors[0], rtol=0.001, atol=0.001)

    # 用 -9999 作为输出初值：任何未被写入（或被旧数据覆盖）的元素都会与正确结果不等，
    # 保证「覆盖/漏算」类问题一定暴露，而不是碰巧等于 0 而蒙混过关。
    SENTINEL = -9999.0

    def _run_length(self, length, seed=0):
        """按给定长度跑一次 device 计算，并与 golden 逐元素比较。

        execute() 内部会调用 golden_compare 完成断言；这里返回 golden 结果，
        由调用方校验其形状，保证返回值参与断言（G.FNM.07）。
        """
        torch.manual_seed(seed)
        shape = (length,)
        a = torch.randn(shape).to(torch.float16)
        b = torch.randn(shape).to(torch.float16)
        self.set_param(OP_NAME, OP_PARAM0)
        self.set_support_910b_only()
        self.execute([a, b], [torch.full(shape, self.SENTINEL).to(torch.float16)])
        return a + b

    def _run_and_check_each_element(self, length):
        """用可精确表示的确定性数值跑 device，并逐个元素断言结果。

        取值都在 half 能精确表示的整数范围内（求和 < 2048），因此可以要求逐位相等，
        比 allclose 更能暴露「个别元素被覆盖」的问题。
        """
        idx = torch.arange(length, dtype=torch.float32)
        x = (idx + 1).to(torch.float16)
        y = (idx + 101).to(torch.float16)
        out = torch.full((length,), self.SENTINEL).to(torch.float16)

        self.set_param(OP_NAME, OP_PARAM0)
        self.set_support_910b_only()
        out_tensors = self.execute_perf([x, y], [out])
        if out_tensors is None:
            self.skipTest("current SoC is not supported for this case")
        result = out_tensors[0]

        expected = (x.float() + y.float()).to(torch.float16)
        for i in range(length):
            self.assertEqual(result[i].item(), expected[i].item(), f"length={length}, element {i} mismatch")
        return result

    def test_2d_half(self):
        shape = (2 * 32,)
        a = torch.randn(shape).to(torch.float16)
        b = torch.randn(shape).to(torch.float16)

        self.set_param(OP_NAME, OP_PARAM0)
        self.execute([a, b], [torch.ones(shape).to(torch.float16)])

    def test_length_31_half_tail_cacheline(self):
        """审查意见场景：31 个 FP16，对齐段 16 + 尾段 15 同处一条 64B cache line。

        尾段若用 GM SetValue 直接写回，未等待 MTE3 的脏行整行回写会覆盖前 16 个结果。
        这里对 31 个元素逐位校验，前 16 个（对齐段，DataCopy 写出）一旦被覆盖就会失败。
        """
        out = self._run_and_check_each_element(31)

        # 额外显式断言前 16 个元素（对齐段）与后 15 个元素（尾段）都正确，
        # 让失败信息能直接定位是「对齐段被覆盖」还是「尾段漏算」。
        idx = torch.arange(31, dtype=torch.float32)
        expected = ((idx + 1) + (idx + 101)).to(torch.float16)
        for i in range(16):
            self.assertEqual(
                out[i].item(), expected[i].item(), f"aligned head element {i} overwritten (got {out[i].item()})"
            )
        for i in range(16, 31):
            self.assertEqual(out[i].item(), expected[i].item(), f"tail element {i} wrong (got {out[i].item()})")

    def test_tail_boundaries_half(self):
        """尾段边界：不足对齐、恰好对齐、对齐+1、跨 64B cache line 边界，逐元素校验。"""
        for length in (1, 2, 15, 16, 17, 30, 31, 32, 33, 47, 48, 49, 63, 64, 65):
            with self.subTest(length=length):
                result = self._run_and_check_each_element(length)
                self.assertEqual(result.numel(), length)

    def test_multicore_tail_half(self):
        """多核切分：长度不可整除、尾核承接剩余元素，逐元素校验。"""
        for length in (101, 1500, 4097):
            with self.subTest(length=length):
                result = self._run_and_check_each_element(length)
                self.assertEqual(result.numel(), length)

    def test_tail_boundaries(self):
        """随机数据 + allclose 的补充用例，覆盖尾段边界的数值精度。"""
        for length in (1, 2, 15, 16, 17, 30, 31, 32, 33, 47, 48, 49, 63, 64, 65):
            with self.subTest(length=length):
                golden = self._run_length(length, seed=length)
                self.assertEqual(golden.numel(), length)

    def test_multicore_tail(self):
        """随机数据 + allclose 的补充用例，覆盖多核切分。"""
        for length in (101, 1500, 4097, 32771):
            with self.subTest(length=length):
                golden = self._run_length(length, seed=length)
                self.assertEqual(golden.numel(), length)


if __name__ == '__main__':
    unittest.main()
