/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-26     MurphyZhao   first implementation
 */

#ifndef __IOTB_WORKQEUE_H__
#define __IOTB_WORKQEUE_H__
#include <rtthread.h>

struct rt_workqueue *iotb_work_hd_get(void);
rt_err_t iotb_workqueue_dowork(void (*func)(void *parameter), void *parameter);
rt_err_t iotb_workqueue_start(void);
#endif
