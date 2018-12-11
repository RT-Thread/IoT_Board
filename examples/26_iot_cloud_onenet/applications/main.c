/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-01     ZYLX         first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <wlan_mgnt.h>
#include "wifi_config.h"
#include <stdlib.h>
#include <onenet.h>
#include <string.h>
#include <ap3216c.h>
#include <easyflash.h>
#include <drv_gpio.h>

#define I2C_BUS_NAME         "i2c1"
#define LED_PIN              PIN_LED_R
#define NUMBER_OF_UPLOADS    100

ap3216c_device_t dev;
static void onenet_cmd_rsp_cb(uint8_t *recv_data, size_t recv_size, uint8_t **resp_data, size_t *resp_size);

int main(void)
{
    /* initializes led */
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    /* waiting for ap3216c work */
    rt_thread_mdelay(2000);

    /* initializes ap3216c, registered device driver */
    dev = ap3216c_init(I2C_BUS_NAME);
    if (dev == RT_NULL)
    {
        rt_kprintf(" The sensor initializes failure\n");
        return 0;
    }

    /* register the wlan ready callback function */
    rt_wlan_register_event_handler(RT_WLAN_EVT_READY, (void ( *)(int , struct rt_wlan_buff *, void *))onenet_mqtt_init, RT_NULL);
    /* initialize the autoconnect configuration */
    wlan_autoconnect_init();
    /* enable wlan auto connect */
    rt_wlan_config_autoreconnect(RT_TRUE);

    return 0;
}

static void onenet_upload_entry(void *parameter)
{
    int value = 0;
    int i = 0;

    /* upload ambient light value to topic light*/
    for (i = 0; i < NUMBER_OF_UPLOADS; i++)
    {
        value = (int)ap3216c_read_ambient_light(dev);

        if (onenet_mqtt_upload_digit("light", value) < 0)
        {
            rt_kprintf("upload has an error, stop uploading\n");
            break;
        }
        else
        {
            rt_kprintf("buffer : {\"light\":%d}\n", value);
        }

        rt_thread_mdelay(5 * 1000);
    }
}

void onenet_upload_cycle(void)
{
    rt_thread_t tid;

    /* set the command response call back function */
    onenet_set_cmd_rsp_cb(onenet_cmd_rsp_cb);

    /* create the ambient light data upload thread */
    tid = rt_thread_create("onenet_send",
                           onenet_upload_entry,
                           RT_NULL,
                           2 * 1024,
                           RT_THREAD_PRIORITY_MAX / 3 - 1,
                           5);
    if (tid)
    {
        rt_thread_startup(tid);
    }
}

rt_err_t onenet_port_save_device_info(char *dev_id, char *api_key)
{
    EfErrCode err = EF_NO_ERR;

    /* save device id */
    err = ef_set_and_save_env("dev_id", dev_id);
    if (err != EF_NO_ERR)
    {
        rt_kprintf("save device info(dev_id : %s) failed!\n", dev_id);
        return -RT_ERROR;
    }

    /* save device api_key */
    err = ef_set_and_save_env("api_key", api_key);
    if (err != EF_NO_ERR)
    {
        rt_kprintf("save device info(api_key : %s) failed!\n", api_key);
        return -RT_ERROR;
    }

    /* save already_register environment variable */
    err = ef_set_and_save_env("already_register", "1");
    if (err != EF_NO_ERR)
    {
        rt_kprintf("save already_register failed!\n");
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t onenet_port_get_register_info(char *dev_name, char *auth_info)
{
    rt_uint32_t cpuid[2] = {0};
    EfErrCode err = EF_NO_ERR;

    /* get stm32 uid */
    cpuid[0] = *(volatile rt_uint32_t *)(0x1FFF7590);
    cpuid[1] = *(volatile rt_uint32_t *)(0x1FFF7590 + 4);

    /* set device name and auth_info */
    rt_snprintf(dev_name, ONENET_INFO_AUTH_LEN, "%d%d", cpuid[0], cpuid[1]);
    rt_snprintf(auth_info, ONENET_INFO_AUTH_LEN, "%d%d", cpuid[0], cpuid[1]);

    /* save device auth_info */
    err = ef_set_and_save_env("auth_info", auth_info);
    if (err != EF_NO_ERR)
    {
        rt_kprintf("save auth_info failed!\n");
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t onenet_port_get_device_info(char *dev_id, char *api_key, char *auth_info)
{
    char *info = RT_NULL;

    /* get device id */
    info = ef_get_env("dev_id");
    if (info == RT_NULL)
    {
        rt_kprintf("read dev_id failed!\n");
        return -RT_ERROR;
    }
    else
    {
        rt_snprintf(dev_id, ONENET_INFO_AUTH_LEN, "%s", info);
    }

    /* get device api_key */
    info = ef_get_env("api_key");
    if (info == RT_NULL)
    {
        rt_kprintf("read api_key failed!\n");
        return -RT_ERROR;
    }
    else
    {
        rt_snprintf(api_key, ONENET_INFO_AUTH_LEN, "%s", info);
    }

    /* get device auth_info */
    info = ef_get_env("auth_info");
    if (info == RT_NULL)
    {
        rt_kprintf("read auth_info failed!\n");
        return -RT_ERROR;
    }
    else
    {
        rt_snprintf(auth_info, ONENET_INFO_AUTH_LEN, "%s", info);
    }

    return RT_EOK;
}

rt_bool_t onenet_port_is_registed(void)
{
    char *already_register = RT_NULL;

    /* check the device has been registered or not */
    already_register = ef_get_env("already_register");
    if (already_register == RT_NULL)
    {
        return RT_FALSE;
    }

    return already_register[0] == '1' ? RT_TRUE : RT_FALSE;
}

static void onenet_cmd_rsp_cb(uint8_t *recv_data, size_t recv_size, uint8_t **resp_data, size_t *resp_size)
{
    char res_buf[20] = { 0 };

    rt_kprintf("recv data is %.s\n", recv_size, recv_data);

    /* match the command */
    if (rt_strncmp(recv_data, "ledon", 5) == 0)
    {
        /* led on */
        rt_pin_write(LED_PIN, PIN_LOW);

        rt_snprintf(res_buf, sizeof(res_buf), "led is on");

        rt_kprintf("led is on\n");
    }
    else if (rt_strncmp(recv_data, "ledoff", 6) == 0)
    {
        /* led off */
        rt_pin_write(LED_PIN, PIN_HIGH);

        rt_snprintf(res_buf, sizeof(res_buf), "led is off");

        rt_kprintf("led is off\n");
    }

    /* user have to use ONENET_MALLOC malloc memory for response data */
    *resp_data = (uint8_t *) ONENET_MALLOC(strlen(res_buf) + 1);

    strncpy(*resp_data, res_buf, strlen(res_buf));

    *resp_size = strlen(res_buf);
}
