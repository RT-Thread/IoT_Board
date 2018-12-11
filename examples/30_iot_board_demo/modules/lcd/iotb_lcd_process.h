/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-17     MurphyZhao   first implementation
 */

#ifndef __IOTB_LCD_PROCESS__
#define __IOTB_LCD_PROCESS__
#include <rtthread.h>
#include <stdint.h>

#define IOTB_LCD_MENU_MAX 14

typedef void (*iotb_lcd_handle)(void*);

typedef enum
{
    IOTB_LCD_CTL_OPEN = 0,
    IOTB_LCD_CTL_SHUTDOWN,
    IOTB_LCD_CTL_POWERSAVE_ENTER,
    IOTB_LCD_CTL_POWERSAVE_EXIT
} iotb_lcd_ctl_t;

typedef enum
{
    IOTB_LCD_CONTENT_NONE = 0,
    IOTB_LCD_STATIC_CONTENT,
    IOTB_LCD_DYNAMIC_CONTENT
} iotb_lcd_content_type_t;

typedef enum
{
    IOTB_LCD_EVENT_NONE                 = (1 << 0),  // 1
    IOTB_LCD_EVENT_ENTER                = (1 << 1),  // 2
    IOTB_LCD_EVENT_ENTER_SUCC           = (1 << 2),  // 4
    IOTB_LCD_EVENT_ENTER_FAIL           = (1 << 3),  // 8
    IOTB_LCD_EVENT_EXIT                 = (1 << 4),  // 1
    IOTB_LCD_EVENT_EXIT_RECEVD          = (1 << 5),  // 2
    IOTB_LCD_EVENT_EXIT_FAIL            = (1 << 6),  // 4
    IOTB_LCD_EVENT_RESET                = (1 << 7),  // 8
    IOTB_LCD_EVENT_KEY2                 = (1 << 8),  // 1
    IOTB_LCD_EVENT_KEY1                 = (1 << 9),  // 2
    IOTB_LCD_EVENT_WKUP                 = (1 << 10), // 4
    IOTB_LCD_EVENT_SMARTCONFIG_START    = (1 << 11), // 8
    IOTB_LCD_EVENT_SMARTCONFIG_STARTED  = (1 << 12), // 8
    IOTB_LCD_EVENT_SMARTCONFIG_FINISH   = (1 << 13), // 1
    IOTB_LCD_EVENT_PM_START             = (1 << 14), //
    IOTB_LCD_EVENT_PM_WKUP              = (1 << 15), //
    IOTB_LCD_EVENT_PREV                 = (1 << 16), // ignore
    IOTB_LCD_EVENT_NEXT                 = (1 << 17), //
    IOTB_LCD_EVENT_START_DEV_INFO_GET   = (1 << 18), //
    IOTB_LCD_EVENT_STOP_DEV_INFO_GET    = (1 << 19), //
    IOTB_LCD_EVENT_MAX                  = (1 << 30)
} iotb_lcd_event_t;

#define GRAY200          0XCE59
#define GRAY220          0XDEFB

typedef struct
{
    uint16_t menu;
    uint16_t refresh_time; /* unit: os tick cnt */
    iotb_lcd_content_type_t content_type;
    iotb_lcd_event_t current_event;
    iotb_lcd_handle lcd_handle;
} iotb_lcd_menu_t;

#define IOTB_LCD_THR_CYCLE (50)

void iotb_lcd_start(void);
void iotb_lcd_show_startup_page(void);
void iotb_lcd_event_put(iotb_lcd_event_t event);
rt_err_t iotb_lcd_event_get(uint32_t set, uint32_t *event, uint8_t clr, uint32_t timeout);
void iotb_lcd_update_menu_index(uint8_t menu_index);
uint8_t iotb_lcd_get_menu_index(void);
void iotb_smartconfig_start(void);
void iotb_smartconfig_stop(void);
void iotb_lcd_thr_set_cycle(uint16_t time);
void iotb_music_finished_cb(void);
#endif
