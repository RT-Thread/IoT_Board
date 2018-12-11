# 工作原理

## IoT 客户端框架

Azure IoT 中心为了方便设备连接提供了丰富的连接协议，如 MQTT、HTTP 等，同时 Azure  IoT 中心只支持安全连接。与 IoT 中心的连接由设备客户端来完成，每一个连接到 IoT 中心的设备都会创建一个 IoT 中心客户端实例，当连接关闭时，将这个实例释放掉即可。

IoT 中心客户端会向下调用 LL 层来完成工作，LL 层向下对接不同通信协议的传输层，传输层向下对接通信协议实现层。下面两幅图展示了 IoT 中心客户端完成功能时的调用层次关系：

- IoT 客户端框架 HTTP/MQTT 功能调用关系图：

![HTTP/MQTT 功能调用关系图](figures/iot_client.png)

- 下图以 HTTP 协议为例展示 API 调用情况：

![HTTP 协议 API 调用](figures/http_api_invoke.png)

