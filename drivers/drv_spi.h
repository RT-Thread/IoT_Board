/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2018-11-5      SummerGift        change to new framework
 * 2018-11-22     balanceTWK        update spi drv
 */

#ifndef __DRV_SPI_H_
#define __DRV_SPI_H_

#include <stm32l4xx.h>
#include "rtdevice.h"
#include <rthw.h>

rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin);

struct stm32_hw_spi_cs
{
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
};

struct stm32_spi_config
{
    SPI_TypeDef *Instance;
    char *bus_name;
};

struct stm32_spi_device
{
    rt_uint32_t pin;
    char *bus_name;
    char *device_name;
};

/* stm32 spi dirver class */
struct stm32_spi
{
    SPI_HandleTypeDef handle;
    const struct stm32_spi_config *config;
    struct rt_spi_configuration *cfg;
    struct rt_spi_bus spi_bus;
};

#ifdef BSP_USING_SPI1
#define SPI1_BUS_CONFIG                                  \
    {                                                    \
        .Instance = SPI1,                                \
        .bus_name = "spi1",                              \
    }
#endif

#ifdef BSP_USING_SPI2
#define SPI2_BUS_CONFIG                                  \
    {                                                    \
        .Instance = SPI2,                                \
        .bus_name = "spi2",                              \
    }
#endif

#ifdef BSP_USING_SPI3
#define SPI3_BUS_CONFIG                                  \
    {                                                    \
        .Instance = SPI3,                                \
        .bus_name = "spi3",                              \
    }
#endif

#endif /*__DRV_SPI_H_ */
