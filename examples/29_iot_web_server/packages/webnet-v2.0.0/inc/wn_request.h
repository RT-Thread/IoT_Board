/*
 * File      : wn_request.h
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
 * 2011-08-18     Bernard      added content length field
 */

#ifndef __WN_REQUEST_H__
#define __WN_REQUEST_H__

#include <rtthread.h>

#include <wn_session.h>

#ifdef __cplusplus
extern "C" {
#endif

/* http request method */
enum webnet_method
{
    WEBNET_UNKNOWN = 0,
    WEBNET_GET,
    WEBNET_POST,
    WEBNET_HEADER,
    WEBNET_HEAD,
	WEBNET_PUT,
	WEBNET_OPTIONS,
	WEBNET_PROPFIND,
	WEBNET_PROPPATCH,
	WEBNET_DELETE,
	WEBNET_CONNECT,
	WEBNET_MKCOL,
	WEBNET_MOVE,
    WEBNET_SUBSCRIBE,
    WEBNET_UNSUBSCRIBE,
    WEBNET_NOTIFY,
};

/* http connection status */
enum webnet_connection
{
    WEBNET_CONN_CLOSE,
    WEBNET_CONN_KEEPALIVE,
};

/* http request structure */
struct webnet_request
{
    enum webnet_method method;
    int result_code;
    int content_length;

    /* the corresponding session */
    struct webnet_session *session;

    /* path and authorization */
    char* path;
    char* host;
    char* authorization;
#if WEBNET_CACHE_LEVEL > 0
    char* modified;
#endif /* WEBNET_CACHE_LEVEL */
#ifdef WEBNET_USING_GZIP
    rt_bool_t support_gzip;
#endif /* WEBNET_USING_GZIP */

    char* user_agent;
    char* accept_language;
    char* cookie;
    char* referer;
#ifdef WEBNET_USING_RANGE
    char *Range;
    size_t pos_start;
    size_t pos_end;
#endif /* WEBNET_USING_RANGE */
#ifdef WEBNET_USING_DAV
	char* depth;
	char* destination;
#endif /* WEBNET_USING_DAV */

    /* DMR */
    char *soap_action;
    char *callback;
    char *sid;

    /* Content-Type */
    char* content_type;

    /* query information */
    char* query;
	int query_offset;
    struct webnet_query_item* query_items;
    rt_uint16_t query_counter;

    enum webnet_connection connection;

    /* whether the string filed is copied */
    rt_bool_t field_copied;
};

struct webnet_request* webnet_request_create(void);
void webnet_request_destory(struct webnet_request* request);

int webnet_request_parse_method(struct webnet_request *request, char* buffer, int length);
int webnet_request_parse_header(struct webnet_request *request, char* buffer, int length);
int webnet_request_parse_post(struct webnet_request* request, char* buffer, int length);

void webnet_request_parse(struct webnet_request* request, char* buffer, int length);

rt_bool_t webnet_request_has_query(struct webnet_request* request, char* name);
const char* webnet_request_get_query(struct webnet_request* request, char* name);

#ifdef  __cplusplus
    }
#endif

#endif /* __WN_REQUEST_H__ */
