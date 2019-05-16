/*
 ***********************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#define _DEBUG

#include "stdio.h"
#include "stdlib.h"

/* macro of assertion */
#ifdef _DEBUG
#define ASSERT(f) \
do \
{ \
    if(f) \
    { \
        NULL; \
    } \
    else \
    { \
        fflush(stdout); \
        fprintf(stderr, "\nAssertion failed: %s, line %u\n",__FILE__, __LINE__); \
        fflush(stderr); \
        exit(EXIT_FAILURE); \
    } \
}while(0)
#else
#define ASSERT(f) NULL
#endif

#endif
