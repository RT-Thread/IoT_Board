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

#include "board.h"
#include "drv_spi.h"

#ifdef RT_USING_SPI

//#define DBG_ENABLE
#define DBG_SECTION_NAME  "drv.spi"
#define DBG_COLOR
#define DBG_LEVEL DBG_INFO
#include <rtdbg.h>

#if !defined(BSP_USING_SPI1) && !defined(BSP_USING_SPI2) && !defined(BSP_USING_SPI3) && !defined(BSP_USING_SPI4) && !defined(BSP_USING_SPI5)
#error "Please define at least one BSP_USING_SPIx"
/* this driver can be disabled at menuconfig → RT-Thread Components → Device Drivers */
#endif

enum
{
#ifdef BSP_USING_SPI1
    SPI1_INDEX,
#endif
#ifdef BSP_USING_SPI2
    SPI2_INDEX,
#endif
#ifdef BSP_USING_SPI3
    SPI3_INDEX,
#endif
};

static struct stm32_spi_config spi_config[] =
{
#ifdef BSP_USING_SPI1
    SPI1_BUS_CONFIG,
#endif

#ifdef BSP_USING_SPI2
    SPI2_BUS_CONFIG,
#endif

#ifdef BSP_USING_SPI3
    SPI3_BUS_CONFIG,
#endif
};

static struct stm32_spi spi_bus_obj[sizeof(spi_config) / sizeof(spi_config[0])];

static rt_err_t stm32_spi_init(struct stm32_spi *spi_drv, struct rt_spi_configuration *cfg)
{
    RT_ASSERT(spi_drv != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    SPI_HandleTypeDef *spi_handle = &spi_drv->handle;

    if (cfg->mode & RT_SPI_SLAVE)
    {
        spi_handle->Init.Mode = SPI_MODE_SLAVE;
    }
    else
    {
        spi_handle->Init.Mode = SPI_MODE_MASTER;
    }

    if (cfg->mode & RT_SPI_3WIRE)
    {
        spi_handle->Init.Direction = SPI_DIRECTION_1LINE;
    }
    else
    {
        spi_handle->Init.Direction = SPI_DIRECTION_2LINES;
    }

    if (cfg->data_width == 8)
    {
        spi_handle->Init.DataSize = SPI_DATASIZE_8BIT;
        spi_handle->TxXferSize = 8;
        spi_handle->RxXferSize = 8;
    }
    else if (cfg->data_width == 16)
    {
        spi_handle->Init.DataSize = SPI_DATASIZE_16BIT;
    }
    else
    {
        return RT_EIO;
    }

    if (cfg->mode & RT_SPI_CPHA)
    {
        spi_handle->Init.CLKPhase = SPI_PHASE_2EDGE;
    }
    else
    {
        spi_handle->Init.CLKPhase = SPI_PHASE_1EDGE;
    }

    if (cfg->mode & RT_SPI_CPOL)
    {
        spi_handle->Init.CLKPolarity = SPI_POLARITY_HIGH;
    }
    else
    {
        spi_handle->Init.CLKPolarity = SPI_POLARITY_LOW;
    }

    if (cfg->mode & RT_SPI_NO_CS)
    {
        spi_handle->Init.NSS = SPI_NSS_SOFT;
    }
    else
    {
        spi_handle->Init.NSS = SPI_NSS_SOFT;
    }

    uint32_t SPI_APB_CLOCK;

    SPI_APB_CLOCK = HAL_RCC_GetPCLK2Freq();

    if (cfg->max_hz >= SPI_APB_CLOCK / 2)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 4)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 8)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 16)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 32)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 64)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 128)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
    }
    else
    {
        /*  min prescaler 256 */
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    }

    LOG_D("sys freq: %d, pclk2 freq: %d, SPI limiting freq: %d, BaudRatePrescaler: %d",
          HAL_RCC_GetSysClockFreq(),
          SPI_APB_CLOCK,
          cfg->max_hz,
          spi_handle->Init.BaudRatePrescaler);

    if (cfg->mode & RT_SPI_MSB)
    {
        spi_handle->Init.FirstBit = SPI_FIRSTBIT_MSB;
    }
    else
    {
        spi_handle->Init.FirstBit = SPI_FIRSTBIT_LSB;
    }

    spi_handle->Init.TIMode = SPI_TIMODE_DISABLE;
    spi_handle->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    spi_handle->State = HAL_SPI_STATE_RESET;
    if (HAL_SPI_Init(spi_handle) != HAL_OK)
    {
        return RT_EIO;
    }

    SET_BIT(spi_handle->Instance->CR2, SPI_RXFIFO_THRESHOLD_HF);

    __HAL_SPI_ENABLE(spi_handle);

    LOG_D("%s init done", spi_drv->config->bus_name);
    return RT_EOK;
}

static rt_uint32_t spixfer(struct rt_spi_device *device, struct rt_spi_message *message)
{
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);
    RT_ASSERT(device->bus->parent.user_data != RT_NULL);
    RT_ASSERT(message != RT_NULL);

    struct stm32_spi *spi_drv =  rt_container_of(device->bus, struct stm32_spi, spi_bus);
    SPI_HandleTypeDef * spi_handle = &spi_drv->handle;
    struct stm32_hw_spi_cs *cs = device->parent.user_data;
    rt_int32_t length = message->length;
    rt_int32_t data_width = spi_drv->cfg->data_width;

    if (message->cs_take)
    {
        HAL_GPIO_WritePin(cs->GPIOx, cs->GPIO_Pin, GPIO_PIN_RESET);
    }
    if (data_width == 8)
    {
        const rt_uint8_t * send_ptr = message->send_buf;
        rt_uint8_t * recv_ptr = message->recv_buf;
        
        while (length--)
        {
            rt_uint8_t data = ~0;

            if(send_ptr != RT_NULL)
            {
                data = *send_ptr++;
            }
            
            /* send data once */
            while (__HAL_SPI_GET_FLAG(spi_handle, SPI_FLAG_TXE) == RESET);
            *(volatile rt_uint8_t *)(&spi_handle->Instance->DR) = data;

            /* receive data once */
            SET_BIT(spi_handle->Instance->CR2, SPI_RXFIFO_THRESHOLD_HF);
            while (__HAL_SPI_GET_FLAG(spi_handle, SPI_FLAG_RXNE) == RESET);
            data = *(volatile rt_uint8_t *)(&spi_handle->Instance->DR);
            
            if(recv_ptr != RT_NULL)
            {
                *recv_ptr++ = data;
            }
        }

    } else
    {
        const rt_uint16_t * send_ptr = message->send_buf;
        rt_uint16_t * recv_ptr = message->recv_buf;
        
        while (length--)
        {
            rt_uint16_t data = ~0;
            
            if(send_ptr != RT_NULL)
            {
                data = *send_ptr++;
            }
            
            /* send data once */
            while (__HAL_SPI_GET_FLAG(spi_handle, SPI_FLAG_TXE) == RESET);
            *(volatile rt_uint16_t *)(&spi_handle->Instance->DR) = data;

            /* receive data once */
            SET_BIT(spi_handle->Instance->CR2, SPI_RXFIFO_THRESHOLD_HF);
            while (__HAL_SPI_GET_FLAG(spi_handle, SPI_FLAG_RXNE) == RESET);
            data = *(volatile rt_uint16_t *)(&spi_handle->Instance->DR);
            
            if(recv_ptr != RT_NULL)
            {
                *recv_ptr++ = data;
            }
        }
    }

    /* Wait until Busy flag is reset before disabling SPI */
    while (__HAL_SPI_GET_FLAG(spi_handle, SPI_FLAG_BSY) == SET);

    if (message->cs_release)
    {
        HAL_GPIO_WritePin(cs->GPIOx, cs->GPIO_Pin, GPIO_PIN_SET);
    }

    return message->length;
}

static rt_err_t spi_configure(struct rt_spi_device *device,
                              struct rt_spi_configuration *configuration)
{
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(configuration != RT_NULL);

    struct stm32_spi *spi_drv =  rt_container_of(device->bus, struct stm32_spi, spi_bus);
    spi_drv->cfg = configuration;

    return stm32_spi_init(spi_drv, configuration);
}

static const struct rt_spi_ops stm_spi_ops =
{
    .configure = spi_configure,
    .xfer = spixfer,
};

static int rt_hw_spi_bus_init(void)
{
    rt_err_t result;
    for (int i = 0; i < sizeof(spi_config) / sizeof(spi_config[0]); i++)
    {
        spi_bus_obj[i].config = &spi_config[i];
        spi_bus_obj[i].spi_bus.parent.user_data = &spi_config[i];
        spi_bus_obj[i].handle.Instance = spi_config[i].Instance;

        result = rt_spi_bus_register(&spi_bus_obj[i].spi_bus, spi_config[i].bus_name, &stm_spi_ops);
        RT_ASSERT(result == RT_EOK);

        LOG_D("%s bus init done", spi_config[i].bus_name);
    }

    return result;
}

/**
  * Attach the spi device to SPI bus, this function must be used after initialization.
  */
rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin)
{
    RT_ASSERT(bus_name != RT_NULL);
    RT_ASSERT(device_name != RT_NULL);

    rt_err_t result;
    struct rt_spi_device *spi_device;
    struct stm32_hw_spi_cs *cs_pin;

    /* initialize the cs pin && select the slave*/
    GPIO_InitTypeDef GPIO_Initure;
    GPIO_Initure.Pin = cs_gpio_pin;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Initure.Pull = GPIO_PULLUP;
    GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(cs_gpiox, &GPIO_Initure);
    HAL_GPIO_WritePin(cs_gpiox, cs_gpio_pin, GPIO_PIN_SET);

    /* attach the device to spi bus*/
    spi_device = (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
    RT_ASSERT(spi_device != RT_NULL);
    cs_pin = (struct stm32_hw_spi_cs *)rt_malloc(sizeof(struct stm32_hw_spi_cs));
    RT_ASSERT(cs_pin != RT_NULL);
    cs_pin->GPIOx = cs_gpiox;
    cs_pin->GPIO_Pin = cs_gpio_pin;
    result = rt_spi_bus_attach_device(spi_device, device_name, bus_name, (void *)cs_pin);

    if (result != RT_EOK)
    {
        LOG_E("%s attach to %s faild, %d\n", device_name, bus_name, result);
    }

    RT_ASSERT(result == RT_EOK);

    LOG_D("%s attach to %s done", device_name, bus_name);

    return result;
}

int rt_hw_spi_init(void)
{
    return rt_hw_spi_bus_init();
}
INIT_BOARD_EXPORT(rt_hw_spi_init);

void HAL_SPI_MspInit(SPI_HandleTypeDef *spiHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    RT_ASSERT(spiHandle != RT_NULL);

#ifdef BSP_USING_SPI1
    if (spiHandle->Instance == SPI1)
    {
        /* SPI1 clock enable */
        __HAL_RCC_SPI1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /**SPI1 GPIO Configuration
        PA5     ------> SPI1_SCK
        PA6     ------> SPI1_MISO
        PA7     ------> SPI1_MOSI
        */
        GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
    else
#endif
#ifdef BSP_USING_SPI2
    if (spiHandle->Instance == SPI2)
    {
        /* SPI2 clock enable */
        __HAL_RCC_SPI2_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /**SPI2 GPIO Configuration
        PB13     ------> SPI2_SCK
        PB14     ------> SPI2_MISO
        PB15     ------> SPI2_MOSI
        */
        GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
    else
#endif
#ifdef BSP_USING_SPI3
    if (spiHandle->Instance == SPI3)
    {
        /* SPI3 clock enable */
        __HAL_RCC_SPI3_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /**SPI3 GPIO Configuration
        PB3 (JTDO-TRACESWO)     ------> SPI3_SCK
        PB4 (NJTRST)            ------> SPI3_MISO
        PB5                     ------> SPI3_MOSI
        */
        GPIO_InitStruct.Pin = GPIO_PIN_3 /* | GPIO_PIN_4 */ | GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
    else
#endif
    {
        RT_ASSERT(0);
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *spiHandle)
{
    RT_ASSERT(spiHandle != RT_NULL);

#ifdef BSP_USING_SPI1
    if (spiHandle->Instance == SPI1)
    {
        /* Peripheral clock disable */
        __HAL_RCC_SPI1_CLK_DISABLE();
        /**SPI1 GPIO Configuration
        PA5     ------> SPI1_SCK
        PA6     ------> SPI1_MISO
        PA7     ------> SPI1_MOSI
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    }
    else
#endif
#ifdef BSP_USING_SPI2
    if (spiHandle->Instance == SPI2)
    {
        /* Peripheral clock disable */
        __HAL_RCC_SPI2_CLK_DISABLE();
        /**SPI2 GPIO Configuration
        PB13     ------> SPI2_SCK
        PB14     ------> SPI2_MISO
        PB15     ------> SPI2_MOSI
        */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    }
    else
#endif
#ifdef BSP_USING_SPI3
    if (spiHandle->Instance == SPI3)
    {
        /* Peripheral clock disable */
        __HAL_RCC_SPI3_CLK_DISABLE();
        /**SPI3 GPIO Configuration
        PB3 (JTDO-TRACESWO)     ------> SPI3_SCK
        PB4 (NJTRST)            ------> SPI3_MISO
        PB5                     ------> SPI3_MOSI
        */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_3 /* | GPIO_PIN_4 */ | GPIO_PIN_5);
    }
#endif
    {
        RT_ASSERT(0);
    }
}

#endif /* RT_USING_SPI */
