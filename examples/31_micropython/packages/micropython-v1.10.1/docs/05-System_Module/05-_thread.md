## _thread – 多线程支持

`_thread` 模块提供了用于处理多线程的基本方法——多个控制线程共享它们的全局数据空间。为了实现同步，提供了简单的锁（也称为互斥锁或二进制信号量）。

### 示例

```python
import _thread
import time
def testThread():
    while True:
        print("Hello from thread")
        time.sleep(2)

_thread.start_new_thread(testThread, ())
while True:
    pass
```

输出结果（启动新的线程，每隔两秒打印字符）：

Hello from thread
Hello from thread
Hello from thread
Hello from thread
Hello from thread

更多内容可参考 [_thread](http://docs.micropython.org/en/latest/pyboard/library/_thread.html)  。
