# MbedTLS RAM 和 ROM 资源占用优化指南

**mbedtls** 软件包采用了模块化的设计，可以使用 `config.h` 文件来进行功能模块的配置选择。

`mbedtls` 默认提供的 `config.h` 文件是一个通用的、全功能的配置，占用了非常大的 RAM 和 ROM 空间，但是保证了 SSL 握手和通讯的建立速度、稳定性、协议兼容性以及数据传输效率。但嵌入式设备受限于其有限的 RAM 和 ROM 空间，我们不得不牺牲速度来节省 RAM 空间，裁剪不需要的功能模块来降低 ROM 占用。

本优化指南，在保证 SSL/TLS 客户端能与服务器建立安全稳定连接的前提下，对 RAM 和 ROM 占用进行优化统计。

**注意：**

**mbedtls** 客户端的优化属于针对性优化，针对特定的 SSL/TLS 服务器进行的优化，不同的 SSL/TLS 服务器配置不同，优化所用到的配置参数也是不同的。

因此，开发者在进行 SSL/TLS 优化前，在 MCU 资源条件允许的情况下，请先使用默认的配置调通 SSL/TLS 握手连接和加密通讯，然后再根据 SSL/TLS 服务器具体的配置进行逐项优化。

当然，多数情况下您并不知道服务器的具体参数配置，因此也只能试探性优化，本文给出了各个配置的说明，来方便开发者进行针对性的优化。

## 优化说明

- RAM 资源占用统计说明

    首先保证 SSL 握手连接正常，加密数据通讯正常。
    运行 `tls_test` 测试例程，进行 RAM 优化测试。测试例程在单独的线程中运行，通过对比 SSL 握手成功前后所占用的内存来确定在握手通讯过程所使用的 RAM 情况。该测试方法只能粗略估计 SSL 客户端成功进行握手连接所需要的 RAM 大小，该数据包含了保证握手通讯所需要的额外的 RAM 空间。

- ROM 资源占用统计说明

    通过对比启动 **mbedtls** 功能组件前后参与链接的文件来统计 **mbedtls** 所占用 ROM 大小。

- 测试平台： iMXRT1052
- 测试 IDE： MDK5
- 优化级别： o2
- 测试例程： `samples/tls_app_test.c`
- 测试使用的 SSL 服务器： `www.rt-thread.org`
- 测试服务器根证书签名算法： `sha1RSA`
- 测试服务器根证书签名哈希算法： `sha1`
- 测试服务器根证书公钥： `RSA 2048 bits`
- 测试服务器根证书指纹算法： `sha1`
- SSL 客户端指定密码套件

```c
#define MBEDTLS_SSL_CIPHERSUITES                        \
    MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA256
```

- SSL 客户端指定帧大小为 `#define MBEDTLS_SSL_MAX_CONTENT_LEN 3584`
- 测试使用的配置（**详见文末**）

## 优化后的资源占用汇总

- 默认的 `tls_config.h` 配置资源占用情况

    **mbedtls** 默认的配置文件为 `mbedtls/include/mbedtls/config.h`，而 **RT-Thread** 使用的配置文件为 `ports/inc/tls_config.h`。用户进行配置优化的时候也是使用的 `ports/inc/tls_config.h` 文件。

```text
RO(CODE + RO)       : 159828 bytes（156.08K）
RW(RW + ZI)         :    720 bytes
ROM(CODE + RO + RW) : 159972 bytes（156.22K）
动态内存使用         :  26849 bytes（26.22K）（包含 1K 的测试 buffer）
```

- 优化后的配置资源占用情况

```text
RO(CODE + RO)       : 71893 bytes（70.21K）
RW(RW + ZI)         :    82 bytes
ROM(CODE + RO + RW) : 71975 bytes（70.29K）
动态内存使用         : 23344 bytes（22.79K）（包含 1K 的测试 buffer）
```

## 优化前的准备

1. 首先你要有准备接入的 SSL 服务器（保证能正常工作）
2. 准备好接入 SSL 服务器的 PEM 格式根证书文件（存放到 mbedtls 软件包 `certs` 目录下，删除其他不需要的证书）
3. 使用默认的 mbedtls 配置文件成功接通 SSL 服务器（优化后更难定位失败原因）
4. 逐项优化 mbedtls 配置，反复测试

**注意：**

如果您的 MCU 资源比较小，无法使用默认的 `tls_config.h` 配置文件，开发者可以选择使用 `QEMU` 虚拟机进行开发调试以及 mbedtls 优化。将 mbedtls 资源占用优化到合适的时候，再使用您需要的 MCU 进行验证测试。

## 优化配置概述

### 常用优化配置

通过对下面列表中的配置进行修改，可以很大程度上降低 mbedtls RAM 和 ROM 的占用。

开发者在进行优化时，建议优先对下面列表中的配置进行优化，如果不能满足要求，再针对其他的配置进行逐项的优化。

| 配置  |依赖    | 说明 | 优化建议 |
| :---- | :---- | :---- | :---- |
| const char mbedtls_root_certificate[] | 无 | 存储根证书的常量数组。编译的时候，会将 PEM 证书添加到该数组。建议只在 `certs` 证书目录存放需要的根证书文件，否则会占用非常大的 RAM 和 ROM 空间 | 只存放需要的证书文件 |
| MBEDTLS_SSL_CIPHERSUITES | 无 | 通过指定密码套件来节省几百字节的 ROM 和 几百字节的 RAM。这里注意需要指定服务器支持的加密套件，并为该加密套件启用相关的功能组件，关闭其他功能组件。如果只接入一个 SSL 服务器，通常这里只需要定义支持一个加密套件即可 | 仅指定根证书需要的加密套件 |
| MBEDTLS_AES_ROM_TABLES | 无 | 将 AES 表存储在 ROM 中以节省 RAM 占用（很大程度上降低 RAM 占用） | 建议启用 |
| MBEDTLS_SSL_MAX_CONTENT_LEN | 无 | 默认为 16384。RFC定义了 SSL/TLS 消息的默认大小，如果您在此处更改值，则其他客户端/服务器可能无法再与您通信。除非你能确定服务端的帧大小。根据服务器发送的最大帧大小做适当的修改 | 适当调小（出现 0x7200 错误时请增大该配置） |
| MBEDTLS_MPI_MAX_SIZE | 无 | 可用的 MPI 最大字节数，默认为 1024，可以根据适当调小 | 适当调小 |
| MBEDTLS_MPI_WINDOW_SIZE | 无 | MPI 用于模幂运算的最大窗口数量，默认为6，选值范围：1-6，可适当调小 | 适当调小 |
| MBEDTLS_ECP_MAX_BITS | 无 | GF(p) 椭圆曲线最大位，默认为 521 | 适当调小 |
| MBEDTLS_ECP_WINDOW_SIZE | 无 | 用于点乘的最大窗口大小，默认为 6，选值范围： 2-7，可以适当减小，减小会影响速度 | 适当调小 |
| MBEDTLS_ECP_FIXED_POINT_OPTIM | 无 | 默认1，启用定点加速。启用后，将加速点乘运算大约 3 到 4 倍，成本是峰值内存占用增加约 2 倍。可以配置为 0，牺牲速度来节省 RAM 占用 | 可优化配置为 0 |
| MBEDTLS_ECP_NIST_OPTIM | 无 | 为每个 NIST 启用特定的实例，使相应曲线上的操作快 4 到 8 倍，缺点是 ROM 占用大。可以选择性优化 | 可禁用 |
| MBEDTLS_ENTROPY_MAX_SOURCES | 无 | 最大的熵源数量，最小为2，默认使用 `mbedtls_platform_entropy_poll` 源。RT-Thread 上使用最小配置 2 | 可优化配置为 2 |

### 系统相关配置

这部分配置跟具体的系统和编译器相关，下表列出了在 **RT-Thread** 上需要做的配置。

| 配置  |依赖   | 说明 | 优化建议 |
| :---- | :---- | :---- | :---- |
| MBEDTLS_HAVE_ASM | 无 | 需要编译器可以处理汇编代码 | 启用 |
| MBEDTLS_HAVE_TIME | 无 | 如果您的系统没有 time.h 和 time() 函数，请注释该配置 | 启用 |
| MBEDTLS_HAVE_TIME_DATE | 无 | 如果您的系统没有 time.h、time()、gmtime() 或者没有正确的时钟，请注释该配置 | 启用 |
| MBEDTLS_DEBUG_C | 无 | 定义该配置以启动调试 log 输出 | 如需调试log，则启用，否则请禁用 |
| MBEDTLS_NET_C | 无 | 该配置仅支持 POSIX/Unix 和 windows 系统，在 RT-Thread 系统上需要关闭 | 禁用 |
| MBEDTLS_NO_PLATFORM_ENTROPY | 无 | 如果您的平台不支持 /dev/urandom 或 Windows CryptoAPI 等标准，则需要启用该配置。RT-Thread 上必须启用 | 启用 |
| MBEDTLS_TIMING_C | 无 | 如果注释，则需要用户自己实现相关的函数。默认启用 | 启用 |
| MBEDTLS_TIMING_ALT | 无 | 如果注释，则需要用户自己实现相关的函数。默认启用 | 启用 |
| MBEDTLS_ENTROPY_HARDWARE_ALT | 无 | 如果注释，则需要用户自己实现相关的函数。默认启用 | 启用 |
| MBEDTLS_ENTROPY_C | MBEDTLS_SHA256_C 或 MBEDTLS_SHA512_C | 启用特定于平台的熵代码。需要启用 | 启用 |
| MBEDTLS_PADLOCK_C | MBEDTLS_HAVE_ASM | 在x86上启用VIA Padlock支持 | 禁用 |
| MBEDTLS_AESNI_C | MBEDTLS_HAVE_ASM | 在x86-64上启用AES-NI支持 | 禁用 |
| MBEDTLS_PLATFORM_C | 无 | 使能平台抽象层，用于重定义实现 free、printf 等函数 | 启用 |

### 功能组件相关配置

用户可以根据要接入的 SSL/TLS 服务器特性以及根证书使用的签名算法来选择启用哪部分的功能。启用或禁用功能组件时请注意将相关的依赖打开或禁用。

| 配置  |依赖   | 说明 | 优化建议 |
| :---- | :---- |  :---- | :---- |
| MBEDTLS_ASN1_PARSE_C | 无  | 使能通用的 ASN1 解析器。ASN1: 一种描述数字对象的方法和标准，需要启用 | 启用 |
| MBEDTLS_ASN1_WRITE_C | 无  | 启用通用 ASN1 编写器 | 启用 |
| MBEDTLS_BIGNUM_C | 无 | 启用大整数库 （multi-precision integer library） | 启用 |
| MBEDTLS_CIPHER_C | 无 | 启用通用密码层 | 启用 |
| MBEDTLS_AES_C | 无 | 启用 AES 加密。PEM_PARSE 使用 AES 来解密被加密的密钥。通过启用 AES 来支持 `*_WITH_AES_*` 类型的加密套件 | 启用 |
| MBEDTLS_CTR_DRBG_C | MBEDTLS_AES_C | 启用基于 CTR_DRBG AES-256 的随机生成器 | 启用 |
| MBEDTLS_MD_C | 无 | 启用通用消息摘要层，需要启用 | 启用 |
| MBEDTLS_OID_C | 无 | 启用OID数据库，此模块在OID和内部值之间进行转换，需要启用 | 启用 |
| MBEDTLS_PK_C | MBEDTLS_RSA_C、MBEDTLS_ECP_C | 启用通用公共（非对称）密钥层，需要启用 | 启用 |
| MBEDTLS_PK_PARSE_C | MBEDTLS_PK_C | 启用通用公共（非对称）密钥解析器，需要启用 | 启用 |
| MBEDTLS_SHA256_C | 无 | 启用 SHA-224 和 SHA-256 加密哈希算法，根据根证书详细信息中的`签名哈希算法`进行选择 | 根据需要选择 |
| MBEDTLS_SHA512_C | 无 | 启用 SHA-384 和 SHA-512 加密哈希算法，根据根证书详细信息中的`签名哈希算法`进行选择 | 根据需要选择 |
| MBEDTLS_SSL_CLI_C | MBEDTLS_SSL_TLS_C | 启用 SSL 客户端代码，作为 SSL 服务端的时候不需要启用 | 启用 |
| MBEDTLS_SSL_SRV_C | MBEDTLS_SSL_TLS_C | 启用 SSL 服务端代码，作为 SSL 客户端的时候不需要启用 | 禁用 | 
| MBEDTLS_SSL_TLS_C | MBEDTLS_CIPHER_C、MBEDTLS_MD_C 和至少定义一个 MBEDTLS_SSL_PROTO_XXX | 使能 SSL/TLS 代码 | 启用 |
| MBEDTLS_X509_CRT_PARSE_C | MBEDTLS_X509_USE_C | 使能 X509 证书解析 | 启用 |
| MBEDTLS_X509_USE_C | MBEDTLS_ASN1_PARSE_C、MBEDTLS_BIGNUM_C、MBEDTLS_OID_C、MBEDTLS_PK_PARSE_C | 启用X.509核心以使用证书 | 启用 |
| MBEDTLS_BASE64_C | 无 | 启用 base64 组件，PEM 证书解析需要使用 | 启用 |
| MBEDTLS_CERTS_C | 无 | 该模块用于测试 SSL 客户端和服务器，可以选择禁用 | 可禁用 |
| MBEDTLS_PEM_PARSE_C | MBEDTLS_BASE64_C | 启用对 PEM 文件解码解析的支持 | 启用 |
| MBEDTLS_RSA_C | MBEDTLS_BIGNUM_C、MBEDTLS_OID_C | 启用RSA公钥密码系统。RSA、DHE-RSA、ECDHE-RSA、RSA-PSK 方式的密钥交换需要使用 | 启用 |
| MBEDTLS_SHA1_C | 无 | 启用 SHA1 加密哈希算法。TLS 1.1/1.2 需要使用 | 启用 |
| MBEDTLS_MD5_C | 无 | 启用MD5哈希算法。PEM 解析需要使用 | 启用 |
| MBEDTLS_PK_PARSE_EC_EXTENDED | 无 | 该宏用以支持 RFC 5915 和RFC 5480 不允许的 SEC1 变体增强对读取 EC 密钥的支持 | 可以禁用 |
| MBEDTLS_ERROR_STRERROR_DUMMY | 无 | 启用虚拟错误功能，以便在禁用 MBEDTLS_ERROR_C 时更容易在第三方库中使用 mbedtls_strerror（）（启用 MBEDTLS_ERROR_C 时无效） | 可以禁用 |
| MBEDTLS_GENPRIME | MBEDTLS_BIGNUM_C | 启用素数生成代码 | 可以禁用 |
| MBEDTLS_FS_IO | 无 | 启用文件系统交互相关的功能函数 | 可以禁用 |
| MBEDTLS_PKCS5_C | MBEDTLS_MD_C | 该模块增加了对 PKCS＃5 功能的支持。AES 算法数据填充方案的需要。根据需要选择是否禁用 | 根据需要选择 |
| MBEDTLS_PKCS12_C | MBEDTLS_ASN1_PARSE_C、MBEDTLS_CIPHER_C、MBEDTLS_MD_C | 添加用于解析 PKCS＃8 加密私钥的算法 | 可以禁用 |
| MBEDTLS_PKCS1_V15 | MBEDTLS_RSA_C | 用于支持 PKCS＃1 v1.5 操作，RSA 密钥套件需要使用。如果使用了 RSA 密钥套件，则需要启用 | 根据需要选择 |
| MBEDTLS_PKCS1_V21 | MBEDTLS_MD_C、MBEDTLS_RSA_C | 启用对 PKCS＃1 v2.1 编码的支持，这样可以支持 RSAES-OAEP 和 RSASSA-PSS 操作 | 可以禁用 |
| MBEDTLS_PK_RSA_ALT_SUPPORT | 无 | 支持 PK 层中的外部私有 RSA 密钥（例如，来自 HSM）。不需要启用，禁用 | 禁用 |
| MBEDTLS_SELF_TEST | 无 | 启用检查功能。建议在启用 debug 的时候启用，其他时候禁用 | 可以禁用 |
| MBEDTLS_SSL_ALL_ALERT_MESSAGES | 无 | 启用警报消息发送功能 | 可以禁用 |
| MBEDTLS_SSL_ENCRYPT_THEN_MAC | 无 | 启用对Encrypt-then-MAC，RFC 7366的支持。用于加强对 CBC 密码套件的保护，可以禁用 | 可以禁用 |
| MBEDTLS_SSL_EXTENDED_MASTER_SECRET | 无 | 启用对扩展主密钥的支持 | 可以禁用 |
| MBEDTLS_SSL_FALLBACK_SCSV | 无 | 注释此宏以禁用客户端使用回退策略 | 可以禁用 |
| MBEDTLS_SSL_CBC_RECORD_SPLITTING | 无 | 在 SSLv3 和 TLS 1.0 中为 CBC 模式启用 1/n-1 记录拆分。启用该宏以降低 BEAST 攻击的风险，可以选择性禁用 | 可以禁用 |
| MBEDTLS_SSL_RENEGOTIATION | 无 | 屏蔽该宏禁用对TLS重新协商的支持。启用可能会带来安全风险，建议禁用 | 禁用 |
| MBEDTLS_SSL_MAX_FRAGMENT_LENGTH | 无 | 在 SSL 中启用对 RFC 6066 最大帧长度扩展的支持 | 可以禁用 |
| MBEDTLS_SSL_ALPN | 无 | 启用对 RFC 7301 应用层协议协商的支持 | 可以禁用 |
| MBEDTLS_SSL_SESSION_TICKETS | 无 | 在SSL中启用对RFC 5077会话 tickets 的支持，需要服务端支持。通常用来优化握手流程 | 可以禁用 |
| MBEDTLS_SSL_EXPORT_KEYS | 无 | 启用对导出密钥块和主密钥的支持。 这对于 TLS 的某些用户是必需的，例如 EAP-TLS | 可以禁用 |
| MBEDTLS_SSL_SERVER_NAME_INDICATION | MBEDTLS_X509_CRT_PARSE_C | 在 SSL 中启用对 RFC 6066 服务器名称指示（SNI）的支持 | 可以禁用 |
| MBEDTLS_SSL_TRUNCATED_HMAC | 无 | 在 SSL 中启用对 RFC 6066 截断 HMAC 的支持 | 可以禁用 |
| MBEDTLS_VERSION_FEATURES | MBEDTLS_VERSION_C | 版本功能信息相关 | 可以禁用 |
| MBEDTLS_VERSION_C | 无 | 该模块提供运行时版本信息 | 可以禁用 |
| MBEDTLS_X509_CHECK_KEY_USAGE | 无 | 启用 keyUsage 扩展（CA和叶证书）的验证。禁用此功能可避免错误发布和/或误用（中间）CA 和叶证书的问题。注释后跳过 keyUsage 检查 CA 和叶证书 | 可以禁用 |
| MBEDTLS_X509_CHECK_EXTENDED_KEY_USAGE | 无 | 启用 extendedKeyUsage 扩展（叶证书）的验证。禁用此功能可避免错误发布和/或误用证书的问题 | 可以禁用 |
| MBEDTLS_X509_RSASSA_PSS_SUPPORT | 无 | 启用使用 RSASSA-PSS（也称为 PKCS＃1 v2.1）签名的 X.509 证书，CRL 和 CSRS 的解析和验证。根据需要选择是否禁用 | 根据需要选择 |
| MBEDTLS_BLOWFISH_C | 无 | 启用 Blowfish 分组密码 | 可以禁用 |
| MBEDTLS_ERROR_C | 无 | 启用错误代码到错误字符串的转换 | 可以禁用 |
| MBEDTLS_HMAC_DRBG_C | MBEDTLS_MD_C | 启用随机字节发生器 | 可以禁用 |
| MBEDTLS_PEM_WRITE_C | MBEDTLS_BASE64_C | 此模块添加了对编码/写入PEM文件的支持。TLS Client 不需要 | 可以禁用 |
| MBEDTLS_PK_WRITE_C | MBEDTLS_PK_C | 启用通用公钥写入功能。嵌入式系统一般不需要，禁用 | 禁用 |
| MBEDTLS_RIPEMD160_C | 无 | RIPEMD (RACE原始完整性校验讯息摘要)是一种加密哈希函数，通用性差于 SHA-1/2 | 可以禁用 |
| MBEDTLS_SSL_CACHE_C | 无 | 启用 SSL 缓存 | 可以禁用 |
| MBEDTLS_SSL_TICKET_C | MBEDTLS_CIPHER_C | 服务端的配置 | 禁用 |
| MBEDTLS_X509_CRL_PARSE_C | MBEDTLS_X509_USE_C | CRL: Certidicate Revocation List (CRL) 证书吊销列表模块 | 可以禁用 |
| MBEDTLS_X509_CSR_PARSE_C | MBEDTLS_X509_USE_C | Certificate Signing Request (CSR).证书签名请求解析，用于 DER 证书 | 可以禁用 |
| MBEDTLS_X509_CREATE_C | MBEDTLS_BIGNUM_C、MBEDTLS_OID_C、MBEDTLS_PK_WRITE_C | 启用X.509核心以创建证书，服务器需要 | 禁用 |
| MBEDTLS_X509_CRT_WRITE_C | MBEDTLS_X509_CREATE_C | 启用创建证书，服务器需要。禁用 |
| MBEDTLS_X509_CSR_WRITE_C | MBEDTLS_X509_CREATE_C | 启用创建X.509证书签名请求（CSR） | 可以禁用 |
| MBEDTLS_XTEA_C | 无 | 启用 XTEA 分组密码 | 可以禁用 |
| MBEDTLS_ECDSA_DETERMINISTIC | MBEDTLS_HMAC_DRBG_C | 启用确定性 ECDSA（RFC 6979），防止签名时缺少熵而导致签名密钥泄露。建议启用 | 建议启用 |

### 密码套件相关配置

**mbedtls** 中密码套件命名形式为 `MBEDTLS_TLS_PSK_WITH_AES_256_GCM_SHA384`。

**mbedtls** 在数组 `static const int ciphersuite_preference[]` 中定义了所有支持的密码套件，如果开启支持所有的密码套件将会占用非常大的 ROM 空间。这里建议用户通过 `MBEDTLS_SSL_CIPHERSUITES` 宏来指定客户端与服务器使用具体哪种加密套件。指定加密套件后，将不需要的加密套件和依赖的功能组件全部禁用，同时禁用不需要的椭圆曲线，来最大程度上节省 ROM 空间。

| 配置  |依赖    | 说明 | 优化建议 |
| :---- | :---- | :---- | :---- |
| MBEDTLS_SSL_CIPHERSUITES | 无 | 通过指定密码套件来节省 ROM 和 几百字节的 RAM。这里注意需要指定服务器支持的加密套件，并为该加密套件启用相关的功能组件，关闭其他功能组件。如果只接入一个 SSL 服务器，通常这里只需要定义支持一个加密套件即可 | 仅指定根证书需要的加密套件 |
| MBEDTLS_AES_C | 无 | 通过启用 AES 来支持 `*_WITH_AES_*` 类型的密码套件 | 根据需要选择 |
| MBEDTLS_GCM_C | MBEDTLS_AES_C、MBEDTLS_CAMELLIA_C | 启用该配置来支持 `*_AES_GCM_*`、`*_CAMELLIA_GCM_*` 类型的密码套件 | 根据需要选择 |
| MBEDTLS_REMOVE_ARC4_CIPHERSUITES | 无 | 默认启用，在 SSL/TLS 中禁用 RC4 密码套件 | 根据需要选择 |
| MBEDTLS_ARC4_C | 无 | 启用 RC4 加密套件，`*_WITH_RC4*` 类型密码套件。根据需要选择是否禁用 | 根据需要选择 |
| MBEDTLS_CAMELLIA_C | 无 | 启用 Camellia 分组密码，用于支持 `*_WITH_CAMELLIA_*` 类型的密码套件。根据需要选择是否禁用 | 根据需要选择 |
| MBEDTLS_CIPHER_MODE_CBC | 无 | 为对称密码启用密码块链接模式（CBC）。如果使用了 CBC 密码套件则需要启用 | 根据需要选择 |
| MBEDTLS_CIPHER_MODE_CFB | 无 | 为对称密码启用密码反馈模式（CFB）。如果使用了 CFB 密码套件则需要启用 | 根据需要选择 |
| MBEDTLS_CIPHER_MODE_CTR | 无 | 启用对称密码的计数器分组密码模式（CTR）。如果使用了 CTR 密码套件则需要启用 | 根据需要选择 |
| **MBEDTLS_CIPHER_PADDING_XXX** | 无 | 在密码层中启用填充模式。如果禁用所有填充模式，则只有完整块可以与 CBC 一起使用 | 根据需要选择，可以全禁用 |
| MBEDTLS_CIPHER_PADDING_PKCS7 | 无 |  | 可以禁用 |
| MBEDTLS_CIPHER_PADDING_ONE_AND_ZEROS | 无 |  | 可以禁用 |
| MBEDTLS_CIPHER_PADDING_ZEROS_AND_LEN | 无 |  | 可以禁用 |
| MBEDTLS_CIPHER_PADDING_ZEROS | 无 |  | 可以禁用 |
| **MBEDTLS_CIPHER_PADDING_XXX** ||||
| MBEDTLS_KEY_EXCHANGE_ECDH_RSA_ENABLED | MBEDTLS_ECDH_C、MBEDTLS_X509_CRT_PARSE_C | 启用 `*_ECDH_RSA_* 类型的密码套件` | 根据需要选择 |
| MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED | MBEDTLS_ECDH_C、MBEDTLS_RSA_C、MBEDTLS_PKCS1_V15、MBEDTLS_X509_CRT_PARSE_C | 启用 `*_ECDHE_RSA_*` 类型密码套件 | 根据需要选择 |
| MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED | MBEDTLS_ECDH_C、MBEDTLS_ECDSA_C、MBEDTLS_X509_CRT_PARSE_C | 启用 `*_ECDHE_ECDSA_*` 类型密码套件 | 根据需要选择 |
| MBEDTLS_KEY_EXCHANGE_ECDH_ECDSA_ENABLED | MBEDTLS_ECDH_C、MBEDTLS_X509_CRT_PARSE_C | 启用 `*_ECDHE_ECDSA_*` 类型密码套件 | 根据需要选择 |
| MBEDTLS_KEY_EXCHANGE_DHE_RSA_ENABLED | MBEDTLS_DHM_C、MBEDTLS_RSA_C、MBEDTLS_PKCS1_V15、MBEDTLS_X509_CRT_PARSE_C | 启用 `*_DHE_RSA_*` 类型密码套件 | 根据需要选择 |
| MBEDTLS_KEY_EXCHANGE_PSK_ENABLED | 无 | 启用 `*_PSK_*` 类型密码套件 | 根据需要选择 |
| MBEDTLS_KEY_EXCHANGE_DHE_PSK_ENABLED | MBEDTLS_DHM_C | 启用 `*_DHE_PSK_*` 类型密码套件 | 根据需要选择 |
| MBEDTLS_KEY_EXCHANGE_ECDHE_PSK_ENABLED | MBEDTLS_ECDH_C | 启用 `*_ECDHE_PSK_*` 类型密码套件 | 根据需要选择 |
| MBEDTLS_KEY_EXCHANGE_RSA_PSK_ENABLED | MBEDTLS_RSA_C、MBEDTLS_PKCS1_V15、MBEDTLS_X509_CRT_PARSE_C | 启用 `*_RSA_PSK_*` 类型密码套件 | 根据需要选择 |
| MBEDTLS_KEY_EXCHANGE_RSA_ENABLED | MBEDTLS_RSA_C、MBEDTLS_PKCS1_V15、MBEDTLS_X509_CRT_PARSE_C | 启用 `*_RSA_*` 类型密码套件 | 根据需要选择 |
| MBEDTLS_CCM_C | MBEDTLS_AES_C 或 MBEDTLS_CAMELLIA_C | 启用具有 CBC-MAC（CCM）模式的计数器用于128位分组密码，用于支持 AES-CCM 密码套件。根据需要选择是否禁用 | 根据需要选择 |
| MBEDTLS_DES_C | 无 | 启用 DES 块密码 | 可以禁用 | 可以禁用 |
| MBEDTLS_DHM_C | 无 | 启用 Diffie-Hellman-Merkle 模块，用于支持 DHE-RSA, DHE-PSK 密码套件。根据需要选择是否禁用 | 根据需要选择 |

### 椭圆曲线相关配置

用户成功选择了匹配的加密套件，并验证可以正常建立握手连接和加密通讯后，可以尝试将加密套件不需要的椭圆曲线禁用。

| 配置  |依赖  | 说明 | 优化建议 |
| :---- | :---- | :---- | :---- |
| MBEDTLS_ECDH_C | MBEDTLS_ECP_C | 启用椭圆曲线 Diffie-Hellman 库。用于支持 `*_ECDHE_ECDSA_*`、`*_ECDHE_RSA_*`、`*_DHE_PSK_*` 类型的密码套件 | 根据需要选择 |
| MBEDTLS_ECDSA_C | MBEDTLS_ECP_C、MBEDTLS_ASN1_WRITE_C、MBEDTLS_ASN1_PARSE_C | 用于支持 `*_ECDHE_ECDSA_*` 类型的密码套件 | 根据需要选择 |
| MBEDTLS_ECP_C | MBEDTLS_BIGNUM_C 和至少一个 MBEDTLS_ECP_DP_XXX_ENABLED | 启用 GF(p) 椭圆曲线 | 根据需要选择 |
| **MBEDTLS_ECP_XXXX_ENABLED** | 无 | 在椭圆曲线模块中启用特定的曲线。默认情况下启用所有支持的曲线。可以根据实际情况选择一个曲线即可 | 根据需要选择 |
| MBEDTLS_ECP_DP_SECP192R1_ENABLED | 无 | | 根据需要选择 |
| MBEDTLS_ECP_DP_SECP224R1_ENABLED | 无 | | 根据需要选择 |
| MBEDTLS_ECP_DP_SECP256R1_ENABLED | 无 | | 根据需要选择 |
| MBEDTLS_ECP_DP_SECP384R1_ENABLED | 无 | | 根据需要选择 |
| MBEDTLS_ECP_DP_SECP521R1_ENABLED | 无 | | 根据需要选择 |
| MBEDTLS_ECP_DP_SECP192K1_ENABLED | 无 | | 根据需要选择 |
| MBEDTLS_ECP_DP_SECP224K1_ENABLED | 无 | | 根据需要选择 |
| MBEDTLS_ECP_DP_SECP256K1_ENABLED | 无 | | 根据需要选择 |
| MBEDTLS_ECP_DP_BP256R1_ENABLED | 无 | | 根据需要选择 |
| MBEDTLS_ECP_DP_BP384R1_ENABLED | 无 | | 根据需要选择 |
| MBEDTLS_ECP_DP_BP512R1_ENABLED | 无 | | 根据需要选择 |
| MBEDTLS_ECP_DP_CURVE25519_ENABLED | 无 | | 根据需要选择 |
| **MBEDTLS_ECP_XXXX_ENABLED** |||

### TLS 版本选择相关配置

通常 SSL/TLS 服务器支持多种 TLS 协议版本，客户端则不需要支持所有的协议版本。因此在确定服务器支持的 TLS 协议版本后，可以禁用其他版本的协议。

| 配置  |依赖    | 说明 | 优化建议 |
| :---- | :---- | :---- | :---- |
| MBEDTLS_SSL_PROTO_TLS1 | MBEDTLS_MD5_C、MBEDTLS_SHA1_C | 启用对 TLS 1.0 版本的支持 | 根据需要选择 |
| MBEDTLS_SSL_PROTO_TLS1_1 | MBEDTLS_MD5_C、MBEDTLS_SHA1_C | 启用对 TLS 1.1 版本的支持 | 根据需要选择 |
| MBEDTLS_SSL_PROTO_TLS1_2 | MBEDTLS_SHA1_C 或者 MBEDTLS_SHA256_C 或者 MBEDTLS_SHA512_C | 启用对 TLS 1.2 版本的支持 | 根据需要选择 |

### DTLS 相关配置

DTLS 是基于 UDP 的安全加密连接，目的是保障 UDP 通讯的数据安全。由于 UDP 本身不支持自动重传，且存在丢包问题，所以在进行握手连接的时候与 TLS 有些许不同，但两者重复了大部分的代码。因此可以通过下表中的配置来优化 DTLS 加密连接。

如果用户的系统中，不需要使用 DTLS，则可以将下表中的所有配置禁用。

| 配置  |依赖    | 说明 | 优化建议 |
| :---- | :---- | :---- | :---- |
| MBEDTLS_SSL_PROTO_DTLS | MBEDTLS_SSL_PROTO_TLS1_1 或 MBEDTLS_SSL_PROTO_TLS1_2 | 启用 DTLS 功能，用于对 UDP 进行加密| 如果不需要 DTLS 加密连接，则禁用 |
| MBEDTLS_SSL_DTLS_ANTI_REPLAY | MBEDTLS_SSL_TLS_C、MBEDTLS_SSL_PROTO_DTLS | 启用对DTLS中的反重放机制的支持 | 可以禁用 |
| MBEDTLS_SSL_DTLS_HELLO_VERIFY | MBEDTLS_SSL_PROTO_DTLS | 启用对 DTLS HelloVerifyRequest 的支持 | 需要开启 |
| MBEDTLS_SSL_DTLS_CLIENT_PORT_REUSE | MBEDTLS_SSL_DTLS_HELLO_VERIFY | 为从同一端口重新连接的客户端启用服务器端支持，需要服务器特殊支持 | 可以禁用 |
| MBEDTLS_SSL_DTLS_BADMAC_LIMIT | MBEDTLS_SSL_PROTO_DTLS |  启用支持 MAC 错误的记录限制 | 可以禁用 |
| MBEDTLS_SSL_COOKIE_C | 无 | DTLS hello cookie 支持。非 DTLS 下可以禁用 | 可以禁用 |

## 参考

- mbedTLS 官方网站：https://tls.mbed.org/
- 测试时用的配置文件

```c
/* tls_config.h*/
#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

#include <rtthread.h>

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#define MBEDTLS_HAVE_ASM
#define MBEDTLS_HAVE_TIME
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_AES_C
#define MBEDTLS_CTR_DRBG_C
// #define MBEDTLS_ECDH_C
// #define MBEDTLS_ECDSA_C
#define MBEDTLS_ECP_C
// #define MBEDTLS_GCM_C
#define MBEDTLS_MD_C
// #define MBEDTLS_NET_C
#define MBEDTLS_OID_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_SHA256_C
// #define MBEDTLS_SHA512_C
#define MBEDTLS_SSL_CLI_C
// #define MBEDTLS_SSL_SRV_C
#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_X509_USE_C
#define MBEDTLS_BASE64_C
// #define MBEDTLS_CERTS_C
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_AES_ROM_TABLES
#define MBEDTLS_MPI_MAX_SIZE         384
#define MBEDTLS_MPI_WINDOW_SIZE        2
#define MBEDTLS_ECP_MAX_BITS         384
#define MBEDTLS_ECP_WINDOW_SIZE        2
#define MBEDTLS_ECP_FIXED_POINT_OPTIM  0
#define MBEDTLS_ECP_NIST_OPTIM
#define MBEDTLS_ENTROPY_MAX_SOURCES 2
#define MBEDTLS_SSL_CIPHERSUITES                        \
    MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA256

// #define MBEDTLS_SSL_MAX_CONTENT_LEN             3584
#define MBEDTLS_NO_PLATFORM_ENTROPY
// #define MBEDTLS_TLS_DEFAULT_ALLOW_SHA1_IN_KEY_EXCHANGE
#define MBEDTLS_RSA_C
#define MBEDTLS_SHA1_C
#define MBEDTLS_TIMING_C
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_TIMING_ALT
// #define MBEDTLS_DEBUG_C
#define MBEDTLS_MD5_C
// #define MBEDTLS_HAVE_TIME_DATE
#define MBEDTLS_CIPHER_MODE_CBC
// #define MBEDTLS_CIPHER_MODE_CFB
// #define MBEDTLS_CIPHER_MODE_CTR
// #define MBEDTLS_CIPHER_PADDING_PKCS7
// #define MBEDTLS_CIPHER_PADDING_ONE_AND_ZEROS
// #define MBEDTLS_CIPHER_PADDING_ZEROS_AND_LEN
// #define MBEDTLS_CIPHER_PADDING_ZEROS
#define MBEDTLS_REMOVE_ARC4_CIPHERSUITES
// #define MBEDTLS_ECP_DP_SECP192R1_ENABLED
// #define MBEDTLS_ECP_DP_SECP224R1_ENABLED
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECP_DP_SECP384R1_ENABLED
// #define MBEDTLS_ECP_DP_SECP521R1_ENABLED
// #define MBEDTLS_ECP_DP_SECP192K1_ENABLED
// #define MBEDTLS_ECP_DP_SECP224K1_ENABLED
// #define MBEDTLS_ECP_DP_SECP256K1_ENABLED
// #define MBEDTLS_ECP_DP_BP256R1_ENABLED
// #define MBEDTLS_ECP_DP_BP384R1_ENABLED
// #define MBEDTLS_ECP_DP_BP512R1_ENABLED
// #define MBEDTLS_ECP_DP_CURVE25519_ENABLED
// #define MBEDTLS_ECDSA_DETERMINISTIC
// #define MBEDTLS_KEY_EXCHANGE_ECDH_RSA_ENABLED
// #define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
// #define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
// #define MBEDTLS_KEY_EXCHANGE_ECDH_ECDSA_ENABLED
// #define MBEDTLS_KEY_EXCHANGE_DHE_RSA_ENABLED
// #define MBEDTLS_KEY_EXCHANGE_PSK_ENABLED
// #define MBEDTLS_KEY_EXCHANGE_DHE_PSK_ENABLED
// #define MBEDTLS_KEY_EXCHANGE_ECDHE_PSK_ENABLED
// #define MBEDTLS_KEY_EXCHANGE_RSA_PSK_ENABLED
#define MBEDTLS_KEY_EXCHANGE_RSA_ENABLED
// #define MBEDTLS_PK_PARSE_EC_EXTENDED
// #define MBEDTLS_ERROR_STRERROR_DUMMY
// #define MBEDTLS_GENPRIME
// #define MBEDTLS_FS_IO
// #define MBEDTLS_PK_RSA_ALT_SUPPORT
// #define MBEDTLS_PKCS12_C
#define MBEDTLS_PKCS1_V15
// #define MBEDTLS_PKCS1_V21
// #define MBEDTLS_SELF_TEST
// #define MBEDTLS_SSL_ALL_ALERT_MESSAGES
// #define MBEDTLS_SSL_ENCRYPT_THEN_MAC
// #define MBEDTLS_SSL_EXTENDED_MASTER_SECRET
// #define MBEDTLS_SSL_FALLBACK_SCSV
// #define MBEDTLS_SSL_CBC_RECORD_SPLITTING
// #define MBEDTLS_SSL_RENEGOTIATION
// #define MBEDTLS_SSL_MAX_FRAGMENT_LENGTH
// #define MBEDTLS_SSL_PROTO_TLS1
// #define MBEDTLS_SSL_PROTO_TLS1_1
#define MBEDTLS_SSL_PROTO_TLS1_2
// #define MBEDTLS_SSL_ALPN
// #define MBEDTLS_SSL_PROTO_DTLS
// #define MBEDTLS_SSL_DTLS_ANTI_REPLAY
// #define MBEDTLS_SSL_DTLS_HELLO_VERIFY
// #define MBEDTLS_SSL_DTLS_CLIENT_PORT_REUSE
// #define MBEDTLS_SSL_DTLS_BADMAC_LIMIT
// #define MBEDTLS_SSL_SESSION_TICKETS
// #define MBEDTLS_SSL_EXPORT_KEYS
// #define MBEDTLS_SSL_SERVER_NAME_INDICATION
// #define MBEDTLS_SSL_TRUNCATED_HMAC
// #define MBEDTLS_VERSION_FEATURES
// #define MBEDTLS_X509_CHECK_KEY_USAGE
// #define MBEDTLS_X509_CHECK_EXTENDED_KEY_USAGE
// #define MBEDTLS_X509_RSASSA_PSS_SUPPORT
// #define MBEDTLS_AESNI_C
// #define MBEDTLS_ARC4_C
// #define MBEDTLS_BLOWFISH_C
// #define MBEDTLS_CAMELLIA_C
// #define MBEDTLS_CCM_C
// #define MBEDTLS_DES_C
// #define MBEDTLS_DHM_C
#define MBEDTLS_ENTROPY_C
// #define MBEDTLS_ERROR_C
// #define MBEDTLS_HMAC_DRBG_C
// #define MBEDTLS_PADLOCK_C
// #define MBEDTLS_PEM_WRITE_C
// #define MBEDTLS_PK_WRITE_C
// #define MBEDTLS_PKCS5_C
#define MBEDTLS_PLATFORM_C
// #define MBEDTLS_RIPEMD160_C
// #define MBEDTLS_SSL_CACHE_C
// #define MBEDTLS_SSL_COOKIE_C
// #define MBEDTLS_SSL_TICKET_C
// #define MBEDTLS_VERSION_C   
// #define MBEDTLS_X509_CRL_PARSE_C
// #define MBEDTLS_X509_CSR_PARSE_C
// #define MBEDTLS_X509_CREATE_C
// #define MBEDTLS_X509_CRT_WRITE_C   
// #define MBEDTLS_X509_CSR_WRITE_C   
// #define MBEDTLS_XTEA_C

#if defined(YOTTA_CFG_MBEDTLS_USER_CONFIG_FILE)
#include YOTTA_CFG_MBEDTLS_USER_CONFIG_FILE
#elif defined(MBEDTLS_USER_CONFIG_FILE)
#include MBEDTLS_USER_CONFIG_FILE
#endif

#include "mbedtls/check_config.h"

#define tls_malloc  rt_malloc
#define tls_free    rt_free
#define tls_realloc rt_realloc
#define tls_calloc  rt_calloc

#endif /* MBEDTLS_CONFIG_H */
```
