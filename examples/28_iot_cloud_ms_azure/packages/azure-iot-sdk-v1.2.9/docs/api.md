# API 说明

在使用 Azure 编写应用的过程中，使用 **IoTHubClient** 库就可以轻松发送和接收消息，本章介绍 **IoTHubClient** 库提供的常用 API。更多详细的 API 请参考 Azure SDK 的 [C API 参考](https://azure.github.io/azure-iot-sdk-c/index.html)。

## 编程模型选择

IoTHubClient 库提供两套简单易用的 API，一套 API 的名称中包含 “LL”，另外一套则不包含，名称中包含 “LL” 的API 级别较低。无论选择带 “LL” 的 API 还是选择不带 “LL” 的 API，都必须前后相一致。 如果首先调用**IoTHubClient_LL_CreateFromConnectionString**，则对于任何后续工作，请务必只使用相应的较低级别的 API：

- IoTHubClient_LL_SendEventAsync
- IoTHubClient_LL_SetMessageCallback
- IoTHubClient_LL_Destroy
- IoTHubClient_LL_DoWork

相反的情况也成立。 如果首先调用 **IoTHubClient_CreateFromConnectionString**，请使用非 LL API 进行任何其他处理。

这些函数的 API 名称中包含“LL”。 除此之外，其中每个函数的参数都与其非 LL 的对等项相同。 但是，这些函数的行为有一个重要的差异。

当你调用 IoTHubClient_CreateFromConnectionString 时，基础库将创建在后台运行的新线程。 此线程将事件发送到 IoT 中心以及从 IoT 中心接收消息。 使用 “LL” API 时则不会创建此类线程。 创建后台线程是为了给开发人员提供方便。 你无需担心如何明确与 IoT 中心相互发送和接收消息，因为此操作会在后台自动进行。 相比之下，借助 “LL”API，可根据需要明确控制与 IoT 中心的通信。

## IoTHubClient 常用 API 

下面以名称中包含 “LL” 的 API 为例介绍常用 API。

### 物联网中心客户端初始化

```c
IOTHUB_CLIENT_LL_HANDLE IoTHubClient_LL_CreateFromConnectionString(
                                const char* connectionString,
                                IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol)
```

使用指定的连接字符串参数创建一个物联网中心客户端来与现有的物联网中心通信。

| 参数              | 描述                                |
|:------------------|:------------------------------------|
|connectionString               | 设备连接字符串 |
|protocol | 协议实现的函数指针 |
| **返回**          | **描述**                                |
|IOTHUB_CLIENT_LL_HANDLE                  | 一个非空的 iothubclientllhandle 值，它在调用物联网中心客户端的其他函数时使用 |
|NULL                 | 失败                                |

### 物联网中心客户端资源释放

```c
void IoTHubDeviceClient_LL_Destroy(IOTHUB_DEVICE_CLIENT_LL_HANDLE iotHubClientHandle)
```

释放物联网中心客户端分配的资源。这是一个阻塞的调用。

| 参数              | 描述                                |
|:------------------|:------------------------------------|
|iotHubClientHandle               | 设备连接字符串 |
| **返回**          | **描述**                                |
|无                  | 无             |

### 设置发送事件回调函数

```c
IOTHUB_CLIENT_RESULT IoTHubClient_LL_SendEventAsync(
                IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, 
                IOTHUB_MESSAGE_HANDLE eventMessageHandle, 
                IOTHUB_CLIENT_EVENT_CONFIRMATION_CALLBACK eventConfirmationCallback, 
                void* userContextCallback)
```

异步调用发送由 eventMessageHandle 指定的消息。

| 参数              | 描述                                |
|:------------------|:------------------------------------|
|iotHubClientHandle               | 已创建的物联网中心客户端句柄 |
|eventMessageHandle | 物联网中心消息句柄 |
|eventConfirmation  Callback | 由设备指定的回调，用于接收物联网中心消息的交付确认。  这个回调可以被同一消息调用：当 iothubclientllsendeventasync 函数，试图重试发送一条失败的消息。用户可以在这里指定一个NULL值，以表明不需要回调 |
|userContextCallback | 用户指定的上下文将被提供给回调，可以为空 |
| **返回**          | **描述**                                |
|IOTHUB_CLIENT_OK                  | 设置成功 |
|error code                 | 失败                                |

### 设置接收消息回调函数

```c
IOTHUB_CLIENT_RESULT IoTHubDeviceClient_LL_SetMessageCallback(
                                IOTHUB_DEVICE_CLIENT_LL_HANDLE iotHubClientHandle, 
                                IOTHUB_CLIENT_MESSAGE_CALLBACK_ASYNC messageCallback, 
                                void* userContextCallback)
```

当物联网中心向设备发送消息时，设置要调用的消息回调。这是一个阻塞的调用。

| 参数              | 描述                                |
|:------------------|:------------------------------------|
|iotHubClientHandle               | 已创建的物联网中心客户端句柄 |
|messageCallback | 由设备指定的回调，用于接收来自物联网中心的消息 |
|userContextCallback | 用户指定的上下文将被提供给回调，可以为空 |
| **返回**          | **描述**                                |
|IOTHUB_CLIENT_RESULT                  | 初始化 IoT 客户端成功              |
|NULL                 | 失败                                |

### 设置物理中心客户端

```c
IOTHUB_CLIENT_RESULT IoTHubDeviceClient_LL_SetOption(
                            IOTHUB_DEVICE_CLIENT_LL_HANDLE iotHubClientHandle, 
                            const char* optionName, 
                            const void* value)
```

这个API设置了一个由参数 `optionName` 标识的运行时选项，选项名和数据类型值指向每个选项都是特定的。

| 参数              | 描述                                |
|:------------------|:------------------------------------|
|iotHubClientHandle               | 已创建的物联网中心客户端句柄 |
|optionName | 选项名称 |
|value | 特定值 |
| **返回**          | **描述**                                |
|IOTHUB_CLIENT_OK                  | 设置成功         |
|error code                 | 失败                                |

### 设置连接状态回调

```c
IOTHUB_CLIENT_RESULT IoTHubDeviceClient_LL_SetConnectionStatusCallback(
                            IOTHUB_DEVICE_CLIENT_LL_HANDLE iotHubClientHandle, 
                            IOTHUB_CLIENT_CONNECTION_STATUS_CALLBACK connectionStatusCallback, 
                            void * userContextCallback)
```

设置要调用的连接状态回调，表示连接到物联网中心的状态。这是一个阻塞的调用。

| 参数              | 描述                                |
|:------------------|:------------------------------------|
|iotHubClientHandle               | 已创建的物联网中心客户端句柄 |
|connectionStatusCallback | 设备指定的回调，用于接收关于连接到物联网中心的状态的更新 |
|userContextCallback | 用户指定的上下文将被提供给回调，可以为空 |
| **返回**          | **描述**                                |
|IOTHUB_CLIENT_OK                  | 设置成功         |
|error code                 | 失败                                |

### 完成（发送/接收）工作

```c
void IoTHubDeviceClient_LL_DoWork(IOTHUB_DEVICE_CLIENT_LL_HANDLE iotHubClientHandle)
```

当工作（发送/接收）可以由 IoTHubClient 完成时，该函数将被用户调用。所有 IoTHubClient 交互（关于网络流量和/或用户级回调）都是调用这个函数的效果，它们在 DoWork 中同步进行。 

| 参数              | 描述                                |
|:------------------|:------------------------------------|
|iotHubClientHandle               | 已创建的物联网中心客户端句柄 |
| **返回**          | **描述**                                |
|无                  | 无         |
