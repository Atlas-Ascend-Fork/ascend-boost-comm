# Ascend Boost Comm

English | [简体中文](./README.md)

🔥 [2025/09] Ascend Boost Comm project was first launched.
## What Is Ascend Boost Comm?
### Introduction to Ascend Boost Comm
Ascend Boost Comm is a common component of domain-specific acceleration libraries. It defines the L0-level APIs for operator calling in a unified manner. It interconnects with operator libraries developed by different organizations and supports different acceleration library applications for M x N operator capability reuse.

### Software Architecture
Software architecture description
1. Invoking relationship
Domain-specific acceleration libraries (such as [Ascend Transformer Boost (ATB)](https://gitcode.com/cann/ascend-transformer-boost) and signal acceleration library) --> Ascend Boost Comm

### Ascend Boost Comm Repository

The directory structure of the Ascend Boost Comm library is as follows:

```
ascend-boost-comm
├── cmake                 // Compilation and link artifacts
├── configs               // Build artifacts
├── docs                  // Script file directory
├── example               // Example code for operator calling
├── scripts             // Script file directory
├── src                 // Main source code directory
├── include             // Public header file directory
│   ├── mki_loader        // Logic code related to operator loading
│   ├── schedule          // Logic code related to operator scheduling
│   ├── schedule          // Utility classes directory
│   └── CMakeLists.txt
├── tests                 # Test code
```

## Product Support

The following table lists the Huawei Ascend hardware architectures supported by this repository.

| Hardware Model| Supported| Description|
|---|---|---|
| Atlas A2 training products/Atlas A2 inference products| √ | — |
| Atlas A3 training products/Atlas A3 inference products| √ | — |
| Atlas inference products| √ | — |

## Environment Setup
### Basic Environment Dependencies

**Compilation dependencies (mandatory for the project):**

| Component| Version Requirement| Description|
|---|---|---|
| Python | 3.10.x or 3.11.x| Dependency for running Python scripts during compilation|
| cmake | ≥ 3.20 | Build system dependency |
| gcc/g++ | Recommended: 7.3.1-11.x| If GCC ≥ 12, add `--no_werror` to the compilation command. (See [Compilation Description](#compilation).)|

**Runtime example / test dependencies (install only when compiling and running the test framework or examples):**

| Component| Version Requirement| Description|
|---|---|---|
| PyTorch | >= 2.1.0 | Must match the CANN and torch_npu versions (see the note below) |
| torch_npu (Ascend Extension for PyTorch)| See the official document.| Must be compatible with CANN and torch versions. For details, see the following instruction.|

> **Note**: The Ascend Boost Comm library (compilation and build) does not depend on PyTorch, torch_npu, or NumPy. The preceding components need to be installed only when you run the examples and test cases in the `example/` and `tests/` directories.

PyTorch/torch_npu installation and version: Install the CANN Toolkit first, then refer to the "Version Description" and "Software Installation" sections in the [Ascend Extension for PyTorch Development Documentation](https://www.hiascend.com/document/detail/en/Pytorch), and select the torch_npu that matches your CANN and PyTorch versions for installation.

> Example: If CANN 9.0.0 and PyTorch 2.7.1 are used, run the following command to install torch_npu:
> ```shell
> pip install torch==2.7.1 torch-npu==2.7.1.post4
> ```

**Additional dependency for building together with an acceleration library (needed only for "Compile and package together with the acceleration library" in [Usage Instructions](#usage-instructions)):**

| Component| Version Requirement| Description|
|---|---|---|
| nnal (Ascend-cann-nnal) | **Must be exactly the same version as toolkit and ops** | Provides `libtbe_adapter.so`, which is mandatory when building the acceleration library (ATB in this example) |

> ⚠️ **Important prerequisite**: A CANN weekly/master-build toolkit **may not have an nnal package of the same version**. If you intend to follow "Compile and package together with the acceleration library", switch to an official release where toolkit, ops, and nnal are all of **the same version** (for example, 9.2.0 for all three), that is, **Installation Method B** below. Otherwise the acceleration library build fails because `libtbe_adapter.so` is missing.

### Quick Installation of CANN Software

For developers with Ascend devices who want to manually set up the Ascend environment, perform the following steps.

#### Installing the Software

**Installation Method A: Experience the latest master capabilities or develop based on the CANN weekly version**

See [CANN Quick Installation](https://hiascend.com/cann/download?versionId=802&ids=d806%2Ch0501%2Ch0601%2Ch0703&currentTab=1), select the Weekly version, and download the corresponding package based on the product series, CPU architecture, and operating system. Then complete the installation by following the commands provided on the page.

> ℹ️ The weekly version is intended for experiencing the latest master capabilities of this repository and for building this repository alone (that is, "Single-operator project" in the usage instructions). A weekly-build toolkit may not yet have a matching nnal package, so it is **not recommended** for "Compile and package together with the acceleration library". Use Installation Method B below instead.

**Installation Method B: Experience branch capabilities or develop based on the CANN stable version (recommended)**

See [CANN Quick Installation](https://hiascend.com/cann/download?versionId=810&ids=d806%2Ch0501%2Ch0601%2Ch0702&currentTab=0), select the stable version (only CANN 8.5.0 and later versions are supported), and download the corresponding package based on the product series, CPU architecture, and operating system. Then complete the installation by following the commands provided on the page.

The stable release line ships all three packages: toolkit, chip-specific ops, and nnal, so it satisfies the `libtbe_adapter.so` dependency of "Compile and package together with the acceleration library".

#### Installing the nnal Package (needed only for "Compile and package together with the acceleration library")

To follow "Compile and package together with the acceleration library", you need the nnal package in addition to toolkit and ops. Its version must be **exactly the same** as toolkit and ops.

You can obtain the nnal package from the [CANN download page](https://www.hiascend.com/developer/download/community): select the product series and model, choose **the same solution version as the installed toolkit**, and download `Ascend-cann-nnal_${VERSION}_linux-$(arch).run` in the CANN area.

```shell
# The package name looks like: Ascend-cann-nnal_${VERSION}_linux-$(arch).run
chmod +x Ascend-cann-nnal_${VERSION}_linux-$(arch).run
./Ascend-cann-nnal_${VERSION}_linux-$(arch).run --check     # Verify package integrity and consistency
./Ascend-cann-nnal_${VERSION}_linux-$(arch).run --install   # "xxx install success!" indicates a successful installation
```

After installation, run a 5-second self-check to confirm that `libtbe_adapter.so` is in place (`/usr/local/Ascend` is the default root installation path; non-root users should replace it with `${HOME}/Ascend`):

```shell
ls /usr/local/Ascend/nnal/atb/latest/atb/cxx_abi_0/lib/libtbe_adapter.so \
   /usr/local/Ascend/nnal/atb/latest/atb/cxx_abi_1/lib/libtbe_adapter.so
```

**At least one** of the two paths must exist: `cxx_abi_1` corresponds to `_GLIBCXX_USE_CXX11_ABI=1`, and `cxx_abi_0` corresponds to `=0`. Which one is used depends on the PyTorch ABI at build time (see Usage Instructions below). If neither path exists, either nnal is not installed or it was installed to a non-default path; in the latter case, set `ATB_BUILD_DEPENDENCY_PATH` (see Usage Instructions).

#### Environment Verification

After installing the CANN package, verify that the environment and driver are working properly.

**Check the NPU device**

```shell
# Run npu-smi. If device information is displayed properly, the driver is working properly.
npu-smi info
```

**Check the CANN version**

```shell
# Check the CANN toolkit package version information (default installation path)
# Docker and manual installation scenarios:
cat /usr/local/Ascend/cann/${arch}-linux/ascend_toolkit_install.info
# CANNLab scenario:
cat /home/developer/Ascend/cann/${arch}-linux/ascend_toolkit_install.info

# Check the CANN ops package version information (default installation path)
# Docker and manual installation scenarios:
cat /usr/local/Ascend/cann/${arch}-linux/ascend_ops_install.info
# CANNLab scenario:
cat /home/developer/Ascend/cann/${arch}-linux/ascend_ops_install.info
```

`${arch}` can be obtained by running `uname -m` to query the current architecture, for example, aarch64 or x86_64.

**Check whether nnal is in place (needed only for "Compile and package together with the acceleration library")**

```shell
# The file is in place if either path is listed (/usr/local/Ascend is the default root installation path)
ls /usr/local/Ascend/nnal/atb/latest/atb/cxx_abi_0/lib/libtbe_adapter.so \
   /usr/local/Ascend/nnal/atb/latest/atb/cxx_abi_1/lib/libtbe_adapter.so
```

The nnal version must be exactly the same as the toolkit and ops versions; otherwise the acceleration library build fails with an ABI mismatch or link error.

#### Environment Variable Configuration

Select the appropriate command to make the environment variables take effect as needed.

```shell
# Default installation path, using the root user as an example (for non-root users, replace /usr/local with ${HOME})
source /usr/local/Ascend/cann/set_env.sh
# Specified installation path
# source ${install_path}/cann/set_env.sh
```

### Detailed CANN Installation Guide
You can refer to the [Huawei Ascend documentation](https://www.hiascend.com/document) > CANN Community Edition > Software Installation to view the CANN software installation guide. Select according to the machine, operating system, and use case, and then read the detailed installation steps.

### Tool Version Requirements and Installation

After installing CANN, you can install some tools to facilitate subsequent development. For details, see the following:

* [CANN Dependencies](https://www.hiascend.com/document/detail/en/CANNCommunityEdition/83RC1alpha002/softwareinst/instg/instg_0045.html?Mode=PmIns&InstallType=local&OS=Debian&Software=cannToolKit)
* [Post-Installation Operations for CANN](https://www.hiascend.com/document/detail/en/CANNCommunityEdition/83RC1alpha002/softwareinst/instg/instg_0094.html?Mode=PmIns&InstallType=local&OS=Debian&Software=cannToolKit)

## Getting Started
### Installation
No installation is required. It is compiled with the operator package. For details, see [Compilation Description](#compilation) and [Usage Description](#usage-instructions).

### Compilation

Use `scripts/build.sh` for compilation. Common targets include `testframework`, `example`, and `release`. You can run `bash scripts/build.sh help` to view all available parameters.

By default, the project enables `-Werror` for C++ code (see `cmake/host_config.cmake`). For GCC 12 or later, third-party header files such as PyTorch may generate warnings and cause compilation failures. In this case, add `--no_werror` to the end of the command. For example:

```shell
bash scripts/build.sh testframework --no_werror
bash scripts/build.sh example --no_werror
```

If `--no_werror` is not specified, the script automatically displays the preceding message after detecting that the GCC version is 12 or later.

### Usage Instructions
Two typical application scenarios:

- Scenario 1: Compile and package together with the acceleration library

  **Prerequisites (all are mandatory — complete them before running the steps below):**

  1. Use **Installation Method B (stable release line)** for CANN, with toolkit, chip-specific ops, and nnal all of the same version; **do not use the weekly version** (weekly builds may lack a matching nnal).
  2. The nnal package is installed and `libtbe_adapter.so` exists (see the "Installing the nnal Package" section above for the self-check command).
  3. `ATB_BUILD_DEPENDENCY_PATH` is set when needed (only required if nnal is installed in a **non-default path**):

     ```shell
     # 1) Determine the ABI used for the build: use cxx_abi_1 if PyTorch is not installed;
     #    if PyTorch is installed, the command below prints 1 for cxx_abi_1 and 0 for cxx_abi_0
     python3 -c "import torch; print(1 if torch.compiled_with_cxx11_abi() else 0)"

     # 2) Set it manually when nnal is installed in a non-default path (example: installed under ${HOME}/Ascend)
     export ATB_BUILD_DEPENDENCY_PATH=${HOME}/Ascend/nnal/atb/latest/atb/cxx_abi_1
     ```

     > If nnal is installed in the default path `/usr/local/Ascend/nnal/...`, this variable is **not required**; the acceleration library build script selects `cxx_abi_0` / `cxx_abi_1` automatically based on the detected ABI.

  4. If the acceleration library was previously built with the other ABI, clean the cache before rebuilding after switching ABI (`bash scripts/build.sh --clean-first`). Otherwise the stale CMake cache and the copied `libtbe_adapter.so` cause ABI mismatch or link errors.

  > Tip: If the conditions above are not met, the acceleration library build reports `libtbe_adapter.so does not exist, please set correct ATB_BUILD_DEPENDENCY_PATH` and later fails while linking `asdops.so` (`gmake Error 2`). In that case, re-check the prerequisites one by one.

In the following example, the Ascend Boost Comm and the acceleration library (Ascend Transformer Boost in this example) code is ready and stored in the same-level directory.

1.  Compile Ascend Boost Comm and copy the compilation output to the 3rdparty directory of the acceleration library.

    ```shell
    cd ascend-boost-comm
    bash scripts/build.sh testframework
    cp -r output/mki ../ascend-transformer-boost/3rdparty/
    ```

2.  Compile the acceleration library.

    ```shell
    cd ../ascend-transformer-boost/
    source scripts/set_env.sh
    bash scripts/build.sh testframework
    source output/atb/set_env.sh
    ```

3.  Run model or operator test cases.

- Scenario 2: Single-operator project
This scenario is for users who want to simply test a newly written single operator without building the full operator library. The following uses the `addcustom` operator in the `example/ directory` to illustrate the operator execution flow:
1. Implement the operator and write test cases by referring to the operator test cases in the example.
2. Compile Ascend Boost Comm with examples. When compiling examples for the first time, you need to compile testframework first, and then compile examples.

    ```shell
    cd ascend-boost-comm
    bash scripts/build.sh testframework
    bash scripts/build.sh example
    source output/mki/set_env.sh
    ```

3. Test the operator.

    ```shell
    python example/tests/pythontest/optest/test_addcustom.py
    ```

    Before running the code, ensure that the `source output/mki/set_env.sh` command has been executed and the CANN/NPU driver environment is available.

    To view `MKI_LOG` debug output from your operator, set the log level and output switch first. For details, see [Debug Logging](docs/custom_operator_example.md#debug-logging).

You can refer to this document to develop custom operators: [Example of Developing a Custom Operator](docs/custom_operator_example.md).

## Contributing

1.  Fork the repository.
2.  Modify and commit code.
3.  Create a pull request (PR).

For details, see [Contribution Guide](docs/contribution_guide.md).

## Reference Document
[CANN Community Edition Documentation](https://www.hiascend.com/document/detail/en/CANNCommunityEdition/83RC1alpha002/index/index.html)
Documents in this repository: [docs/](docs/) (including custom operator and contribution guide)
