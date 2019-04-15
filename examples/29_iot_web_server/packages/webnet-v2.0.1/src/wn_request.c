/*
 * File      : wn_request.c
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
 * 2011-08-05     Bernard      fixed the query issue
 * 2011-08-18     Bernard      fixed chrome query issue
 * 2011-11-12     Bernard      fixed Content-Length zero issue in POST.
 * 2012-06-25     Bernard      added parsing for Content-Type
 * 2012-09-15     Bernard      fixed basic authentication issue in Safari.
 * 2012-10-27     Bernard      fixed the upload issue in FireFox.
 * 2012-12-17     Bernard      fixed the content multi-transmission issue in POST.
 */

#include <string.h>
#include <stdlib.h>

#include <webnet.h>
#include <wn_request.h>
#include <wn_utils.h>

#define DBG_ENABLE
#define DBG_COLOR
#define DBG_SECTION_NAME    "wn.request"
#ifdef WEBNET_USING_LOG
#define DBG_LEVEL           DBG_LOG
#else
#define DBG_LEVEL           DBG_INFO
#endif /* WEBNET_USING_LOG */
#include <rtdbg.h>

#define POST_BUFSZ_MAX      (8 * 1024)

/**
 * parse a query
 */
static void _webnet_request_parse_query(struct webnet_request* request)
{
    char *ptr, *end_ptr;
    rt_uint32_t index;

    if ((request->query == RT_NULL) || (*request->query == '\0')) return; /* no query */

    /* get the query counter */
    ptr = request->query;
    end_ptr = request->query + request->content_length;

    request->query_counter = 1;
    while (*ptr && ptr <= end_ptr)
    {
        if (*ptr == '&')
        {
            while ((*ptr == '&') && (*ptr != '\0')) ptr ++;
            if (*ptr == '\0') break;

            request->query_counter ++;
        }
        ptr ++;
    }
    if (request->query_counter == 0) return; /* no query */

    /* allocate query item */
    request->query_items = (struct webnet_query_item*) wn_malloc (sizeof(struct webnet_query_item)
                           * request->query_counter);
    if (request->query_items == RT_NULL)
    {
        request->result_code = 500;
        return;
    }

    /* parse the query */
    ptr = request->query;
    for (index = 0; index < request->query_counter; index ++)
    {
        request->query_items[index].name = ptr;
        request->query_items[index].value = RT_NULL;

        /* get value or goto next item */
        while ((*ptr != '&') && (*ptr != '\0'))
        {
            /* get value */
            if (*ptr == '=')
            {
                *ptr = '\0';
                ptr ++;
                request->query_items[index].value = ptr;
                urldecode(request->query_items[index].value, strlen(request->query_items[index].value));
            }
            else ptr ++;
        }

        if (*ptr == '\0') break;
        if (*ptr == '&')
        {
            /* make a item */
            *ptr = '\0';
            ptr ++;
            while (*ptr == '&' && *ptr != '\0' && ptr <= end_ptr)ptr ++;
            if (*ptr == '\0') break;
        }
    }
}

/**
 * copy string from request and the field_copied set to TRUE
 */
static void _webnet_request_copy_str(struct webnet_request* request)
{
    if (request->path != RT_NULL) request->path = wn_strdup(request->path);
    if (request->host != RT_NULL)
    {
        char *ptr;
        ptr = request->host;
        while (*ptr && *ptr != ':') ptr ++;
        if (*ptr == ':') *ptr = '\0';

        request->host = wn_strdup(request->host);
    }
    if (request->cookie != RT_NULL) request->cookie = wn_strdup(request->cookie);
    if (request->user_agent != RT_NULL) request->user_agent = wn_strdup(request->user_agent);
    if (request->authorization != RT_NULL) request->authorization = wn_strdup(request->authorization);
    if (request->accept_language != RT_NULL) request->accept_language = wn_strdup(request->accept_language);
    if (request->referer != RT_NULL) request->referer = wn_strdup(request->referer);
    if (request->content_type != RT_NULL) request->content_type = wn_strdup(request->content_type);

    /* DMR */
    if (request->callback) request->callback = wn_strdup(request->callback);
    if (request->soap_action) request->soap_action = wn_strdup(request->soap_action);
    if (request->sid) request->sid = wn_strdup(request->sid);

    /* DAV */
#ifdef WEBNET_USING_DAV
    if (request->depth) request->depth = wn_strdup(request->depth);
#endif
#ifdef WEBNET_USING_RANGE
    if (request->Range) request->Range = wn_strdup(request->Range);
#endif /* WEBNET_USING_RANGE */
    request->field_copied = RT_TRUE;
}

/**
 * to check whether a query on the http request.
 */
rt_bool_t webnet_request_has_query(struct webnet_request* request, char* name)
{
    rt_uint32_t index;

    for (index = 0; index < request->query_counter; index ++)
    {
        if (strncmp(request->query_items[index].name, name, strlen(name)) == 0)
            return RT_TRUE;
    }

    return RT_FALSE;
}
RTM_EXPORT(webnet_request_has_query);

/**
 * get query value according to the name
 */
const char* webnet_request_get_query(struct webnet_request* request, char* name)
{
    rt_uint32_t index;

    for (index = 0; index < request->query_counter; index ++)
    {
        if (strncmp(request->query_items[index].name, name, strlen(name)) == 0)
            return request->query_items[index].value;
    }

    return RT_NULL;
}
RTM_EXPORT(webnet_request_get_query);

struct web_method
{
    const char *method_str;
    enum webnet_method method_value;
};

const struct web_method methods [] = {
    {"GET ",        WEBNET_GET},
    {"POST ",       WEBNET_POST},
    {"HEADER ",     WEBNET_HEADER},
    {"HEAD ",       WEBNET_HEAD},
    {"PUT ",        WEBNET_PUT},
    {"OPTIONS ",    WEBNET_OPTIONS},
    {"PROPFIND ",   WEBNET_PROPFIND},
    {"PROPPATCH ",  WEBNET_PROPPATCH},
    {"DELETE ",     WEBNET_DELETE},
    {"CONNECT ",    WEBNET_CONNECT},
    {"MKCOL ",      WEBNET_MKCOL},
    {"MOVE ",       WEBNET_MOVE},
    {"SUBSCRIBE ",  WEBNET_SUBSCRIBE},
    {"UNSUBSCRIBE ", WEBNET_UNSUBSCRIBE},
    {"NOTIFY ",     WEBNET_NOTIFY},
    {NULL, WEBNET_UNKNOWN},
};

int webnet_request_parse_method(struct webnet_request *request, char* buffer, int length)
{
    char *ch;
    int index;
    char *request_buffer;
    const struct web_method *method;

    RT_ASSERT(request != RT_NULL);
    RT_ASSERT(request->session != RT_NULL);

    request_buffer = buffer;

    if (strstr(request_buffer, "\r\n") == RT_NULL) return 0;

    /* parse method */
    for (index = 0; ; index ++)
    {
        method = &methods[index];
        if (method->method_value == WEBNET_UNKNOWN)
        {
            /* Not implemented for other method */
            request->result_code = 501;
            return 0;
        }

        if (str_begin_with(request_buffer, method->method_str))
        {
            request->method = method->method_value;
            request_buffer += strlen(method->method_str);
            break;
        }
    }

    /* get path */
    ch = strchr(request_buffer, ' ');
    if (ch == RT_NULL)
    {
        /* Bad Request */
        request->result_code = 400;
        return request_buffer - buffer;
    }
    *ch++ = '\0';
    request->path = wn_strdup(request_buffer);
    request_buffer = ch;

    /* check path, whether there is a query */
    ch = strchr(request->path, '?');
    if (ch != RT_NULL)
    {
        *ch++ = '\0';
        while (*ch == ' ') ch ++;

        /* copy query and parse query */
        request->query = wn_strdup(ch);
        /* copy query and parse parameter */
        _webnet_request_parse_query(request);
    }

    /* check protocol */
    if (!str_begin_with(request_buffer, "HTTP/1"))
    {
        /* Not implemented, webnet does not support HTTP 0.9 protocol */
        request->result_code = 501;
        return request_buffer - buffer;
    }

    ch = strstr(request_buffer, "\r\n");
    *ch ++ = '\0';
    *ch ++ = '\0';
    request_buffer = ch;

    /* move to next phase */
    request->session->session_phase = WEB_PHASE_HEADER;

    return request_buffer - buffer;
}

int webnet_request_parse_header(struct webnet_request *request, char* buffer, int length)
{
    char *ch;
    char *request_buffer;
    struct webnet_session *session;

    RT_ASSERT(request != RT_NULL);
    RT_ASSERT(request->session != RT_NULL);

    session = request->session;
    request_buffer = buffer;

    for (;;)
    {
        if (str_begin_with(request_buffer, "\r\n"))
        {
            /* end of http request */
            request_buffer += 2;

            if (request->content_length && request->method == WEBNET_POST)
            {
                session->session_phase = WEB_PHASE_RESPONSE;
                if (!str_begin_with(request->content_type, "multipart/form-data") &&
                    request->content_length < POST_BUFSZ_MAX)
                {
                    session->session_phase = WEB_PHASE_QUERY;

                    /* allocate query buffer */
                    request->query = (char*) wn_malloc(request->content_length + 1);
                    rt_memset(request->query, 0, request->content_length + 1);
                    request->query_offset = 0;
                }
            }
            else
            {
                /* end of http request */
                request->result_code = 200;
                /* move to next phase */
                session->session_phase = WEB_PHASE_RESPONSE;
            }
            break;
        }

        if (*request_buffer == '\0')
        {
            /* not the end of http request */
            return request_buffer - buffer;
        }

        ch = strstr(request_buffer, "\r\n");
        if (ch == RT_NULL)
        {
            /* not the end of http header request line */
            return request_buffer - buffer;
        }
        /* set terminal field */
        *ch ++ = '\0';
        *ch ++ = '\0';

        if (str_begin_with(request_buffer, "Host:"))
        {
            /* get host */
            request_buffer += 5;
            while (*request_buffer == ' ') request_buffer ++;
            request->host = wn_strdup(request_buffer);
        }
        else if (str_begin_with(request_buffer, "User-Agent:"))
        {
            /* get user agent */
            request_buffer += 11;
            while (*request_buffer == ' ') request_buffer ++;
            request->user_agent = wn_strdup(request_buffer);
        }
        else if (str_begin_with(request_buffer, "Accept-Language:"))
        {
            /* get accept language */
            request_buffer += 16;
            while (*request_buffer == ' ') request_buffer ++;
            request->accept_language = wn_strdup(request_buffer);
        }
        else if (str_begin_with(request_buffer, "Content-Length:"))
        {
            /* get content length */
            request_buffer += 15;
            while (*request_buffer == ' ') request_buffer ++;
            request->content_length = atoi(request_buffer);
        }
        else if (str_begin_with(request_buffer, "Content-Type:"))
        {
            /* get content type */
            request_buffer += 13;
            while (*request_buffer == ' ') request_buffer ++;
            request->content_type = wn_strdup(request_buffer);
        }
        else if (str_begin_with(request_buffer, "Referer:"))
        {
            /* get referer */
            request_buffer += 8;
            while (*request_buffer == ' ') request_buffer ++;
            request->referer = wn_strdup(request_buffer);
        }
#ifdef WEBNET_USING_RANGE
        else if (str_begin_with(request_buffer, "Range:"))
        {
            /* get range */
            request_buffer += 6;
            while (*request_buffer == ' ') request_buffer ++;
            request->Range = wn_strdup(request_buffer);
        }
#endif /* WEBNET_USING_RANGE */
#ifdef WEBNET_USING_DAV
        else if(str_begin_with(request_buffer, "Depth:"))
        {
            request_buffer += 6;
            while (*request_buffer == ' ') request_buffer ++;
            request->depth = wn_strdup(request_buffer);
        }
        else if (str_begin_with(request_buffer, "Destination:"))
        {
            request_buffer += 12;
            while (*request_buffer == ' ') request_buffer ++;
            request->destination = wn_strdup(request_buffer);
        }
#endif /* WEBNET_USING_DAV */
#ifdef WEBNET_USING_KEEPALIVE
        else if (str_begin_with(request_buffer, "Connection:"))
        {
            /* set default connection to keep alive */
            request->connection = WEBNET_CONN_KEEPALIVE;

            /* get connection */
            request_buffer += 11;
            while (*request_buffer == ' ') request_buffer ++;

            if (str_begin_with(request_buffer, "close"))
                request->connection = WEBNET_CONN_CLOSE;
            else if (str_begin_with(request_buffer, "Keep-Alive"))
                request->connection = WEBNET_CONN_KEEPALIVE;
        }
#endif
#ifdef WEBNET_USING_COOKIE
        else if (str_begin_with(request_buffer, "Cookie:"))
        {
            /* get cookie */
            request_buffer += 7;
            while (*request_buffer == ' ') request_buffer ++;
            request->cookie = wn_strdup(request_buffer);
        }
#endif /* WEBNET_USING_COOKIE */
#ifdef WEBNET_USING_AUTH
        else if (str_begin_with(request_buffer, "Authorization: Basic"))
        {
            /* get authorization */
            request_buffer += 20;
            while (*request_buffer == ' ') request_buffer ++;
            request->authorization = wn_strdup(request_buffer);
        }
#endif /* WEBNET_USING_AUTH */
#if WEBNET_CACHE_LEVEL > 0
        else if (str_begin_with(request_buffer, "If-Modified-Since:"))
        {
            /* get If-Modified-Since */
            request_buffer += 18;
            while (*request_buffer == ' ') request_buffer ++;
            request->modified = wn_strdup(request_buffer);
        }
#endif /* WEBNET_CACHE_LEVEL > 0 */
#ifdef WEBNET_USING_GZIP
        else if (str_begin_with(request_buffer, "Accept-Encoding:"))
        {
            const char *gzip = strstr(request_buffer, "gzip");

            if( (gzip != RT_NULL))
            {
                request->support_gzip = RT_TRUE;
            }
        }
#endif /* WEBNET_USING_GZIP */
        else if (str_begin_with(request_buffer, "SOAPACTION:"))
        {
            request_buffer += 11;
            while (*request_buffer == ' ') request_buffer ++;

            request->soap_action = wn_strdup(request_buffer);
        }
        else if (str_begin_with(request_buffer, "CALLBACK:"))
        {
            request_buffer += 9;
            while (*request_buffer == ' ') request_buffer ++;

            request->callback = wn_strdup(request_buffer);
        }

        request_buffer = ch;
    }

    return request_buffer - buffer;
}

int webnet_request_parse_post(struct webnet_request* request, char* buffer, int length)
{
    struct webnet_session* session = request->session;

    if (request->query && length)
    {
        if (request->query_offset + length > request->content_length)
            length = request->content_length - request->query_offset;

        memcpy(&request->query[request->query_offset], buffer, length);
        request->query_offset += length;

        if (request->query_offset == request->content_length)
        {
            /* set terminal charater */
            buffer[request->content_length] = '\0';

            /* parse query */
            if (str_begin_with(request->content_type, "application/x-www-form-urlencoded"))
            {
                _webnet_request_parse_query(request);
            }

            /* set to http response phase */
            request->result_code = 200;
            session->session_phase = WEB_PHASE_RESPONSE;
        }
    }

    return length;
}

/**
 * parse web request
 */
void webnet_request_parse(struct webnet_request* request, char* buffer, int length)
{
    char *ch;
    char *request_buffer;
    char *content_length;

    RT_ASSERT(request != RT_NULL);
    RT_ASSERT(request->session != RT_NULL);

    content_length = RT_NULL;
    request_buffer = buffer;
    /* web request begin with method */
    if (str_begin_with(request_buffer, "GET "))
    {
        request->method = WEBNET_GET;
        request_buffer += 4;
    }
    else if (str_begin_with(request_buffer, "POST "))
    {
        request->method = WEBNET_POST;
        request_buffer += 5;
    }
    else if (str_begin_with(request_buffer, "HEADER "))
    {
        request->method = WEBNET_HEADER;
        request_buffer += 7;
    }
    else if (str_begin_with(request_buffer, "SUBSCRIBE "))
    {
        request->method = WEBNET_SUBSCRIBE;
        request_buffer += 10;
    }
    else if (str_begin_with(request_buffer, "UNSUBSCRIBE "))
    {
        request->method = WEBNET_UNSUBSCRIBE;
        request_buffer += 12;
    }
    else if (str_begin_with(request_buffer, "NOTIFY "))
    {
        request->method = WEBNET_NOTIFY;
        request_buffer += 7;
    }
#ifdef WEBNET_USING_DAV
    else if (str_begin_with(request_buffer, "PUT "))
    {
        request->method = WEBNET_PUT;
        request_buffer += 4;
    }
    else if(str_begin_with(request_buffer, "OPTIONS "))
    {
        request->method = WEBNET_OPTIONS;
        request_buffer += 8;
    }
    else if(str_begin_with(request_buffer, "PROPFIND "))
    {
        request->method = WEBNET_PROPFIND;
        request_buffer += 9;
    }
    else if(str_begin_with(request_buffer, "PROPPATCH "))
    {
        request->method = WEBNET_PROPPATCH;
        request_buffer += 10;
    }
    else if (str_begin_with(request_buffer, "DELETE "))
    {
        request->method = WEBNET_DELETE;
        request_buffer += 7;
    }
    else if (str_begin_with(request_buffer, "MKCOL "))
    {
        request->method = WEBNET_MKCOL;
        request_buffer += 6;
    }
    else if (str_begin_with(request_buffer, "HEAD "))
    {
        request->method = WEBNET_HEAD;
        request_buffer += 5;
    }
#endif /* WEBNET_USING_DAV */
    else
    {
        /* Not implemented for other method */
        request->result_code = 501;
        return ;
    }

    /* get path */
    request->path = request_buffer;
    ch = strchr(request_buffer, ' ');
    if (ch == RT_NULL)
    {
        /* Bad Request */
        request->result_code = 400;
        return;
    }
    *ch++ = '\0';
    request_buffer = ch;

    /* check path, whether there is a query */
    ch = strchr(request->path, '?');
    if (ch != RT_NULL)
    {
        *ch++ = '\0';
        while (*ch == ' ') ch ++;

        /* copy query and parse query */
        request->query = wn_strdup(ch);
        /* copy query and parse parameter */
        _webnet_request_parse_query(request);
    }

    /* check protocol */
    if (!str_begin_with(request_buffer, "HTTP/1"))
    {
        /* Not implemented, webnet does not support HTTP 0.9 protocol */
        request->result_code = 501;
        return;
    }

    ch = strstr(request_buffer, "\r\n");
    if (ch == RT_NULL)
    {
        /* Bad Request */
        request->result_code = 400;
        return;
    }
    *ch ++ = '\0';
    *ch ++ = '\0';
    request_buffer = ch;

    for (;;)
    {
        if (str_begin_with(request_buffer, "\r\n"))
        {
            /* end of get request */

            /* made a string field copy */
            _webnet_request_copy_str(request);

            /* get content length */
            if (content_length != RT_NULL)
                request->content_length = atoi(content_length);

            if (request->method == WEBNET_POST && request->content_length > 0) /* POST method */
            {
                char *read_ptr = RT_NULL;
                int read_length;
                struct webnet_session* session = request->session;

                /* skip '\r\n' */
                request->query = request_buffer + 2;

                /* check whether it's an uploading request */
                if (str_begin_with(request->content_type, "multipart/form-data"))
                {
                    /* end of http request */
                    request->result_code = 200;
                    session->buffer_offset = (rt_uint16_t)(length -
                                                      ((rt_uint32_t)request->query - (rt_uint32_t)buffer));
                    /* move the buffer to the session */
                    if (session->buffer_offset > 0)
                    {
                        /*
                         * NOTIC: the rest of buffer must not be great than session buffer
                         * Therefore, the size of read buffer can equal to the size of session buffer.
                         */
                        memcpy(session->buffer, request->query, session->buffer_offset);
                    }
                    request->query = RT_NULL;
                    return;
                }

                if (request->content_length > POST_BUFSZ_MAX)
                {
                    LOG_E("content length too long: %d, discard it.", request->content_length);
                    /* we can not provide enough buffer for post */
                    request->query = RT_NULL;
                    return;
                }

                /* allocate a new query content and copy the already read content */
                read_ptr = (char *)wn_malloc(request->content_length);
                if (read_ptr == RT_NULL)
                {
                    LOG_E("No memory for request read buffer!");
                    request->query = RT_NULL;
                    return ;
                }

                /* get the length of already read bytes */
                read_length = (int)(length - ((rt_uint32_t)request->query - (rt_uint32_t)buffer));
                if (read_length > 0)
                {
                    if (read_length > request->content_length) read_length = request->content_length;
                    memcpy(read_ptr, request->query, read_length);
                }
                request->query = read_ptr;

                read_ptr += read_length;
                /* read all of post content */
                while (read_length < request->content_length)
                {
                    int read_bytes;

                    /* read the rest content of POST */
                    read_bytes = webnet_session_read(session, read_ptr, request->content_length - read_length);
                    if (read_bytes < 0)
                    {
                        /* read failed and session should been closed. */
                        wn_free(request->query);
                        request->query = RT_NULL;
                        return;
                    }

                    read_length += read_bytes;
                    read_ptr += read_bytes;
                }

                session->buffer_offset = request->content_length;
                if (!str_begin_with(request->content_type, "text/xml"))
                    _webnet_request_parse_query(request);
                else
                {
                    /* do other POST action */
                }
            }

            /* end of http request */
            request->result_code = 200;

            return;
        }

        if (*request_buffer == '\0')
        {
            /* end of http request */
            request->result_code = 200;

            /* made a string field copy */
            _webnet_request_copy_str(request);
            return;
        }

        if (str_begin_with(request_buffer, "Host:"))
        {
            /* get host */
            request_buffer += 5;
            while (*request_buffer == ' ') request_buffer ++;
            request->host = request_buffer;
        }
        else if (str_begin_with(request_buffer, "User-Agent:"))
        {
            /* get user agent */
            request_buffer += 11;
            while (*request_buffer == ' ') request_buffer ++;
            request->user_agent = request_buffer;
        }
        else if (str_begin_with(request_buffer, "Accept-Language:"))
        {
            /* get accept language */
            request_buffer += 16;
            while (*request_buffer == ' ') request_buffer ++;
            request->accept_language = request_buffer;
        }
        else if (str_begin_with(request_buffer, "Content-Length:"))
        {
            /* get content length */
            request_buffer += 15;
            while (*request_buffer == ' ') request_buffer ++;
            content_length = request_buffer;
        }
        else if (str_begin_with(request_buffer, "Content-Type:"))
        {
            /* get content type */
            request_buffer += 13;
            while (*request_buffer == ' ') request_buffer ++;
            request->content_type = request_buffer;
        }
        else if (str_begin_with(request_buffer, "Referer:"))
        {
            /* get referer */
            request_buffer += 8;
            while (*request_buffer == ' ') request_buffer ++;
            request->referer = request_buffer;
        }
#ifdef WEBNET_USING_DAV
        else if(str_begin_with(request_buffer, "Depth:"))
        {
            request_buffer += 6;
            while (*request_buffer == ' ') request_buffer ++;
            request->depth = request_buffer;
        }
#endif /* WEBNET_USING_DAV */
#ifdef WEBNET_USING_KEEPALIVE
        else if (str_begin_with(request_buffer, "Connection:"))
        {
            /* set default connection to keep alive */
            request->connection = WEBNET_CONN_KEEPALIVE;

            /* get connection */
            request_buffer += 11;
            while (*request_buffer == ' ') request_buffer ++;

            if (str_begin_with(request_buffer, "close"))
                request->connection = WEBNET_CONN_CLOSE;
            else if (str_begin_with(request_buffer, "Keep-Alive"))
                request->connection = WEBNET_CONN_KEEPALIVE;
        }
#endif
#ifdef WEBNET_USING_COOKIE
        else if (str_begin_with(request_buffer, "Cookie:"))
        {
            /* get cookie */
            request_buffer += 7;
            while (*request_buffer == ' ') request_buffer ++;
            request->cookie = request_buffer;
        }
#endif /* WEBNET_USING_COOKIE */
#ifdef WEBNET_USING_AUTH
        else if (str_begin_with(request_buffer, "Authorization: Basic"))
        {
            /* get authorization */
            request_buffer += 20;
            while (*request_buffer == ' ') request_buffer ++;
            request->authorization = request_buffer;
        }
#endif /* WEBNET_USING_AUTH */
#if WEBNET_CACHE_LEVEL > 0
        else if (str_begin_with(request_buffer, "If-Modified-Since:"))
        {
            /* get If-Modified-Since */
            request_buffer += 18;
            while (*request_buffer == ' ') request_buffer ++;
            request->modified = request_buffer;
        }
#endif /* WEBNET_CACHE_LEVEL > 0 */
#ifdef WEBNET_USING_GZIP
        else if (str_begin_with(request_buffer, "Accept-Encoding:"))
        {
            const char *end = strstr(request_buffer, "\r\n");
            const char *gzip = strstr(request_buffer, "gzip");

            if( (gzip != RT_NULL) && (end != RT_NULL) && (gzip < end) )
            {
                request->support_gzip = RT_TRUE;
            }
        }
#endif /* WEBNET_USING_GZIP */
        else if (str_begin_with(request_buffer, "SOAPACTION:"))
        {
            request_buffer += 11;
            while (*request_buffer == ' ') request_buffer ++;

            request->soap_action = request_buffer;
        }
        else if (str_begin_with(request_buffer, "CALLBACK:"))
        {
            request_buffer += 9;
            while (*request_buffer == ' ') request_buffer ++;

            request->callback = request_buffer;
        }

        ch = strstr(request_buffer, "\r\n");
        if (ch == RT_NULL)
        {
            /* Bad Request */
            request->result_code = 400;
            return;
        }
        /* terminal field */
        *ch ++ = '\0';
        *ch ++ = '\0';

        request_buffer = ch;
    }
}

struct webnet_request* webnet_request_create()
{
    struct webnet_request* request;

    request = (struct webnet_request*) wn_malloc (sizeof(struct webnet_request));
    if (request != RT_NULL)
    {
        rt_memset(request, 0, sizeof(struct webnet_request));
        request->field_copied = RT_FALSE;
    }

    return request;
}

void webnet_request_destory(struct webnet_request* request)
{
    if (request != RT_NULL)
    {
        // if (request->field_copied == RT_TRUE)
        {
            if (request->path != RT_NULL) wn_free(request->path);
            if (request->host != RT_NULL) wn_free(request->host);
            if (request->cookie != RT_NULL) wn_free(request->cookie);
            if (request->user_agent != RT_NULL) wn_free(request->user_agent);
            if (request->authorization != RT_NULL) wn_free(request->authorization);
            if (request->accept_language != RT_NULL) wn_free(request->accept_language);
            if (request->referer != RT_NULL) wn_free(request->referer);
            if (request->content_type != RT_NULL) wn_free(request->content_type);
            if (request->query != RT_NULL) wn_free(request->query);
            if (request->query_items != RT_NULL) wn_free(request->query_items);

            if (request->callback) wn_free(request->callback);
            if (request->soap_action) wn_free(request->soap_action);
            if (request->sid) wn_free(request->sid);
#ifdef WEBNET_USING_RANGE
            if (request->Range) wn_free(request->Range);
#endif /* WEBNET_USING_RANGE */
#ifdef WEBNET_USING_DAV
            if (request->depth) wn_free(request->depth);
#endif
        }

        /* free request memory block */
        wn_free(request);
    }
}
