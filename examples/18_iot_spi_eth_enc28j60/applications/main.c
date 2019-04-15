/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-16     ZYLX         first implementation
 */

#include <rtthread.h>

int main(void)
{
    extern rt_err_t ping(char* target_name, rt_uint32_t times, rt_size_t size);
    
    /* 等待网络连接成功 */
    rt_thread_mdelay(500);
    
    while(1)
    {
        if(ping("www.rt-thread.org", 4, 0) != RT_EOK)
        {
            rt_thread_mdelay(5000);
        }
        else
        {
            break;
        }
    }
    
    return 0;
}
