# ICM20608 软件包

## 1 介绍

ICM20608 软件包是 RT-Thread 针对六轴传感器 `icm20608` 功能使用的实现，使用这个软件包，可以让该传感器在 RT-Thread 上非常方便使用 `icm20608` 的基本功能，包括读取三轴加速度（3-axis accelerometer）、三轴陀螺仪（3-axis gyroscope）、零值校准等功能。

软件包具有以下优点：

- 支持4种三轴加速度量程
- 支持4种三轴陀螺仪量程
- 支持零值校准

基于该软件包，本文介绍其主要使用方式、API，以及 `Finsh/MSH` 测试命令。

### 1.1 目录结构

| 名称 | 说明 |
| ---- | ---- |
| icm20608.h | 程序头文件               |
| icm20608.c | 程序源代码               |
| SConscript | RT-Thread 默认的构建脚本 |
| README.md | 软件包使用说明 |
| icm20608_datasheet.pdf | 官方数据手册 |

### 1.2 许可证

ICM20608 软件包遵循  Apache-2.0 许可，详见 LICENSE 文件。

### 1.3 依赖

依赖 RT-Thread **I2C** 设备驱动框架。

## 2 获取软件包

使用 `icm20608` 软件包需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages
    peripheral libraries and drivers  --->
        [*] icm20608: a 3-axis gyroscope and a 3-axis accelerometer driver library  --->
              Version (latest)  --->
```


每个功能的配置说明如下：

- `icm20608: a 3-axis gyroscope and a 3-axis accelerometer driver library`：选择使用 `icm20608` 软件包；
- `Version`：配置软件包版本，默认最新版本。

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3 使用 icm20608 软件包

按照前文介绍，获取 `icm20608` 软件包后，就可以按照下文提供的 API 使用传感器 `icm20608` 与 `Finsh/MSH` 命令进行测试，详细内容如下。

### 3.1 API

#### 3.1.1  初始化 

`icm20608_device_t icm20608_init(const char *i2c_bus_name)`

根据总线名称，自动初始化对应的 `icm20608` 设备，具体参数与返回说明如下表

| 参数    | 描述                      |
| :----- | :----------------------- |
| i2c_bus_name | i2c 设备名称 |
| **返回** | **描述** |
| != RT_NULL | 将存储返回 icm20608 设备对象结构体 |
| =RT_ NULL | 查找失败 |

初始化存储返回设备对象结构体如下

```c
struct icm20608_device
{
    struct rt_i2c_bus_device *i2c; // i2c 设备句柄
    cm20608_axes_t accel_offset;   // 三轴加速度零值
    cm20608_axes_t gyro_offset;    // 三轴陀螺仪零值
    rt_mutex_t lock;               // 互斥锁
};
```

三轴加速度零值与三轴陀螺仪零值在使用校准 API 时，用来存储初始误差，作为零值参考值，具体使用参照“校准零值”小节。

#### 3.1.2  反初始化

`void icm20608_deinit(icm20608_device_t dev)`

如果设备不再使用，反初始化将回收 `icm20608` 设备的相关资源，具体参数说明如下表

| 参数 | 描述           |
| :--- | :------------- |
| dev  | icm20608 设备对象 |

#### 3.1.3 读取三轴加速度

`rt_err_t icm20608_get_accel(icm20608_device_t dev, rt_int16_t *accel_x, rt_int16_t *accel_y, rt_int16_t *accel_z)`

通过 `icm20608` 传感器读取三轴加速度测量值，返回带符号的 16 位 ADC 值，具体参数与返回说明如下表

| 参数     | 描述             |
| :------- | :--------------- |
| dev      | icm20608 设备对象 |
| accel_x | 用来保存读取的 x 轴加速度 |
| accel_y | 用来保存读取的 y 轴加速度 |
| accel_z | 用来保存读取的 z 轴加速度 |
| **返回** | **描述**         |
| RT_EOK | 测量三轴加速度成功 |
| -RT_ERROR | 测量失败         |


在读取三轴加速度过程中，默认量程为 ICM20608_ACCELEROMETER_RANGE0，如需修改，请参照下文的设置参数 API 进行设置，可配置的量程列表如下


```c
enum icm20608_accelerometer_range
{
    ICM20608_ACCELEROMETER_RANGE0, // 加速度满量程 ±2g
    ICM20608_ACCELEROMETER_RANGE1, // 加速度满量程 ±4g
    ICM20608_ACCELEROMETER_RANGE2, // 加速度满量程 ±8g
    ICM20608_ACCELEROMETER_RANGE3, // 加速度满量程 ±16g
};
```

#### 3.1.4 读取三轴陀螺仪

`rt_err_t icm20608_get_gyro(icm20608_device_t dev, rt_int16_t *gyro_x, rt_int16_t *gyro_y, rt_int16_t *gyro_z)`

通过 `icm20608` 传感器读取三轴陀螺仪测量值，返回带符号的 16 位 ADC 值，具体参数与返回说明如下表

| 参数     | 描述             |
| :------- | :--------------- |
| dev      | icm20608 设备对象 |
| gyro_x | 用来保存读取的 x 轴陀螺仪 |
| gyro_y | 用来保存读取的 y 轴陀螺仪 |
| gyro_z | 用来保存读取的 z 轴陀螺仪 |
| **返回** | **描述**         |
| RT_EOK | 测量三轴陀螺仪成功 |
| -RT_ERROR | 测量失败         |


在读取三轴陀螺仪过程中，默认量程为 ICM20608_GYROSCOPE_RANGE0，如需修改，请参照下文的设置参数 API 进行设置，可配置的量程列表如下


```c
enum icm20608_gyroscope_range
{
    ICM20608_GYROSCOPE_RANGE0, // 陀螺仪满量程 ±250dps
    ICM20608_GYROSCOPE_RANGE1, // 陀螺仪满量程 ±500dps
    ICM20608_GYROSCOPE_RANGE2, // 陀螺仪满量程 ±1000dps
    ICM20608_GYROSCOPE_RANGE3, // 陀螺仪满量程 ±2000dps
};
```

#### 3.1.5 校准零值

`rt_err_t icm20608_calib_level(icm20608_device_t dev, rt_size_t times)`

可以在使用 `icm20608` 传感器读取测量值的之前，进行零值校准，具体参数与返回说明如下表

| 参数      | 描述                 |
| :-------- | :------------------- |
| dev       | icm20608 设备对象    |
| times     | 校准时读取零值的次数 |
| **返回**  | **描述**             |
| RT_EOK    | 获取零值成功         |
| -RT_ERROR | 获取零值错误         |

值得注意的是在进行零值校准时，`x` 轴、`y` 轴应处于**水平状态**，且传感器处于**静态**，这样读取的零值才是准确的。进行零值校准时，设备对象 `dev` 中三轴加速度零值与三轴陀螺仪零值用来存放初始误差，作为零值。


#### 3.1.6 设置参数

`rt_err_t icm20608_set_param(icm20608_device_t dev, icm20608_cmd_t cmd, rt_uint8_t value)`

通过 `icm20608` 传感器设置参数，具体参数与返回说明如下表

| 参数      | 描述               |
| :-------- | :----------------- |
| dev       | icm20608 设备对象   |
| cmd       | 将要设置参数的命令 |
| value     | 将要设置命令的值   |
| **返回**  | **描述**           |
| RT_EOK    | 设置参数成功       |
| -RT_ERROR | 设置参数错误       |

通过设置命令，设置不同的命令类型，具体命令类型如下所示

```c
enum icm20608_set_cmd
{
    ICM20608_PWR_MGMT1,     // 电源管理 1（设置传感器重启、休眠模式、采集模式与时钟设置等）
    ICM20608_PWR_MGMT2,     // 电源管理 2（使能三轴加速度与三轴陀螺仪）
    ICM20608_GYRO_CONFIG,   // 陀螺仪参数配置（量程配置）
    ICM20608_ACCEL_CONFIG1, // 加速度参数配置1（量程配置）
    ICM20608_ACCEL_CONFIG2, // 加速度参数配置2（平均数过滤配置）
    ICM20608_INT_ENABLE,    // 中断使能
};
```

例如 ，需要设置三轴陀螺仪量程为 ICM20608_GYROSCOPE_RANGE1，即陀螺仪满量程 ±500dps，使用命令为ICM20608_GYRO_CONFIG，使用方式如下

```c
icm20608_set_param(dev, ICM20608_GYRO_CONFIG, ICM20608_GYROSCOPE_RANGE1);  //dev 为设备对象
```

#### 3.1.7 获取参数

`rt_err_t icm20608_get_param(icm20608_device_t dev, icm20608_set_cmd_t cmd, rt_uint8_t *value)`

通过 `icm20608` 传感器获取参数，具体参数与返回说明如下表

| 参数      | 描述               |
| :-------- | :----------------- |
| dev       | icm20608 设备对象   |
| cmd       | 将要获取参数的命令 |
| value     | 保存获取的参数命令值 |
| **返回**  | **描述**           |
| RT_EOK    | 获取参数成功       |
| -RT_ERROR | 获取参数错误       |

获取参数与设置参数类似，命令类型相同。

### 3.2 Finsh/MSH 测试命令

icm20608 软件包提供了丰富的测试命令，项目只要在 RT-Thread 上开启 Finsh/MSH 功能即可。在做一些基于 `icm20608` 的应用开发、调试时，这些命令会非常实用，它可以探测传感器设备、读取三轴加速度与三轴陀螺仪。具体功能可以输入 `icm20608` ，可以查看完整的命令列表

```shell
msh />icm20608
Usage:
icm20608 probe <dev_name>   - probe sensor by given name
icm20608 read [times]       - read sensor icm20608 data
msh />
```

#### 3.2.1 在指定的 i2c 总线上探测传感器 

当第一次使用 `icm20608` 命令时，直接输入 `icm20608 probe <dev_name>` ，其中 `<dev_name>` 为指定的 `i2c` 总线的名称，例如：`i2c1`。如果有这个传感器，就不会提示错误；如果总线上没有这个传感器，将会显示提示找不到相关设备，日志如下：

```shell
msh />icm20608 probe i2c1      #探测成功，没有错误日志
msh />
msh />icm20608 probe i2c88     #探测失败，提示对应的 I2C 设备找不到
[E/icm20608] can't find icm20608 device on 'i2c88'
msh />
```

#### 3.2.2 校准零值

探测成功之后，输入 `icm20608 calib` 命令，进行传感器零值校准。需要传感器在静止状态，并且 `X` 轴、`Y` 轴处于水平，测量此时传感器的值，作为标准零值。读取成功了，就会打印偏移值，日志如下：

```shell
msh />icm20608 calib
accel_offset: X   -14    Y    25    Z  -131  
gyro_offset : X     4    Y    -4    Z     4
```

#### 3.2.3 读取数据

零值校准之后，输入 `icm20608 read [times]` 命令，即可按照读取次数（times）读取三轴加速度与三轴陀螺仪检测值，日志如下： 

```shell
msh />icm20608 read 100         #读取100次
accelerometer: X   -36    Y  -252    Z 17456
gyroscope    : X  -266    Y    33    Z    93
msh /> 
msh />icm20608 read             #没有写次数，读取1一次
accelerometer: X   -36    Y  -250    Z 17466
gyroscope    : X  -226    Y    31    Z    90
msh /> 
msh />icm20608 read -1          #输入小于1的数
[times] should be set bigger than 1

```

## 4 注意事项

暂无。

## 5 联系方式

* 维护：[Ernest](https://github.com/ErnestChen1)
* 主页：https://github.com/RT-Thread-packages/icm20608
