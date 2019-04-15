/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-17     zylx         first implementation.
 */
 
#include <board.h>
#include <drv_qspi.h>
#include <rtdevice.h>
#include <rthw.h>

#ifdef BSP_USING_FLASH

#include "spi_flash.h"
#include "spi_flash_sfud.h"

static struct rt_spi_device *spi_dev_w25q128;

int rt_hw_qspi_flash_with_sfud_init(void)
{
    rt_uint8_t w25x_enter_qspi_mode = 0x38;
    rt_uint8_t w25x_write_status_reg2 = 0x31;
    rt_uint8_t w25x_reg2_qspi_mode = 0x02;
    rt_uint8_t w25x_write_enable = 0x06;
    
    /* find qspi device */
    spi_dev_w25q128 = (struct rt_spi_device *) rt_device_find("qspi10");
    if(!spi_dev_w25q128)
    {
        rt_kprintf("qspi flash with sfud init failed! can't find qspi device.");
        return -RT_ERROR;
    }
    
    /* enter qspi mode */
    rt_spi_send(spi_dev_w25q128, &w25x_write_enable, 1);
    rt_spi_send_then_send(spi_dev_w25q128, &w25x_write_status_reg2 , 1, &w25x_reg2_qspi_mode , 1);
    rt_spi_send(spi_dev_w25q128, &w25x_enter_qspi_mode, 1);

    /* wait for enter qspi mode */
    rt_thread_mdelay(10);

    /* init w25q128 */
    if (RT_NULL == rt_sfud_flash_probe("w25q128", "qspi10"))
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_qspi_flash_with_sfud_init);

#endif/* BSP_USING_FLASH */
