/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-28     heyuanjie87  the first version
 */

#ifndef __ADBTYPES_H_
#define __ADBTYPES_H_

#include <rtthread.h>
#include <stdbool.h>

typedef struct rt_mailbox adb_queue_t;

struct adb_msg
{
    unsigned command;       /* command identifier constant (A_CNXN, ...) */
    unsigned arg0;          /* first argument                            */
    unsigned arg1;          /* second argument                           */
    unsigned data_length;   /* length of payload (0 is allowed)          */
    unsigned data_crc32;    /* crc32 of data payload                     */
    unsigned magic;         /* command ^ 0xffffffff */
};

struct adb_packet
{
    int split;              /* split MAX_PAYLOAD into  small packet */
    struct adb_msg msg;
    char payload[1];        /* variable-length */
};

#endif
