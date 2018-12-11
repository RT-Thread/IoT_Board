/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-11     MurphyZhao   first implementation
 * 2018-09-15     SummerGift   add azure example
 */

#include <rtthread.h>
#include <wlan_mgnt.h>
#include "wifi_config.h"

int main(void)
{
    /* Config the dependencies of the wlan autoconnect function */
    wlan_autoconnect_init();

    /* Enable wlan auto connect function */
    rt_wlan_config_autoreconnect(RT_TRUE);

    return 0;
}
