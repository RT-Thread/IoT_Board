/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-15     Ernest Chen  the first version
 */
 
#ifndef __AHT10_H__
#define __AHT10_H__

#include <rthw.h>
#include <rtthread.h>

#include <rthw.h>
#include <rtdevice.h>

#ifdef AHT10_USING_SOFT_FILTER

typedef struct filter_data
{
    float buf[AHT10_AVERAGE_TIMES];
    float average;

    rt_off_t index;
    rt_bool_t is_full;

} filter_data_t;
#endif /* AHT10_USING_SOFT_FILTER */

struct aht10_device
{
    struct rt_i2c_bus_device *i2c;

#ifdef AHT10_USING_SOFT_FILTER
    filter_data_t temp_filter;
    filter_data_t humi_filter;

    rt_thread_t thread;
    rt_uint32_t period; //sample period
#endif /* AHT10_USING_SOFT_FILTER */

    rt_mutex_t lock;
};
typedef struct aht10_device *aht10_device_t;

/**
 * This function initializes aht10 registered device driver
 *
 * @param dev the name of aht10 device
 *
 * @return the aht10 device.
 */
aht10_device_t aht10_init(const char *i2c_bus_name);

/**
 * This function releases memory and deletes mutex lock
 *
 * @param dev the pointer of device driver structure
 */
void aht10_deinit(aht10_device_t dev);

/**
 * This function reads temperature by aht10 sensor measurement
 *
 * @param dev the pointer of device driver structure
 *
 * @return the relative temperature converted to float data.
 */
float aht10_read_temperature(aht10_device_t dev);

/**
 * This function reads relative humidity by aht10 sensor measurement
 *
 * @param dev the pointer of device driver structure
 *
 * @return the relative humidity converted to float data.
 */
float aht10_read_humidity(aht10_device_t dev);

#endif /* __DRV_AHT10_H__ */
