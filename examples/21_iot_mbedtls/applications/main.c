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

static struct rt_semaphore net_ready;

extern int mbedtls_client_start(void);

/**
 * The callback of network ready event
 */
void wlan_ready_handler(int event, struct rt_wlan_buff *buff, void *parameter)
{
    rt_sem_release(&net_ready);
}

/**
 * The callback of wlan disconected event
 */
void wlan_station_disconnect_handler(int event, struct rt_wlan_buff *buff, void *parameter)
{
    rt_kprintf("disconnect from the network!\n");
}

int main(void)
{
    int result = RT_EOK;

    /* Config the dependencies of the wlan autoconnect function */
    wlan_autoconnect_init();

    /* Enable wlan auto connect function */
    rt_wlan_config_autoreconnect(RT_TRUE);

    /* Create 'net_ready' semaphore */
    result = rt_sem_init(&net_ready, "net_ready", 0, RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        return -RT_ERROR;
    }

    /* Register wlan event callback */
    rt_wlan_register_event_handler(RT_WLAN_EVT_READY, wlan_ready_handler, RT_NULL);
    rt_wlan_register_event_handler(RT_WLAN_EVT_STA_DISCONNECTED, wlan_station_disconnect_handler, RT_NULL);

    /* wait ip up */
    result = rt_sem_take(&net_ready, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        rt_kprintf("Wait net ready failed!\n");
        return -RT_ERROR;
    }

    /* startup mbedtls client thread */
    mbedtls_client_start();

    return 0;
}
