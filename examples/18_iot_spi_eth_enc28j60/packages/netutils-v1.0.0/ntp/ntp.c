/*
 *
 * (C) 2014 David Lettier.
 * (C) 2018 Armink (armink.ztl@gmail.com)
 *
 * http://www.lettier.com/
 *
 * NTP client.
 *
 * Compiled with gcc version 4.7.2 20121109 (Red Hat 4.7.2-8) (GCC).
 *
 * Tested on Linux 3.8.11-200.fc18.x86_64 #1 SMP Wed May 1 19:44:27 UTC 2013 x86_64 x86_64 x86_64 GNU/Linux.
 * Tested on RT-Thread 3.0.0+
 *
 * To compile: $ gcc main.c -o ntpClient.out
 *
 * Usage: $ ./ntpClient.out
 *
 */

#include <rtthread.h>

#ifdef PKG_NETUTILS_NTP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>

#ifdef NETUTILS_NTP_TIMEZONE
#define NTP_TIMEZONE                   NETUTILS_NTP_TIMEZONE
#endif

#ifdef NETUTILS_NTP_HOSTNAME
#define NTP_HOSTNAME                   NETUTILS_NTP_HOSTNAME
#endif

#define NTP_TIMESTAMP_DELTA            2208988800ull
#define NTP_GET_TIMEOUT                5

#ifndef NTP_TIMEZONE
#define NTP_TIMEZONE                   8
#endif

#ifndef NTP_HOSTNAME
#define NTP_HOSTNAME                   "cn.ntp.org.cn"
#endif

#define LI(packet)   (uint8_t) ((packet.li_vn_mode & 0xC0) >> 6) // (li   & 11 000 000) >> 6
#define VN(packet)   (uint8_t) ((packet.li_vn_mode & 0x38) >> 3) // (vn   & 00 111 000) >> 3
#define MODE(packet) (uint8_t) ((packet.li_vn_mode & 0x07) >> 0) // (mode & 00 000 111) >> 0

// Structure that defines the 48 byte NTP packet protocol.
typedef struct {

    uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                         // li.   Two bits.   Leap indicator.
                         // vn.   Three bits. Version number of the protocol.
                         // mode. Three bits. Client will pick mode 3 for client.

    uint8_t stratum;         // Eight bits. Stratum level of the local clock.
    uint8_t poll;            // Eight bits. Maximum interval between successive messages.
    uint8_t precision;       // Eight bits. Precision of the local clock.

    uint32_t rootDelay;      // 32 bits. Total round trip delay time.
    uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;          // 32 bits. Reference clock identifier.

    uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

    uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

    uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

    uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
    uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

} ntp_packet;              // Total: 384 bits or 48 bytes.

static ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static void ntp_error(char* msg)
{
    rt_kprintf("\033[31;22m[E/NTP]: ERROR %s\033[0m\n", msg); // Print the error message to stderr.
}

/**
 * Get the UTC time from NTP server
 *
 * @param host_name NTP server host name, NULL: will using default host name
 *
 * @note this function is not reentrant
 *
 * @return >0: success, current UTC time
 *         =0: get failed
 */
time_t ntp_get_time(const char *host_name)
{
    int sockfd, n; // Socket file descriptor and the n return result from writing/reading from the socket.

    int portno = 123; // NTP UDP port number.

    time_t new_time = 0;

    fd_set readset;
    struct timeval timeout;

    // Using default host name when host_name is NULL
    if (host_name == NULL)
    {
        host_name = NTP_HOSTNAME;
    }

    // Create and zero out the packet. All 48 bytes worth.

    memset(&packet, 0, sizeof(ntp_packet));

    // Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.

    *((char *) &packet + 0) = 0x1b; // Represents 27 in base 10 or 00011011 in base 2.

    // Create a UDP socket, convert the host-name to an IP address, set the port number,
    // connect to the server, send the packet, and then read in the return packet.

    struct sockaddr_in serv_addr; // Server address data structure.
    struct hostent *server;      // Server data structure.

    sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP); // Create a UDP socket.

    if (sockfd < 0) {
        ntp_error("opening socket");
        return 0;
    }

    server = gethostbyname(host_name); // Convert URL to IP.

    if (server == NULL) {
        ntp_error("no such host");
        goto __exit;
    }

    // Zero out the server address structure.

    memset((char *) &serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;

    // Copy the server's IP address to the server address structure.

    memcpy((char *) &serv_addr.sin_addr.s_addr, (char *) server->h_addr, server->h_length);

    // Convert the port number integer to network big-endian style and save it to the server address structure.

    serv_addr.sin_port = htons(portno);

    // Call up the server using its IP address and port number.

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        ntp_error("connecting");
        goto __exit;
    }

    // Send it the NTP packet it wants. If n == -1, it failed.

    n = send(sockfd, (char*) &packet, sizeof(ntp_packet), 0);

    if (n < 0) {
        ntp_error("writing to socket");
        goto __exit;
    }

    timeout.tv_sec = NTP_GET_TIMEOUT;
    timeout.tv_usec = 0;

    FD_ZERO(&readset);
    FD_SET(sockfd, &readset);

    if (select(sockfd + 1, &readset, RT_NULL, RT_NULL, &timeout) <= 0) {
        ntp_error("select the socket timeout(5s)");
        goto __exit;
    }

    // Wait and receive the packet back from the server. If n == -1, it failed.

    n = recv(sockfd, (char*) &packet, sizeof(ntp_packet), 0);

    if (n < 0) {
        ntp_error("reading from socket");
        goto __exit;
    }

    // These two fields contain the time-stamp seconds as the packet left the NTP server.
    // The number of seconds correspond to the seconds passed since 1900.
    // ntohl() converts the bit/byte order from the network's to host's "endianness".

    packet.txTm_s = ntohl(packet.txTm_s); // Time-stamp seconds.
    packet.txTm_f = ntohl(packet.txTm_f); // Time-stamp fraction of a second.

    // Extract the 32 bits that represent the time-stamp seconds (since NTP epoch) from when the packet left the server.
    // Subtract 70 years worth of seconds from the seconds since 1900.
    // This leaves the seconds since the UNIX epoch of 1970.
    // (1900)------------------(1970)**************************************(Time Packet Left the Server)

    new_time = (time_t) (packet.txTm_s - NTP_TIMESTAMP_DELTA);

__exit:

    closesocket(sockfd);

    return new_time;
}

/**
 * Get the local time from NTP server
 *
 * @param host_name NTP server host name, NULL: will using default host name
 *
 * @return >0: success, current local time, offset timezone by NTP_TIMEZONE
 *         =0: get failed
 */
time_t ntp_get_local_time(const char *host_name)
{
    time_t cur_time = ntp_get_time(host_name);

    if (cur_time)
    {
        /* add the timezone offset for set_time/set_date */
        cur_time += NTP_TIMEZONE * 3600;
    }

    return cur_time;
}

/**
 * Sync current local time to RTC by NTP
 *
 * @param host_name NTP server host name, NULL: will using default host name
 *
 * @return >0: success, current local time, offset timezone by NTP_TIMEZONE
 *         =0: sync failed
 */
time_t ntp_sync_to_rtc(const char *host_name)
{
#ifdef RT_USING_RTC
    struct tm *cur_tm;
#endif

    time_t cur_time = ntp_get_local_time(host_name);

    if (cur_time)
    {

#ifdef RT_USING_RTC
        cur_tm = localtime(&cur_time);
        set_time(cur_tm->tm_hour, cur_tm->tm_min, cur_tm->tm_sec);

        cur_tm = localtime(&cur_time);
        set_date(cur_tm->tm_year + 1900, cur_tm->tm_mon + 1, cur_tm->tm_mday);
#endif /* RT_USING_RTC */

    }

    return cur_time;
}

static void ntp_sync(const char *host_name)
{
    time_t cur_time = ntp_sync_to_rtc(host_name);

    if (cur_time)
    {
        rt_kprintf("Get local time from NTP server: %s", ctime((const time_t*) &cur_time));

#ifdef RT_USING_RTC
        rt_kprintf("The system time is updated. Timezone is %d.\n", NTP_TIMEZONE);
#else
        rt_kprintf("The system time update failed. Please enable RT_USING_RTC.\n");
#endif /* RT_USING_RTC */

    }
}

static void cmd_ntp_sync(int argc, char **argv)
{
    char *host_name = NULL;

    if (argc > 1)
    {
        host_name = argv[1];
    }

    ntp_sync(host_name);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(ntp_sync, Update time by NTP(Network Time Protocol): ntp_sync(host_name));
MSH_CMD_EXPORT_ALIAS(cmd_ntp_sync, ntp_sync, Update time by NTP(Network Time Protocol): ntp_sync [host_name]);
#endif /* RT_USING_FINSH */
#endif /* PKG_NETUTILS_NTP */
