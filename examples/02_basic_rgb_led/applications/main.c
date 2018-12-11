/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-22     balanceTWK   first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

void red_led(rt_uint8_t on)
{
    if (on)
    {
        rt_pin_write(PIN_LED_R, PIN_LOW);
        rt_kprintf("red led [ON ] | ");
    }
    else
    {
        rt_pin_write(PIN_LED_R, PIN_HIGH);
        rt_kprintf("red led [OFF] | ");
    }
}
void green_led(rt_uint8_t on)
{
    if (on)
    {
        rt_pin_write(PIN_LED_G, PIN_LOW);
        rt_kprintf("green led [ON ] | ");
    }
    else
    {
        rt_pin_write(PIN_LED_G, PIN_HIGH);
        rt_kprintf("green led [OFF] | ");
    }
}
void blue_led(rt_uint8_t on)
{
    if (on)
    {
        rt_pin_write(PIN_LED_B, PIN_LOW);
        rt_kprintf("blue led [ON ] \n ");
    }
    else
    {
        rt_pin_write(PIN_LED_B, PIN_HIGH);
        rt_kprintf("blue led [OFF] \n ");
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
    case 4:
        red_led(1);
        green_led(1);
        blue_led(0);
        break;
    case 5:
        red_led(0);
        green_led(1);
        blue_led(1);
        break;
    case 6:
        red_led(1);
        green_led(0);
        blue_led(1);
        break;
    case 7:
        red_led(1);
        green_led(1);
        blue_led(1);
        break;
    default:
        rt_kprintf("err parameter ! Please enter 0-7. \n");
        break;
    }
}

int main(void)
{
    unsigned int count = 1;
    /* set RGB_LED pin mode to output */
    rt_pin_mode(PIN_LED_R, PIN_MODE_OUTPUT);
    rt_pin_mode(PIN_LED_G, PIN_MODE_OUTPUT);
    rt_pin_mode(PIN_LED_B, PIN_MODE_OUTPUT);

    while (count > 0)
    {
        rt_kprintf("group: %d | ", (count % 8));
        rgb_ctrl(count % 8);
        rt_thread_mdelay(500);
        count++;
    }
    return 0;
}

