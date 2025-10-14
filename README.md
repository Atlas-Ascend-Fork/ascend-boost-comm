# Ascend-Boost-Comm

#### 介绍
领域加速库公共组件，Ascend Boost Comm，缩写ABC。它统一定义了算子调用的L0级接口。南向对接不同组织开发的算子库，北向支撑不同加速库应用，实现M x N算子能力复用。 

#### 软件架构
软件架构说明
1. 调用关系  
[Ascend Transformer Boost加速库(ATB)](https://gitcode.com/cann/ascend-transformer-boost) --> ABC

#### 安装教程
无需安装， 直接与算子包一起编译。见下一章使用说明。  

#### 使用说明
两种典型使用场景：

- 场景1. 与加速库一起编译出包使用
下面假设 Ascend Boost Comm、加速库（本例中是Ascend Transformer Boost）代码都已经准备好，且处于同一级目录下。
1.  使用算子命名空间作为参数， 编译abc, 并将编译产物拷贝到加速库的3rdparty目录：在这里例子中， 命名空间参数是AtbOps

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

- 场景2：单算子工程（待补充）
适用于仅简单测试新写的单个算子， 而不想构建完整算子库的用户。
1. 参照example中的算子用例， 实现算子并编写测试用例
2. 编译带example的abc
3. 测试算子 

#### 参与贡献

1.  Fork 本仓库
- git clone https://gitcode.com/cann/ascend-boost-comm.git
2.  新建开发分支
- git checkout -B dev
3.  提交代码
- git push origin dev:dev
4.  新建 Pull Request

