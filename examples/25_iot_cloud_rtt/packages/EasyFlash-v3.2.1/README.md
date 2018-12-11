# EasyFlash

[![GitHub release](https://img.shields.io/github/release/armink/EasyFlash.svg)](https://github.com/armink/EasyFlash/releases/latest) [![GitHub commits](https://img.shields.io/github/commits-since/armink/EasyFlash/3.2.1.svg)](https://github.com/armink/EasyFlash/compare/3.2.1...master) [![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/armink/EasyFlash/master/LICENSE)

## 1、介绍

[EasyFlash](https://github.com/armink/EasyFlash)是一款开源的轻量级嵌入式Flash存储器库，主要为MCU(Micro Control Unit)提供便捷、通用的上层应用接口，使得开发者更加高效实现基于的Flash存储器常见应用开发。该库目前提供 **三大实用功能** ：

- **Env** 快速保存产品参数，支持 **写平衡（磨损平衡）** 及 **掉电保护** 模式

EasyFlash不仅能够实现对产品的 **设定参数** 或 **运行日志** 等信息的掉电保存功能，还封装了简洁的 **增加、删除、修改及查询** 方法， 降低了开发者对产品参数的处理难度，也保证了产品在后期升级时拥有更好的扩展性。让Flash变为NoSQL（非关系型数据库）模型的小型键值（Key-Value）存储数据库。

- **IAP** 在线升级再也不是难事儿

该库封装了IAP(In-Application Programming)功能常用的接口，支持CRC32校验，同时支持Bootloader及Application的升级。

- **Log** 无需文件系统，日志可直接存储在Flash上

非常适合应用在小型的不带文件系统的产品中，方便开发人员快速定位、查找系统发生崩溃或死机的原因。同时配合[EasyLogger](https://github.com/armink/EasyLogger)(我开源的超轻量级、高性能C日志库，它提供与EasyFlash的无缝接口)一起使用，轻松实现C日志的Flash存储功能。

> 更多详细介绍，请查看 EasyFlash 项目的 GitHub 仓库：https://github.com/armink/EasyFlash

## 2、文档

- 基于 RT-Thread 的移植说明文档：[`ports/README.md`](ports/README.md)
- API 说明文档：[ `docs/zh/api.md`](docs/zh/api.md)
- 通用移植文档：[ `docs/zh/port.md`](docs/zh/port.md)

## 3、配置说明

```shell
EasyFlash: Lightweight embedded flash memory library.
    [*]   ENV: Environment variables
            ENV mode (Normal mode)  --->
    (2048)  ENV setting size. MUST be word alignment
    [ ]     Auto update ENV to latest default when current ENV version number is changed.
    [ ]   LOG: Save logs on flash
    [ ]   IAP: In Application Programming
    (4096) Erase minimum granularity
    (0)   Start addr on flash or partition
    [*]   Enable debug log output
          Version (v3.2.1)  --->
```

- `ENV: Environment variables`： 是否使能环境变量功能
  - `ENV mode`：环境变量功能的模式，各个模式下对于 Flash 资源的消耗各不相同，详见 [ `docs/zh/port.md`](docs/zh/port.md)
    - 正常模式
    - 磨损平衡模式
    - 掉电保护模式
    - 磨损平衡 + 掉电保护模式
- `ENV setting size. MUST be word alignment` ：设定的 ENV 大小，也就是可以存放 ENV 缓存区的最大长度。注意：目前 EasyFlash 的 ENV 都会在 RAM 中新建一个同样长度的缓冲区，该数值越大，RAM 消耗越多。
- `Auto update ENV to latest default when current ENV version number is changed.`：是否启用环境变量自动更新功能。启动这个功能后，环境变量将在其版本号发生变化时自动更新。
- `LOG: Save logs on flash`：日志功能，可以将日志顺序保存至 Flash 中。还可以配合 EasyLogger 完成产品日志的掉电存储。
- `IAP: In Application Programming`：IAP 在线升级功能，开启后将提供一些 IAP 功能里常用的 API 。
- `Erase minimum granularity`：擦写的最小粒度，一般 SPI Flash 通常为 4KB，STM32F4 片内 Flash 通常为 128KB。
- `Start addr on flash or partition`：EasyFlash 的整个存储区相对于 Flash 或者 分区 的偏移地址，视移植代码而定。
- `Enable debug log output`：是否使能调试日志输出。开启后将会看到更多调试日志信息。


