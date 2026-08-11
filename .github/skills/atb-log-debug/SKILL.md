---
name: atb-log-debug
description: '调试 ATB/MKI 日志系统导致编译失败的问题。Use when: ATB 编译报错、日志文件过多、stdout 被路径字符串污染、build.sh $(...) 捕获异常、ASCEND_PROCESS_LOG_PATH 配置问题、log_sink_file 诊断信息误入 stdout'
argument-hint: '[问题描述]'
---

# ATB 日志系统调试

## 何时使用

- ATB/MKI 编译过程出现无法解释的错误
- `build.sh` 中 `$(python3 ...)` 或 `$(...)` 捕获到异常内容
- `ASCEND_PROCESS_LOG_PATH` 目录下日志文件累积超过 50 个
- cmake 收到垃圾路径导致 `configure` 或 `build` 失败

## 核心知识点

### 日志路径解析链（优先级从低到高）

| 优先级 | 环境变量 | 路径 |
|--------|----------|------|
| 1 | `$HOME` | `$HOME/ascend/log/atb/` |
| 2 | `$ASCEND_WORK_PATH` | `$ASCEND_WORK_PATH/log/atb/` |
| 3 | `$ASCEND_PROCESS_LOG_PATH` | `$ASCEND_PROCESS_LOG_PATH/atb/` |

### 关键常量

| 常量 | 位置 | 值 | 作用 |
|------|------|-----|------|
| `MAX_LOG_FILE_COUNT` | `src/utils/log/log_sink_file.cpp:37` | 50 | 日志文件回滚上限 |
| `MAX_FILE_NUM` | `src/include/mki/utils/file_system/file_system.h:18` | 100 | 目录扫描硬限制 |

### 关键文件

| 文件 | 作用 |
|------|------|
| `src/utils/log/log_sink_file.cpp` | 文件日志 sink，含 `DeleteOldestFile()` |
| `src/utils/log/log_sink_stdout.cpp` | stdout 日志 sink（正常通道，勿改） |
| `src/utils/log/log_stream.cpp` | 日志格式化与输出 |
| `src/utils/log/log_core.cpp` | 日志核心，初始化 sinks |
| `scripts/build.sh` | 构建脚本，含 `$(...)` 捕获点 |

## 典型问题：日志文件过多导致编译失败

### 因果链

```
120个日志文件 > MAX_LOG_FILE_COUNT(50)
  → DeleteOldestFile() 触发清理
  → std::cout 输出 70 行文件路径到 stdout（原版代码）
  → build.sh 的 $(...) 捕获 stdout
  → cmake 变量被路径字符串污染
  → 编译失败
```

### 诊断步骤

1. 检查日志文件数量：
   ```bash
   ls $ASCEND_PROCESS_LOG_PATH/atb/*.log | wc -l
   ```

2. 检查 `std::cout` 是否被用于诊断输出：
   ```bash
   grep -n 'std::cout' src/utils/log/log_sink_file.cpp src/utils/log/log_stream.cpp
   ```

3. 验证 stdout 是否被污染：
   ```bash
   result=$(./some_binary_using_mki 2>/dev/null)
   echo "$result" | wc -l  # 期望1行，异常则为N行
   ```

### 修复方案

将诊断信息从 `std::cout` 改为 `std::cerr`：

- `src/utils/log/log_sink_file.cpp`: 10 处 `std::cout` → `std::cerr`
- `src/utils/log/log_stream.cpp`: 2 处 `std::cout` → `std::cerr`
- **不要改** `log_sink_stdout.cpp`（那是正常的 stdout 日志通道）

### 验证方法

```bash
# 1. 构造场景：创建 120 个模拟日志文件
for i in $(seq 1 120); do
  touch "$ASCEND_PROCESS_LOG_PATH/atb/atb_${i}_202608101200$(printf '%02d' $i).log"
done

# 2. 运行触发日志的程序，分离 stdout/stderr
./test_binary 1>/tmp/stdout.txt 2>/tmp/stderr.txt

# 3. 验证 stdout 干净
wc -l < /tmp/stdout.txt  # 修复后应为 1 行
```

## Build 脚本中的危险模式

`scripts/build.sh` 中以下位置使用 `$(...)` 捕获 stdout，容易受污染：

- `fn_init_pytorch_env()`: `$(python3 -c '...')` 设置 Python 路径
- `fn_init_use_cxx11_abi()`: `$(python3 -c 'import torch...')` 检测 ABI

任何加载 `libmki.so` 的进程都可能触发日志输出，污染这些捕获点。
