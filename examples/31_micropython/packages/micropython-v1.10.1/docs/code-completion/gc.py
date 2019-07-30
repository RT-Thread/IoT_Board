"""
gc 模块提供了垃圾收集器的控制接口。
"""

def enable() -> None:
    """允许自动回收内存碎片。"""
    ...

def disable() -> None:
    """禁止自动回收，但可以通过collect()函数进行手动回收内存碎片。"""
    ...

def collect() -> None:
    """运行一次垃圾回收。"""
    ...

 def mem_alloc() -> None:
    """返回已分配的内存数量。"""
    ...

def mem_free() -> None:
    """返回剩余的内存数量。"""
    ...
