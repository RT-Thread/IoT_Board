"""
uio 模块包含流类型 (类似文件) 对象和帮助函数。
"""

def open(name, mode='r', **kwargs) -> None:
    """打开一个文件，关联到内建函数open()。所有端口 (用于访问文件系统) 需要支持模式参数，但支持其他参数不同的端口。"""
    ...

class FileIO(...):
    """这个文件类型用二进制方式打开文件，等于使用open(name, “rb”)。 不应直接使用这个实例。"""
    ...

class TextIOWrapper(...):
    """这个类型以文本方式打开文件，等同于使用open(name, “rt”)不应直接使用这个实例。"""
    ...

class StringIO(string):
    """这个类型以文本方式打开文件，等同于使用open(name, “rt”)不应直接使用这个实例。"""
    ...

class BytesIO(string):
    """
    内存文件对象。StringIO 用于文本模式 I/O (用 “t” 打开文件)，BytesIO 用于二进制方式 (用 “b” 方式)。
    文件对象的初始内容可以用字符串参数指定（stringio用普通字符串，bytesio用bytes对象）。
    所有的文件方法，如 read(), write(), seek(), flush(), close() 都可以用在这些对象上。
    """
    def __init__(self) -> None:
    ...

    def getvalue() -> None:
        """获取缓存区内容。"""
        ...
