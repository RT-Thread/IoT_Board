/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-23     SummerGift   Add tf card filesystem example
 */

#include <rtthread.h>
#include <dfs_fs.h>

int main(void)
{
#ifdef BSP_USING_TF_CARD
    /* mount the file system from tf card */
    if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
    {
        rt_kprintf("Filesystem initialized!\n");
    }
    else
    {
        rt_kprintf("Failed to initialize filesystem!\n");
    }
#endif /*BSP_USING_TF_CARD*/
    return 0;
}

