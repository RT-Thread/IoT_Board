# OneNET 工作原理



OneNET 软件包数据的上传和命令的接收是基于 MQTT 实现的，OneNET 的初始化其实就是 MQTT 客户端的初始化，初始化完成后，MQTT 客户端会自动连接 OneNET 平台。数据的上传其实就是往特定的 topic 发布消息。当服务器有命令或者响应需要下发时，会将消息推送给设备。

获取数据流、数据点，发布命令则是基于 HTTP Client 实现的，通过 POST 或 GET 将相应的请求发送给 OneNET 平台，OneNET 将对应的数据返回，这样，我们就能在网页上或者手机 APP 上看到设备上传的数据了。

下图是应用显示设备上传数据的流程图

![onenet_upload](figures/onenet_upload.png)

下图是应用下发命令给设备的流程图

![onenet_send_cmd](figures/onenet_send_cmd.png)
