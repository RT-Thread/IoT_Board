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
#include "aht10.h"

int main(void)
{
    float humidity, temperature;
    aht10_device_t dev;                /* device object */
    const char *i2c_bus_name = "i2c2"; /* i2c bus station */
    int count = 0;

    rt_thread_mdelay(2000);/* waiting for sensor work */
	
    /* initializes aht10, registered device driver */
    dev = aht10_init(i2c_bus_name);
    if(dev == RT_NULL)
    {
        rt_kprintf(" The sensor initializes failure");
        return 0;
    }

    while (count++ < 100)
    {
        /* read humidity */
        humidity = aht10_read_humidity(dev);
        rt_kprintf("humidity   : %d.%d %%\n", (int)humidity, (int)(humidity * 10) % 10); /* former is integer and behind is decimal */

        /* read temperature */
        temperature = aht10_read_temperature(dev);
        rt_kprintf("temperature: %d.%d \n", (int)temperature, (int)(temperature * 10) % 10); /* former is integer and behind is decimal */

        rt_thread_mdelay(1000);
    }
    return 0;
}
