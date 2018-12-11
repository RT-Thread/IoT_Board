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

#define APP_VERSION  "1.0.0"

/**
 * Function    ota_app_vtor_reconfig
 * Description Set Vector Table base location to the start addr of app(RT_APP_PART_ADDR).
*/
static int ota_app_vtor_reconfig(void)
{
    #define NVIC_VTOR_MASK   0x3FFFFF80
    #define RT_APP_PART_ADDR 0x08010000
    /* Set the Vector Table base location by user application firmware definition */
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
    /* Register wlan event callback */
    rt_wlan_register_event_handler(RT_WLAN_EVT_READY, wlan_ready_handler, RT_NULL);
    
    /* initialize the autoconnect configuration */
    wlan_autoconnect_init();
    /* enable wlan auto connect */
    rt_wlan_config_autoreconnect(RT_TRUE);
    
    rt_kprintf("The current version of APP firmware is %s\n", APP_VERSION);

    return 0;
}
