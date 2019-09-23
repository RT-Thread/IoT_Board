## machine.Pin

**machine.Pin** 类是 machine 模块下面的一个硬件类，用于对引脚的配置和控制，提供对 `Pin` 设备的操作方法。

`Pin` 对象用于控制输入/输出引脚（也称为 `GPIO`）。`Pin` 对象通常与一个物理引脚相关联，他可以驱动输出电压和读取输入电压。Pin 类中有设置引脚模式（输入/输出）的方法，也有获取和设置数字逻辑（`0` 或 `1`）的方法。

一个 `Pin` 对象是通过一个标识符来构造的，它明确地指定了一个特定的输入输出。标识符的形式和物理引脚的映射是特定于一次移植的。标识符可以是整数，字符串或者是一个带有端口和引脚号码的元组。在 RT-Thread MicroPython 中，引脚标识符是一个由代号和引脚号组成的元组，如 `Pin(("PB15", 31), Pin.OUT_PP)` 中的` ("PB15", 31)`。

### 构造函数

在 RT-Thread MicroPython 中 `Pin` 对象的构造函数如下：

#### **class machine.Pin**( id, mode = -1, pull = -1，value)
- **id** ：由用户自定义的引脚名和 `Pin`  设备引脚号组成，如 ("PB15", 31)，"PB15" 为用户自定义的引脚名，`31` 为 `RT-Thread Pin` 设备驱动在本次移植中的引脚号。

- **mode** ： 指定引脚模式，可以是以下几种：
    - **Pin.IN** ：输入模式
    - **Pin.OUT** ：输出模式
    - **Pin.OPEN_DRAIN** ：开漏模式

- **pull** ： 如果指定的引脚连接了上拉下拉电阻，那么可以配置成下面的状态：
    - **None** ：没有上拉或者下拉电阻。
    - **Pin.PULL_UP** ：使能上拉电阻。
    - **Pin.PULL_DOWN** ：使能下拉电阻。

- **value** ： `value` 的值只对输出模式和开漏输出模式有效，用来设置初始输出值。

### 方法

#### **Pin.init**(mode= -1, pull= -1, \*, value, drive, alt)

根据输入的参数重新初始化引脚。只有那些被指定的参数才会被设置，其余引脚的状态将保持不变，详细的参数可以参考上面的构造函数。

#### **Pin.value**([x])
如果没有给定参数 `x` ,这个方法可以获得引脚的值。  
如果给定参数 `x` ，如 `0` 或 `1`，那么设置引脚的值为 逻辑 `0` 或 逻辑 `1`。

#### **Pin.name**()
返回引脚对象在构造时用户自定义的引脚名。

#### **Pin.irq**(handler=None, trigger=(Pin.IRQ_RISING))

配置在引脚的触发源处于活动状态时调用的中断处理程序。如果引脚模式是， `Pin.IN` 则触发源是引脚上的外部值。 如果引脚模式是， `Pin.OUT` 则触发源是引脚的输出缓冲器。 否则，如果引脚模式是， `Pin.OPEN_DRAIN` 那么触发源是状态'0'的输出缓冲器和状态'1'的外部引脚值。

参数:

- `handler` 是一个可选的函数，在中断触发时调用
- `trigger` 配置可以触发中断的事件。可能的值是：
    - `Pin.IRQ_FALLING` 下降沿中断
    - `Pin.IRQ_RISING` 上升沿中断
    - `Pin.IRQ_RISING_FALLING` 上升沿或下降沿中断
    - `Pin.IRQ_LOW_LEVEL` 低电平中断
    - `Pin.IRQ_HIGH_LEVEL` 高电平中断

### 常量

下面的常量用来配置 `Pin` 对象。 

#### 选择引脚模式：
##### **Pin.IN**
##### **Pin.OUT**
##### **Pin.OPEN_DRAIN**

#### 选择上/下拉模式：
##### **Pin.PULL_UP**
##### **Pin.PULL_DOWN**
##### **None**  
使用值 `None` 代表不进行上下拉。

#### 选择中断触发模式：
##### **Pin.IRQ_FALLING**
##### **Pin.IRQ_RISING**
##### **Pin.IRQ_RISING_FALLING**
##### **Pin.IRQ_LOW_LEVEL** 
##### **Pin.IRQ_HIGH_LEVEL** 

### 示例一

控制引脚输出高低电平信号，并读取按键引脚电平信号。

```
from machine import Pin

PIN_OUT = 31
PIN_IN  = 58

p_out = Pin(("PB15", PIN_OUT), Pin.OUT_PP)
p_out.value(1)                 # set io high
p_out.value(0)                 # set io low

p_in = Pin(("key_0", PIN_IN), Pin.IN, Pin.PULL_UP)
print(p_in.value() )           # get value, 0 or 1
```

### 示例二

上升沿信号触发引脚中断后执行中断处理函数。

```
from machine import Pin

PIN_KEY0 = 58    # PD10
key_0 = Pin(("key_0", PIN_KEY0), Pin.IN, Pin.PULL_UP)

def func(v):
    print("Hello rt-thread!")

key_0.irq(trigger=Pin.IRQ_RISING, handler=func)
```
更多内容可参考 [machine.Pin](http://docs.micropython.org/en/latest/pyboard/library/machine.Pin.html)  。