# MicroPython

## 1、介绍

这是一个在 RT-Thread 上的 `MicroPython` 移植，可以运行在 **RT-Thread 3.0** 版本以上。通过这个软件包可以在搭载了 RT-Thread 的嵌入式系统上运行 `MicroPython`。

### 1.1 目录结构

| 名称 | 说明 |
| ---- | ---- |
| docs  | 文档目录，包括入门指南和开发手册 |
| drivers | MicroPython 源代码目录 |
| extmod | MicroPython 源代码目录 |
| lib | MicroPython 源代码目录 |
| py | MicroPython 源代码目录 |
| port | 移植代码目录 |
| LICENSE | Micropython MIT 许可证 |

### 1.2 许可证

RT-Thread MicroPython  遵循 MIT 许可，详见 `LICENSE` 文件。

### 1.3 依赖

- RT-Thread 3.0+

## 2、如何打开 RT-Thread MicroPython

使用 `MicroPython package` 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

![elect_micropytho](./docs/figures/select_micropython.png)

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3、使用 RT-Thread MicroPython

在选中 `MicroPython package` 后，再次进行 `bsp` 编译时，它会被加入到 `bsp` 工程中进行编译。

* 快速入门可查看 [快速上手](./docs/01-Getting_Started_Guide.md) 说明文档。
* 开发过程可参考 `docs` 目录下的开发文档或者查看 [RT-Thread 文档中心](https://www.rt-thread.org/document/site/) 中的 `MicroPython 开发手册`。

## 4、注意事项

- 需要使用 **RT-Thread 3.0** 以上版本。
- 在 `menuconfig` 选项中选择 `Micropython` 的 `latest` 版本。
- 目前 `System Module` 下的 `ffi` 模块只支持 GCC 工具链，且需要在链接脚本中添加相关段信息。 

## 5、开发资源

* [RT-Thread MicroPython 源码](https://github.com/RT-Thread-packages/micropython)
* [RT-Thread MicroPython 论坛](https://www.rt-thread.org/qa/forum.php)
* [MicroPython 官方网站](https://micropython.org/)
* [官方在线文档](http://docs.micropython.org/en/latest/pyboard/)
* [MicroPython 在线演示](https://micropython.org/unicorn)
* [MicroPython 源码](https://github.com/micropython/micropython)
* [MicroPython 官方论坛](http://forum.micropython.org/)
* [MicroPython 中文社区](http://www.micropython.org.cn/)
