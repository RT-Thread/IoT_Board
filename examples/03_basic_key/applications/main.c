/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-23     balanceTWK   first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

int main(void)
{
    unsigned int count = 1;
    /* set RGB_LED pin mode to output */
    rt_pin_mode(PIN_LED_R, PIN_MODE_OUTPUT);
    /* set KEY0 pin mode to input */
    rt_pin_mode(PIN_KEY0, PIN_MODE_INPUT);

    while (count > 0)
    {
        /* read PIN_KEY0 pin state */
        if (rt_pin_read(PIN_KEY0) == PIN_LOW)
        {
            rt_thread_mdelay(50);
            if (rt_pin_read(PIN_KEY0) == PIN_LOW)
            {
                rt_kprintf("KEY0 pressed!\n");
                rt_pin_write(PIN_LED_R, PIN_LOW);
            }
        }
        else
        {
            rt_pin_write(PIN_LED_R, PIN_HIGH);
        }
        rt_thread_mdelay(10);
        count++;
    }
    return 0;
}
