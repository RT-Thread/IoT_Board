## machine.UART

**machine.UART** 类是 machine 模块下面的一个硬件类，用于对 UART 的配置和控制，提供对 UART 设备的操作方法。

`UART` 实现了标准的 `uart/usart` 双工串行通信协议，在物理层上，他由两根数据线组成：`RX` 和 `TX`。通信单元是一个字符，它可以是 8 或 9 位宽。

### 构造函数

在 RT-Thread MicroPython 中 `UART` 对象的构造函数如下：

#### **class machine.UART**(id, ...)
在给定总线上构造一个 `UART` 对象，`id` 取决于特定的移植。  
初始化参数可以参考下面的 `UART.init` 方法。 

使用硬件 UART 在初始化时只需传入 `UART` 设备的编号即可，如传入 `1` 表示 `uart1` 设备。   
初始化方式可参考 [示例](#_3)。

### 方法

#### **UART.init**(baudrate = 9600, bits=8, parity=None, stop=1)
- **baudrate** ：`SCK` 时钟频率。
- **bits** ：每次发送数据的长度。
- **parity** ：校验方式。
- **stop** ：停止位的长度。

#### **UART.deinit**()
关闭串口总线。

#### **UART.read**([nbytes])
读取字符，如果指定读 n 个字节，那么最多读取 n 个字节，否则就会读取尽可能多的数据。
返回值：一个包含读入数据的字节对象。如果如果超时则返回 `None`。

#### **UART.readinto**(buf[, nbytes])
读取字符到 `buf` 中，如果指定读 n 个字节，那么最多读取 n 个字节，否则就读取尽可能多的数据。另外读取数据的长度不超过 `buf` 的长度。
返回值：读取和存储到 `buf` 中的字节数。如果超时则返回 `None`。

#### **UART.readline**()
读一行数据，以换行符结尾。
返回值：读入的行数，如果超时则返回 `None`。

#### **UART.write**(buf)
将 `buf` 中的数据写入总线。
返回值：写入的字节数，如果超时则返回 `None`。

### 示例

在构造函数的第一个参数传入`1`，系统就会搜索名为 `uart1` 的设备，找到之后使用这个设备来构建 `UART` 对象：

```python
from machine import UART
uart = UART(1, 115200)                         # init with given baudrate
uart.init(115200, bits=8, parity=None, stop=1) # init with given parameters
uart.read(10)       # read 10 characters, returns a bytes object
uart.read()         # read all available characters
uart.readline()     # read a line
uart.readinto(buf)  # read and store into the given buffer
uart.write('abc')   # write the 3 characters
```

  更多内容可参考 [machine.UART](http://docs.micropython.org/en/latest/pyboard/library/machine.UART.html) 。