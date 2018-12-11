# 示例程序

该示例程序提供了一个简单的 TLS client，与测试网站建立 TLS 连接并获取加密数据。

**示例文件**

| 示例程序路径                   | 说明 |
| ----                          | ---- |
| samples/tls_app_test.c        | TLS 测试例程 |

## 例程工作流程

本例程使用了 RT-Thread 官方 TLS 测试网站 `www.rt-thread.org`，使用 `mbedtls_client_write` 函数发送 HTTP 测试请求，成功后，该网站会返回文本数据，测试例程将解析后的数据输出到控制台。

- 例程使用的 HTTP 请求数据如下所示

```c
"GET /download/rt-thread.txt HTTP/1.0\r\n"
"Host: www.rt-thread.org\r\n"
"User-Agent: rtthread/3.1 rtt\r\n"
"\r\n";
```

- mbedTLS 测试例程的基本工作流程如下所示

    - client连接测试网站 `www.rt-thread.org`
    - client 和 server 握手成功
    - client 发送请求
    - server 回应请求
    - TLS 测试成功/失败

## 准备工作

### 获取软件包

- menuconfig 配置软件包

    打开 RT-Thread 提供的 ENV 工具，使用 **menuconfig** 配置软件包。

    启用 mbedtls 软件包，并配置使能测试例程（Enable a mbedtls client example），如下所示：

```shell
RT-Thread online packages --->
    security packages  --->
            Select Root Certificate  --->      # 选择证书文件
        [*] mbedtls: An portable and flexible SSL/TLS library  ---  # 打开 mbedtls 软件包
        [*]   Store the AES tables in ROM      # 将 AES 表存储在 ROM 中，优化内存占用
        (2)   Maximum window size used         # 用于点乘的最大“窗口”大小（2-7，该值越小内存占用也越小）
        (3584) Maxium fragment length in bytes # 配置数据帧大小（0x7200 错误可尝试增加该大小）
        [*]   Enable a mbedtls client example  # 开启 mbedtls 测试例程
        [ ]   Enable Debug log output          # 开启调试 log 输出
              version (latest)  --->           # 选择软件包版本，默认为最新版本
```

- 使用 `pkgs --update` 命令下载软件包
- 编译下载

### 同步设备时间

SSL/TLS 服务器进行证书校验的过程中，会对当前发起校验请求的时间进行认证，如果时间不满足服务器的要求，就会校验证书失败。因此，我们需要为设备同步本地时间。

- 方式一： 使用 **`date`** 命令

    未同步过时间的设备输入 **`date`** 命令后如下所示：

    ```shell
    msh />date
    Thu Jan  1 00:00:06 1970
    ```

    使用 **`date`** 设置当前时间，如下所示：

    ```shell
    msh />date 2018 08 02 12 23 00
    msh />date
    Thu Aug  2 12:23:01 2018
    msh />
    ```

- 方式二： 使用 NTP 同步网络时间

    该方式需要依赖 NTP 工具包，使用 `menuconfig` 配置获取，如下所示：

    ```shell
    RT-Thread online packages --->
        IoT - internet of things  --->
            -*- netutils: Networking utilities for RT-Thread  --->
                -*-   Enable NTP(Network Time Protocol) client
                (8)     Timezone for calculate local time
                (cn.ntp.org.cn) NTP server name
    ```

    使用命令 **`ntp_sync`** 同步网络时间

    ```shell
    msh />ntp_sync
    Get local time from NTP server: Thu Aug  2 14:31:30 2018
    The system time is updated. Timezone is 8.
    msh />date
    Thu Aug  2 14:31:34 2018
    ```

## 启动例程

在 MSH 中使用命令 **`tls_test`** 执行示例程序，成功建立 TLS 连接后，设备会从服务器拿到一组密码套件，设备 log 如下所示：

```shell
msh />tls_test
MbedTLS test sample!
Memory usage before the handshake connection is established:
total memory: 33554408
used memory : 20968
maximum allocated memory: 20968
Start handshake tick:3313
[tls]mbedtls client struct init success...
[tls]Loading the CA root certificate success...
[tls]mbedtls client context init success...
msh />[tls]Connected www.rt-thread.org:443 success...
[tls]Certificate verified success...
Finish handshake tick:6592
MbedTLS connect success...
Memory usage after the handshake connection is established:
total memory: 33554408
used memory : 45480
maximum allocated memory: 50808
Writing HTTP request success...
Getting HTTP response...
HTTP/1.1 200 OK
Server: nginx/1.10.3 (Ubuntu)
Date: Fri, 31 Aug 2018 08:29:24 GMT
Content-Type: text/plain
Content-Length: 267
Last-Modified: Sat, 04 Aug 2018 02:14:51 GMT
Connection: keep-alive
ETag: "5b650c1b-10b"
Strict-Transport-Security: max-age=1800; includeSubdomains; preload
Accept-Ranges: bytes

RT-Thread is an open source IoT operating system from China, which has strong scalability: from a tiny kernel running on a tiny core, for example ARM Cortex-M0, or Cortex-M3/4/7, to a rich feature system running on MIPS32, ARM Cortex-A8, ARM Cortex-A9 DualCore etc.

MbedTLS connection close success.
```
