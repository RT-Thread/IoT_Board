# WebNet

## 1、介绍

WebNet 软件包是 RT-Thread 自主研发的，基于 HTTP 协议的 Web 服务器实现，它不仅提供设备与 HTTP Client 通讯的基本功能，而且支持多种模块功能扩展，且资源占用少、可裁剪性强，充分满足开发者对嵌入式设备服务器的功能需求。

WebNet 软件包功能特点如下：

- 支持 HTTP 1.0/1.1
- 支持 AUTH 基本认证功能
- 支持 CGI 功能
- 支持 ASP 变量替换功能
- 支持 SSI 文件嵌入功能
- 支持 INDEX 目录文件显示功能
- 支持 ALIAS 别名访问功能
- 支持文件上传功能
- 支持预压缩功能
- 支持缓存功能
- 支持断点续传功能

更多软件包功能特点介绍请查看 [详细介绍](docs/introduction.md)。 

### 1.1 目录结构

| 名称       | 说明                     |
| ---------- | ------------------------ |
| docs       | 文档目录                 |
| inc        | 头文件目录               |
| src        | 源文件目录               |
| module     | 功能模块文件目录         |
| samples    | 示例文件目录             |
| LICENSE    | 许可证文件               |
| README.md  | 软件包使用说明           |
| SConscript | RT-Thread 默认的构建脚本 |

### 1.2 许可证

WebNet 软件包遵循 GPL2+ 商业双许可。该软件包可以根据 GNU 标准使用通用公共许可证，详见 LICENSE 文件。如果用于商业应用，可以通过电子邮箱 <business@rt-thread.com > 与我们联系获取商业许可。

### 1.3 依赖

- RT_Thread 3.0+
- DFS 文件系统

## 2、 获取软件包

使用 WebNet软件包需要在 RT-Thread 的包管理中选中它，具体路径如下： 

```c
RT-Thread online packages
    IoT - internet of things  --->
    	[*] WebNet: A HTTP Server for RT-Thread
            (80)  Server listen port
            (16)  Maximum number of server connections
            (/webnet)   Server root directory
                  Select supported modules  --->
                     [ ] LOG: Enanle output log support
                     [ ] AUTH: Enanle basic HTTP authentication support
                     [ ] CGI: Enanle Common Gateway Interface support
                     [ ] ASP: Enanle Active Server Pages support
                     [ ] SSI: Enanle Server Side Includes support
                     [ ] INDEX: Enanle list all the file in the directory support
                     [ ] ALIAS: Enanle alias support
                     [ ] DAV: Enanle Web-based Distributed Authoring and Versioning support
                     [ ] UPLOAD: Enanle upload file support
                     [ ] GZIP: Enable compressed file support by GZIP
                     (0) CACHE: Configure cache level
            [ ]   Enable webnet samples
            	  Version (latest)  --->
```

**Server listen port**：配置服务器监听端口号；

**Maximum number of server connections**：配置服务器最大连接数量；

**Server root directory**：配置服务器根目录路径；

**Select supported modules**：选择服务器支持的功能模块；

**Enable webnet samples** ：配置添加服务器示例文件；

**Version**：配置软件包版本。

配置完成后让 RT-Thread 的包管理器自动更新，或者使用 pkgs --update 命令更新包到 BSP 中。 

## 3、使用 WebNet 软件包

- 软件包详细介绍，请参考 [软件包介绍](docs/introduction.md)
- 详细的示例介绍，请参考 [示例文档](docs/samples.md)
- 如何从零开始使用，请参考 [用户指南](docs/user-guide.md)
- 完整的 API 文档，请参考 [API 手册](docs/api.md)
- 软件包工作原理，请参考 [工作原理](docs/principle.md)
- 更多**详细介绍文档**位于 `/docs` 文件夹下，**使用软件包进行开发前请务必查看**。

## 4、注意事项

- WebNet 软件包使用需要文件系统支持，需要确保运行设备上能使用文件系统。
- WebNet 软件包默认未开启任何模块功能支持，使用的需要根据[软件包介绍](docs/introduction.md)在 Env 中开启需要的功能。

## 5、联系方式 & 感谢

- 维护：RT-Thread 开发团队
- 主页：<https://github.com/RT-Thread-packages/webnet>

