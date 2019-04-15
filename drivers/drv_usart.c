/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2010-03-29     Bernard      remove interrupt Tx and DMA Rx mode
 * 2012-02-08     aozima       update for F4.
 * 2012-07-28     aozima       update for ART board.
 * 2016-05-28     armink       add DMA Rx mode
 */

#include <stm32l4xx.h>
#include <rtdevice.h>
#include <rthw.h>

#ifdef BSP_USING_UART1
    #define USART1_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
    #define USART1_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

    /* Definition for USART1 Pins */
    #define USART1_TX_PIN                    GPIO_PIN_9
    #define USART1_TX_GPIO_PORT              GPIOA
    #define USART1_TX_AF                     GPIO_AF7_USART1
    #define USART1_RX_PIN                    GPIO_PIN_10
    #define USART1_RX_GPIO_PORT              GPIOA
    #define USART1_RX_AF                     GPIO_AF7_USART1

    #define USART1_RX_DMA_CHANNEL            DMA1_Channel5
    #define USART1_RX_DMA_REUQEST            DMA_REQUEST_2
    #define USART1_RX_DMA_IRQN               DMA1_Channel5_IRQn
    #define USART1_RX_DMA_IRQHandler         DMA1_Channel5_IRQHandler
#endif

#ifdef BSP_USING_UART2
    #define USART2_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
    #define USART2_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

    /* Definition for USART2 Pins */
    #define USART2_TX_PIN                    GPIO_PIN_2
    #define USART2_TX_GPIO_PORT              GPIOA
    #define USART2_TX_AF                     GPIO_AF7_USART2
    #define USART2_RX_PIN                    GPIO_PIN_3
    #define USART2_RX_GPIO_PORT              GPIOA
    #define USART2_RX_AF                     GPIO_AF7_USART2

    #define USART2_RX_DMA_CHANNEL            DMA1_Channel6
    #define USART2_RX_DMA_REUQEST            DMA_REQUEST_2
    #define USART2_RX_DMA_IRQN               DMA1_Channel6_IRQn
    #define USART2_RX_DMA_IRQHandler         DMA1_Channel6_IRQHandler
#endif

#ifdef BSP_USING_UART3
    #define USART3_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
    #define USART3_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()

    /* Definition for USART3 Pins */
    #define USART3_TX_PIN                    GPIO_PIN_4
    #define USART3_TX_GPIO_PORT              GPIOC
    #define USART3_TX_AF                     GPIO_AF7_USART3
    #define USART3_RX_PIN                    GPIO_PIN_5
    #define USART3_RX_GPIO_PORT              GPIOC
    #define USART3_RX_AF                     GPIO_AF7_USART3

    #define USART3_RX_DMA_CHANNEL            DMA1_Channel3
    #define USART3_RX_DMA_REUQEST            DMA_REQUEST_2
    #define USART3_RX_DMA_IRQN               DMA1_Channel3_IRQn
    #define USART3_RX_DMA_IRQHandler         DMA1_Channel3_IRQHandler
#endif

#ifdef BSP_USING_LPUART1
    #define LPUART1_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
    #define LPUART1_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

    /* Definition for LPUART1 Pins */
    #define LPUART1_TX_PIN                    GPIO_PIN_11
    #define LPUART1_TX_GPIO_PORT              GPIOB
    #define LPUART1_TX_AF                     GPIO_AF8_LPUART1
    #define LPUART1_RX_PIN                    GPIO_PIN_10
    #define LPUART1_RX_GPIO_PORT              GPIOB
    #define LPUART1_RX_AF                     GPIO_AF8_LPUART1

    #define LPUART1_RX_DMA_CHANNEL            DMA2_Channel7
    #define LPUART1_RX_DMA_REUQEST            DMA_REQUEST_4
    #define LPUART1_RX_DMA_IRQN               DMA2_Channel7_IRQn
    #define LPUART1_RX_DMA_IRQHandler         DMA2_Channel7_IRQHandler
#endif

/* STM32 uart driver */
struct stm32_uart
{
    UART_HandleTypeDef UartHandle;
    IRQn_Type irq;

#ifdef BSP_UART_USING_DMA_RX
    IRQn_Type dma_irq;
    rt_size_t last_index;
    DMA_HandleTypeDef hdma_rx;
#endif

    char * uart_name;
    struct rt_serial_device serial;
};

#ifdef BSP_UART_USING_DMA_RX
static void stm32_dma_config(struct rt_serial_device *serial);
#endif

static rt_err_t stm32_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct stm32_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    uart = (struct stm32_uart *)serial->parent.user_data;
    RT_ASSERT(uart != RT_NULL);

    uart->UartHandle.Init.BaudRate     = cfg->baud_rate;
    uart->UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    uart->UartHandle.Init.Mode         = UART_MODE_TX_RX;
    uart->UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    switch (cfg->data_bits)
    {
    case DATA_BITS_8:
        uart->UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
        break;
    case DATA_BITS_9:
        uart->UartHandle.Init.WordLength = UART_WORDLENGTH_9B;
        break;
    default:
        uart->UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
        break;
    }

    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
        uart->UartHandle.Init.StopBits   = UART_STOPBITS_1;
        break;
    case STOP_BITS_2:
        uart->UartHandle.Init.StopBits   = UART_STOPBITS_2;
        break;
    default:
        uart->UartHandle.Init.StopBits   = UART_STOPBITS_1;
        break;
    }

    switch (cfg->parity)
    {
    case PARITY_NONE:
        uart->UartHandle.Init.Parity     = UART_PARITY_NONE;
        break;
    case PARITY_ODD:
        uart->UartHandle.Init.Parity     = UART_PARITY_ODD;
        break;
    case PARITY_EVEN:
        uart->UartHandle.Init.Parity     = UART_PARITY_EVEN;
        break;
    default:
        uart->UartHandle.Init.Parity     = UART_PARITY_NONE;
        break;
    }

    if (HAL_UART_Init(&uart->UartHandle) != HAL_OK)
    {
        return -RT_ERROR;
    }

    if (uart->UartHandle.Instance == LPUART1)
    {
        UART_WakeUpTypeDef WakeUpSelection;
        WakeUpSelection.WakeUpEvent = UART_WAKEUP_ON_READDATA_NONEMPTY;

        if (HAL_UARTEx_StopModeWakeUpSourceConfig(&uart->UartHandle, WakeUpSelection) != HAL_OK)
        {
            return -RT_ERROR;
        }
        HAL_UARTEx_EnableStopMode(&uart->UartHandle);
    }

    return RT_EOK;
}

static rt_err_t stm32_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct stm32_uart *uart;

    RT_ASSERT(serial != RT_NULL);

    uart = (struct stm32_uart *)serial->parent.user_data;
    RT_ASSERT(uart != RT_NULL);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        /* disable rx irq */
        NVIC_DisableIRQ(uart->irq);
        /* disable interrupt */
        __HAL_UART_DISABLE_IT(&uart->UartHandle, UART_IT_RXNE);
        break;

    case RT_DEVICE_CTRL_SET_INT:
        /* enable rx irq */
        NVIC_EnableIRQ(uart->irq);
        /* enable interrupt */
        __HAL_UART_ENABLE_IT(&uart->UartHandle, UART_IT_RXNE);
        break;

#ifdef BSP_UART_USING_DMA_RX
    case RT_DEVICE_CTRL_CONFIG:
        if ((rt_ubase_t)arg == RT_DEVICE_FLAG_DMA_RX)
        {
            stm32_dma_config(serial);
        }
        break;
#endif
    }

    return RT_EOK;
}

static int stm32_putc(struct rt_serial_device *serial, char c)
{
    struct stm32_uart *uart;

    RT_ASSERT(serial != RT_NULL);

    uart = (struct stm32_uart *)serial->parent.user_data;
    RT_ASSERT(uart != RT_NULL);

    uart->UartHandle.Instance->TDR = c;
    while ((__HAL_UART_GET_FLAG(&uart->UartHandle, UART_FLAG_TC) == RESET));

    return 1;
}

static int stm32_getc(struct rt_serial_device *serial)
{
    int ch;
    struct stm32_uart *uart;

    RT_ASSERT(serial != RT_NULL);

    uart = (struct stm32_uart *)serial->parent.user_data;
    RT_ASSERT(uart != RT_NULL);

    ch = -1;
    if (__HAL_UART_GET_FLAG(&uart->UartHandle, UART_FLAG_RXNE) != RESET)
        ch = uart->UartHandle.Instance->RDR & 0xff;

    return ch;
}

/**
 * Uart common interrupt process. This need add to uart ISR.
 *
 * @param serial serial device
 */
static void uart_isr(struct rt_serial_device *serial)
{
    struct stm32_uart *uart;

    RT_ASSERT(serial != RT_NULL);

    uart = (struct stm32_uart *) serial->parent.user_data;
    RT_ASSERT(uart != RT_NULL);

    /* UART in mode Receiver -------------------------------------------------*/
    if ((__HAL_UART_GET_FLAG(&uart->UartHandle, UART_FLAG_RXNE) != RESET) &&
        (__HAL_UART_GET_IT_SOURCE(&uart->UartHandle, UART_IT_RXNE) != RESET))
    {
        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_IND);
        /* Clear RXNE interrupt flag */
        __HAL_UART_CLEAR_FLAG(&uart->UartHandle, UART_FLAG_RXNE);
    }
#ifdef BSP_UART_USING_DMA_RX
    else if ((__HAL_UART_GET_FLAG(&uart->UartHandle, UART_FLAG_IDLE) != RESET) &&
             (__HAL_UART_GET_IT_SOURCE(&uart->UartHandle, UART_IT_IDLE) != RESET))
    {
        rt_base_t level;
        rt_size_t recv_total_index, recv_len;
        level = rt_hw_interrupt_disable();
        recv_total_index = serial->config.bufsz - __HAL_DMA_GET_COUNTER(uart->UartHandle.hdmarx);
        recv_len = recv_total_index - uart->last_index;
        uart->last_index = recv_total_index;
        rt_hw_interrupt_enable(level);

        if (recv_len)
        {
            rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_DMADONE | (recv_len << 8));
        }

        __HAL_UART_CLEAR_IDLEFLAG(&uart->UartHandle);
    }
#endif
    else
    {
        if (__HAL_UART_GET_FLAG(&uart->UartHandle, UART_FLAG_ORE) != RESET)
        {
            __HAL_UART_CLEAR_OREFLAG(&uart->UartHandle);
        }
        if (__HAL_UART_GET_FLAG(&uart->UartHandle, UART_FLAG_NE) != RESET)
        {
            __HAL_UART_CLEAR_NEFLAG(&uart->UartHandle);
        }
        if (__HAL_UART_GET_FLAG(&uart->UartHandle, UART_FLAG_FE) != RESET)
        {
            __HAL_UART_CLEAR_FEFLAG(&uart->UartHandle);
        }
        if (__HAL_UART_GET_FLAG(&uart->UartHandle, UART_FLAG_PE) != RESET)
        {
            __HAL_UART_CLEAR_PEFLAG(&uart->UartHandle);
        }
        if (__HAL_UART_GET_FLAG(&uart->UartHandle, UART_FLAG_WUF) != RESET)
        {
            __HAL_UART_CLEAR_FLAG(&uart->UartHandle, UART_CLEAR_WUF);
        }
    }
}

/**
* @brief UART MSP Initialization
*        This function configures the hardware resources used in this example:
*           - Peripheral's clock enable
*           - Peripheral's GPIO Configuration
*           - NVIC configuration for UART interrupt request enable
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

#if defined(BSP_USING_UART1)
    if (huart->Instance == USART1)
    {
        /*##-1- Enable peripherals and GPIO Clocks #################################*/
        /* Enable GPIO TX/RX clock */
        USART1_TX_GPIO_CLK_ENABLE();
        USART1_RX_GPIO_CLK_ENABLE();

        /* Enable USARTx clock */
        __HAL_RCC_USART1_CLK_ENABLE();

        /*##-2- Configure peripheral GPIO ##########################################*/
        /* UART TX GPIO pin configuration  */
        GPIO_InitStruct.Pin       = USART1_TX_PIN;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = USART1_TX_AF;

        HAL_GPIO_Init(USART1_TX_GPIO_PORT, &GPIO_InitStruct);

        /* UART RX GPIO pin configuration  */
        GPIO_InitStruct.Pin = USART1_RX_PIN;
        GPIO_InitStruct.Alternate = USART1_RX_AF;

        HAL_GPIO_Init(USART1_RX_GPIO_PORT, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
    else
#endif

#if defined(BSP_USING_UART2)
    if (huart->Instance == USART2)
    {
        /*##-1- Enable peripherals and GPIO Clocks #################################*/
        /* Enable GPIO TX/RX clock */
        USART2_TX_GPIO_CLK_ENABLE();
        USART2_RX_GPIO_CLK_ENABLE();

        /* Enable USARTx clock */
        __HAL_RCC_USART2_CLK_ENABLE();

        /*##-2- Configure peripheral GPIO ##########################################*/
        /* UART TX GPIO pin configuration  */
        GPIO_InitStruct.Pin       = USART2_TX_PIN;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = USART2_TX_AF;

        HAL_GPIO_Init(USART2_TX_GPIO_PORT, &GPIO_InitStruct);

        /* UART RX GPIO pin configuration  */
        GPIO_InitStruct.Pin = USART2_RX_PIN;
        GPIO_InitStruct.Alternate = USART2_RX_AF;

        HAL_GPIO_Init(USART2_RX_GPIO_PORT, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(USART2_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
    else
#endif

#if defined(BSP_USING_UART3)
    if (huart->Instance == USART3)
    {
        /*##-1- Enable peripherals and GPIO Clocks #################################*/
        /* Enable GPIO TX/RX clock */
        USART3_TX_GPIO_CLK_ENABLE();
        USART3_RX_GPIO_CLK_ENABLE();

        /* Enable USARTx clock */
        __HAL_RCC_USART3_CLK_ENABLE();

        /*##-2- Configure peripheral GPIO ##########################################*/
        /* UART TX GPIO pin configuration  */
        GPIO_InitStruct.Pin       = USART3_TX_PIN;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = USART3_TX_AF;

        HAL_GPIO_Init(USART3_TX_GPIO_PORT, &GPIO_InitStruct);

        /* UART RX GPIO pin configuration  */
        GPIO_InitStruct.Pin = USART3_RX_PIN;
        GPIO_InitStruct.Alternate = USART3_RX_AF;

        HAL_GPIO_Init(USART3_RX_GPIO_PORT, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(USART3_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(USART3_IRQn);
    }
    else
#endif

#if defined(BSP_USING_LPUART1)
    if (huart->Instance == LPUART1)
    {
        /*##-1- Enable peripherals and GPIO Clocks #################################*/
        /* Enable GPIO TX/RX clock */
        LPUART1_TX_GPIO_CLK_ENABLE();
        LPUART1_RX_GPIO_CLK_ENABLE();

        /* Enable USARTx clock */
        __HAL_RCC_LPUART1_CLK_ENABLE();

        /*##-2- Configure peripheral GPIO ##########################################*/
        /* UART TX GPIO pin configuration  */
        GPIO_InitStruct.Pin       = LPUART1_TX_PIN;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = LPUART1_TX_AF;

        HAL_GPIO_Init(LPUART1_TX_GPIO_PORT, &GPIO_InitStruct);

        /* UART RX GPIO pin configuration  */
        GPIO_InitStruct.Pin       = LPUART1_RX_PIN;
        GPIO_InitStruct.Alternate = LPUART1_RX_AF;

        HAL_GPIO_Init(LPUART1_RX_GPIO_PORT, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(LPUART1_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(LPUART1_IRQn);
    }
    else
#endif

    {
        RT_ASSERT(0);
    }
}

/**
* @brief UART MSP De-Initialization
*        This function frees the hardware resources used in this example:
*          - Disable the Peripheral's clock
*          - Revert GPIO and NVIC configuration to their default state
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
#if defined(BSP_USING_UART1)
    if (huart->Instance == USART1)
    {
        /*##-1- Reset peripherals ##################################################*/
        __HAL_RCC_USART1_FORCE_RESET();
        __HAL_RCC_USART1_RELEASE_RESET();

        /*##-2- Disable peripherals and GPIO Clocks #################################*/
        /* Configure UART Tx as alternate function  */
        HAL_GPIO_DeInit(USART1_TX_GPIO_PORT, USART1_TX_PIN);
        /* Configure UART Rx as alternate function  */
        HAL_GPIO_DeInit(USART1_RX_GPIO_PORT, USART1_RX_PIN);

        HAL_NVIC_DisableIRQ(USART1_IRQn);
    }
    else
#endif

#if defined(BSP_USING_UART2)
    if (huart->Instance == USART2)
    {
        /*##-1- Reset peripherals ##################################################*/
        __HAL_RCC_USART2_FORCE_RESET();
        __HAL_RCC_USART2_RELEASE_RESET();

        /*##-2- Disable peripherals and GPIO Clocks #################################*/
        /* Configure UART Tx as alternate function  */
        HAL_GPIO_DeInit(USART2_TX_GPIO_PORT, USART2_TX_PIN);
        /* Configure UART Rx as alternate function  */
        HAL_GPIO_DeInit(USART2_RX_GPIO_PORT, USART2_RX_PIN);

        HAL_NVIC_DisableIRQ(USART2_IRQn);
    }
    else
#endif

#if defined(BSP_USING_UART3)
    if (huart->Instance == USART3)
    {
        /*##-1- Reset peripherals ##################################################*/
        __HAL_RCC_USART3_FORCE_RESET();
        __HAL_RCC_USART3_RELEASE_RESET();

        /*##-2- Disable peripherals and GPIO Clocks #################################*/
        /* Configure UART Tx as alternate function  */
        HAL_GPIO_DeInit(USART3_TX_GPIO_PORT, USART3_TX_PIN);
        /* Configure UART Rx as alternate function  */
        HAL_GPIO_DeInit(USART3_RX_GPIO_PORT, USART3_RX_PIN);

        HAL_NVIC_DisableIRQ(USART3_IRQn);
    }
    else
#endif

#if defined(BSP_USING_LPUART1)
    if (huart->Instance == LPUART1)
    {
        /*##-1- Reset peripherals ##################################################*/
        __HAL_RCC_LPUART1_FORCE_RESET();
        __HAL_RCC_LPUART1_RELEASE_RESET();

        /*##-2- Disable peripherals and GPIO Clocks #################################*/
        /* Configure UART Tx as alternate function  */
        HAL_GPIO_DeInit(LPUART1_TX_GPIO_PORT, LPUART1_TX_PIN);
        /* Configure UART Rx as alternate function  */
        HAL_GPIO_DeInit(LPUART1_RX_GPIO_PORT, LPUART1_RX_PIN);

        HAL_NVIC_DisableIRQ(LPUART1_IRQn);
    }
    else
#endif

    {
        RT_ASSERT(0);
    }
}

static const struct rt_uart_ops stm32_uart_ops =
{
    stm32_configure,
    stm32_control,
    stm32_putc,
    stm32_getc,
};

#if defined(BSP_USING_UART1)
/* UART1 device driver structure */
static struct stm32_uart uart1 =
{
    {USART1},               // UART_HandleTypeDef UartHandle;
    USART1_IRQn,            // IRQn_Type irq;

#ifdef BSP_UART_USING_DMA_RX
    USART1_RX_DMA_IRQN,     // IRQn_Type dma_irq;
    0,                      // rt_size_t last_index;
    // DMA_HandleTypeDef hdma_rx;
    {USART1_RX_DMA_CHANNEL, {USART1_RX_DMA_REUQEST}},
#endif

    "uart1",                // char * uart_name;
    // struct rt_serial_device serial;
    {{0}, &stm32_uart_ops, RT_SERIAL_CONFIG_DEFAULT}
};

void USART1_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    uart_isr(&uart1.serial);

    /* leave interrupt */
    rt_interrupt_leave();
}

void USART1_RX_DMA_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(uart1.UartHandle.hdmarx);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* BSP_USING_UART1 */

#if defined(BSP_USING_UART2)
/* UART2 device driver structure */
static struct stm32_uart uart2 =
{
    {USART2},               // UART_HandleTypeDef UartHandle;
    USART2_IRQn,            // IRQn_Type irq;

#ifdef BSP_UART_USING_DMA_RX
    USART2_RX_DMA_IRQN,     // IRQn_Type dma_irq;
    0,                      // rt_size_t last_index;
    // DMA_HandleTypeDef hdma_rx;
    {USART2_RX_DMA_CHANNEL, {USART2_RX_DMA_REUQEST}},
#endif

    "uart2",                // char * uart_name;
    // struct rt_serial_device serial;
    {{0}, &stm32_uart_ops, RT_SERIAL_CONFIG_DEFAULT}
};

void USART2_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    uart_isr(&uart2.serial);

    /* leave interrupt */
    rt_interrupt_leave();
}

void USART2_RX_DMA_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(uart2.UartHandle.hdmarx);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* BSP_USING_UART2 */

#if defined(BSP_USING_UART3)
/* UART3 device driver structure */
static struct stm32_uart uart3 =
{
    {USART3},               // UART_HandleTypeDef UartHandle;
    USART3_IRQn,            // IRQn_Type irq;

#ifdef BSP_UART_USING_DMA_RX
    USART3_RX_DMA_IRQN,     // IRQn_Type dma_irq;
    0,                      // rt_size_t last_index;
    // DMA_HandleTypeDef hdma_rx;
    {USART3_RX_DMA_CHANNEL, {USART3_RX_DMA_REUQEST}},
#endif

    "uart3",                // char * uart_name;
    // struct rt_serial_device serial;
    {{0}, &stm32_uart_ops, RT_SERIAL_CONFIG_DEFAULT}
};

void USART3_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    uart_isr(&uart3.serial);

    /* leave interrupt */
    rt_interrupt_leave();
}

void USART3_RX_DMA_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(uart3.UartHandle.hdmarx);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* BSP_USING_UART3 */

#if defined(BSP_USING_LPUART1)
/* lpuart1 device driver structure */
static struct stm32_uart lpuart1 =
{
    {LPUART1},              // UART_HandleTypeDef UartHandle;
    LPUART1_IRQn,           // IRQn_Type irq;

#ifdef BSP_UART_USING_DMA_RX
    LPUART1_RX_DMA_IRQN,    // IRQn_Type dma_irq;
    0,                      // rt_size_t last_index;
    // DMA_HandleTypeDef hdma_rx;
    {LPUART1_RX_DMA_CHANNEL, {LPUART1_RX_DMA_REUQEST}},
#endif

    "lpuart1",              // char * uart_name;
    // struct rt_serial_device serial;
    {
        {0},
        &stm32_uart_ops,
        {
            /* LPUART1 using 32.768 kHz LSE clock, and support up to 9600 baud/s.
             * But in my test, the most of data received at 9600 baud/s was wrong.
             */
            BAUD_RATE_4800,     /* 4800 bits/s    */
            DATA_BITS_8,        /* 8 databits     */
            STOP_BITS_1,        /* 1 stopbit      */
            PARITY_NONE,        /* No parity      */
            BIT_ORDER_LSB,      /* LSB first sent */
            NRZ_NORMAL,         /* Normal mode    */
            RT_SERIAL_RB_BUFSZ, /* Buffer size    */
            0
        }
    }
};

void LPUART1_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    uart_isr(&lpuart1.serial);

    /* leave interrupt */
    rt_interrupt_leave();
}

void LPUART1_RX_DMA_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(lpuart1.UartHandle.hdmarx);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* BSP_USING_LPUART1 */

#ifdef BSP_UART_USING_DMA_RX
static void stm32_dma_config(struct rt_serial_device *serial)
{
    struct stm32_uart *uart;
    UART_HandleTypeDef *uart_handle;
    struct rt_serial_rx_fifo *rx_fifo;

    RT_ASSERT(serial != RT_NULL);

    uart = (struct stm32_uart *)serial->parent.user_data;
    RT_ASSERT(uart != RT_NULL);

    uart_handle = &uart->UartHandle;
    RT_ASSERT(uart_handle != RT_NULL);

    if ((uint32_t)(uart->hdma_rx.Instance) < (uint32_t)(DMA2_Channel1))
    {
        __HAL_RCC_DMA1_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_DMA2_CLK_ENABLE();
    }

    HAL_NVIC_SetPriority(uart->dma_irq, 0, 0);
    HAL_NVIC_EnableIRQ(uart->dma_irq);

    uart->hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    uart->hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    uart->hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
    uart->hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    uart->hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    uart->hdma_rx.Init.Mode = DMA_CIRCULAR;
    uart->hdma_rx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&uart->hdma_rx) != HAL_OK)
    {
        RT_ASSERT(0);
    }

    __HAL_LINKDMA(uart_handle, hdmarx, uart->hdma_rx);

    rx_fifo = (struct rt_serial_rx_fifo *)serial->serial_rx;
    if (HAL_UART_Receive_DMA(&uart->UartHandle, rx_fifo->buffer, serial->config.bufsz) != HAL_OK)
    {
        /* Transfer error in reception process */
        RT_ASSERT(0);
    }

    /* enable interrupt */
    __HAL_UART_ENABLE_IT(&uart->UartHandle, UART_IT_IDLE);
    /* enable rx irq */
    NVIC_EnableIRQ(uart->irq);
}

/**
  * @brief  UART error callbacks
  * @param  huart: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    struct stm32_uart *uart;

    RT_ASSERT(huart != NULL);
    uart = (struct stm32_uart *)huart;

    rt_kprintf("%s: %s %d\n", __FUNCTION__, uart->uart_name, huart->ErrorCode);
}

/**
  * @brief  Rx Transfer completed callback
  * @param  huart: UART handle
  * @note   This example shows a simple way to report end of DMA Rx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    struct rt_serial_device *serial;
    struct stm32_uart *uart;
    rt_size_t recv_len;
    rt_base_t level;

    RT_ASSERT(huart != NULL);
    uart = (struct stm32_uart *)huart;
    serial = &uart->serial;

    level = rt_hw_interrupt_disable();

    recv_len = serial->config.bufsz - uart->last_index;
    uart->last_index = 0;

    rt_hw_interrupt_enable(level);
    if (recv_len)
    {
        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_DMADONE | (recv_len << 8));
    }

}
#endif

int stm32_hw_usart_init(void)
{
    struct stm32_uart* uarts[] =
    {
#ifdef BSP_USING_UART1
        &uart1,
#endif

#ifdef BSP_USING_UART2
        &uart2,
#endif

#ifdef BSP_USING_UART3
        &uart3,
#endif

#ifdef BSP_USING_LPUART1
        &lpuart1,
#endif
    };
    int i;

    for (i = 0; i < sizeof(uarts) / sizeof(uarts[0]); i++)
    {
        struct stm32_uart *uart = uarts[i];
        rt_err_t result;

        /* register UART device */
        result = rt_hw_serial_register(&uart->serial,
                              uart->uart_name,
#ifdef BSP_UART_USING_DMA_RX
                              RT_DEVICE_FLAG_DMA_RX |
#endif
                              RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                              uart);
        RT_ASSERT(result == RT_EOK);
        (void)result;
    }

    return 0;
}
