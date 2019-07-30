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
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "paho_mqtt.h"
#include "wifi_config.h"
#include <wlan_mgnt.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/**
 * MQTT URI farmat:
 * domain mode
 * tcp://iot.eclipse.org:1883
 *
 * ipv4 mode
 * tcp://192.168.10.1:1883
 * ssl://192.168.10.1:1884
 *
 * ipv6 mode
 * tcp://[fe80::20c:29ff:fe9a:a07e]:1883
 * ssl://[fe80::20c:29ff:fe9a:a07e]:1884
 */
#define MQTT_URI "tcp://iot.eclipse.org:1883"
#define MQTT_USERNAME "admin"
#define MQTT_PASSWORD "admin"
#define MQTT_SUBTOPIC "/mqtt/test/"
#define MQTT_PUBTOPIC "/mqtt/test/"

/* define MQTT client context */
static MQTTClient client;
static void mq_start(void);
static void mq_publish(const char *send_str);

char sup_pub_topic[48] = {0};

int main(void)
{
    /* 注册 wlan 回调函数 */
    rt_wlan_register_event_handler(RT_WLAN_EVT_READY, (void (*)(int, struct rt_wlan_buff *, void *))mq_start, RT_NULL);
    /* 初始化自动连接功能 */
    wlan_autoconnect_init();
    /* 使能 wlan 自动连接 */
    rt_wlan_config_autoreconnect(RT_TRUE);
}

static void mqtt_sub_callback(MQTTClient *c, MessageData *msg_data)
{
    *((char *)msg_data->message->payload + msg_data->message->payloadlen) = '\0';
    LOG_D("Topic: %.*s receive a message: %.*s",
          msg_data->topicName->lenstring.len,
          msg_data->topicName->lenstring.data,
          msg_data->message->payloadlen,
          (char *)msg_data->message->payload);

    return;
}

static void mqtt_sub_default_callback(MQTTClient *c, MessageData *msg_data)
{
    *((char *)msg_data->message->payload + msg_data->message->payloadlen) = '\0';
    LOG_D("mqtt sub default callback: %.*s %.*s",
          msg_data->topicName->lenstring.len,
          msg_data->topicName->lenstring.data,
          msg_data->message->payloadlen,
          (char *)msg_data->message->payload);
    return;
}

static void mqtt_connect_callback(MQTTClient *c)
{
    LOG_I("Start to connect mqtt server");
}

static void mqtt_online_callback(MQTTClient *c)
{
    LOG_D("Connect mqtt server success");
    LOG_D("Publish message: Hello,RT-Thread! to topic: %s", sup_pub_topic);
    mq_publish("Hello,RT-Thread!");
}

static void mqtt_offline_callback(MQTTClient *c)
{
    LOG_I("Disconnect from mqtt server");
}

/* 创建与配置 mqtt 客户端 */
static void mq_start(void)
{
    /* 初始 condata 参数 */
    MQTTPacket_connectData condata = MQTTPacket_connectData_initializer;
    static char cid[20] = {0};

    static int is_started = 0;
    if (is_started)
    {
        return;
    }
    /* 配置 MQTT 文本参数 */
    {
        client.isconnected = 0;
        client.uri = MQTT_URI;

        /* 生成随机客户端 ID */
        rt_snprintf(cid, sizeof(cid), "rtthread%d", rt_tick_get());
        rt_snprintf(sup_pub_topic, sizeof(sup_pub_topic), "%s%s", MQTT_PUBTOPIC, cid);
        /* 配置连接参数 */
        memcpy(&client.condata, &condata, sizeof(condata));
        client.condata.clientID.cstring = cid;
        client.condata.keepAliveInterval = 60;
        client.condata.cleansession = 1;
        client.condata.username.cstring = MQTT_USERNAME;
        client.condata.password.cstring = MQTT_PASSWORD;

        /* 配置 mqtt 参数 */
        client.condata.willFlag = 0;
        client.condata.will.qos = 1;
        client.condata.will.retained = 0;
        client.condata.will.topicName.cstring = sup_pub_topic;

        client.buf_size = client.readbuf_size = 1024;
        client.buf = malloc(client.buf_size);
        client.readbuf = malloc(client.readbuf_size);
        if (!(client.buf && client.readbuf))
        {
            LOG_E("no memory for MQTT client buffer!");
            goto _exit;
        }

        /* 设置事件回调 */
        client.connect_callback = mqtt_connect_callback;
        client.online_callback = mqtt_online_callback;
        client.offline_callback = mqtt_offline_callback;
        /* 设置要订阅的 topic 和 topic 对应的回调函数 */
        client.messageHandlers[0].topicFilter = sup_pub_topic;
        client.messageHandlers[0].callback = mqtt_sub_callback;
        client.messageHandlers[0].qos = QOS1;

        /* 设置默认订阅回调函数 */
        client.defaultMessageHandler = mqtt_sub_default_callback;
    }

    /* 启动 MQTT 客户端 */
    LOG_D("Start mqtt client and subscribe topic:%s", sup_pub_topic);
    paho_mqtt_start(&client);
    is_started = 1;

_exit:
    return;
}

/* MQTT 消息发布函数 */
static void mq_publish(const char *send_str)
{
    MQTTMessage message;
    const char *msg_str = send_str;
    const char *topic = sup_pub_topic;
    message.qos = QOS1;
    message.retained = 0;
    message.payload = (void *)msg_str;
    message.payloadlen = strlen(message.payload);

    MQTTPublish(&client, topic, &message);

    return;
}
