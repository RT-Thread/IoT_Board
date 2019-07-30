/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-17     MurphyZhao   first implementation
 */

#include <rtthread.h>
#include <stdint.h>
#include <rtdevice.h>
#include <board.h>

#include "iotb_key_process.h"
#include "iotb_event.h"
#include <rthw.h>

#undef DBG_TAG
#undef DBG_LVL
#undef DBG_COLOR
#undef DBG_ENABLE

// #define IOTB_KEY_DEBUG

#define DBG_ENABLE
#define DBG_TAG               "IOTB_KEY"
#ifdef IOTB_KEY_DEBUG
#define DBG_LVL                      DBG_LOG
#else
#define DBG_LVL                      DBG_INFO /* DBG_ERROR */
#endif 
#define DBG_COLOR
#include <rtdbg.h>

static volatile uint8_t iotb_key_scan_cycle = IOTB_KEY_SCAN_CYCLE;

typedef enum
{
    IOTB_USER_BUTTON_0 = 0,
    IOTB_USER_BUTTON_1,
    IOTB_USER_BUTTON_2,
    IOTB_USER_BUTTON_WK_UP,
    IOTB_USER_BUTTON_MAX
} iotb_user_button_t;

static flex_button_t iotb_user_button[IOTB_USER_BUTTON_MAX];
static iotb_event_msg_t key_msg = {IOTB_EVENT_SRC_NONE, IOTB_EVENT_TYPE_NONE};

static void iotb_btn_0_cb(flex_button_t *btn)
{
    LOG_D("btn_0_cb");

    iotb_event_msg_t key_msg = {IOTB_EVENT_SRC_NONE, IOTB_EVENT_TYPE_NONE};

    switch (btn->event)
    {
        case FLEX_BTN_PRESS_DOWN:
            LOG_D("button[KEY0] is pressed!");
            break;

        case FLEX_BTN_PRESS_DOUBLE_CLICK:
        case FLEX_BTN_PRESS_CLICK:
            key_msg.event.event_src = IOTB_EVENT_SRC_KEY0;
            key_msg.event.event_type = IOTB_EVENT_TYPE_KEY_CLICK;

            if (iotb_event_put(&key_msg) != RT_EOK)
            {
                LOG_E("iotb event put failed!");
            }

            LOG_I("button[KEY0] click event!");
            break;

        case FLEX_BTN_PRESS_LONG_START:
            key_msg.event.event_src = IOTB_EVENT_SRC_KEY0;
            key_msg.event.event_type = IOTB_EVENT_TYPE_KEY_LONG_PRESSED;

            if (iotb_event_put(&key_msg) != RT_EOK)
            {
                LOG_E("iotb event put failed!");
            }

            LOG_D("button[KEY0] long press start event!");
            break;

        case FLEX_BTN_PRESS_LONG_UP:
            key_msg.event.event_src = IOTB_EVENT_SRC_KEY0;
            key_msg.event.event_type = IOTB_EVENT_TYPE_KEY_LONG_PRESSED_UP;

            if (iotb_event_put(&key_msg) != RT_EOK)
            {
                LOG_E("iotb event put failed!");
            }

            LOG_D("button[KEY0] long press up event!");
            break;
    }
}

static void iotb_btn_1_cb(flex_button_t *btn)
{
    LOG_D("btn_1_cb");
    switch (btn->event)
    {
        case FLEX_BTN_PRESS_DOWN:
            LOG_D("button[KEY1] is pressed!");
            break;

        case FLEX_BTN_PRESS_CLICK:
            key_msg.event.event_src = IOTB_EVENT_SRC_KEY1;
            key_msg.event.event_type = IOTB_EVENT_TYPE_KEY_CLICK;

            if (iotb_event_put(&key_msg) != RT_EOK)
            {
                LOG_E("iotb event put failed!");
            }

            LOG_I("button[KEY1] click event!");
            break;

        case FLEX_BTN_PRESS_LONG_START:
            key_msg.event.event_src = IOTB_EVENT_SRC_KEY1;
            key_msg.event.event_type = IOTB_EVENT_TYPE_KEY_LONG_PRESSED;

            if (iotb_event_put(&key_msg) != RT_EOK)
            {
                LOG_E("iotb event put failed!");
            }

            LOG_D("button[KEY1] long press start event!");
            break;

        case FLEX_BTN_PRESS_LONG_UP:
            key_msg.event.event_src = IOTB_EVENT_SRC_KEY1;
            key_msg.event.event_type = IOTB_EVENT_TYPE_KEY_LONG_PRESSED_UP;

            if (iotb_event_put(&key_msg) != RT_EOK)
            {
                LOG_E("iotb event put failed!");
            }

            LOG_D("button[KEY1] long press up event!");
            break;
    }
}

static void iotb_btn_2_cb(flex_button_t *btn)
{
    LOG_D("btn_2_cb");
    switch (btn->event)
    {
        case FLEX_BTN_PRESS_DOWN:
            LOG_D("button[KEY2] is pressed!");
            break;

        case FLEX_BTN_PRESS_DOUBLE_CLICK:
        case FLEX_BTN_PRESS_CLICK:
            key_msg.event.event_src = IOTB_EVENT_SRC_KEY2;
            key_msg.event.event_type = IOTB_EVENT_TYPE_KEY_CLICK;

            if (iotb_event_put(&key_msg) != RT_EOK)
            {
                LOG_E("iotb event put failed!");
            }

            LOG_I("button[KEY2] click event!");
            break;

        case FLEX_BTN_PRESS_LONG_START:
            key_msg.event.event_src = IOTB_EVENT_SRC_KEY2;
            key_msg.event.event_type = IOTB_EVENT_TYPE_KEY_LONG_PRESSED;

            if (iotb_event_put(&key_msg) != RT_EOK)
            {
                LOG_E("iotb event put failed!");
            }

            LOG_D("button[KEY2] long press start event!");
            break;

        case FLEX_BTN_PRESS_LONG_UP:
            key_msg.event.event_src = IOTB_EVENT_SRC_KEY2;
            key_msg.event.event_type = IOTB_EVENT_TYPE_KEY_LONG_PRESSED_UP;

            if (iotb_event_put(&key_msg) != RT_EOK)
            {
                LOG_E("iotb event put failed!");
            }

            LOG_D("button[KEY2] long press up event!");
            break;
    }
}

static void iotb_btn_wkup_cb(flex_button_t *btn)
{
    LOG_D("btn_wkup_cb");
    switch (btn->event)
    {
        case FLEX_BTN_PRESS_DOWN:
            LOG_D("button[WK_UP]] is pressed!");
            break;

        case FLEX_BTN_PRESS_CLICK:
            key_msg.event.event_src = IOTB_EVENT_SRC_KEYWKUP;
            key_msg.event.event_type = IOTB_EVENT_TYPE_KEY_CLICK;

            if (iotb_event_put(&key_msg) != RT_EOK)
            {
                LOG_E("iotb event put failed!");
            }

            LOG_I("button[WK_UP] click event!");
            break;

        case FLEX_BTN_PRESS_LONG_START:
            key_msg.event.event_src = IOTB_EVENT_SRC_KEYWKUP;
            key_msg.event.event_type = IOTB_EVENT_TYPE_KEY_LONG_PRESSED;

            if (iotb_event_put(&key_msg) != RT_EOK)
            {
                LOG_E("iotb event put failed!");
            }

            LOG_D("button[WK_UP] long press start event!");
            break;

        case FLEX_BTN_PRESS_LONG_UP:
            key_msg.event.event_src = IOTB_EVENT_SRC_KEYWKUP;
            key_msg.event.event_type = IOTB_EVENT_TYPE_KEY_LONG_PRESSED_UP;

            if (iotb_event_put(&key_msg) != RT_EOK)
            {
                LOG_E("iotb event put failed!");
            }

            LOG_D("button[WK_UP] long press up event!");
            break;
    }
}

static uint8_t iotb_button_key0_read(void)
{
    return rt_pin_read(PIN_KEY0);
}

static uint8_t iotb_button_key1_read(void)
{
    return rt_pin_read(PIN_KEY1);
}

static uint8_t iotb_button_key2_read(void)
{
    return rt_pin_read(PIN_KEY2);
}

static uint8_t iotb_button_keywkup_read(void)
{
    return rt_pin_read(PIN_WK_UP);
}

void iotb_key_thr_set_cycle(uint16_t time)
{
    rt_base_t level = rt_hw_interrupt_disable();
    iotb_key_scan_cycle = time;
    rt_hw_interrupt_enable(level);
}

static rt_bool_t iotb_button_scan_enable = RT_TRUE;
void iotb_btn_scan_disable(void)
{
    rt_base_t level = rt_hw_interrupt_disable();
    iotb_button_scan_enable = RT_FALSE;
    rt_hw_interrupt_enable(level);
}

void iotb_btn_scan_enable(void)
{
    rt_base_t level = rt_hw_interrupt_disable();
    iotb_button_scan_enable = RT_TRUE;
    rt_hw_interrupt_enable(level);
}

static void iotb_button_scan(void *arg)
{
    while(1)
    {
        if (iotb_button_scan_enable)
        {
            flex_button_scan();
        }
        rt_thread_mdelay(iotb_key_scan_cycle);
    }
}

static void iotb_key_init(void)
{
    uint8_t i;

    rt_memset(&iotb_user_button[0], 0x0, sizeof(iotb_user_button));

    iotb_user_button[IOTB_USER_BUTTON_0].usr_button_read = iotb_button_key0_read;
    iotb_user_button[IOTB_USER_BUTTON_0].cb = (flex_button_response_callback)iotb_btn_0_cb;

    iotb_user_button[IOTB_USER_BUTTON_1].usr_button_read = iotb_button_key1_read;
    iotb_user_button[IOTB_USER_BUTTON_1].cb = (flex_button_response_callback)iotb_btn_1_cb;

    iotb_user_button[IOTB_USER_BUTTON_2].usr_button_read = iotb_button_key2_read;
    iotb_user_button[IOTB_USER_BUTTON_2].cb = (flex_button_response_callback)iotb_btn_2_cb;

    iotb_user_button[IOTB_USER_BUTTON_WK_UP].usr_button_read = iotb_button_keywkup_read;
    iotb_user_button[IOTB_USER_BUTTON_WK_UP].cb = (flex_button_response_callback)iotb_btn_wkup_cb;

    rt_pin_mode(PIN_KEY0, PIN_MODE_INPUT); /* set KEY pin mode to input */
    rt_pin_mode(PIN_KEY1, PIN_MODE_INPUT); /* set KEY pin mode to input */
    rt_pin_mode(PIN_KEY2, PIN_MODE_INPUT); /* set KEY pin mode to input */
    rt_pin_mode(PIN_WK_UP, PIN_MODE_INPUT); /* set KEY pin mode to input */

    for (i = 0; i < IOTB_USER_BUTTON_MAX; i ++)
    {
        iotb_user_button[i].status = 0;
        iotb_user_button[i].pressed_logic_level = 0;
        iotb_user_button[i].click_start_tick = 20;
        iotb_user_button[i].short_press_start_tick = 100;
        iotb_user_button[i].long_press_start_tick = 200;
        iotb_user_button[i].long_hold_start_tick = 300;

        if (i == IOTB_USER_BUTTON_WK_UP)
        {
            iotb_user_button[IOTB_USER_BUTTON_WK_UP].pressed_logic_level = 1;
        }

        flex_button_register(&iotb_user_button[i]);
    }
}

rt_err_t iotb_key_process_start(void)
{
    rt_thread_t iotb_key_tid;

    iotb_key_init();

    iotb_key_tid = rt_thread_create("key_thr",
        iotb_button_scan,
        RT_NULL,
        512, 6, 10);
    if (iotb_key_tid != RT_NULL)
        rt_thread_startup(iotb_key_tid);
    return RT_EOK;
}
