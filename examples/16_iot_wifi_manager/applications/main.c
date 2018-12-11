/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-01     ZeroFree     first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <msh.h>

#include "drv_wlan.h"
#include "wifi_config.h"

#include <stdio.h>
#include <stdlib.h>

#define WLAN_SSID               "test_ssid"
#define WLAN_PASSWORD           "12345678"
#define NET_READY_TIME_OUT       (rt_tick_from_millisecond(15 * 1000))

static void print_scan_result(struct rt_wlan_scan_result *scan_result);
static void print_wlan_information(struct rt_wlan_info *info);

static struct rt_semaphore net_ready;

/**
 * The callback of network ready event
 */
void wlan_ready_handler(int event, struct rt_wlan_buff *buff, void *parameter)
{
    rt_sem_release(&net_ready);
}

/**
 * The callback of wlan disconected event
 */
void wlan_station_disconnect_handler(int event, struct rt_wlan_buff *buff, void *parameter)
{
    rt_kprintf("disconnect from the network!\n");
}

int main(void)
{
    int result = RT_EOK;
    struct rt_wlan_info info;

    /* wait 500 milliseconds for wifi low level initialize complete */
    rt_hw_wlan_wait_init_done(500);

    /* scan ap */
    struct rt_wlan_scan_result *scan_result = RT_NULL;
    rt_kprintf("\nstart to scan ap ...\n");
    /* execute synchronous scan function */
    scan_result = rt_wlan_scan_sync();
    if (scan_result)
    {
        rt_kprintf("the scan is complete, results is as follows: \n");
        /* print scan results */
        print_scan_result(scan_result);
        /* clean scan results */
        rt_wlan_scan_result_clean();
    }
    else
    {
        rt_kprintf("not found ap information \n");
    }

    /* station connect */
    rt_kprintf("start to connect ap ...\n");
    rt_sem_init(&net_ready, "net_ready", 0, RT_IPC_FLAG_FIFO);

    /* register network ready event callback */
    rt_wlan_register_event_handler(RT_WLAN_EVT_READY, wlan_ready_handler, RT_NULL);
    /* register wlan disconnect event callback */
    rt_wlan_register_event_handler(RT_WLAN_EVT_STA_DISCONNECTED, wlan_station_disconnect_handler, RT_NULL);
    result = rt_wlan_connect(WLAN_SSID, WLAN_PASSWORD);
    if (result == RT_EOK)
    {
        rt_memset(&info, 0, sizeof(struct rt_wlan_info));
        /* Get the information of the current connection AP */
        rt_wlan_get_info(&info);
        rt_kprintf("station information:\n");
        print_wlan_information(&info);
        /* waiting for IP to be got successfully  */
        result = rt_sem_take(&net_ready, NET_READY_TIME_OUT);
        if (result == RT_EOK)
        {
            rt_kprintf("networking ready!\n");
            msh_exec("ifconfig", rt_strlen("ifconfig"));
        }
        else
        {
            rt_kprintf("wait ip got timeout!\n");
        }
        /* unregister network ready event */
        rt_wlan_unregister_event_handler(RT_WLAN_EVT_READY);
        rt_sem_detach(&net_ready);
    }
    else
    {
        rt_kprintf("The AP(%s) is connect failed!\n", WLAN_SSID);
    }

    /* wait for 5 seconds, then disconnect. */
    rt_thread_mdelay(5000);
    rt_kprintf("\n");
    rt_kprintf("ready to disconect from ap ...\n");
    rt_wlan_disconnect();

    /* auto connect */
    rt_kprintf("\n");
    rt_kprintf("start to autoconnect ...\n");
    /* initialize the autoconnect configuration */
    wlan_autoconnect_init();
    /* enable wlan auto connect */
    rt_wlan_config_autoreconnect(RT_TRUE);

    return 0;
}

static void print_scan_result(struct rt_wlan_scan_result *scan_result)
{
    char *security;
    int index, num;

    num = scan_result->num;
    rt_kprintf("             SSID                      MAC            security    rssi chn Mbps\n");
    rt_kprintf("------------------------------- -----------------  -------------- ---- --- ----\n");
    for (index = 0; index < num; index ++)
    {
        rt_kprintf("%-32.32s", &scan_result->info[index].ssid.val[0]);
        rt_kprintf("%02x:%02x:%02x:%02x:%02x:%02x  ",
                   scan_result->info[index].bssid[0],
                   scan_result->info[index].bssid[1],
                   scan_result->info[index].bssid[2],
                   scan_result->info[index].bssid[3],
                   scan_result->info[index].bssid[4],
                   scan_result->info[index].bssid[5]
                  );
        switch (scan_result->info[index].security)
        {
        case SECURITY_OPEN:
            security = "OPEN";
            break;
        case SECURITY_WEP_PSK:
            security = "WEP_PSK";
            break;
        case SECURITY_WEP_SHARED:
            security = "WEP_SHARED";
            break;
        case SECURITY_WPA_TKIP_PSK:
            security = "WPA_TKIP_PSK";
            break;
        case SECURITY_WPA_AES_PSK:
            security = "WPA_AES_PSK";
            break;
        case SECURITY_WPA2_AES_PSK:
            security = "WPA2_AES_PSK";
            break;
        case SECURITY_WPA2_TKIP_PSK:
            security = "WPA2_TKIP_PSK";
            break;
        case SECURITY_WPA2_MIXED_PSK:
            security = "WPA2_MIXED_PSK";
            break;
        case SECURITY_WPS_OPEN:
            security = "WPS_OPEN";
            break;
        case SECURITY_WPS_SECURE:
            security = "WPS_SECURE";
            break;
        default:
            security = "UNKNOWN";
            break;
        }
        rt_kprintf("%-14.14s ", security);
        rt_kprintf("%-4d ", scan_result->info[index].rssi);
        rt_kprintf("%3d ", scan_result->info[index].channel);
        rt_kprintf("%4d\n", scan_result->info[index].datarate / 1000000);
    }
    rt_kprintf("\n");
}

static void print_wlan_information(struct rt_wlan_info *info)
{
    rt_kprintf("SSID : %-.32s\n", &info->ssid.val[0]);
    rt_kprintf("MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x\n", info->bssid[0],
               info->bssid[1],
               info->bssid[2],
               info->bssid[3],
               info->bssid[4],
               info->bssid[5]);
    rt_kprintf("Channel: %d\n", info->channel);
    rt_kprintf("DataRate: %dMbps\n", info->datarate / 1000000);
    rt_kprintf("RSSI: %d\n", info->rssi);
}
