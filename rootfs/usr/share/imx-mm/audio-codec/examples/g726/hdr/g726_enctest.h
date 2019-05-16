
/*****************************************************************************
 *
 * Motorola Inc.
 * (c) Copyright 2004 Motorola, Inc.
 * ALL RIGHTS RESERVED.
 *
 *****************************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc.
 *****************************************************************************
 *
 * File Name: g726_enctest.h
 *
 * Description: This is a header file for g726_enctest.c
 *
 ****************************** Change History********************************
 *
 *   DD/MMM/YYYY     Code Ver     Description                   Author
 *   -----------     --------     -----------                   ------
 *   15/Mar/2005     0.1          File created                  Tommy Tang
 *****************************************************************************/
#ifndef G726_ENCTEST_H
#define G726_ENCTEST_H
/******************************************************************************/
#ifdef	OS_VRTX
#include "staticmemory.h"
#define alloc_fast(A)        MALLOC(A)
#define alloc_slow(A)        MALLOC(A)
#define mem_free(A)
#else
#define alloc_fast(A)        malloc(A)
#define alloc_slow(A)        malloc(A)
#define mem_free(A)          free(A)
#endif /* _OS_VRTX */
/************************************ END OF FILE *****************************/
#endif
