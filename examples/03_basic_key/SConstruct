import os
import sys
import rtconfig

if os.getenv('RTT_ROOT'):
    RTT_ROOT = os.getenv('RTT_ROOT')
else:
    RTT_ROOT = os.path.normpath(os.getcwd() + '/../../rt-thread')

sys.path = sys.path + [os.path.join(RTT_ROOT, 'tools')]
try:
    from building import *
except:
    print('Cannot found RT-Thread root directory, please check RTT_ROOT')
    print(RTT_ROOT)
    exit(-1)

TARGET = 'rtthread-stm32l4xx.' + rtconfig.TARGET_EXT

env = Environment(tools = ['mingw'],
    AS = rtconfig.AS, ASFLAGS = rtconfig.AFLAGS,
    CC = rtconfig.CC, CCFLAGS = rtconfig.CFLAGS,
    AR = rtconfig.AR, ARFLAGS = '-rc',
    LINK = rtconfig.LINK, LINKFLAGS = rtconfig.LFLAGS)
env.PrependENVPath('PATH', rtconfig.EXEC_PATH)

if rtconfig.PLATFORM == 'iar':
    env.Replace(CCCOM = ['$CC $CCFLAGS $CPPFLAGS $_CPPDEFFLAGS $_CPPINCFLAGS -o $TARGET $SOURCES'])
    env.Replace(ARFLAGS = [''])
    env.Replace(LINKCOM = env["LINKCOM"] + ' --map project.map')

Export('RTT_ROOT')
Export('rtconfig')

# prepare building environment
objs = PrepareBuilding(env, RTT_ROOT, has_libcpu=False)

SDK_ROOT = os.path.abspath('./')
bsp_vdir = 'build'

if os.path.exists(SDK_ROOT + '/libraries'):
    libraries_path_prefix = SDK_ROOT
else:
# include libraries and drivers
    libraries_path_prefix = os.path.dirname(os.path.dirname(SDK_ROOT))
    objs.extend(SConscript(libraries_path_prefix + '/libraries/SConscript', variant_dir = bsp_vdir + '/libraries', duplicate = 0))
    objs.extend(SConscript(libraries_path_prefix + '/drivers/SConscript', variant_dir = bsp_vdir + '/drivers', duplicate = 0))

# make a building
DoBuilding(TARGET, objs)
