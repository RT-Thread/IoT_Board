/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-11     heyuanjie87  the first version
 */

#include <rtthread.h>
#include <string.h>
#include "transport.h"

#ifdef ADB_TR_TCPIP_ENABLE
#include <sys/socket.h>
#include <sys/time.h>
#include <dfs_select.h>
#include <dfs_poll.h>

//#define DBG_ENABLE
#define DBG_SECTION_NAME "ADB iosk"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

static int is_running = 0;

static int sk_read(int fd, void *buf, int size)
{
    return recv(fd, buf, size, 0);
}

static int sk_write(int fd, void *buf, int size)
{
    return send(fd, buf, size, 0);
}

static int sk_poll(int fd, int evt, int ms)
{
    struct pollfd pfd = {0};
    int ret;

    pfd.fd = fd;
    if (evt & TRE_READ)
        pfd.events |= POLLIN;
    if (evt & TRE_WRITE)
        pfd.events |= POLLOUT;

    ret = poll(&pfd, 1, ms);
    if (ret == 0)
        return 0;

    ret = 0;
    if (pfd.revents & POLLIN)
        ret |= TRE_READ;
    if (pfd.revents & POLLOUT)
        ret |= TRE_WRITE;
    if (pfd.revents & (POLLERR | POLLHUP))
        ret |= TRE_ERROR;

    return ret;
}

static void sk_close(int fd)
{
    closesocket(fd);
}

static const struct adb_tr_ops _ops =
{
    sk_read,
    sk_write,
    sk_poll,
    sk_close,
};

static void tcp_server(void *arg)
{
    int ret;
    int sock, connected;
    struct sockaddr_in server_addr, client_addr;

    struct timeval timeout;
    fd_set readset;
    socklen_t sin_size = sizeof(struct sockaddr_in);
    int port;

    is_running = 1;
    port = (int)arg;

    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        LOG_E("Create socket error");
        goto __exit;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(server_addr.sin_zero), 0x0, sizeof(server_addr.sin_zero));

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        LOG_E("Unable to bind");
        goto __exit;
    }

    if (listen(sock, 1) == -1)
    {
        LOG_E("Listen error");
        goto __exit;
    }

    while (is_running)
    {
        FD_ZERO(&readset);
        FD_SET(sock, &readset);

        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        /* Wait for read or write */
        if (select(sock + 1, &readset, RT_NULL, RT_NULL, &timeout) == 0)
            continue;

        /* 接受一个客户端连接socket的请求，这个函数调用是阻塞式的 */
        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
        /* 返回的是连接成功的socket */
        if (connected < 0)
        {
            LOG_E("accept connection failed! errno = %d", errno);
            continue;
        }
        LOG_D("accept connection");

        adb_transport_unregister(TR_TCPIP);
        ret = adb_transport_register(TR_TCPIP, connected, &_ops);
        if (ret != 0)
        {
            closesocket(connected);
            LOG_E("register transport tcpip fail");
        }
    }

__exit:
    if (sock >= 0)
    {
        closesocket(sock);
    }
    is_running = 0;
}

int adb_tcpip(int port)
{
    rt_thread_t tid;
    int ret;

    if (is_running)
    {
        LOG_E("adbd tcpip is exist");
    }

    tid = rt_thread_create("adbd-sk",
                           tcp_server,
                           (void *)port,
                           1024,
                           22,
                           20);
    if (tid)
    {
        ret = rt_thread_startup(tid);
    }

    return ret;
}

int adb_socket_init(void)
{
    int ret;

    ret = adb_tcpip(5555);

    return ret;
}
INIT_APP_EXPORT(adb_socket_init);
#endif
