## MQTT

MQTT 是一种基于发布/订阅（publish/subscribe）模式的“轻量级”通讯协议 。本章介绍如何在 RT-Thread MicroPython 上使用 MQTT 功能，使用到的模块为 `umqtt.simple` 模块。

### 获取并安装 umqtt.simple 模块

同样的可以使用包管理中的两种方式来获取，使用 upip 安装的方式可使用  `upip.install("micropython-umqtt.simple")`如图：

![1525690229174](../figures/install_umqtt_simple.png)

### umqtt.simple 模块的使用

####  MQTT 订阅功能

- 使用 `iot.eclipse.org` 作为测试服务器

```python
import time
from umqtt.simple import MQTTClient

# Publish test messages e.g. with:
# mosquitto_pub -t foo_topic -m hello

# Received messages from subscriptions will be delivered to this callback
def sub_cb(topic, msg):
    print((topic, msg))

def main(server="iot.eclipse.org"):   # 测试 server 为 iot.eclipse.org
    c = MQTTClient("RT-Thread", server)
    c.set_callback(sub_cb)
    c.connect()
    c.subscribe(b"foo_topic")         # 订阅 foo_topic 主题
    while True:
        if True:
            # Blocking wait for message
            c.wait_msg()
        else:
            # Non-blocking wait for message
            c.check_msg()
            # Then need to sleep to avoid 100% CPU usage (in a real
            # app other useful actions would be performed instead)
            time.sleep(1)

    c.disconnect()

if __name__ == "__main__":
    main()
```

- 使用 python 命令执行上述代码文件，就会连接上 MQTT 服务器，可收到我们从另一个客户端发布的以 `foo_topic`为主题的内容

![1525665942426](../figures/sub_topic.png)

####  MQTT 发布功能

- 执行下面的代码后将向 MQTT 服务器发布以 `foo_topic` 为主题的信息

```python
from umqtt.simple import MQTTClient

# Test reception e.g. with:
# mosquitto_sub -t foo_topic

def main(server="iot.eclipse.org"):
    c = MQTTClient("SummerGift", server)
    c.connect()
    c.publish(b"foo_topic", b"Hello RT-Thread !!!")
    c.disconnect()

if __name__ == "__main__":
    main()

```
