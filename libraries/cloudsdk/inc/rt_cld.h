/*
 * File      : rt_cld.h
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-02-06     chenyong     the first version
 */
#ifndef _RT_CLD_H_
#define _RT_CLD_H_

#include <rtthread.h>

#define RT_CLD_SW_VERSION              "2.0.0"

#ifndef cld_malloc
#define cld_malloc                     rt_malloc
#endif

#ifndef cld_realloc
#define cld_realloc                    rt_realloc
#endif

#ifndef cld_calloc
#define cld_calloc                     rt_calloc
#endif

#ifndef cld_free
#define cld_free                       rt_free
#endif

#ifndef cld_strdup
#define cld_strdup                     rt_strdup
#endif

#define CLD_SERVER_URI                 "tcp://iot.rt-thread.com:1883"

/* ENV information name */
#define CLD_ENV_ACTIVATE               "cld_act"
#define CLD_ENV_USERNAME               "cld_usr"
#define CLD_ENV_PASSWORD               "cld_pwd"
#define CLD_ENV_DEVICE_ID              "cld_did"
#define CLD_ENV_DEVICE_KEY             "cld_dkey"
#define CLD_ENV_PRODUCT_ID             "cld_pid"
#define CLD_ENV_PRODUCT_KEY            "cld_pkey"
#define CLD_ENV_SN                     "cld_sn"
#define CLD_ENV_MAJOR_VER              "cld_dev_mver"
#define CLD_ENV_OTA_MAJOR_VER          "cld_ota_mver"
#define CLD_ENV_OTA_FW_VER             "cld_ota_sver"
#define CLD_ENV_OTA_FW_NAME            "cld_ota_name"

/* The maximum allocation memory of device information */
#define CLD_URI_MAX_LEN                128
#define CLD_USERNAME_MAX_LEN           16
#define CLD_PASSWORD_MAX_LEN           32
#define CLD_MAC_MAX_LEN                32
#define CLD_SIGN_MAX_LEN               32
#define CLD_SN_MAX_LEN                 32
#define CLD_DEV_ID_MAX_LEN             16
#define CLD_DEV_KEY_MAX_LEN            32
#define CLD_PRODUCT_ID_MAX_LEN         16
#define CLD_PRODUCT_KEY_MAX_LEN        36

#define CLD_PART_NAME_MAX_LEN          20
#define CLD_FW_VERSION_MAX_LEN         24
#define CLD_URL_MAX_LEN                256

/* Storage device proprietary parameters */
struct cld_dev_info
{
    char server_uri[CLD_URI_MAX_LEN];

    char product_id[CLD_PRODUCT_ID_MAX_LEN + 1];
    char product_key[CLD_PRODUCT_KEY_MAX_LEN + 1];

    char username[CLD_USERNAME_MAX_LEN + 1];
    char password[CLD_PASSWORD_MAX_LEN + 1];

    char device_id[CLD_DEV_ID_MAX_LEN + 1];
    char device_key[CLD_DEV_KEY_MAX_LEN + 1];

    char mac[CLD_MAC_MAX_LEN + 1];
    char sign[CLD_SIGN_MAX_LEN + 1];
    char sn[CLD_SN_MAX_LEN + 1];

#ifdef CLD_USING_OTA
    char major_version[CLD_FW_VERSION_MAX_LEN];
#endif
};

enum cld_ota_status
{
    CLD_OTA_OK,
    CLD_OTA_ERROR,
    CLD_OTA_NOMEM,
};

/* Stores parameters for upgrading service before OTA upgrade, not power protection!*/
struct cld_ota_info
{
    char name[CLD_PART_NAME_MAX_LEN];
    char version[CLD_FW_VERSION_MAX_LEN];
    char url[CLD_URL_MAX_LEN];
    int file_size;

    char major_version[CLD_FW_VERSION_MAX_LEN];
    int major_version_change;
    
    void (*ota_start_cb)(void);
    void (*ota_end_cb)(enum cld_ota_status status);
};

/**
 * Cloud SDK initialization and start
 *
 * @return -1: OTA initialization failed
 *         -2: network connect failed
 *        >=0: initialize success
 */
int rt_cld_sdk_init(void);

/* ========================== User defined function ============================ */

/* Get device unique identifier, string format of no more than 32 bytes */
void cld_port_get_device_sn(char *sn);

/* Get device product ID, string format must be 16 bytes */
void cld_port_get_product_id(char *id);

/* Get device product key, string format must be 32 bytes */
void cld_port_get_product_key(char *key);

/* OTA start callback function, set OTA related configuration */
void cld_port_ota_start(void);

/* OTA end callback function, the usual way is to reset the device than jump to bootloader */
void cld_port_ota_end(enum cld_ota_status status);

#endif /* _RT_CLD_H_ */
