/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-03-22     Murphy       the first version
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <rtthread.h>
#include <finsh.h>

#include "webclient.h"
#include <fal.h>

#undef DBG_SECTION_NAME
#undef DBG_LEVEL
#undef DBG_COLOR
#undef DBG_ENABLE

#define DBG_ENABLE
#define DBG_SECTION_NAME          "http_ota"
#ifdef HTTP_OTA_DEBUG
#define DBG_LEVEL                 DBG_LOG
#else
#define DBG_LEVEL                 DBG_INFO
#endif 
#define DBG_COLOR
#include <rtdbg.h>

#define HTTP_OTA_BUFF_LEN         4096
#define GET_HEADER_BUFSZ          1024
#define GET_RESP_BUFSZ            1024
#define HTTP_OTA_DL_DELAY         (10 * RT_TICK_PER_SECOND)

#define HTTP_OTA_URL              "http://192.168.10.135:80/rtthread.rbl"

static void print_progress(size_t cur_size, size_t total_size)
{
    static unsigned char progress_sign[100 + 1];
    uint8_t i, per = cur_size * 100 / total_size;

    if (per > 100)
    {
        per = 100;
    }

    for (i = 0; i < 100; i++)
    {
        if (i < per)
        {
            progress_sign[i] = '=';
        }
        else if (per == i)
        {
            progress_sign[i] = '>';
        }
        else
        {
            progress_sign[i] = ' ';
        }
    }

    progress_sign[sizeof(progress_sign) - 1] = '\0';

    LOG_I("\033[2A");
    LOG_I("Download: [%s] %d%%", progress_sign, per);
}

static int http_ota_fw_download(const char* uri)
{
    int ret = 0, resp_status;
    int file_size = 0, length, total_length = 0;
    rt_uint8_t *buffer_read = RT_NULL;
    struct webclient_session* session = RT_NULL;
    const struct fal_partition * dl_part = RT_NULL;

    /* create webclient session and set header response size */
    session = webclient_session_create(GET_HEADER_BUFSZ);
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
        goto __exit;
    }

    file_size = webclient_content_length_get(session);

    if (file_size == 0)
    {
        LOG_E("Request file size is 0!");
        ret = -RT_ERROR;
        goto __exit;
    }
    else if (file_size < 0)
    {
        LOG_E("webclient GET request type is chunked.");
        ret = -RT_ERROR;
        goto __exit;
    }

    /* Get download partition information and erase download partition data */
    if ((dl_part = fal_partition_find("download")) == RT_NULL)
    {
        LOG_E("Firmware download failed! Partition (%s) find error!", "download");
        ret = -RT_ERROR;
        goto __exit;
    }

    LOG_I("Start erase flash (%s) partition!", dl_part->name);

    if (fal_partition_erase(dl_part, 0, file_size) == 0)
    {
        LOG_E("Firmware download failed! Partition (%s) erase error!", dl_part->name);
        ret = -RT_ERROR;
        goto __exit;
    }
    LOG_I("Erase flash (%s) partition success!", dl_part->name);

    buffer_read = web_malloc(HTTP_OTA_BUFF_LEN);
    if (buffer_read == RT_NULL)
    {
        LOG_E("No memory for http ota!");
        ret = -RT_ERROR;
        goto __exit;
    }
    memset(buffer_read, 0x00, HTTP_OTA_BUFF_LEN);

    LOG_I("OTA file size is (%d)", file_size);

    do
    {
        length = webclient_read(session, buffer_read, file_size - total_length > HTTP_OTA_BUFF_LEN ?
                            HTTP_OTA_BUFF_LEN : file_size - total_length);   
        if (length > 0)
        {
            /* Write the data to the corresponding partition address */
            if (fal_partition_write(dl_part, total_length, buffer_read, length) < 0)
            {
                LOG_E("Firmware download failed! Partition (%s) write data error!", dl_part->name);
                ret = -RT_ERROR;
                goto __exit;
            }
            total_length += length;

            print_progress(total_length, file_size);
        }
        else
        {
            LOG_E("Exit: server return err (%d)!", length);
            ret = -RT_ERROR;
            goto __exit;
        }

    } while(total_length != file_size);

    ret = RT_EOK;

    if (total_length == file_size)
    {
        if (session != RT_NULL)
            webclient_close(session);
        if (buffer_read != RT_NULL)
            web_free(buffer_read);

        LOG_I("Download firmware to flash success.");
        LOG_I("System now will restart...");

        rt_thread_delay(rt_tick_from_millisecond(5));

        /* Reset the device, Start new firmware */
        extern void rt_hw_cpu_reset(void);
        rt_hw_cpu_reset();
    }

__exit:
    if (session != RT_NULL)
        webclient_close(session);
    if (buffer_read != RT_NULL)
        web_free(buffer_read);

    return ret;
}

void http_ota(uint8_t argc, char **argv)
{
    if (argc < 2)
    {
        rt_kprintf("using uri: " HTTP_OTA_URL "\n");
        http_ota_fw_download(HTTP_OTA_URL);
    }
	else
	{
        http_ota_fw_download(argv[1]);
    }
}
/**
 * msh />http_ota [url]
*/
MSH_CMD_EXPORT(http_ota, OTA by http client: http_ota [url]);
