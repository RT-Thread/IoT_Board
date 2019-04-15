/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2018-08-26     balanceTWK        the first version
 */

#ifndef __DECODER_H__
#define __DECODER_H__
#include <rtthread.h>

struct nec_data_struct
{
    rt_uint8_t addr;
    rt_uint8_t key;
    rt_uint8_t repeat;
};

struct infrared_decoder_data
{
    union 
    {
        struct nec_data_struct    nec;          /* Temperature.         unit: dCelsius    */
    }data;
};

#endif
