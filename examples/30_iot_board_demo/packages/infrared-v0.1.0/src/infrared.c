/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-25     balanceTWK   the first version
 */

#include <infrared.h>
#include "ipc/ringbuffer.h"

#define DBG_SECTION_NAME     "infrared"
#define DBG_LEVEL     DBG_INFO
#include <rtdbg.h>

static struct infrared_class infrared;

struct decoder_class* ir_find_decoder(const char* name)
{
    for(rt_uint8_t i = 0;i<infrared.count;i++)
    {
        if(rt_strcmp(infrared.decoder_tab[i]->name,name) == 0)
        {
            return infrared.decoder_tab[i];
        }
    }
    return RT_NULL;
}

rt_err_t ir_select_decoder(const char* name)/* Selective decoder */
{
    struct decoder_class* decoder;

    decoder = ir_find_decoder(name);

    if(decoder)
    {
        if(infrared.current_decoder)
        {
            infrared.current_decoder->ops->deinit();
        }
        infrared.current_decoder = decoder;
        if(infrared.current_decoder->ops->init)
        {
            infrared.current_decoder->ops->init();
        }
        LOG_D("select decoder name:%s\n",infrared.decoder_tab[i]->name);
        return RT_EOK;
    }
    LOG_W("The decoder%s cannot be found",name);

    return -RT_ERROR;
}

rt_err_t ir_decoder_register(struct decoder_class *decoder)/* Registered decoder */
{
    infrared.decoder_tab[infrared.count] = decoder;
    infrared.count++;
    return RT_EOK;
}

rt_err_t decoder_read_data(struct ir_raw_data* data)
{
    if(rt_ringbuffer_get(infrared.ringbuff, (rt_uint8_t*)data, 4) == sizeof(struct ir_raw_data))
    {
        LOG_D("rt_ringbuffer get success | %01X:%d",data->level,data->us);
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}

rt_err_t driver_report_raw_data(rt_uint8_t level,rt_uint32_t us)/* Low-level driver usage */
{
    struct ir_raw_data data;

    if(infrared.current_decoder)
    {
        data.level = level;
        data.us = us;

        if( rt_ringbuffer_put(infrared.ringbuff, (rt_uint8_t*)&data, sizeof(struct ir_raw_data)) == sizeof(struct ir_raw_data) )
        {
            LOG_D("it_ringbuffer put success | count:%d;0x%01X;us:%d",(rt_ringbuffer_data_len(&infrared.ir_ringbuff)/sizeof(struct ir_raw_data)),data.level,data.us);
            infrared.current_decoder->ops->decode(rt_ringbuffer_data_len(infrared.ringbuff)/4);
        }
        else
        {
            LOG_E("ir_ringbuffer put fail");
        }

        return RT_EOK;
    }
    return -RT_ERROR;
}

struct infrared_class* infrared_init(void)/* Initializes the necessary functions of the decoder */
{
    if(!infrared.ringbuff)
    {
        infrared.ringbuff = rt_ringbuffer_create((INFRARED_BUFF_SIZE*(sizeof(struct ir_raw_data))));
    }

    return &infrared;
}

int infrared_deinit(void)/* release resource */
{
    rt_ringbuffer_destroy(infrared.ringbuff);
    return 0;
}

rt_err_t decoder_write_data(struct ir_raw_data* data, rt_size_t size)
{
    infrared.send(data, size);
    return RT_EOK;
}
rt_err_t infrared_read(const char* decoder_name, struct infrared_decoder_data* data)
{
    struct decoder_class *decoder;

    if(decoder_name)
    {
        decoder = ir_find_decoder(decoder_name);
    }
    else
    {
        decoder = infrared.current_decoder;
    }

    if(decoder)
    {
        return decoder->ops->read(data);
    }
    return -RT_ERROR;
}

rt_err_t infrared_write(const char* decoder_name, struct infrared_decoder_data* data)
{
    struct decoder_class *decoder;

    if(decoder_name)
    {
        decoder = ir_find_decoder(decoder_name);
    }
    else
    {
        decoder = infrared.current_decoder;
    }

    if(decoder)
    {
        return decoder->ops->write(data);
    }
    return -RT_ERROR;
}
