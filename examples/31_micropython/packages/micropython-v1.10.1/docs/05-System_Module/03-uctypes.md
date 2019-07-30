## **uctypes** – 以结构化的方式访问二进制数据

uctypes 模块用来访问二进制数据结构，它提供 C 兼容的数据类型。

### 常量
- uctypes.LITTLE_ENDIAN — 小端压缩结构。
- uctypes.BIG_ENDIAN — 大端压缩结构类型。
- NATIVE —  mricopython 本地的存储类型


### 构造函数

#### class uctypes.struct(addr, descriptor, type)
将内存中以 c 形式打包的结构体或联合体转换为字典，并返回该字典。
```
addr：开始转换的地址
descriptor：转换描述符
格式："field_name":offset|uctypes.UINT32
offset：偏移量，
单位：字节、VOID、UINT8、INT8、UINT16、INT16、UINT32、INT32、UINT64、INT64、BFUINT8、BFINT8、BFUINT16、BFINT16、BFUINT32、BFINT32、BF_POS、BF_LEN、FLOAT32、FLOAT64、PTR、ARRAY
type：c 结构体或联合体存储类型，默认为本地存储类型
```

示例：

```python
>>> a = b"0123"
>>> s = uctypes.struct(uctypes.addressof(a), {"a": uctypes.UINT8 | 0, "b": uctypes.UINT16 | 1}, uctypes.LITTLE_ENDIAN)
>>> print(s)
<struct STRUCT 3ffc7360>
>>> print(s.a)
48
>>> s.a = 49
>>> print(a)
b'1123'
```

### 方法

#### **uctypes.sizeof**(struct)  
按字节返回数据的大小。参数可以是类或者数据对象 (或集合)。 
示例：
```python
>>> a = b"0123"
>>>b = uctypes.struct(uctypes.addressof(a), {"a": uctypes.UINT8 | 0, "b": uctypes.UINT16 | 1}, uctypes.LITTLE_ENDIAN)
>>> b.a
48
>>> print(uctypes.sizeof(b))
3
```

#### **uctypes.addressof**(obj)  
返回对象地址。参数需要是 bytes, bytearray 。 
示例：

```python
>>> a = b"0123"
>>> print(uctypes.addressof(a))
1073504048
```

#### **uctypes.bytes_at**(addr, size)  
捕捉从 addr 开始到 size 个地址偏移量结束的内存数据为 bytearray 对象并返回。 
示例：

```python
>>> a = b"0123"
>>>print( uctypes.bytes_at(uctypes.addressof(a), 4))
b'0123'
```

#### **uctypes.bytearray_at**(addr, size)  
捕捉给定大小和地址内存为 bytearray 对象。与 bytes_at() 函数不同的是，它可以被再次写入，可以访问给定地址的参数。 
示例：

```python
>>> a = b"0123"
>>> print(uctypes.bytearray_at(uctypes.addressof(a), 2))
bytearray(b'01')
```

更多内容可参考 [uctypes](http://docs.micropython.org/en/latest/pyboard/library/uctypes.html) 。
