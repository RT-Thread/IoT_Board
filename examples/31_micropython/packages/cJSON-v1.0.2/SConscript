from building import *

cwd     = GetCurrentDir()
src     = Glob('*.c')
CPPPATH = [cwd]

group = DefineGroup('cJSON', src, depend = ['PKG_USING_CJSON'], CPPPATH = CPPPATH)

Return('group')
