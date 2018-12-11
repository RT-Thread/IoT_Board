# 示例程序

WebClient 软件包提供两个 HTTP Client 示例程序, 分别用于演示软件包支持的 GET 和 POST 功能，完成数据的上传与下载。

**示例文件**

| 示例程序路径                         | 说明  |
| ----                                | ---- |
| samples/webclient_get_sample.c      | GET 请求测试例程 |
| samples/webclient_post_sample.c     | POST 请求测试例程 |

## 准备工作

### 获取软件包

- menuconfig 配置获取软件包和示例代码

    打开 RT-Thread 提供的 ENV 工具，使用 **menuconfig** 配置软件包。

    启用 WebClient 软件包，并配置使能测试例程（Enable webclient GET/POST samples），如下所示：

```shell
RT-Thread online packages
    IoT - internet of things  --->
	    [*] WebClient: A HTTP/HTTPS Client for RT-Thread    
	    [ ]   Enable support tls protocol       
	    [*]   Enable webclient GET/POST samples # 开启 WebClient 测试例程
	          Version (latest)  --->            # 开启使用最新版本软件包
```

- 使用 `pkgs --update` 命令下载软件包
- 编译下载


## 启动例程 

本例程使用的测试网站是 RT-Thread 系统的官方网站。GET 请求示例可以从网站中获取并打印显示文件内容；POST 请求示例可以上传数据到测试网站，测试网站会响应相同的数据。

> HTTP 收发数据包括头部数据和正文数据两部分，以下称头部数据为 `header 数据`，正文数据为 `body 数据`。

### GET 请求示例

GET 请求示例流程：

- 创建 client 会话结构体
- client 发送 GET 请求 header 数据（使用默认header 数据）
- server 响应 header 数据和 body 数据
- 打印 server 响应 body 数据
- GET 请求测试完成/失败

GET 请求示例使用方式有如下两种：

- 在 MSH 中使用命令 `web_get_test` 执行 GET 请求示例程序，可以获取并打印显示默认网址下载的文件信息，如下图 LOG 显示：

```c
msh />web_get_test
webclient GET request response data :
RT-Thread is an open source IoT operating system from China, which has strong scalability: from a tiny kernel running on a tiny core, for example ARM Cortex-M0, or Cortex-M3/4/7, to a rich feature system running on MIPS32, ARM Cortex-A8, ARM Cortex-A9 DualCore etc.

msh />
```

- 在 MSH 中使用命令 `web_get_test [URI]` 格式命令执行 GET 请求示例程序，其中 URI 为用户自定义的支持 GET 请求的地址。

### POST 请求示例

POST 请求示例流程如下：

- 创建 client 会话结构体
- 拼接 POST 请求需要的 header 数据
- client 发送拼接的 header 数据和 body 数据
- server 响应 header 数据和 body 数据
- 打印 server 响应 body 数据
- POST 请求测试完成/失败

POST 请求示例使用方式有如下两种：

- 在 MSH 中使用命令 `web_post_test` 执行 POST 请求示例程序，可以获取并打印显示响应数据（默认 POST 请求的地址是类似于回显的地址，会返回上传的数据），如下图 LOG 显示：

```c
msh />web_post_test
webclient POST request response data :
RT-Thread is an open source IoT operating system from China!
msh /> 
```
- 在 MSH 中使用命令 `web_post_test [URI]` 格式命令执行 POST 请求示例程序，其中 URI 为用户自定义的支持 POST 请求的地址。