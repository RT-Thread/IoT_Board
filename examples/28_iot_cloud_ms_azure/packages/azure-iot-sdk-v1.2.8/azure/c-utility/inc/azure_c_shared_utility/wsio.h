// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef WSIO_H
#define WSIO_H

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#include <stdbool.h>
#endif /* __cplusplus */

typedef struct WSIO_CONFIG_TAG
{
    const IO_INTERFACE_DESCRIPTION* underlying_io_interface;
    void* underlying_io_parameters;
    const char* hostname;
    int port;
    const char* resource_name;
    const char* protocol;
} WSIO_CONFIG;

MOCKABLE_FUNCTION(, const IO_INTERFACE_DESCRIPTION*, wsio_get_interface_description);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WSIO_H */
