/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2018-08-17     zylx              the first version
 */
 
#include "drv_qspi.h"
#include <stm32l4xx_hal_qspi.h>

#ifdef BSP_USING_QSPI

#define DBG_ENABLE
#define DBG_SECTION_NAME               "QSPI"
#ifdef QSPI_DEBUG
#define DBG_LEVEL                      DBG_LOG
#else
#define DBG_LEVEL                      DBG_INFO
#endif 
#define DBG_COLOR
#include <rtdbg.h>

/* 42-PE11 */
#define QSPI10_CS    GET_PIN(E, 11)

static void qspi_send_cmd(rt_uint32_t instruction, rt_uint32_t address, rt_uint32_t dummyCycles, rt_uint32_t instructionMode, rt_uint32_t addressMode, rt_uint32_t addressSize, rt_uint32_t dataMode, rt_uint32_t datasize);

struct stm32_hw_spi_cs
{
    rt_uint32_t pin;
};

struct stm32_qspi
{
    QSPI_HandleTypeDef QSPI_Handler;
    struct rt_spi_configuration *cfg;
} hqspi;

struct rt_qspi_ops
{
    rt_err_t (*configure)(struct rt_spi_device *device, struct rt_spi_configuration *configuration);
    rt_uint32_t (*xfer)(struct rt_spi_device *device, struct rt_spi_message *message);
};

static int stm32_qspi_init(QUADSPI_TypeDef *qspix, struct rt_spi_configuration *cfg)
{
    static rt_bool_t init_ok = RT_FALSE;
    int result = RT_EOK;

    if (init_ok)
    {
        LOG_D("qspi already init!");
        return RT_EOK;
    }

    rt_memset(&hqspi, 0, sizeof(hqspi));

    hqspi.QSPI_Handler.Instance = qspix;
    /* 80/(1+1)=40M */
    hqspi.QSPI_Handler.Init.ClockPrescaler = 1;   
    /* fifo threshold is 4 byte */
    hqspi.QSPI_Handler.Init.FifoThreshold = 4;
    /* Sampling shift half a cycle */
    hqspi.QSPI_Handler.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    /* flash size */
    hqspi.QSPI_Handler.Init.FlashSize = POSITION_VAL(0X1000000) - 1;
    /* cs high time */
    hqspi.QSPI_Handler.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_4_CYCLE;
    /* QSPI MODE 0 */
    hqspi.QSPI_Handler.Init.ClockMode = QSPI_CLOCK_MODE_0;

    result = HAL_QSPI_Init(&hqspi.QSPI_Handler);
    if (result  == HAL_OK)
    {
        LOG_D("qspi init succsee!");
        init_ok = RT_TRUE;
    }
    else
    {
        LOG_E("qspi init failed (%d)!", result);
        result = -RT_ERROR;
    }

    return result;
}

static rt_uint32_t qspixfer(struct rt_spi_device *device, struct rt_spi_message *message)
{
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);
    
    rt_size_t len = 0;

    struct stm32_hw_spi_cs *cs = device->parent.user_data;
    
    const rt_uint8_t *sndb = message->send_buf;
    rt_uint8_t *rcvb = message->recv_buf;
    rt_int32_t length = message->length;

    if (message->cs_take)
    {
        rt_pin_write(cs->pin, 0);
    }
    
    /* send data */
    if (sndb)
    {

        qspi_send_cmd(0, 0, 0, QSPI_INSTRUCTION_NONE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE, length);

        if (HAL_QSPI_Transmit(&hqspi.QSPI_Handler, (rt_uint8_t *)sndb, 5000) == HAL_OK) //·¢ËÍÊý¾Ý
        {

            len = length;
        }
        else
        {
            LOG_E("qspi send data failed!");
            goto __exit;
        }
    }
    else if (rcvb)/* recv data */
    {
        qspi_send_cmd(0, 0 , 0, QSPI_INSTRUCTION_NONE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE, length);

        if (HAL_QSPI_Receive(&hqspi.QSPI_Handler, rcvb, 5000) == HAL_OK)
        {
            len = length;
        }
        else
        {
            LOG_E("qspi recv data failed");
            goto __exit;
        }
    }

__exit:
    if (message->cs_release)
    {
        rt_pin_write(cs->pin, 1);
    }
    
    return len;
}

rt_err_t qspi_configure(struct rt_spi_device *device,struct rt_spi_configuration *configuration)
{
    struct stm32_qspi *hqspi = (struct stm32_qspi *)device->bus->parent.user_data;
    
    hqspi->cfg = configuration;
    
    return stm32_qspi_init(hqspi->QSPI_Handler.Instance, configuration);
}

const struct rt_spi_ops stm_qspi_ops =
{
    .configure = qspi_configure,
    .xfer = qspixfer,
};

struct rt_spi_bus _qspi_bus1;
struct stm32_qspi _qspi1;
int stm32_qspi_register_bus(QUADSPI_TypeDef *qspix, const char *name)
{
    struct rt_spi_bus *spi_bus;
    struct stm32_qspi *qspi;
    
    if (qspix == QUADSPI)
    {
        spi_bus = &_qspi_bus1;
        qspi = &_qspi1;
    }
    else
    {
        return -1;
    }
    
    qspi->QSPI_Handler.Instance = QUADSPI;
    spi_bus->parent.user_data = qspi;
    
    return rt_spi_bus_register(spi_bus, name, &stm_qspi_ops);
}

/**
 * attach device to qspi bus
 *
 * @param   pin              qspi cs pin number
 * @param   bus_name         qspi bus name
 * @param   device_name      qspi device name
 *
 * @return  0 : success
 *         -1 : failed
 */
rt_err_t stm32_qspi_bus_attach_device(rt_uint32_t pin, const char *bus_name, const char *device_name)
{
    struct rt_spi_device *spi_device = (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
    RT_ASSERT(spi_device != RT_NULL);
    struct stm32_hw_spi_cs *cs_pin = (struct stm32_hw_spi_cs *)rt_malloc(sizeof(struct stm32_hw_spi_cs));
    RT_ASSERT(cs_pin != RT_NULL);
    
    cs_pin->pin = pin;
    rt_pin_mode(pin, PIN_MODE_OUTPUT);
    rt_pin_write(pin, 1);
    
    return rt_spi_bus_attach_device(spi_device, device_name, bus_name, (void *)cs_pin);
}

int stm32_hw_qspi_init(void)
{
    int result = RT_EOK;

    result = stm32_qspi_register_bus(QUADSPI, "qspi1");
    if (result != RT_EOK)
    {
        return result;
    }

    result = stm32_qspi_bus_attach_device(QSPI10_CS, "qspi1", "qspi10");
    if (result != RT_EOK)
    {
        return result;
    }

    return result;
}
INIT_BOARD_EXPORT(stm32_hw_qspi_init);

void HAL_QSPI_MspInit(QSPI_HandleTypeDef *hqspi)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* QSPI clock enable */
    __HAL_RCC_QSPI_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /* QSPI1 GPIO Configuration
    PE10     ------> QSPI_BK1_CLK
    PE11     ------> QSPI_SOFT_CS
    PE12     ------> QSPI_BK1_IO0
    PE13     ------> QSPI_BK1_IO1
    PE14     ------> QSPI_BK1_IO2
    PE15     ------> QSPI_BK1_IO3
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef *hqspi)
{
    /* Peripheral clock disable */
    __HAL_RCC_QSPI_CLK_DISABLE();
    /* QSPI GPIO Configuration
    PE10     ------> QSPI_BK1_CLK
    PE11     ------> QSPI_SOFT_CS
    PE12     ------> QSPI_BK1_IO0
    PE13     ------> QSPI_BK1_IO1
    PE14     ------> QSPI_BK1_IO2
    PE15     ------> QSPI_BK1_IO3
    */
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
}

/**
 * qspi send cmd
 *
 * @param   instruction     the command to be sent
 * @param   address         message to be sent
 * @param   dummyCycles     dummy cycles
 * @param   instructionMode instruction Mode
 * @param   addressMode     address Mode
 * @param   addressSize     address Size
 * @param   dataMode        data Mode
 *
 * @return  void
 */
static void qspi_send_cmd(rt_uint32_t instruction, rt_uint32_t address, rt_uint32_t dummycycles, rt_uint32_t instructionmode, rt_uint32_t addressmode, rt_uint32_t addresssize, rt_uint32_t datamode, rt_uint32_t datasize)
{
    QSPI_CommandTypeDef Cmdhandler;

    Cmdhandler.Instruction = instruction;
    Cmdhandler.Address = address;
    Cmdhandler.DummyCycles = dummycycles;
    Cmdhandler.InstructionMode = instructionmode;
    Cmdhandler.AddressMode = addressmode;
    Cmdhandler.AddressSize = addresssize;
    Cmdhandler.DataMode = datamode;
    Cmdhandler.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    Cmdhandler.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    Cmdhandler.DdrMode = QSPI_DDR_MODE_DISABLE;      
    Cmdhandler.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    Cmdhandler.NbData = datasize;
    HAL_QSPI_Command(&hqspi.QSPI_Handler, &Cmdhandler, 5000);
}
#endif /*BSP_USING_QSPI*/
