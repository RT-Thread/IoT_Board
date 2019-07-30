/*
 * File      : at_socket_esp8266.c
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
 * 2018-06-20     chenyong     first version
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

#define LOG_TAG              "at.esp8266"
#include <at_log.h>

#ifdef AT_DEVICE_ESP8266

#define ESP8266_MODULE_SEND_MAX_SIZE   2048
#define ESP8266_WAIT_CONNECT_TIME      5000
#define ESP8266_THREAD_STACK_SIZE      1024
#define ESP8266_THREAD_PRIORITY        (RT_THREAD_PRIORITY_MAX/2)

/* set real event by current socket and current state */
#define SET_EVENT(socket, event)       (((socket + 1) << 16) | (event))

/* AT socket event type */
#define ESP8266_EVENT_CONN_OK          (1L << 0)
#define ESP8266_EVENT_SEND_OK          (1L << 1)
#define ESP8266_EVENT_RECV_OK          (1L << 2)
#define ESP8266_EVNET_CLOSE_OK         (1L << 3)
#define ESP8266_EVENT_CONN_FAIL        (1L << 4)
#define ESP8266_EVENT_SEND_FAIL        (1L << 5)

#define ESP8266_NETDEV_NAME            "esp8266"

static int cur_socket;
static int cur_send_bfsz;
static struct rt_delayed_work esp8266_net_work;
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
static int esp8266_socket_close(int socket)
{
    at_response_t resp = RT_NULL;
    int result = RT_EOK;

    resp = at_create_resp(64, 0, rt_tick_from_millisecond(5000));
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        return -RT_ENOMEM;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

    if (at_exec_cmd(resp, "AT+CIPCLOSE=%d", socket) < 0)
    {
        result = -RT_ERROR;
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
static int esp8266_socket_connect(int socket, char *ip, int32_t port, enum at_socket_type type, rt_bool_t is_client)
{
    at_response_t resp = RT_NULL;
    int result = RT_EOK;
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
        switch (type)
        {
        case AT_SOCKET_TCP:
            /* send AT commands to connect TCP server */
            if (at_exec_cmd(resp, "AT+CIPSTART=%d,\"TCP\",\"%s\",%d,60", socket, ip, port) < 0)
            {
                result = -RT_ERROR;
            }
            break;

        case AT_SOCKET_UDP:
            if (at_exec_cmd(resp, "AT+CIPSTART=%d,\"UDP\",\"%s\",%d", socket, ip, port) < 0)
            {
                result = -RT_ERROR;
            }
            break;

        default:
            LOG_E("Not supported connect type : %d.", type);
            result = -RT_ERROR;
            goto __exit;
        }
    }

    if (result != RT_EOK && !retryed)
    {
        LOG_D("socket (%d) connect failed, maybe the socket was not be closed at the last time and now will retry.", socket);
        if (esp8266_socket_close(socket) < 0)
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
static int esp8266_socket_send(int socket, const char *buff, size_t bfsz, enum at_socket_type type)
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
        if (bfsz - sent_size < ESP8266_MODULE_SEND_MAX_SIZE)
        {
            cur_pkt_size = bfsz - sent_size;
        }
        else
        {
            cur_pkt_size = ESP8266_MODULE_SEND_MAX_SIZE;
        }

        /* send the "AT+CIPSEND" commands to AT server than receive the '>' response on the first line. */
        if (at_exec_cmd(resp, "AT+CIPSEND=%d,%d", socket, cur_pkt_size) < 0)
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
        if (at_socket_event_recv(SET_EVENT(socket, 0), rt_tick_from_millisecond(5 * 1000), RT_EVENT_FLAG_OR) < 0)
        {
            LOG_E("socket (%d) send failed, wait connect result timeout.", socket);
            result = -RT_ETIMEOUT;
            goto __exit;
        }
        /* waiting OK or failed result */
        if ((event_result = at_socket_event_recv(ESP8266_EVENT_SEND_OK | ESP8266_EVENT_SEND_FAIL, rt_tick_from_millisecond(5 * 1000),
                RT_EVENT_FLAG_OR)) < 0)
        {
            LOG_E("socket (%d) send failed, wait connect OK|FAIL timeout.", socket);
            result = -RT_ETIMEOUT;
            goto __exit;
        }
        /* check result */
        if (event_result & ESP8266_EVENT_SEND_FAIL)
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
static int esp8266_domain_resolve(const char *name, char ip[16])
{
#define RESOLVE_RETRY        5

    int i, result = RT_EOK;
    char recv_ip[16] = { 0 };
    at_response_t resp = RT_NULL;

    RT_ASSERT(name);
    RT_ASSERT(ip);

    resp = at_create_resp(128, 0, rt_tick_from_millisecond(20000));
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        return -RT_ENOMEM;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

    for (i = 0; i < RESOLVE_RETRY; i++)
    {
        if (at_exec_cmd(resp, "AT+CIPDOMAIN=\"%s\"", name) < 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }

        /* parse the third line of response data, get the IP address */
        if (at_resp_parse_line_args_by_kw(resp, "+CIPDOMAIN:", "+CIPDOMAIN:%s", recv_ip) < 0)
        {
            rt_thread_delay(rt_tick_from_millisecond(100));
            /* resolve failed, maybe receive an URC CRLF */
            continue;
        }

        if (strlen(recv_ip) < 8)
        {
            rt_thread_delay(rt_tick_from_millisecond(100));
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
static void esp8266_socket_set_event_cb(at_socket_evt_t event, at_evt_cb_t cb)
{
    if (event < sizeof(at_evt_cb_set) / sizeof(at_evt_cb_set[1]))
    {
        at_evt_cb_set[event] = cb;
    }
}

static void urc_send_func(const char *data, rt_size_t size)
{
    RT_ASSERT(data && size);

    if (strstr(data, "SEND OK"))
    {
        at_socket_event_send(SET_EVENT(cur_socket, ESP8266_EVENT_SEND_OK));
    }
    else if (strstr(data, "SEND FAIL"))
    {
        at_socket_event_send(SET_EVENT(cur_socket, ESP8266_EVENT_SEND_FAIL));
    }
}

static void urc_send_bfsz_func(const char *data, rt_size_t size)
{
    int send_bfsz = 0;

    RT_ASSERT(data && size);

    sscanf(data, "Recv %d bytes", &send_bfsz);

    cur_send_bfsz = send_bfsz;
}

static void urc_close_func(const char *data, rt_size_t size)
{
    int socket = 0;

    RT_ASSERT(data && size);

    sscanf(data, "%d,CLOSED", &socket);
    /* notice the socket is disconnect by remote */
    if (at_evt_cb_set[AT_SOCKET_EVT_CLOSED])
    {
        at_evt_cb_set[AT_SOCKET_EVT_CLOSED](socket, AT_SOCKET_EVT_CLOSED, RT_NULL, 0);
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
    sscanf(data, "+IPD,%d,%d:", &socket, (int *) &bfsz);
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

static void urc_busy_p_func(const char *data, rt_size_t size)
{
    RT_ASSERT(data && size);

    LOG_D("system is processing a commands and it cannot respond to the current commands.");
}

static void urc_busy_s_func(const char *data, rt_size_t size)
{
    RT_ASSERT(data && size);

    LOG_D("system is sending data and it cannot respond to the current commands.");
}

static void urc_func(const char *data, rt_size_t size)
{
    RT_ASSERT(data && size);

    if (strstr(data, "WIFI CONNECTED"))
    {
        netdev_low_level_set_link_status(netdev_get_by_name(ESP8266_NETDEV_NAME), RT_TRUE);
        rt_work_submit(&(esp8266_net_work.work), RT_TICK_PER_SECOND);
        LOG_I("ESP8266 WIFI is connected.");
    }
    else if (strstr(data, "WIFI DISCONNECT"))
    {
        netdev_low_level_set_link_status(netdev_get_by_name(ESP8266_NETDEV_NAME), RT_FALSE);
        LOG_I("ESP8266 WIFI is disconnect.");
    }
}

static struct at_urc urc_table[] = {
        {"SEND OK",          "\r\n",           urc_send_func},
        {"SEND FAIL",        "\r\n",           urc_send_func},
        {"Recv",             "bytes\r\n",      urc_send_bfsz_func},
        {"",                 ",CLOSED\r\n",    urc_close_func},
        {"+IPD",             ":",              urc_recv_func},
        {"busy p",           "\r\n",           urc_busy_p_func},
        {"busy s",           "\r\n",           urc_busy_s_func},
        {"WIFI CONNECTED",   "\r\n",           urc_func},
        {"WIFI DISCONNECT",  "\r\n",           urc_func},
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

static void exp8266_get_netdev_info(struct rt_work *work, void *work_data)
{
#define AT_ADDR_LEN     32
    at_response_t resp = RT_NULL;
    char ip[AT_ADDR_LEN], mac[AT_ADDR_LEN];
    char gateway[AT_ADDR_LEN], netmask[AT_ADDR_LEN];
    char dns_server1[AT_ADDR_LEN] = {0}, dns_server2[AT_ADDR_LEN] = {0};
    const char *resp_expr = "%*[^\"]\"%[^\"]\"";
    const char *resp_dns = "+CIPDNS_CUR:%s";
    ip_addr_t sal_ip_addr;
    rt_uint32_t mac_addr[6] = {0};
    rt_uint32_t num = 0; 
    rt_uint8_t dhcp_stat = 0;
    struct netdev *netdev = RT_NULL;

    netdev = (struct netdev *)work_data;

    rt_memset(ip, 0x00, sizeof(ip));
    rt_memset(mac, 0x00, sizeof(mac));
    rt_memset(gateway, 0x00, sizeof(gateway));
    rt_memset(netmask, 0x00, sizeof(netmask));

    resp = at_create_resp(512, 0, rt_tick_from_millisecond(300));
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        return;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);
    /* send mac addr query commond "AT+CIFSR" and wait response */
    if (at_exec_cmd(resp, "AT+CIFSR") < 0)
    {
        LOG_E("AT send \"AT+CIFSR\" commands error!");
        goto __exit;
    }

    if (at_resp_parse_line_args(resp, 2, resp_expr, mac) <= 0)
    {
        LOG_E("Parse error, current line buff : %s", at_resp_get_line(resp, 2));
        goto __exit;
    }

    /* send addr info query commond "AT+CIPSTA?" and wait response */
    if (at_exec_cmd(resp, "AT+CIPSTA?") < 0)
    {
        LOG_E("AT send \"AT+CIPSTA?\" commands error!");
        goto __exit;
    }

    if (at_resp_parse_line_args(resp, 1, resp_expr, ip) <= 0 ||
            at_resp_parse_line_args(resp, 2, resp_expr, gateway) <= 0 ||
            at_resp_parse_line_args(resp, 3, resp_expr, netmask) <= 0)
    {
        LOG_E("Prase \"AT+CIPSTA?\" commands resposne data error!");
        goto __exit;
    }

    /* set netdev info */
    inet_aton(ip, &sal_ip_addr);
    netdev_low_level_set_ipaddr(netdev, &sal_ip_addr);
    inet_aton(gateway, &sal_ip_addr);
    netdev_low_level_set_gw(netdev, &sal_ip_addr);
    inet_aton(netmask, &sal_ip_addr);
    netdev_low_level_set_netmask(netdev, &sal_ip_addr);
    sscanf(mac, "%x:%x:%x:%x:%x:%x", &mac_addr[0], &mac_addr[1], &mac_addr[2], &mac_addr[3], &mac_addr[4], &mac_addr[5]);
    for (num = 0; num < netdev->hwaddr_len; num++)
    {
        netdev->hwaddr[num] = mac_addr[num];
    }

    /* send dns server query commond "AT+CIPDNS_CUR?" and wait response */
    if (at_exec_cmd(resp, "AT+CIPDNS_CUR?") < 0)
    {
        LOG_W("Get dns server failed! Please check and update your firmware to support the \"AT+CIPDNS_CUR?\" command.");
        goto __exit;
    }

    if (at_resp_parse_line_args(resp, 1, resp_dns, dns_server1) <= 0 &&
            at_resp_parse_line_args(resp, 2, resp_dns, dns_server2) <= 0)
    {
        LOG_E("Prase \"AT+CIPDNS_CUR?\" commands resposne data error!");
        goto __exit;
    }

    if (strlen(dns_server1) > 0)
    {
        inet_aton(dns_server1, &sal_ip_addr);
        netdev_low_level_set_dns_server(netdev, 0, &sal_ip_addr);
    }

    if (strlen(dns_server2) > 0)
    {
        inet_aton(dns_server2, &sal_ip_addr);
        netdev_low_level_set_dns_server(netdev, 1, &sal_ip_addr);
    }

    /* send DHCP query commond " AT+CWDHCP_CUR?" and wait response */
    if (at_exec_cmd(resp, "AT+CWDHCP_CUR?") < 0)
    {
        LOG_E("AT send \"AT+CWDHCP_CUR?\" commands error!");
        goto __exit;
    }

    /* parse response data, get the DHCP status */
    if (at_resp_parse_line_args_by_kw(resp, "+CWDHCP_CUR:", "+CWDHCP_CUR:%d", &dhcp_stat) < 0)
    {
        LOG_E("get DHCP status failed!");
        goto __exit;
    }

    /* Bit0 - SoftAP DHCP status, Bit1 - Station DHCP status */
    if (dhcp_stat & 0x02)
    {
        netdev_low_level_set_dhcp_status(netdev, RT_TRUE);
    }
    else
    {
        netdev_low_level_set_dhcp_status(netdev, RT_FALSE);
    }

__exit:
    rt_mutex_release(at_event_lock);

    if (resp)
    {
        at_delete_resp(resp);
    }
}

static void esp8266_init_thread_entry(void *parameter)
{
    at_response_t resp = RT_NULL;
    rt_err_t result = RT_EOK;
    rt_size_t i;

    resp = at_create_resp(128, 0, rt_tick_from_millisecond(5000));
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        result = -RT_ENOMEM;
        goto __exit;
    }

    rt_thread_delay(rt_tick_from_millisecond(5000));
    /* reset module */
    AT_SEND_CMD(resp, "AT+RST");
    /* reset waiting delay */
    rt_thread_delay(rt_tick_from_millisecond(1000));
    /* disable echo */
    AT_SEND_CMD(resp, "ATE0");
    /* set current mode to Wi-Fi station */
    AT_SEND_CMD(resp, "AT+CWMODE=1");
    /* get module version */
    AT_SEND_CMD(resp, "AT+GMR");
    /* show module version */
    for (i = 0; i < resp->line_counts - 1; i++)
    {
        LOG_D("%s", at_resp_get_line(resp, i + 1));
    }
    /* connect to WiFi AP */
    if (at_exec_cmd(at_resp_set_info(resp, 128, 0, 20 * RT_TICK_PER_SECOND), "AT+CWJAP=\"%s\",\"%s\"",
            AT_DEVICE_WIFI_SSID, AT_DEVICE_WIFI_PASSWORD) != RT_EOK)
    {
        LOG_E("AT network initialize failed, check ssid(%s) and password(%s).", AT_DEVICE_WIFI_SSID, AT_DEVICE_WIFI_PASSWORD);
        result = -RT_ERROR;
        goto __exit;
    }

    AT_SEND_CMD(resp, "AT+CIPMUX=1");

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    if (!result)
    {
        netdev_low_level_set_status(netdev_get_by_name(ESP8266_NETDEV_NAME), RT_TRUE);
        LOG_I("AT network initialize success!");
    }
    else
    {
        netdev_low_level_set_status(netdev_get_by_name(ESP8266_NETDEV_NAME), RT_FALSE);
        LOG_E("AT network initialize failed (%d)!", result);
    }
}

int esp8266_net_init(void)
{
#ifdef PKG_AT_INIT_BY_THREAD
    rt_thread_t tid;

    tid = rt_thread_create("esp8266_net_init", esp8266_init_thread_entry, RT_NULL, ESP8266_THREAD_STACK_SIZE, ESP8266_THREAD_PRIORITY, 20);
    if (tid)
    {
        rt_thread_startup(tid);
    }
    else
    {
        LOG_E("Create AT initialization thread fail!");
    }
#else
    esp8266_init_thread_entry(RT_NULL);
#endif

    return RT_EOK;
}
#ifdef FINSH_USING_MSH
    #include <finsh.h>
    MSH_CMD_EXPORT_ALIAS(esp8266_net_init, at_net_init, initialize AT network);
#endif

static const struct at_device_ops esp8266_socket_ops =
{
    esp8266_socket_connect,
    esp8266_socket_close,
    esp8266_socket_send,
    esp8266_domain_resolve,
    esp8266_socket_set_event_cb,
};

static int esp8266_netdev_set_up(struct netdev *netdev)
{
    netdev_low_level_set_status(netdev, RT_TRUE);
    LOG_D("esp8266 network interface set up status.");
    return RT_EOK;
}

static int esp8266_netdev_set_down(struct netdev *netdev)
{
    netdev_low_level_set_status(netdev, RT_FALSE);
    LOG_D("esp8266 network interface set down status.");
    return RT_EOK;
}

static int esp8266_netdev_set_addr_info(struct netdev *netdev, ip_addr_t *ip_addr, ip_addr_t *netmask, ip_addr_t *gw)
{
#define RESP_SIZE               128
#define IPV4_ADDR_STRLEN_MAX    16
    at_response_t resp = RT_NULL;
    int result = RT_EOK;
    char esp8266_ip_addr[IPV4_ADDR_STRLEN_MAX] = {0};
    char esp8266_gw_addr[IPV4_ADDR_STRLEN_MAX] = {0};
    char esp8266_netmask_addr[IPV4_ADDR_STRLEN_MAX] = {0};

    RT_ASSERT(netdev);
    RT_ASSERT(ip_addr || netmask || gw);

    resp = at_create_resp(RESP_SIZE, 0, rt_tick_from_millisecond(300));
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        result = -RT_ENOMEM;
        goto __exit;
    }

    /* Convert numeric IP address into decimal dotted ASCII representation. */
    if (ip_addr)
        rt_memcpy(esp8266_ip_addr, inet_ntoa(*ip_addr), IPV4_ADDR_STRLEN_MAX);
    else
        rt_memcpy(esp8266_ip_addr, inet_ntoa(netdev->ip_addr), IPV4_ADDR_STRLEN_MAX);

    if (gw)
        rt_memcpy(esp8266_gw_addr, inet_ntoa(*gw), IPV4_ADDR_STRLEN_MAX);
    else
        rt_memcpy(esp8266_gw_addr, inet_ntoa(netdev->gw), IPV4_ADDR_STRLEN_MAX);

    if (netmask)
        rt_memcpy(esp8266_netmask_addr, inet_ntoa(*netmask), IPV4_ADDR_STRLEN_MAX);
    else
        rt_memcpy(esp8266_netmask_addr, inet_ntoa(netdev->netmask), IPV4_ADDR_STRLEN_MAX);

    /* send addr info set commond "AT+CIPSTA_CUR=<ip>[,<gateway>,<netmask>]" and wait response */
    if (at_exec_cmd(resp, "AT+CIPSTA_CUR=\"%s\",\"%s\",\"%s\"", esp8266_ip_addr, esp8266_gw_addr, esp8266_netmask_addr) < 0)
    {
        LOG_D("esp8266 set addr info failed.");
        result = -RT_ERROR;
    }
    else
    {
        /* Update netdev information */
        if (ip_addr)
            netdev_low_level_set_ipaddr(netdev, ip_addr);

        if (gw)
            netdev_low_level_set_gw(netdev, gw);

        if (netmask)
            netdev_low_level_set_netmask(netdev, netmask);

        LOG_D("esp8266 set addr info successfully.");
    }

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    return result;
}

static int esp8266_netdev_set_dns_server(struct netdev *netdev, uint8_t dns_num, ip_addr_t *dns_server)
{
#define RESP_SIZE           128
    at_response_t resp = RT_NULL;
    int result = RT_EOK;

    RT_ASSERT(netdev);
    RT_ASSERT(dns_server);

    resp = at_create_resp(RESP_SIZE, 0, rt_tick_from_millisecond(300));
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        return -RT_ENOMEM;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

    /* send dns server set commond "AT+CIPDNS_CUR=<enable>[,<DNS	server0>,<DNS	server1>]" and wait response */
    if (at_exec_cmd(resp, "AT+CIPDNS_CUR=1,\"%s\"", inet_ntoa(*dns_server)) < 0)
    {
        LOG_E("set dns server(%s) failed", inet_ntoa(*dns_server));
        result = -RT_ERROR;
    }
    else
    {
        netdev_low_level_set_dns_server(netdev, dns_num, dns_server);
        LOG_D("esp8266 set dns server successfully.");
    }

    if (resp)
    {
        at_delete_resp(resp);
    }

    rt_mutex_release(at_event_lock);

    return result;
}

static int esp8266_netdev_set_dhcp(struct netdev *netdev, rt_bool_t is_enabled)
{
#define ESP8266_STATION     1
#define RESP_SIZE           128

    at_response_t resp = RT_NULL;
    int result = RT_EOK;

    RT_ASSERT(netdev);

    resp = at_create_resp(RESP_SIZE, 0, rt_tick_from_millisecond(300));
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        return -RT_ENOMEM;
    }
    
    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

    /* send dhcp set commond "AT+CWDHCP_CUR=<mode>,<en>" and wait response */
    if (at_exec_cmd(resp, "AT+CWDHCP_CUR=%d,%d", ESP8266_STATION, is_enabled) < 0)
    {
        LOG_E("set dhcp status(%d) failed", is_enabled);
        result = -RT_ERROR;
        goto __exit;
    }
    else
    {
        netdev_low_level_set_dhcp_status(netdev, is_enabled);
        LOG_D("esp8266 set dhcp successfully.");
    }

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    rt_mutex_release(at_event_lock);

    return result;
}

static int esp8266_netdev_ping(struct netdev *netdev, const char *host, size_t data_len, uint32_t timeout, struct netdev_ping_resp *ping_resp)
{
#define ESP8266_PING_IP_SIZE         16

    at_response_t resp = RT_NULL;
    rt_err_t result = RT_EOK;
    int req_time;
    char ip_addr[ESP8266_PING_IP_SIZE] = {0};

    RT_ASSERT(netdev);
    RT_ASSERT(host);
    RT_ASSERT(ping_resp);

    resp = at_create_resp(64, 0, timeout);
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        return -RT_ENOMEM;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

    /* send domain commond "AT+CIPDOMAIN=<domain name>" and wait response */
    if (at_exec_cmd(resp, "AT+CIPDOMAIN=\"%s\"", host) < 0)
    {
        LOG_D("ping: send commond AT+CIPDOMAIN=<domain name> failed");
        result = -RT_ERROR;
        goto __exit;
    }

    /* parse the third line of response data, get the IP address */
    if (at_resp_parse_line_args_by_kw(resp, "+CIPDOMAIN:", "+CIPDOMAIN:%s", ip_addr) < 0)
    {
        LOG_E("ping: get the IP address failed");
        result = -RT_ERROR;
        goto __exit;
    }

    /* send ping commond "AT+PING=<IP>" and wait response */
    if (at_exec_cmd(resp, "AT+PING=\"%s\"", host) < 0)
    {
        LOG_D("ping: unknown remote server host");
        result = -RT_ERROR;
        goto __exit;
    }

    if (at_resp_parse_line_args_by_kw(resp, "+", "+%d", &req_time) < 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }

    if (req_time)
    {
        inet_aton(ip_addr, &(ping_resp->ip_addr));
        ping_resp->data_len = data_len;
        ping_resp->ttl = 0;
        ping_resp->ticks = req_time;
    }

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    rt_mutex_release(at_event_lock);

    return result;
}

void esp8266_netdev_netstat(struct netdev *netdev)
{
#define ESP8266_NETSTAT_RESP_SIZE         320
#define ESP8266_NETSTAT_TYPE_SIZE         4
#define ESP8266_NETSTAT_IPADDR_SIZE       17
#define ESP8266_NETSTAT_EXPRESSION        "+CIPSTATUS:%*d,\"%[^\"]\",\"%[^\"]\",%d,%d,%*d"

    at_response_t resp = RT_NULL;
    int remote_port, local_port, i;
    char *type = RT_NULL;
    char *ipaddr = RT_NULL;

    type = rt_calloc(1, ESP8266_NETSTAT_TYPE_SIZE);
    ipaddr = rt_calloc(1, ESP8266_NETSTAT_IPADDR_SIZE);
    if ((type && ipaddr) == RT_NULL)
    {
        LOG_E("No memory for response structure!");
        goto __exit;
    }

    resp = at_create_resp(ESP8266_NETSTAT_RESP_SIZE, 0, rt_tick_from_millisecond(5000));
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        goto __exit;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

    /* send network connection information commond "AT+CIPSTATUS" and wait response */
    if (at_exec_cmd(resp, "AT+CIPSTATUS") < 0)
    {
        LOG_E("netstat: send commond AT+CIPSTATUS failed");
        goto __exit;
    }

    for (i = 1; i <= resp->line_counts; i++)
    {
        if (strstr(at_resp_get_line(resp, i), "+CIPSTATUS"))
        {
            /* parse the third line of response data, get the network connection information */
            if (at_resp_parse_line_args(resp, i, ESP8266_NETSTAT_EXPRESSION, type, ipaddr, &remote_port, &local_port) < 0)
                goto __exit;
            else
            {
                LOG_RAW("%s: %s:%d ==> %s:%d\n", type, inet_ntoa(netdev->ip_addr), local_port, ipaddr, remote_port);
            }
        }
    }

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    if (type)
    {
        rt_free(type);
    }

    if (ipaddr)
    {
        rt_free(ipaddr);
    }

    rt_mutex_release(at_event_lock);
}

const struct netdev_ops esp8266_netdev_ops =
{
    esp8266_netdev_set_up,
    esp8266_netdev_set_down,

    esp8266_netdev_set_addr_info,
    esp8266_netdev_set_dns_server,
    esp8266_netdev_set_dhcp,

    esp8266_netdev_ping,
    esp8266_netdev_netstat,
};

static struct netdev *esp8266_netdev_add(const char *netdev_name)
{
#define ETHERNET_MTU        1500
#define HWADDR_LEN          6
    struct netdev *netdev = RT_NULL;

    RT_ASSERT(netdev_name);

    netdev = (struct netdev *) rt_calloc(1, sizeof(struct netdev));
    if (netdev == RT_NULL)
    {
        return RT_NULL;
    }

    netdev->mtu = ETHERNET_MTU;
    netdev->ops = &esp8266_netdev_ops;
    netdev->hwaddr_len = HWADDR_LEN;

#ifdef SAL_USING_AT
    extern int sal_at_netdev_set_pf_info(struct netdev *netdev);
    /* set the network interface socket/netdb operations */
    sal_at_netdev_set_pf_info(netdev);
#endif

    netdev_register(netdev, netdev_name, RT_NULL);

    return netdev;
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

    /* Add esp8266 to the netdev list */
    esp8266_netdev_add(ESP8266_NETDEV_NAME);

    /* initialize esp8266 net workqueue */
    rt_delayed_work_init(&esp8266_net_work, exp8266_get_netdev_info, (void *)netdev_get_by_name(ESP8266_NETDEV_NAME));

    /* initialize esp8266 network */
    esp8266_net_init();

    /* set esp8266 AT Socket options */
    at_socket_device_register(&esp8266_socket_ops);

    return RT_EOK;
}
INIT_APP_EXPORT(at_socket_device_init);

#endif /* AT_DEVICE_ESP8266 */
