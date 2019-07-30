/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-07     Tanek        first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <stm32l4xx.h>
#include "board.h"
#include "drv_gpio.h"

#define USER_WAKEUP_PIN       PIN_WK_UP

#if (HARDWARE_VERSION == 0x0202U)
#define DRV_WKUP_PIN          PWR_WAKEUP_PIN2
#define DRV_WKUP_PIN_MODE     PWR_WAKEUP_PIN2_HIGH
#define DRV_WKUP_PIN_IRQ_MODE PIN_IRQ_MODE_RISING
#else
#define DRV_WKUP_PIN          PWR_WAKEUP_PIN4
#define DRV_WKUP_PIN_MODE     PWR_WAKEUP_PIN4_LOW
#define DRV_WKUP_PIN_IRQ_MODE PIN_IRQ_MODE_FALLING
#endif

static void (*_wakeup_hook)(void);

void bsp_register_wakeup(void (*hook)(void))
{
    RT_ASSERT(hook != RT_NULL);

    _wakeup_hook = hook;
}

static void _wakeup_button_update(void)
{
    /* The Following Wakeup sequence is highly recommended prior to each Standby mode entry
       mainly when using more than one wakeup source this is to not miss any wakeup event.
        - Disable all used wakeup sources,
        - Clear all related wakeup flags,
        - Re-enable all used wakeup sources,
        - Enter the Standby mode.
    */

    /* Disable all used wakeup sources: WKUP pin */
    HAL_PWR_DisableWakeUpPin(DRV_WKUP_PIN);
    /* Clear wake up Flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    /* Enable wakeup pin WKUP2 */
    HAL_PWR_EnableWakeUpPin(DRV_WKUP_PIN_MODE);
}

static void _wakeup_callback(void *args)
{
    /* Wait that user release the User push-button */
    //while (rt_pin_read(USER_WAKEUP_PIN) == 0);

    _wakeup_button_update();

    if (_wakeup_hook)
        _wakeup_hook();
}

static int rt_hw_wakeup_init(void)
{
    rt_pin_mode(USER_WAKEUP_PIN, PIN_MODE_INPUT);
    rt_pin_attach_irq(USER_WAKEUP_PIN, DRV_WKUP_PIN_IRQ_MODE, _wakeup_callback, RT_NULL);
    rt_pin_irq_enable(USER_WAKEUP_PIN, 1);

    /* Configure Wakeup pin 2 */
    /* Enable Power Clock*/
    __HAL_RCC_PWR_CLK_ENABLE();
    /* Ensure that MSI is wake-up system clock */
    __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
    /* Update Wakeup Button */
    _wakeup_button_update();

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_wakeup_init);
