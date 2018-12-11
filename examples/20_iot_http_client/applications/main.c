/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-04     ChenYong     first implementation
 */

#include <rtthread.h>
#include <wlan_mgnt.h>
#include <wifi_config.h>
#include <webclient.h>

#define HTTP_GET_URL          "http://www.rt-thread.com/service/rt-thread.txt"
#define HTTP_POST_URL         "http://www.rt-thread.com/service/echo"

static struct rt_semaphore net_ready;
const char *post_data = "RT-Thread is an open source IoT operating system from China!";

extern void wlan_ready_handler(int event, struct rt_wlan_buff *buff, void *parameter);
extern void wlan_station_disconnect_handler(int event, struct rt_wlan_buff *buff, void *parameter);
extern int webclient_get_data(void);
extern int webclient_post_data(void);

int main(void)
{
    int result = RT_EOK;

    /* Create 'net_ready' semaphore */
    result = rt_sem_init(&net_ready, "net_ready", 0, RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        return -RT_ERROR;
    }

    /* Register wlan event callback */
    rt_wlan_register_event_handler(RT_WLAN_EVT_READY, wlan_ready_handler, RT_NULL);
    rt_wlan_register_event_handler(RT_WLAN_EVT_STA_DISCONNECTED, wlan_station_disconnect_handler, RT_NULL);

    /* Config the dependencies of the wlan autoconnect function */
    wlan_autoconnect_init();

    /* Enable wlan auto connect function */
    rt_wlan_config_autoreconnect(RT_TRUE);
    
    /* wait ip up */
    result = rt_sem_take(&net_ready, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        rt_kprintf("Wait net ready failed!\n");
        return -RT_ERROR;
    }

    /* HTTP GET request send */
    webclient_get_data();
    /* HTTP POST request send */
    webclient_post_data();

    return 0;
}

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

/* HTTP client download data by GET request */
int webclient_get_data(void)
{
    unsigned char *buffer = RT_NULL;
    int length = 0;
    
    length = webclient_request(HTTP_GET_URL, RT_NULL, RT_NULL, &buffer);
    if (length < 0)
    {
        rt_kprintf("webclient GET request response data error.\n");
        return -RT_ERROR;
    }

    rt_kprintf("webclient GET request response data :\n");
    rt_kprintf("%s\n", buffer);
    
    web_free(buffer);  
    return RT_EOK;
}

/* HTTP client upload data to server by POST request */
int webclient_post_data(void)
{
    unsigned char *buffer = RT_NULL;
    int length = 0;
    
    length = webclient_request(HTTP_POST_URL, RT_NULL, post_data, &buffer);
    if (length < 0)
    {
        rt_kprintf("webclient POST request response data error.\n");
        return -RT_ERROR;
    }

    rt_kprintf("webclient POST request response data :\n");
    rt_kprintf("%s\n", buffer);
    
    web_free(buffer);  
    return RT_EOK;
}
