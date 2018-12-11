
# CA 证书文件

- `certs/default` 目录中存储着常用的 CA 证书文件
- `certs` 目录下存储着用户增加的 CA 证书文件

如果 `certs/default` 目录下没有包含用户需要的 CA 根证书文件，则需要用户将自己的 PEM 格式的 CA 证书拷贝 `certs` 根目录下。（仅支持 PEM 格式证书，不支持 DER 格式证书）

## 说明

- `PEM 格式证书`

    **PEM 格式证书** 通常是以 **.pem** 和 **.cer** 后缀名结尾的文件。

    使用文本编辑器打开后，文件内容以 `-----BEGIN CERTIFICATE-----` 开头，以 `-----END CERTIFICATE-----` 结尾。
- `DER 格式证书`

    **DER 格式证书** 是二进制文件类型。
