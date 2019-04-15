// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// DEPRECATED: the USE_MBED_TLS #define is deprecated.

#define USE_MBED_TLS

#ifdef USE_MBED_TLS

#include <stdlib.h>

#ifdef TIZENRT
#include "tls/config.h"
#include "tls/debug.h"
#include "tls/ssl.h"
#include "tls/entropy.h"
#include "tls/ctr_drbg.h"
#include "tls/error.h"
#include "tls/certs.h"
#include "tls/entropy_poll.h"
#else
#include "mbedtls/config.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/entropy_poll.h"
#endif

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/tlsio_mbedtls.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/shared_util_options.h"

#define OPTION_UNDERLYING_IO_OPTIONS        "underlying_io_options"

// DEPRECATED: debug functions do not belong in the tree.
//#define MBED_TLS_DEBUG_ENABLE

typedef enum TLSIO_STATE_ENUM_TAG
{
    TLSIO_STATE_NOT_OPEN,
    TLSIO_STATE_OPENING_UNDERLYING_IO,
    TLSIO_STATE_IN_HANDSHAKE,
    TLSIO_STATE_OPEN,
    TLSIO_STATE_CLOSING,
    TLSIO_STATE_ERROR
} TLSIO_STATE_ENUM;

typedef struct TLS_IO_INSTANCE_TAG
{
    XIO_HANDLE socket_io;
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    ON_IO_CLOSE_COMPLETE on_io_close_complete;
    ON_IO_ERROR on_io_error;
    void* on_bytes_received_context;
    void* on_io_open_complete_context;
    void* on_io_close_complete_context;
    void* on_io_error_context;
    TLSIO_STATE_ENUM tlsio_state;
    unsigned char* socket_io_read_bytes;
    size_t socket_io_read_byte_count;
    ON_SEND_COMPLETE on_send_complete;
    void* on_send_complete_callback_context;
    mbedtls_entropy_context    entropy;
    mbedtls_ctr_drbg_context   ctr_drbg;
    mbedtls_ssl_context        ssl;
    mbedtls_ssl_config         config;
    mbedtls_x509_crt           trusted_certificates_parsed;
    mbedtls_ssl_session        ssn;
    char*                      trusted_certificates;
} TLS_IO_INSTANCE;

static const IO_INTERFACE_DESCRIPTION tlsio_mbedtls_interface_description =
{
    tlsio_mbedtls_retrieveoptions,
    tlsio_mbedtls_create,
    tlsio_mbedtls_destroy,
    tlsio_mbedtls_open,
    tlsio_mbedtls_close,
    tlsio_mbedtls_send,
    tlsio_mbedtls_dowork,
    tlsio_mbedtls_setoption
};

// DEPRECATED: debug functions do not belong in the tree.
#if defined (MBED_TLS_DEBUG_ENABLE)
void mbedtls_debug(void *ctx, int level, const char *file, int line, const char *str)
{
    ((void)level);
    printf("%s (%d): %s\r\n", file, line, str);
}
#endif

static void indicate_error(TLS_IO_INSTANCE* tls_io_instance)
{
    if (tls_io_instance->on_io_error != NULL)
    {
        tls_io_instance->on_io_error(tls_io_instance->on_io_error_context);
    }
}

static void indicate_open_complete(TLS_IO_INSTANCE* tls_io_instance, IO_OPEN_RESULT open_result)
{
    if (tls_io_instance->on_io_open_complete != NULL)
    {
        tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, open_result);
    }
}

static int decode_ssl_received_bytes(TLS_IO_INSTANCE* tls_io_instance)
{
    int result = 0;
    unsigned char buffer[64];
    int rcv_bytes = 1;

    while (rcv_bytes > 0)
    {
        rcv_bytes = mbedtls_ssl_read(&tls_io_instance->ssl, buffer, sizeof(buffer));
        if (rcv_bytes > 0)
        {
            if (tls_io_instance->on_bytes_received != NULL)
            {
                tls_io_instance->on_bytes_received(tls_io_instance->on_bytes_received_context, buffer, rcv_bytes);
            }
        }
    }

    return result;
}

static void on_underlying_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;
    int result = 0;

    if (open_result != IO_OPEN_OK)
    {
        xio_close(tls_io_instance->socket_io, NULL, NULL);
        tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
        indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
    }
    else
    {
        tls_io_instance->tlsio_state = TLSIO_STATE_IN_HANDSHAKE;

        do {
            result = mbedtls_ssl_handshake(&tls_io_instance->ssl);
        } while (result == MBEDTLS_ERR_SSL_WANT_READ || result == MBEDTLS_ERR_SSL_WANT_WRITE);

        if (result == 0)
        {
            tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
            indicate_open_complete(tls_io_instance, IO_OPEN_OK);
        }
        else
        {
            xio_close(tls_io_instance->socket_io, NULL, NULL);
            tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
            indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
        }
    }
}

static void on_underlying_io_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    unsigned char* new_socket_io_read_bytes = (unsigned char*)realloc(tls_io_instance->socket_io_read_bytes, tls_io_instance->socket_io_read_byte_count + size);

    if (new_socket_io_read_bytes == NULL)
    {
        tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
        indicate_error(tls_io_instance);
    }
    else
    {
        tls_io_instance->socket_io_read_bytes = new_socket_io_read_bytes;
        (void)memcpy(tls_io_instance->socket_io_read_bytes + tls_io_instance->socket_io_read_byte_count, buffer, size);
        tls_io_instance->socket_io_read_byte_count += size;
    }
}

static void on_underlying_io_error(void* context)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    switch (tls_io_instance->tlsio_state)
    {
    default:
    case TLSIO_STATE_NOT_OPEN:
    case TLSIO_STATE_ERROR:
        break;

    case TLSIO_STATE_OPENING_UNDERLYING_IO:
    case TLSIO_STATE_IN_HANDSHAKE:
        // Existing socket impls are all synchronous close, and this 
        // adapter does not yet support async close.
        xio_close(tls_io_instance->socket_io, NULL, NULL);
        tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
        indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
        break;

    case TLSIO_STATE_OPEN:
        tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
        indicate_error(tls_io_instance);
        break;
    }
}

static void on_underlying_io_close_complete_during_close(void* context)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;

    if (tls_io_instance->on_io_close_complete != NULL)
    {
        tls_io_instance->on_io_close_complete(tls_io_instance->on_io_close_complete_context);
    }
}

static int on_io_recv(void *context, unsigned char *buf, size_t sz)
{
    int result;
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;
    unsigned char* new_socket_io_read_bytes;

    while (tls_io_instance->socket_io_read_byte_count == 0)
    {
        xio_dowork(tls_io_instance->socket_io);
        if (tls_io_instance->tlsio_state == TLSIO_STATE_OPEN)
        {
            break;
        }
    }

    result = tls_io_instance->socket_io_read_byte_count;
    if (result > sz)
    {
        result = sz;
    }

    if (result > 0)
    {
        (void)memcpy((void *)buf, tls_io_instance->socket_io_read_bytes, result);
        (void)memmove(tls_io_instance->socket_io_read_bytes, tls_io_instance->socket_io_read_bytes + result, tls_io_instance->socket_io_read_byte_count - result);
        tls_io_instance->socket_io_read_byte_count -= result;
        if (tls_io_instance->socket_io_read_byte_count > 0)
        {
            new_socket_io_read_bytes = (unsigned char*)realloc(tls_io_instance->socket_io_read_bytes, tls_io_instance->socket_io_read_byte_count);
            if (new_socket_io_read_bytes != NULL)
            {
                tls_io_instance->socket_io_read_bytes = new_socket_io_read_bytes;
            }
        }
        else
        {
            free(tls_io_instance->socket_io_read_bytes);
            tls_io_instance->socket_io_read_bytes = NULL;
        }
    }


    if ((result == 0) && (tls_io_instance->tlsio_state == TLSIO_STATE_OPEN))
    {
        result = MBEDTLS_ERR_SSL_WANT_READ;
    }

    return result;
}

static int on_io_send(void *context, const unsigned char *buf, size_t sz)
{
    int result;
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    if (xio_send(tls_io_instance->socket_io, buf, sz, tls_io_instance->on_send_complete, tls_io_instance->on_send_complete_callback_context) != 0)
    {
        tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
        indicate_error(tls_io_instance);
        result = 0;
    }
    else
    {
        result = sz;
    }

    return result;
}

static int tlsio_entropy_poll(void *v, unsigned char *output, size_t len, size_t *olen)
{
    srand(time(NULL));
    char *c = (char*)malloc(len);
    memset(c, 0, len);
    for (uint16_t i = 0; i < len; i++) {
        c[i] = rand() % 256;
    }
    memmove(output, c, len);
    *olen = len;

    free(c);
    return(0);
}

static void mbedtls_init(void *instance, const char *host) {
    TLS_IO_INSTANCE *result = (TLS_IO_INSTANCE *)instance;
    char *pers = "azure_iot_client";

    // mbedTLS initialize...
    mbedtls_entropy_init(&result->entropy);
    mbedtls_ctr_drbg_init(&result->ctr_drbg);
    mbedtls_ssl_init(&result->ssl);
    mbedtls_ssl_session_init(&result->ssn);
    mbedtls_ssl_config_init(&result->config);
    mbedtls_x509_crt_init(&result->trusted_certificates_parsed);
    mbedtls_entropy_add_source(&result->entropy, tlsio_entropy_poll, NULL, 128, 0);
    mbedtls_ctr_drbg_seed(&result->ctr_drbg, mbedtls_entropy_func, &result->entropy, (const unsigned char *)pers, strlen(pers));
    mbedtls_ssl_config_defaults(&result->config, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_rng(&result->config, mbedtls_ctr_drbg_random, &result->ctr_drbg);
    mbedtls_ssl_conf_authmode(&result->config, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_min_version(&result->config, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);          // v1.2
    mbedtls_ssl_set_bio(&result->ssl, instance, on_io_send, on_io_recv, NULL);
    mbedtls_ssl_set_hostname(&result->ssl, host);
    mbedtls_ssl_set_session(&result->ssl, &result->ssn);

    // DEPRECATED: debug functions do not belong in the tree.
#if defined (MBED_TLS_DEBUG_ENABLE)
    mbedtls_ssl_conf_dbg(&result->config, mbedtls_debug, stdout);
    mbedtls_debug_set_threshold(1);
#endif

    mbedtls_ssl_setup(&result->ssl, &result->config);
}

CONCRETE_IO_HANDLE tlsio_mbedtls_create(void* io_create_parameters)
{
    TLSIO_CONFIG* tls_io_config = io_create_parameters;
    TLS_IO_INSTANCE* result;

    if (tls_io_config == NULL)
    {
        LogError("NULL tls_io_config");
        result = NULL;
    }
    else
    {
        result = malloc(sizeof(TLS_IO_INSTANCE));
        if (result != NULL)
        {
            SOCKETIO_CONFIG socketio_config;
            const IO_INTERFACE_DESCRIPTION* underlying_io_interface;
            void* io_interface_parameters;

            if (tls_io_config->underlying_io_interface != NULL)
            {
                underlying_io_interface = tls_io_config->underlying_io_interface;
                io_interface_parameters = tls_io_config->underlying_io_parameters;
            }
            else
            {
                socketio_config.hostname = tls_io_config->hostname;
                socketio_config.port = tls_io_config->port;
                socketio_config.accepted_socket = NULL;

                underlying_io_interface = socketio_get_interface_description();
                io_interface_parameters = &socketio_config;
            }

            if (underlying_io_interface == NULL)
            {
                free(result);
                result = NULL;
                LogError("Failed getting socket IO interface description.");
            }
            else
            {
                result->on_bytes_received = NULL;
                result->on_bytes_received_context = NULL;

                result->on_io_open_complete = NULL;
                result->on_io_open_complete_context = NULL;

                result->on_io_close_complete = NULL;
                result->on_io_close_complete_context = NULL;

                result->on_io_error = NULL;
                result->on_io_error_context = NULL;

                result->trusted_certificates = NULL;

                result->socket_io = xio_create(underlying_io_interface, io_interface_parameters);
                if (result->socket_io == NULL)
                {
                    LogError("socket xio create failed");
                    free(result);
                    result = NULL;
                }
                else
                {
                    result->socket_io_read_bytes = NULL;
                    result->socket_io_read_byte_count = 0;
                    result->on_send_complete = NULL;
                    result->on_send_complete_callback_context = NULL;

                    // mbeTLS initialize
                    mbedtls_init((void *)result, tls_io_config->hostname);
                    result->tlsio_state = TLSIO_STATE_NOT_OPEN;
                }
            }
        }
    }

    return result;
}

void tlsio_mbedtls_destroy(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io != NULL)
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        // mbedTLS cleanup...
        mbedtls_ssl_close_notify(&tls_io_instance->ssl);
        mbedtls_ssl_free(&tls_io_instance->ssl);
        mbedtls_ssl_config_free(&tls_io_instance->config);
        mbedtls_x509_crt_free(&tls_io_instance->trusted_certificates_parsed);
        mbedtls_ctr_drbg_free(&tls_io_instance->ctr_drbg);
        mbedtls_entropy_free(&tls_io_instance->entropy);

        xio_close(tls_io_instance->socket_io, NULL, NULL);

        if (tls_io_instance->socket_io_read_bytes != NULL)
        {
            free(tls_io_instance->socket_io_read_bytes);
        }

        xio_destroy(tls_io_instance->socket_io);
        if (tls_io_instance->trusted_certificates != NULL)
        {
            free(tls_io_instance->trusted_certificates);
        }
        free(tls_io);
    }
}

int tlsio_mbedtls_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result = 0;

    if (tls_io == NULL)
    {
        LogError("NULL tls_io");
        result = __FAILURE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN)
        {
            LogError("IO should not be open: %d\n", tls_io_instance->tlsio_state);
            result = __FAILURE__;
        }
        else
        {
            tls_io_instance->on_bytes_received = on_bytes_received;
            tls_io_instance->on_bytes_received_context = on_bytes_received_context;

            tls_io_instance->on_io_open_complete = on_io_open_complete;
            tls_io_instance->on_io_open_complete_context = on_io_open_complete_context;

            tls_io_instance->on_io_error = on_io_error;
            tls_io_instance->on_io_error_context = on_io_error_context;

            tls_io_instance->tlsio_state = TLSIO_STATE_OPENING_UNDERLYING_IO;

            if (xio_open(tls_io_instance->socket_io, on_underlying_io_open_complete, tls_io_instance, on_underlying_io_bytes_received, tls_io_instance, on_underlying_io_error, tls_io_instance) != 0)
            {
                LogError("Underlying IO open failed");
                tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
                result = __FAILURE__;
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

int tlsio_mbedtls_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    int result = 0;

    if (tls_io == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if ((tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN) ||
            (tls_io_instance->tlsio_state == TLSIO_STATE_CLOSING))
        {
            result = __FAILURE__;
        }
        else
        {
            tls_io_instance->tlsio_state = TLSIO_STATE_CLOSING;
            tls_io_instance->on_io_close_complete = on_io_close_complete;
            tls_io_instance->on_io_close_complete_context = callback_context;

            if (xio_close(tls_io_instance->socket_io,
                on_underlying_io_close_complete_during_close, tls_io_instance) != 0)
            {
                result = __FAILURE__;
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

int tlsio_mbedtls_send(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;

    if (tls_io == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_OPEN)
        {
            result = __FAILURE__;
        }
        else
        {
            tls_io_instance->on_send_complete = on_send_complete;
            tls_io_instance->on_send_complete_callback_context = callback_context;

            int res = mbedtls_ssl_write(&tls_io_instance->ssl, buffer, size);
            if (res != size)
            {
                result = __FAILURE__;
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

void tlsio_mbedtls_dowork(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io != NULL)
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if ((tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN) &&
            (tls_io_instance->tlsio_state != TLSIO_STATE_ERROR))
        {
            decode_ssl_received_bytes(tls_io_instance);
            xio_dowork(tls_io_instance->socket_io);
        }
    }
}

const IO_INTERFACE_DESCRIPTION* tlsio_mbedtls_get_interface_description(void)
{
    return &tlsio_mbedtls_interface_description;
}

/*this function will clone an option given by name and value*/
static void* tlsio_mbedtls_CloneOption(const char* name, const void* value)
{
    void* result;
    if (
        (name == NULL) || (value == NULL)
        )
    {
        LogError("invalid parameter detected: const char* name=%p, const void* value=%p", name, value);
        result = NULL;
    }
    else
    {
        if (strcmp(name, OPTION_UNDERLYING_IO_OPTIONS) == 0)
        {
            result = (void*)value;
        }
        else if (strcmp(name, OPTION_TRUSTED_CERT) == 0)
        {
            if (mallocAndStrcpy_s((char**)&result, value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s TrustedCerts value");
                result = NULL;
            }
            else
            {
                /*return as is*/
            }
        }
        else
        {
            LogError("not handled option : %s", name);
            result = NULL;
        }
    }
    return result;
}

/*this function destroys an option previously created*/
static void tlsio_mbedtls_DestroyOption(const char* name, const void* value)
{
    /*since all options for this layer are actually string copies., disposing of one is just calling free*/
    if (name == NULL || value == NULL)
    {
        LogError("invalid parameter detected: const char* name=%p, const void* value=%p", name, value);
    }
    else
    {
        if (strcmp(name, OPTION_TRUSTED_CERT) == 0)
        {
            free((void*)value);
        }
        else if (strcmp(name, OPTION_UNDERLYING_IO_OPTIONS) == 0)
        {
            OptionHandler_Destroy((OPTIONHANDLER_HANDLE)value);
        }
        else
        {
            LogError("not handled option : %s", name);
        }
    }
}

OPTIONHANDLER_HANDLE tlsio_mbedtls_retrieveoptions(CONCRETE_IO_HANDLE handle)
{
    OPTIONHANDLER_HANDLE result;
    if (handle == NULL)
    {
        LogError("invalid parameter detected: CONCRETE_IO_HANDLE handle=%p", handle);
        result = NULL;
    }
    else
    {
        result = OptionHandler_Create(tlsio_mbedtls_CloneOption, tlsio_mbedtls_DestroyOption, tlsio_mbedtls_setoption);
        if (result == NULL)
        {
            LogError("unable to OptionHandler_Create");
            /*return as is*/
        }
        else
        {
            /*this layer cares about the certificates*/
            TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)handle;
            OPTIONHANDLER_HANDLE underlying_io_options;

            if ((underlying_io_options = xio_retrieveoptions(tls_io_instance->socket_io)) == NULL ||
                OptionHandler_AddOption(result, OPTION_UNDERLYING_IO_OPTIONS, underlying_io_options) != OPTIONHANDLER_OK)
            {
                LogError("unable to save underlying_io options");
                OptionHandler_Destroy(underlying_io_options);
                OptionHandler_Destroy(result);
                result = NULL;
            }
            else if (tls_io_instance->trusted_certificates != NULL &&
                OptionHandler_AddOption(result, OPTION_TRUSTED_CERT, tls_io_instance->trusted_certificates) != OPTIONHANDLER_OK)
            {
                LogError("unable to save TrustedCerts option");
                OptionHandler_Destroy(result);
                result = NULL;
            }
            else
            {
                /*all is fine, all interesting options have been saved*/
                /*return as is*/
            }
        }
    }
    return result;
}

int tlsio_mbedtls_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value)
{
    int result;

    if (tls_io == NULL || optionName == NULL)
    {
        result = __FAILURE__;
    }
    else
    {

        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (strcmp(OPTION_TRUSTED_CERT, optionName) == 0)
        {
            if (tls_io_instance->trusted_certificates != NULL)
            {
                // Free the memory if it has been previously allocated
                free(tls_io_instance->trusted_certificates);
                tls_io_instance->trusted_certificates = NULL;
            }
            if (mallocAndStrcpy_s(&tls_io_instance->trusted_certificates, (const char*)value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s");
                result = __FAILURE__;
            }
            else
            {
                int parse_result = mbedtls_x509_crt_parse(&tls_io_instance->trusted_certificates_parsed, (const unsigned char *)value, (int)(strlen(value) + 1));
                if (parse_result != 0)
                {
                    LogInfo("Malformed pem certificate");
                    result = __FAILURE__;
                }
                else
                {
                    mbedtls_ssl_conf_ca_chain(&tls_io_instance->config, &tls_io_instance->trusted_certificates_parsed, NULL);
                    result = 0;
                }
            }
        }
        else
        {
            // tls_io_instance->socket_io is never NULL
            result = xio_setoption(tls_io_instance->socket_io, optionName, value);
        }
    }

    return result;
}

// DEPRECATED: the USE_MBED_TLS #define is deprecated.
#endif // USE_MBED_TLS
