/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-17     MurphyZhao   first implementation
 */

#ifndef __IOTB_SENSOR_H__
#define __IOTB_SENSOR_H__
#include <rtthread.h>
#include <stdint.h>
#include <rtdevice.h>

/**
 * 1. RGB sensor
*/
typedef enum
{
    IOTB_SENSOR_RGB_INIT   = 0,
    IOTB_SENSOR_RGB_DEINIT = 1,
    IOTB_SENSOR_RGB_OPEN   = 2,
    IOTB_SENSOR_RGB_R      = 3,
    IOTB_SENSOR_RGB_G      = 4,
    IOTB_SENSOR_RGB_B      = 5,
    IOTB_SENSOR_RGB_RG     = 6,
    IOTB_SENSOR_RGB_RB     = 7,
    IOTB_SENSOR_RGB_GB     = 8,
    IOTB_SENSOR_RGB_RGB    = 9,
    IOTB_SENSOR_RGB_CLOSE  = 10,
    IOTB_SENSOR_RGB_MAX    = 11
} iotb_sensor_rgb_t;

/**
 * 2. BEEP sensor
*/
typedef enum
{
    IOTB_SENSOR_BEEP_INIT,
    IOTB_SENSOR_BEEP_DEINIT,
    IOTB_SENSOR_BEEP_OPEN,
    IOTB_SENSOR_BEEP_CLOSE
} iotb_sensor_beep_t;

/**
 * 3. MOTOR sensor
*/
typedef enum
{
    IOTB_SENSOR_MOTOR_INIT,
    IOTB_SENSOR_MOTOR_DEINIT,
    IOTB_SENSOR_MOTOR_START,
    IOTB_SENSOR_MOTOR_STOP
} iotb_sensor_motor_t;

/**
 * 4. WiFi status
*/
typedef enum
{
    IOTB_WIFI_NONE,
    IOTB_WIFI_UP,
    IOTB_WIFI_DOWN,
    IOTB_WIFI_MAX
} iotb_wifi_t;

/**
 * 5. aht10/ap3216/icm20608 sensor data & status
*/
typedef struct
{
    float aht10_data_temp;
    rt_err_t aht10_temp_status;

    float aht10_data_humi;
    rt_err_t aht10_humi_status;

    float ap3216_data_als;
    rt_err_t ap3216_als_status;

    uint16_t ap3216_data_ps;
    rt_err_t ap3216_ps_status;

    rt_int16_t ax;
    rt_int16_t ay;
    rt_int16_t az;
    rt_err_t icm20608_accel_status;

    rt_int16_t gx;
    rt_int16_t gy;
    rt_int16_t gz;
    rt_err_t icm20608_gyro_status;

    rt_uint8_t infrared_receive;
    rt_uint8_t infrared_send;

    uint16_t status; /* 0: read finish; 1: not finish */

    uint8_t reserved;
} iotb_sensor_data_t;

/**
 * 6. WiFi scan data
*/
typedef struct
{
    struct rt_wlan_info info[7];
    rt_int32_t num;

    uint16_t status; /* 0: read finish; 1: not finish */
} iotb_sensor_scan_data_t;

/**
 * 7. smartconfig status
*/
typedef struct
{
    uint8_t iotb_smartconfig_is_finished;  /* 1: finish */
    uint8_t iotb_smartconfig_join_is_succ; /* 0: default; 1: succ; 2: fail */
    char ssid[33];
    char pwd[64];
} iotb_smartconfig_t;

#define IOTB_SENSOR_THR_CYCLE (500)
#define IOTB_SENSOR_WIFI_IMAGE_PARTITION "wifi_image"

#define WIFI_IMAGE_PATH "/SYSTEM/WIFI/wifi_image_1.0.rbl"
#define MUSIC_PATH "/SYSTEM/MUSIC/"
#define MUSIC_PATH_MAX_LEN (128u)

#define GBK32_PATH  "/SYSTEM/FONT/GBK32.FON"
#define GBK24_PATH  "/SYSTEM/FONT/GBK24.FON"
#define GBK16_PATH  "/SYSTEM/FONT/GBK16.FON"
#define GBK12_PATH  "/SYSTEM/FONT/GBK12.FON"
#define UNIGBK_PATH "/SYSTEM/FONT/UNIGBK.BIN"

/* aht10 */
rt_err_t iotb_sensor_aht10_init(void);
rt_err_t iotb_sensor_aht10_deinit(void);
rt_err_t iotb_sensor_aht10_read(uint8_t ops, float *data);

/* rgb */
rt_err_t iotb_sensor_rgb_ctl(iotb_sensor_rgb_t rgb_t);

/* beep */
rt_err_t iotb_sensor_beep_ctl(iotb_sensor_beep_t beep_t);

/* motor */
rt_err_t iotb_sensor_motor_ctl(iotb_sensor_motor_t motor_t);

/* ap3216c */
rt_err_t iotb_sensor_ap3216c_init(void);
rt_err_t iotb_sensor_ap3216c_deinit(void);
rt_err_t iotb_sensor_ap3216c_read_als(float *data);
rt_err_t iotb_sensor_ap3216c_read_ps(uint16_t *data);

/* icm20608 */
rt_err_t iotb_sensor_icm20608_init(void);
rt_err_t iotb_sensor_icm20608_deinit(void);
rt_err_t iotb_sensor_icm20608_read_accel(rt_int16_t *accel_x,
        rt_int16_t *accel_y,
        rt_int16_t *accel_z);
rt_err_t iotb_sensor_icm20608_read_gyro(rt_int16_t *gyro_x,
                                        rt_int16_t *gyro_y,
                                        rt_int16_t *gyro_z);

/* sensor data handle */
void iotb_sensor_data_read(void *arg);
iotb_sensor_data_t *iotb_sensor_data_result_get(void);
void iotb_sensor_data_upload(iotb_sensor_data_t *in_data, iotb_sensor_data_t *out_data);

/* filesystem */
rt_err_t iotb_sensor_sdcard_fs_init(void);
rt_err_t iotb_sensor_sdcard_fs_deinit(void);
uint8_t iotb_sensor_sdcard_fs_status_get(void);

/* wifi */
rt_err_t iotb_sensor_wifi_init(void);
iotb_wifi_t iotb_sensor_wifi_status_get(void);
rt_bool_t iotb_sensor_wifi_isinited(void);
void iotb_sensor_wifi_status_clr(void);
char* iotb_sensor_wifi_sta_ip_get(void);
void iotb_sensor_wifi_scan(void *arg);
iotb_sensor_scan_data_t iotb_sensor_scan_data_get(void);
iotb_sensor_scan_data_t *iotb_sensor_scan_handle_get(void);

/* web data handle */
rt_err_t iotb_web_api_get(const char *url, int port,const char* send_data,char *recv_data,rt_size_t size);
rt_err_t iotb_web_api_wan_get(char* wan_ip,rt_size_t size);

/* smartconfig */
void iotb_smartconfig_start(void);
iotb_smartconfig_t *iotb_smartconfig_result_get(void);
void iotb_smartconfig_stop(void);
rt_bool_t iotb_smartconfig_is_started(void);
void iotb_smartconfig_is_started_clr(void);
rt_bool_t iotb_smartconfig_complete_get(void);
void iotb_smartconfig_complete_clr(void);

/* pm */
void iotb_pm_init(void);
void iotb_pm_start(void);
void iotb_pm_exit(void);
uint8_t iotb_pm_status_get(void);

/* sdcard upgrade */
rt_err_t iotb_partition_fontlib_check(void);
rt_err_t iotb_sdcard_wifi_image_upgrade(void);
rt_err_t iotb_sdcard_font_upgrade(void);

/* rtcloud */
#ifdef IOTB_USING_RTT_CLOUD
rt_bool_t iotb_rtcld_active_status_get(void);
#endif

/* init */
void iotb_init(void *arg);

#endif
