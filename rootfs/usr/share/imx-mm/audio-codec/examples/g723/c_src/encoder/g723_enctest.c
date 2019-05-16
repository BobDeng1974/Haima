/*****************************************************************************
 *
 * Motorola Inc.
 * (c) Copyright 2004 Motorola, Inc.
 * ALL RIGHTS RESERVED.
 *
 *****************************************************************************
 ***********************************************************************
 * Copyright (c) 2005-2010, 2012, Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc.
 ***********************************************************************
 * File Name: g723_enctest.c
 *
 * Description: G.723.1 encoder test wrapper is defined in this file.
 *
 ***************************** Change History*********************************
 *
 *   DD/MMM/YYYY     Code Ver     Description      Author
 *   -----------     --------     -----------      ------
 *   18/Oct/2004     0.1          File created     Tommy Tang
 *   11/Nov/2004     0.2          Add comments     Tommy Tang
 *   08/May/2005     1.0          Support table    Tommy Tang
 *                                relocation
 *   13/12/2006      1.1          Added profiling code for RVDS         Shyam Krishnan M
 *   13/Jun/2008     1.2          Added ARM9 MCPS  Tao Jun
 *                                test codes
 *****************************************************************************/
/*****************************<INCLUDE_FILES BEGIN>***************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "g723_enc_api.h"
#include "g723_enctest.h"
#ifdef TIME_PROFILE
#ifdef __SYMBIAN32__
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#ifdef __WINCE
#include<windows.h>
#endif
#define OUTPUT_DUMP
#if defined TIME_PROFILE || defined TIME_PROFILE_RVDS || defined TIME_PROFILE_RVDS_ARM9|| defined ARM_MIPS_TEST_WINCE
#undef OUTPUT_DUMP
#endif
#ifdef MEASURE_STACK_USAGE
#include "mm_codec_common.h"
#endif

#ifdef TIME_PROFILE_RVDS
#include "strfunc.h"
#endif
#ifdef TIME_PROFILE_RVDS_ARM9
void    dummyCall1(void);
void    dummyCall2(void);
G723_S32  uiframe;
#endif

#ifdef ARM_MIPS_TEST_WINCE        		 /* for taking timing on wince platform*/
G723_S8    *gps8InpFileName;
G723_S8    *gps8OutFileName;
	extern void INTERRUPTS_SET(int k);
	unsigned int	Flag;
	LARGE_INTEGER	lpFrequency;
	LARGE_INTEGER	lpPerformanceCountBegin;
	LARGE_INTEGER	lpPerformanceCountEnd;
	__int64	Temp;
	FILE	*pfCycleCount;
	__int64	totalTime = 0;
	unsigned int	numFrame = 0;
	__int64	minFrameTime = 0x7FFFFFFFFFFFFFFF;
	__int64	maxFrameTime = -1;
	unsigned int	minFrameNumber = 0;
	unsigned int	maxFrameNumber = 0;
	unsigned char	chname[] = "[PROFILE-INFO]";
#endif

/******************************<INCLUDE_FILES END>****************************/
#ifdef G723_ENC_TABRELOC_TST
G723_Void *app_tables[22];

G723_Void init_tables ()
{
    app_tables[0]  = (G723_Void *)(const G723_S32 *)gas32L_bseg;
    app_tables[1]  = (G723_Void *)(const G723_S32 *)gas32CombinatorialTable;
    app_tables[2]  = (G723_Void *)(const G723_S16 *)gas16AcbkGainTable085;
    app_tables[3]  = (G723_Void *)(const G723_S16 *)gas16AcbkGainTable170;
    app_tables[4]  = (G723_Void *)(const G723_S16 *)gas16epsi170;
    app_tables[5]  = (G723_Void *)(const G723_S16 *)gas16gain170;
    app_tables[6]  = (G723_Void *)(const G723_S16 *)gas16LspDcTable;
    app_tables[7]  = (G723_Void *)(const G723_S16 *)gas16BandInfoTable;
    app_tables[8]  = (G723_Void *)(const G723_S16 *)gas16BandTb8;
    app_tables[9]  = (G723_Void *)(const G723_S16 *)gas16CosineTable;
    app_tables[10] = (G723_Void *)(const G723_S16 *)gas16fact;
    app_tables[11] = (G723_Void *)(const G723_S16 *)gas16base;
    app_tables[12] = (G723_Void *)(const G723_S16 *)gas16Nb_puls;
    app_tables[13] = (G723_Void *)(const G723_S16 *)gas16FcbkGainTable;

    app_tables[14] = (G723_Void *)(const G723_S16 *)gas16BandExpTable;
    app_tables[15] = (G723_Void *)(const G723_S16 *)gas16HammingWindowTable;
    app_tables[16] = (G723_Void *)(const G723_S16 *)gas16BinomialWindowTable;
    app_tables[17] = (G723_Void *)(const G723_S16 *)gas16ScfTable;
    app_tables[18] = (G723_Void *)(const G723_S16 *)gas16PerFiltZeroTable;
    app_tables[19] = (G723_Void *)(const G723_S16 *)gas16PerFiltPoleTable;
    app_tables[20] = (G723_Void *)(const G723_S16 *)gas16tabgain85;
    app_tables[21] = (G723_Void *)(const G723_S16 *)gas16tabgain170;
}
#endif
/*****************************************************************************
 *
 * Function:  vDisplayUsage
 *
 * Description:  Display Usage
 *
 * Returns:      None
 *
 * Arguments:    None
 *
 * Global Variables: None
 *
 * Range Issues: None
 *
 * Special Issues: None
 *
 *****************************************************************************/

G723_Void vDisplayUsage()
{
       printf("G723.1 Codec Demo Program. Written by Motorola. All rights reserved.\n");
	printf("\n");
	printf("Usage: G723EncTest [-Options] <InpFile> <OutFile>\n");
	printf( "\n");
	printf( "Where:\n");
	printf( "  InpFile      is the name of the file to be processed.\n");
	printf( "  OutFile      is the name with the processed data.\n");
	printf( "Options:\n");
	printf( "  -DTX         Enable/Disable VAD\n");
	printf( "               Default is disable.\n");
	printf( "  -rate #      is the bit-rate (53 for 5.3 kbit/s and 63 for\n");
	printf( "               6.3kbit/s) or file\n");
	printf( "               Default is 6.3 kbit/s.\n");
	printf( "  -Noh         Enable/Disable Hp Filter\n");
	printf( "               Default is enable.\n");
	printf( "  -?/-help     print help message\n");
#ifdef G723_DEBUG_PRINT
	fprintf(stderr, "G723.1 Codec Demo Program. Written by Motorola. All rights reserved.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage: G723EncTest [-Options] <InpFile> <OutFile>\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Where:\n");
	fprintf(stderr, "  InpFile      is the name of the file to be processed.\n");
	fprintf(stderr, "  OutFile      is the name with the processed data.\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -DTX         Enable/Disable VAD\n");
	fprintf(stderr, "               Default is disable.\n");
	fprintf(stderr, "  -rate #      is the bit-rate (53 for 5.3 kbit/s and 63 for\n");
	fprintf(stderr, "               6.3kbit/s) or file\n");
	fprintf(stderr, "               Default is 6.3 kbit/s.\n");
	fprintf(stderr, "  -Noh         Enable/Disable Hp Filter\n");
	fprintf(stderr, "               Default is enable.\n");
	fprintf(stderr, "  -?/-help     print help message\n");
#endif
	exit(1);
}

#ifdef G723_BIG_ENDIAN
/*****************************************************************************
 *
 * Function: vSwapBytes
 *
 * Description: This fucntions used to swap the high-byte and low-byte.
 *
 *
 * Returns:    None
 *
 * Arguments:     ps16Buf -> pointer to memory address
 *                 s32Cnt -> the memmory size
 *
 *
 * Range Issues: None
 *
 * Special Issues: None
 *
 *****************************************************************************/

void vSwapBytes(G723_S16 *ps16Buf, G723_S32 s32Cnt)
{
    G723_S8 s8ch;
    G723_S8 *s8p;

    while(--s32Cnt >= 0)
    {
         s8p = (G723_S8 *)(ps16Buf+s32Cnt);
         s8ch = s8p[0];
         s8p[0] = s8p[1];
         s8p[1] = s8ch;
    }
}
#endif

/*****************************************************************************
 *
 * Function:  vLineWrite
 *
 * Description:  Write a string to file
 *
 * Returns:  None
 *
 * Arguments:
 *       G723_S8 *ps8Line
 *       FILE     *Fp
 *
 * Global Variables:   None
 *
 * Range Issues: None
 *
 * Special Issues: None
 *
 *****************************************************************************/
G723_Void vLineWrite(G723_S8 *ps8Line, FILE *Fp)
{
    G723_S16  s16Info;
    G723_S32  s32Size;
    s16Info = ps8Line[0] & (G723_S16)0x0003 ;

    /* Check frame type and rate informations */
    switch(s16Info)
    {
        case 0x0002 :    /* SID frame */
            s32Size  = 4;
            break;

        case 0x0003 :    /* untransmitted silence frame */
            s32Size  = 1;
            break;

        case 0x0001 :    /* active frame, low rate */
            s32Size  = 20;
            break;

        default :        /* active frame, high rate */
            s32Size  = 24;
    }
#ifdef OUTPUT_DUMP
    fwrite(ps8Line, s32Size , 1, Fp);
#endif
}

/*****************************************************************************
 *
 * Function:  vReadlbc
 *
 * Description:  Read in a file
 *
 * Returns:  None
 *
 * Arguments:
 *       G723_S16 *ps16Dpnt
 *       G723_S32 s32Len
 *       FILE *Fp
 *
 * Global Variables:   None
 *
 * Range Issues: None
 *
 * Special Issues: None
 *
 *****************************************************************************/

G723_Void vReadlbc(G723_S16 *ps16Dpnt, G723_S32 s32Len, FILE *Fp )
{
    G723_S32 s32i;

    for(s32i = 0; s32i <s32Len; s32i++)
        ps16Dpnt[s32i] = (G723_S16)0;

    fread((G723_S8 *)ps16Dpnt, sizeof(G723_S16), s32Len, Fp);

#ifdef G723_BIG_ENDIAN
    vSwapBytes(ps16Dpnt, s32Len);
#endif
    return;
}

/*****************************************************************************
 *
 * Function:  eG723EProcessCmdLineOptions
 *
 * Description:  This function processes command line options.
 *
 * Returns: eG723ReturnType
 *
 * Arguments:
 *             ppInpFile -> pointer to pointer to Input File
 *             ppOutFile -> pointer to pointer to Output File
 *            ppRateFile -> pointer to pointer to Rate File
 *               s32Argc -> number of command line arguments
 *               ps8Argv -> pointer to command line arguments
 *       ps32UseHighPass -> pointer to UseHighPass flag
 *            ps32UseVAD -> pointer to UseVad flag
 *             ps32Quiet -> pointer to Quiet flag
 *              ps32Rate -> pointer to Rate flag
 *
 * Global Variables:  None
 *
 * Range Issues: None
 *
 * Special Issues: None
 *
 *****************************************************************************/
 G723_S8  *ps8InpFileName = G723_NULL;
G723_S8  *ps8OutFileName = G723_NULL;
G723_S32  s32G723EProcessCmdLineOptions(
		FILE **ppInpFile, FILE **ppOutFile, FILE **ppRateFile,
		G723_S32 s32Argc, G723_S8 *ps8Argv[],
		G723_S32 *ps32UseHighPass, G723_S32 *ps32UseVAD, G723_S32 *ps32Quiet,
		G723_S32 *ps32Rate)
{
    G723_S32 Flen ;        
    G723_S8  *ps8RateFileName = G723_NULL;

	/* Process encoding argument*/
	if (s32Argc < 3)
	{
		vDisplayUsage();
	}
	else
	{
		*ppInpFile = G723_NULL;        /* Input file */
		*ppOutFile = G723_NULL;        /* Output file */
		*ppRateFile = G723_NULL;       /* Rate file */
		*ps32Quiet = G723_FALSE; /* Default: Verbose Mode */
		*ps32Rate = E_G723E_BITRATE_63;  /* Default is 6.3 kbit/s */
		*ps32UseVAD = E_G723_VAD_DISABLE;  /* Default: Disable VAD */
		*ps32UseHighPass = E_G723E_HPFILTER_ENABLE; /* Default: Enable */

		while (s32Argc > 1 && ps8Argv[1][0] == '-')
		{

#ifdef TIME_PROFILE_RVDS
            if (mystrcmp(ps8Argv[1], "-rate") == 0)
#else
			if (strcmp(ps8Argv[1], "-rate") == 0)
#endif
			{
				if(s32Argc > 2)
				{
#ifdef TIME_PROFILE_RVDS
					if (!mystrcmp("63", ps8Argv[2]) )
#else
					if (!strcmp("63", ps8Argv[2]) )
#endif
					{
						*ps32Rate = E_G723E_BITRATE_63;
					}
#ifdef TIME_PROFILE_RVDS
					else if (!mystrcmp("53", ps8Argv[2]))
#else
					else if (!strcmp("53", ps8Argv[2]))
#endif
					{
						*ps32Rate = E_G723E_BITRATE_53;
					}
					else
					{
						ps8RateFileName = ps8Argv[2];
					}
				}
				else
				{
					vDisplayUsage();
				}

				/* Update argc/argv to next valid option/argument */
				ps8Argv+=2;
				s32Argc-=2;
			}

#ifdef TIME_PROFILE_RVDS
			else if (mystrcmp(ps8Argv[1], "-DTX") == 0)
#else
			else if (strcmp(ps8Argv[1], "-DTX") == 0)
#endif
			{
				*ps32UseVAD = E_G723_VAD_ENABLE;

				/* Move argv over the option to the next argument */
				ps8Argv++;
				s32Argc--;
			}
#ifdef TIME_PROFILE_RVDS
			else if (mystrcmp(ps8Argv[1], "-Noh") == 0)
#else
			else if (strcmp(ps8Argv[1], "-Noh") == 0)
#endif
			{
				*ps32UseHighPass = E_G723E_HPFILTER_DISABLE;

				/* Move argv over the option to the next argument */
				ps8Argv++;
				s32Argc--;
			}

#ifdef TIME_PROFILE_RVDS
			else if (mystrcmp(ps8Argv[1], "-q") == 0)
#else
			else if (strcmp(ps8Argv[1], "-q") == 0)
#endif
			{
				*ps32Quiet = G723_TRUE;

				/* Move argv over the option to the next argument */
				ps8Argv++;
				s32Argc--;
			}

#ifdef TIME_PROFILE_RVDS
			else if (mystrcmp(ps8Argv[1], "-?") == 0 || mystrcmp(ps8Argv[1], "-help") == 0)
#else
			else if (strcmp(ps8Argv[1], "-?") == 0 || strcmp(ps8Argv[1], "-help") == 0)
#endif
			{
				/* Print help */
				vDisplayUsage();
			}
			else
			{
#ifdef G723_DEBUG_PRINT
				fprintf(stderr, "ERROR! Invalid option \"%s\" in command line\n\n",
					ps8Argv[1]);
#endif
				vDisplayUsage();
			}
		}
	}

	if(s32Argc == 3)
	{
		ps8InpFileName = ps8Argv[1];
		ps8OutFileName = ps8Argv[2];
	}
	else
	{
		vDisplayUsage();
	}

    *ppInpFile = fopen(ps8InpFileName, "rb") ;
    if ( *ppInpFile == G723_NULL ) {
#ifdef G723_DEBUG_PRINT
        fprintf(stderr, "Invalid input file name: %s\n", ps8InpFileName) ;
#endif
        exit(1) ;
    }

    if (*ps32Quiet == G723_FALSE)
	{
#ifdef G723_DEBUG_PRINT
        printf("Input  file:     %s\n", ps8InpFileName) ;
#endif
	}

    *ppOutFile = fopen(ps8OutFileName, "wb") ;
    if ( *ppOutFile == G723_NULL )
	{
#ifdef G723_DEBUG_PRINT
        fprintf(stderr, "Can't open output file: %s\n", ps8OutFileName) ;
#endif
        exit(1) ;
    }

    if (*ps32Quiet == G723_FALSE)
	{
#ifdef G723_DEBUG_PRINT
        printf("Output file:     %s\n", ps8OutFileName) ;
#endif
	}

    if (ps8RateFileName)
	{
        *ppRateFile = fopen(ps8RateFileName, "rb" ) ;
        if ( *ppRateFile == G723_NULL ) {
#ifdef G723_DEBUG_PRINT
            fprintf(stderr, "Can't open Rate file: %s\n", ps8RateFileName);
#endif
            exit(1) ;
        }
        if (*ps32Quiet == G723_FALSE)
        {
#ifdef G723_DEBUG_PRINT
            printf("Rate   file:     %s\n", ps8RateFileName );
#endif
        }
    }
#ifdef G723_DEBUG_PRINT
    /* Options report */
    if (*ps32Quiet == G723_FALSE)
	{
        printf("Options:\n");
        printf("Encoder\n");

        if(ps8RateFileName == G723_NULL)
		{
            if (*ps32Rate == E_G723E_BITRATE_63)
			{
                printf("Rate 6.3 kb/s\n");
			}
            else
			{
                printf("Rate 5.3 kb/s\n");
			}
        }
        if (*ps32UseHighPass == E_G723E_HPFILTER_ENABLE)
		{
            printf("Highpassfilter enabled\n");
		}
        else
		{
            printf("Highpassfilter disabled\n");
		}
        if (*ps32UseVAD == E_G723_VAD_DISABLE)
		{
            printf("VAD/CNG disabled\n");
		}
        else
		{
            printf("VAD/CNG enabled\n");
		}
    }
#endif
    /*
      Compute the file length
    */
    fseek(*ppInpFile, 0L, SEEK_END ) ;
    Flen = ftell(*ppInpFile) ;
#ifdef __WINCE
	fseek ( *ppInpFile , 0L , SEEK_SET );
#else
    rewind(*ppInpFile) ;
#endif

    Flen /= sizeof(G723_S16)*G723_L_FRAME ;


	return Flen;
}

/*****************************************************************************
 * Function: eG723EEncodeExit
 *
 * Description: This fucntions deallocates all dynamically allocated memory
 *
 * Returns: eG723ReturnType
 *
 * Arguments:
 *                InpFile -> pointer to input file
 *                OutFile -> pointer to output file
 *               RateFile -> pointer to Rate file
 *            psEncConfig -> pointer to encoder config structure
 *              ps16InBuf -> pointer to input speech buffer
 *             ps16OutBuf -> pointer to output buffer
 *
 * Global Variables: gpfFileIn
 *                   gpfFileOut
 *
 * Range Issues: None
 *
 * Special Issues: None
 *
 *****************************************************************************/
eG723EReturnType eG723EEncodeExit(
                    FILE *InpFile,
					FILE *OutFile,
					FILE *RateFile,
					sG723EEncoderConfigType *psEncConfig,
					G723_S16 *ps16InBuf,
					G723_S16 *ps16OutBuf
					)
{
    G723_S16 s16Counter;

    if (ps16InBuf != G723_NULL)
    {
		/* free input buffer */
        mem_free (ps16InBuf);
        ps16InBuf = G723_NULL;
    }

    if (ps16OutBuf != G723_NULL)
    {
		/* free output buffer */
        mem_free (ps16OutBuf);
        ps16OutBuf = G723_NULL;
    }

    /* Free all the memory allocated for encoder config param*/
    for (s16Counter=0; s16Counter<psEncConfig->sG723EMemInfo.s32G723ENumMemReqs; s16Counter++)
    {
        if ((psEncConfig->sG723EMemInfo.asMemInfoSub[s16Counter].pvAPPEBasePtr) != G723_NULL)
        {
            mem_free ((psEncConfig->sG723EMemInfo.asMemInfoSub[s16Counter].pvAPPEBasePtr));
            (psEncConfig->sG723EMemInfo.asMemInfoSub[s16Counter].pvAPPEBasePtr) = G723_NULL;
        }
    }

    if (psEncConfig != G723_NULL)
    {
        mem_free (psEncConfig);
    }

	if(InpFile)
	{
		fclose(InpFile);
	}

	if(OutFile)
	{
		fclose(OutFile);
	}

	if(RateFile)
	{
		fclose(RateFile);
	}

    return E_G723E_OK;
}

/*****************************************************************************
 * Function:  main
 *
 * Description: main function. Function is defined in application space.
 *
 * Returns: G723_SUCCESS/G723_FAILURE
 *
 * Arguments:   s32Argc
 *              ps8Argv
 *
 * Global Variables: None
 *
 * Range Issues: None
 *
 * Special Issues: None
 *
 *****************************************************************************/
G723_S32 main(G723_S32 s32Argc, G723_S8 *psArgv[])
{
    eG723EReturnType eRetVal;
    G723_S16        *ps16InBuf;
    G723_S16        *ps16OutBuf;
    G723_S32        s32i;
    G723_S32        s32NumMemReqs;
    sG723EMemAllocInfoSubType *psMem;
    sG723EEncoderConfigType *psEncConfig;
    FILE             *InpFile, *OutFile, *RateFile;
    G723_S32        s32UseHighPass, s32UseVx, s32Quiet, s32Rate;
    G723_S32        FlLen, FrCnt;
    G723_S8         Rate_Rd;
    const char *version;
#ifdef MEASURE_STACK_USAGE
    int           *ps32SP, *ps32BaseSP;
    int           s32StackCount, s32StackValue;
    int           s32PeakStack = 0;
#endif

#ifdef MEASURE_HEAP_USAGE
    int      s32TotalMallocBytes=0;
#endif

#ifdef TIME_PROFILE_RVDS
    int     s32Frame = 0;
    int     prev_clk, curr_clk, clk;
    long    total_clk = 0;
    int     frame_min_clk = 0x7fffffff;
    int     frame_max_clk = 0;
    int     frame_min_no = 0;
    int     frame_max_no = 0;
    FILE    *pfCycleCount;
    unsigned char chname[] = "[PROFILE-INFO]";
    pfCycleCount = fopen ("enc_cycle.txt", "a");
#endif
#ifdef TIME_PROFILE
    struct timeval tvProf;
    long timeBefore =0, timeAfter=0;
    int retVal = -1;
    int minFrameNumber=0, maxFrameNumber=0, numFrame =0 ,FrameNo=0;
    long minFrameTime=0x7FFFFFFF;
    long maxFrameTime =0, totalTime =0;
    long timeVal=0;
    unsigned char timeFlag =0;
    FILE    *fSysTime;
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
    int bit_rate = 0;
    double performance = 0.0;

#ifndef __SYMBIAN32__
    fSysTime = fopen ("../audio_performance.txt", "a");
#else
    fSysTime = fopen ("D:\\enc_sys_time.txt", "a");
#endif
#endif
#ifdef ARM_MIPS_TEST_WINCE
    FILE    *fSysTime;
    fSysTime = fopen ("g723_enc_sys_time.txt", "a");
    if(!fSysTime)
    {
		printf("\nUnable to open log file g723_enc_sys_time.txt\n");
		exit(1);
	}
#endif

    ps16InBuf = G723_NULL;
    ps16OutBuf = G723_NULL;
    FrCnt=0;
    version = G723_get_version_info();
    printf("Running %s\n",version);

    FlLen = s32G723EProcessCmdLineOptions(&InpFile, &OutFile, &RateFile, s32Argc, psArgv,
            &s32UseHighPass, &s32UseVx, &s32Quiet, &s32Rate);

    psEncConfig = (sG723EEncoderConfigType *)alloc_fast(sizeof(sG723EEncoderConfigType));

#ifdef G723_ENC_TABRELOC_TST
    init_tables();
    /* Fill up the Relocated data position */
    psEncConfig->pu8APPEInitializedDataStart = (G723_Void *)app_tables;
#else
	psEncConfig->pu8APPEInitializedDataStart =G723_NULL ;
#endif
    psEncConfig->pvG723EEncodeInfoPtr = G723_NULL;

    /* Query for memory */
    eRetVal = eG723EQueryMem(psEncConfig);

    if(eRetVal != E_G723E_OK)
    {
        eG723EEncodeExit(InpFile, OutFile, RateFile, psEncConfig, ps16InBuf, ps16OutBuf);
        return G723_FAILURE;
    }

    /* Number of memory chunk requests by the encoder */
    s32NumMemReqs = psEncConfig->sG723EMemInfo.s32G723ENumMemReqs;

    /* Allocae memory requested by the encoder */
    for(s32i = 0; s32i < s32NumMemReqs; s32i++)
    {
        psMem = &(psEncConfig->sG723EMemInfo.asMemInfoSub[s32i]);
        if(psMem->u8G723EMemTypeFs == G723_FAST_MEMORY)
        {
            /* Check for priority and memory description can be added here */
            psMem->pvAPPEBasePtr = alloc_fast(psMem->s32G723ESize);
			#ifdef MEASURE_HEAP_USAGE
			s32TotalMallocBytes +=  psMem->s32G723ESize ;
			#endif
        }
        else
        {
            psMem->pvAPPEBasePtr = alloc_slow(psMem->s32G723ESize);
			#ifdef MEASURE_HEAP_USAGE
			s32TotalMallocBytes +=  psMem->s32G723ESize ;
			#endif
        }
    }

    /* Initialize the G723.1 encoder */
    eRetVal = eG723EEncodeInit(psEncConfig);
    if(eRetVal != E_G723E_OK)
    {
        eG723EEncodeExit(InpFile, OutFile, RateFile, psEncConfig, ps16InBuf, ps16OutBuf);
        return G723_FAILURE;
    }

    if((ps16InBuf = alloc_fast(G723_L_FRAME * sizeof(G723_S16))) == G723_NULL)
    {
        eG723EEncodeExit(InpFile, OutFile, RateFile, psEncConfig, ps16InBuf, ps16OutBuf);
        return G723_FAILURE;
    }

    if((ps16OutBuf = alloc_fast((CODED_FRAMESIZE/2) * sizeof(G723_S16))) == G723_NULL)
    {
        eG723EEncodeExit(InpFile, OutFile, RateFile, psEncConfig, ps16InBuf, ps16OutBuf);
        return G723_FAILURE;
    }

    if(RateFile == G723_NULL)
    {
        psEncConfig->u8APPEBitRate =(G723_U8) s32Rate;
    }

    do
    {
        /* Process all the input file */
        if(RateFile != G723_NULL)
        {
            fread((G723_S8 *)&Rate_Rd, sizeof(G723_S8), 1, RateFile);
            psEncConfig->u8APPEBitRate =
                (G723_U8)((Rate_Rd == 0)?E_G723E_BITRATE_63:E_G723E_BITRATE_53);
        }

        vReadlbc(ps16InBuf, G723_L_FRAME, InpFile);

        psEncConfig->u8APPEVADFlag = (G723_U8)s32UseVx;
        psEncConfig->u8APPEHighPassFilter = (G723_U8)s32UseHighPass;

#ifdef MEASURE_STACK_USAGE
				PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
#endif
#ifdef TIME_PROFILE_RVDS
        s32Frame++;
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
				FrameNo++;
                retVal = gettimeofday(&tvProf, 0);
                if (retVal == 0)
                {
                    timeBefore = tvProf.tv_sec * 1000000 + tvProf.tv_usec;
                    timeFlag = 1;
                }
#endif
#ifdef TIME_PROFILE_RVDS_ARM9
		dummyCall1();
#endif
#ifdef	ARM_MIPS_TEST_WINCE						/* for taking timing on wince platform*/
		INTERRUPTS_SET(0xDF);  						/*disable interrupt*/
		Flag=QueryPerformanceFrequency(&lpFrequency);
		Flag=QueryPerformanceCounter(&lpPerformanceCountBegin);
#endif


        eRetVal = eG723EEncodeFrame(psEncConfig, ps16InBuf, ps16OutBuf);
        if (eRetVal != E_G723E_OK)
        {
            /* free all the dynamic memory and exit */
            eG723EEncodeExit(InpFile, OutFile, RateFile, psEncConfig, ps16InBuf, ps16OutBuf);
            exit (-1);
        }
#ifdef TIME_PROFILE_RVDS_ARM9
		dummyCall2();
#endif


#ifdef TIME_PROFILE_RVDS
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
                    retVal = gettimeofday(&tvProf, 0);
                    if (retVal == 0)
                    {
                        timeAfter = tvProf.tv_sec * 1000000 + tvProf.tv_usec;
                        timeVal = timeAfter - timeBefore;

                        numFrame++;
                        if (timeVal > maxFrameTime)
                        {
                            maxFrameTime = timeVal;
                            maxFrameNumber = FrameNo;
                        }
                        else if (timeVal < minFrameTime)
                        {
                            minFrameTime = timeVal;
                            minFrameNumber = FrameNo;
                        }
                        totalTime += timeVal;
                    }
                    timeFlag = 0;
                }
#endif

#ifdef ARM_MIPS_TEST_WINCE
                Flag=QueryPerformanceCounter(&lpPerformanceCountEnd);
                Temp=(((lpPerformanceCountEnd.QuadPart - lpPerformanceCountBegin.QuadPart)*1000000)/(lpFrequency.QuadPart));/*this is the duration*/
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

#ifdef MEASURE_STACK_USAGE
        GET_STACK_USAGE (ps32BaseSP, ps32SP, s32StackCount, s32StackValue);
		if (s32PeakStack < s32StackValue)
            s32PeakStack = s32StackValue;
#endif
        vLineWrite((G723_S8 *)ps16OutBuf, OutFile ) ;
        FrCnt ++ ;

#ifdef G723_DEBUG_PRINT
        if(!s32Quiet)
        {
            fprintf( stdout, "Done : %6ld %3ld\r", FrCnt, FrCnt*100/FlLen ) ;
            fflush(stdout);
        }
#endif
    }  while ( FrCnt < FlLen ) ;

    eG723EEncodeExit(InpFile, OutFile, RateFile, psEncConfig, ps16InBuf, ps16OutBuf);

#ifdef TIME_PROFILE_RVDS

	#ifdef MEASURE_STACK_USAGE
	#ifdef MEASURE_HEAP_USAGE
		printf("\n\n %s\t%s\t%d\t%ld\t%ld\t%d\t%d\t%d\t%ld\t%ld\t%ld\t \n\n",chname,psArgv[3],Rate_Rd,frame_max_clk * 64,frame_min_clk * 64,s32Frame,frame_max_no,frame_min_no,total_clk * 64,s32PeakStack,s32TotalMallocBytes);
	#endif
	#endif

	#ifndef MEASURE_STACK_USAGE
	#ifndef MEASURE_HEAP_USAGE
		printf("\n\n %s\t%s\t%d\t%ld\t%ld\t%d\t%d\t%d\t%ld\t \n\n",chname,psArgv[3],Rate_Rd,\
						frame_max_clk * 64,frame_min_clk * 64,s32Frame,frame_max_no,frame_min_no,total_clk * 64);
	#endif
	#endif

	fprintf(pfCycleCount,"InputFile= %s  OutputFile= %s RateFile= %s\n",psArgv[4],psArgv[5],psArgv[2]);
    fprintf(pfCycleCount, "Peak   Frame no: %10ld, Peak    clk: %10ld, Peak    MCPS: %5.2f\n",
            frame_max_no, frame_max_clk * 64, (float)(frame_max_clk * 64.0 / 30.0 / 1000));
    fprintf(pfCycleCount, "Min    Frame no: %10ld, Min     clk: %10ld, Min     MCPS: %5.2f\n",
            frame_min_no, frame_min_clk * 64, (float)(frame_min_clk * 64.0 / 30.0 / 1000));
    fprintf(pfCycleCount, "Total  Frames  : %10ld, Average clk: %10ld, Average MCPS: %5.2f\n",
            s32Frame, s32Frame?(total_clk / s32Frame * 64):0, (float)s32Frame?(float)(total_clk / s32Frame * 64.0 / 30.0 / 1000):(float)0);
    fprintf(pfCycleCount, "-------------------------------------------------------------------------\n\n");
    fclose(pfCycleCount);
	fprintf(stdout,"\nEncoder");

#endif

#ifdef TIME_PROFILE
	fprintf(stdout,"\nEncoder");
	fprintf(stdout,"\n%s\t%s\t%d\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname,psArgv[3],Rate_Rd,
	                    maxFrameTime,minFrameTime,numFrame,maxFrameNumber, minFrameNumber,totalTime);
      	fprintf (fSysTime,"\n|\tAudio Format\t|\tPlat\t|\tSamplerate\t|\tCh\t|\tBiterate\t|\tBit\t|\tPerf(MIPS)\t|\tAudio file name\n");
		
	performance = (double)totalTime * 0.000001*cpu_freq*8000/numFrame/G723_L_FRAME;
	
	if(E_G723E_BITRATE_63 == psEncConfig->u8APPEBitRate)
		bit_rate = 6300;
	else if(E_G723E_BITRATE_53 == psEncConfig->u8APPEBitRate)
		bit_rate = 5300;
	
       fprintf(fSysTime, "|\tG.723.1 Enc \t|\tARM%d\t|\t%9d\t|\t%d\t|\t%8d\t|\t%d\t|\t%.8f\t|%s\n", platform, 8000, 1, bit_rate, 16, performance, ps8OutFileName);

//         fprintf(fSysTime,"\n%s\t%s\t%d\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname,psArgv[3],Rate_Rd,
//	                    maxFrameTime,minFrameTime,numFrame,maxFrameNumber, minFrameNumber,totalTime);
	/*fprintf(fSysTime,"InputFile= %s  OutputFile= %s RateFile= %s\n",psArgv[4],psArgv[5],psArgv[2]);
    fprintf(fSysTime,"\tPeak Frame no: %ld,  Peak Time: %ld\n", maxFrameNumber, maxFrameTime);
    fprintf(fSysTime,"\tMin  Frame no: %ld,  Min Time: %ld\n", minFrameNumber, minFrameTime);
    fprintf(fSysTime,"\tTotal Frame : %ld, Average Time: %ld\n", (FrameNo*64), FrameNo?totalTime/numFrame:0);
    fprintf(fSysTime, "----------------------------------------\n\n");*/
    fclose(fSysTime);
#endif
#ifdef __SYMBIAN32__
#ifdef MEASURE_HEAP_USAGE
			printf("\n Stack and Heap are  %d\t%d\t",s32PeakStack,s32TotalMallocBytes);
#endif
#endif
#ifdef	ARM_MIPS_TEST_WINCE
	printf("\nDencoder");
	printf("\n%s\t%s\t%ld",chname,gps8InpFileName,maxFrameTime);
	printf("\t%ld",minFrameTime);
	printf("\t%d\t%d\t%d\t%ld\t",numFrame,maxFrameNumber,minFrameNumber,totalTime);
	fprintf(fSysTime,"\n%s\t%s\t%ld",chname,gps8InpFileName,maxFrameTime);
	fprintf(fSysTime,"\t%ld",minFrameTime);
	fprintf(fSysTime,"\t%d\t%d\t%d\t%ld\t",numFrame,maxFrameNumber,minFrameNumber,totalTime);
	fclose(fSysTime);
#endif
    return G723_SUCCESS;
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
#ifdef TIME_PROFILE_RVDS_ARM9
void dummyCall1(void)
{
	uiframe++;
}

void dummyCall2(void)
{
	uiframe++;
}
#endif


/**************************<END OF THE FILE>**********************************/
