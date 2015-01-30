/*********************************************************************************
 * Copyright (C) 2012 Sybase, Inc.                                               *
 * All rights reserved. Unpublished rights reserved under U.S. copyright laws.   *
 * This product is the confidential and trade secret information of Sybase, Inc. *
 *********************************************************************************/

// Header file for Windows integer types
#ifdef _WIN32
#ifndef _STDINT_H
#define _STDINT_H 1
#include <sys/types.h>
typedef signed char int8_t;
typedef short int   int16_t;
typedef int         int32_t;
typedef __int64     int64_t;
#ifndef uint8_t
typedef unsigned char       uint8_t;
#endif
typedef unsigned short int  uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

typedef _off_t off64_t;
typedef int pid_t;
typedef uint32_t mode_t;

#define ESPPRId64 "I64d"
#endif //_STDINT_H
#else
#ifndef WIN_STDINT_HPP
#define WIN_STDINT_HPP
#if __WORDSIZE == 64
#define ESPPRId64 "ld"
#else
#define ESPPRId64 "lld"
#endif
#endif //WIN_STDINT_HPP
#endif //_WIN32


