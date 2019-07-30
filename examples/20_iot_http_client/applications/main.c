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

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define HTTP_GET_URL "http://www.rt-thread.com/service/rt-thread.txt"
#define HTTP_POST_URL "http://www.rt-thread.com/service/echo"

static struct rt_semaphore net_ready;
const char *post_data = "RT-Thread is an open source IoT operating system from China!";

extern void wlan_ready_handler(int event, struct rt_wlan_buff *buff, void *parameter);
extern void wlan_station_disconnect_handler(int event, struct rt_wlan_buff *buff, void *parameter);
extern int webclient_get_data(void);
extern int webclient_post_data(void);

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
        LOG_E("Wait net ready failed!");
        rt_sem_delete(&net_ready);
        return -RT_ERROR;
    }

    /* HTTP GET 请求发送 */
    webclient_get_data();
    /* HTTP POST 请求发送 */
    webclient_post_data();
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
    LOG_I("disconnect from the network!");
}

/* HTTP client download data by GET request */
int webclient_get_data(void)
{
    unsigned char *buffer = RT_NULL;
    int length = 0;

    length = webclient_request(HTTP_GET_URL, RT_NULL, RT_NULL, &buffer);
    if (length < 0)
    {
        LOG_E("webclient GET request response data error.");
        return -RT_ERROR;
    }

    LOG_D("webclient GET request response data :");
    LOG_D("%s", buffer);

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
        LOG_E("webclient POST request response data error.");
        return -RT_ERROR;
    }

    LOG_D("webclient POST request response data :");
    LOG_D("%s", buffer);

    web_free(buffer);
    return RT_EOK;
}
