# API 说明

为了方便用户使用，这里列出了常用的 API，并给出了相关的使用说明。 

## 初始化函数

```c
int webnet_init(void);
```

用于初始化 WebNet 服务器，包括创建线程用于监听客户端连接事件、初始化开启的功能模块等功能；

|**参数**     | **描述**       |
| :------- | :--------- |
| 无       | 无         |
| **返回** | **--**   |
| = 0      | 初始化成功 |
| < 0      | 初始化失败 |

## 设置监听套接字端口

```c
void webnet_set_port(int port);
```

用于设置当前 WebNet 服务器监听端口号，WebNet 服务器默认监听端口号是 80，这也是 HTTP 协议默认端口号。使用默认端口号访问 URL 地址时可以不输入端口号直接访问，当使用非默认端口号时，需要在 URL 地址上指明端口号，如：`http://host:8080/index.html` 。该函数只能**用于 WebNet 服务器初始化之前**。

|**参数**      | **描述**                  |
| :------- | :------------------- |
| port     | 设置的监听套接字端口    |
| **返回**  | **--**              |
| 无       | 无                   |

## 获取监听套接字端口

```c
int webnet_get_port(void);
```

用于获取当前 WebNet 服务器监听套接字端口号。

|**参数**     | ****描述****             |
| :------- | :--------------- |
| 无       | 无               |
| **返回** | **--**         |
| >=0      | 监听套接字端口号 |

## 设置服务器根目录

```c
void webnet_set_root(const char* webroot_path);
```

用于设置当前 WebNet 服务器根目录路径，WebNet 服务器默认根目录为 `/webnet`，浏览器和 WebNet 函数中使用或访问的路径都是基于根目录路径。当浏览器访问 `http://host/index.html` 时，会把文件系统中的 `/webnet/index.html` 返回给浏览器。

|**参数**         | **描述**             |
| :----------- | :--------------- |
| webroot_path | 设置的根目录地址 |
| **返回**     | **--**         |
| 无           | 无               |

## 获取服务器根目录

```c
const char* webnet_get_root(void);
```

用于获取当前 WebNet 服务器根目录地址。

|**参数**     | **描述**       |
| :------- | :--------- |
| 无       | 无         |
| **返回** | **--**   |
| != NULL  | 根目录地址 |

## 获取请求链接的类型

```c
const char* mime_get_type(const char* url);
```

用于获取当前请求 URL 链接的类型，如：网页、图片、文本等。

|**参数**     | **描述**           |
| :------- | :------------- |
| url      | 请求链接的地址 |
| **返回** | **--**       |
| != NULL  | 请求链接的类型 |

## 添加 ASP 变量处理方式

```c
void webnet_asp_add_var(const char* name, void (*handler)(struct webnet_session* session));
```

该函数用于添加一个 ASP 变量处理方式，当 ASP 文件中出现添加的 `name` 变量名时，会执行对应的 `handle`  操作。

|**参数**                                            | **描述**             |
| :---------------------------------------------- | :--------------- |
| name                                            | ASP 变量名称     |
| void (*handler)(struct webnet_session* session) | ASP 变量处理方式 |
| **返回**                                        | **--**         |
| 无                                              | 无               |

## 添加 CGI 事件处理方式

```c
void webnet_cgi_register(const char* name, void (*handler)(struct webnet_session* session));
```

该函数用于注册一个 CGI 事件处理方式，当浏览器请求带有 `name` 名称的 URL 时，会执行相应的 `handle` 操作。

|**参数**                                            | **描述**             |
| :---------------------------------------------- | :--------------- |
| name                                            | CGI 事件名称     |
| void (*handler)(struct webnet_session* session) | CGI 事件处理方式 |
| **返回**                                        | **--**         |
| 无                                              | 无               |

## 设置 CGI 事件根目录

```c
void webnet_cgi_set_root(const char* root);
```

WebNet 服务器默认的 CGI 事件根目录为`/cgi-bin`，当浏览器请求 `http://host/cgi-bin/test` 地址时，会执行 `test` 名称对应的 CGI 事件处理函数。

该函数用于设置新的 CGI 事件根目录，设置成功之前的 CGI 根目录将不再起作用。

|**参数**     | **描述**           |
| :------- | :------------- |
| root     | CGI 事件根目录 |
| **返回** | **--**       |
| 无       | 无             |

## 设置基本认证信息

```c
void webnet_auth_set(const char* path, const char* username_password);
```

用于设置目录访问时的基本认证信息，包括用户名和密码。

|**参数**              | **描述**                                           |
| :---------------- | :--------------------------------------------- |
| path              | 需要设置基本认证信息的目录                     |
| username_password | 设置的用户名和密码，格式为 `username:password` |
| **返回**          | **--**                                       |
| 无                | 无                                             |

## 设置目录别名

```c
void webnet_alias_set(char* old_path, char* new_path);
```

用于设置目录的别名，设置成功之后可以使用目录别名访问该目录。

|**参数**     | **描述**                                     |
| :------- | :--------------------------------------- |
| old_path | 需要设置别名的目录                       |
| new_path | 设置的目录别名，一般为服务器中存在的目录 |
| **返回** | **--**                                 |
| 无       | 无                                       |

## 发送 HTTP 请求头部

```c
void webnet_session_set_header(struct webnet_session* session, const char* mimetype, int code, const char* title, int length);
```

用于拼接并发送头部信息到连接的客户端，一般用于 ASP 变量处理函数和 CGI 事件处理函数中。

|**参数**     | **描述**                                                         |
| :------- | :----------------------------------------------------------- |
| session  | 当前服务器连接的会话                                         |
| mimetype | 需要发送的响应文件类型（Content-Type），可以使用  `mime_get_type`  函数获取 |
| code     | 发送的响应状态码，正常为 200                                 |
| title    | 发送的响应状态类型，正常为 OK                                |
| length   | 需要发送的响应文件长度（Content-Length）                     |
| **返回** | **--**                                                     |
| 无       | 无                                                           |

## 发送 HTTP 响应数据

```c
int  webnet_session_write(struct webnet_session* session, const rt_uint8_t* data, rt_size_t size);
```

用于发送响应数据到客户端，一般用于 ASP 变量处理函数和 CGI 事件处理函数中。

|**参数**     | **描述**                 |
| :------- | :------------------- |
| session  | 当前服务器连接的会话 |
| data     | 发送的数据指针       |
| size     | 发送的数据长度       |
| **返回** | **--**             |
| 无       | 无                   |


## 发送 HTTP 固定格式响应数据

```c
void webnet_session_printf(struct webnet_session* session, const char* fmt, ...);
```

用于发送固定格式的响应数据到客户端，一般用于 ASP 变量处理函数和 CGI 事件处理函数中。

|**参数**     | **描述**                     |
| :------- | :----------------------- |
| session  | 当前服务器连接的会话     |
| fmt      | 自定义的输入数据的表达式 |
| ...      | 输入的参数               |
| **返回** | **--**                 |
| 无       | 无                       |

## 获取上传文件的名称

```c
const char* webnet_upload_get_filename(struct webnet_session* session);
```

获取当前上传文件的名称，用于打开或创建文件。

|**参数**     | **描述**                 |
| :------- | :------------------- |
| session  | 当前服务器连接的会话 |
| **返回** | **--**             |
| != NULL  | 当前上传文件的名称   |

## 获取上传文件的类型

```c
const char* webnet_upload_get_content_type(struct webnet_session* session);
```

获取当前上传文件的类型。

|**参数**     | **描述**                 |
| :------- | :------------------- |
| session  | 当前服务器连接的会话 |
| **返回** | **--**             |
| != NULL  | 当前上传文件的类型   |

## 获取上传文件参数

```c
const char* webnet_upload_get_nameentry(struct webnet_session* session, const char* name);
```

获取注册的上传文件的分隔符（HTTP 请求 boundary 参数）。

|**参数**     | **描述**                 |
| :------- | :------------------- |
| session  | 当前服务器连接的会话 |
| name     | 上传文件的目录路径   |
| **返回** | **--**             |
| != NULL  | 当前上传文件的类型   |

## 获取上传文件打开的文件描述符

```c
const void* webnet_upload_get_userdata(struct webnet_session* session);
```

获取当前上传文件打开之后生成的文件描述符，用于读写数据到文件中。

|**参数**     | **描述**                     |
| :------- | :----------------------- |
| session  | 当前服务器连接的会话     |
| **返回** | **--**                 |
| != NULL  | 上传文件打开的文件描述符 |