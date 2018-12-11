# 工作原理

**mbedtls** 软件包是对 SSL/TLS 协议的实现。SSL（安全套接层）和 TLS（传输安全层）均是为了保证传输过程中信息的安全，是在明文传输基础上进行的加密，然后以密文的形式传输数据。

mbedTLS 建立安全通信连接需要经过以下几个步骤：

- 初始化 SSL/TLS 上下文
- 建立 SSL/TLS 握手
- 发送、接收数据
- 交互完成，关闭连接

其中，最关键的步骤就是 **SSL/TLS 握手** 连接的建立，这里需要进行证书校验。

## SSL/TLS 握手流程

![SSL/TLS 握手交互流程](./figures/mbedtlsHandShake.png)

## DTLS 握手流程

为了避免拒绝服务攻击，DTLS采用和IKE一样的无状态 cookie 技术。当客户端发送 client hello 消息后，服务器发送 HelloVerifyRequest 消息，这个消息包含了无状态 cookie。客户端收到之后必须重传添加上了 cookie 的 clienthello。

DTLS 握手流程如下图所示：

![DTLS 握手流程](./figures/mbedtlsHandShakeDTLS.png)
