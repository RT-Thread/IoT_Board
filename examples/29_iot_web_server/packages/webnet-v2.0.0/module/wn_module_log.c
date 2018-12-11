/*
 * File      : wn_module_log.c
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
 */
#include <webnet.h>
#include <wn_module.h>
#include <wn_utils.h>

#ifdef RT_USING_SAL
#include <arpa/inet.h>
#else
#include <lwip/inet.h>
#endif /* RT_USING_SAL */

#define DBG_ENABLE
#define DBG_COLOR
#define DBG_SECTION_NAME    "wn.log"
#define DBG_LEVEL           DBG_LOG
#include <rtdbg.h>

#ifdef WEBNET_USING_LOG

int webnet_module_log(struct webnet_session* session, int event)
{
    struct webnet_request* request;

    if (session != RT_NULL)
    {
        request = session->request;
    }
    else
    {
        request = RT_NULL;
    }

    if (event == WEBNET_EVENT_INIT)
    {
        LOG_D("server initialize success.");
    }
    else if (event == WEBNET_EVENT_URI_PHYSICAL)
    {
        rt_uint32_t index;
        LOG_RAW("\n");
        LOG_D("  new client: %s:%u",
                   inet_ntoa(session->cliaddr.sin_addr),
                   ntohs(session->cliaddr.sin_port));

        switch (request->method)
        {
        case WEBNET_GET:
            LOG_D("      method: GET");
            break;
        case WEBNET_PUT:
            LOG_D("      method: PUT");
            break;
        case WEBNET_POST:
            LOG_D("      method: POST");
            break;
        case WEBNET_HEADER:
            LOG_D("      method: HEADER");
            break;
        case WEBNET_SUBSCRIBE:
            LOG_D("      method: SUBSCRIBE");
            break;
        case WEBNET_UNSUBSCRIBE:
            LOG_D("      method: UNSUBSCRIBE");
            break;
        default:
            break;
        }
        LOG_D("     request: %s", request->path);
        for (index = 0; index < request->query_counter; index ++)
        {
            LOG_D("    query[%d]: %s => %s", index,
                       request->query_items[index].name,
                       request->query_items[index].value);
        }
    }
    else if (event == WEBNET_EVENT_URI_POST)
    {
        LOG_D("physical url: %s", request->path);
        LOG_D("   mime type: %s", mime_get_type(request->path));
    }
    else if (event == WEBNET_EVENT_RSP_HEADER)
    {
    }
    else if (event == WEBNET_EVENT_RSP_FILE)
    {
    }

    return WEBNET_MODULE_CONTINUE;
}

#endif /* WEBNET_USING_LOG */
