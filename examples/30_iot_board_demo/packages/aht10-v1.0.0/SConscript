
from building import *

cwd     = GetCurrentDir()
src     = Glob('*.c') + Glob('*.cpp')
path    = [cwd]

group = DefineGroup('aht10', src, depend = ['PKG_USING_AHT10'], CPPPATH = path)

Return('group')
