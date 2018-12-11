/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     ZeroFree     first implementation
 */

#ifndef __DRV_AUDIO_H__
#define __DRV_AUDIO_H__

#include <rtthread.h>
#include <drivers/audio.h>
#include <audio_pipe.h>

int SAIA_SampleRate_Set(uint32_t samplerate);
int rt_hw_audio_init(char *i2c_bus_name);

#endif
