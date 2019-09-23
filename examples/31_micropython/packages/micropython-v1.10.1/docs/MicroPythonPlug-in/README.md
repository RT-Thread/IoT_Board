# VSCode 最好用的 MicroPython 插件

VSCode 最好用的 MicroPython 插件，为 MicroPython 开发提供了强大的开发环境，主要特性如下：

- 支持通过网络连接远程调试
- 支持网络、USB 或串口的方式连接开发板
- 支持基于 MicroPython 的代码智能补全
- 支持 MicroPython REPL 交互环境
- 提供丰富的代码示例
- 支持自动同步工程代码
- 支持在设备上运行示例代码
- 支持运行选中的代码片段
- 支持多款 MicroPython 开发板
- 支持在 windows 以及 ubuntu 操作系统下运行

## 准备工作

1. 在 windows 操作系统下使用插件需要将 vscode 的默认终端修改为 powershell，如下图所示：

   ![select_powershell](assets/select_powershell.gif)

如果想要使用 MicroPython 自动补全功能（如果暂时不需要自动补全功能，可以跳过后续步骤），还需要进行如下操作：

2. 安装 Python 插件

3. 按照 Python 插件的提示在 PC 上安装 Python3 并加入到系统环境变量中

![Python plug](assets/install_python_plug.png)

如果在 PC 上已经安装过上述插件和程序，可以跳过此准备步骤。

### ubuntu 支持

本插件支持在 **ubuntu 18.04** 版本下运行，为了避免在 ubuntu 系统下频繁获取串口权限，需要将当前用户加入到 `dialout` 用户组中，手动输入如下命令即可，`$USERNAME` 是系统当前用户名：

`sudo usermod -aG dialout $USERNAME`

注意：配置修改后需要 **重启一下操作系统** 使配置生效。

## 快速入门

### 创建一个 MicroPython 工程

![create_blank_dir](assets/create_blank_prj.gif)

![create_demo_dir](assets/create_demo_prj.gif)

### 连接开发板

可以通过多种方式与开发板建立连接，现支持 USB 和网络连接方式。

- 串口连接方式

![uart_connect](assets/uart_connect.gif)

- USB 连接方式

直接将开发板通过 USB 连接到 PC 机，将会自动通过 USB 连接设备，如下图所示：

![usb_connect_device](assets/usb_connect.gif)

- 网络连接方式

点击连接按钮，然后选择想要连接的设备名称，如下图所示：

![connect_device](assets/connect_device.gif)

注意：第一次连接网络时请先用 USB 或者串口连接电脑，然后参考 network 例程进行网络连接。

### 运行示例代码

和开发板建立连接后，可以直接运行示例代码，并观察代码在开发板上的运行效果，如下图所示：

![run_example](assets/run_example.gif)

## 功能介绍

- 通过网络、USB 或串口的方式连接开发板

- 提供丰富的 MicroPython 代码示例程序

![example_code](assets/example_code.png)

- 支持在设备上直接运行示例代码

![run_example_code](assets/run_example_code.png)

- 支持运行代码片段

![run_code_snippet](assets/run_code_snippet.gif)

- 支持 STM32L4 Pandora IoT Board 、W601 IoT Board 等多款开发板
- 优秀的代码编辑环境
- 基于 MicroPython 的代码智能补全

![auto_complete](assets/auto_complete.gif)

## 开发资源

- [RT-Thread MicroPython 开发用户手册](https://www.rt-thread.org/document/site/submodules/micropython/docs/)
- [RT-Thread MicroPython 软件包](https://github.com/RT-Thread-packages/micropython)
- [RT-Thread MicroPython 示例程序及库](https://github.com/RT-Thread/mpy-snippets)
- [RT-Thread MicroPython 论坛](https://www.rt-thread.org/qa/forum.php?mod=forumdisplay&fid=2&filter=typeid&typeid=20)
- [MicroPython IDE 用户指南](https://www.rt-thread.org/document/site/submodules/micropython/docs/MicroPythonPlug-in/MicroPython_IDE_User_Manual/)
- [MicroPython 固件开发指南](https://www.rt-thread.org/document/site/submodules/micropython/docs/MicroPythonPlug-in/MicroPython_Firmware_Development_Guide/)
- RT-Thread MicroPython 交流 QQ 群：703840633

## 注意事项

- 请选择 PowerShell 作为默认终端

在 PowerShell 终端中输入 `Set-ItemProperty HKCU:\Console VirtualTerminalLevel -Type DWORD 1` 可以解决退格键等显示乱码的问题。

- 不要删除工程目录下的 `.mpyproject.json` 文件

该文件是 MicroPython 工程的配置文件，删除后将无法正常运行 MicroPython 代码程序。

  