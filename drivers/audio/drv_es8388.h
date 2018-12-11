/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-15     ZeroFree     first implementation
 */

#ifndef __DRV_ES8388_H__
#define __DRV_ES8388_H__

#include <rtthread.h>
#include <stdint.h>

int es8388_init(struct rt_i2c_bus_device *dev);
void es8388_reset(struct rt_i2c_bus_device *dev);
void es8388_suspend(struct rt_i2c_bus_device *dev);
void es8388_set_volume(struct rt_i2c_bus_device *dev, uint16_t v);

#endif
