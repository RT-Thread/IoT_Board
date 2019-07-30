/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-19     MurphyZhao   first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdint.h>
#include "aht10.h"
#include "iotb_sensor.h"
#include "drv_lcd.h"
#include "drv_audio.h"
#include "iotb_event.h"
#include "iotb_lcd_process.h"
#include "iotb_key_process.h"
#include "iotb_workqeue.h"
#include "wav_player.h"
#include "app_infrared.h"

#ifndef IOTB_WIFI_AUTO_CONNECT
#include <easyflash.h>
#include <fal.h>
#endif

#ifdef IOTB_USING_RTT_CLOUD
#include <rt_cld.h>
#include "rt_cld_port.h"
#include <webclient.h>
#include <rt_ota.h>

#include <cJSON.h>
#include <cJSON_util.h>

#include <tiny_md5.h>

#define IOTB_RTCLD_GET_DEV_INFO_BY_SN_URI "http://118.31.42.51:8090/devices?sn="
#define IOTB_RT_CLD_HEADER_SIZE (1024)
static struct cld_dev_info iotb_rtcld_dev_info;
#endif

// #define IOTB_SENSOR_DEBUG

#define DBG_ENABLE
#define DBG_TAG               "IOTB_SENSOR"
#ifdef IOTB_SENSOR_DEBUG
#define DBG_LVL                      DBG_LOG
#else
#define DBG_LVL                      DBG_INFO /* DBG_ERROR */
#endif
#define DBG_COLOR
#include <rtdbg.h>

static aht10_device_t iotb_aht10_dev = RT_NULL;
static uint8_t iotb_aht10_inited = 0;
static uint8_t iotb_aht10_busy = 0;

/**
 * 1. aht10 sensor
*/
rt_err_t iotb_sensor_aht10_init(void)
{
    if (iotb_aht10_inited)
    {
        return RT_EOK;
    }

    const char *aht10_dev_name = "i2c2";

    if (iotb_aht10_dev != RT_NULL)
    {
        iotb_sensor_aht10_deinit();
    }

    iotb_aht10_busy = 1;

    /* initializes aht10, registered device driver */
    iotb_aht10_dev = aht10_init((const char *)aht10_dev_name);
    if (iotb_aht10_dev == RT_NULL)
    {
        LOG_E("The sensor initializes failure");
        return -RT_ERROR;
    }
    iotb_aht10_inited = 1;
    iotb_aht10_busy = 0;

    LOG_I("aht10 device init success!");

    return RT_EOK;
}

rt_err_t iotb_sensor_aht10_deinit(void)
{
    if (iotb_aht10_dev == RT_NULL)
    {
        return -RT_ERROR;
    }

    iotb_aht10_busy = 1;

    aht10_deinit(iotb_aht10_dev);
    iotb_aht10_dev = RT_NULL;
    iotb_aht10_inited = 0;
    iotb_aht10_busy = 0;

    return RT_EOK;
}

/** iotb_sensor_aht10_read_humi
 * @param ops: 0 - temperature; 1 - humidity
*/
rt_err_t iotb_sensor_aht10_read(uint8_t ops, float *data)
{
    if (!iotb_aht10_dev || !data || !iotb_aht10_inited)
    {
        return -RT_ERROR;
    }

    if (iotb_aht10_busy == 1)
    {
        return -RT_EBUSY;
    }

    iotb_aht10_busy = 1;

    *data = ops ? aht10_read_humidity(iotb_aht10_dev) : aht10_read_temperature(iotb_aht10_dev);

    iotb_aht10_busy = 0;
    if ((ops == 0) && (*data == -50))
    {
        return -RT_ERROR;
    }
    else if ((ops == 1) && (*data == 0.0f))
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

/**
 * 2. RGB sensor
*/
rt_err_t iotb_sensor_rgb_ctl(iotb_sensor_rgb_t rgb_t)
{
    switch (rgb_t)
    {
    case IOTB_SENSOR_RGB_INIT:
        /* set RGB_LED pin mode to output */
        rt_pin_mode(PIN_LED_R, PIN_MODE_OUTPUT);
        rt_pin_mode(PIN_LED_G, PIN_MODE_OUTPUT);
        rt_pin_mode(PIN_LED_B, PIN_MODE_OUTPUT);

        rt_pin_write(PIN_LED_R, PIN_HIGH);
        rt_pin_write(PIN_LED_G, PIN_HIGH);
        rt_pin_write(PIN_LED_B, PIN_HIGH);
        break;
    case IOTB_SENSOR_RGB_DEINIT:
        rt_pin_write(PIN_LED_R, PIN_HIGH);
        rt_pin_write(PIN_LED_G, PIN_HIGH);
        rt_pin_write(PIN_LED_B, PIN_HIGH);
        break;
    case IOTB_SENSOR_RGB_OPEN:
        rt_pin_write(PIN_LED_R, PIN_LOW);
        rt_pin_write(PIN_LED_G, PIN_LOW);
        rt_pin_write(PIN_LED_B, PIN_LOW);
        break;
    case IOTB_SENSOR_RGB_CLOSE:
        rt_pin_write(PIN_LED_R, PIN_HIGH);
        rt_pin_write(PIN_LED_G, PIN_HIGH);
        rt_pin_write(PIN_LED_B, PIN_HIGH);
        break;
    case IOTB_SENSOR_RGB_R:
        rt_pin_write(PIN_LED_R, PIN_LOW);
        rt_pin_write(PIN_LED_G, PIN_HIGH);
        rt_pin_write(PIN_LED_B, PIN_HIGH);
        break;
    case IOTB_SENSOR_RGB_G:
        rt_pin_write(PIN_LED_R, PIN_HIGH);
        rt_pin_write(PIN_LED_G, PIN_LOW);
        rt_pin_write(PIN_LED_B, PIN_HIGH);
        break;
    case IOTB_SENSOR_RGB_B:
        rt_pin_write(PIN_LED_R, PIN_HIGH);
        rt_pin_write(PIN_LED_G, PIN_HIGH);
        rt_pin_write(PIN_LED_B, PIN_LOW);
        break;
    case IOTB_SENSOR_RGB_RG:
        rt_pin_write(PIN_LED_R, PIN_LOW);
        rt_pin_write(PIN_LED_G, PIN_LOW);
        rt_pin_write(PIN_LED_B, PIN_HIGH);
        break;
    case IOTB_SENSOR_RGB_RB:
        rt_pin_write(PIN_LED_R, PIN_LOW);
        rt_pin_write(PIN_LED_G, PIN_HIGH);
        rt_pin_write(PIN_LED_B, PIN_LOW);
        break;
    case IOTB_SENSOR_RGB_GB:
        rt_pin_write(PIN_LED_R, PIN_HIGH);
        rt_pin_write(PIN_LED_G, PIN_LOW);
        rt_pin_write(PIN_LED_B, PIN_LOW);
        break;
    case IOTB_SENSOR_RGB_RGB:
        rt_pin_write(PIN_LED_R, PIN_LOW);
        rt_pin_write(PIN_LED_G, PIN_LOW);
        rt_pin_write(PIN_LED_B, PIN_LOW);
        break;
    default:
        break;
    }
    return RT_EOK;
}

/**
 * 3. BEEP sensor
*/
rt_err_t iotb_sensor_beep_ctl(iotb_sensor_beep_t beep_t)
{
    switch (beep_t)
    {
    case IOTB_SENSOR_BEEP_INIT:
        /* set beep pin mode to output */
        rt_pin_mode(PIN_BEEP, PIN_MODE_OUTPUT);
        break;
    case IOTB_SENSOR_BEEP_DEINIT:
        rt_pin_write(PIN_BEEP, PIN_LOW);
        break;
    case IOTB_SENSOR_BEEP_OPEN:
        rt_pin_write(PIN_BEEP, PIN_HIGH);
        break;
    case IOTB_SENSOR_BEEP_CLOSE:
        rt_pin_write(PIN_BEEP, PIN_LOW);
        break;
    default:
        break;
    }

    return RT_EOK;
}

/**
 * 4. MOTOR sensor
*/
rt_err_t iotb_sensor_motor_ctl(iotb_sensor_motor_t motor_t)
{
    switch (motor_t)
    {
    case IOTB_SENSOR_MOTOR_INIT:
        /* set motor pin mode to output */
        rt_pin_mode(PIN_MOTOR_A, PIN_MODE_OUTPUT);
        rt_pin_mode(PIN_MOTOR_B, PIN_MODE_OUTPUT);
        break;
    case IOTB_SENSOR_MOTOR_DEINIT:
        rt_pin_write(PIN_MOTOR_A, PIN_LOW);
        rt_pin_write(PIN_MOTOR_B, PIN_LOW);
        break;
    case IOTB_SENSOR_MOTOR_START:
        rt_pin_write(PIN_MOTOR_A, PIN_HIGH);
        rt_pin_write(PIN_MOTOR_B, PIN_LOW);
        break;
    case IOTB_SENSOR_MOTOR_STOP:
        rt_pin_write(PIN_MOTOR_A, PIN_LOW);
        rt_pin_write(PIN_MOTOR_B, PIN_LOW);
        break;
    default:
        break;
    }

    return RT_EOK;
}

/**
 * 5. ALS/PS sensor
*/
#include "ap3216c.h"
static ap3216c_device_t iotb_ap3216c_dev = RT_NULL;
static uint8_t iotb_ap3216c_inited = 0;
static uint8_t iotb_ap3216c_busy = 0;

rt_err_t iotb_sensor_ap3216c_init(void)
{
    if (iotb_ap3216c_inited)
    {
        return RT_EOK;
    }
    const char *i2c_bus_name = "i2c1";

    if (iotb_ap3216c_dev != RT_NULL)
    {
        iotb_sensor_ap3216c_deinit();
    }

    iotb_ap3216c_busy = 1;

    /* initializes ap3216c, registered device driver */
    iotb_ap3216c_dev = ap3216c_init(i2c_bus_name);
    if (iotb_ap3216c_dev == RT_NULL)
    {
        LOG_E("The sensor initializes failure");
        return -RT_ERROR;
    }
    iotb_ap3216c_inited = 1;
    iotb_ap3216c_busy = 0;

    LOG_I("ap3216c device init success!");

    return RT_EOK;
}

rt_err_t iotb_sensor_ap3216c_deinit(void)
{
    if (iotb_ap3216c_dev == RT_NULL)
    {
        return -RT_ERROR;
    }

    iotb_ap3216c_busy = 1;

    ap3216c_deinit(iotb_ap3216c_dev);
    iotb_ap3216c_dev = RT_NULL;
    iotb_ap3216c_inited = 0;
    iotb_ap3216c_busy = 0;

    return RT_EOK;
}

rt_err_t iotb_sensor_ap3216c_read_als(float *data)
{
    if (iotb_ap3216c_dev == RT_NULL || data == RT_NULL || !iotb_ap3216c_inited)
    {
        return -RT_ERROR;
    }

    if (iotb_ap3216c_busy == 1)
    {
        return -RT_EBUSY;
    }

    iotb_ap3216c_busy = 1;
    *data = ap3216c_read_ambient_light(iotb_ap3216c_dev);
    iotb_ap3216c_busy = 0;

    if (*data == 0.0f)
    {
        return -RT_ERROR;
    }
    else
    {
        return RT_EOK;
    }
}

rt_err_t iotb_sensor_ap3216c_read_ps(uint16_t *data)
{
    if (iotb_ap3216c_dev == RT_NULL || data == RT_NULL || !iotb_ap3216c_inited)
    {
        return -RT_ERROR;
    }

    if (iotb_ap3216c_busy == 1)
    {
        return -RT_EBUSY;
    }

    iotb_ap3216c_busy = 1;
    *data = ap3216c_read_ps_data(iotb_ap3216c_dev);
    iotb_ap3216c_busy = 0;

    return RT_EOK;
}

/**
 * 6. AXIS6 sensor
*/
#include "icm20608.h"
static icm20608_device_t iotb_icm20608_dev = RT_NULL;
static uint8_t iotb_icm20608_inited = 0;
static uint8_t iotb_icm20608_busy = 0;

rt_err_t iotb_sensor_icm20608_init(void)
{
    if (iotb_icm20608_inited)
    {
        return RT_EOK;
    }
    const char *i2c_bus_name = "i2c1";

    if (iotb_icm20608_dev != RT_NULL)
    {
        LOG_I("Retry init icm20608");
        iotb_sensor_icm20608_deinit();
        iotb_icm20608_dev = RT_NULL;
    }
    iotb_icm20608_busy = 1;

    /* initialize icm20608, registered device driver */
    iotb_icm20608_dev = icm20608_init(i2c_bus_name);
    if (iotb_icm20608_dev == RT_NULL)
    {
        LOG_E("icm20608 init failed");
        return -RT_ERROR;
    }

    /* calibrate icm20608 zero level and average 10 times with sampling data */
    if (icm20608_calib_level(iotb_icm20608_dev, 10) != RT_EOK)
    {
        LOG_E("icm20608 calibrate failed");
        return -RT_ERROR;
    }
    iotb_icm20608_busy = 0;
    iotb_icm20608_inited = 1;

    LOG_I("icm20608 device init success!");

    return RT_EOK;
}

rt_err_t iotb_sensor_icm20608_deinit(void)
{
    if (iotb_icm20608_dev == RT_NULL)
    {
        return -RT_ERROR;
    }
    iotb_icm20608_busy = 1;

    icm20608_deinit(iotb_icm20608_dev);
    iotb_icm20608_dev = RT_NULL;
    iotb_icm20608_inited = 0;
    iotb_icm20608_busy = 0;

    return RT_EOK;
}

rt_err_t iotb_sensor_icm20608_read_accel(rt_int16_t *accel_x,
        rt_int16_t *accel_y,
        rt_int16_t *accel_z)
{
    if (!iotb_icm20608_dev || !accel_x || !accel_y || !accel_z)
    {
        return -RT_ERROR;
    }

    if (iotb_icm20608_busy == 1)
    {
        LOG_E("icm20608 read busy!");
        return -RT_EBUSY;
    }

    iotb_icm20608_busy = 1;
    if (icm20608_get_accel(iotb_icm20608_dev, accel_x, accel_y, accel_z) !=
            RT_EOK)
    {
        iotb_icm20608_busy = 0;
        LOG_E("icm20608 read error!");
        return -RT_ERROR;
    }
    iotb_icm20608_busy = 0;

    return RT_EOK;
}

rt_err_t iotb_sensor_icm20608_read_gyro(rt_int16_t *gyro_x,
                                        rt_int16_t *gyro_y,
                                        rt_int16_t *gyro_z)
{
    if (!iotb_icm20608_dev || !gyro_x || !gyro_y || !gyro_z)
    {
        return -RT_ERROR;
    }

    if (iotb_icm20608_busy == 1)
    {
        return -RT_EBUSY;
    }

    iotb_icm20608_busy = 1;
    if (icm20608_get_gyro(iotb_icm20608_dev, gyro_x, gyro_y, gyro_z) != RT_EOK)
    {
        iotb_icm20608_busy = 0;
        LOG_E("icm20608 read error!");
        return -RT_ERROR;
    }
    iotb_icm20608_busy = 0;

    return RT_EOK;
}


rt_err_t iotb_sensor_infrared_send(rt_uint8_t* infrared_keycode)
{
    app_infrared_nec_send(0x00,*infrared_keycode,0);
    return RT_EOK;
}

rt_err_t iotb_sensor_infrared_read(rt_uint8_t *key_value)
{
    return app_infrared_nec_decode(key_value);
}

/**
 * 7. Sensor data handle
*/
static iotb_sensor_data_t iotb_sensor_data_result =
{
    .aht10_data_temp = 0.0f,
    .aht10_data_humi = 0.0f,
    .ap3216_data_als = 0.0f,
    .ap3216_data_ps  = 0,
    .ax = 0, .ay = 0, .az = 0,
    .gx = 0, .gy = 0, .gz = 0,

    .infrared_receive = 0,

    .aht10_temp_status = -RT_ERROR,
    .aht10_humi_status = -RT_ERROR,
    .ap3216_als_status = -RT_ERROR,
    .ap3216_ps_status  = -RT_ERROR,
    .icm20608_accel_status = -RT_ERROR,
    .icm20608_gyro_status  = -RT_ERROR,

    .status          = 0 /* 0: read finish; 1: not finish */
};

void iotb_sensor_data_upload(iotb_sensor_data_t *in_data, iotb_sensor_data_t *out_data)
{
    rt_base_t level = rt_hw_interrupt_disable();

    out_data->aht10_temp_status = in_data->aht10_temp_status;
    out_data->aht10_data_temp = in_data->aht10_data_temp;
    out_data->aht10_humi_status = in_data->aht10_humi_status;
    out_data->aht10_data_humi = in_data->aht10_data_humi;
    out_data->ap3216_als_status = in_data->ap3216_als_status;
    out_data->ap3216_data_als = in_data->ap3216_data_als;
    out_data->ap3216_ps_status = in_data->ap3216_ps_status;
    out_data->ap3216_data_ps = in_data->ap3216_data_ps;
    out_data->icm20608_accel_status = in_data->icm20608_accel_status;
    out_data->icm20608_gyro_status = in_data->icm20608_gyro_status;
    out_data->ax = in_data->ax;
    out_data->ay = in_data->ay;
    out_data->az = in_data->az;
    out_data->gx = in_data->gx;
    out_data->gy = in_data->gy;
    out_data->gz = in_data->gz;

    out_data->infrared_send = in_data->infrared_send;
    out_data->infrared_receive = in_data->infrared_receive;

    rt_hw_interrupt_enable(level);
}

void iotb_sensor_data_read(void *arg)
{
    iotb_sensor_data_t sensor_data;

    iotb_sensor_data_upload(iotb_sensor_data_result_get(), &sensor_data);

    if (iotb_lcd_get_menu_index() == 2)
    {
        if (iotb_aht10_inited == 0)
        {
            LOG_E("aht10 init failed! Retry...");
            rt_thread_mdelay(2000);
            if (iotb_aht10_busy == 0)
            {
                iotb_sensor_aht10_init();
            }
        }
        else
        {
            rt_thread_mdelay(1);
            sensor_data.aht10_temp_status =
                iotb_sensor_aht10_read(0, &sensor_data.aht10_data_temp);

            sensor_data.aht10_humi_status =
                iotb_sensor_aht10_read(1, &sensor_data.aht10_data_humi);
        }

        if (iotb_ap3216c_inited == 0)
        {
            LOG_E("ap3216 init failed! Retry...");
            rt_thread_mdelay(2000);
            if (iotb_ap3216c_inited == 0 && iotb_ap3216c_busy == 0)
            {
                iotb_sensor_ap3216c_init();
            }
        }
        else
        {
            sensor_data.ap3216_als_status =
                iotb_sensor_ap3216c_read_als(&sensor_data.ap3216_data_als);

            sensor_data.ap3216_ps_status =
                iotb_sensor_ap3216c_read_ps(&sensor_data.ap3216_data_ps);
        }
    }
    else if (iotb_lcd_get_menu_index() == 3)
    {
        if (iotb_icm20608_inited == 0)
        {
            LOG_E("icm20608 init failed! Retry...");
            rt_thread_mdelay(2000);
            if (iotb_icm20608_busy == 0)
            {
                iotb_sensor_icm20608_init();
            }
        }
        else
        {
            sensor_data.icm20608_accel_status =
                iotb_sensor_icm20608_read_accel(&sensor_data.ax, &sensor_data.ay, &sensor_data.az);

            sensor_data.icm20608_gyro_status =
                iotb_sensor_icm20608_read_gyro(&sensor_data.gx, &sensor_data.gy, &sensor_data.gz);
        }
    }
    else if (iotb_lcd_get_menu_index() == 6)
    {
        iotb_sensor_infrared_read(&sensor_data.infrared_receive);
        sensor_data.infrared_send++;
        iotb_sensor_infrared_send(&sensor_data.infrared_send);
    }
    else
    {
        return;
    }

    iotb_sensor_data_upload(&sensor_data, iotb_sensor_data_result_get());
}

iotb_sensor_data_t *iotb_sensor_data_result_get(void)
{
    return &iotb_sensor_data_result;
}

static uint16_t iotb_sensor_thr_cycle = IOTB_SENSOR_THR_CYCLE;
void iotb_sensor_thr_set_cycle(uint16_t time)
{
    rt_base_t level = rt_hw_interrupt_disable();
    iotb_sensor_thr_cycle = time;
    rt_hw_interrupt_enable(level);
}

static void _iotb_sensor_read_thr(void *arg)
{
    do
    {
        iotb_sensor_data_read(RT_NULL);
        rt_thread_mdelay(iotb_sensor_thr_cycle);
    } while(1);
}

void iotb_sensor_read_start(void)
{
    rt_thread_t sensor_read_thr = RT_NULL;
    sensor_read_thr = rt_thread_create("sensor",
                                    _iotb_sensor_read_thr,
                                    RT_NULL,
                                    2048, 12, 10);
    if (sensor_read_thr != RT_NULL)
    {
        rt_thread_startup(sensor_read_thr);
    }
}

/**
 * 8. SD CARD filesystem
*/
#include <dfs_fs.h>
#include <dfs_file.h>
static uint8_t iotb_sensor_sd_fs_inited = 0;
rt_err_t iotb_sensor_sdcard_fs_init(void)
{
    if (iotb_sensor_sd_fs_inited)
    {
        return RT_EOK;
    }

    /* mount the file system from tf card */
    if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
    {
        LOG_I("Filesystem initialized!");
        iotb_sensor_sd_fs_inited = 1;
    }
    else
    {
        LOG_E("Failed to initialize filesystem!");
        iotb_sensor_sd_fs_inited = 0;
        return -RT_ERROR;
    }
    return RT_EOK;
}

rt_err_t iotb_sensor_sdcard_fs_deinit(void)
{
    /* mount the file system from tf card */
    if (dfs_unmount("/") == 0)
    {
        LOG_I("Filesystem unmounted");
        iotb_sensor_sd_fs_inited = 0;
    }
    else
    {
        LOG_E("Failed to unmount filesystem!");
        iotb_sensor_sd_fs_inited = 0;
        return -RT_ERROR;
    }
    return RT_EOK;
}

uint8_t iotb_sensor_sdcard_fs_status_get(void)
{
    return iotb_sensor_sd_fs_inited;
}

static rt_bool_t iotb_file_is_exist(const char *filename, rt_uint32_t *file_size)
{
    struct dfs_fd file_fd;
    if (!iotb_sensor_sdcard_fs_status_get())
    {
        LOG_E("sdcard fs not init!");
        return false;
    }

    if (dfs_file_open(&file_fd, filename, O_RDONLY) != 0)
    {
        LOG_E("dfs file open failed!");
        return false;
    }
    *file_size = file_fd.size;
    dfs_file_close(&file_fd);

    return true;
}

__packed typedef struct 
{
    rt_uint8_t fontok;     /* magic word: 0xAA */
    rt_uint32_t ugbkaddr;  /* unigbk Absolute address */
    rt_uint32_t ugbksize;  /* unigbk size */
    rt_uint32_t f12addr;   /* gbk12 Absolute address	*/
    rt_uint32_t gbk12size; /* gbk12 size */
    rt_uint32_t f16addr;   /* gbk16 Absolute address*/
    rt_uint32_t gbk16size; /* gbk16 size */
    rt_uint32_t f24addr;   /* gbk24 Absolute address */
    rt_uint32_t gbk24size; /* gbk24 size*/
    rt_uint32_t f32addr;   /* gbk32 Absolute address */
    rt_uint32_t gbk32size; /* gbk32 size*/
} iotb_font_info_t;

static iotb_font_info_t iotb_font_info;
static rt_err_t iotb_sdcard_upgrade(const char *filename, const struct fal_partition *part, rt_uint32_t addr);

rt_err_t iotb_partition_fontlib_check(void)
{
    rt_err_t rst = RT_EOK;
    const struct fal_partition *font_part;

    if ((font_part = fal_partition_find("font")) != NULL)
    {
        fal_partition_read(font_part, 0, (uint8_t*)&iotb_font_info, sizeof(iotb_font_info_t));
        LOG_D("iotb_font_info_t:%d", sizeof(iotb_font_info_t));
        if (!(iotb_font_info.fontok == 0xAA))
        {
            lcd_set_color(BLACK, WHITE);
            lcd_clear(BLACK);
            lcd_show_string(0, 108 -38, 24, "fontlib not exist");
            lcd_show_string(0, 108 -12, 24, "SDCard Upgrade");
            lcd_show_string(0, 108 + 26 -12, 24, "OR");
            lcd_show_string(0, 108 + 26 + 26 - 12, 24, "Ymodem Upgrade");
            rt_thread_mdelay(2000);
            return -RT_ERROR;
        }
    }
    else
    {
        lcd_set_color(BLACK, WHITE);
        lcd_clear(BLACK);
        lcd_show_string(0, 96, 24, "fontlib partition");
        lcd_show_string(0, 96 + 26, 24, "     not exist      ");
        rt_thread_mdelay(5000);
        return -RT_ERROR;
    }
    rst = RT_EOK;
    return rst;
}

rt_err_t iotb_sdcard_font_upgrade(void)
{
    rt_err_t rst = RT_EOK;
    const struct fal_partition *part = RT_NULL;
    rt_uint32_t font_total_size = 0;
    rt_uint32_t file_size = 0;

    if (!iotb_sensor_sdcard_fs_status_get())
    {
        LOG_E("sdcard fs not init!");
        return -RT_ERROR;
    }

    lcd_set_color(BLACK, WHITE);
    lcd_clear(BLACK);
    lcd_show_string(0, 100, 24,  "will upgrade font");

    if (!iotb_file_is_exist(UNIGBK_PATH, &file_size))
    {
        goto __exit;
    }
    font_total_size += file_size;
    iotb_font_info.ugbksize = file_size;
    iotb_font_info.ugbkaddr = sizeof(iotb_font_info);

    if (!iotb_file_is_exist(GBK12_PATH, &file_size))
    {
        goto __exit;
    }
    font_total_size += file_size;
    iotb_font_info.gbk12size = file_size;
    iotb_font_info.f12addr = iotb_font_info.ugbkaddr + iotb_font_info.ugbksize;

    if (!iotb_file_is_exist(GBK16_PATH, &file_size))
    {
        goto __exit;
    }
    font_total_size += file_size;
    iotb_font_info.gbk16size = file_size;
    iotb_font_info.f16addr = iotb_font_info.f12addr + iotb_font_info.gbk12size;

    if (!iotb_file_is_exist(GBK24_PATH, &file_size))
    {
        goto __exit;
    }
    font_total_size += file_size;
    iotb_font_info.gbk24size = file_size;
    iotb_font_info.f24addr = iotb_font_info.f16addr + iotb_font_info.gbk16size;

    if (!iotb_file_is_exist(GBK32_PATH, &file_size))
    {
        goto __exit;
    }
    font_total_size += file_size;
    iotb_font_info.gbk32size = file_size;
    iotb_font_info.f32addr = iotb_font_info.f24addr + iotb_font_info.gbk24size;

    if ((part = fal_partition_find("font")) == NULL)
    {
        LOG_E("FAL partition (font) find failed! Please flash new bootloader firmware using ST-Utility!");
        return -RT_EEMPTY;
    }

    lcd_show_string(0, 100 + 26, 24,  "is erasing          ");
    if (fal_partition_erase(part, 0, font_total_size + sizeof(iotb_font_info_t)) < 0)
    {
        LOG_E("fal erase failed!");
        rst = -RT_ERROR;
        goto __exit;
    }
    lcd_show_string(0, 100 + 26, 24,  "erase success       ");

    LOG_I("[font] upgrade (%s)", UNIGBK_PATH);
    if (iotb_sdcard_upgrade(UNIGBK_PATH, part, iotb_font_info.ugbkaddr) != RT_EOK)
    {
        LOG_E("(%s) upgrade failed!", UNIGBK_PATH);
        goto __exit;
    }

    LOG_I("[font] upgrade (%s)", GBK12_PATH);
    if (iotb_sdcard_upgrade(GBK12_PATH, part, iotb_font_info.f12addr) != RT_EOK)
    {
        LOG_E("(%s) upgrade failed!", GBK12_PATH);
        goto __exit;
    }

    LOG_I("[font] upgrade (%s)", GBK16_PATH);
    if (iotb_sdcard_upgrade(GBK16_PATH, part, iotb_font_info.f16addr) != RT_EOK)
    {
        LOG_E("(%s) upgrade failed!", GBK16_PATH);
        goto __exit;
    }

    LOG_I("[font] upgrade (%s)", GBK24_PATH);
    if (iotb_sdcard_upgrade(GBK24_PATH, part, iotb_font_info.f24addr) != RT_EOK)
    {
        LOG_E("(%s) upgrade failed!", GBK24_PATH);
        goto __exit;
    }

    LOG_I("[font] upgrade (%s)", GBK32_PATH);
    if (iotb_sdcard_upgrade(GBK32_PATH, part, iotb_font_info.f32addr) != RT_EOK)
    {
        LOG_E("(%s) upgrade failed!", GBK32_PATH);
        goto __exit;
    }

    iotb_font_info.fontok = 0xAA;
    iotb_font_info.ugbkaddr += part->offset;
    iotb_font_info.f12addr += part->offset;
    iotb_font_info.f16addr += part->offset;
    iotb_font_info.f24addr += part->offset;
    iotb_font_info.f32addr += part->offset;

    LOG_D("ugbkaddr:0x%x; fontinfo size:%x", iotb_font_info.ugbkaddr, sizeof(iotb_font_info));

    if (fal_partition_write(part, 0,
        (const uint8_t *)&iotb_font_info,
        sizeof(iotb_font_info)) != sizeof(iotb_font_info))
    {
        LOG_E("fal write failed!");
        iotb_font_info.fontok = 0x0;
        goto __exit;
    }

    lcd_set_color(BLACK, WHITE);
    lcd_clear(BLACK);
    lcd_show_string(0, 100, 24,  "upgrade font success");
    rt_thread_mdelay(2000);

    rst = RT_EOK;
    return rst;

__exit:
    rst = -RT_ERROR;
    return rst;
}

rt_err_t iotb_sdcard_wifi_image_upgrade(void)
{
    rt_err_t rst = RT_EOK;
    rt_uint32_t file_size;
    const struct fal_partition *part = RT_NULL;

    if (!iotb_file_is_exist(WIFI_IMAGE_PATH, &file_size))
    {
        goto __exit;
    }

    if ((part = fal_partition_find("download")) == NULL)
    {
        LOG_E("FAL find failed!");
        goto __exit;
    }

    LOG_D("wifi image size: %d", file_size);
    if (fal_partition_erase(part, 0, file_size) < 0)
    {
        LOG_E("fal erase failed!");
        goto __exit;
    }

    if (iotb_sdcard_upgrade(WIFI_IMAGE_PATH, part, 0) != RT_EOK)
    {
        LOG_E("wifi image upgrade failed!");
        goto __exit;
    }

    /* upgrade new firmware */
    if (rt_ota_check_upgrade() == 1)
    {
        if (rt_ota_upgrade() < 0)
        {
            LOG_E("OTA upgrade failed!");
            goto __exit;
        }
        else
        {
            LOG_I("OTA upgrade success!");

            lcd_set_color(BLACK, WHITE);
            lcd_clear(BLACK);
            lcd_show_string(0, 100, 24, "upgrade wifi success");
            lcd_show_string(0, 100 + 26, 24, "will reboot         ");
            lcd_set_color(WHITE, BLACK);
            fal_partition_erase_all(part);
            rt_thread_mdelay(2);
            extern void rt_hw_cpu_reset(void);
            rt_hw_cpu_reset();
            rt_thread_mdelay(5);
            return RT_EOK;
        }
    }

__exit:
    rst = -RT_ERROR;
    return rst;
}

static rt_err_t iotb_sdcard_upgrade(const char *filename, const struct fal_partition *part, rt_uint32_t addr)
{
    rt_err_t rst = RT_EOK;
    struct dfs_fd file_fd;
    size_t file_size;
    int len, read_len, write_len;
    char *read_buf = RT_NULL;
    char buf[21];

    if (!iotb_sensor_sdcard_fs_status_get())
    {
        LOG_E("sdcard fs not init!");
        return -RT_ERROR;
    }

    if (dfs_file_open(&file_fd, filename, O_RDONLY) != 0)
    {
        LOG_E("dfs file open failed!");
        return -RT_ERROR;
    }

    file_size = file_fd.size;
    LOG_D("filename: %s, size: %d", (char *)filename, file_size);
    if (file_size > part->len)
    {
        LOG_E("File is too large!");
        rst = -RT_ERROR;
        goto __exit;
    }

    read_buf = rt_malloc(1024);
    if (read_buf == RT_NULL)
    {
        return -RT_ERROR;
    }
    rt_memset(read_buf, 0x0, 1024);

    lcd_set_color(BLACK, WHITE);
    lcd_clear(BLACK);
    lcd_show_string(0, 100 - 18, 32,  "upgrading...");

    rt_memset(buf, 0x0, sizeof(buf));
    rt_snprintf(buf, sizeof(buf), "%s", strrchr(filename, '/') + 1);
    lcd_show_string(0, 100 + 16, 24, buf);

    rt_memset(buf, 0x0, sizeof(buf));
    rt_snprintf(buf, sizeof(buf), "size: %dK/%dK", 0, file_fd.size/1024);
    lcd_show_string(0, 100 + 16 + 26, 24, buf);

    write_len = 0;
    do
    {
        if (file_size >= 1024)
        {
            read_len = 1024;
        }
        else
        {
            read_len = file_size;
        }
        len = dfs_file_read(&file_fd, read_buf, read_len);
        if (len != read_len )
        {
            LOG_E("File read failed!");
            rst = -RT_ERROR;
            break;
        }
        file_size -= read_len;

        len = fal_partition_write(part, addr + write_len, (const uint8_t *)read_buf, read_len);
        if (len < 0)
        {
            LOG_E("fal write failed!");
            rst = -RT_ERROR;
            break;
        }
        write_len += read_len;
        rt_memset(read_buf, 0x0, 1024);
        rt_memset(buf, 0x0, sizeof(buf));
        rt_snprintf(buf, sizeof(buf), "size: %dK/%dK", write_len/1024, file_fd.size/1024); /* size: 8192k/8192k */
        lcd_show_string(0, 100 + 16 + 26, 24, buf);

    } while(write_len != file_fd.size);
    lcd_set_color(WHITE, BLACK);

    if (write_len == file_fd.size)
    {
        rst = RT_EOK;
        goto __exit;
    }
    rst = -RT_ERROR;

__exit:
    if (read_buf)
    {
        free(read_buf);
        read_buf = RT_NULL;
    }
    dfs_file_close(&file_fd);

    return rst;
}

/**
 * 9. WiFi manager
*/
#include "drv_wlan.h"
#include <wlan_mgnt.h>
#include "wifi_config.h"
#include <wlan_cfg.h>
#include <wlan_prot.h>
#include <sys/socket.h>
#include <msh.h>

static uint8_t iotb_wifi_inited = 0;
static iotb_wifi_t iotb_wifi_status = IOTB_WIFI_NONE;
static char iotb_wlan_sta_ip[24];

/**
 * The callback of network ready event
 */
static void iotb_wlan_up_handler(int event,
                                 struct rt_wlan_buff *buff,
                                 void *parameter)
{
    uint8_t *ip_addr = (uint8_t *)(buff->data);

    rt_memset(iotb_wlan_sta_ip, 0x0, sizeof(iotb_wlan_sta_ip));
    rt_snprintf(iotb_wlan_sta_ip, sizeof(iotb_wlan_sta_ip),
                "%d.%d.%d.%d", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);

    LOG_I("IP UP! %s", iotb_wlan_sta_ip);

    iotb_wifi_status = IOTB_WIFI_UP;
    /* ntp sync */
    msh_exec("ntp_sync", strlen("ntp_sync"));
}

/**
 * The callback of wlan disconected event
 */
static void iotb_wlan_sta_down_handler(int event,
                                       struct rt_wlan_buff *buff,
                                       void *parameter)
{
    LOG_I("IP Down!");
    rt_memset(iotb_wlan_sta_ip, 0x0, sizeof(iotb_wlan_sta_ip));

    iotb_wifi_status = IOTB_WIFI_DOWN;
}

static int _iotb_sensor_wifi_init(void *arg)
{
    const struct fal_partition *wifi_image_part = RT_NULL;
    iotb_wifi_status = IOTB_WIFI_DOWN;

    /* check wifi image patition */
    if ((wifi_image_part = fal_partition_find(IOTB_SENSOR_WIFI_IMAGE_PARTITION)) != NULL)
    {
        if (rt_ota_part_fw_verify(wifi_image_part) < 0)
        {
            lcd_set_color(BLACK, WHITE);
            lcd_clear(BLACK);
            lcd_show_string(0, 108 -38, 24, "wifi image not exist");
            lcd_show_string(0, 108 -12, 24, "SDCard Upgrade");
            lcd_show_string(0, 108 + 26 -12, 24, "OR");
            lcd_show_string(0, 108 + 26 + 26 - 12, 24, "Ymodem Upgrade");
            rt_thread_mdelay(5000);
            return -RT_ERROR;
        }
    }
    else
    {
        lcd_set_color(BLACK, WHITE);
        lcd_clear(BLACK);
        lcd_show_string(0, 96, 24, "wifi_image partition");
        lcd_show_string(0, 96 + 26, 24, "     not exist      ");
        rt_thread_mdelay(5000);
        // while(1);
        return -RT_ERROR;
    }

    if (rt_hw_wlan_init() != RT_EOK)
    {
        LOG_E("wlan init failed!");
        return -RT_ERROR;
    }

    /* Config the dependencies of the wlan autoconnect function */
    // wlan_autoconnect_init();

    /* Enable wlan auto connect function */
    // rt_wlan_config_autoreconnect(0);

    /* Register wlan event callback */
    rt_wlan_register_event_handler(RT_WLAN_EVT_READY,
                                   iotb_wlan_up_handler,
                                   RT_NULL);
    rt_wlan_register_event_handler(RT_WLAN_EVT_STA_DISCONNECTED,
                                   iotb_wlan_sta_down_handler,
                                   RT_NULL);

    iotb_wifi_inited = 1;
    iotb_sensor_wifi_scan(iotb_sensor_scan_handle_get());
    return RT_EOK;
}

rt_err_t iotb_sensor_wifi_init(void)
{
    rt_err_t rst = RT_EOK;
#ifdef IOTB_WIFI_INIT_BY_THREAD
    rt_thread_t iotb_wifi_tid;
#endif

    if (iotb_wifi_inited)
    {
        rst = RT_EOK;
        return rst;
    }

#ifndef IOTB_WIFI_AUTO_CONNECT
    fal_init();
    easyflash_init();
#endif

    if (_iotb_sensor_wifi_init(RT_NULL) == RT_EOK)
    {
        rst = RT_EOK;
    }
    else
    {
        rst = -RT_ERROR;
    }

    return rst;
}

rt_bool_t iotb_sensor_wifi_isinited(void)
{
    return (iotb_wifi_inited == 1) ? RT_TRUE : RT_FALSE;
}

iotb_wifi_t iotb_sensor_wifi_status_get(void)
{
    return iotb_wifi_status;
}

void iotb_sensor_wifi_status_clr(void)
{
    iotb_wifi_status = IOTB_WIFI_NONE;
}

char *iotb_sensor_wifi_sta_ip_get(void)
{
    return &iotb_wlan_sta_ip[0];
}

static iotb_sensor_scan_data_t iotb_sensor_scan_data =
{
    .status = 0 /* 0: read finish; 1: not finish */
};

void iotb_sensor_wifi_scan(void *arg)
{
    struct rt_wlan_scan_result *scan_result = RT_NULL;

    LOG_D("Work iotb_sensor_wifi_scan start tick:%d", rt_tick_get());
    iotb_sensor_scan_data_t *scan_s = (iotb_sensor_scan_data_t *)arg;

    if (scan_s == NULL)
    {
        return;
    }

    LOG_D("iotb_sensor_wifi_scan!");

    scan_s->status = 1; /* 0: read finish; 1: not finish */

    /* execute synchronous scan function */
    scan_result = rt_wlan_scan_sync();
    if (scan_result)
    {
        scan_s->num = rt_wlan_scan_get_info(scan_s->info, sizeof(scan_s->info)/sizeof(struct rt_wlan_info));
        rt_wlan_scan_result_clean();
    }

    scan_s->status = 0; /* 0: read finish; 1: not finish */
    LOG_D("Work iotb_sensor_wifi_scan finish tick:%d", rt_tick_get());
}

iotb_sensor_scan_data_t iotb_sensor_scan_data_get(void)
{
    return iotb_sensor_scan_data;
}

iotb_sensor_scan_data_t *iotb_sensor_scan_handle_get(void)
{
    return &iotb_sensor_scan_data;
}

/**
 * 10. Smart config
*/
#include <sys/socket.h>
#include "smartconfig.h"

#define NET_READY_TIME_OUT       (rt_tick_from_millisecond(30 * 1000))

static int rt_wlan_device_connetct(char *ssid, char *passwd)
{
    int result = RT_EOK;
    rt_uint8_t time_cnt = 0;
    rt_uint8_t retry_cnt = 10;

    do
    {
        result = rt_wlan_connect(ssid, passwd);
        if (result != RT_EOK)
        {
            LOG_E("connect ssid %s error: %d! Retry cnt: %d", ssid, result, retry_cnt);
            rt_thread_mdelay(3000);
        }
        else
        {
            break;
        }
    } while (retry_cnt--);

    if (retry_cnt == 0)
    {
        result = -RT_ERROR;
        return result;
    }

    do
    {
        rt_thread_mdelay(1000);
        time_cnt ++;
        if (rt_wlan_is_ready())
        {
            break;
        }
    }
    while (time_cnt <= (NET_READY_TIME_OUT / 1000));

    if (time_cnt <= (NET_READY_TIME_OUT / 1000))
    {
        LOG_D("networking ready!");
    }
    else
    {
        LOG_E("wait ip got timeout!");
        result = -RT_ETIMEOUT;
    }

    return result;
}

static void airkiss_send_notification(uint8_t random)
{
    int sock = -1;
    int udpbufsize = 2;
    struct sockaddr_in UDPBCAddr, UDPBCServerAddr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        LOG_E("airkiss notify create socket error!");
        goto _exit;
    }

    UDPBCAddr.sin_family = AF_INET;
    UDPBCAddr.sin_port = htons(10000);
    UDPBCAddr.sin_addr.s_addr = htonl(0xffffffff);
    rt_memset(&(UDPBCAddr.sin_zero), 0, sizeof(UDPBCAddr.sin_zero));

    UDPBCServerAddr.sin_family = AF_INET;
    UDPBCServerAddr.sin_port = htons(10000);
    UDPBCServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    rt_memset(&(UDPBCServerAddr.sin_zero), 0, sizeof(UDPBCServerAddr.sin_zero));

    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &udpbufsize, sizeof(int)) != 0)
    {
        LOG_E("airkiss notify socket setsockopt error!");
        goto _exit;
    }

    if (bind(sock, (struct sockaddr *)&UDPBCServerAddr, sizeof(UDPBCServerAddr)) != 0)
    {
        LOG_E("airkiss notify socket bind error!");
        goto _exit;
    }

    for (int i = 0; i <= 20; i++)
    {
        int ret = sendto(sock, (char *)&random, 1, 0, (struct sockaddr *)&UDPBCAddr, sizeof(UDPBCAddr));
        rt_thread_delay(10);
    }

    LOG_D("airkiss notification thread exit!");

_exit:
    if (sock >= 0)
    {
        closesocket(sock);
    }
}

static iotb_smartconfig_t iotb_smartconfig_s = {
    .iotb_smartconfig_is_finished = 0,
    .iotb_smartconfig_join_is_succ = 0
};

static int smartconfig_result(rt_smartconfig_type result_type, char *ssid, char *passwd, void *userdata, rt_uint8_t userdata_len)
{
    rt_uint8_t random = *(rt_uint8_t *)userdata;

    LOG_I("type:%d\n", result_type);
    LOG_I("ssid:%s\n", ssid);
    LOG_I("passwd:%s\n", passwd);
    LOG_I("user_data:0x%02x\n", random);
    
    iotb_smartconfig_s.iotb_smartconfig_is_finished = 1;
    iotb_smartconfig_s.iotb_smartconfig_join_is_succ = 0;

    rt_memset(iotb_smartconfig_s.ssid, 0x0, sizeof(iotb_smartconfig_s.ssid));
    rt_memset(iotb_smartconfig_s.pwd, 0x0, sizeof(iotb_smartconfig_s.pwd));

    rt_snprintf(iotb_smartconfig_s.ssid, sizeof(iotb_smartconfig_s.ssid), "%s", ssid);
    rt_snprintf(iotb_smartconfig_s.pwd, sizeof(iotb_smartconfig_s.pwd), "%s", passwd);

    if (rt_wlan_device_connetct(ssid, passwd) == RT_EOK)
    {
        airkiss_send_notification(random);
        iotb_smartconfig_s.iotb_smartconfig_join_is_succ = 1;
    }
    else
    {
        iotb_smartconfig_s.iotb_smartconfig_join_is_succ = 2;
    }

    return 0;
}

static volatile uint8_t smart_cfg_lock_channel = 0;
static volatile uint8_t smart_cfg_success = 0;

rt_bool_t iotb_smartconfig_is_started(void)
{
    return (smart_cfg_lock_channel == 1)? RT_TRUE : RT_FALSE;
}

void iotb_smartconfig_is_started_clr(void)
{
    smart_cfg_lock_channel = 0;
}

rt_bool_t iotb_smartconfig_complete_get(void)
{
    return (smart_cfg_success == 1)? RT_TRUE : RT_FALSE;
}

void iotb_smartconfig_complete_clr(void)
{
    smart_cfg_success = 0;
}

static void iotb_smartconfig_event_handle(rt_uint32_t event, void *userdata)
{
    rt_uint32_t recv_event = event;

    if (recv_event & SMARTCONFIG_EVENT_CHANGE_CHANNEL)
    {
        LOG_I("SMARTCONFIG_EVENT_CHANGE_CHANNEL");
    }

    if (recv_event & SMARTCONFIG_EVENT_LOCKED_CHANNEL)
    {
        LOG_I("SMARTCONFIG_EVENT_LOCKED_CHANNEL");
        smart_cfg_lock_channel = 1;
        smart_cfg_success = 0;
    }

    if (recv_event & SMARTCONFIG_EVENT_COMPLETE)
    {
        LOG_I("SMARTCONFIG_EVENT_COMPLETE");
        smart_cfg_lock_channel = 0;
        smart_cfg_success = 1;
    }

    if (recv_event & SMARTCONFIG_EVENT_RESTART)
    {
        LOG_I("SMARTCONFIG_EVENT_RESTART");
    }

    if (recv_event & SMARTCONFIG_EVENT_STOP)
    {
        LOG_I("SMARTCONFIG_EVENT_STOP");
    }
}

iotb_smartconfig_t *iotb_smartconfig_result_get(void)
{
    return &iotb_smartconfig_s;
}

void iotb_smartconfig_start(void)
{
    // rt_wlan_config_autoreconnect(0); /* disable auto reconnect */
    iotb_sensor_wifi_status_clr();
    iotb_smartconfig_is_started_clr();
    iotb_smartconfig_complete_clr();

    rt_smartconfig_set_event_handle(iotb_smartconfig_event_handle, RT_NULL);
    LOG_I("smartconfig start");
    rt_smartconfig_start(SMARTCONFIG_TYPE_AIRKISS, SMARTCONFIG_ENCRYPT_NONE, RT_NULL, smartconfig_result);
    LOG_I("smartconfig has started");
}

void iotb_smartconfig_stop(void)
{
    rt_smartconfig_stop();
    // rt_wlan_config_autoreconnect(1);
    iotb_smartconfig_is_started_clr();
    iotb_smartconfig_complete_clr();
}

/**
 * 11. Web Data get
*/
#include <sys/socket.h>
#include "netdb.h"
rt_err_t iotb_web_api_get(const char *url, int port, const char *send_data, char *recv_data, rt_size_t size)
{
    rt_err_t rst = RT_EOK;
    struct sockaddr_in server_addr;
    int sock = -1, bytes_received;
    int retry_cnt = 5;

    struct hostent *host;
    if (url == RT_NULL || send_data == RT_NULL || recv_data == RT_NULL)
    {
        LOG_E("url is null.");
        return -RT_ERROR;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        LOG_E("Socket create error");
        return -RT_ERROR;
    }

    host = gethostbyname(url);
    if (!host)
    {
        LOG_E("gethostbyname fail!");
        rst = -RT_ERROR;
        goto __exit;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        LOG_E("Connect fail!");
        rst = -RT_ERROR;
        goto __exit;
    }

    LOG_D("web api get connect success!");

    if (send(sock, send_data, strlen(send_data), 0) <= 0)
    {
        LOG_E("send return <= 0");
        rst = -RT_ERROR;
        goto __exit;
    }

    while (retry_cnt--)
    {
        bytes_received = recv(sock, recv_data, size - 1, 0);
        LOG_D("bytes_received : %d", bytes_received);

        if (bytes_received <= 0)
        {
            LOG_E("socket recv err.");
            rst = -RT_ERROR;
            goto __exit;
        }
        else
        {
            recv_data[bytes_received] = '\0';
            break;
        }
    }

__exit:
    if (sock != -1)
    {
        closesocket(sock);
        sock = -1;
    }
    return rst;
}

rt_err_t iotb_web_api_wan_get(char *wan_ip, rt_size_t size)
{
#define RECV_SIZE 300
    char recv_data[RECV_SIZE];
    char *wan;

    const char send_data[] =
        "GET /ip HTTP/1.0\r\n"
        "Host: www.httpbin.org\r\n\r\n"
        ;
    if (iotb_web_api_get("www.httpbin.org", 80 , send_data , recv_data, RECV_SIZE) == RT_EOK)
    {
        wan = rt_strstr(recv_data, "\": \"") + 4;
        *rt_strstr(wan, "\"") = '\0';
        rt_strncpy(wan_ip, wan, size);
        LOG_I(wan);
    }
    else
    {
        wan_ip = "0.0.0.0";
        return -RT_ERROR;
    }
    return RT_EOK;
}

/**
 * 12. pm
*/
#include <drv_wakeup.h>
#include "drivers/pm.h"
static uint8_t iotb_pm_started = 0; /* 0: not start; 1: started */

static void iotb_pm_wakeup_callback(void)
{
    iotb_event_msg_t msg;

    if (iotb_pm_started == 1)
    {
        iotb_pm_started = 0;

        iotb_key_thr_set_cycle(IOTB_KEY_SCAN_CYCLE);
        iotb_event_thr_set_cycle(IOTB_EVENT_THR_CYCLE);
        iotb_sensor_thr_set_cycle(IOTB_SENSOR_THR_CYCLE);
        iotb_lcd_thr_set_cycle(IOTB_LCD_THR_CYCLE);

        iotb_event_put_set_enable();
        iotb_btn_scan_enable();

        msg.event.event_src = IOTB_EVENT_SRC_PM;
        msg.event.event_type = IOTB_EVENT_TYPE_PM_WKUP;
        iotb_event_put(&msg);
    }
}

static void iotb_pm_wakeup_init(void)
{
    bsp_register_wakeup(iotb_pm_wakeup_callback);
}

static void iotb_pm_mode_init(void)
{
    rt_pm_request(PM_SLEEP_MODE_NONE);
}

void iotb_pm_init(void)
{
    iotb_pm_wakeup_init();
    iotb_pm_mode_init();
}

void iotb_pm_start(void)
{
    if (iotb_pm_started)
    {
        return;
    }

    lcd_display_off();

    iotb_btn_scan_disable();
    iotb_key_thr_set_cycle(1000);
    iotb_event_thr_set_cycle(200);
    iotb_sensor_thr_set_cycle(1000);
    iotb_lcd_thr_set_cycle(1000);
    rt_wlan_set_powersave(1);

    iotb_event_put_set_disable();

    rt_pm_request(PM_SLEEP_MODE_DEEP);
    rt_pm_release(PM_SLEEP_MODE_NONE);

    iotb_pm_started = 1;
}

uint8_t iotb_pm_status_get(void)
{
    return iotb_pm_started;
}

void iotb_pm_exit(void)
{
    rt_pm_request(PM_SLEEP_MODE_NONE);
    rt_pm_release(PM_SLEEP_MODE_DEEP);

    lcd_display_on();
    rt_wlan_set_powersave(0);
}

/**
 * 13. RT-Thread cloud
*/
#ifdef IOTB_USING_RTT_CLOUD

static int iotb_rtcld_save_devinfo(struct cld_dev_info *dev_info)
{
    if (ef_set_env(CLD_ENV_USERNAME, dev_info->username) != EF_NO_ERR
            || ef_set_env(CLD_ENV_PASSWORD, dev_info->password) != EF_NO_ERR
            || ef_set_env(CLD_ENV_DEVICE_ID, dev_info->device_id) != EF_NO_ERR
            || ef_set_env(CLD_ENV_DEVICE_KEY, dev_info->device_key) != EF_NO_ERR
            || ef_set_env(CLD_ENV_PRODUCT_ID, dev_info->product_id) != EF_NO_ERR
            || ef_set_env(CLD_ENV_SN, dev_info->sn) != EF_NO_ERR
            || ef_set_env(CLD_ENV_MAJOR_VER, "0.0") != EF_NO_ERR)
    {
        LOG_E("Set device information to easyflash failed! Set environment error!");
        return -1;
    }

    if (ef_save_env() != EF_NO_ERR)
    {
        LOG_E("Set device information to easyflash failed! save environment error!");
        return -1;
    }

    if (ef_set_and_save_env(CLD_ENV_ACTIVATE, "true") != EF_NO_ERR)
    {
        LOG_E("Check device information failed! Set and save  devinfo_activated environment error!");
        return -2;
    }

    return 0;
}

/**
 * return product id + device id + device key
*/
static rt_err_t iotb_rt_cld_get_dev_info_by_sn(char *sn, struct cld_dev_info *dev_info)
{
    int ret = 0, len = 0, resp_status;
    struct webclient_session* session = RT_NULL;
    char uri[128];
    unsigned char *resp_body = RT_NULL;

    cJSON *bstr = RT_NULL;
    const char *productid = RT_NULL, *deviceid = RT_NULL, *devicekey = RT_NULL;
    unsigned char password[16] = { 0 };

    RT_ASSERT(sn != RT_NULL);
    RT_ASSERT(dev_info != RT_NULL);

    rt_memset(uri, 0x0, sizeof(uri));
    rt_snprintf(uri, sizeof(uri), "%s%s", IOTB_RTCLD_GET_DEV_INFO_BY_SN_URI, sn);

    ret = RT_EOK;
    /* create webclient session and set header response size */
    session = webclient_session_create(IOTB_RT_CLD_HEADER_SIZE);
    if (!session)
    {
        LOG_E("open uri failed.");
        ret = -RT_ERROR;
        goto __exit;
    }

    /* send GET request by default header */
    if ((resp_status = webclient_get(session, uri)) != 200)
    {
        LOG_E("webclient GET request failed, response(%d) error.", resp_status);
        ret = -RT_ERROR;
        goto __exit; /* TODO Retry */
    }

    if (session->content_length == 0)
    {
        goto __exit;
    }

    LOG_I("RT-Cloud response data len: %d", session->content_length);

    resp_body = rt_malloc(256);
    if (!resp_body)
    {
        ret = -RT_ERROR;
        goto __exit;
    }
    rt_memset(resp_body, 0x0, 256);
    len = webclient_read(session, (unsigned char *)resp_body, 256);
    if(len < 0)
    {
        LOG_E("Read data from RT-Cld failed (%d)", len);
        ret = -RT_ERROR;
        LOG_E("Get RTT Cloud Device ID failed!");
        goto __exit;
    }

    LOG_I("RT-Cld resp data: %s", resp_body);

    /* handle response data (json) product id + device id + device key */
    bstr = cJSON_Parse((const char *) resp_body);
    if (!bstr)
    {
        LOG_E("cJSON parse error return NULL !");
        ret = -RT_ERROR;
        goto __exit;
    }

    productid = cJSON_item_get_string(bstr, "ProductId");
    deviceid = cJSON_item_get_string(bstr, "DeviceId");
    devicekey = cJSON_item_get_string(bstr, "DeviceKey");
    if (!productid || !deviceid || !devicekey)
    {
        ret = -RT_ERROR;
        LOG_E("Get RTT Cloud Device ID failed!");
        goto __exit;
    }

    if (strlen(productid) < 1)
    {
        ret = -RT_ERROR;
        LOG_E("Get RTT Cloud Device Info failed!");
        goto __exit;
    }

    rt_memcpy(dev_info->product_id, productid, CLD_PRODUCT_ID_MAX_LEN);
    rt_memcpy(dev_info->device_id, deviceid, CLD_DEV_ID_MAX_LEN);
    rt_memcpy(dev_info->device_key, devicekey, CLD_DEV_KEY_MAX_LEN);
    rt_memcpy(dev_info->sn, sn, rt_strlen(sn));

    rt_memcpy(dev_info->username, dev_info->device_id, CLD_DEV_ID_MAX_LEN);
    tiny_md5((unsigned char *) dev_info->device_key, rt_strlen(dev_info->device_key), password);
    for (int i = 0; i < 16; i++)
    {
        rt_snprintf(dev_info->password + i * 2, 2 + 1, "%02x", password[i]);
    }

    cld_port_set_product_id((char*)dev_info->product_id);

    iotb_rtcld_save_devinfo(dev_info);

    /* connect */
    rt_cld_sdk_init();

__exit:
    if (session != RT_NULL)
        webclient_close(session);

    if (bstr)
    {
        cJSON_Delete(bstr);
        bstr = RT_NULL;
    }

    if (resp_body)
    {
        rt_free(resp_body);
        resp_body = RT_NULL;
    }

    return ret;
}

rt_bool_t iotb_rtcld_env_check(void)
{
    char *cld_usr = RT_NULL;
    char *cld_pwd = RT_NULL;
    char *cld_did = RT_NULL;
    char *cld_dkey = RT_NULL;
    char *cld_pid = RT_NULL;
    char *cld_sn = RT_NULL;
    char *cld_dev_mver = RT_NULL;
    char *cld_act = RT_NULL;

    cld_usr  = ef_get_env(CLD_ENV_USERNAME);
    cld_pwd  = ef_get_env(CLD_ENV_PASSWORD);
    cld_did  = ef_get_env(CLD_ENV_DEVICE_ID);
    cld_dkey = ef_get_env(CLD_ENV_DEVICE_KEY);
    cld_pid  = ef_get_env(CLD_ENV_PRODUCT_ID);
    cld_sn   = ef_get_env(CLD_ENV_SN);
    cld_dev_mver = ef_get_env(CLD_ENV_MAJOR_VER);
    cld_act = ef_get_env(CLD_ENV_ACTIVATE);

    if (!(cld_usr && cld_pwd && cld_did && cld_dkey && cld_pid && cld_sn && cld_dev_mver && cld_act))
    {
        return RT_FALSE;
    }
    if (!(rt_strlen(cld_usr) > 0 && rt_strlen(cld_pwd) > 0 && rt_strlen(cld_did) > 0 && \
        rt_strlen(cld_dkey) > 0 && rt_strlen(cld_pid) > 0 && rt_strlen(cld_sn) > 0 && \
        rt_strlen(cld_dev_mver) > 0 && rt_strlen(cld_act) > 0))
    {
        return RT_FALSE;
    }
    return RT_TRUE;
}

/**
 * deactive = 0, active = 1
*/
rt_bool_t iotb_rtcld_active_status_get(void)
{
    char *activate = RT_NULL;
    char *sn = RT_NULL;
    char *id = RT_NULL;

    /* activate device and set information */
    activate = ef_get_env(CLD_ENV_ACTIVATE);
    if (!activate)
    {
        ef_set_and_save_env(CLD_ENV_ACTIVATE, "false");
        LOG_I("The device has not been activated, now will activate!");
        return RT_FALSE;
    }
    else
    {
        if ((rt_memcmp(activate, "true", rt_strlen(activate)) == 0) && \
            iotb_rtcld_env_check())
        {
            LOG_I("The device has been activated successfully!");
            sn = ef_get_env(CLD_ENV_SN);
            id = ef_get_env(CLD_ENV_PRODUCT_ID);
            cld_port_set_device_sn(sn);
            cld_port_set_product_id(id);
            return RT_TRUE;
        }
        else
        {
            LOG_I("Please activate your device!");
            return RT_FALSE;
        }
    }
}

void iotb_rtcld_init_thr(void *arg)
{
    char sn[CLD_SN_MAX_LEN + 1];
    uint32_t event = 0;
    uint8_t is_start = 0;

    /* wait for net connected */
    while(iotb_sensor_wifi_status_get() != IOTB_WIFI_UP)
    {
        rt_thread_mdelay(2000);
    }

    if (iotb_rtcld_active_status_get())
    {
        /* connect */
        rt_cld_sdk_init();
        return;
    }

    LOG_I("iotb RT-Cloud Get Dev Info!");

    cld_port_get_device_sn(sn);
    if (iotb_rt_cld_get_dev_info_by_sn(sn, &iotb_rtcld_dev_info) == RT_EOK)
    {
        return;
    }

    while(1)
    {
        if (iotb_lcd_event_get(IOTB_LCD_EVENT_START_DEV_INFO_GET | IOTB_LCD_EVENT_STOP_DEV_INFO_GET, &event, 1, 0) == RT_EOK)
        {
            if (event & IOTB_LCD_EVENT_START_DEV_INFO_GET)
            {
                LOG_I("rtcld recv START event");
                is_start = 1;
            }

            if (event & IOTB_LCD_EVENT_STOP_DEV_INFO_GET)
            {
                LOG_I("rtcld recv STOP event");
                is_start = 0;
            }
        }
        if (is_start)
        {
            if (iotb_rt_cld_get_dev_info_by_sn(sn, &iotb_rtcld_dev_info) == RT_EOK)
            {
                break;
            }
        }
        rt_thread_mdelay(5000);
    }
}

void iotb_rt_cld_init()
{
    rt_thread_t iotb_cld_thr = RT_NULL;
    rt_uint32_t cpuid[3] = {0};
    char uid[32];

    /* get stm32 uid */
    rt_get_cpu_id(cpuid);
    memset(uid, 0x0, sizeof(uid));
    rt_snprintf(uid, sizeof(uid), "%08x%08x%08x", cpuid[0], cpuid[1], cpuid[2]);
    LOG_I("MCU ID: %s", uid);

    cld_port_set_device_sn((char *)uid);

    /* create thread */
    iotb_cld_thr = rt_thread_create("iotb_cld",
                                    iotb_rtcld_init_thr,
                                    RT_NULL,
                                    4096, 12, 10);
    if (iotb_cld_thr != RT_NULL)
    {
        rt_thread_startup(iotb_cld_thr);
    }
}
#endif

void iotb_init(void *arg)
{
    /* Sensor data can be read after one second of power-on */
    app_infrared_init();
    rt_thread_mdelay(100);
    iotb_sensor_aht10_init();
    rt_thread_mdelay(100);
    iotb_sensor_ap3216c_init();
    rt_thread_mdelay(100);
    iotb_sensor_icm20608_init();
    rt_thread_mdelay(100);
    rt_hw_audio_init("i2c1");
    iotb_wav_player_init(iotb_music_finished_cb);

    iotb_pm_init();
    iotb_sensor_read_start();
#ifdef IOTB_USING_RTT_CLOUD
    iotb_rt_cld_init();
#endif
}
