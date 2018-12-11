# MQTT API 介绍

## 订阅列表

Paho MQTT 中采用订阅列表的形式进行多个 Topic 的订阅，订阅列表存储在 `MQTTClient` 结构体实例中，在 MQTT 启动前配置，如下所示：

```c
... // 省略代码

MQTTClient client;

... // 省略代码

/* set subscribe table and event callback */
client.messageHandlers[0].topicFilter = MQTT_SUBTOPIC;
client.messageHandlers[0].callback = mqtt_sub_callback;
client.messageHandlers[0].qos = QOS1;
```
详细的代码讲解请参考 Samples 章节，订阅列表的最大数量可以由 `menuconfig` 中的 `Max pahomqtt subscribe topic handlers` 选项进行配置。 

## callback
paho-mqtt 使用 callback 的方式向用户提供 MQTT 的工作状态以及相关事件的处理，需要在 `MQTTClient` 结构体实例中注册使用。

|callback 名称                           |描述|
|:-----                                  |:----|
|connect_callback                        |MQTT 连接成功的回调|
|online_callback                         |MQTT 客户端成功上线的回调|
|offline_callback                        |MQTT 客户端掉线的回调|
|defaultMessageHandler                   |默认的订阅消息接收回调|
|messageHandlers[x].callback             |订阅列表中对应的订阅消息接收回调|

用户可以使用 `defaultMessageHandler` 回调默认处理接收到的订阅消息，也可以使用 `messageHandlers` 订阅列表，为 `messageHandlers` 数组中对应的每一个 Topic 提供一个独立的订阅消息接收回调。

## MQTT_URI

paho-mqtt 中提供了 uri 解析功能，可以解析域名地址、ipv4 和 ipv6 地址，可解析 `tcp://` 和 `ssl://` 类型的 URI，用户只需要按照要求填写可用的 uri 即可。

- 示例 uri：

```
    domain 类型
    tcp://iot.eclipse.org:1883

    ipv4 类型
    tcp://192.168.10.1:1883
    ssl://192.168.10.1:1884

    ipv6 类型
    tcp://[fe80::20c:29ff:fe9a:a07e]:1883
    ssl://[fe80::20c:29ff:fe9a:a07e]:1884
```

## paho_mqtt_start 接口
- 功能：启动 MQTT 客户端，根据配置项订阅相应的主题。

- 函数原型：

```c
int paho_mqtt_start(MQTTClient *client)
```
- 函数参数：

|参数                               |描述|
|:-----                             |:----|
|client                             |MQTT 客户端实例对象|
|return                             |0 : 成功; 其他 : 失败|

## MQTT Publish 接口
- 功能：向指定的 Topic 主题发布 MQTT 消息。

- 函数原型：

```c
int MQTTPublish(MQTTClient *c, const char *topicName, MQTTMessage *message)
```

- 函数参数：

|参数                               |描述|
|:-----                             |:----|
|c                                  |MQTT 客户端实例对象|
|topicName                          |MQTT 消息发布主题|
|message                            |MQTT 消息内容|
|return                             |0 : 成功; 其他 : 失败|
