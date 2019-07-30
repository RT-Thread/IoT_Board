# STM32 SDIO驱动简介

这是一个STM32平台 SDIO控制器驱动包

## 支持平台

- STM32L475

- STM32L476

> 其他STM32平台理论上也能支持，目前未作验证

## 下载方法

请使用 ENV 工具辅助下载：

包的路径为：`RT-Thread online packages` -> `peripheral libraries and drivers` -> `STM32 SDIO device`

## 选项介绍

- `SDIO buff size` : 指定缓存BUFF大小，默认值为:4KB
- `SDIO max freq`  : 输出的最大时钟速率，默认值为:24Mhz
- `SDIO allgn`     : 指定缓存BUFF对齐长度，默认值为:32
- `SDIO use 1 bit` : 是否使用1bit通信，默认值：不使用

## 使用介绍

- 1.需要用户自己配置IO，SDIO时钟，DMA等外设。

- 2.需要用户调用 `sdio_host_create` 函数创建一个SDIO驱动, 传递一个结构体指针 `struct stm32_sdio_des`。该结构体内容包括：SDIO外设地址，DMA发送配置（发送数据调用），DMA接收配置（接收数据调用）,获得SDIO控制器时钟。

- 3.需要用户在SDIO中断中调用 `rthw_sdio_irq_process` 函数，该函数需要传递参数 `struct rt_mmcsd_host *` , 该参数可通过 `sdio_host_create` 获得。

## 其他

- 当上层传递的buff大小超过`SDIO buff size`大小，会触发断言

- 目前不支持CPU轮询发送
