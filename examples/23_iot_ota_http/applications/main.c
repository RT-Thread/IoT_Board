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
#include "wifi_config.h"
#include "board.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define APP_VERSION  "2.0.0"

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

int main(void)
{
    wlan_autoconnect_init();

    LOG_D("The current version of APP firmware is %s", APP_VERSION);

    return 0;
}
