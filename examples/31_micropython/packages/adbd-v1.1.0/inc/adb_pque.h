/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-28     heyuanjie87  the first version
 */

#ifndef __ADB_PQUE_H_
#define __ADB_PQUE_H_

#include "adbtypes.h"

struct adb_packet* adb_packet_new(int datlen);
void adb_packet_delete(struct adb_packet *p);
bool adb_packet_enqueue(adb_queue_t *q, struct adb_packet *p, int ms);
bool adb_packet_dequeue(adb_queue_t *q, struct adb_packet **p, int ms);
void adb_packet_clear(adb_queue_t *q);
unsigned adb_packet_checksum(struct adb_packet *p);

#endif
