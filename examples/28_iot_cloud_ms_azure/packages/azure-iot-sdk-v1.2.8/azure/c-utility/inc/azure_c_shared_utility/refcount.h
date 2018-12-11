// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


/*this header contains macros for ref_counting a variable. 

There are no upper bound checks related to uint32_t overflow because we expect that bigger issues are in
the system when more than 4 billion references exist to the same variable. In the case when such an overflow
occurs, the object's ref count will reach zero (while still having 0xFFFFFFFF references) and likely the
controlling code will take the decision to free the object's resources. Then, any of the 0xFFFFFFFF references
will interact with deallocated memory / resources resulting in an undefined behavior.
*/

#ifndef REFCOUNT_H
#define REFCOUNT_H

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/macro_utils.h"

#ifdef __cplusplus
#include <cstdlib>
#include <cstdint>
extern "C" 
{
#else
#include <stdlib.h>
#include <stdint.h>
#endif

// Include the platform-specific file that defines atomic functionality
#include "refcount_os.h"

#define REFCOUNT_TYPE(type) \
struct C2(C2(REFCOUNT_, type), _TAG)

#define REFCOUNT_SHORT_TYPE(type) \
C2(REFCOUNT_, type)

#define REFCOUNT_TYPE_DECLARE_CREATE(type) C2(REFCOUNT_SHORT_TYPE(type), _Create)
#define REFCOUNT_TYPE_CREATE(type) C2(REFCOUNT_SHORT_TYPE(type), _Create)()

/*this introduces a new refcount'd type based on another type */
/*and an initializer for that new type that also sets the ref count to 1. The type must not have a flexible array*/
/*the newly allocated memory shall be free'd by free()*/
/*and the ref counting is handled internally by the type in the _Create/ _Clone /_Destroy functions */

#define DEFINE_REFCOUNT_TYPE(type)                                                                   \
REFCOUNT_TYPE(type)                                                                                  \
{                                                                                                    \
    type counted;                                                                                    \
    COUNT_TYPE count;                                                                                \
};                                                                                                   \
static type* REFCOUNT_TYPE_DECLARE_CREATE(type) (void)                                               \
{                                                                                                    \
    REFCOUNT_TYPE(type)* result = (REFCOUNT_TYPE(type)*)malloc(sizeof(REFCOUNT_TYPE(type)));         \
    if (result != NULL)                                                                              \
    {                                                                                                \
        INIT_REF(type, result);                                                                      \
    }                                                                                                \
    return (type*)result;                                                                            \
}                                                                                                    \

#ifndef DEC_RETURN_ZERO
#error refcount_os.h does not define DEC_RETURN_ZERO
#endif // !DEC_RETURN_ZERO
#ifndef INC_REF
#error refcount_os.h does not define INC_REF
#endif // !INC_REF
#ifndef DEC_REF
#error refcount_os.h does not define DEC_REF
#endif // !DEC_REF
#ifndef INIT_REF
#error refcount_os.h does not define INIT_REF
#endif // !INIT_REF

#ifdef __cplusplus
}
#endif

#endif /*REFCOUNT_H*/


