## **ustruct** – 打包和解包原始数据类型

**ustruct** 模块在 Python 值和以 Python 字节对象表示的 C 结构之间执行转换。

- 支持 size/byte 的前缀: @, <, >, !.
- 支持的格式代码: b, B, h, H, i, I, l, L, q, Q, s, P, f, d (最后2个需要支持浮点数).

### 函数

#### **ustruct.calcsize**(fmt)  
返回存放某一类型数据 fmt 需要的字节数。

```
fmt：数据类型
    b — 字节型
    B — 无符号字节型
    h — 短整型
    H — 无符号短整型
    i — 整型
    I — 无符号整型
    l — 整型
    L — 无符号整型
    q — 长整型
    Q — 无符号长整型
    f — 浮点型
    d — 双精度浮点型
    P — 无符号型
```

示例：

```python
>>> print(struct.calcsize("i"))
4
>>> print(struct.calcsize("B"))
1
```

#### **ustruct.pack**(fmt, v1, v2, ...)  
按照格式字符串 fmt 打包参数 v1, v2, ... 。返回值是参数打包后的字节对象。

```
fmt：同上
```

示例：

```python
>>> struct.pack("ii", 3, 2)
b'\x03\x00\x00\x00\x02\x00\x00\x00'
```

#### **ustruct.unpack**(fmt, data)  
从 fmt 中解包数据。返回值是解包后参数的元组。

```
data：要解压的字节对象
```

示例：

```python
>>> buf = struct.pack("bb", 1, 2)
>>> print(buf)
b'\x01\x02'
>>> print(struct.unpack("bb", buf))
(1, 2)
```

#### **ustruct.pack_into**(fmt, buffer, offset, v1, v2, ...)  
按照格式字符串 fmt 压缩参数 v1, v2, ... 到缓冲区 buffer，开始位置是 offset。当offset 为负数时，从缓冲区末尾开始计数。 

#### **ustruct.unpack_from**(fmt, data, offset=0)  
以 fmt 作为规则从 data 的 offset 位置开始解包数据，如果 offset 是负数就是从缓冲区末尾开始计算。返回值是解包后的参数元组。
```python
>>> buf = struct.pack("bb", 1, 2)
>>> print(struct.unpack("bb", buf))
(1, 2)
>>> print(struct.unpack_from("b", buf, 1))
(2,)
```
更多的内容可参考  [ustruct](http://docs.micropython.org/en/latest/pyboard/library/ustruct.html) 。
