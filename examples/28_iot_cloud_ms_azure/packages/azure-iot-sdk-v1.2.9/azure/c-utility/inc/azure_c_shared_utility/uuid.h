// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UUID_H
#define UUID_H

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
extern "C" {
#else
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#endif /* __cplusplus */

#include "azure_c_shared_utility/umock_c_prod.h"

typedef unsigned char UUID[16];

/* @brief               Generates a true UUID
*  @param uuid          A pre-allocated buffer for the bytes of the generated UUID
*  @returns             Zero if no failures occur, non-zero otherwise.
*/
MOCKABLE_FUNCTION(, int, UUID_generate, UUID*, uuid);

/* @brief               Gets the UUID value (byte sequence) of an well-formed UUID string.
*  @param uuid_string   A null-terminated well-formed UUID string (e.g., "7f907d75-5e13-44cf-a1a3-19a01a2b4528").
*  @param uuid          Sequence of bytes representing an UUID.
*  @returns             Zero if no failures occur, non-zero otherwise.
*/
MOCKABLE_FUNCTION(, int, UUID_from_string, const char*, uuid_string, UUID*, uuid);

/* @brief               Gets the string representation of the UUID value.
*  @param uuid          Sequence of bytes representing an UUID.
*  @returns             A null-terminated string representation of the UUID value provided (e.g., "7f907d75-5e13-44cf-a1a3-19a01a2b4528").
*/
MOCKABLE_FUNCTION(, char*, UUID_to_string, UUID*, uuid);

#ifdef __cplusplus
}
#endif

#endif /* UUID_H */
