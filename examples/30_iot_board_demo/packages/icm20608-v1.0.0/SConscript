
from building import *

cwd     = GetCurrentDir()
src     = Glob('*.c') + Glob('*.cpp')
path = [cwd]

group = DefineGroup('icm20608', src, depend = ['PKG_USING_ICM20608'], CPPPATH = path)

Return('group')
