# Ascend Boost Comm

English | [简体中文](./README.md)

🔥 [2025/09] Ascend Boost Comm project was first launched.
## 1. What Is Ascend Boost Comm?
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
├── document              // Script file directory
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

## 2. Product Support

The following table lists the Ascend hardware architectures supported by this repository.

| Hardware Model| Supported| Description|
|---|---|---|
| Atlas A2 training products/Atlas A2 inference products| √ | — |
| Atlas A3 training products/Atlas A3 inference products| √ | — |
| Atlas inference products| √ | — |

## 3. Environment Setup
### Basic Environment Dependencies

**Compilation dependencies (mandatory for the project):**

| Component| Version Requirement| Description|
|---|---|---|
| Python | 3.10.x or 3.11.x| Dependency for running Python scripts during compilation|
| cmake | ≥ 3.20 | |
| gcc/g++ | Recommended: 7.3.1-11.x| If GCC ≥ 12, add `--no_werror` to the compilation command. (See [Compilation Description](#compilation-description).)|

**Runtime example / test dependencies (install only when compiling and running the test framework or examples):**

| Component| Version Requirement| Description|
|---|---|---|
| PyTorch | >= 2.1.0 | |
| torch_npu (Ascend Extension for PyTorch)| See the official document.| Must be compatible with CANN and torch versions. For details, see the following instruction.|

> **Note**: The Ascend Boost Comm library (compilation and build) does not depend on PyTorch, torch_npu, or NumPy. The preceding components need to be installed only when you run the examples and test cases in the `example/` and `tests/` directories.

PyTorch/torch_npu installation and version: Install the CANN Toolkit first, then refer to the "Version Description" and "Software Installation" sections in the [Ascend Extension for PyTorch Development Documentation](https://www.hiascend.com/document/detail/en/Pytorch), and select the torch_npu that matches your CANN and PyTorch versions for installation.

> Example: If CANN 9.0.0 and PyTorch 2.7.1 are used, run the following command to install torch_npu:
> ```shell
> pip install torch==2.7.1 torch-npu==2.7.1.post4
> ```

### Quick Installation of CANN Software
#### Manual Installation
For developers with Ascend devices who want to manually set up the Ascend environment, perform the following steps.

#### Installing the Software
**Scenario 1: Experience the latest master capabilities or develop based on the CANN weekly version**

See [CANN Quick Installation](https://hiascend.com/cann/download?versionId=794&ids=d803,h0501,h0601,h0703&currentTab=1), select the Weekly version, and download the corresponding package based on the product series, CPU architecture, and operating system. Then complete the installation by following the commands provided on the page.

**Scenario 2: Experience branch capabilities or develop based on the CANN stable version**

See [CANN Quick Installation](https://hiascend.com/cann/download?versionId=794&ids=d803,h0501,h0601,h0703&currentTab=1), select the stable version (only CANN 8.5.0 and later versions are supported), and download the corresponding package based on the product series, CPU architecture, and operating system. Then complete the installation by following the commands provided on the page.

#### Installing Basic Dependencies
The basic dependencies of this project are as follows. Pay attention to the version requirements.

* python >= 3.7.0 (recommended: <= 3.10)
* gcc >= 7.3.0
* cmake >= 3.16.0
* pigz (optional; improves packaging speed after installation, recommended version: >= 2.4)
* dos2unix
* make
* patch
* googletest (required only for running UT; recommended version: [release-1.11.0](https://github.com/google/googletest/releases/tag/release-1.11.0))

The preceding dependencies can be installed with one click using the project script. Perform the following steps:

**1. Download the source code**

Download the branch source code matching the CANN version. Run the following command and replace ${tag_version} with the branch tag name.

```shell
git clone -b ${tag_version} https://gitcode.com/cann/ops-nn.git
```

**2. Install dependencies**

First, use install_deps.sh in the project root directory to install the preceding dependencies with one click. If an unsupported system is encountered, adapt the script by referring to this file.

```shell
bash install_deps.sh
```

Then install the Python third-party library dependencies based on the usage scenario:

* If you only compile the project, install the dependencies in requirements.txt in the project root directory.
  ```shell
  pip3 install -r requirements.txt
  ```
* If you run UT test cases, install the dependencies in tests/requirements.txt. This file already contains the dependency constraints of requirements.txt in the project root directory, so there is no need to run the preceding command again.
  ```shell
  pip3 install -r tests/requirements.txt
  ```

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

${arch} can be obtained by running uname -m to query the current architecture, for example, aarch64 or x86_64.

#### Environment Variable Configuration
Select the appropriate command to make the environment variables take effect as needed.

```shell
# Default installation path, using the root user as an example (for non-root users, replace /usr/local with ${HOME})
source /usr/local/Ascend/cann/set_env.sh
# Specified installation path
# source ${install_path}/cann/set_env.sh
```

### Detailed CANN Installation Guide
You can refer to the [Ascend documentation](https://www.hiascend.com/document) > CANN Community Edition > Software Installation to view the CANN software installation guide. Select according to the machine, operating system, and use case, and then read the detailed installation steps.

### Tool Version Requirements and Installation

After installing CANN, you can install some tools to facilitate subsequent development. For details, see the following:

* [CANN Dependencies](https://www.hiascend.com/document/detail/en/CANNCommunityEdition/83RC1alpha002/softwareinst/instg/instg_0045.html?Mode=PmIns&InstallType=local&OS=Debian&Software=cannToolKit)
* [Post-Installation Operations for CANN](https://www.hiascend.com/document/detail/en/CANNCommunityEdition/83RC1alpha002/softwareinst/instg/instg_0094.html?Mode=PmIns&InstallType=local&OS=Debian&Software=cannToolKit)

## 4. Getting Started
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
In the following example, the Ascend Boost Comm and the acceleration library (Ascend Transformer Boost in this example) code is ready and stored in the same-level directory.
1.  Use the operator namespace as a parameter to compile Ascend Boost Comm and copy the compilation output to the 3rdparty directory of the acceleration library. In this example, the namespace parameter is AtbOps.

    ```shell
    cd ascend-boost-comm
    bash scripts/build.sh testframework
    cp -r output/mki ../ascend-transformer-boost/3rdparty/
    ```

2.  Compile the acceleration library.

    ```shell
    cd ascend-transformer-boost/
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

You can refer to this document to develop custom operators: [Example of Developing a Custom Operator](document/custom_operator_example.md).

## 5. Contributing

1.  Fork the repository.
2.  Modify and commit code.
3.  Create a pull request (PR).

For details, see [Contribution Guide](document/contribution_guide.md).

## 6. Reference Document
[CANN Community Edition Documentation](https://www.hiascend.com/document/detail/en/CANNCommunityEdition/83RC1alpha002/index/index.html)
Documents in this repository: [document/](document/) (including custom operator and contribution guide)
