#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Configuration */

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
#define RT_ALIGN_SIZE 4
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 1000
#define RT_USING_OVERFLOW_CHECK
#define RT_USING_HOOK
#define RT_USING_IDLE_HOOK
#define RT_IDEL_HOOK_LIST_SIZE 4
#define IDLE_THREAD_STACK_SIZE 256

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_MEMHEAP
#define RT_USING_MEMHEAP_AS_HEAP
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 256
#define RT_CONSOLE_DEVICE_NAME "uart1"
#define RT_VER_NUM 0x40001

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 2048
#define RT_MAIN_THREAD_PRIORITY 10

/* C++ features */


/* Command shell */

#define RT_USING_FINSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 4096
#define FINSH_CMD_SIZE 80
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
#define FINSH_USING_MSH_ONLY
#define FINSH_ARG_MAX 10

/* Device virtual file system */

#define RT_USING_DFS
#define DFS_USING_WORKDIR
#define DFS_FILESYSTEMS_MAX 2
#define DFS_FILESYSTEM_TYPES_MAX 2
#define DFS_FD_MAX 16
#define RT_USING_DFS_DEVFS

/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_PIPE_BUFSZ 512
#define RT_USING_SYSTEM_WORKQUEUE
#define RT_SYSTEM_WORKQUEUE_STACKSIZE 2048
#define RT_SYSTEM_WORKQUEUE_PRIORITY 23
#define RT_USING_SERIAL
#define RT_SERIAL_USING_DMA
#define RT_SERIAL_RB_BUFSZ 64
#define RT_USING_PIN
#define RT_USING_RTC
#define RT_USING_SOFT_RTC
#define RTC_SYNC_USING_NTP
#define RTC_NTP_FIRST_SYNC_DELAY 30
#define RTC_NTP_SYNC_PERIOD 3600
#define RT_USING_SDIO
#define RT_SDIO_STACK_SIZE 512
#define RT_SDIO_THREAD_PRIORITY 15
#define RT_MMCSD_STACK_SIZE 1024
#define RT_MMCSD_THREAD_PREORITY 22
#define RT_MMCSD_MAX_PARTITION 16
#define RT_USING_SPI
#define RT_USING_SFUD
#define RT_SFUD_USING_SFDP
#define RT_SFUD_USING_FLASH_INFO_TABLE

/* Using WiFi */

#define RT_USING_WIFI
#define RT_WLAN_DEVICE_STA_NAME "wlan0"
#define RT_WLAN_DEVICE_AP_NAME "wlan1"
#define RT_WLAN_DEFAULT_PROT "lwip"
#define RT_WLAN_SCAN_WAIT_MS 10000
#define RT_WLAN_CONNECT_WAIT_MS 10000
#define RT_WLAN_SSID_MAX_LENGTH 32
#define RT_WLAN_PASSWORD_MAX_LENGTH 32
#define RT_WLAN_SCAN_SORT
#define RT_WLAN_CFG_INFO_MAX 3
#define RT_WLAN_WORKQUEUE_THREAD_NAME "wlan_job"
#define RT_WLAN_WORKQUEUE_THREAD_SIZE 2048
#define RT_WLAN_WORKQUEUE_THREAD_PRIO 22
#define RT_WLAN_DEV_EVENT_NUM 2
#define RT_WLAN_PROT_LWIP_PBUF_FORCE

/* Using USB */


/* POSIX layer and C standard library */

#define RT_USING_LIBC
#define RT_USING_POSIX

/* Network */

/* Socket abstraction layer */

#define RT_USING_SAL

/* protocol stack implement */

#define SAL_USING_LWIP
#define SAL_USING_TLS
#define SAL_USING_POSIX

/* Network interface device */

#define RT_USING_NETDEV
#define NETDEV_USING_IFCONFIG
#define NETDEV_USING_PING
#define NETDEV_USING_NETSTAT
#define NETDEV_USING_AUTO_DEFAULT

/* light weight TCP/IP stack */

#define RT_USING_LWIP
#define RT_USING_LWIP202
#define RT_LWIP_IGMP
#define RT_LWIP_ICMP
#define RT_LWIP_DNS
#define RT_LWIP_DHCP
#define IP_SOF_BROADCAST 1
#define IP_SOF_BROADCAST_RECV 1

/* Static IPv4 Address */

#define RT_LWIP_IPADDR "192.168.1.30"
#define RT_LWIP_GWADDR "192.168.1.1"
#define RT_LWIP_MSKADDR "255.255.255.0"
#define RT_LWIP_UDP
#define RT_LWIP_TCP
#define RT_LWIP_RAW
#define RT_MEMP_NUM_NETCONN 8
#define RT_LWIP_PBUF_NUM 8
#define RT_LWIP_RAW_PCB_NUM 4
#define RT_LWIP_UDP_PCB_NUM 4
#define RT_LWIP_TCP_PCB_NUM 4
#define RT_LWIP_TCP_SEG_NUM 40
#define RT_LWIP_TCP_SND_BUF 4096
#define RT_LWIP_TCP_WND 4096
#define RT_LWIP_TCPTHREAD_PRIORITY 10
#define RT_LWIP_TCPTHREAD_MBOX_SIZE 8
#define RT_LWIP_TCPTHREAD_STACKSIZE 1024
#define LWIP_NO_RX_THREAD
#define LWIP_NO_TX_THREAD
#define RT_LWIP_ETHTHREAD_PRIORITY 12
#define RT_LWIP_ETHTHREAD_STACKSIZE 1024
#define RT_LWIP_ETHTHREAD_MBOX_SIZE 8
#define LWIP_NETIF_STATUS_CALLBACK 1
#define LWIP_NETIF_LINK_CALLBACK 1
#define SO_REUSE 1
#define LWIP_SO_RCVTIMEO 1
#define LWIP_SO_SNDTIMEO 1
#define LWIP_SO_RCVBUF 1
#define LWIP_NETIF_LOOPBACK 0
#define RT_LWIP_USING_PING

/* Modbus master and slave stack */


/* AT commands */


/* VBUS(Virtual Software BUS) */


/* Utilities */


/* RT-Thread online packages */

/* IoT - internet of things */


/* Wi-Fi */

/* Marvell WiFi */


/* Wiced WiFi */

#define PKG_USING_NETUTILS
#define PKG_NETUTILS_NTP
#define NETUTILS_NTP_TIMEZONE 8
#define NETUTILS_NTP_HOSTNAME "ntp.rt-thread.org"
#define NETUTILS_NTP_HOSTNAME2 "cn.ntp.org.cn"
#define NETUTILS_NTP_HOSTNAME3 "edu.ntp.org.cn"
#define PKG_USING_NETUTILS_V100

/* IoT Cloud */

#define PKG_USING_AZURE
#define PKG_USING_AZURE_MQTT_PROTOCOL
#define PKG_USING_AZURE_TELEMTRY_EXAMPLE
#define PKG_USING_AZURE_C2D_EXAMPLE
#define PKG_USING_AZURE_V10209

/* security packages */

#define PKG_USING_MBEDTLS

/* Select Root Certificate */

#define MBEDTLS_AES_ROM_TABLES
#define MBEDTLS_ECP_WINDOW_SIZE 2
#define MBEDTLS_SSL_MAX_CONTENT_LEN 3584
#define PKG_USING_MBEDTLS_V260

/* language packages */


/* multimedia packages */


/* tools packages */

#define PKG_USING_EASYFLASH
#define PKG_EASYFLASH_ENV
#define PKG_EASYFLASH_ENV_USING_PFS_MODE
#define PKG_EASYFLASH_ENV_SETTING_SIZE 2048
#define PKG_EASYFLASH_ENV_AUTO_UPDATE
#define PKG_EASYFLASH_ENV_VER_NUM 0
#define PKG_EASYFLASH_ERASE_GRAN 4096
#define PKG_EASYFLASH_START_ADDR 0
#define PKG_USING_EASYFLASH_V321
#define PKG_EASYFLASH_VER_NUM 0x30201

/* system packages */

#define PKG_USING_FAL
#define FAL_DEBUG 0
#define FAL_PART_HAS_TABLE_CFG
#define PKG_USING_FAL_V00200
#define PKG_FAL_VER_NUM 0x00200

/* peripheral libraries and drivers */

#define PKG_USING_STM32_SDIO
#define SDIO_BUFF_SIZE 4096
#define SDIO_MAX_FREQ 24000000
#define SDIO_ALIGN_LEN 32
#define PKG_USING_STM32_SDIO_V102

/* miscellaneous packages */


/* samples: kernel and components samples */


/* Privated Packages of RealThread */


/* Network Utilities */


/* Hardware Drivers Config */

/* Onboard Peripheral Drivers */

#define BSP_USING_FLASH
#define BSP_USING_WIFI
#define BSP_USING_WIFI_AUTO_INIT

/* Offboard Peripheral Drivers */


/* On-chip Peripheral Drivers */

#define BSP_USING_UART1
#define BSP_UART_USING_DMA_RX
#define BSP_USING_QSPI
#define BSP_USING_SDIO
#define BSP_USING_GPIO

/* External Libraries */

#define RT_USING_WIFI_6181_LIB
#define RT_USING_OTA_LIB

#endif
