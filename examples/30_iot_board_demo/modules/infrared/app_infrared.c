/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2018-11-22     balanceTWK        the first version
 */

#include <rthw.h>
#include <rtdevice.h>
#include <board.h>
#include "drv_infrared.h"
#include "app_infrared.h"

// #define DBG_ENABLE
#define DBG_SECTION_NAME  "app.infrared"
#define DBG_COLOR
#define DBG_LEVEL DBG_INFO
#include <rtdbg.h>

int app_infrared_init(void)
{
    infrared_receive_init();
    infrared_send_init();
    return RT_EOK;
}

int app_infrared_start(void)
{
    start_ir_receive();
    return RT_EOK;
}

int app_infrared_stop(void)
{
    stop_ir_receive();
    return RT_EOK;
}

rt_err_t app_infrared_nec_decode(rt_uint8_t* key)
{
    rt_size_t receive_size = 0;
    rt_uint32_t data[200];
    rt_uint8_t t1, t2 , repetition = 0, guidance_sta = 0;
    rt_uint32_t count = 0, command = 0;

    receive_size = 0;
    repetition = 0;
    guidance_sta = 0;
    count = 0;
    command = 0;

    for (rt_size_t i = 0; i < 200; i++)
    {
        if (infrared_raw_receive(&data[receive_size]) == RT_EOK)
        {
            receive_size++;
            rt_thread_mdelay(5);
            LOG_I("receive_size:%d",receive_size);
            LOG_I("data        :%d",data[receive_size]);
        }
        else
        {
            break;
        }
    }

    for(rt_uint32_t i = 0 ;i < receive_size ; i++)
    {
        if (data[i] & (0xAA000000))
        {
            LOG_I("-buf:%d",(data[i] & (0x00FFFFFF)));
        }
        else
        {
            LOG_I("+buf:%d",data[i]);
            if (guidance_sta & 0X80)
            {
                if (data[i] > 300 && data[i] < 1000)
                {
                    command <<= 1;
                    command |= 0;
                    count++;
                }
                else if (data[i] > 1400 && data[i] < 2000)
                {
                    command <<= 1;
                    command |= 1;
                    count++;
                }
                else if (data[i] > 2200 && data[i] < 2600)
                {
                    repetition++;
                    guidance_sta &= 0XF0;
                }
            }
            else if (data[i] > 4200 && data[i] < 4700)
            {
                guidance_sta |= 1 << 7;
            }
        }
    }
    t1 = command >> 8;
    t2 = command;
    if (t1 == (rt_uint8_t)~t2)
    {
        LOG_I("comm1:%d",t1);
        LOG_I("comm2:%d",t2);
        LOG_I("count:%d , repetition:%d",count,repetition);
        *key = t1;
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR ;
    }
}

void app_infrared_nec_send(rt_uint8_t addrcode, rt_uint8_t keycode, rt_uint8_t times)
{
    rt_uint8_t index;
    rt_uint32_t data;

    data = ((addrcode & 0xFF) << 24) + ((~addrcode & 0xFF) << 16) + ((keycode & 0xff) << 8) + (~keycode & 0xFF);
    infrared_raw_send(1,9000);
    infrared_raw_send(0,4500);

    for(index = 0; index < 32; index++)
    {
        infrared_raw_send(1,560);
        if(((data << index) & 0x80000000))
        {
            infrared_raw_send(0,1680);
        }
        else
        {
            infrared_raw_send(0,550);
        }
    }
    infrared_raw_send(1,560);
    for(index = 0; index < times; index++)
    {
        infrared_raw_send(1,9000);
        infrared_raw_send(0,2250);
        infrared_raw_send(1,560);
        infrared_raw_send(0,7940);
        rt_thread_mdelay(90);
    }
}
