#
# File      : SConscript
# This file is part of RT-Thread RTOS/WebNet Server
# COPYRIGHT (C) 2011, Shanghai Real-Thread Technology Co., Ltd
#
# All rights reserved.
#
# Change Logs:
# Date           Author       Notes
# 2011-08-02     Bernard      the first version
#

Import('RTT_ROOT')
from building import *

cwd = GetCurrentDir()

src = Split("""
src/webnet.c
src/wn_mimetype.c
src/wn_request.c
src/wn_session.c
src/wn_utils.c
src/wn_module.c
""")

if GetDepend(['WEBNET_USING_ASP']):
    src += Glob('module/wn_module_asp.c')

if GetDepend(['WEBNET_USING_AUTH']):
    src += Glob('module/wn_module_auth.c')
    
if GetDepend(['WEBNET_USING_CGI']):
    src += Glob('module/wn_module_cgi.c')

if GetDepend(['WEBNET_USING_INDEX']):
    src += Glob('module/wn_module_index.c')

if GetDepend(['WEBNET_USING_ALIAS']):
    src += Glob('module/wn_module_alias.c')
    
if GetDepend(['WEBNET_USING_LOG']):
    src += Glob('module/wn_module_log.c')
    
if GetDepend(['WEBNET_USING_UPLOAD']):
    src += Glob('module/wn_module_upload.c')
    
if GetDepend(['WEBNET_USING_SSI']):
    src += Glob('module/wn_module_ssi.c')

if GetDepend(['WEBNET_USING_DAV']):
    src += Glob('module/wn_module_dav.c')
    
if GetDepend(['WEBNET_USING_SAMPLES']):
    src += Glob('samples/wn_sample.c')

if GetDepend(['WEBNET_USING_SAMPLES']) and GetDepend(['WEBNET_USING_UPLOAD']):
    src += Glob('samples/wn_sample_upload.c')

CPPPATH = [cwd + '/inc']

group = DefineGroup('WebNet', src, depend = ['PKG_USING_WEBNET'], CPPPATH = CPPPATH)

Return('group')
