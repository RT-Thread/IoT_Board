/*
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rtthread.h>

#include "tls_client.h"
#include "tls_certificate.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#define rt_kprintf rt_kprintf("[tls]");rt_kprintf

#if defined(MBEDTLS_DEBUG_C)
#define DEBUG_LEVEL (2)
#endif

static void _ssl_debug(void *ctx, int level, const char *file, int line, const char *str)
{
    ((void) level);

    rt_kprintf("%s:%04d: %s", file, line, str);
}

int mbedtls_client_init(MbedTLSSession *session, void *entropy, size_t entropyLen)
{
    int ret = 0;

#if defined(MBEDTLS_DEBUG_C)
    rt_kprintf("[tls] Set debug level (%d)", (int)DEBUG_LEVEL);
    mbedtls_debug_set_threshold((int)DEBUG_LEVEL);
#endif

    mbedtls_net_init(&session->server_fd);
    mbedtls_ssl_init(&session->ssl);
    mbedtls_ssl_config_init(&session->conf);
    mbedtls_ctr_drbg_init(&session->ctr_drbg);
    mbedtls_entropy_init(&session->entropy);
    mbedtls_x509_crt_init(&session->cacert);
    
    if((ret = mbedtls_ctr_drbg_seed(&session->ctr_drbg, mbedtls_entropy_func, &session->entropy,
                                    (unsigned char *)entropy, entropyLen)) != 0)
    {
        rt_kprintf("mbedtls_ctr_drbg_seed returned -0x%x\n", -ret);
        return ret;
    }
    rt_kprintf("mbedtls client struct init success...\n");

    return RT_EOK;
}

int mbedtls_client_close(MbedTLSSession *session)
{
    mbedtls_ssl_close_notify(&session->ssl);
    mbedtls_net_free(&session->server_fd);
    mbedtls_x509_crt_free(&session->cacert);
    mbedtls_entropy_free(&session->entropy);
    mbedtls_ctr_drbg_free(&session->ctr_drbg);
    mbedtls_ssl_config_free(&session->conf);
    mbedtls_ssl_free(&session->ssl);
    
    if(session->buffer)
        tls_free(session->buffer);

    if(session->host)
        tls_free(session->host);
    
    if(session->port)
        tls_free(session->port);
    
    if(session)
    {   
        tls_free(session);
        session = RT_NULL;
    }
    
    return RT_EOK;
}

int mbedtls_client_context(MbedTLSSession *session)
{
    int ret = 0;

    rt_kprintf("Loading the CA root certificate success...\n");
    
    ret = mbedtls_x509_crt_parse(&session->cacert, (const unsigned char *)mbedtls_root_certificate,
                                 mbedtls_root_certificate_len);
    if(ret < 0)
    {
        rt_kprintf("mbedtls_x509_crt_parse err returned -0x%x\n", -ret);
       return ret;
    }

    /* Hostname set here should match CN in server certificate */
    if((ret = mbedtls_ssl_set_hostname(&session->ssl, session->host)) != 0)
    {
        rt_kprintf("mbedtls_ssl_set_hostname err returned -0x%x\n", -ret);
        return ret;
    }
    
    if((ret = mbedtls_ssl_config_defaults(&session->conf,
                                          MBEDTLS_SSL_IS_CLIENT,
                                          MBEDTLS_SSL_TRANSPORT_STREAM,
                                          MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        rt_kprintf("mbedtls_ssl_config_defaults returned -0x%x\n", -ret);
        return ret;
    }
    
    mbedtls_ssl_conf_authmode(&session->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&session->conf, &session->cacert, NULL);
    mbedtls_ssl_conf_rng(&session->conf, mbedtls_ctr_drbg_random, &session->ctr_drbg);

    mbedtls_ssl_conf_dbg(&session->conf, _ssl_debug, NULL);

    if ((ret = mbedtls_ssl_setup(&session->ssl, &session->conf)) != 0)
    {
        rt_kprintf("mbedtls_ssl_setup returned -0x%x\n", -ret);
        return ret;
    }
    rt_kprintf("mbedtls client context init success...\n");
        
    return RT_EOK;
}

int mbedtls_client_connect(MbedTLSSession *session)
{
    int ret = 0;

    if ((ret = mbedtls_net_connect(&session->server_fd, session->host,
                                  session->port, MBEDTLS_NET_PROTO_TCP)) != 0)
    {
        rt_kprintf("mbedtls_net_connect err returned -0x%x\n", -ret);
        return ret;
    }

    rt_kprintf("Connected %s:%s success...\n", session->host, session->port);

    mbedtls_ssl_set_bio(&session->ssl, &session->server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    while ((ret = mbedtls_ssl_handshake(&session->ssl)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            rt_kprintf("mbedtls_ssl_handshake returned -0x%x\n", -ret);
            return ret;
        }
    }

    if ((ret = mbedtls_ssl_get_verify_result(&session->ssl)) != 0)
    {
        rt_kprintf("verify peer certificate fail....\n");
        memset(session->buffer, 0x00, session->buffer_len);
        mbedtls_x509_crt_verify_info((char *)session->buffer, session->buffer_len, "  ! ", ret);
        rt_kprintf("verification info: %s\n", session->buffer);
    }
    else 
    {
        rt_kprintf("Certificate verified success...\n");
    }

    return RT_EOK;
}

int mbedtls_client_read(MbedTLSSession *session, unsigned char *buf , size_t len)
{
    if(!session || !buf)
        return RT_ERROR;

    return mbedtls_ssl_read(&session->ssl, (unsigned char *)buf, len);
}

int mbedtls_client_write(MbedTLSSession *session, const unsigned char *buf , size_t len)
{
    if(!session || !buf)
        return RT_ERROR;

    return mbedtls_ssl_write(&session->ssl, (unsigned char *)buf, len);
}
