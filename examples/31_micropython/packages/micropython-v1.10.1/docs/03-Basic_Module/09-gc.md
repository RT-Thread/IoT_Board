## **gc** – 控制垃圾回收

**gc** 模块提供了垃圾收集器的控制接口。

### 函数

#### **gc.enable**()  
允许自动回收内存碎片。 

#### **gc.disable**()  
禁止自动回收，但可以通过collect()函数进行手动回收内存碎片。 

#### **gc.collect**()  
运行一次垃圾回收。

#### **gc.mem_alloc**()  
返回已分配的内存数量。 

#### **gc.mem_free**()  
返回剩余的内存数量。 

更多内容可参考  [gc](http://docs.micropython.org/en/latest/pyboard/library/gc.html) 。
