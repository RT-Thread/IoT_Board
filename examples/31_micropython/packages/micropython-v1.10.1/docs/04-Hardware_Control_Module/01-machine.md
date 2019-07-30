## **machine** – 与硬件相关的功能

**machine** 模块包含与特定开发板上的硬件相关的特定函数。 在这个模块中的大多数功能允许实现直接和不受限制地访问和控制系统上的硬件块（如CPU，定时器，总线等）。如果使用不当，会导致故障，死机，崩溃，在极端的情况下，硬件会损坏。

需要注意的是，由于不同开发板的硬件资源不同，MicroPython 移植所能控制的硬件也是不一样的。因此对于控制硬件的例程来说，在使用前需要修改相关的配置参数来适配不同的开发板，或者直接运行已经对某一开发板适配好的 MicroPython 示例程序。本文档中的例程都是基于 RT-Thread IoT Board 潘多拉开发板而讲解的。

### 函数

#### 复位相关函数

##### **machine.info**()  
  显示关于系统介绍和内存占用等信息。

##### **machine.rest**()  
  重启设备，类似于按下复位按钮。

##### **machine.reset_cause**()  
  获得复位的原因，查看可能的返回值的常量。

#### 中断相关函数

##### **machine.disable_irq**()  
  禁用中断请求。返回先前的 `IRQ` 状态，该状态应该被认为是一个未知的值。这个返回值应该在 `disable_irq` 函数被调用之前被传给 `enable_irq` 函数来重置中断到初始状态。

##### **machine.enable_irq**(state)  
  重新使能中断请求。状态参数应该是从最近一次禁用功能的调用中返回的值。

#### 功耗相关函数

##### **machine.freq**()  
  返回 `CPU` 的运行频率。

##### **machine.idle**()  
  阻断给 `CPU` 的时钟信号，在较短或者较长的周期里减少功耗。当中断发生时，外设将继续工作。

##### **machine.sleep**()  
  停止 `CPU` 并禁止除了 `WLAN` 之外的所有外设。系统会从睡眠请求的地方重新恢复工作。为了确保唤醒一定会发生，应当首先配置中断源。

##### **machine.deepsleep**()  
  停止 `CPU` 和所有外设（包括网络接口）。执行从主函数中恢复，就像被复位一样。复位的原因可以检查 `machine.DEEPSLEEP` 参数获得。为了确保唤醒一定会发生，应该首先配置中断源，比如一个引脚的变换或者 `RTC` 的超时。

### 常数

#### **machine.IDLE**
#### **machine.SLEEP**
#### **machine.DEEPSLEEP**
`IRQ` 的唤醒值。

#### **machine.PWRON_RESET **
#### **machine.HARD_RESET **
#### **machine.WDT_RESET **
#### **machine.DEEPSLEEP_RESET **
#### **machine.SOFT_RESET**
复位的原因。

#### **machine.WLAN_WAKE**
#### **machine.PIN_WAKE**
#### **machine.RTC_WAKE**
唤醒的原因。

### 类

#### [class Pin](02-machine-Pin.md)     - 控制 I/O 引脚
#### [class I2C](03-machine-I2C.md)     - I2C 协议
#### [class SPI](04-machine-SPI.md)     - SPI 协议
#### [class UART](05-machine-UART.md)   - 串口
#### [class LCD](06-machine-LCD.md)     - LCD
#### [class RTC](07-machine-RTC.md)     - RTC
#### [class PWM](08-machine-PWM.md)     - PWM
#### [class ADC](09-machine-ADC.md)     - ADC
#### [class WDT](10-machine-WDT.md)     - 看门狗
#### [class TIMER](11-machine-Timer.md) - 定时器

### 示例 

```
>>> import machine
>>>
>>> machine.info()              # show information about the board
---------------------------------------------
RT-Thread
---------------------------------------------
total memory: 131048
used memory : 4920
maximum allocated memory: 5836
thread     pri  status      sp     stack size max used left tick  error
---------- ---  ------- ---------- ----------  ------  ---------- ---
elog_async  31  suspend 0x000000a8 0x00000400    26%   0x00000003 000
tshell      20  ready   0x0000019c 0x00001000    39%   0x00000006 000
tidle       31  ready   0x0000006c 0x00000100    50%   0x0000000b 000
SysMonitor  30  suspend 0x000000a8 0x00000200    32%   0x00000005 000
timer        4  suspend 0x0000007c 0x00000200    24%   0x00000009 000
---------------------------------------------
qstr:
  n_pool=0
  n_qstr=0
  n_str_data_bytes=0
  n_total_bytes=0
---------------------------------------------
GC:
  16064 total
  464 : 15600
  1=14 2=6 m=3
>>> machine.enable_irq()        # enable interrupt
>>> machine.disable_irq()       # disable interrupt, WARNING: this operation is dangerous
>>> machine.reset()             # hard reset, like push RESET button
```

更多内容可参考 [machine](http://docs.micropython.org/en/latest/pyboard/library/machine.html) 。