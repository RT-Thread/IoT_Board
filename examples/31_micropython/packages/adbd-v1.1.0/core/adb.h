/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-11     heyuanjie87  the first version
 */

#ifndef __ADB_H__
#define __ADB_H__

#include <stdbool.h>
#include <adbtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PAYLOAD    4*1024
#define A_VERSION      0x01000000

#define A_SYNC 0x434e5953
#define A_CNXN 0x4e584e43
#define A_OPEN 0x4e45504f
#define A_OKAY 0x59414b4f
#define A_CLSE 0x45534c43
#define A_WRTE 0x45545257
#define A_AUTH 0x48545541

struct adb;
struct adb_tr_ops;
struct adb_service;

struct adb_features
{
    int num;
    char **value;
};

struct adb
{
    rt_list_t node;
    rt_list_t s_list;
    bool quit;
    adb_queue_t send_que;
    int sque_buf[8];

    int tr_type;
    int tr_fd;
    int tr_refcnt;
    rt_thread_t tr_rtid;
    rt_thread_t tr_wtid;
    const struct adb_tr_ops *ops;
    struct adb_features *features;

    long user_data;
};

struct adb* adb_new(int trtype);
void adb_delete(struct adb* d);

void adb_send_close(struct adb *d, unsigned local, unsigned remote);

void adb_packet_handle(struct adb *d, struct adb_packet *p, bool pisnew);
void adb_kill(int trtype);

struct adb_service* adb_service_find(struct adb *d, unsigned localid, unsigned remoteid);
unsigned adb_service_create(struct adb *d, char *name, unsigned remoteid);
void adb_service_destroy(struct adb *d, unsigned localid, unsigned remoteid);

#ifdef __cplusplus
}
#endif
#endif
