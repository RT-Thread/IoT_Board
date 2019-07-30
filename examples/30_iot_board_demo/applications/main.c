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

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static int ota_app_vtor_reconfig(void)
{
    #define NVIC_VTOR_MASK   0x3FFFFF80
    #define RT_APP_PART_ADDR 0x08010000
    /* 设置应用程序中断向量表地址 */
    SCB->VTOR = RT_APP_PART_ADDR & NVIC_VTOR_MASK;

    return 0;
}
INIT_BOARD_EXPORT(ota_app_vtor_reconfig);

int main(void)
{
    /* 显示启动页 */
    iotb_lcd_show_startup_page();

    /* 在 SD 卡上挂载文件系统 */
    if (iotb_sensor_sdcard_fs_init() != RT_EOK)
    {
        LOG_E("Init sdcard fs failed!");
    }

    /* 初始化 WIFI */
    if (iotb_sensor_wifi_init() != RT_EOK)
    {
        if (iotb_sdcard_wifi_image_upgrade() != RT_EOK)
        {
            /* 使用 'ymodem start' 命令升级 WIFI 固件 */
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

    /* 检测是否存在字库 */
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

    /* 启动工作队列，异步处理耗时任务 */
    if (iotb_workqueue_start() != RT_EOK)
    {
        return -RT_ERROR;
    }
    iotb_workqueue_dowork(iotb_init, RT_NULL);

    /* 启动 LCD 线程，用于接收处理 menu 事件 */
    iotb_lcd_start();
    /* 启动事件处理器 */
    iotb_event_start();
    /* 启动按键处理线程 */
    iotb_key_process_start();

    return 0;
}
