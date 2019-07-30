/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-16     armink       first implementation
 * 2019-04-22     misonyo      add uart dma code
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define DBG_ENABLE
#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

/* 串口设备名称 */
#define UART_NAME       "uart2"

/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    rt_uint32_t rx_length;
    char rx_buffer[RT_SERIAL_RB_BUFSZ + 1];
    
    /* 从串口读取数据*/
    rx_length = rt_device_read(dev, 0, rx_buffer, size);
    rx_buffer[rx_length] = '\0';

    /* 通过控制台打印数据 */
    rt_kprintf("len:%2d    recv:%s\n",size,rx_buffer);

    return RT_EOK;
}

int main(void)
{
    rt_device_t serial;
    char str[] = "hello RT-Thread!\r\n";

    /* 查找串口设备 */
    serial = rt_device_find(UART_NAME);
    if (!serial)
    {
        LOG_E("find uart device failed!\n");
    }
    else
    {
        /* 以 DMA 接收及轮询发送方式打开串口设备 */
        rt_device_open(serial, RT_DEVICE_FLAG_DMA_RX);
        /* 设置接收回调函数 */
        rt_device_set_rx_indicate(serial, uart_input);
        /* 发送字符串 */
        rt_device_write(serial, 0, str, (sizeof(str) - 1));
    }

    return 0;
}
