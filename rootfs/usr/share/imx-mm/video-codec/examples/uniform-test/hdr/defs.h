/*
 ***********************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */
#ifndef __DEFS_H__
#define __DEFS_H__

#if defined(TGT_OS_WINCE) || defined(TGT_OS_WIN32)
#include <windows.h>
#endif

#define APP_DEBUG_PRINTF printf
#ifndef NULL
#define NULL  (void*)0
#endif
/* data type definition */
typedef void            Void;
typedef long            I32;
typedef unsigned long   U32;
typedef short           I16;
typedef unsigned short  U16;
typedef char            I8;
typedef unsigned char   U8;
typedef unsigned char * String;
typedef float           Float;
typedef double          Double;
typedef int             Bool;
typedef unsigned long   Time;

#define NAME_SIZE 128 // maximum function name size


#define ASSERT(x)
#define av_malloc malloc
#define av_free free
#define PRINT_ERROR printf

#endif /* __DEFS_H__ */
