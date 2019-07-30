/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2018-11-22     balanceTWK        the first version
 */

#include <rthw.h>
#include <rtdevice.h>
#include <board.h>
#include "infrared.h"
#include "app_infrared.h"

// #define DBG_ENABLE
#define DBG_TAG  "app.infrared"
#define DBG_COLOR
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

int app_infrared_init(void)
{
    ir_select_decoder("nec");
    return RT_EOK;
}

rt_err_t app_infrared_nec_decode(rt_uint8_t* key)
{
    struct infrared_decoder_data infrared_data;
    if(infrared_read("nec", &infrared_data) == RT_EOK)
    {
        *key = infrared_data.data.nec.key;
        return RT_EOK;
    }
    return -RT_ERROR ;
}

void app_infrared_nec_send(rt_uint8_t addrcode, rt_uint8_t keycode, rt_uint8_t times)
{
    struct infrared_decoder_data infrared_data;
    infrared_data.data.nec.addr = addrcode;
    infrared_data.data.nec.key = keycode;
    infrared_data.data.nec.repeat = times;
    infrared_write("nec", &infrared_data);
}
