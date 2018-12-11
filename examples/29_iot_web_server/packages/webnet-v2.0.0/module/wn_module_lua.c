/*
 * File      : wn_module_lua.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 * This software is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. For the terms of this
 * license, see <http://www.gnu.org/licenses/>.
 *
 * You are free to use this software under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * Alternatively for commercial application, you can contact us
 * by email <business@rt-thread.com> for commercial license.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-08-02     Bernard      the first version
 * 2013-11-23     aozima       redirect lua print.
 */

#include <string.h>

#include <webnet.h>
#include <wn_module.h>

#ifdef RT_USING_DFS
#include <dfs_posix.h>
#endif

#if defined(WEBNET_USING_LUA) && defined(RT_USING_LUA)

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

static struct webnet_session* lua_session;

static int print(lua_State *L)
{
    int n = lua_gettop(L);
    int i;

    if(lua_session == RT_NULL)
        return 0;

    for (i=1; i<=n; i++)
    {
        if (lua_isstring(L,i))
            webnet_session_printf(lua_session, "%s", lua_tostring(L,i));
        else if (lua_isnumber(L, i))
            webnet_session_printf(lua_session, "%d", lua_tointeger(L,i));
        else if (lua_isnil(L, i))
            webnet_session_printf(lua_session, "%s", "nil");
        else if (lua_isboolean(L, i))
            webnet_session_printf(lua_session, "%s", lua_toboolean(L,i) ? "true" : "false");
        else
            webnet_session_printf(lua_session, "%s:%p", luaL_typename(L,i), lua_topointer(L, i));
    }

    return 0;
}

static void _webnet_lua_dofile(struct webnet_session* session, const char * path)
{
    int ret = 0;

    lua_State *L = lua_open();
    if(L == RT_NULL)
        return;

    lua_session = session;

//    luaopen_base(L);
//    luaopen_table(L);
//    luaopen_io(L);
//    luaopen_os(L);
//    luaopen_string(L);
//    luaopen_math(L);
//    luaopen_debug(L);
//    luaopen_package(L);

    luaL_openlibs(L);

    lua_register(L, "print", print);

    ret = luaL_loadfile(L, path);
    if ( ret != 0 ) goto _exception;

    ret = lua_pcall(L,0, LUA_MULTRET,0) ;
    if ( ret != 0 ) goto _exception;

_exception:
    if(ret != 0)
    {
        const char* fmt = "<html><head><title>%d %s</title></head><body>%d %s</body></html>\r\n";
        struct webnet_request* request;

        const char* title = "Internal Server Error";
        int code = 500;


        RT_ASSERT(session != RT_NULL);
        request = session->request;
        RT_ASSERT(request != RT_NULL);

        webnet_session_set_header(session, "text/html", request->result_code, title, -1);
        webnet_session_printf(session, fmt, request->result_code, title, code, lua_tostring(L,-1));

        request->result_code = 200; /* set code 200 to end the session. */
    }

    lua_close(L);
    lua_session = RT_NULL;
    return;
}

int webnet_module_lua(struct webnet_session* session, int event)
{
    struct webnet_request* request;

    RT_ASSERT(session != RT_NULL);
    request = session->request;
    RT_ASSERT(request != RT_NULL);

    if (event == WEBNET_EVENT_URI_POST)
    {
        int fd;

        /* check whether a lua script */
        if ((strstr(request->path, ".lua") != RT_NULL) ||
                (strstr(request->path, ".LUA") != RT_NULL))
        {
            /* try to open this file */
            fd = open(request->path, O_RDONLY, 0);
            if ( fd >= 0)
            {
                close(fd);
                _webnet_lua_dofile(session, request->path);

                return WEBNET_MODULE_FINISHED;
            }
            else
            {
                /* no this file */
                request->result_code = 404;
                return WEBNET_MODULE_FINISHED;
            }
        }
        else
        {
            /* try index.lua */
            char *lua_filename;

            lua_filename = (char*) wn_malloc (WEBNET_PATH_MAX);
            if (lua_filename != RT_NULL)
            {
                rt_snprintf(lua_filename, WEBNET_PATH_MAX, "%s/index.lua", request->path);
                fd = open(lua_filename, O_RDONLY, 0);

                if (fd >= 0)
                {
                    close(fd);
                    _webnet_lua_dofile(session, lua_filename);
                    wn_free(lua_filename);

                    return WEBNET_MODULE_FINISHED;
                }

                wn_free(lua_filename);
            }
        }
    }

    return WEBNET_MODULE_CONTINUE;
}

#endif /* defined(WEBNET_USING_LUA) && defined(RT_USING_LUA) */
