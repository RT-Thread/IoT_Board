/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2018-8-23      SummerGift        the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_spi.h"
#include "spi_msd.h"

#ifdef BSP_USING_TF_CARD

static int rt_hw_spi1_tfcard(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    rt_hw_spi_device_attach("spi1", "spi10", GPIOC, GPIO_PIN_3);
    return msd_init("sd0", "spi10");
}
INIT_COMPONENT_EXPORT(rt_hw_spi1_tfcard);

#endif /*BSP_USING_TF_CARD*/
