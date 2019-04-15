/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-11     heyuanjie87  the first version
 */

#ifndef __ADB_SERVICE_H__
#define __ADB_SERVICE_H__

#include "adb_pque.h"

#ifdef __cplusplus
extern "C" {
#endif

struct adb;
struct adb_service;

struct adb_service_ops
{
    int (*open)(struct adb_service * ser, char *args);
    int (*close)(struct adb_service * ser);
    bool (*enqueue)(struct adb_service * ser, struct adb_packet *p, int ms);
};

struct adb_service_handler
{
    rt_list_t list;
    const char *name;
    struct adb_service* (*create)(struct adb_service_handler *h);
    void (*destroy)(struct adb_service_handler *h, struct adb_service *s);
};

struct adb_service
{
    rt_list_t list;
    int online;

    unsigned localid;
    unsigned remoteid;
    struct adb *d;
    struct adb_service_handler *h;
    const struct adb_service_ops *ops;
    void *extptr;
};

void adb_service_close_report(struct adb_service *ser);
bool adb_service_sendpacket(struct adb_service *ser, 
                             struct adb_packet *p, int ms);


int adb_service_handler_register(struct adb_service_handler *h);
struct adb_service* adb_service_alloc(const struct adb_service_ops *ops, 
                                       int extsize);


#ifdef __cplusplus
}
#endif

#endif
