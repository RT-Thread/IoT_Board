## machine.WDT

**machine.WDT** 类是 machine 模块下的一个硬件类，用于 WDT 设备的配置和控制，提供对 WDT 设备的操作方法。

如下为 WDT 设备基本介绍：

- WDT（WatchDog Timer，硬件看门狗），是一个定时器设备，用于系统程序结束或出错导致系统进入不可恢复状态时重启系统。

- WDT 启动之后，计数器开始计数，在计数器溢出前没有被复位，会对 CPU 产生一个复位信号使设备重启（简称 “被狗咬”）；

- 系统正常运行时，需要在 WDT 设备允许的时间间隔内对看门狗计数清零（简称“喂狗”），WDT 设备一旦启动，需要定时“喂狗”以确保设备正常运行。

### 构造函数

在 RT-Thread MicroPython 中 `WDT` 对象的构造函数如下：

#### **class machine.WDT**(timeout=5)

- **timeout**：设置看门狗超时时间，单位：秒（s）；

用于创建一个 WDT 对象并且启动 WDT 功能。一旦启动，设置的超时时间无法改动，WDT 功能无法停止。

如果该函数入参为空，则设置超时时间为 5 秒；如果入参非空，则使用该入参设置 WDT 超时时间，超时时间最小设置为 1 秒。

### 方法

#### **WDT.feed**()

用于执行“喂狗”操作，清空看门狗设备计数。应用程序应该合理的周期性调用该函数，以防止系统重启。

### 示例

``` python
>>> from machine import WDT     # 从 machine 导入 WDT 类
>>> wdt = WDT()                 # 创建 WDT 对象，默认超时时间为 5 秒
>>> wdt = WDT(10)               # 创建 WDT 对象，设置超时时间为 10 秒
>>> wdt.feed()                  # 在 10 秒超时时间内需要执行“喂狗”操作，清空看门狗设备计数，否则系统将重启
```

更多内容可参考 [machine.WDT](http://docs.micropython.org/en/latest/library/machine.WDT.html) 。
