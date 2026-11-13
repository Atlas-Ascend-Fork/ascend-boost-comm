# object文件
原始object文件，由毕昇C++编译生成，或者位于TBE二进制包里

# bin文件
`Mki::BinFile` 将文本头部和对象的原始字节连续写入文件。字段定义见
[`bin_file.h`](../src/include/mki/utils/bin_file/bin_file.h)，读写实现见
[`bin_file.cpp`](../src/utils/bin_file/bin_file.cpp)。

头部每行采用 `名称=值\n` 格式，名称区分大小写，等号两侧不加空格。

| 字段 | 含义 |
| --- | --- |
| `$Version` | 写入器输出的格式版本，当前为 `1.0` |
| `$Object.Count` | 对象数量；当前读取器忽略此字段 |
| `$Object.Length` | 头部之后全部对象数据的总字节数，不包含头部 |
| `$Object.<对象名>` | 对象的 `偏移,长度`，均为十进制字节数；偏移从对象数据区起点计算 |
| `$End` | 头部结束标记；写入器输出 `$End=1`，其换行之后立即是对象数据 |

通过 `AddAttr` 添加的自定义属性写在 `$Object.Length` 与对象条目之间。
通过 `AddObject` 添加的对象按添加顺序连续存储。

以下仅演示容器布局：对象 `object0` 的内容是三个字节 `ABC`，对象 `object1`
的内容是两个字节 `DE`，因此总长度为五个字节，第二个对象的偏移为三个字节。

```text
$Version=1.0
$Object.Count=2
$Object.Length=5
$Object.object0=0,3
$Object.object1=3,2
$End=1
ABCDE
```

上例实际文件在 `E` 后结束；代码块末尾的排版换行不属于对象数据。
真实对象数据可包含任意字节，不要求是文本。

# fusionbin文件
若使用 `BinFile` 将多个bin文件作为对象打包，外层仍使用上述 `$Object.*`
头部，每个对象的内容为对应bin文件的完整字节。偏移与长度描述外层数据区，
应根据内层文件的实际字节数计算。`BinFile` 没有单独的 `bin.count` 或
`binN.offset` 系统字段。
