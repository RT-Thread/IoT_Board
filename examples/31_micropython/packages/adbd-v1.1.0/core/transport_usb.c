/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-12-04     heyuanjie87  the first version
 */

#include <rtthread.h>
#include <dfs_posix.h>
#include <dfs_poll.h>
#include "transport.h"

//#define DBG_ENABLE
#define DBG_SECTION_NAME "ADB usb"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

#ifdef ADB_TR_USB_ENABLE
static int _closefd = 0;
static int is_running = 0;

static int sk_read(int fd, void *buf, int size)
{
    return read(fd, buf, size);
}

static int sk_write(int fd, void *buf, int size)
{
    return write(fd, buf, size);
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
    close(fd);
    _closefd = fd;
}

static const struct adb_tr_ops _ops =
{
    sk_read,
    sk_write,
    sk_poll,
    sk_close,
};

static void usb_daemon(void *arg)
{
    int fd;

    if (is_running)
        return;
    is_running = 1;

_wait:
    rt_thread_mdelay(2000);
    fd = open("/dev/winusb", O_RDWR);
    if (fd < 0)
        goto _wait;
    if (sk_poll(fd, TRE_READ, 0) & TRE_ERROR)
    {
        close(fd);
        goto _wait;
    }

    adb_transport_unregister(TR_USB);
    if (adb_transport_register(TR_USB, fd, &_ops) != 0)
    {
        close(fd);
        LOG_E("register transport usb fail");
        goto _wait;
    }

    _closefd = -1;
    while (is_running)
    {
        if (_closefd != -1)
            goto _wait;

        rt_thread_mdelay(1000);
    }
}

int adb_usb_init(void)
{
    rt_thread_t tid;
    int ret;

    if (is_running)
    {
        LOG_E("adbd usb is exist");
    }

    tid = rt_thread_create("adbd-usb",
                           usb_daemon,
                           0,
                           1024,
                           22,
                           20);
    if (tid)
    {
        ret = rt_thread_startup(tid);
    }

    return ret;
}
INIT_APP_EXPORT(adb_usb_init);
#endif
