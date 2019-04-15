/*
 * Copyright (c) 2019, Real-Thread Information Technology Ltd
 * All rights reserved
 *
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-02-27     tyx          the first version
 */

#include <rtthread.h>
#include "file_exmod.h"
#include <dfs_fs.h>
#include <dfs_posix.h>
#include "cJSON.h"

#ifdef ADB_FILELIST_SUP_MD5
#include <tiny_md5.h>
#endif

#define DBG_ENABLE
#define DBG_SECTION_NAME  "list_mod"
#define DBG_LEVEL         DBG_INFO
#define DBG_COLOR
#include <rtdbg.h>

#define FILE_LIST_MOD_NAME              ("list_mod")
#define FILE_LIST_PATH_NAME_MAX         (512)
#define FILE_LIST_TRANSFER_BUFF_MAX     (512)

#if defined(ADB_EXTERNAL_MOD_ENABLE) && defined(ADB_FILELIST_MOD_ENABLE)
static struct f_exmod *current_exmod;

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
struct json_tranf *json_to_str_dynamic(struct json_tranf *tranf, cJSON *json)
{
    char *out_str;
    int str_len, send_len, i;

    /* if tranf is null.Start transmission */
    if (tranf == NULL)
    {
        tranf = malloc(sizeof(struct json_tranf) + FILE_LIST_TRANSFER_BUFF_MAX);
        if (tranf == NULL)
        {
            LOG_D("malloc json str buff failed");
            return NULL;
        }
        memset(tranf, 0, sizeof(struct json_tranf));
        tranf->buff = (char *)&tranf[1];
        tranf->buff_len = FILE_LIST_TRANSFER_BUFF_MAX;
        tranf->start_flag = false;
        LOG_D("send json str begin");
    }

    /* if json is null. Stop transmission */
    if (json == NULL)
    {
        /* Splicing Array End Character */
        if (tranf->data_len < FILE_LIST_TRANSFER_BUFF_MAX)
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
            if (tranf->data_len == FILE_LIST_TRANSFER_BUFF_MAX)
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

static cJSON *json_add_item(cJSON *json_obj, const char *name, const char *value)
{
    if (name == NULL)
    {
        return json_obj;
    }
    if (json_obj == NULL)
    {
        json_obj = cJSON_CreateObject();
        if (json_obj == NULL)
        {
            LOG_E("create cjson failed\n");
            return NULL;
        }
    }
    cJSON_AddItemToObject(json_obj, name, cJSON_CreateString(value));
    return json_obj;
}

#ifdef ADB_FILELIST_SUP_MD5
static int file_calcmd5(char* filename, void *buffer, int size, uint8_t value[16], char str[32])
{
    tiny_md5_context c;
    int fd;
    int i = 0,j = 0;
    uint8_t temp;
    struct stat filestat;

    /* Check whether the file exists */
    if (stat(filename, &filestat) == -1)
    {
        return -1;
    }
    /* Check if it is a folder? */
    if (S_ISDIR(filestat.st_mode))
    {
        rt_memset(value, 0, 16);
        rt_memset(str, 0, 32);
        return 0;
    }

    /* read file and calculation md5 */
    fd = open(filename, O_RDONLY, 0);
    if (fd < 0)
    {
        return -1;
    }
    tiny_md5_starts(&c);
    while (1)
    {
        int len = read(fd, buffer, size);
        if (len < 0)
        {
            close(fd);
            return -1;
        }
        else if (len == 0)
        {
            break;
        }
        tiny_md5_update(&c, (unsigned char*)buffer, len);
    }
    tiny_md5_finish(&c, value);
    close(fd);

    /* MD5 value converted to string  */
    for (i = 0, j = 0; i < 16; i ++, j += 2)
    {
        temp = value[i] / 16;
        str[j] = temp >= 10 ? (temp - 10) + 'a' : temp + '0';
        temp = value[i] % 16;
        str[j + 1] = temp >= 10 ? (temp - 10) + 'a' : temp + '0';
    }
    return 0;
}
#endif

/* Check whether the string ends with a '/' symbol */
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
    int fullpath_len;
    void *temp_buff;
    cJSON *json_obj;
    struct json_tranf *tranf = NULL;
    int send_count = 0;

#define TEMP_BUFF_SIZE  (512)
    LOG_D("F:%s L:%d is run", __FUNCTION__, __LINE__);
    /* Check whether the file exists */
    if (stat(pathname, &filestat) == -1)
    {
        LOG_E("path does not exist:%s", pathname);
        return send_count;
    }
    temp_buff = malloc(TEMP_BUFF_SIZE);
    if (temp_buff == RT_NULL)
    {
        LOG_E("md5 check mem malloc failed!!");
        return send_count;
    }
    /* Check if it is a folder? */
    if (!S_ISDIR(filestat.st_mode))
    {
        free(temp_buff);
        LOG_E("this path[%s] is not Folder!", pathname);
        return send_count;
    }
    fullpath = malloc(FILE_LIST_PATH_NAME_MAX);
    if (fullpath == NULL)
    {
        LOG_E("full path mem malloc failed!!");
        free(temp_buff);
        return send_count;
    }
    /* Copy path */
    strncpy(fullpath, pathname, FILE_LIST_PATH_NAME_MAX);
    /* Check path name and get str len */
    fullpath_len = path_name_check(fullpath);
    LOG_D("open dir:%s", fullpath);
    /* open dir */
    curr_dir = opendir(fullpath);
    if (curr_dir == NULL)
    {
        LOG_D("open dir failed");
        free(temp_buff);
        free(fullpath);
        return send_count;
    }

    while (1)
    {
        LOG_D("read dir");
        dir = readdir(curr_dir);
        if(dir == NULL)
        {
            LOG_D("read dir is null. close dir");
            break;
        }
        RT_ASSERT((fullpath_len + strlen(dir->d_name)) < FILE_LIST_PATH_NAME_MAX);
        /* Get the full path */
        strcpy(&fullpath[fullpath_len], dir->d_name);
        /* Get file attributes */
        if (stat(fullpath, &filestat) == -1)
        {
            LOG_E("cannot access the file %s", fullpath);
            continue;
        }

#ifdef ADB_FILELIST_FULL_PATH
        json_obj = json_add_item(NULL, "name", fullpath);
#else
        json_obj = json_add_item(NULL, "name", dir->d_name);
#endif
        if (json_obj != NULL)
        {
#ifdef ADB_FILELIST_SUP_DEV
            snprintf(temp_buff, TEMP_BUFF_SIZE, "0x%x", filestat.st_dev);
            json_obj = json_add_item(json_obj, "dev", temp_buff);
#endif

#ifdef ADB_FILELIST_SUP_INO
            snprintf(temp_buff, TEMP_BUFF_SIZE, "0x%x", filestat.st_ino);
            json_obj = json_add_item(json_obj, "ino", temp_buff);
#endif

#ifdef ADB_FILELIST_SUP_MODE
            snprintf(temp_buff, TEMP_BUFF_SIZE, "0x%lx", filestat.st_mode);
            json_obj = json_add_item(json_obj, "mode", temp_buff);
#endif

#ifdef ADB_FILELIST_SUP_NLINK
            snprintf(temp_buff, TEMP_BUFF_SIZE, "0x%x", filestat.st_nlink);
            json_obj = json_add_item(json_obj, "nlink", temp_buff);
#endif
#ifdef ADB_FILELIST_SUP_UID
            snprintf(temp_buff, TEMP_BUFF_SIZE, "0x%x", filestat.st_uid);
            json_obj = json_add_item(json_obj, "uid", temp_buff);
#endif
#ifdef ADB_FILELIST_SUP_GID
            snprintf(temp_buff, TEMP_BUFF_SIZE, "0x%x", filestat.st_gid);
            json_obj = json_add_item(json_obj, "gid", temp_buff);
#endif
#ifdef ADB_FILELIST_SUP_RDEV
            snprintf(temp_buff, TEMP_BUFF_SIZE, "0x%x", filestat.st_rdev);
            json_obj = json_add_item(json_obj, "rdev", temp_buff);
#endif
#ifdef ADB_FILELIST_SUP_SIZE
            snprintf(temp_buff, TEMP_BUFF_SIZE, "0x%lx", filestat.st_size);
            json_obj = json_add_item(json_obj, "size", temp_buff);
#endif
#ifdef ADB_FILELIST_SUP_BLKSIZE
            snprintf(temp_buff, TEMP_BUFF_SIZE, "0x%lx", filestat.st_blksize);
            json_obj = json_add_item(json_obj, "blksize", temp_buff);
#endif
#ifdef ADB_FILELIST_SUP_BLOCKS
            snprintf(temp_buff, TEMP_BUFF_SIZE, "0x%lx", filestat.st_blocks);
            json_obj = json_add_item(json_obj, "blocks", temp_buff);
#endif
#ifdef ADB_FILELIST_SUP_ATIME
            snprintf(temp_buff, TEMP_BUFF_SIZE, "0x%lx", filestat.st_atime);
            json_obj = json_add_item(json_obj, "atime", temp_buff);
#endif
#ifdef ADB_FILELIST_SUP_MTIME
            snprintf(temp_buff, TEMP_BUFF_SIZE, "0x%lx", filestat.st_mtime);
            json_obj = json_add_item(json_obj, "mtime", temp_buff);
#endif
#ifdef ADB_FILELIST_SUP_CTIME
            snprintf(temp_buff, TEMP_BUFF_SIZE, "0x%lx", filestat.st_ctime);
            json_obj = json_add_item(json_obj, "ctime", temp_buff);
#endif
#ifdef ADB_FILELIST_SUP_MD5
            {
                uint8_t md5_value[16];
                char md5_str[33];
                if (file_calcmd5(fullpath, temp_buff, TEMP_BUFF_SIZE, md5_value, md5_str) == 0)
                {
                    md5_str[32] = '\0';
                    json_obj = json_add_item(json_obj, "md5", md5_str);
                }
                else
                {
                    json_obj = json_add_item(json_obj, "md5", "err");
                }
            }
#endif
            LOG_D("send json");
            /* json to str and send str */
            tranf = json_to_str_dynamic(tranf, json_obj);
            send_count ++;
            cJSON_Delete(json_obj);
            json_obj = NULL;
            if (tranf == NULL)
            {
                LOG_E("F:%s L:%d send data err!", __FUNCTION__, __LINE__);
                break;
            }
        }
    }
    closedir(curr_dir);
    LOG_D("send json count:%d", send_count);
    if (send_count > 0)
    {
        json_to_str_dynamic(tranf, NULL);
    }
    free(temp_buff);
    free(fullpath);
    return send_count;
}

int file_list_pull(struct f_exmod *exmod)
{
    char *path = NULL;
    int i, count;

    LOG_D("F:%s L:%d is run", __FUNCTION__, __LINE__);
    /* Get path */
    for (i = 0; i < exmod->param_num; i++)
    {
        if (rt_strcmp(exmod->param_res[i].key, "path") == 0)
        {
            path = exmod->param_res[i].value;
        }
    }
    if (path == NULL)
    {
        LOG_E("No listhronization path was found! return");
        return -1;
    }
    LOG_D("local path:%s", path);
    current_exmod = exmod;
    LOG_D("Do file name to json!");
    /* Loop to get file information */
    count = file_to_json_loop_through(path);
    if (count == 0)
    {
        char buff[2] = {'[',']'};
        LOG_D("Empty folder!");
        file_exmod_write_data(current_exmod, buff, sizeof(buff));
    }
    current_exmod = NULL;

    return 0;
}

void file_list_mod_uninit(void)
{
    struct f_exmod_handler *handle;

    LOG_D("F:%s L:%d is run", __FUNCTION__, __LINE__);
    handle = file_exmod_unregister(FILE_LIST_MOD_NAME);
    rt_free(handle);
}

int file_list_mod_init(void)
{
    struct f_exmod_handler *handle;

    LOG_D("F:%s L:%d is run", __FUNCTION__, __LINE__);
    handle = rt_malloc(sizeof(struct f_exmod_handler));
    if (handle == RT_NULL)
    {
        return -1;
    }
    handle->name = FILE_LIST_MOD_NAME;
    handle->push = NULL;
    handle->pull = file_list_pull;
    LOG_D("list mod init! mod name:%s", FILE_LIST_MOD_NAME);
    file_exmod_register(handle);

    return 0;
}
INIT_APP_EXPORT(file_list_mod_init);

#endif
