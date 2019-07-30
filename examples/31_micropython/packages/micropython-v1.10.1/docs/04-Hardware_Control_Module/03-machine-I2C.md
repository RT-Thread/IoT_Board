## machine.I2C

**machine.I2C** 类是 `machine` 模块下面的一个硬件类，用于对 `I2C` 的配置和控制，提供对 `I2C` 设备的操作方法。

- `I2C` 是一种用于设备间通信的两线协议。在物理层上，它由两根线组成：`SCL`  和 `SDA` ，即时钟和数据线。
- `I2C` 对象被创建到一个特定的总线上，它们可以在创建时被初始化，也可以之后再来初始化。
- 打印 `I2C` 对象会打印出配置时的信息。

### 构造函数

在 RT-Thread MicroPython 中 `I2C` 对象的构造函数如下：

#### **class machine.I2C**(id= -1,  scl, sda, freq=400000)
使用下面的参数构造并返回一个新的 `I2C` 对象：

- **id** ：标识特定的 `I2C`  外设。如果填入 id = -1，即选择软件模拟的方式实现 `I2C`，这时可以使用任意引脚来模拟 `I2C` 总线 ，这样在初始化时就必须指定 `scl` 和 `sda` 。  
软件 I2C 的初始化方式可参考 [软件 I2C 示例](#i2c_2)。  
硬件 I2C 的初始化方式可参考 [硬件 I2C 示例](#i2c_3)。

- **scl** : 应该是一个 `Pin` 对象，指定为一个用于 `scl` 的 `Pin` 对象。
- **sda** : 应该是一个 `Pin` 对象，指定为一个用于 `sda` 的 `Pin` 对象。
- **freq** ：应该是为 `scl` 设置的最大频率。

### 方法

#### **I2C.init**(scl, sda, freq=400000)
初始化 `I2C` 总线，参数介绍可以参考构造函数中的参数。

#### **I2C.deinit**()
关闭 `I2C` 总线。

#### **I2C.scan**()
扫描所有 0x08 和 0x77 之间的 `I2C` 地址，然后返回一个有响应地址的列表。如果一个设备在总线上收到了他的地址，就会通过拉低 `SDA` 的方式来响应。

### I2C 基础方法
下面的方法实现了基本的 `I2C` 总线操作，可以组合成任何的 `I2C` 通信操作，如果需要对总线进行更多的控制，可以可以使用他们，否则可以使用后面介绍的标准使用方法。

#### **I2C.start**()
在总线上产生一个启动信号。（`SCL` 为高时，`SDA` 转换为低）

#### **I2C.stop**()
在总线上产生一个停止信号。（`SCL` 为高时，`SDA` 转换为高）

#### **I2C.readinto**(buf, nack=True)
从总线上读取字节并将他们存储到 `buf` 中，读取的字节数时 `buf` 的长度。在收到最后一个字节以外的所有内容后，将在总线上发送 `ACK`。在收到最后一个字节之后，如果 `NACK` 是正确的，那么就会发送一个 `NACK`，否则将会发送 `ACK`。

####  **I2C.write**(buf)
将 `buf` 中的数据接入到总线，检查每个字节之后是否收到 `ACK`，并在收到 `NACK` 时停止传输剩余的字节。这个函数返回接收到的 `ACK` 的数量。

### I2C 标准总线操作
下面的方法实现了标准 `I2C` 主设备对一个给定从设备的读写操作。

#### **I2C.readfrom**(addr, nbytes, stop=True)
从 `addr` 指定的从设备中读取 n 个字节，如果 `stop = True`，那么在传输结束时会产生一个停止信号。函数会返回一个存储着读到数据的字节对象。

#### **I2C.readfrom_into**(addr, buf, stop=True)
从 `addr` 指定的从设备中读取数据存储到 `buf` 中，读取的字节数将是 `buf` 的长度，如果 `stop = True`，那么在传输结束时会产生一个停止信号。  
这个方法没有返回值。

#### **I2C.writeto**(addr, buf, stop=True)
将 `buf` 中的数据写入到 `addr` 指定的的从设备中，如果在写的过程中收到了 `NACK` 信号，那么就不会发送剩余的字节。如果 `stop = True`，那么在传输结束时会产生一个停止信号，即使收到一个 `NACK`。这个函数返回接收到的 `ACK` 的数量。

### 内存操作

一些 `I2C` 设备充当一个内存设备，可以读取和写入。在这种情况下，有两个与 `I2C` 相关的地址，从机地址和内存地址。下面的方法是与这些设备进行通信的便利函数。

#### **I2C.readfrom_mem**(addr, memaddr, nbytes, \*, addrsize=8)
从 `addr` 指定的从设备中 `memaddr` 地址开始读取 n 个字节。`addrsize` 参数指定地址的长度。返回一个存储读取数据的字节对象。

#### **I2C.readfrom_mem_into**(addr, memaddr, buf, \*, addrsize=8)
从 `addr` 指定的从设备中 `memaddr` 地址读取数据到 `buf` 中，，读取的字节数是 `buf` 的长度。  
这个方法没有返回值。

#### **I2C.writeto_mem**(addr, memaddr, buf, \*, addrsize=8)
将 `buf` 里的数据写入 `addr` 指定的从机的 `memaddr` 地址中。 
这个方法没有返回值。

### 示例

#### 软件模拟 I2C
```python
>>> from machine import Pin, I2C
>>> clk = Pin(("clk", 29), Pin.OUT_OD)   # Select the 29 pin device as the clock
>>> sda = Pin(("sda", 30), Pin.OUT_OD)   # Select the 30 pin device as the data line
>>> i2c = I2C(-1, clk, sda, freq=100000) # create I2C peripheral at frequency of 100kHz
>>> i2c.scan()                        # scan for slaves, returning a list of 7-bit addresses
[81]                                  # Decimal representation
>>> i2c.writeto(0x51, b'123')         # write 3 bytes to slave with 7-bit address 42
3
>>> i2c.readfrom(0x51, 4)             # read 4 bytes from slave with 7-bit address 42
b'\xf8\xc0\xc0\xc0'
>>> i2c.readfrom_mem(0x51, 0x02, 1)   # read 1 bytes from memory of slave 0x51(7-bit),
b'\x12'                               # starting at memory-address 8 in the slave
>>> i2c.writeto_mem(0x51, 2, b'\x10') # write 1 byte to memory of slave 42,
                                      # starting at address 2 in the slave
```

#### 硬件 I2C

需要先开启 `I2C` 设备驱动，查找设备可以在 `msh` 中输入`list_device` 命令。  
在构造函数的第一个参数传入 `0`，系统就会搜索名为 `i2c0` 的设备，找到之后使用这个设备来构建 `I2C` 对象：

```python
>>> from machine import Pin, I2C
>>> i2c = I2C(0)                      # create I2C peripheral at frequency of 100kHz
>>> i2c.scan()                        # scan for slaves, returning a list of 7-bit addresses
[81]                                  # Decimal representation
```

  更多内容可参考 [machine.I2C](http://docs.micropython.org/en/latest/pyboard/library/machine.I2C.html) 。

