## **sys** – 系统特有功能函数

**sys** 模块提供系统特有的功能。

### 函数

#### **sys.exit**(retval=0)  
  终止当前程序给定的退出代码。 函数会抛出 `SystemExit` 异常。
#### **sys.print_exception**(exc, file=sys.stdout)  
  打印异常与追踪到一个类似文件的对象 file (或者缺省 `sys.stdout` ).

> 提示：这是 CPython 中回溯模块的简化版本。不同于 `traceback.print_exception()`，这个函数用异常值代替了异常类型、异常参数和回溯对象。文件参数在对应位置，不支持更多参数。CPython 兼容回溯模块在 `micropython-lib`。  

### 常数

#### **sys.argv**  
  当前程序启动时参数的可变列表。

#### **sys.byteorder**  
  系统字节顺序 (“little” or “big”).

#### **sys.implementation**  
  关于当前 Python 实现的信息，对于 MicroPython 来说，有以下属性：  
  - 名称 -  ‘’micropython“  
  - 版本 - 元组（主要，次要，小），比如（1，9，3） 

#### **sys.modules**  
  已加载模块的字典。在一部分移植中，它可能不包含内置模块。

#### **sys.path**  
  用来搜索导入模块地址的列表。

#### **sys.platform**  
  返回当前平台的信息。

#### **sys.stderr**  
  标准错误流。

#### **sys.stdin**  
  标准输入流。

#### **sys.stdout**  
  标准输出流。

#### **sys.version**  
  符合的 Python 语言版本，如字符串。

#### **sys.version_info**  
  本次实现使用的 Python 语言版本，用一个元组的方式表示。

### 示例 

```
>>> import sys
>>> sys.version
'3.4.0'
>>> sys.version_info
(3, 4, 0)
>>> sys.path
['', '/libs/mpy/']
>>> sys.__name__
'sys'
>>> sys.platform
'rt-thread'
>>> sys.byteorder
'little'
```

更多内容可参考 [sys](http://docs.micropython.org/en/latest/pyboard/library/sys.html) 。
