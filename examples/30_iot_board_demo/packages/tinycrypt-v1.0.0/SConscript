# RT-Thread building script for component

from building import *

cwd = GetCurrentDir()
src = Glob('src/*.c') + Glob('src/*.cpp')
CPPPATH = [cwd + '/include']

group = DefineGroup('TinyCrypt', src, depend = ['PKG_USING_TINYCRYPT'], CPPPATH = CPPPATH)

Return('group')
