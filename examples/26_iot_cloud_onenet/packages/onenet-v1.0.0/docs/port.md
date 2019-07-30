# OneNET 包移植说明

本文主要介绍拿到 OneNET 软件包后，需要做的移植工作。

OneNET 软件包已经将硬件平台相关的特性剥离出去，因此 OneNET 本身的移植工作非常少，如果不启用自动注册功能就不需要移植任何接口。

如果启用了自动注册，用户需要新建 **onenet_port.c**，并将文件添加至工程。**onenet_port.c** 主要是实现开启自动注册后，获取注册信息、获取设备信息和保存设备信息等功能。接口定义如下所示： 

```{.c}
/* 检查是否已经注册 */
rt_bool_t onenet_port_is_registed(void);
/* 获取注册信息 */
rt_err_t onenet_port_get_register_info(char *dev_name, char *auth_info);
/* 保存设备信息 */
rt_err_t onenet_port_save_device_info(char *dev_id, char *api_key);
/* 获取设备信息 */
rt_err_t onenet_port_get_device_info(char *dev_id, char *api_key, char *auth_info);
```

## 获取注册信息

> rt_err_t onenet_port_get_register_info(char *ds_name, char *auth_info)

开发者只需要在该接口内，实现注册信息的读取和拷贝即可。

```{.c}
onenet_port_get_register_info(char *dev_name, char *auth_info)
{
    /* 读取或生成设备名字和鉴权信息 */
    
    /* 将设备名字和鉴权信息分别拷贝到 dev_name 和 auth_info 中*/
}
```

## 保存设备信息

> rt_err_t onenet_port_save_device_info(char *dev_id, char *api_key)

开发者只需要在该接口内，将注册返回的设备信息保存在设备里即可。

```{.c}
onenet_port_save_device_info(char *dev_id, char *api_key)
{
    /* 保存返回的 dev_id 和 api_key */
    
    /* 保存设备状态为已注册状态 */
}
```

## 检查是否已经注册

> rt_bool_t onenet_port_is_registed(void)

开发者只需要在该接口内，返回本设备是否已经在 OneNET 平台注册即可。

```{.c}
onenet_port_is_registed(void)
{
    /* 读取并判断设备的注册状态 */
    
    /* 返回设备是否已经注册 */
}
```

## 获取设备信息

> rt_err_t onenet_port_get_device_info(char *dev_id, char *api_key, char *auth_info)

开发者只需要在该接口内，读取并返回设备信息即可

```{.c}
onenet_port_get_device_info(char *dev_id, char *api_key, char *auth_info)
{
    /* 读取设备id，api_key和鉴权信息 */
    
    /* 将设备id，api_key和鉴权信息分别拷贝到 dev_id，api_key 和 auth_info 中*/
}
```