from building import *

cwd     = GetCurrentDir()
src     = []
path    = [cwd + '/inc']

src += ['src/infrared.c']
src += ['src/drv_infrared.c']

if GetDepend(['INFRARED_NEC_DECODER']):
    src += ['src/nec_decoder.c']

group = DefineGroup('Infrared_frame', src, depend = ['PKG_USING_INFRARED'], CPPPATH = path)

Return('group')
