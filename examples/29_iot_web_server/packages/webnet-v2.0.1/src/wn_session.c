/*
 * File      : wn_session.c
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
 * 2012-06-25     Bernard      fixed the close session issue.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <webnet.h>
#include <wn_module.h>
#include <wn_utils.h>

static struct webnet_session *_session_list = 0;

static void (*webnet_err_callback)(struct webnet_session *session);

/**
 * create a webnet session
 *
 * @param listenfd, the listen file descriptor
 *
 * @return the created web session
 */
struct webnet_session* webnet_session_create(int listenfd)
{
    struct webnet_session* session;

    /* create a new session */
    session = (struct webnet_session *)wn_malloc(sizeof(struct webnet_session));
    if (session != RT_NULL)
    {
        socklen_t clilen;
		
        memset(session, 0x0, sizeof(struct webnet_session));
        session->session_ops = RT_NULL;

        clilen = sizeof(struct sockaddr_in);
        session->socket = accept(listenfd, (struct sockaddr *) &(session->cliaddr), &clilen);
        if (session->socket < 0)
        {
            wn_free(session);
            session = RT_NULL;

            return session;
        }
        else
        {
            /* keep this session in our list */
            session->next = _session_list;
            _session_list = session;
        }

        /* initial buffer length */
        session->buffer_length = WEBNET_SESSION_BUFSZ;
    }

    return session;
}

/**
 * read data from a webnet session
 *
 * @param session, the web session
 * @param buffer, the buffer to save read data
 * @param length, the maximal length of data buffer to save read data
 *
 * @return the number of bytes actually read data
 */
int webnet_session_read(struct webnet_session *session, char *buffer, int length)
{
    int read_count;

    /* Read some data */
    read_count = recvfrom(session->socket, buffer, length, 0, NULL, NULL);
    if (read_count <= 0)
    {
        session->session_phase = WEB_PHASE_CLOSE;
        return -1;
    }

    return read_count;
}
RTM_EXPORT(webnet_session_read);

/**
 * close a webnet session
 *
 * @param session, the web session
 */
void webnet_session_close(struct webnet_session *session)
{
    struct webnet_session *iter;

    /* invoke session close */
    if (session->session_ops != RT_NULL &&
            session->session_ops->session_close != RT_NULL)
    {
        session->session_ops->session_close(session);
    }

    /* Either an error or tcp connection closed on other
     * end. Close here */
    closesocket(session->socket);

    /* Free webnet_session */
    if (_session_list == session)
        _session_list = session->next;
    else
    {
        for (iter = _session_list; iter; iter = iter->next)
        {
            if (iter->next == session)
            {
                iter->next = session->next;
                break;
            }
        }
    }

    if (session->request != RT_NULL)
    {
        webnet_request_destory(session->request);
        session->request = RT_NULL;
    }

    wn_free(session);
}

/**
 * print formatted data to session
 *
 * @param session, the web session
 * @param fmt, the format string
 */
void webnet_session_printf(struct webnet_session *session, const char* fmt, ...)
{
    va_list args;
    rt_uint32_t length;

    va_start(args, fmt);
    length = vsnprintf((char*)(session->buffer),
                       sizeof(session->buffer) - 1,
                       fmt, args);
    session->buffer[length] = '\0';
    va_end(args);

    send(session->socket, session->buffer, length, 0);
}
RTM_EXPORT(webnet_session_printf);

/**
 * write data to session
 *
 * @param session, the web session
 * @param data, the data will be write to the session
 * @param size, the size of data bytes
 *
 * @return the number of bytes actually written to the session
 */
int webnet_session_write(struct webnet_session *session, const rt_uint8_t* data, rt_size_t size)
{
    /* send data directly */
    send(session->socket, data, size, 0);

    return size;
}
RTM_EXPORT(webnet_session_write);

/**
 * redirect to another local URL
 *
 * @param session, the web session
 * @param url, the new URL link on the local
 *
 * @return
 */
int webnet_session_redirect(struct webnet_session *session, const char* url)
{
    struct webnet_request *request;

    RT_ASSERT(session != RT_NULL);
    request = session->request;
    RT_ASSERT(request != RT_NULL);

    /* change the request path to URL */
    if (request->path != RT_NULL)
        wn_free(request->path);
    request->path = wn_strdup(url);

    /* handle this URL */
    return webnet_module_handle_uri(session);
}
RTM_EXPORT(webnet_session_redirect);

/**
 * Get physical path according to a virtual path
 *
 * @param session, the webnet session
 * @param virtual_path, the virtual path
 * @param full_path, the output full path
 *
 * @return 0 on convert successfull.
 *
 * NOTE: the length of full path is WEBNET_PATH_MAX */
int webnet_session_get_physical_path(struct webnet_session *session, const char* virtual_path, char* full_path)
{
    int result;

    result = 0;
    if (full_path == RT_NULL) return -1;

    /* made a full path */
    rt_snprintf(full_path, WEBNET_PATH_MAX, "%s/%s", webnet_get_root(), virtual_path);
    /* normalize path */
    str_normalize_path(full_path);

    /* check URI valid */
    if (!str_begin_with(full_path, webnet_get_root()))
    {
        /* not found */
        result = -1;
    }

    return result;
}
RTM_EXPORT(webnet_session_get_physical_path);

/**
 * set the http response header field Status-Line.
 *
 * @param session the web session
 * @param code the http response code
 * @param reason_phrase reason phrase string
 */
void webnet_session_set_header_status_line(struct webnet_session *session,
        int code,
        const char * reason_phrase)
{
    char status_line[16]; /* "HTTP/1.1 ### " */

    rt_snprintf(status_line, sizeof(status_line) - 1, "HTTP/1.1 %d ", code);
    webnet_session_printf(session, status_line);
    webnet_session_printf(session, reason_phrase);
    webnet_session_printf(session, "\r\n");
    webnet_session_printf(session, WEBNET_SERVER);
}
RTM_EXPORT(webnet_session_set_header_status_line);

/**
 * set the http response header
 *
 * @param session the web session
 * @param mimetype the mime type of http response
 * @param code the http response code
 * @param title the code title string
 * @param length the length of http response content
 */
void webnet_session_set_header(struct webnet_session *session, const char* mimetype, int code, const char* title, int length)
{
    static const char* fmt = "HTTP/1.1 %d %s\r\n%s";
    static const char* content = "Content-Type: %s\r\nContent-Length: %ld\r\nConnection: %s\r\n\r\n";
    static const char* content_nolength = "Content-Type: %s\r\nConnection: %s\r\n\r\n";
    static const char* auth = "WWW-Authenticate: Basic realm=%s\r\n";

    char *ptr, *end_buffer;
    int offset;

    ptr = (char*)session->buffer;
    end_buffer = (char*)session->buffer + session->buffer_length;

    offset = rt_snprintf(ptr, end_buffer - ptr, fmt, code, title, WEBNET_SERVER);
    ptr += offset;

    if (code == 401)
    {
        offset = rt_snprintf(ptr, end_buffer - ptr, auth, session->request->host);
        ptr += offset;
    }
    if (length >= 0)
    {
        offset = rt_snprintf(ptr, end_buffer - ptr, content, mimetype, length,
                             session->request->connection == WEBNET_CONN_CLOSE? "close" : "Keep-Alive");
        ptr += offset;
    }
    else
    {
        offset = rt_snprintf(ptr, end_buffer - ptr, content_nolength, mimetype, "close");
        ptr += offset;
    }
    /* get the total length */
    length = ptr - (char*)session->buffer;
	
    /* invoke webnet event */
    if (webnet_module_handle_event(session, WEBNET_EVENT_RSP_HEADER) == WEBNET_MODULE_CONTINUE)
    {
        /* write to session */
        webnet_session_write(session, session->buffer, length);
    }
}
RTM_EXPORT(webnet_session_set_header);

static void _webnet_session_handle_read(struct webnet_session* session)
{
    int read_length;
    rt_uint8_t *buffer_ptr;

    buffer_ptr = &session->buffer[session->buffer_offset];
    /* to read data from the socket */
    read_length = webnet_session_read(session, (char*)buffer_ptr, session->buffer_length - session->buffer_offset);
	
    if (read_length > 0) session->buffer_offset += read_length;

    if (session->buffer_offset)
    {
        /* parse web method phase */
        if (session->session_phase == WEB_PHASE_METHOD)
        {
            int length = webnet_request_parse_method(session->request, (char*)&session->buffer[0],
                session->buffer_offset);

            if (length)
            {
                if (length < session->buffer_offset)
                {
                    session->buffer_offset -= length;
                    /* move to the begining of buffer */
                    memmove(session->buffer, &session->buffer[length], session->buffer_offset);
                }
                else session->buffer_offset = 0;
            }
        }

        /* parse web request header phase */
        if (session->session_phase == WEB_PHASE_HEADER)
        {
            int length = webnet_request_parse_header(session->request, (char*)&session->buffer[0],
                session->buffer_offset);

            if (length)
            {
                if (length < session->buffer_offset)
                {
                    session->buffer_offset -= length;
                    /* move to the begining of buffer */
                    memmove(session->buffer, &session->buffer[length], session->buffer_offset);
                }
                else session->buffer_offset = 0;
            }
        }

        if (session->session_phase == WEB_PHASE_QUERY)
        {
            int length = webnet_request_parse_post(session->request, (char*)&session->buffer[0],
                session->buffer_offset);

            if (length)
            {
                if (length < session->buffer_offset)
                {
                    session->buffer_offset -= length;
                    /* move to the begining of buffer */
                    memmove(session->buffer, &session->buffer[length], session->buffer_offset);
                }
                else session->buffer_offset = 0;
            }
        }
    }
}

static void _webnet_session_handle_write(struct webnet_session* session)
{
}

static void _webnet_session_handle(struct webnet_session* session, int event)
{
    switch (event)
    {
    case WEBNET_EVENT_READ:
        _webnet_session_handle_read(session);

        /* in the response phase */
        if (session->session_phase == WEB_PHASE_RESPONSE)
        {
            /* remove the default session ops */
            session->session_ops = NULL;
            session->user_data = 0;

            /* to handle response, then let module to handle url */
            webnet_module_handle_uri(session);
        }
        break;

    case WEBNET_EVENT_WRITE:
        _webnet_session_handle_write(session);
        break;
    }
}

const struct webnet_session_ops _default_session_ops =
{
    _webnet_session_handle,
    RT_NULL
};

static void _webnet_session_badrequest(struct webnet_session *session, int code)
{
    const char* title;
    static const char* fmt = "<html><head><title>%d %s</title></head><body>%d %s</body></html>\r\n";

    title = "Unknown";
    switch (code)
    {
    case 304:
        title = "Not Modified";
        break;
    case 400:
        title = "Bad Request";
        break;
    case 401:
        title = "Authorization Required";
        break;
    case 403:
        title = "Forbidden";
        break;
    case 404:
        title = "Not Found";
        break;
    case 405:
        title = "Method Not Allowed";
        break;
    case 500:
        title = "Internal Server Error";
        break;
    case 501:
        title = "Not Implemented";
        break;
    case 505:
        title = "HTTP Version Not Supported";
        break;
    }
#ifdef WEBNET_USING_KEEPALIVE
    if (code >= 400)
    {
        session->request->connection = WEBNET_CONN_CLOSE;
    }
#endif

    webnet_session_set_header(session, "text/html", code, title, -1);
    webnet_session_printf(session, fmt, code, title, code, title);
}

/**
 * set the file descriptors
 *
 * @param readset, the file descriptors set for read
 * @param writeset, the file descriptors set for write
 *
 * @return the maximal file descriptor
 */
int webnet_sessions_set_fds(fd_set *readset, fd_set *writeset)
{
    int maxfdp1 = 0;
    struct webnet_session *session;

    for (session = _session_list; session; session = session->next)
    {
        if (maxfdp1 < session->socket + 1)
            maxfdp1 = session->socket + 1;

        FD_SET(session->socket, readset);
        if (session->session_event_mask & WEBNET_EVENT_WRITE)
            FD_SET(session->socket, writeset);
    }

    return maxfdp1;
}

void webnet_sessions_set_err_callback(void (*callback)(struct webnet_session *session))
{
    webnet_err_callback = callback;
}

/**
 * handle the file descriptors request
 *
 * @param readset, the file descriptors set for read
 * @param writeset, the file descriptors set for write
 */
void webnet_sessions_handle_fds(fd_set *readset, fd_set *writeset)
{
    struct webnet_session *session, *next_session;

    /* Go through list of connected session and process data */
    for (session = _session_list; session; session = next_session)
    {
        /* get next session firstly if this session is closed */
        next_session = session->next;

        if (FD_ISSET(session->socket, readset))
        {
            if (session->session_ops == RT_NULL)
            {
                struct webnet_request *request;

                /* destroy old request */
                if (session->request != RT_NULL)
                {
                    webnet_request_destory(session->request);
                    session->request = RT_NULL;
                }

                /* create request and use the default session ops */
                request = webnet_request_create();
                if (request)
                {
                    session->request = request;
                    session->session_phase = WEB_PHASE_METHOD;
                    /* set the default session ops */
                    session->session_ops = &_default_session_ops;
                    session->user_data = RT_NULL;

                    request->session = session;
                    request->result_code = 200; /* set the default result code to 200 */

                    /* handle read event */
                    session->session_ops->session_handle(session, WEBNET_EVENT_READ);
                }
                else
                {
                    /* no memory, close this session */
                    session->session_phase = WEB_PHASE_CLOSE;
                }
            }
            else
            {
                if (session->session_ops->session_handle)
                    session->session_ops->session_handle(session, WEBNET_EVENT_READ);
            }

            /* whether close this session */
            if (session->session_ops == RT_NULL || session->session_phase == WEB_PHASE_CLOSE)
            {
                /* check result code */
                if (session->request->result_code != 200)
                {
                    /* do request err callback */
                    if (webnet_err_callback != RT_NULL)
                    {
                        webnet_err_callback(session);
                    }
                    else
                    {
                        _webnet_session_badrequest(session, session->request->result_code);
                    }
                }
                /* close this session */
                webnet_session_close(session);
            }
        }
        else if (FD_ISSET(session->socket, writeset))
        {
            /* handle for write fd set */
            if (session->session_ops != RT_NULL &&
                session->session_ops->session_handle != RT_NULL)
            {
                session->session_ops->session_handle(session, WEBNET_EVENT_WRITE);
            }

            /* whether close this session */
            if (session->session_ops == RT_NULL || session->session_phase == WEB_PHASE_CLOSE)
            {	
                /* close this session */
                webnet_session_close(session);
            }
        }
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>

static void list_webnet(void)
{
    struct webnet_session *session;
    char client_ip_str[16]; /* ###.###.###.### */
    rt_uint32_t num = 0;

    rt_enter_critical();
    for (session = _session_list; session != RT_NULL; session = session->next)
    {
        strcpy(client_ip_str,
               inet_ntoa(*((struct in_addr*)&(session->cliaddr.sin_addr))));

        rt_kprintf("#%u client %s:%u \n",
                   num++,
                   client_ip_str,
                   ntohs(session->cliaddr.sin_port));

        if (session->request != RT_NULL)
        {
            rt_kprintf("path: %s\n", session->request->path);
        }

        rt_kprintf("\r\n");
    }
    rt_exit_critical();
}
FINSH_FUNCTION_EXPORT(list_webnet, list webnet session);
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(list_webnet, list webnet session);
#endif
#endif /* RT_USING_FINSH */

