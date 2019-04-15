# Ymodem 协议固件升级例程

## 背景知识

### 固件升级简述

固件升级，通常称为 OTA（Over the Air） 升级或者 FOTA（Firmware Over-The-Air） 升级，即固件通过空中下载进行升级的技术。

### Ymodem 简述

Ymodem 是一种文本传输协议，在 OTA 应用中为空中下载技术提供文件传输的支持。基于 Ymodem 协议的固件升级即为 OTA 固件升级的一个具体应用实例。

### Flash 分区简述

通常嵌入式系统程序是没有文件系统的，而是将 Flash 分成不同的功能区块，从而形成不同的功能分区。

要具备 OTA 固件升级能力，通常需要至少有两个程序运行在设备上。其中负责固件校验升级的程序称之为 **bootloader**，另一个负责业务逻辑的程序称之为 **app**。它们负责不同的功能，存储在 Flash 的不同地址范围，从而形成了 `bootloader 分区`和 `app 分区`。

但多数情况下嵌入式系统程序是运行在 Flash 中的，下载升级固件的时候不会直接向 `app 分区`写入新的固件，而是先下载到另外的一个分区暂存，这个分区就是 `download 分区`，也有称之为 `app2 分区`，这取决于 bootloader 的升级模式。

bootloader 分区、app 分区、download 分区及其他分区一起构成了**分区表**。分区表标识了该分区的特有属性，通常包含分区名、分区大小、分区的起止地址等。

### bootloader 升级模式

bootloader 的升级模式常见有以下两种：

1. bootloader 分区 + app1 分区 + app2 分区模式

    该模式下，bootloader 启动后，检查 app1 和 app2 分区，哪个固件版本最新就运行哪个分区的固件。当有新版本的升级固件时，固件下载程序会将新的固件下载到另外的一个没有运行的 app 分区，下次启动的时候重新选择执行新版本的固件。

    优点：无需固件搬运，启动速度快。

    缺点：app1 分区和 app2 分区通常要大小相等，占用 Flash 资源；且 app1 和 app2 分区都只能存放 app 固件，不能存放其他固件（如 WiFi 固件）。

2. bootloader 分区 + app 分区 + download 分区模式

    该模式下，bootloader 启动后，检查 download 分区是否有新版本的固件，如果 download 分区内有新版本固件，则将新版本固件从 download 分区搬运到 app 分区，完成后执行 app 分区内的固件；如果download 分区内没有新版本的固件，则直接执行 app 分区内的固件。

    当有新版本的升级固件时，固件下载程序会将新的固件下载到 download 分区内，重启后进行升级。

    优点：download 分区可以比 app 分区小很多（使用压缩固件），节省 Flash 资源，节省下载流量；download 分区也可以下载其他固件，从而升级其他的固件，如 WiFi 固件、RomFs。
    
    缺点：需要搬运固件，首次升级启动速度略慢。

RT-Thread OTA 使用的是 bootloader 升级模式2，bootloader 分区 + app 分区 + download 分区的组合。

### 固件下载器

在嵌入式设备程序中实现的，具有从远端下载升级固件功能的代码块称之为 **固件下载器（OTA Downloader）**。固件下载器的工作是把新版本的升级固件下载设备的 Flash 中，不同的协议以及不同的 OTA 固件托管平台会有不同的固件下载器，即不同的代码实现。

目前 RT-Thread 已经支持了多种固件下载器，如下所示：

- 基于 Ymodem 协议的 Ymodem OTA 固件下载器（可通过串口等方式升级固件）
- 基于 HTTP 协议的 HTTP OTA 固件下载器（可通过网络方式升级固件）
- 基于特定云平台的 OTA 固件下载器

    1. RT-Cloud OTA 固件下载器（适配 RT-Cloud OTA 功能）

    2. ali-iotkit 固件下载器（适配阿里云 LinkDevelop 平台和 LinkPlatform 平台的 OTA 功能）

    3. Ayla 云 OTA 固件下载器（适配 Ayla 云平台的 OTA 功能）

    4. 其他第三方 OTA 托管平台（其他平台请联系 RT-Thread 团队）

### RT-Thread OTA 介绍

**RT-Thread OTA** 是 RT-Thread 开发的跨 OS、跨芯片平台的固件升级技术，轻松实现对设备端固件的管理、升级与维护。

RT-Thread 提供的 OTA 固件升级技术具有以下优势：

- 固件防篡改 ： 自动检测固件签名，保证固件安全可靠
- 固件加密 ： 支持 AES-256 加密算法，提高固件下载、存储安全性
- 固件压缩 ： 高效压缩算法，降低固件大小，减少 Flash 空间占用，节省传输流量，降低下载时间
- 差分升级 ： 根据版本差异生成差分包，进一步节省 Flash 空间，节省传输流量，加快升级速度
- 断电保护 ： 断电后保护，重启后继续升级
- 智能还原 ： 固件损坏时，自动还原至出厂固件，提升可靠性
- 高度可移植 ： 可跨 OS、跨芯片平台、跨 Flash 型号使用
- 多可用的固件下载器：支持多种协议的 OTA 固件托管平台和物联网云平台

**RT-Thread OTA** 框架图如下所示：

![OTA 框架图](../../docs/figures/22_iot_ota_ymodem/rt_ota_SoftwareFramework.png)

从上面的 OTA 框架图可以发现，Ymodem 在 OTA 流程中充当的是 **OTA Downloader**（固件下载器）的角色，核心的 OTA 业务逻辑在 **RT OTA** 中，也就是封装到了 bootloader 固件里。 OTA 业务逻辑与应用程序解耦，极大简化了 OTA 功能增加的难度。

### OTA 升级流程

在嵌入式系统方案里，要完成一次 OTA 固件远端升级，通常需要以下阶段：

1. 准备升级固件（RT-Thread OTA 使用特定的 rbl 格式固件），并上传 OTA 固件到固件托管服务器
2. 设备端使用固件托管服务器对应的**固件下载器**下载 OTA 升级固件
3. 新版本固件下载完成后，在适当的时候重启进入 bootloader
4. bootloader 对 OTA 固件进行校验、解密和搬运（搬运到 app 分区）
5. 升级成功，执行新版本 app 固件

OTA 升级流程如下图所示：

![OTA 升级流程](../../docs/figures/22_iot_ota_ymodem/RT-Thread_OTA_Process.png)

## Ymodem OTA 例程说明

**Ymodem OTA 升级** 是 RT-Thread OTA 支持的固件下载器中的一种。在嵌入式设备中通常用于通过串口（UART）进行文件传输及 IAP 在线升级，是常用固件升级方式。

本例程介绍如何使用 RT-Thread **Ymodem OTA 固件下载器** 下载固件，完成 OTA 升级。

在该例程中用到的 bootloader 程序以 bin 文件的形式提供，只能用在该 STM32L4 设备平台，文件位于 `/examples/22_iot_ota_ymodem/bin/bootloader.bin`。

该例程中，为了方便用户操作，特提供了 all.bin 供用户使用，all.bin 中已经集成了 Ymodem 固件下载器，用户只需要关心如何使用 app 工程打包 升级固件即可。all.bin 文件位于 `/examples/22_iot_ota_ymodem/bin/all.bin` 中。

## 硬件说明

本例程使用到的硬件资源如下所示：

- UART1 (Tx: PA9; Rx: PA10)
- 片内 FLASH (512KBytes)
- 片外 Nor Flash (16MBytes)

本例程中，**bootloader 程序**和 **app 程序**存放在 STM32L4 MCU 的内部 FLASH 中，**download** 下载区域存放在外部扩展的 **Nor FLASH** 中。

## 分区表

| 分区名称    | 存储位置   | 起始地址 | 分区大小 | 结束地址 | 说明 |
| :-----     | :-----    | :----- | :----- | :----- | :------ |
| bootloader | 片内 FLASH | 0x08000000 | 64K  | 0x08010000 | bootloader 程序存储区 |
| app        | 片内 FLASH | 0x08010000 | 448K | 0x08080000 | app 应用程序存储区 |
| easyflash  | Nor FLASH | 0x00000000 | 512K | 0x00080000 | easyflash 存储区 |
| download   | Nor FLASH | 0x00080000 | 1M   | 0x00180000 | download 下载存储区 |
| wifi_image | Nor FLASH | 0x00180000 | 512K | 0x00200000 | wifi 固件存储区 |
| font | Nor FLASH | 0x00200000 | 7M | 0x00900000 | font 字库分区 |
| filesystem | Nor FLASH | 0x00900000 | 7M  | 0x01000000 | filesystem 文件系统区 |

分区表定义在 **`bootloader 程序`** 中，如果需要修改分区表，则需要修改 bootloader 程序。目前不支持用户自定义 bootloader，如果有商用需求，请联系 **RT-Thread** 获取支持。

## 软件说明

**Ymodem 例程**位于 `/examples/22_iot_ota_ymodem` 目录下，重要文件摘要说明如下所示：

| 文件                      | 说明   |
| :-----                    | :-----    |
| applications              | 应用 |
| applications/main.c  | app 入口 |
| applications/ymodem_update.c | ymodem 应用，实现了 OTA 固件下载业务 |
| bin                       | bootloader 程序 |
| bin/all.bin          | 需要烧录到 0x08000000 地址 |
| ports/fal   | Flash 抽象层软件包（fal）的移植文件 |
| packages/fal              | fal 软件包 |

Ymodem 固件升级流程如下所示：

1. Ymodem 串口终端使用 ymodem 协议发送升级固件
2. APP 使用 Ymodem 协议下载固件到 download 分区
3. bootloader 对 OTA 升级固件进行校验、解密和搬运（搬运到 app 分区）
4. 程序从 bootloader 跳转到 app 分区执行新的固件

### Ymodem 代码说明

Ymodem 升级固件下载程序代码在 `/examples/22_iot_ota_ymodem/applications/ymodem_update.c` 文件中，仅有三个 API 接口，介绍如下：

**update 函数**

```c
void update(uint8_t argc, char **argv);
MSH_CMD_EXPORT_ALIAS(update, ymodem_start, Update user application firmware);
```

`update` 函数调用底层接口 `rym_recv_on_device` 启动 Ymodem 升级程序，并使用 RT-Thread `MSH_CMD_EXPORT_ALIAS` API 函数将其导出为 **`ymodem_start`** MSH 命令。

固件下载完成后，通过底层接口 `rym_recv_on_device` 获取下载状态，下载成功则重启系统，进行 OTA 升级。

**ymodem_on_begin 函数**

```c
static enum rym_code ymodem_on_begin(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len);
```

这是一个回调函数，通过底层接口 `rym_recv_on_device` 注册，在 Ymodem 程序启动后，获取到通过 Ymodem 协议发送给设备的文件后执行。主要是获取到文件大小信息，为文件存储做准备，完成相应的初始化工作（如 FAL download 分区擦除，为固件写入做准备）。

**ymodem_on_data 函数**

```c
static enum rym_code ymodem_on_data(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len);
```

这是一个数据处理回调函数，通过底层接口 `rym_recv_on_device` 注册，在接收到通过 Ymodem 协议发送给设备的数据后，执行该回调函数处理数据（这里将接收到的数据写入到 download 分区）。

## 运行

为了方便用户操作，本例程预先提供了带 Ymodem OTA 功能的 all.bin 固件，all.bin 固件包含了 bootloader 程序和 v1.0.0 版本的 app 程序。本例程先下载 all.bin 固件，然后演示使用 Ymodem OTA 功能烧录 v2.0.0 版本的 app 程序。

### 烧录 all.bin

**ST-LINK Utility 烧录**

1. 解压 `/tools/ST-LINK Utility.rar` 到当前目录（解压后有 **/tools/ST-LINK Utility** 目录）
2. 打开 **/tools/ST-LINK Utility** 目录下的 **STM32 ST-LINK Utility.exe** 软件
3. 点击菜单栏的 **Target** --> **Connect** 连接到开发板，如下图所示：

    ![连接设备](../../docs/figures/22_iot_ota_ymodem/stlink_utility_connect.png)

4. 打开 **/examples/22_iot_ota_ymodem/bin/all.bin** 文件

    ![打开文件](../../docs/figures/22_iot_ota_ymodem/stlink_utility_openfile.png)

5. 烧录

    ![烧录](../../docs/figures/22_iot_ota_ymodem/stlink_utility_download.png)

    ![开始烧录](../../docs/figures/22_iot_ota_ymodem/stlink_utility_download_start.png)

### all.bin 运行效果

烧录完成后，此时可以在 PC 端使用终端工具打开开发板的 ST-Link 提供的虚拟串口，设置串口波特率为 115200，数据位 8 位，停止位 1 位，无流控，开发板的运行日志信息即可实时输出出来。

烧录后程序会自动运行（或按下复位按键重启开发板查看日志），设备打印日志如下图所示：

![all bin 程序运行效果](../../docs/figures/22_iot_ota_ymodem/download_allbin.png)

从以上日志里可以看到前半部分是 **bootloader 固件**打印的日志，后半部分是 **app 固件**打印的日志，输出了 app 固件的版本号，并成功进入了 RT-Thread MSH 命令行。

串口打印的日志上，对 `download` 分区校验失败，这是因为设备初次烧录固件，位于片外 Flash 的 download 分区内没有数据，所以会校验失败。

### 制作升级固件

以 **22_iot_ota_ymodem** 例程为基础，制作用于 Ymodem 升级演示所用到的 **app 固件**。

1. **MDK**：双击 `project.uvprojx` 打开 MDK5 工程
2. **IAR**：双击 `project.eww` 打开 IAR 工程
3. 修改 `/examples/22_iot_ota_ymodem/application/main.c` 中的版本号 `#define APP_VERSION  "1.0.0"` 为 `#define APP_VERSION  "2.0.0"`
4. 编译得到 `rt-thread.bin`，文件位置 `/examples/22_iot_ota_ymodem/rt-thread.bin`

编译器编译出来的应用程序 `rt-thread.bin` 属于原始 app 固件，并不能用于 RT-Thread OTA 的升级固件，需要用户使用 `RT-Thread OTA 固件打包器` 打包生成 `.rbl` 后缀名的固件，然后才能进行 OTA 升级。

使用 **/tools/ota_packager** 目录下的 OTA 打包工具制作 OTA 升级固件（**`.rbl`** 后缀名的文件）。

`RT-Thread OTA 固件打包器` 如下图所示：

![OTA 打包工具](../../docs/figures/22_iot_ota_ymodem/OTAFirmwarePacker.png)

用户可以根据需要，选择是否对固件进行加密和压缩，提供多种压缩算法和加密算法支持，基本操作步骤如下：

- 选择待打包的固件（`/examples/22_iot_ota_ymodem/rt-thread.bin`）
- 选择生成固件的位置
- 选择压缩算法（不压缩则留空）
- 选择加密算法（不加密则留空）
- 配置加密密钥（不加密则留空）
- 配置加密 IV （不加密则留空）
- 填写固件名称（对应分区名称，这里为 app）
- 填写固件版本（填写 `/examples/22_iot_ota_ymodem/application/main.c` 中的版本号 2.0.0）
- 开始打包

通过以上步骤制作完成的 `rt-thread.rbl` 文件即可用于后续的升级文件。

**Note：**

- **加密密钥** 和 **加密 IV** 必须与 bootloader 程序中的一致，否则无法正确加解密固件

    默认提供的 bootloader.bin 支持加密压缩，使用的 **加密密钥** 为 `0123456789ABCDEF0123456789ABCDEF`，使用的 **加密 IV** 为 `0123456789ABCDEF`。

- 固件打包过程中有 **`固件名称`** 的填写，这里注意需要填入 Flash 分区表中对应分区的名称，不能有误

    如果要升级 **app** 程序，则填写 `app`；如果升级 **WiFi 固件**，则填写 `wifi_image`。

- 使用 **OTA 打包工具**制作升级固件 `rt-thread.rbl`

    正确填写固件名称为 **app**，版本号填写 `main.c` 中定义的版本号 `2.0.0`。

### 启动 Ymodem 升级

使用命令 **`ymodem_start`** 启动 Ymodem 升级，这里演示升级 **app 固件**。

- 打开支持 **Ymodem 协议**的串口终端工具（推荐使用 **Xshell**）
- 连接开发板串口，复位开发板，进入 MSH 命令行
- 在设备的命令行里输入 **`ymodem_start`** 命令启动 Ymodem 升级
- 选择升级使用 **Ymodem 协议**发送升级固件（打开你制作的 **rt-thread.rbl** 固件）

    ![选择 Ymodem 协议发送固件](../../docs/figures/22_iot_ota_ymodem/ymodem_send_select.png)

- 设备升级过程

    ![Ymodem OTA 升级演示](../../docs/figures/22_iot_ota_ymodem/ymodem_ota.png)

**Ymodem** 下载固件完成后，会自动重启，并在串口终端打印如下log：

```shell
Download firmware to flash success.
System now will restart...
```

设备重启后，**bootloader** 会对升级固件进行合法性和完整性校验，验证成功后将升级固件从**download** 分区搬运到目标分区（这里是 **app** 分区）。

升级成功后设备状态如下图所示：

![OTA 升级成功](../../docs/figures/22_iot_ota_ymodem/ymodem_ota_success.png)

设备升级完成后会自动运行新的固件，从上图中的日志上可以看到，app 固件已经从 **1.0.0 版本**升级到了 **2.0.0 版本**。

**2.0.0 版本**的固件同样是支持 Ymodem 固件下载功能的，因此可以一直使用 Ymodem 进行 OTA 升级。用户如何需要增加自己的业务代码，可以基于该例程进行修改。

## 注意事项

- 在运行该例程前，请务必先将 **all.bin** 固件烧录到设备
- 必须使用 **.rbl** 格式的升级固件
- 打包 OTA 升级固件时，分区名字必须与分区表中的名字相同（升级 app 固件对应 app 分区），参考分区表章节
- 串口终端工具需要支持 Ymodem 协议，并使用确认使用 Ymodem 协议发送固件
- 串口波特率 115200，无奇偶校验，无流控
- app 固件必须从 0x08010000 地址开始链接，否则应用 bootloader 会跳转到 app 失败

    app 固件存储在 app 分区内，起始地址为 0x08010000，如果用户需要升级其他 app 程序，请确保编译器从 0x08010000 地址链接 app 固件。

    MDK 工程设置如下图所示：

    ![MDK 工程 app 链接地址配置](../../docs/figures/22_iot_ota_ymodem/app_linksctaddr.png)

- app 应用重新设置中断向量（使用 bootloader 的时候需要）

    使用 bootloader 的时候，app 固件从 0x08010000 地址开始链接，因此需要将中断向量重新设置到 0x08010000 地址，程序如下所示：

    ```c
    /* 将中断向量表起始地址重新设置为 app 分区的起始地址 */
    static int ota_app_vtor_reconfig(void)
    {
        #define NVIC_VTOR_MASK   0x3FFFFF80
        #define RT_APP_PART_ADDR 0x08010000
        /* 根据应用设置向量表 */
        SCB->VTOR = RT_APP_PART_ADDR & NVIC_VTOR_MASK;
    
        return 0;
    }
    INIT_BOARD_EXPORT(ota_app_vtor_reconfig); // 使用自动初始化
    ```

## 引用参考

- 《RT-Thread 编程指南 》: docs/RT-Thread 编程指南.pdf
- 《RT-Thread OTA 用户手册》: docs/UM1004-RT-Thread-OTA 用户手册.pdf
