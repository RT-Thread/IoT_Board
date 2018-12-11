
# API 说明

为了方便用户使用，这里列出了常用的 API，并给出了相关的使用说明。

> 注：更多详细 API 内容请参阅 [**ARM mbedtls API 手册**](https://tls.mbed.org/api/)。

## 应用层 API

应用层 API 是提供给用户在 App 中直接使用的 API，这部分 API 屏蔽了 mbedtls 内部具体的操作步骤，简化了用户使用。

### mbedtls 初始化

> int mbedtls_client_init(MbedTLSSession *session, void *entropy, size_t entropyLen);

mbedtls 客户端初始化函数，用于初始化底层网络接口、设置证书、设置 SSL 会话等。

| 参数           | 描述    |
| :-----         | :-----  |
| session        | 入参，mbedtls 会话对象 MbedTLSSession |
| entropy        | 入参，mbedtls 熵字符串 |
| entropyLen     | 入参，mbedtls 熵字符串长度 |
| **返回**       | **描述**  |
| `= 0`          | 成功 |
| !0             | 失败 |

### 配置 mbedtls 上下文

> int mbedtls_client_context(MbedTLSSession *session);

SSL 层配置，应用程序使用 `mbedtls_client_context` 函数配置客户端上下文信息，包括证书解析、设置主机名、设置默认 SSL 配置、设置认证模式（默认 MBEDTLS_SSL_VERIFY_OPTIONAL）等。

| 参数           | 描述    |
| :-----         | :-----  |
| session        | 入参，mbedtls 会话对象 MbedTLSSession |
| **返回**       | **描述**  |
| `= 0`          | 成功 |
| !0             | 失败 |

### 建立 SSL/TLS 连接

> int mbedtls_client_connect(MbedTLSSession *session);

使用 `mbedtls_client_connect` 函数为 SSL/TLS 连接建立通道。这里包含整个的握手连接过程，以及证书校验结果。

| 参数           | 描述    |
| :-----         | :-----  |
| session        | 入参，mbedtls 会话对象 MbedTLSSession |
| **返回**       | **描述**  |
| `= 0`          | 成功 |
| !0             | 失败 |

### 读取数据

- **向加密连接写入数据**

> int mbedtls_client_write(MbedTLSSession *session, const unsigned char *buf , size_t len);

| 参数           | 描述    |
| :-----         | :-----  |
| session        | 入参，mbedtls 会话对象 MbedTLSSession |
| buf            | 入参，待写入的数据缓冲区 |
| len            | 入参，待写入的数据长度 |
| **返回**       | **描述**  |
| `= 0`          | 成功 |
| !0             | 失败 |

- **从加密连接读取数据**

> int mbedtls_client_read(MbedTLSSession *session, unsigned char *buf , size_t len);

| 参数           | 描述    |
| :-----         | :-----  |
| session        | 入参，mbedtls 会话对象 MbedTLSSession |
| buf            | 入参，mbedtls 读取内容的缓冲区 |
| len            | 入参，mbedtls 待读取内容长度 |
| **返回**       | **描述**  |
| `= 0`          | 成功 |
| !0             | 失败 |

### 关闭 mbedtls 客户端

> int mbedtls_client_close(MbedTLSSession *session);

客户端主动关闭连接或者因为异常错误关闭连接，都需要使用 `mbedtls_client_close` 关闭连接并释放资源。

| 参数           | 描述    |
| :-----         | :-----  |
| session        | 入参，mbedtls 会话对象 MbedTLSSession |
| **返回**       | **描述**  |
| `= 0`          | 成功 |
| !0             | 失败 |

## mbedtls 相关 API

### 设置调试级别

> void mbedtls_debug_set_threshold( int threshold );

如果开启了 `MBEDTLS_DEBUG_C`，可以使用该函数设置调试级别，用于控制不同级别的调试日志输出。

| 参数           | 描述    |
| :-----         | :-----  |
| threshold      | 入参，Debug 级别，默认为 0 没有调试日志 |
| **返回**       | **描述**  |
| 无             | 无 |

mbedtls 定义了 5 种调试级别，如下所示：

| 调试级别        | 描述    |
| :-----         | :-----  |
| 0              | No debug |
| 1              | Error |
| 2              | State change |
| 3              | Informational |
| 4              | Verbose |

### 初始化阶段相关 API

- **网络上下文初始化**

> void mbedtls_net_init( mbedtls_net_context *ctx );

初始化 TLS 网络上下文，目前只有 fd 描述符。

| 参数           | 描述    |
| :-----         | :-----  |
| ctx            | 入参，网络上下文对象 |
| **返回**       | **描述**  |
| 无             | 无 |

- **SSL 上下文初始化**

> void mbedtls_ssl_init( mbedtls_ssl_context *ssl );

SSL 上下文初始化，主要是清空 SSL 上下文对象，为 SSL 连接做准备。

| 参数           | 描述    |
| :-----         | :-----  |
| ssl            | 入参，SSL 上下文对象 |
| **返回**       | **描述**  |
| 无             | 无 |

- **初始化 SSL 配置**

> void mbedtls_ssl_config_init( mbedtls_ssl_config *conf );

SSL 配置初始化，主要是清空 SSL 配置结构体对象，为 SSL 连接做准备。

| 参数           | 描述    |
| :-----         | :-----  |
| conf            | 入参，SSL 配置结构体对象 |
| **返回**       | **描述**  |
| 无             | 无 |

- **初始化 SSL 随机字节发生器**

> void mbedtls_ctr_drbg_init( mbedtls_ctr_drbg_context *ctx );

清空 CTR_DRBG（SSL 随机字节发生器）上下文结构体对象，为 `mbedtls_ctr_drbg_seed` 做准备。

| 参数           | 描述    |
| :-----         | :-----  |
| ctx            | 入参，CTR_DRBG 结构体对象 |
| **返回**       | **描述**  |
| 无             | 无 |

- **初始化 SSL 熵**

> void mbedtls_entropy_init( mbedtls_entropy_context *ctx );

初始化 SSL 熵结构体对象。

| 参数           | 描述    |
| :-----         | :-----  |
| ctx            | 入参，熵结构体对象 |
| **返回**       | **描述**  |
| 无             | 无 |

- **设置 SSL/TLS 熵源**

```c
int mbedtls_ctr_drbg_seed( mbedtls_ctr_drbg_context *ctx,
                   int (*f_entropy)(void *, unsigned char *, size_t),
                   void *p_entropy,
                   const unsigned char *custom,
                   size_t len );
```

为 SSL/TLS 熵设置熵源，方便产生子种子。

| 参数           | 描述    |
| :-----         | :-----  |
| ctx            | 入参，CTR_DRBG 结构体对象 |
| f_entropy      | 入参，熵回调 |
| p_entropy      | 入参，熵结构体（mbedtls_entropy_context）对象 |
| custom         | 入参，个性化数据（设备特定标识符），可以为空 |
| len            | 个性化数据长度 |
| **返回**       | **描述**  |
| 无             | 无 |

- **设置根证书列表**

> void mbedtls_x509_crt_init( mbedtls_x509_crt *crt );

初始化根证书链表。

| 参数           | 描述    |
| :-----         | :-----  |
| crt            | 入参，x509 证书结构体对象 |
| **返回**       | **描述**  |
| 无             | 无 |

- **解析根证书**

> int mbedtls_x509_crt_parse( mbedtls_x509_crt *chain, const unsigned char *buf, size_t buflen );

解释性地解析。解析 buf 中一个或多个证书并将其添加到根证书链接列表中。如果可以解析某些证书，则结果是它遇到的失败证书的数量。 如果没有正确完成，则返回第一个错误。

根证书位于 `ports/src/tls_certificate.c` 文件 `mbedtls_root_certificate` 数组中。

| 参数           | 描述    |
| :-----         | :-----  |
| chain          | 入参，x509 证书结构体对象 |
| buf            | 入参，存储根证书的 buffer，`mbedtls_root_certificate` 数组 |
| buflen         | 入参，存储根证书的 buffer 大小 |
| **返回**       | **描述** |
| 无             | 无 |

- **设置主机名**

> int mbedtls_ssl_set_hostname( mbedtls_ssl_context *ssl, const char *hostname );

注意，这里设置的 `hostname` 必须对应服务器证书中的 `common name`，即 CN 字段。

- **加载默认的 SSL 配置**

```c
int mbedtls_ssl_config_defaults( mbedtls_ssl_config *conf,
                                 int endpoint, int transport, int preset );
```

使用前，需要先调用 `mbedtls_ssl_config_init` 函数初始化 SSL 配置结构体对象。

| 参数           | 描述    |
| :-----         | :-----  |
| conf           | 入参，SSL 配置结构体对象 |
| endpoint       | 入参，MBEDTLS_SSL_IS_CLIENT 或者 MBEDTLS_SSL_IS_SERVER |
| transport      | 入参，TLS: MBEDTLS_SSL_TRANSPORT_STREAM; DTLS: MBEDTLS_SSL_TRANSPORT_DATAGRAM |
| preset         | 入参， 预定义的 MBEDTLS_SSL_PRESET_XXX 类型值，默认使用 MBEDTLS_SSL_PRESET_DEFAULT |
| **返回**       | **描述** |
| 无             | 无 |

- **设置证书验证模式**
> void mbedtls_ssl_conf_authmode( mbedtls_ssl_config *conf, int authmode );

设置证书验证模式默认值：服务器上为 `MBEDTLS_SSL_VERIFY_NONE`，客户端上为 `MBEDTLS_SSL_VERIFY_REQUIRED` 或者 `MBEDTLS_SSL_VERIFY_OPTIONAL`（默认使用）。

`MBEDTLS_SSL_VERIFY_OPTIONAL` 表示证书验证失败也可以继续通讯。

| 参数           | 描述    |
| :-----         | :-----  |
| conf           | 入参，SSL 配置结构体对象 |
| authmode       | 入参，证书验证模式 |
| **返回**       | **描述** |
| 无             | 无 |

- **设置验证对等证书所需的数据**

```c
void mbedtls_ssl_conf_ca_chain( mbedtls_ssl_config *conf,
                                mbedtls_x509_crt *ca_chain,
                                mbedtls_x509_crl *ca_crl );
```

将受信的证书链配置到 SSL 配置结构体对象中。

| 参数           | 描述    |
| :-----         | :-----  |
| conf           | 入参，SSL 配置结构体对象 |
| ca_chain       | 入参，受信的 CA 证书链，存储在 MbedTLSSession 的成员对象 cacert 中 |
| ca_crl         | 入参，受信的 CA CRLs，可为空|
| **返回**       | **描述** |
| 无             | 无 |

- **设置随机数生成器回调**

```c
void mbedtls_ssl_conf_rng( mbedtls_ssl_config *conf,
                           int (*f_rng)(void *, unsigned char *, size_t),
                           void *p_rng );
```

| 参数           | 描述    |
| :-----         | :-----  |
| conf           | 入参，SSL 配置结构体对象 |
| f_rng          | 入参，随机数生成器函数 |
| p_rng          | 入参，随机数生成器函数参数 |
| **返回**       | **描述** |
| 无             | 无 |

- **设置 SSL 上下文**

```c
int mbedtls_ssl_setup( mbedtls_ssl_context *ssl,
                       const mbedtls_ssl_config *conf );
```

将 SSL 配置结构体对象设置到 SSL 上下文中。

| 参数           | 描述    |
| :-----         | :-----  |
| ssl            | 入参，SSL 上下文结构体对象 |
| conf           | 入参，SSL 配置结构体对象 |
| **返回**       | **描述** |
| `= 0`          | 成功 |
| - 0x7F00       | 内存分配失败 |

### 连接阶段相关 API

```c
int mbedtls_net_connect( mbedtls_net_context *ctx,
                         const char *host, const char *port,
                         int proto );
```

与给定的 `host`、`port` 及 `proto` 协议建立网络连接。

| 参数           | 描述    |
| :-----         | :-----  |
| ctx            | 入参，NET 网络配置结构体对象 |
| host           | 入参，指定的待连接主机名 |
| port           | 入参，指定的主机端口号 |
| proto          | 入参，指定的协议类型，MBEDTLS_NET_PROTO_TCP 或者 MBEDTLS_NET_PROTO_UDP |
| **返回**       | **描述** |
| `= 0`          | 成功 |
| - 0x0042       | socket 创建失败 |
| - 0x0052       | 未知的主机名，DNS 解析失败 |
| - 0x0044       | 网络连接失败 |

- **设置网络层读写接口**

```c
void mbedtls_ssl_set_bio( mbedtls_ssl_context *ssl,
                          void *p_bio,
                          mbedtls_ssl_send_t *f_send,
                          mbedtls_ssl_recv_t *f_recv,
                          mbedtls_ssl_recv_timeout_t *f_recv_timeout );
```

为网络层设置读写函数，被 `mbedtls_ssl_read` 和 `mbedtls_ssl_write` 函数调用。

- 对于 TLS，用户提供 f_recv 和 f_recv_timeout 其中之一即可，如果都有提供，默认使用 `f_recv_timeout` 回调
- 对于 DTLS，用户需要提供 `f_recv_timeout` 回调函数

| 参数           | 描述    |
| :-----         | :-----  |
| ssl            | 入参，SSL 上下文结构体对象 |
| p_bio          | 入参，socket 描述符 |
| f_send          | 入参，网络层写回调函数 |
| f_recv          | 入参，网络层读回调函数 |
| f_recv_timeout          | 入参，网络层非阻塞带超时读回调函数 |
| **返回**       | **描述** |
| 无             | 无 |

- **SSL/TLS 握手接口**

> int mbedtls_ssl_handshake( mbedtls_ssl_context *ssl );

执行 SSL/TLS 握手操作。

| 参数           | 描述    |
| :-----         | :-----  |
| ssl            | 入参，SSL 上下文结构体对象 |
| **返回**       | **描述** |
| `= 0`          | 成功 |
| - 0x6900       | SSL 客户端需要读取调用 |
| - 0x6880       | SSL 客户端需要写入调用 |
| - 0x6A80       | DTLS 客户端必须重试才能进行 hello 验证 |
| 其它            | 其它 SSL 指定的错误码 |

注意，如果您使用的是 DTLS，你需要单独处理 `- 0x6A80` 错误，因为它是预期的返回值而不是实际错误。

- **获取证书验证结果**

> uint32_t mbedtls_ssl_get_verify_result( const mbedtls_ssl_context *ssl );

| 参数           | 描述    |
| :-----         | :-----  |
| ssl            | 入参，SSL 上下文结构体对象 |
| **返回**       | **描述** |
| `= 0`          | 成功 |
| - 1            | 返回结果不可用 |
| 其它           | BADCERT_xxx 和 BADCRL_xxx 标志的组合，请参阅 x509.h |

或者证书验证结果的 API 接口，具体的错误信息需要使用 `mbedtls_x509_crt_verify_info` 接口获取。

```c
int mbedtls_x509_crt_verify_info( char *buf, size_t size,
                                  const char *prefix,
                                  uint32_t flags );
```

使用 `mbedtls_x509_crt_verify_info` 函数获取有关证书验证状态的信息字符串，存储在 MbedTLSSession 的对象的 buffer 成员中。

| 参数           | 描述    |
| :-----         | :-----  |
| buf            | 入参，存储验证状态信息字符串的缓冲区 |
| size           | 入参，缓冲区大小 |
| prefix         | 入参，行前缀 |
| flags          | 入参，由 mbedtls_x509_crt_verify_info 函数返回的值 |
| **返回**       | **描述** |
| 整数           | 写入的字符串的长度（不包括结束符）或负的错误代码 |

### 读写 API

**SSL/TLS 写函数**

> int mbedtls_ssl_read( mbedtls_ssl_context *ssl, unsigned char *buf, size_t len );

从 SLL/TLS 读取数据，最多读取 'len' 字节长度数据字节。

| 参数           | 描述    |
| :-----         | :-----  |
| ssl            | 入参，SSL 上下文结构体对象 |
| buf            | 入参，接收读取数据的缓冲区 |
| len            | 入参，要读取的数据长度 |
| **返回**       | **描述** |
| > 0            | 读取到的数据长度 |
| = 0 | 读取到结束符 |
| - 0x6900       | SSL 客户端需要读取调用 |
| - 0x6880       | SSL 客户端需要写入调用 |
| - 0x6780       | SSL 客户端需要重连 |
| 其它           | 其它 SSL 指定的错误码 |

**SSL/TLS 读函数**

> int mbedtls_ssl_write( mbedtls_ssl_context *ssl, const unsigned char *buf, size_t len );

向 SSL/TLS 写入数据，最多写入 'len' 字节长度数据。

| 参数           | 描述    |
| :-----         | :-----  |
| ssl            | 入参，SSL 上下文结构体对象 |
| buf            | 入参，待写入数据的缓冲区 |
| len            | 入参，待写入数据的长度 |
| **返回**       | **描述** |
| > 0            | 实际写入的数据长度 |
| = 0 | 读取到结束符 |
| - 0x6900       | SSL 客户端需要读取调用 |
| - 0x6880       | SSL 客户端需要写入调用 |
| 其它           | 其它 SSL 指定的错误码 |
