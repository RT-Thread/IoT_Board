/*
 * File      : at_socket_ec20.c
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
 * 2018-08-12     Marcus       port to ec20
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

#define LOG_TAG              "at.ec20"
#include <at_log.h>

#ifdef AT_DEVICE_EC20

#define EC20_NETDEV_NAME               "ec20"

#define EC20_MODULE_SEND_MAX_SIZE       1460
#define EC20_WAIT_CONNECT_TIME          5000
#define EC20_THREAD_STACK_SIZE          1024
#define EC20_THREAD_PRIORITY            (RT_THREAD_PRIORITY_MAX/2)

/* set real event by current socket and current state */
#define SET_EVENT(socket, event)       (((socket + 1) << 16) | (event))

/* AT socket event type */
#define EC20_EVENT_CONN_OK              (1L << 0)
#define EC20_EVENT_SEND_OK              (1L << 1)
#define EC20_EVENT_RECV_OK              (1L << 2)
#define EC20_EVNET_CLOSE_OK             (1L << 3)
#define EC20_EVENT_CONN_FAIL            (1L << 4)
#define EC20_EVENT_SEND_FAIL            (1L << 5)
#define EC20_EVENT_DOMAIN_OK            (1L << 6)

/* AT+QICSGP command default*/
char *QICSGP_CHINA_MOBILE = "AT+QICSGP=1,1,\"CMNET\",\"\",\"\",0";
char *QICSGP_CHINA_UNICOM = "AT+QICSGP=1,1,\"UNINET\",\"\",\"\",0";
char *QICSGP_CHINA_TELECOM = "AT+QICSGP=1,1,\"CTNET\",\"\",\"\",0";

static int cur_socket;
static char recv_ip[16] = { 0 };
static rt_event_t at_socket_event;
static rt_mutex_t at_event_lock;
static at_evt_cb_t at_evt_cb_set[] = {
        [AT_SOCKET_EVT_RECV] = NULL,
        [AT_SOCKET_EVT_CLOSED] = NULL,
};

static void at_cme_errcode_parse(int result)
{
    switch(result)
    {
    case 0   : LOG_E("%d : Phone failure",                result); break;
    case 1   : LOG_E("%d : No connection to phone",       result); break;
    case 2   : LOG_E("%d : Phone-adaptor link reserved",  result); break;
    case 3   : LOG_E("%d : Operation not allowed",        result); break;
    case 4   : LOG_E("%d : Operation not supported",      result); break;
    case 5   : LOG_E("%d : PH-SIM PIN required",          result); break;
    case 6   : LOG_E("%d : PH-FSIM PIN required",         result); break;
    case 7   : LOG_E("%d : PH-FSIM PUK required",         result); break;
    case 10  : LOG_E("%d : SIM not inserted",             result); break;
    case 11  : LOG_E("%d : SIM PIN required",             result); break;
    case 12  : LOG_E("%d : SIM PUK required",             result); break;
    case 13  : LOG_E("%d : SIM failure",                  result); break;
    case 14  : LOG_E("%d : SIM busy",                     result); break;
    case 15  : LOG_E("%d : SIM wrong",                    result); break;
    case 16  : LOG_E("%d : Incorrect password",           result); break;
    case 17  : LOG_E("%d : SIM PIN2 required",            result); break;
    case 18  : LOG_E("%d : SIM PUK2 required",            result); break;
    case 20  : LOG_E("%d : Memory full",                  result); break;
    case 21  : LOG_E("%d : Invalid index",                result); break;
    case 22  : LOG_E("%d : Not found",                    result); break;
    case 23  : LOG_E("%d : Memory failure",               result); break;
    case 24  : LOG_E("%d : Text string too long",         result); break;
    case 25  : LOG_E("%d : Invalid characters in text string", result); break;
    case 26  : LOG_E("%d : Dial string too long",         result); break;
    case 27  : LOG_E("%d : Invalid characters in dial string", result); break;
    case 30  : LOG_E("%d : No network service",           result); break;
    case 31  : LOG_E("%d : Network timeout",              result); break;
    case 32  : LOG_E("%d : Network not allowed - emergency calls only", result); break;
    case 40  : LOG_E("%d : Network personalization PIN required", result); break;
    case 41  : LOG_E("%d : Network personalization PUK required", result); break;
    case 42  : LOG_E("%d : Network subset personalization PIN required", result); break;
    case 43  : LOG_E("%d : Network subset personalization PUK required", result); break;
    case 44  : LOG_E("%d : Service provider personalization PIN required", result); break;
    case 45  : LOG_E("%d : Service provider personalization PUK required", result); break;
    case 46  : LOG_E("%d : Corporate personalization PIN required", result); break;
    case 47  : LOG_E("%d : Corporate personalization PUK required", result); break;
    case 901 : LOG_E("%d : Audio unknown error",          result); break;
    case 902 : LOG_E("%d : Audio invalid parameters",     result); break;
    case 903 : LOG_E("%d : Audio operation not supported", result); break;
    case 904 : LOG_E("%d : Audio device busy",            result); break;
    default  : LOG_E("%d : Unknown err code",             result); break;
    }
}

static void at_cms_errcode_parse(int result)
{
    switch(result)
    {
    case 300 : LOG_E("%d : ME failure",                   result); break;
    case 301 : LOG_E("%d : SMS ME reserved",              result); break;
    case 302 : LOG_E("%d : Operation not allowed",        result); break;
    case 303 : LOG_E("%d : Operation not supported",      result); break;
    case 304 : LOG_E("%d : Invalid PDU mode",             result); break;
    case 305 : LOG_E("%d : Invalid text mode",            result); break;
    case 310 : LOG_E("%d : SIM not inserted",             result); break;
    case 311 : LOG_E("%d : SIM pin necessary",            result); break;
    case 312 : LOG_E("%d : PH SIM pin necessary",         result); break;
    case 313 : LOG_E("%d : SIM failure",                  result); break;
    case 314 : LOG_E("%d : SIM busy",                     result); break;
    case 315 : LOG_E("%d : SIM wrong",                    result); break;
    case 316 : LOG_E("%d : SIM PUK required",             result); break;
    case 317 : LOG_E("%d : SIM PIN2 required",            result); break;
    case 318 : LOG_E("%d : SIM PUK2 required",            result); break;
    case 320 : LOG_E("%d : Memory failure",               result); break;
    case 321 : LOG_E("%d : Invalid memory index",         result); break;
    case 322 : LOG_E("%d : Memory full",                  result); break;
    case 330 : LOG_E("%d : SMSC address unknown",         result); break;
    case 331 : LOG_E("%d : No network",                   result); break;
    case 332 : LOG_E("%d : Network timeout",              result); break;
    case 500 : LOG_E("%d : Unknown",                      result); break;
    case 512 : LOG_E("%d : SIM not ready",                result); break;
    case 513 : LOG_E("%d : Message length exceeds",       result); break;
    case 514 : LOG_E("%d : Invalid request parameters",   result); break;
    case 515 : LOG_E("%d : ME storage failure",           result); break;
    case 517 : LOG_E("%d : Invalid service mode",         result); break;
    case 528 : LOG_E("%d : More message to send state error", result); break;
    case 529 : LOG_E("%d : MO SMS is not allow",          result); break;
    case 530 : LOG_E("%d : GPRS is suspended",            result); break;
    case 531 : LOG_E("%d : ME storage full",              result); break;
    default  : LOG_E("%d : Unknown err code",             result); break;
    }
}

static void at_mms_errcode_parse(int result)//MMS
{
    switch(result)
    {
    case 751 : LOG_E("%d : Unknown error",                result); break;
    case 752 : LOG_E("%d : URL length error",             result); break;
    case 753 : LOG_E("%d : URL error",                    result); break;
    case 754 : LOG_E("%d : Invalid proxy type",           result); break;
    case 755 : LOG_E("%d : Proxy address error",          result); break;
    case 756 : LOG_E("%d : Invalid parameter",            result); break;
    case 757 : LOG_E("%d : Recipient address full",       result); break;
    case 758 : LOG_E("%d : CC recipient address full",    result); break;
    case 759 : LOG_E("%d : BCC recipient address full",   result); break;
    case 760 : LOG_E("%d : Attachments full",             result); break;
    case 761 : LOG_E("%d : File error",                   result); break;
    case 762 : LOG_E("%d : No recipient",                 result); break;
    case 763 : LOG_E("%d : File not found",               result); break;
    case 764 : LOG_E("%d : MMS busy",                     result); break;
    case 765 : LOG_E("%d : Server response failed",       result); break;
    case 766 : LOG_E("%d : Error response of HTTP(S) post", result); break;
    case 767 : LOG_E("%d : Invalid report of HTTP(S) post", result); break;
    case 768 : LOG_E("%d : PDP activation failed",        result); break;
    case 769 : LOG_E("%d : PDP deactivated",              result); break;
    case 770 : LOG_E("%d : Socket creation failed",       result); break;
    case 771 : LOG_E("%d : Socket connection failed",     result); break;
    case 772 : LOG_E("%d : Socket read failed",           result); break;
    case 773 : LOG_E("%d : Socket write failed",          result); break;
    case 774 : LOG_E("%d : Socket closed",                result); break;
    case 775 : LOG_E("%d : Timeout",                      result); break;
    case 776 : LOG_E("%d : Encode data error",            result); break;
    case 777 : LOG_E("%d : HTTP(S) decode data error",    result); break;
    default  : LOG_E("%d : Unknown err code",             result); break;
    }
}
    
static void at_tcp_ip_errcode_parse(int result)//TCP/IP_QIGETERROR
{
    switch(result)
    {
    case 0   : LOG_D("%d : Operation successful",         result); break;
    case 550 : LOG_E("%d : Unknown error",                result); break;
    case 551 : LOG_E("%d : Operation blocked",            result); break;
    case 552 : LOG_E("%d : Invalid parameters",           result); break;
    case 553 : LOG_E("%d : Memory not enough",            result); break;
    case 554 : LOG_E("%d : Create socket failed",         result); break;
    case 555 : LOG_E("%d : Operation not supported",      result); break;
    case 556 : LOG_E("%d : Socket bind failed",           result); break;
    case 557 : LOG_E("%d : Socket listen failed",         result); break;
    case 558 : LOG_E("%d : Socket write failed",          result); break;
    case 559 : LOG_E("%d : Socket read failed",           result); break;
    case 560 : LOG_E("%d : Socket accept failed",         result); break;
    case 561 : LOG_E("%d : Open PDP context failed",      result); break;
    case 562 : LOG_E("%d : Close PDP context failed",     result); break;
    case 563 : LOG_W("%d : Socket identity has been used", result); break;
    case 564 : LOG_E("%d : DNS busy",                     result); break;
    case 565 : LOG_E("%d : DNS parse failed",             result); break;
    case 566 : LOG_E("%d : Socket connect failed",        result); break;
    case 567 : LOG_W("%d : Socket has been closed",       result); break;
    case 568 : LOG_E("%d : Operation busy",               result); break;
    case 569 : LOG_E("%d : Operation timeout",            result); break;
    case 570 : LOG_E("%d : PDP context broken down",      result); break;
    case 571 : LOG_E("%d : Cancel send",                  result); break;
    case 572 : LOG_E("%d : Operation not allowed",        result); break;
    case 573 : LOG_E("%d : APN not configured",           result); break;
    case 574 : LOG_E("%d : Port busy",                    result); break;
    default  : LOG_E("%d : Unknown err code",             result); break;
    }
}

static void at_http_errcode_parse(int result)//HTTP
{
    switch(result)
    {
    case 0   : LOG_D("%d : Operation successful",         result); break;
    case 701 : LOG_E("%d : HTTP(S) unknown error",        result); break;
    case 702 : LOG_E("%d : HTTP(S) timeout",              result); break;
    case 703 : LOG_E("%d : HTTP(S) busy",                 result); break;
    case 704 : LOG_E("%d : HTTP(S) UART busy",            result); break;
    case 705 : LOG_E("%d : HTTP(S) no GET/POST requests", result); break;
    case 706 : LOG_E("%d : HTTP(S) network busy",         result); break;
    case 707 : LOG_E("%d : HTTP(S) network open failed",  result); break;
    case 708 : LOG_E("%d : HTTP(S) network no configuration", result); break;
    case 709 : LOG_E("%d : HTTP(S) network deactivated",  result); break;
    case 710 : LOG_E("%d : HTTP(S) network error",        result); break;
    case 711 : LOG_E("%d : HTTP(S) URL error",            result); break;
    case 712 : LOG_E("%d : HTTP(S) empty URL",            result); break;
    case 713 : LOG_E("%d : HTTP(S) IP address error",     result); break;
    case 714 : LOG_E("%d : HTTP(S) DNS error",            result); break;
    case 715 : LOG_E("%d : HTTP(S) socket create error",  result); break;
    case 716 : LOG_E("%d : HTTP(S) socket connect error", result); break;
    case 717 : LOG_E("%d : HTTP(S) socket read error",    result); break;
    case 718 : LOG_E("%d : HTTP(S) socket write error",   result); break;
    case 719 : LOG_E("%d : HTTP(S) socket closed",        result); break;
    case 720 : LOG_E("%d : HTTP(S) data encode error",    result); break;
    case 721 : LOG_E("%d : HTTP(S) data decode error",    result); break;
    case 722 : LOG_E("%d : HTTP(S) read timeout",         result); break;
    case 723 : LOG_E("%d : HTTP(S) response failed",      result); break;
    case 724 : LOG_E("%d : Incoming call busy",           result); break;
    case 725 : LOG_E("%d : Voice call busy",              result); break;
    case 726 : LOG_E("%d : Input timeout",                result); break;
    case 727 : LOG_E("%d : Wait data timeout",            result); break;
    case 728 : LOG_E("%d : Wait HTTP(S) response timeout", result); break;
    case 729 : LOG_E("%d : Memory allocation failed",     result); break;
    case 730 : LOG_E("%d : Invalid parameter",            result); break;
    default  : LOG_E("%d : Unknown err code",             result); break;
    }
}

static void at_http_rsponsecode_parse(int result)//HTTP
{
    switch(result)
    {
    case 200 : LOG_D("%d : OK",                           result); break;
    case 400 : LOG_E("%d : Bad request",                  result); break;
    case 403 : LOG_E("%d : Forbidden",                    result); break;
    case 404 : LOG_E("%d : Not found",                    result); break;
    case 409 : LOG_E("%d : Conflict",                     result); break;
    case 411 : LOG_E("%d : Length required",              result); break;
    case 500 : LOG_E("%d : Internal server error",        result); break;
    case 502 : LOG_E("%d : Bad gate way",                 result); break;
    default  : LOG_E("%d : Unknown err code",             result); break;
    }
}

static void at_ftp_errcode_parse(int result)//FTP
{
    switch(result)
    {
    case 0   : LOG_D("%d : Operation successful",         result); break;
    case 601 : LOG_E("%d : Unknown error",                result); break;
    case 602 : LOG_E("%d : FTP(S) server blocked",        result); break;
    case 603 : LOG_E("%d : FTP(S) server busy",           result); break;
    case 604 : LOG_E("%d : DNS parse failed",             result); break;
    case 605 : LOG_E("%d : Network error",                result); break;
    case 606 : LOG_E("%d : Control connection closed.",   result); break;
    case 607 : LOG_E("%d : Data connection closed",       result); break;
    case 608 : LOG_E("%d : Socket closed by peer",        result); break;
    case 609 : LOG_E("%d : Timeout error",                result); break;
    case 610 : LOG_E("%d : Invalid parameter",            result); break;
    case 611 : LOG_E("%d : Failed to open file",          result); break;
    case 612 : LOG_E("%d : File position invalid",        result); break;
    case 613 : LOG_E("%d : File error",                   result); break;
    case 614 : LOG_E("%d : Service not available, closing control connection", result); break;
    case 615 : LOG_E("%d : Open data connection failed",  result); break;
    case 616 : LOG_E("%d : Connection closed; transfer aborted", result); break;
    case 617 : LOG_E("%d : Requested file action not taken", result); break;
    case 618 : LOG_E("%d : Requested action aborted: local error in processing", result); break;
    case 619 : LOG_E("%d : Requested action not taken: insufficient system storage", result); break;
    case 620 : LOG_E("%d : Syntax error, command unrecognized", result); break;
    case 621 : LOG_E("%d : Syntax error in parameters or arguments", result); break;
    case 622 : LOG_E("%d : Command not implemented",      result); break;
    case 623 : LOG_E("%d : Bad sequence of commands",     result); break;
    case 624 : LOG_E("%d : Command parameter not implemented", result); break;
    case 625 : LOG_E("%d : Not logged in",                result); break;
    case 626 : LOG_E("%d : Need account for storing files", result); break;
    case 627 : LOG_E("%d : Requested action not taken",   result); break;
    case 628 : LOG_E("%d : Requested action aborted: page type unknown", result); break;
    case 629 : LOG_E("%d : Requested file action aborted", result); break;
    case 630 : LOG_E("%d : Requested file name invalid",  result); break;
    case 631 : LOG_E("%d : SSL authentication failed",    result); break;
    default  : LOG_E("%d : Unknown err code",             result); break;
    }
}

static void at_ftp_protocol_errcode_parse(int result)//FTP_Protocol
{
    switch(result)
    {
    case 421 : LOG_E("%d : Service not available, closing control connection", result); break;
    case 425 : LOG_E("%d : Open data connection failed",  result); break;
    case 426 : LOG_E("%d : Connection closed; transfer aborted", result); break;
    case 450 : LOG_E("%d : Requested file action not taken", result); break;
    case 451 : LOG_E("%d : Requested action aborted: local error in processing", result); break;
    case 452 : LOG_E("%d : Requested action not taken: insufficient system storage", result); break;
    case 500 : LOG_E("%d : Syntax error, command unrecognized", result); break;
    case 501 : LOG_E("%d : Syntax error in parameters or arguments", result); break;
    case 502 : LOG_E("%d : Command not implemented",      result); break;
    case 503 : LOG_E("%d : Bad sequence of commands",     result); break;
    case 504 : LOG_E("%d : Command parameter not implemented", result); break;
    case 530 : LOG_E("%d : Not logged in",                result); break;
    case 532 : LOG_E("%d : Need account for storing files", result); break;
    case 550 : LOG_E("%d : Requested action not taken: file unavailable", result); break;
    case 551 : LOG_E("%d : Requested action aborted: page type unknown", result); break;
    case 552 : LOG_E("%d : Requested file action aborted: exceeded storage allocation", result); break;
    case 553 : LOG_E("%d : Requested action not taken: file name not allowed", result); break;
    default  : LOG_E("%d : Unknown err code",             result); break;
    }
}

static void at_smtp_errcode_parse(int result)//Email
{
    switch(result)
    {
    case 651 : LOG_E("%d : Unknown error",                result); break;
    case 652 : LOG_E("%d : The SMTP server is busy, such as uploading the body or sending an email.", result); break;
    case 653 : LOG_E("%d : Failed to get IP address according to the domain name.", result); break;
    case 654 : LOG_E("%d : Network error, such as failed to activate GPRS/CSD context, failed to establish the TCP connection with the SMTP server or failed to send an email to the SMTP server, etc.", result); break;
    case 655 : LOG_E("%d : Unsupported authentication type", result); break;
    case 656 : LOG_E("%d : The connection for the SMTP server is closed by peer.", result); break;
    case 657 : LOG_E("%d : GPRS/CSD context is deactivated.", result); break;
    case 658 : LOG_E("%d : Timeout",                      result); break;
    case 659 : LOG_E("%d : No recipient for the SMTP server", result); break;
    case 660 : LOG_E("%d : Failed to send an email",      result); break;
    case 661 : LOG_E("%d : Failed to open a file",        result); break;
    case 662 : LOG_E("%d : No enough memory for the attachment", result); break;
    case 663 : LOG_E("%d : Failed to save the attachment", result); break;
    case 664 : LOG_E("%d : The input parameter is wrong", result); break;
    case 665 : LOG_E("%d : SSL authentication failed",    result); break;
    case 666 : LOG_E("%d : Service not available, closing transmission channel", result); break;
    case 667 : LOG_E("%d : Requested mail action not taken: mailbox unavailable", result); break;
    case 668 : LOG_E("%d : Requested action aborted: local error in processing", result); break;
    case 669 : LOG_E("%d : Requested action not taken: insufficient system storage", result); break;
    case 670 : LOG_E("%d : Syntax error, command unrecognized", result); break;
    case 671 : LOG_E("%d : Syntax error in parameters or arguments", result); break;
    case 672 : LOG_E("%d : Command not implemented",      result); break;
    case 673 : LOG_E("%d : Bad sequence of commands",     result); break;
    case 674 : LOG_E("%d : Command parameter not implemented", result); break;
    case 675 : LOG_E("%d : <domain> does not accept mail (see RFC1846)", result); break;
    case 676 : LOG_E("%d : Access denied",                result); break;
    case 677 : LOG_E("%d : Authentication failed",        result); break;
    case 678 : LOG_E("%d : Requested action not taken: mailbox unavailable", result); break;
    case 679 : LOG_E("%d : User not local; please try <forward-path>", result); break;
    case 680 : LOG_E("%d : Requested mail action aborted: exceeded storage allocation", result); break;
    case 681 : LOG_E("%d : Requested action not taken: mailbox name not allowed", result); break;
    case 682 : LOG_E("%d : Transaction failed",           result); break;
    default  : LOG_E("%d : Unknown err code",             result); break;
    }
}

static void at_smtp_protocol_errcode_parse(int result)//Email_Protocol
{
    switch(result)
    {
    case 421 : LOG_E("%d : Service not available, closing transmission channel", result); break;
    case 450 : LOG_E("%d : Requested mail action not taken: mailbox unavailable", result); break;
    case 451 : LOG_E("%d : Requested action aborted: local error in processing", result); break;
    case 452 : LOG_E("%d : Requested action not taken: insufficient system storage", result); break;
    case 500 : LOG_E("%d : Syntax error, command unrecognized", result); break;
    case 501 : LOG_E("%d : Syntax error in parameters or arguments", result); break;
    case 502 : LOG_E("%d : Command not implemented",      result); break;
    case 503 : LOG_E("%d : Bad sequence of commands",     result); break;
    case 504 : LOG_E("%d : Command parameter not implemented", result); break;
    case 521 : LOG_E("%d : <domain> does not accept mail (see RFC1846)", result); break;
    case 530 : LOG_E("%d : Access denied",                result); break;
    case 535 : LOG_E("%d : Authentication failed",        result); break;
    case 550 : LOG_E("%d : Requested action not taken: mailbox unavailable", result); break;
    case 551 : LOG_E("%d : User not local; please try <forward-path>", result); break;
    case 552 : LOG_E("%d : Requested mail action aborted: exceeded storage allocation", result); break;
    case 553 : LOG_E("%d : Requested action not taken: mailbox name not allowed", result); break;
    case 554 : LOG_E("%d : Transaction failed",           result); break;
    default  : LOG_E("%d : Unknown err code",             result); break;
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
static int ec20_socket_close(int socket)
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

    /* default connection timeout is 10 seconds, but it set to 1 seconds is convenient to use.*/
    result = at_exec_cmd(resp, "AT+QICLOSE=%d,1", socket);
    if (result < 0)
    {
        return result;
    }

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
static int ec20_socket_connect(int socket, char *ip, int32_t port, enum at_socket_type type, rt_bool_t is_client)
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
    at_socket_event_recv(SET_EVENT(socket, EC20_EVENT_CONN_OK | EC20_EVENT_CONN_FAIL), 0, RT_EVENT_FLAG_OR);

    if (is_client)
    {
        switch (type)
        {
        case AT_SOCKET_TCP:
            /* send AT commands(AT+QIOPEN=<contextID>,<socket>,"<TCP/UDP>","<IP_address>/<domain_name>",<remote_port>,<local_port>,<access_mode>) to connect TCP server */
            /* contextID = 1 : use same contextID as AT+QICSGP & AT+QIACT */
            /* local_port=0 : local port assigned automatically */
            /* access_mode = 1 : Direct push mode */
            if (at_exec_cmd(resp, "AT+QIOPEN=1,%d,\"TCP\",\"%s\",%d,0,1", socket, ip, port) < 0)
            {
                result = -RT_ERROR;
                goto __exit;
            }
            break;

        case AT_SOCKET_UDP:
            if (at_exec_cmd(resp, "AT+QIOPEN=1,%d,\"UDP\",\"%s\",%d,0,1", socket, ip, port) < 0)
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
    if ((event_result = at_socket_event_recv(EC20_EVENT_CONN_OK | EC20_EVENT_CONN_FAIL, 1 * RT_TICK_PER_SECOND,
            RT_EVENT_FLAG_OR)) < 0)
    {
        LOG_E("socket (%d) connect failed, wait connect OK|FAIL timeout.", socket);
        result = -RT_ETIMEOUT;
        goto __exit;
    }
    /* check result */
    if (event_result & EC20_EVENT_CONN_FAIL)
    {
        if (!retryed)
        {
            LOG_W("socket (%d) connect failed, maybe the socket was not be closed at the last time and now will retry.", socket);
            /* default connection timeout is 10 seconds, but it set to 1 seconds is convenient to use.*/
            if (ec20_socket_close < 0)
            {
                result = -RT_ERROR;
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
    at_response_t resp = at_create_resp(128, 0, 5 * RT_TICK_PER_SECOND);
    int result = 0;

    if (!resp)
    {
        LOG_E("No memory for response structure!");
        result = -RT_ENOMEM;
        goto __exit;
    }

    if (at_exec_cmd(resp, "AT+QISEND=%d,0", socket) < 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }

    if (at_resp_parse_line_args_by_kw(resp, "+QISEND:", "+QISEND: %d,%d,%d", size, acked, nacked) <= 0)
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
        rt_thread_mdelay(50);
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
static int ec20_socket_send(int socket, const char *buff, size_t bfsz, enum at_socket_type type)
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
    at_socket_event_recv(SET_EVENT(socket, EC20_EVENT_SEND_OK | EC20_EVENT_SEND_FAIL), 0, RT_EVENT_FLAG_OR);

    /* set current socket for send URC event */
    cur_socket = socket;
    /* set AT client end sign to deal with '>' sign.*/
    at_set_end_sign('>');

    while (sent_size < bfsz)
    {
        if (bfsz - sent_size < EC20_MODULE_SEND_MAX_SIZE)
        {
            cur_pkt_size = bfsz - sent_size;
        }
        else
        {
            cur_pkt_size = EC20_MODULE_SEND_MAX_SIZE;
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
        if (at_socket_event_recv(SET_EVENT(socket, 0), 10 * RT_TICK_PER_SECOND, RT_EVENT_FLAG_OR) < 0)
        {
            result = -RT_ETIMEOUT;
            goto __exit;
        }
        /* waiting OK or failed result */
        if ((event_result = at_socket_event_recv(EC20_EVENT_SEND_OK | EC20_EVENT_SEND_FAIL, 1 * RT_TICK_PER_SECOND,
                RT_EVENT_FLAG_OR)) < 0)
        {
            LOG_E("socket (%d) send failed, wait connect OK|FAIL timeout.", socket);
            result = -RT_ETIMEOUT;
            goto __exit;
        }
        /* check result */
        if (event_result & EC20_EVENT_SEND_FAIL)
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
static int ec20_domain_resolve(const char *name, char ip[16])
{
#define RESOLVE_RETRY                  3

    int i, result;
//    char recv_ip[16] = { 0 };
    at_response_t resp = RT_NULL;

    RT_ASSERT(name);
    RT_ASSERT(ip);

    /* The maximum response time is 60 seconds, but it set to 10 seconds is convenient to use. */
    resp = at_create_resp(128, 0, 10 * RT_TICK_PER_SECOND);
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        return -RT_ENOMEM;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);
    
    /* Clear EC20_EVENT_DOMAIN_OK */
    at_socket_event_recv(EC20_EVENT_DOMAIN_OK, 0, RT_EVENT_FLAG_OR);

    result = at_exec_cmd(resp, "AT+QIDNSGIP=1,\"%s\"", name);
    if (result < 0)
    {
        goto __exit;
    }
    
    if (result == RT_EOK)
    {
        for(i = 0; i < RESOLVE_RETRY; i++)
        {
            /* waiting result event from AT URC, the device default connection timeout is 60 seconds.*/
            if (at_socket_event_recv(EC20_EVENT_DOMAIN_OK, 10 * RT_TICK_PER_SECOND, RT_EVENT_FLAG_OR) < 0)
            {
                continue;
            }
            else
            {
                if (strlen(recv_ip) < 8)
                {
                    rt_thread_mdelay(100);
                    /* resolve failed, maybe receive an URC CRLF */
                    result = -RT_ERROR;
                    continue;
                }
                else
                {
                    strncpy(ip, recv_ip, 15);
                    ip[15] = '\0';
                    result = RT_EOK;
                    break;
                }
            }
        }
        
        /* response timeout */
        if (i == RESOLVE_RETRY)
        {
            result = -RT_ENOMEM;
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
static void ec20_socket_set_event_cb(at_socket_evt_t event, at_evt_cb_t cb)
{
    if (event < sizeof(at_evt_cb_set) / sizeof(at_evt_cb_set[1]))
    {
        at_evt_cb_set[event] = cb;
    }
}

static void urc_connect_func(const char *data, rt_size_t size)
{
    int socket = 0;
    int result = 0;

    RT_ASSERT(data && size);

    sscanf(data, "+QIOPEN: %d,%d", &socket , &result);

    if (result == 0)
    {
        at_socket_event_send(SET_EVENT(socket, EC20_EVENT_CONN_OK));
    }
    else
    {
        at_tcp_ip_errcode_parse(result);
        at_socket_event_send(SET_EVENT(socket, EC20_EVENT_CONN_FAIL));
    }
}

static void urc_send_func(const char *data, rt_size_t size)
{
    RT_ASSERT(data && size);

    if (strstr(data, "SEND OK"))
    {
        at_socket_event_send(SET_EVENT(cur_socket, EC20_EVENT_SEND_OK));
    }
    else if (strstr(data, "SEND FAIL"))
    {
        at_socket_event_send(SET_EVENT(cur_socket, EC20_EVENT_SEND_FAIL));
    }
}

static void urc_close_func(const char *data, rt_size_t size)
{
    int socket = 0;

    RT_ASSERT(data && size);

    sscanf(data, "+QIURC: \"closed\",%d", &socket);

    /* notice the socket is disconnect by remote */
    if (at_evt_cb_set[AT_SOCKET_EVT_CLOSED])
    {
        at_evt_cb_set[AT_SOCKET_EVT_CLOSED](socket, AT_SOCKET_EVT_CLOSED, NULL, 0);
    }
    
    // /* when TCP socket service is closed, host must send "AT+QICLOSE= <connID>,0" command to close socket */
    // at_exec_cmd(RT_NULL, "AT+QICLOSE=%d,0\r\n", socket);
    // rt_thread_mdelay(100);
}

static void urc_recv_func(const char *data, rt_size_t size)
{
    int socket = 0;
    rt_size_t bfsz = 0, temp_size = 0;
    rt_int32_t timeout;
    char *recv_buf = RT_NULL, temp[8];

    RT_ASSERT(data && size);
    
    /* get the current socket and receive buffer size by receive data */
    sscanf(data, "+QIURC: \"recv\",%d,%d", &socket, (int *) &bfsz);
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

static void urc_pdpdeact_func(const char *data, rt_size_t size)
{
    int connectID = 0;

    RT_ASSERT(data && size);

    sscanf(data, "+QIURC: \"pdpdeact\",%d", &connectID);

    LOG_E("Context (%d) is deactivated.", connectID);
}

static void urc_dnsqip_func(const char *data, rt_size_t size)
{
    int i = 0, j = 0;
    int result, ip_count, dns_ttl;

    RT_ASSERT(data && size);

    for (i = 0; i < size; i++)
    {
        if (*(data + i) == '.')
            j++;
    }
    /* There would be several dns result, we just pickup one */
    if (j == 3)
    {
        sscanf(data, "+QIURC: \"dnsgip\",\"%[^\"]", recv_ip);
        recv_ip[15] = '\0';
        at_socket_event_send(EC20_EVENT_DOMAIN_OK);
    }
    else
    {
        sscanf(data, "+QIURC: \"dnsgip\",%d,%d,%d", &result, &ip_count, &dns_ttl);
        if (result)
        {
            at_tcp_ip_errcode_parse(result);
        }
    }
}

static void urc_func(const char *data, rt_size_t size)
{
    RT_ASSERT(data);

    LOG_I("URC data : %.*s", size, data);
}

static void urc_qiurc_func(const char *data, rt_size_t size)
{
    RT_ASSERT(data && size);

//    LOG_D("qiurc : %s", data);
    switch(*(data+9))
    {
    case 'c' : urc_close_func(data, size); break;//+QIURC: "closed"
    case 'r' : urc_recv_func(data, size); break;//+QIURC: "recv"
    case 'p' : urc_pdpdeact_func(data, size); break;//+QIURC: "pdpdeact"
    case 'd' : urc_dnsqip_func(data, size); break;//+QIURC: "dnsgip"
    default  : urc_func(data, size);      break;//
    }
}

static const struct at_urc urc_table[] = {
        {"SEND OK",     "\r\n",                 urc_send_func},
        {"SEND FAIL",   "\r\n",                 urc_send_func},
        {"+QIOPEN:",    "\r\n",                 urc_connect_func},
        {"+QIURC:",     "\r\n",                 urc_qiurc_func},
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

static int ec20_netdev_set_info(struct netdev *netdev);
static int ec20_netdev_check_link_status(struct netdev *netdev); 

/* init for EC20 */
static void ec20_init_thread_entry(void *parameter)
{
#define AT_RETRY                       10
#define CIMI_RETRY                     10
#define CSQ_RETRY                      20
#define CREG_RETRY                     10
#define CGREG_RETRY                    20

    at_response_t resp = RT_NULL;
    int i, qi_arg[3];
    char parsed_data[20];
    rt_err_t result = RT_EOK;

    resp = at_create_resp(128, 0, rt_tick_from_millisecond(300));
    if (!resp)
    {
        LOG_E("No memory for response structure!");
        result = -RT_ENOMEM;
        goto __exit;
    }
    LOG_D("Start initializing the EC20 module");
    /* wait EC20 startup finish, Send AT every 500ms, if receive OK, SYNC success*/
    if (at_client_wait_connect(EC20_WAIT_CONNECT_TIME))
    {
        result = -RT_ETIMEOUT;
        goto __exit;
    }
    /* set response format to ATV1 */
    AT_SEND_CMD(resp, 0, 300, "ATV1");
    /* disable echo */
    AT_SEND_CMD(resp, 0, 300, "ATE0");
    /* Use AT+CMEE=2 to enable result code and use verbose values */
    AT_SEND_CMD(resp, 0, 300, "AT+CMEE=2");
    /* Get the baudrate */
    AT_SEND_CMD(resp, 0, 300, "AT+IPR?");
    at_resp_parse_line_args_by_kw(resp, "+IPR:", "+IPR: %d", &i);
    LOG_D("Baudrate %d", i);
    /* get module version */
    AT_SEND_CMD(resp, 0, 300, "ATI");
    /* show module version */
    for (i = 0; i < (int) resp->line_counts - 1; i++)
    {
        LOG_D("%s", at_resp_get_line(resp, i + 1));
    }
    /* Use AT+GSN to query the IMEI of module */
    AT_SEND_CMD(resp, 0, 300, "AT+GSN");
    
    /* check SIM card */
    AT_SEND_CMD(resp, 2, 5 * 1000, "AT+CPIN?");
    if (!at_resp_get_line_by_kw(resp, "READY"))
    {
        LOG_E("SIM card detection failed");
        result = -RT_ERROR;
        goto __exit;
    }
    /* waiting for dirty data to be digested */
    rt_thread_mdelay(10);
    
    
    /* Use AT+CIMI to query the IMSI of SIM card */
//    AT_SEND_CMD(resp, 2, 300, "AT+CIMI");
    i = 0;
    while(at_exec_cmd(at_resp_set_info(resp, 128, 0, rt_tick_from_millisecond(300)), "AT+CIMI") < 0)
    {
        i++;
        LOG_D("AT+CIMI %d", i);
        if(i > CIMI_RETRY)
        {
            LOG_E("Read CIMI failed");
            result = -RT_ERROR;
            goto __exit;
        }
        rt_thread_mdelay(1000);
    }

    /* Use AT+QCCID to query ICCID number of SIM card */
    AT_SEND_CMD(resp, 0, 300, "AT+QCCID");
    /* check signal strength */
    for (i = 0; i < CSQ_RETRY; i++)
    {
        AT_SEND_CMD(resp, 0, 300, "AT+CSQ");
        at_resp_parse_line_args_by_kw(resp, "+CSQ:", "+CSQ: %d,%d", &qi_arg[0], &qi_arg[1]);
        if (qi_arg[0] != 99)
        {
            LOG_D("Signal strength: %d  Channel bit error rate: %d", qi_arg[0], qi_arg[1]);
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
    /*Use AT+CEREG? to query current EPS Network Registration Status*/
    AT_SEND_CMD(resp, 0, 300, "AT+CEREG?");
    /* Use AT+COPS? to query current Network Operator */
    AT_SEND_CMD(resp, 0, 300, "AT+COPS?");
    at_resp_parse_line_args_by_kw(resp, "+COPS:", "+COPS: %*[^\"]\"%[^\"]", &parsed_data);
    if(strcmp(parsed_data,"CHINA MOBILE") == 0)
    {
        /* "CMCC" */
        LOG_I("%s", parsed_data);
        AT_SEND_CMD(resp, 0, 300, QICSGP_CHINA_MOBILE);
    }
    else if(strcmp(parsed_data,"CHN-UNICOM") == 0)
    {
        /* "UNICOM" */
        LOG_I("%s", parsed_data);
        AT_SEND_CMD(resp, 0, 300, QICSGP_CHINA_UNICOM);
    }
    else if(strcmp(parsed_data,"CHN-CT") == 0)
    {
        AT_SEND_CMD(resp, 0, 300, QICSGP_CHINA_TELECOM);
        /* "CT" */
        LOG_I("%s", parsed_data);
    }
    /* Enable automatic time zone update via NITZ and update LOCAL time to RTC */
    AT_SEND_CMD(resp, 0, 300, "AT+CTZU=3");
    /* Get RTC time */
    AT_SEND_CMD(resp, 0, 300, "AT+CCLK?");

    /* Deactivate context profile */
    AT_SEND_CMD(resp, 0, 40 * 1000, "AT+QIDEACT=1");
    /* Activate context profile */
    AT_SEND_CMD(resp, 0, 150 * 1000, "AT+QIACT=1");
    /* Query the status of the context profile */
    AT_SEND_CMD(resp, 0, 150 * 1000, "AT+QIACT?");
    at_resp_parse_line_args_by_kw(resp, "+QIACT:", "+QIACT: %*[^\"]\"%[^\"]", &parsed_data);
    LOG_I("%s", parsed_data);

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    if (!result)
    {
        /* set network interface device status and address information */
        ec20_netdev_set_info(netdev_get_by_name(EC20_NETDEV_NAME));
        ec20_netdev_check_link_status(netdev_get_by_name(EC20_NETDEV_NAME));

        LOG_I("AT network initialize success!");
    }
    else
    {
        LOG_E("AT network initialize failed (%d)!", result);
    }

}

/* EC20 device network initialize */
void ec20_net_init(void)
{
#ifdef PKG_AT_INIT_BY_THREAD
    rt_thread_t tid;
    tid = rt_thread_create("ec20_net_init", ec20_init_thread_entry, RT_NULL, EC20_THREAD_STACK_SIZE, EC20_THREAD_PRIORITY, 20);
    if (tid)
    {
        rt_thread_startup(tid);
    }
    else
    {
        LOG_E("Create AT initialization thread failed!");
    }
#else
    ec20_init_thread_entry(RT_NULL);
#endif
}

#ifdef FINSH_USING_MSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(ec20_net_init, at_net_init, initialize AT network);
#endif

static const struct at_device_ops ec20_socket_ops = {
    ec20_socket_connect,
    ec20_socket_close,
    ec20_socket_send,
    ec20_domain_resolve,
    ec20_socket_set_event_cb,
};

/* set ec20 network interface device status and address information */
static int ec20_netdev_set_info(struct netdev *netdev)
{
#define EC20_IEMI_RESP_SIZE      32
#define EC20_IPADDR_RESP_SIZE    64
#define EC20_DNS_RESP_SIZE       96
#define EC20_INFO_RESP_TIMO      rt_tick_from_millisecond(300)

    int result = RT_EOK;
    at_response_t resp = RT_NULL;
    ip_addr_t addr;

    if (netdev == RT_NULL)
    {
        LOG_E("Input network interface device is NULL.\n");
        return -RT_ERROR;
    }

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

    /* set network interface device status */
    netdev_low_level_set_status(netdev, RT_TRUE);
    netdev_low_level_set_link_status(netdev, RT_TRUE);
    netdev_low_level_set_dhcp_status(netdev, RT_TRUE);

    resp = at_create_resp(EC20_IEMI_RESP_SIZE, 0, EC20_INFO_RESP_TIMO);
    if (resp == RT_NULL)
    {
        LOG_E("EC20 set netdev information failed, no memory for response object.");
        result = -RT_ENOMEM;
        goto __exit;
    }

    /* set network interface device hardware address(IEMI) */
    {
        #define EC20_NETDEV_HWADDR_LEN   8
        #define EC20_IEMI_LEN            15

        char iemi[EC20_IEMI_LEN] = {0};
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

        LOG_D("EC20 IEMI number: %s", iemi);

        netdev->hwaddr_len = EC20_NETDEV_HWADDR_LEN;
        /* get hardware address by IEMI */
        for (i = 0, j = 0; i < EC20_NETDEV_HWADDR_LEN && j < EC20_IEMI_LEN; i++, j+=2)
        {
            if (j != EC20_IEMI_LEN - 1)
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
        
        at_resp_set_info(resp, EC20_IPADDR_RESP_SIZE, 0, EC20_INFO_RESP_TIMO);

        /* send "AT+QIACT?" commond to get IP address */
        if (at_exec_cmd(resp, "AT+QIACT?") < 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }

        /* parse response data "+QIACT: 1,<context_state>,<context_type>[,<IP_address>]" */
        if (at_resp_parse_line_args_by_kw(resp, "+QIACT:", "+QIACT: %*[^\"]\"%[^\"]", ipaddr) <= 0)
        {
            LOG_E("Prase \"AT+QIACT?\" commands resposne data error!");
            result = -RT_ERROR;
            goto __exit;
        }
        
        LOG_D("EC20 IP address: %s", ipaddr);

        /* set network interface address information */
        inet_aton(ipaddr, &addr);
        netdev_low_level_set_ipaddr(netdev, &addr);
    }

    /* set network interface device dns server */
    {
        #define DNS_ADDR_SIZE_MAX   16
        char dns_server1[DNS_ADDR_SIZE_MAX] = {0}, dns_server2[DNS_ADDR_SIZE_MAX] = {0};

        at_resp_set_info(resp, EC20_DNS_RESP_SIZE, 0, EC20_INFO_RESP_TIMO);

        /* send "AT+QIDNSCFG=1" commond to get DNS servers address */
        if (at_exec_cmd(resp, "AT+QIDNSCFG=1") < 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }

        /* parse response data "+QIDNSCFG: <contextID>,<pridnsaddr>,<secdnsaddr>" */
        if (at_resp_parse_line_args_by_kw(resp, "+QIDNSCFG:", "+QIDNSCFG: 1,\"%[^\"]\",\"%[^\"]\"", 
                dns_server1, dns_server2) <= 0)
        {
            LOG_E("Prase \"AT+QIDNSCFG=1\" commands resposne data error!");
            result = -RT_ERROR;
            goto __exit;
        }

        LOG_D("EC20 primary DNS server address: %s", dns_server1);
        LOG_D("EC20 secondary DNS server address: %s", dns_server2);

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

static void ec20_check_link_status_entry(void *parameter)
{
#define EC20_LINK_RESP_SIZE     64
#define EC20_LINK_RESP_TIMO     (3 * RT_TICK_PER_SECOND)
#define EC20_LINK_DELAY_TIME    (30 * RT_TICK_PER_SECOND)

    int link_stat = 0;
    at_response_t resp = RT_NULL;
    struct netdev *netdev = (struct netdev *) parameter;
    
    resp = at_create_resp(EC20_LINK_RESP_SIZE, 0, EC20_LINK_RESP_TIMO);
    if (resp == RT_NULL)
    {
        LOG_E("EC20 set check link status failed, no memory for response object.");
        return;
    }

    while (1)
    {
        rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

        /* send "AT+CGREG" commond  to check netweork interface device link status */
        if (at_exec_cmd(resp, "AT+CGREG?") < 0)
        {
            if (netdev_is_link_up(netdev))
            {
                netdev_low_level_set_link_status(netdev, RT_FALSE);
            }
            
            rt_mutex_release(at_event_lock);
            rt_thread_mdelay(EC20_LINK_DELAY_TIME);
            continue;
        }
        else
        {
            at_resp_parse_line_args_by_kw(resp, "+CGREG:", "+CGREG: %*d,%d", &link_stat);

            /* 1 Registered, home network,5 Registered, roaming */
            if (link_stat == 1 || link_stat == 5)
            {
                if (netdev_is_link_up(netdev) == RT_FALSE)
                {
                    netdev_low_level_set_link_status(netdev, RT_TRUE);
                }
            }
            else
            {
                if (netdev_is_link_up(netdev))
                {
                    netdev_low_level_set_link_status(netdev, RT_FALSE);
                }
            }
        }

        rt_mutex_release(at_event_lock);
        rt_thread_mdelay(EC20_LINK_DELAY_TIME);
    }
}

static int ec20_netdev_check_link_status(struct netdev *netdev)
{
#define EC20_LINK_THREAD_STACK_SIZE     1024
#define EC20_LINK_THREAD_PRIORITY       (RT_THREAD_PRIORITY_MAX - 2)
#define EC20_LINK_THREAD_TICK           20

    rt_thread_t tid;

    if (netdev == RT_NULL)
    {
        LOG_E("Input network interface device is NULL.\n");
        return -RT_ERROR;
    }

    /* create WIZnet link status Polling thread  */
    tid = rt_thread_create("ec20_link", ec20_check_link_status_entry, (void *) netdev, 
            EC20_LINK_THREAD_STACK_SIZE, EC20_LINK_THREAD_PRIORITY, EC20_LINK_THREAD_TICK);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }

    return RT_EOK;
}

static int ec20_netdev_set_up(struct netdev *netdev)
{
    netdev_low_level_set_status(netdev, RT_TRUE);
    LOG_D("EC20 network interface set up status.");
    return RT_EOK;
}

static int ec20_netdev_set_down(struct netdev *netdev)
{
    netdev_low_level_set_status(netdev, RT_FALSE);
    LOG_D("EC20 network interface set down status.");
    return RT_EOK;
}

static int ec20_netdev_set_dns_server(struct netdev *netdev, uint8_t dns_num, ip_addr_t *dns_server)
{
#define EC20_DNS_RESP_LEN    8
#define EC20_DNS_RESP_TIMEO  rt_tick_from_millisecond(300)

    at_response_t resp = RT_NULL;
    int result = RT_EOK;

    RT_ASSERT(netdev);
    RT_ASSERT(dns_server);

    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);
    
    resp = at_create_resp(EC20_DNS_RESP_LEN, 0, EC20_DNS_RESP_TIMEO);
    if (resp == RT_NULL)
    {
        LOG_D("EC20 set dns server failed, no memory for response object.");
        result = -RT_ENOMEM;
        goto __exit;
    }

    /* send "AT+QIDNSCFG=<pri_dns>[,<sec_dns>]" commond to set dns servers */
    if (at_exec_cmd(resp, "AT+QIDNSCFG=1,\"%s\"", inet_ntoa(*dns_server)) < 0)
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

static int ec20_netdev_ping(struct netdev *netdev, const char *host, size_t data_len, uint32_t timeout, struct netdev_ping_resp *ping_resp)
{
#define EC20_PING_RESP_SIZE       128
#define EC20_PING_IP_SIZE         16
#define EC20_PING_TIMEO           (5 * RT_TICK_PER_SECOND)

    at_response_t resp = RT_NULL;
    rt_err_t result = RT_EOK;
    int response = -1, recv_data_len, ping_time, ttl;
    char ip_addr[EC20_PING_IP_SIZE] = {0};

    RT_ASSERT(netdev);
    RT_ASSERT(host);
    RT_ASSERT(ping_resp);
    
    rt_mutex_take(at_event_lock, RT_WAITING_FOREVER);

    resp = at_create_resp(EC20_PING_RESP_SIZE, 4, EC20_PING_TIMEO);
    if (resp == RT_NULL)
    {
        LOG_D("No memory for response structure!\n");
        return -RT_ENOMEM;
    }

    /* send "AT+QPING="<host>"[,[<timeout>][,<pingnum>]]" commond to send ping request */
    if (at_exec_cmd(resp, "AT+QPING=1,\"%s\",%d,1", host, timeout / RT_TICK_PER_SECOND) < 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }

    at_resp_parse_line_args_by_kw(resp, "+QPING:", "+QPING:%d", &response);
    /* Received the ping response from the server */
    if (response == 0)
    {
        if (at_resp_parse_line_args_by_kw(resp, "+QPING:", "+QPING:%d,\"%[^\"]\",%d,%d,%d",
                    &response, ip_addr, &recv_data_len, &ping_time, &ttl) <= 0)
        {
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
        ping_resp->ticks = ping_time;
        ping_resp->ttl = ttl;
        result = RT_EOK;
        break;
    case 569:
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

void ec20_netdev_netstat(struct netdev *netdev)
{
    // TODO
    return;
}

const struct netdev_ops ec20_netdev_ops =
{
    ec20_netdev_set_up,
    ec20_netdev_set_down,

    RT_NULL,
    ec20_netdev_set_dns_server,
    RT_NULL,

    ec20_netdev_ping,
    ec20_netdev_netstat,
};

static int ec20_netdev_add(const char *netdev_name)
{
#define ETHERNET_MTU        1500
#define HWADDR_LEN          6
    struct netdev *netdev = RT_NULL;

    netdev = (struct netdev *)rt_calloc(1, sizeof(struct netdev));
    if (netdev == RT_NULL)
    {
        return RT_NULL;
    }

    netdev->mtu = ETHERNET_MTU;
    netdev->ops = &ec20_netdev_ops;
    netdev->hwaddr_len = HWADDR_LEN;

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
    if (!at_socket_event)
    {
        LOG_E("AT client port initialize failed! at_sock_event create failed!");
        return -RT_ENOMEM;
    }

    /* create current AT socket lock */
    at_event_lock = rt_mutex_create("at_se", RT_IPC_FLAG_FIFO);
    if (!at_event_lock)
    {
        LOG_E("AT client port initialize failed! at_sock_lock create failed!");
        rt_event_delete(at_socket_event);
        return -RT_ENOMEM;
    }

    /* initialize AT client */
    at_client_init(AT_DEVICE_NAME, AT_DEVICE_RECV_BUFF_LEN);
    
    /* register URC data execution function  */
    at_set_urc_table(urc_table, sizeof(urc_table) / sizeof(urc_table[0]));

    /* Add ec20 network inetrface device to the netdev list */
    if (ec20_netdev_add(EC20_NETDEV_NAME) < 0)
    {
        LOG_E("EC20 network interface device(%d) add failed.", EC20_NETDEV_NAME);
        return -RT_ENOMEM;
    }

    /* initialize EC20 network */
    ec20_net_init();

    /* set EC20 AT Socket options */
    at_socket_device_register(&ec20_socket_ops);

    return RT_EOK;
}
INIT_APP_EXPORT(at_socket_device_init);

#endif /* AT_DEVICE_EC20 */
