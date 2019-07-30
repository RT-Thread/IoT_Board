## **uheapq** – 堆排序算法

`uheapq` 模块提供了堆排序相关算法，堆队列是一个列表，它的元素以特定的方式存储。

### 函数

#### **uheapq.heappush**(heap, item)  
将对象压入堆中。

#### **uheapq.heappop**(heap)  
从 heap 弹出第一个元素并返回。 如果是堆时空的会抛出 IndexError。

#### **uheapq.heapify**(x)  
将列表 x 转换成堆。

更多内容可参考 [uheapq](http://docs.micropython.org/en/latest/pyboard/library/uheapq.html)  。
