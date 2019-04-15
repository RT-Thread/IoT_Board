// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_c_shared_utility/gballoc.h"
#include <stdint.h>
#include <time.h>

#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"

#include <rtthread.h>
#include <sys/time.h>

typedef struct TICK_COUNTER_INSTANCE_TAG
{
    uint32_t original_tick_count;
} TICK_COUNTER_INSTANCE;

TICK_COUNTER_HANDLE tickcounter_create(void)
{
    /* Codes_SRS_TICKCOUNTER_RT-THREAD_30_003: [ `tickcounter_create` shall allocate and initialize an internally-defined TICK_COUNTER_INSTANCE structure and return its pointer on success.] */
    TICK_COUNTER_INSTANCE* result = (TICK_COUNTER_INSTANCE*)malloc(sizeof(TICK_COUNTER_INSTANCE));
    if (result == NULL)
    {
        LogError("Failed creating tick counter");
    }
    else
    {
        result->original_tick_count = rt_tick_get();
    }
    return result;
}

void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
    /* Codes_SRS_TICKCOUNTER_RT-THREAD_30_006: [ If the tick_counter parameter is NULL, tickcounter_destroy shall do nothing. ] */
    if (tick_counter != NULL)
    {
        /* Codes_SRS_TICKCOUNTER_RT-THREAD_30_005: [ tickcounter_destroy shall delete the internally-defined TICK_COUNTER_INSTANCE structure specified by the tick_counter parameter. (This call has no failure case.) ] */
        free(tick_counter);
    }
}

int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t * current_ms)
{
    int result;

    if (tick_counter == NULL || current_ms == NULL)
    {
        /* Codes_SRS_TICKCOUNTER_RT-THREAD_30_007: [ If the tick_counter parameter is NULL, tickcounter_get_current_ms shall return a non-zero value to indicate error. ] */
        /* Codes_SRS_TICKCOUNTER_RT-THREAD_30_008: [ If the current_ms parameter is NULL, tickcounter_get_current_ms shall return a non-zero value to indicate error. ] */
        LogError("Invalid Arguments.");
        result = __FAILURE__;
    }
    else
    {
        *current_ms = (tickcounter_ms_t)(
            // Subtraction of two uint32_t's followed by a cast to uint32_t
            // ensures that the result remains valid until the real difference exceeds 32 bits.
            ((uint32_t)(rt_tick_get() - tick_counter->original_tick_count))
            // Now that overflow behavior is ensured it is safe to scale. RT_TICK_PER_SECOND is typically
            // equal to 1000 or less, so overflow won't happen until the 49.7 day limit
            // of this call's effective uint32_t return value.
            * 1000.0 / RT_TICK_PER_SECOND
            );
        result = 0;
    }

    return result;
}
