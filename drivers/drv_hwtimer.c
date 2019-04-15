/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-12-10     zylx         first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define DBG_SECTION_NAME     "drv.hwtimer"
#define DBG_LEVEL     DBG_INFO
#include <rtdbg.h>

#ifdef RT_USING_HWTIMER

/**
* @brief TIM_Base MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{

  if(htim_base->Instance==TIM4)
  {
  /* USER CODE BEGIN TIM4_MspInit 0 */

  /* USER CODE END TIM4_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();
  /* USER CODE BEGIN TIM4_MspInit 1 */

  /* USER CODE END TIM4_MspInit 1 */
  }
  else if(htim_base->Instance==TIM15)
  {
  /* USER CODE BEGIN TIM15_MspInit 0 */

  /* USER CODE END TIM15_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM15_CLK_ENABLE();
  /* USER CODE BEGIN TIM15_MspInit 1 */

  /* USER CODE END TIM15_MspInit 1 */
  }
  else if(htim_base->Instance==TIM16)
  {
  /* USER CODE BEGIN TIM16_MspInit 0 */

  /* USER CODE END TIM16_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM16_CLK_ENABLE();
  /* USER CODE BEGIN TIM16_MspInit 1 */

  /* USER CODE END TIM16_MspInit 1 */
  }
  else if(htim_base->Instance==TIM17)
  {
  /* USER CODE BEGIN TIM17_MspInit 0 */

  /* USER CODE END TIM17_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM17_CLK_ENABLE();
  /* USER CODE BEGIN TIM17_MspInit 1 */

  /* USER CODE END TIM17_MspInit 1 */
  }

}

enum
{
#ifdef BSP_USING_TIM15
    TIM15_INDEX,
#endif
#ifdef BSP_USING_TIM16
    TIM16_INDEX,
#endif
};

struct stm32_hwtimer
{
    rt_hwtimer_t time_device;
    TIM_HandleTypeDef    tim_handle;
    IRQn_Type tim_irqn;
    char *name;
};

static struct stm32_hwtimer stm32_hwtimer_obj[] =
{
#ifdef BSP_USING_TIM15
    {                                                       \
       .tim_handle.Instance     = TIM15,                    \
       .tim_irqn                = TIM1_BRK_TIM15_IRQn,      \
       .name                    = "timer15",                \
    },
#endif

#ifdef BSP_USING_TIM16
    {                                                       \
       .tim_handle.Instance     = TIM16,                    \
       .tim_irqn                = TIM1_UP_TIM16_IRQn,       \
       .name                    = "timer16",                \
    },
#endif
};

static void timer_init(struct rt_hwtimer_device *timer, rt_uint32_t state)
{
    uint32_t prescaler_value = 0;
    TIM_HandleTypeDef *tim = RT_NULL;
    struct stm32_hwtimer *tim_device = RT_NULL;

    RT_ASSERT(timer != RT_NULL);
    if (state)
    {
        tim = (TIM_HandleTypeDef *)timer->parent.user_data;
        tim_device = (struct stm32_hwtimer *)timer;

        /* time init */
        if (tim->Instance == TIM15 || tim->Instance == TIM16 || tim->Instance == TIM17)
        {
            prescaler_value = (uint32_t)(HAL_RCC_GetPCLK2Freq() * 2 / 10000) - 1;
        }
        else
        {
            prescaler_value = (uint32_t)(HAL_RCC_GetPCLK1Freq() * 2 / 10000) - 1;
        }
        tim->Init.Period            = 10000 - 1;
        tim->Init.Prescaler         = prescaler_value;
        tim->Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
        if (timer->info->cntmode == HWTIMER_CNTMODE_UP)
        {
            tim->Init.CounterMode   = TIM_COUNTERMODE_UP;
        }
        else
        {
            tim->Init.CounterMode   = TIM_COUNTERMODE_DOWN;
        }
        tim->Init.RepetitionCounter = 0;
        tim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
        if (HAL_TIM_Base_Init(tim) != HAL_OK)
        {
            LOG_E("%s init failed", tim_device->name);
            return;
        }
        else
        {
            /* set the TIMx priority */
            HAL_NVIC_SetPriority(tim_device->tim_irqn, 3, 0);

            /* enable the TIMx global Interrupt */
            HAL_NVIC_EnableIRQ(tim_device->tim_irqn);

            /* clear update flag */
            __HAL_TIM_CLEAR_FLAG(tim, TIM_FLAG_UPDATE);
            /* enable update request source */
            __HAL_TIM_URS_ENABLE(tim);

            LOG_D("%s init success", tim_device->name);
        }
    }
}

static rt_err_t timer_start(rt_hwtimer_t *timer, rt_uint32_t t, rt_hwtimer_mode_t opmode)
{
    rt_err_t result = RT_EOK;
    TIM_HandleTypeDef *tim = RT_NULL;

    RT_ASSERT(timer != RT_NULL);

    tim = (TIM_HandleTypeDef *)timer->parent.user_data;

    /* set tim cnt */
    __HAL_TIM_SET_AUTORELOAD(tim, t);

    if (opmode == HWTIMER_MODE_ONESHOT)
    {
        /* set timer to single mode */
        tim->Instance->CR1 |= TIM_OPMODE_SINGLE;
    }

    /* start timer */
    if (HAL_TIM_Base_Start_IT(tim) != HAL_OK)
    {
        LOG_E("TIM2 start failed");
        result = -RT_ERROR;
    }

    return result;
}

static void timer_stop(rt_hwtimer_t *timer)
{
    TIM_HandleTypeDef *tim = RT_NULL;

    RT_ASSERT(timer != RT_NULL);

    tim = (TIM_HandleTypeDef *)timer->parent.user_data;

    /* stop timer */
    HAL_TIM_Base_Stop_IT(tim);
}

static rt_err_t timer_ctrl(rt_hwtimer_t *timer, rt_uint32_t cmd, void *arg)
{
    TIM_HandleTypeDef *tim = RT_NULL;
    rt_err_t result = RT_EOK;

    RT_ASSERT(timer != RT_NULL);
    RT_ASSERT(arg != RT_NULL);

    tim = (TIM_HandleTypeDef *)timer->parent.user_data;

    switch (cmd)
    {
    case HWTIMER_CTRL_FREQ_SET:
    {
        rt_uint32_t freq;
        rt_uint16_t val;

        /* set timer frequence */
        freq = *((rt_uint32_t *)arg);

        if (tim->Instance == TIM15 || tim->Instance == TIM16 || tim->Instance == TIM17)
        {
            val = HAL_RCC_GetPCLK2Freq() / freq;
        }

        __HAL_TIM_SET_PRESCALER(tim, val - 1);

        /* Update frequency value */
        tim->Instance->EGR |= TIM_EVENTSOURCE_UPDATE;
    }
    break;
    default:
    {
        result = -RT_ENOSYS;
    }
    break;
    }

    return result;
}

static rt_uint32_t timer_counter_get(rt_hwtimer_t *timer)
{
    TIM_HandleTypeDef *tim = RT_NULL;

    RT_ASSERT(timer != RT_NULL);

    tim = (TIM_HandleTypeDef *)timer->parent.user_data;

    return tim->Instance->CNT;
}

#define TIM_DEV_INFO_CONFIG                     \
    {                                           \
        .maxfreq = 1000000,                     \
        .minfreq = 2000,                        \
        .maxcnt  = 0xFFFF,                      \
        .cntmode = HWTIMER_CNTMODE_UP,          \
    }

static const struct rt_hwtimer_info _info = TIM_DEV_INFO_CONFIG;

static const struct rt_hwtimer_ops _ops =
{
    .init = timer_init,
    .start = timer_start,
    .stop = timer_stop,
    .count_get = timer_counter_get,
    .control = timer_ctrl,
};

#ifdef BSP_USING_TIM15
void TIM1_BRK_TIM15_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();
    HAL_TIM_IRQHandler(&stm32_hwtimer_obj[TIM15_INDEX].tim_handle);
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif
#ifdef BSP_USING_TIM16
void TIM1_UP_TIM16_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();
    HAL_TIM_IRQHandler(&stm32_hwtimer_obj[TIM16_INDEX].tim_handle);
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
#ifdef BSP_USING_TIM15
    if (htim->Instance == TIM15)
    {
        rt_device_hwtimer_isr(&stm32_hwtimer_obj[TIM15_INDEX].time_device);
    }
#endif
#ifdef BSP_USING_TIM16
    if (htim->Instance == TIM16)
    {
        rt_device_hwtimer_isr(&stm32_hwtimer_obj[TIM16_INDEX].time_device);
    }
#endif
}

static int stm32_hwtimer_init(void)
{
    int i = 0;
    int result = RT_EOK;

    for (i = 0; i < sizeof(stm32_hwtimer_obj) / sizeof(stm32_hwtimer_obj[0]); i++)
    {
        stm32_hwtimer_obj[i].time_device.info = &_info;
        stm32_hwtimer_obj[i].time_device.ops  = &_ops;
        if (rt_device_hwtimer_register(&stm32_hwtimer_obj[i].time_device, stm32_hwtimer_obj[i].name, &stm32_hwtimer_obj[i].tim_handle) == RT_EOK)
        {
            LOG_D("%s register success", stm32_hwtimer_obj[i].name);
        }
        else
        {
            LOG_E("%s register failed", stm32_hwtimer_obj[i].name);
            result = -RT_ERROR;
        }
    }

    return result;
}
INIT_BOARD_EXPORT(stm32_hwtimer_init);

#endif /* RT_USING_HWTIMER */
