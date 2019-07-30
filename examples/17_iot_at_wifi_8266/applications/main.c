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
    extern int netdev_ping(int argc, char **argv);

    char *cmd[] = {"esp8266_ping", "www.rt-thread.org"};

    while (1)
    {
        if (netdev_ping(2, cmd) == RT_EOK)
        {
            break;
        }
        else
        {
            rt_thread_mdelay(5000);
        }
    }

    return 0;
}
