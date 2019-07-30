"""
uselect 模块提供了等待数据流的事件功能。
"""

def select(rlist, wlist, xlist) -> None:
    """
    监控对象何时可读或可写，一旦监控的对象状态改变，返回结果（阻塞线程）。这个函数是为了兼容，效率不高，推荐用 poll 函数 。

    - select.select(rlist, wlist, xlist[, timeout])

    - rlist：等待读就绪的文件描述符数组
    - wlist：等待写就绪的文件描述符数组
    - xlist：等待异常的数组
    - timeout：等待时间（单位：秒）

    示例：

    - def selectTest():
    - global s
    - rs, ws, es = select.select([s,], [], [])
    - #程序会在此等待直到对象s可读
    - print(rs)
    - for i in rs:
    -     if i == s:
    -     print("s can read now")
    -     data,addr=s.recvfrom(1024)
    -     print('received:',data,'from',addr)"""
    ...

class poll(string):
    """
    创建 poll 实例。

    示例：

    - poller = select.poll()
    - print(poller)
    - <poll>
    """
    def __init__(self) -> None:
    ...

    def register(obj) -> None:
        """
        - register(obj[, eventmask])
        注册一个用以监控的对象，并设置被监控对象的监控标志位 flag。

        - obj：被监控的对象
        - flag：被监控的标志
        -     select.POLLIN — 可读
        -     select.POLLHUP — 已挂断
        -     select.POLLERR — 出错
        -     select.POLLOUT — 可写
        """
        ...

    def unregister(obj) -> None:
        """
        解除监控的对象的注册。

        - obj：注册过的对象

        示例：

        - READ_ONLY = select.POLLIN | select.POLLHUP | select.POLLERR
        - READ_WRITE = select.POLLOUT | READ_ONLY
        - poller.register(s, READ_WRITE)
        - poller.unregister(s)
        """
        ...

    def modify(obj, eventmask) -> None:
        """
        修改已注册的对象监控标志。

        - obj：已注册的被监控对象
        - flag：修改为的监控标志
        
        示例：

        - READ_ONLY = select.POLLIN | select.POLLHUP | select.POLLERR
        - READ_WRITE = select.POLLOUT | READ_ONLY
        - poller.register(s, READ_WRITE)
        - poller.modify(s, READ_ONLY)
        """
        ...

    def poll(timeout) -> None:
        """
        - poll([timeout])
        等待至少一个已注册的对象准备就绪。
        返回 (obj, event, ...) 元组, event 元素指定了一个流发生的事件，是上面所描述的 select.POLL*常量组合。 
        根据平台和版本的不同，在元组中可能有其他元素，所以不要假定元组的大小是 2 。如果超时，则返回空列表。
        """
        ...

POLLIN = ...  # type: int
POLLOUT = ...  # type: int
POLLERR = ...  # type: int
POLLHUP = ...  # type: int
