#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <rtthread.h>

#include "cJSON.h"

int cJSON_hook_init(void)
{
    cJSON_Hooks cJSON_hook;

    cJSON_hook.malloc_fn = (void *(*)(size_t sz))rt_malloc;
    cJSON_hook.free_fn = rt_free;

    cJSON_InitHooks(&cJSON_hook);

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(cJSON_hook_init);
