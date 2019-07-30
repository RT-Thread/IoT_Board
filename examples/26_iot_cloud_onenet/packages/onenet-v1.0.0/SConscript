from building import *

cwd = GetCurrentDir()
src = Glob('src/onenet_http.c')

if GetDepend(['PKG_USING_ONENET_SAMPLE']):
    src += Glob('samples/*.c')

if GetDepend(['ONENET_USING_MQTT']):
    src += Glob('src/onenet_mqtt.c')

path = [cwd + '/inc']

group = DefineGroup('onenet', src, depend = ['PKG_USING_ONENET'], CPPPATH = path)

Return('group')
