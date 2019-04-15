# HTTP Client 功能实现例程

## 简介

本例程介绍如何使用 WebClient 软件包发送 HTTP 协议 GET 和 POST 请求 ，并且接收响应的数据。

### HTTP 协议

HTTP （Hypertext Transfer Protocol）协议， 即超文本传输协议，是互联网上应用最为广泛的一种网络协议，由于其简捷、快速的使用方式，适用于分布式和合作式超媒体信息系统。HTTP 协议是基于 TCP/IP 协议的网络应用层协议。默认端口为 80 端口。协议最新版本是 HTTP 2.0，目前是用最广泛的是 HTTP 1.1。

HTTP 协议是一种请求/响应式的协议。一个客户端与服务器建立连接之后，发送一个请求给服务器。服务器接收到请求之后，通过接收到的信息判断响应方式，并且给予客户端相应的响应，完成整个 HTTP 数据交互流程。

### WebClient 软件包

WebClient 软件包是 RT-Thread 自主研发的，基于 HTTP 协议的客户端实现，它提供设备与 HTTP 服务器的通讯的基本功能。

WebClient 软件包功能特点：

- **支持 IPV4/IPV6 地址**

  WebClient 软件包会自动根据传入的 URI 地址的格式判断是 IPV4 地址或 IPV6 地址，并且从中解析出连接服务器需要的信息，提高代码兼容性。

- **支持 GET/POST 请求方法**

  目前 WebClient 软件包支持 HTTP 协议 GET 和 POST 请求方法，这也是嵌入式设备最常用到的两个命令类型，满足设备开发需求。

- **支持文件的上传和下载功能**

  WebClient 软件包提供文件上传和下载的接口函数，方便用户直接通过 GET/POST 请求方法上传本地文件到服务器或者下载服务器文件到本地。

- **支持 HTTPS 加密传输**

  WebClient 软件包可以采用 TLS 加密方式传输数据，保证数据的安全性和完整性 。

- **完善的头部数据添加和处理方式**

  WebClient 软件包中提供简单的添加发送请求头部信息的方式，方便用于快速准确的拼接头部信息。

## 硬件说明

本例程需要依赖 IoTBoard 板卡上的 WiFi 模块完成网络通信，因此请确保硬件平台上的 WiFi 模组可以正常工作。

## 软件说明

**HTTP Client 例程**位于 `/examples/20_iot_http_client` 目录下，重要文件摘要说明如下所示：

| 文件                          | 说明                           |
| :---------------------------- | :----------------------------  |
| applications/main.c           | app 入口（WebClient 例程程序） |
| packages/webclient-v2.0.1     | webclient 软件包               |
| packages/webclient-v2.0.1/inc | webclient 软件包头文件         |
| packages/webclient-v2.0.1/src | webclient 软件包源码文件       |

### 例程使用说明

本例程主要实现设备通过 GET 请求方式从指定服务器中获取数据，之后通过 POST 请求方式上传一段数据到指定服务器，并接收服务器响应数据。例程的源代码位于 `/examples/19_iot_http_client/applications/main.c` 中。

其中 main 函数主要完成 wlan 网络初始化配置，并等待设备联网成功，程序如下所示：


```c
int main(void)
{
    int result = RT_EOK;

    /* 初始化 wlan 自动连接功能 */
    wlan_autoconnect_init();

    /* 使能 wlan 自动连接功能 */
    rt_wlan_config_autoreconnect(RT_TRUE);

    /* 创建 'net_ready' 信号量 */
    result = rt_sem_init(&net_ready, "net_ready", 0, RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        return -RT_ERROR;
    }

    /* 注册 wlan 连接网络成功的回调，wlan 连接网络成功后释放 'net_ready' 信号量 */
    rt_wlan_register_event_handler(RT_WLAN_EVT_READY, wlan_ready_handler, RT_NULL);
    /* 注册 wlan 网络断开连接的回调 */
    rt_wlan_register_event_handler(RT_WLAN_EVT_STA_DISCONNECTED, wlan_station_disconnect_handler, RT_NULL);

    /* 等待 wlan 连接网络成功 */
    result = rt_sem_take(&net_ready, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        LOG_E("Wait net ready failed!");
        rt_sem_delete(&net_ready);
        return -RT_ERROR;
    }

    /* HTTP GET 请求发送 */
    webclient_get_data();
    /* HTTP POST 请求发送 */
    webclient_post_data();
}
```

设备成功接入网络之后，会自动顺序的执行 `webclient_get_test()` 和 `webclient_post_test()` 函数，通过 HTTP 协议发送 GET 和 POST 请求。

**1. GET 请求发送**

本例程中使用 GET 方式请求 HTTP 服务器 `www.rt-thread.com` 中的 `rt-thread.txt` 文本文件，文件在服务器中的地址为 `/server/rt-thread.txt`，使用端口为默认 HTTP 端口 80。

例程中调用封装的 `webclient_get_data()` 函数发送 GET 请求，该函数内部直接使用 `webclient_request()` 完成整个 GET 请求发送头部信息和获取响应数据的流程，这里客户端发送默认 GET 请求头部信息，响应的数据存储到 buffer 缓冲区并打印到 FinSH 控制台。

`webclient_request()` 函数一般用于响应数据较短的情况，因为该函数会将响应数据**一次性**全部读取并存储到缓冲区，更多 WebClient 软件包 GET 请求使用方式可查看 `/examples/19_iot_http_client/applications/packages/webclient-v2.0.1/samples `目录下 GET 请求例程文件。

```c
#define HTTP_GET_URL          "http://www.rt-thread.com/service/rt-thread.txt"

int webclient_get_data(void)
{
    unsigned char *buffer = RT_NULL;
    int length = 0;
    
    /* 发送 GET 请求，使用默认头部信息，buffer 读取接收的数据 */
    length = webclient_request(HTTP_GET_URL, RT_NULL, RT_NULL, &buffer);
    if (length < 0)
    {
        LOG_E("webclient GET request response data error.");
        return -RT_ERROR;
    }

    LOG_D("webclient GET request response data :");
    LOG_D("%s", buffer);
    
    web_free(buffer);  
    return RT_EOK;
}
```

 **2. POST 请求发送**

本例程中使用 POST 方式请求发送一段数据到 HTTP 服务器 `www.rt-thread.com` ，HTTP 服务器响应并下发同样的数据到设备上。

例程中调用封装的 `webclient_post_data()` 函数发送POST 请求，该函数内部直接使用 `webclient_request()`  完成整个 POST 请求发送头部信息和获取响应数据的流程，这里客户端发送默认 POST 请求头部信息，响应的数据存储到 buffer 缓冲区并打印到 FinSH 控制台。更多 WebClient 软件包 POST 请求使用方式可查看 `/examples/19_iot_http_client/applications/packages/webclient-v2.0.1/samples `目录下 POST 请求例程文件。

```c
#define HTTP_POST_URL         "http://www.rt-thread.com/service/echo"
/* POST 请求发送到服务器的数据 */
const char *post_data = "RT-Thread is an open source IoT operating system from China!";

int webclient_post_data(void)
{
    unsigned char *buffer = RT_NULL;
    int length = 0;
    
    /* 发送 POST 请求，使用默认头部信息，buffer 读取响应的数据 */
    length = webclient_request(HTTP_POST_URL, RT_NULL, post_data, &buffer);
    if (length < 0)
    {
        LOG_E("webclient POST request response data error.");
        return -RT_ERROR;
    }

    LOG_D("webclient POST request response data :");
    LOG_D("%s", buffer);
    
    web_free(buffer);  
    return RT_EOK;
}
```

## 运行

### 编译&下载

- **MDK**：双击 `project.uvprojx` 打开 MDK5 工程，执行编译。
- **IAR**：双击 `project.eww` 打开 IAR 工程，执行编译。

编译完成后，将开发板的 ST-Link USB 口与 PC 机连接，然后将固件下载至开发板。

按下复位按键重启开发板，程序运行日志如下所示：

```shell
 \ | /
- RT -     Thread Operating System
 / | \     4.0.1 build Mar 27 2019
 2006 - 2019 Copyright by rt-thread team
lwIP-2.0.2 initialized!
[I/SAL_SKT] Socket Abstraction Layer initialize success.
[SFUD] Find a Winbond flash chip. Size is 16777216 bytes.
[SFUD] w25q128 flash device is initialize success.
msh />[I/FAL] RT-Thread Flash Abstraction Layer (V0.2.0) initialize success.
[I/OTA] RT-Thread OTA package(V0.1.3) initialize success.
[I/OTA] Verify 'wifi_image' partition(fw ver: 1.0, timestamp: 1529386280) success.
[I/WICED] wifi initialize done. wiced version 3.3.1
[I/WLAN.dev] wlan init success
[I/WLAN.lwip] eth device init ok name:w0
[Flash] EasyFlash V3.2.1 is initialize success.
[Flash] You can get the latest version on https://github.com/armink/EasyFlash .
```

### 连接无线网络

程序运行后会进行 MSH 命令行，等待用户配置设备接入网络。使用 MSH 命令 `wifi join <ssid> <password>` 配置网络（ssid 和 password 分别为设备连接的 WIFI 用户名和密码），如下所示：

```shell
msh />wifi join test 12345678
join ssid:test
[I/WLAN.mgnt] wifi connect success ssid:test
.......
msh />[I/WLAN.lwip] Got IP address : 192.168.12.155    
```

### 发送 GET 和 POST 请求

WiFi 连接成功后，先运行 HTTP GET 请求，获取并打印 GET 请求接收的数据，接着运行 HTTP POST 请求，上传指定数据到服务器并获取服务器响应数据。如下所示：

```shell
[D/main] webclient GET request response data :               # GET 请求接收数据
[D/main] RT-Thread is an open source IoT operating system from China, which has strong scalability: from a tiny kernel running on a tiny core, for example ARM Cortex-M0, or Cortex-M3/4/7, to a rich feature system running on MIPS32, ARM Cortex-A8, ARM Cortex-A9 DualCore etc.

[D/main] webclient POST request response data :              # POST 请求响应数据
[D/main] RT-Thread is an open source IoT operating system from China!
```

## 注意事项

使用本例程前需要先连接 WiFi。

## 引用参考

- 《WebClient 软件包用户手册》: docs/UM1001-RT-Thread-WebClient 用户手册.pdf
- 《RT-Thread 编程指南》: docs/RT-Thread 编程指南.pdf