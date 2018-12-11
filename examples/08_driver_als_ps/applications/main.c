/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-27     Ernest Chan  first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "ap3216c.h"

int main(void)
{
    ap3216c_device_t dev;              /* device object */
    const char *i2c_bus_name = "i2c1"; /* i2c bus */
    int count = 0;

    rt_thread_mdelay(2000);/* waiting for sensor work */
        
    /* initializes ap3216c, registered device driver */
    dev = ap3216c_init(i2c_bus_name);
    if(dev == RT_NULL)
    {
        rt_kprintf(" The sensor initializes failure");
        return 0;
    }

    while (count++ < 100)
    {
        rt_uint16_t ps_data;
        float brightness;

        /* read ps data */
        ps_data = ap3216c_read_ps_data(dev);
        if (ps_data == 0)
        {
            rt_kprintf("object is not proximity of sensor \n");
        }
        else
        {
            rt_kprintf("current ps data   : %d\n", ps_data);
        }

        /* read als data */
        brightness = ap3216c_read_ambient_light(dev);
        rt_kprintf("current brightness: %d.%d(lux) \n", (int)brightness, ((int)(10 * brightness) % 10));

        rt_thread_mdelay(1000);
    }
    return 0;
}
