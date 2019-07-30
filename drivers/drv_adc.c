/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-12-05     zylx         first version
 * 2018-12-12     greedyhao    Porting for stm32f7xx
 * 2019-02-01     yuneizhilin   fix the stm32_adc_init function initialization issue
 */

#include <stm32l4xx.h>
#include <rtdevice.h>
#include <rthw.h>

#ifdef RT_USING_ADC

//#define DRV_DEBUG
#define LOG_TAG             "drv.adc"
#include <drv_log.h>

/**
* @brief ADC MSP Initialization
* This function configures the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hadc->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_ADC_CLK_ENABLE();
  
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**ADC1 GPIO Configuration    
    PC4     ------> ADC1_IN13 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }

}

/**
* @brief ADC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC_CLK_DISABLE();
  
    /**ADC1 GPIO Configuration    
    PC4     ------> ADC1_IN13 
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_4);

  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }

}

#define BSP_USING_ADC1

#define ADC1_CONFIG                                                 \
    {                                                               \
       .Instance                   = ADC1,                          \
       .Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4,      \
       .Init.Resolution            = ADC_RESOLUTION_12B,            \
       .Init.DataAlign             = ADC_DATAALIGN_RIGHT,           \
       .Init.ScanConvMode          = ADC_SCAN_DISABLE,              \
       .Init.EOCSelection          = ADC_EOC_SINGLE_CONV,           \
       .Init.LowPowerAutoWait      = DISABLE,                       \
       .Init.ContinuousConvMode    = DISABLE,                       \
       .Init.NbrOfConversion       = 1,                             \
       .Init.DiscontinuousConvMode = DISABLE,                       \
       .Init.NbrOfDiscConversion   = 1,                             \
       .Init.ExternalTrigConv      = ADC_SOFTWARE_START,            \
       .Init.DMAContinuousRequests = DISABLE,                       \
       .Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN,      \
    }

static ADC_HandleTypeDef adc_config[] =
{
#ifdef BSP_USING_ADC1
    ADC1_CONFIG,
#endif

#ifdef BSP_USING_ADC2
    ADC2_CONFIG,
#endif

#ifdef BSP_USING_ADC3
    ADC3_CONFIG,
#endif
};

struct stm32_adc
{
    ADC_HandleTypeDef ADC_Handler;
    struct rt_adc_device stm32_adc_device;
};

static struct stm32_adc stm32_adc_obj[sizeof(adc_config) / sizeof(adc_config[0])];

static rt_err_t stm32_adc_enabled(struct rt_adc_device *device, rt_uint32_t channel, rt_bool_t enabled)
{
    ADC_HandleTypeDef *stm32_adc_handler = device->parent.user_data;

    RT_ASSERT(device != RT_NULL);

    if (enabled)
    {
        ADC_Enable(stm32_adc_handler);
    }
    else
    {
        ADC_Disable(stm32_adc_handler);
    }

    return RT_EOK;
}

static rt_uint32_t stm32_adc_get_channel(rt_uint32_t channel)
{
    rt_uint32_t stm32_channel = 0;

    switch (channel)
    {
    case  0:
        stm32_channel = ADC_CHANNEL_0;
        break;
    case  1:
        stm32_channel = ADC_CHANNEL_1;
        break;
    case  2:
        stm32_channel = ADC_CHANNEL_2;
        break;
    case  3:
        stm32_channel = ADC_CHANNEL_3;
        break;
    case  4:
        stm32_channel = ADC_CHANNEL_4;
        break;
    case  5:
        stm32_channel = ADC_CHANNEL_5;
        break;
    case  6:
        stm32_channel = ADC_CHANNEL_6;
        break;
    case  7:
        stm32_channel = ADC_CHANNEL_7;
        break;
    case  8:
        stm32_channel = ADC_CHANNEL_8;
        break;
    case  9:
        stm32_channel = ADC_CHANNEL_9;
        break;
    case 10:
        stm32_channel = ADC_CHANNEL_10;
        break;
    case 11:
        stm32_channel = ADC_CHANNEL_11;
        break;
    case 12:
        stm32_channel = ADC_CHANNEL_12;
        break;
    case 13:
        stm32_channel = ADC_CHANNEL_13;
        break;
    case 14:
        stm32_channel = ADC_CHANNEL_14;
        break;
    case 15:
        stm32_channel = ADC_CHANNEL_15;
        break;
    case 16:
        stm32_channel = ADC_CHANNEL_16;
        break;
    case 17:
        stm32_channel = ADC_CHANNEL_17;
        break;
    case 18:
        stm32_channel = ADC_CHANNEL_18;
        break;
    }

    return stm32_channel;
}

static rt_err_t stm32_get_adc_value(struct rt_adc_device *device, rt_uint32_t channel, rt_uint32_t *value)
{
    ADC_ChannelConfTypeDef ADC_ChanConf;
    ADC_HandleTypeDef *stm32_adc_handler = device->parent.user_data;

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(value != RT_NULL);

    rt_memset(&ADC_ChanConf, 0, sizeof(ADC_ChanConf));

    if (channel <= 18)
    {
        /* set stm32 ADC channel */
        ADC_ChanConf.Channel =  stm32_adc_get_channel(channel);
    }
    else
    {
        LOG_E("ADC channel must be between 0 and 18.");
        return -RT_ERROR;
    }
    ADC_ChanConf.Rank = 1;
    ADC_ChanConf.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
    ADC_ChanConf.Offset = 0;
    ADC_ChanConf.OffsetNumber = ADC_OFFSET_NONE;
    ADC_ChanConf.SingleDiff = LL_ADC_SINGLE_ENDED;
    HAL_ADC_ConfigChannel(stm32_adc_handler, &ADC_ChanConf);

    /* start ADC */
    HAL_ADC_Start(stm32_adc_handler);

    /* Wait for the ADC to convert */
    HAL_ADC_PollForConversion(stm32_adc_handler, 100);

    /* get ADC value */
    *value = (rt_uint32_t)HAL_ADC_GetValue(stm32_adc_handler);

    return RT_EOK;
}

static const struct rt_adc_ops stm_adc_ops =
{
    .enabled = stm32_adc_enabled,
    .convert = stm32_get_adc_value,
};

static int stm32_adc_init(void)
{
    int result = RT_EOK;
    /* save adc name */
    char name_buf[5] = {'a', 'd', 'c', '0', 0};
    int i = 0;

    for (i = 0; i < sizeof(adc_config) / sizeof(adc_config[0]); i++)
    {
        /* ADC init */
        name_buf[3] = '0';
        stm32_adc_obj[i].ADC_Handler = adc_config[i];
#if defined(ADC1)
        if (stm32_adc_obj[i].ADC_Handler.Instance == ADC1)
        {
            name_buf[3] = '1';
        }
#endif
#if defined(ADC2)
        if (stm32_adc_obj[i].ADC_Handler.Instance == ADC2)
        {
            name_buf[3] = '2';
        }
#endif
#if defined(ADC3)
        if (stm32_adc_obj[i].ADC_Handler.Instance == ADC3)
        {
            name_buf[3] = '3';
        }
#endif
        if (HAL_ADC_Init(&stm32_adc_obj[i].ADC_Handler) != HAL_OK)
        {
            LOG_E("%s init failed", name_buf);
            result = -RT_ERROR;
        }
        else
        {
            /* register ADC device */
            if (rt_hw_adc_register(&stm32_adc_obj[i].stm32_adc_device, name_buf, &stm_adc_ops, &stm32_adc_obj[i].ADC_Handler) == RT_EOK)
            {
                LOG_D("%s init success", name_buf);
            }
            else
            {
                LOG_E("%s register failed", name_buf);
                result = -RT_ERROR;
            }
        }
    }

    return result;
}
INIT_BOARD_EXPORT(stm32_adc_init);

#endif /* RT_USING_ADC */
