# Ascend Boost Comm

简体中文 | [English](./README_en.md)

🔥 [2025/09] Ascend Boost Comm项目首次上线。
## 什么是Ascend Boost Comm
### Ascend Boost Comm介绍
领域加速库公共组件，Ascend Boost Comm。它统一定义了算子调用的L0级接口。南向对接不同组织开发的算子库，北向支撑不同加速库应用，实现M x N算子能力复用。

### 软件架构
软件架构说明
1. 调用关系
领域加速库（[Ascend Transformer Boost加速库(ATB)](https://gitcode.com/cann/ascend-transformer-boost)、信号加速库等） --> Ascend Boost Comm

### Ascend Boost Comm仓介绍

Ascend Boost Comm库的目录结构如下

```
ascend-boost-comm
├── cmake                 // 编译和链接相关配置文件
├── configs               // 构建相关配置文件
├── docs                  // 文档文件存放目录
├── example               // 算子调用示例代码
├── scripts               // 脚本文件存放目录
├── src                   // 主体源代码目录
│   ├── include           // 存放公共头文件
│   ├── mki_loader        // 算子加载相关逻辑代码
│   ├── schedule          // 算子调度相关逻辑代码
│   ├── utils             // 工具类存放目录
│   └── CMakeLists.txt
└── tests                 // 测试代码
```

## 产品支持情况

本仓支持的昇腾硬件架构如下表所示。

| 硬件型号 | 是否支持 | 说明 |
|---|---|---|
| Atlas A2系列产品 | √ | — |
| Atlas A3系列产品 | √ | — |
| Atlas推理系列产品 | √ | — |

## 环境构建
### 基础环境依赖

**编译构建依赖（项目本身必需）**：

| 组件 | 版本要求 | 说明 |
|---|---|---|
| Python | 3.10.x 或 3.11.x | 构建时python脚本运行依赖 |
| cmake | ≥ 3.20 | 构建系统依赖 |
| gcc/g++ | 推荐 7.3.1–11.x | **GCC ≥ 12** 时编译需加 `--no_werror`（见[编译说明](#编译说明)） |

**运行示例 / 测试依赖（仅当需要编译运行 测试框架 或 Examples 时安装）**：

| 组件 | 版本要求 | 说明 |
|---|---|---|
| PyTorch | >= 2.1.0 | 须与 CANN 及 torch_npu 版本配套（见下方说明） |
| torch_npu（Ascend Extension for PyTorch） | 见官方文档 | 须与 CANN、torch 版本配套，见下 |

> **说明**：Ascend Boost Comm 库本身（编译构建）不依赖 PyTorch、torch_npu 或 numpy。上述组件仅在运行 `example/`、`tests/` 目录下的示例和测试用例时才需要安装。

**PyTorch / torch_npu 安装与版本**：请先完成 CANN Toolkit 安装，再按 [Ascend Extension for PyTorch 开发文档](https://www.hiascend.com/document/detail/zh/Pytorch) 查阅 **「版本说明」** 与 **「软件安装」**，选择与您所用 CANN 版本以及PyTorch版本匹配的 torch_npu 进行安装。

> **示例**：假设使用 CANN 9.0.0 和 PyTorch 2.7.1，可通过以下命令安装：
> ```shell
> pip install torch==2.7.1 torch-npu==2.7.1.post4
> ```

**与加速库一起编译出包的附加依赖（仅「使用说明」中的『与加速库一起编译出包使用』需要）**：

| 组件 | 版本要求 | 说明 |
|---|---|---|
| nnal（Ascend-cann-nnal） | **须与toolkit、ops的版本号完全一致** | 提供`libtbe_adapter.so`，加速库（本仓以ATB为例）编译时必需 |

> ⚠️ **重要前提**：CANN weekly / 主线构建的toolkit **可能没有同版本的nnal软件包**。若您要按「使用说明」中的『与加速库一起编译出包使用』操作，请改用toolkit、ops、nnal三个包**统一版本**的正式发布版本（例如统一使用9.2.0），即下文的**安装方式B**；否则加速库编译时会因缺少`libtbe_adapter.so`而失败。

### 快速安装CANN软件

对于有昇腾设备的开发者，若您想手动搭建昇腾环境，请参考下述步骤。

#### 安装软件

**安装方式A：基于CANN weekly版本体验最新master能力或开发**

请参考[CANN快速安装](https://hiascend.com/cann/download?versionId=802&ids=d806%2Ch0501%2Ch0601%2Ch0703&currentTab=1)，选择Weekly版本，并根据产品系列、CPU架构、操作系统等下载对应包，最后参考页面提供的命令完成安装。

> ℹ️ weekly版本适用于体验本仓最新master能力、以及只编译本仓自身的场景（即「使用说明」中的『单算子工程』）。weekly构建的toolkit可能暂无同版本的nnal软件包，因此**不建议**用它执行「与加速库一起编译出包使用」，请改用下方的安装方式B。

**安装方式B：基于CANN稳定版本体验分支能力或开发（推荐）**

请参考[CANN快速安装](https://hiascend.com/cann/download?versionId=810&ids=d806%2Ch0501%2Ch0601%2Ch0702&currentTab=0)，选择稳定版本（仅支持CANN 8.5.0及后续版本），并根据产品系列、CPU架构、操作系统等下载对应包，最后参考页面提供的命令完成安装。

稳定版本线同时提供toolkit、各芯片ops与nnal三个包，可满足「与加速库一起编译出包使用」对`libtbe_adapter.so`的依赖。

#### 安装nnal软件包（仅「与加速库一起编译出包使用」需要）

若您要执行「使用说明」中的『与加速库一起编译出包使用』，除toolkit与ops外还需安装nnal软件包，其版本号必须与toolkit、ops **完全一致**。

nnal软件包可在[CANN下载页](https://www.hiascend.com/developer/download/community)获取：选择产品系列与型号、选择**与已安装toolkit相同的解决方案版本**，在CANN区域选择`Ascend-cann-nnal_${VERSION}_linux-$(arch).run`下载。

```shell
# 包名形如：Ascend-cann-nnal_${VERSION}_linux-$(arch).run
chmod +x Ascend-cann-nnal_${VERSION}_linux-$(arch).run
./Ascend-cann-nnal_${VERSION}_linux-$(arch).run --check     # 校验包完整性与一致性
./Ascend-cann-nnal_${VERSION}_linux-$(arch).run --install   # 出现 "xxx install success!" 即安装成功
```

安装后先做一次5秒自检，确认`libtbe_adapter.so`已就位（`/usr/local/Ascend`为root默认安装路径，非root用户请替换为`${HOME}/Ascend`）：

```shell
ls /usr/local/Ascend/nnal/atb/latest/atb/cxx_abi_0/lib/libtbe_adapter.so \
   /usr/local/Ascend/nnal/atb/latest/atb/cxx_abi_1/lib/libtbe_adapter.so
```

两个路径**至少要有一个存在**：`cxx_abi_1`对应`_GLIBCXX_USE_CXX11_ABI=1`，`cxx_abi_0`对应`=0`。具体用哪个由编译时的PyTorch ABI决定（见下方「使用说明」）。若两个路径都不存在，说明nnal未安装或安装到了非默认路径，请按`nnal安装在非默认路径`的情况处理（设置`ATB_BUILD_DEPENDENCY_PATH`）。

#### 环境验证

安装完CANN包后，需验证环境和驱动是否正常。

**检查NPU设备**

```shell
# 运行npu-smi，若能正常显示设备信息，则驱动正常
npu-smi info
```

**检查CANN版本**

```shell
# 查看CANN toolkit包版本信息（默认路径安装）
# docker和手动安装场景：
cat /usr/local/Ascend/cann/${arch}-linux/ascend_toolkit_install.info
# CANNLab场景：
cat /home/developer/Ascend/cann/${arch}-linux/ascend_toolkit_install.info

# 查看CANN ops包版本信息（默认路径安装）
# docker和手动安装场景：
cat /usr/local/Ascend/cann/${arch}-linux/ascend_ops_install.info
# CANNLab场景：
cat /home/developer/Ascend/cann/${arch}-linux/ascend_ops_install.info
```

其中`${arch}`可通过`uname -m`查询当前架构，如aarch64、x86_64。

**检查nnal是否就位（仅「与加速库一起编译出包使用」需要）**

```shell
# 任一路径能列出libtbe_adapter.so即为就位（/usr/local/Ascend为root默认安装路径）
ls /usr/local/Ascend/nnal/atb/latest/atb/cxx_abi_0/lib/libtbe_adapter.so \
   /usr/local/Ascend/nnal/atb/latest/atb/cxx_abi_1/lib/libtbe_adapter.so
```

nnal版本号需与toolkit、ops完全一致，不一致时加速库编译会出现ABI不匹配或链接错误。

#### 环境变量配置

按需选择合适的命令使环境变量生效。

```shell
# 默认路径安装，以root用户为例（非root用户，将/usr/local替换为${HOME}）
source /usr/local/Ascend/cann/set_env.sh
# 指定路径安装
# source ${install_path}/cann/set_env.sh
```

### CANN详细安装指南
开发者可访问[昇腾文档-昇腾社区](https://www.hiascend.com/document)->CANN社区版->软件安装，查看CANN软件安装引导，根据机器环境、操作系统和业务场景选择后阅读详细安装步骤。

### 基础工具版本要求与安装

安装CANN之后，您可安装一些工具方便后续开发，参见以下内容：

* [CANN依赖列表](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/83RC1alpha002/softwareinst/instg/instg_0045.html?Mode=PmIns&InstallType=local&OS=Debian&Software=cannToolKit)
* [CANN安装后操作](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/83RC1alpha002/softwareinst/instg/instg_0094.html?Mode=PmIns&InstallType=local&OS=Debian&Software=cannToolKit)

## 快速上手
### 安装教程
无需安装， 直接与算子包一起编译。见[编译说明](#编译说明)以及[使用说明](#使用说明)。

### 编译说明

编译使用 `scripts/build.sh`，常用目标包括 `testframework`、`example`、`release` 等；完整参数可执行 `bash scripts/build.sh help` 查看。

工程默认对 C++ 代码启用 `-Werror`（见 `cmake/host_config.cmake`）。在 **GCC 12 及以上** 环境中，PyTorch 等第三方头文件可能产生警告并导致编译失败，请在命令末尾追加 **`--no_werror`**，例如：

```shell
bash scripts/build.sh testframework --no_werror
bash scripts/build.sh example --no_werror
```

未指定 `--no_werror` 时，脚本会在检测到 GCC ≥ 12 后自动打印上述提示。

### 使用说明
两种典型使用场景：

- 场景1. 与加速库一起编译出包使用

  **前置条件（缺一不可，请先完成再执行下方步骤）**：

  1. CANN环境采用**安装方式B（稳定版本线）**，且toolkit、各芯片ops、nnal三个包版本号完全一致；**不要使用weekly版本**（weekly可能无同版本nnal）。
  2. 已安装nnal软件包，`libtbe_adapter.so`存在（自检命令见上文『安装nnal软件包』小节）。
  3. 已按需设置`ATB_BUILD_DEPENDENCY_PATH`（仅当nnal安装在**非默认路径**时需要）：

     ```shell
     # 1) 先确定编译使用的ABI：未安装PyTorch用cxx_abi_1；
     #    已安装PyTorch时，下面命令输出1用cxx_abi_1，输出0用cxx_abi_0
     python3 -c "import torch; print(1 if torch.compiled_with_cxx11_abi() else 0)"

     # 2) nnal安装在非默认路径时手动指定（示例：安装在${HOME}/Ascend）
     export ATB_BUILD_DEPENDENCY_PATH=${HOME}/Ascend/nnal/atb/latest/atb/cxx_abi_1
     ```

     > nnal安装在默认路径`/usr/local/Ascend/nnal/...`时**无需设置**该变量，加速库编译脚本会按检测到的ABI自动选择`cxx_abi_0` / `cxx_abi_1`。

  4. 若此前用另一种ABI编译过加速库，切换ABI后需先清理缓存再重编（`bash scripts/build.sh --clean-first`），否则残留的CMake缓存与已拷贝的`libtbe_adapter.so`会导致ABI不匹配或链接错误。

  > 提示：若未满足上述条件，加速库编译会报`libtbe_adapter.so does not exist, please set correct ATB_BUILD_DEPENDENCY_PATH`，并在后续链接`asdops.so`时失败（`gmake Error 2`），此时请回到前置条件逐项核对。

下面假设 Ascend Boost Comm、加速库（本例中是Ascend Transformer Boost）代码都已经准备好，且处于同一级目录下。

1.  编译Ascend Boost Comm, 并将编译产物拷贝到加速库的3rdparty目录

    ```shell
    cd ascend-boost-comm
    bash scripts/build.sh testframework
    cp -r output/mki ../ascend-transformer-boost/3rdparty/
    ```

2.  编译加速库

    ```shell
    cd ../ascend-transformer-boost/
    source scripts/set_env.sh
    bash scripts/build.sh testframework
    source output/atb/set_env.sh
    ```

3.  运行模型或算子测试用例

- 场景2：单算子工程
适用于仅简单测试新写的单个算子，而不想构建完整算子库的用户，下面以example目录中的addcustom算子为例介绍算子运行流程：
1. 参照example中的算子用例，实现算子并编写测试用例
2. 编译带example的Ascend Boost Comm，第一次编译example时，需要先编译testframework，再编译example

    ```shell
    cd ascend-boost-comm
    bash scripts/build.sh testframework
    bash scripts/build.sh example
    source output/mki/set_env.sh
    ```

3. 测试算子

    ```shell
    python example/tests/pythontest/optest/test_addcustom.py
    ```

    运行前请确认已执行 `source output/mki/set_env.sh`，且 CANN / NPU 驱动环境可用。

    如需查看算子中的 `MKI_LOG` 调试输出，需先设置日志级别与输出开关，详见[调试日志](docs/自定义算子开发示例.md#调试日志)。

您可参考该文档进行自定义算子开发：[自定义算子开发示例](docs/自定义算子开发示例.md)

## 参与贡献

1.  fork仓库
2.  修改并提交代码
3.  新建 Pull-Request

详细步骤可参考[贡献指南](docs/贡献指南.md)

## 参考文档
**[CANN社区版文档](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/83RC1alpha002/index/index.html)**
**本仓文档**：[docs/](docs/)（含自定义算子、贡献指南等）
