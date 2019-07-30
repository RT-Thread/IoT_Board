/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-16     armink       first implementation
 * 2018-09-04     MurphyZhao   update to http ota example
 */

#include <rtthread.h>
#include <wlan_mgnt.h>
#include "wifi_config.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static struct rt_semaphore net_ready;

extern int mbedtls_client_start(void);

/* wlan ready 回调函数 */
void wlan_ready_handler(int event, struct rt_wlan_buff *buff, void *parameter)
{
    rt_sem_release(&net_ready);
}

/* wlan 断开连接回调函数 */
void wlan_station_disconnect_handler(int event, struct rt_wlan_buff *buff, void *parameter)
{
    LOG_D("disconnect from the network!");
}

int main(void)
{
    int result = RT_EOK;

    /* 初始化 wlan 自动连接功能 */
    wlan_autoconnect_init();

    /* 使能 wlan 自动连接功能 */
    rt_wlan_config_autoreconnect(RT_TRUE);

    /* 创建 'net_ready' 信号量 */
    result = rt_sem_init(&net_ready, "net_ready", 0, RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        return -RT_ERROR;
    }

    /* 注册 wlan 连接网络成功的回调，wlan 连接网络成功后释放 'net_ready' 信号量 */
    rt_wlan_register_event_handler(RT_WLAN_EVT_READY, wlan_ready_handler, RT_NULL);
    /* 注册 wlan 网络断开连接的回调 */
    rt_wlan_register_event_handler(RT_WLAN_EVT_STA_DISCONNECTED, wlan_station_disconnect_handler, RT_NULL);

    /* 等待 wlan 连接网络成功 */
    result = rt_sem_take(&net_ready, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        rt_sem_detach(&net_ready);
        LOG_E("Wait net ready failed!");
        return -RT_ERROR;
    }

    /* 网络连接成功，启动 mbedTLS 客户端 */
    mbedtls_client_start();

    return 0;
}
