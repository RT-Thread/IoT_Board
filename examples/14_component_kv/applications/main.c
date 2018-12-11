/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-16     armink       first implementation
 * 2018-08-29     ZYLX         add EasyFlash
 */

#include <rtthread.h>
#include <easyflash.h>
#include <fal.h>
#include <stdlib.h>

static void test_env(void);

int main(void)
{
    fal_init();

    if (easyflash_init() == EF_NO_ERR)
    {
        /* test Env demo */
        test_env();
    }

    return 0;
}

static void test_env(void)
{
    uint32_t i_boot_times = NULL;
    char *c_old_boot_times, c_new_boot_times[11] = {0};

    /* get the boot count number from Env */
    c_old_boot_times = ef_get_env("boot_times");
    /* get the boot count number failed */
    if (c_old_boot_times == RT_NULL)
        c_old_boot_times[0] = '0';

    i_boot_times = atol(c_old_boot_times);
    /* boot count +1 */
    i_boot_times ++;
    rt_kprintf("===============================================\n");
    rt_kprintf("The system now boot %d times\n", i_boot_times);
    rt_kprintf("===============================================\n");
    /* interger to string */
    sprintf(c_new_boot_times, "%d", i_boot_times);
    /* set and store the boot count number to Env */
    ef_set_env("boot_times", c_new_boot_times);
    ef_save_env();
}
