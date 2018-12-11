# AP3216C 软件包

## 1 介绍

AP3216C 软件包提供了使用接近感应（ps）与光照强度（als）传感器 `ap3216c` 基本功能，并且提供了硬件中断的可选功能。本文介绍该软件包的基本功能，以及 `Finsh/MSH` 测试命令等。

- **光照强度** ：支持 4 个量程
- **接近感应** ：支持 4 种增益
- **中断触发** ：光照强度及接近感应同时支持 `高于阈值` 或 `低于阈值` 的两种硬件中断触发方式

### 1.1 目录结构

| 名称 | 说明 |
| ---- | ---- |
| ap3216c.h | 传感器使用头文件 |
| ap3216c.c | 传感器使用源代码 |
| SConscript | RT-Thread 默认的构建脚本 |
| README.md | 软件包使用说明 |
| ap3216c_datasheet.pdf | 官方数据手册 |

### 1.2 许可证

AP3216C 软件包遵循  Apache-2.0 许可，详见 LICENSE 文件。

### 1.3 依赖

依赖 RT-Thread `I2C` 设备驱动框架。

## 2 获取软件包

使用 `ap3216c` 软件包需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages
    peripheral libraries and drivers  --->
        [*] ap3216c: a digital ambient light and a proximity sensor ap3216c driver library  --->
        [*]   Enable hardware interrupt
        (47)    The number of the sensor hardware interrupt pin
               Version (latest)  --->
```


每个功能的配置说明如下：

- `ap3216c: a digital ambient light and a proximity sensor ap3216c driver library`：选择使用 `ap3216c` 软件包；
- `Enable hardware interrupt`：使能硬件中断功能；
- `The number of the sensor hardware interrupt pin`：传感器硬件中断的引脚号；
- `Version`：配置软件包版本，默认最新版本。

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3 使用 ap3216c 软件包

按照前文介绍，获取 `ap3216c` 软件包后，就可以按照 下文提供的 API 使用传感器 `ap3216c` 与 `Finsh/MSH` 命令进行测试，详细内容如下。

### 3.1 API

#### 3.1.1  初始化 

`ap3216c_device_t ap3216c_init(const char *i2c_bus_name)`

根据总线名称，自动初始化对应的 `ap3216c` 设备，具体参数与返回说明如下表

| 参数    | 描述                      |
| :----- | :----------------------- |
| name   | i2c 设备名称 |
| **返回** | **描述** |
| != NULL | 将返回 ap3216c 设备对象 |
| = NULL | 查找失败 |

#### 3.1.2  反初始化

`void ap3216c_deinit(ap3216c_device_t dev)`

如果设备不再使用，反初始化将回收 `ap3216c` 设备的相关资源，具体参数说明如下表

| 参数 | 描述           |
| :--- | :------------- |
| dev  | ap3216c 设备对象 |

#### 3.1.3 读取光照强度

`float ap3216c_read_ambient_light(ap3216c_device_t dev)`

通过 `ap3216c` 传感器读取光照强度测量值，返回浮点型光照强度值(单位：lux)，具体参数与返回说明如下表

| 参数     | 描述             |
| :------- | :--------------- |
| dev      | ap3216c 设备对象 |
| **返回** | **描述**         |
| != 0.0   | 测量光照强度值   |
| =0.0     | 测量失败         |


在读取光照强度过程中，默认量程为 AP3216C_ALS_RANGE_20661，如需修改，请参照下文的设置参数 API 进行设置，可配置的量程列表如下


```c
enum als_range
{
    AP3216C_ALS_RANGE_20661, // 量程对应精度为 0.35   lux/count.
    AP3216C_ALS_RANGE_5162,  // 量程对应精度为 0.0788 lux/count.
    AP3216C_ALS_RANGE_1291,  // 量程对应精度为 0.0197 lux/count.
    AP3216C_ALS_RANGE_323,   // 量程对应精度为 0.0049 lux/count.
};
```

#### 3.1.4 读取接近感应

`uint16_t ap3216c_read_humidity(ap3216c_device_t dev)`

通过 `ap3216c` 传感器读取接近感应测量值，返回 `16` 位接近感应值，具体参数与返回说明如下表

| 参数     | 描述            |
| :------- | :----------------|
| dev      | ap3216c 设备对象 |
| **返回** | **描述**         |
| != 0     | 测量接近感应值   |
| =0       | 测量物体不在传感器探测范围内 |

在读取接近感应过程中，计算出接近感应值，默认测量增益为 AP3216C_ALS_GAIN2。如需修改，请参照下文的设置参数 API 进行设置，可配置的增益列表如下

```c
enum als_gain
{
    AP3216C_ALS_GAIN1, //一倍检测增益
    AP3216C_ALS_GAIN2, //二倍检测增益
    AP3216C_ALS_GAIN4, //四倍检测增益
    AP3216C_ALS_GAIN8, //八倍检测增益
};
```


#### 3.1.5 设置参数

`rt_err_t ap3216c_set_param(ap3216c_device_t dev, ap3216c_cmd_t cmd, rt_uint8_t value)`

通过 `ap3216c` 传感器设置参数，具体参数与返回说明如下表

| 参数      | 描述               |
| :-------- | :----------------- |
| dev       | ap3216c 设备对象   |
| cmd       | 将要设置参数的命令 |
| value     | 将要设置命令的值   |
| **返回**  | **描述**           |
| RT_EOK    | 设置参数成功       |
| -RT_ERROR | 设置参数错误       |

通过设置命令，设置不同的命令类型，如果设置与默认相同，则不需要重新设置，具体命令类型与默认如下所示

```
AP3216C_SYSTEM_MODE,          //系统配置(默认 : 0b000)
AP3216C_INT_PARAM,            //中断清除方式(默认 : 0b0)
AP3216C_ALS_RANGE,            //ALS 量程范围(默认 : 0b00)
AP3216C_ALS_PERSIST,          //ALS 中断过滤次数(默认 : 0b0000)
AP3216C_ALS_CALIBRATION,      //ALS 校准(默认 : 0x40)
AP3216C_ALS_LOW_THRESHOLD_L,  //ALS 最小阈值低位字节(默认 : 0x00)
AP3216C_ALS_LOW_THRESHOLD_H,  //ALS 最小阈值高位字节(默认 : 0x00)
AP3216C_ALS_HIGH_THRESHOLD_L, //ALS 最大阈值低位字节(默认 : 0xFF)
AP3216C_ALS_HIGH_THRESHOLD_H, //ALS 最大阈值高位字节(默认 : 0xFF)
AP3216C_PS_INTEGRATED_TIME,   //PS 或者 IR 转换时间(默认 : 0000)
AP3216C_PS_AGAIN,             //PS 检测增益 (默认 : 0b01)
AP3216C_PS_PERSIST,           //PS 中断过滤次数(默认 : 0b01)
AP3216C_PS_LED_CONTROL,       //IR LED 发送时间(默认 : 0b01)
AP3216C_PS_LED_DRIVER_RATIO,  //IR LED 驱动电流比率(默认 : 0b11)
AP3216C_PS_INT_MODE,          //PS 中断模式(默认 : 0x01)
AP3216C_PS_MEAN_TIME,         //PS 平均采集时间(默认 : 0x00)
AP3216C_PS_WAITING_TIME,      //PS 采集等待实际(默认 : 0x00)
AP3216C_PS_CALIBRATION_L,     //PS 校准阈值低位字节(默认 : 0x00)
AP3216C_PS_CALIBRATION_H,     //PS 校准阈值高位字节(默认 : 0x00)
AP3216C_PS_LOW_THRESHOLD_L,   //PS 最小阈值低位字节(默认 :0x00)
AP3216C_PS_LOW_THRESHOLD_H,   //PS 最小阈值高位字节(默认 :0x00)
AP3216C_PS_HIGH_THRESHOLD_L,  //PS 最大阈值低位字节(默认 :0xff)
AP3216C_PS_HIGH_THRESHOLD_H,  //PS 最大阈值高位字节(默认 :0xff)
```

例如 ，需要设置 ALS 量程为 AP3216C_ALS_RANGE_20661，使用命令如下

```c
ap3216c_set_param(dev, AP3216C_ALS_RANGE, AP3216C_ALS_RANGE_20661);  //dev 为设备对象
```

#### 3.1.6 获取参数

`rt_err_t ap3216c_get_param(ap3216c_device_t dev, ap3216c_cmd_t cmd, rt_uint8_t *value)`

通过 `ap3216c` 传感器获取设置参数，具体参数与返回说明如下表

| 参数      | 描述               |
| :-------- | :----------------- |
| dev       | ap3216c 设备对象   |
| cmd       | 将要获取参数的命令 |
| value     | 将要获取命令的参数 |
| **返回**  | **描述**           |
| RT_EOK    | 获取参数成功       |
| -RT_ERROR | 获取参数错误       |

#### 3.1.7 设置 **als** 硬件中断回调函数

`void ap3216c_int_als_cb(ap3216c_device_t dev, rt_bool_t enabled, ap3216c_threshold_t threshold, ap3216c_int_cb int_cb)`

该功能为可选功能，支持使用硬件中断功能，配置相应的中断阈值与中断噪音过滤次数，具体参数与返回说明如下表

| 参数      | 描述                 |
| :-------- | :------------------- |
| dev       | ap3216c 设备对象     |
| enabled   | 是否使能注册中断回调 |
| threshold | 中断阈值与过滤次数   |
| int_cb    | 中断回调函数         |

表中 `threshold` 为中断阈值与中断噪音过滤次数结构体，内容如下

```c
struct ap3216c_threshold
{
    rt_uint16_t min;        /* als 16 bits, ps 10 bits available(0-1 bit and 8-15 bit ) */
    rt_uint16_t max;        /* als 16 bits, ps 10 bits available(0-1 bit and 8-15 bit ) */
    rt_uint8_t noises_time; /* filter special noises trigger interrupt */
}
```

- 注：
- 最大阈值与最小阈值在 als 中是16位，在 pa 中只有 10 位有效。

表中 `int_cb` 为用户自定义函数，函数结构如下

```c
typedef void (*ap3216c_int_cb)(void *args)
```

#### 3.1.8 设置 **ps** 硬件中断回调函数

`void ap3216c_int_ps_cb(ap3216c_device_t dev, rt_bool_t enabled, ap3216c_threshold_t threshold, ap3216c_int_cb int_cb)`

同设置 *als* 硬件中断回调函数功能一样，为可选功能，支持使用硬件中断功能，配置相应的中断阈值与中断噪音过滤次数，具体参数与返回说明如下表

| 参数      | 描述                 |
| :-------- | :------------------- |
| dev       | ap3216c 设备对象     |
| enabled   | 是否使能注册中断回调 |
| threshold | 中断阈值与过滤次数   |
| int_cb    | 中断回调函数         |

### 3.2 Finsh/MSH 测试命令

ap3216c 软件包提供了丰富的测试命令，项目只要在 RT-Thread 上开启 Finsh/MSH 功能即可。在做一些基于 `ap3216c` 的应用开发、调试时，这些命令会非常实用，它可以准确的读取指传感器测量的光照强度与接近感应。具体功能可以输入 `ap3216c` ，可以查看完整的命令列表

```
msh />ap3216c
Usage:
ap3216c probe <dev_name>   - probe sensor by given name
ap3216c read               - read sensor ap3216c data
msh />
```

#### 3.2.1 在指定的 i2c 总线上探测传感器 

当第一次使用 `ap3216c` 命令时，直接输入 `ap3216c probe <dev_name>` ，其中 `<dev_name>` 为指定的 `i2c` 总线，例如：`i2c1`。如果有这个传感器，就不会提示错误；如果总线上没有这个传感器，将会显示提示找不到相关设备，日志如下：

```
msh />ap3216c probe i2c1      #探测成功，没有错误日志
msh />
msh />ap3216c probe i2c88     #探测失败，提示对应的 I2C 设备找不到
[E/ap3216c] can't find ap3216c device on 'i2c88'
msh />
```

#### 3.2.2 读取数据

探测成功之后，输入 `ap3216c read` 即可获取光照强度与接近感应检测值，包括提示信息，日志如下： 

```
msh />ap3216c read            #接近没有探测到物体
object is not proximity of sensor
ap3216c measure current brightness: 25.9 (lux)  
msh /> 
msh />ap3216c read            #接近有物体
ap3216c read current ps data      : 219
ap3216c measure current brightness: 1.0(lux) 
msh /> 
```

## 4 注意事项

暂无。

## 5 联系方式

* 维护：[Ernest](https://github.com/ErnestChen1)
* 主页：https://github.com/RT-Thread-packages/ap3216c
