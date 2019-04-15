/*
 * File      : webnet.c
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
 * 2012-07-03     Bernard      Add webnet port and webroot setting interface.
 */

#include <stdint.h>
#include <string.h>

#include <webnet.h>
#include <wn_module.h>

#ifdef SAL_USING_POSIX
#include <sys/select.h>
#else
#include <lwip/select.h>
#endif

#define DBG_ENABLE
#define DBG_COLOR
#define DBG_SECTION_NAME    "wn"
#ifdef WEBNET_USING_LOG
#define DBG_LEVEL           DBG_LOG
#else
#define DBG_LEVEL           DBG_INFO
#endif /* WEBNET_USING_LOG */
#include <rtdbg.h>

static rt_uint16_t webnet_port = WEBNET_PORT;
static char webnet_root[64] = WEBNET_ROOT;
static rt_bool_t init_ok = RT_FALSE;

void webnet_set_port(int port)
{
    RT_ASSERT(init_ok == RT_FALSE);
    webnet_port = port;
}

int webnet_get_port(void)
{
    return webnet_port;
}

void webnet_set_root(const char* webroot_path)
{
    rt_strncpy(webnet_root, webroot_path, sizeof(webnet_root) - 1);
    webnet_root[sizeof(webnet_root) - 1] = '\0';
}

const char* webnet_get_root(void)
{
    return webnet_root;
}

/**
 * webnet thread entry
 */
static void webnet_thread(void *parameter)
{
    int listenfd = -1;
    fd_set readset, tempfds;
    fd_set writeset, tempwrtfds;
    int sock_fd, maxfdp1;
    struct sockaddr_in webnet_saddr;

    /* First acquire our socket for listening for connections */
    listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenfd < 0)
    {
        LOG_E("Create socket failed.");
        goto __exit;
    }

    rt_memset(&webnet_saddr, 0, sizeof(webnet_saddr));
    webnet_saddr.sin_family = AF_INET;
    webnet_saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    webnet_saddr.sin_port = htons(webnet_port); /* webnet server port */

    if (bind(listenfd, (struct sockaddr *) &webnet_saddr, sizeof(webnet_saddr)) == -1)
    {
        LOG_E("Bind socket failed, errno=%d\n", errno);
        goto __exit;
    }

    /* Put socket into listening mode */
    if (listen(listenfd, WEBNET_CONN_MAX) == -1)
    {
        LOG_E("Socket listen(%d) failed.", WEBNET_CONN_MAX);
        goto __exit;
    }

    /* initialize module (no session at present) */
    webnet_module_handle_event(RT_NULL, WEBNET_EVENT_INIT);

    /* Wait forever for network input: This could be connections or data */
    for (;;)
    {
        /* Determine what sockets need to be in readset */
        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_SET(listenfd, &readset);

        /* set fds in each sessions */
        maxfdp1 = webnet_sessions_set_fds(&readset, &writeset);
        if (maxfdp1 < listenfd + 1)
        {
            maxfdp1 = listenfd + 1;
        }

        /* use temporary fd set in select */
        tempfds = readset;
        tempwrtfds = writeset;
        /* Wait for data or a new connection */
        sock_fd = select(maxfdp1, &tempfds, &tempwrtfds, 0, 0);
        if (sock_fd == 0)
        {
            continue;
        }

        /* At least one descriptor is ready */
        if (FD_ISSET(listenfd, &tempfds))
        {
            struct webnet_session* accept_session;
            /* We have a new connection request */
            accept_session = webnet_session_create(listenfd);
            if (accept_session == RT_NULL)
            {
                /* create session failed, just accept and then close */
                int sock;
                struct sockaddr cliaddr;
                socklen_t clilen;

                clilen = sizeof(struct sockaddr_in);
                sock = accept(listenfd, &cliaddr, &clilen);
                if (sock >= 0)
                {
                    closesocket(sock);
                }
            }
            else
            {
                /* add read fdset */
                FD_SET(accept_session->socket, &readset);
            }
        }

        webnet_sessions_handle_fds(&tempfds, &writeset);
    }

__exit:
    if (listenfd >= 0)
    {
        closesocket(listenfd);
    }
}

int webnet_init(void)
{
    rt_thread_t tid;

    if (init_ok == RT_TRUE)
    {
        LOG_I("RT-Thread webnet package is already initialized.");
        return 0;
    }

    tid = rt_thread_create(WEBNET_THREAD_NAME,
                           webnet_thread, RT_NULL,
                           WEBNET_THREAD_STACKSIZE, WEBNET_PRIORITY, 5);

    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
        init_ok = RT_TRUE;
        LOG_I("RT-Thread webnet package (V%s) initialize success.", WEBNET_VERSION);
    }
    else
    {
        LOG_E("RT-Thread webnet package (V%s) initialize failed.", WEBNET_VERSION);
        return -1;
    }

    return 0;
}
