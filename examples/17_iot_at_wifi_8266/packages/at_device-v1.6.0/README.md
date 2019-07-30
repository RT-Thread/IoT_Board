# AT device #

## 1. 简介 ##

AT device 软件包是由 RT-Thread AT 组件针对不同 AT 设备的移植文件和示例代码组成，目前支持的 AT 设备有：ESP8266、M26、MC20、EC20、RW007、SIM800C 以及 SIM76XX 系列设备等，目前上述设备都完成对 `AT socket` 功能的移植，及设备通过 AT 命令实现标准 socket 编程接口，完成 socket 通讯的功能，具体功能介绍可参考 [《RT-Thread 编程指南》](https://github.com/RT-Thread/rtthread-manual-doc)AT 命令章节 。 

### 1.1. 文件结构 ###

| 名称 | 说明 |
| ---- | ---- |
| at_socket_esp8266.c  |  ESP8266 模块针对 AT 组件的移植文件，实现 AT socket |
| at_socket_rw007.c  |  RW007 模块针对 AT 组件的移植文件，实现 AT socket |
| at_socket_sim800c.c  |  SIM800C 模块针对 AT 组件的移植文件，实现 AT socket |
| at_socket_sim76xx.c  |  SIM76XX 模块针对 AT 组件的移植文件，实现 AT socket |
| at_socket_m26.c | M26/MC20 模块针对 AT 组件的移植文件，实现 AT socket |
| at_socket_ec20.c | EC20 模块针对 AT 组件的移植文件，实现 AT socket |
| at_client_sample.c | ESP8266 模块 AT Client 功能示例文件 |

### 1.2 许可证 ###

at_device package 遵循 LGPLv2.1 许可，详见 `LICENSE` 文件。

### 1.3 依赖 ###

- RT_Thread 4.0.1+
- RT_Thread AT 组件  1.2.0+
- RT_Thread SAL 组件
- RT-Thread netdev 组件

## 2. 获取方式 ##

AT device 软件包是对 AT 组件库和 AT socket 功能的移植，需开启 AT 组件库和 AT socket 功能来获取 AT device 软件包。

先要开启 AT 组件库和 AT socket 功能， 具体路径如下所示：

    RT-Thread Components  --->
        Network stack  --->
             Socket abstraction layer  --->
                protocol stack implement  --->
                  [ ] Support lwIP stack
                  [*] Support AT Commands stack

开启 AT socket 功能之后，默认开启 AT device 软件包， 具体路径如下所示：

    RT-Thread online packages  --->
         IoT - internet of things  --->
            -*- AT DEVICE: RT-Thread AT component porting or samples for different device  
            [ ]   Enable at device init by thread
                  AT socket device modules (Not selected, please select)  --->    
                  Version (latest)  --->

- `Enable at device init by thread`: 配置开启设备网络初始化是否通过创建线程完成；
- `AT socket device modules`: AT 设备选择，目前支持 RW007、ESP8266、M26/MC20、EC20、SIM800C、SIM76XX  等设备；
- `Version`: 下载软件包版本；
  - **V1.2.0**：RT-Thread 版本小于 V3.1.3，AT 组件版本等于 V1.0.0；
  - **V1.3.0**：RT-Thread 版本小于 V3.1.3，AT 组件版本等于 V1.1.0；
  - **V1.4.0**：RT-Thread 版本小于 V3.1.3 或等于 V4.0.0， AT 组件版本等于 V1.2.0；
  - **V1.5.0**：RT-Thread 版本小于 V3.1.3 或等于 V4.0.0， AT 组件版本等于 V1.2.0；
  - **V1.6.0**：RT-Thread 版本等于 V3.1.3 或等于 V4.0.1， AT 组件版本等于 V1.2.0；
  - **laster**：RT-Thread 版本小于大于 V4.0.1 或者 大于 3.1.3， AT 组件版本大于 V1.2.0；

>上述 版本判断在 menuconfig 中自动完成，at_device 软件包选择版本时会根据当前系统环境给出最佳版本支持，这里版本介绍作为运行环境参考。

## 3. 注意事项  ##

- AT device 软件包适配的模块暂时不支持作为 TCP Server 完成服务器相关操作（如 accept 等）；
- AT device 软件包默认设备类型为未选择，使用时需要指定使用设备型号；
- AT device 软件包目前多个版本主要用于适配 AT 组件和系统的改动，推荐使用最新版本  RT-Thread 系统，并在 menuconfig 选项中选择 `latest` 版本；

## 5. 联系方式

* 维护：RT-Thread 开发团队及社区开发者
* 主页：https://github.com/RT-Thread-packages/at_device
