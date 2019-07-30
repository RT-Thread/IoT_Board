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

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define FS_PARTITION_NAME "filesystem"

int fs_init();

int main(void)
{
    /* 配置 wlan 自动连接功能的依赖项 */
    wlan_autoconnect_init();

    /* 开启 wlan 自动连接 */
    rt_wlan_config_autoreconnect(RT_TRUE);
    /* 初始化文件系统 */
    fs_init();

    return 0;
}

int fs_init()
{
    /* Create a block device on the file system partition of spi flash */
    struct rt_device *flash_dev = fal_blk_device_create(FS_PARTITION_NAME);
    if (flash_dev == NULL)
    {
        LOG_E("Can't create a block device on '%s' partition.", FS_PARTITION_NAME);
    }
    else
    {
        LOG_D("Create a block device on the %s partition of flash successful.", FS_PARTITION_NAME);
    }

    /* mount the file system from "filesystem" partition of spi flash. */
    if (dfs_mount(FS_PARTITION_NAME, "/", "elm", 0, 0) == 0)
    {
        LOG_D("Filesystem initialized!");
    }
    else
    {
        LOG_E("Failed to initialize filesystem!");
        LOG_E("You should create a filesystem on the block device first!");
    }

    return 0;
}
