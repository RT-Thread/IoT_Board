/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2018-11-11     heyuanjie87    the first version
 */

#include "adb.h"
#include <adb_service.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME  "ADB ser"
#define DBG_LEVEL         DBG_INFO
#define DBG_COLOR
#include <rtdbg.h>

static rt_list_t _shr_list = RT_LIST_OBJECT_INIT(_shr_list);

struct adb_service* adb_service_find(struct adb *d, unsigned localid, unsigned remoteid)
{
    struct rt_list_node *node, *head;
    struct adb_service *ret = 0;

    head = &d->s_list;
    for (node = head->next; node != head; node = node->next)
    {
        struct adb_service *ser;

        ser = rt_list_entry(node, struct adb_service, list);
        if (ser->localid == localid)
        {
            ret = ser;
            goto _ok;
        }
    }
    LOG_D("service: no id [%X][%X]", localid, remoteid);
_ok:
    return ret;
}

bool adb_service_sendpacket(struct adb_service *ser, 
                             struct adb_packet *p, int ms)
{
    p->msg.command = A_WRTE;
    p->msg.arg0 = ser->localid;
    p->msg.arg1 = ser->remoteid;
    p->msg.data_crc32 = adb_packet_checksum(p);
    p->msg.magic = p->msg.command ^ 0xffffffff;

    return adb_packet_enqueue(&ser->d->send_que, p, ms);
}

static unsigned _service_init(struct adb_service_handler *h, 
                              struct adb_service *ser, 
                              struct adb *d, unsigned rid, char *args)
{
    if (!ser)
    {
        return 0;
    }
    if (ser->ops->open(ser, args) != 0)
    {
        h->destroy(h, ser);
        return 0;
    }

    ser->h = h;
    ser->d = d;
    ser->localid = rt_tick_get();
    ser->remoteid = rid;
    rt_list_init(&ser->list);
    rt_list_insert_after(&d->s_list, &ser->list);

    return ser->localid;
}

void adb_service_destroy(struct adb *d, unsigned localid, unsigned remoteid)
{
    struct adb_service *ser;

    ser = adb_service_find(d, localid, remoteid);
    if (ser)
    {
        struct adb_service_handler *h = ser->h;

        rt_list_remove(&ser->list);
        ser->ops->close(ser);
        h->destroy(h, ser);
    }
}

unsigned adb_service_create(struct adb *d, char *name, unsigned remoteid)
{
    struct rt_list_node *node, *head;
    struct adb_service_handler *h;

    head = &_shr_list;
    for (node = head->next; node != head; node = node->next)
    {
        int n;

        h = rt_list_entry(node, struct adb_service_handler, list);
        n = rt_strlen(h->name);
        if (rt_strncmp(h->name, name, n) == 0)
        {
            struct adb_service *ser;

            ser = h->create(h);
            return _service_init(h, ser, d, remoteid, &name[n]);
        }
    }

    return 0;    
}

int adb_service_handler_register(struct adb_service_handler *h)
{
    struct rt_list_node *node, *head;
    struct adb_service_handler *entry;

    if (!h->name || !h->create || !h->destroy)
        return -1;

    head = &_shr_list;
    for (node = head->next; node != head; node = node->next)
    {
        entry = rt_list_entry(node, struct adb_service_handler, list);
        if (rt_strcmp(h->name, entry->name) == 0)
            return -1;
    }

    rt_list_init(&h->list);
    rt_list_insert_after(head, &h->list);

    return 0;
}

struct adb_service* adb_service_alloc(const struct adb_service_ops *ops, 
                                       int extsize)
{
    struct adb_service* ser;

    ser = rt_calloc(sizeof(*ser) + extsize, 1);
    if (ser && extsize)
    {
        ser->ops = ops;
        ser->extptr = (char*)ser + sizeof(*ser);
    }

    return ser;
}

void adb_service_close_report(struct adb_service *ser)
{
    adb_send_close(ser->d, ser->localid, ser->remoteid);
}
