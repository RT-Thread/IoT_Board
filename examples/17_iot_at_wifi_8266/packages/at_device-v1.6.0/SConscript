from building import *

cwd = GetCurrentDir()

src = []

if GetDepend(['AT_DEVICE_M26']):
    src = Glob('at_socket_m26.c')

if GetDepend(['AT_DEVICE_EC20']):
    src = Glob('at_socket_ec20.c')

if GetDepend(['AT_DEVICE_ESP8266']):
    src = Glob('at_socket_esp8266.c')

if GetDepend(['AT_DEVICE_RW007']):
    src = Glob('at_socket_rw007.c')

if GetDepend(['AT_DEVICE_SIM800C']):
    src += Glob('at_socket_sim800c.c')

if GetDepend(['AT_DEVICE_SIM76XX']):
    src += Glob('at_socket_sim76xx.c')

if GetDepend(['AT_DEVICE_NOT_SELECTED']):
   src = Glob('*.c')

group = DefineGroup('at_device', src, depend = ['PKG_USING_AT_DEVICE','AT_USING_SOCKET'], CPPPATH = [cwd])

Return('group')
