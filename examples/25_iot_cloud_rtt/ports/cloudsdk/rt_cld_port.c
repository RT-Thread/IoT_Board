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
 */
 
#include <stdio.h>
#include <string.h>

#include <board.h>
#include <rt_cld.h>

/* Must be configured custom device and RTT cloud configuration  */
#define CLD_SN               "123456789"
#define CLD_PRODUCT_ID       "abcdefghi"
#define CLD_PRODUCT_KEY      "abcdefghiihgfedcba"

void cld_port_get_device_sn(char *sn)
{
    rt_strncpy(sn, CLD_SN, rt_strlen(CLD_SN));
}

void cld_port_get_product_id(char *id)
{
    rt_strncpy(id, CLD_PRODUCT_ID, rt_strlen(CLD_PRODUCT_ID));
}

void cld_port_get_product_key(char *key)
{
    rt_strncpy(key, CLD_PRODUCT_KEY, rt_strlen(CLD_PRODUCT_KEY));
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



