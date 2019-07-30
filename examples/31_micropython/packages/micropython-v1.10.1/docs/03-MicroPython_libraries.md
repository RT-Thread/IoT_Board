# MicroPython 模块

- MicroPython 提供丰富的模块，每个模块提供特定的功能。了解开发的过程中一些常用的模块的使用方式，可以让你很好的使用 MicroPython 的功能。

- 这些模块可以通过 env 工具的 menuconfig 功能来开启和关闭，如果你需要使用特定的模块，在 menuconfig 中选中模块名，保存退出后，重新编译运行即可。

## Python 标准库和微型库

Python 的标准库被 “微型化”后，就是 MicroPython 标准库，也称 MicroPython 模块。它们仅仅提供了该模块的核心功能，用来替代 Python 标准库 。一些模块使用 Python  标准库的名字，但是加上了前缀 "u"，例如``ujson``代替``json``。也就是说 MicroPython 的标准库(微型库)，只实现了一部分模块功能。通过给这些库以不同的方式命名，用户可以写一个 Python 级的模块来扩展微型库的功能，以便于兼容 CPython 的标准库（这项工作就是 [micropython-lib](https://github.com/micropython/micropython-lib) 项目的正在做的）。

在一些嵌入式平台上，可添加 Python 级别封装库从而实现命名兼容 CPython，使用 MicroPython 标准库既可使用他们的 u-name，也可以使用 non-u-name。使用 non-u-name 的模块可以被库路径文件夹里面的同名模块所覆盖。

例如，当``import json``时，首先会在库路径文件夹中搜索一个 ``json.py`` 文件或 ``json`` 目录进行加载。如果没有找到，它才会去加载内置 ``ujson`` 模块。

## RT-Thread MicroPython 模块

### 常用内建模块
- [rtthread][1]       – RT-Thread 系统相关函数
- [utime][2]          – 时间相关函数
- [sys][3]            – 系统特有功能函数
- [math][4]           – 数学函数
- [uio][5]            – 输入/输出流
- [ucollections][6]   – 提供有用的集合类
- [ustruct][7]        – 打包和解包原始数据类型
- [array][8]          – 数字数据数组
- [gc][9]             – 控制垃圾回收
- [uos][15]           – 基本的 “操作系统” 服务
- [select][16]        – 等待流事件
- [uctypes][17]       – 以结构化的方式访问二进制数据
- [uerrno][18]        – 系统错误码模块
- [_thread][19]       – 多线程支持

### 硬件模块
- [machine][10]       – 与硬件相关的功能
- [machine.Pin][11]   - Pin 引脚控制类
- [machine.UART][14]  - UART 外设控制类
- [machine.I2C][12]   - I2C 外设控制类
- [machine.SPI][13]   - SPI 外设控制类
- [machine.RTC][29]   - RTC 外设控制类
- [machine.PWM][30]   - PWM 外设控制类
- [machine.ADC][31]   - ADC 外设控制类
- [machine.LCD][34]   - LCD 外设控制类

### 网络模块
- [usocket][28]       – 网络套接字模块
- [network][32]       – 网络连接控制模块
- [network.WLAN][33]  – WiFi 连接控制类

### 常用第三方模块
- [cmath][20]         – 复数的数学函数
- [ubinascii][21]     – 二进制/ ASCII转换
- [uhashlib][22]      – 哈希算法
- [uheapq][23]        – 堆排序算法
- [ujson][24]         – JSON编码与解码
- [ure][25]           – 正则表达式
- [uzlib][26]         – zlib 解压缩
- [urandom][27]       – 随机数生成模块

[1]: 03-Basic_Module/01-rtthread.md
[2]: 03-Basic_Module/02-utime.md
[3]: 03-Basic_Module/03-sys.md
[4]: 03-Basic_Module/04-math.md
[5]: 03-Basic_Module/05-uio.md
[6]: 03-Basic_Module/06-ucollections.md
[7]: 03-Basic_Module/07-ustruct.md
[8]: 03-Basic_Module/08-array.md
[9]: 03-Basic_Module/09-gc.md
[10]: 04-Hardware_Control_Module/01-machine.md
[11]: 04-Hardware_Control_Module/02-machine-Pin.md
[12]: 04-Hardware_Control_Module/03-machine-I2C.md
[13]: 04-Hardware_Control_Module/04-machine-SPI.md
[14]: 04-Hardware_Control_Module/05-machine-UART.md
[15]: 05-System_Module/01-uos.md
[16]: 05-System_Module/02-uselect.md
[17]: 05-System_Module/03-uctypes.md
[18]: 05-System_Module/04-uerrno.md
[19]: 05-System_Module/05-_thread.md
[20]: 06-Tools_Module/01-cmath.md
[21]: 06-Tools_Module/02-ubinascii.md
[22]: 06-Tools_Module/03-uhashlib.md
[23]: 06-Tools_Module/04-uheapq.md
[24]: 06-Tools_Module/05-ujson.md
[25]: 06-Tools_Module/06-ure.md
[26]: 06-Tools_Module/07-uzlib.md
[27]: 06-Tools_Module/08-urandom.md
[28]: 07-Network_Module/01-usocket.md
[29]: 04-Hardware_Control_Module/07-machine-RTC.md
[30]: 04-Hardware_Control_Module/08-machine-PWM.md
[31]: 04-Hardware_Control_Module/09-machine-ADC.md
[32]: 07-Network_Module/02-network.md
[33]: 07-Network_Module/03-network-WLAN.md
[34]: 04-Hardware_Control_Module/06-machine-LCD.md