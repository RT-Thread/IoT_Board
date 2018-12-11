Import('RTT_ROOT')
Import('rtconfig')
from building import *

cwd = GetCurrentDir()
src = Glob('*.c')

CPPPATH = [cwd]

group = DefineGroup('stm32_sdio', src, depend = ['PKG_USING_STM32_SDIO'], CPPPATH = CPPPATH)

Return('group')
