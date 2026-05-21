# Ascend Boost Comm

🔥 [2025/09] Ascend Boost Comm项目首次上线。
## 一、什么是Ascend Boost Comm
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
├── document              // 文档文件存放目录
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

## 二、环境构建
### 基础环境依赖
Ascend Boost Comm库依赖 Python、PyTorch、torch_npu 以及本地编译工具链，版本要求如下：

| 组件 | 版本要求 | 说明 |
|---|---|---|
| Python | 3.10.x 或 3.11.x | |
| PyTorch | >= 2.1.0 | |
| torch_npu（Ascend Extension for PyTorch） | 见官方文档 | 须与 CANN、torch 版本配套，见下 |
| cmake | ≥ 3.20 | |
| gcc/g++ | 推荐 7.3.1–11.x | **GCC ≥ 12** 时编译需加 `--no_werror`（见[编译说明](#编译说明)） |

**PyTorch / torch_npu 安装与版本**：请先完成 CANN Toolkit 安装，再按 [Ascend Extension for PyTorch 开发文档](https://www.hiascend.com/document/detail/zh/Pytorch) 查阅 **「版本说明」** 与 **「软件安装」**，选择与您所用 CANN 版本以及PyTorch版本匹配的 torch_npu 进行安装。

### 快速安装CANN软件
本节提供快速安装CANN软件的示例命令，更多安装步骤请参考[详细安装指南](#cann详细安装指南)。

#### 安装前准备
在线安装和离线安装时，需确保已具备Python环境及pip3，当前CANN支持Python3.7.x至3.11.4版本。
离线安装时，请单击[获取链接](https://www.hiascend.com/developer/download/community/result?module=cann)下载与当前环境匹配的 CANN Toolkit 软件包。

#### 安装CANN
```shell
# ${VERSION} 为下载页实际包名中的版本号，例如 8.2.RC1、8.5.0 等，请以官网为准
chmod +x Ascend-cann-toolkit_${VERSION}_linux-$(arch).run
./Ascend-cann-toolkit_${VERSION}_linux-$(arch).run --install
```
#### 安装后配置
配置环境变量脚本set_env.sh，当前安装路径以${HOME}/Ascend为例。
```
source ${HOME}/Ascend/ascend-toolkit/set_env.sh
```  
安装业务运行时依赖的Python第三方库（如果使用root用户安装，请将命令中的--user删除）。
```
pip3 install attrs cython 'numpy>=1.19.2,<=1.24.0' decorator sympy cffi pyyaml pathlib2 psutil protobuf==3.20.0 'scipy<1.11' requests absl-py --user
```

### CANN详细安装指南 
开发者可访问[昇腾文档-昇腾社区](https://www.hiascend.com/document)->CANN社区版->软件安装，查看CANN软件安装引导，根据机器环境、操作系统和业务场景选择后阅读详细安装步骤。

### 基础工具版本要求与安装

安装CANN之后，您可安装一些工具方便后续开发，参见以下内容：

* [CANN依赖列表](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/83RC1alpha002/softwareinst/instg/instg_0045.html?Mode=PmIns&InstallType=local&OS=Debian&Software=cannToolKit)
* [CANN安装后操作](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/83RC1alpha002/softwareinst/instg/instg_0094.html?Mode=PmIns&InstallType=local&OS=Debian&Software=cannToolKit)

## 三、快速上手
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
下面假设 Ascend Boost Comm、加速库（本例中是Ascend Transformer Boost）代码都已经准备好，且处于同一级目录下。
1.  使用算子命名空间作为参数， 编译Ascend Boost Comm, 并将编译产物拷贝到加速库的3rdparty目录：在这里例子中， 命名空间参数是AtbOps

    ```shell
    cd ascend-boost-comm
    bash scripts/build.sh testframework
    cp -r output/mki ../ascend-transformer-boost/3rdparty/
    ```

2.  编译加速库

    ```shell
    cd ascend-transformer-boost/
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

您可参考该文档进行自定义算子开发：[自定义算子开发示例](document/自定义算子开发示例.md)

## 四、参与贡献
 
1.  fork仓库
2.  修改并提交代码
3.  新建 Pull-Request

详细步骤可参考[贡献指南](document/贡献指南.md)

## 五、参考文档
**[CANN社区版文档](https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/83RC1alpha002/index/index.html)**  
**本仓文档**：[document/](document/)（含自定义算子、贡献指南等）
