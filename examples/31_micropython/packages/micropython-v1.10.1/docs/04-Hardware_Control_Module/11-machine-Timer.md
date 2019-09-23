## machine.Timer

**machine.Timer** 类是 machine 模块下的一个硬件类，用于 Timer 设备的配置和控制，提供对 Timer 设备的操作方法。

- Timer（硬件定时器），是一种用于处理周期性和定时性事件的设备。
- Timer 硬件定时器主要通过内部计数器模块对脉冲信号进行计数，实现周期性设备控制的功能。
- Timer 硬件定时器可以自定义**超时时间**和**超时回调函数**，并且提供两种**定时器模式**：
  - `ONE_SHOT`：定时器只执行一次设置的回调函数；
  - `PERIOD`：定时器会周期性执行设置的回调函数；
- 打印 Timer 对象会打印出配置的信息。

### 构造函数

在 RT-Thread MicroPython 中 `Timer` 对象的构造函数如下：

#### **class machine.Timer**(id)

- **id**：使用的 Timer 设备编号，`id = 1` 表示编号为 1 的 Timer 设备，或者表示使用的 timer 设备名，如 `id = "timer"` 表示设备名为 `timer` 的 Timer 设备；

该函数主要用于通过设备编号创建 Timer 设备对象。

### 方法

#### **Timer.init**(mode = Timer.PERIODIC, period = 0, callback = None)

- **mode**：设置 Timer 定时器模式，可以设置两种模式：`ONE_SHOT`（执行一次）、`PERIOD`（周期性执行），默认设置的模式为 `PERIOD` 模式；

- **period**：设置 Timer 定时器定时周期，单位：毫秒（ms）

- **callback**：设置 Timer 定义器超时回调函数，默认设置的函数为 None 空函数，设置的函数格式如下所示：

```python
def callback_test(device):         # 回调函数有且只有一个入参，为创建的 Timer 对象
    print("Timer callback test")
    print(device)                  # 打印 Timer 对象配置信息
```

该函数使用方式如下示例所示：

```python
timer.init(wdt.PERIOD, 5000, callback_test)   # 设置定时器模式为周期性执行，超时时间为 5 秒, 超时函数为 callback_test
```
#### **Timer.deinit**()

该函数用于停止并关闭 Timer 设备。

### 常量

下面的常量用来配置 `Timer` 对象。

#### 选择定时器模式：
##### **Timer.PERIODIC**
##### **Timer.ONE_SHOT**

### 示例

```python
>>> from machine import Timer                       # 从 machine 导入 Timer 类
>>> timer = Timer(15)                               # 创建 Timer 对象，当前设备编号为 11
>>>                                                 # 进入粘贴模式
paste mode; Ctrl-C to cancel, Ctrl-D to finish
=== def callback_test(device):                      # 定义超时回调函数 
===     print("Timer callback test")
>>> timer.init(timer.PERIODIC, 5000, callback_test) # 初始化 Timer 对象，设置定时器模式为循环执行，超时时间为 5 秒，超时回调函数 callback_test
>>> Timer callback test                             # 5 秒超时循环执行回调函数，打印日志
>>> Timer callback test
>>> Timer callback test
>>> timer.init(timer.ONE_SHOT, 5000, callback_test) # 设置定时器模式为只执行一次，超时时间为 5 秒，超时回调函数为 callback_test
>>> Timer callback test                             # 5 秒超时后执行一次回调函数，打印日志
>>> timer.deinit()                                  # 停止并关闭 Timer 定时器
```

更多内容可参考 [machine.Timer](http://docs.micropython.org/en/latest/library/machine.Timer.html)。
