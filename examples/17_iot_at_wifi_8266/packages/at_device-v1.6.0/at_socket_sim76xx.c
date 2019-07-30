/*
 * File      : at_socket_sim76xx.c
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
 * 2018-12-22    thomasonegd  first version
 * 2019-03-06    thomasonegd  fix udp connection.
 * 2019-03-08    thomasonegd  add power_on & power_off api
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

#define LOG_TAG              "at.sim76xx"
#include <at_log.h>

#ifdef AT_DEVICE_SIM76XX

#define SIM76XX_NETDEV_NAME            "sim76xx"

#define SIM76XX_MODULE_SEND_MAX_SIZE   1500
#define SIM76XX_WAIT_CONNECT_TIME      5000
#define SIM76XX_THREAD_STACK_SIZE      1024
#define SIM76XX_THREAD_PRIORITY        (RT_THREAD_PRIORITY_MAX/2)

#define SIM76XX_MAX_CONNECTIONS        10

/* set real event by current socket and current state */
#define SET_EVENT(socket, event)       (((socket + 1) << 16) | (event))

/* AT socket event type */
#define SIM76XX_EVENT_CONN_OK          (1L << 0)
#define SIM76XX_EVENT_SEND_OK          (1L << 1)
#define SIM76XX_EVENT_RECV_OK          (1L << 2)
#define SIM76XX_EVNET_CLOSE_OK         (1L << 3)
#define SIM76XX_EVENT_CONN_FAIL        (1L << 4)
#define SIM76XX_EVENT_SEND_FAIL        (1L << 5)

uint8_t at_socket_init = 0;

static int cur_socket;
static int cur_send_bfsz;
static rt_event_t at_socket_event;
static rt_mutex_t at_event_lock;
static at_evt_cb_t at_evt_cb_set[] = {
        [AT_SOCKET_EVT_RECV] = NULL,
        [AT_SOCKET_EVT_CLOSED] = NULL,
};

static char udp_ipstr[SIM76XX_MAX_CONNECTIONS][16];
static int udp_port[SIM76XX_MAX_CONNECTIONS];

static void at_tcp_ip_errcode_parse(int result)//Unsolicited TCP/IP command<err> codes
{
    switch(result)
    {
    case 0   : LOG_D("%d : operation succeeded ",           result); break;
    case 1 : LOG_E("%d : UNetwork failure",                 result); break;
    case 2 : LOG_E("%d : Network not opened",               result); break;
    case 3 : LOG_E("%d : Wrong parameter",                  result); break;
    case 4 : LOG_E("%d : Operation not supported",          result); break;
    case 5 : LOG_E("%d : Failed to create socket",          result); break;
    case 6 : LOG_E("%d : Failed to bind socket",            result); break;
    case 7 : LOG_E("%d : TCP server is already listening",  result); break;
    case 8 : LOG_E("%d : Busy",                             result); break;
    case 9 : LOG_E("%d : Sockets opened",                   result); break;
    case 10 : LOG_E("%d : Timeout ",                        result); break;
    case 11 : LOG_E("%d : DNS parse failed for AT+CIPOPEN", result); break;
    case 255 : LOG_E("%d : Unknown error",                  result); break;
    }
}


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
static int sim76xx_socket_close(int socket)
{
    at_response_t resp = RT_NULL;
    int result = RT_EOK;
    int activated;
    uint8_t lnk_stat[10];
    
    resp = at_create_resp(128, 0, rt_tick_from_millisecond(500));
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        return -RT_ENOMEM;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);
    
    rt_thread_delay(rt_tick_from_millisecond(100));
    
    // check socket link_state
    if (at_exec_cmd(resp,"AT+CIPCLOSE?") < 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }
    
    if(at_resp_parse_line_args_by_kw(resp,"+CIPCLOSE:", "+CIPCLOSE: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&lnk_stat[0],&lnk_stat[1],&lnk_stat[1]
        ,&lnk_stat[2],&lnk_stat[3],&lnk_stat[4],&lnk_stat[5],&lnk_stat[6],&lnk_stat[7],&lnk_stat[8],&lnk_stat[9]) < 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }
       
    if (lnk_stat[socket])
    {
        // close tcp or udp socket if connected
        if (at_exec_cmd(resp, "AT+CIPCLOSE=%d", socket) < 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }            
    }
    // check the network open or not
    if (at_exec_cmd(resp,"AT+NETOPEN?") < 0)
    {
        result = -RT_ERROR;
        
        goto __exit;
    }
    
    if(at_resp_parse_line_args_by_kw(resp, "+NETOPEN:", "+NETOPEN: %d", &activated) < 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }
    
    if (activated)
    {
        // if already open,then close it.
        if (at_exec_cmd(resp,"AT+NETCLOSE") < 0)
        {
            result = -RT_ERROR;
            
            goto __exit;
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
 * open packet network
 */
static int sim76xx_network_socket_open(void)
{
    int result = RT_EOK;
    at_response_t resp = RT_NULL;
    int activated;
 
    resp = at_create_resp(128, 0, rt_tick_from_millisecond(5000));
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        return -RT_ENOMEM;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);
   
    // check the network open or not
    if (at_exec_cmd(resp,"AT+NETOPEN?") < 0)
    {
        result = -RT_ERROR;
        
        goto __exit;
    }
    
    if(at_resp_parse_line_args_by_kw(resp, "+NETOPEN:", "+NETOPEN: %d", &activated) < 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }
    
    if (activated)
    {//network socket is already opened
        goto __exit;
    }else
    {
        // if not opened the open it.
        if (at_exec_cmd(resp,"AT+NETOPEN") < 0)
        {
            result = -RT_ERROR;
            
            goto __exit;
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
static int sim76xx_socket_connect(int socket, char *ip, int32_t port, enum at_socket_type type, rt_bool_t is_client)
{
    at_response_t resp = RT_NULL;
    int result = RT_EOK,event_result = 0;
    rt_bool_t retryed = RT_FALSE;

    RT_ASSERT(ip);
    RT_ASSERT(port >= 0);

    resp = at_create_resp(128, 0, rt_tick_from_millisecond(5000));
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        return -RT_ENOMEM;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

__retry:
    if (is_client)
    {
        //open network socket first(AT+NETOPEN)
        sim76xx_network_socket_open();
        
        switch (type)
        {
        case AT_SOCKET_TCP:        
            /* send AT commands to connect TCP server */
            if (at_exec_cmd(resp, "AT+CIPOPEN=%d,\"TCP\",\"%s\",%d", socket, ip, port) < 0)
            {
                result = -RT_ERROR;
            }
            break;

        case AT_SOCKET_UDP:
            if (at_exec_cmd(resp, "AT+CIPOPEN=%d,\"UDP\",,,%d", socket, port) < 0)
            {
                result = -RT_ERROR;
            }
            strcpy(udp_ipstr[socket],ip);
            udp_port[socket] = port;
            break;

        default:
            LOG_E("Not supported connect type : %d.", type);
            result = -RT_ERROR;
            goto __exit;
        }
    }
    
    /* waiting result event from AT URC, the device default connection timeout is 75 seconds, but it set to 10 seconds is convenient to use.*/
    if (at_socket_event_recv(SET_EVENT(socket, 0), rt_tick_from_millisecond(10 * 1000), RT_EVENT_FLAG_OR) < 0)
    {
        LOG_E("socket (%d) connect failed, wait connect result timeout.", socket);
        result = -RT_ETIMEOUT;
        goto __exit;
    }
    /* waiting OK or failed result */
    if ((event_result = at_socket_event_recv(SIM76XX_EVENT_CONN_OK | SIM76XX_EVENT_CONN_FAIL, rt_tick_from_millisecond(1 * 1000),
            RT_EVENT_FLAG_OR)) < 0)
    {
        LOG_E("socket (%d) connect failed, wait connect OK|FAIL timeout.", socket);
        result = -RT_ETIMEOUT;
        goto __exit;
    }
    /* check result */
    if (event_result & SIM76XX_EVENT_CONN_FAIL)
    {
        if (!retryed)
        {
            LOG_E("socket (%d) connect failed, maybe the socket was not be closed at the last time and now will retry.", socket);
            if (sim76xx_socket_close(socket) < 0)
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

    if (result != RT_EOK && !retryed)
    {
        LOG_D("socket (%d) connect failed, maybe the socket was not be closed at the last time and now will retry.", socket);
        if (sim76xx_socket_close(socket) < 0)
        {
            goto __exit;
        }
        retryed = RT_TRUE;
        result = RT_EOK;
        goto __retry;
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
static int sim76xx_socket_send(int socket, const char *buff, size_t bfsz, enum at_socket_type type)
{
    int result = RT_EOK;
    int event_result = 0;
    at_response_t resp = RT_NULL;
    size_t cur_pkt_size = 0, sent_size = 0;

    RT_ASSERT(buff);
    RT_ASSERT(bfsz > 0);

    resp = at_create_resp(128, 2, rt_tick_from_millisecond(5000));
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        return -RT_ENOMEM;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

    /* set current socket for send URC event */
    cur_socket = socket;
    /* set AT client end sign to deal with '>' sign.*/
    at_set_end_sign('>');

    while (sent_size < bfsz)
    {
        if (bfsz - sent_size < SIM76XX_MODULE_SEND_MAX_SIZE)
        {
            cur_pkt_size = bfsz - sent_size;
        }
        else
        {
            cur_pkt_size = SIM76XX_MODULE_SEND_MAX_SIZE;
        }
        
        switch(type)
        {
        case AT_SOCKET_TCP:
            /* send the "AT+CIPSEND" commands to AT server than receive the '>' response on the first line. */
            if (at_exec_cmd(resp, "AT+CIPSEND=%d,%d", socket, cur_pkt_size) < 0)
            {
                result = -RT_ERROR;
                goto __exit;
            }
            break;
        case AT_SOCKET_UDP:
            /* send the "AT+CIPSEND" commands to AT server than receive the '>' response on the first line. */
            if (at_exec_cmd(resp, "AT+CIPSEND=%d,%d,\"%s\",%d", socket, cur_pkt_size,udp_ipstr[socket],udp_port[socket]) < 0)
            {
                result = -RT_ERROR;
                goto __exit;
            }
            break;
        }
        
        /* send the real data to server or client */
        result = (int) at_client_send(buff + sent_size, cur_pkt_size);
        if (result == 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }

        /* waiting result event from AT URC */
        if (at_socket_event_recv(SET_EVENT(socket, 0), rt_tick_from_millisecond(5 * 1000), RT_EVENT_FLAG_OR) < 0)
        {
            LOG_E("socket (%d) send failed, wait connect result timeout.", socket);
            result = -RT_ETIMEOUT;
            goto __exit;
        }
        /* waiting OK or failed result */
        if ((event_result = at_socket_event_recv(SIM76XX_EVENT_SEND_OK | SIM76XX_EVENT_SEND_FAIL, rt_tick_from_millisecond(5 * 1000),
                RT_EVENT_FLAG_OR)) < 0)
        {
            LOG_E("socket (%d) send failed, wait connect OK|FAIL timeout.", socket);
            result = -RT_ETIMEOUT;
            goto __exit;
        }
        /* check result */
        if (event_result & SIM76XX_EVENT_SEND_FAIL)
        {
            LOG_E("socket (%d) send failed, return failed.", socket);
            result = -RT_ERROR;
            goto __exit;
        }

        if (type == AT_SOCKET_TCP)
        {
            cur_pkt_size = cur_send_bfsz;
        }

        sent_size += cur_pkt_size;
    }

__exit:
    /* reset the end sign for data */
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
 *         -2: wait socket event timeout
 *         -5: no memory
 */
static int sim76xx_domain_resolve(const char *name, char ip[16])
{
#define RESOLVE_RETRY        5

    int i, result = RT_EOK;
    char domain[32] = { 0 };
    char domain_ip[16] = {0};
    at_response_t resp = RT_NULL;

    RT_ASSERT(name);
    RT_ASSERT(ip);

    resp = at_create_resp(128, 0, rt_tick_from_millisecond(5000));
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        return -RT_ENOMEM;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

    for(i = 0; i < RESOLVE_RETRY; i++)
    {
        if (at_exec_cmd(resp, "AT+CDNSGIP=\"%s\"", name) < 0)
        {
            rt_thread_delay(rt_tick_from_millisecond(200));
            /* resolve failed, maybe receive an URC CRLF */
            continue;
        }

        /* parse the third line of response data, get the IP address */
        /* +CDNSGIP: 1,"www.baidu.com","14.215.177.39" */
        if(at_resp_parse_line_args_by_kw(resp, "+CDNSGIP:", "+CDNSGIP: 1,%[^,],\"%[^\"]", domain,domain_ip) < 0)
        {
            rt_thread_delay(rt_tick_from_millisecond(200));
            /* resolve failed, maybe receive an URC CRLF */
            continue;
        }

        if (strlen(domain_ip) < 8)
        {
            rt_thread_delay(rt_tick_from_millisecond(200));
            /* resolve failed, maybe receive an URC CRLF */
            continue;
        }
        else
        {
            strncpy(ip, domain_ip, 15);
            ip[15] = '\0';
            break;
        }
    }

    rt_mutex_release(at_event_lock);

    if (i == RESOLVE_RETRY)
    {
        result = -RT_ERROR;        
    }
    
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
static void sim76xx_socket_set_event_cb(at_socket_evt_t event, at_evt_cb_t cb)
{
    if (event < (sizeof(at_evt_cb_set) / sizeof(at_evt_cb_set[1])))
    {
        at_evt_cb_set[event] = cb;
    }
}

static void urc_send_func(const char *data, rt_size_t size)
{
    int socket = 0;
    int rqst_size;
    int cnf_size;
    
    RT_ASSERT(data && size);

    sscanf(data,"+CIPSEND: %d,%d,%d",&socket,&rqst_size,&cnf_size);
    
    cur_send_bfsz = cnf_size;
    
    at_socket_event_send(SET_EVENT(socket, SIM76XX_EVENT_SEND_OK));
}

static void urc_ping_func(const char *data, rt_size_t size)
{
    static int icmp_seq = 0;
    int i, j = 0;
    int result, recv_len, time, ttl;
    int sent, rcvd, lost, min, max, avg;
    char dst_ip[16] = { 0 };

    RT_ASSERT(data);

    for (i=0;i<size;i++)
    {
        if(*(data+i) == '.')
            j++;
    }
    if (j != 0)
    {
        sscanf(data, "+CPING: %d,%[^,],%d,%d,%d", &result, dst_ip, &recv_len, &time, &ttl);
        if (result == 1)
            LOG_I("%d bytes from %s icmp_seq=%d ttl=%d time=%d ms", recv_len, dst_ip, icmp_seq++, ttl, time);
    }
    else
    {
        sscanf(data, "+CPING: %d,%d,%d,%d,%d,%d,%d", &result, &sent, &rcvd, &lost, &min, &max, &avg);
        if (result == 3)
            LOG_I("%d sent %d received %d lost, min=%dms max=%dms average=%dms", sent, rcvd, lost, min, max, avg);
        if (result == 2)
            LOG_I("ping requst timeout");            
    }
}


static void urc_connect_func(const char *data, rt_size_t size)
{
    int socket = 0;
    int result = 0;

    RT_ASSERT(data && size);

    sscanf(data, "+CIPOPEN: %d,%d", &socket , &result);

    if (result == 0)
    {
        at_socket_event_send(SET_EVENT(socket, SIM76XX_EVENT_CONN_OK));
    }
    else
    {
        at_tcp_ip_errcode_parse(result);
        at_socket_event_send(SET_EVENT(socket, SIM76XX_EVENT_CONN_FAIL));
    }
}

static void urc_close_func(const char *data, rt_size_t size)
{
    int socket = 0;
    int reason = 0;

    RT_ASSERT(data && size);

    sscanf(data, "+IPCLOSE %d,%d", &socket, &reason);
    
    switch(reason)
    {
    case 0:
        LOG_E("socket is closed by local,active");
        break;
    case 1:
        LOG_E("socket is closed by remote,passive");
        break;
    case 2:
        LOG_E("socket is closed for sending timeout");
        break;
    }
    
    /* notice the socket is disconnect by remote */
    if (at_evt_cb_set[AT_SOCKET_EVT_CLOSED])
    {
        at_evt_cb_set[AT_SOCKET_EVT_CLOSED](socket, AT_SOCKET_EVT_CLOSED, RT_NULL, 0);
    }
}

static void urc_recv_func(const char *data, rt_size_t size)
{
    rt_size_t bfsz = 0, temp_size = 0;
    rt_int32_t timeout;
    char *recv_buf = RT_NULL, temp[8];

    RT_ASSERT(data && size);

    /* get the current socket and receive buffer size by receive data */
    sscanf(data, "+IPD%d:",(int *) &bfsz);
    /* get receive timeout by receive buffer length */
    timeout = bfsz * 10;

    if (bfsz == 0)
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
        at_evt_cb_set[AT_SOCKET_EVT_RECV](cur_socket, AT_SOCKET_EVT_RECV, recv_buf, bfsz);
    }
}

static struct at_urc urc_table[] = {
        {"+CIPSEND:",      "\r\n",           urc_send_func},
        {"+CIPOPEN:",      "\r\n",           urc_connect_func},
        {"+CPING:",        "\r\n",           urc_ping_func},
        {"+IPCLOSE",       "\r\n",           urc_close_func},
        {"+IPD",           "\r\n",           urc_recv_func},
};

#define AT_SEND_CMD(resp, cmd)                                                                          \
    do                                                                                                  \
    {                                                                                                   \
        if (at_exec_cmd(at_resp_set_info(resp, 256, 0, rt_tick_from_millisecond(5000)), cmd) < 0)       \
        {                                                                                               \
            LOG_E("RT AT send commands(%s) error!", cmd);                                               \
            result = -RT_ERROR;                                                                         \
            goto __exit;                                                                                \
        }                                                                                               \
    } while(0);                                                                                         \


/**
 * power up sim76xx modem
 */
static void sim76xx_power_on(void)
{
    rt_pin_write(AT_DEVICE_POWER_PIN, PIN_HIGH);
    rt_thread_delay(rt_tick_from_millisecond(300));
    rt_pin_write(AT_DEVICE_POWER_PIN, PIN_LOW);
    
    while (rt_pin_read(AT_DEVICE_STATUS_PIN) == PIN_LOW)
    {
        rt_thread_delay(rt_tick_from_millisecond(10));
    }
}

static void sim76xx_power_off(void)
{
    rt_pin_write(AT_DEVICE_POWER_PIN, PIN_HIGH);
    rt_thread_delay(rt_tick_from_millisecond(3000));
    rt_pin_write(AT_DEVICE_POWER_PIN, PIN_LOW);
    
    while (rt_pin_read(AT_DEVICE_STATUS_PIN) == PIN_HIGH)
    {
        rt_thread_delay(rt_tick_from_millisecond(10));
    }
    rt_pin_write(AT_DEVICE_POWER_PIN, PIN_LOW);
}
        
  
static void sim76xx_init_thread_entry(void *parameter)
{
#define CSQ_RETRY                      20
#define CREG_RETRY                     10
#define CGREG_RETRY                    20
#define CGATT_RETRY                    10
#define CCLK_RETRY                     10
    
    at_response_t resp = RT_NULL;
    rt_err_t result = RT_EOK;
    rt_size_t i, qi_arg[3];
    char parsed_data[20];

    resp = at_create_resp(128, 0, rt_tick_from_millisecond(300));
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        result = -RT_ENOMEM;
        goto __exit;
    }
    
    /* power-up sim76xx */
    sim76xx_power_on();
        
    LOG_D("Start initializing the SIM76XXE module");
    /* wait SIM76XX startup finish, Send AT every 5s, if receive OK, SYNC success*/
    if (at_client_wait_connect(SIM76XX_WAIT_CONNECT_TIME))
    {
        result = -RT_ETIMEOUT;
        goto __exit;
    }
    
    /* disable echo */
    AT_SEND_CMD(resp, "ATE0");
    
    /* get module version */
    AT_SEND_CMD(resp, "ATI");
    
    /* show module version */
    for (i = 0; i < (int) resp->line_counts - 1; i++)
    {
        LOG_D("%s", at_resp_get_line(resp, i + 1));
    }
    /* check SIM card */
    
    AT_SEND_CMD(resp, "AT+CPIN?");
    if (!at_resp_get_line_by_kw(resp, "READY"))
    {
        LOG_E("SIM card detection failed");
        result = -RT_ERROR;
        goto __exit;
    }
    
    /* waiting for dirty data to be digested */
    rt_thread_delay(rt_tick_from_millisecond(10));
    /* check signal strength */
    for (i = 0; i < CSQ_RETRY; i++)
    {
        AT_SEND_CMD(resp, "AT+CSQ");
        at_resp_parse_line_args_by_kw(resp, "+CSQ:", "+CSQ: %d,%d", &qi_arg[0], &qi_arg[1]);
        if (qi_arg[0] != 99)
        {
            LOG_D("Signal strength: %d  Channel bit error rate: %d", qi_arg[0], qi_arg[1]);
            break;
        }
        rt_thread_delay(rt_tick_from_millisecond(1000));
    }
        
    if (i == CSQ_RETRY)
    {
        LOG_E("Signal strength check failed (%s)", parsed_data);
        result = -RT_ERROR;
        goto __exit;
    }
    
    //do not show the prompt when receiving data    
    AT_SEND_CMD(resp, "AT+CIPSRIP=0");
    
    /* check the GSM network is registered */
    for (i = 0; i < CREG_RETRY; i++)
    {
        AT_SEND_CMD(resp,"AT+CREG?");
        at_resp_parse_line_args_by_kw(resp, "+CREG:", "+CREG: %s", &parsed_data);
        if (!strncmp(parsed_data, "0,1", sizeof(parsed_data)) || !strncmp(parsed_data, "0,5", sizeof(parsed_data)))
        {
            LOG_D("GSM network is registered (%s)", parsed_data);
            break;
        }
        rt_thread_delay(rt_tick_from_millisecond(1000));
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
        AT_SEND_CMD(resp,"AT+CGREG?");
        at_resp_parse_line_args_by_kw(resp, "+CGREG:", "+CGREG: %s", &parsed_data);
        if (!strncmp(parsed_data, "0,1", sizeof(parsed_data)) || !strncmp(parsed_data, "0,5", sizeof(parsed_data)))
        {
            LOG_D("GPRS network is registered (%s)", parsed_data);
            break;
        }
        rt_thread_delay(rt_tick_from_millisecond(1000));
    }
    
    if (i == CGREG_RETRY)
    {
        LOG_E("The GPRS network is register failed (%s)", parsed_data);
        result = -RT_ERROR;
        goto __exit;
    }
    
    /* check packet domain attach or detach */
    for (i = 0;i < CGATT_RETRY; i++)
    {
        AT_SEND_CMD(resp,"AT+CGATT?")
        at_resp_parse_line_args_by_kw(resp,"+CGATT:","+CGATT: %s",&parsed_data);
        if (!strncmp(parsed_data,"1",1))
        {
            LOG_I("Packet domain attach");
            break;
        }
        
        rt_thread_delay(rt_tick_from_millisecond(1000));
    }
    
    if (i == CGATT_RETRY)
    {
        LOG_E("The GPRS network attach failed");
        result = -RT_ERROR;
        goto __exit;
    }
    
    /* get real time */
    int year,month,day,hour,min,sec;
    
    for (i = 0;i < CCLK_RETRY;i++)
    {
        if (at_exec_cmd(at_resp_set_info(resp, 256, 0, rt_tick_from_millisecond(5000)), "AT+CCLK?") < 0)
        { 
            rt_thread_delay(rt_tick_from_millisecond(500));
            continue;
        }
        
        //+CCLK: "18/12/22,18:33:12+32"
        if (at_resp_parse_line_args_by_kw(resp,"+CCLK:","+CCLK: \"%d/%d/%d,%d:%d:%d",&year,&month,&day,&hour,&min,&sec) < 0)
        {
            rt_thread_delay(rt_tick_from_millisecond(500));
            continue;
        }
        
        set_date(year + 2000,month,day);
        set_time(hour,min,sec);
        
        break;
    }
    
    if (i == CCLK_RETRY)
    {
        LOG_E("The GPRS network attach failed");
        result = -RT_ERROR;
        goto __exit;
    }
    
    /* set active PDP context's profile number */
    AT_SEND_CMD(resp, "AT+CSOCKSETPN=1");

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    if (!result)
    {
        LOG_I("AT network initialize success!");
        at_socket_init = 1;
    }
    else
    {
        LOG_E("AT network initialize failed (%d)!", result);
    }
}

int sim76xx_net_init(void)
{
#ifdef PKG_AT_INIT_BY_THREAD
    rt_thread_t tid;

    tid = rt_thread_create("sim76xx_net_init", sim76xx_init_thread_entry, RT_NULL,SIM76XX_THREAD_STACK_SIZE, SIM76XX_THREAD_PRIORITY, 20);
    if (tid)
    {
        rt_thread_startup(tid);
    }
    else
    {
        LOG_E("Create AT initialization thread fail!");
    }
#else
    sim76xx_init_thread_entry(RT_NULL);
#endif

    return RT_EOK;
}

int sim76xx_ping(int argc, char **argv)
{
    at_response_t resp = RT_NULL;

    if (argc != 2)
    {
        rt_kprintf("Please input: at_ping <host address>\n");
        return -RT_ERROR;
    }

    resp = at_create_resp(64, 0, rt_tick_from_millisecond(5000));
    if (!resp)
    {
        rt_kprintf("No memory for response structure!\n");
        return -RT_ENOMEM;
    }

    if (at_exec_cmd(resp, "AT+CPING=\"%s\",1,4,64,1000,10000,255", argv[1]) < 0)
    {
        if (resp)
        {
            at_delete_resp(resp);
        }
        rt_kprintf("AT send ping commands error!\n");
        return -RT_ERROR;
    }
    
    if (resp)
    {
        at_delete_resp(resp);
    }

    return RT_EOK;
}

int sim76xx_connect(int argc, char **argv)
{
    int32_t port;

    if (argc != 3)
    {
        rt_kprintf("Please input: at_connect <host address>\n");
        return -RT_ERROR;
    }
    sscanf(argv[2],"%d",&port);
    sim76xx_socket_connect(0, argv[1], port, AT_SOCKET_TCP, 1);

    return RT_EOK;
}

int sim76xx_close(int argc, char **argv)
{
    if (sim76xx_socket_close(0) < 0)
    {
        rt_kprintf("sim76xx_socket_close fail\n");
    }
    else
    {
        rt_kprintf("sim76xx_socket_closeed\n");
    }
    return RT_EOK;
}

int sim76xx_send(int argc, char **argv)
{
    const char *buff = "1234567890\n";
    if (sim76xx_socket_send(0, buff, 11, AT_SOCKET_TCP) < 0)
    {
        rt_kprintf("sim76xx_socket_send fail\n");
    }

    return RT_EOK;
}

int sim76xx_domain(int argc, char **argv)
{
    char ip[16];
    if (sim76xx_domain_resolve("www.baidu.com", ip) < 0)
    {
        rt_kprintf("sim76xx_socket_send fail\n");
    }
    else
    {
        rt_kprintf("baidu.com : %s\n", ip);
    }

    return RT_EOK;
}

int sim76xx_ifconfig(void)
{
    at_response_t resp = RT_NULL;
    char resp_arg[AT_CMD_MAX_LEN] = { 0 };
    rt_err_t result = RT_EOK;

    resp = at_create_resp(128, 2, rt_tick_from_millisecond(300));
    if (!resp)
    {
        rt_kprintf("No memory for response structure!\n");
        return -RT_ENOMEM;
    }
    
    /* Show PDP address */
    AT_SEND_CMD(resp, "AT+CGPADDR");
    at_resp_parse_line_args_by_kw(resp, "+CGPADDR:", "+CGPADDR: %s", &resp_arg);
    rt_kprintf("IP adress : %s\n", resp_arg);

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    return result;
}


#ifdef FINSH_USING_MSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(sim76xx_net_init, at_net_init, initialize AT network);
MSH_CMD_EXPORT_ALIAS(sim76xx_ping, at_ping, AT ping network host);
MSH_CMD_EXPORT_ALIAS(sim76xx_ifconfig, at_ifconfig, list the information of network interfaces);
MSH_CMD_EXPORT_ALIAS(sim76xx_connect, at_connect, AT connect network host);
MSH_CMD_EXPORT_ALIAS(sim76xx_close, at_close, AT close a socket);
MSH_CMD_EXPORT_ALIAS(sim76xx_send, at_send, AT send a pack);
MSH_CMD_EXPORT_ALIAS(sim76xx_domain, at_domain, AT domain resolve);
#endif

static const struct at_device_ops sim76xx_socket_ops = {
    sim76xx_socket_connect,
    sim76xx_socket_close,
    sim76xx_socket_send,
    sim76xx_domain_resolve,
    sim76xx_socket_set_event_cb,
};

static int sim76xx_netdev_add(const char *netdev_name)
{
#define ETHERNET_MTU        1500
#define HWADDR_LEN          8
    struct netdev *netdev = RT_NULL;
    int result = 0;

    RT_ASSERT(netdev_name);

    netdev = (struct netdev *) rt_calloc(1, sizeof(struct netdev));
    if (netdev == RT_NULL)
    {
        return RT_NULL;
    }

    netdev->mtu = ETHERNET_MTU;
    netdev->hwaddr_len = HWADDR_LEN;
    netdev->ops = RT_NULL;

#ifdef SAL_USING_AT
    extern int sal_at_netdev_set_pf_info(struct netdev *netdev);
    /* set the network interface socket/netdb operations */
    sal_at_netdev_set_pf_info(netdev);
#endif

    result = netdev_register(netdev, netdev_name, RT_NULL);

    /*TODO: improve netdev adaptation */
    netdev_low_level_set_status(netdev, RT_TRUE);
    netdev_low_level_set_link_status(netdev, RT_TRUE);
    netdev_low_level_set_dhcp_status(netdev, RT_TRUE);
    netdev->flags |= NETDEV_FLAG_INTERNET_UP;

    return result;
}

static int at_socket_device_init(void)
{
    /* create current AT socket event */
    at_socket_event = rt_event_create("at_se", RT_IPC_FLAG_FIFO);
    if (at_socket_event == RT_NULL)
    {
        LOG_E("RT AT client port initialize failed! at_sock_event create failed!");
        return -RT_ENOMEM;
    }

    /* create current AT socket event lock */
    at_event_lock = rt_mutex_create("at_se", RT_IPC_FLAG_FIFO);
    if (at_event_lock == RT_NULL)
    {
        LOG_E("RT AT client port initialize failed! at_sock_lock create failed!");
        rt_event_delete(at_socket_event);
        return -RT_ENOMEM;
    }

    /* initialize AT client */
    at_client_init(AT_DEVICE_NAME, AT_DEVICE_RECV_BUFF_LEN);

    /* register URC data execution function  */
    at_set_urc_table(urc_table, sizeof(urc_table) / sizeof(urc_table[0]));

    /* add network interface device to netdev list */
    if (sim76xx_netdev_add(SIM76XX_NETDEV_NAME) < 0)
    {
        LOG_E("SIM76xx network interface device(%d) add failed.", SIM76XX_NETDEV_NAME);
        return -RT_ENOMEM;
    }

    /* initialize sim76xx pin config */
    rt_pin_mode(AT_DEVICE_POWER_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(AT_DEVICE_STATUS_PIN, PIN_MODE_INPUT);
    
    /* initialize sim76xx network */
    sim76xx_net_init();

    /* set sim76xx AT Socket options */
    at_socket_device_register(&sim76xx_socket_ops);

    return RT_EOK;
}
INIT_APP_EXPORT(at_socket_device_init);

#endif /* AT_DEVICE_SIM76XX */
