# OneNET 软件包使用指南

## 准备工作

### OneNET 注册及 ENV 配置

这一部分的内容请阅读软件包中示例说明文档，完成 OneNET 平台的注册和 ENV 的配置。

### OneNET port 接口移植（仅适用于开启自动注册功能）

不启用自动注册不需要移植接口，如需启用自动注册功能，请详细阅读软件包中的移植说明文档，完成移植工作。

## 开始使用

### OneNET 初始化

在 ENV 里面已经配置好了连接云平台需要的各种信息，直接调用`onenet_mqtt_init`函数进行初始化即可，设备会自动连接 OneNET 平台。

### 推送数据

当需要上传数据时，可以按照数据类型选择对应的 API 来上传数据。代码示例如下： 

```{.c}
char str[] = { "hello world" };

/* 获得温度值 */
temp = get_temperature_value();
/* 将温度值上传到 temperature 数据流 */
onenet_mqtt_upload_digit("temperature",temp);

/* 将hello world上传到 string 数据流 */
onenet_mqtt_upload_string("string",str);
```

除了支持上传数字和字符串外，软件包还支持上传二进制文件。

可以通过`onenet_mqtt_upload_bin`或`onenet_mqtt_upload_bin_by_path`来上传二进制文件。代码示例如下

```{.c}
uint8_t buf[] = {0x01, 0x02, 0x03};

/* 将根目录下的1.bin文件上传到 bin 数据流 */
onenet_mqtt_upload_bin_by_path("bin", "/1.bin");
/* 将 buf 中的数据上传到 bin 数据流 */
onenet_mqtt_upload_bin(("bin", buf, 3);
```


### 命令接收

OneNET 支持下发命令，命令是用户自定义的。用户需要自己实现命令响应回调函数，然后利用`onenet_set_cmd_rsp_cb`将回调函数装载上。当设备收到平台下发的命令后，会调用用户实现的命令响应回调函数，等待回调函数执行完成后，将回调函数返回的响应内容再发给云平台。保存响应的内存必须是动态申请出来的，在发送完响应后，程序会自动释放申请的内存。代码示例如下： 

```{.c}
static void onenet_cmd_rsp_cb(uint8_t *recv_data, size_t recv_size,uint8_t **resp_data,
size_t *resp_size)
{
  /* 申请内存 */

  /* 解析命令 */
  
  /* 执行动作 */
  
  /* 返回响应 */
  
}

int main()
{
    /* 用户代码 */
	
    onenet_mqtt_init();

    onenet_set_cmd_rsp_cb(onenet_cmd_rsp_cb);
    
    /* 用户代码 */
   
}
```

### 信息获取

#### 数据流信息获取

用户可以通过`onenet_http_get_datastream`来获取数据流的信息，包括数据流 id，数据流最后更新时间，数据流单位，数据流当前值等等，获取的数据流信息会保存在传入的 datastream 结构体指针所指向的结构体中。代码示例如下

```{.c}
struct rt_onenet_ds_info ds_temp;

/* 获取到 temperature 数据流的信息后保存到 ds_temp 结构体中 */
onenet_http_get_datastream("temperature",ds_temp);
```

#### 数据点信息获取

数据点信息可以通过以下 3 个 API 来获取

> cJSON *onenet_get_dp_by_limit(char *ds_name, size_t limit);
> 
> cJSON *onenet_get_dp_by_start_end(char *ds_name, uint32_t start, uint32_t end, size_t limit);
> 
> cJSON *onenet_get_dp_by_start_duration(char *ds_name, uint32_t start, size_t duration, size_t limit);

这三个 API 返回的都是 cJSON 格式的数据点信息，区别只是查询的方法不一样，下面通过示例来讲解如何使用这 3 个 API。

```{.c}
/* 获取 temperature 数据流的最后10个数据点信息 */
dp = onenet_get_dp_by_limit("temperature",10);

/* 获取 temperature 数据流2018年7月19日14点50分0秒到2018年7月19日14点55分20秒的前10个
数据点信息 */
/* 第二、三个参数是Unix时间戳 */
dp = onenet_get_dp_by_start_end("temperature",1531983000，1531983320，10);

/* 获取 temperature 数据流2018年7月19日14点50分0秒往后50秒内的前10个数据点信息 */
/* 第二个参数是Unix时间戳 */
dp = onenet_get_dp_by_start_end("temperature",1531983000，50，10);
```

## 注意事项

- 设置命令响应回调函数之前必须要先调用`onenet_mqtt_init`函数，在初始化函数里会将回调函数指向RT_NULL。
- 命令响应回调函数里存放响应内容的 buffer 必须是 malloc 出来的，在发送完响应内容后，程序会将这个 buffer 释放掉。

