/*
 * File      : tls_app_test.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date          Author          Notes
 * 2018-01-22    chenyong     first version
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rtthread.h>

#include <tls_certificate.h>
#include <tls_client.h>

#if !defined(MBEDTLS_CONFIG_FILE)
#include <mbedtls/config.h>
#else
#include MBEDTLS_CONFIG_FILE
#endif

// https://www.rt-thread.org/download/rt-thread.txt
#define MBEDTLS_WEB_SERVER  "www.rt-thread.org"
#define MBEDTLS_WEB_PORT    "443"

#define MBEDTLS_READ_BUFFER 1024

static const char *REQUEST = "GET /download/rt-thread.txt HTTP/1.1\r\n"
    "Host: www.rt-thread.org\r\n"
    "User-Agent: rtthread/3.1 rtt\r\n"
    "\r\n";

static void mbedtls_client_entry(void *parament)
{
    int ret = 0;
    MbedTLSSession *tls_session = RT_NULL;
    char *pers = "hello_world";

    rt_kprintf("MbedTLS test sample!\r\n");

#ifdef FINSH_USING_MSH
    rt_kprintf("Memory usage before the handshake connection is established:\r\n");
    msh_exec("free", rt_strlen("free"));
#endif
    tls_session = (MbedTLSSession *) tls_malloc(sizeof(MbedTLSSession));
    if (tls_session == RT_NULL)
    {
        rt_kprintf("No memory for MbedTLS session object.\n");
        return;
    }

    tls_session->host = tls_strdup(MBEDTLS_WEB_SERVER);
    tls_session->port = tls_strdup(MBEDTLS_WEB_PORT);

    tls_session->buffer_len = MBEDTLS_READ_BUFFER;
    tls_session->buffer = tls_malloc(tls_session->buffer_len);
    if (tls_session->buffer == RT_NULL)
    {
        rt_kprintf("No memory for MbedTLS buffer\n");
        tls_free(tls_session);
        return;
    }

    rt_kprintf("Start handshake tick:%d\n", rt_tick_get());

    if ((ret = mbedtls_client_init(tls_session, (void *) pers, rt_strlen(pers))) != 0)
    {
        rt_kprintf("MbedTLSClientInit err return : -0x%x\n", -ret);
        goto __exit;
    }

    if ((ret = mbedtls_client_context(tls_session)) < 0)
    {
        rt_kprintf("MbedTLSCLlientContext err return : -0x%x\n", -ret);
        goto __exit;
    }

    if ((ret = mbedtls_client_connect(tls_session)) != 0)
    {
        rt_kprintf("MbedTLSCLlientConnect err return : -0x%x\n", -ret);
        goto __exit;
    }

    rt_kprintf("Finish handshake tick:%d\n", rt_tick_get());

    rt_kprintf("MbedTLS connect success...\n");

#ifdef FINSH_USING_MSH
    rt_kprintf("Memory usage after the handshake connection is established:\r\n");
    msh_exec("free", rt_strlen("free"));
#endif

    while ((ret = mbedtls_client_write(tls_session, (const unsigned char *) REQUEST, rt_strlen(REQUEST))) <= 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            rt_kprintf("mbedtls_ssl_write returned -0x%x\n", -ret);
            goto __exit;
        }
    }
    rt_kprintf("Writing HTTP request success...\n");

    rt_kprintf("Getting HTTP response...\n");
    {
        int i = 0;

        rt_memset(tls_session->buffer, 0x00, MBEDTLS_READ_BUFFER);
        ret = mbedtls_client_read(tls_session, (unsigned char *) tls_session->buffer, MBEDTLS_READ_BUFFER);
        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE
                || ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
            goto __exit;

        if (ret < 0)
        {
            rt_kprintf("Mbedtls_ssl_read returned -0x%x\n", -ret);
            goto __exit;
        }

        if (ret == 0)
        {
            rt_kprintf("TCP server connection closed.\n");
            goto __exit;
        }

        for (i = 0; i < ret; i++)
            rt_kprintf("%c", tls_session->buffer[i]);

        rt_kprintf("\n");
    }

__exit:
    mbedtls_client_close(tls_session);

    rt_kprintf("MbedTLS connection close success.\n");

    return;
}

int mbedtls_client_start(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("tls_c", mbedtls_client_entry, RT_NULL, 6 * 1024, RT_THREAD_PRIORITY_MAX / 3 - 1, 5);
    if (tid)
    {
        rt_thread_startup(tid);
    }

    return RT_EOK;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT_ALIAS(mbedtls_client_start, tls_test, mbedtls client test start);
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(mbedtls_client_start, tls_test, mbedtls client test start);
#endif
#endif
