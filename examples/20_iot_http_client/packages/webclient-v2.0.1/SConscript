from building import *

cwd  = GetCurrentDir()
path = [cwd + '/inc']

src  = Glob('src/*.c')

if GetDepend(['WEBCLIENT_USING_SAMPLES']):
    src += Glob('samples/*.c')

group = DefineGroup('WebClient', src, depend = ['PKG_USING_WEBCLIENT'], CPPPATH = path)

Return('group')
