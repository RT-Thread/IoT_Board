/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-11     Ernest Chen  first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "icm20608.h"

int main(void)
{
    icm20608_device_t dev = RT_NULL;   /* device object */
    const char *i2c_bus_name = "i2c1"; /* i2c bus */
    int count = 0;
    rt_err_t result;

    /* waiting for sensor going into calibration mode */
    rt_thread_mdelay(1000);

    /* initialize icm20608, registered device driver */
    dev = icm20608_init(i2c_bus_name);
    if (dev == RT_NULL)
    {
        rt_kprintf("The sensor initializes failure\n");

        return 0;
    }
    else
    {
        rt_kprintf("The sensor initializes success\n");
    }

    /* calibrate icm20608 zero level and average 10 times with sampling data */
    result = icm20608_calib_level(dev, 10);
    if (result == RT_EOK)
    {
        rt_kprintf("The sensor calibrates success\n");
        rt_kprintf("accel_offset: X%6d  Y%6d  Z%6d\n", dev->accel_offset.x, dev->accel_offset.y, dev->accel_offset.z);
        rt_kprintf("gyro_offset : X%6d  Y%6d  Z%6d\n", dev->gyro_offset.x, dev->gyro_offset.y, dev->gyro_offset.z);
    }
    else
    {
        rt_kprintf("The sensor calibrates failure\n");
        icm20608_deinit(dev);

        return 0;
    }

    while (count++ < 100)
    {
        rt_int16_t accel_x, accel_y, accel_z;
        rt_int16_t gyros_x, gyros_y, gyros_z;

        /* get 3 accelerometer data */
        result = icm20608_get_accel(dev, &accel_x, &accel_y, &accel_z);
        if (result == RT_EOK)
        {
            rt_kprintf("current accelerometer: accel_x%6d, accel_y%6d, accel_z%6d\n", accel_x, accel_y, accel_z);
        }
        else
        {
            rt_kprintf("The sensor does not work\n");
            break;
        }

        /* get 3 gyroscope data */
        result = icm20608_get_gyro(dev, &gyros_x, &gyros_y, &gyros_z);
        if (result == RT_EOK)
        {
            rt_kprintf("current gyroscope    : gyros_x%6d, gyros_y%6d, gyros_z%6d\n\n", gyros_x, gyros_y, gyros_z);
        }
        else
        {
            rt_kprintf("The sensor does not work\n");
            break;
        }
        rt_thread_mdelay(1000);
    }

    return 0;
}
