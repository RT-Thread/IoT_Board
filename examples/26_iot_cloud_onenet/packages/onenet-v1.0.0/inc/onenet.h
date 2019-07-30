/*
 * File      : onenet.h
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-04-24     chenyong     first version
 */
#ifndef _ONENET_H_
#define _ONENET_H_

#include <rtthread.h>

#include <cJSON.h>

#define ONENET_DEBUG                   1

#define ONENET_SW_VERSION              "1.0.0"

#ifndef ONENET_MALLOC
#define ONENET_MALLOC                  rt_malloc
#endif

#ifndef ONENET_CALLOC
#define ONENET_CALLOC                  rt_calloc
#endif

#ifndef ONENET_FREE
#define ONENET_FREE                    rt_free
#endif

/* error level log */
#ifdef  log_e
#undef  log_e
#endif
#define log_e(...)                     rt_kprintf("\033[31;22m[E/ONENET] (%s:%d) ", __FUNCTION__, __LINE__);rt_kprintf(__VA_ARGS__);rt_kprintf("\033[0m\n")

/* info level log */
#ifdef  log_i
#undef  log_i
#endif
#define log_i(...)                     rt_kprintf("\033[36;22m[I/ONENET] ");                                rt_kprintf(__VA_ARGS__);rt_kprintf("\033[0m\n")

#if ONENET_DEBUG
#ifdef assert
#undef assert
#endif
#define assert(EXPR)                                                           \
if (!(EXPR))                                                                   \
{                                                                              \
    rt_kprintf("(%s) has assert failed at %s.\n", #EXPR, __FUNCTION__);        \
    while (1);                                                                 \
}

/* debug level log */
#ifdef  log_d
#undef  log_d
#endif
#define log_d(...)                     rt_kprintf("[D/ONENET] (%s:%d) ", __FUNCTION__, __LINE__);           rt_kprintf(__VA_ARGS__);rt_kprintf("\n")

#else

#ifdef assert
#undef assert
#endif
#define assert(EXPR)                   ((void)0);

/* debug level log */
#ifdef  log_d
#undef  log_d
#endif
#define log_d(...)
#endif /* ONENET_DEBUG */

#ifndef ONENET_MQTT_SUBTOPIC
#define ONENET_MQTT_SUBTOPIC           "/topic_test"
#endif

#define ONENET_SERVER_URL              "tcp://183.230.40.39:6002"
#define ONENET_INFO_DEVID_LEN          16
#define ONENET_INFO_APIKEY_LEN         32
#define ONENET_INFO_PROID_LEN          16
#define ONENET_INFO_AUTH_LEN           64
#define ONENET_INFO_NAME_LEN           64
#define ONENET_INFO_URL_LEN            32

#define ONENET_DATASTREAM_NAME_MAX     32

struct rt_onenet_info
{
    char device_id[ONENET_INFO_DEVID_LEN];
    char api_key[ONENET_INFO_APIKEY_LEN];

    char pro_id[ONENET_INFO_PROID_LEN];
    char auth_info[ONENET_INFO_AUTH_LEN];

    char server_uri[ONENET_INFO_URL_LEN];

};
typedef struct rt_onenet_info *rt_onenet_info_t;

/* onenet datastream info */
struct rt_onenet_ds_info
{
    char id[ONENET_DATASTREAM_NAME_MAX];
    char tags[ONENET_DATASTREAM_NAME_MAX];

    char update_time[ONENET_DATASTREAM_NAME_MAX];
    char create_time[ONENET_DATASTREAM_NAME_MAX];

    char unit[ONENET_DATASTREAM_NAME_MAX];
    char unit_symbol[ONENET_DATASTREAM_NAME_MAX];

    char current_value[ONENET_DATASTREAM_NAME_MAX];

};
typedef struct rt_onenet_ds_info *rt_onenet_ds_info_t;

/* OneNET MQTT initialize. */
int onenet_mqtt_init(void);

/* Publish MQTT data to subscribe topic. */
rt_err_t onenet_mqtt_publish(const char *topic, const uint8_t *msg, size_t len);

#ifdef RT_USING_DFS
/* Publish MQTT binary data to onenet by path. */
rt_err_t onenet_mqtt_upload_bin_by_path(const char *ds_name, const char *bin_path);
#endif

/* Publish MQTT binary data to onenet. */
rt_err_t onenet_mqtt_upload_bin(const char *ds_name, uint8_t *bin, size_t len);

/* Publish MQTT string data to onenet. */
rt_err_t onenet_mqtt_upload_string(const char *ds_name, const char *str);
/* Publish MQTT digit data to onenet. */
rt_err_t onenet_mqtt_upload_digit(const char *ds_name, const double digit);

/* Device send data to OneNET cloud. */
rt_err_t onenet_http_upload_digit(const char *ds_name, const double digit);
rt_err_t onenet_http_upload_string(const char *ds_name, const char *str);

#ifdef ONENET_USING_AUTO_REGISTER
/* Register a device to OneNET cloud. */
rt_err_t onenet_http_register_device(const char *dev_name, const char *auth_info);
#endif

/* get a datastream from OneNET cloud. */
rt_err_t onenet_http_get_datastream(const char *ds_name, struct rt_onenet_ds_info *datastream);
/* get datapoints from OneNET cloud. Returned cJSON need to be free when user finished using the data. */
cJSON *onenet_get_dp_by_limit(char *ds_name, size_t limit);
cJSON *onenet_get_dp_by_start_end(char *ds_name, uint32_t start, uint32_t end, size_t limit);
cJSON *onenet_get_dp_by_start_duration(char *ds_name, uint32_t start, size_t duration, size_t limit);
/* Set the command response callback function. User needs to malloc memory for response data. */
void onenet_set_cmd_rsp_cb(void(*cmd_rsp_cb)(uint8_t *recv_data, size_t recv_size, uint8_t **resp_data, size_t *resp_size));

/* ========================== User port function ============================ */
#ifdef ONENET_USING_AUTO_REGISTER
/* Save device info. */
rt_err_t onenet_port_save_device_info(char *dev_id, char *api_key);
/* Get device name and auth info for register. */
rt_err_t onenet_port_get_register_info(char *dev_name, char *auth_info);
/* Get device info. */
rt_err_t onenet_port_get_device_info(char *dev_id, char *api_key, char *auth_info);
/* Check the device has been registered or not. */
rt_bool_t onenet_port_is_registed(void);
#endif

#endif /* _ONENET_H_ */
