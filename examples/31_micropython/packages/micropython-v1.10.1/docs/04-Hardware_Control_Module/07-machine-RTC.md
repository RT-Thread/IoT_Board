## machine.RTC

**machine.RTC** 类是 machine 模块下面的一个硬件类，用于对指定 RTC 设备的配置和控制，提供对 RTC 设备的操作方法。

- RTC（Real-Time Clock ）实时时钟可以提供精确的实时时间，它可以用于产生年、月、日、时、分、秒等信息。

### 构造函数

在 RT-Thread MicroPython 中 `RTC` 对象的构造函数如下：

#### **class machine.RTC**()

所以在给定的总线上构造一个 `RTC` 对象，无入参对象，使用方式可参考 [示例](#_3)。 

### 方法

#### **RTC.init**(datetime)

根据传入的参数初始化 RTC 设备起始时间。入参 `datetime` 为一个时间元组，格式如下：

```
(year, month, day, wday, hour, minute, second, yday)
```
参数介绍如下所示：

- **year**：年份；
- **month**：月份，范围 [1, 12]；
- **day**：日期，范围 [1, 31]；
- **wday**：星期，范围 [0, 6]，0 表示星期一，以此类推；
- **hour**：小时，范围 [0, 23]；
- **minute**：分钟，范围[0, 59]；
- **second**：秒，范围[0, 59]；
- **yday**：从当前年份 1 月 1 日开始的天数，范围 [0, 365]，一般置位 0 未实现。

使用的方式可参考 [示例](#_3)。

#### **RTC.deinit**()

重置 RTC 设备时间到 2015 年 1 月 1日，重新运行 RTC 设备。

#### **RTC.now**()

获取当前时间，返回值为上述 `datetime` 时间元组格式。

### 示例

```python
>>> from machine import RTC
>>> rtc = RTC()                        # 创建 RTC 设备对象
>>> rtc.init((2019,6,5,2,10,22,30,0))  # 设置初始化时间
>>> rtc.now()                          # 获取当前时间
(2019, 6, 5, 2, 10, 22, 40, 0)
>>> rtc.deinit()                       # 重置时间到2015年1月1日
>>> rtc.now()                          # 获取当前时间
(2015, 1, 1, 3, 0, 0, 1, 0)
```
