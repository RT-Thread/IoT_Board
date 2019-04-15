/*
 * Copyright (c) 2018, Real-Thread Information Technology Ltd
 * All rights reserved
 *
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-12-18     tyx          the first version
 */

#ifndef __FILE_EXMOD_H__
#define __FILE_EXMOD_H__

#include <stdint.h>
#include <adb_service.h>

#define FILE_EXMOD_HEAD_PRE     ("#")
#define FILE_EXMOD_HEAD_POS     ("#")
#define FILE_EXMOD_SIZE_PRE     ("<")
#define FILE_EXMOD_SIZE_POS     (">")
#define FILE_EXMOD_PARAM_PRE    ("*")
#define FILE_EXMOD_PARAM_POS    ("*")

#define FILE_EXMOD_PARAM_DIV    (',')
#define FILE_EXMOD_KEY_DIV      ('=')

struct f_param
{
    char *key;
    char *value;
};

struct f_exmod
{
    struct adb_service *ser;
    union file_syncmsg *msg;
    char *name;
    char *path;
    int file_size;
    int param_num;
    struct f_param *param_res;
    char *param;
};

struct f_exmod_handler
{
    //TODO return type
    rt_slist_t list;
    const char *name;
    int (*push)(struct f_exmod *exmod);
    int (*pull)(struct f_exmod *exmod);
};

struct f_exmod *file_exmod_create(const char *name);
void file_exmod_delete(struct f_exmod *exmod);
int file_exmod_register(struct f_exmod_handler *handle);
struct f_exmod_handler *file_exmod_unregister(const char *name);
struct f_exmod_handler *file_exmod_find(const char *name);

int file_exmod_read_data(struct f_exmod *exmod, void *buffer, int size);
int file_exmod_write_data(struct f_exmod *exmod, void *buffer, int size);

bool file_exmod_do_lstat_v1(struct adb_service *ser, struct f_exmod *exmod);
bool file_exmod_do_list(struct adb_service *ser, struct f_exmod *exmod);
bool file_exmod_do_recv(struct adb_service *ser,  struct f_exmod *exmod);
bool file_exmod_do_send(struct adb_service *ser, struct f_exmod *exmod);
#endif
