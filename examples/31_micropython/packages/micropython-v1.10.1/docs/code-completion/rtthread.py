"""
rtthread 模块提供了与 RT-Thread 操作系统相关的功能，如查看栈使用情况等。
"""

def current_tid() -> None:
    """返回当前线程的 id 。"""
    ...

def is_preempt_thread() -> None:
    """返回是否是可抢占线程。"""
    ...

def stacks_analyze() -> None:
    """返回当前系统线程和栈使用信息。"""
    ...
	
def list_device() -> None:
    """列出当前板卡上可使用的设备信息，包括设备名和设备类型。"""
    ...
