"""
uctypes 模块用来访问二进制数据结构，它提供 C 兼容的数据类型。
"""

LITTLE_ENDIAN  = ...  # type: int
BIG_ENDIAN  = ...  # type: int
NATIVE  = ...  # type: int

class struct(addr, descriptor, type):
    """
    将内存中以 c 形式打包的结构体或联合体转换为字典，并返回该字典。

    - addr：开始转换的地址
    - descriptor：转换描述符
    格式："field_name":offset|uctypes.UINT32
    - offset：偏移量，
    单位：字节、VOID、UINT8、INT8、UINT16、INT16、UINT32、INT32、UINT64、INT64、BFUINT8、BFINT8、BFUINT16、BFINT16、BFUINT32、BFINT32、BF_POS、BF_LEN、FLOAT32、FLOAT64、PTR、ARRAY
    - type：c 结构体或联合体存储类型，默认为本地存储类型
    示例：

    - a = b"0123"
    - s = uctypes.struct(uctypes.addressof(a), {"a": uctypes.UINT8 | 0, "b": uctypes.UINT16 | 1}, uctypes.LITTLE_ENDIAN)
    - print(s)
    - <struct STRUCT 3ffc7360>
    - print(s.a)
    - 48
    - s.a = 49
    - print(a)
    - b'1123'
    """
    def __init__(self) -> None:
    ...

    def sizeof(struct) -> None:
        """
        按字节返回数据的大小。参数可以是类或者数据对象 (或集合)。 示例：

        - a = b"0123"
        - b = uctypes.struct(uctypes.addressof(a), {"a": uctypes.UINT8 | 0, "b": uctypes.UINT16 | 1}, uctypes.LITTLE_ENDIAN)
        - b.a
        - 48
        - print(uctypes.sizeof(b))
        - 3
        """
        ...

    def addressof(obj) -> None:
        """
        返回对象地址。参数需要是 bytes, bytearray 。 示例：

        - a = b"0123"
        - print(uctypes.addressof(a))
        - 1073504048
        """
        ...

    def bytes_at(addr, size)-> None:
        """
        捕捉从 addr 开始到 size 个地址偏移量结束的内存数据为 bytearray 对象并返回。 示例：

        - a = b"0123"
        - print( uctypes.bytes_at(uctypes.addressof(a), 4))
        - b'0123'
        """
        ...

    def bytearray_at(addr, size) -> None:
        """
        捕捉给定大小和地址内存为 bytearray 对象。与 bytes_at() 函数不同的是，它可以被再次写入，可以访问给定地址的参数。 示例：

        - a = b"0123"
        - print(uctypes.bytearray_at(uctypes.addressof(a), 2))
        - bytearray(b'01')
        """
        ...


