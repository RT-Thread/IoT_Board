# 软件包介绍

> [**mbedtls**](https://github.com/RT-Thread-packages/mbedtls) 软件包是 **RT-Thread** 基于 [ARMmbed/mbedtls](https://github.com/ARMmbed/mbedtls/) 开源库的移植。

**mbedTLS**（前身 PolarSSL）是一个由 ARM 公司开源和维护的 SSL/TLS 算法库。其使用 C 编程语言以最小的编码占用空间实现了 SSL/TLS 功能及各种加密算法，易于理解、使用、集成和扩展，方便开发人员轻松地在嵌入式产品中使用 SSL/TLS 功能。

**mbedTLS** 软件包提供了如下的能力:

- 完整的 **SSL v3**、**TLS v1.0**、**TLS v1.1** 和 **TLS v1.2** 协议实现
- **X.509** 证书处理
- 基于 TCP 的 TLS 传输加密
- 基于 UDP 的 DTLS（Datagram TLS）传输加密
- 其它加解密库实现

> 有关 mbedTLS 的更多信息，请参阅 [https://tls.mbed.org](https://tls.mbed.org)。

## 软件框架图

mbedTLS 软件包提供了一组可以单独使用和编译的加密组件，各组件及其可能的依赖关系如下图所示：

![mbedtls 软件框架图](./figures/mbedtlsComponentsDependencies.png)

## 软件包目录结构

**ports** 目录是 RT-Thread 移植 mbedtls 软件包时所涉及到的移植文件，使用 scons 进行重新构建。

```shell
mbedtls
|   LICENSE                         // 软件包许可协议
|   README.md                       // 软件包使用说明
|   SConscript                      // RT-Thread 默认的构建脚本
+---certs                           // certs 根目录存放用户 CA 证书
|   +---default                     // default 目录保存着预置的 CA 证书
+---docs
|   +---figures                     // 文档使用图片
|   |   api.md                      // API 使用说明
|   |   introduction.md             // 软件包详细介绍
|   |   LICENSE                     // 许可证文件
|   |   principle.md                // 实现原理
|   |   footprint-optimization-guide.md // 资源占用优化参考指南
|   |   README.md                   // 文档结构说明
|   |   samples.md                  // 软件包示例
|   |   user-guide.md               // 使用说明
|   +---version.md                  // 版本说明
+---ports                           // 移植文件
|   +---inc
|   +---src
+---samples                         // 示例程序
+---mbedtls                         // ARM mbedtls 源码
```
