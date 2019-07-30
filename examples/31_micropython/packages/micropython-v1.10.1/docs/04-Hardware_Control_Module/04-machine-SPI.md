## machine.SPI

**machine.SPI** 类是 machine 模块下面的一个硬件类，用于对 SPI 的配置和控制，提供对 SPI 设备的操作方法。

- `SPI` 是一个由主机驱动的同步串行协议。在物理层，总线有三根：`SCK`、`MOSI`、`MISO`。多个设备可以共享同一总线，每个设备都由一个单独的信号 `SS` 来选中，也称片选信号。
- 主机通过片选信号选定一个设备进行通信。`SS` 信号的管理应该由用户代码负责。（通过 [machine.Pin](02-machine-Pin.md)）

### 构造函数

在 RT-Thread MicroPython 中 `SPI` 对象的构造函数如下：

#### **class machine.SPI**(id, ...)
在给定总线上构造一个 `SPI` 对象，`id` 取决于特定的移植。

如果想要使用软件 `SPI` , 即使用引脚模拟 `SPI` 总线，那么初始化的第一个参数需要设置为 `-1` ，可参考 [软件 SPI 示例](#spi) 。

使用硬件 `SPI` 在初始化时只需传入 `SPI` 设备的编号即可，如 '50' 表示 `SPI5` 总线上的第 0 个设备。初始化方式可参考 [硬件 SPI 示例](#spi_1)。

如果没有额外的参数，`SPI` 对象会被创建，但是不会被初始化，如果给出额外的参数，那么总线将被初始化，初始化参数可以参考下面的 `SPI.init` 方法。

### 方法

#### **SPI.init**(baudrate=1000000, \*, polarity=0, phase=0, bits=8, firstbit=SPI.MSB, sck=None, mosi=None, miso=None)

用给定的参数初始化`SPI`总线：

- **baudrate** ：`SCK` 时钟频率。
- **polarity** ：极性可以是 `0` 或 `1`，是时钟空闲时所处的电平。
- **phase** ：相位可以是 `0` 或 `1`，分别在第一个或者第二个时钟边缘采集数据。
- **bits** ：每次传输的数据长度，一般是 8 位。
- **firstbit** ：传输数据从高位开始还是从低位开始，可以是 `SPI.MSB` 或者 `SPI.LSB`。
- **sck** ：用于 `sck` 的 `machine.Pin` 对象。
- **mosi** ：用于 `mosi` 的 `machine.Pin` 对象。
- **miso** ：用于`miso` 的 `machine.Pin` 对象。

#### **SPI.deinit**()
关闭 `SPI` 总线。

#### **SPI.read**(nbytes, write=0x00)
读出 n 字节的同时不断的写入 `write` 给定的单字节。返回一个存放着读出数据的字节对象。

#### **SPI.readinto**(buf, write=0x00)
读出 n 字节到 `buf` 的同时不断地写入 `write` 给定的单字节。  
这个方法返回读入的字节数。

#### **SPI.write**(buf)
写入 `buf` 中包含的字节。返回`None`。

#### **SPI.write_readinto**(write_buf, read_buf)
在读出数据到 `readbuf` 时，从 `writebuf` 中写入数据。缓冲区可以是相同的或不同，但是两个缓冲区必须具有相同的长度。返回 `None`。

### 常量

#### **SPI.MASTER**
用于初始化 `SPI` 总线为主机。

#### **SPI.MSB**
设置从高位开始传输数据。

#### **SPI.LSB**
设置从低位开始传输数据。

### 示例

#### 软件模拟 SPI
```
>>> from machine import Pin, SPI
>>> clk = Pin(("clk", 26), Pin.OUT_PP)
>>> mosi = Pin(("mosi", 27), Pin.OUT_PP)
>>> miso = Pin(("miso", 28), Pin.IN)
>>> spi = SPI(-1, 500000, polarity = 0, phase = 0, bits = 8, firstbit = 0, sck = clk, mosi = mosi, miso = miso)
>>> print(spi)
SoftSPI(baudrate=500000, polarity=0, phase=0, sck=clk, mosi=mosi, miso=miso)
>>> spi.write("hello rt-thread!")
>>> spi.read(10)
b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
```

#### 硬件 SPI

需要先开启 `SPI` 设备驱动，查找设备可以在 `msh` 中输入`list_device` 命令。  
在构造函数的第一个参数传入 `50`，系统就会搜索名为 `spi50` 的设备，找到之后使用这个设备来构建 `SPI` 对象：

```
>>> from machine import SPI
>>> spi = SPI(50)
>>> print(spi)
SPI(device port : spi50)
>>> spi.write(b'\x9f')
>>> spi.read(5)
b'\xff\xff\xff\xff\xff'
>>> buf = bytearray(1)
>>> spi.write_readinto(b"\x9f",buf)
>>> buf
bytearray(b'\xef')
>>> spi.init(100000,0,0,8,1)     # Resetting SPI parameter
```

  更多内容可参考 [machine.SPI](http://docs.micropython.org/en/latest/pyboard/library/machine.SPI.html) 。
