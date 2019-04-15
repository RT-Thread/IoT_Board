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

#include <rtthread.h>
#include <string.h>
#include <dfs_posix.h>
#include "file_exmod.h"
#include "file_sync_service.h"

#define DBG_ENABLE
#define DBG_SECTION_NAME  "sync_mod"
#define DBG_LEVEL         DBG_INFO
#define DBG_COLOR
#include <rtdbg.h>

#ifdef ADB_EXTERNAL_MOD_ENABLE

static struct rt_slist_node _excnd_mod_head;

static bool file_exmod_write(struct adb_service *ser, void *buf, int size, int ms)
{
    struct adb_packet *p;

    if (ser->online == 0)
    {
        return false;
    }

    p = adb_packet_new(size);
    if (!p)
    {
        LOG_E("new packet fail. discard packets");
        return false;
    }
    memcpy(p->payload, buf, size);
    if (!adb_service_sendpacket(ser, p, ms))
    {
        LOG_E("send packet fail. delete packets");
        adb_packet_delete(p);
        return false;
    }

    return true;
}

static bool file_exmod_read(struct adb_service *ser, void *buf, int size, int ms)
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
                LOG_E("depacket fail. return false");
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

static bool file_exmod_send_fail(struct adb_service *ser, const char *reason)
{
    union file_syncmsg msg = { 0 };
    bool ret;

    msg.data.id = ID_FAIL;
    msg.data.size = strlen(reason);

    LOG_D("F:%s L:%d reason:%s", __FUNCTION__, __LINE__, reason);

    ret = file_exmod_write(ser, &msg.data, sizeof(msg.data), 50);
    if (ret == true)
    {
        ret = file_exmod_write(ser, (void*)reason, msg.data.size, 50);
    }

    return ret;
}

int file_exmod_write_data(struct f_exmod *exmod, void *buffer, int size)
{
    int curr_size = 0, r;
    union file_syncmsg msg = { 0 };
    bool ret = true;

    LOG_D("F:%s L:%d run. size:%d", __FUNCTION__, __LINE__, size);

    if((exmod == NULL) || (buffer == NULL) || (size == 0))
    {
        return 0;
    }
    while (curr_size < size)
    {
        r = (size - curr_size) > SYNC_DATA_MAX ? SYNC_DATA_MAX : size - curr_size;
        msg.data.id = ID_DATA;
        msg.data.size = r;
        ret = file_exmod_write(exmod->ser, &msg.data, sizeof(msg.data), 400);
        if (ret)
        {
            ret = file_exmod_write(exmod->ser, (char*)buffer + curr_size, r, 100);
        }
        if (!ret) 
        {
            LOG_E("write data err. return");
            return 0;
        }
        curr_size += r;
    }
    return curr_size;
}

int file_exmod_read_data(struct f_exmod *exmod, void *buffer, int size)
{
    union file_syncmsg *msg;
    int curr_size = 0, r;

    LOG_D("F:%s L:%d run. size:%d", __FUNCTION__, __LINE__, size);

    while (curr_size < size)
    {
        if (exmod->msg == NULL)
        {
            msg = malloc(sizeof(union file_syncmsg));
            if (msg == NULL)
            {
                file_exmod_send_fail(exmod->ser, "malloc file sync msg head fail");
                return 0;
            }

            if (!file_exmod_read(exmod->ser, &msg->data, sizeof(msg->data), 100))
            {
                file_exmod_send_fail(exmod->ser, "read data head fail");
                return 0;
            }

            if (msg->data.id != ID_DATA)
            {
                if (msg->data.id == ID_DONE)
                {
                    free(msg);
                    exmod->msg = NULL;
                    LOG_D("read data finish");
                    break;
                }
                file_exmod_send_fail(exmod->ser, "invalid data message");
                return 0;
            }
        }
        else
        {
            msg = exmod->msg;
        }

        /* recv file data */
        while ((msg->data.size > 0) && (curr_size < size))
        {
            r = (msg->data.size > SYNC_DATA_MAX) ? SYNC_DATA_MAX : msg->data.size;
            r = r > (size - curr_size) ? (size - curr_size) : r;
            if (!file_exmod_read(exmod->ser, (char *)buffer + curr_size, r, 800))
            {
                file_exmod_send_fail(exmod->ser, "read packet timeout");
                return 0;
            }
            msg->data.size -= r;
            curr_size += r;
        }

        if (msg->data.size > 0)
        {
            exmod->msg = msg;
            break;
        }
        else
        {
            free(msg);
            exmod->msg = NULL;
        }
    }
    return curr_size;
}

bool file_exmod_do_lstat_v1(struct adb_service *ser, struct f_exmod *exmod)
{
    union file_syncmsg msg = {0};

    LOG_D("F:%s L:%d run", __FUNCTION__, __LINE__);
    msg.stat_v1.id = ID_LSTAT_V1;
    msg.stat_v1.mode = 0x000081b6;
    msg.stat_v1.size = 0x00000000;
    msg.stat_v1.time = 0x00000000;

    return file_exmod_write(ser, &msg.stat_v1, sizeof(msg.stat_v1), 50);
}

bool file_exmod_do_list(struct adb_service *ser, struct f_exmod *exmod)
{
    union file_syncmsg msg;
    bool ret = true;

    LOG_D("F:%s L:%d run", __FUNCTION__, __LINE__);
    msg.dent.id = ID_DENT;
    msg.dent.id = ID_DONE;
    msg.dent.mode = 0;
    msg.dent.size = 0;
    msg.dent.time = 0;
    msg.dent.namelen = 0;

    ret = file_exmod_write(ser, &msg.dent, sizeof(msg.dent), 50);

    return ret;
}

bool file_exmod_do_recv(struct adb_service *ser,  struct f_exmod *exmod)
{
    union file_syncmsg msg = { 0 };
    struct f_exmod_handler *handle;

    LOG_D("F:%s L:%d run", __FUNCTION__, __LINE__);

    if (exmod == RT_NULL)
    {
        file_exmod_send_fail(ser, "file exmod is NULL");
        return false;
    }

    handle = file_exmod_find(exmod->name);
    if (handle == RT_NULL)
    {
        file_exmod_send_fail(ser, "not find mod!");
        return false;
    }
    else if (handle->pull == RT_NULL)
    {
        file_exmod_send_fail(ser, "this mod not support pull mode!");
        return false;
    }

    handle->pull(exmod);
    msg.data.id = ID_DONE;
    msg.data.size = 0;
    return file_exmod_write(ser, &msg.data, sizeof(msg.data), 200);
}

bool file_exmod_do_send(struct adb_service *ser, struct f_exmod *exmod)
{
    union file_syncmsg msg;
    struct f_exmod_handler *handle;

    if (exmod == RT_NULL)
    {
        file_exmod_send_fail(ser, "file exmod is NULL");
        return false;
    }

    handle = file_exmod_find(exmod->name);
    if (handle == RT_NULL)
    {
        file_exmod_send_fail(ser, "not find mod!");
        return false;
    }
    else if (handle->push == RT_NULL)
    {
        file_exmod_send_fail(ser, "this mod not support push mode!");
        return false;
    }

    handle->push(exmod);
    msg.status.id = ID_OKAY;
    msg.status.msglen = 0;
    return file_exmod_write(ser, &msg.status, sizeof(msg.status), 100);
}

int file_exmod_register(struct f_exmod_handler *handle)
{
    struct rt_slist_node *node, *head;
    struct f_exmod_handler *entry;

    if ((handle == RT_NULL) || (handle->name == RT_NULL))
    {
        LOG_E("handle or name is null! return");
        return -1;
    }

    head = &_excnd_mod_head;
    for (node = head->next; node != RT_NULL; node = node->next)
    {
        entry = rt_list_entry(node, struct f_exmod_handler, list);
        if (strcmp(handle->name, entry->name) == 0)
        {
            LOG_E("The same mod name has been registered! return");
            return -2;
        }
    }

    rt_slist_init(&handle->list);
    rt_slist_append(head, &handle->list);

    return 0;
}

struct f_exmod_handler *file_exmod_find(const char *name)
{
    struct rt_slist_node *node, *head;
    struct f_exmod_handler *entry, *handler = RT_NULL;

    LOG_D("F:%s L:%d name:%s", __FUNCTION__, __LINE__, name);
    if (name == RT_NULL)
    {
        return RT_NULL;
    }

    head = &_excnd_mod_head;
    for (node = head->next; node != RT_NULL; node = node->next)
    {
        entry = rt_list_entry(node, struct f_exmod_handler, list);
        if (strcmp(name, entry->name) == 0)
        {
            handler = entry;
        }
    }

    return handler;
}

struct f_exmod_handler *file_exmod_unregister(const char *name)
{
    struct f_exmod_handler *handler = RT_NULL;
    struct rt_slist_node *head;

    LOG_D("F:%s L:%d name:%s", __FUNCTION__, __LINE__, name);
    head = &_excnd_mod_head;
    handler = file_exmod_find(name);
    if (handler != RT_NULL)
    {
        rt_slist_remove(head, &handler->list);
    }
    return handler;
}

struct f_exmod *file_exmod_create(const char *name)
{
    const char *str = name, *str_temp;
    char *str_index;
    int str_len, name_len, i;
    struct f_exmod *exmod = RT_NULL;
    char size_str[16];
    char hex_flag = 0;

    LOG_D("F:%s L:%d name:%s", __FUNCTION__, __LINE__, name);
    if (name == RT_NULL)
    {
        return RT_NULL;
    }
    /* String Length Check */
    name_len = strlen(name);
    str_len = strlen(FILE_EXMOD_HEAD_PRE);
    if (name_len <= str_len)
    {
        return RT_NULL;
    }

    /* Find the head of the append command */
    if (strncmp(str, FILE_EXMOD_HEAD_PRE, str_len) != 0)
    {
        return RT_NULL;
    }
    LOG_D("Find the packet header prefix");

    exmod = rt_malloc(sizeof(struct f_exmod));
    if (exmod == RT_NULL)
    {
        LOG_D("malloc f_exmod failed. return");
        return RT_NULL;
    }
    memset(exmod, 0, sizeof(struct f_exmod));

    /* Check for head legitimacy */
    str += str_len;
    str_temp = strstr(str, FILE_EXMOD_HEAD_POS);
    if (str_temp == RT_NULL)
    {
        goto nfound;
    }
    LOG_D("Find the packet header suffix");

    /* Get Name Length And Save Name */
    str_len = str_temp - str;
    if (str_len != 0)
    {
        exmod->name = rt_malloc(str_len + 1);
        if (exmod->name == RT_NULL)
        {
            goto nfound;
        }
        exmod->name[str_len] = '\0';
        strncpy(exmod->name, str, str_len);
    }
    str_len = strlen(FILE_EXMOD_HEAD_POS);
    str = str_temp + str_len;
    LOG_D("get exmod name:%s", exmod->name);

    /* Check file size */
    str_len = strlen(FILE_EXMOD_SIZE_PRE);
    if (strncmp(str, FILE_EXMOD_SIZE_PRE, str_len) != 0)
    {
        goto nfound;
    }
    LOG_D("Find File Size Header Prefix");
    str += str_len;
    str_temp = strstr(str, FILE_EXMOD_SIZE_POS);
    if (str_temp == RT_NULL)
    {
        goto nfound;
    }
    LOG_D("Find File Size Header Suffix");
    if (str_temp != str)
    {
        /* Get file size */
        if ((str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X')))
        {
            hex_flag = RT_TRUE;
            str += 2;
        }
        str_len = str_temp - str;
        if ((str_len) > sizeof(size_str))
        {
            goto nfound;
        }

        for (i = 0; i < str_len; i++)
        {
            if ((*str >= '0') && (*str <= '9'))
            {
                size_str[i] = *str - '0';
            }
            else if ((*str >= 'A') && (*str <= 'F'))
            {
                size_str[i] = 10 + *str - 'A';
                hex_flag = RT_TRUE;
            }
            else if ((*str >= 'a') && (*str <= 'f'))
            {
                size_str[i] = 10 + *str - 'F';
                hex_flag = RT_TRUE;
            }
            else
            {
                goto nfound;
            }
            str ++;
        }

        for (i = 0; i < str_len; i++)
        {
            if (hex_flag == RT_TRUE)
            {
                exmod->file_size = exmod->file_size * 16 + size_str[i];
            }
            else
            {
                exmod->file_size = exmod->file_size * 10 + size_str[i];
            }
        }
        LOG_D("get File Size:%d", exmod->file_size);
    }

    /* Check whether there are parameters */
    str = str_temp + strlen(FILE_EXMOD_SIZE_POS);
    /* No parameters, Copy path */
    if (*str == '/')
    {
        exmod->path = strdup(str);
        LOG_D("No parameters.get path:%s", exmod->path);
        return exmod;
    }

    str_len = strlen(FILE_EXMOD_PARAM_PRE);
    /* Check parameter head */
    if (strncmp(str, FILE_EXMOD_PARAM_PRE, str_len) != 0)
    {
        goto nfound;
    }
    str += str_len;
    LOG_D("find parameter prefix");

    /* Find parameter end address */
    str_temp = strstr(str, FILE_EXMOD_PARAM_POS);
    if (str_temp == RT_NULL)
    {
        goto nfound;
    }
    LOG_D("find parameter Suffix");

    /* parameters head == parameters end, No parameters */
    if (str == str_temp)
    {
        str_temp += strlen(FILE_EXMOD_PARAM_POS);
        if (*str_temp == '/')
        {
            exmod->path = strdup(str_temp);
            LOG_D("The parameter is empty.get path: %s", exmod->path);
            return exmod;
        }
        else
        {
            LOG_E("Illegal path");
            goto nfound;
        }
    }
    /* Get the number of parameters */
    str_index = (char *)str;
    exmod->param_num = 1;
    while ((*str_index != '\0') && (str_index < str_temp))
    {
        if (*str_index == FILE_EXMOD_PARAM_DIV)
        {
            exmod->param_num ++;
        }
        str_index++;
    }
    LOG_D("get parameter num:%d", exmod->param_num);
    /* The parameter region is empty, Copy path and return */
    if (exmod->param_num == 0)
    {
        str = str_temp + strlen(FILE_EXMOD_PARAM_POS);
        if (*str == '/')
        {
            exmod->path = strdup(str);
            LOG_D("The number of parameters is 0. get path:%s", exmod->path);
            return exmod;
        }
        else
        {
            LOG_E("parameters is 0 && Illegal path");
            goto nfound;
        }
    }

    /* Cache parameter region */
    str_len = str_temp - str;
    exmod->param = rt_malloc(str_len + 1);
    if (exmod->param == RT_NULL)
    {
        LOG_E("malloc parameter cache buff failed");
        goto nfound;
    }
    exmod->param[str_len] = '\0';
    strncpy(exmod->param, str, str_len);

    exmod->param_res = rt_malloc(exmod->param_num * sizeof(struct f_param));
    if(exmod->param_res == RT_NULL)
    {
        LOG_E("malloc Parameter descriptor failed");
        goto nfound;
    }
    memset(exmod->param_res, 0, exmod->param_num * sizeof(struct f_param));

    /* Get parameters */
    str_index = exmod->param;
    for (i = 0; (i < exmod->param_num) && (*str_index != '\0'); i++)
    {
        if (*str_index == FILE_EXMOD_PARAM_DIV)
        {
            exmod->param_res[i].key = RT_NULL;
            str_index ++;
            continue;
        }
        else
        {
            exmod->param_res[i].key = str_index;
        }
        while (*str_index != '\0')
        {
            if (*str_index == FILE_EXMOD_KEY_DIV)
            {
                *str_index = '\0';
                exmod->param_res[i].value = str_index + 1;
            }
            else if (*str_index == FILE_EXMOD_PARAM_DIV)
            {
                *str_index = '\0';
                str_index++;
                break;
            }
            str_index++;
        }
    }
#ifdef DBG_ENABLE
    for (i = 0; i < exmod->param_num; i++)
    {
        LOG_D("param_res[i] %s=%s", exmod->param_res[i].key, exmod->param_res[i].value);
    }
#endif
    /* get path */
    str = str_temp + strlen(FILE_EXMOD_PARAM_POS);
    exmod->path = strdup(str);
    if (exmod->path == RT_NULL)
    {
        goto nfound;
    }
    LOG_D("get file path:%s", exmod->path);
    return exmod;

nfound:
    if (exmod->name != RT_NULL)
    {
        rt_free(exmod->name);
    }
    if (exmod->param != RT_NULL)
    {
        rt_free(exmod->param);
    }
    if (exmod->path != RT_NULL)
    {
        rt_free(exmod->path);
    }
    if (exmod->param_res != RT_NULL)
    {
        rt_free(exmod->param_res);
    }
    if (exmod != RT_NULL)
    {
        rt_free(exmod);
    }
    return RT_NULL;
}

void file_exmod_delete(struct f_exmod *exmod)
{
    if (exmod == RT_NULL)
    {
        return;
    }
    if (exmod->name != RT_NULL)
    {
        rt_free(exmod->name);
    }
    if (exmod->param != RT_NULL)
    {
        rt_free(exmod->param);
    }
    if (exmod->path != RT_NULL)
    {
        rt_free(exmod->path);
    }
    if (exmod->param_res != RT_NULL)
    {
        rt_free(exmod->param_res);
    }
    if (exmod != RT_NULL)
    {
        rt_free(exmod);
    }
}
#endif
