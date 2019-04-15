/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-11     MurphyZhao   first implementation
 */

#include <rtthread.h>
#include <wlan_mgnt.h>
#include "wifi_config.h"

int main(void)
{
    /* 初始化wlan 自动连接 */
    wlan_autoconnect_init();

    /* 使能 wlan 自动连接 */
    rt_wlan_config_autoreconnect(RT_TRUE);

    return 0;
}
