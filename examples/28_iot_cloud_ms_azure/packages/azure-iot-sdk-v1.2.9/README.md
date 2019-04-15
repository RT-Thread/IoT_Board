# Azure：微软物联网云平台 SDK
## 1. 介绍

**Azure** 是 RT-Thread 移植的用于连接微软  Azure IoT 中心的软件包，原始 SDK 为： [**azure-iot-sdk-c**](https://github.com/Azure/azure-iot-sdk-c/tree/5299d0010226fdf2dabb3ac3c1f38eabe4500986)。通过该软件包，可以让运行 RT-Thread 的设备轻松接入 Azure IoT 中心。

Azure IoT 中心的服务托管在云中运行，充当中央消息中心，用于 IoT 应用程序与其管理的设备之间的双向通信。 通过 Azure IoT 中心，可以在数百万 IoT 设备和云托管解决方案后端之间建立可靠又安全的通信，生成 IoT 解决方案。几乎可以将任何设备连接到 IoT 中心。

使用 Azure 软件包连接 IoT 中心可以实现如下功能：

- 轻松连入 Azure IoT 中心，建立与 Azure IoT 的可靠通讯
- 为每个连接的设备设置标识和凭据，并帮助保持云到设备和设备到云消息的保密性
- 管理员可在云端大规模地远程维护、更新和管理 IoT 设备
- 从设备大规模接收遥测数据
- 将数据从设备路由到流事件处理器
- 设备上传文件到 IoT 中心
- 将云到设备的消息发送到特定设备

可以使用 Azure IoT 中心来实现自己的解决方案后端。 此外，IoT 中心还包含标识注册表，可用于预配设备、其安全凭据以及其连接到 IoT 中心的权限。

### 1.1 目录结构

`Azure` 软件包目录结构如下所示：

``` 
azure
├───azure                             // Azure 云平台 SDK
├───azure-port                        // 移植文件
├───docs 
│     └───figures                     // 文档使用图片
│     │   api.md                      // API 使用说明
│     │   introduction.md             // 介绍文档
│     │   principle.md                // 实现原理
│     │   README.md                   // 文档结构说明  
│     │   samples.md                  // 软件包示例
│     │   user-guide.md               // 使用说明
│     └───version.md                  // 版本
├───samples                           // 示例代码
│     │   iothub_ll_telemetry_sample  // 设备上传遥测数据示例
│     └───iothub_ll_c2d_sample        // 设备接收云端数据示例
│   LICENSE                           // 软件包许可证
│   README.md                         // 软件包使用说明
└───SConscript                        // RT-Thread 默认的构建脚本
```

### 1.2 许可证

Azure 软件包遵循 GNU GENERAL PUBLIC LICENSE 许可，详见 `LICENSE` 文件。

### 1.3 资源占用

当统计 Azure IoT SDK 软件包资源占用时，仅统计软件包本身占用的 RAM 和 ROM 大小，未统计 LibC 及外部依赖的软件包所占用的资源大小（如 TLS）。当统计例程运行时资源占用时，统计全部的动态内存占用情况。

- 测试平台： i.mxrt1052
- 测试 IDE： MDK5
- 优化级别：O2
- 软件包资源占用如下所示：
```
    RW(RW + ZI)             :  988   bytes（0.98K）
    RO(CODE + RO)           :  72888 bytes（72.88K）
    ROM(CODE + RO + RW)     :  73876 bytes（73.88K）
    sample Dynamic memory（heap)    ： 68248 bytes（68.24K）
```


### 1.4 依赖

- [RT_Thread 3.0+](https://github.com/RT-Thread/rt-thread/releases/tag/v3.0.4)
- [MbedTLS 软件包](https://github.com/RT-Thread-packages/mbedtls)
- [netutils 软件包](https://github.com/RT-Thread-packages/netutils)

## 2、获取软件包

使用 `azure` 软件包需要在 BSP 目录下使用 menuconfig 命令打开 env 配置界面，在 `RT-Thread online packages → IoT - internet of things`  中选择 azure 软件包，操作界面如下图所示：

```shell
RT-Thread online packages  --->
    IoT - internet of things  --->
        IoT Cloud  --->
          [*] Azure: Microsoft azure cloud SDK for RT-Thread  --->       
              Choose Protocol (Using MQTT Protocol)  --->        #选择例程所使用的通信协议
              [*]   Enable Azure iothub telemetry example        #设备向 IoT 中心发送遥测数据示例
              [*]   Enable Azure iothub cloud to device example  #设备接收云端下发数据示例       
              Version (latest)  --->                             #选择软件包版本
```
选择合适的配置项后，使用 `pkgs --update` 命令下载软件包并添加到工程中即可。

**注意：本软件包移植过程中删除了多余的文件，如需要使用完整功能的SDK，可以下载[原始SDK](https://github.com/Azure/azure-iot-sdk-c/tree/5299d0010226fdf2dabb3ac3c1f38eabe4500986)进行对比添加。**

## 3、使用 Azure 软件包

* 了解软件包提供的功能，请参考 [用户手册](docs/user-guide.md)。
* 完整的 API 文档，请参考 [API 手册](docs/api.md)。
* 详细的示例介绍，请参考 [示例文档](docs/samples.md) 。
* Azure 工作原理，请参考 [工作原理](docs/principle.md) 。
* 更多**详细介绍文档**位于 [`/docs`](/docs) 文件夹下，**使用软件包进行开发前请务必查看**。

## 4、注意事项

* azure  软件包编译时请使用较为完整的 libc 库，请不要勾选 MDK 中的 `Use Microlib` 选项

## 5、联系方式 & 感谢

* 维护：RT-Thread 开发团队
* 主页：https://github.com/RT-Thread-packages/
