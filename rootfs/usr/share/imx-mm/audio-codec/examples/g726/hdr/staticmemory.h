
 /************************************************************************
  * Copyright 2005-2010 by Freescale Semiconductor, Inc.
  * All modifications are confidential and proprietary information
  * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
  ************************************************************************/

extern char LibraryStaticArray[];
extern unsigned int LibraryStaticArrayIndex;
extern volatile unsigned int malloc_temp_size;

#define MALLOC(x) (void *)(&LibraryStaticArray[LibraryStaticArrayIndex]);\
							malloc_temp_size = (x);\
							malloc_temp_size += 0x3;\
							malloc_temp_size &= 0xFFFFFFFC;\
							LibraryStaticArrayIndex += malloc_temp_size;


#define LIBRARYSTATICARRAY_SIZE (5*1024*1024)

