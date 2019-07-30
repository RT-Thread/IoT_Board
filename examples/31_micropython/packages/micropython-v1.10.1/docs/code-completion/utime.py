"""
utime 模块提供获取当前时间和日期、测量时间间隔和延迟的功能。
"""

def localtime(secs : int) -> None:
    """
    从初始时间的秒转换为元组: (年, 月, 日, 时, 分, 秒, 星期, yearday) 。如果 secs 是空或者 None，那么使用当前时间。
    year 年份包括世纪（例如2014）。

    - month 范围 1-12
    - day 范围 1-31
    - hour 范围 0-23
    - minute 范围 0-59
    - second 范围 0-59
    - weekday 范围 0-6 对应周一到周日
    - yearday 范围 1-366
    """
    ...

def mktime(time : tuple) -> None:
    """时间的反函数，它的参数是完整8参数的元组，返回值一个整数自2000年1月1日以来的秒数。"""
    ...

def sleep(seconds) -> None:
    """休眠指定的时间（秒），Seconds 可以是浮点数。注意有些版本的 MicroPython不支持浮点数，为了兼容可以使用 sleep_ms() 和 sleep_us()函数。"""
    ...

def sleep_ms(ms) -> None:
    """延时指定毫秒，参数不能小于0。"""
    ...

def sleep_us(us) -> None:
    """延时指定微秒，参数不能小于0。"""
    ...

def ticks_ms() -> None:
    """
    返回不断递增的毫秒计数器，在某些值后会重新计数(未指定)。
    计数值本身无特定意义，只适合用在ticks_diff()。
    注： 直接在这些值上执行标准数学运算（+，-）或关系运算符（<，>，>，> =）会导致无效结果。
    执行数学运算然后传递结果作为参数给ticks_diff() 或 ticks_add() 也将导致函数产生无效结果。
    """
    ...

def ticks_us() -> None:
    """
    返回不断递增的微秒计数器，在某些值后会重新计数(未指定)。
    计数值本身无特定意义，只适合用在ticks_diff()。
    注： 直接在这些值上执行标准数学运算（+，-）或关系运算符（<，>，>，> =）会导致无效结果。
    执行数学运算然后传递结果作为参数给ticks_diff() 或 ticks_add() 也将导致函数产生无效结果。
    """
    ...

def ticks_cpu() -> None:
    """与 ticks_ms() 和 ticks_us() 类似，具有更高精度 (使用 CPU 时钟)，并非每个端口都实现此功能。"""
    ...

def ticks_add(ticks, delta) -> None:
    """
    给定一个数字作为节拍的偏移值 delta，这个数字的值是正数或者负数都可以。 
    给定一个 ticks 节拍值，本函数允许根据节拍值的模算数定义来计算给定节拍值之前或者之后 delta 个节拍的节拍值 。 
    ticks 参数必须是 ticks_ms(), ticks_us(), or ticks_cpu() 函数的直接返回值。
    然而，delta 可以是一个任意整数或者是数字表达式。ticks_add 函数对计算事件/任务的截至时间很有用。
    （注意：必须使用 ticksdiff() 函数来处理 最后期限)。
    """
    ...

def ticks_diff(ticks1, ticks2) -> None:
    """
    计算两次调用 ticksms(), ticks_us(), 或 ticks_cpu()之间的时间。
    因为这些函数的计数值可能会回绕，所以不能直接相减，需要使用 ticks_diff() 函数。
    “旧” 时间需要在 “新” 时间之前，否则结果无法确定。
    这个函数不要用在计算很长的时间 (因为 ticks*() 函数会回绕，通常周期不是很长)。
    通常用法是在带超时的轮询事件中调用:
    代码示例：
    # 等待 GPIO 引脚有效，但是最多等待500微秒

    - start = time.ticks_us()
    - while pin.value() == 0:
    -     if time.ticks_diff(time.ticks_us(), start) > 500:
    -         raise TimeoutError
    """
    ...

def time() -> None:
    """
    返回从开始时间的秒数（整数），假设 RTC 已经按照前面方法设置好。
    如果 RTC 没有设置，函数将返回参考点开始计算的秒数 (对于 RTC 没有后备电池的板子，上电或复位后的情况)。
    如果你开发便携版的 MicroPython 应用程序，你不要依赖函数来提供超过秒级的精度。
    如果需要高精度，使用 ticks_ms() 和 ticks_us() 函数。
    如果需要日历时间，使用不带参数的 localtime() 是更好选择。"""
    ...
