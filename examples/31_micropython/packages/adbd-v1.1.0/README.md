# Android Debug Bridge daemon implementation in RT-Thread

在PC与RT-Thread之间建立文件传输与执行shell的通道。

## 1. 已实现功能
- 通信：tcpip
- 通信：usb
- 服务：文件pull/push
- 服务：shell

## 2. 依赖

- 传输 - LWIP/winusb
- 文件系统、POSIX、LIBC
- shell - 依赖finsh/msh 
- 上位机工具

## 3. 配置ADBD

### 3.1 启用ADBD
env配置如下:

```
Using TCPIP transfer                    /* 启用 TCP/IP传输数据 */
Using USB transfer                      /* 启动 USB 传输数据 */
Set transfer thread stack size          /* 设置传输线程栈大小 */
Enable Shell service                    /* 开启 Shell 服务 */
Enable File service                     /* 开启 文件 服务 */
  Set file service thread stack size    /* 设置文件服务线程栈大小 */
  Set file service receive timeout      /* 设置文件接收超时时间 */
Enable external MOD                     /* 使能外部模块 */
  Enable File SYNC Mod                  /* 启用文件同步模块，支持校验MD5，跳过相同文件 */
  Enable File LIST Mod                  /* 启用获取文件目录模块 */
```

## 3. 外部模块

在ADB文件服务的基础上，实现了一套数据传输机制。PC端可使用脚本往板子上发送任意数据。

### 3.1 基本原理

ADB文件服务功能可以将PC端的路径原封不动的发给板子。当PC端输入 adb push ./pc_sync /dev_sync 命令时，ADB会将远端路径 /dev_sync 原封不动的发给设备端。即使包含特殊的字符，也会发送。可以利用这个特性，将远端目录按照一定格式进行构造，然后设备解析构造过的路径，从而执行特殊的操作。当PC机需要发送数据时，先将需要发送的数据生成文件，然后构造特殊的远端路径，执行 adb push 命令，将临时文件发送到设备端。完成一次 PC 到设备端的数据交换。设备端到PC的数据传递原理一样。

### 3.2 文件同步

文件同步时，PC端需要知道设备端的文件信息。所以先构造特殊命令，从设备端把文件信息拉回来。PC端根据设备端发回的信息，判断哪些文件需要同步，哪些文件需要删除。同步文件直接调用 adb push 命令，将文件发送到设备端。删除文件则需要给设备端发送需要删除的文件信息，让设备自行删除。

### 3.2.1 脚本使用

脚本路径为:adbd/tools/script/adb_sync.py。在命令行中输入 python adb_sync.py 本地路径/ 远端路径/,即可将本地的一个文件夹同步到设备端。

## 4. 参考文档

### 4.1 ADB官方文档

- [ADB简介](docs/OVERVIEW.TXT)
- [协议简介](docs/PROTOCOL.TXT)
- [文件服务](docs/SYNC.TXT)

### PC工具

- win:adbd/tools/adb/adb.exe
