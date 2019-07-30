## **uio** – 输入/输出流

**uio** 模块包含流类型 (类似文件) 对象和帮助函数。

### 函数

#### **uio.open**(name, mode='r', \*\*kwargs)

打开一个文件，关联到内建函数``open()``。所有端口 (用于访问文件系统) 需要支持模式参数，但支持其他参数不同的端口。

### 类

#### **class uio.FileIO**(...)
  这个文件类型用二进制方式打开文件，等于使用``open(name, “rb”)``。 不应直接使用这个实例。

#### **class uio.TextIOWrapper**(...)  
  这个类型以文本方式打开文件，等同于使用``open(name, “rt”)``不应直接使用这个实例。

#### **class uio.StringIO**([string])  

#### **class uio.BytesIO**([string])  
  内存文件对象。`StringIO` 用于文本模式 I/O (用 “t” 打开文件)，`BytesIO` 用于二进制方式 (用 “b” 方式)。文件对象的初始内容可以用字符串参数指定（`stringio`用普通字符串，`bytesio`用`bytes`对象）。所有的文件方法，如 `read(), write(), seek(), flush(), close()` 都可以用在这些对象上，包括下面方法:

#### **getvalue**()  
  获取缓存区内容。

更多内容可参考  [uio](http://docs.micropython.org/en/latest/pyboard/library/uio.html) 。
