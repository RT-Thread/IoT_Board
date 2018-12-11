/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-16     armink       first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <app_infrared.h>

int main(void)
{
    unsigned int count = 1;
    app_infrared_init();
    while (count > 0)
    {
        app_ir_learn_and_send();
        count++;
    }
    return 0;
}
