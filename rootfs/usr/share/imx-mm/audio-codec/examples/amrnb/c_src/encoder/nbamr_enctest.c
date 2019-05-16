
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
***********************************************************************
********************************************************************************
* File Name: nbamr_enctest.c
*
* Description:  This file contains test application for NB-AMR encoder.
*
* Functions Included:
*                       - main
*                       - s16AMREReadSpeechSample
*                       - eAMREEncodeExit
*                       - s32AMREReadMode
*                       - eAMREProcessCmdLineOptions
*                       - vDisplayUsage
****************************** Change History***********************************
*
*   DD/MMM/YYYY     Code Ver     Description                        Author
*   -----------     --------     -----------                        ------
*   25/May/2004     0.1          File created                       Ashok
*   02/Jun/2004     1.0          Review rework                      Ashok
*   20/Jul/2004     1.1          Implemented                        Ashok
*                                API based on the
*                                doc
*   23/Sep/2004     1.3          Update type name               Tommy Tang
*   19/Nov/2004     1.4          Byte swap added for NBAMR_BIG_ENDIAN           Ashok
*                                testing
*   13/12/2006      1.5          Added profiling code for RVDS      Shyam Krishnan M
*   01/Jul/2008     1.6          Added version API                  Tao Jun
*   03/Jul/2008     1.7          Added A9 MCPS test code            Tao Jun
*   07/Oct/2008     1.8          Added WinCE build                  Tao Jun
*   07/Oct/2008     1.9          Added WinCE MCPS test              Tao Jun
*
*******************************************************************************/
/* section definition */
#if  WINCE_BUILD==1
#else
#ifdef ARM_OPTS
#pragma arm section rodata="APPE_TestApp_RO"
#pragma arm section rwdata="APPE_TestApp_RW"
#pragma arm section zidata="APPE_TestApp_ZI"
#endif
#endif
int temp=0;
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
#include "nb_amr_enc_api.h"
#include "nbamr_enctest.h"

#define OUTPUT_DUMP
#if defined TIME_PROFILE || defined TIME_PROFILE_RVDS|| ARM_MIPS_TEST_WINCE
#undef OUTPUT_DUMP
#endif
#ifdef MEASURE_STACK_USAGE
#include "mm_codec_common.h"
#endif

#if defined(TIME_PROFILE)
/* conversion table between mode name and mode ID */
const sConvTableType nbamrc_gasModeTable[MODE_TABLE_SIZE]=
{
    {"MR475", MR475},
    {"MR515", MR515},
    {"MR59",  MR59},
    {"MR67",  MR67},
    {"MR74",  MR74},
    {"MR795", MR795},
    {"MR102", MR102},
    {"MR122", MR122},
    {"MRDTX", MRDTX},
    {NULL,    -1}
};
#endif

#ifdef ALIGNMENT_CHECK
    int cp_ctrl_data = 0;
#endif
#ifdef ARM_MIPS_TEST_WINCE        		 /* for taking timing on wince platform*/
NBAMR_S8    *gps8InpFileName;
NBAMR_S8    *gps8OutFileName;
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

/***************************<GLOBAL_VARIABLES BEGIN>*****************************/
                                /* None */
/***************************<GLOBAL_VARIABLES END>*****************************/

/**************************<FUNCTION_PROTOTYPES BEGIN>*************************/
/* Local function */
/**************************<FUNCTION_PROTOTYPES END>***************************/
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
* Function: s32AMREReadMode
*
* Description: This function reads mode string from user supplied mode file.
*
* Returns: 0 -> NBAMR_SUCCESS
*          1 -> NBAMRE_FAILURE
*          EOF -> End of file (defined as -1 in  stdio.h)
*
* Arguments: pfFileModes -> pointer to mode file
*            pps8ModeStr -> pointer to mode string
*
* Global Variables: None
*
* Range Issues: None
*
* Special Issues: None
*
*******************************************************************************/
NBAMR_S32 s32AMREReadMode(FILE *pfFileModes, NBAMR_S8 **pps8ModeStr)
{
    if (fscanf(pfFileModes, "%9s\n", *pps8ModeStr) != 1)
    {
        if (feof(pfFileModes))
        {
            return EOF;
        }
        #if  WINCE_BUILD==1
	    fprintf(stderr, "\nerror reading mode control file: \n");
        #else
            fprintf(stderr, "\nerror reading mode control file: %s\n", strerror(errno));
	#endif
        {
            return NBAMR_FALSE;
        }
    }
    return NBAMR_SUCCESS;
}

/*******************************************************************************
*
* Function: main
*
* Description:  This is main encoder function. It is defined in application
*               space. It does memory as requested by encoder.
*
* Returns: NBAMR_SUCCESS/NBAMRE_FAILURE
*
* Arguments: For file IO testing it has following 2 arguments
*            s32Argc -> Number of command line arguments
*            ps8Argv -> pointer to command line arguments
*            Following are command line arguments
*            -dtx1 -> to enable VAD1
*            -dtx2 -> to enable VAD2
*            mode string
*            input file
*            output file
*
* Global Variables: None
*
* Range Issues: None
*
* Special Issues: None
*
*******************************************************************************/
NBAMR_S8 *InputFileName = NULL;
NBAMR_S8 *OutputFilename =NULL;
int main (NBAMR_S32 s32Argc, NBAMR_S8 *ps8Argv[])
{
    NBAMR_S16  *ps16InBuf = NULL;         /*pointer to input speech samples*/
    NBAMR_S16  *ps16OutBuf = NULL;        /* pointer to output (encoded) buffer */
    NBAMR_S8   *ps8ModeStr = NULL;
    sAMREMemAllocInfoSubType *psMem;       /*pointer to encoder sub-memory */
    sAMREEncoderConfigType   *psEncConfig; /* Pointer to encoder config structure */
    eAMREReturnType  eRetVal;              /* local variable */
    NBAMR_S16  s16NumMemReqs;
    NBAMR_S16  s16Counter;
    NBAMR_S16 s16DtxFlag=0;       /* DTX enable/disable flag */
    NBAMR_S8 s8UseModeFile=0;
  	NBAMR_S32 s32Frame;
    FILE *pfFileSpeech = NULL;       /* File of speech data  */
    FILE *pfFileSerial = NULL;       /* File of coded bits   */
    FILE *pfFileModes = NULL;        /* File with mode information */

    NBAMR_U8 u8BitStreamFormat = NBAMR_ETSI;    /* default format */

    NBAMR_U8 u8NumFrame = 1;    /* number of speech frame to be encoded in single call of
                                   EncodeFrame */
    NBAMR_S16 s16Size = 0;

#ifdef MEASURE_STACK_USAGE
    int           *ps32SP, *ps32BaseSP;
    int           s32StackCount, s32StackValue;
    int           s32PeakStack = 0;
#endif

#ifdef MEASURE_HEAP_USAGE
    int      s32TotalMallocBytes=0;
#endif

#ifdef NBAMR_BIG_ENDIAN
    NBAMR_S16 *ps16Out = NULL; /* local variable to store outbuf pointer */
#endif

#ifdef NBAMRE_DEBUG_PRINT
    NBAMR_S8   *ps8UsedModeStr = NULL;
#endif
    long infilesize = 0;

#if defined(TARGET_ARM11) && defined(TIME_PROFILE_RVDS ) /* Cycle measurement should be
                                                           enabled only when numframe
 	                                                         to encode is set to one */
    NBAMR_S32     prev_clk, curr_clk, clk;
    NBAMR_S32     total_clk = 0;
    NBAMR_S32     frame_min_clk = 0x7fffffff;
    NBAMR_S32     frame_max_clk = 0;
    NBAMR_S32     frame_min_no = 0;
    NBAMR_S32     frame_max_no = 0;
    unsigned char chname[] = "[PROFILE-INFO]";
	FILE    *pfCycleCount;
    pfCycleCount = fopen ("nbamr_enc_cycle.txt", "a");
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
    int mode_id = 0;
	int i = 0;
    double performance = 0.0;
#ifdef __SYMBIAN32__
    pfCycleCount = fopen ("D:\\nbamr_enc_cycle.txt", "a");
#else //__SYMBIAN32__
	pfCycleCount = fopen ("../audio_performance.txt", "a");
#endif //__SYMBIAN32__
#endif

#ifdef ARM_MIPS_TEST_WINCE
    FILE    *fSysTime;
    fSysTime = fopen ("nbamr_enc_wince_mcps.txt", "a");
    if(!fSysTime)
    {
		printf("\nUnable to open log file nbamr_enc_wince_mcps.txt\n");
		exit(1);
	}
#endif


    	/* Get the version info of the sign-on NB AMR */
    	fprintf(stderr,"Running %s\n",eAMREVersionInfo());



    eRetVal = eAMREProcessCmdLineOptions (s32Argc, &ps8Argv[0], &s16DtxFlag,
                        &ps8ModeStr, &s8UseModeFile, &u8BitStreamFormat, &u8NumFrame,
                        &pfFileSpeech, &pfFileSerial, &pfFileModes);
#if defined(TIME_PROFILE) || defined(TIME_PROFILE_RVDS)
	u8NumFrame = 1; //for calulating the max and min MHz for a frame
#endif //#if defined(TIME_PROFILE) || defined(TIME_PROFILE_RVDS)

    if (eRetVal != E_NBAMRE_OK)
    {
        exit(1);
    }

    /* allocate fast memory for encoder config structure */
    psEncConfig = (sAMREEncoderConfigType *)\
                            alloc_fast(sizeof(sAMREEncoderConfigType));
    if (psEncConfig == NULL)
    {
        return NBAMR_FAILURE;
    }

    /* allocate memory for encoder to use */
    psEncConfig->pvAMREEncodeInfoPtr = NULL;
    /* Not used */
    psEncConfig->pu8APPEInitializedDataStart = NULL;

    /* Set DTX flag */
    psEncConfig->s16APPEDtxFlag = s16DtxFlag;

    psEncConfig->u8BitStreamFormat = u8BitStreamFormat;
    psEncConfig->u8NumFrameToEncode= u8NumFrame;    /* number of frame to be encoded */

    /* encoded data size */
    psEncConfig->pu32AMREPackedSize = (NBAMR_U32 *)alloc_fast(u8NumFrame*sizeof(NBAMR_U32));

    /* user requested mode */
    psEncConfig->pps8APPEModeStr = alloc_fast(u8NumFrame * sizeof(NBAMR_S8 *));

    /* used mode by encoder */
    psEncConfig->pps8AMREUsedModeStr = alloc_fast(u8NumFrame * sizeof(NBAMR_S8*));

    for (s16Counter =0; s16Counter<u8NumFrame; s16Counter++)
    {
        /* set user requested encoding mode */
        psEncConfig->pps8APPEModeStr[s16Counter] = ps8ModeStr;
    }

    for (s16Counter =0; s16Counter<u8NumFrame; s16Counter++)
    {
        /* allocate memory to strore used mode, used mode value will be updated by
         * encoder libarary
         */
        psEncConfig->pps8AMREUsedModeStr[s16Counter] = alloc_fast(6*sizeof(NBAMR_S8*));
    }

    /* initialize packed data variable for all the frames */
    for (s16Counter=0; s16Counter<u8NumFrame; s16Counter++)
    {
        *(psEncConfig->pu32AMREPackedSize+s16Counter) = 0;
    }

    /* initialize config structure memory to NULL */
    for(s16Counter = 0; s16Counter <NBAMR_MAX_NUM_MEM_REQS; s16Counter++)
    {
        (psEncConfig->sAMREMemInfo.asMemInfoSub[s16Counter].pvAPPEBasePtr) = NULL;
    }

    /* Find encoder memory requiremet */
    eRetVal = eAMREQueryMem (psEncConfig);

    if (eRetVal != E_NBAMRE_OK)
    {
        /* de-allocate memory allocated for encoder config */
        mem_free(psEncConfig);
        return NBAMR_FAILURE;
    }
    /* Number of memory chunk requested by the encoder */
   	s16NumMemReqs = (NBAMR_S16)(psEncConfig->sAMREMemInfo.s32AMRENumMemReqs);
    /* allocate memory requested by the encoder*/
	for(s16Counter = 0; s16Counter <s16NumMemReqs; s16Counter++)
    {
        psMem = &(psEncConfig->sAMREMemInfo.asMemInfoSub[s16Counter]);
        if (psMem->s32AMREMemTypeFs == NBAMR_FAST_MEMORY)
        {
            psMem->pvAPPEBasePtr = alloc_fast(psMem->s32AMRESize);
			#ifdef MEASURE_HEAP_USAGE
				s32TotalMallocBytes +=  psMem->s32AMRESize;
			#endif
            if (psMem->pvAPPEBasePtr == NULL)
            {
                mem_free(psEncConfig);
                return NBAMR_FAILURE;
            }
        }
        else
        {
            psMem->pvAPPEBasePtr = alloc_slow(psMem->s32AMRESize);
			#ifdef MEASURE_HEAP_USAGE
				s32TotalMallocBytes +=  psMem->s32AMRESize;
			#endif
            if (psMem->pvAPPEBasePtr == NULL)
            {
                mem_free(psEncConfig);
                return NBAMR_FAILURE;
            }
        }
    }
	/* Call encoder init routine */
	eRetVal = eAMREEncodeInit (psEncConfig);
	if (eRetVal != E_NBAMRE_OK)
	{
		/* free all the dynamic memory and exit */
        eRetVal=eAMREEncodeExit(psEncConfig, s8UseModeFile, ps16InBuf, ps16OutBuf);
        return NBAMR_FAILURE;
	}

	/* allocate memory for input buffer */
	ps16InBuf = alloc_fast((psEncConfig->u8NumFrameToEncode * L_FRAME) * sizeof (NBAMR_S16));
    if (ps16InBuf == NULL)
    {
		/* free all the dynamic memory and exit */
        eRetVal=eAMREEncodeExit(psEncConfig, s8UseModeFile, ps16InBuf,
                                ps16OutBuf);
        return NBAMR_FAILURE;
    }
	/* allocate memory for output buffer (unpacked) */
    if (psEncConfig->u8BitStreamFormat == NBAMR_ETSI)
    {
        ps16OutBuf = alloc_fast ((psEncConfig->u8NumFrameToEncode*SERIAL_FRAMESIZE)*\
                            sizeof (NBAMR_S16));
    }
    else  /* it is either, mms or if1 or if 2 format */
    {
        ps16OutBuf = alloc_fast ((psEncConfig->u8NumFrameToEncode*\
                ((NBAMR_MAX_PACKED_SIZE/2)+(NBAMR_MAX_PACKED_SIZE%2)))*\
                                sizeof (NBAMR_S16));
    }
    if (ps16OutBuf == NULL)
    {
		/* free all the dynamic memory and exit */
        eRetVal=eAMREEncodeExit(psEncConfig, s8UseModeFile, ps16InBuf,
                                ps16OutBuf);
        return NBAMR_FAILURE;
    }
    /* store outbuf pointer to local variable */
#ifdef NBAMR_BIG_ENDIAN
    ps16Out = ps16OutBuf;
#endif
    if (psEncConfig->u8BitStreamFormat == NBAMR_MMSIO)
    {
#ifdef OUTPUT_DUMP
        /* write AMR magic number to indicate single channel AMR file format */
        temp +=sizeof(NBAMR_U8) * fwrite(NBAMR_MAGIC_NUMBER, sizeof(NBAMR_U8),
                strlen(NBAMR_MAGIC_NUMBER), pfFileSerial);
#endif
        fflush(pfFileSerial);
    }
	/*************************************************************
       * Encode speech frame till end of frame
	 *************************************************************/
    s32Frame = 0;

    while ((s16Size=s16AMREReadSpeechSample (pfFileSpeech, ps16InBuf, s8UseModeFile,
           pfFileModes, psEncConfig->pps8APPEModeStr, psEncConfig->u8NumFrameToEncode)) > 0)
    {
        /* Find out how may frame were actually read */
        psEncConfig->u8NumFrameToEncode = (NBAMR_U8)(s16Size/L_FRAME);
        infilesize += s16Size;
        if (psEncConfig->u8NumFrameToEncode != 0)
        {
            /* increment frame number */
            s32Frame = s32Frame + psEncConfig->u8NumFrameToEncode;
            /* initialize output buffer */
            if (psEncConfig->u8BitStreamFormat == NBAMR_ETSI)
            {
                for (s16Counter = 0; s16Counter < (psEncConfig->u8NumFrameToEncode*SERIAL_FRAMESIZE);
                        s16Counter++)
                    ps16OutBuf[s16Counter] = 0;
            }
            else
            {
                for (s16Counter = 0; s16Counter <
                        (psEncConfig->u8NumFrameToEncode*((NBAMR_MAX_PACKED_SIZE/2)+\
                                    (NBAMR_MAX_PACKED_SIZE%2))); s16Counter++)
                {
                    ps16OutBuf[s16Counter] = 0;
                }
            }

#ifdef MEASURE_STACK_USAGE
				printf("\n Frame No = %d\n", s32Frame);			// put as a solution for hanging due to optimization level
				PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
#endif
#ifdef TIME_PROFILE_RVDS
                                        dummyCall1();
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
#ifdef	ARM_MIPS_TEST_WINCE						/* for taking timing on wince platform*/
		INTERRUPTS_SET(0xDF);  						/*disable interrupt*/
		Flag=QueryPerformanceFrequency(&lpFrequency);
		Flag=QueryPerformanceCounter(&lpPerformanceCountBegin);
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
            eRetVal = eAMREEncodeFrame (psEncConfig, ps16InBuf, ps16OutBuf);
            //eRetVal  = E_NBAMRE_OK;
            if (eRetVal != E_NBAMRE_OK)
            {
                if (eRetVal == E_NBAMRE_INVALID_MODE)
                {
                    fprintf(stderr, "Invalid amr_mode specified: '%s'\n", ps8ModeStr);
                }
                /* free all the dynamic memory and exit */
                eRetVal=eAMREEncodeExit(psEncConfig, s8UseModeFile, ps16InBuf,
                        ps16OutBuf);
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
#ifdef ALIGNMENT_CHECK
                __asm
                {
                    mrc p15, 0, cp_ctrl_data, c1, c0, 0;  /* Read the control register value */
                    and cp_ctrl_data, cp_ctrl_data, 0xfffffffd;   /* disable alignment chect */
                    mcr p15, 0, cp_ctrl_data, c1, c0, 0   /* write back control register data */
                        mrc p15, 0, cp_ctrl_data, c1, c0, 0;  /* Read control register data for verification */
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
#endif
#ifdef NBAMRE_DEBUG_PRINT
            ps8UsedModeStr = (NBAMR_S8 *)\
                        psEncConfig->pps8AMREUsedModeStr[psEncConfig->u8NumFrameToEncode-1];
            if ( (s32Frame%50) == 0)
            {
                fprintf (stderr, "\rframe=%-8d mode=%-5s used_mode=%-5s", s32Frame,
                        ps8ModeStr, ps8UsedModeStr);
            }
#endif

            if (psEncConfig->u8BitStreamFormat == NBAMR_ETSI)
            {
#ifdef OUTPUT_DUMP
temp+= sizeof (NBAMR_S16) * fwrite (ps16OutBuf, sizeof (NBAMR_S16),
                            (psEncConfig->u8NumFrameToEncode*SERIAL_FRAMESIZE), pfFileSerial);
                /* write bitstream to output file */
                /*if (()
                        != (psEncConfig->u8NumFrameToEncode*SERIAL_FRAMESIZE))
                {
                    fprintf(stderr, "\nerror writing output file: %s\n", strerror(errno));
                    exit(-1);
                }*/
#endif
                fflush(pfFileSerial);
            }
                else
                {
                if (psEncConfig->u8BitStreamFormat == NBAMR_IF1IO)
                {
                    for (s16Counter=0; s16Counter < psEncConfig->u8NumFrameToEncode; s16Counter++)
                    {
                        /*
                         * IF1 frame format returns number of bits based on the mode used. This
                         * includes the header part of 8 * 3 = 24 bits.
                         * Following calculation is done to get the number of packed bytes to be
                         * written into the file. 1 is added for the remainder bits.
                         */
                        psEncConfig->pu32AMREPackedSize[s16Counter]=
                                  (psEncConfig->pu32AMREPackedSize[s16Counter] >> 3) + 1;
                    }
                }
                for (s16Counter=0; s16Counter < psEncConfig->u8NumFrameToEncode; s16Counter++)
                {
#ifdef OUTPUT_DUMP
                    /* write bitstream to output file */
                    if (fwrite ((ps16OutBuf+s16Counter*((NBAMR_MAX_PACKED_SIZE/2)+\
                               (NBAMR_MAX_PACKED_SIZE%2))), sizeof (NBAMR_U8),
                                psEncConfig->pu32AMREPackedSize[s16Counter], pfFileSerial)
                        != psEncConfig->pu32AMREPackedSize[s16Counter])
                    {
                    #if  WINCE_BUILD==1
		        fprintf(stderr, "\nerror writing output file: \n");
		    #else
                        fprintf(stderr, "\nerror writing output file: %s\n", strerror(errno));
		    #endif
                        exit(-1);
                    }
#endif
                    fflush(pfFileSerial);
                }
            }
        }
    }

    fprintf (stderr, "\n%d frame(s) processed\n", s32Frame);

  	/***************************************************************************
   	 *Closedown speech coder
   	****************************************************************************/
#if defined(TARGET_ARM11) && defined(TIME_PROFILE_RVDS )
	#ifdef MEASURE_STACK_USAGE
	#ifdef MEASURE_HEAP_USAGE
		fprintf(stderr,"\n\n%s\t%s\t%ld\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t%ld\t%ld\t\n\n",chname, InputFileName,2*infilesize,ps8ModeStr,
						(frame_max_clk*64),(frame_min_clk*64),s32Frame,frame_max_no,frame_min_no,(total_clk*64),s32PeakStack,s32TotalMallocBytes);
	#endif
	#endif

	#ifndef MEASURE_STACK_USAGE
	#ifndef MEASURE_HEAP_USAGE
		fprintf(stderr,"\n\n%s\t%s\t%ld\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t\n\n",chname, InputFileName,2*infilesize,ps8ModeStr,
						(frame_max_clk*64),(frame_min_clk*64),s32Frame,frame_max_no,frame_min_no,(total_clk*64));
	#endif
	#endif

	fprintf(pfCycleCount,"\tPeak Frame no: %ld,  clk: %ld\n", frame_max_no,
                                                                frame_max_clk);
    fprintf(pfCycleCount,"\tMin  Frame no: %ld,  clk: %ld\n", frame_min_no,
                                                                frame_min_clk);
    fprintf(pfCycleCount,"\tTotal Frame: %ld, average clk: %ld\n",
                                       s32Frame, s32Frame?total_clk/s32Frame:0);
    fprintf(pfCycleCount, "------------------------------------------\n\n");
    fclose(pfCycleCount);

#endif

#ifdef TIME_PROFILE
	printf("\nEncoder");
	printf("\n%s\t%s\t%ld\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname, InputFileName,2*infilesize,ps8ModeStr,
						maxFrameTime,minFrameTime,s32Frame,maxFrameNumber,minFrameNumber,totalTime);	

	for(i=0; i<MODE_TABLE_SIZE; i++)
	{
		if(0 == strcmp(ps8ModeStr, nbamrc_gasModeTable[i].name))
		{
			mode_id = nbamrc_gasModeTable[i].id;
			break;
		}
	}
	printf("\nmode_id:%d, i=%d\n ", mode_id, i);
	switch(mode_id)
	{
	case MR475:
		bit_rate = 4750;
		break;
	case MR515:
	       bit_rate = 5150;
		   break;
       case MR59:
		bit_rate = 5900;
		break;
	case MR67:
	       bit_rate = 6700;
		break;
	case MR74:
		bit_rate = 7400;
		break;
	case MR795:
		bit_rate = 7950;
		break;
	case MR102:
		bit_rate = 10200;
		break;
	case MR122:
		bit_rate = 12200;
		break;
	case MRDTX:
	default:
		bit_rate = 0;
		break;
	}
   	fprintf (pfCycleCount,"\n|\tAudio Format\t|\tPlat\t|\tSamplerate\t|\tCh\t|\tBiterate\t|\tBit\t|\tPerf(MIPS)\t|\tAudio file name\n");
	performance = (double)totalTime * 0.000001*cpu_freq*50/numFrame;	//totalTime * cpu_freq / (numFrame *0.02)
       fprintf(pfCycleCount, "|\tNB_AMR Enc  \t|\tARM%d\t|\t%9d\t|\t%d\t|\t%8d\t|\t%d\t|\t%.8f\t|%s\n", platform, 8000, 1, bit_rate, 16, performance, OutputFilename);

//	fprintf(pfCycleCount,"\n%s\t%s\t%ld\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname, InputFileName,2*infilesize,ps8ModeStr,
//						maxFrameTime,minFrameTime,s32Frame,maxFrameNumber,minFrameNumber,totalTime);
    //fprintf(pfCycleCount,"\tPeak Frame no: %ld,  Peak Time: %ld\n",
    //            maxFrameNumber, maxFrameTime);
    //fprintf(pfCycleCount,"\tMin Frame no: %ld,  Min Time: %ld\n",
    //            minFrameNumber, minFrameTime);
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
	//printf("\nEncoder");
	//printf("\n%s\t%s\t%ld",chname,InputFileName,maxFrameTime);
	//printf("\t%ld",minFrameTime);
	//printf("\t%d\t%d\t%d\t%ld\t",numFrame,maxFrameNumber,minFrameNumber,totalTime);
	//printf("\nEncoder");

	printf("\n%s\t%s\t%ld\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname, InputFileName,2*infilesize,ps8ModeStr,
						maxFrameTime,minFrameTime,s32Frame,maxFrameNumber,minFrameNumber,totalTime);

	//fprintf(fSysTime,"\n%s\t%s\t%ld",chname,InputFileName,maxFrameTime);
	//fprintf(fSysTime,"\t%ld",minFrameTime);
	//fprintf(fSysTime,"\t%d\t%d\t%d\t%ld\t",numFrame,maxFrameNumber,minFrameNumber,totalTime);
	//fclose(fSysTime);
	fprintf(fSysTime,"\n%s\t%s\t%ld\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname, InputFileName,2*infilesize,ps8ModeStr,
						maxFrameTime,minFrameTime,s32Frame,maxFrameNumber,minFrameNumber,totalTime);
#endif
    /* free all the dynamic memory and exit */
    eRetVal = eAMREEncodeExit(psEncConfig, s8UseModeFile, ps16InBuf, ps16OutBuf);
    return NBAMR_SUCCESS;
}

/*******************************************************************************
*
* Function: s16AMREReadSpeechSample
*
* Description: This fucntions reads one frame of speech sample from buffer or
*              a file
*
* Returns: Number of speech sample read. This should be 160 in case of success
*
* Arguments: pfFileSpeech -> pointer to speech file
*            ps16InBuf -> pointer to input buffer
*            s8UseModeFile -> Flag to indicate whether mode switching is enabled
*            pfFileModes -> pointer to mode file, used in case of mode switching
*                           enabled
*            ppsAPPEModeStr -> pointer to pointer to mode string
*
* Global Variables: None
*
* Range Issues: None
*
* Special Issues: None
*
*******************************************************************************/
NBAMR_S16 s16AMREReadSpeechSample (FILE *pfFileSpeech, NBAMR_S16 *ps16InBuf,
                                   NBAMR_S8 s8UseModeFile, FILE * pfFileModes,
                                   NBAMR_S8 **pps8ModeStr, NBAMR_U8 u8NumFrame)
{
    NBAMR_S16  s16RetVal = -1;
    NBAMR_S16 s16Index;

    s16RetVal = (NBAMR_S16)(fread (ps16InBuf, sizeof (NBAMR_S16), (u8NumFrame*L_FRAME), pfFileSpeech));
    if (s16RetVal > 0)
    {
        /* Not end of file but read size not same as requested size */
#ifdef NBAMR_BIG_ENDIAN
        for(s16Index =0; s16Index <(u8NumFrame*L_FRAME); s16Index++)
        {
            ps16InBuf[s16Index]=(((ps16InBuf[s16Index]&0xff)<<8)
                    |((ps16InBuf[s16Index]&0xff00)>>8));
        }
#endif
        /* read new mode string from file if required */
        if (s8UseModeFile)
        {
            NBAMR_S32 s32Result;
            for (s16Index =0; s16Index<u8NumFrame; s16Index++)
            {
                pps8ModeStr[s16Index] = (NBAMR_S8 *)alloc_fast(10*sizeof(NBAMR_S8*));

                if ((s32Result = s32AMREReadMode(pfFileModes,&pps8ModeStr[s16Index])) == EOF)
                {
                    fprintf(stderr, "\nend of mode control file reached");
                    return s16RetVal;
                }
                else if (s32Result == 1)
                {
                    return s16RetVal;
                }
            }
        }
    }
    else if (feof(pfFileSpeech) != (int)NULL)
    {
        /* end of file reached */
        s16RetVal = 0;
#ifdef NBAMRE_DEBUG_PRINT
        fprintf(stderr, "\nEnd Of File Reached\n");
#endif
    }
    else
    {
        /* error in reading */
        s16RetVal = -1;
        fprintf(stderr, "\nError in reading\n");
    }
    return s16RetVal;
}

/*******************************************************************************
*
* Function: eAMREEncodeExit
*
* Description: This fucntions deallocates all dynamically allocated memory
*
* Returns: eAMREReturnType
*
* Arguments: psEncConfig -> pointer to encoder config structure
*            s8UseModeFile -> flag to indicate whether mode file is used or not
*
* Global Variables: None
*
* Range Issues: None
*
* Special Issues: None
*
*******************************************************************************/
eAMREReturnType eAMREEncodeExit(
            sAMREEncoderConfigType *psEncConfig,
            NBAMR_S8 s8UseModeFile,
            NBAMR_S16 *ps16InBuf,
            NBAMR_S16 *ps16OutBuf
            )
{
    NBAMR_S16 s16Counter;
    if (ps16OutBuf != NULL)
    {
        /* free input buffer */
        mem_free (ps16OutBuf);
        ps16OutBuf = NULL;
    }
    if (ps16InBuf != NULL)
    {
        /* free output buffer */
        mem_free (ps16InBuf);
        ps16InBuf = NULL;
    }
    for (s16Counter=0; s16Counter<NBAMR_MAX_NUM_MEM_REQS; s16Counter++)
    {
        if ((psEncConfig->sAMREMemInfo.asMemInfoSub[s16Counter].pvAPPEBasePtr) != NULL)
        {
            mem_free ((psEncConfig->sAMREMemInfo.asMemInfoSub[s16Counter].pvAPPEBasePtr));
            (psEncConfig->sAMREMemInfo.asMemInfoSub[s16Counter].pvAPPEBasePtr) = NULL;
        }
    }
    if (s8UseModeFile)
    {
        if (psEncConfig->pps8APPEModeStr != NULL)
        {
            mem_free (psEncConfig->pps8APPEModeStr);
            psEncConfig->pps8APPEModeStr = NULL;
        }
    }
    if (psEncConfig != NULL)
    {
        mem_free (psEncConfig);
        psEncConfig = NULL;
    }
    return E_NBAMRE_OK;
}

/*******************************************************************************
*
* Function: eAMREProcessCmdLineOptions
*
* Description:  This function processes command line options and
*
* Returns: eAMREReturnType
*
* Arguments:    s32Argc -> number of command line arguments
*               ps8Argv -> pointer to command line arguments
*               ps16DtxFlag -> pointer to DTX flag
*               pps8ModeStr -> pointer to mode string
*               ps8UseModeFile -> pointer to flag that indicates whether mode file
*                                  need to be used
*               ps8APPEModeStr -> pointer to pointer to mode string
*               ps8UseModeFile -> pointer to mode file flag
*               ppfFileSpeech -> pointer to pointer to speech file
*               ppfFileSerial -> pointer to pointer to serial file
*               ppfFileModes -> pointer to pointer to Mode file
*
* Global Variables: None
*
* Range Issues: None
*
* Special Issues: None
*
*******************************************************************************/
eAMREReturnType eAMREProcessCmdLineOptions (
           NBAMR_S32 s32Argc, NBAMR_S8 *ps8Argv[],
           NBAMR_S16 *ps16DtxFlag, NBAMR_S8  **pps8ModeStr,
           NBAMR_S8 *ps8UseModeFile, NBAMR_U8 *pu8BitStreamFormat,
           NBAMR_U8 *pu8NumFrame, FILE **ppfFileSpeech,
           FILE **ppfFileSerial, FILE **ppfFileModes
           )
{
    NBAMR_S8 *ps8FileName = NULL;
    NBAMR_S8 *ps8ModeFileName = NULL;
    NBAMR_S8 *ps8SerialFileName = NULL;

    /* Initialize mode file flag */
    *ps8UseModeFile = 0;
    /* Process command line options */
    while (s32Argc > 1)
    {
#if !defined(TARGET_ARM11) || !defined(TIME_PROFILE_RVDS )
        if (strcmp((char *)ps8Argv[1], "-dtx1") == 0)
        {
            *ps16DtxFlag = NBAMR_VAD1;
        }
        else if (strcmp((char *)ps8Argv[1], "-dtx2") == 0)
        {
            *ps16DtxFlag = NBAMR_VAD2;
        }
        else if (strcmp((char *)ps8Argv[1], "-mms") == 0)
        {
            *pu8BitStreamFormat = NBAMR_MMSIO;
            fprintf (stderr, "Requested Bitstreamformat is MMS\n");
        }
        else if (strcmp((char *)ps8Argv[1], "-etsi") == 0)
        {
            *pu8BitStreamFormat = NBAMR_ETSI;
            fprintf (stderr, "Requested Bitstreamformat is ETSI\n");
        }
        else if (strcmp((char *)ps8Argv[1], "-if1") == 0)
        {
            *pu8BitStreamFormat = NBAMR_IF1IO;
            fprintf (stderr, "Requested Bitstreamformat is IF1\n");
        }
        else if (strcmp((char *)ps8Argv[1], "-if2") == 0)
        {
            *pu8BitStreamFormat = NBAMR_IF2IO;
            fprintf (stderr, "Requested Bitstreamformat is IF2\n");
        }
        else if (strncmp((char *)ps8Argv[1], "-numframe=", 10) == 0)
        {
            *pu8NumFrame = (NBAMR_U8)(*(ps8Argv[1]+10));
            *pu8NumFrame = *pu8NumFrame - '0';
            if ((*pu8NumFrame < 1))
            {
                fprintf(stderr, "NumFrame should be between 1 and 255\n");
                return E_NBAMRE_INVALID_ENCODER_ARGS;
            }
        }
        else if (strncmp((char *)ps8Argv[1], "-modefile=", 10) == 0)
        {
            *ps8UseModeFile = 1;
            ps8ModeFileName = ps8Argv[1]+10;
        }
#else
        if (mystrcmp_test((char *)ps8Argv[1], "-dtx1") == 0)
        {
            *ps16DtxFlag = NBAMR_VAD1;
        }
        else if (mystrcmp_test((char *)ps8Argv[1], "-dtx2") == 0)
        {
            *ps16DtxFlag = NBAMR_VAD2;
        }
        else if (mystrcmp_test((char *)ps8Argv[1], "-etsi") == 0)
        {
            *pu8BitStreamFormat = NBAMR_ETSI;
            fprintf (stderr, "Requested Bitstreamformat is ETSI\n");
        }
        else if (mystrcmp_test((char *)ps8Argv[1], "-mms") == 0)
        {
            *pu8BitStreamFormat = NBAMR_MMSIO;
            fprintf (stderr, "Requested Bitstreamformat is MMS\n");
        }
        else if (mystrcmp_test((char *)ps8Argv[1], "-if1") == 0)
        {
            *pu8BitStreamFormat = NBAMR_IF1IO;
            fprintf (stderr, "Requested Bitstreamformat is IF1\n");
        }
        else if (mystrcmp_test((char *)ps8Argv[1], "-if2") == 0)
        {
            *pu8BitStreamFormat = NBAMR_IF2IO;
            fprintf (stderr, "Requested Bitstreamformat is IF2\n");
        }
        else if (mystrncmp((const char *)ps8Argv[1], (const char *)("-numframe="), (NBAMR_U8)10) == 0)
        {
            *pu8NumFrame = (NBAMR_U8)ps8Argv[1]+10;
        }
        else if (mystrncmp((char *)ps8Argv[1], "-modefile=", 10) == 0)
        {
            *ps8UseModeFile = 1;
            ps8ModeFileName = ps8Argv[1]+10;
        }
#endif
        else
        {
            break;
        }
        s32Argc--;
        ps8Argv++;
    }
    if ((*pu8BitStreamFormat != NBAMR_MMSIO) && (*pu8BitStreamFormat != NBAMR_IF1IO)
       && (*pu8BitStreamFormat != NBAMR_IF2IO))
    {
        fprintf (stderr, "Requested Bitstreamformat is ETSI\n");
    }
    /* Check number of arguments */
    if ((s32Argc != 4 && !(*ps8UseModeFile)) || (s32Argc != 3 && (*ps8UseModeFile)))
    {
        vDisplayUsage(ps8Argv[0]);
        return E_NBAMRE_ERROR;
    }
    /* Open mode file or convert mode string */
    if ((*ps8UseModeFile))
    {
        /* allocate the memory for modeString */
        *pps8ModeStr = alloc_fast (10*sizeof(NBAMR_S8));
        ps8FileName = ps8Argv[1];
        ps8SerialFileName = ps8Argv[2];
        /* Open mode control file */
#if !defined(TARGET_ARM11) || !defined(TIME_PROFILE_RVDS )
        if (strcmp((char *)ps8ModeFileName, "-") == 0)
#else
        if (mystrcmp_test((char *)ps8ModeFileName, "-") == 0)
#endif
        {
            *ppfFileModes = stdin;
        }
        else if ((*ppfFileModes = fopen ((char *)ps8ModeFileName, "rt")) == NULL)
        {
            fprintf (stderr, "Error opening mode control file  %s !!\n",
                    ps8ModeFileName);
            return E_NBAMRE_ERROR;
        }
        fprintf (stderr, " Mode control file:      %s\n", ps8ModeFileName);
    }
    else
    {
        *pps8ModeStr = ps8Argv[1];
        ps8FileName = ps8Argv[2];
        ps8SerialFileName = ps8Argv[3];
    }

    /* Open speech file and result file (output serial bit stream) */
#if !defined(TARGET_ARM11) || !defined(TIME_PROFILE_RVDS )
    if (strcmp((char *)ps8FileName, "-") == 0)
#else
    if (mystrcmp_test((char *)ps8FileName, "-") == 0)
#endif
    {
        *ppfFileSpeech = stdin;
    }
    else if ((*ppfFileSpeech = fopen ((char *)ps8FileName, "rb")) == NULL)
    {
        fprintf (stderr, "Error opening input file  %s !!\n", ps8FileName);
        return E_NBAMRE_ERROR;
    }
    InputFileName = ps8FileName;
    OutputFilename = ps8SerialFileName;
    fprintf (stderr, " Input speech file:      %s\n", ps8FileName);

#if !defined(TARGET_ARM11) || !defined(TIME_PROFILE_RVDS )
    if (strcmp((char *)ps8SerialFileName, "-") == 0)
#else
    if (mystrcmp_test((char *)ps8SerialFileName, "-") == 0)
#endif
    {
        *ppfFileSerial = stdout;
    }
    else if ((*ppfFileSerial = fopen ((char *)ps8SerialFileName, "wb")) == NULL)
    {
        fprintf (stderr,"Error opening output bitstream file %s !!\n",
                                                            ps8SerialFileName);
        return E_NBAMRE_ERROR;
    }
    fprintf (stderr, " Output bitstream file:  %s\n", ps8SerialFileName);

    return E_NBAMRE_OK;
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

    /* return 0; */  /* matched, but code should not come here */
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
            "  %s [-dtx] amr_mode speech_file  bitstream_file\n\n"
            " or \n\n"
            " [-dtx] [-bitstreamformat] amr_mode speech_file  bitstream_file\n\n"
            " or \n\n"
            " [-dtx] [-bitstreamformat] [-numframetoencode] amr_mode speech_file  bitstream_file\n\n"
            " or \n\n"
            "  %s [-dtx] -modefile=mode_file speech_file  bitstream_file\n\n"
            " -mms|if1|if2        MMS or IF1 or IF2 file format\n"
            " -numframe=          Number of frame to encode  should be between 1 and 255 \n"
            " -modefile=mode_file reads AMR modes from text file \n"
            " -dtx                enables DTX mode \n"
            "(one line per frame)\n\n", ps8ProgName, ps8ProgName);
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
