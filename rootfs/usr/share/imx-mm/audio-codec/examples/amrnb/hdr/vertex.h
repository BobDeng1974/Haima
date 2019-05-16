
 /************************************************************************
  * Copyright 2005-2010 by Freescale Semiconductor, Inc.
  * All modifications are confidential and proprietary information
  * of Freescale Semiconductor, Inc.
  ************************************************************************/

#ifndef _VERTEX_H_

#ifdef OS_VRTX
//extern unsigned char bAmrNbDecArray[];    //Decoder
extern unsigned char bAmrNbEncArray[];
extern unsigned int pCounter;
//#define MALLOC(x) &bAmrNbDecArray[pCounter]; \
					pCounter += (x);            //decoder

extern volatile unsigned int malloc_temp_size;

#define MALLOC(x) (void *)(&bAmrNbEncArray[pCounter]);\
       malloc_temp_size = (x);\
       malloc_temp_size += 0x3;\
       malloc_temp_size &= 0xFFFFFFFC;\
       pCounter += malloc_temp_size;



//#define MALLOC(x) (void *)&bAmrNbEncArray[pCounter]; \
//					pCounter += (x);			//encoder

#define mem_free(x)
#else
#define MALLOC(x) malloc(x)
#endif

#endif


