/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-11     MurphyZhao   first implementation
 * 2018-09-14     SummerGift   add netutils example
 */

#include <rtthread.h>
#include <wlan_mgnt.h>
#include <dfs_fs.h>
#include <fal.h>
#include "wifi_config.h"

#define FS_PARTITION_NAME  "filesystem"

int fs_init();

int main(void)
{
    /* Config the dependencies of the wlan autoconnect function */
    wlan_autoconnect_init();

    /* Enable wlan auto connect function */
    rt_wlan_config_autoreconnect(RT_TRUE);

    /* Filesystem init */
    fs_init();

    return 0;
}

int fs_init()
{
    /* Create a block device on the file system partition of spi flash */
    struct rt_device *flash_dev = fal_blk_device_create(FS_PARTITION_NAME);
    if (flash_dev == NULL)
    {
        rt_kprintf("Can't create a block device on '%s' partition.\n", FS_PARTITION_NAME);
    }
    else
    {
        rt_kprintf("Create a block device on the %s partition of flash successful.\n", FS_PARTITION_NAME);
    }

    /* mount the file system from "filesystem" partition of spi flash. */
    if (dfs_mount(FS_PARTITION_NAME, "/", "elm", 0, 0) == 0)
    {
        rt_kprintf("Filesystem initialized!\n");
    }
    else
    {
        rt_kprintf("Failed to initialize filesystem!\n");
        rt_kprintf("You should create a filesystem on the block device first!\n");
    }
	
	return 0;
}

