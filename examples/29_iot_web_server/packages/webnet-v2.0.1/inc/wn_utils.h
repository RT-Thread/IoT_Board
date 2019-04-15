/*
 * File      : wn_utils.c
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

#ifndef __WN_UTILS_H__
#define __WN_UTILS_H__

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

int str_begin_with(const char *s, const char *t);
int str_end_with(const char* s, const char* t);
int str_path_with(const char *s, const char *t);
char *str_decode_path(char *path);
char *str_base64_encode(const char* src);
char* str_normalize_path(char* fullpath);
char * urlencode(const char *str, int len, int *new_length);
int urldecode(char *str, int len);

#ifdef  __cplusplus
    }
#endif

#endif /* __WN_UTILS_H__ */

