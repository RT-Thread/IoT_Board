# API 说明

## 创建会话

> struct webclient_session *webclient_session_create(size_t header_sz);

创建客户端会话结构体。

| 参数              | 描述                                |
|:------------------|:-----------------------------------|
|header_sz          | 最大支持的头部长度                   |
| **返回**          | **描述**                            |
|!= NULL            | webclient 会话结构体指针             |
|= NULL             | 创建失败                            |

## 关闭会话连接

> int webclient_close(struct webclient_session *session);

关闭传入的客户端会话连接，并释放内存。

| 参数              | 描述                                |
|:------------------|:-----------------------------------|
|session            | 当前连接会话结构体指针               |
| **返回**          | **描述**                            |
|=0                 | 成功                                |


## 发送 GET 请求

> int webclient_get(struct webclient_session *session, const char *URI);

发送 HTTP GET 请求命令。

| 参数              | 描述                                |
|:------------------|:-----------------------------------|
|session            | 当前连接会话结构体指针               |
|URI                | 连接的 HTTP 服务器地址               |
| **返回**          | **描述**                            |
|`>0`               | HTTP 响应状态码                     |
|<0                 | 发送请求失败                        |

## 发送获取部分数据的 GET 请求

> int webclient_get_position(struct webclient_session *session, const char *URI, int position);

发送带有 Range 头信息的 HTTP GET 请求命令，多用于完成断点续传功能。

| 参数              | 描述                                |
|:------------------|:-----------------------------------|
|session            | 当前连接会话结构体指针               |
|URI                | 连接的 HTTP 服务器地址               |
|position           | 数据偏移量                 |
| **返回**          | **描述**                            |
|`>0`               | HTTP 响应状态码                     |
|<0                 | 发送请求失败                        |

## 发送 POST 请求

> int webclient_post(struct webclient_session *session, const char *URI, const char *post_data);

发送 HTTP POST 请求命令，上传数据到 HTTP 服务器。

| 参数              | 描述                                |
|:------------------|:-----------------------------------|
|session            | 当前连接会话结构体指针               |
|URI                | 连接的 HTTP 服务器地址               |
|post_data          | 需要上传的数据地址                   |
| **返回**          | **描述**                            |
|`>0`               | HTTP 响应状态码                     |
|<0                 | 发送请求失败                        |

## 发送数据

> int webclient_write(struct webclient_session *session, const unsigned char *buffer, size_t size);

发送数据到连接的服务器。

| 参数              | 描述                                |
|:------------------|:-----------------------------------|
|session            | 当前连接会话结构体指针               |
|buffer             | 发送数据的地址                       |
|size               | 发送数据的长度                       |
| **返回**          | **描述**                            |
|`>0`               | 成功发送数据的长度                   |
|=0                 | 连接关闭                            |
|<0                 | 发送数据失败                        |

## 接收数据

> int webclient_read(struct webclient_session *session, unsigned char *buffer, size_t size);

从连接的服务器接收数据。

| 参数              | 描述                                |
|:------------------|:-----------------------------------|
|session            | 当前连接会话结构体指针               |
|buffer             | 接收数据的存放地址                   |
|size               | 最大接收数据的长度                   |
| **返回**          | **描述**                            |
|`>0`               | 成功接收数据的长度                   |
|=0                 | 连接关闭                            |
|<0                 | 接收数据失败                        |


## 设置接收和发送数据超时时间

> int webclient_set_timeout(struct webclient_session *session, int millisecond);

设置连接的接收和发送数据超时时间。

| 参数              | 描述                                |
|:------------------|:-----------------------------------|
|session            | 当前连接会话结构体指针               |
|millisecond        | 设置的超时时间，单位毫秒             |
| **返回**          | **描述**                           |
|=0                 | 设置超时成功                        |

## 在请求头中添加字段数据

> int webclient_header_fields_add(struct webclient_session *session, const char *fmt, ...);

该函数用于创建会话之后和发送 GET 或 POST 请求之前，用于添加请求头字段数据。

| 参数              | 描述                                |
|:------------------|:-----------------------------------|
|session            | 当前连接会话结构体指针               |
|fmt                | 添加字段数据的表达式                 |
|...                | 添加的字段数据，为可变参数           |
| **返回**          | **描述**                           |
|`>0`               | 成功添加的字段数据的长度            |
| <=0               | 添加失败或者头部数据长度超出        |

## 通过字段名获取字段值数据

> const char *webclient_header_fields_get(struct webclient_session *session, const char *fields);

该函数用于发送 GET 或 POST 请求之后，可以通过传入的字段名称获取对应的字段数据。

| 参数              | 描述                                |
|:------------------|:-----------------------------------|
|session            | 当前连接会话结构体指针               |
|fields             | HTTP 字段名称                       |
| **返回**          | **描述**                            |
|= NULL             | 获取数据失败                        |
|!= NULL            | 成功获取的字段数据                   |


## 接收响应数据到指定地址

> int webclient_response(struct webclient_session *session, unsigned char **response);

该函数用于发送 GET 或 POST 请求之后， 可以接收响应数据到指定地址。

| 参数              | 描述                                |
|:------------------|:-----------------------------------|
|session            | 当前连接会话结构体指针               |
|response           | 存放接收数据的字符串地址             |
| **返回**          | **描述**                           |
| `>0`              | 成功接收数据的长度                  |
| <=0               | 接收数据失败                        |

## 发送 GET/POST 请求并接收响应数据

> int webclient_request(const char *URI, const char *header, const char *post_data, unsigned char **response);

| 参数              | 描述                                |
|:------------------|:-----------------------------------|
|URI                | 连接的 HTTP 服务器地址              |
|header             | 需要发送的头部数据                  |
|                   | = NULL，发送默认头数据信息，只用于发送 GET 请求       |
|                   | != NULL，发送指定头数据信息，可用于发送 GET/POST请求  |
|post_data          | 发送到服务器的数据                  |
|                   | = NULL，该发送请求为 GET 请求       |
|                   | != NULL，该发送请求为 POST 请求     |
|response           | 存放接收数据的字符串地址             |
| **返回**          | **描述**                           |
| `>0`              | 成功接收数据的长度                  |
| <=0               | 接收数据失败                        |


## 获取 HTTP 响应状态码

> int webclient_resp_status_get(struct webclient_session *session);

该函数用于发送 GET 或 POST 请求之后，用于获取返回的响应状态码。

| 参数              | 描述                                |
|:------------------|:-----------------------------------|
|session            | 当前连接会话结构体指针               |
| **返回**          | **描述**                           |
| `>0`              | HTTP 响应状态码                     |

## 获取 Content-Length 字段数据

> int webclient_content_length_get(struct webclient_session *session);

该函数用于发送 GET 或 POST 请求之后，用于获取返回的 Content-Length 字段数据。

| 参数              | 描述                                |
|:------------------|:-----------------------------------|
|session            | 当前连接会话结构体指针               |
| **返回**          | **描述**                           |
| `>0`                | Content-Length 字段数据            | 
| <0                | 获取失败                           | 

## 下载文件到本地

> int webclient_get_file(const char *URI, const char *filename);

从 HTTP 服务器下载文件并存放到本地。

| 参数              | 描述                                |
|:------------------|:-----------------------------------|
|URI                | 连接的 HTTP 服务器地址               |
|filename           | 存放文件位置、名称                   |
| **返回**          | **描述**                            |
|=0                 | 下载文件成功                        |
|<0                 | 下载文件失败                        |

## 上传文件到服务器

> int webclient_post_file(const char *URI, const char *filename, const char *form_data);

从 HTTP 服务器下载文件并存放到本地。

| 参数              | 描述                                |
|:------------------|:-----------------------------------|
|URI                | 连接的 HTTP 服务器地址               |
|filename           | 需要上传的文件位置、名称              |
|form_data          | 附加选项                            |
| **返回**          | **描述**                            |
|=0                 | 上传文件成功                        |
|<0                 | 上传文件失败                        |