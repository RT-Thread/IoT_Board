Import('RTT_ROOT')
Import('rtconfig')
from building import *

cwd = GetCurrentDir()
src = Glob('core/*.c')

CPPPATH = [cwd + '/inc']

CPPPATH += [cwd + '/services']

if GetDepend('ADB_SERVICE_SHELL_ENABLE'):
    src += ['services/shell_service.c']

if GetDepend('ADB_SERVICE_FILE_ENABLE'):
    src += ['services/file_sync_service.c']

if GetDepend('ADB_EXTERNAL_MOD_ENABLE'):
    src += ['services/file_exmod.c']

if GetDepend('ADB_FILESYNC_MOD_ENABLE'):
    src += ['services/file_sync_mod.c']

if GetDepend('ADB_FILELIST_MOD_ENABLE'):
    src += ['services/file_list_mod.c']

group = DefineGroup('adb', src, depend = ['PKG_USING_ADBD'], CPPPATH = CPPPATH)

Return('group')
