/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-11     heyuanjie87  the first version
 */

#include "adb.h"
#include "transport.h"
#include <adb_pque.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME  "ADB TR"
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

#define TR_READ_SPLIT_SIZE    512

#ifndef ADB_TR_STACK_SIZE
#define ADB_TR_STACK_SIZE     1280
#endif

#if !defined(ADB_TR_TCPIP_ENABLE) && \
    !defined(ADB_TR_USB_ENABLE)
#error "at least one transport is needed."
#endif

static bool tr_read(struct adb *d, void *buf, int size, int ms)
{
    bool ret = false;
    char *pos;
    int fail = 0;

    pos = (char*)buf;
    while (1)
    {
        int val;

        if (size == 0)
            return true;
        val = d->ops->poll(d->tr_fd, TRE_READ, ms);
        if (val & TRE_ERROR)
            break;
        if (val == 0)
        {
            if (fail)
                break;
            fail = 1;
            continue;
        }

        val = d->ops->read(d->tr_fd, pos, size);
        if (val > 0)
        {
            pos += val;
            size -= val;
            fail = 0;
        }
        else
        {
            break;
        }
    }

    return ret;
}

static bool tr_write(struct adb *d, void *buf, int size, int ms)
{
    bool ret = false;
    char *pos;
    int fail = 0;

    pos = (char*)buf;
    while (1)
    {
        int val;

        if (size == 0)
            return true;
        val = d->ops->write(d->tr_fd, pos, size);
        if (val > 0)
        {
            pos += val;
            size -= val;
            fail = 0;
        }
        else
        {
            if (fail)
                break;
            val = d->ops->poll(d->tr_fd, TRE_WRITE, ms);
            if (val & TRE_ERROR)
                break;
            fail = 1;
        }
    }

    return ret;
}

static bool write_packet(struct adb *d, struct adb_packet *p)
{
    if (!tr_write(d, &p->msg, sizeof(p->msg), 100))
        return false;

    if (!tr_write(d, p->payload, p->msg.data_length, 200))
        return false;

    return true;
}

static struct adb_packet* _packet_msgdup(struct adb_msg *msg, int datlen)
{
    struct adb_packet *p;

    p = adb_packet_new(datlen);
    if (p)
    {
        rt_memcpy(&p->msg, msg, sizeof(*msg));
        p->msg.data_length = datlen;
    }
    LOG_CON(!p, "msgdup - no mem");
    return p;
}

static bool check_header(struct adb_packet *p)
{
    if (p->msg.magic != (p->msg.command ^ 0xffffffff)) 
    {
        LOG_E("magic command err");
        return false;
    }

    if (p->msg.data_length > MAX_PAYLOAD) 
    {
        LOG_E("payload too long");
        return false;
    }

    return true;
}

static int read_packet_split(struct adb *d, struct adb_packet *ck)
{
    struct adb_packet *p;
    int ret;

    ret = d->ops->poll(d->tr_fd, TRE_READ, 1000);
    if (ret & TRE_ERROR)
        return -1;
    if (ret == 0)
        return 0;

    if (!tr_read(d, &ck->msg, sizeof(struct adb_msg), 0))
        return -1;
    if (!check_header(ck))
    {
        /* clear remain data */
        while (tr_read(d, ck, sizeof(*ck), 20));
        return 1;
    }

    LOG_D("r:%c%c%c%c,len %d", ((char*) (&(ck->msg.command)))[0],
                               ((char*) (&(ck->msg.command)))[1],
                               ((char*) (&(ck->msg.command)))[2],
                               ((char*) (&(ck->msg.command)))[3],
                                           ck->msg.data_length);    

    if (ck->msg.data_length == 0)
    {
        adb_packet_handle(d, ck, false);
    }
    else
    {
        ck->split = (ck->msg.data_length + TR_READ_SPLIT_SIZE - 1)/
                    TR_READ_SPLIT_SIZE;

        while (ck->msg.data_length)
        {
            int rlen = ck->msg.data_length > TR_READ_SPLIT_SIZE ?
                   TR_READ_SPLIT_SIZE : ck->msg.data_length;

            p = _packet_msgdup(&ck->msg, rlen);
            if (!p)//todo
                return 2;

            if (!tr_read(d, p->payload, rlen, 200))
            {
                adb_packet_delete(p);
                LOG_E("read packet %d fail", ck->split - 1);
                return 1;
            }
            ck->msg.data_length -= rlen;
            p->split = --(ck->split);
            adb_packet_handle(d, p, true);
        }
    }

    return 0;
}

static void transport_unref(struct adb *d)
{
    d->tr_refcnt --;//todo lock
    if (d->tr_refcnt == 0)
    {
        adb_packet_clear(&d->send_que);
        d->ops->close(d->tr_fd);
        adb_delete(d);
    }
}

static bool send_sync(struct adb *d, unsigned a0, unsigned a1)
{
    struct adb_packet *p;

    p = adb_packet_new(0);
    if (!p)
        return false;

    p->msg.command = A_SYNC;
    p->msg.arg0 = a0;
    p->msg.arg1 = a1;
    p->msg.magic = A_SYNC ^ 0xffffffff;
    if (!adb_packet_enqueue(&d->send_que, p, 100))
    {
        adb_packet_delete(p);
        return false;
    }

    return true;
}

static void read_thread(void *arg)
{
    struct adb *d = (struct adb *)arg;
    struct adb_packet p;
    int ret;

    d->tr_refcnt ++;
    if (!send_sync(d, 1, 1))
        goto _exit;

    while (!d->quit)
    {
        ret = read_packet_split(d, &p);
        if (ret == -1)
        {
            LOG_D("remote read failed");
            break;   
        }
    }

    if (!send_sync(d, 0, 0))
    {

    }

_exit:
    transport_unref(d);
}

static void write_thread(void *arg)
{
    struct adb *d = (struct adb *)arg;
    struct adb_packet *p = 0;
    bool ret;

    d->tr_refcnt ++;

    /* wait read thread online */
    if (!adb_packet_dequeue(&d->send_que, &p, 500))
        goto _exit; 
    adb_packet_delete(p);

    while (!d->quit)
    {
        if (!adb_packet_dequeue(&d->send_que, &p, 50))
            continue;

        LOG_D("w:%c%c%c%c,len %d", ((char*) (&(p->msg.command)))[0],
              ((char*) (&(p->msg.command)))[1],
              ((char*) (&(p->msg.command)))[2],
              ((char*) (&(p->msg.command)))[3],
              p->msg.data_length);

        if (p->msg.command == A_SYNC) 
        {
            if (p->msg.arg0 == 0) 
            {
                LOG_D("transport SYNC offline");
                adb_packet_delete(p);
                break;
            }
        }

        ret = write_packet(d, p);
        adb_packet_delete(p);
        if (!ret)
        {
            LOG_D("remote write failed");
            break;
        }    
    }

_exit:
    transport_unref(d);
}

int adb_transport_register(int trtype, int fd, const struct adb_tr_ops *ops)
{
    struct adb *d;
    int ret;

    d = adb_new(trtype);
    if (!d)
        return -1;

    d->ops = ops;
    d->tr_fd = fd;

    d->tr_wtid = rt_thread_create("adb-trw",
                            write_thread,
                            d,
                            ADB_TR_STACK_SIZE,
                            22,
                            20);
    d->tr_rtid = rt_thread_create("adb-trr",
                            read_thread,
                            d,
                            ADB_TR_STACK_SIZE,
                            22,
                            20);

    if (rt_thread_startup(d->tr_wtid) == 0)
    {
        ret = rt_thread_startup(d->tr_rtid);
    }

    return ret;
}

void adb_transport_unregister(int trtype)
{
    if (trtype != 0)
        adb_kill(trtype);
}
