// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef CRT_ABSTRACTIONS_H
#define CRT_ABSTRACTIONS_H

#include "azure_c_shared_utility/umock_c_prod.h"

#ifdef __cplusplus
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cmath>
extern "C" {
#else // __cplusplus
#include <stdio.h>
#include <string.h>
#include <errno.h>
#endif // __cplusplus

#ifdef _MSC_VER

#ifdef QUARKGALILEO
#define HAS_STDBOOL
#ifdef __cplusplus
typedef bool _Bool;
#else
/*galileo apparently has _Bool and bool as built in types*/
#endif
#endif // QUARKGALILEO

#ifndef _WIN32_WCE
#define HAS_STDBOOL
#ifdef __cplusplus
/*because C++ doesn't do anything about _Bool... */
#define _Bool bool
#else // __cplusplus
#include <stdbool.h>
#endif // __cplusplus
#else // _WIN32_WCE
/* WINCE does not support bool as C datatype */
#define __bool_true_false_are_defined	1

#define HAS_STDBOOL

#define _Bool bool

#ifdef __cplusplus
#define _CSTDBOOL_
#else // __cplusplus
typedef unsigned char bool;

#define false   0
#define true    1
#endif // __cplusplus
#endif // _WIN32_WCE

#else //  _MSC_VER

#if defined __STDC_VERSION__
#if ((__STDC_VERSION__  == 199901L) || (__STDC_VERSION__ == 201000L) || (__STDC_VERSION__ == 201112L))
/*C99 compiler or C11*/
#define HAS_STDBOOL
#include <stdbool.h>
#endif //  ((__STDC_VERSION__  == 199901L) || (__STDC_VERSION__ == 201000L) || (__STDC_VERSION__ == 201112L))
#endif // __STDC_VERSION__
#endif //  _MSC_VER

#ifndef HAS_STDBOOL
#ifdef __cplusplus
#define _Bool bool
#else // __cplusplus
typedef unsigned char _Bool;
typedef unsigned char bool;
#define false 0
#define true 1
#endif // __cplusplus
#endif // HAS_STDBOOL


/* Codes_SRS_CRT_ABSTRACTIONS_99_001:[The module shall not redefine the secure functions implemented by Microsoft CRT.] */
/* Codes_SRS_CRT_ABSTRACTIONS_99_040 : [The module shall still compile when building on a Microsoft platform.] */
/* Codes_SRS_CRT_ABSTRACTIONS_99_002: [CRTAbstractions module shall expose the following API]*/
#ifdef _MSC_VER
#else // _MSC_VER

/* Adding definitions from errno.h & crtdefs.h */
#if !defined (_TRUNCATE)
#define _TRUNCATE ((size_t)-1)
#endif  /* !defined (_TRUNCATE) */

#if !defined STRUNCATE
#define STRUNCATE       80
#endif  /* !defined (STRUNCATE) */

extern int strcpy_s(char* dst, size_t dstSizeInBytes, const char* src);
extern int strcat_s(char* dst, size_t dstSizeInBytes, const char* src);
extern int strncpy_s(char* dst, size_t dstSizeInBytes, const char* src, size_t maxCount);
extern int sprintf_s(char* dst, size_t dstSizeInBytes, const char* format, ...);
#endif // _MSC_VER

extern unsigned long long strtoull_s(const char* nptr, char** endPtr, int base);
extern float strtof_s(const char* nptr, char** endPtr);
extern long double strtold_s(const char* nptr, char** endPtr);

#ifdef _MSC_VER
#define stricmp _stricmp
#endif // _MSC_VER

MOCKABLE_FUNCTION(, int, mallocAndStrcpy_s, char**, destination, const char*, source);
MOCKABLE_FUNCTION(, int, unsignedIntToString, char*, destination, size_t, destinationSize, unsigned int, value);
MOCKABLE_FUNCTION(, int, size_tToString, char*, destination, size_t, destinationSize, size_t, value);

/*following logic shall define the TOUPPER and ISDIGIT, we do that because the SDK is not happy with some Arduino implementation of it.*/
#define TOUPPER(c)      ((((c)>='a') && ((c)<='z'))?(c)-'a'+'A':c)
#define ISDIGIT(c)      ((((c)>='0') && ((c)<='9'))?1:0)

/*following logic shall define the ISNAN macro*/
/*if runing on Microsoft Visual C compiler, than ISNAN shall be _isnan*/
/*else if running on C99 or C11, ISNAN shall be isnan*/
/*else if running on C89 ... #error and inform user*/

#ifdef _MSC_VER
#define ISNAN _isnan
#else // _MSC_VER
#if defined __STDC_VERSION__
#if ((__STDC_VERSION__  == 199901L) || (__STDC_VERSION__ == 201000L) || (__STDC_VERSION__ == 201112L))
/*C99 compiler or C11*/
#define ISNAN isnan
#else //  ((__STDC_VERSION__  == 199901L) || (__STDC_VERSION__ == 201000L) || (__STDC_VERSION__ == 201112L))
#error update this file to contain the latest C standard.
#endif // ((__STDC_VERSION__  == 199901L) || (__STDC_VERSION__ == 201000L) || (__STDC_VERSION__ == 201112L))
#else // __STDC_VERSION__
#ifdef __cplusplus
/*C++ defines isnan... in C11*/
extern "C++" {
#define ISNAN std::isnan
}
#else // __cplusplus
#error unknown (or C89) compiler, provide ISNAN with the same meaning as isnan in C99 standard  
#endif // __cplusplus

#endif // __STDC_VERSION__
#endif // _MSC_VER

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* CRT_ABSTRACTIONS_H */
