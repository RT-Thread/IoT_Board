# OneNET 软件包介绍 #

OneNET 平台是中国移动基于物联网产业打造的生态平台，具有高并发可用、多协议接入、丰富 API 支持、数据安全存储、快速应用孵化等特点，同时，OneNET 平台还提供全方位支撑，加速用户产品的开发速度。

OneNET平台是一个基于物联网产业特点打造的生态环境，可以适配各种网络环境和协议类型，现在支持的协议有LWM2M（NB-IOT）、EDP、MQTT、HTTP、MODBUS、JT\T808、TCP透传、RGMP等。用户可以根据不同的应用场景选择不同的接入协议。

该组件包是 RT-Thread 系统针对 OneNET 平台连接的适配，通过这个组件包可以让设备在 RT-Thread 上非常方便的连接 OneNet 平台，完成数据的发送、接收、设备的注册和控制等功能。

## 文件目录结构

``` {.c}
OneNET
│   README.md                       // 软件包使用说明
│   SConscript                      // RT-Thread 默认的构建脚本
├───docs 
│   └───figures                     // 文档使用图片
│   │   api.md                      // API 使用说明
│   │   introduction.md             // 软件包详细介绍
│   │   principle.md                // 实现原理
│   │   README.md                   // 文档结构说明
│   │   samples.md                  // 软件包示例
│   │   user-guide.md               // 使用说明
│   │   port.md                     // 移植说明文档
│   └───version.md                  // 版本
├───ports                           // 移植文件                 
│   └───rt_ota_key_port.c           // 移植文件模板
├───samples                         // 示例代码
│   └───onenet_sample.c             // 软件包应用示例代码
├───inc                             // 头文件
└───src                             // 源文件
```

## OneNET 软件包功能特点 ##

RT-Thread OneNET 软件包功能特点如下：

**断线重连**

RT-Thread OneNET 软件包实现了断线重连机制，在断网或网络不稳定导致连接断开时，会维护登陆状态，重新连接，并自动登陆 OneNET 平台。提高连接的可靠性，增加了软件包的易用性。

**自动注册**

RT-Thread OneNET 软件包实现了设备自动注册功能。不需要在 web 页面上手动的一个一个创建设备，输入设备名字和鉴权信息。当开启设备注册功能后，设备第一次登陆 OneNET 平台时，会自动调用注册函数向 OneNET 平台注册设备，并将返回的设备信息保存下来，用于下次登陆。

**自定义响应函数**

RT-Thread OneNET 软件包提供了一个命令响应回调函数，当 OneNET 平台下发命令后，RT-Thread 会自动调用命令响应回调函数，用户处理完命令后，返回要发送的响应内容，RT-Thread 会自动将响应发回 OneNET 平台。

**自定义 topic 和回调函数**

RT-Thread OneNET 软件包除了可以响应 OneNET 官方 topic 下发的命令外，还可以订阅用户自定义的 topic，并为每个 topic 单独设置一个命令处理回调函数。方便用户开发自定义功能。

**上传二进制数据**

RT-Thread OneNET 软件包除了上传数字和字符串外，还支持二进制文件上传。当启用了 RT-Thread 的文件系统后，可以直接将文件系统内的文件以二进制的方式上传至云端。

