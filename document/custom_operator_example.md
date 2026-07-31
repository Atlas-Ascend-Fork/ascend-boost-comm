# Single Operator Example Using the Ascend Boost Comm Library

This tutorial uses the add operator as an example to provide a guide for writing operators that run on Ascend Boost Comm.

### Operator Function

Add two input tensors along a specified dimension to produce one output tensor.
![AddcustomOperation](images/AddcustomOperation.png)
### New Files

- Create the `addcustom` directory under `examples/ops`. This directory primarily stores the operator implementation code. See the following sections for the specific file contents. The directory structure is as follows:
    ```
    addcustom
    ├── op_kernel                       // Device-side implementation files (including kernel entry and implementation files)
    │   └── addcustom.cpp
    ├── tiling                          // New operator tiling
    │   ├── addcustom_tiling.cpp        // Core tiling implementation algorithm
    │   ├── addcustom_tiling.h          // Operator tiling interface
    │   └── tiling_data.h              // Definition of the tiling_data structure passed between tiling and kernel
    ├── CMakeLists.txt                  // CMake file for the new operator build
    ├── addcustom_kernel.cpp            // Validation
    └── addcustom_operation.cpp         // Shape validation
    ```

- Create the `example/include/asbops/params/addcustom.h` file to define the parameter structure for the `Addcustom` operation. The content is as follows:
    ```c++
    #ifndef ATBOPS_PARAMS_ADDCUSTOM_H
    #define ATBOPS_PARAMS_ADDCUSTOM_H

    #include <cstdint>
    #include <string>
    #include <sstream>
    #include <mki/utils/SVector/SVector.h>

    namespace Mki {
    namespace OpParam {
    struct Addcustom {

        bool operator==(const Addcustom &other) const
        {
            (void)other;
            return true;
        }
    };

    } // namespace OpParam
    } // namespace Mki

    #endif
    ```
### Modified Files

- Include the new header file `example/include/asbops/params/addcustom.h` in `example/include/asbops/params/params.h`:
    ```c++
    #include "atbops/params/addcustom.h"
    ```
## Environment Preparation

Refer to [Environment Preparation](../README_en.md#3-environment-setup) to set up the build and test environment. Once the environment is ready, you can begin developing the operator.
## Operator Implementation

Operator implementation mainly includes device-side operator implementation and host-side tiling implementation.
### Tiling Development

Core concepts of tiling development include `TilingData`, `Workspace`, `TilingKey`, `BlockDim`, and more. For details, see [Glossary - Ascend Community](https://www.hiascend.com/document/detail/en/canncommercial/82RC1/opdevg/Ascendcopdevg/atlas_ascendc_10_00013.html).

#### tiling_data.h

File path: `example/ops/addcustom/tiling/tiling_data.h`
Main function: Describes the structured data definitions required for tiling computation.

```c++
#ifndef ASCEND_OPS_ADDCUSTOM_TILING_DATA
#define ASCEND_OPS_ADDCUSTOM_TILING_DATA

#include <cstdint>

namespace Mki {
struct AddcustomTilingData {
    uint32_t totalLength;  // Total data length
    uint32_t tileNum;      // Number of tiles
};
}
#endif
```

#### addcustom_tiling.h
File path: `example/ops/addcustom/tiling/addcustom_tiling.h`
Main function: The tiling process primarily performs data partitioning. This file contains function declarations.
```c++
#ifndef ASCEND_OPS_ADDCUSTOM_TILING_H
#define ASCEND_OPS_ADDCUSTOM_TILING_H

#include <mki/launch_param.h>
#include <mki/kernel_info.h>
#include <mki/utils/status/status.h>

namespace Mki {
Status AddcustomTiling(const LaunchParam &launchParam, KernelInfo &kernelInfo);
}

#endif
```

#### addcustom_tiling.cpp
File path: `example/ops/addcustom/tiling/addcustom_tiling.cpp`
Main function: Implements the main function for data partitioning. Refer to the content under the file paths above for specific function implementations.
```c++
#include "addcustom_tiling.h"
#include <mki/utils/assert/assert.h>
#include <mki/utils/log/log.h>
#include <mki/utils/platform/platform_info.h>
#include <mki/utils/math/math.h>
#include <mki/utils/SVector/SVector.h>
#include "atbops/params/addcustom.h"
#include "tiling_data.h"

// Define the minimum block length
constexpr uint32_t MIN_BLOCK_LENGTH = 32;

namespace Mki {
Status AddcustomTiling(const LaunchParam &launchParam, KernelInfo &kernelInfo)
{
    // Refer to the content under the file paths above for specific function implementations
}
} // namespace Mki
```

### Kernel Development

For kernel-related concepts such as `Compute`, `CopyIn`, and `CopyOut`, see [Glossary - Ascend Community](https://www.hiascend.com/document/detail/en/canncommercial/82RC1/opdevg/Ascendcopdevg/atlas_ascendc_10_00013.html).
#### addcustom.cpp
File path: `example/ops/addcustom/op_kernel/addcustom.cpp`
Main function: Handles all data movement and computation based on the tiling information.
```c++
#include "kernel_operator.h"
#include "ops/utils/common/kernel/kernel_utils.h"
#include "ops/addcustom/tiling/tiling_data.h"

static constexpr uint32_t BUFFER_NUM = 1;
static constexpr uint32_t MAX_UB_SIZE = 188 * 1024; // Double buffer, 94KB per block, 188KB total

class Addcustom {
public:
    __aicore__ inline Addcustom() {}
    __aicore__ inline void Init(GM_ADDR x, GM_ADDR y, GM_ADDR z, uint32_t totalLength, uint32_t tileNum)
    {
        // Refer to the content under the file paths above for specific function implementations
    }
    __aicore__ inline void Process()
    {
        // Refer to the content under the file paths above for specific function implementations
    }

private:
    __aicore__ inline void CopyIn(int32_t progress)
    {
        // Refer to the content under the file paths above for specific function implementations
    }
    __aicore__ inline void Compute(int32_t progress)
    {
        // Refer to the content under the file paths above for specific function implementations
    }
    __aicore__ inline void CopyOut(int32_t progress)
    {
        // Refer to the content under the file paths above for specific function implementations
    }

private:
    AscendC::TPipe pipe;
    AscendC::TQue<AscendC::QuePosition::VECIN, BUFFER_NUM> inQueueX, inQueueY;
    AscendC::TQue<AscendC::QuePosition::VECOUT, BUFFER_NUM> outQueueZ;
    AscendC::GlobalTensor<half> xGm;
    AscendC::GlobalTensor<half> yGm;
    AscendC::GlobalTensor<half> zGm;
    uint32_t blockLength;
    uint32_t tileNum;
    uint32_t tileLength;
};

inline __aicore__ void InitTilingData(const __gm__ uint8_t *p_tilingdata, Mki::AddcustomTilingData *tilingdata)
{
    // Refer to the content under the file paths above for specific function implementations
}

extern "C" __global__ __aicore__ void addcustom(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR tiling)
{
    // Refer to the content under the file paths above for specific function implementations
}
```

#### addcustom_kernel.cpp
File path: `example/ops/addcustom/addcustom_kernel.cpp`
Main function: Performs input and output checks and initializes the device side before launching the device-side implementation.
```c++
#include <mki/base/kernel_base.h>
#include <mki_loader/op_register.h>
#include <mki/utils/assert/assert.h>
#include <mki/utils/log/log.h>
#include "atbops/params/params.h"
#include "ops/addcustom/tiling/addcustom_tiling.h"
#include "ops/addcustom/tiling/tiling_data.h"

namespace Mki {

class AddcustomKernel : public KernelBase {
public:
    explicit AddcustomKernel(const std::string &kernelName, const BinHandle *handle) noexcept
        : KernelBase(kernelName, handle)
    {
    }

    /* ---------- Framework Callbacks ---------- */
    bool CanSupport(const LaunchParam &launchParam) const override
    {
        // Refer to the content under the file paths above for specific function implementations
    }

    uint64_t GetTilingSize(const LaunchParam &launchParam) const override
    {
        // Refer to the content under the file paths above for specific function implementations
    }

    Status InitImpl(const LaunchParam &launchParam) override
    {
       // Refer to the content under the file paths above for specific function implementations
    }
};

/* ---------- Register with the Framework ---------- */
REG_KERNEL_BASE(AddcustomKernel);

}   // namespace Mki
```
#### addcustom_operation.cpp

File path: `example/ops/addcustom/addcustom_operation.cpp`
Main function: Defines the behaviors of the Operation, which is the highest level of abstraction for an op under the Ascend Boost Comm framework, including the strategy for selecting the best kernel.

```c++
#include <mki/base/operation_base.h>
#include <mki_loader/op_register.h>
#include <mki/utils/log/log.h>
#include "atbops/params/params.h"

namespace Mki {
using namespace Mki;

static constexpr int32_t INPUT_NUM  = 2;
static constexpr int32_t OUTPUT_NUM = 1;

class AddcustomOperation : public OperationBase {
public:
    explicit AddcustomOperation(const std::string &opName) noexcept : OperationBase(opName) {}

    /* ---------- Select the Best Kernel Strategy ---------- */
    Kernel *GetBestKernel(const LaunchParam &launchParam) const override
    {
        // Refer to the content under the file paths above for specific function implementations
    }

    /* ---------- Number of Tensors ---------- */
    int64_t GetInputNum(const Any &specificParam) const override
    {
        // Refer to the content under the file paths above for specific function implementations
    }

    int64_t GetOutputNum(const Any &specificParam) const override
    {
        // Refer to the content under the file paths above for specific function implementations
    }

protected:
    /* ---------- Shape Inference ---------- */
    Status InferShapeImpl(const LaunchParam &launchParam,
                          SVector<Tensor> &outTensors) const override
    {
        // Refer to the content under the file paths above for specific function implementations
    }
};

/* ---------- Registration ---------- */
REG_OPERATION(AddcustomOperation);

}  // namespace Mki
```

## Build and Test

### CMakeLists.txt
File path: `example/ops/addcustom/CMakeLists.txt`
Main function: File compilation.
```
set(addcustom_srcs
    ${CMAKE_CURRENT_LIST_DIR}/addcustom_operation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/addcustom_kernel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tiling/addcustom_tiling.cpp
)

add_operation(AddcustomOperation "${addcustom_srcs}")

add_kernel(addcustom ascend910 vector
    op_kernel/addcustom.cpp
    AddcustomKernel)

add_kernel(addcustom ascend910b vector
    op_kernel/addcustom.cpp
    AddcustomKernel)
```

### Operator Build and Environment Variable Setup
The build script for the Ascend Boost Comm repository is `scripts/build.sh`. Before running operator tests for the first time, you need to build the test framework (testframework). If your local environment has **GCC ≥ 12**, append `--no_werror` to the command. For details, see [README Build Instructions](../README_en.md#compilation):
```shell
bash scripts/build.sh testframework
```
Then build the operators in the example directory:
```shell
bash scripts/build.sh example
```
After building, set the environment variables:
```shell
source output/mki/set_env.sh
```

### Testing
To test the operator you have written, create `test_addcustom.py` under the `example/tests/pythontest/optest/` directory:
```python
import os
import unittest
import numpy as np
import torch
import sys
import logging

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

    def test_2d_half(self):
        shape = (2 * 16,)
        a = torch.randn(shape).to(torch.float16)
        b = torch.randn(shape).to(torch.float16)

        self.set_param(OP_NAME, OP_PARAM0)
        self.execute([a, b], [torch.ones(shape).to(torch.float16)])


if __name__ == '__main__':
    unittest.main()

```
Run the test script with the following command:
```shell
python example/tests/pythontest/optest/test_addcustom.py
```
