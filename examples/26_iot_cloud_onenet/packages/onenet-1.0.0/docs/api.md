# OneNET API

## 初始化

### OneNET 初始化

> int onenet_mqtt_init(void);

OneNET 初始化函数，需要在使用 OneNET 功能前调用。

| **参数**  | **描述**  |
| :-----   | :-----    |
|无         | 无       |
| **返回**  | **描述**  |
|0         | 成功                |
|-1        | 获得设备信息失败      |
|-2        | mqtt 客户端初始化失败 |

### 设置命令响应函数

> void onenet_set_cmd_rsp_cb(void(*cmd_rsp_cb)(uint8_t *recv_data, size_t recv_size, uint8_t **resp_data, size_t *resp_size));

设置命令响应回调函数。

| **参数**    | **描述**    |
| :-----	 | :-----  	   |
|recv_data   | 接收到的数据  |
|recv_size   | 数据的长度    |
|resp_data   | 响应数据      |
|resp_size   | 响应数据的长度 |
| **返回**    | **描述**     |
|无           | 无          |

## 数据上传

### mqtt 上传数据到指定主题

> rt_err_t onenet_mqtt_publish(const char *topic, const uint8_t *msg, size_t len);

利用 mqtt 向指定 topic 发送消息。

| **参数**  | **描述**    	|
| :-----   | :-----  	  |
|topic     | 主题       	|
|msg       | 要上传的数据   |
|len       | 数据长度      |
| **返回**  | **描述**     |
|0         | 上传成功 	   |
|-1        | 上传失败 	   |

### mqtt 上传字符串到 OneNET

> rt_err_t onenet_mqtt_upload_string(const char *ds_name, const char *str);

利用 mqtt 向 OneNET 平台发送字符串数据。

| **参数**   | **描述**    	|
| :-----   	| :-----  	   |
|ds_name    | 数据流名称    |
|str        | 要上传的字符串 |
| **返回**   | **描述**  	|
|0       	| 上传成功 		|
|-5      	| 内存不足 		|

### mqtt 上传数字到 OneNET

> rt_err_t onenet_mqtt_upload_digit(const char *ds_name, const double digit);

利用 mqtt 向 OneNET 平台发送数字数据。

| **参数**   | **描述**    |
| :-----   	| :-----  	  |
|ds_name    | 数据流名称   |
|digit      | 要上传的数字  |
| **返回**   | **描述**    |
|0       	| 上传成功 	   |
|-5      	| 内存不足 	   |

### mqtt 上传二进制文件到 OneNET

> rt_err_t onenet_mqtt_upload_bin(const char *ds_name, const uint8_t *bin, size_t len);

利用 mqtt 向 OneNET 平台发送二进制文件。会动态申请内存来保存二进制文件，使用前请确保有足够的内存。

| **参数**   | **描述**     |
| :-----   	| :-----  	  |
|ds_name    | 数据流名称    |
|bin		| 二进制文件	   |
|len		| 二进制文件大小 |
| **返回**   | **描述**     |
|0       	| 上传成功      |
|-1      	| 上传失败      |

### mqtt 通过路径上传二进制文件到 OneNET

> rt_err_t onenet_mqtt_upload_bin_by_path(const char *ds_name, const char *bin_path);

利用 mqtt 向 OneNET 平台发送二进制文件。

| **参数**   | **描述**     |
| :-----   	| :-----  	  |
|ds_name    | 数据流名称    |
|bin_path   | 二进制文件路径 |
| **返回**   | **描述**     |
|0       	| 上传成功      |
|-1      	| 上传失败      |

### http 上传字符串到 OneNET

> rt_err_t onenet_http_upload_string(const char *ds_name, const char *str);

利用 http 向 OneNET 平台发送字符串数据，不推荐使用，推荐使用mqtt上传。

| **参数**   | **描述**    	  |
| :-----   	| :-----      	|
|ds_name    | 数据流名称   	 |
|str        | 要上传的字符串   |
| **返回**   | **描述**  	  |
|0       	| 上传成功 		  |
|-5      	| 内存不足	 	  |

### http 上传数字到 OneNET

> rt_err_t onenet_http_upload_digit(const char *ds_name, const double digit);

利用 http 向 OneNET 平台发送数字数据，不推荐使用，推荐使用mqtt上传。

| **参数**   | **描述**    |
| :-----   	| :-----  	  |
|ds_name    | 数据流名称   |
|digit      | 要上传的数字  |
| **返回**   | **描述**    |
|0       	| 上传成功     |
|-5      	| 内存不足     |

## 信息获取

###  获取数据流信息

> rt_err_t onenet_http_get_datastream(const char *ds_name, struct rt_onenet_ds_info *datastream);

从 OneNET 平台获取指定数据流信息，并将信息保存在 datastream 结构体中。

| **参数**       	   | **描述**    			|
| :-----   			| :-----               |
|ds_name    		| 数据流名称   			|
|datastream 	| 保存数据流信息的结构体   |
| **返回**   		   | **描述**  			 |
|0       			| 成功 				  |
|-1       			| 获取响应失败 		   |
|-5      			| 内存不足 				|

###  获取最后N个数据点信息

> cJSON *onenet_get_dp_by_limit(char *ds_name, size_t limit);

从 OneNET 平台获取指定数据流的 n 个数据点信息。

| **参数**	| **描述**    	 |
| :-----	 | :-----  	   	    |
|ds_name     | 数据流名称    	|
|limit 		 | 要获取的数据点个数  |
| **返回**    | **描述**  		 |
|cJSON       | 数据点信息 		 |
|RT_NULL     | 失败 			   |

###  获取指定时间内的数据点信息

> cJSON *onenet_get_dp_by_start_end(char *ds_name, uint32_t start, uint32_t end, size_t limit);

从 OneNET 平台获取指定数据流指定时间段内的n个数据点信息。时间参数需要填入Unix时间戳。

| **参数**	| **描述**    	  |
| :-----	 | :-----  			 |
|ds_name     | 数据流名称   		 |
|start     	 | 开始查询的时间   	|
|end     	 | 结束查询的时间   	|
|limit 		 | 要获取的数据点个数   |
| **返回**    | **描述**  		  |
|cJSON       | 数据点信息 	  	  |
|RT_NULL     | 失败 	        	|

###  获取指定时间n秒的数据点信息

> cJSON *onenet_get_dp_by_start_duration(char *ds_name, uint32_t start, size_t duration, size_t limit);

从 OneNET 平台获取指定数据流指定时间后n秒内的n个数据点信息。时间参数需要填入Unix时间戳。

| **参数**	| **描述**    	 |
| :-----	 | :-----  			|
|ds_name     | 数据流名称   	 	 |
|start     	 | 开始查询的时间   	|
|duration    | 要查询的秒数   	|
|limit 		 | 要获取的数据点个数  |
| **返回**    | **描述**  	     |
|cJSON       | 数据点信息        |
|RT_NULL     | 失败   	  	  |

## 设备管理

### 注册设备

> rt_err_t onenet_http_register_device(const char *dev_name, const char *auth_info);

向 OneNET 平台注册设备，并返回设备 id 和 apikey。设备id 和 apikey 会调用 `onenet_port_save_device_info`交由用户处理。

| **参数**	| **描述**|
| :-----	 | :-----  |
|dev_name    | 设备名字 |
|auth_info   | 鉴权信息 |
| **返回**    | **描述**|
|0       	 | 注册成功 |
|-5     	 | 内存不足 |

### 保存设备信息

> rt_err_t onenet_port_save_device_info(char *dev_id, char *api_key); 

保存注册后返回的设备信息，需要用户实现。

| **参数**	| **描述**   |
| :-----	 | :-----     |
|dev_id      | 设备id     |
|api_key     | 设备apikey |
| **返回**    | **描述**   |
|0       	 | 成功       |
|-1          | 失败       |

### 获取设备注册信息

> rt_err_t onenet_port_get_register_info(char *dev_name, char *auth_info);

获取注册设备需要的信息，需要用户实现。

| **参数**	| **描述**   |
| :-----	 | :-----     |
|ds_name     | 指向存放设备名字的指针 |
|auth_info   | 指向存放鉴权信息的指针 |
| **返回**    | **描述**   |
|0       	  | 成功 		|
|-1       	  | 失败 		|

### 获取设备信息

> rt_err_t onenet_port_get_device_info(char *dev_id, char *api_key, char *auth_info);

获取设备信息用于登陆 OneNET 平台，需要用户实现。

| **参数**	| **描述**   |
| :-----	 | :-----     |
|dev_id      | 指向存放设备id的指针 |
|api_key     | 指向存放设备apikey的指针 |
|auth_info   | 指向存放鉴权信息的指针 |
| **返回**    | **描述**   |
|0           | 成功       |
|-1          | 失败       |

### 设备是否注册

> rt_bool_t onenet_port_is_registed(void);

判断设备使用已经注册，需要用户实现。

| **参数**	| **描述**  |
| :-----	 | :-----   |
|无   		 | 无       |
| **返回**    | **描述** |
|RT_TURE     | 已经注册  |
|RT_FALSE    | 未注册    |
