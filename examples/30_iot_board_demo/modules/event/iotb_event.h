/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-17     MurphyZhao   first implementation
 */

#ifndef __IOTB_EVENT__
#define __IOTB_EVENT__
#include <rtthread.h>

typedef enum
{
    IOTB_EVENT_SRC_NONE,
    IOTB_EVENT_SRC_KEY0,
    IOTB_EVENT_SRC_KEY1,
    IOTB_EVENT_SRC_KEY2,
    IOTB_EVENT_SRC_KEYWKUP,
    IOTB_EVENT_SRC_WIFI,
    IOTB_EVENT_SRC_PM,
    IOTB_EVENT_SRC_MAX
} iotb_event_src_t;

typedef enum
{
    /* key */
    IOTB_EVENT_TYPE_NONE,
    IOTB_EVENT_TYPE_KEY_CLICK,
    IOTB_EVENT_TYPE_KEY_LONG_PRESSED,
    IOTB_EVENT_TYPE_KEY_LONG_PRESSED_UP,

    /* wifi */
    IOTB_EVENT_TYPE_WIFI_UP,
    IOTB_EVENT_TYPE_WIFI_DOWN,
    IOTB_EVENT_TYPE_WIFI_AIRKISS_STARTED,
    IOTB_EVENT_TYPE_WIFI_AIRKISS_FAILED,
    IOTB_EVENT_TYPE_WIFI_AIRKISS_SUCCESS,

    /* PM */
    IOTB_EVENT_TYPE_PM_START,
    IOTB_EVENT_TYPE_PM_WKUP,
    IOTB_EVENT_TYPE_MAX
} iotb_event_type_t;

typedef struct _event
{
    iotb_event_src_t  event_src;
    iotb_event_type_t event_type;
} iotb_event_t;

typedef struct _event_msg
{
    iotb_event_t event;
} iotb_event_msg_t;

#define IOTB_EVENT_THR_CYCLE (100)

rt_err_t iotb_event_start(void);
rt_err_t iotb_event_put(iotb_event_msg_t *msg);
rt_err_t iotb_event_get(iotb_event_msg_t *msg, uint32_t timeout);
void iotb_event_thr_set_cycle(uint16_t time);
void iotb_event_put_set_enable(void);
void iotb_event_put_set_disable(void);

#endif
