/*
 * File      : rt_cld_port.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-10-10     chenyong     the first version
 * 2018-10-22     MurphyZhao   Adapt to iot board
 */
 
#include <stdio.h>
#include <string.h>

#include <board.h>
#include <rt_cld.h>
#include "rt_cld_port.h"

/* Must be configured before rt-cloud start */
static char cld_sn[CLD_SN_MAX_LEN + 1];
static char cld_product_id[CLD_PRODUCT_ID_MAX_LEN + 1];
static char cld_product_key[CLD_PRODUCT_KEY_MAX_LEN + 1];

void cld_port_get_device_sn(char *sn)
{
    RT_ASSERT(sn != RT_NULL);
    rt_strncpy(sn, cld_sn, rt_strlen(cld_sn));
}

void cld_port_set_device_sn(char *sn)
{
    RT_ASSERT(sn != RT_NULL);
    rt_strncpy(cld_sn, sn, sizeof(cld_sn) - 1);
}

void cld_port_get_product_id(char *id)
{
    RT_ASSERT(id != RT_NULL);
    rt_strncpy(id, cld_product_id, rt_strlen(cld_product_id));
}

void cld_port_set_product_id(char *id)
{
    RT_ASSERT(id != RT_NULL);
    rt_strncpy(cld_product_id, id, sizeof(cld_product_id) - 1);
}

void cld_port_get_product_key(char *key)
{
    RT_ASSERT(key != RT_NULL);
    rt_strncpy(key, cld_product_key, rt_strlen(cld_product_key));
}

void cld_port_set_product_key(char *key)
{
    RT_ASSERT(key != RT_NULL);
    rt_strncpy(cld_product_key, key, sizeof(cld_product_key) - 1);
}

void cld_port_ota_start(void)
{
    
}

void cld_port_ota_end(enum cld_ota_status status)
{
    //Reset the device
    if(status == CLD_OTA_OK)
    {
        NVIC_SystemReset();
    }
}
