/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-25     balanceTWK   the first version
 */
 
#ifndef __INFRARED__
#define __INFRARED__

#include <rtthread.h>
#include "decoder.h"

#define CARRIER_WAVE         0xA
#define IDLE_SIGNAL          0xB
#define NO_SIGNAL            0x0

#define MAX_SIZE             5
#define INFRARED_BUFF_SIZE   200

struct ir_raw_data
{
    rt_uint32_t level : 4,
                us    : 28;
};

struct decoder_ops
{
    rt_err_t (*init)(void);
    rt_err_t (*deinit)(void);
    rt_err_t (*read)(struct infrared_decoder_data* data);
    rt_err_t (*write)(struct infrared_decoder_data* data);
    rt_err_t (*decode)(rt_size_t size);
    rt_err_t (*control)(int cmd, void *arg);
};

struct decoder_class 
{
    char* name;
    struct decoder_ops* ops;
    void* user_data;
};

struct infrared_class
{
    struct decoder_class* current_decoder;
    struct decoder_class* decoder_tab[MAX_SIZE];
    rt_uint32_t count;
    struct rt_ringbuffer *ringbuff;
    rt_size_t (*send)(struct ir_raw_data* data, rt_size_t size);
};

rt_err_t driver_report_raw_data(rt_uint8_t level, rt_uint32_t us);

struct infrared_class* infrared_init(void);
int infrared_deinit(void);

rt_err_t ir_decoder_register(struct decoder_class *decoder);

rt_err_t ir_select_decoder(const char* name);

rt_err_t decoder_read_data(struct ir_raw_data* data);
rt_err_t decoder_write_data(struct ir_raw_data* data, rt_size_t size);

rt_err_t infrared_read(const char* decoder_name,struct infrared_decoder_data* data);
rt_err_t infrared_write(const char* decoder_name, struct infrared_decoder_data* data);

#endif /* __INFRARED__ */
