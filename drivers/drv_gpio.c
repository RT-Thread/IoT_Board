/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2017-11-20     DQL               the first version
 * 2018-08-16     Tanek             add IO function description
 */

#include <rthw.h>
#include <rtdevice.h>
#include <board.h>

#ifdef BSP_USING_GPIO

#define __STM32_PIN(index, gpio, gpio_index)                                \
    {                                                                       \
        index, GPIO##gpio##_CLK_ENABLE, GPIO##gpio, GPIO_PIN_##gpio_index   \
    }

#define __STM32_PIN_DEFAULT                                                 \
    {                                                                       \
        -1, 0, 0, 0                                                         \
    }

static void GPIOA_CLK_ENABLE(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

static void GPIOB_CLK_ENABLE(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

static void GPIOC_CLK_ENABLE(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
}

static void GPIOD_CLK_ENABLE(void)
{
    __HAL_RCC_GPIOD_CLK_ENABLE();
}

static void GPIOE_CLK_ENABLE(void)
{
    __HAL_RCC_GPIOE_CLK_ENABLE();
}

/* STM32 GPIO driver */
struct pin_index
{
    int index;
    void (*rcc)(void);
    GPIO_TypeDef *gpio;
    uint32_t pin;
};

static const struct pin_index pins[] =
{
    __STM32_PIN_DEFAULT,
    __STM32_PIN(1, E, 2),       // PE2 :  SAI1_MCLK_A  --> ES8388
    __STM32_PIN(2, E, 3),       // PE3 :  SAI1_SD_B    --> ES8388
    __STM32_PIN(3, E, 4),       // PE4 :  SAI1_FS_A    --> ES8388
    __STM32_PIN(4, E, 5),       // PE5 :  SAI1_SCK_A   --> ES8388
    __STM32_PIN(5, E, 6),       // PE6 :  SAI1_SD_A    --> ES8388
    __STM32_PIN_DEFAULT,        //     :  VBAT
    __STM32_PIN(7, C, 13),      // PC13:  SD_CS        --> SD_CARD
    __STM32_PIN(8, C, 14),      // PC14:  OSC32_IN
    __STM32_PIN(9, C, 15),      // PC15:  OSC32_OUT
    __STM32_PIN_DEFAULT,        //     :  VSS
    __STM32_PIN_DEFAULT,        //     :  VDD
    __STM32_PIN_DEFAULT,        // PH0 :  OSC_IN
    __STM32_PIN_DEFAULT,        // PH1 :  OSC_OUT
    __STM32_PIN_DEFAULT,        //     :  RESET
    __STM32_PIN(15, C, 0),      // PC0 :  I2C_SCL      --> ES8388
    __STM32_PIN(16, C, 1),      // PC1 :  I2C_SDA      --> ES8388
    __STM32_PIN(17, C, 2),      // PC2 :  GBC_LED      --> ATK MODULE
    __STM32_PIN(18, C, 3),      // PC3 :  GBC_KEY      --> ATK MODULE
    __STM32_PIN_DEFAULT,        //     :  VSSA
    __STM32_PIN_DEFAULT,        //     :  VREF-
    __STM32_PIN_DEFAULT,        //     :  VREF+
    __STM32_PIN_DEFAULT,        //     :  VDDA
    __STM32_PIN(23, A, 0),      // PA0 :  MOTOR_A      --> MOTOR
    __STM32_PIN(24, A, 1),      // PA1 :  MOTOR_B      --> MOTOR
    __STM32_PIN(25, A, 2),      // PA2 :  UART2_TX     --> EXTERNAL MODULE
    __STM32_PIN(26, A, 3),      // PA3 :  UART2_RX     --> EXTERNAL MODULE
    __STM32_PIN_DEFAULT,        //     :  VSS
    __STM32_PIN_DEFAULT,        //     :  VDD
    __STM32_PIN(29, A, 4),      // PA4 :  ADC12_IN9    --> EXTERNAL MODULE
    __STM32_PIN(30, A, 5),      // PA5 :  SPI1_SCK     --> SD_CARD
    __STM32_PIN(31, A, 6),      // PA6 :  SPI1_MISO    --> SD_CARD
    __STM32_PIN(32, A, 7),      // PA7 :  SPI1_MOSI    --> SD_CARD
    __STM32_PIN(33, C, 4),      // PC4 :  GBC_RX       --> ATK MODULE
    __STM32_PIN(34, C, 5),      // PC5 :  WIFI_INT     --> WIFI
    __STM32_PIN(35, B, 0),      // PB0 :  EMISSION     --> INFRARED EMISSION
    __STM32_PIN(36, B, 1),      // PB1 :  RECEPTION    --> INFRARED EMISSION
    __STM32_PIN(37, B, 2),      // PB2 :  BEEP         --> BEEP
    __STM32_PIN(38, E, 7),      // PE7 :  LED_R        --> LED
    __STM32_PIN(39, E, 8),      // PE8 :  LED_G        --> LED
    __STM32_PIN(40, E, 9),      // PE9 :  LED_B        --> LED
    __STM32_PIN(41, E, 10),     // PE10:  QSPI_BK1_CLK --> SPI_FLASH
    __STM32_PIN(42, E, 11),     // PE11:  QSPI_BK1_NCS --> SPI_FLASH
    __STM32_PIN(43, E, 12),     // PE12:  QSPI_BK1_IO0 --> SPI_FLASH
    __STM32_PIN(44, E, 13),     // PE13:  QSPI_BK1_IO1 --> SPI_FLASH
    __STM32_PIN(45, E, 14),     // PE14:  QSPI_BK1_IO2 --> SPI_FLASH
    __STM32_PIN(46, E, 15),     // PE15:  QSPI_BK1_IO3 --> SPI_FLASH
    __STM32_PIN(47, B, 10),     // PB10:  AP_INT       --> ALS&PS SENSOR
    __STM32_PIN(48, B, 11),     // PB11:  ICM_INT      --> AXIS SENSOR
    __STM32_PIN_DEFAULT,        //     :  VSS
    __STM32_PIN_DEFAULT,        //     :  VDD
    __STM32_PIN(51, B, 12),     // PB12:  SPI2_CS      --> EXTERNAL MODULE
    __STM32_PIN(52, B, 13),     // PB13:  SPI2_SCK     --> EXTERNAL MODULE
    __STM32_PIN(53, B, 14),     // PB14:  SPI2_MISO    --> EXTERNAL MODULE
    __STM32_PIN(54, B, 15),     // PB15:  SPI2_MOSI    --> EXTERNAL MODULE
    __STM32_PIN(55, D, 8),      // PD8 :  KEY0         --> KEY
    __STM32_PIN(56, D, 9),      // PD9 :  KEY1         --> KEY
    __STM32_PIN(57, D, 10),     // PD10:  KEY2         --> KEY
    __STM32_PIN(58, D, 11),     // PD11:  WK_UP        --> KEY
    __STM32_PIN(59, D, 12),     // PD12:  IO_PD12      --> EXTERNAL MODULEL
    __STM32_PIN(60, D, 13),     // PD13:  IO_PD13      --> EXTERNAL MODULE
    __STM32_PIN(61, D, 14),     // PD14:  IO_PD14      --> EXTERNAL MODULE
    __STM32_PIN(62, D, 15),     // PD15:  IO_PD15      --> EXTERNAL MODULE
    __STM32_PIN(63, C, 6),      // PC6 :  TIM3_CH1     --> EXTERNAL MODULE
    __STM32_PIN(64, C, 7),      // PC7 :  TIM3_CH2     --> EXTERNAL MODULE
    __STM32_PIN(65, C, 8),      // PC8 :  SDIO_D0      --> WIFI
    __STM32_PIN(66, C, 9),      // PC9 :  SDIO_D1      --> WIFI
    __STM32_PIN(67, A, 8),      // PA8 :  IO_PA8       --> EXTERNAL MODULE
    __STM32_PIN(68, A, 9),      // PA9 :  UART1_TX     --> STLINK_RX
    __STM32_PIN(69, A, 10),     // PA10:  UART1_RX     --> STLINK_RX
    __STM32_PIN(70, A, 11),     // PA11:  USB_D-       --> USB OTG && EXTERNAL MODULE
    __STM32_PIN(71, A, 12),     // PA12:  USB_D+       --> USB OTG && EXTERNAL MODULE
    __STM32_PIN(72, A, 13),     // PA13:  T_JTMS       --> STLINK
    __STM32_PIN_DEFAULT,        //     :  VDDUSB
    __STM32_PIN_DEFAULT,        //     :  VSS
    __STM32_PIN_DEFAULT,        //     :  VDD
    __STM32_PIN(76, A, 14),     // PA14:  T_JTCK       --> STLINK
    __STM32_PIN(77, A, 15),     // PA15:  AUDIO_PWR    --> AUDIO && POWER
    __STM32_PIN(78, C, 10),     // PC10:  SDIO_D2      --> WIFI
    __STM32_PIN(79, C, 11),     // PC11:  SDIO_D3      --> WIFI
    __STM32_PIN(80, C, 12),     // PC12:  SDIO_CLK     --> WIFI
    __STM32_PIN(81, D, 0),      //
    __STM32_PIN(82, D, 1),      // PD1 :  WIFI_REG_ON  --> WIFI
    __STM32_PIN(83, D, 2),      // PD2 :  SDIO_CMD     --> WIFI
    __STM32_PIN(84, D, 3),      // PD3 :  IO_PD3       --> EXTERNAL MODULE
    __STM32_PIN(85, D, 4),      // PD4 :  NRF_IRQ      --> WIRELESS
    __STM32_PIN(86, D, 5),      // PD5 :  NRF_CE       --> WIRELESS
    __STM32_PIN(87, D, 6),      // PD6 :  NRF_CS       --> WIRELESS
    __STM32_PIN(88, D, 7),      // PD7 :  LCD_CS       --> LCD
    __STM32_PIN(89, B, 3),      // PB3 :  LCD_SPI_SCK  --> LCD
    __STM32_PIN(90, B, 4),      // PB4 :  LCD_WR       --> LCD
    __STM32_PIN(91, B, 5),      // PB5 :  LCD_SPI_SDA  --> LCD
    __STM32_PIN(92, B, 6),      // PB6 :  LCD_RESET    --> LCD
    __STM32_PIN(93, B, 7),      // PB7 :  LCD_PWR      --> LCD
    __STM32_PIN_DEFAULT,        //     :  BOOT0
    __STM32_PIN(95, B, 8),      // PB8 :  I2C1_SCL     --> EXTERNAL MODULE
    __STM32_PIN(96, B, 9),      // PB9 :  I2C1_SDA     --> EXTERNAL MODULE
    __STM32_PIN(97, E, 0),      // PE0 :  IO_PE0       --> EXTERNAL MODULE
    __STM32_PIN(98, E, 1),      // PE1 :  IO_PE1       --> EXTERNAL MODULE
    __STM32_PIN_DEFAULT,        //     :  VSS
    __STM32_PIN_DEFAULT,        //     :  VDD
};

struct pin_irq_map
{
    rt_uint16_t pinbit;
    IRQn_Type irqno;
};

static const struct pin_irq_map pin_irq_map[] =
{
    {GPIO_PIN_0, EXTI0_IRQn},
    {GPIO_PIN_1, EXTI1_IRQn},
    {GPIO_PIN_2, EXTI2_IRQn},
    {GPIO_PIN_3, EXTI3_IRQn},
    {GPIO_PIN_4, EXTI4_IRQn},
    {GPIO_PIN_5, EXTI9_5_IRQn},
    {GPIO_PIN_6, EXTI9_5_IRQn},
    {GPIO_PIN_7, EXTI9_5_IRQn},
    {GPIO_PIN_8, EXTI9_5_IRQn},
    {GPIO_PIN_9, EXTI9_5_IRQn},
    {GPIO_PIN_10, EXTI15_10_IRQn},
    {GPIO_PIN_11, EXTI15_10_IRQn},
    {GPIO_PIN_12, EXTI15_10_IRQn},
    {GPIO_PIN_13, EXTI15_10_IRQn},
    {GPIO_PIN_14, EXTI15_10_IRQn},
    {GPIO_PIN_15, EXTI15_10_IRQn},
};

static struct rt_pin_irq_hdr pin_irq_hdr_tab[] =
{
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
};

#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])
static const struct pin_index *get_pin(uint8_t pin)
{
    const struct pin_index *index;

    if (pin < ITEM_NUM(pins))
    {
        index = &pins[pin];
        if (index->index == -1)
            index = RT_NULL;
    }
    else
    {
        index = RT_NULL;
    }

    return index;
};

static void stm32_pin_write(rt_device_t dev, rt_base_t pin, rt_base_t value)
{
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return;
    }

    HAL_GPIO_WritePin(index->gpio, index->pin, (GPIO_PinState)value);
}

static int stm32_pin_read(rt_device_t dev, rt_base_t pin)
{
    int value;
    const struct pin_index *index;

    value = PIN_LOW;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return value;
    }

    value = HAL_GPIO_ReadPin(index->gpio, index->pin);

    return value;
}

static void stm32_pin_mode(rt_device_t dev, rt_base_t pin, rt_base_t mode)
{
    const struct pin_index *index;
    GPIO_InitTypeDef GPIO_InitStruct;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return;
    }

    /* GPIO Periph clock enable */
    index->rcc();

    /* Configure GPIO_InitStructure */
    GPIO_InitStruct.Pin = index->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    if (mode == PIN_MODE_OUTPUT)
    {
        /* output setting */
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* input setting: not pull. */
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* input setting: pull up. */
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        /* input setting: pull down. */
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        /* output setting: od. */
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    }

    HAL_GPIO_Init(index->gpio, &GPIO_InitStruct);
}

rt_inline rt_int32_t bit2bitno(rt_uint32_t bit)
{
    int i;
    for (i = 0; i < 32; i++)
    {
        if ((0x01 << i) == bit)
        {
            return i;
        }
    }
    return -1;
}

rt_inline const struct pin_irq_map *get_pin_irq_map(uint32_t pinbit)
{
    rt_int32_t mapindex = bit2bitno(pinbit);
    if (mapindex < 0 || mapindex >= ITEM_NUM(pin_irq_map))
    {
        return RT_NULL;
    }
    return &pin_irq_map[mapindex];
};

static rt_err_t stm32_pin_attach_irq(struct rt_device *device, rt_int32_t pin,
                                     rt_uint32_t mode, void (*hdr)(void *args), void *args)
{
    const struct pin_index *index;
    rt_base_t level;
    rt_int32_t irqindex = -1;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return RT_ENOSYS;
    }
    irqindex = bit2bitno(index->pin);
    if (irqindex < 0 || irqindex >= ITEM_NUM(pin_irq_map))
    {
        return RT_ENOSYS;
    }

    level = rt_hw_interrupt_disable();
    if (pin_irq_hdr_tab[irqindex].pin == pin &&
            pin_irq_hdr_tab[irqindex].hdr == hdr &&
            pin_irq_hdr_tab[irqindex].mode == mode &&
            pin_irq_hdr_tab[irqindex].args == args)
    {
        rt_hw_interrupt_enable(level);
        return RT_EOK;
    }
    if (pin_irq_hdr_tab[irqindex].pin != -1)
    {
        rt_hw_interrupt_enable(level);
        return RT_EBUSY;
    }
    pin_irq_hdr_tab[irqindex].pin = pin;
    pin_irq_hdr_tab[irqindex].hdr = hdr;
    pin_irq_hdr_tab[irqindex].mode = mode;
    pin_irq_hdr_tab[irqindex].args = args;
    rt_hw_interrupt_enable(level);

    return RT_EOK;
}

static rt_err_t stm32_pin_dettach_irq(struct rt_device *device, rt_int32_t pin)
{
    const struct pin_index *index;
    rt_base_t level;
    rt_int32_t irqindex = -1;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return RT_ENOSYS;
    }
    irqindex = bit2bitno(index->pin);
    if (irqindex < 0 || irqindex >= ITEM_NUM(pin_irq_map))
    {
        return RT_ENOSYS;
    }

    level = rt_hw_interrupt_disable();
    if (pin_irq_hdr_tab[irqindex].pin == -1)
    {
        rt_hw_interrupt_enable(level);
        return RT_EOK;
    }
    pin_irq_hdr_tab[irqindex].pin = -1;
    pin_irq_hdr_tab[irqindex].hdr = RT_NULL;
    pin_irq_hdr_tab[irqindex].mode = 0;
    pin_irq_hdr_tab[irqindex].args = RT_NULL;
    rt_hw_interrupt_enable(level);

    return RT_EOK;
}

static rt_err_t stm32_pin_irq_enable(struct rt_device *device, rt_base_t pin,
                                     rt_uint32_t enabled)
{
    const struct pin_index *index;
    const struct pin_irq_map *irqmap;
    rt_base_t level;
    rt_int32_t irqindex = -1;
    GPIO_InitTypeDef GPIO_InitStruct;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return RT_ENOSYS;
    }

    if (enabled == PIN_IRQ_ENABLE)
    {
        irqindex = bit2bitno(index->pin);
        if (irqindex < 0 || irqindex >= ITEM_NUM(pin_irq_map))
        {
            return RT_ENOSYS;
        }

        level = rt_hw_interrupt_disable();

        if (pin_irq_hdr_tab[irqindex].pin == -1)
        {
            rt_hw_interrupt_enable(level);
            return RT_ENOSYS;
        }

        irqmap = &pin_irq_map[irqindex];

        /* GPIO Periph clock enable */
        index->rcc();

        /* Configure GPIO_InitStructure */
        GPIO_InitStruct.Pin = index->pin;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        switch (pin_irq_hdr_tab[irqindex].mode)
        {
        case PIN_IRQ_MODE_RISING:
            GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
            break;
        case PIN_IRQ_MODE_FALLING:
            GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
            break;
        case PIN_IRQ_MODE_RISING_FALLING:
            GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
            break;
        }
        HAL_GPIO_Init(index->gpio, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(irqmap->irqno, 5, 0);
        HAL_NVIC_EnableIRQ(irqmap->irqno);

        rt_hw_interrupt_enable(level);
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        irqmap = get_pin_irq_map(index->pin);
        if (irqmap == RT_NULL)
        {
            return RT_ENOSYS;
        }

        HAL_NVIC_DisableIRQ(irqmap->irqno);
    }
    else
    {
        return RT_ENOSYS;
    }

    return RT_EOK;
}
const static struct rt_pin_ops _stm32_pin_ops =
{
    stm32_pin_mode,
    stm32_pin_write,
    stm32_pin_read,
    stm32_pin_attach_irq,
    stm32_pin_dettach_irq,
    stm32_pin_irq_enable,
};

int rt_hw_pin_init(void)
{
    return rt_device_pin_register("pin", &_stm32_pin_ops, RT_NULL);
}

rt_inline void pin_irq_hdr(int irqno)
{
    if (pin_irq_hdr_tab[irqno].hdr)
    {
        pin_irq_hdr_tab[irqno].hdr(pin_irq_hdr_tab[irqno].args);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    pin_irq_hdr(bit2bitno(GPIO_Pin));
}

void EXTI0_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
    rt_interrupt_leave();
}

void EXTI1_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
    rt_interrupt_leave();
}

void EXTI2_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
    rt_interrupt_leave();
}

void EXTI3_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
    rt_interrupt_leave();
}

void EXTI4_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
    rt_interrupt_leave();
}

void EXTI9_5_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
    rt_interrupt_leave();
}

void EXTI15_10_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
    rt_interrupt_leave();
}

#endif
