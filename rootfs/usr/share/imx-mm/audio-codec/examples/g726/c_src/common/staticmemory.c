/*
 ***********************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */


#include "staticmemory.h"

unsigned int LibraryStaticArrayIndex = 0;
volatile unsigned int malloc_temp_size = 0;

#ifdef OS_VRTX
#pragma arm section zidata = "mallocSection"
#endif
char LibraryStaticArray[LIBRARYSTATICARRAY_SIZE];


