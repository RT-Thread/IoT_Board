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
#include "iotb_sensor.h"
#include "iotb_event.h"
#include "iotb_workqeue.h"
#include "iotb_lcd_process.h"
#include "iotb_key_process.h"
#include "board.h"
#include "drv_lcd.h"

#define IOTB_MAIN_DEBUG

#define DBG_ENABLE
#define DBG_SECTION_NAME               "IOTB_MAIN"
#ifdef IOTB_MAIN_DEBUG
#define DBG_LEVEL                      DBG_LOG
#else
#define DBG_LEVEL                      DBG_INFO /* DBG_ERROR */
#endif
#define DBG_COLOR
#include <rtdbg.h>

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

int main(void)
{
    iotb_lcd_show_startup_page();

    if (iotb_sensor_sdcard_fs_init() != RT_EOK)
    {
        LOG_E("Init sdcard fs failed!");
    }

    /* Need to init wifi before all others thread */
    if (iotb_sensor_wifi_init() != RT_EOK)
    {
        if (iotb_sdcard_wifi_image_upgrade() != RT_EOK)
        {
            /* use 'ymodem start' cmd to update wifi image */
            LOG_E("sdcard upgrad 'wifi image' failed!");
            LOG_E("Input 'ymodem_start' cmd to try to upgrade!");
            lcd_set_color(BLACK, WHITE);
            lcd_clear(BLACK);
            lcd_show_string(0, 120 - 26 - 26, 24,  "SDCard upgrade wifi");
            lcd_show_string(0, 120 - 26, 24,  " image failed!");
            lcd_show_string(0, 120, 24,  "Input 'ymodem_start'");
            lcd_show_string(0, 120 + 26, 24,  "cmd to upgrade");
            return 0;
        }
    }

    if (iotb_partition_fontlib_check() != RT_EOK)
    {
        if (iotb_sdcard_font_upgrade() == (-RT_EEMPTY))
        {
            lcd_set_color(BLACK, WHITE);
            lcd_clear(BLACK);
            lcd_show_string(0, 100, 24, "No font partition  ");
            lcd_show_string(0, 100 + 26, 24, "Using ST-Utility  ");
            lcd_show_string(0, 100 + 26 + 26, 24, "Flash new bootloader");
            rt_thread_mdelay(2000);
            return 0;
        }
        else if (iotb_sdcard_font_upgrade() == (-RT_ERROR))
        {
            LOG_E("sdcard upgrad 'font library' failed!");
            LOG_E("Input 'ymodem_start' cmd to try to upgrade!");
            lcd_set_color(BLACK, WHITE);
            lcd_clear(BLACK);
            lcd_show_string(0, 120 - 26 - 26, 24, "SDCard upgrade font");
            lcd_show_string(0, 120 - 26, 24, "library failed!");
            lcd_show_string(0, 120, 24, "Input 'ymodem_start'");
            lcd_show_string(0, 120 + 26, 24, "cmd to upgrade");
            rt_thread_mdelay(2000);
            return 0;
        }
    }

    if (iotb_workqueue_start() != RT_EOK)
    {
        return -RT_ERROR;
    }
    iotb_workqueue_dowork(iotb_init, RT_NULL);

    iotb_lcd_start();
    iotb_event_start();
    iotb_key_process_start();

    return 0;
}
