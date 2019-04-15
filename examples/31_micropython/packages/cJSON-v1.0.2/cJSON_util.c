#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <rtthread.h>

#include "cJSON.h"
#include "cJSON_util.h"

void cJSON_free(void *ptr)
{
    rt_free(ptr);
}

const char * cJSON_item_get_string(cJSON *object, const char *item_name)
{
    cJSON *item;
    const char * string;

    item = cJSON_GetObjectItem(object, item_name);

    if(!item)
        return 0;

    if( (item->type != cJSON_String) && (item->type != cJSON_Array) )
        return 0;

    if(item->type == cJSON_Array)
        return item->child->valuestring; // TODO

    return item->valuestring;
}

int cJSON_item_get_number(cJSON *object, const char *item_name, int * result)
{
    cJSON *item;
    const char * string;

    item = cJSON_GetObjectItem(object, item_name);

    if(!item)
        return -1;

    if(item->type != cJSON_Number)
        return -1;

    if(result)
        *result = item->valueint;

    return 0;
}

void cJSON_AddInteger2StringToObject(cJSON *object, const char *name, int i)
{
    char str_buf[10+2];

    sprintf(str_buf, "%d", i);
    cJSON_AddStringToObject(object, name, str_buf);
}
