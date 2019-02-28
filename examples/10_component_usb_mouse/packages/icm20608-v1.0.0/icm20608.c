/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-03     Ernest Chen  the first version
 */

#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <finsh.h>

#include <string.h>
#include "icm20608.h"

#ifdef PKG_USING_ICM20608

#define DBG_ENABLE
#define DBG_SECTION_NAME "icm20608"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

// Register map for icm20608
#define ICM20608_CONFIG_REG 0x1A        // configuration:fifo, ext sync and dlpf
#define ICM20608_GYRO_CONFIG_REG 0x1B   // gyroscope configuration
#define ICM20608_ACCEL_CONFIG1_REG 0x1C // accelerometer configuration
#define ICM20608_ACCEL_CONFIG2_REG 0x1D // accelerometer configuration
#define ICM20608_INT_ENABLE_REG 0x38    // interrupt enable
#define ICM20608_ACCEL_MEAS 0x3B        // accelerometer measurements
#define ICM20608_GYRO_MEAS 0x43         // gyroscope measurements
#define ICM20608_PWR_MGMT1_REG 0x6B     // power management 1
#define ICM20608_PWR_MGMT2_REG 0x6C     // power management 2

#define ICM20608_ADDR 0x68 /* slave address, ad0 set 0 */

/**
 * This function burst writes sequences, include single-byte.
 *
 * @param bus the pointer of iic bus
 * @param reg the address regisgter
 * @param len the length of bytes
 * @param buf the writing value
 *
 * @return the writing status of accelerometer, RT_EOK reprensents setting successfully.
 */
static rt_err_t write_regs(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t len, rt_uint8_t *buf)
{
    struct rt_i2c_msg msgs[2];

    msgs[0].addr = ICM20608_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = &reg;
    msgs[0].len = 1;

    msgs[1].addr = ICM20608_ADDR;
    msgs[1].flags = RT_I2C_WR | RT_I2C_NO_START;
    msgs[1].buf = buf;
    msgs[1].len = len;

    if (rt_i2c_transfer(bus, msgs, 2) == 2)
    {
        return RT_EOK;
    }
    else
    {
        LOG_E("Writing command error");
        return -RT_ERROR;
    }
}

static rt_err_t read_regs(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t len, rt_uint8_t *buf)
{
    struct rt_i2c_msg msgs[2];

    msgs[0].addr = ICM20608_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = &reg;
    msgs[0].len = 1;

    msgs[1].addr = ICM20608_ADDR;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf = buf;
    msgs[1].len = len;

    if (rt_i2c_transfer(bus, msgs, 2) == 2)
    {
        return RT_EOK;
    }
    else
    {
        LOG_E("Reading command error");
        return -RT_ERROR;
    }
}

static rt_err_t reset_sensor(icm20608_device_t dev)
{
    rt_uint8_t value = 0;

    RT_ASSERT(dev);

    return write_regs(dev->i2c, ICM20608_PWR_MGMT1_REG, 1, &value);
}

static rt_err_t icm20608_sensor_init(icm20608_device_t dev)
{
    rt_err_t result = -RT_ERROR;

    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        goto __exit;
    }

    result = reset_sensor(dev);
    if (result != RT_EOK)
    {
        goto __exit;
    }
    rt_thread_mdelay(50);

    /* open 3 accelerometers and 3 gyroscope */
    result = icm20608_set_param(dev, ICM20608_PWR_MGMT2, 0);
    if (result != RT_EOK)
    {
        goto __exit;
    }

    /* set gyroscope range, default 250 dps */
    result = icm20608_set_param(dev, ICM20608_GYRO_CONFIG, 0);
    if (result != RT_EOK)
    {
        goto __exit;
    }

    /* set accelerometer range, default 2g */
    result = icm20608_set_param(dev, ICM20608_ACCEL_CONFIG1, 0);
    if (result != RT_EOK)
    {
        goto __exit;
    }

    /* ACCEL_FCHOICE_B = 0 and A_DLPF_CFG[2:0] = 1 */
    result = icm20608_set_param(dev, ICM20608_ACCEL_CONFIG2, 1);
    if (result != RT_EOK)
    {
        goto __exit;
    }

__exit:
    if (result != RT_EOK)
    {
        LOG_E("This sensor initializes failure");
    }
    rt_mutex_release(dev->lock);

    return result;
}

/**
 * This function calibrates original gyroscope and accelerometer, which are bias, in calibration mode
 *
 * @param dev the pointer of device driver structure
 * @param times averaging times in calibration mode
 *
 * @return the getting original status, RT_EOK reprensents getting gyroscope successfully.
 */
rt_err_t icm20608_calib_level(icm20608_device_t dev, rt_size_t times)
{
    rt_int32_t accel[3] = {0, 0, 0};
    rt_int32_t gyro[3] = {0, 0, 0};
    rt_size_t i;
    rt_err_t result;

    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        for (i = 0; i < times; i++)
        {
            rt_int16_t x, y, z;

            /* read the sensor digital output */
            result = icm20608_get_accel(dev, &x, &y, &z);
            if (result == RT_EOK)
            {
                accel[0] += x;
                accel[1] += y;
                accel[2] += z;
            }
            else
            {
                break;
            }

            result = icm20608_get_gyro(dev, &x, &y, &z);
            if (result == RT_EOK)
            {
                gyro[0] += x;
                gyro[1] += y;
                gyro[2] += z;
            }
            else
            {
                break;
            }
        }

        if (result == RT_EOK)
        {
            dev->accel_offset.x = (accel[0] / (int)times);
            dev->accel_offset.y = (accel[1] / (int)times);
            dev->accel_offset.z = (accel[2] / (int)times) - 0x3fff; //default full range

            dev->gyro_offset.x = (int16_t)(gyro[0] / (int)times);
            dev->gyro_offset.y = (int16_t)(gyro[1] / (int)times);
            dev->gyro_offset.z = (int16_t)(gyro[2] / (int)times);
        }
    }

    if (result == RT_EOK)
    {
        rt_mutex_release(dev->lock);
    }
    else
    {
        LOG_E("Can't calibrate the sensor");
    }

    return result;
}

/**
 * This function initializes icm20608 registered device driver
 *
 * @param dev the name of icm20608 device
 *
 * @return the icm20608 device.
 */
icm20608_device_t icm20608_init(const char *i2c_bus_name)
{
    icm20608_device_t dev;

    RT_ASSERT(i2c_bus_name);

    dev = rt_calloc(1, sizeof(struct icm20608_device));
    if (dev == RT_NULL)
    {
        LOG_E("Can't allocate memory for icm20608 device on '%s' ", i2c_bus_name);
        rt_free(dev);

        return RT_NULL;
    }

    dev->i2c = rt_i2c_bus_device_find(i2c_bus_name);

    if (dev->i2c == RT_NULL)
    {
        LOG_E("Can't find icm20608 device on '%s'", i2c_bus_name);
        rt_free(dev);

        return RT_NULL;
    }

    dev->lock = rt_mutex_create("mutex_icm20608", RT_IPC_FLAG_FIFO);
    if (dev->lock == RT_NULL)
    {
        LOG_E("Can't create mutex for icm20608 device on '%s'", i2c_bus_name);
        rt_free(dev);

        return RT_NULL;
    }

    /* init icm20608 sensor */
    if (icm20608_sensor_init(dev) != RT_EOK)
    {
        LOG_E("Can't init icm20608 device on '%s'", i2c_bus_name);
        rt_free(dev);
        rt_mutex_delete(dev->lock);

        return RT_NULL;
    }

    return dev;
}

/**
 * This function releases memory and deletes mutex lock
 *
 * @param dev the pointer of device driver structure
 */
void icm20608_deinit(icm20608_device_t dev)
{
    RT_ASSERT(dev);

    rt_mutex_delete(dev->lock);
    rt_free(dev);
}

/**
 * This function gets accelerometer by icm20608 sensor measurement
 *
 * @param dev the pointer of device driver structure
 * @param accel_x the accelerometer of x-axis digital output
 * @param accel_y the accelerometer of y-axis digital output
 * @param accel_z the accelerometer of z-axis digital output
 *
 * @return the getting status of accelerometer, RT_EOK reprensents  getting accelerometer successfully.
 */
rt_err_t icm20608_get_accel(icm20608_device_t dev, rt_int16_t *accel_x, rt_int16_t *accel_y, rt_int16_t *accel_z)
{
    rt_err_t result = -RT_ERROR;
    rt_uint8_t value[6];
    rt_uint8_t range = 0;

    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        result = icm20608_get_param(dev, ICM20608_ACCEL_CONFIG1, &range); // default 2g

        if (range < 4 && result == RT_EOK)
        {
            result = read_regs(dev->i2c, ICM20608_ACCEL_MEAS, 6, &value[0]); //self test x,y,z accelerometer;

            if (result != RT_EOK)
            {
                LOG_E("Failed to get accelerometer value of icm20608");
            }
            else
            {
                *accel_x = (value[0] << 8) + value[1] - dev->accel_offset.x;
                *accel_y = (value[2] << 8) + value[3] - dev->accel_offset.y;
                *accel_z = (value[4] << 8) + value[5] - dev->accel_offset.z;
            }
        }
        else
        {
            LOG_E("Failed to get accelerometer value of icm20608");
        }

        rt_mutex_release(dev->lock);
    }
    else
    {
        LOG_E("Failed to get accelerometer value of icm20608");
    }

    return result;
}

/**
 * This function gets gyroscope by icm20608 sensor measurement
 *
 * @param dev the pointer of device driver structure
 * @param gyro_x the gyroscope of x-axis digital output
 * @param gyro_y the gyroscope of y-axis digital output
 * @param gyro_z the gyroscope of z-axis digital output
 *
 * @return the getting status of gyroscope, RT_EOK reprensents getting gyroscope successfully.
 */
rt_err_t icm20608_get_gyro(icm20608_device_t dev, rt_int16_t *gyro_x, rt_int16_t *gyro_y, rt_int16_t *gyro_z)
{
    rt_err_t result = -RT_ERROR;
    rt_uint8_t range = 0;
    rt_uint8_t value[6];

    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        result = icm20608_get_param(dev, ICM20608_GYRO_CONFIG, &range); // default 250dps

        if (range < 4 && result == RT_EOK)
        {
            result = read_regs(dev->i2c, ICM20608_GYRO_MEAS, 6, &value[0]); //self test x,y,z gyroscope;

            if (result != RT_EOK)
            {
                LOG_E("Failed to get gyroscope value of icm20608");
            }
            else
            {
                *gyro_x = (value[0] << 8) + value[1] - dev->gyro_offset.x;
                *gyro_y = (value[2] << 8) + value[3] - dev->gyro_offset.y;
                *gyro_z = (value[4] << 8) + value[5] - dev->gyro_offset.z;
            }
        }
        else
        {
            LOG_E("Failed to get gyroscope value of icm20608");
        }

        rt_mutex_release(dev->lock);
    }
    else
    {
        LOG_E("Failed to get gyroscope value of icm20608");
    }

    return result;
}

/**
 * This function sets parameter of icm20608 sensor
 *
 * @param dev the pointer of device driver structure
 * @param cmd the parameter cmd of device
 * @param value for setting value in cmd register
 *
 * @return the setting parameter status,RT_EOK reprensents setting successfully.
 */
rt_err_t icm20608_set_param(icm20608_device_t dev, icm20608_set_cmd_t cmd, uint8_t value)
{
    rt_err_t result = -RT_ERROR;
    RT_ASSERT(dev);

    switch (cmd)
    {
    case ICM20608_GYRO_CONFIG:
    {
        rt_uint8_t args;

        if (!(value == ICM20608_GYROSCOPE_RANGE0 || value == ICM20608_GYROSCOPE_RANGE1 || value == ICM20608_GYROSCOPE_RANGE2 || value == ICM20608_GYROSCOPE_RANGE3))
        {
            LOG_E("Setting gyroscope range is wrong, please refer gyroscope range");

            return -RT_ERROR;
        }

        result = read_regs(dev->i2c, ICM20608_GYRO_CONFIG_REG, 1, &args);
        if (result == RT_EOK)
        {
            args &= 0xE7; //clear [4:3]
            args |= value << 3;
            result = write_regs(dev->i2c, ICM20608_GYRO_CONFIG_REG, 1, &args);
        }

        break;
    }
    case ICM20608_ACCEL_CONFIG1:
    {
        rt_uint8_t args;

        if (!(value == ICM20608_ACCELEROMETER_RANGE0 || value == ICM20608_ACCELEROMETER_RANGE1 || value == ICM20608_ACCELEROMETER_RANGE2 || value == ICM20608_ACCELEROMETER_RANGE3))
        {
            LOG_E("Setting als accelerometer range is wrong, please refer accelerometer range");

            return -RT_ERROR;
        }

        result = read_regs(dev->i2c, ICM20608_ACCEL_CONFIG1_REG, 1, &args);
        if (result == RT_EOK)
        {
            args &= 0xE7; //clear [4:3]
            args |= value << 3;
            result = write_regs(dev->i2c, ICM20608_ACCEL_CONFIG1_REG, 1, &args);
        }
        break;
    }
    case ICM20608_ACCEL_CONFIG2:
    {
        result = write_regs(dev->i2c, ICM20608_ACCEL_CONFIG2_REG, 1, &value);
        break;
    }
    case ICM20608_PWR_MGMT2:
    {
        result = write_regs(dev->i2c, ICM20608_PWR_MGMT2_REG, 1, &value);
        break;
    }
    case ICM20608_INT_ENABLE:
    {
        result = write_regs(dev->i2c, ICM20608_INT_ENABLE_REG, 1, &value);
        break;
    }

    default:
    {
        LOG_E("This cmd'%2x' can not be set or supported", cmd);

        return -RT_ERROR;
    }
    }

    return result;
}

/**
 * This function gets parameter of icm20608 sensor
 *
 * @param dev the pointer of device driver structure
 * @param cmd the parameter cmd of device
 * @param value to get value in cmd register
 *
 * @return the getting parameter status,RT_EOK reprensents getting successfully.
 */
rt_err_t icm20608_get_param(icm20608_device_t dev, icm20608_set_cmd_t cmd, rt_uint8_t *value)
{
    rt_err_t result = -RT_ERROR;

    RT_ASSERT(dev);

    switch (cmd)
    {
    case ICM20608_GYRO_CONFIG:
    {
        rt_uint8_t args;

        result = read_regs(dev->i2c, ICM20608_GYRO_CONFIG_REG, 1, &args);
        *value = (args >> 3) & 0x3;
        break;
    }
    case ICM20608_ACCEL_CONFIG1:
    {
        rt_uint8_t args;

        result = read_regs(dev->i2c, ICM20608_ACCEL_CONFIG1_REG, 1, &args);
        *value = (args >> 3) & 0x3;
        break;
    }
    case ICM20608_ACCEL_CONFIG2:
    {
        rt_uint8_t args;

        result = read_regs(dev->i2c, ICM20608_ACCEL_CONFIG2_REG, 1, &args);
        break;
    }

    case ICM20608_PWR_MGMT2:
    {
        result = read_regs(dev->i2c, ICM20608_PWR_MGMT2_REG, 1, value);
        break;
    }
    case ICM20608_INT_ENABLE:
    {
        result = read_regs(dev->i2c, ICM20608_INT_ENABLE_REG, 1, value);
        break;
    }
    default:
    {
        LOG_E("This cmd'%2x' can not be get or supported", cmd);

        return -RT_ERROR;
    }
    }

    return result;
}

void icm20608(int argc, char *argv[])
{
    static icm20608_device_t dev = RT_NULL;

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
                        icm20608_deinit(dev);
                    }
                    dev = icm20608_init(argv[2]);
                }
            }
            else
            {
                rt_kprintf("icm20608 probe <dev_name>   - probe sensor by given name\n");
            }
        }
        else if (!strcmp(argv[1], "read"))
        {
            if (dev)
            {
                if (argc > 2 && atoi(argv[2]) > 0)
                {
                    rt_int16_t x, y, z;
                    rt_uint32_t i;

                    for (i = 0; i < atoi(argv[2]); i++)
                    {
                        /* read the sensor digital output */
                        icm20608_get_accel(dev, &x, &y, &z);
                        rt_kprintf("accelerometer: X%6d    Y%6d    Z%6d\n", x, y, z);
                        icm20608_get_gyro(dev, &x, &y, &z);
                        rt_kprintf("gyroscope    : X%6d    Y%6d    Z%6d\r\033[1A", x, y, z);
                    }
                    rt_kprintf("\n\n");
                }
                else if (argc == 2)
                {
                    rt_int16_t x, y, z;

                    /* read the sensor digital output */
                    icm20608_get_accel(dev, &x, &y, &z);
                    rt_kprintf("accelerometer: X%6d    Y%6d    Z%6d\n", x, y, z);
                    icm20608_get_gyro(dev, &x, &y, &z);
                    rt_kprintf("gyroscope    : X%6d    Y%6d    Z%6d\n", x, y, z);
                }
                else
                {
                    rt_kprintf("[times] should be set bigger than 1\n");
                }
            }
            else
            {
                rt_kprintf("Please using 'icm20608 probe <dev_name>' first\n");
            }
        }
        else if (!strcmp(argv[1], "calib"))
        {
            if (dev)
            {
                /* calibration icm20608 zero level */
                icm20608_calib_level(dev, 10);
                rt_kprintf("accel_offset: X%6d    Y%6d    Z%6d\n", dev->accel_offset.x, dev->accel_offset.y, dev->accel_offset.z);
                rt_kprintf("gyro_offset : X%6d    Y%6d    Z%6d\n", dev->gyro_offset.x, dev->gyro_offset.y, dev->gyro_offset.z);
            }
            else
            {
                rt_kprintf("Please using 'icm20608 probe <dev_name>' first\n");
            }
        }
        else
        {
            rt_kprintf("Unknown command. Please enter 'icm20608' for help\n");
        }
    }
    else
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("icm20608 probe <dev_name>   - probe sensor by given name\n");
        rt_kprintf("icm20608 calib              - calibrate sensor in calibration status\n");
        rt_kprintf("icm20608 read [times]       - read sensor icm20608 data\n");
    }
}
MSH_CMD_EXPORT(icm20608, icm20608 sensor function);

#endif /* PKG_USING_ICM20608 */
