
/*******************************************************************************
*
* Motorola Inc.
* (c) Copyright 2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
***********************************************************************
* Copyright (c) 2005-2010, 2012, Freescale Semiconductor, Inc.
* All modifications are confidential and proprietary information
* of Freescale Semiconductor, Inc.
********************************************************************************
* File Name: nbamr_dectest.c
*
* Description: NB-AMR decoder test wrapper is defined in this file.
*
* Functions Included: - main
*                     - s16ReadSerialData
*                     - s16ReadPackedData
*                     - eAMRDProcessCmdLineOptions
*                     - vDisplayUsage
****************************** Change History***********************************
*   DD/MMM/YYYY     Code Ver     Description                            Author
*   -----------     --------     -----------                            ------
*   20/May/2004     0.1          File created                           Ashok
*   03/Jun/2004     1.0          Review rework                          Ashok
*   22/Jul/2004     1.1          Clened up code                         Ashok
*   10/Aug/2004     1.2          Remove RVDS                        Dale Tian
*                                compiling
*                                warning
*   23/Sep/2004     1.3          Update type name                   Tommy Tang
*
*   19/Nov/2004     1.4          Byte swap added for NBAMR_BIG_ENDIAN   Ashok
*                                testing
*   10/Jan/2005     1.5          Cleaned up and added arg               Ashok
*                                processing to test runtime config
*                                of VAD and file format
*   13/12/2006      1.6          Added profiling code for RVDS      Shyam Krishnan M
*   01/Jul/2008     1.7          Added version API                     Tao Jun
*   03/Jul/2008     1.8          Added A9 MCPS test code               Tao Jun
*   08/Oct/2008     1.9          Added WinCE build                     Tao Jun
*   10/Oct/2008     1.10         Added WinCE MCPS test                 Tao Jun
*
*******************************************************************************/
#if  WINCE_BUILD==1
#else
#ifdef ARM_OPTS
/* section definition */
#pragma arm section rodata="APPD_TestApp_RO"
#pragma arm section rwdata="APPD_TestApp_RW"
#pragma arm section zidata="APPD_TestApp_ZI"
#endif
#endif
/*****************************<INCLUDE_FILES BEGIN>****************************/
#include <stdio.h>  /* for declaration of FILE */
#include <string.h>
#include <stdlib.h>
#ifdef TIME_PROFILE
#ifdef __SYMBIAN32__
#include <sys/time.h>
#else //__SYMBIAN32__
#include <time.h>
#endif //__SYMBIAN32__
#endif

#ifdef __WINCE
#include<windows.h>
#endif

#if  WINCE_BUILD==1
#else
#include <errno.h>
#endif

#include "nb_amr_dec_api.h"
#include "nbamr_dectest.h"
#define OUTPUT_DUMP
#if defined TIME_PROFILE || defined TIME_PROFILE_RVDS|| ARM_MIPS_TEST_WINCE
#undef OUTPUT_DUMP
#endif
int temp;
#ifdef MEASURE_STACK_USAGE
#include "mm_codec_common.h"
#endif

#ifdef ALIGNMENT_CHECK
    int cp_ctrl_data = 0;
#endif
#ifdef ARM_MIPS_TEST_WINCE        		 /* for taking timing on wince platform*/
	extern void INTERRUPTS_SET(int k);
	unsigned int	Flag;
	LARGE_INTEGER	lpFrequency;
	LARGE_INTEGER	lpPerformanceCountBegin;
	LARGE_INTEGER	lpPerformanceCountEnd;
	long	Temp;
	FILE	*pfCycleCount;
	long	totalTime = 0;
	unsigned int	numFrame = 0;
        long minFrameTime=0x7FFFFFFF;
	long	maxFrameTime = -1;
	unsigned int	minFrameNumber = 0;
	unsigned int	maxFrameNumber = 0;
	unsigned char	chname[] = "[PROFILE-INFO]";
#endif
/*******************************<INCLUDE_FILES END>****************************/
#ifdef TIME_PROFILE_RVDS
	int uiframe = 0;

        void dummyCall1(void)
{
            uiframe++;
}

void dummyCall2(void)
{
            uiframe++;
}
#endif

/*******************************************************************************
* Function:  main
*
* Description: decoder main function. Function is defined in application space.
*
* Returns: NBAMR_SUCCESS/NBAMR_FAILURE
*
* Arguments: None (In case encoded framne contain RX frame information
*                  rxframetypemode flag together with input and output
*                  test file shall be passed)
*
* Global Variables: None
*
* Range Issues: None
*
* Special Issues: None
*
*******************************************************************************/
NBAMR_S8 *InputFileName = NULL;
int main (NBAMR_S32 s32Argc, NBAMR_S8 *ps8Argv[])
{
    NBAMR_S16  *ps16InBuf = NULL;        /*pointer to input speech samples*/
    NBAMR_S16  *ps16OutBuf = NULL;       /* pointer to output (encoded) buffer */

    sAMRDMemAllocInfoSubType *psMem;   /*pointer to encoder sub-memory */
    sAMRDDecoderConfigType   *psDecConfig; /* Pointer to encoder config
                                                structure */
    eAMRDReturnType  eRetVal;     /* local variable */
    NBAMR_S16  s16NumReqs;
    NBAMR_S16  s16Counter;
  	NBAMR_U32 s32Frame;

    NBAMR_U8  s8MagicNumber[8];
    NBAMR_U8  u8Mode;

    NBAMR_U16 u16DataSize = 0;

    NBAMR_U16 s16NumRead = 0;   /* to read number of data in case of
                                 3GPP format */

    /* size of packed frame for each mode */
    NBAMR_S16 gas16PackedCodedSizeMMS[]=
    {
        13, 14, 16, 18, 20, 21, 27, 32,
        6,  0,  0,  0,  0,  0,  0,  1
    };
    /* size of packed frame for each mode */
    NBAMR_S16 gas16PackedCodedSizeIF1[]=
    {
        15, 16, 18, 20, 22, 23, 29, 34,
        8,  0,  0,  0,  0,  0,  0,  1
    };
    /* size of packed frame for each mode */
    NBAMR_S16 gas16PackedCodedSizeIF2[]=
    {
        13, 14, 16, 18, 19, 21, 26, 31,
        6,  0,  0,  0,  0,  0,  0,  1
    };

    FILE *pfFileSyn = NULL;
    FILE *pfFileSerial = NULL;
    NBAMR_U8 u8BitStreamFormat = NBAMR_ETSI; /* default file format */
    NBAMR_U8 u8NumFrame = 1;    /* Default number of frame */

    NBAMR_S16 *ps16In = NULL;   /* local variable */

    long infilesize=0;

#ifdef MEASURE_STACK_USAGE
    int           *ps32SP, *ps32BaseSP;
    int           s32StackCount, s32StackValue;
    int           s32PeakStack = 0;
#endif

#ifdef MEASURE_HEAP_USAGE
    int      s32TotalMallocBytes=0;
#endif

#if defined(TARGET_ARM11) && defined(TIME_PROFILE_RVDS )
    NBAMR_S32     prev_clk, curr_clk, clk;
    NBAMR_S32     total_clk = 0;
    NBAMR_S32     frame_min_clk = 0x7fffffff;
    NBAMR_S32     frame_max_clk = 0;
    NBAMR_S32     frame_min_no = 0;
    NBAMR_S32     frame_max_no = 0;
	unsigned char chname[] = "[PROFILE-INFO]";
    FILE    *pfCycleCount;
    pfCycleCount = fopen ("nbamr_dec_cycle.txt", "a");
#endif

#ifdef TIME_PROFILE
    FILE    *pfCycleCount;
    struct timeval tv_prof;
    long time_before =0, time_after=0;
    int retVal = -1;
    int minFrameNumber=0, maxFrameNumber=0, numFrame =0;
    long minFrameTime=0x7FFFFFFF;
    long maxFrameTime =0, totalTime =0;
    long timeVal=0;
    unsigned char timeFlag =0;
    unsigned char chname[] = "[PROFILE-INFO]";
#ifdef ARMPLAT
    int platform = ARMPLAT;
#else
    int platform = 12;
#endif
#ifdef CPUFREQ
    int cpu_freq = CPUFREQ;
#else    
    int cpu_freq = 1000;
#endif
    int bit_rate = 8000;
    double performance = 0.0;
#ifdef __SYMBIAN32__
    pfCycleCount = fopen ("D:\\nbamr_dec_cycle.txt", "a");
#else //__SYMBIAN32__
	pfCycleCount = fopen ("../audio_performance.txt", "a");
#endif //__SYMBIAN32__
#endif
#ifdef ARM_MIPS_TEST_WINCE
    FILE    *fSysTime;
    fSysTime = fopen ("nbamr_dec_wince_mcps.txt", "a");
    if(!fSysTime)
    {
		printf("\nUnable to open log file nbamr_enc_wince_mcps.txt\n");
		exit(1);
	}
#endif
    	fprintf(stderr,"Running %s\n",eAMRDVersionInfo());

    eRetVal = eAMRDProcessCmdLineOptions (s32Argc, &ps8Argv[0], &u8BitStreamFormat,
                                          &u8NumFrame, &pfFileSerial, &pfFileSyn);
    if (eRetVal != E_NBAMRD_OK)
    {
        exit(1);
    }

    /* allocate fast memory for encoder config structure */
    psDecConfig=(sAMRDDecoderConfigType *)alloc_fast(sizeof(sAMRDDecoderConfigType));

    if (psDecConfig == NULL)
    {
        return NBAMR_FAILURE;
    }

    psDecConfig->pvAMRDDecodeInfoPtr = NULL;
    psDecConfig->pu8APPDInitializedDataStart = NULL;

    psDecConfig->u8RXFrameType = 0; /* Encoded with frame type set to TXType */

    psDecConfig->u8BitStreamFormat = u8BitStreamFormat;

    psDecConfig->u8NumFrameToDecode = u8NumFrame;

    /* initialize decoder app config base pointers */
	for(s16Counter = 0; s16Counter < NBAMR_MAX_NUM_MEM_REQS; s16Counter++)
	{
        (psDecConfig->sAMRDMemInfo.asMemInfoSub[s16Counter].pvAPPDBasePtr) = NULL;
    }

    /* Find decoder memory requiremet */
    eRetVal = eAMRDQueryMem(psDecConfig);

    if (eRetVal != E_NBAMRD_OK)
    {
        /* de-allocate memory allocated for encoder config */
        mem_free(psDecConfig);
        return NBAMR_FAILURE;
    }
    /* Number of memory chunk requested by the encoder */
   	s16NumReqs = (NBAMR_S16)(psDecConfig->sAMRDMemInfo.s32AMRDNumMemReqs);

    /* allocate memory requested by the encoder*/
	for(s16Counter = 0; s16Counter < s16NumReqs; s16Counter++)
	{
	    psMem = &(psDecConfig->sAMRDMemInfo.asMemInfoSub[s16Counter]);
		if (psMem->s32AMRDMemTypeFs == NBAMR_FAST_MEMORY)
      	{
			psMem->pvAPPDBasePtr = alloc_fast(psMem->s32AMRDSize);
			#ifdef MEASURE_HEAP_USAGE
				s32TotalMallocBytes +=  psMem->s32AMRDSize;
			#endif
            if (psMem->pvAPPDBasePtr == NULL)
            {
                mem_free(psDecConfig);
                return NBAMR_FAILURE;
            }
		}
		else
		{
			psMem->pvAPPDBasePtr = alloc_slow(psMem->s32AMRDSize);
			#ifdef MEASURE_HEAP_USAGE
				s32TotalMallocBytes +=  psMem->s32AMRDSize;
			#endif
            if (psMem->pvAPPDBasePtr == NULL)
            {
                mem_free(psDecConfig);
                return NBAMR_FAILURE;
            }
		}
	}
	/* Call decoder init routine */
	eRetVal = eAMRDDecodeInit (psDecConfig);
	if (eRetVal != E_NBAMRD_OK)
    {
        /* free all the dynamic memory and exit */
        eRetVal = eAMRDDecodeExit (psDecConfig, ps16InBuf, ps16OutBuf);
        return NBAMR_FAILURE;
    }
    /* allocate memory for output buffer (unpacked) */
	if ((ps16OutBuf = alloc_fast ((psDecConfig->u8NumFrameToDecode*L_FRAME) * sizeof (NBAMR_S16))) == NULL)
    {
        /* free all the dynamic memory and exit */
        eRetVal = eAMRDDecodeExit (psDecConfig, ps16InBuf, ps16OutBuf);
        return NBAMR_FAILURE;
    }
    if (u8BitStreamFormat == NBAMR_ETSI)
    {
        if ((ps16InBuf = alloc_fast((psDecConfig->u8NumFrameToDecode*SERIAL_FRAMESIZE)*\
                                        sizeof (NBAMR_S16))) == NULL)
        {
            /* free all the dynamic memory and exit */
            eRetVal = eAMRDDecodeExit (psDecConfig, ps16InBuf, ps16OutBuf);
            return NBAMR_FAILURE;
        }
    }
    else
    {
        /* it is MMS or IF1 or IF2 format */
        if ((ps16InBuf = alloc_fast((psDecConfig->u8NumFrameToDecode*\
           ((NBAMR_MAX_PACKED_SIZE/2)+(NBAMR_MAX_PACKED_SIZE%2)))*sizeof (NBAMR_S16))) == NULL)
        {
            /* free all the dynamic memory and exit */
            eRetVal = eAMRDDecodeExit (psDecConfig, ps16InBuf, ps16OutBuf);
            return NBAMR_FAILURE;
        }
        ps16In = ps16InBuf;
    }
	/*************************************************************
       * Decode speech frame till the end
	 *************************************************************/
  	s32Frame = 0;
    if (psDecConfig->u8BitStreamFormat == NBAMR_ETSI)
    {

        while(((s16NumRead = s16AMRDReadSerialData(ps16InBuf,
             pfFileSerial,
             psDecConfig->u8NumFrameToDecode*\
             SERIAL_FRAMESIZE))/SERIAL_FRAMESIZE) != 0)
        {
            psDecConfig->u8NumFrameToDecode =(NBAMR_U8)(s16NumRead/SERIAL_FRAMESIZE);
            s32Frame=s32Frame+psDecConfig->u8NumFrameToDecode;
            infilesize += s16NumRead;
#ifdef NBAMRD_DEBUG_PRINT
            if ( (s32Frame%50) == 0)
            {
                fprintf (stderr, "\rframe=%d  ", s32Frame);
            }
#endif

#ifdef MEASURE_STACK_USAGE
				PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
#endif
#ifdef TIME_PROFILE_RVDS
           dummyCall1();
#endif
#ifdef	ARM_MIPS_TEST_WINCE						/* for taking timing on wince platform*/
		INTERRUPTS_SET(0xDF);  						/*disable interrupt*/
		Flag=QueryPerformanceFrequency(&lpFrequency);
		Flag=QueryPerformanceCounter(&lpPerformanceCountBegin);
#endif
#if defined(TARGET_ARM11) && defined(TIME_PROFILE_RVDS )
            __asm
            {
                mrc p15, 0, prev_clk, c15, c12, 0;  /* Read the Monitor Control
                                                       Register in variable prev_clk */
                orr prev_clk, prev_clk, 0xf;       /* Configure the Monitor conrol register
                                                      to reset the Cycle Count Register and
                                                      enable divide by 64 counter */
                mcr p15, 0, prev_clk, c15, c12, 0   /* write back to Monitor
                                                       control regsiter */
                    mrc p15, 0, prev_clk, c15, c12, 1;  /* Read Cycle Count Register into
                                                           variable prev_clk */
            }
#endif
#ifdef TIME_PROFILE
            retVal = gettimeofday(&tv_prof, 0);
            if (retVal == 0)
            {
                time_before = tv_prof.tv_sec * 1000000 + tv_prof.tv_usec;
                timeFlag = 1;
            }
#endif

#ifdef ALIGNMENT_CHECK
                __asm
                {
                    mrc p15, 0, cp_ctrl_data, c1, c0, 0;  /* Read the CP15 control 								register value in
                                                             cp_ctrl_data variable */
                    orr cp_ctrl_data, cp_ctrl_data, 0x2;   /* enable strict 									alignment check by
                                                              resetting A bit
                                                              [bit number 										2] of control register */
                    mcr p15, 0, cp_ctrl_data, c1, c0, 0   /* write back to control register */
                        mrc p15, 0, cp_ctrl_data, c1, c0, 0;  /* read control register for verification */
                }
#endif


				/* call encode frame routine */
            eRetVal=eAMRDDecodeFrame (psDecConfig, ps16InBuf, ps16OutBuf);
            if (eRetVal != E_NBAMRD_OK)
            {
                /* free all the dynamic memory and exit */
                eRetVal = eAMRDDecodeExit (psDecConfig, ps16InBuf, ps16OutBuf);
                exit (-1);
            }


#ifdef ALIGNMENT_CHECK
                __asm
                {
                    mrc p15, 0, cp_ctrl_data, c1, c0, 0;  /* Read the control register value */
                    and cp_ctrl_data, cp_ctrl_data, 0xfffffffd;   /* disable alignment chect */
                    mcr p15, 0, cp_ctrl_data, c1, c0, 0   /* write back control register data */
                        mrc p15, 0, cp_ctrl_data, c1, c0, 0;  /* Read control register data for verification */
                }
#endif
#if defined(TARGET_ARM11) && defined(TIME_PROFILE_RVDS )
            __asm
            {
                mrc p15, 0, curr_clk, c15, c12, 1;  /* Read Cycle Count Register in
                                                       variable curr_clk */
            }
            clk = (curr_clk-prev_clk);    /* clk gives the total core cycles per
                                             frame call */
            total_clk += clk;

            if(clk > frame_max_clk)
            {
                frame_max_clk = clk;
                frame_max_no = s32Frame;
            }
            if(clk < frame_min_clk)
            {
                frame_min_clk = clk;
                frame_min_no = s32Frame;
            }
#endif

#ifdef MEASURE_STACK_USAGE
			GET_STACK_USAGE (ps32BaseSP, ps32SP, s32StackCount, s32StackValue);
			if (s32PeakStack < s32StackValue)
				s32PeakStack = s32StackValue;
#endif

#ifdef TIME_PROFILE
            if (timeFlag == 1)
            {
                retVal = gettimeofday(&tv_prof, 0);
                if (retVal == 0)
                {
                    time_after = tv_prof.tv_sec * 1000000 + tv_prof.tv_usec;
                    timeVal = time_after - time_before;

                    numFrame++;
                    if (timeVal > maxFrameTime)
                    {
                        maxFrameTime = timeVal;
                        maxFrameNumber = s32Frame;
                    }
                    else if (timeVal < minFrameTime)
                    {
                        minFrameTime = timeVal;
                        minFrameNumber = s32Frame;
                    }
                    totalTime += timeVal;
                }
                timeFlag = 0;
            }
#endif

#ifdef TIME_PROFILE_RVDS
           dummyCall2();
#endif
#ifdef ARM_MIPS_TEST_WINCE
                Flag=QueryPerformanceCounter(&lpPerformanceCountEnd);
                Temp=(long)(((lpPerformanceCountEnd.QuadPart - lpPerformanceCountBegin.QuadPart)*1000000)/(lpFrequency.QuadPart));/*this is the duration*/
                INTERRUPTS_SET(0x1F) ;  					/*enable interrupt*/
                numFrame++;
                if (Temp > maxFrameTime)
                {
                    maxFrameTime = Temp;
                    maxFrameNumber = numFrame;
                }
                else if (Temp < minFrameTime)
                {
                    minFrameTime = Temp;
                    minFrameNumber = numFrame;
                }
                totalTime += Temp;
                printf("totaltime: %d",totalTime);

#endif
#ifdef NBAMR_BIG_ENDIAN
            {
                NBAMR_S16 s16Index;
                for(s16Index = 0; s16Index <(psDecConfig->u8NumFrameToDecode*L_FRAME) ; s16Index++)
                {
                    ps16OutBuf[s16Index]=(((ps16OutBuf[s16Index]&0xff)<<8)
                            |((ps16OutBuf[s16Index]&0xff00)>>8));
                }
            }
#endif
#ifdef OUTPUT_DUMP
           temp=fwrite (ps16OutBuf, sizeof (NBAMR_S16),
                        (psDecConfig->u8NumFrameToDecode*L_FRAME), pfFileSyn);
                        /* write synthesized speech to file */
            if ( temp != (psDecConfig->u8NumFrameToDecode*L_FRAME))
            {
            #if  WINCE_BUILD==1
		        fprintf(stderr, "\nerror writing output file: \n");
            #else
                        fprintf(stderr, "\nerror writing output file: %s\n", strerror(errno));
            #endif
            }
           fflush(pfFileSyn);
#endif
        }
    }
    else    /* MMS or IF1 or IF2 format */
    {
        if (psDecConfig->u8BitStreamFormat == NBAMR_MMSIO)
        {
            s16AMRDReadPackedData((NBAMR_U8 *)s8MagicNumber, pfFileSerial,
                                                    (NBAMR_U16)(strlen(NBAMR_MAGIC_NUMBER)));
            if (strncmp((const char *)s8MagicNumber, NBAMR_MAGIC_NUMBER,
                        strlen(NBAMR_MAGIC_NUMBER)))
            {
                fprintf(stderr, "Invalid magic number: %s\n", s8MagicNumber);
                eRetVal = eAMRDDecodeExit (psDecConfig, ps16InBuf, ps16OutBuf);
                return E_NBAMRD_INVALID_BITSTREAM;
            }
        }
        while(!feof(pfFileSerial))
        {
        //int temp;
            for (s16Counter=0; s16Counter<psDecConfig->u8NumFrameToDecode;
                        s16Counter++)
            {
                ps16InBuf = ps16In + (s16Counter*((NBAMR_MAX_PACKED_SIZE/2)
                                                  +(NBAMR_MAX_PACKED_SIZE%2)));
                if ((s16AMRDReadPackedData((NBAMR_U8 *)ps16InBuf, pfFileSerial,
                                           1)) == -1)
                {
#ifdef NBAMRD_DEBUG_PRINT
                    fprintf(stderr, "End of file reached\n");
#endif
                    break;
                }
                if (psDecConfig->u8BitStreamFormat == NBAMR_MMSIO)
                {
                    u8Mode = (NBAMR_U8) (0x0F & (*(NBAMR_U8 *)ps16InBuf >> 3));
                    if (u8Mode <  NBAMR_MAX_NUM_MODES)
                    {
                        u16DataSize = gas16PackedCodedSizeMMS[u8Mode]-1;
                    }
                    else
                    {
                        fprintf(stderr, "Invalid Mode\n");
                        return E_NBAMRD_INVALID_MODE;
                    }
                }
                else if (psDecConfig->u8BitStreamFormat == NBAMR_IF1IO)
                {
                    u8Mode = (NBAMR_U8) (0x0F & (*(NBAMR_U8 *)ps16InBuf >> 4));
                    if (u8Mode <  NBAMR_MAX_NUM_MODES)
                    {
                        u16DataSize = gas16PackedCodedSizeIF1[u8Mode]-1;
                    }
                    else
                    {
                        fprintf(stderr, "Invalid Mode\n");
                        return E_NBAMRD_INVALID_MODE;
                    }
                }
                else if (psDecConfig->u8BitStreamFormat == NBAMR_IF2IO)
                {
                    u8Mode = (NBAMR_U8) (0x0F & (*(NBAMR_U8 *)ps16InBuf));
                    if (u8Mode <  NBAMR_MAX_NUM_MODES)
                    {
                        u16DataSize = gas16PackedCodedSizeIF2[u8Mode]-1;
                    }
                    else
                    {
                        fprintf(stderr, "Invalid Mode\n");
                        return E_NBAMRD_INVALID_MODE;
                    }
                }
                else
                {
                    u16DataSize = 0;
                    /* invalid bitstream format */
                    fprintf(stderr, "Invalid Bitstream Format: %d\n",
                            psDecConfig->u8BitStreamFormat);
                    eRetVal = eAMRDDecodeExit (psDecConfig, ps16InBuf, ps16OutBuf);
                    vDisplayUsage(ps8Argv[0]);
                    return E_NBAMRD_INVALID_DECODER_ARGS;
                }
                if((s16AMRDReadPackedData (
                            (NBAMR_U8 *)((NBAMR_U8 *)(ps16InBuf)+1),
                                    pfFileSerial, u16DataSize)) == -1)
                {
#ifdef NBAMRD_DEBUG_PRINT
                    fprintf(stderr, "End of file reached\n");
#endif
                    break;
                }
            }
            ps16InBuf = ps16In;
            psDecConfig->u8NumFrameToDecode = (NBAMR_U8)s16Counter;
            s32Frame=s32Frame+psDecConfig->u8NumFrameToDecode;

#ifdef NBAMRD_DEBUG_PRINT
            if ( (s32Frame%50) == 0)
            {
                fprintf (stderr, "\rframe=%d  ", s32Frame);
            }
#endif

#ifdef TIME_PROFILE_RVDS
           dummyCall1();
#endif
#ifdef	ARM_MIPS_TEST_WINCE						/* for taking timing on wince platform*/
		INTERRUPTS_SET(0xDF);  						/*disable interrupt*/
		Flag=QueryPerformanceFrequency(&lpFrequency);
		Flag=QueryPerformanceCounter(&lpPerformanceCountBegin);
#endif
#ifdef TIME_PROFILE
            retVal = gettimeofday(&tv_prof, 0);
            if (retVal == 0)
            {
                time_before = tv_prof.tv_sec * 1000000 + tv_prof.tv_usec;
                timeFlag = 1;
            }
#endif
#if defined(TARGET_ARM11) && defined(TIME_PROFILE_RVDS )
            __asm
            {
                mrc p15, 0, prev_clk, c15, c12, 0;  /* Read the Monitor Control
                                                       Register in variable prev_clk */
                orr prev_clk, prev_clk, 0xf;       /* Configure the Monitor conrol register
                                                      to reset the Cycle Count Register and
                                                      enable divide by 64 counter */
                mcr p15, 0, prev_clk, c15, c12, 0   /* write back to Monitor
                                                       control regsiter */
                    mrc p15, 0, prev_clk, c15, c12, 1;  /* Read Cycle Count Register into
                                                           variable prev_clk */
            }
#endif
            /* call decode frame routine */
            eRetVal=eAMRDDecodeFrame (psDecConfig, ps16InBuf, ps16OutBuf);
            if (eRetVal != E_NBAMRD_OK)
            {
                /* free all the dynamic memory and exit */
                eRetVal = eAMRDDecodeExit (psDecConfig, ps16InBuf, ps16OutBuf);
                exit (-1);
            }
#if defined(TARGET_ARM11) && defined(TIME_PROFILE_RVDS )
            __asm
            {
                mrc p15, 0, curr_clk, c15, c12, 1;  /* Read Cycle Count Register in
                                                       variable curr_clk */
            }
            clk = (curr_clk-prev_clk);    /* clk gives the total core cycles per
                                             frame call */
            total_clk += clk;

            if(clk > frame_max_clk)
            {
                frame_max_clk = clk;
                frame_max_no = s32Frame;
            }

            if(clk < frame_min_clk)
            {
                frame_min_clk = clk;
                frame_min_no = s32Frame;
            }
#endif
#ifdef TIME_PROFILE
            if (timeFlag == 1)
            {
                retVal = gettimeofday(&tv_prof, 0);
                if (retVal == 0)
                {
                    time_after = tv_prof.tv_sec * 1000000 + tv_prof.tv_usec;
                    timeVal = time_after - time_before;

                    numFrame++;
                    if (timeVal > maxFrameTime)
                    {
                        maxFrameTime = timeVal;
                        maxFrameNumber = s32Frame;
                    }
                    else if (timeVal < minFrameTime)
                    {
                        minFrameTime = timeVal;
                        minFrameNumber = s32Frame;
                    }
                    totalTime += timeVal;
                }
                timeFlag = 0;
            }
#endif
#ifdef TIME_PROFILE_RVDS
           dummyCall2();
#endif
#ifdef ARM_MIPS_TEST_WINCE
                Flag=QueryPerformanceCounter(&lpPerformanceCountEnd);
                Temp=(long)(((lpPerformanceCountEnd.QuadPart - lpPerformanceCountBegin.QuadPart)*1000000)/(lpFrequency.QuadPart));/*this is the duration*/
                INTERRUPTS_SET(0x1F) ;  					/*enable interrupt*/
                numFrame++;
                if (Temp > maxFrameTime)
                {
                    maxFrameTime = Temp;
                    maxFrameNumber = numFrame;
                }
                else if (Temp < minFrameTime)
                {
                    minFrameTime = Temp;
                    minFrameNumber = numFrame;
                }
                totalTime += Temp;
                printf("totaltime: %d",totalTime);

#endif
#ifdef NBAMR_BIG_ENDIAN
            {
                NBAMR_S16 s16Index;
                for(s16Index = 0; s16Index <(psDecConfig->u8NumFrameToDecode*L_FRAME); s16Index++)
                {
                    ps16OutBuf[s16Index]=(((ps16OutBuf[s16Index]&0xff)<<8)
                            |((ps16OutBuf[s16Index]&0xff00)>>8));
                }
            }
#endif
#ifdef OUTPUT_DUMP
			temp= fwrite (ps16OutBuf, sizeof (NBAMR_S16), (psDecConfig->u8NumFrameToDecode*L_FRAME),
                        pfFileSyn);
            /* write synthesized speech to file */
            if (temp != (psDecConfig->u8NumFrameToDecode*L_FRAME))
            {
            #if  WINCE_BUILD==1
		        fprintf(stderr, "\nerror writing output file: \n");
            #else
                        fprintf(stderr, "\nerror writing output file: %s\n", strerror(errno));
           #endif
            }
            fflush(pfFileSyn);
#endif
        }   /* end of while loop */
    } /* end of else */

    if(pfFileSerial)
    	{    fclose(pfFileSerial);
    	}
    if (pfFileSyn)
    	{    fclose(pfFileSyn);
    	}

    fprintf (stderr, "\n%d frame(s) processed\n", s32Frame);

  	/*************************************************************
   	 *Closedown NB-AMR decoder
   	************************************************************/
#if defined(TARGET_ARM11) && defined(TIME_PROFILE_RVDS )
    #ifdef MEASURE_STACK_USAGE
    #ifdef MEASURE_HEAP_USAGE
 		fprintf(stderr,"\n\n%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t%ld\t%ld\t\n\n",chname, InputFileName,
					(frame_max_clk*64),(frame_min_clk*64),s32Frame,frame_max_no,frame_min_clk,(total_clk*64),s32PeakStack,s32TotalMallocBytes);
	#endif
	#endif

    #ifndef MEASURE_STACK_USAGE
    #ifndef MEASURE_HEAP_USAGE
		fprintf(stderr,"\n\n%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t\n\n",chname, InputFileName,
					(frame_max_clk*64),(frame_min_clk*64),s32Frame,frame_max_no,frame_min_clk,(total_clk*64));
	#endif
	#endif

	fprintf(pfCycleCount,"\tPeak Frame no: %ld,  clk: %ld\n", frame_max_no,
                                                                frame_max_clk);
    fprintf(pfCycleCount,"\tMin  Frame no: %ld,  clk: %ld\n", frame_min_no,
                                                                frame_min_clk);
    fprintf(pfCycleCount,"\tTotal Frame : %ld, average clk: %ld\n", s32Frame,
                                                s32Frame?total_clk/s32Frame:0);
    fprintf(pfCycleCount, "------------------------------------------\n\n");
    fclose(pfCycleCount);

#endif

#ifdef TIME_PROFILE
	printf("\nDecoder");
	printf("\n%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname, InputFileName,
						maxFrameTime,minFrameTime,s32Frame,maxFrameNumber,minFrameNumber,totalTime);
	
       bit_rate = (u16DataSize+1) * 8 * 50;
	   
      	fprintf (pfCycleCount,"\n|\tAudio Format\t|\tPlat\t|\tSamplerate\t|\tCh\t|\tBiterate\t|\tBit\t|\tPerf(MIPS)\t|\tAudio file name\n");
	performance = (double)totalTime * 0.000001*cpu_freq*50/numFrame;	//totalTime * cpu_freq / (numFrame *0.02)
       fprintf(pfCycleCount, "|\tNB_AMR Dec  \t|\tARM%d\t|\t%9d\t|\t%d\t|\t%8d\t|\t%d\t|\t%.8f\t|%s\n", platform, 8000, 1, bit_rate, 16, performance, InputFileName);
	
//	fprintf(pfCycleCount,"\n%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname, InputFileName,
//						maxFrameTime,minFrameTime,s32Frame,maxFrameNumber,minFrameNumber,totalTime);

    //fprintf(pfCycleCount,"\tPeak Frame no: %ld,  Peak Time: %ld\n",
    //          maxFrameNumber, maxFrameTime);
    //fprintf(pfCycleCount,"\tMin  Frame no: %ld,  Min Time: %ld\n",
    //           minFrameNumber, minFrameTime);
    //fprintf(pfCycleCount,"\tTotal Frame : %ld, Average Time: %ld\n",
    //        s32Frame, s32Frame?totalTime/numFrame:0);
    //fprintf(pfCycleCount, "----------------------------------------\n\n");
    fclose(pfCycleCount);
#endif
#ifdef __SYMBIAN32__
#ifdef MEASURE_HEAP_USAGE
			printf("\n Stack and Heap are  %d\t%d\t",s32PeakStack,s32TotalMallocBytes);
#endif
#endif
#ifdef	ARM_MIPS_TEST_WINCE
	printf("\n%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname, InputFileName,
						maxFrameTime,minFrameTime,s32Frame,maxFrameNumber,minFrameNumber,totalTime);
	fprintf(fSysTime,"\n%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname, InputFileName,
						maxFrameTime,minFrameTime,s32Frame,maxFrameNumber,minFrameNumber,totalTime);
#endif
    /* free all the dynamic memory and exit */
    eRetVal = eAMRDDecodeExit (psDecConfig, ps16InBuf, ps16OutBuf);
    return NBAMR_SUCCESS;
}

/*******************************************************************************
* Function: eAMRDDecodeExit
*
* Description: This function deallocates dynamic memorry
*
* Returns: eAMRDReturnType
*
* Arguments: psDecConfig -> pointer to decoder config
*            ps16InBuf -> pointer to input buffer
*            ps16OutBuf -> pointer to output buffer
*
* Global Variables: None
*
* Range Issues: None
*
* Special Issues: None
*
*******************************************************************************/
eAMRDReturnType eAMRDDecodeExit(sAMRDDecoderConfigType *psDecConfig, NBAMR_S16 *ps16InBuf,
                                NBAMR_S16 *ps16OutBuf)
{
    NBAMR_S16 s16Counter;
    if (ps16OutBuf != NULL)
    {
        mem_free (ps16OutBuf);
        ps16OutBuf = NULL;
    }
    if (ps16InBuf != NULL)
    {
        mem_free (ps16InBuf);
        ps16InBuf = NULL;
    }
	for (s16Counter=0; s16Counter<NBAMR_MAX_NUM_MEM_REQS; s16Counter++)
	{
        if ((psDecConfig->sAMRDMemInfo.asMemInfoSub[s16Counter].pvAPPDBasePtr) != NULL)
        {
            mem_free ((psDecConfig->sAMRDMemInfo.asMemInfoSub[s16Counter].pvAPPDBasePtr));
            (psDecConfig->sAMRDMemInfo.asMemInfoSub[s16Counter].pvAPPDBasePtr) = NULL;
        }
	}
    if (psDecConfig != NULL)
    {
        mem_free (psDecConfig);
        psDecConfig = NULL;
    }
    return E_NBAMRD_OK;
}

/*******************************************************************************
* Function: s16AMRDReadSerialData
*
* Description: This function reads encoded speech data of requested size
*
* Returns: Size of read data
*
* Arguments: ps16InBuf -> pointer to input buffer
*            pfFileSerial -> pointer to serial file
*            u16DataSize -> pointer to requested data size in number of bytes
*
* Global Variables: None
*
* Range Issues: None
*
* Special Issues: None
*
********************************************************************************/
NBAMR_S16 s16AMRDReadSerialData (NBAMR_S16 *ps16InBuf, FILE *pfFileSerial,
                                                        NBAMR_U16 u16DataSize)
{
    NBAMR_S16 s16NumData = -1;

    s16NumData = (NBAMR_U8)(fread (ps16InBuf, sizeof (NBAMR_S16), u16DataSize, pfFileSerial));
    if ((s16NumData == u16DataSize) || ((s16NumData != 0) && (!feof(pfFileSerial))))
    {
#ifdef NBAMR_BIG_ENDIAN
        {
            NBAMR_S16 s16Index;
            for(s16Index =0; s16Index < s16NumData ; s16Index++)
            {
                ps16InBuf[s16Index]=(((ps16InBuf[s16Index]&0xff)<<8)
                                     |((ps16InBuf[s16Index]&0xff00)>>8));
            }
        }
#endif
    }
    return s16NumData;
}

/*******************************************************************************
* Function: s16AMRDReadPackedData
*
* Description: This function reads encoded speech data of requested size in case of
*               MMS, IF1 or IF2 file format
*
* Returns: Size of read data
*
* Arguments: pu8InBuf -> pointer to input buffer
*            pfFileSerial -> pointer to serial file
*            u16DataSize -> pointer to requested data size in number of bytes
*
* Global Variables: None
*
* Range Issues: None
*
* Special Issues: None
*
********************************************************************************/
NBAMR_S16 s16AMRDReadPackedData (NBAMR_U8 *pu8InBuf, FILE *pfFileSerial,
                                                        NBAMR_U16 u16DataSize)
{
    NBAMR_S16 s16NumData = -1;

    if ((fread (pu8InBuf, sizeof (NBAMR_U8), u16DataSize, pfFileSerial)) == u16DataSize)
    {
        s16NumData = u16DataSize;
    }
    return s16NumData;
}

/*******************************************************************************
* Function: eAMRDProcessCmdLineOptions
*
* Description: Function process decoder command line options
*
* Returns: eAMRDReturnType
*
* Arguments: s32Argc -> Number of command line option
*            ps8Argv -> pointer to an array of command line arguments
*            ppfFileSerial -> pointer to pointer to file serial file name
*            ppfFileSyn -> pointer to pointer to synthesis file name
*
* Global Variables: None
*
* Range Issues: None
*
* Special Issues: None
*
*******************************************************************************/
eAMRDReturnType eAMRDProcessCmdLineOptions (
            NBAMR_S32 s32Argc, NBAMR_S8 *ps8Argv[],
            NBAMR_U8 *pu8BitStreamFormat, NBAMR_U8 *pu8NumFrame,
            FILE **ppfFileSerial, FILE **ppfFileSyn
            )
{
    NBAMR_S8  *ps8FileName = NULL;
    NBAMR_S8  *ps8SerialFileName = NULL;

    while (s32Argc > 1)
    {
#if !defined(TARGET_ARM11) || !defined(TIME_PROFILE_RVDS )
        if (strcmp((char *)ps8Argv[1], "-mms") == 0)
        {
            *pu8BitStreamFormat = NBAMR_MMSIO;
        }
        else if (strcmp((char *)ps8Argv[1], "-etsi") == 0)
        {
            *pu8BitStreamFormat = NBAMR_ETSI;
        }
        else if (strcmp((char *)ps8Argv[1], "-if1") == 0)
        {
            *pu8BitStreamFormat = NBAMR_IF1IO;
        }
        else if (strcmp((char *)ps8Argv[1], "-if2") == 0)
        {
            *pu8BitStreamFormat = NBAMR_IF2IO;
        }
        else if (strncmp((char *)ps8Argv[1], "-numframe=", 10) == 0)
        {
            *pu8NumFrame = (NBAMR_U8)(*(ps8Argv[1]+10));
            *pu8NumFrame = *pu8NumFrame - '0';
            /*   Deleted by jun.tao@freescale.com
             *if ((*pu8NumFrame < 1) || (*pu8NumFrame > 255))
             *{
             *    fprintf(stderr, "NumFrame should be between 1 and 255\n");
             *    return E_NBAMRD_INVALID_DECODER_ARGS;
             *}
             */

        }
#else
        if (mystrcmp_test((char *)ps8Argv[1], "-mms") == 0)
        {
            *pu8BitStreamFormat = NBAMR_MMSIO;
        }
        else if (mystrcmp_test((char *)ps8Argv[1], "-etsi") == 0)
        {
            *pu8BitStreamFormat = NBAMR_ETSI;
        }
        else if (mystrcmp_test((char *)ps8Argv[1], "-if1") == 0)
        {
            *pu8BitStreamFormat = NBAMR_IF1IO;
        }
        else if (mystrcmp_test((char *)ps8Argv[1], "-if2") == 0)
        {
            *pu8BitStreamFormat = NBAMR_IF2IO;
        }
        else if (mystrncmp((char *)ps8Argv[1], "-numframe=", 10) == 0)
        {
            *pu8NumFrame = ((NBAMR_U8)ps8Argv[1]+10);
        }
#endif
        else
        {
            break;
        }
        s32Argc--;
        ps8Argv++;
    }
    /* check command line options */
    if (s32Argc != 3)
    {
        vDisplayUsage(ps8Argv[0]);
        return E_NBAMRD_INVALID_DECODER_ARGS;
    }

    if (*pu8BitStreamFormat == NBAMR_MMSIO)
    {
        fprintf (stderr,"MMS file format\n");
    }
    else if (*pu8BitStreamFormat == NBAMR_IF1IO)
    {
        fprintf (stderr,"IF1 file format\n");
    }
    else if (*pu8BitStreamFormat == NBAMR_IF2IO)
    {
        fprintf (stderr,"IF2 file format\n");
    }
    else
    {
        fprintf (stderr,"Default ETSI file format\n");
    }

    ps8SerialFileName = ps8Argv[1];
    ps8FileName = ps8Argv[2];

    if ((*ppfFileSerial = fopen ((char *)ps8SerialFileName, "rb")) == NULL)
    {
        fprintf (stderr,"Error opening input serial file %s !!\n", ps8SerialFileName);
        return E_NBAMRD_ERROR;
    }
    InputFileName = ps8SerialFileName;
    fprintf (stderr,"Input serial file : %s\n", ps8SerialFileName);
    if ((*ppfFileSyn = fopen ((char *)ps8FileName, "wb")) == NULL)
    {
        fprintf (stderr,"Error opening output file %s !!\n", ps8FileName);
        return E_NBAMRD_ERROR;
    }
    fprintf (stderr,"output file : %s \n", ps8FileName);

    return E_NBAMRD_OK;
}

#if defined(TARGET_ARM11) && defined(TIME_PROFILE_RVDS )
NBAMR_S32 mystrcmp_test(const char* pcStr1, const char* pcStr2)
{
    const char* pcLocal1 = pcStr1;
    const char* pcLocal2 = pcStr2;
    NBAMR_S32   diff;

    /* TEDebug ("Comparing strings %s and %s\n", pcStr1, pcStr2); */

    do
    {
        diff = (NBAMR_S32)(*pcLocal1 - *pcLocal2);

        if (diff != 0)
        {
            /* TEDebug ("Returning %d at position %d\n", diff, pcLocal1 - pcStr1
            ); */
            return diff;
        }

        if (*pcLocal1 == '\0') /* means other is also null */
            return 0;

        pcLocal1++;
        pcLocal2++;
    } while (1);
}
NBAMR_S32 mystrncmp(const char* pcStr1, const char* pcStr2, NBAMR_U8 u8NumBytes)
{
    const char* pcLocal1 = pcStr1;
    const char* pcLocal2 = pcStr2;
    NBAMR_S32   diff;
    NBAMR_U8    u8Num=0;

    /* TEDebug ("Comparing strings %s and %s\n", pcStr1, pcStr2); */

    do
    {
        diff = (NBAMR_S32)(*pcLocal1 - *pcLocal2);

        if (diff != 0)
        {
            /* TEDebug ("Returning %d at position %d\n", diff, pcLocal1 - pcStr1
            ); */
            return diff;
        }

        if (*pcLocal1 == '\0') /* means other is also null */
            return 0;

        pcLocal1++;
        pcLocal2++;
        u8Num++;
    } while (u8Num<u8NumBytes);
}
#endif

NBAMR_Void vDisplayUsage(NBAMR_S8 *ps8ProgName)
{
    fprintf (stderr,
            " Usage:\n\n"
            "  %s [-bitstreamformat] [-numframetodecode] encoded_file  output_file\n\n", ps8ProgName);
}
#ifdef __WINCE
#define NAME_SIZE 255
int _tmain(int argc,_TCHAR *argv[])
{

            char* argv_char[NAME_SIZE];
            int argc_size,i;

            for(i=0;i < argc; i++)
            {
                        argv_char[i] = (char *) malloc(sizeof(char)*NAME_SIZE);
                        argc_size=wcstombs(argv_char[i],argv[i],NAME_SIZE);
            }
            main(argc,argv_char);

            for(i=0;i < argc; i++)
            {
                        free(argv_char[i]);
                        argv_char[i]=NULL;
            }
    return 0;
}
#endif


/**************************<END OF THE FILE>***********************************/
