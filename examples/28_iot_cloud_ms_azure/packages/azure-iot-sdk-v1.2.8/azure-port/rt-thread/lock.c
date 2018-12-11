// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/lock.h"
#include <rtthread.h>
#include <stdlib.h>
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/macro_utils.h"

LOCK_HANDLE Lock_Init(void)
{
    /* Codes_SRS_LOCK_10_002: [Lock_Init on success shall return a valid lock handle which should be a non NULL value] */
    /* Codes_SRS_LOCK_10_003: [Lock_Init on error shall return NULL ] */
    rt_mutex_t result = rt_mutex_create("azure_mutex", RT_IPC_FLAG_FIFO);
    if (result == RT_NULL)
    {
        LogError("CreateSemaphore failed.");
    }

    return (LOCK_HANDLE)result;
}

LOCK_RESULT Lock_Deinit(LOCK_HANDLE handle)
{
    LOCK_RESULT result;
    if (handle == NULL)
    {
        /* Codes_SRS_LOCK_10_007: [Lock_Deinit on NULL handle passed returns LOCK_ERROR] */
        LogError("Invalid argument; handle is NULL.");
        result = LOCK_ERROR;
    }
    else
    {
        /* Codes_SRS_LOCK_10_012: [Lock_Deinit frees the memory pointed by handle] */
        rt_mutex_delete(handle);
        result = LOCK_OK;
    }

    return result;
}

LOCK_RESULT Lock(LOCK_HANDLE handle)
{
    rt_err_t result;
    if (handle == NULL)
    {
        /* Codes_SRS_LOCK_10_007: [Lock on NULL handle passed returns LOCK_ERROR] */
        LogError("Invalid argument; handle is NULL.");
        result = LOCK_ERROR;
    }
    else 
    {
        result = rt_mutex_take((rt_mutex_t)handle, RT_WAITING_FOREVER);

        if (result != RT_EOK)
        {
            LogError("WaitForSingleObject returned an invalid value.");
            rt_mutex_delete(handle);
            result = LOCK_ERROR;
        }
    }

    return (LOCK_RESULT)result;
}

LOCK_RESULT Unlock(LOCK_HANDLE handle)
{
    LOCK_RESULT result;
    if (handle == NULL)
    {
        /* Codes_SRS_LOCK_10_007: [Unlock on NULL handle passed returns LOCK_ERROR] */
        LogError("Invalid argument; handle is NULL.");
        result = LOCK_ERROR;
    }
    else
    {
        if (rt_mutex_release(handle) == RT_EOK)
        {
            /* Codes_SRS_LOCK_10_009: [Unlock on success shall return LOCK_OK] */
            result = LOCK_OK;
        }
        else
        {
            /* Codes_SRS_LOCK_10_010: [Unlock on error shall return LOCK_ERROR] */
            LogError("ReleaseSemaphore failed !");
            result = LOCK_ERROR;
        }
    }
    
    return result;
}
