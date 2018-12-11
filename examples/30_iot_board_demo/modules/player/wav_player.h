/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-17     MurphyZhao   first implementation
 */

#ifndef __WAV_PLAYER__
#define __WAV_PLAYER__
#include <rtthread.h>

typedef void (*play_finish)(void);

typedef enum
{
    IOTB_PLAYER_NONE  = (1 << 0), /* 1 */
    IOTB_PLAYER_START = (1 << 1),
    IOTB_PLAYER_STOP  = (1 << 2),
    IOTB_PLAYER_MAX   = (1 << 3),
} iotb_player_event_t;

int iotb_wav_player_init(play_finish cb);
void iotb_player_start(const char* path);
void iotb_player_stop(void);
rt_bool_t iotb_player_isbusy(void);

#endif /* __WAV_PLAYER__ */
