/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2018-08-26     balanceTWK        the first version
 */

#ifndef __DRV_INFRARED_H__
#define __DRV_INFRARED_H__

#include <rtthread.h>

#define  DWT_CYCCNT  *(volatile unsigned int *)0xE0001004
#define  DWT_CR      *(volatile unsigned int *)0xE0001000
#define  DEM_CR      *(volatile unsigned int *)0xE000EDFC

#define  DEM_CR_TRCENA               (1 << 24)
#define  DWT_CR_CYCCNTENA            (1 <<  0)

#define  INFRARED_BUFF_SIZE 200

enum
{
    LED_ALL_OFF,
    LED_RED,
    LED_GREEN,
    LED_BLUE
};

int infrared_receive_init(void);
int infrared_send_init(void);
rt_err_t start_ir_receive(void);
void stop_ir_receive(void);
rt_err_t infrared_raw_receive(rt_uint32_t *data);
void infrared_raw_send(rt_uint8_t sign, rt_uint16_t us);
void infrared_buff_free(void);

int infrared_init(void);
rt_err_t start_ir_learning(rt_int32_t start_timeout_ms, rt_int32_t recv_timeout_ms);
rt_size_t infrared_receive(rt_uint32_t *data, rt_size_t max_size, rt_int32_t start_timeout_ms, rt_int32_t recv_timeout_ms);
rt_size_t infrared_send(const rt_uint32_t *data, rt_size_t size);

#endif
