/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-01-15     armink       first implementation
 */

#include <stdio.h>
#include <rtthread.h>
#include <string.h>
#include <rthw.h>
#include <fal.h>
#include <dfs_fs.h>
#include <rtdevice.h>

#define LOG_TAG               "main"
#include <ulog.h>

#define FS_PARTITION_NAME     "filesystem"

int main(void)
{
    /* 分区表初始化 */
    fal_init();

    /* 在文件系统分区上创建块设备*/
    struct rt_device *flash_dev = fal_blk_device_create(FS_PARTITION_NAME);
    if (flash_dev == NULL)
    {
        LOG_E("Can't create a block device on '%s' partition.", FS_PARTITION_NAME);
    }

    /* 在文件系统分区上挂载文件系统 */
    if (dfs_mount(FS_PARTITION_NAME, "/", "elm", 0, 0) == 0)
    {
        LOG_I("Filesystem initialized!");
    }
    else
    {
        /* 创建文件系统 */
        dfs_mkfs("elm", FS_PARTITION_NAME);
        /* 重新挂载文件系统 */
        if (dfs_mount(FS_PARTITION_NAME, "/", "elm", 0, 0) == 0)
        {
            LOG_E("Failed to initialize filesystem! The mpy fs module is not available.");
        }
    }

    /* 配置 wifi 工作模式 */
    rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);
    rt_wlan_set_mode(RT_WLAN_DEVICE_AP_NAME, RT_WLAN_AP);

    extern void wlan_autoconnect_init(void);
    /* 自动连接初始化 */
    wlan_autoconnect_init();
    /* 打开自动连接 */
    rt_wlan_config_autoreconnect(RT_TRUE);

    /* 等待系统初始化完毕 */
    rt_thread_mdelay(100);
    
    while(1)
    {
        /* 打开 MicroPython 命令交互界面 */
        extern void mpy_main(const char *filename);
        mpy_main(NULL);
    }
}
