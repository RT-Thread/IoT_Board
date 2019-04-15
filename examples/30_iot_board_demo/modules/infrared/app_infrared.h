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
rt_err_t app_infrared_nec_decode(rt_uint8_t* key);
void app_infrared_nec_send(rt_uint8_t addrcode, rt_uint8_t keycode, rt_uint8_t times);

#endif
