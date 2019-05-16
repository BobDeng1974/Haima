
/*******************************************************************************
*
* Motorola Inc.
* (c) Copyright 2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
********************************************************************************
*
* File Name: nbamr_dectest.h
*
* Description: This is a header file for nbamr_dectest.c.
*
***********************************************************************
* Copyright 2005-2010 by Freescale Semiconductor, Inc.
* All modifications are confidential and proprietary information
* of Freescale Semiconductor, Inc.
***********************************************************************
****************************** Change History***********************************
*
*   DD/MMM/YYYY     Code Ver     Description						Author
*   -----------     --------     -----------						------
*   20/May/2004     0.1          File created						 Ashok
*   03/Jun/2004     1.0          Review rework						 Ashok
*   22/Jul/2004     1.1          Clened up code						 Ashok
*   13/12/2006      1.2			 Added profiling code for RVDS       Shyam Krishnan M
*******************************************************************************/

#ifndef NBAMR_DECTEST_H
#define NBAMR_DECTEST_H

/*****************************<INCLUDE_FILES BEGIN>****************************/
#ifndef OS_VRTX
#include <stdio.h>
#endif
/*******************************<INCLUDE_FILES END>****************************/

/******************************<DEFINES BEGIN>*********************************/
#ifndef OS_VRTX
#define     alloc_fast(A)   malloc(A)
#define     alloc_slow(A)   malloc(A)
#define     mem_free(A)     free(A)
#endif
/******************************<DEFINES END>***********************************/

/******************************<ENUMS BEGIN>***********************************/
                                /** NONE **/
/******************************<ENUMS END>*************************************/

/****************************<STRUCTURES/UNIONS BEGIN>*************************/
                                /* NONE */
/****************************<STRUCTURES/UNIONS END>***************************/

/***************************<GLOBAL_VARIABLES BEGIN>***************************/
                                   /* None */
/***************************<GLOBAL_VARIABLES END>*****************************/
                                   /* None */
/**************************<STATIC_VARIABLES BEGIN>****************************/
                                    /* None */
/**************************<STATIC_VARIABLES END>******************************/

/**************************<FUNCTION_PROTOTYPES BEGIN>*************************/
/* decoder exit function */
eAMRDReturnType eAMRDDecodeExit (sAMRDDecoderConfigType *psDecConfig,
                                NBAMR_S16 *ps16InBuf, NBAMR_S16 *ps16OutBuf);

/* function used by app to read serial encoded samples */
#ifndef OS_VRTX
NBAMR_S16 s16AMRDReadSerialData (NBAMR_S16 *ps16InBuf, FILE *pfFileSerail,
                                NBAMR_U16 u16DataSize);
#else
NBAMR_S16 s16AMRDReadSerialData (NBAMR_S16 *ps16InBuf, NBAMR_U16 u16DataSize);
#endif

#ifndef OS_VRTX
NBAMR_S16 s16AMRDReadPackedData (NBAMR_U8 *pu8InBuf, FILE *pfFileSerial,
                                                        NBAMR_U16 u16DataSize);


/* function called to process command line function */
eAMRDReturnType eAMRDProcessCmdLineOptions(
            NBAMR_S32 s32Argc, NBAMR_S8 *ps8Argv[],
            NBAMR_U8 *pu8BitStremFormat, NBAMR_U8 *pu8NumFrame,
            FILE **ppfFileSerial, FILE **ppfFileSyn
            );

#endif

NBAMR_Void vDisplayUsage(NBAMR_S8 *ps8ProgName);

#ifdef CYCLE_MEASUREMENT
extern int mystrcmp(const char* pcStr1, const char* pcStr2);
#endif

#ifdef	TIME_PROFILE_RVDS
extern NBAMR_S32 mystrcmp_test(const char* pcStr1, const char* pcStr2);
extern NBAMR_S32 mystrncmp(const char* pcStr1, const char* pcStr2, NBAMR_U8 u8NumBytes);
#endif
/**************************<FUNCTION_PROTOTYPES END>***************************/

#endif /* end of NBAMR_DECTEST_H header file */
/**************************<END OF THE FILE>***********************************/
