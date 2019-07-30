## HttpClient

本节介绍如何在 RT-Thread MicroPython 上使用 Http Client 功能，本章主要使用的模块为 `urequests` 。

### 获取并安装 urequests 模块

获取该模块有两种方式，详细操作可参考包管理章节：

- 方法1：使用 upip 包管理工具下载，这里使用 `upip.install("micropython-urequests")` 命令，upip 工具将自动下载并安装  `urequests` 模块，下载过程如图所示：

![1525690379859](../figures/install_urequests.png)

- 方法2：从 MicroPython-lib 中复制到开发板上文件系统的 `/libs/mpy` 目录下。

接下来 `urequests` 模块就可以被导入使用了。

### urequests 模块的使用

下面示例程序使用 `get` 命令来抓取 `http://www.baidu.com/` 的首页信息，并格式化输出：

```python
try:
    import urequests as requests
except ImportError:
    import requests

r = requests.get("http://www.baidu.com/")
print(r)
print(r.content)
print(r.text)
r.close()
```
