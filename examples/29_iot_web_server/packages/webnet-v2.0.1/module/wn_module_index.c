/*
 * File      : wn_module_index.c
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
 * 2012-12-21     aozima       fixed version print.
 */

#include <webnet.h>
#include <wn_module.h>
#include <wn_utils.h>

#ifdef RT_USING_DFS
#include <dfs_posix.h>
#endif

#ifdef WEBNET_USING_INDEX

int webnet_module_dirindex(struct webnet_session* session, int event)
{
    if( (session->request->method != WEBNET_GET)
            && (session->request->method != WEBNET_POST) )
    {
        return WEBNET_MODULE_CONTINUE;
    }

    if (event == WEBNET_EVENT_URI_POST)
    {
        DIR *dir;
        struct stat file_stat;
        struct webnet_request *request;
        static const char* header = "<html><head><title>Index of %s</title></head><body bgcolor=\"white\"><h1>Index of %s</h1><hr><pre>";
        static const char* foot = "</pre><hr>WebNet/%s (RT-Thread)</body></html>";

        RT_ASSERT(session != RT_NULL);
        request = session->request;
        RT_ASSERT(request != RT_NULL);

        if (stat(request->path, &file_stat) < 0 || !S_ISDIR(file_stat.st_mode))
        {
            return WEBNET_MODULE_CONTINUE;
        }
        
        dir = opendir(request->path);
        if (dir != RT_NULL)
        {
            struct stat s;
            struct dirent* dirent;
            const char* sub_path;
            char *fullpath;
            char *delim;

            dirent = RT_NULL;
            fullpath = wn_malloc (WEBNET_PATH_MAX);
            if (fullpath == RT_NULL)
            {
                request->result_code = 500;
                return WEBNET_MODULE_FINISHED;
            }

            webnet_session_set_header(session, "text/html", 200, "OK", -1);
            /* get sub path */
            sub_path = request->path + strlen(webnet_get_root());
            delim = strrchr(sub_path, '/');
            rt_snprintf(fullpath, delim - sub_path + 1, "%s", sub_path);
            webnet_session_printf(session, header, sub_path, sub_path);
            /* display parent directory */
            webnet_session_printf(session, "<a href=\"../\">..</a>\n");

            /* list directory */
            do
            {
                dirent = readdir(dir);
                if (dirent == RT_NULL) break;

                rt_memset(&s, 0, sizeof(struct stat));

                /* build full path for each file */
                rt_sprintf(fullpath, "%s/%s", request->path, dirent->d_name);
                str_normalize_path(fullpath);

                stat(fullpath, &s);
                rt_sprintf(fullpath, "%s/%s", sub_path, dirent->d_name);
                if ( s.st_mode & S_IFDIR )
                {
                    webnet_session_printf(session, "<a href=\"%s/\">%s/</a>\n", fullpath, dirent->d_name);
                }
                else
                {
                    webnet_session_printf(session, "<a href=\"%s\">%s</a>\t\t\t\t\t%d\n", fullpath, dirent->d_name, s.st_size);
                }
            }
            while (dirent != RT_NULL);

            closedir(dir);
            wn_free(fullpath);

            /* set foot */
            webnet_session_printf(session, foot, WEBNET_VERSION);

            return WEBNET_MODULE_FINISHED;
        }
    }

    return WEBNET_MODULE_CONTINUE;
}

#endif /* WEBNET_USING_INDEX */

