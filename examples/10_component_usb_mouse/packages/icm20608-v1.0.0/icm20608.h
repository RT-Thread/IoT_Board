/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-03     Ernest Chen  the first version
 */

#ifndef __ICM20608_H__
#define __ICM20608_H__

#include <rthw.h>
#include <rtthread.h>

enum icm20608_set_cmd
{
    ICM20608_PWR_MGMT1,     // power management 1
    ICM20608_PWR_MGMT2,     // power management 2
    ICM20608_GYRO_CONFIG,   // gyroscope configuration(range)
    ICM20608_ACCEL_CONFIG1, // accelerometer configuration(range)
    ICM20608_ACCEL_CONFIG2, // accelerometer configuration2
    ICM20608_INT_ENABLE,    //interrupt enable
};
typedef enum icm20608_set_cmd icm20608_set_cmd_t;

enum icm20608_gyroscope_range
{
    ICM20608_GYROSCOPE_RANGE0, // ±250dps
    ICM20608_GYROSCOPE_RANGE1, // ±500dps
    ICM20608_GYROSCOPE_RANGE2, // ±1000dps
    ICM20608_GYROSCOPE_RANGE3, // ±2000dps
};
typedef enum icm20608_gyroscope_range icm20608_gyro_range_t;

enum icm20608_accelerometer_range
{
    ICM20608_ACCELEROMETER_RANGE0, // ±2g
    ICM20608_ACCELEROMETER_RANGE1, // ±4g
    ICM20608_ACCELEROMETER_RANGE2, // ±8g
    ICM20608_ACCELEROMETER_RANGE3, // ±16g
};
typedef enum icm20608_accelerometer_range icm20608_accel_range_t;

typedef struct cm20608_axes
{
    rt_int16_t x;
    rt_int16_t y;
    rt_int16_t z;
} cm20608_axes_t;

struct icm20608_device
{
    struct rt_i2c_bus_device *i2c;
    cm20608_axes_t accel_offset;
    cm20608_axes_t gyro_offset;
    rt_mutex_t lock;
};
typedef struct icm20608_device *icm20608_device_t;

/**
 * This function initializes icm20608 registered device driver
 *
 * @param dev the name of icm20608 device
 *
 * @return the icm20608 device.
 */
icm20608_device_t icm20608_init(const char *i2c_bus_name);

/**
 * This function releases memory and deletes mutex lock
 *
 * @param dev the pointer of device driver structure.
 */
void icm20608_deinit(icm20608_device_t dev);

/**
 * This function gets accelerometer by icm20608 sensor measurement
 *
 * @param dev the pointer of device driver structure
 * @param accel_x the accelerometer of x-axis digital output
 * @param accel_y the accelerometer of y-axis digital output
 * @param accel_z the accelerometer of z-axis digital output
 *
 * @return the getting status of accelerometer, RT_EOK reprensents getting accelerometer successfully.
 */
rt_err_t icm20608_get_accel(icm20608_device_t dev, rt_int16_t *accel_x, rt_int16_t *accel_y, rt_int16_t *accel_z);

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
rt_err_t icm20608_get_gyro(icm20608_device_t dev, rt_int16_t *gyro_x, rt_int16_t *gyro_y, rt_int16_t *gyro_z);

/**
 * This function calibrates original gyroscope and accelerometer, which are bias, in calibration mode
 *
 * @param dev the pointer of device driver structure
 * @param times averaging times in calibration mode
 *
 * @return the getting original status, RT_EOK reprensents getting gyroscope successfully.
 */
rt_err_t icm20608_calib_level(icm20608_device_t dev, rt_size_t times);

/**
 * This function sets parameter of icm20608 sensor
 *
 * @param dev the pointer of device driver structure
 * @param cmd the parameter cmd of device
 * @param value for setting value in cmd register
 *
 * @return the setting parameter status, RT_EOK reprensents setting successfully.
 */
rt_err_t icm20608_set_param(icm20608_device_t dev, icm20608_set_cmd_t cmd, rt_uint8_t value);

/**
 * This function gets parameter of icm20608 sensor
 *
 * @param dev the pointer of device driver structure
 * @param cmd the parameter cmd of device
 * @param value to get value in cmd register
 *
 * @return the getting parameter status,RT_EOK reprensents getting successfully.
 */
rt_err_t icm20608_get_param(icm20608_device_t dev, icm20608_set_cmd_t cmd, rt_uint8_t *value);

#endif /*__DRV_ICM20608_H__ */
