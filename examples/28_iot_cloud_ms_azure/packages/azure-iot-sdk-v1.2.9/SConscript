Import('RTT_ROOT')
from building import *

# get current directory
cwd = GetCurrentDir()

# The set of source files associated with this SConscript file.
src  = Glob('azure/c-utility/src/xlogging.c')
src += Glob('azure/c-utility/src/singlylinkedlist.c')
src += Glob('azure/c-utility/src/buffer.c')
src += Glob('azure/c-utility/src/consolelogger.c')
src += Glob('azure/c-utility/src/constbuffer.c')
src += Glob('azure/c-utility/src/constmap.c')
#src += Glob('azure/c-utility/src/crt_abstractions.c')
src += Glob('azure-port/lib/c_utility_crt_abstractions.c')
src += Glob('azure/c-utility/src/doublylinkedlist.c')
src += Glob('azure/c-utility/src/gballoc.c')
src += Glob('azure/c-utility/src/gb_stdio.c')
src += Glob('azure/c-utility/src/gb_time.c')
src += Glob('azure/c-utility/src/hmac.c')
src += Glob('azure/c-utility/src/hmacsha256.c')
src += Glob('azure/c-utility/src/httpapiex.c')
src += Glob('azure/c-utility/src/httpapiexsas.c')
src += Glob('azure/c-utility/src/httpheaders.c')
src += Glob('azure/c-utility/src/map.c')
src += Glob('azure/c-utility/src/optionhandler.c')
src += Glob('azure/c-utility/src/sastoken.c')
#src += Glob('azure/c-utility/src/sha1.c')
src += Glob('azure-port/lib/c_utility_sha1.c')
src += Glob('azure/c-utility/src/sha224.c')
src += Glob('azure/c-utility/src/sha384-512.c')
src += Glob('azure/c-utility/src/string_tokenizer.c')
src += Glob('azure/c-utility/src/urlencode.c')
src += Glob('azure/c-utility/src/usha.c')
src += Glob('azure/c-utility/src/vector.c')
src += Glob('azure/c-utility/src/xio.c')
src += Glob('azure/c-utility/src/strings.c')

#src += Glob('azure/c-utility/src/base64.c')
src += Glob('azure-port/lib/c_utility_base64.c')

# add iothub_client files
src += Glob('azure/iothub_client/src/iothub_client_ll.c')
src += Glob('azure/iothub_client/src/iothub_device_client_ll.c')
src += Glob('azure/iothub_client/src/iothub_client_core_ll.c')
src += Glob('azure/iothub_client/src/iothub_client_authorization.c')
src += Glob('azure/iothub_client/src/iothub_message.c')
src += Glob('azure/iothub_client/src/iothub_client_diagnostic.c')
src += Glob('azure/iothub_client/src/iothub_client_ll_uploadtoblob.c')
src += Glob('azure/iothub_client/src/blob.c')
src += Glob('azure/iothub_client/src/iothub.c')
src += Glob('azure/iothub_client/src/iothubtransportmqtt.c')
src += Glob('azure/iothub_client/src/iothubtransport_mqtt_common.c')
src += Glob('azure/iothub_client/src/iothub_client_retry_control.c')
src += Glob('azure/iothub_client/src/iothubtransporthttp.c')

src += Glob('azure/c-utility/src/iothub_client_ll_uploadtoblob.c')
src += Glob('azure/c-utility/src/iothub_client_authorization.c')
src += Glob('azure/c-utility/src/iothub_client_retry_control.c')
src += Glob('azure/c-utility/src/iothub_client_diagnostic.c')
src += Glob('azure/c-utility/src/iothub_message.c')
src += Glob('azure/c-utility/src/iothubtransport.c')
src += Glob('azure/c-utility/src/iothubtransportmqtt.c')
src += Glob('azure/c-utility/src/iothubtransport_mqtt_common.c')
src += Glob('azure/c-utility/src/version.c')

# add umqtt files
src += Glob('azure/umqtt/src/mqtt_client.c')
src += Glob('azure/umqtt/src/mqtt_codec.c')
src += Glob('azure/umqtt/src/mqtt_message.c')

# add deps
src += Glob('azure/deps/parson/parson.c')

# add serializer files
src += Glob('azure/serializer/src/codefirst.c')
src += Glob('azure/serializer/src/agenttypesystem.c')
src += Glob('azure/serializer/src/commanddecoder.c')
src += Glob('azure/serializer/src/datamarshaller.c')
src += Glob('azure/serializer/src/datapublisher.c')
src += Glob('azure/serializer/src/dataserializer.c')
src += Glob('azure/serializer/src/iotdevice.c')
src += Glob('azure/serializer/src/jsondecoder.c')
src += Glob('azure/serializer/src/jsonencoder.c')
src += Glob('azure/serializer/src/methodreturn.c')
src += Glob('azure/serializer/src/multitree.c')
src += Glob('azure/serializer/src/schema.c')
src += Glob('azure/serializer/src/schemalib.c')
src += Glob('azure/serializer/src/schemaserializer.c')

# add rtos port files 
src += Glob('azure-port/rt-thread/lock.c')
src += Glob('azure-port/rt-thread/threadapi.c')
src += Glob('azure-port/rt-thread/tickcounter.c')

# add internet port file
src += Glob('azure/c-utility/pal/socket_async.c')
src += Glob('azure/c-utility/pal/dns_async.c')

# add adapter port file
src += Glob('azure-port/pal/src/agenttime.c')

src += Glob('azure-port/pal/src/azure_platform.c')

src += Glob('azure-port/pal/src/tlsio_mbedtls.c')
src += Glob('azure-port/pal/src/socketio_berkeley.c')
src += Glob('azure/c-utility/adapters/httpapi_compact.c')
#src += Glob('azure/c-utility/adapters/httpapi_curl.c')

# add sample port file

if GetDepend(['PKG_USING_AZURE_TELEMTRY_EXAMPLE']):
    src += Glob('samples/iothub_ll_telemetry_sample.c')

if GetDepend(['PKG_USING_AZURE_C2D_EXAMPLE']):
    src += Glob('samples/iothub_ll_c2d_sample.c')  

# add certs
# src += Glob('azure/certs/certs.c')
src += Glob('azure-port/lib/azure_certs.c')

# add source directory

path  = [cwd + '/azure/c-utility/inc']
path += [cwd + '/azure']
path += [cwd + '/azure/c-utility/inc/azure_c_shared_utility']
path += [cwd + '/azure/iothub_client/inc']
path += [cwd + '/azure/serializer/inc']
path += [cwd + '/azure/umqtt/inc']
path += [cwd + '/azure/umqtt/inc/azure_umqtt_c']
path += [cwd + '/azure/deps/parson']

# add port source directory
path += [cwd + '/azure/c-utility/pal/generic']
path += [cwd + '/azure/c-utility/pal']
path += [cwd + '/azure/c-utility/pal/inc']
path += [cwd + '/azure/c-utility/pal/lwip']
path += [cwd + '/azure/c-utility/src']
path += [cwd + '/azure/c-utility/adapters']
path += [cwd + '/azure/umqtt/src']
path += [cwd + '/azure/iothub_client/src']
path += [cwd + '/azure/serializer/src']
path += [cwd + '/azure/deps/parson']
path += [cwd + '/azure-port/pal/inc']

CPPDEFINES = ['SET_TRUSTED_CERT_IN_SAMPLES']

group = DefineGroup('azure', src, depend = ['PKG_USING_AZURE'], CPPPATH = path, CPPDEFINES = CPPDEFINES)

Return('group')
