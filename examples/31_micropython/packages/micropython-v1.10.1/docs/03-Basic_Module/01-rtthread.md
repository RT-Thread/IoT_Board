## **rtthread** – 系统相关函数

**rtthread** 模块提供了与 RT-Thread 操作系统相关的功能，如查看栈使用情况等。

### 函数

#### rtthread.current_tid()  
返回当前线程的 id 。

#### rtthread.is_preempt_thread()  
返回是否是可抢占线程。

#### rtthread.stacks_analyze()  
返回当前系统线程和栈使用信息。

### 示例

```
>>> import rtthread
>>> 
>>> rtthread.is_preempt_thread()       # determine if code is running in a preemptible thread
True
>>> rtthread.current_tid()             # current thread id
268464956
>>> rtthread.stacks_analyze()          # show thread information
thread     pri  status      sp     stack size max used left tick  error
---------- ---  ------- ---------- ----------  ------  ---------- ---
elog_async  31  suspend 0x000000a8 0x00000400    26%   0x00000003 000
tshell      20  ready   0x00000260 0x00001000    39%   0x00000003 000
tidle       31  ready   0x00000070 0x00000100    51%   0x0000000f 000
SysMonitor  30  suspend 0x000000a4 0x00000200    32%   0x00000005 000
timer        4  suspend 0x00000080 0x00000200    25%   0x00000009 000
>>> 
```
