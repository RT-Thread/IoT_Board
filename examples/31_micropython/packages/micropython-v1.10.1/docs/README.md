# RT-Thread MicroPython 开发手册介绍

本手册介绍了 RT-Thread MicroPython 的基础知识、常用模块，以及开发新模块的流程。带领读者了解 MicroPython ，并学会使用 MicroPython 进行开发。

## 主要特性

- MicroPython 是 Python 3 编程语言的一种精简而高效的实现，它包含 Python 标准库的一个子集，并被优化为在微控制器和受限环境中运行。

- RT-Thread MicroPython 可以运行在任何搭载了 RT-Thread 操作系统并且有一定资源的嵌入式平台上。

- MicroPython 可以运行在有一定资源的开发板上，给你一个低层次的 Python 操作系统，可以用来控制各种电子系统。

- MicroPython 富有各种高级特性，比如交互式提示、任意精度整数、闭包函数、列表解析、生成器、异常处理等等。

- MicroPython 的目标是尽可能与普通 Python 兼容，使开发者能够轻松地将代码从桌面端转移到微控制器或嵌入式系统。程序可移植性很强，因为不需要考虑底层驱动，所以程序移植变得轻松和容易。

## MicroPython 的优势

- Python 是一款容易上手的脚本语言，同时具有强大的功能，语法优雅简单。使用 MicroPython 编程可以降低嵌入式的开发门槛，让更多的人体验嵌入式的乐趣。
- 通过 MicroPython 实现硬件底层的访问和控制，不需要了解底层寄存器、数据手册、厂家的库函数等，即可轻松控制硬件。
- 外设与常用功能都有相应的模块，降低开发难度，使开发和移植变得容易和快速。

## MicroPython 的应用领域

- MicroPython 在嵌入式系统上完整实现了 Python3 的核心功能，可以在产品开发的各个阶段给开发者带来便利。
- 通过 MicroPython 提供的库和函数，开发者可以快速控制 LED、液晶、舵机、多种传感器、SD、UART、I2C 等，实现各种功能，而不用再去研究底层硬件模块的使用方法，翻看寄存器手册。这样不但降低了开发难度，而且减少了重复开发工作，可以加快开发速度，提高开发效率。以前需要较高水平的嵌入式工程师花费数天甚至数周才能完成的功能，现在普通的嵌入式开发者用几个小时就能实现类似的功能。
- 随着半导体技术的不断发展，芯片的功能、内部的存储器容量和资源不断增加，成本不断降低，可以使用 MicroPython 来进行开发设计的应用领域也会越来越多。

### 产品原型验证

- 众所周知，在开发新产品时，原型设计是一个非常重要的环节，这个环节需要以最快速的方式设计出产品的大致模型，并验证业务流程或者技术点。与传统开发方法相比，使用 MicroPython 对于原型验证非常有用，让原型验证过程变得轻松，加速原型验证过程。
- 在进行一些物联网功能开发时，网络功能也是 MicroPython 的长处，可以利用现成的众多 MicroPython 网络模块，节省开发时间。而这些功能如果使用 C/C++ 来完成，会耗费几倍的时间。

### 硬件测试

- 嵌入式产品在开发时，一般会分为硬件开发及软件开发。硬件工程师并不一定都擅长软件开发，所以在测试新硬件时，经常需要软件工程师参与。这就导致软件工程师可能会耗费很多时间帮助硬件工程师查找设计或者焊接问题。有了 MicroPython 后，将 MicroPython 固件烧入待测试的新硬件，在检查焊接、连线等问题时，只需使用简单的 Python 命令即可测试。这样，硬件工程师一人即可搞定，再也不用麻烦别人了。

### 教育

- MicroPython 使用简单、方便，非常适合于编程入门。在校学生或者业余爱好者都可以通过 MicroPython 快速的开发一些好玩的项目，在开发的过程中学习编程思想，提高自己的动手能力。
- 下面是一些 MicroPython 教育项目：
    - [从TurnipBit开始完成编程启蒙](https://www.cnblogs.com/xxosu/p/7206414.html)
    - [MicroBit 创意编程](http://microbit.org/)

### 创客 DIY

- MicroPython 无需复杂的设置，不需要安装特别的软件环境和额外的硬件，使用任何文本编辑器就可以进行编程。大部分硬件功能，使用一个命令就能驱动，不用了解硬件底层就能快速开发。这些特性使得 MicroPython 非常适合创客使用来开发一些有创意的项目。
- 下面是使用 MicroPython 制作的一些 DIY 项目：
    - [显示温湿度的 WIFI 时钟](https://www.bilibili.com/video/av15929152?from=search&seid=16285206333541196172)
    - [OpenMV 智能摄像头](https://www.bilibili.com/video/av16418889?from=search&seid=16285206333541196172)
    - [手机遥控车](https://www.bilibili.com/video/av15008143?from=search&seid=16285206333541196172)
    - [搭建 MQTT 服务器](http://www.360doc.com/content/17/1218/22/8473307_714341237.shtml)

## MicroPython 开发资源

- [RT-Thread MicroPython 开发手册](https://www.rt-thread.org/document/site/rtthread-development-guide/micropython/docs/README/)

- [RT-Thread MicroPython 源码](https://github.com/RT-Thread-packages/micropython)

- [RT-Thread MicroPython 论坛](https://www.rt-thread.org/qa/forum.php)

- [MicroPython 官方网站](https://micropython.org/)

- [官方在线文档](http://docs.micropython.org/en/latest/pyboard/)

- [MicroPython 在线演示](https://micropython.org/unicorn)

- [MicroPython 源码](https://github.com/micropython/micropython)

- [MicroPython 官方论坛](http://forum.micropython.org/)

- [MicroPython 中文社区](http://www.micropython.org.cn/)
