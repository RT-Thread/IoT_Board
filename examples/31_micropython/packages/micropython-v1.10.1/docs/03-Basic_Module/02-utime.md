## **utime** – 时间相关函数

**utime** 模块提供获取当前时间和日期、测量时间间隔和延迟的功能。

**初始时刻**: `Unix` 使用 `POSIX` 系统标准，从 1970-01-01 00:00:00 `UTC` 开始。
嵌入式程序从 2000-01-01 00:00:00 `UTC` 开始。

**保持实际日历日期/时间**:需要一个实时时钟 `(RTC)`。在底层系统 (包括一些 `RTOS` 中)，`RTC` 已经包含在其中。设置时间是通过 `OS/RTOS` 而不是 MicroPython 完成，查询日期/时间也需要通过系统 `API`。对于裸板系统时钟依赖于 ``machine.RTC()`` 对象。设置时间通过 ``machine.RTC().datetime(tuple)`` 函数，并通过下面方式维持:

* 后备电池 (可能是选件、扩展板等)。
* 使用网络时间协议 (需要用户设置)。
* 每次上电时手工设置 (大部分只是在硬复位时需要设置，少部分每次复位都需要设置)。

如果实际时间不是通过系统 / MicroPython RTC 维持，那么下面函数结果可能不是和预期的相同。

### 函数

#### **utime.localtime**([secs])  
   从初始时间的秒转换为元组: (年, 月, 日, 时, 分, 秒, 星期, ``yearday``) 。如果 ``secs`` 是空或者 ``None``，那么使用当前时间。  

- `year ` 年份包括世纪（例如2014）。 
- `month`  范围 1-12     
- `day`    范围 1-31  
- `hour`    范围 0-23  
- `minute` 范围 0-59  
- `second`  范围 0-59  
- `weekday` 范围 0-6 对应周一到周日  
- `yearday` 范围 1-366  

#### **utime.mktime**()  
   时间的反函数，它的参数是完整8参数的元组，返回值一个整数自2000年1月1日以来的秒数。

#### **utime.sleep**(seconds)  
  休眠指定的时间（秒），``Seconds`` 可以是浮点数。注意有些版本的 MicroPython不支持浮点数，为了兼容可以使用 ``sleep_ms()`` 和 ``sleep_us()``函数。

#### **utime.sleep_ms**(ms)  
  延时指定毫秒，参数不能小于0。

#### **utime.sleep_us**(us)  
  延时指定微秒，参数不能小于0。

#### **utime.ticks_ms**()  
  返回不断递增的毫秒计数器，在某些值后会重新计数(未指定)。计数值本身无特定意义，只适合用在``ticks_diff()``。  
  注： 直接在这些值上执行标准数学运算（+，-）或关系运算符（<，>，>，> =）会导致无效结果。执行数学运算然后传递结果作为参数给`ticks_diff()` 或 ` ticks_add() ` 也将导致函数产生无效结果。

#### **utime.ticks_us**()  
  和上面 `ticks_ms()` 类似，只是返回微秒。

#### **utime.ticks_cpu**()  
  与 ``ticks_ms()`` 和 ``ticks_us()`` 类似，具有更高精度 (使用 CPU 时钟)，并非每个端口都实现此功能。

#### **utime.ticks_add**(ticks, delta)
  给定一个数字作为节拍的偏移值 `delta`，这个数字的值是正数或者负数都可以。
  给定一个 `ticks` 节拍值，本函数允许根据节拍值的模算数定义来计算给定节拍值之前或者之后 `delta` 个节拍的节拍值 。
  `ticks` 参数必须是 `ticks_ms()`, `ticks_us()`, or `ticks_cpu()` 函数的直接返回值。然而，`delta` 可以是一个任意整数或者是数字表达式。`ticks_add` 函数对计算事件/任务的截至时间很有用。（注意：必须使用 `ticksdiff()` 函数来处理
最后期限)。

代码示例：
```python
## 查找 100ms 之前的节拍值
print(utime.ticks_add(utime.ticks_ms(), -100))

## 计算操作的截止时间然后进行测试
deadline = utime.ticks_add(utime.ticks_ms(), 200)
while utime.ticks_diff(deadline, utime.ticks_ms()) > 0:
    do_a_little_of_something()

## 找出本次移植节拍值的最大值
print(utime.ticks_add(0, -1))
```

#### **utime.ticks_diff**(ticks1, ticks2)
   计算两次调用 `ticksms()`, `ticks_us()`, 或 `ticks_cpu()`之间的时间。因为这些函数的计数值可能会回绕，所以不能直接相减，需要使用 `ticks_diff()` 函数。“旧” 时间需要在 “新” 时间之前，否则结果无法确定。这个函数不要用在计算很长的时间 (因为 `ticks*()` 函数会回绕，通常周期不是很长)。通常用法是在带超时的轮询事件中调用:

代码示例：
```python
## 等待 GPIO 引脚有效，但是最多等待500微秒
start = time.ticks_us()
while pin.value() == 0:
    if time.ticks_diff(time.ticks_us(), start) > 500:
        raise TimeoutError
```

#### **utime.time**()
  返回从开始时间的秒数（整数），假设 `RTC` 已经按照前面方法设置好。如果 `RTC` 没有设置，函数将返回参考点开始计算的秒数 (对于 `RTC` 没有后备电池的板子，上电或复位后的情况)。如果你开发便携版的 MicroPython 应用程序，你不要依赖函数来提供超过秒级的精度。如果需要高精度，使用 `ticks_ms()` 和 `ticks_us()` 函数。如果需要日历时间，使用不带参数的 `localtime()` 是更好选择。

!!! tip "与 CPython 的区别"
    在 `CPython` 中，这个函数用浮点数返回从 `Unix` 开始时间（1970-01-01 00:00 `UTC`）的秒数，通常是毫秒级的精度。在 MicroPython 中，只有 `Unix` 版才使用相同开始时间，如果允许浮点精度，将返回亚秒精度。嵌入式硬件通常没有用浮点数表示长时间访问和亚秒精度，所以返回值是整数。一些嵌入式系统硬件不支持 `RTC` 电池供电方式，所以返回的秒数是从最后上电、或相对某个时间、以及特定硬件时间 (如复位)。

###   示例 

```
>>> import utime
>>> utime.sleep(1)           # sleep for 1 second
>>> utime.sleep_ms(500)      # sleep for 500 milliseconds
>>> utime.sleep_us(10)       # sleep for 10 microseconds
>>> start = utime.ticks_ms() # get value of millisecond counter
>>> delta = utime.ticks_diff(utime.ticks_ms(), start) # compute time difference
>>> delta
6928
>>> print(utime.ticks_add(utime.ticks_ms(), -100))
1140718
>>> print(utime.ticks_add(0, -1))
1073741823
```

更多内容可参考 [`utime`](http://docs.micropython.org/en/latest/pyboard/library/utime.html#module-utime)  。
