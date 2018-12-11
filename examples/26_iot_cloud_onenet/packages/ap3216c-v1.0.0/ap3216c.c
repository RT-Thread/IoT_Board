/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-20     Ernest Chen  the first version
 */

#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <finsh.h>

#include <string.h>
#include "ap3216c.h"

#ifdef PKG_USING_AP3216C

#define DBG_ENABLE
#define DBG_SECTION_NAME "ap3216c"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

//System Register
#define AP3216C_SYS_CONFIGURATION_REG 0x00 //default
#define AP3216C_SYS_INT_STATUS_REG 0x01
#define AP3216C_SYS_INT_CLEAR_MANNER_REG 0x02
#define AP3216C_IR_DATA_L_REG 0x0A
#define AP3216C_IR_DATA_H_REG 0x0B
#define AP3216C_ALS_DATA_L_REG 0x0C
#define AP3216C_ALS_DATA_H_REG 0x0D
#define AP3216C_PS_DATA_L_REG 0x0E
#define AP3216C_PS_DATA_H_REG 0x0F

//ALS Register
#define AP3216C_ALS_CONFIGURATION_REG 0x10 //range 5:4,persist 3:0
#define AP3216C_ALS_CALIBRATION_REG 0x19
#define AP3216C_ALS_THRESHOLD_LOW_L_REG 0x1A  //bit 7:0
#define AP3216C_ALS_THRESHOLD_LOW_H_REG 0x1B  //bit 15:8
#define AP3216C_ALS_THRESHOLD_HIGH_L_REG 0x1C //bit 7:0
#define AP3216C_ALS_THRESHOLD_HIGH_H_REG 0x1D //bit 15:8

//PS Register
#define AP3216C_PS_CONFIGURATION_REG 0x20
#define AP3216C_PS_LED_DRIVER_REG 0x21
#define AP3216C_PS_INT_FORM_REG 0x22
#define AP3216C_PS_MEAN_TIME_REG 0x23
#define AP3216C_PS_LED_WAITING_TIME_REG 0x24
#define AP3216C_PS_CALIBRATION_L_REG 0x28
#define AP3216C_PS_CALIBRATION_H_REG 0x29
#define AP3216C_PS_THRESHOLD_LOW_L_REG 0x2A  //bit 1:0
#define AP3216C_PS_THRESHOLD_LOW_H_REG 0x2B  //bit 9:2
#define AP3216C_PS_THRESHOLD_HIGH_L_REG 0x2C //bit 1:0
#define AP3216C_PS_THRESHOLD_HIGH_H_REG 0x2D //bit 9:2

#define AP3216C_ADDR 0x1e /*0x3c=0x1e<<1*/

static rt_err_t write_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t data)
{
    struct rt_i2c_msg msgs;
    rt_uint8_t temp[2];

    temp[0] = reg;
    temp[1] = data;

    msgs.addr = AP3216C_ADDR;
    msgs.flags = RT_I2C_WR;
    msgs.buf = temp;
    msgs.len = 2;

    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
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

    msgs[0].addr = AP3216C_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = &reg;
    msgs[0].len = 1;

    msgs[1].addr = AP3216C_ADDR;
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

static rt_err_t reset_sensor(ap3216c_device_t dev)
{
    RT_ASSERT(dev);

    write_reg(dev->i2c, AP3216C_SYS_CONFIGURATION_REG, AP3216C_MODE_SW_RESET); //reset

    return RT_EOK;
}

/**
 * This function is convenient to getting data except including high and low data for this sensor.
 * note:after reading lower register first,reading higher add one.
 */
static rt_uint32_t read_low_and_high(ap3216c_device_t dev, rt_uint8_t reg, rt_uint8_t len)
{
    rt_uint32_t data;
    rt_uint8_t buf = 0;

    read_regs(dev->i2c, reg, len, &buf); //low
    data = buf;
    read_regs(dev->i2c, reg + 1, len, &buf); //high
    data = data + (buf << len * 8);

    return data;
}

#ifdef AP3216C_USING_HW_INT

/**
 * This function is only used to set threshold without filtering times
 *
 * @param dev the name of ap3216c device
 * @param cmd first register , and other cmd count by it.
 * @param threshold threshold and filtering times of als threshold
 */
static void set_threshold(ap3216c_device_t dev, ap3216c_cmd_t cmd, ap3216c_threshold_t threshold)
{
    ap3216c_set_param(dev, cmd, (threshold.min & 0xff));
    ap3216c_set_param(dev, (ap3216c_cmd_t)(cmd + 1), (threshold.min >> 8));
    ap3216c_set_param(dev, (ap3216c_cmd_t)(cmd + 2), (threshold.max & 0xff));
    ap3216c_set_param(dev, (ap3216c_cmd_t)(cmd + 3), threshold.max >> 8);
}

static void ap3216c_hw_interrupt(void *args)
{
    ap3216c_device_t dev = (ap3216c_device_t)args;

    if (dev->als_int_cb)
    {
        dev->als_int_cb(dev->als_int_cb);
    }
    if (dev->ps_int_cb)
    {
        dev->ps_int_cb(dev->ps_int_cb);
    }
}

static void ap3216c_int_init(ap3216c_device_t dev)
{
    RT_ASSERT(dev);

    rt_pin_mode(AP3216C_INT_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(AP3216C_INT_PIN, PIN_IRQ_MODE_FALLING, ap3216c_hw_interrupt, (void *)dev);
    rt_pin_irq_enable(AP3216C_INT_PIN, PIN_IRQ_ENABLE);
}

/**
 * This function registers als interrupt with callback function
 *
 * @param dev the name of ap3216c device
 * @param enabled enable or disenable als interrupt
 * @param threshold threshold and filtering times of als threshold
 *
 * @param int_cb callback funtion is defined by user.
 */
void ap3216c_int_als_cb(ap3216c_device_t dev, rt_bool_t enabled, ap3216c_threshold_t threshold, ap3216c_int_cb int_cb)
{
    RT_ASSERT(dev);

    if (enabled)
    {
        dev->als_int_cb = int_cb;
        set_threshold(dev, AP3216C_ALS_LOW_THRESHOLD_L, threshold);
    }
    else
    {
        dev->als_int_cb = RT_NULL;
    }
}

/**
 * This function registers ps interrupt with callback function
 *
 * @param dev the name of ap3216c device
 * @param enabled enable or disenable ps interrupt
 * @param threshold threshold and filtering times of ps threshold
 *
 * @param int_cb callback funtion is defined by user.
 */
void ap3216c_int_ps_cb(ap3216c_device_t dev, rt_bool_t enabled, ap3216c_threshold_t threshold, ap3216c_int_cb int_cb)
{
    RT_ASSERT(dev);

    if (enabled)
    {
        dev->ps_int_cb = int_cb;
        set_threshold(dev, AP3216C_PS_LOW_THRESHOLD_L, threshold);
    }
    else
    {
        dev->ps_int_cb = RT_NULL;
    }
}

#endif /* AP3216C_USING_HW_INT */

/**
 * This function initializes ap3216c registered device driver
 *
 * @param dev the name of ap3216c device
 *
 * @return the ap3216c device.
 */
ap3216c_device_t ap3216c_init(const char *i2c_bus_name)
{
    ap3216c_device_t dev;

    RT_ASSERT(i2c_bus_name);

    dev = rt_calloc(1, sizeof(struct ap3216c_device));
    if (dev == RT_NULL)
    {
        LOG_E("Can't allocate memory for ap3216c device on '%s' ", i2c_bus_name);
        rt_free(dev);

        return RT_NULL;
    }

    dev->i2c = rt_i2c_bus_device_find(i2c_bus_name);

    if (dev->i2c == RT_NULL)
    {
        LOG_E("Can't find ap3216c device on '%s'", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

    dev->lock = rt_mutex_create("mutex_ap3216c", RT_IPC_FLAG_FIFO);
    if (dev->lock == RT_NULL)
    {
        LOG_E("Can't create mutex for ap3216c device on '%s'", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

    /* reset ap3216c */
    reset_sensor(dev);
    rt_thread_delay(rt_tick_from_millisecond(100)); // delay at least  100ms

    ap3216c_set_param(dev, AP3216C_SYSTEM_MODE, AP3216C_MODE_ALS_AND_PS);
    rt_thread_delay(rt_tick_from_millisecond(100)); // delay at least  100ms

#ifdef AP3216C_USING_HW_INT
    /* init interrupt mode	*/
    ap3216c_int_init(dev);
#endif /* AP3216C_USING_HW_INT */

    return dev;
}

/**
 * This function releases memory and deletes mutex lock
 *
 * @param dev the pointer of device driver structure
 */
void ap3216c_deinit(ap3216c_device_t dev)
{
    RT_ASSERT(dev);

    rt_mutex_delete(dev->lock);

    rt_free(dev);
}

/**
 * This function reads temperature by ap3216c sensor measurement
 *
 * @param dev the pointer of device driver structure
 *
 * @return the ambient light converted to float data.
 */
float ap3216c_read_ambient_light(ap3216c_device_t dev)
{
    float brightness = 0.0; // default error data
    rt_err_t result;
    rt_uint32_t read_data;
    rt_uint8_t temp;

    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        read_data = read_low_and_high(dev, AP3216C_ALS_DATA_L_REG, 1);

        ap3216c_get_param(dev, AP3216C_ALS_RANGE, &temp);
        if (temp == AP3216C_ALS_RANGE_20661)
        {
            brightness = 0.35 * read_data; //sensor ambient light converse to reality
        }
        else if (temp == AP3216C_ALS_RANGE_5162)
        {
            brightness = 0.0788 * read_data; //sensor ambient light converse to reality
        }
        else if (temp == AP3216C_ALS_RANGE_1291)
        {
            brightness = 0.0197 * read_data; //sensor ambient light converse to reality
        }
        else if (temp == AP3216C_ALS_RANGE_323)
        {
            brightness = 0.0049 * read_data; //sensor ambient light converse to reality
        }
        else
        {
            LOG_E("Failed to get range of ap3216c");
        }
    }
    else
    {
        LOG_E("Failed to reading ambient light");
    }
    rt_mutex_release(dev->lock);

    return brightness;
}

/**
 * This function reads temperature by ap3216c sensor measurement
 *
 * @param dev the pointer of device driver structure
 *
 * @return the proximity data.
 */
uint16_t ap3216c_read_ps_data(ap3216c_device_t dev)
{
    rt_uint16_t proximity = 0;
    rt_err_t result;

    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        rt_uint32_t read_data;
        read_data = read_low_and_high(dev, AP3216C_PS_DATA_L_REG, 1); //read two data

        if (1 == ((read_data >> 6) & 0x01 || (read_data >> 14) & 0x01))
        {
            LOG_I("The data of PS is invalid for high intensive IR light ");
        }

        proximity = (read_data & 0x000f) + (((read_data >> 8) & 0x3f) << 4); //sensor proximity converse to reality
    }
    else
    {
        LOG_E("Failed to reading ps data ");
    }
    rt_mutex_release(dev->lock);

    return proximity;
}

/**
 * This function sets parameter of ap3216c sensor
 *
 * @param dev the pointer of device driver structure
 * @param cmd the parameter cmd of device
 * @param value for setting value in cmd register
 *
 * @return the setting parameter status,RT_EOK reprensents setting successfully.
 */
rt_err_t ap3216c_set_param(ap3216c_device_t dev, ap3216c_cmd_t cmd, uint8_t value)
{
    RT_ASSERT(dev);

    switch (cmd)
    {
    case AP3216C_SYSTEM_MODE:
    {
        if (value > AP3216C_MODE_ALS_AND_PS_ONCE)
        {
            LOG_E("Setting system mode parameter is wrong !");
            return -RT_ERROR;
        }
        /* default 000,power down */
        write_reg(dev->i2c, AP3216C_SYS_CONFIGURATION_REG, value);

        break;
    }
    case AP3216C_INT_PARAM:
    {
        if (value > AP3216C_ALS_CLEAR_MANNER_BY_SOFTWARE)
        {
            LOG_E("Setting int parameter is wrong !");
            return -RT_ERROR;
        }
        write_reg(dev->i2c, AP3216C_SYS_INT_CLEAR_MANNER_REG, value);

        break;
    }

    case AP3216C_ALS_RANGE:
    {
        rt_uint8_t args;

        if (!(value == AP3216C_ALS_RANGE_20661 || value == AP3216C_ALS_RANGE_5162 || value == AP3216C_ALS_RANGE_1291 || value == AP3216C_ALS_RANGE_323))
        {
            LOG_E("Setting als dynamic range is wrong, please refer als_range");
            return -RT_ERROR;
        }
        read_regs(dev->i2c, AP3216C_ALS_CONFIGURATION_REG, 1, &args);
        args &= 0xcf;
        args |= value << 4;
        write_reg(dev->i2c, AP3216C_ALS_CONFIGURATION_REG, args);

        break;
    }
    case AP3216C_ALS_PERSIST:
    {
        rt_uint8_t args = 0;

        if (value > 0x0f)
        {
            LOG_E("Setting als persist overflows ");
            return -RT_ERROR;
        }
        read_regs(dev->i2c, AP3216C_ALS_CONFIGURATION_REG, 1, &args);
        args &= 0xf0;
        args |= value;
        write_reg(dev->i2c, AP3216C_ALS_CONFIGURATION_REG, args);

        break;
    }
    case AP3216C_ALS_LOW_THRESHOLD_L:
    {
        write_reg(dev->i2c, AP3216C_ALS_THRESHOLD_LOW_L_REG, value);

        break;
    }
    case AP3216C_ALS_LOW_THRESHOLD_H:
    {
        write_reg(dev->i2c, AP3216C_ALS_THRESHOLD_LOW_H_REG, value);

        break;
    }
    case AP3216C_ALS_HIGH_THRESHOLD_L:
    {
        write_reg(dev->i2c, AP3216C_ALS_THRESHOLD_HIGH_L_REG, value);

        break;
    }
    case AP3216C_ALS_HIGH_THRESHOLD_H:
    {
        write_reg(dev->i2c, AP3216C_ALS_THRESHOLD_HIGH_H_REG, value);

        break;
    }
    case AP3216C_PS_GAIN:
    {
        rt_uint8_t args = 0;

        if (value > 0x3)
        {
            LOG_E("Setting ps again overflows ");
            return -RT_ERROR;
        }
        read_regs(dev->i2c, AP3216C_PS_CONFIGURATION_REG, 1, &args);
        args &= 0xf3;
        args |= value;
        write_reg(dev->i2c, AP3216C_PS_CONFIGURATION_REG, args);

        break;
    }
    case AP3216C_PS_PERSIST:
    {
        rt_uint8_t args = 0;

        if (value > 0x3)
        {
            LOG_E("Setting ps persist overflows ");
            return -RT_ERROR;
        }
        read_regs(dev->i2c, AP3216C_PS_CONFIGURATION_REG, 1, &args);
        args &= 0xfc;
        args |= value;
        write_reg(dev->i2c, AP3216C_PS_CONFIGURATION_REG, args);

        break;
    }
    case AP3216C_PS_LOW_THRESHOLD_L:
    {
        if (value > 0x3)
        {
            LOG_E("Setting ps low threshold of low bit is wrong !");
            return -RT_ERROR;
        }
        write_reg(dev->i2c, AP3216C_PS_THRESHOLD_LOW_L_REG, value);

        break;
    }
    case AP3216C_PS_LOW_THRESHOLD_H:
    {
        write_reg(dev->i2c, AP3216C_PS_THRESHOLD_LOW_H_REG, value);

        break;
    }
    case AP3216C_PS_HIGH_THRESHOLD_L:
    {
        if (value > 0x3)
        {
            LOG_E("Setting ps high threshold of low bit is wrong !");
            return -RT_ERROR;
        }
        write_reg(dev->i2c, AP3216C_PS_THRESHOLD_HIGH_L_REG, value);

        break;
    }
    case AP3216C_PS_HIGH_THRESHOLD_H:
    {
        write_reg(dev->i2c, AP3216C_PS_THRESHOLD_HIGH_H_REG, value);

        break;
    }

    default:
    {
        return -RT_ERROR;
    }
    }

    return RT_EOK;
}

/**
 * This function gets parameter of ap3216c sensor
 *
 * @param dev the pointer of device driver structure
 * @param cmd the parameter cmd of device
 * @param value to get value in cmd register
 *
 * @return the getting parameter status,RT_EOK reprensents getting successfully.
 */
rt_err_t ap3216c_get_param(ap3216c_device_t dev, ap3216c_cmd_t cmd, rt_uint8_t *value)
{
    RT_ASSERT(dev);

    switch (cmd)
    {
    case AP3216C_SYSTEM_MODE:
    {
        read_regs(dev->i2c, AP3216C_SYS_CONFIGURATION_REG, 1, value);

        if (*value > AP3216C_MODE_ALS_AND_PS_ONCE)
        {
            LOG_E("Getting system mode parameter is wrong !");
            return -RT_ERROR;
        }
        break;
    }
    case AP3216C_INT_PARAM:
    {
        read_regs(dev->i2c, AP3216C_SYS_INT_CLEAR_MANNER_REG, 1, value);

        if (*value > AP3216C_ALS_CLEAR_MANNER_BY_SOFTWARE)
        {
            LOG_E("Setting int parameter is wrong !");
            return -RT_ERROR;
        }
        break;
    }
    case AP3216C_ALS_RANGE:
    {
        rt_uint8_t temp;

        read_regs(dev->i2c, AP3216C_ALS_CONFIGURATION_REG, 1, value);
        temp = (*value & 0xff) >> 4;

        if (!(temp == AP3216C_ALS_RANGE_20661 || temp == AP3216C_ALS_RANGE_5162 || temp == AP3216C_ALS_RANGE_1291 || temp == AP3216C_ALS_RANGE_323))
        {
            LOG_E("Getting als dynamic range is wrong, please refer als_range");
            return -RT_ERROR;
        }

        *value = temp;

        break;
    }
    case AP3216C_ALS_PERSIST:
    {
        rt_uint8_t temp;

        read_regs(dev->i2c, AP3216C_ALS_CONFIGURATION_REG, 1, value);
        temp = *value & 0x0f;

        if (temp > 0x0f)
        {
            LOG_E("Getting als persist is wrong, please refer als_range");
            return -RT_ERROR;
        }
        *value = temp;

        break;
    }
    case AP3216C_ALS_LOW_THRESHOLD_L:
    {
        read_regs(dev->i2c, AP3216C_ALS_THRESHOLD_LOW_L_REG, 1, value);

        break;
    }
    case AP3216C_ALS_LOW_THRESHOLD_H:
    {
        read_regs(dev->i2c, AP3216C_ALS_THRESHOLD_LOW_H_REG, 1, value);

        break;
    }
    case AP3216C_ALS_HIGH_THRESHOLD_L:
    {
        read_regs(dev->i2c, AP3216C_ALS_THRESHOLD_HIGH_L_REG, 1, value);

        break;
    }
    case AP3216C_ALS_HIGH_THRESHOLD_H:
    {
        read_regs(dev->i2c, AP3216C_ALS_THRESHOLD_HIGH_H_REG, 1, value);

        break;
    }
    case AP3216C_PS_GAIN:
    {
        rt_uint8_t temp;

        read_regs(dev->i2c, AP3216C_PS_CONFIGURATION_REG, 1, &temp);

        *value = (temp & 0xc) >> 2;

        break;
    }
    case AP3216C_PS_PERSIST:
    {
        rt_uint8_t temp;

        read_regs(dev->i2c, AP3216C_PS_CONFIGURATION_REG, 1, &temp);

        *value = temp & 0x3;

        break;
    }
    case AP3216C_PS_LOW_THRESHOLD_L:
    {
        read_regs(dev->i2c, AP3216C_PS_THRESHOLD_LOW_L_REG, 1, value);
        if ((*value & 0xff) > 0x3)
        {
            LOG_E("Getting ps low threshold of low bit is wrong !");
            return -RT_ERROR;
        }
        break;
    }
    case AP3216C_PS_LOW_THRESHOLD_H:
    {
        read_regs(dev->i2c, AP3216C_PS_THRESHOLD_LOW_H_REG, 1, value);
        break;
    }
    case AP3216C_PS_HIGH_THRESHOLD_L:
    {
        read_regs(dev->i2c, AP3216C_PS_THRESHOLD_HIGH_L_REG, 1, value);

        if ((*value & 0xff) > 3)
        {
            LOG_E("Getting ps high threshold of low bit is wrong !");
            return -RT_ERROR;
        }
        break;
    }
    case AP3216C_PS_HIGH_THRESHOLD_H:
    {
        read_regs(dev->i2c, AP3216C_PS_THRESHOLD_HIGH_H_REG, 1, value);

        break;
    }

    default:
    {
        return -RT_ERROR;
    }
    }

    return RT_EOK;
}

void ap3216c(int argc, char *argv[])
{
    static ap3216c_device_t dev = RT_NULL;

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
                        ap3216c_deinit(dev);
                    }
                    dev = ap3216c_init(argv[2]);
                }
            }
            else
            {
                rt_kprintf("ap3216c probe <dev_name>   - probe sensor by given name\n");
            }
        }
        else if (!strcmp(argv[1], "read"))
        {
            if (dev)
            {
                rt_uint16_t ps_data;
                float brightness;

                /* read the sensor */
                ps_data = ap3216c_read_ps_data(dev);
                if (ps_data == 0)
                {
                    rt_kprintf("object is not proximity of sensor \n");
                }
                else
                {
                    rt_kprintf("ap3216c read current ps data      : %d\n", ps_data);
                }

                brightness = ap3216c_read_ambient_light(dev);
                rt_kprintf("ap3216c measure current brightness: %d.%d(lux) \n", (int)brightness, ((int)(10 * brightness) % 10));
            }
            else
            {
                rt_kprintf("Please using 'ap3216c probe <dev_name>' first\n");
            }
        }
        else
        {
            rt_kprintf("Unknown command. Please enter 'ap3216c' for help\n");
        }
    }
    else
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("ap3216c probe <dev_name>   - probe sensor by given name\n");
        rt_kprintf("ap3216c read               - read sensor ap3216c data\n");
    }
}
MSH_CMD_EXPORT(ap3216c, ap3216c sensor function);

#endif /* PKG_USING_AP3216C */
