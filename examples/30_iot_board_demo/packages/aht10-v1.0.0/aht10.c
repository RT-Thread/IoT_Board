/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-15     Ernest Chen  the first version
 */

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME "AHT10"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

#include "aht10.h"

#ifdef PKG_USING_AHT10

#define AHT10_ADDR 0x38 //connect GND

#define AHT10_CALIBRATION_CMD 0xE1 //calibration cmd for measuring
#define AHT10_NORMAL_CMD 0xA8      //normal cmd
#define AHT10_GET_DATA 0xAC        //get data cmd

static rt_err_t write_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t *data)
{
    rt_uint8_t buf[3];

    buf[0] = reg; //cmd
    buf[1] = data[0];
    buf[2] = data[1];

    if (rt_i2c_master_send(bus, AHT10_ADDR, 0, buf, 3) == 3)
        return RT_EOK;
    else
        return -RT_ERROR;
}

static rt_err_t read_regs(struct rt_i2c_bus_device *bus, rt_uint8_t len, rt_uint8_t *buf)
{
    struct rt_i2c_msg msgs;

    msgs.addr = AHT10_ADDR;
    msgs.flags = RT_I2C_RD;
    msgs.buf = buf;
    msgs.len = len;

    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}

static rt_err_t sensor_init(aht10_device_t dev)
{
    rt_uint8_t temp[2] = {0, 0};

    if (write_reg(dev->i2c, AHT10_NORMAL_CMD, temp) != RT_EOK)
    {
        return -RT_ERROR;
    }
    rt_thread_delay(rt_tick_from_millisecond(500)); //at least 300 ms

    temp[0] = 0x08;
    temp[1] = 0x00;
    if (write_reg(dev->i2c, AHT10_CALIBRATION_CMD, temp) != RT_EOK) //go into calibration
    {
        return -RT_ERROR;
    }
    rt_thread_delay(rt_tick_from_millisecond(450));   //at least 300 ms

    return RT_EOK;
}

/*check calibration enable */
static rt_uint8_t calibration_enabled(aht10_device_t dev)
{
    rt_uint8_t val = 0;

    read_regs(dev->i2c, 1, &val);

    if ((val & 0x68) == 0x08)
        return RT_EOK;
    else
        return RT_ERROR;
}

static float read_hw_temperature(aht10_device_t dev)
{
    rt_uint8_t temp[6];
    float cur_temp = -50.0;  //The data is error with missing measurement.  
    rt_err_t result;

    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        rt_uint8_t cmd[2] = {0, 0};
        write_reg(dev->i2c, AHT10_GET_DATA, cmd); // sample data cmd

        result = calibration_enabled(dev);
        if (result != RT_EOK)
        {
            // rt_thread_mdelay(1500);
            sensor_init(dev); // reset sensor
            LOG_E("The aht10 is under an abnormal status. Please try again");
        }
        else
        {
            read_regs(dev->i2c, 6, temp); // get data
            /*sensor temperature converse to reality */
            cur_temp = ((temp[3] & 0xf) << 16 | temp[4] << 8 | temp[5]) * 200.0 / (1 << 20) - 50;
        }
    }
    else
    {
        LOG_E("The aht10 could not respond temperature measurement at this time. Please try again");
    }
    rt_mutex_release(dev->lock);

    return cur_temp;
}

static float read_hw_humidity(aht10_device_t dev)
{
    rt_uint8_t temp[6];
    float cur_humi = 0.0;  //The data is error with missing measurement.  
    rt_err_t result;

    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        rt_uint8_t cmd[2] = {0, 0};
        write_reg(dev->i2c, AHT10_GET_DATA, cmd); // sample data cmd

        result = calibration_enabled(dev);
        if (result != RT_EOK)
        {
            // rt_thread_mdelay(1500);
            sensor_init(dev);
            LOG_E("The aht10 is under an abnormal status. Please try again");
        }
        else
        {
            read_regs(dev->i2c, 6, temp);                                                          // get data
            cur_humi = (temp[1] << 12 | temp[2] << 4 | (temp[3] & 0xf0) >> 4) * 100.0 / (1 << 20); //sensor humidity converse to reality
        }
    }
    else
    {
        LOG_E("The aht10 could not respond temperature measurement at this time. Please try again");
    }
    rt_mutex_release(dev->lock);

    return cur_humi;
}

#ifdef AHT10_USING_SOFT_FILTER

static void average_measurement(aht10_device_t dev, filter_data_t *filter)
{
    rt_uint32_t i;
    float sum = 0;
    rt_uint32_t temp;
    rt_err_t result;

    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        if (filter->is_full)
        {
            temp = AHT10_AVERAGE_TIMES;
        }
        else
        {
            temp = filter->index + 1;
        }

        for (i = 0; i < temp; i++)
        {
            sum += filter->buf[i];
        }
        filter->average = sum / temp;
    }
    else
    {
        LOG_E("The software failed to average at this time. Please try again");
    }
    rt_mutex_release(dev->lock);
}

static void aht10_filter_entry(void *device)
{
    RT_ASSERT(device);

    aht10_device_t dev = (aht10_device_t)device;

    while (1)
    {
        if (dev->temp_filter.index >= AHT10_AVERAGE_TIMES)
        {
            if (dev->temp_filter.is_full != RT_TRUE)
            {
                dev->temp_filter.is_full = RT_TRUE;
            }

            dev->temp_filter.index = 0;
        }
        if (dev->humi_filter.index >= AHT10_AVERAGE_TIMES)
        {
            if (dev->humi_filter.is_full != RT_TRUE)
            {
                dev->humi_filter.is_full = RT_TRUE;
            }

            dev->humi_filter.index = 0;
        }

        dev->temp_filter.buf[dev->temp_filter.index] = read_hw_temperature(dev);
        dev->humi_filter.buf[dev->humi_filter.index] = read_hw_humidity(dev);

        rt_thread_delay(rt_tick_from_millisecond(dev->period));

        dev->temp_filter.index++;
        dev->humi_filter.index++;
    }
}
#endif /* AHT10_USING_SOFT_FILTER */

/**
 * This function reads temperature by aht10 sensor measurement
 *
 * @param dev the pointer of device driver structure
 *
 * @return the relative temperature converted to float data.
 */
float aht10_read_temperature(aht10_device_t dev)
{
#ifdef AHT10_USING_SOFT_FILTER
    average_measurement(dev, &dev->temp_filter);

    return dev->temp_filter.average;
#else
    return read_hw_temperature(dev);
#endif /* AHT10_USING_SOFT_FILTER */
}

/**
 * This function reads relative humidity by aht10 sensor measurement
 *
 * @param dev the pointer of device driver structure
 *
 * @return the relative humidity converted to float data.
 */
float aht10_read_humidity(aht10_device_t dev)
{
#ifdef AHT10_USING_SOFT_FILTER
    average_measurement(dev, &dev->humi_filter);

    return dev->humi_filter.average;
#else
    return read_hw_humidity(dev);
#endif /* AHT10_USING_SOFT_FILTER */
}

/**
 * This function initializes aht10 registered device driver
 *
 * @param dev the name of aht10 device
 *
 * @return the aht10 device.
 */
aht10_device_t aht10_init(const char *i2c_bus_name)
{
    aht10_device_t dev;

    RT_ASSERT(i2c_bus_name);

    dev = rt_calloc(1, sizeof(struct aht10_device));
    if (dev == RT_NULL)
    {
        LOG_E("Can't allocate memory for aht10 device on '%s' ", i2c_bus_name);
        return RT_NULL;
    }

    dev->i2c = rt_i2c_bus_device_find(i2c_bus_name);
    if (dev->i2c == RT_NULL)
    {
        LOG_E("Can't find aht10 device on '%s' ", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

    dev->lock = rt_mutex_create("mutex_aht10", RT_IPC_FLAG_FIFO);
    if (dev->lock == RT_NULL)
    {
        LOG_E("Can't create mutex for aht10 device on '%s' ", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

#ifdef AHT10_USING_SOFT_FILTER
    dev->period = AHT10_SAMPLE_PERIOD;

    dev->thread = rt_thread_create("aht10", aht10_filter_entry, (void *)dev, 1024, 15, 10);
    if (dev->thread != RT_NULL)
    {
        rt_thread_startup(dev->thread);
    }
    else
    {
        LOG_E("Can't start filtering function for aht10 device on '%s' ", i2c_bus_name);
        rt_mutex_delete(dev->lock);
        rt_free(dev);
    }
#endif /* AHT10_USING_SOFT_FILTER */

    if (sensor_init(dev) != RT_EOK)
    {
        if (dev != RT_NULL)
        {
            rt_free(dev);
            rt_mutex_delete(dev->lock);
#ifdef AHT10_USING_SOFT_FILTER
            if (dev->thread != RT_NULL)
                rt_thread_delete(dev->thread);
#endif
            dev = RT_NULL;
        }
        return RT_NULL;
    }

    return dev;
}

/**
 * This function releases memory and deletes mutex lock
 *
 * @param dev the pointer of device driver structure
 */
void aht10_deinit(aht10_device_t dev)
{
    RT_ASSERT(dev);

    rt_mutex_delete(dev->lock);

#ifdef AHT10_USING_SOFT_FILTER
    rt_thread_delete(dev->thread);
#endif

    rt_free(dev);
}

void aht10(int argc, char *argv[])
{
    static aht10_device_t dev = RT_NULL;

    if (argc > 1)
    {
        if (!strcmp(argv[1], "probe"))
        {
            if (argc > 2)
            {
                /* initialize the sensor when first probe */
                if (!dev || strcmp(dev->i2c->parent.parent.name, argv[2]))
                {
                    /* deinit the old device */
                    if (dev)
                    {
                        aht10_deinit(dev);
                    }
                    dev = aht10_init(argv[2]);
                }
            }
            else
            {
                rt_kprintf("aht10 probe <dev_name>   - probe sensor by given name\n");
            }
        }
        else if (!strcmp(argv[1], "read"))
        {
            if (dev)
            {
                float humidity, temperature;

                /* read the sensor data */
                humidity = aht10_read_humidity(dev);
                temperature = aht10_read_temperature(dev);
                
                rt_kprintf("read aht10 sensor humidity   : %d.%d %%\n", (int)humidity, (int)(humidity * 10) % 10);
                rt_kprintf("read aht10 sensor temperature: %d.%d \n", (int)temperature, (int)(temperature * 10) % 10);
            }
            else
            {
                rt_kprintf("Please using 'aht10 probe <dev_name>' first\n");
            }
        }
        else
        {
            rt_kprintf("Unknown command. Please enter 'aht10' for help\n");
        }
    }
    else
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("aht10 probe <dev_name>   - probe sensor by given name\n");
        rt_kprintf("aht10 read               - read sensor aht10 data\n");
    }
}
MSH_CMD_EXPORT(aht10, aht10 sensor function);

#endif /* PKG_USING_AHT10 */
