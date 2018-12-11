
from building import *

cwd     = GetCurrentDir()
src     = Glob('*.c') + Glob('*.cpp')
path    = [cwd]

group = DefineGroup('ap3216c', src, depend = ['PKG_USING_AP3216C'], CPPPATH = path)

Return('group')
