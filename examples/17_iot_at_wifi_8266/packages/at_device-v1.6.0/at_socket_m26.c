/*
 * File      : at_socket_m26.c
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
 * Date           Author       Notes
 * 2018-06-12     chenyong     first version
 */

#include <stdio.h>
#include <string.h>

#include <rtthread.h>
#include <sys/socket.h>

#include <at.h>
#include <at_socket.h>

#if !defined(RT_USING_NETDEV)
#error "This RT-Thread version is older, please check and updata laster RT-Thread!"
#else
#include <arpa/inet.h>
#include <netdev.h>
#endif /* RT_USING_NETDEV */

#if !defined(AT_SW_VERSION_NUM) || AT_SW_VERSION_NUM < 0x10200
#error "This AT Client version is older, please check and update latest AT Client!"
#endif

#define LOG_TAG              "at.m26|mc20"
#include <at_log.h>

#ifdef AT_DEVICE_M26

#define M26_NETDEV_NAME                "m26"

#define M26_MODULE_SEND_MAX_SIZE       1460
#define M26_WAIT_CONNECT_TIME          5000
#define M26_THREAD_STACK_SIZE          1024
#define M26_THREAD_PRIORITY            (RT_THREAD_PRIORITY_MAX/2)

/* set real event by current socket and current state */
#define SET_EVENT(socket, event)       (((socket + 1) << 16) | (event))

/* AT socket event type */
#define M26_EVENT_CONN_OK              (1L << 0)
#define M26_EVENT_SEND_OK              (1L << 1)
#define M26_EVENT_RECV_OK              (1L << 2)
#define M26_EVNET_CLOSE_OK             (1L << 3)
#define M26_EVENT_CONN_FAIL            (1L << 4)
#define M26_EVENT_SEND_FAIL            (1L << 5)

static int cur_socket;
static rt_event_t at_socket_event;
static rt_mutex_t at_event_lock;
static at_evt_cb_t at_evt_cb_set[] = {
        [AT_SOCKET_EVT_RECV] = NULL,
        [AT_SOCKET_EVT_CLOSED] = NULL,
};

static int at_socket_event_send(uint32_t event)
{
    return (int) rt_event_send(at_socket_event, event);
}

static int at_socket_event_recv(uint32_t event, uint32_t timeout, rt_uint8_t option)
{
    int result = 0;
    rt_uint32_t recved;

    result = rt_event_recv(at_socket_event, event, option | RT_EVENT_FLAG_CLEAR, timeout, &recved);
    if (result != RT_EOK)
    {
        return -RT_ETIMEOUT;
    }

    return recved;
}

/**
 * close socket by AT commands.
 *
 * @param current socket
 *
 * @return  0: close socket success
 *         -1: send AT commands error
 *         -2: wait socket event timeout
 *         -5: no memory
 */
static int m26_socket_close(int socket)
{
    int result = 0;
    at_response_t resp = RT_NULL;

    resp = at_create_resp(128, 0, RT_TICK_PER_SECOND);
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        return -RT_ENOMEM;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);
    cur_socket = socket;

    /* Clear socket close event */
    at_socket_event_recv(SET_EVENT(socket, M26_EVNET_CLOSE_OK), 0, RT_EVENT_FLAG_OR);

    if (at_exec_cmd(resp, "AT+QICLOSE=%d", socket) < 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }

    if (at_socket_event_recv(SET_EVENT(socket, M26_EVNET_CLOSE_OK), rt_tick_from_millisecond(300*3), RT_EVENT_FLAG_AND) < 0)
    {
        LOG_E("socket (%d) close failed, wait close OK timeout.", socket);
        result = -RT_ETIMEOUT;
        goto __exit;
    }

__exit:
    rt_mutex_release(at_event_lock);

    if (resp)
    {
        at_delete_resp(resp);
    }

    return result;
}


/**
 * create TCP/UDP client or server connect by AT commands.
 *
 * @param socket current socket
 * @param ip server or client IP address
 * @param port server or client port
 * @param type connect socket type(tcp, udp)
 * @param is_client connection is client
 *
 * @return   0: connect success
 *          -1: connect failed, send commands error or type error
 *          -2: wait socket event timeout
 *          -5: no memory
 */
static int m26_socket_connect(int socket, char *ip, int32_t port, enum at_socket_type type, rt_bool_t is_client)
{
    int result = 0, event_result = 0;
    rt_bool_t retryed = RT_FALSE;
    at_response_t resp = RT_NULL;

    RT_ASSERT(ip);
    RT_ASSERT(port >= 0);

    resp = at_create_resp(128, 0, 5 * RT_TICK_PER_SECOND);
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        return -RT_ENOMEM;
    }

    /* lock AT socket connect */
    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

__retry:

    /* Clear socket connect event */
    at_socket_event_recv(SET_EVENT(socket, M26_EVENT_CONN_OK | M26_EVENT_CONN_FAIL), 0, RT_EVENT_FLAG_OR);

    if (is_client)
    {
        switch (type)
        {
        case AT_SOCKET_TCP:
            /* send AT commands(eg: AT+QIOPEN=0,"TCP","x.x.x.x", 1234) to connect TCP server */
            if (at_exec_cmd(resp, "AT+QIOPEN=%d,\"TCP\",\"%s\",%d", socket, ip, port) < 0)
            {
                result = -RT_ERROR;
                goto __exit;
            }
            break;

        case AT_SOCKET_UDP:
            if (at_exec_cmd(resp, "AT+QIOPEN=%d,\"UDP\",\"%s\",%d", socket, ip, port) < 0)
            {
                result = -RT_ERROR;
                goto __exit;
            }
            break;

        default:
            LOG_E("Not supported connect type : %d.", type);
            return -RT_ERROR;
        }
    }

    /* waiting result event from AT URC, the device default connection timeout is 75 seconds, but it set to 10 seconds is convenient to use.*/
    if (at_socket_event_recv(SET_EVENT(socket, 0), 10 * RT_TICK_PER_SECOND, RT_EVENT_FLAG_OR) < 0)
    {
        LOG_E("socket (%d) connect failed, wait connect result timeout.", socket);
        result = -RT_ETIMEOUT;
        goto __exit;
    }
    /* waiting OK or failed result */
    if ((event_result = at_socket_event_recv(M26_EVENT_CONN_OK | M26_EVENT_CONN_FAIL, 1 * RT_TICK_PER_SECOND,
            RT_EVENT_FLAG_OR)) < 0)
    {
        LOG_E("socket (%d) connect failed, wait connect OK|FAIL timeout.", socket);
        result = -RT_ETIMEOUT;
        goto __exit;
    }
    /* check result */
    if (event_result & M26_EVENT_CONN_FAIL)
    {
        if (!retryed)
        {
            LOG_E("socket (%d) connect failed, maybe the socket was not be closed at the last time and now will retry.", socket);
            if (m26_socket_close(socket) < 0)
            {
                goto __exit;
            }
            retryed = RT_TRUE;
            goto __retry;
        }
        LOG_E("socket (%d) connect failed, failed to establish a connection.", socket);
        result = -RT_ERROR;
        goto __exit;
    }

__exit:
    /* unlock AT socket connect */
    rt_mutex_release(at_event_lock);

    if (resp)
    {
        at_delete_resp(resp);
    }

    return result;
}

static int at_get_send_size(int socket, size_t *size, size_t *acked, size_t *nacked)
{
    at_response_t resp = at_create_resp(64, 0, 5 * RT_TICK_PER_SECOND);
    int result = 0;

    if (!resp)
    {
        LOG_E("No memory for response structure!");
        result = -RT_ENOMEM;
        goto __exit;
    }

    if (at_exec_cmd(resp, "AT+QISACK=%d", socket) < 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }

    if (at_resp_parse_line_args_by_kw(resp, "+QISACK:", "+QISACK: %d, %d, %d", size, acked, nacked) <= 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    return result;
}

static int at_wait_send_finish(int socket, size_t settings_size)
{
    /* get the timeout by the input data size */
    rt_tick_t timeout = rt_tick_from_millisecond(settings_size);
    rt_tick_t last_time = rt_tick_get();
    size_t size = 0, acked = 0, nacked = 0xFFFF;

    while (rt_tick_get() - last_time <= timeout)
    {
        at_get_send_size(socket, &size, &acked, &nacked);
        if (nacked == 0)
        {
            return RT_EOK;
        }
        rt_thread_delay(rt_tick_from_millisecond(50));
    }

    return -RT_ETIMEOUT;
}

/**
 * send data to server or client by AT commands.
 *
 * @param socket current socket
 * @param buff send buffer
 * @param bfsz send buffer size
 * @param type connect socket type(tcp, udp)
 *
 * @return >=0: the size of send success
 *          -1: send AT commands error or send data error
 *          -2: waited socket event timeout
 *          -5: no memory
 */
static int m26_socket_send(int socket, const char *buff, size_t bfsz, enum at_socket_type type)
{
    int result = 0, event_result = 0;
    at_response_t resp = RT_NULL;
    size_t cur_pkt_size = 0, sent_size = 0;

    RT_ASSERT(buff);

    resp = at_create_resp(128, 2, 5 * RT_TICK_PER_SECOND);
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        return -RT_ENOMEM;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

    /* Clear socket send event */
    at_socket_event_recv(SET_EVENT(socket, M26_EVENT_SEND_OK | M26_EVENT_SEND_FAIL), 0, RT_EVENT_FLAG_OR);

    /* set current socket for send URC event */
    cur_socket = socket;
    /* set AT client end sign to deal with '>' sign.*/
    at_set_end_sign('>');

    while (sent_size < bfsz)
    {
        if (bfsz - sent_size < M26_MODULE_SEND_MAX_SIZE)
        {
            cur_pkt_size = bfsz - sent_size;
        }
        else
        {
            cur_pkt_size = M26_MODULE_SEND_MAX_SIZE;
        }

        /* send the "AT+QISEND" commands to AT server than receive the '>' response on the first line. */
        if (at_exec_cmd(resp, "AT+QISEND=%d,%d", socket, cur_pkt_size) < 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }

        /* send the real data to server or client */
        result = (int) at_client_send(buff + sent_size, cur_pkt_size);
        if (result == 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }

        /* waiting result event from AT URC */
        if (at_socket_event_recv(SET_EVENT(socket, 0), 1 * RT_TICK_PER_SECOND, RT_EVENT_FLAG_OR) < 0)
        {
            LOG_E("socket (%d) send failed, wait connect result timeout.", socket);
            result = -RT_ETIMEOUT;
            goto __exit;
        }
        /* waiting OK or failed result */
        if ((event_result = at_socket_event_recv(M26_EVENT_SEND_OK | M26_EVENT_SEND_FAIL, 1 * RT_TICK_PER_SECOND,
                RT_EVENT_FLAG_OR)) < 0)
        {
            LOG_E("socket (%d) send failed, wait connect OK|FAIL timeout.", socket);
            result = -RT_ETIMEOUT;
            goto __exit;
        }
        /* check result */
        if (event_result & M26_EVENT_SEND_FAIL)
        {
            LOG_E("socket (%d) send failed, return failed.", socket);
            result = -RT_ERROR;
            goto __exit;
        }

        if (type == AT_SOCKET_TCP)
        {
            at_wait_send_finish(socket, cur_pkt_size);
        }

        sent_size += cur_pkt_size;
    }


__exit:
    /* reset the end sign for data conflict */
    at_set_end_sign(0);

    rt_mutex_release(at_event_lock);

    if (resp)
    {
        at_delete_resp(resp);
    }

    return result;
}

/**
 * domain resolve by AT commands.
 *
 * @param name domain name
 * @param ip parsed IP address, it's length must be 16
 *
 * @return  0: domain resolve success
 *         -1: send AT commands error or response error
 *         -2: wait socket event timeout
 *         -5: no memory
 */
static int m26_domain_resolve(const char *name, char ip[16])
{
#define RESOLVE_RETRY                  5

    int i, result = RT_EOK;
    char recv_ip[16] = { 0 };
    at_response_t resp = RT_NULL;

    RT_ASSERT(name);
    RT_ASSERT(ip);

    /* The maximum response time is 14 seconds, affected by network status */
    resp = at_create_resp(128, 4, 14 * RT_TICK_PER_SECOND);
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        return -RT_ENOMEM;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

    for(i = 0; i < RESOLVE_RETRY; i++)
    {
        if (at_exec_cmd(resp, "AT+QIDNSGIP=\"%s\"", name) < 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }

        /* parse the third line of response data, get the IP address */
        if(at_resp_parse_line_args_by_kw(resp, ".", "%s", recv_ip) < 0)
        {
            rt_thread_mdelay(100);
            /* resolve failed, maybe receive an URC CRLF */
            continue;
        }

        if (strlen(recv_ip) < 8)
        {
            rt_thread_mdelay(100);
            /* resolve failed, maybe receive an URC CRLF */
            continue;
        }
        else
        {
            strncpy(ip, recv_ip, 15);
            ip[15] = '\0';
            break;
        }
    }

__exit:
    rt_mutex_release(at_event_lock);

    if (resp)
    {
        at_delete_resp(resp);
    }

    return result;

}

/**
 * set AT socket event notice callback
 *
 * @param event notice event
 * @param cb notice callback
 */
static void m26_socket_set_event_cb(at_socket_evt_t event, at_evt_cb_t cb)
{
    if (event < sizeof(at_evt_cb_set) / sizeof(at_evt_cb_set[1]))
    {
        at_evt_cb_set[event] = cb;
    }
}

static void urc_connect_func(const char *data, rt_size_t size)
{
    int socket = 0;

    RT_ASSERT(data && size);

    sscanf(data, "%d%*[^0-9]", &socket);

    if (strstr(data, "CONNECT OK"))
    {
        at_socket_event_send(SET_EVENT(socket, M26_EVENT_CONN_OK));
    }
    else
    {
        at_socket_event_send(SET_EVENT(socket, M26_EVENT_CONN_FAIL));
    }
}

static void urc_send_func(const char *data, rt_size_t size)
{
    RT_ASSERT(data && size);

    if (strstr(data, "SEND OK"))
    {
        at_socket_event_send(SET_EVENT(cur_socket, M26_EVENT_SEND_OK));
    }
    else if (strstr(data, "SEND FAIL"))
    {
        at_socket_event_send(SET_EVENT(cur_socket, M26_EVENT_SEND_FAIL));
    }
}

static void urc_close_func(const char *data, rt_size_t size)
{
    int socket = 0;

    RT_ASSERT(data && size);

    if (strstr(data, "CLOSE OK"))
    {
        at_socket_event_send(SET_EVENT(cur_socket, M26_EVNET_CLOSE_OK));
    }
    else if (strstr(data, "CLOSED"))
    {
        sscanf(data, "%d, CLOSED", &socket);
        /* notice the socket is disconnect by remote */
        if (at_evt_cb_set[AT_SOCKET_EVT_CLOSED])
        {
            at_evt_cb_set[AT_SOCKET_EVT_CLOSED](socket, AT_SOCKET_EVT_CLOSED, NULL, 0);
        }
    }
}

static void urc_recv_func(const char *data, rt_size_t size)
{
    int socket = 0;
    rt_size_t bfsz = 0, temp_size = 0;
    rt_int32_t timeout;
    char *recv_buf = RT_NULL, temp[8];

    RT_ASSERT(data && size);

    /* get the current socket and receive buffer size by receive data */
    sscanf(data, "+RECEIVE: %d, %d", &socket, (int *) &bfsz);
    /* get receive timeout by receive buffer length */
    timeout = bfsz;

    if (socket < 0 || bfsz == 0)
        return;

    recv_buf = rt_calloc(1, bfsz);
    if (!recv_buf)
    {
        LOG_E("no memory for URC receive buffer (%d)!", bfsz);
        /* read and clean the coming data */
        while (temp_size < bfsz)
        {
            if (bfsz - temp_size > sizeof(temp))
            {
                at_client_recv(temp, sizeof(temp), timeout);
            }
            else
            {
                at_client_recv(temp, bfsz - temp_size, timeout);
            }
            temp_size += sizeof(temp);
        }
        return;
    }

    /* sync receive data */
    if (at_client_recv(recv_buf, bfsz, timeout) != bfsz)
    {
        LOG_E("receive size(%d) data failed!", bfsz);
        rt_free(recv_buf);
        return;
    }

    /* notice the receive buffer and buffer size */
    if (at_evt_cb_set[AT_SOCKET_EVT_RECV])
    {
        at_evt_cb_set[AT_SOCKET_EVT_RECV](socket, AT_SOCKET_EVT_RECV, recv_buf, bfsz);
    }
}

static void urc_func(const char *data, rt_size_t size)
{
    RT_ASSERT(data);

    LOG_I("URC data : %.*s", size, data);
}

static const struct at_urc urc_table[] = {
        {"RING",        "\r\n",                 urc_func},
        {"Call Ready",  "\r\n",                 urc_func},
        {"RDY",         "\r\n",                 urc_func},
        {"NO CARRIER",  "\r\n",                 urc_func},
        {"",            ", CONNECT OK\r\n",     urc_connect_func},
        {"",            ", CONNECT FAIL\r\n",   urc_connect_func},
        {"SEND OK",     "\r\n",                 urc_send_func},
        {"SEND FAIL",   "\r\n",                 urc_send_func},
        {"",            ", CLOSE OK\r\n",       urc_close_func},
        {"",            ", CLOSED\r\n",         urc_close_func},
        {"+RECEIVE:",   "\r\n",                 urc_recv_func},
};

#define AT_SEND_CMD(resp, resp_line, timeout, cmd)                                                              \
    do                                                                                                          \
    {                                                                                                           \
        if (at_exec_cmd(at_resp_set_info(resp, 128, resp_line, rt_tick_from_millisecond(timeout)), cmd) < 0)    \
        {                                                                                                       \
            result = -RT_ERROR;                                                                                 \
            goto __exit;                                                                                        \
        }                                                                                                       \
    } while(0);                                                                                                 \

static int m26_netdev_set_info(struct netdev *netdev);
static int m26_netdev_check_link_status(struct netdev *netdev);

/* init for M26 or MC20 */
static void m26_init_thread_entry(void *parameter)
{
#define CPIN_RETRY                     10
#define CSQ_RETRY                      10
#define CREG_RETRY                     10
#define CGREG_RETRY                    20

    at_response_t resp = RT_NULL;
    int i, qimux, qimode;
    char parsed_data[10];
    rt_err_t result = RT_EOK;

    resp = at_create_resp(128, 0, rt_tick_from_millisecond(300));
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        result = -RT_ENOMEM;
        goto __exit;
    }
    LOG_D("Start initializing the M26/MC20 module");
    /* wait M26 startup finish */
    if (at_client_wait_connect(M26_WAIT_CONNECT_TIME))
    {
        LOG_E("AT device connection error.");
        result = -RT_ETIMEOUT;
        goto __exit;
    }
    /* disable echo */
    AT_SEND_CMD(resp, 0, 300, "ATE0");
    /* get module version */
    AT_SEND_CMD(resp, 0, 300, "ATI");
    /* show module version */
    for (i = 0; i < (int) resp->line_counts - 1; i++)
    {
        LOG_D("%s", at_resp_get_line(resp, i + 1));
    }
    /* check SIM card */
    for (i = 0; i < CPIN_RETRY; i++)
    {
        at_exec_cmd(at_resp_set_info(resp, 128, 2, 5 * RT_TICK_PER_SECOND), "AT+CPIN?");

        if (at_resp_get_line_by_kw(resp, "READY"))
        {
            LOG_D("SIM card detection success");
            break;
        }
        rt_thread_mdelay(1000);
    }
    if (i == CPIN_RETRY)
    {
        LOG_E("SIM card detection failed!");
        result = -RT_ERROR;
        goto __exit;
    }
    /* waiting for dirty data to be digested */
    rt_thread_mdelay(10);
    /* check signal strength */
    for (i = 0; i < CSQ_RETRY; i++)
    {
        AT_SEND_CMD(resp, 0, 300, "AT+CSQ");
        at_resp_parse_line_args_by_kw(resp, "+CSQ:", "+CSQ: %s", &parsed_data);
        if (strncmp(parsed_data, "99,99", sizeof(parsed_data)))
        {
            LOG_D("Signal strength: %s", parsed_data);
            break;
        }
        rt_thread_mdelay(1000);
    }
    if (i == CSQ_RETRY)
    {
        LOG_E("Signal strength check failed (%s)", parsed_data);
        result = -RT_ERROR;
        goto __exit;
    }
    /* check the GSM network is registered */
    for (i = 0; i < CREG_RETRY; i++)
    {
        AT_SEND_CMD(resp, 0, 300, "AT+CREG?");
        at_resp_parse_line_args_by_kw(resp, "+CREG:", "+CREG: %s", &parsed_data);
        if (!strncmp(parsed_data, "0,1", sizeof(parsed_data)) || !strncmp(parsed_data, "0,5", sizeof(parsed_data)))
        {
            LOG_D("GSM network is registered (%s)", parsed_data);
            break;
        }
        rt_thread_mdelay(1000);
    }
    if (i == CREG_RETRY)
    {
        LOG_E("The GSM network is register failed (%s)", parsed_data);
        result = -RT_ERROR;
        goto __exit;
    }
    /* check the GPRS network is registered */
    for (i = 0; i < CGREG_RETRY; i++)
    {
        AT_SEND_CMD(resp, 0, 300, "AT+CGREG?");
        at_resp_parse_line_args_by_kw(resp, "+CGREG:", "+CGREG: %s", &parsed_data);
        if (!strncmp(parsed_data, "0,1", sizeof(parsed_data)) || !strncmp(parsed_data, "0,5", sizeof(parsed_data)))
        {
            LOG_D("GPRS network is registered (%s)", parsed_data);
            break;
        }
        rt_thread_mdelay(1000);
    }
    if (i == CGREG_RETRY)
    {
        LOG_E("The GPRS network is register failed (%s)", parsed_data);
        result = -RT_ERROR;
        goto __exit;
    }

    AT_SEND_CMD(resp, 0, 300, "AT+QIFGCNT=0");
    AT_SEND_CMD(resp, 0, 300, "AT+QICSGP=1, \"CMNET\"");
    AT_SEND_CMD(resp, 0, 300, "AT+QIMODE?");

    at_resp_parse_line_args_by_kw(resp, "+QIMODE:", "+QIMODE: %d", &qimode);
    if (qimode == 1)
    {
        AT_SEND_CMD(resp, 0, 300, "AT+QIMODE=0");
    }

    /* the device default response timeout is 40 seconds, but it set to 15 seconds is convenient to use. */
    AT_SEND_CMD(resp, 2, 20 * 1000, "AT+QIDEACT");

    /* Set to multiple connections */
    AT_SEND_CMD(resp, 0, 300, "AT+QIMUX?");
    at_resp_parse_line_args_by_kw(resp, "+QIMUX:", "+QIMUX: %d", &qimux);
    if (qimux == 0)
    {
        AT_SEND_CMD(resp, 0, 300, "AT+QIMUX=1");
    }

    AT_SEND_CMD(resp, 0, 300, "AT+QIREGAPP");

    /* the device default response timeout is 150 seconds, but it set to 20 seconds is convenient to use. */
    AT_SEND_CMD(resp, 0, 20 * 1000, "AT+QIACT");

    AT_SEND_CMD(resp, 2, 300, "AT+QILOCIP");

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    if (!result)
    {
        m26_netdev_set_info(netdev_get_by_name(M26_NETDEV_NAME));
        m26_netdev_check_link_status(netdev_get_by_name(M26_NETDEV_NAME));

        LOG_I("AT network initialize success!");
    }
    else
    {
        LOG_E("AT network initialize failed (%d)!", result);
    }
}

int m26_net_init(void)
{
#ifdef PKG_AT_INIT_BY_THREAD
    rt_thread_t tid;

    tid = rt_thread_create("m26_net_init", m26_init_thread_entry, RT_NULL, M26_THREAD_STACK_SIZE, M26_THREAD_PRIORITY, 20);
    if (tid)
    {
        rt_thread_startup(tid);
    }
    else
    {
        LOG_E("Create AT initialization thread fail!");
    }
#else
    m26_init_thread_entry(RT_NULL);
#endif

    return RT_EOK;
}

#ifdef FINSH_USING_MSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(m26_net_init, at_net_init, initialize AT network);
#endif

static const struct at_device_ops m26_socket_ops = {
    m26_socket_connect,
    m26_socket_close,
    m26_socket_send,
    m26_domain_resolve,
    m26_socket_set_event_cb,
};


/* set m26 network interface device status and address information */
static int m26_netdev_set_info(struct netdev *netdev)
{
#define M26_IEMI_RESP_SIZE      32
#define M26_IPADDR_RESP_SIZE    32
#define M26_DNS_RESP_SIZE       96
#define M26_INFO_RESP_TIMO      rt_tick_from_millisecond(300)

    int result = RT_EOK;
    at_response_t resp = RT_NULL;
    ip_addr_t addr;

    if (netdev == RT_NULL)
    {
        LOG_E("Input network interface device is NULL.\n");
        return -RT_ERROR;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

    /* set network interface device up status */
    netdev_low_level_set_status(netdev, RT_TRUE);
    netdev_low_level_set_dhcp_status(netdev, RT_TRUE);

    resp = at_create_resp(M26_IEMI_RESP_SIZE, 0, M26_INFO_RESP_TIMO);
    if (resp == RT_NULL)
    {
        LOG_E("M26 set IP address failed, no memory for response object.");
        result = -RT_ENOMEM;
        goto __exit;
    }

    /* set network interface device hardware address(IEMI) */
    {
        #define M26_NETDEV_HWADDR_LEN   8
        #define M26_IEMI_LEN            15

        char iemi[M26_IEMI_LEN] = {0};
        int i = 0, j = 0;

        /* send "AT+GSN" commond to get device IEMI */
        if (at_exec_cmd(resp, "AT+GSN") < 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }

        if (at_resp_parse_line_args(resp, 2, "%s", iemi) <= 0)
        {
            LOG_E("Prase \"AT+GSN\" commands resposne data error!");
            result = -RT_ERROR;
            goto __exit;
        }

        LOG_D("M26 IEMI number: %s", iemi);

        netdev->hwaddr_len = M26_NETDEV_HWADDR_LEN;
        /* get hardware address by IEMI */
        for (i = 0, j = 0; i < M26_NETDEV_HWADDR_LEN && j < M26_IEMI_LEN; i++, j+=2)
        {
            if (j != M26_IEMI_LEN - 1)
            {
                netdev->hwaddr[i] = (iemi[j] - '0') * 10 + (iemi[j + 1] - '0');
            }
            else
            {
                netdev->hwaddr[i] = (iemi[j] - '0');
            }
        }
    }

    /* set network interface device IP address */
    {
        #define IP_ADDR_SIZE_MAX    16
        char ipaddr[IP_ADDR_SIZE_MAX] = {0};
        
        at_resp_set_info(resp, M26_IPADDR_RESP_SIZE, 2, M26_INFO_RESP_TIMO);

        /* send "AT+QILOCIP" commond to get IP address */
        if (at_exec_cmd(resp, "AT+QILOCIP") < 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }

        if (at_resp_parse_line_args_by_kw(resp, ".", "%s", ipaddr) <= 0)
        {
            LOG_E("Prase \"AT+QILOCIP\" commands resposne data error!");
            result = -RT_ERROR;
            goto __exit;
        }
        
        LOG_D("M26 IP address: %s", ipaddr);

        /* set network interface address information */
        inet_aton(ipaddr, &addr);
        netdev_low_level_set_ipaddr(netdev, &addr);
    }

    /* set network interface device dns server */
    {
        #define DNS_ADDR_SIZE_MAX   16
        char dns_server1[DNS_ADDR_SIZE_MAX] = {0}, dns_server2[DNS_ADDR_SIZE_MAX] = {0};

        at_resp_set_info(resp, M26_DNS_RESP_SIZE, 0, M26_INFO_RESP_TIMO);

        /* send "AT+QIDNSCFG?" commond to get DNS servers address */
        if (at_exec_cmd(resp, "AT+QIDNSCFG?") < 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }

        if (at_resp_parse_line_args_by_kw(resp, "PrimaryDns:", "PrimaryDns:%s", dns_server1) <= 0 ||
                at_resp_parse_line_args_by_kw(resp, "SecondaryDns:", "SecondaryDns:%s", dns_server2) <= 0)
        {
            LOG_E("Prase \"AT+QIDNSCFG?\" commands resposne data error!");
            result = -RT_ERROR;
            goto __exit;
        }

        LOG_D("M26 primary DNS server address: %s", dns_server1);
        LOG_D("M26 secondary DNS server address: %s", dns_server2);

        inet_aton(dns_server1, &addr);
        netdev_low_level_set_dns_server(netdev, 0, &addr);

        inet_aton(dns_server2, &addr);
        netdev_low_level_set_dns_server(netdev, 1, &addr);
    }

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    rt_mutex_release(at_event_lock);
    
    return result;
}

static void check_link_status_entry(void *parameter)
{
#define M26_LINK_STATUS_OK   0
#define M26_LINK_RESP_SIZE   64
#define M26_LINK_RESP_TIMO   (3 * RT_TICK_PER_SECOND)
#define M26_LINK_DELAY_TIME  (30 * RT_TICK_PER_SECOND)

    struct netdev *netdev = (struct netdev *)parameter;
    at_response_t resp = RT_NULL;
    int link_status;

    resp = at_create_resp(M26_LINK_RESP_SIZE, 0, M26_LINK_RESP_TIMO);
    if (resp == RT_NULL)
    {
        LOG_E("m26 set check link status failed, no memory for response object.");
        return;
    }

    while (1)
    { 

        rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);
        
        /* send "AT+QNSTATUS" commond  to check netweork interface device link status */
        if (at_exec_cmd(resp, "AT+QNSTATUS") < 0)
        {
            rt_mutex_release(at_event_lock);
            rt_thread_mdelay(M26_LINK_DELAY_TIME);

            continue;
        }
        
        link_status = -1;
        at_resp_parse_line_args_by_kw(resp, "+QNSTATUS:", "+QNSTATUS: %d", &link_status);

        /* check the network interface device link status  */
        if ((M26_LINK_STATUS_OK == link_status) != netdev_is_link_up(netdev))
        {
            netdev_low_level_set_link_status(netdev, (M26_LINK_STATUS_OK == link_status));
        }

        rt_mutex_release(at_event_lock);
        rt_thread_mdelay(M26_LINK_DELAY_TIME);
    }
}

static int m26_netdev_check_link_status(struct netdev *netdev)
{
#define M26_LINK_THREAD_STACK_SIZE     512
#define M26_LINK_THREAD_PRIORITY       (RT_THREAD_PRIORITY_MAX - 2)
#define M26_LINK_THREAD_TICK           20

    rt_thread_t tid;
    char tname[RT_NAME_MAX];

    if (netdev == RT_NULL)
    {
        LOG_E("Input network interface device is NULL.\n");
        return -RT_ERROR;
    }

    rt_memset(tname, 0x00, sizeof(tname));
    rt_snprintf(tname, RT_NAME_MAX, "%s_link", netdev->name);

    tid = rt_thread_create(tname, check_link_status_entry, (void *)netdev, 
            M26_LINK_THREAD_STACK_SIZE, M26_LINK_THREAD_PRIORITY, M26_LINK_THREAD_TICK);
    if (tid)
    {
        rt_thread_startup(tid);
    }

    return RT_EOK;
}

static int m26_netdev_set_up(struct netdev *netdev)
{
    netdev_low_level_set_status(netdev, RT_TRUE);
    LOG_D("The network interface device(%s) set up status", netdev->name);

    return RT_EOK;
}

static int m26_netdev_set_down(struct netdev *netdev)
{
    netdev_low_level_set_status(netdev, RT_FALSE);
    LOG_D("The network interface device(%s) set down status", netdev->name);
    return RT_EOK;
}

static int m26_netdev_set_dns_server(struct netdev *netdev, uint8_t dns_num, ip_addr_t *dns_server)
{
#define M26_DNS_RESP_LEN    8
#define M26_DNS_RESP_TIMEO   rt_tick_from_millisecond(300)

    at_response_t resp = RT_NULL;
    int result = RT_EOK;

    RT_ASSERT(netdev);
    RT_ASSERT(dns_server);

    resp = at_create_resp(M26_DNS_RESP_LEN, 0, M26_DNS_RESP_TIMEO);
    if (resp == RT_NULL)
    {
        LOG_D("m26 set dns server failed, no memory for response object.");
        return -RT_ENOMEM;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

    /* send "AT+QIDNSCFG=<pri_dns>[,<sec_dns>]" commond to set dns servers */
    if (at_exec_cmd(resp, "AT+QIDNSCFG=\"%s\"", inet_ntoa(*dns_server)) < 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }

    netdev_low_level_set_dns_server(netdev, dns_num, dns_server);

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    rt_mutex_release(at_event_lock);

    return result;
}

static int m26_netdev_ping(struct netdev *netdev, const char *host, size_t data_len, uint32_t timeout, struct netdev_ping_resp *ping_resp)
{
#define M26_PING_RESP_SIZE       128
#define M26_PING_IP_SIZE         16
#define M26_PING_TIMEO           (5 * RT_TICK_PER_SECOND)

    int result = RT_EOK;
    at_response_t resp = RT_NULL;
    char ip_addr[M26_PING_IP_SIZE] = {0};
    int response, recv_data_len, time, ttl;

    RT_ASSERT(netdev);
    RT_ASSERT(host);
    RT_ASSERT(ping_resp);

    resp = at_create_resp(M26_PING_RESP_SIZE, 5, M26_PING_TIMEO);
    if (resp == RT_NULL)
    {
        LOG_D("m26 set dns server failed, no memory for response object.");
        return  -RT_ENOMEM;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

    /* send "AT+QPING="<host>"[,[<timeout>][,<pingnum>]]" commond to send ping request */
    if (at_exec_cmd(resp, "AT+QPING=\"%s\",%d,1", host, M26_PING_TIMEO / RT_TICK_PER_SECOND) < 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }

    at_resp_parse_line_args_by_kw(resp, "+QPING:","+QPING:%d", &response);
    /* Received the ping response from the server */
    if (response == 0)
    {
        if (at_resp_parse_line_args_by_kw(resp, "+QPING:", "+QPING:%d,%[^,],%d,%d,%d",
                &response, ip_addr, &recv_data_len, &time, &ttl) <= 0)
        {
            LOG_D("Prase \"AT+QPING\" commands resposne data error!");
            result = -RT_ERROR;
            goto __exit;
        }   
    }


    /* prase response number */
    switch (response)
    {
    case 0:
        inet_aton(ip_addr, &(ping_resp->ip_addr));
        ping_resp->data_len = recv_data_len;
        ping_resp->ticks = time;
        ping_resp->ttl = ttl;
        result = RT_EOK;
        break;
    case 1:
        result = -RT_ETIMEOUT;
        break;
    default:
        result = -RT_ERROR;
        break;
    }

 __exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    rt_mutex_release(at_event_lock);

    return result;
}

void m26_netdev_netstat(struct netdev *netdev)
{ 
    // TODO netstat support
}

const struct netdev_ops m26_netdev_ops =
{
    m26_netdev_set_up,
    m26_netdev_set_down,

    RT_NULL, /* not support set ip, netmask, gatway address */
    m26_netdev_set_dns_server,
    RT_NULL, /* not support set DHCP status */

    m26_netdev_ping,
    m26_netdev_netstat,
};

static int m26_netdev_add(const char *netdev_name)
{
#define M26_NETDEV_MTU       1500

    struct netdev *netdev = RT_NULL;

    RT_ASSERT(netdev_name);

    netdev = (struct netdev *) rt_calloc(1, sizeof(struct netdev));
    if (netdev == RT_NULL)
    {
        return RT_NULL;
    }

    netdev->mtu = M26_NETDEV_MTU;
    netdev->ops = &m26_netdev_ops;

#ifdef SAL_USING_AT
    extern int sal_at_netdev_set_pf_info(struct netdev *netdev);
    /* set the network interface socket/netdb operations */
    sal_at_netdev_set_pf_info(netdev);
#endif

    return netdev_register(netdev, netdev_name, RT_NULL);
}

static int at_socket_device_init(void)
{
    /* create current AT socket event */
    at_socket_event = rt_event_create("at_se", RT_IPC_FLAG_FIFO);
    if (at_socket_event == RT_NULL)
    {
        LOG_E("AT client port initialize failed! at_sock_event create failed!");
        return -RT_ENOMEM;
    }

    /* create current AT socket event lock */
    at_event_lock = rt_mutex_create("at_se", RT_IPC_FLAG_FIFO);
    if (at_event_lock == RT_NULL)
    {
        LOG_E("AT client port initialize failed! at_sock_lock create failed!");
        rt_event_delete(at_socket_event);
        return -RT_ENOMEM;
    }

    /* initialize AT client */
    at_client_init(AT_DEVICE_NAME, AT_DEVICE_RECV_BUFF_LEN);

    /* register URC data execution function  */
    at_set_urc_table(urc_table, sizeof(urc_table) / sizeof(urc_table[0]));

    /* add network interface device to netdev list */
    if (m26_netdev_add(M26_NETDEV_NAME) < 0)
    {
        LOG_E("M26 network interface device(%d) add failed.", M26_NETDEV_NAME);
        return -RT_ENOMEM;
    }

    /* initialize m26 network */
    m26_net_init();

    /* set m26 AT Socket options */
    at_socket_device_register(&m26_socket_ops);

    return RT_EOK;
}
INIT_APP_EXPORT(at_socket_device_init);

#endif /* AT_DEVICE_M26 */
