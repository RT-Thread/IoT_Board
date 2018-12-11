/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2018-10-10     balanceTWK        the first version
 */

#include <rthw.h>
#include <rtdevice.h>
#include <board.h>
#include "drv_infrared.h"
#include "app_infrared.h"

#define DBG_ENABLE
#define DBG_SECTION_NAME  "app.infrared"
#define DBG_COLOR

#define DBG_LEVEL DBG_INFO

#include <rtdbg.h>

rt_uint32_t buf[INFRARED_BUFF_SIZE];

int app_infrared_init(void)
{
    infrared_init();

    /* set beep pin mode to output */
    rt_pin_mode(PIN_BEEP, PIN_MODE_OUTPUT);
    /* set KEY  pin mode to input */
    rt_pin_mode(PIN_KEY0, PIN_MODE_INPUT);
    rt_pin_mode(PIN_KEY1, PIN_MODE_INPUT);
    rt_pin_mode(PIN_KEY2, PIN_MODE_INPUT);
    /* set RGB  pin mode to input */
    rt_pin_mode(PIN_LED_R, PIN_MODE_OUTPUT);
    rt_pin_mode(PIN_LED_G, PIN_MODE_OUTPUT);
    rt_pin_mode(PIN_LED_B, PIN_MODE_OUTPUT);

    rt_pin_write(PIN_LED_R, PIN_HIGH);
    rt_pin_write(PIN_LED_G, PIN_HIGH);
    rt_pin_write(PIN_LED_B, PIN_HIGH);

    return 0;
}

void beep_sign(rt_uint8_t ms, rt_uint8_t count)
{
    for (rt_uint8_t i = 0; i < count ; i++)
    {
        rt_pin_write(PIN_BEEP, PIN_HIGH);
        rt_thread_mdelay(ms);
        rt_pin_write(PIN_BEEP, PIN_LOW);
        rt_thread_mdelay(ms);
    }
}
rt_int16_t key_scan(void)
{
    if (rt_pin_read(PIN_KEY0) == PIN_LOW)
    {
        rt_thread_mdelay(50);
        if (rt_pin_read(PIN_KEY0) == PIN_LOW)
        {
            beep_sign(50, 1);
            return PIN_KEY0;
        }
    }
    else if (rt_pin_read(PIN_KEY1) == PIN_LOW)
    {
        rt_thread_mdelay(50);
        if (rt_pin_read(PIN_KEY1) == PIN_LOW)
        {
            beep_sign(50, 1);
            return PIN_KEY1;
        }
    }
    else if (rt_pin_read(PIN_KEY2) == PIN_LOW)
    {
        rt_thread_mdelay(50);
        if (rt_pin_read(PIN_KEY2) == PIN_LOW)
        {
            beep_sign(50, 1);
            return PIN_KEY2;
        }
    }
    return -RT_ERROR;
}
void red_led(rt_uint8_t on)
{
    if (on)
    {
        rt_pin_write(PIN_LED_R, PIN_LOW);
    }
    else
    {
        rt_pin_write(PIN_LED_R, PIN_HIGH);
    }
}
void green_led(rt_uint8_t on)
{
    if (on)
    {
        rt_pin_write(PIN_LED_G, PIN_LOW);
    }
    else
    {
        rt_pin_write(PIN_LED_G, PIN_HIGH);
    }
}
void blue_led(rt_uint8_t on)
{
    if (on)
    {
        rt_pin_write(PIN_LED_B, PIN_LOW);
    }
    else
    {
        rt_pin_write(PIN_LED_B, PIN_HIGH);
    }
}
void rgb_ctrl(rt_uint8_t color)
{
    switch (color)
    {
    case 0:
        red_led(0);
        green_led(0);
        blue_led(0);
        break;
    case 1:
        red_led(1);
        green_led(0);
        blue_led(0);
        break;
    case 2:
        red_led(0);
        green_led(1);
        blue_led(0);
        break;
    case 3:
        red_led(0);
        green_led(0);
        blue_led(1);
        break;
    default:
        LOG_W("err parameter ! Please enter 0-3.");
        break;
    }
}

void app_ir_learn_and_send(void)
{
    volatile rt_size_t size;
    rt_int16_t key;
    rt_kprintf("\n");
    LOG_I("learning mode START.");
    rgb_ctrl(LED_RED);

    size = infrared_receive(buf, 1000, RT_WAITING_FOREVER, 750);
    if (size == 0)
    {
        LOG_W("nothing receive.");
    }

    LOG_I("learning mode STOP.");
    rgb_ctrl(LED_ALL_OFF);

    LOG_I("sending  mode .       | Press KEY2 exit send mode or Press KEY0 send infrared sign.");
    rgb_ctrl(LED_GREEN);
    while (1)
    {
        key = key_scan();
        if (key == PIN_KEY0)
        {
            rgb_ctrl(LED_BLUE);

            if (infrared_send(buf, size) == size)
            {
                LOG_I("send ok.");
            }
            else
            {
                LOG_I("send fail.");
            }
            rgb_ctrl(LED_GREEN);
        }
        else if (key == PIN_KEY2)
        {
            LOG_I("sending  mode EXIT.");
            break;
        }
        rt_thread_mdelay(10);
    }
}
