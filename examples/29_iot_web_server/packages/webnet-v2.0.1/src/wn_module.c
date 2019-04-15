/*
 * File      : wn_module.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 * This software is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. For the terms of this
 * license, see <http://www.gnu.org/licenses/>.
 *
 * You are free to use this software under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * Alternatively for commercial application, you can contact us
 * by email <business@rt-thread.com> for commercial license.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-08-02     Bernard      the first version
 * 2012-06-25     Bernard      add SSI and Upload module
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <webnet.h>
#include <wn_module.h>
#include <wn_utils.h>

#ifdef RT_USING_DFS
#include <dfs_posix.h>
#endif

static int _webnet_module_system_init(struct webnet_session *session, int event)
{
#ifdef WEBNET_USING_LOG
    webnet_module_log(session, event);
#endif

#ifdef WEBNET_USING_SSL
    webnet_module_ssl(session, event);
#endif

#ifdef WEBNET_USING_CGI
    webnet_module_cgi(session, event);
#endif

#ifdef WEBNET_USING_DMR
    webnet_module_dmr(session, event);
#endif

    return WEBNET_MODULE_CONTINUE;
}

static int _webnet_module_system_uri_physical(struct webnet_session *session, int event)
{
    int result;
    result = WEBNET_MODULE_CONTINUE;

#ifdef WEBNET_USING_LOG
    webnet_module_log(session, event);
#endif

#ifdef WEBNET_USING_ALIAS
    result = webnet_module_alias(session, event);
    if (result == WEBNET_MODULE_FINISHED) return result;
#endif

#ifdef WEBNET_USING_AUTH
    result = webnet_module_auth(session, event);
    if (result == WEBNET_MODULE_FINISHED) return result;
#endif

#ifdef WEBNET_USING_CGI
    result = webnet_module_cgi(session, event);
    if (result == WEBNET_MODULE_FINISHED) return result;
#endif

#ifdef WEBNET_USING_DMR
    result = webnet_module_dmr(session, event);
    if (result == WEBNET_MODULE_FINISHED) return result;
#endif

#ifdef WEBNET_USING_UPLOAD
    result = webnet_module_upload(session, event);
    if (result == WEBNET_MODULE_FINISHED) return result;
#endif

    return result;
}

static void _webnet_dofile_handle(struct webnet_session *session, int event)
{
    int fd = session->user_data;

    if (event & WEBNET_EVENT_WRITE)
    {
        rt_size_t readbytes;
        rt_size_t length = RT_ALIGN_DOWN(WEBNET_SESSION_BUFSZ, 4);
#ifdef WEBNET_USING_RANGE
        if(session->request->Range)
        {
            length = session->request->pos_end - session->request->pos_start;
            if(length == 0)
            {
                goto __exit;
            }
       
            if(length > WEBNET_SESSION_BUFSZ)
            {
                length = WEBNET_SESSION_BUFSZ;
            }
           lseek(fd, session->request->pos_start, SEEK_SET);
           session->request->pos_start += length;
        }
#endif            
        readbytes = read(fd, session->buffer, length);
        if (readbytes <= 0) /* end of file */
            goto __exit;

        if (webnet_session_write(session, session->buffer, readbytes) == 0)
            goto __exit;
        return;
    }

__exit:
    close(fd);
    session->user_data = 0;
    session->session_event_mask = 0; /* clean event */
    /* destroy session */
    session->session_phase = WEB_PHASE_CLOSE;

    return;
}

static const struct webnet_session_ops _dofile_ops =
{
    _webnet_dofile_handle,
    RT_NULL
};

/* send a file to http client */
int webnet_module_system_dofile(struct webnet_session *session)
{
    int fd = -1;    /* file descriptor */
    struct stat file_stat;
    const char *mimetype;
    rt_size_t file_length;
    struct webnet_request *request;

#if WEBNET_CACHE_LEVEL > 0
    char ctime_str[32];
    int stat_result = -1;
#endif /* WEBNET_CACHE_LEVEL */

    RT_ASSERT(session != RT_NULL);
    request = session->request;
    RT_ASSERT(request != RT_NULL);

#if WEBNET_CACHE_LEVEL > 0
#ifdef WEBNET_USING_GZIP
    /* get .gz Last-Modified. */
    if (request->support_gzip)
    {
        struct stat file_stat;

        char *path_gz = wn_malloc(strlen(request->path) + 4);  /* ".gz\0" */

        if (path_gz != RT_NULL)
        {
            sprintf(path_gz, "%s.gz", request->path);
            stat_result = stat(request->path, &file_stat);
            wn_free(path_gz);
        }

        if (stat_result == 0)
        {
            rt_enter_critical();
            strcpy(ctime_str, ctime((time_t *)&file_stat.st_mtime));
            rt_exit_critical();

            ctime_str[strlen(ctime_str) - 1] = '\0'; /* clear the end \n */

            if ((request->modified != RT_NULL)
                    && (strcmp(request->modified, ctime_str) == 0))
            {
                request->result_code = 304;
                return WEBNET_MODULE_FINISHED;
            }
        }
    }

    /* .gz not exist, use raw. */
#endif /* WEBNET_USING_GZIP */
    /* get Last-Modified. */
    if (stat_result != 0)
    {
        struct stat file_stat;
        stat_result = stat(request->path, &file_stat);

        if (stat_result == 0)
        {
            rt_enter_critical();
            strcpy(ctime_str, ctime((time_t *)&file_stat.st_mtime));
            rt_exit_critical();

            ctime_str[strlen(ctime_str) - 1] = '\0'; /* clear the end \n */

            if ((request->modified != RT_NULL)
                    && (strcmp(request->modified, ctime_str) == 0))
            {
                request->result_code = 304;
                return WEBNET_MODULE_FINISHED;
            }
        }
    }
#endif /* WEBNET_CACHE_LEVEL > 0 */

    /* get mime type */
    mimetype = mime_get_type(request->path);

#ifdef WEBNET_USING_GZIP
    if (request->support_gzip)
    {
        char *path_gz = wn_malloc(strlen(request->path) + 4);  /* ".gz\0" */

        if (path_gz != RT_NULL)
        {
            sprintf(path_gz, "%s.gz", request->path);
            fd = open(path_gz, O_RDONLY, 0);
            wn_free(path_gz);

            if (fd < 0)
            {
                /* .gz not exist, use raw. */
                request->support_gzip = RT_FALSE;
            }
        }
    }

    /* .gz not exist, use raw. */
#endif /* WEBNET_USING_GZIP */
    if (fd < 0 && stat(request->path, &file_stat) >= 0 && !S_ISDIR(file_stat.st_mode))
    {
        fd = open(request->path, O_RDONLY, 0);
    }

    if (fd < 0)
    {
        request->result_code = 404;
        return WEBNET_MODULE_FINISHED;
    }

    /* get file size */
    file_length = lseek(fd, 0, SEEK_END);
    /* seek to beginning of file */
    lseek(fd, 0, SEEK_SET);

    /*************todo**********************/
#ifdef WEBNET_USING_RANGE
    if (request->Range)
    {
        char *range_start, *range_end;
        int32_t pos_start = 0;
        uint32_t pos_end = file_length - 1;
        range_start = strstr(request->Range, "bytes=");
        if (range_start)
        {
            range_start += 6;
            range_end = strstr(range_start, "-");
            if (range_start == range_end)
                pos_start = 0;
            else
                pos_start = atoi(range_start);

            /* send file to remote */
            if ((!range_end) || (strstr(range_start, ",")))
            {
                request->result_code = 400;
                goto _error_exit;
            }
            if (range_end)
            {
                *range_end = '\0';
                range_end += 1;
                pos_end = atoi(range_end);
            }
        }
#ifdef WEBNET_USING_GZIP
        if (request->support_gzip)
        {
            pos_start = 0; /*  */
        }
#endif /* WEBNET_USING_GZIP */
        if ((pos_start >= file_length) || (pos_end >= file_length))
        {
            request->result_code = 416;
            webnet_session_set_header_status_line(session, request->result_code, "Requested Range Not Satisfiable");
            goto _error_exit;
        }
        if (lseek(fd, pos_start, SEEK_SET) != pos_start)
        {
            request->result_code = 500;
            goto _error_exit;
        }
        if (pos_end == 0)
        {
            pos_end = file_length - 1;
        }
        file_length = pos_end - pos_start + 1;
        request->result_code = 216;
        request->pos_start = pos_start;
        request->pos_end = pos_end;
        webnet_session_set_header_status_line(session, request->result_code, "Partial Content");
        webnet_session_printf(session, "Content-Range: %d-%d/%d\r\n", pos_start, pos_end, file_length);
    }else

#endif /* WEBNET_USING_RANGE */

    /*************todo**********************/
    {
        /* send file to remote */
        request->result_code = 200;
        webnet_session_set_header_status_line(session, request->result_code, "OK");
    }
#if WEBNET_CACHE_LEVEL > 0
    /* send Last-Modified. */
    webnet_session_printf(session,
                          "Last-Modified: %s\r\n",
                          ctime_str);
#endif /* WEBNET_CACHE_LEVEL > 0 */

#if WEBNET_CACHE_LEVEL > 1
    /* Cache-Control. */
    webnet_session_printf(session,
                          "Cache-Control: max-age=%d\r\n",
                          WEBNET_CACHE_MAX_AGE);
#endif /* WEBNET_CACHE_LEVEL > 1 */

    /* send Content-Type. */
    webnet_session_printf(session,
                          "Content-Type: %s\r\n",
                          mimetype);

    /* send Content-Length. */
    webnet_session_printf(session,
                          "Content-Length: %ld\r\n",
                          file_length);

#ifdef WEBNET_USING_KEEPALIVE
	if(session->request->connection == WEBNET_CONN_KEEPALIVE)
	{
		webnet_session_printf(session,
                          "Connection: %s\r\n",
                          "Keep-Alive");
	}
	else
	{
		webnet_session_printf(session,
                          "Connection: %s\r\n",
                          "close");
	}
#else
	webnet_session_printf(session,
                        "Connection: %s\r\n",
                        "close");
#endif

#ifdef WEBNET_USING_GZIP
    if (request->support_gzip)
    {
        /* gzip deflate. */
        webnet_session_printf(session, "Content-Encoding: gzip\r\n");
    }
#endif /* WEBNET_USING_GZIP */

    /* send Access-Control-Allow-Origin. */
    webnet_session_printf(session, "Access-Control-Allow-Origin:*\r\n");

    /* send http header end. */
    webnet_session_printf(session, "\r\n");

    if (file_length <= 0)
    {
        close(fd);
        return WEBNET_MODULE_FINISHED;
    }

    /*
     * set session write context
     */
    if (request->method != WEBNET_HEADER)
    {
        /* set dofile session ops */
        session->session_event_mask = WEBNET_EVENT_WRITE;
        session->user_data = (rt_uint32_t)fd;
        session->session_ops = &_dofile_ops;
    }
    return WEBNET_MODULE_FINISHED;

_error_exit:
    if (fd >= 0)
    {
        close(fd);
    }

    return WEBNET_MODULE_FINISHED;
}

static int _webnet_module_system_uri_post(struct webnet_session *session, int event)
{
    int result;
    result = WEBNET_MODULE_CONTINUE;

#ifdef WEBNET_USING_LOG
    webnet_module_log(session, event);
#endif

#ifdef WEBNET_USING_LUA
    result = webnet_module_lua(session, event);
    if (result == WEBNET_MODULE_FINISHED) return result;
#endif

#ifdef WEBNET_USING_ASP
    result = webnet_module_asp(session, event);
    if (result == WEBNET_MODULE_FINISHED) return result;
#endif

#ifdef WEBNET_USING_SSI
    result = webnet_module_ssi(session, event);
    if (result == WEBNET_MODULE_FINISHED) return result;
#endif

#ifdef WEBNET_USING_DAV
    result = webnet_module_dav(session, event);
    if (result == WEBNET_MODULE_FINISHED) return result;
#endif

#ifdef WEBNET_USING_INDEX
    result = webnet_module_dirindex(session, event);
    if (result == WEBNET_MODULE_FINISHED) return result;
#endif

    /* always module finished in dofile */
    result = webnet_module_system_dofile(session);
    if (result == WEBNET_MODULE_FINISHED) return result;

    return WEBNET_MODULE_CONTINUE;
}

static int _webnet_module_system_response_header(struct webnet_session *session, int event)
{
    int result;
    result = WEBNET_MODULE_CONTINUE;

    return result;
}

static int _webnet_module_system_response_file(struct webnet_session *session, int event)
{
    int result;
    result = WEBNET_MODULE_CONTINUE;

    return result;
}

int webnet_module_handle_event(struct webnet_session *session, int event)
{
    switch (event)
    {
    case WEBNET_EVENT_INIT:
        return _webnet_module_system_init(session, event);
    case WEBNET_EVENT_URI_PHYSICAL:
        return _webnet_module_system_uri_physical(session, event);
    case WEBNET_EVENT_URI_POST:
        return _webnet_module_system_uri_post(session, event);
    case WEBNET_EVENT_RSP_HEADER:
        return _webnet_module_system_response_header(session, event);
    case WEBNET_EVENT_RSP_FILE:
        return _webnet_module_system_response_file(session, event);
    default:
        RT_ASSERT(0);
        break;
    }

    return WEBNET_MODULE_CONTINUE;
}

/* default index file */
static const char *default_files[] =
{
    "",
    "/index.html",
    "/index.htm",
    RT_NULL
};

/**
 * handle uri
 * there are two phases on uri handling:
 * - map url to physical
 * - url handling
 */
int webnet_module_handle_uri(struct webnet_session *session)
{
    int result, fd;
    char *full_path;
    rt_uint32_t index;
    struct webnet_request *request;

    RT_ASSERT(session != RT_NULL);
    /* get request */
    request = session->request;
    RT_ASSERT(request != RT_NULL);

    /* map uri to physical */
    result = webnet_module_handle_event(session, WEBNET_EVENT_URI_PHYSICAL);
    if (result == WEBNET_MODULE_FINISHED) return result;

    /* made a full physical path */
    full_path = (char *) wn_malloc(WEBNET_PATH_MAX);
    RT_ASSERT(full_path != RT_NULL);

    /* only GET or POST need try default page. */
    if ((session->request->method != WEBNET_GET)
            && (session->request->method != WEBNET_POST))
    {
        index = sizeof(default_files) / sizeof(default_files[0]);
        index -= 1;

        goto _end_default_files;
    }

    index = 0;
    while (default_files[index] != RT_NULL)
    {
        struct stat file_stat;
        
        /* made a full path */
        rt_snprintf(full_path, WEBNET_PATH_MAX, "%s/%s%s",
                    webnet_get_root(), request->path, default_files[index]);
        /* normalize path */
        str_normalize_path(full_path);

        if (stat(full_path, &file_stat) >= 0 && !S_ISDIR(file_stat.st_mode))
        {
            break;
        }

        index ++;
    }
_end_default_files:

    /* no this file */
    if (default_files[index] == RT_NULL)
    {
        /* use old full path */
        rt_snprintf(full_path, WEBNET_PATH_MAX, "%s/%s", webnet_get_root(), request->path);
        /* normalize path */
        str_normalize_path(full_path);
    }

    /* mark path as full physical path */
    wn_free(request->path);
    request->path = full_path;

    /* check uri valid */
    if (!str_begin_with(request->path, webnet_get_root()))
    {
        /* not found */
        request->result_code = 404;
        return WEBNET_MODULE_FINISHED;
    }

    /* uri post handle */
    return webnet_module_handle_event(session, WEBNET_EVENT_URI_POST);
}
