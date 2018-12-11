/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2018-08-17     zylx              the first version
 */
 
#ifndef __STM32_QSPI_H_
#define __STM32_QSPI_H_
#include <rtthread.h>
#include <rthw.h>
#include "board.h"
#include <rtdevice.h>

rt_err_t stm32_qspi_bus_attach_device(rt_uint32_t pin, const char *bus_name, const char *device_name);

#endif
