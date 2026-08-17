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
This section provides example commands for quickly installing the CANN software. For more installation steps, see the [Detailed Installation Guide](#detailed-cann-installation-guide).

#### Installation preparation
For both online and offline installation, ensure that the Python environment and pip3 are available. Currently, CANN supports Python 3.7.x to 3.11.4.
For offline installation, go to [this website](https://www.hiascend.com/developer/download/community/result?module=cann) to download the CANN Toolkit software package that matches the current environment.

#### Installing CANN
```shell
# ${VERSION} indicates the version number in the actual package name on the download page, for example, 8.2.RC1 or 8.5.0. For details, see the official website.
chmod +x Ascend-cann-toolkit_${VERSION}_linux-$(arch).run
./Ascend-cann-toolkit_${VERSION}_linux-$(arch).run --install
```
#### Post-installation Configuration
Configure the environment variable script `set_env.sh`. The following example uses `${HOME}/Ascend` as the installation path.
```
source ${HOME}/Ascend/ascend-toolkit/set_env.sh
```
Install the Python third-party libraries required for service runtime (if installing as the root user, remove `--user` from the command).
```
pip3 install attrs cython 'numpy>=1.19.2,<=1.24.0' decorator sympy cffi pyyaml pathlib2 psutil protobuf==3.20.0 'scipy<1.11' requests absl-py --user
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
