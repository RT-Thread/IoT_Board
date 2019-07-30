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
#include <drv_wakeup.h>
#include <board.h>
#include <stm32l4xx.h>

#define WAKEUP_APP_THREAD_STACK_SIZE        1024
#define WAKEUP_APP__THREAD_PRIORITY         RT_THREAD_PRIORITY_MAX / 3
#define WAKEUP_EVENT_BUTTON                 (1 << 0)

static rt_event_t wakeup_event;

static void _pin_as_analog(void)
{
    GPIO_InitTypeDef GPIOInitstruct = {0};

    GPIOInitstruct.Pin    = GPIO_PIN_7;
    GPIOInitstruct.Mode   = GPIO_MODE_ANALOG;
    GPIOInitstruct.Pull   = GPIO_NOPULL;
    GPIOInitstruct.Speed  = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOE, &GPIOInitstruct);
}

static void led_app(void)
{
    rt_pm_request(PM_SLEEP_MODE_NONE);

    rt_pin_mode(PIN_LED_R, PIN_MODE_OUTPUT);
    rt_pin_write(PIN_LED_R, 0);
    rt_thread_mdelay(2000);
    rt_pin_write(PIN_LED_R, 1);
    _pin_as_analog();

    rt_pm_release(PM_SLEEP_MODE_NONE);
}

static void wakeup_callback(void)
{
    rt_event_send(wakeup_event, WAKEUP_EVENT_BUTTON);
}

static void wakeup_init(void)
{
    wakeup_event = rt_event_create("wakup", RT_IPC_FLAG_FIFO);
    RT_ASSERT(wakeup_event != RT_NULL);

    bsp_register_wakeup(wakeup_callback);
}

static void pm_mode_init(void)
{
    rt_pm_request(PM_SLEEP_MODE_DEEP);
}

int main(void)
{
    /* 唤醒回调函数初始化 */
    wakeup_init();

    /* 电源管理初始化 */
    pm_mode_init();

    while (1)
    {
        /* 等待唤醒事件 */
        if (rt_event_recv(wakeup_event,
                          WAKEUP_EVENT_BUTTON,
                          RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                          RT_WAITING_FOREVER, RT_NULL) == RT_EOK)
        {
            led_app();
        }
    }
}
