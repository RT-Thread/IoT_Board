from building import *

cwd  = GetCurrentDir()

src = Glob('src/*.c')

if GetDepend(['PKG_QRCODE_SAMPLE']):
    src += Glob('samples/*.c')

CPPPATH = [cwd + '/inc']

group = DefineGroup('qrcode', src, depend = ['PKG_USING_QRCODE'], CPPPATH = CPPPATH)

Return('group')
