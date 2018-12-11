/*
 * Copyright (c) 2006-2018 RT-Thread Development Team. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "iot_import.h"
#include "iot_export.h"

#include "rtthread.h"

#if defined(MQTT_ID2_AUTH) && defined(ON_DAILY)
    #define PRODUCT_KEY             "9rx2yMNV5l0"
    #define DEVICE_NAME             "sh_online_sample_mqtt"
    #define DEVICE_SECRET           "v9mqGzepKEphLhXmAoiaUIR2HZ7XwTky"
#elif defined(ON_DAILY)
    #define PRODUCT_KEY             "gsYfsxQJgeD"
    #define DEVICE_NAME             "DailyEnvDN"
    #define DEVICE_SECRET           "y1vzFkEgcuXnvkAfm627pwarx4HRNikX"
#elif defined(MQTT_ID2_AUTH)
    #define PRODUCT_KEY             "micKUvuzOps"
    #define DEVICE_NAME             "00AAAAAABBBBBB4B645F5800"
    #define DEVICE_SECRET           "v9mqGzepKEphLhXmAoiaUIR2HZ7XwTky"
#else
#ifdef PKG_USING_ALI_IOTKIT_PRODUCT_KEY
    #define PRODUCT_KEY             PKG_USING_ALI_IOTKIT_PRODUCT_KEY
#else
    #define PRODUCT_KEY             "yfTuLfBJTiL"
#endif

#ifdef PKG_USING_ALI_IOTKIT_DEVICE_NAME
    #define DEVICE_NAME             PKG_USING_ALI_IOTKIT_DEVICE_NAME
#else
    #define DEVICE_NAME             "TestDeviceForDemo"
#endif

#ifdef PKG_USING_ALI_IOTKIT_DEVICE_SECRET
    #define DEVICE_SECRET           PKG_USING_ALI_IOTKIT_DEVICE_SECRET
#else
    #define DEVICE_SECRET           "fSCl9Ns5YPnYN8Ocg0VEel1kXFnRlV6c"
#endif
#endif

#ifdef PKG_USING_ALI_IOTKIT_IS_LINKDEVELOP
/* ALINK TSL Device attribute report */
#define ALINK_PROPERTY_POST_PUB          "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/event/property/post"        
#define ALINK_PROPERTY_POST_REPLY_SUB    "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/event/property/post_reply"
#define ALINK_PROPERTY_SET_REPLY_SUB     "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/event/property/set_reply"
#define ALINK_SERVICE_SET_SUB            "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/service/property/set"
#else
#define TOPIC_UPDATE                     "/"PRODUCT_KEY"/"DEVICE_NAME"/update"
#define TOPIC_ERROR                      "/"PRODUCT_KEY"/"DEVICE_NAME"/update/error"
#define TOPIC_GET                        "/"PRODUCT_KEY"/"DEVICE_NAME"/get"
#define TOPIC_DATA                       "/"PRODUCT_KEY"/"DEVICE_NAME"/data"
#endif

/* These are pre-defined topics format*/
#define TOPIC_UPDATE_FMT                 "/%s/%s/update"
#define TOPIC_ERROR_FMT                  "/%s/%s/update/error"
#define TOPIC_GET_FMT                    "/%s/%s/get"
#define TOPIC_DATA_FMT                   "/%s/%s/data"

#define MQTT_MSGLEN                      (1024)

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

static char __product_key[PRODUCT_KEY_LEN + 1];
static char __device_name[DEVICE_NAME_LEN + 1];
static char __device_secret[DEVICE_SECRET_LEN + 1];

static int     user_argc;
static char   *user_param = NULL;

static void   *pclient;

static uint8_t is_running = 0;

static char* rt_strlwr(char *str)
 {
    if(str == NULL)
        return NULL;
         
    char *p = str;
    while (*p != '\0')
    {
        if(*p >= 'A' && *p <= 'Z')
            *p = (*p) + 0x20;
        p++;
    }
    return str;
}

static void event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;
    if (topic_info == NULL)
    {
        rt_kprintf("Topic info is null! Exit.");
        return;
    }
    uintptr_t packet_id = (uintptr_t)topic_info->packet_id;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            EXAMPLE_TRACE("undefined event occur.");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT:
            EXAMPLE_TRACE("MQTT disconnect.");
            break;

        case IOTX_MQTT_EVENT_RECONNECT:
            EXAMPLE_TRACE("MQTT reconnect.");
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
            EXAMPLE_TRACE("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
            EXAMPLE_TRACE("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            EXAMPLE_TRACE("publish success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
            EXAMPLE_TRACE("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            EXAMPLE_TRACE("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECVEIVED:
            EXAMPLE_TRACE("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                          topic_info->topic_len,
                          topic_info->ptopic,
                          topic_info->payload_len,
                          topic_info->payload);
            break;

        case IOTX_MQTT_EVENT_BUFFER_OVERFLOW:
            EXAMPLE_TRACE("buffer overflow, %s", msg->msg);
            break;

        default:
            EXAMPLE_TRACE("Should NOT arrive here.");
            break;
    }
}

static void _demo_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    /* print topic name and topic message */
    EXAMPLE_TRACE("----");
    EXAMPLE_TRACE("packetId: %d", ptopic_info->packet_id);
    EXAMPLE_TRACE("Topic: '%.*s' (Length: %d)",
                  ptopic_info->topic_len,
                  ptopic_info->ptopic,
                  ptopic_info->topic_len);
    EXAMPLE_TRACE("Payload: '%.*s' (Length: %d)",
                  ptopic_info->payload_len,
                  ptopic_info->payload,
                  ptopic_info->payload_len);
    EXAMPLE_TRACE("----");
}

#ifndef MQTT_ID2_AUTH
static void mqtt_client(void)
{
    int rc = 0;

    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;
    
    char *msg_buf = NULL, *msg_readbuf = NULL;

    IOT_OpenLog("mqtt");
    IOT_SetLogLevel(IOT_LOG_DEBUG);    

    if (NULL == (msg_buf = (char *)HAL_Malloc(MQTT_MSGLEN))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    if (NULL == (msg_readbuf = (char *)HAL_Malloc(MQTT_MSGLEN))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    HAL_GetProductKey(__product_key);
    HAL_GetDeviceName(__device_name);
    HAL_GetDeviceSecret(__device_secret);

    /* Device AUTH */
    if (0 != IOT_SetupConnInfo(__product_key, __device_name, __device_secret, (void **)&pconn_info)) {
        EXAMPLE_TRACE("AUTH request failed!");
        rc = -1;
        goto do_exit;
    }

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = pconn_info->port;
    mqtt_params.host = pconn_info->host_name;
    mqtt_params.client_id = pconn_info->client_id;
    mqtt_params.username = pconn_info->username;
    mqtt_params.password = pconn_info->password;
    mqtt_params.pub_key = pconn_info->pub_key;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = MQTT_MSGLEN;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = MQTT_MSGLEN;

    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;

    /* Convert uppercase letters in host to lowercase */
	rt_kprintf("host: %s\r\n", rt_strlwr((char*)mqtt_params.host));

    /* Construct a MQTT client with specify parameter */
    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        rc = -1;
        goto do_exit;
    }

#ifdef PKG_USING_ALI_IOTKIT_IS_LINKDEVELOP
    /* Subscribe the specific topic */
    rc = IOT_MQTT_Subscribe(pclient, ALINK_SERVICE_SET_SUB, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
#else
    rc = IOT_MQTT_Subscribe(pclient, TOPIC_DATA, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
#endif

    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        rc = -1;
        goto do_exit;
    }

#ifdef PKG_USING_ALI_IOTKIT_IS_LINKDEVELOP
    /* Subscribe the specific topic */
    rc = IOT_MQTT_Subscribe(pclient, ALINK_PROPERTY_POST_REPLY_SUB, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        rc = -1;
        goto do_exit;
    }
#endif

    IOT_MQTT_Yield(pclient, 200);

    do {
        /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient, 200);

        HAL_SleepMs(2000);

    } while (is_running);

    IOT_MQTT_Yield(pclient, 200);

#ifdef PKG_USING_ALI_IOTKIT_IS_LINKDEVELOP
    IOT_MQTT_Unsubscribe(pclient, ALINK_PROPERTY_POST_REPLY_SUB);
    IOT_MQTT_Unsubscribe(pclient, ALINK_SERVICE_SET_SUB);
#else
    IOT_MQTT_Unsubscribe(pclient, TOPIC_DATA);
#endif

    IOT_MQTT_Yield(pclient, 200);

    IOT_MQTT_Destroy(&pclient);

do_exit:
    if (NULL != msg_buf) {
        HAL_Free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    }

    if (NULL != user_param)
        HAL_Free(user_param);
    user_param = NULL;

    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_CloseLog();

    is_running = 0;

    EXAMPLE_TRACE("out of sample!");
}
#endif /* MQTT_ID2_AUTH */

#ifdef MQTT_ID2_AUTH
#include "tfs.h"
static char __device_id2[TFS_ID2_LEN + 1];
static void mqtt_client_secure()
{
    int rc = 0, msg_len, cnt = 0;
    void *pclient;
    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;
    iotx_mqtt_topic_info_t topic_msg;
    char msg_pub[128];
    char *msg_buf = NULL, *msg_readbuf = NULL;
    char  topic_update[IOTX_URI_MAX_LEN] = {0};
    char  topic_error[IOTX_URI_MAX_LEN] = {0};
    char  topic_get[IOTX_URI_MAX_LEN] = {0};
    char  topic_data[IOTX_URI_MAX_LEN] = {0};

    IOT_OpenLog("mqtt");
    IOT_SetLogLevel(IOT_LOG_DEBUG);    

    if (NULL == (msg_buf = (char *)HAL_Malloc(MQTT_MSGLEN))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    if (NULL == (msg_readbuf = (char *)HAL_Malloc(MQTT_MSGLEN))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    HAL_GetProductKey(__product_key);
    HAL_GetID2(__device_id2);

    /* Device AUTH */
    rc = IOT_SetupConnInfoSecure(__product_key, __device_id2, __device_id2, (void **)&pconn_info);
    if (rc != 0) {
        EXAMPLE_TRACE("AUTH request failed!");
        goto do_exit;
    }

    HAL_Snprintf(topic_update,IOTX_URI_MAX_LEN,TOPIC_UPDATE_FMT,__product_key,__device_id2);
    HAL_Snprintf(topic_error,IOTX_URI_MAX_LEN,TOPIC_ERROR_FMT,__product_key,__device_id2);
    HAL_Snprintf(topic_get,IOTX_URI_MAX_LEN,TOPIC_GET_FMT,__product_key,__device_id2);
    HAL_Snprintf(topic_data,IOTX_URI_MAX_LEN,TOPIC_DATA_FMT,__product_key,__device_id2);

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = pconn_info->port;
    mqtt_params.host = pconn_info->host_name;
    mqtt_params.client_id = pconn_info->client_id;
    mqtt_params.username = pconn_info->username;
    mqtt_params.password = pconn_info->password;
    mqtt_params.pub_key = pconn_info->pub_key;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = MQTT_MSGLEN;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = MQTT_MSGLEN;

    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;

    /* Construct a MQTT client with specify parameter */
    pclient = IOT_MQTT_ConstructSecure(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        rc = -1;
        goto do_exit;
    }

    /* Subscribe the specific topic */
    rc = IOT_MQTT_Subscribe(pclient, topic_data, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        rc = -1;
        goto do_exit;
    }

    HAL_SleepMs(1000);

    /* Initialize topic information */
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    strcpy(msg_pub, "message: hello! start!");

    topic_msg.qos = IOTX_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)msg_pub;
    topic_msg.payload_len = strlen(msg_pub);

    rc = IOT_MQTT_Publish(pclient, topic_data, &topic_msg);
    EXAMPLE_TRACE("rc = IOT_MQTT_Publish() = %d", rc);

    do {
        /* Generate topic message */
        cnt++;
        msg_len = snprintf(msg_pub, sizeof(msg_pub), "{\"attr_name\":\"temperature\", \"attr_value\":\"%d\"}", cnt);
        if (msg_len < 0) {
            EXAMPLE_TRACE("Error occur! Exit program");
            rc = -1;
            break;
        }

        topic_msg.payload = (void *)msg_pub;
        topic_msg.payload_len = msg_len;

        rc = IOT_MQTT_Publish(pclient, topic_data, &topic_msg);
        if (rc < 0) {
            EXAMPLE_TRACE("error occur when publish");
            rc = -1;
            break;
        }
        EXAMPLE_TRACE("packet-id=%u, publish topic msg='0x%02x%02x%02x%02x'...",
                      (uint32_t)rc,
                      msg_pub[0], msg_pub[1], msg_pub[2], msg_pub[3]
                     );

        /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient, 200);

        /* infinite loop if running with 'loop' argument */
        if (user_argc >= 2 && user_param && !strcmp("loop", user_param)) {
            HAL_SleepMs(2000);
            cnt = 0;
        }

    } while (cnt < 1);

    IOT_MQTT_Unsubscribe(pclient, TOPIC_DATA);

    HAL_SleepMs(200);

    IOT_MQTT_Destroy(&pclient);

do_exit:
    if (NULL != msg_buf) {
        HAL_Free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    }
    if (NULL != user_param)
        HAL_Free(user_param);
    user_param = NULL;

    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_CloseLog();

    EXAMPLE_TRACE("out of sample!");
}
#endif /* MQTT_ID2_AUTH*/

static int ali_mqtt_test_pub(void)
{
    int rc = 0;
    static uint16_t pub_msg_cnt = 0;
    uint8_t rgb_switch = 0, rand_num_r = 0, rand_num_g = 0, rand_num_b = 0;

    char   msg_pub[512];
    iotx_mqtt_topic_info_t topic_msg;

    if (!is_running)
    {
        HAL_Printf("MQTT test is not running! Please start MQTT first by using the \"ali_mqtt_test start\" command");
        return 0;
    }

    if (user_param && !strcmp("open", user_param))
    {
        rgb_switch = 1;
    }
    else if (user_param && !strcmp("close", user_param))
    {
        rgb_switch = 0;
    }
    else
    {
        return 0;
    }

    srand((int)(rt_tick_get()) / (pub_msg_cnt + 1));
    rand_num_r = (uint8_t)(rand()%255);

    srand((int)(rt_tick_get()) / (pub_msg_cnt + 2));
    rand_num_g = (uint8_t)(rand()%255);

    srand((int)(rt_tick_get()) / (pub_msg_cnt + 3));
    rand_num_b = (uint8_t)(rand()%255);

    /* Initialize topic information */
    memset(msg_pub, 0x0, sizeof(msg_pub));

    snprintf(msg_pub, sizeof(msg_pub), 
            "{\"id\" : \"%d\",\"version\":\"1.0\",\"params\" : "
            "{\"RGBColor\" : {\"Red\":%d,\"Green\":%d,\"Blue\":%d},"
            "\"LightSwitch\" : %d},"
            "\"method\":\"thing.event.property.post\"}",
            ++pub_msg_cnt, rand_num_r, rand_num_g, rand_num_b, rgb_switch);

    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    topic_msg.qos = IOTX_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)msg_pub;
    topic_msg.payload_len = strlen(msg_pub);

#ifdef PKG_USING_ALI_IOTKIT_IS_LINKDEVELOP
    rc = IOT_MQTT_Publish(pclient, ALINK_PROPERTY_POST_PUB, &topic_msg);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("error occur when publish");
        rc = -1;
        return rc;
    }

    EXAMPLE_TRACE("\n publish message: \n topic: %s\n payload: %s\n rc = %d", ALINK_PROPERTY_POST_PUB, topic_msg.payload, rc);
#else
    rc = IOT_MQTT_Publish(pclient, TOPIC_UPDATE, &topic_msg);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("error occur when publish");
        rc = -1;
        return rc;
    }

    EXAMPLE_TRACE("\n publish message: \n topic: %s\n payload: \%s\n rc = %d", TOPIC_UPDATE, topic_msg.payload, rc);

    rc = IOT_MQTT_Publish(pclient, TOPIC_DATA, &topic_msg);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("error occur when publish");
        rc = -1;
        return rc;
    }

    EXAMPLE_TRACE("\n publish message: \n topic: %s\n payload: \%s\n rc = %d", TOPIC_DATA, topic_msg.payload, rc);
#endif
    return 0;
}

static int ali_mqtt_main(int argc, char **argv)
{
    rt_thread_t tid;

    user_argc = argc;
    if (2 == user_argc)
    {
        if (!strcmp("start", argv[1]))
        {
            if (1 == is_running)
            {
                HAL_Printf("MQTT test is already running! Please stop running first by using the \"ali_mqtt_test stop\" command\n");
                return 0;
            }
            is_running = 1;
        }
        else if (!strcmp("stop", argv[1]))
        {
            if (0 == is_running)
            {
                HAL_Printf("MQTT test is already stopped!\n");
                return 0;
            }
            is_running = 0;
            // stop mqtt test
            return 0;
        }
        else
        {
            HAL_Printf("Input param error! Example: ali_mqtt_test start/stop or ali_mqtt_test pub open/close\n");
            return 0;            
        }
    }
    else if(3 == user_argc)
    {
        if (!strcmp("pub", argv[1]))
        {
            user_param = (char*)rt_strdup((const char*)argv[2]);
            HAL_Printf("param:%s\n", user_param);

            // publish
            ali_mqtt_test_pub();
            return 0;
        }
        else
        {
            HAL_Printf("Input param error! Example: ali_mqtt_test start/stop or ali_mqtt_test pub open/close\n");
            return 0;
        }
    }
    else
    {
        HAL_Printf("Input param error! Example: ali_mqtt_test start/stop or ali_mqtt_test pub open/close\n");
        return 0;
    }

#ifdef IOTX_PRJ_VERSION
    HAL_Printf("iotkit-embedded sdk version: %s\n", IOTX_PRJ_VERSION);
#endif

    HAL_SetProductKey(PRODUCT_KEY);
    HAL_SetDeviceName(DEVICE_NAME);
    HAL_SetDeviceSecret(DEVICE_SECRET);

#ifndef MQTT_ID2_AUTH
    tid = rt_thread_create("ali-mqtt",
                    (void (*)(void *))mqtt_client, NULL,
                    6 * 1024, RT_THREAD_PRIORITY_MAX / 2 - 1, 10);
#else
    tid = rt_thread_create("ali-mqtt",
                    mqtt_client_secure, NULL,
                    6 * 1024, RT_THREAD_PRIORITY_MAX / 2 - 1, 10);                    
#endif
    if (tid != RT_NULL)
            rt_thread_startup(tid);

    return 0;
}
#ifdef RT_USING_FINSH
#include <finsh.h>

MSH_CMD_EXPORT_ALIAS(ali_mqtt_main, ali_mqtt_test, Example: ali_mqtt_test start/pub [open/close]/stop);
#endif
