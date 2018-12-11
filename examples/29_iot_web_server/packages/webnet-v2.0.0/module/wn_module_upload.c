/*
 * File      : wn_module_upload.c
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
 * 2012-06-25     Bernard      the first version
 */

#include <webnet.h>
#include <wn_module.h>
#include <wn_utils.h>

#ifdef RT_USING_DFS
#include <dfs_posix.h>
#endif

#if defined(WEBNET_USING_UPLOAD)

#define MULTIPART_FORM_DATA_STRING              "multipart/form-data"
#define BOUNDARY_STRING                         "boundary="
#define CONTENT_DISPOSITION_STRING              "Content-Disposition:"
#define CONTENT_TYPE_STRING                     "Content-Type:"
#define CONTENT_RANGE_STRING                    "Content-Range:"
#define FORM_DATA_STRING                        "form-data"
#define FILENAME_STRING                         "filename=\""
#define FIELDNAME_STRING                        "name=\""

#define FIRST_BOUNDARY                          2, upload_session->boundary, "\r\n"
#define NORMAL_BOUNDARY                         3, "\r\n", upload_session->boundary, "\r\n"
#define COMMON_BOUNDARY                         2, "\r\n", upload_session->boundary
#define LAST_BOUNDARY                           3, "\r\n", upload_session->boundary, "--\r\n"
#define FIRST_BOUNDARY_SIZE                     (strlen(upload_session->boundary) + 2)
#define NORMAL_BOUNDARY_SIZE                    (strlen(upload_session->boundary) + 4)
#define COMMON_BOUNDARY_SIZE                    (strlen(upload_session->boundary) + 2)
#define LAST_BOUNDARY_SIZE                      (strlen(upload_session->boundary) + 6)

struct webnet_upload_name_entry
{
    char *name;
    char *value;
};
static const struct webnet_module_upload_entry **_upload_entries = RT_NULL;
static rt_uint16_t _upload_entries_count = 0;

struct webnet_module_upload_session
{
    char* boundary;

    char* filename;
    char* content_type;

    struct webnet_upload_name_entry* name_entries;
    rt_uint16_t name_entries_count;
    rt_uint16_t file_opened;

    /* upload entry */
    const struct webnet_module_upload_entry* entry;

    /* user data */
    rt_uint32_t user_data;
};

static int str_begin_with_strs(const char* str, int num, ...)
{
    char *match;
    va_list args;
    int index, result;

    result = 1;
    va_start(args,num);
    for (index = 0; index < num; index ++)
    {
        match = va_arg(args, char*);
        if (strncasecmp(str, match, strlen(match)) != 0)
        {
            result = 0;
            break;
        }

        str += strlen(match);
    }
    va_end(args);

    return result;
}

/* Search string on binary data */
static char *memstr(const char *haystack, size_t length, const char *needle)
{
    size_t nl=strlen(needle);
    size_t hl=length;
    size_t i;

    if (!nl) goto __found;
    if (nl > length) return 0;

    for (i=hl-nl+1; i; --i)
    {
        if (*haystack==*needle && !memcmp(haystack,needle,nl))
__found:
            return (char*)haystack;
        ++haystack;
    }

    return 0;
}

/* Search string array on binary data */
static char* memstrs(const char* str, size_t length, int num, ...)
{
    char *substr;
    char *index_str, *ptr;

    va_list args;
    int index;
    int total_size;

    /* get the match total string length */
    va_start(args, num);
    total_size = 0;
    for (index = 0; index < num; index ++)
    {
        index_str = va_arg(args, char*);
        total_size += strlen(index_str);
    }
    va_end(args);

    substr = wn_malloc(total_size + 1);
    ptr = substr;
    va_start(args, num);
    for (index = 0; index < num; index++)
    {
        index_str = va_arg(args, char*);
        memcpy(ptr, index_str, strlen(index_str));
        ptr += strlen(index_str);
    }
    substr[total_size] = '\0';
    va_end(args);

    /* find in binary data */
    ptr = memstr(str, length, substr);
    wn_free(substr);

    return ptr;
}

#ifndef __GNUC__
static void* memrchr(const void *s, int c, size_t n)
{
    register const char* t=s;
    register const char* last=0;
    int i;

    t = t + n;
    for (i=n; i; --i)
    {
        if (*t==c)
        {
            last=t;
            break;
        }
        t--;
    }
    return (void*)last; /* man, what an utterly b0rken prototype */
}
#endif

static char* _webnet_module_upload_parse_header(struct webnet_session* session, char* buffer, rt_size_t length)
{
    char *ptr, *ch, *end_ptr;
    struct webnet_module_upload_session *upload_session;
    char *name = RT_NULL, *filename = RT_NULL, *content_type = RT_NULL;

    /* get upload session */
    upload_session = (struct webnet_module_upload_session *)session->user_data;

    ptr = buffer;
    end_ptr = buffer + length;
    /* begin with last boundary */
    if (str_begin_with_strs(ptr, LAST_BOUNDARY))
    {
        ptr += LAST_BOUNDARY_SIZE;
        return ptr;
    }
    /* begin with normal boundary */
    if (str_begin_with_strs(ptr, NORMAL_BOUNDARY))
        ptr += NORMAL_BOUNDARY_SIZE;
    else if (str_begin_with_strs(ptr, FIRST_BOUNDARY))
        ptr += FIRST_BOUNDARY_SIZE;
    if (ptr == buffer) return RT_NULL; /* not begin with boundary */

    if (upload_session->filename != RT_NULL &&
            upload_session->content_type != RT_NULL)
    {
        /* end of file name section */
        wn_free(upload_session->filename);
        wn_free(upload_session->content_type);

        upload_session->filename = RT_NULL;
        upload_session->content_type = RT_NULL;
    }

    while ((rt_uint32_t)ptr - (rt_uint32_t)buffer < length)
    {
        /* handle Content-Disposition: */
        if (str_begin_with(ptr, CONTENT_DISPOSITION_STRING))
        {
            ptr += sizeof(CONTENT_DISPOSITION_STRING) - 1;
            while (*ptr == ' ') ptr ++;

            /* form-data */
            if (str_begin_with(ptr, FORM_DATA_STRING))
            {
                ptr += sizeof(FORM_DATA_STRING) - 1;
                while (*ptr == ' ' || *ptr == ';') ptr ++;

                /* get name="str" */
                if (str_begin_with(ptr, FIELDNAME_STRING))
                {
                    ptr += sizeof(FIELDNAME_STRING) - 1;
                    name = ptr;

                    ch = memchr(ptr, '"', buffer - ptr);
                    if (ch != RT_NULL)
                    {
                        *ch = '\0';
                        ch ++;
                        while (*ch == ' ' || *ch ==';') ch ++;

                        ptr = ch;
                    }
                }

                /* get filename="str" */
                if (str_begin_with(ptr, FILENAME_STRING))
                {
                    ptr += sizeof(FILENAME_STRING) - 1;
                    filename = ptr;

                    ch = memchr(ptr, '"', buffer - ptr);
                    if (ch != RT_NULL)
                    {
                        *ch = '\0';
                        ch ++;
                        ptr = ch;
                    }
                }
            }
        }
        /* handle Content-Type */
        else if (str_begin_with(ptr, CONTENT_TYPE_STRING))
        {
            ptr += sizeof(CONTENT_TYPE_STRING) - 1;
            while (*ptr == ' ') ptr ++;

            content_type = ptr;
        }
        /* handle Content-Range: */
        else if (str_begin_with(ptr, CONTENT_RANGE_STRING))
        {
            ptr += sizeof(CONTENT_RANGE_STRING) - 1;
            while (*ptr == ' ') ptr ++;
        }
        /* whether end of header */
        else if (str_begin_with(ptr, "\r\n"))
        {
            ptr += 2;

            if (upload_session->filename != RT_NULL)
            {
                wn_free(upload_session->filename);
                upload_session->filename = RT_NULL;
            }
            if (upload_session->content_type != RT_NULL)
            {
                wn_free(upload_session->content_type);
                upload_session->content_type = RT_NULL;
            }

            if (filename != RT_NULL)
            {
                upload_session->filename = wn_strdup(filename);
            }
            if (content_type != RT_NULL)
            {
                upload_session->content_type = wn_strdup(content_type);
            }
            if (name != RT_NULL)
            {
                /* add a name entry in the upload session */
                if (upload_session->name_entries == RT_NULL)
                {
                    upload_session->name_entries_count = 1;
                    upload_session->name_entries = (struct webnet_upload_name_entry*)
                                                   wn_malloc(sizeof(struct webnet_upload_name_entry));
                }
                else
                {
                    upload_session->name_entries_count += 1;
                    upload_session->name_entries = (struct webnet_upload_name_entry*)
                                                   wn_realloc(upload_session->name_entries,
                                                           sizeof(struct webnet_upload_name_entry) * upload_session->name_entries_count);
                }

                upload_session->name_entries[upload_session->name_entries_count - 1].name = wn_strdup(name);
                upload_session->name_entries[upload_session->name_entries_count - 1].value = RT_NULL;
            }

            return ptr;
        }

        /* skip this request header line */
        ch = memstr(ptr, end_ptr - ptr, "\r\n");
        *ch = '\0';
        ch ++;
        *ch = '\0';
        ch ++;
        ptr = ch;
    }

    return ptr;
}

static char* _next_possible_boundary(struct webnet_session* session, char* buffer, rt_size_t length)
{
    char *ptr;
    struct webnet_module_upload_session *upload_session;

    /* get upload session */
    upload_session = (struct webnet_module_upload_session *)session->user_data;

    /* try the beginning */
    if (str_begin_with_strs(buffer, COMMON_BOUNDARY)) return buffer;
    /* search in all the string */
    ptr = memstrs(buffer, length, COMMON_BOUNDARY);
    if (ptr != RT_NULL) return ptr;

    /* search from the end of string */
    ptr = memrchr(buffer, '\r', length);
    if (ptr == RT_NULL ) return RT_NULL;
    if (ptr - buffer == length - 1) return ptr; /* only \r */
    if (*(ptr + 1) == '\n' && ptr - buffer == length - 2) return RT_NULL; /* only \r\n */

    if (memcmp(ptr + 2, upload_session->boundary, length - (ptr - buffer + 2)) == 0) return ptr;

    return RT_NULL;
}

static void _handle_section(struct webnet_session* session, char* buffer, rt_size_t length)
{
    char *ptr;
    struct webnet_module_upload_session *upload_session;

#define name_entry  \
    (upload_session->name_entries[upload_session->name_entries_count - 1])

    /* get upload session */
    upload_session = (struct webnet_module_upload_session *)session->user_data;
    if (upload_session->filename != RT_NULL &&
            upload_session->content_type != RT_NULL&&
            length != 0)
    {
        upload_session->entry->upload_write(session, buffer, length);
    }
    else
    {
        /* get name value */
        ptr = memstr(buffer, length, "\r\n");
        if (ptr != RT_NULL)
        {
            int value_size = ptr - buffer + 1;
            name_entry.value = wn_malloc(value_size);
            memcpy(name_entry.value, buffer, value_size - 1);
            name_entry.value[value_size - 1] = '\0';
        }
    }
}

static void _webnet_module_upload_handle(struct webnet_session* session, int event)
{
    int length;
    char *ptr, *end_ptr;
    char *upload_buffer;
    rt_uint32_t chunk_size;
    struct webnet_module_upload_session *upload_session;

    if (event != WEBNET_EVENT_READ) return;

    /* get upload session */
    upload_session = (struct webnet_module_upload_session *)session->user_data;
    upload_buffer = (char*)session->buffer;

    /* read stream */
    if (memstrs((const char *)session->buffer, session->buffer_offset, LAST_BOUNDARY))
    {
        length = session->buffer_offset;
    }
    else
    {
        if (session->buffer_offset != 0)
        {
            length = webnet_session_read(session,
                (char*)&session->buffer[session->buffer_offset],
                sizeof(session->buffer) - session->buffer_offset - 1);
            if (length > 0)
            {
                length += session->buffer_offset;
            }
            else
            {
                length = session->buffer_offset;
            }
        }
        else
        {
            length = webnet_session_read(session, (char *)session->buffer, sizeof(session->buffer) - 1);
        }
    }

    /* connection break out */
    if (length <= 0)
    {
        /* read stream failed (connection break out), close this session */
        session->session_phase = WEB_PHASE_CLOSE;
        return;
    }
    end_ptr = (char*)(session->buffer + length);

    session->buffer[length] = '\0';
    while (upload_buffer < end_ptr)
    {
        if (str_begin_with_strs(upload_buffer, LAST_BOUNDARY))
        {
            /* upload done */
            upload_session->entry->upload_done(session);
            session->session_phase = WEB_PHASE_CLOSE;
            return;
        }

        /* read more data */
        if (end_ptr - upload_buffer < sizeof(session->buffer)/3 &&
                memstrs(upload_buffer, end_ptr - upload_buffer, LAST_BOUNDARY) == RT_NULL)
        {
            /* read more data */
            rt_memmove(session->buffer, upload_buffer, end_ptr - upload_buffer);
            session->buffer_offset = end_ptr - upload_buffer;
            return ;
        }

        ptr = _webnet_module_upload_parse_header(session, upload_buffer,
                (rt_uint32_t)end_ptr - (rt_uint32_t)upload_buffer);
        if (ptr == RT_NULL)
        {
            /* not begin with a boundary */
            ptr = _next_possible_boundary(session, upload_buffer,
                                          (rt_uint32_t)end_ptr - (rt_uint32_t)upload_buffer);
            if (ptr == RT_NULL)
            {
                /* all are the data section */
                _handle_section(session, upload_buffer, (rt_uint32_t)end_ptr - (rt_uint32_t)upload_buffer);
                upload_buffer = end_ptr;
            }
            else
            {
                chunk_size = (rt_uint32_t)ptr - (rt_uint32_t)upload_buffer;
                _handle_section(session, upload_buffer, chunk_size);
                upload_buffer += chunk_size;
            }
        }
        else
        {
            if (upload_session->filename != RT_NULL &&
                    upload_session->content_type != RT_NULL)
            {
                if (upload_session->file_opened == 0)
                {
                    /* open file */
                    upload_session->user_data = upload_session->entry->upload_open(session);
                    upload_session->file_opened = 1;
                }
            }
            else
            {
                if (upload_session->file_opened == 1)
                {
                    /* close file */
                    upload_session->entry->upload_close(session);
                    upload_session->user_data = 0;
                    upload_session->file_opened = 0;
                }
            }

            upload_buffer = ptr;
            ptr = _next_possible_boundary(session, upload_buffer,
                                          (rt_uint32_t)end_ptr - (rt_uint32_t)upload_buffer);
            if (ptr == RT_NULL)
            {
                /* all are the data section */
                _handle_section(session, upload_buffer, (rt_uint32_t)end_ptr - (rt_uint32_t)upload_buffer);
                upload_buffer = end_ptr;
            }
            else
            {
                chunk_size = (rt_uint32_t)ptr - (rt_uint32_t)upload_buffer;
                _handle_section(session, upload_buffer, chunk_size);
                upload_buffer += chunk_size;
            }
        }
    }

    session->buffer_offset = 0;
}

static void _webnet_module_upload_close(struct webnet_session* session)
{
    struct webnet_module_upload_session *upload_session;

    /* get upload session */
    upload_session = (struct webnet_module_upload_session *)session->user_data;
    if (upload_session == RT_NULL) return;

    /* close file */
    if (upload_session->file_opened == 1)
    {
        upload_session->entry->upload_close(session);
        upload_session->file_opened = 0;
    }

    /* free session */
    if (upload_session->filename != RT_NULL)
        wn_free(upload_session->filename);
    if (upload_session->content_type != RT_NULL)
        wn_free(upload_session->content_type);
    if (upload_session->boundary != RT_NULL)
        wn_free(upload_session->boundary);
    if (upload_session->entry != RT_NULL)
    {
        rt_uint32_t index;
        for (index = 0; index < upload_session->name_entries_count; index ++)
        {
            if (upload_session->name_entries[index].value != RT_NULL)
                wn_free(upload_session->name_entries[index].value);
            if (upload_session->name_entries[index].name != RT_NULL)
                wn_free(upload_session->name_entries[index].name);
        }
        wn_free(upload_session->name_entries);
        upload_session->name_entries = RT_NULL;
    }
    wn_free(upload_session);

    /* remove private data */
    session->user_data = 0;
    session->session_ops = RT_NULL;
    session->session_phase = WEB_PHASE_CLOSE;
}

static const struct webnet_session_ops _upload_ops =
{
    _webnet_module_upload_handle,
    _webnet_module_upload_close
};

int webnet_module_upload_open(struct webnet_session* session)
{
    char* boundary;
    rt_uint32_t index, length;
    const struct webnet_module_upload_entry *entry = RT_NULL;
    struct webnet_module_upload_session *upload_session;

    if (session->request->method != WEBNET_POST)
        return WEBNET_MODULE_CONTINUE;

    /* get upload entry */
    for (index = 0; index < _upload_entries_count; index ++)
    {
        if (str_begin_with(session->request->path, _upload_entries[index]->url))
        {
            length = rt_strlen(_upload_entries[index]->url);
            if (session->request->path[length] == '\0' ||
                    session->request->path[length] == '?')
            {
                /* found entry */
                entry = _upload_entries[index];
                break;
            }
        }
    }
    if (entry == RT_NULL) /* no this entry */
        return WEBNET_MODULE_CONTINUE;

    /* create a uploading session */
    upload_session = (struct webnet_module_upload_session*) wn_malloc (
                         sizeof (struct webnet_module_upload_session));
    if (upload_session == RT_NULL) return 0; /* no memory */

    /* get boundary */
    boundary = strstr(session->request->content_type, BOUNDARY_STRING);
    if (boundary != RT_NULL)
    {
        int boundary_size;

        /* make boundary */
        boundary_size = strlen(boundary + sizeof(BOUNDARY_STRING) - 1) + 3;
        upload_session->boundary = wn_malloc(boundary_size);
        rt_sprintf(upload_session->boundary, "--%s", boundary + sizeof(BOUNDARY_STRING) - 1);
    }
    upload_session->filename = RT_NULL;
    upload_session->content_type = RT_NULL;
    upload_session->name_entries = RT_NULL;
    upload_session->name_entries_count = 0;
    upload_session->file_opened = 0;
    upload_session->entry = entry;
    upload_session->user_data = 0;

    /* add this upload session into webnet session */
    session->user_data = (rt_uint32_t) upload_session;
    /* set webnet session operations */
    session->session_ops = &_upload_ops;

    session->session_ops->session_handle(session, WEBNET_EVENT_READ);

    return WEBNET_MODULE_FINISHED;
}

int webnet_module_upload(struct webnet_session* session, int event)
{
    if (event == WEBNET_EVENT_URI_PHYSICAL)
    {
        return webnet_module_upload_open(session);
    }

    return WEBNET_MODULE_CONTINUE;
}

/* webnet upload module APIs */
void webnet_upload_add(const struct webnet_module_upload_entry* entry)
{
    if (_upload_entries == RT_NULL)
    {
        _upload_entries_count = 1;

        /* first entries, malloc upload entry */
        _upload_entries = (const struct webnet_module_upload_entry**)
                          wn_malloc (sizeof(struct webnet_module_upload_entry));
    }
    else
    {
        _upload_entries_count += 1;
        _upload_entries = (const struct webnet_module_upload_entry**) wn_realloc (_upload_entries,
                          sizeof(void*) * _upload_entries_count);
    }
    RT_ASSERT(_upload_entries != RT_NULL);

    _upload_entries[_upload_entries_count - 1] = entry;
}
RTM_EXPORT(webnet_upload_add);

const char* webnet_upload_get_filename(struct webnet_session* session)
{
    struct webnet_module_upload_session *upload_session;

    /* get upload session */
    upload_session = (struct webnet_module_upload_session *)session->user_data;
    if (upload_session == RT_NULL) return RT_NULL;

    return upload_session->filename;
}
RTM_EXPORT(webnet_upload_get_filename);

const char* webnet_upload_get_content_type(struct webnet_session* session)
{
    struct webnet_module_upload_session *upload_session;

    /* get upload session */
    upload_session = (struct webnet_module_upload_session *)session->user_data;
    if (upload_session == RT_NULL) return RT_NULL;

    return upload_session->content_type;
}
RTM_EXPORT(webnet_upload_get_content_type);

const char* webnet_upload_get_nameentry(struct webnet_session* session, const char* name)
{
    rt_uint32_t index;
    struct webnet_module_upload_session *upload_session;

    /* get upload session */
    upload_session = (struct webnet_module_upload_session *)session->user_data;
    if (upload_session == RT_NULL) return RT_NULL;

    for (index = 0; index < upload_session->name_entries_count; index ++)
    {
        if (strcasecmp(upload_session->name_entries[index].name, name) == 0)
        {
            return upload_session->name_entries[index].value;
        }
    }

    return RT_NULL;
}
RTM_EXPORT(webnet_upload_get_nameentry);

const void* webnet_upload_get_userdata(struct webnet_session* session)
{
    struct webnet_module_upload_session *upload_session;

    /* get upload session */
    upload_session = (struct webnet_module_upload_session *)session->user_data;
    if (upload_session == RT_NULL) return RT_NULL;

    return (const void*) upload_session->user_data;
}
RTM_EXPORT(webnet_upload_get_userdata);

/* ---- upload file interface ---- */
int webnet_upload_file_open(struct webnet_session* session)
{
    struct webnet_module_upload_session* upload_session;

    /* get upload session */
    upload_session = (struct webnet_module_upload_session *)session->user_data;
    if (upload_session->filename != RT_NULL)
    {
        /* open file */
        upload_session->user_data = open(upload_session->filename, O_WRONLY, 0);
        return upload_session->user_data;
    }

    return -1;
}

int webnet_upload_file_close(struct webnet_session* session)
{
    int fd;
    struct webnet_module_upload_session* upload_session;

    /* get upload session */
    upload_session = (struct webnet_module_upload_session *)session->user_data;
    fd = upload_session->user_data;
    if (fd >= 0)
    {
        return close(fd);
    }
    return -1;
}

int webnet_upload_file_write(struct webnet_session* session, const void* data, rt_size_t length)
{
    int fd;
    struct webnet_module_upload_session* upload_session;

    /* get upload session */
    upload_session = (struct webnet_module_upload_session *)session->user_data;
    fd = upload_session->user_data;
    if (fd >= 0)
    {
        return write(fd, data, length);
    }

    return 0;
}

#endif /* WEBNET_USING_UPLOAD */
