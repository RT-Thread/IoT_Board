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

#include <adb_service.h>
#include "file_sync_service.h"
#include "file_exmod.h"
#include <dfs_posix.h>

#ifdef ADB_FILESYNC_MD5_ENABLE
#include <tiny_md5.h>
#endif

#ifndef ADB_FILESYNC_STACK_SIZE
#define ADB_FILESYNC_STACK_SIZE    2304
#endif

#ifndef ADB_FILESYNC_RECV_TIMEOUT
#define ADB_FILESYNC_RECV_TIMEOUT  2000
#endif

#define DBG_ENABLE
#define DBG_SECTION_NAME  "ADB sync"
#define DBG_LEVEL         DBG_INFO
#define DBG_COLOR
#include <rtdbg.h>

static char* sync_string_new(char *s1, char *s2, int s2n)
{
    char *s;
    int len;

    len = rt_strlen(s1);
    s = rt_malloc(len + s2n + 1);
    if (s)
    {
        rt_memcpy(s, s1, len);
        rt_memcpy(&s[len], s2, s2n);
        s[len + s2n] = 0;
    }

    return s;
}

static bool sync_read(struct adb_service *ser, void *buf, int size, int ms)
{
    struct filesync_ext *ext = (struct filesync_ext *)ser->extptr;
    int len;
    struct adb_packet *p;
    char *dpos = (char*)buf;
    char *spos;

    while (size)
    {
        if (!ext->cur)
        {
            if (!adb_packet_dequeue(&ext->recv_que, &ext->cur, ms))
            {
                return false;
            }
            ext->cur->split = 0; //used as postion of read
        }

        p = ext->cur;
        spos = (char*)(p->payload + p->split);

        len = size > p->msg.data_length ? p->msg.data_length : size;
        rt_memcpy(dpos, spos, len);
        p->msg.data_length -= len;
        p->split += len;
        dpos += len;
        size -= len;

        if (p->msg.data_length == 0)
        {
            ext->cur = 0;
            adb_packet_delete(p);
        }
    }

    return true;
}

static bool sync_write(struct adb_service *ser, void *buf, int size, int ms)
{
    struct adb_packet *p;

    p = adb_packet_new(size);
    if (!p)
    {
        return false;
    }
    rt_memcpy(p->payload, buf, size);
    if (!adb_service_sendpacket(ser, p, ms))
    {
        adb_packet_delete(p);
        return false;
    }

    return true;
}

static bool SendSyncFail(struct adb_service *ser, const char *reason)
{
    union file_syncmsg msg;
    bool ret;

    LOG_E("fail - %s", reason);

    msg.data.id = ID_FAIL;
    msg.data.size = rt_strlen(reason);

    ret = sync_write(ser, &msg.data, sizeof(msg.data), 50);
    if (ret)
        ret = sync_write(ser, (void*)reason, msg.data.size, 50);

    return ret;
}

static bool sync_read_path(struct adb_service *ser, char **name, int len)
{
    *name = 0;
    if (len == 0)
        return true;

    if (len > 1024)
    {
        SendSyncFail(ser, "path too long");
        return false;
    }
    *name = rt_malloc(len + 1);
    if (!*name)
    {
        SendSyncFail(ser, "alloc memory failure");
        return false;
    }
    if (!sync_read(ser, *name, len, ADB_FILESYNC_RECV_TIMEOUT))
    {
        SendSyncFail(ser, "filename read failure");
        rt_free(*name);
        *name = 0;
        return false;
    }
    (*name)[len] = 0;

    return true;
}

static bool do_lstat_v1(struct adb_service *ser, const char* path) 
{
    union file_syncmsg msg = {0};
    struct stat st = {0};

    msg.stat_v1.id = ID_LSTAT_V1;

    stat(path, &st);
    msg.stat_v1.mode = st.st_mode;
    msg.stat_v1.size = st.st_size;
    msg.stat_v1.time = st.st_mtime;

    return sync_write(ser, &msg.stat_v1, sizeof(msg.stat_v1), 50);
}

static bool do_recv(struct adb_service *ser, char* path, char* buffer) 
{
    union file_syncmsg msg;
    int fd;
    bool ret = false;

    fd = open(path, O_RDONLY);
    if (fd < 0) 
    {
        SendSyncFail(ser, "open failed");
        return false;
    }

    msg.data.id = ID_DATA;
    while (1)
    {
        int r = read(fd, &buffer[0], SYNC_DATA_MAX);
        if (r <= 0) 
        {
            if (r == 0) 
                break;
            SendSyncFail(ser, "read failed");
            close(fd);
            return false;
        }

        msg.data.size = r;
        ret = sync_write(ser, &msg.data, sizeof(msg.data), 400);
        if (ret)
            ret = sync_write(ser, &buffer[0], r, 100);
        if (!ret) 
        {
            close(fd);
            return false;
        }
    }

    close(fd);

    msg.data.id = ID_DONE;
    msg.data.size = 0;
    return sync_write(ser, &msg.data, sizeof(msg.data), 200);
}

static bool do_list(struct adb_service *ser, char* path) 
{
    union file_syncmsg msg;
    struct dirent *de;
    DIR* d;
    bool ret = true;

    msg.dent.id = ID_DENT;

    d = opendir(path);
    if (!d) 
        goto done;

    while (1) 
    {
        struct stat st = {0};
        char *filename;
        int  d_name_length;

        de = readdir(d);
				if (!de)
            break;

        d_name_length = rt_strlen(de->d_name);
				filename = sync_string_new(path, de->d_name, d_name_length);
        if (!filename)
        {
            ret = false;
            SendSyncFail(ser, "alloc memory fail");
            goto fail;
        }
        if (stat(filename, &st) == 0) 
        {
            msg.dent.mode = st.st_mode;
            msg.dent.size = st.st_size;
            msg.dent.time = st.st_mtime;
            msg.dent.namelen = d_name_length;

            ret = sync_write(ser, &msg.dent, sizeof(msg.dent), 200);
            if (ret)
                ret = sync_write(ser, de->d_name, d_name_length, 100);
            if (!ret) 
            {
                LOG_E("sync: list write fail");
                rt_free(filename);
                goto fail;
            }
        }
        rt_free(filename);
    }

fail:
    closedir(d);

done:
    if (ret)
    {
        msg.dent.id = ID_DONE;
        msg.dent.mode = 0;
        msg.dent.size = 0;
        msg.dent.time = 0;
        msg.dent.namelen = 0;

        ret = sync_write(ser, &msg.dent, sizeof(msg.dent), 50);
    }

    return ret;
}

static bool hasdir(char *path)
{
    struct stat st = {0};

    if (stat(path, &st) < 0)
        return false;

    if (st.st_mode & S_IFDIR)
        return true;

    return false;
}

static bool secure_mkdirs(char *path)
{
    int plen;
    int pos;
    int sub;

    if (path[0] != '/') 
        return false;

    plen = rt_strlen(path);
    for (pos = 0; pos < plen; pos ++)
    {
        if (path[pos] != '/')
            continue;

        for (sub = pos + 1; sub < plen; sub ++)
        {
            if (path[sub] != '/')
                continue;
            path[sub] = 0;
            if (hasdir(path))
            {
                path[sub] = '/';
                break;
            }

            if (mkdir(path, 0) < 0)
                return false;
            path[sub] = '/';
        }
    }

    return true;
}

static bool handle_send_file(struct adb_service *ser, char* path, 
                             mode_t mode, char* buffer, bool do_unlink)
{
    union file_syncmsg msg;
    int fd;

    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);

    if (fd < 0) 
    {
        int eno;

        eno = rt_get_errno();
        if (eno == ENOENT || eno == -ENOENT)
        {
            if (!secure_mkdirs(path)) 
            {
                SendSyncFail(ser, "secure_mkdirs failed");
                goto fail;
            }           
        }

        fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
    }

    if (fd < 0) 
    {
        SendSyncFail(ser, "couldn't create file");
        goto fail;
    }

    while (1) 
    {
        if (!sync_read(ser, &msg.data, sizeof(msg.data), ADB_FILESYNC_RECV_TIMEOUT))
        {
            SendSyncFail(ser, "read data head fail");
            goto fail;
        }

        if (msg.data.id != ID_DATA) 
        {
            if (msg.data.id == ID_DONE) 
            {
                break;
            }
            SendSyncFail(ser, "invalid data message");
            goto abort;
        }

        /* recv file data */
        while (msg.data.size > 0)
        {
            int size = (msg.data.size > SYNC_DATA_MAX) ? 
                   SYNC_DATA_MAX : msg.data.size;

            if (!sync_read(ser, &buffer[0], size, ADB_FILESYNC_RECV_TIMEOUT))
            {
                SendSyncFail(ser, "read packet timeout");
                goto abort;
            }
#ifndef FILESYNC_DATA_NO_WRITE /* for test */
            if (write(fd, &buffer[0], size) != size) 
            {
                SendSyncFail(ser, "write failed");
                goto fail;
            }
#endif
            msg.data.size -= size;           
        }
    }

    close(fd);

    msg.status.id = ID_OKAY;
    msg.status.msglen = 0;
    return sync_write(ser, &msg.status, sizeof(msg.status), 100);

fail:
abort:
    if (fd >= 0) 
        close(fd);
    if (do_unlink) 
        unlink(path);

    return false;
}

static bool do_send(struct adb_service *ser, char* spec, char *buffer) 
{
    char *path;
    mode_t mode = 0;
    bool do_unlink = true;
    int comma = 0;

    while (spec[comma] && (spec[comma] != ','))
        comma ++;

    if (comma == rt_strlen(spec))
    {
        SendSyncFail(ser, "missing , in ID_SEND");
        return false;
    }

    spec[comma] = 0;
    path = spec;

    return handle_send_file(ser, path, mode, buffer, do_unlink);
}

#ifdef ADB_FILESYNC_MD5_ENABLE
static bool do_calcmd5(struct adb_service *ser, char* spec, char *buffer)
{
    union file_syncmsg msg;
    tiny_md5_context c;
    int fd;

    fd = open(spec, O_RDONLY, 0);
    if (fd < 0)
    {
        SendSyncFail(ser, "md5 open file fail");
        return false;
    }

    tiny_md5_starts(&c);
    while (1)
    {
        int len = read(fd, buffer, SYNC_DATA_MAX);
        if (len < 0)
        {
            close(fd);
            SendSyncFail(ser, "md5 read io fail");
            return false;
        }
        else if (len == 0)
            break;

        tiny_md5_update(&c, (unsigned char*)buffer, len);
    }
    tiny_md5_finish(&c, msg.md5.value);
    close(fd);

    msg.md5.id = ID_CMD5;
    return sync_write(ser, &msg.md5, sizeof(msg.md5), 50);
}
#else
static bool do_calcmd5(struct adb_service *ser, char* spec, char *buffer)
{
    ((void)spec);
    ((void)buffer);
    SendSyncFail(ser, "ID_CMD5 not enabled");
    return false;
}
#endif

static bool handle_sync_command(struct adb_service *ser, char *buffer)
{
    bool ret = false;
    struct file_syncreq req;
    char *name;
    struct f_exmod *exmod;

    if (!buffer)
    {
        SendSyncFail(ser, "buffer is null");
        return false;
    }

    if (!sync_read(ser, &req, sizeof(req), ADB_FILESYNC_RECV_TIMEOUT))
    {
        SendSyncFail(ser, "command read failure");
        return false;
    }
 
    if (!sync_read_path(ser, &name, req.path_length))
        return false;

    exmod = file_exmod_create(name);
    if (exmod != NULL)
    {
        exmod->ser = ser;
    }

    switch (req.id)
    {
    case ID_LSTAT_V1:
        if (exmod != RT_NULL)
        {
            ret = file_exmod_do_lstat_v1(ser, exmod);
        }
        else
        {
            ret = do_lstat_v1(ser, name);
        }
        break;
    case ID_LSTAT_V2:
    case ID_STAT_V2:
        break;
    case ID_LIST:
        if (exmod != RT_NULL)
        {
            ret = file_exmod_do_list(ser, exmod);
        }
        else
        {
            ret = do_list(ser, name);
        }
        break;
    case ID_SEND:
        if (exmod != RT_NULL)
        {
            ret = file_exmod_do_send(ser, exmod);
        }
        else
        {
            ret = do_send(ser, name, buffer);
        }
        break;
    case ID_RECV:
        if (exmod != RT_NULL)
        {
            ret = file_exmod_do_recv(ser, exmod);
        }
        else
        {
            ret = do_recv(ser, name, buffer);
        }
        break;
    case ID_CMD5:
        ret = do_calcmd5(ser, name, buffer);
        break;
    case ID_QUIT:
        break;
    default:
        SendSyncFail(ser, "unknown command");
        break;
    }
    rt_free(name);
    file_exmod_delete(exmod);

    return ret;
}

static void filesync_thread(void *arg)
{
    struct adb_service *ser = (struct adb_service *)arg;
    struct filesync_ext *ext = (struct filesync_ext *)ser->extptr;
    char *buf;

    LOG_D("start");
    buf = rt_malloc(SYNC_DATA_MAX);

    while (handle_sync_command(ser, buf)){}

    ser->online = 0;
    rt_free(buf);
    adb_packet_delete(ext->cur);
    adb_packet_clear(&ext->recv_que);
    adb_service_close_report(ser);

    rt_event_send(&ext->join, 1);

    LOG_D("quit");
}

static int _filesync_open(struct adb_service *ser, char *args)
{
    int ret = -1;
    struct filesync_ext *ext;

    ext = (struct filesync_ext *)ser->extptr;
    rt_mb_init(&ext->recv_que, "sync", ext->rque_buf, 
               sizeof(ext->rque_buf)/sizeof(ext->rque_buf[0]), 0);
    rt_event_init(&ext->join, "sync", 0);

    ret = rt_thread_startup(ext->worker);
    ser->online = 1;

    return ret;
}

static int _filesync_close(struct adb_service *ser)
{
    int ret = 0;
    struct filesync_ext *ext;

    ext = (struct filesync_ext *)ser->extptr;
    ser->online = 0;
    rt_event_recv(&ext->join, 1, RT_EVENT_FLAG_OR,
                      rt_tick_from_millisecond(ADB_FILESYNC_RECV_TIMEOUT * 2), 0);
    rt_event_detach(&ext->join);
    rt_mb_detach(&ext->recv_que);

    return ret;
}

static bool _filesync_enqueue(struct adb_service * ser, struct adb_packet *p, int ms)
{
    struct filesync_ext *ext;
    bool ret;

    if (!ser->online)
    {
        return false;
    }

    ext = (struct filesync_ext *)ser->extptr;
    ret = adb_packet_enqueue(&ext->recv_que, p, ADB_FILESYNC_RECV_TIMEOUT);
    return ret;
}

static const struct adb_service_ops _ops =
{
    _filesync_open,
    _filesync_close,
    _filesync_enqueue
};

static struct adb_service *_filesync_create(struct adb_service_handler *h)
{
    struct adb_service *ser;
    struct filesync_ext *ext;

    ser = adb_service_alloc(&_ops, sizeof(struct filesync_ext));
    if (!ser)
        return ser;
    ext = (struct filesync_ext *)ser->extptr;

    ext->worker = rt_thread_create("sync:",
                            filesync_thread,
                            ser,
                            ADB_FILESYNC_STACK_SIZE,
                            22,
                            20);
    if (!ext->worker)
    {
        rt_free(ser);
        return RT_NULL;
    }

    return ser;
}

static void _filesync_destroy(struct adb_service_handler *h, struct adb_service *s)
{
    rt_free(s);
}

int adb_filesync_init(void)
{
    static struct adb_service_handler _h;

    _h.name = "sync:";
    _h.create = _filesync_create;
    _h.destroy = _filesync_destroy;

    return adb_service_handler_register(&_h);
}
INIT_APP_EXPORT(adb_filesync_init);
