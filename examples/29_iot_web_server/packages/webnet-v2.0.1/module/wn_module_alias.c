/*
 * File      : wn_module_alias.c
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
 */

#include <webnet.h>
#include <wn_module.h>
#include <wn_utils.h>

#ifdef WEBNET_USING_ALIAS

struct webnet_alias_item
{
    char* old_path;
    char* new_path;
};
static struct webnet_alias_item *_alias_items = RT_NULL;
static rt_uint32_t _alias_item_count = 0;

void webnet_alias_set(char* old_path, char* new_path)
{
    if (_alias_items == RT_NULL)
    {
        _alias_item_count = 1;
        _alias_items = (struct webnet_alias_item*)wn_malloc (sizeof(struct webnet_alias_item) *
                       _alias_item_count);
    }
    else
    {
        _alias_item_count += 1;
        _alias_items = (struct webnet_alias_item*) wn_realloc (_alias_items, sizeof(struct webnet_alias_item) *
                       _alias_item_count);
    }

    RT_ASSERT(_alias_items != RT_NULL);

    _alias_items[_alias_item_count - 1].old_path = wn_strdup(old_path);
    RT_ASSERT(_alias_items[_alias_item_count - 1].old_path != RT_NULL);
    _alias_items[_alias_item_count - 1].new_path = wn_strdup(new_path);
    RT_ASSERT(_alias_items[_alias_item_count - 1].new_path != RT_NULL);
}
RTM_EXPORT(webnet_alias_set);

int webnet_module_alias(struct webnet_session* session, int event)
{
    if (event == WEBNET_EVENT_URI_PHYSICAL)
    {
        int index;
        struct webnet_request* request;

        RT_ASSERT(session != RT_NULL);
        request = session->request;
        RT_ASSERT(request != RT_NULL);

        /* check whether the uri is a alias */
        for (index = 0; index < _alias_item_count; index ++)
        {
            if (str_path_with(request->path, _alias_items[index].old_path))
            {
                char* map_path;
                map_path = (char*) wn_malloc (WEBNET_PATH_MAX);
                RT_ASSERT(map_path != RT_NULL);

                rt_snprintf(map_path, WEBNET_PATH_MAX, "%s/%s",
                            _alias_items[index].new_path,
                            request->path + strlen(_alias_items[index].old_path));

                /* set new path */
                wn_free(request->path);
                request->path = map_path;

                return WEBNET_MODULE_CONTINUE;
            }
        }
    }

    return WEBNET_MODULE_CONTINUE;
}

#endif /* WEBNET_USING_ALIAS */
