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
 * 2018-12-22     tyx          the first version
 */
#include <rtthread.h>
#include "file_exmod.h"
#include <dfs_fs.h>
#include <dfs_posix.h>
#include "cJSON.h"
#include <tiny_md5.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME  "sync_mod"
#define DBG_LEVEL         DBG_INFO
#define DBG_COLOR
#include <rtdbg.h>

#define FILE_SYNC_MOD_NAME              ("sync_mod")
#define FILE_SYNC_PATH_NAME_MAX         (512)
#define FILE_SYNC_TRANSFER_BUFF_MAX     (512)

#if defined(ADB_EXTERNAL_MOD_ENABLE) && defined(ADB_FILESYNC_MOD_ENABLE)

static struct f_exmod *current_exmod;

struct dir_des
{
    rt_slist_t list;
    DIR *dir;
};

struct json_tranf
{
    bool start_flag;
    int data_len;
    int buff_len;
    char *buff;
};

static bool send_ram_data(void *buff, int len)
{
    if (file_exmod_write_data(current_exmod, buff, len) != len)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/* This function dynamically constructs json arrays */
static struct json_tranf *json_to_str_dynamic(struct json_tranf *tranf, cJSON *json)
{
    char *out_str;
    int str_len, send_len, i;

    /* if tranf is null.Start transmission */
    if (tranf == NULL)
    {
        tranf = malloc(sizeof(struct json_tranf) + FILE_SYNC_TRANSFER_BUFF_MAX);
        if (tranf == NULL)
        {
            LOG_D("malloc json str buff failed");
            return NULL;
        }
        memset(tranf, 0, sizeof(struct json_tranf));
        tranf->buff = (char *)&tranf[1];
        tranf->buff_len = FILE_SYNC_TRANSFER_BUFF_MAX;
        tranf->start_flag = false;
        LOG_D("send json str begin");
    }

    /* if json is null. Stop transmission */
    if (json == NULL)
    {
        /* Splicing Array End Character */
        if (tranf->data_len < FILE_SYNC_TRANSFER_BUFF_MAX)
        {
            tranf->buff[tranf->data_len] = ']';
            tranf->data_len += 1;
            /* send all data */
            send_ram_data(tranf->buff, tranf->data_len);
        }
        else
        {
            send_ram_data(tranf->buff, tranf->data_len);
            tranf->data_len = 0;
            tranf->buff[tranf->data_len] = ']';
            tranf->data_len += 1;
            send_ram_data(tranf->buff, tranf->data_len);
        }
        LOG_D("send json str stop");
        free(tranf);
        return NULL;
    }

    out_str = cJSON_PrintUnformatted(json);
    str_len = strlen(out_str);
    LOG_D("json to str[%d]:%s", str_len, out_str);
    if (out_str != NULL)
    {
        if (tranf->start_flag == false)
        {
            /* First transmission. Send Array begin Character */
            tranf->buff[0] = '[';
            tranf->data_len = 1;
            tranf->start_flag = true;
        }
        else
        {
            /* Send Array interval Character */
            tranf->buff[tranf->data_len] = ',';
            tranf->data_len += 1;
        }
        /* send data */
        i = 0;
        while(i < str_len)
        {
            /* Waiting for str BUFF Full */
            send_len = tranf->buff_len - tranf->data_len  > str_len - i ? str_len - i : tranf->buff_len - tranf->data_len;
            memcpy(&tranf->buff[tranf->data_len], &out_str[i], send_len);
            i += send_len;
            tranf->data_len += send_len;
            /* if buff full.Send a packet of data */
            if (tranf->data_len == FILE_SYNC_TRANSFER_BUFF_MAX)
            {
                if (send_ram_data(tranf->buff, tranf->data_len) != true)
                {
                    free(tranf);
                    return NULL;
                }
                tranf->data_len = 0;
            }
        }
        free(out_str);
    }

    return tranf;
}

static cJSON *dir_to_json(const char *pathname, const char *md5_str)
{
    cJSON *json_obj;

    if (pathname == NULL)
    {
        return NULL;
    }
    json_obj = cJSON_CreateObject();
    if (json_obj == NULL)
    {
        LOG_E("create cjson failed\n");
        return NULL;
    }
    cJSON_AddItemToObject(json_obj, "name", cJSON_CreateString(pathname));
    if (md5_str != NULL)
    {
        cJSON_AddItemToObject(json_obj, "md5", cJSON_CreateString(md5_str));
    }
    else
    {
        cJSON_AddItemToObject(json_obj, "md5", cJSON_CreateString(""));
    }
    return json_obj;
}

static void md5_str(uint8_t md5[16], char str[32])
{
    int i = 0,j = 0;
    uint8_t temp;

    for (i = 0, j = 0; i < 16; i ++, j += 2)
    {
        temp = md5[i] / 16;
        str[j] = temp >= 10 ? (temp - 10) + 'a' : temp + '0';
        temp = md5[i] % 16;
        str[j + 1] = temp >= 10 ? (temp - 10) + 'a' : temp + '0';
    }
}

static void calcmd5(char* spec, void *buffer, int size, uint8_t value[16])
{
    tiny_md5_context c;
    int fd;

    if (buffer == RT_NULL)
    {
        return;
    }
    fd = open(spec, O_RDONLY, 0);
    if (fd < 0)
    {
        return;
    }

    tiny_md5_starts(&c);
    while (1)
    {
        int len = read(fd, buffer, size);
        if (len < 0)
        {
            close(fd);
            return;
        }
        else if (len == 0)
            break;

        tiny_md5_update(&c, (unsigned char*)buffer, len);
    }
    tiny_md5_finish(&c, value);
    close(fd);
}

static struct rt_slist_node _dir_head;

rt_inline int push_dir(DIR *dir)
{
    struct rt_slist_node *head;
    struct dir_des *insert_dir;

    head = &_dir_head;
    insert_dir = malloc(sizeof(struct dir_des));
    if (insert_dir == NULL)
    {
        return -1;
    }
    insert_dir->dir = dir;
    rt_slist_init(&insert_dir->list);
    rt_slist_insert(head, &insert_dir->list);
    return 0;
}

rt_inline DIR *pop_dir(void)
{
    struct rt_slist_node *head, *node;
    struct dir_des *entry;
    DIR *dir_temp;

    head = &_dir_head;
    node = rt_slist_first(head);
    if (node == NULL)
    {
        return NULL;
    }
    entry = rt_slist_entry(node, struct dir_des, list);
    rt_slist_remove(head, &entry->list);
    dir_temp = entry->dir;
    free(entry);
    return dir_temp;
}

rt_inline int path_name_check(char *pathname)
{
    int path_len = 0;

    path_len = strlen(pathname);
    if ((path_len > 0) && (pathname[path_len - 1] != '/'))
    {
        pathname[path_len] = '/';
        pathname[path_len + 1] = '\0';
        path_len += 1;
    }
    return path_len;
}

static int file_to_json_loop_through(const char *pathname)
{
    DIR *curr_dir = NULL;
    struct dirent * dir = NULL;
    char *fullpath;
    struct stat filestat;
    int pathname_len, fullpath_len;
    void *md5_buff;
    struct dfs_fd *d;
    cJSON *json_obj;
    struct json_tranf *tranf = NULL;
    int send_count = 0;

    LOG_D("F:%s L:%d is run", __FUNCTION__, __LINE__);
    if (stat(pathname, &filestat) == -1)
    {
        LOG_E("path does not exist:%s", pathname);
    }
    pathname_len = strlen(pathname);
    md5_buff = malloc(512);
    if (md5_buff == RT_NULL)
    {
        LOG_E("md5 check mem malloc failed!!");
        return send_count;
    }
    if (!S_ISDIR(filestat.st_mode))
    {
        free(md5_buff);
        LOG_E("this path[%s] is not Folder!", pathname);
        return send_count;
    }
    fullpath = malloc(FILE_SYNC_PATH_NAME_MAX);
    if (fullpath == NULL)
    {
        LOG_E("full path mem malloc failed!!");
        free(md5_buff);
        return send_count;
    }
    strncpy(fullpath, pathname, FILE_SYNC_PATH_NAME_MAX);
    fullpath_len = path_name_check(fullpath);
    LOG_D("open dir:%s", fullpath);
    curr_dir = opendir(fullpath);
    if (curr_dir == NULL)
    {
        LOG_D("open dir failed");
        free(md5_buff);
        free(fullpath);
        return send_count;
    }

    while (1)
    {
        if (curr_dir == NULL)
        {
            LOG_D("pop dir");
            curr_dir = pop_dir();
            if (curr_dir == NULL)
            {
                LOG_D("pop dir is null");
                break;
            }
            d = fd_get(curr_dir->fd);
            RT_ASSERT (d != NULL);
            strncpy(fullpath, d->path, FILE_SYNC_PATH_NAME_MAX);
            fullpath_len = path_name_check(fullpath);
            fd_put(d);
            LOG_D("current path:%s", fullpath);
        }
        LOG_D("read dir");
        dir = readdir(curr_dir);
        if(dir == NULL)
        {
            LOG_D("read dir is null. close dir");
            closedir(curr_dir);
            curr_dir = NULL;
            continue;
        }
        RT_ASSERT((fullpath_len + strlen(dir->d_name)) < FILE_SYNC_PATH_NAME_MAX);
        strcpy(&fullpath[fullpath_len], dir->d_name);
        if (stat(fullpath, &filestat) == -1)
        {
            LOG_E("cannot access the file %s", fullpath);
            closedir(curr_dir);
            curr_dir = NULL;
            continue;
        }
        if (S_ISDIR(filestat.st_mode))
        {
            LOG_D("find dir.name:%s", fullpath);
            if (push_dir(curr_dir) == 0)
            {
                fullpath_len = path_name_check(fullpath);
                LOG_D("push current dir. open %s dir", fullpath);
                curr_dir = opendir(fullpath);
            }
            LOG_D("create dir name to json");
            json_obj = dir_to_json(&fullpath[pathname_len], NULL);
            if (json_obj != NULL)
            {
                LOG_D("send json");
                tranf = json_to_str_dynamic(tranf, json_obj);
                send_count ++;
                cJSON_Delete(json_obj);
                if (tranf == NULL)
                {
                    LOG_E("F:%s L:%d send data err!", __FUNCTION__, __LINE__);
                    while (curr_dir != NULL)
                    {
                        closedir(curr_dir);
                        curr_dir = pop_dir();
                    }
                    break;
                }
            }
            continue;
        }
        else
        {
            uint8_t md5[16];
            char str[33];
            LOG_D("find file.name:%s", fullpath);
            calcmd5(fullpath, md5_buff, 512, md5);
            md5_str(md5, str);
            str[32] = '\0';
            LOG_D("file md5:%s", str);
            LOG_D("create file name to json");
            json_obj = dir_to_json(&fullpath[pathname_len], str);
            if (json_obj != NULL)
            {
                LOG_D("send json");
                tranf = json_to_str_dynamic(tranf, json_obj);
                send_count ++;
                cJSON_Delete(json_obj);
                if (tranf == NULL)
                {
                    LOG_E("F:%s L:%d send data err!", __FUNCTION__, __LINE__);
                    while (curr_dir != NULL)
                    {
                        closedir(curr_dir);
                        curr_dir = pop_dir();
                    }
                    break;
                }
            }
        }
    }
    LOG_D("send json count:%d", send_count);
    if (send_count > 0)
    {
        json_to_str_dynamic(tranf, NULL);
    }
    free(md5_buff);
    free(fullpath);
    return send_count;
}

static void file_delete_loop_through(const char *pathname)
{
    DIR *curr_dir = NULL;
    struct dirent * dir = NULL;
    char *fullpath;
    struct stat filestat;
    int fullpath_len;
    struct dfs_fd *d;

    if ((pathname == NULL) || (stat(pathname, &filestat) == -1))
    {
        LOG_W("File path not found!");
        return;
    }
    if (S_ISREG(filestat.st_mode))
    {
        LOG_D("delete file:%s", pathname);
        dfs_file_unlink(pathname);
        return;
    }

    fullpath = malloc(FILE_SYNC_PATH_NAME_MAX);
    if (fullpath == NULL)
    {
        LOG_E("full path mem malloc failed!!");
        return;
    }
    strncpy(fullpath, pathname, FILE_SYNC_PATH_NAME_MAX);
    fullpath_len = path_name_check(fullpath);
    curr_dir = opendir(fullpath);
    if (curr_dir == NULL)
    {
        LOG_E("open dir failed! return");
        free(fullpath);
        return;
    }

    while (1)
    {
        if (curr_dir == NULL)
        {
            LOG_D("pop dir");
            curr_dir = pop_dir();
            if (curr_dir == NULL)
            {
                break;
            }
            d = fd_get(curr_dir->fd);
            RT_ASSERT (d != NULL);
            strncpy(fullpath, d->path, FILE_SYNC_PATH_NAME_MAX);
            fullpath_len = path_name_check(fullpath);
            fd_put(d);
            LOG_D("get current file path:%s", fullpath);
        }
        dir = readdir(curr_dir);
        if(dir == NULL)
        {
            d = fd_get(curr_dir->fd);
            RT_ASSERT (d != NULL);
            strncpy(fullpath, d->path, FILE_SYNC_PATH_NAME_MAX);
            fullpath_len = path_name_check(fullpath);
            fd_put(d);
            closedir(curr_dir);
            LOG_D("Delete folder:%s", fullpath);
            dfs_file_unlink(fullpath);
            curr_dir = NULL;
            continue;
        }
        RT_ASSERT((fullpath_len + strlen(dir->d_name)) < FILE_SYNC_PATH_NAME_MAX);
        strcpy(&fullpath[fullpath_len], dir->d_name);
        if (stat(fullpath, &filestat) == -1)
        {
            LOG_E("cannot access the file %s", fullpath);
            closedir(curr_dir);
            curr_dir = NULL;
            continue;
        }
        if (S_ISDIR(filestat.st_mode))
        {
            LOG_D("Find a folder. :%s", fullpath);
            if (push_dir(curr_dir) == 0)
            {
                fullpath_len = path_name_check(fullpath);
                LOG_D("push current path. open file path:%s", fullpath);
                curr_dir = opendir(fullpath);
            }
            continue;
        }
        else
        {
            LOG_D("Delete file:%s", fullpath);
            dfs_file_unlink(fullpath);
        }
    }
    free(fullpath);
}

static void file_delete_from_json(const char *path, cJSON *json)
{
    int json_size, i;
    cJSON *temp;
    char *fullpath, *filename = NULL;

    LOG_D("F:%s L:%d is run", __FUNCTION__, __LINE__);
    if (json->type != cJSON_Array)
    {
        LOG_W("json type are not arrays. return");
        return;
    }
    fullpath = malloc(FILE_SYNC_PATH_NAME_MAX);
    if (fullpath == NULL)
    {
        LOG_D("full path malloc failed. return");
        return;
    }
    json_size = cJSON_GetArraySize(json);
    LOG_D("json array size:%d", json_size);
    for (i = 0; i < json_size; i++)
    {
        temp = cJSON_GetArrayItem(json, i);
        if (temp != NULL)
        {
            filename = temp->valuestring;
        }
        if (filename != RT_NULL)
        {
            sprintf(fullpath, "%s%s", path, filename);
            LOG_D("delete %s", fullpath);
            file_delete_loop_through(fullpath);
        }
        filename = NULL;
    }
    free(fullpath);
}

static int file_sync_push(struct f_exmod *exmod)
{
    cJSON *dir_json;
    char *path = NULL, *buff;
    int i, r_size;

    LOG_D("F:%s L:%d is run", __FUNCTION__, __LINE__);
    for (i = 0; i < exmod->param_num; i++)
    {
        if (rt_strcmp(exmod->param_res[i].key, "path") == 0)
        {
            path = exmod->param_res[i].value;
        }
    }
    if (path == NULL)
    {
        LOG_E("No synchronization path was found! return");
        return -1;
    }
    LOG_D("local path:%s", path);
    buff = malloc(512);
    if (buff == NULL)
    {
        LOG_E("malloc read buff err. return");
        return -1;
    }

    while (1)
    {
        r_size = file_exmod_read_data(exmod, buff, 512);
        LOG_D("recv data. len:%d", r_size);
        if (r_size > 0)
        {
            dir_json = cJSON_Parse(buff);
            file_delete_from_json(path, dir_json);
            cJSON_Delete(dir_json);
        }
        if (r_size != 512)
        {
            break;
        }
    }
    free(buff);
    return 0;
}

static int file_sync_pull(struct f_exmod *exmod)
{
    char *path = NULL;
    int i, count;

    LOG_D("F:%s L:%d is run", __FUNCTION__, __LINE__);
    for (i = 0; i < exmod->param_num; i++)
    {
        if (rt_strcmp(exmod->param_res[i].key, "path") == 0)
        {
            path = exmod->param_res[i].value;
        }
    }
    if (path == NULL)
    {
        LOG_E("No synchronization path was found! return");
        return -1;
    }
    LOG_D("local path:%s", path);
    current_exmod = exmod;
    LOG_D("Do file name to json!")
    count = file_to_json_loop_through(path);
    if (count == 0)
    {
        char buff[2] = {'[',']'};
        LOG_D("Empty folder!")
        file_exmod_write_data(current_exmod, buff, sizeof(buff));
    }
    current_exmod = NULL;

    return 0;
}

void file_sync_mod_uninit(void)
{
    struct f_exmod_handler *handle;

    LOG_D("F:%s L:%d is run", __FUNCTION__, __LINE__);
    handle = file_exmod_unregister(FILE_SYNC_MOD_NAME);
    rt_free(handle);
}

int file_sync_mod_init(void)
{
    struct f_exmod_handler *handle;

    LOG_D("F:%s L:%d is run", __FUNCTION__, __LINE__);
    handle = rt_malloc(sizeof(struct f_exmod_handler));
    if (handle == RT_NULL)
    {
        return -1;
    }
    handle->name = FILE_SYNC_MOD_NAME;
    handle->push = file_sync_push;
    handle->pull = file_sync_pull;
    LOG_D("sync mod init! mod name:%s", FILE_SYNC_MOD_NAME);
    file_exmod_register(handle);

    return 0;
}
INIT_APP_EXPORT(file_sync_mod_init);

#endif
