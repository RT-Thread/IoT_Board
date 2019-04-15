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
 * 2018-11-11     heyuanjie87  the first version
 */

#include "adb.h"
#include <adb_service.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME  "ADB"
#define DBG_LEVEL         DBG_INFO
#define DBG_COLOR
#include <rtdbg.h>

#ifdef DBG_ENABLE
#define LOG_CON(c, fmt, ...) \
    if (c)   \
        LOG_E(fmt, ##__VA_ARGS__)
#else
#define LOG_CON(...)
#endif

static rt_list_t _adb_list = RT_LIST_OBJECT_INIT(_adb_list);

static void write_packet(struct adb *d, struct adb_packet *p)
{
    bool ret;

    p->msg.data_crc32 = adb_packet_checksum(p);
    p->msg.magic = p->msg.command ^ 0xffffffff;
    ret = adb_packet_enqueue(&d->send_que, p, 100);
    if (!ret)
    {
        adb_packet_delete(p);
        LOG_E("write packet fail");
    }
}

static void send_connect(struct adb *d) 
{
    struct adb_packet* cp;
    int slen;
    char *con_str = "device::" \
     "ro.product.name=mido;" \
     "ro.product.model=rtthread;" \
     "ro.product.device=mido;" \
     "features=cmd,shell_v1";

    slen = rt_strlen(con_str);
    cp = adb_packet_new(slen);

    cp->msg.command = A_CNXN;
    // Send the max supported version, but because the transport is
    // initialized to A_VERSION_MIN, this will be compatible with every
    // device.
    cp->msg.arg0 = A_VERSION;
    cp->msg.arg1 = MAX_PAYLOAD;

    cp->msg.data_length = slen;
    rt_memcpy(cp->payload, con_str, slen);

    write_packet(d, cp);
}

static void send_ready(struct adb *d, unsigned local, unsigned remote)
{
    struct adb_packet* p = adb_packet_new(0);

    p->msg.command = A_OKAY;
    p->msg.arg0 = local;
    p->msg.arg1 = remote;
    write_packet(d, p);
}

void adb_send_close(struct adb *d, unsigned local, unsigned remote)
{
    struct adb_packet* p = adb_packet_new(0);

    p->msg.command = A_CLSE;
    p->msg.arg0 = local;
    p->msg.arg1 = remote;
    write_packet(d, p);
}

static struct adb_features *get_features_for_handle(const char *handle, int handle_len)
{
    const char *curr;
    int str_len, i;
    int num = 1;
    struct adb_features *adb_ft;
    char *temp;

    curr = handle;
    str_len = rt_strlen("host::");
    if (rt_strncmp("host::", curr, str_len) != 0)
    {
        LOG_D("Invalid connection request.");
        return NULL;
    }

    curr += str_len;
    str_len = rt_strlen("features=");
    if (rt_strncmp("features=", curr, str_len) != 0)
    {
        LOG_D("Invalid feature information.");
        return NULL;
    }

    curr += str_len;
    str_len = handle + handle_len - curr;
    for (i = 0; i < str_len; i++)
    {
        if (curr[i] == ',')
        {
            num += 1;
        }
    }
    adb_ft = rt_malloc(sizeof(struct adb_features) + sizeof(char *) * num + str_len + 1);
    adb_ft->num = num;
    adb_ft->value = (char **)&adb_ft[1];
    temp = (char *)adb_ft + sizeof(struct adb_features) + sizeof(char *) * num;
    rt_strncpy(temp, curr, str_len);
    temp[str_len] = '\0';

    num = 0;
    adb_ft->value[num] = temp;
    num += 1;
    for (i = 0; num < adb_ft->num && i < str_len; i++)
    {
        if (temp[i] == ',')
        {
            temp[i] = '\0';
            adb_ft->value[num] = &temp[i+1];
            num += 1;
        }
    }

    return adb_ft;
}

static void handle_new_connection(struct adb *d, struct adb_packet *p)
{
    struct adb_features *adb_ft;

    adb_ft = get_features_for_handle(p->payload, p->msg.data_length);
    if (adb_ft == NULL)
    {
        return;
    }

    rt_free(d->features);
    d->features = adb_ft;
    send_connect(d);
}

static bool _service_enqueue(struct adb_service *ser, struct adb_packet *p)
{
    bool ret;

    ret = ser->ops->enqueue(ser, p, 1000);

    return ret; 
}

void adb_packet_handle(struct adb *d, struct adb_packet *p, bool pisnew)
{
    bool del = true;

    if (!p)
        return;

    switch(p->msg.command)
    {
    case A_CNXN: /* CONNECT(version, maxdata, "system-id-string") */
    {
        handle_new_connection(d, p);
    }break;
    case A_OPEN: /* OPEN(local-id, 0, "destination") */
    {
        if (p->msg.arg0 != 0 && p->msg.arg1 == 0)
        {
            int localid;

            localid = adb_service_create(d, p->payload, p->msg.arg0);
            if (localid)
            {
                send_ready(d, localid, p->msg.arg0);
            }
            else
            {
                adb_send_close(d, 0, p->msg.arg0);
            }
        }
    }break;
    case A_WRTE:
    {
        if (p->msg.arg0 != 0 && p->msg.arg1 != 0)
        {
            struct adb_service *ser;
            int split = p->split;

            ser = adb_service_find(d, p->msg.arg1, p->msg.arg0);
            if (ser)
            {
                if (_service_enqueue(ser, p))
                {
                    if (split == 0)
                        send_ready(d, ser->localid, ser->remoteid);
                    del = false;
                }
                else
                {
                    LOG_E("service enqueue failed");
                }
            }
        }
    }break;
    case A_OKAY:
    {

    }break;
    case A_CLSE:
    {
        if (p->msg.arg1 != 0)
        {
            adb_service_destroy(d, p->msg.arg1, p->msg.arg0);
        }
    }break;
    }

    if (del && pisnew)
        adb_packet_delete(p);
}

static bool _isexist(int trtype)
{
    struct rt_list_node *node, *head;
    struct adb *d;
    bool e = false;

    head = &_adb_list;
    for (node = head->next; node != head; )
    {
        d = rt_list_entry(node, struct adb, node);
        node = node->next;
        if (d->tr_type == trtype)
        {
            e = true;
            break;
        }
    }

    return e;
}

struct adb* adb_new(int trtype)
{
    struct adb *d = 0;

    if (_isexist(trtype))
        return d;

    d = rt_calloc(sizeof(struct adb), 1);
    if (!d)
        return d;

    d->tr_type = trtype;
    rt_list_init(&d->node);
    rt_list_insert_after(&_adb_list, &d->node);

    rt_list_init(&d->s_list);
    rt_mb_init(&d->send_que, "adbsque", d->sque_buf, 
               sizeof(d->sque_buf)/sizeof(d->sque_buf[0]), 0);

    return d;
}

void adb_delete(struct adb *d)
{
    struct rt_list_node *node, *head;

    rt_list_remove(&d->node);

    head = &d->s_list;
    for (node = head->next; node != head; )
    {
        struct adb_service *ser;
        struct adb_service_handler *h;

        ser = rt_list_entry(node, struct adb_service, list);
        node = node->next;
        h = ser->h;

        rt_list_remove(&ser->list);
        ser->ops->close(ser);
        h->destroy(h, ser);
    }
    rt_mb_detach(&d->send_que);
    rt_free(d->features);
    rt_free(d);
}

void adb_kill(int trtype)
{
    struct rt_list_node *node, *head;
    struct adb *d;

    head = &_adb_list;
    for (node = head->next; node != head; )
    {
        d = rt_list_entry(node, struct adb, node);
        node = node->next;
        if ((d->tr_type == trtype) || (trtype == 0))
        {
            d->quit = true;
            rt_thread_mdelay(100);
        }
    }
}
