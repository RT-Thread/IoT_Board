# AHT10 软件包

## 1 介绍

AHT10 软件包提供了使用温度与湿度传感器 `aht10` 基本功能，并且提供了软件平均数滤波器可选功能。本文介绍该软件包的基本功能，以及 `Finsh/MSH` 测试命令等。

基本功能主要由传感器 `aht10` 决定：在输入电压为 `1.8v-3.3v` 范围内，测量温度与湿度的量程、精度如下表所示

| 功能 | 量程 | 精度 |
| ---- | ---- | ---- |
| 温度 | `-40℃ - 85℃` |`±0.5℃`|
| 湿度 | `0% - 100%` |`±3%`|

### 1.1 目录结构

| 名称 | 说明 |
| ---- | ---- |
| aht10.h | 传感器使用头文件 |
| aht10.c | 传感器使用源代码 |
| SConscript | RT-Thread 默认的构建脚本 |
| README.md | 软件包使用说明 |
| aht10_datasheet.pdf | 官方数据手册 |

### 1.2 许可证

AHT10 软件包遵循  Apache-2.0 许可，详见 LICENSE 文件。

### 1.3 依赖

依赖 `RT-Thread I2C` 设备驱动框架。

## 2 获取软件包

使用 `aht10` 软件包需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages
    peripheral libraries and drivers  --->
        [*] aht10: digital humidity and temperature sensor aht10 driver library  --->
        [*]    Enable support filter function
               Version (latest)  --->
```


每个功能的配置说明如下：

- `aht10: digital humidity and temperature sensor aht10 driver library`：选择使用 `aht10` 软件包；
- `Enable support filter function`：开启采集温湿度软件平均数滤波器功能；
- `Version`：配置软件包版本，默认最新版本。

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3 使用 aht10 软件包

按照前文介绍，获取 `aht10` 软件包后，就可以按照 下文提供的 API 使用传感器 `aht10` 与 `Finsh/MSH` 命令进行测试，详细内容如下。

### 3.1 API

#### 3.1.1  初始化 

`aht10_device_t aht10_init(const char *i2c_bus_name)`

根据总线名称，自动初始化对应的 AHT10 设备，具体参数与返回说明如下表

| 参数    | 描述                      |
| :----- | :----------------------- |
| name   | i2c 设备名称 |
| **返回** | **描述** |
| != NULL | 将返回 aht10 设备对象 |
| = NULL | 查找失败 |

#### 3.1.2  反初始化

void aht10_deinit(aht10_device_t dev)

如果设备不再使用，反初始化将回收 aht10 设备的相关资源，具体参数说明如下表

| 参数 | 描述           |
| :--- | :------------- |
| dev  | aht10 设备对象 |

#### 3.1.3 读取温度

float aht10_read_temperature(aht10_device_t dev)

通过 `aht10` 传感器读取温度测量值，返回浮点型温度值，具体参数与返回说明如下表

| 参数     | 描述           |
| :------- | :------------- |
| dev      | aht10 设备对象 |
| **返回** | **描述**       |
| != 0.0   | 测量温度值     |
| =0.0     | 测量失败       |

#### 3.1.4 读取湿度

float aht10_read_humidity(aht10_device_t dev)

通过 `aht10` 传感器读取湿度测量值，返回浮点型湿度值，具体参数与返回说明如下表

| 参数     | 描述          |
| :------- | :------------ |
| dev      | aht10设备对象 |
| **返回** | **描述**      |
| != 0.0   | 测量湿度值    |
| =0.0     | 测量失败      |

### 3.2 Finsh/MSH 测试命令

aht10 软件包提供了丰富的测试命令，项目只要在 RT-Thread 上开启 Finsh/MSH 功能即可。在做一些基于 `aht10` 的应用开发、调试时，这些命令会非常实用，它可以准确的读取指传感器测量的温度与湿度。具体功能可以输入 `aht10` ，可以查看完整的命令列表

```
msh />aht10
Usage:
aht10 probe <dev_name>   - probe sensor by given name
aht10 read               - read sensor aht10 data
msh />
```

#### 3.2.1 在指定的 i2c 总线上探测传感器 

当第一次使用 `aht10` 命令时，直接输入 `aht10 probe <dev_name>` ，其中 `<dev_name>` 为指定的 i2c 总线，例如：i2c0。如果有这个传感器，就不会提示错误；如果总线上没有这个传感器，将会显示提示找不到相关设备，日志如下：

```
msh />aht10 probe i2c1      #探测成功，没有错误日志
msh />
msh />aht10 probe i2c88     #探测失败，提示对应的 I2C 设备找不到
[E/aht10] can't find aht10 device on 'i2c88'
msh />
```

#### 3.2.2 读取数据

探测成功之后，输入 `aht10 read` 即可获取温度与湿度，包括提示信息，日志如下： 

```
msh />aht10 read
read aht10 sensor humidity   : 54.7 %
read aht10 sensor temperature: 27.3 
msh />
```

## 4 注意事项

暂无。

## 5 联系方式

* 维护：[Ernest](https://github.com/ErnestChen1)
* 主页：https://github.com/RT-Thread-packages/aht10

