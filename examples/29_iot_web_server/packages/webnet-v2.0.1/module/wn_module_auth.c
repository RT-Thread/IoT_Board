/*
 * File      : wn_module_auth.c
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
 * 2011-11-16     Bernard      added password modification feature.
 * 2011-11-19     Bernard      fixed the auth string length issue.
 * 2012-06-25     Bernard      fixed the authorization is NULL issue.
 */

#include <string.h>

#include <webnet.h>
#include <wn_module.h>
#include <wn_utils.h>

#ifdef WEBNET_USING_AUTH

struct webnet_auth_item
{
    char *path;

    /* username and password, which will encode as base64 as - username:password*/
    char *username_password;
};
static struct webnet_auth_item* _auth_items = RT_NULL;
static rt_uint32_t _auth_items_count = 0;

/**
 * set the authorization on the path
 *
 * @param path the path to be authorized
 * @param username_password the user and password, which format shall be
 * username:password
 */
void webnet_auth_set(const char* path, const char* username_password)
{
    if (_auth_items == RT_NULL)
    {
        _auth_items_count = 1;
        _auth_items = (struct webnet_auth_item*)wn_malloc (sizeof(struct webnet_auth_item) *
                      _auth_items_count);
    }
    else
    {
        rt_ubase_t index;

        /* check whether modify a password */
        for (index = 0; index < _auth_items_count; index ++)
        {
            if (strcmp(path, _auth_items[index].path) == 0)
            {
                wn_free(_auth_items[index].username_password);
                _auth_items[index].username_password = str_base64_encode(username_password);
                return;
            }
        }

        _auth_items_count += 1;
        _auth_items = (struct webnet_auth_item*) wn_realloc (_auth_items, sizeof(struct webnet_auth_item) *
                      _auth_items_count);
    }

    RT_ASSERT(_auth_items != RT_NULL);

    _auth_items[_auth_items_count - 1].path = wn_strdup(path);
    _auth_items[_auth_items_count - 1].username_password = str_base64_encode(username_password);
}
RTM_EXPORT(webnet_auth_set);

/**
 * Authorization module handler
 */
int webnet_module_auth(struct webnet_session* session, int event)
{
    if (event == WEBNET_EVENT_URI_PHYSICAL)
    {
        rt_uint32_t index;
        struct webnet_request *request;

        RT_ASSERT(session != RT_NULL);
        request = session->request;
        RT_ASSERT(request != RT_NULL);

        /* check authorization item */
        for (index = 0; index < _auth_items_count; index ++)
        {
            if (str_path_with(request->path, _auth_items[index].path))
            {
                if (request->authorization == RT_NULL ||
                        rt_strlen(_auth_items[index].username_password) !=
                        rt_strlen(request->authorization))
                {
                    /* set authorization request, 401 */
                    request->result_code = 401;
                    return WEBNET_MODULE_FINISHED;
                }

                /* check authorization */
                if (strcmp(request->authorization,
                           _auth_items[index].username_password) == 0)
                {
                    /* authorization OK */
                    request->result_code = 200;
                    return WEBNET_MODULE_CONTINUE;
                }
                else
                {
                    /* set authorization request, 401 */
                    request->result_code = 401;
                    return WEBNET_MODULE_FINISHED;
                }
            }
        }
    }

    return WEBNET_MODULE_CONTINUE;
}

#endif /* WEBNET_USING_AUTH */
