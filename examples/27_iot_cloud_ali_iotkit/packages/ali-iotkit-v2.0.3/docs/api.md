
# API 说明

**ali-iotkit** 是 RT-Thread 移植的用于连接阿里云 IoT 平台的软件包。基础 SDK 是阿里提供的 [**iotkit-embedded C-SDK**](https://github.com/aliyun/iotkit-embedded)。

这里引用阿里 **iotkit-embedded** API 使用说明，内容如下。

注：以下的 API 描述信息来自阿里云，更多详细内容请参阅 [iotkit-embedded **wiki**](https://github.com/aliyun/iotkit-embedded/wiki)。

## 必选 API

| 序号  | 函数名 | 说明  |
|:----  | :---- |:----|
|  1 | IOT_OpenLog         | 开始打印日志信息(log), 接受一个const char *为入参, 表示模块名字    |
|  2 | IOT_CloseLog        | 停止打印日志信息(log), 入参为空                                  |
|  3 | IOT_SetLogLevel     | 设置打印的日志等级, 接受入参从1到5, 数字越大, 打印越详细           |
|  4 | IOT_DumpMemoryStats | 调试函数, 打印内存的使用统计情况, 入参为1-5, 数字越大, 打印越详细  |


## MQTT 功能相关 API

| 序号  | 函数名   | 说明   |
|:----  | :---- |:----|
|  1    | IOT_SetupConnInfo            | MQTT连接前的准备, 基于`DeviceName + DeviceSecret + ProductKey`产生MQTT连接的用户名和密码等 |
|  2    | IOT_SetupConnInfoSecure      | MQTT连接前的准备, 基于`ID2 + DeviceSecret + ProductKey`产生MQTT连接的用户名和密码等,ID2模式启用|
|  3    | IOT_MQTT_CheckStateNormal    | MQTT连接后, 调用此函数检查长连接是否正常                                       |
|  4    | IOT_MQTT_Construct           | MQTT实例的构造函数, 入参为`iotx_mqtt_param_t`结构体, 连接MQTT服务器, 并返回被创建句柄 |
|  5    | IOT_MQTT_ConstructSecure     | MQTT实例的构造函数, 入参为`iotx_mqtt_param_t`结构体, 连接MQTT服务器, 并返回被创建句柄 ，ID2模式启用|
|  6    | IOT_MQTT_Destroy             | MQTT实例的摧毁函数, 入参为`IOT_MQTT_Construct()`创建的句柄                     |
|  7    | IOT_MQTT_Publish             | MQTT会话阶段, 组织一个完整的`MQTT Publish`报文, 向服务端发送消息发布报文       |
|  8    | IOT_MQTT_Subscribe           | MQTT会话阶段, 组织一个完整的`MQTT Subscribe`报文, 向服务端发送订阅请求         |
|  9    | IOT_MQTT_Unsubscribe         | MQTT会话阶段, 组织一个完整的`MQTT UnSubscribe`报文, 向服务端发送取消订阅请求   |
|  10   | IOT_MQTT_Yield               | MQTT会话阶段, MQTT主循环函数, 内含了心跳的维持, 服务器下行报文的收取等         |


## CoAP 功能相关 API

| 序号  | 函数名   | 说明   |
|:----  | :---- |:----|
|  1    | IOT_CoAP_Init                | CoAP实例的构造函数, 入参为`iotx_coap_config_t`结构体, 返回创建的CoAP会话句柄   |
|  2    | IOT_CoAP_Deinit              | CoAP实例的摧毁函数, 入参为`IOT_CoAP_Init()`所创建的句柄                        |
|  3    | IOT_CoAP_DeviceNameAuth      | 基于控制台申请的`DeviceName`, `DeviceSecret`, `ProductKey`做设备认证           |
|  4    | IOT_CoAP_GetMessageCode      | CoAP会话阶段, 从服务器的`CoAP Response`报文中获取`Respond Code`                |
|  5    | IOT_CoAP_GetMessagePayload   | CoAP会话阶段, 从服务器的`CoAP Response`报文中获取报文负载                      |
|  6    | IOT_CoAP_SendMessage         | CoAP会话阶段, 连接已成功建立后调用, 组织一个完整的CoAP报文向服务器发送         |
|  7    | IOT_CoAP_Yield               | CoAP会话阶段, 连接已成功建立后调用, 检查和收取服务器对`CoAP Request`的回复报文 |

## HTTP 功能相关 API

| 序号  | 函数名   | 说明   |
|:----  | :---- |:----|
|  1    | IOT_HTTP_Init                | Https实例的构造函数, 创建一个HTTP会话的句柄并返回                                      |
|  2    | IOT_HTTP_DeInit              | Https实例的摧毁函数, 销毁所有相关的数据结构                                            |
|  3    | IOT_HTTP_DeviceNameAuth      | 基于控制台申请的`DeviceName`, `DeviceSecret`, `ProductKey`做设备认证                   |
|  4    | IOT_HTTP_SendMessage         | Https会话阶段, 组织一个完整的HTTP报文向服务器发送,并同步获取HTTP回复报文               |
|  5    | IOT_HTTP_Disconnect          | Https会话阶段, 关闭HTTP层面的连接, 但是仍然保持TLS层面的连接                           |

## OTA 功能相关 API

| 序号  | 函数名   | 说明   |
|:----  | :---- |:----|
|  1    | IOT_OTA_Init                 | OTA实例的构造函数, 创建一个OTA会话的句柄并返回                                         |
|  2    | IOT_OTA_Deinit               | OTA实例的摧毁函数, 销毁所有相关的数据结构                                              |
|  3    | IOT_OTA_Ioctl                | OTA实例的输入输出函数, 根据不同的命令字可以设置OTA会话的属性, 或者获取OTA会话的状态    |
|  4    | IOT_OTA_GetLastError         | OTA会话阶段, 若某个`IOT_OTA_*()`函数返回错误, 调用此接口可获得最近一次的详细错误码     |
|  5    | IOT_OTA_ReportVersion        | OTA会话阶段, 向服务端汇报当前的固件版本号                                              |
|  6    | IOT_OTA_FetchYield           | OTA下载阶段, 在指定的`timeout`时间内, 从固件服务器下载一段固件内容, 保存在入参buffer中 |
|  7    | IOT_OTA_IsFetchFinish        | OTA下载阶段, 判断迭代调用`IOT_OTA_FetchYield()`是否已经下载完所有的固件内容            |
|  8    | IOT_OTA_IsFetching           | OTA下载阶段, 判断固件下载是否仍在进行中, 尚未完成全部固件内容的下载                    |
|  9    | IOT_OTA_ReportProgress       | 可选API, OTA下载阶段, 调用此函数向服务端汇报已经下载了全部固件内容的百分之多少         |
|  10   | IOT_OTA_RequestImage         | 可选API，向服务端请求固件下载                                                          |
|  11   | IOT_OTA_GetConfig            | 可选API，向服务端请求远程配置                                                          |

## 云端连接 Cloud Connection 功能相关 API

| 序号  | 函数名   | 说明   |
|:----  | :---- |:----|
|  1    | IOT_Cloud_Connection_Init    | 云端连接实例的构造函数, 入参为`iotx_cloud_connection_param_pt`结构体, 返回创建的云端连接会话句柄   |
|  2    | IOT_Cloud_Connection_Deinit  | 云端连接实例的摧毁函数, 入参为`IOT_Cloud_Connection_Init()`所创建的句柄                        |
|  3    | IOT_Cloud_Connection_Send_Message      | 发送数据给云端           |
|  4    | IOT_Cloud_Connection_Yield   | 云端连接成功建立后，收取服务器发送的报文                |

## CMP 功能相关 API

| 序号  | 函数名   | 说明   |
|:----  | :---- |:----|
|  1    | IOT_CMP_Init                 | CMP实例的构造函数, 入参为`iotx_cmp_init_param_pt`结构体，只存在一个CMP实例     |
|  2    | IOT_CMP_Register             | 通过CMP订阅服务                                                                |
|  3    | IOT_CMP_Unregister           | 通过CMP取消服务订阅                                                            |
|  4    | IOT_CMP_Send                 | 通过CMP发送数据，可以送给云端，也可以送给本地设备                              |
|  5    | IOT_CMP_Send_Sync            | 通过CMP同步发送数据   ，暂不支持                                               |
|  6    | IOT_CMP_Yield                | 通过CMP接收数据，单线程情况下才支持                                            |
|  7    | IOT_CMP_Deinit               | CMP示例的摧毁函数                                                              |
|  8    | IOT_CMP_OTA_Start            | 初始化ota功能，上报版本                                                        |
|  9    | IOT_CMP_OTA_Set_Callback     | 设置OTA回调函数                                                                |
|  10   | IOT_CMP_OTA_Get_Config       | 获取远程配置                                                                   |
|  11   | IOT_CMP_OTA_Request_Image    | 获取固件                                                                       |
|  12   | IOT_CMP_OTA_Yield            | 通过CMP完成OTA功能                                                             |


## 设备影子相关(可选功能) API

| 序号  | 函数名   | 说明   |
|:----  | :---- |:----|
|  1    | IOT_Shadow_Construct            | 建立一个设备影子的MQTT连接, 并返回被创建的会话句柄                              |
|  2    | IOT_Shadow_Destroy              | 摧毁一个设备影子的MQTT连接, 销毁所有相关的数据结构, 释放内存, 断开连接          |
|  3    | IOT_Shadow_Pull                 | 把服务器端被缓存的JSON数据下拉到本地, 更新本地的数据属性                        |
|  4    | IOT_Shadow_Push                 | 把本地的数据属性上推到服务器缓存的JSON数据, 更新服务端的数据属性                |
|  5    | IOT_Shadow_Push_Async           | 和`IOT_Shadow_Push()`接口类似, 但是异步的, 上推后便返回, 不等待服务端回应       |
|  6    | IOT_Shadow_PushFormat_Add       | 向已创建的数据类型格式中增添成员属性                                            |
|  7    | IOT_Shadow_PushFormat_Finalize  | 完成一个数据类型格式的构造过程                                                  |
|  8    | IOT_Shadow_PushFormat_Init      | 开始一个数据类型格式的构造过程                                                  |
|  9    | IOT_Shadow_RegisterAttribute    | 创建一个数据类型注册到服务端, 注册时需要`*PushFormat*()`接口创建的数据类型格式  |
| 10    | IOT_Shadow_DeleteAttribute      | 删除一个已被成功注册的数据属性                                                  |
| 11    | IOT_Shadow_Yield                | MQTT的主循环函数, 调用后接受服务端的下推消息, 更新本地的数据属性                |

## 主子设备相关(可选功能) API

| 序号  | 函数名   | 说明   |
|:----  | :---- |:----|
|  1    | IOT_Gateway_Construct           | 建立一个主设备，建立MQTT连接, 并返回被创建的会话句柄                            |
|  2    | IOT_Gateway_Destroy             | 摧毁一个主设备的MQTT连接, 销毁所有相关的数据结构, 释放内存, 断开连接            |
|  3    | IOT_Subdevice_Login             | 子设备上线，通知云端建立子设备session                                           |
|  4    | IOT_Subdevice_Logout            | 子设备下线，销毁云端建立子设备session及所有相关的数据结构, 释放内存             |
|  5    | IOT_Gateway_Yield               | MQTT的主循环函数, 调用后接受服务端的下推消息                                    |
|  6    | IOT_Gateway_Subscribe           | 通过MQTT连接向服务端发送订阅请求                                                |
|  7    | IOT_Gateway_Unsubscribe         | 通过MQTT连接向服务端发送取消订阅请求                                            |
|  8    | IOT_Gateway_Publish             | 通过MQTT连接服务端发送消息发布报文                                              |
|  9    | IOT_Gateway_RRPC_Register       | 注册设备的RRPC回调函数，接收云端发起的RRPC请求                                  |
| 10    | IOT_Gateway_RRPC_Response       | 对云端的RRPC请求进行应答                                                        |
| 11    | IOT_Gateway_Generate_Message_ID | 生成消息id                                                                      |
| 12    | IOT_Gateway_Get_TOPO            | 向topo/get topic发送包并等待回复（TOPIC_GET_REPLY 回复）                        |
| 13    | IOT_Gateway_Get_Config          | 向conifg/get topic发送包并等待回复（TOPIC_CONFIG_REPLY 回复）                   |
| 14    | IOT_Gateway_Publish_Found_List  | 发现设备列表上报                                                                |

## linkkit 功能相关 API

| 序号  | 函数名   | 说明   |
|:----  | :---- |:----|
|  1    | linkkit_start                   | 启动 linkkit 服务，与云端建立连接并安装回调函数                                 |
|  2    | linkkit_end                     | 停止 linkkit 服务，与云端断开连接并回收资源                                     |
|  3    | linkkit_dispatch                | 事件分发函数,触发 linkkit_start 安装的回调                                      |
|  4    | linkkit_yield                   | linkkit 主循环函数，内含了心跳的维持, 服务器下行报文的收取等;如果允许多线程，请不要调用此函数     |
|  5    | linkkit_set_value               | 根据identifier设置物对象的 TSL 属性，如果标识符为struct类型、event output类型或者service output类型，使用点'.'分隔字段；例如"identifier1.identifier2"指向特定的项    |
|  6    | linkkit_get_value               | 根据identifier获取物对象的 TSL 属性                                             |
|  7    | linkkit_set_tsl                 | 从本地读取 TSL 文件,生成物的对象并添加到 linkkit 中                             |
|  8    | linkkit_answer_service          | 对云端服务请求进行回应                                                          |
|  9    | linkkit_invoke_raw_service      | 向云端发送裸数据                                                                |
| 10    | linkkit_trigger_event           | 上报设备事件到云端                                                              |
| 11    | linkkit_fota_init               | 初始化 OTA-fota 服务，并安装回调函数(需编译设置宏 SERVICE_OTA_ENABLED )              |
| 12    | linkkit_invoke_fota_service     | 执行fota服务                                                                    |
| 13    | linkkit_fota_init               | 初始化 OTA-cota 服务，并安装回调函数(需编译设置宏 SERVICE_OTA_ENABLED SERVICE_COTA_ENABLED )     |
| 14    | linkkit_invoke_cota_get_config  | 设备请求远程配置                                                                    |
| 15    | linkkit_invoke_cota_service     | 执行cota服务                                                                    |
