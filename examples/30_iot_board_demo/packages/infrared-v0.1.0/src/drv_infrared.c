/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-25     balanceTWK   the first version
 */

#include <rthw.h>
#include <rtdevice.h>
#include <board.h>
#include "infrared.h"
#include "drv_infrared.h"

#define DBG_SECTION_NAME     "drv.infrared"
#define DBG_LEVEL     DBG_INFO
#include <rtdbg.h>

#ifdef PKG_USING_INFRARED

static struct infrared_class* infrared;

#ifdef INFRARED_SEND

/* Infrared transmission configuration parameters */
#define PWM_DEV_NAME              INFRARED_SEND_PWM         /* PWM name */
#define PWM_DEV_CHANNEL           INFRARED_PWM_DEV_CHANNEL
#define SEND_HWTIMER              INFRARED_SEND_HWTIMER     /* Timer name */
#define MAX_SEND_SIZE             INFRARED_MAX_SEND_SIZE

struct rt_device_pwm         *pwm_dev;
static rt_uint32_t  infrared_send_buf[MAX_SEND_SIZE];
static rt_device_t           send_time_dev ;
static rt_hwtimerval_t       timeout_s;

static rt_err_t send_timeout_callback(rt_device_t dev, rt_size_t size)
{
    static rt_size_t i = 0;
    rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);
    if ((infrared_send_buf[i] != 0x5A5A5A5A))/* Determine if it is a stop bit */
    {
        if ((infrared_send_buf[i] & 0xF0000000) == 0xA0000000) /* Determine if it is a carrier signal */
        {
            rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
        }
        timeout_s.sec = 0;
        timeout_s.usec = (infrared_send_buf[i] & 0x0FFFFFFF); /* Get the delay time */
        rt_device_write(send_time_dev, 0, &timeout_s, sizeof(timeout_s));
        i++;
    }
    else
    {
        i = 0;
    }
    return 0;
}

rt_err_t infrared_send_init(void)
{
    rt_err_t ret = RT_EOK;
    rt_hwtimer_mode_t mode;
    rt_uint32_t freq = 1000000;

    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL)
    {
        LOG_E("pwm sample run failed! can't find %s device!", PWM_DEV_NAME);
        return RT_ERROR;
    }

    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, 26316, 8770);
    rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);

    send_time_dev = rt_device_find(SEND_HWTIMER);
    if (send_time_dev == RT_NULL)
    {
        LOG_E("hwtimer sample run failed! can't find %s device!", SEND_HWTIMER);
        return RT_ERROR;
    }
    ret = rt_device_open(send_time_dev, RT_DEVICE_OFLAG_RDWR);
    if (ret != RT_EOK)
    {
        LOG_E("open %s device failed!\n", SEND_HWTIMER);
        return ret;
    }
    rt_device_set_rx_indicate(send_time_dev, send_timeout_callback);
    ret = rt_device_control(send_time_dev, HWTIMER_CTRL_FREQ_SET, &freq);
    if (ret != RT_EOK)
    {
        LOG_E("set frequency failed! ret is :%d", ret);
        return ret;
    }
    mode = HWTIMER_MODE_ONESHOT;
    ret = rt_device_control(send_time_dev, HWTIMER_CTRL_MODE_SET, &mode);
    if (ret != RT_EOK)
    {
        LOG_E("set mode failed! ret is :%d", ret);
        return ret;
    }
    return ret;
}

static rt_size_t infrared_send(struct ir_raw_data* data, rt_size_t size)
{
    rt_size_t send_size;
    if(size >= MAX_SEND_SIZE)
    {
        LOG_E("The length of the sent data exceeds the MAX_SEND_SIZE.");
        return 0;
    }
    for (send_size = 0; send_size < size; send_size++)
    {
        infrared_send_buf[send_size] = (data[send_size].level<<28) + (data[send_size].us);
    }
    infrared_send_buf[size] = 0x5A5A5A5A;

    timeout_s.sec = 0;
    timeout_s.usec = 500;
    rt_device_write(send_time_dev, 0, &timeout_s, sizeof(timeout_s));
    rt_thread_mdelay(100);
    return send_size;
}

#endif /* INFRARED_SEND */


#ifdef INFRARED_RECEIVE

/* Infrared receiver configuration parameters */
#define RECEIVE_PIN              INFRARED_RECEIVE_PIN       /* receive pin */
#define RECEIVE_HWTIMER          INFRARED_RECEIVE_HWTIMER   /* Timer name */

#define RECEIVE_HWTIMEER_SEC      0
#define RECEIVE_HWTIMEER_USEC     1000 * 1000

static rt_device_t           receive_time_dev ;
static rt_uint32_t diff_us;
static rt_uint32_t receive_flag = 0x00000000;

static void receive_pin_callback(void* param)
{
    static rt_hwtimerval_t receive_time;
    static rt_uint32_t last_us = 0 ,now_us;

    if( (receive_flag & (1<<0)) )
    {
        rt_device_read(receive_time_dev,0,&receive_time,sizeof(receive_time));
        now_us = (receive_time.sec * 1000000) + receive_time.usec;

        if(now_us >= last_us)
        {
            diff_us = now_us - last_us;
        }
        else
        {
            diff_us = now_us + RECEIVE_HWTIMEER_SEC * 1000000 + RECEIVE_HWTIMEER_USEC;
        }

        if(rt_pin_read(RECEIVE_PIN) == PIN_HIGH)
        {
            driver_report_raw_data(CARRIER_WAVE,diff_us);
            LOG_D("H%d",diff_us);
        }
        else
        {
            driver_report_raw_data(IDLE_SIGNAL,diff_us);
            LOG_D("L%d",diff_us);
        }

        last_us = now_us;
    }
    else
    {
        receive_time.sec = RECEIVE_HWTIMEER_SEC;
        receive_time.usec = RECEIVE_HWTIMEER_USEC;

        rt_device_write(receive_time_dev, 0, &receive_time, sizeof(receive_time));

        receive_flag |= 1<<0;

        last_us = 0;

        LOG_D("Start timer");
    }
}

static rt_err_t receive_timeout_callback(rt_device_t dev, rt_size_t size)
{
    if(diff_us > (1000*1000))
    {
        rt_device_control(receive_time_dev, HWTIMER_CTRL_STOP, RT_NULL);
        LOG_D("timeout and stop");

        receive_flag &= ~(1<<0);
    }
    diff_us = diff_us + RECEIVE_HWTIMEER_SEC * 1000000 + RECEIVE_HWTIMEER_USEC;
    return 0;
}
rt_err_t infrared_receive_init(void)
{
    rt_err_t ret = RT_EOK;
    rt_hwtimer_mode_t mode;
    rt_uint32_t freq = 100000;

    rt_pin_mode(RECEIVE_PIN,PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(RECEIVE_PIN,PIN_IRQ_MODE_RISING_FALLING,receive_pin_callback,RT_NULL);
    rt_pin_irq_enable(RECEIVE_PIN,PIN_IRQ_ENABLE);

    receive_time_dev = rt_device_find(RECEIVE_HWTIMER);
    if (receive_time_dev == RT_NULL)
    {
        LOG_E("hwtimer sample run failed! can't find %s device!", RECEIVE_HWTIMER);
        return RT_ERROR;
    }
    ret = rt_device_open(receive_time_dev, RT_DEVICE_OFLAG_RDWR);
    if (ret != RT_EOK)
    {
        LOG_E("open %s device failed!\n", RECEIVE_HWTIMER);
        return ret;
    }
    rt_device_set_rx_indicate(receive_time_dev, receive_timeout_callback);
    ret = rt_device_control(receive_time_dev, HWTIMER_CTRL_FREQ_SET, &freq);
    if (ret != RT_EOK)
    {
        LOG_E("set frequency failed! ret is :%d", ret);
        return ret;
    }
    mode = HWTIMER_MODE_PERIOD;
    ret = rt_device_control(receive_time_dev, HWTIMER_CTRL_MODE_SET, &mode);
    if (ret != RT_EOK)
    {
        LOG_E("set mode failed! ret is :%d", ret);
        return ret;
    }

    return ret;
}

#endif /* INFRARED_RECEIVE */


int drv_infrared_init()
{
    infrared = infrared_init();

    if(infrared == RT_NULL)
    {
        return -1;
    }

#ifdef INFRARED_SEND
    infrared_send_init();
    infrared->send = infrared_send;
#endif /* INFRARED_SEND */

#ifdef INFRARED_RECEIVE
    infrared_receive_init();
#endif /* INFRARED_RECEIVE */

    return 0;
}
INIT_APP_EXPORT(drv_infrared_init);

#endif /* PKG_USING_INFRARED */
