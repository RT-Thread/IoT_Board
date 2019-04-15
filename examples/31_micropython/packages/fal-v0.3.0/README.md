# fal：Flash 抽象层

## 1、介绍

FAL (Flash Abstraction Layer) Flash 抽象层，是对 Flash 及基于 Flash 的分区进行管理、操作的抽象层，对上层统一了 Flash 及 分区操作的 API ，并具有以下特性：

- 支持静态可配置的分区表，并可关联多个 Flash 设备；
- 分区表支持 **自动装载** 。避免在多固件项目，分区表被多次定义的问题；
- 代码精简，对操作系统 **无依赖** ，可运行于裸机平台，比如对资源有一定要求的 Bootloader；
- 统一的操作接口。保证了文件系统、OTA、NVM 等对 Flash 有一定依赖的组件，底层 Flash 驱动的可重用性；
- 自带基于 Finsh/MSH 的测试命令，可以通过 Shell 按字节寻址的方式操作（读写擦） Flash 或分区，方便开发者进行调试、测试；

### 1.1 目录结构

| 名称 | 说明 |
| ---- | ---- |
| inc  | 头文件目录 |
| src  | 源代码目录 |
| samples | 例程目录 |

### 1.2 许可证

fal package 遵循 LGPLv2.1 许可，详见 `LICENSE` 文件。

### 1.3 依赖

对 RT-Thread 无依赖，也可用于裸机。

> 测试命令功能需要依赖 RT-Thread Finsh/MSH

## 2、如何打开 fal

使用 fal package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages
    system packages --->
        [*] fal: Flash Abstraction Layer implement. Manage flash device and partition.
        [*]   Enable debug log output
        [ ]   FAL partition table config has defined on 'fal_cfg.h'
        (onchip) The flash device which saving partition table
        (65536) The patition table end address relative to flash device offset.
              version (latest)  --->
```

每个功能的配置说明如下：

- 开启调试日志输出（默认开启）；
- 分区表是否在 `fal_cfg.h` 中定义（默认开启）。如果关闭此选项，fal 将会自动去指定 Flash 的指定位置去检索并装载分区表，具体配置详见下面两个选项；
  - 存放分区表的 Flash 设备；
  - 分区表的 **结束地址** 位于 Flash 设备上的偏移。fal 将从此地址开始往回进行检索分区表，直接读取到 Flash 顶部。如果不确定分区表具体位置，这里也可以配置为 Flash 的结束地址，fal 将会检索整个 Flash ，检索时间可能会增加。

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3、使用 fal

使用 fal 前需要对项目的 Flash 进行移植工作，移植的文档位于： [/samples/porting/README.md](samples/porting/README.md) 。移植完成后，调用 `fal_init() ` 初始化该库。接下来即可调用如下 API：

## 3.1 API

### 3.1.1 查找 Flash 设备 

`const struct fal_flash_dev *fal_flash_device_find(const char *name)`

| 参数    | 描述                      |
| :----- | :----------------------- |
| name   | Flash 设备名称 |
| return | 如果查找成功，将返回 Flash 设备对象，查找失败返回 NULL    |

### 3.1.2 查找 Flash 分区

`const struct fal_partition *fal_partition_find(const char *name)`

| 参数    | 描述                      |
| :----- | :----------------------- |
| name   | Flash 分区名称 |
| return | 如果查找成功，将返回 Flash 分区对象，查找失败返回 NULL    |

### 3.1.3 获取分区表

`const struct fal_partition *fal_get_partition_table(size_t *len)`

| 参数    | 描述                      |
| :----- | :----------------------- |
| len    | 分区表的长度 |
| return | 分区表   |

### 3.1.4 临时设置分区表

FAL 初始化时会自动装载默认分区表。使用该设置将临时修改分区表，重启后会 **丢失** 该设置

`void fal_set_partition_table_temp(struct fal_partition *table, size_t len)`

| 参数    | 描述                      |
| :----- | :----------------------- |
| table  | 分区表 |
| len    | 分区表的长度 |

### 3.1.5 从分区读取数据

`int fal_partition_read(const struct fal_partition *part, uint32_t addr, uint8_t *buf, size_t size)`

| 参数    | 描述                      |
| :----- | :----------------------- |
| part   | 分区对象 |
| addr   | 相对分区的偏移地址 |
| buf    | 存放待读取数据的缓冲区 |
| size   | 待读取数据的大小 |
| return | 返回实际读取的数据大小   |

### 3.1.6 往分区写入数据

`int fal_partition_write(const struct fal_partition *part, uint32_t addr, const uint8_t *buf, size_t size)`

| 参数    | 描述                      |
| :----- | :----------------------- |
| part   | 分区对象 |
| addr   | 相对分区的偏移地址 |
| buf    | 存放待写入数据的缓冲区 |
| size   | 待写入数据的大小 |
| return | 返回实际写入的数据大小   |

### 3.1.7 擦除分区数据

`int fal_partition_erase(const struct fal_partition *part, uint32_t addr, size_t size)`

| 参数    | 描述                      |
| :----- | :----------------------- |
| part   | 分区对象 |
| addr   | 相对分区的偏移地址 |
| size   | 擦除区域的大小 |
| return | 返回实际擦除的区域大小   |

### 3.1.8 擦除整个分区数据

`int fal_partition_erase_all(const struct fal_partition *part)`

| 参数    | 描述                      |
| :----- | :----------------------- |
| part   | 分区对象 |
| return | 返回实际擦除的区域大小   |

### 3.1.9 打印分区表

`void fal_show_part_table(void)`

### 3.1.10 根据分区名称，创建对应的块设备

该函数可以根据指定的分区名称，创建对应的块设备，以便于在指定的分区上挂载文件系统

`struct rt_device *fal_blk_device_create(const char *parition_name)`

| 参数           | 描述                      |
| :-----        | :-----------------------  |
| parition_name | 分区名称 |
| return        | 创建成功，则返回对应的块设备，失败返回空   |

### 3.1.11 根据分区名称，创建对应的 MTD Nor Flash 设备

该函数可以根据指定的分区名称，创建对应的 MTD Nor Flash 设备，以便于在指定的分区上挂载文件系统

`struct rt_device *fal_mtd_nor_device_create(const char *parition_name)`

| 参数          | 描述                                                  |
| :------------ | :---------------------------------------------------- |
| parition_name | 分区名称                                              |
| return        | 创建成功，则返回对应的 MTD Nor Flash 设备，失败返回空 |

### 3.1.12 根据分区名称，创建对应的字符设备

该函数可以根据指定的分区名称，创建对应的字符设备，以便于通过 deivice 接口或 devfs 接口操作分区，开启了 POSIX 后，还可以通过 oepn/read/write 函数操作分区。

`struct rt_device *fal_char_device_create(const char *parition_name)`

| 参数          | 描述                                       |
| :------------ | :----------------------------------------- |
| parition_name | 分区名称                                   |
| return        | 创建成功，则返回对应的字符设备，失败返回空 |

## 3.2 Finsh/MSH 测试命令

fal 提供了丰富的测试命令，项目只要在 RT-Thread 上开启 Finsh/MSH 功能即可。在做一些基于 Flash 的应用开发、调试时，这些命令会非常实用。它可以准确的写入或者读取指定位置的原始 Flash 数据，快速的验证 Flash 驱动的完整性，甚至可以对 Flash 进行性能测试。

具体功能如下：输入 fal 可以看到完整的命令列表

```
msh />fal
Usage:
fal probe [dev_name|part_name]   - probe flash device or partition by given name
fal read addr size               - read 'size' bytes starting at 'addr'
fal write addr data1 ... dataN   - write some bytes 'data' starting at 'addr'
fal erase addr size              - erase 'size' bytes starting at 'addr'
fal bench <blk_size>             - benchmark test with per block size

msh />
```

### 3.2.1 指定待操作的 Flash 设备或 Flash 分区

当第一次使用 fal 命令时，直接输入 `fal probe`  将会显示分区表信息。可以指定待操作的对象为分区表里的某个分区，或者某个 Flash 设备。

分区或者 Flash 被成功选中后，还将会显示它的一些属性情况。大致效果如下：

```
msh />fal probe    
No flash device or partition was probed.
Usage: fal probe [dev_name|part_name]   - probe flash device or partition by given name.
[I/FAL] ==================== FAL partition table ====================
[I/FAL] | name      | flash_dev    |   offset   |    length  |
[I/FAL] -------------------------------------------------------------
[I/FAL] | bl        | stm32_onchip | 0x00000000 | 0x00010000 |
[I/FAL] | app       | stm32_onchip | 0x00010000 | 0x000b0000 |
[I/FAL] | ef        | norflash0    | 0x00000000 | 0x00100000 |
[I/FAL] | download  | norflash0    | 0x00100000 | 0x00100000 |
[I/FAL] =============================================================
msh />
msh />fal probe download
Probed a flash partition | download | flash_dev: norflash0 | offset: 1048576 | len: 1048576 |.
msh />
```

### 3.2.2 擦除数据

先输入 `fal erase` ，后面跟着待擦除数据的起始地址以及长度。以下命令为：从 0 地址（相对 Flash 或分区）开始擦除 4096 字节数据

> 注意：根据 Flash 特性，擦除动作将按扇区对齐进行处理。所以，如果擦除操作地址或长度未按照 Flash 的扇区对齐，将会擦除掉与其关联的整个扇区数据。

```
msh />fal erase 0 4096
Erase data success. Start from 0x00000000, size is 4096.
msh />
```

### 3.2.3 写入数据

先输入 `fal write` ，后面跟着 N 个待写入的数据，并以空格隔开。以下命令为：从地址 8 的位置依次开始写入 1、2、3、4 、 5 这 5 个字节数据

```
msh />fal write 8 1 2 3 4 5
Write data success. Start from 0x00000008, size is 5.
Write data: 1 2 3 4 5 .
msh />
```

### 3.2.4 读取数据

先输入 `fal read` ，后面跟着待读取数据的起始地址以及长度。以下命令为：从 0 地址开始读取 64 字节数据

```
msh />fal read 0 64
Read data success. Start from 0x00000000, size is 64. The data is:
Offset (h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
[00000000] FF FF FF FF FF FF FF FF 01 02 03 04 05 FF FF FF 
[00000010] FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF 
[00000020] FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF 
[00000030] FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF 

msh />
```

### 3.2.5 性能测试

性能测试将会测试 Flash 的擦除、写入及读取速度，同时将会测试写入及读取数据的准确性，保证整个 Flash 或整个分区的 写入与读取 数据的一致性。

先输入 `fal bench` ，后面跟着待测试 Flash 的扇区大小（请查看对应的 Flash 手册，SPI Nor Flash 一般为 4096）。由于性能测试将会让整个 Flash 或者整个分区的数据丢失，所以命令最后必须跟 `yes` 。

```
msh />fal bench 4096 yes
Erasing 1048576 bytes data, waiting...
Erase benchmark success, total time: 2.674S.
Writing 1048576 bytes data, waiting...
Write benchmark success, total time: 7.107S.
Reading 1048576 bytes data, waiting...
Read benchmark success, total time: 2.716S.
msh />
```

## 4、注意事项

暂无

## 5、联系方式

* 维护：[armink](https://github.com/armink)
* 主页：https://github.com/RT-Thread-packages/fal
