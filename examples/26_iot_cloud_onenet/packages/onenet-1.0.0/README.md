# OneNET

## 1、介绍

[OneNET](https://open.iot.10086.cn/) 平台是中国移动基于物联网产业打造的生态平台，具有高并发可用、多协议接入、丰富 API 支持、数据安全存储、快速应用孵化等特点，同时，OneNET 平台还提供全方位支撑，加速用户产品的开发速度。

OneNET 软件包是 RT-Thread 针对 OneNET 平台连接做的的适配，通过这个软件包，可以让设备在 RT-Thread 上非常方便的连接 OneNet 平台，完成数据的发送、接收、设备的注册和控制等功能。

软件包具有以下优点：

- 断线重连
- 自动注册
- 自定义响应函数
- 自定义 topic 和 topic 对应的回调函数
- 上传二进制数据

更多介绍请查看[详细介绍](.\docs\introduction.md)

### 1.1 目录结构

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
│   └───onenet_port.c               // 移植文件模板
├───samples                         // 示例代码
│   └───onenet_sample.c             // 软件包应用示例代码
├───inc                             // 头文件
└───src                             // 源文件
```

### 1.2 许可证

OneNET package  遵循 GUN GPL 许可，详见 `LICENSE` 文件。

### 1.3 依赖

- RT_Thread 3.0+
- [paho-mqtt](https://github.com/RT-Thread-packages/paho-mqtt.git)
- [webclient](https://github.com/RT-Thread-packages/webclient.git)
- [cJSON](https://github.com/RT-Thread-packages/cJSON.git)

## 2、获取方式

使用 `OneNET package` 需要在 RT-Thread 的包管理中选中它，具体路径如下：

```{.c}
RT-Thread online packages
    IoT - internet of things  --->
        IoT Cloud  --->
            [*] OneNET: China Mobile OneNet cloud SDK for RT-Thread
```

进入 onenet 软件包的配置菜单按下图所示配置，里面的信息依据自己的产品和设备的**实际情况**填写

```{.c}
--- OneNET: China Mobile OneNet cloud SDK for RT-Thread                            
    [ ]   Enable OneNET sample                                                  
    [*]   Enable support MQTT protocol                                                 
    [ ]   Enable OneNET automatic register device (NEW)                             
    (35936966) device id                                                             
    (201807171718) auth info
    (H3ak5Bbl0NxpW3QVVe33InnPxOg=) api key                                              
    (156418) product id                                                                 
    (dVZ=ZjVJvGjXIUDsbropzg1a8Dw=) master/product apikey (NEW)                       
        version (latest)  --->
```

**Enable OneNET sample** ：开启 OneNET 示例代码

**Enable support MQTT protocol** ：开启 MQTT 协议连接 OneNET 支持

**Enable OneNET automatic register device** ：开启  OneNET 自动注册设备功能

**device id** ：配置云端创建设备时获取的 `设备ID`

**auth info** ：配置云端创建产品时 `用户自定义的鉴权信息` (每个产品的每个设备唯一)

**api key** ：配置云端创建设备时获取的 `APIkey`

**product id** ：配置云端创建产品时获取的 `产品ID`

**master/product apikey** ：配置云端创建产品时获取的 `产品APIKey`

配置完成后让 RT-Thread 的包管理器自动更新，或者使用 pkgs --update 命令更新包到 BSP 中。

## 3、使用 OneNET 软件包

- 详细的示例介绍，请参考 [示例文档](docs/samples.md) 。

- 如何从零开始使用，请参考 [用户手册](docs/user-guide.md)。

- 完整的 API 文档，请参考 [API 手册](docs/api.md)。

- OneNET 软件包工作原理，请参考 [工作原理](docs/principle.md) 。

- OneNET 软件包移植，请参考 [移植手册](docs/port.md) 。

- 更多**详细介绍文档**位于 [`/docs`](/docs) 文件夹下，**使用软件包进行开发前请务必查看**。

## 4、注意事项

- 未启用自动注册功能，在 menuconfig 选项中配置的 `device id`、`api key`、`product id`、`auth info` 等信息需要和 OneNET 云端新建产品和新建设备时获取的信息一致。
- 启用自动注册功能后，需要阅读移植手册并完成移植工作。
- 初始化 OneNET package 之前需要设备**联网成功**。

## 5、联系方式 & 感谢

- 维护：RT-Thread 开发团队
- 主页：https://github.com/RT-Thread-packages/onenet
