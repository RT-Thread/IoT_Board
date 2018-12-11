/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2018-10-10     balanceTWK        the first version
 */

#ifndef __APP_INFRARED_H__
#define __APP_INFRARED_H__

#include <rtthread.h>

int app_infrared_init(void);
void app_ir_learn_and_send(void);

#endif
