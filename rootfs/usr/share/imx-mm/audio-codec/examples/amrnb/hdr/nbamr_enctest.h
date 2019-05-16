
/*******************************************************************************
*
* Motorola Inc.
* (c) Copyright 2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
********************************************************************************
***********************************************************************
* Copyright (c) 2005-2010, 2012, Freescale Semiconductor, Inc.
* All modifications are confidential and proprietary information
* of Freescale Semiconductor, Inc.
***********************************************************************
*
* File Name: nbamr_enctest.h
*
* Description: This is a header file for nbamr_dectest.c.
*
****************************** Change History***********************************
*
*   DD/MMM/YYYY     Code Ver     Description						Author
*   -----------     --------     -----------						------
*   20/May/2004     0.1          File created						Ashok
*   03/Jun/2004     1.0          Review rework						Ashok
*   22/Jul/2004     1.1          Clened up code						Ashok
*   13/12/2006      1.2          Added profiling code for RVDS      Shyam Krishnan M
*******************************************************************************/

#ifndef NBAMR_ENCTEST_H
#define NB_AMR_ENCTEST_H

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
                                /* NONE */
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
/* encoder exit function called to free all dynamically allocated memory */
eAMREReturnType eAMREEncodeExit(
            sAMREEncoderConfigType *psEncConfig,
            NBAMR_S8 s8UseModeFile,
            NBAMR_S16 *ps16InBuf,
            NBAMR_S16 *ps16OutBuf
            );

/* function prototype of read speech sample function, called to read PCM
 * samples from file
 */
#ifndef OS_VRTX
NBAMR_S16 s16AMREReadSpeechSample (
           FILE *pfFileSpeech,
           NBAMR_S16 *ps16InBuf,
           NBAMR_S8 s8UseModeFile,
           FILE * pfFileModes,
           NBAMR_S8 **pps8ModeStr,
           NBAMR_U8 u8NumFrame
           );
#else
NBAMR_S16 s16AMREReadSpeechSample (
           NBAMR_S16 *ps16InBuf,
           NBAMR_S8 s8UseModeFile,
           NBAMR_S8 **pps8ModeStr,
           NBAMR_U8 u8NumFrame
           );
#endif

#ifndef OS_VRTX
/* function used by application to process command line fnctions */
eAMREReturnType eAMREProcessCmdLineOptions (
           NBAMR_S32 s32Argc,
           NBAMR_S8 *ps8Argv[],
           NBAMR_S16 *ps16DtxFlag,
           NBAMR_S8  **pps8ModeStr,
           NBAMR_S8 *ps8UseModeFile,
           NBAMR_U8 *pu8BitStreamFormat,
           NBAMR_U8 *pu8NumFrame,
           FILE **ppfFileSpeech,
           FILE **ppfFileSerial,
           FILE **ppfFileModes
           );
#endif

NBAMR_Void vDisplayUsage(NBAMR_S8 *ps8ProgName);

#ifdef CYCLE_MEASUREMENT
//extern NBAMR_S32 mystrcmp(const char* pcStr1, const char* pcStr2);
#endif

#if defined(TIME_PROFILE)
#define MODE_TABLE_SIZE             10
enum Mode { MR475 = 0,
            MR515,            
            MR59,
            MR67,
            MR74,
            MR795,
            MR102,
            MR122,            
            MRDTX,
            N_MODES     /* number of (SPC) modes */
          };
typedef struct
{
    char *name; /* name string */
    int   id;   /* integer id  */
}sConvTableType;
#endif

#ifdef	TIME_PROFILE_RVDS
extern NBAMR_S32 mystrcmp(const char* pcStr1, const char* pcStr2);

extern NBAMR_S32 mystrcmp_test(const char* pcStr1, const char* pcStr2);
extern NBAMR_S32 mystrncmp(const char* pcStr1, const char* pcStr2, NBAMR_U8 u8NumBytes);
#endif

/**************************<FUNCTION_PROTOTYPES END>***************************/
#endif /* end of NBAMR_DECTEST_H header file */
/**************************<END OF THE FILE>***********************************/
