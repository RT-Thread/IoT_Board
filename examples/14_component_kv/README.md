# KV 参数存储例程

## 简介

本例程将演示使用 EasyFlash 存储 KV 参数，记录开机次数。

## 背景知识

KV 是 key-value（键值对）的缩写，保存数据时，都是 key 和 value 一起保存，读取数据时，只要输入 key 值，就可读取到 value 值，十分方便。本例程中的环境变量就是利用 KV 值存储的。

[EasyFlash](https://github.com/armink/EasyFlash) 是一款开源的轻量级嵌入式Flash存储器库，主要为 MCU 提供便捷、通用的上层应用接口，使得开发者更加高效实现基于的Flash存储器常见应用开发。

该库目前提供 **三大实用功能** ：

- **Env** 快速保存产品参数，支持 **写平衡（磨损平衡）** 及 **掉电保护** 模式

    EasyFlash 不仅能够实现对产品的 **设定参数** 或 **运行日志** 等信息的掉电保存功能，还封装了简洁的 **增加、删除、修改及查询** 方法， 降低了开发者对产品参数的处理难度，也保证了产品在后期升级时拥有更好的扩展性。让 Flash 变为 NoSQL（非关系型数据库）模型的小型键值（Key-Value）存储数据库。

- **IAP** 在线升级再也不是难事儿

    该库封装了 IAP（In-Application Programming）功能常用的接口，支持 CRC32 校验，同时支持 Bootloader 及 Application 的升级。

- **Log** 无需文件系统，日志可直接存储在 Flash 上

    非常适合应用在小型的不带文件系统的产品中，方便开发人员快速定位、查找系统发生崩溃或死机的原因。同时配合 [EasyLogger](https://github.com/armink/EasyLogger)（开源的超轻量级、高性能C日志库，它提供与EasyFlash的无缝接口）一起使用，轻松实现 C 日志的 Flash 存储功能。

## 硬件说明

本例程使用到的硬件资源如下所示：

- UART1 (Tx: PA9; Rx: PA10)
- 片内 FLASH (512KBytes)
- 片外 Nor Flash (16MBytes)

## 软件说明

### EasyFlash 配置说明

EasyFlash 配置存放在 `/examples/14_component_kv/packages/EasyFlash-v3.2.1/inc/ef_cfg.h` 文件中，主要包括环境变量功能的配置、在线升级功能的配置和日志功能的配置等。

本例程只演示环境变量功能，配置的是掉电保护模式（在写入环境变量时出现掉电现象，写入前的数据并不会丢失）。同时还开启了环境变量自动更新功能。当版本号变动时，会自动追加新添加的环境变量。 设定 ENV 缓冲区的大小为 2K，擦写的最小粒度为 4K。详细的配置说明见 [官方主页](https://github.com/armink-rtt-pkgs/EasyFlash)。

### EasyFlash 移植说明

EasyFlash 在使用前需要进项移植，不同的底层驱动，移植方法也不一样。本例程的底层驱动是 FAL，对 FAL 还不熟悉的用户可以去学习下第 13 个例程 13_component_fal。

基于 FAL 的移植十分方便，因为官方已经做好了基于 FAL 的移植文件。

移植主要分为3步：

1. 复制移植文件 ef_fal_port.c

    从 /examples/14_component_kv/packages/EasyFlash-v3.2.1/ports 复制到 /examples/14_component_kv/ports/easyflash。

2. 修改 FAL_EF_PART_NAME 宏定义（存储环境变量的分区名）的值

    本例程里存储环境变量的分区名为 `easyflash`。

3. 修改 static const ef_env default_env_set[] 数组里的环境变量。

    本例程只记录开机次数，所以数组里只有 {"boot_times", "0"} 一个环境变量

详细的移植说明见[移植参考示例](https://github.com/armink-rtt-pkgs/EasyFlash/blob/master/ports/README.md)。

### 例程使用说明

在 main 函数中，首先执行的是 FAL 的初始化，完成分区表的加载。接着执行 EasyFlash 的初始化，初始化成功后，会执行读取和写入 KV 值的 demo。EasyFlash 会将 KV 参数存储在 easyflash 分区。

例程启动后，会从 easyflash 分区读取 boot_times（开机次数）参数，读取成功后，将字符串转换成数字，累加后再次存储到 easyflash 分区 ，并将开机次数通过串口打印出来。

```c
int main(void)
{
    fal_init();

    if (easyflash_init() == EF_NO_ERR) 
    {
        /* 演示环境变量功能 */
        test_env();
    } 
    
    return 0;
}

static void test_env(void)
{
    uint32_t i_boot_times = NULL;
    char *c_old_boot_times, c_new_boot_times[11] = {0};

    /* 获得启动次数的值 */
    c_old_boot_times = ef_get_env("boot_times");
    /* 获取启动次数是否失败 */
    if (c_old_boot_times == RT_NULL)
        c_old_boot_times[0] = '0';

    i_boot_times = atol(c_old_boot_times);
    /* 启动次数加 1 */
    i_boot_times++;
    LOG_D("===============================================");
    LOG_D("The system now boot %d times", i_boot_times);
    LOG_D("===============================================");
    /* 数字转字符串 */
    LOG_D(c_new_boot_times, "%d", i_boot_times);
    /* 保存开机次数的值 */
    ef_set_env("boot_times", c_new_boot_times);
    ef_save_env();
}
```

## 运行

### 编译&下载

- **MDK**：双击 `project.uvprojx` 打开 MDK5 工程，执行编译。
- **IAR**：双击 `project.eww` 打开 IAR 工程，执行编译。

编译完成后，将开发板的 ST-Link USB 口与 PC 机连接，然后将固件下载至开发板。

### 运行效果

开机后开发板会通过串口自动打印开机次数，示例如下：

```shell
 \ | /
- RT -     Thread Operating System
 / | \     4.0.1 build Mar 26 2019
 2006 - 2019 Copyright by rt-thread team
[SFUD] Find a Winbond flash chip. Size is 16777216 bytes.
[SFUD] w25q128 flash device is initialize success.
[I/FAL] RT-Thread Flash Abstraction Layer (V0.2.0) initialize success.
[Flash] EasyFlash V3.2.1 is initialize success.
[Flash] You can get the latest version on https://github.com/armink/EasyFlash .
[D/main] ===============================================
[D/main] The system now boot 5 times
[D/main] ===============================================
msh >
```

按下复位按键，可以看到开机次数加一。

EasyFlash 还提供了 5 个测试命令，可以在 FinSH 里非常方便的查看，修改环境变量。

- **setenv** : 设置环境变量
- **printenv** : 打印环境变量
- **saveenv** : 存储环境变量
- **getvalue** : 获取环境变量值
- **resetenv** : 复位环境变量

下面是这些命令的使用示例。

```shell
msh >printenv 				# 打印所有的环境变量
boot_times=13

mode: power fail safeguard
size: 32/2048 bytes, write bytes 64/8192.
saved count: 15
ver num: 0
msh >getvalue boot_times	# 获取 boot_times 的值
The boot_times value is 13.
msh >setenv boot_times 5	# 设置 boot_times 的值为5
msh >saveenv				# 保存环境变量
[Flash] Erased ENV OK.
[Flash] Saved ENV OK.
msh >getvalue boot_times	# 获取 boot_times 的值
The boot_times value is 5.
msh >resetenv				# 复位环境变量的值
[Flash] Erased ENV OK.
[Flash] Saved ENV OK.
[Flash] Erased ENV OK.
[Flash] Saved ENV OK.
msh >printenv				# 打印所有的环境变量
boot_times=0

mode: power fail safeguard
size: 32/2048 bytes, write bytes 64/8192.
saved count: 2
ver num: 0
msh >setenv boot_times      # 删除 boot_times 环境变量
msh >saveenv				# 保存环境变量
msh >printenv				# 打印所有的环境变量

mode: power fail safeguard
size: 16/2048 bytes, write bytes 32/8192.
saved count: 9
ver num: 0
```

## 引用参考

- 《RT-Thread 编程指南 》: docs/RT-Thread 编程指南.pdf
- 《FAL 软件包介绍》 : https://github.com/RT-Thread-packages/fal