import os
from building import *
import rtconfig

cwd  = GetCurrentDir()

src  = []
CPPPATH = []
CPPDEFINES = []
LOCAL_CCFLAGS = ''

#sample
if GetDepend(['PKG_USING_ALI_IOTKIT_MQTT_SAMPLE']):
    src += Glob('samples/mqtt/mqtt-example.c')

if GetDepend(['PKG_USING_ALI_IOTKIT_OTA']):
	src += Glob('samples/ota/ota_mqtt-example.c')

if GetDepend(['PKG_USING_ALI_IOTKIT_COAP_SAMPLE']):
	src += Glob('samples/coap/coap-example.c')

#src/cmp, need to enable CMP_ENABLED
#src += Glob('iotkit-embedded/src/cmp/Link-CMP/src/*.c')
#CPPPATH += [cwd + '/iotkit-embedded/src/cmp/Link-CMP']
#CPPPATH += [cwd + '/iotkit-embedded/src/cmp/Link-CMP/inc']

#src/coap
src += Glob('iotkit-embedded/src/coap/*.c')

#packages/iot-coap-c
src += Glob('iotkit-embedded/src/packages/iot-coap-c/*.c')
CPPPATH += [cwd + '/iotkit-embedded/src/packages/iot-coap-c']

#src/dm

#src/http, need to enable HTTP_COMM_ENABLED
#src += Glob('iotkit-embedded/src/http/*.c')

#src/import

#src/log
src += Glob('iotkit-embedded/src/log/LITE-log/*.c')
CPPPATH += [cwd + '/iotkit-embedded/src/log/LITE-log']

#src/mqtt
if GetDepend(['PKG_USING_ALI_IOTKIT_MQTT']):
	src += Glob('iotkit-embedded/src/mqtt/Link-MQTT/*.c')
	src += Glob('iotkit-embedded/src/mqtt/Link-MQTT/MQTTPacket/*.c')
	CPPPATH += [cwd + '/iotkit-embedded/src/mqtt/Link-MQTT']
	CPPPATH += [cwd + '/iotkit-embedded/src/mqtt/Link-MQTT/MQTTPacket']

#src/ota
if GetDepend(['PKG_USING_ALI_IOTKIT_OTA']):
    src += Glob('iotkit-embedded/src/ota/Link-OTA/src/*.c')
    CPPPATH += [cwd + '/iotkit-embedded/src/ota/Link-OTA']

SrcRemove(src, 'iotkit-embedded/src/ota/Link-OTA/src/ota_lib.c')   # have been include by ota.c
SrcRemove(src, 'iotkit-embedded/src/ota/Link-OTA/src/ota_mqtt.c')  # have been include by ota.c
SrcRemove(src, 'iotkit-embedded/src/ota/Link-OTA/src/ota_coap.c')  # have been include by ota.c
SrcRemove(src, 'iotkit-embedded/src/ota/Link-OTA/src/ota_fetch.c') # have been include by ota.c

#src/cota
#src += Glob('iotkit-embedded/src/cota/*.c')

#src/fota
#src += Glob('iotkit-embedded/src/fota/*.c')

#src/packages
src += Glob('iotkit-embedded/src/packages/LITE-utils/*.c')
CPPPATH += [cwd + '/iotkit-embedded/src/packages/LITE-utils']

SrcRemove(src, 'iotkit-embedded/src/packages/LITE-utils/lite-utils_prog.c')

#src/platform
#src/scripts

#src/sdk-tests
#src/shadow
#src/subdev

#src/system
src += Glob('iotkit-embedded/src/system/iotkit-system/src/*.c')
CPPPATH += [cwd + '/iotkit-embedded/src/system/iotkit-system']

#src/tfs
#src/tls

#src/utils
src += Glob('iotkit-embedded/src/utils/misc/*.c')
src += Glob('iotkit-embedded/src/utils/digest/*.c')
CPPPATH += [cwd + '/iotkit-embedded/src/utils/misc']
CPPPATH += [cwd + '/iotkit-embedded/src/utils/digest']

#ports
src += Glob('ports/rtthread/*.c')

if GetDepend(['PKG_USING_ALI_IOTKIT_MQTT_TLS']):
	src += Glob('ports/ssl/mbedtls/*.c')

#src/sdk-impl
CPPPATH += [cwd + '/iotkit-embedded/src/sdk-impl']
CPPPATH += [cwd + '/iotkit-embedded/src/sdk-impl/exports']
CPPPATH += [cwd + '/iotkit-embedded/src/sdk-impl/imports']

if GetDepend(['PKG_USING_ALI_IOTKIT_MQTT']):
	CPPDEFINES += ['MQTT_COMM_ENABLED']
	if GetDepend(['PKG_USING_ALI_IOTKIT_MQTT_DIRECT']):
		CPPDEFINES += ['MQTT_DIRECT']
	if not GetDepend(['PKG_USING_ALI_IOTKIT_MQTT_TLS']):
		CPPDEFINES += ['IOTX_WITHOUT_TLS']

if GetDepend(['PKG_USING_ALI_IOTKIT_COAP']):
	CPPDEFINES += ['COAP_COMM_ENABLED']
	if GetDepend(['PKG_USING_ALI_IOTKIT_COAP_DTLS']):
		CPPDEFINES += ['COAP_DTLS_SUPPORT']

# OTA_SIGNAL_CHANNEL: 1-mqtt; 2:coap; 4:http
if GetDepend(['PKG_USING_ALI_IOTKIT_MQTT_OTA']):
	CPPDEFINES += ['SERVICE_OTA_ENABLED', 'OTA_SIGNAL_CHANNEL=1']

if GetDepend(['PKG_USING_ALI_IOTKIT_COAP_OTA']):
	CPPDEFINES += ['SERVICE_OTA_ENABLED', 'OTA_SIGNAL_CHANNEL=2']

CPPDEFINES += ['IOTX_NET_INIT_WITH_PK_EXT', '_PLATFORM_IS_RTTHREAD_', 'IOTX_WITHOUT_ITLS']

if rtconfig.CROSS_TOOL == 'gcc' :
	CPPDEFINES += ['IOTX_PRJ_VERSION=\\"V2.10\\"']

group = DefineGroup('ali-iotkit', src, depend = ['PKG_USING_ALI_IOTKIT'], CPPPATH = CPPPATH, LOCAL_CCFLAGS = LOCAL_CCFLAGS, CPPDEFINES = CPPDEFINES)
Return('group')
