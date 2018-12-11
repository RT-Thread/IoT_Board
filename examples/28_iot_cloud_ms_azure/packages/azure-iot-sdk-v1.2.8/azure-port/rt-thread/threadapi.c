// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <rtthread.h>
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/xlogging.h"

DEFINE_ENUM_STRINGS(THREADAPI_RESULT, THREADAPI_RESULT_VALUES);

typedef struct thread_arg_tag
{
    char *thread_name;
    int stack_size;
    int thread_priority;
    int thread_time_tiumeslice;
} thread_arg_tag_t;

THREADAPI_RESULT ThreadAPI_Create(THREAD_HANDLE* threadHandle, THREAD_START_FUNC func, void* arg)
{
    THREADAPI_RESULT result;
    if ((threadHandle == NULL) ||
        (func == NULL))
    {
        result = THREADAPI_INVALID_ARG;
        LogError("(result = %s)", ENUM_TO_STRING(THREADAPI_RESULT, result));
    }
    else
    {
        thread_arg_tag_t *thread_arg = arg;
        *threadHandle = rt_thread_create(thread_arg->thread_name ,
                                         (void (*)(void *))func, (void*)1, 
                                         thread_arg->stack_size, 
                                         thread_arg->thread_priority, 
                                         thread_arg->thread_time_tiumeslice);

        if(*threadHandle == NULL)
        {
            result = THREADAPI_ERROR;
            LogError("(result = %s)", ENUM_TO_STRING(THREADAPI_RESULT, result));
        }
        else
        {
            result = THREADAPI_OK;
        }
    }

    return result;
}

THREADAPI_RESULT ThreadAPI_Join(THREAD_HANDLE threadHandle, int *res)
{
    return RT_EOK;
}

void ThreadAPI_Exit(int res)
{
    rt_thread_exit();
}

void ThreadAPI_Sleep(unsigned int milliseconds)
{
    rt_tick_t tick_count = rt_tick_from_millisecond(milliseconds);
    rt_thread_delay(tick_count);
}
