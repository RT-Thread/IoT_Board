// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/xlogging.h"

#include "azure_c_shared_utility/tlsio_mbedtls.h"

#include "ntp.h"
#include "rtthread.h"

#include <stdlib.h>
#include <unistd.h>

int platform_init(void)
{
    time_t result = ntp_sync_to_rtc(NETUTILS_NTP_HOSTNAME);

    if (result > 0)
    {
        rt_kprintf("ntp init successful.\n");
        return RT_EOK;
    }

    return -RT_ERROR;
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return tlsio_mbedtls_get_interface_description();
}

STRING_HANDLE platform_get_platform_info(void)
{
    return STRING_construct("(native; rt-thread; undefined)");
}

void platform_deinit(void)
{
    return;
}
