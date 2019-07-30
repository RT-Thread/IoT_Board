/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-10-10     chenyong     the first version
 */

#include <rtthread.h>
#include <wlan_mgnt.h>
#include <wifi_config.h>
#include <board.h>

#include <rt_cld.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define APP_VERSION  "1.0.0"

/* 将中断向量表起始地址重新设置为 app 分区的起始地址 */
static int ota_app_vtor_reconfig(void)
{
    #define NVIC_VTOR_MASK   0x3FFFFF80
    #define RT_APP_PART_ADDR 0x08010000
        /* 根据应用设置向量表 */
    SCB->VTOR = RT_APP_PART_ADDR & NVIC_VTOR_MASK;

    return 0;
}
INIT_BOARD_EXPORT(ota_app_vtor_reconfig);

void wlan_ready_handler(int event, struct rt_wlan_buff *buff, void *parameter)
{
    rt_cld_sdk_init();
}

int main(void)
{
    /* 注册 wlan 回调函数 */
    rt_wlan_register_event_handler(RT_WLAN_EVT_READY, wlan_ready_handler, RT_NULL);
    
    /* 初始化 wlan 自动连接 */
    wlan_autoconnect_init();
    /* 使能wlan 自动连接 */
    rt_wlan_config_autoreconnect(RT_TRUE);
    
    LOG_D("The current version of APP firmware is %s", APP_VERSION);

    return 0;
}
