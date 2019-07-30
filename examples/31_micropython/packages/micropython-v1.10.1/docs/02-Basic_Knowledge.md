# RT-Thread MicroPython 基础知识

## 运行 python 文件

在 MicroPython 上运行 Python文件有以下要求：

- 系统内使用了 `rt-thread` 的文件系统。
- 开启 `msh` 功能。

符合以上两点，就可以使用 `msh` 命令行中的 `python` 命令加上 `*.py` 文件名来执行一个 Python文件了。

## 术语表

### board

  开发板，通常这个术语用来表示以一个特定的 `MCU` 为核心的开发板 。它也可以被用来表示移植 MicroPython 到一个特定的开发板上，也可以表示像 `Unix` 移植这样没有开发板的移植。

### CPython

  `CPython` 是 Python 编程语言的一种实现，是最被人们所熟知和使用的一种。然而它只是许多种实现中的一种（其中包括 `Jython、IronPython、PyPy、 MicroPython` 等）。由于没有正式的 Python 语言规范，只有 `CPython` 文档，在 Python 语言本身和 `Cpython` 这种实现之间画出一条界限并不容易。这同时也给其他方式的实现留下了更多的自由。比如 MicroPython 做了许多和 `Cpython` 不一样的事情，同时仍然成为 Python 语言的一种实现。

### GPIO

  通用输入/输出。控制电子信号最简单的方法。通过 `GPIO` 用户可以配置硬件信号引脚为输入或者输出，并设置或者获取其数字信号值（逻辑  '0' 或 '1'）。 MicroPython 使用抽象类  [`machine.Pin`](04-Hardware_Control_Module/02-machine-Pin.md)  来访问 `GPIO`。

### interned string

  由其唯一的标识而不是其地址引用的字符串。因此，可以用他们的标识符而不是通过内容快速地比较内部字符串。`interned` 字符串的缺点是插入操作需要时间（与现有 `interned` 字符串的数量成正比，也就是说会随着时间的推移耗时越来越久），而用于插入 `interned` 字符串的空间是不可回收的。当某个 `interned` 字符串被应用需求（作为关键字参数）或者对系统有益（可以减少查找的时间）时，就会被 MicroPython 的编译器和运行环境自动生成。由于上面的缺点，大多数字符串和输入/输出操作都不会产生 `interned` 字符串。

### MCU

  微控制器，也称单片机，通常资源比成熟的计算机系统要少的多，但是体积更小，也更便宜，功耗更低。MicroPython 被设计的足够小，并且优化到可以运行在一个普通的现代微控制器上。

### micropython-lib

  MicroPython 通常是单个的可执行/二进制文件，只有很少的内置模块。没有广泛的标准库可以用来和  `Cpython` 相比。与 `Cpython` 不同的是 MicroPython 有一个相关但是独立的项目 [`micropython-lib`](https://github.com/micropython/micropython-lib)，它提供了来自 `CPython` 标准库的许多模块的实现。然而这些模块大部分需要类似于 `POSIX` 的环境，只能在 MicroPython 的 `Unix` 移植上工作。安装方法与 `Cpython` 也不同，需要使用手动复制或者使用 `upip` 来安装。由于 `RT-Thread` 操作系统提供了很好的 `POSIX` 标准支持，所以 [`micropython-lib`](https://github.com/micropython/micropython-lib) 中很多模块可以在 `RT-Thread MicroPython` 上运行。

### stream

  也被称为文件类对象，一种对底层数据提供顺序读写访问的对象。`stream` 对象实现了对应的接口，包括` read()`, `write()`, `readinto()`, `seek()`, `flush()`, `close()`  等方法。在 MicroPython 中，流是一个重要的概念，许多输入/输出对象都实现了流接口，因此可以在不同的上下文中一致地使用。更多关于 MicroPython 流的信息，可以参考  [uio](03-Basic_Module/05-uio.md) 。

### upip

  字面意思是微型的 `pip` 工具。由 `CPython` 启发而开发的 MicroPython 包管理程序，但它要小的多，功能也更少。`upip` 可以在 MicroPython 的 `Unix` 移植上运行。由于 `RT-Thread` 操作系统提供了很好的 `POSIX` 标准支持，所以 `upip` 也可以运行在 `RT-Thread MicroPython` 上。使用 `upip` 工具可以在线下载 MicroPython 的扩展模块，并且自动下载其依赖的模块，为用户扩展 MicroPython 功能提供了很大的便利。 