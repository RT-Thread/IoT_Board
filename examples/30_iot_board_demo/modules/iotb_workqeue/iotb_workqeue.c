/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-26     MurphyZhao   first implementation
 */

#include <rtthread.h>
#include <stdint.h>
#include <ipc/workqueue.h>

#define IOTB_WK_DEBUG

#define DBG_ENABLE
#define DBG_TAG               "IOTB_JOB"
#ifdef IOTB_WK_DEBUG
#define DBG_LVL                      DBG_LOG
#else
#define DBG_LVL                      DBG_INFO /* DBG_ERROR */
#endif 
#define DBG_COLOR
#include <rtdbg.h>

static struct rt_workqueue * iotb_work_hd = RT_NULL;

typedef struct
{
    struct rt_work work;
    void (*fun)(void *parameter);
    void *parameter;
} iotb_work_t;

static void iotb_workqueue_fun(struct rt_work *work, void *work_data)
{
    iotb_work_t *iotb_work = work_data;

    iotb_work->fun(iotb_work->parameter);
    rt_free(iotb_work);
}

struct rt_workqueue *iotb_work_hd_get(void)
{
    return iotb_work_hd;
}

rt_err_t iotb_workqueue_dowork(void (*func)(void *parameter), void *parameter)
{
    iotb_work_t *iotb_work;
    rt_err_t err = RT_EOK;

    LOG_D("F:%s is run", __FUNCTION__);
    if (func == RT_NULL)
    {
        LOG_E("F:%s L:%d func is null", __FUNCTION__, __LINE__);
        return -RT_EINVAL;
    }

    if (iotb_work_hd == RT_NULL)
    {
        LOG_E("F:%s L:%d not init iotb work queue", __FUNCTION__, __LINE__);
        return -RT_ERROR;
    }

    iotb_work = rt_malloc(sizeof(iotb_work_t));
    if (iotb_work == RT_NULL)
    {
        LOG_E("F:%s L:%d create work failed, no memory", __FUNCTION__, __LINE__);
        return -RT_ENOMEM;
    }

    iotb_work->fun = func;
    iotb_work->parameter = parameter;

    rt_work_init(&iotb_work->work, iotb_workqueue_fun, iotb_work);
    err = rt_workqueue_dowork(iotb_work_hd, &iotb_work->work);
    if (err != RT_EOK)
    {
        LOG_E("F:%s L:%d do work failed", __FUNCTION__, __LINE__);
        rt_free(iotb_work);
        iotb_work = RT_NULL;
    }

    return err;
}

rt_err_t iotb_workqueue_start(void)
{
    static rt_int8_t iotb_work_started = 0;

    if (iotb_work_started == 0)
    {
        iotb_work_hd = rt_workqueue_create("iotb_job", 2048, RT_THREAD_PRIORITY_MAX/2-2);
        if (iotb_work_hd == RT_NULL)
        {
            LOG_E("F:%s L:%d iotb work queue create failed", __FUNCTION__, __LINE__);
            return -RT_ERROR;
        }
        iotb_work_started = 1;
        return RT_EOK;
    }
    return RT_EOK;
}
