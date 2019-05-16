/*
***********************************************************************
* Copyright (c) 2005-2009, 2012, Freescale Semiconductor, Inc.
* All modifications are confidential and proprietary information
* of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
***********************************************************************
*/

/*****************************************************************************
* File Name: g729_enctest.c
*
* Description: G.729AB encoder test wrapper is defined in this file.
*
***************************** Change History*********************************
*
*   DD/MMM/YYYY     Code Ver     Description      Author
*   -----------     --------     -----------      ------
*   15/Jul/2008     0.1          File created     Bing Song
*****************************************************************************/
/*****************************<INCLUDE_FILES BEGIN>***************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "g729_enc_api.h"
#include "g729_enctest.h"

#ifdef TIME_PROFILE
#include <time.h>
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

#define OCTET_TX_MODE

//#define TEST_BUF_SIZE	(0x100000)	/* 1M Byte; used for input test files */


/******************************<INCLUDE_FILES END>****************************/

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

G729_Void vDisplayUsage(G729_U8 *sExeFile)
{
	printf("Usage :%s speech_file  bitstream_file  VAD_flag\n", sExeFile);
	printf("\n");
	printf("Format for speech_file:\n");
	printf("  Speech is read from a binary file of 16 bits PCM data.\n");
	printf("\n");
	printf("Format for bitstream_file:\n");
	printf("  One (2-byte) synchronization word \n");
	printf("  One (2-byte) size word,\n");
	printf("  80 words (2-byte) containing 80 bits.\n");
	printf("\n");
	printf("VAD flag:\n");
	printf("  0 to disable the VAD\n");
	printf("  1 to enable the VAD\n");
}

/*****************************************************************************
* Function: eG729EEncodeExit
*
* Description: This fucntions deallocates all dynamically allocated memory
*
* Returns: eG729ReturnType
*
* Arguments:
*                f_speech -> pointer to input file
*                f_serial -> pointer to output file
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
eG729EReturnType eG729EEncodeExit(
								  FILE *f_speech,
								  FILE *f_serial,
								  sG729EEncoderConfigType *psEncConfig,
								  G729_S16 *ps16InBuf,
								  G729_S16 *ps16OutBuf
								  )
{
    G729_S16 s16Counter;

    if (ps16InBuf != G729_NULL)
    {
		/* free input buffer */
        mem_free (ps16InBuf);
        ps16InBuf = G729_NULL;
    }

    if (ps16OutBuf != G729_NULL)
    {
		/* free output buffer */
        mem_free (ps16OutBuf);
        ps16OutBuf = G729_NULL;
    }

    /* Free all the memory allocated for encoder config param*/
    for (s16Counter=0; s16Counter<psEncConfig->sG729EMemInfo.s32G729ENumMemReqs; s16Counter++)
    {
        if ((psEncConfig->sG729EMemInfo.asMemInfoSub[s16Counter].pvAPPEBasePtr) != G729_NULL)
        {
            mem_free ((psEncConfig->sG729EMemInfo.asMemInfoSub[s16Counter].pvAPPEBasePtr));
            (psEncConfig->sG729EMemInfo.asMemInfoSub[s16Counter].pvAPPEBasePtr) = G729_NULL;
        }
    }

    if (psEncConfig != G729_NULL)
    {
        mem_free (psEncConfig);
    }

	if(f_speech)
	{
		fclose(f_speech);
	}

	if(f_serial)
	{
		fclose(f_serial);
	}

    return E_G729E_OK;
}

/* debug.c part  */
#ifdef DEBUG_G729

void filetw_function(char *filename, char *text)
{
	FILE *f1;
        char filestr[80];

	if ((f1 = fopen(filename, "a")) == NULL)
	{
		sprintf(filestr, "filetw: unable to open file, %s", filename);
		return;
	}

	if (fprintf(f1, "\n\"%s\"\n", text) < 0)
	{
		sprintf(filestr, "filetw: unable to write to file, %s", filename);
		return;
	}

	if (fclose(f1))
	{
		sprintf(filestr, "filetw: unable to close file, %s", filename);
		return;
	}
}

void filefw_function(char *filename, int *buf, int start, int count)
{
	FILE *f1;
	char filestr[80];

	if ((f1 = fopen(filename, "a")) == NULL)
	{
		sprintf(filestr, "filefw: unable to open file, %s", filename);
		return;
	}

	while (count--)
	{
		if (fprintf(f1, "%d\n", *buf++) < 0)
		{
			sprintf(filestr, "filefw: unable to write to file, %s", filename);
	    		return;
		}
		/*if (fprintf(f1, "%4d    % 8.8x\n", start++, *buf++) < 0)
		{
			sprintf(filestr, "filefw: unable to write to file, %s", filename);
	    		return;
		}*/
	}

	if (fclose(f1))
	{
		sprintf(filestr, "filefw: unable to close file, %s", filename);
		return;
	}
}

void filesw_function(char *filename, short *buf, int start, int count)
{
	FILE *f1;
	char filestr[80];

	if ((f1 = fopen(filename, "a")) == NULL)
	{
		sprintf(filestr, "filesw: unable to open file, %s", filename);
		return;
	}

	while (count--)
	{
		if (fprintf(f1, "%d\n", *buf++) < 0)
		{
			sprintf(filestr, "filesw: unable to write to file, %s", filename);
			return;
		}
		/*if (fprintf(f1, "%4d    %04X\n", start++, *buf++) < 0)
		{
			sprintf(filestr, "filesw: unable to write to file, %s", filename);
			return;
		}*/
	}

	if (fclose(f1))
	{
		sprintf(filestr, "filesw: unable to close file, %s", filename);
		return;
	}
}


void fileinit_function(char *filename,int useverbose,int * filecnt, char files[][128])
{
	int i;

	char backname[128];
	FILE *fptr;

#ifdef WINCE_BUILD
	_TCHAR  filename_word[128];
	_TCHAR  backname_word[128];
#endif
	for (i = 0; i < (*filecnt); i++)		/* check if file initialized */
	{
		if (strncmp(filename, files[i], strlen(filename)) == 0)
		{
			return;
		}
	}

/*	Initialize file */

	if ( (*filecnt) >= MAXDBGFILE)
	{
		/* error_msg("fileinit: debug file table out of space", FATAL);*/
	}
	else
	{
		strcpy(files[(*filecnt)], filename);

		(* filecnt) =(*filecnt)  + 1;

		if ((fptr = fopen(filename, "r")) != NULL)	/* check if file exists */
		{
			fclose(fptr);
			strcpy(backname, filename);				/* create backup fname  */
			backname[strlen(backname) - 1] = 'k';
			if ((fptr = fopen(backname, "r")) != NULL)/* check back file    */
			{
				fclose(fptr);						/* delete old backfile  */
				if (useverbose)
				{
					/*fprintf(stderr, "Deleting %s.\n",backname);*/
				}
#ifdef WINCE_BUILD
				i=0;
				while(backname[i] != '\0')
				{
					backname_word[i]=(_TCHAR)backname[i];
					i++;
				}
				backname_word[i]=(_TCHAR)'\0';

				DeleteFile(backname_word);
#else
				remove(backname);
#endif
			}									/* rename file to backfile */
			if (useverbose)
			{
				/*fprintf(stderr, "Renaming %s to %s.\n", filename, backname);*/
			}
#ifdef WINCE_BUILD
			i=0;
			while(backname[i] != '\0')
			{
				backname_word[i]=(_TCHAR)backname[i];
				i++;
			}
			backname_word[i]=(_TCHAR)'\0';

			i=0;
			while(filename[i] != '\0')
			{
				filename_word[i]=(_TCHAR)filename[i];
				i++;
			}
			filename_word[i]=(_TCHAR)'\0';


			MoveFile(filename_word, backname_word);
#else
			rename(filename, backname);
#endif
		}
	}
}

#endif /* DEBUG_G729 */

#ifdef TIME_PROFILE_RVDS_ARM9
void    dummyCall1(void);
void    dummyCall2(void);
G729_S32  uiframe;
#endif

/*****************************************************************************
* Function:  main
*
* Description: main function. Function is defined in application space.
*
* Returns: G729_SUCCESS/G729_FAILURE
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
G729_S32 main(G729_S32 s32Argc, G729_S8 *psArgv[])
{
    eG729EReturnType eRetVal;
    G729_S16        *ps16InBuf, *ps16InputFileBuf;
    G729_S16        *ps16OutBuf;
    G729_S32        s32i;
    G729_S32        s32NumMemReqs;
    sG729EMemAllocInfoSubType *psMem;
    sG729EEncoderConfigType *psEncConfig;
    G729_S32        FrCnt;
	FILE *f_speech = G729_NULL;               /* File of speech data                   */
	FILE *f_serial = G729_NULL;               /* File of serial bits for transmission  */
    G729_S32 TotalLen;
    G729_S8 *ps8InputFileBuf,*ps8InFlBufBak;
	/* For G.729B */
#ifdef OUTPUT_DUMP
	G729_S16 nb_words;
#endif
	G729_S16 vad_enable;

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
    int bit_rate = 8000;
    double performance = 0.0;
    fSysTime = fopen ("../audio_performance.txt", "a");
    if(!fSysTime)
    {
        printf("open ../audio_performance.txt file failed!");
    }
#endif
#ifdef ARM_MIPS_TEST_WINCE        		 /* for taking timing on wince platform*/
	G729_S8    *gps8InpFileName;
	G729_S8    *gps8OutFileName;
	extern void INTERRUPTS_SET(int k);
	unsigned int	Flag;
	LARGE_INTEGER	lpFrequency;
	LARGE_INTEGER	lpPerformanceCountBegin;
	LARGE_INTEGER	lpPerformanceCountEnd;
	__int64 Temp;
	FILE	*fSysTime = fopen ("enc_sys_time.txt", "a");
	__int64 totalTime = 0;
	unsigned int	numFrame = 0;
	__int64 minFrameTime = 0x7FFFFFFFFFFFFFFF;
	__int64 maxFrameTime = -1;
	unsigned int	minFrameNumber = 0;
	unsigned int	maxFrameNumber = 0;
	unsigned char	chname[] = "[PROFILE-INFO]";
#endif


    ps16InBuf = G729_NULL;
    ps16OutBuf = G729_NULL;
    FrCnt=0;

	/* Output the G.729AB Encoder Version Info */
    printf("%s \n", s8G729VersionInfo());

    if (s32Argc < 4)
    {
        printf(" The number of input parameters less than 3!!\n");
        vDisplayUsage(psArgv[0]);
        exit(0);
    } else if (s32Argc > 4)
    {
        printf(" Two many input parameters!!\n");
        printf(" The last %d parameters are ignored!!\n", s32Argc-4);
        vDisplayUsage(psArgv[0]);
    }

	if ( (f_speech = fopen(psArgv[1], "rb")) == NULL) {
		printf("%s - Error opening file  %s !!\n", psArgv[0], psArgv[1]);
		vDisplayUsage(psArgv[0]);
		exit(0);
	}
	printf(" Input speech file:  %s\n", psArgv[1]);

#ifdef OUTPUT_DUMP
	if ( (f_serial = fopen(psArgv[2], "wb")) == NULL) {
		printf("%s - Error opening file  %s !!\n", psArgv[0], psArgv[2]);
		vDisplayUsage(psArgv[0]);
		exit(0);
	}
	printf(" Output bitstream file:  %s\n", psArgv[2]);
#endif

	vad_enable = (G729_S16)atoi(psArgv[3]);
	if (vad_enable == 1)
		printf(" VAD enabled\n");
	else
		printf(" VAD disabled\n");

#ifndef OCTET_TX_MODE
	printf(" OCTET TRANSMISSION MODE is disabled\n");
#endif

    psEncConfig = (sG729EEncoderConfigType *)alloc_fast(sizeof(sG729EEncoderConfigType));

    psEncConfig->pvG729EEncodeInfoPtr = G729_NULL;

    /* Query for memory */
    eRetVal = eG729EQueryMem(psEncConfig);

    if(eRetVal != E_G729E_OK)
    {
        eG729EEncodeExit(f_speech, f_serial, psEncConfig, ps16InBuf, ps16OutBuf);
        return G729_FAILURE;
    }

    /* Number of memory chunk requests by the encoder */
    s32NumMemReqs = psEncConfig->sG729EMemInfo.s32G729ENumMemReqs;

    /* Allocae memory requested by the encoder */
    for(s32i = 0; s32i < s32NumMemReqs; s32i++)
    {
        psMem = &(psEncConfig->sG729EMemInfo.asMemInfoSub[s32i]);
        if(psMem->u8G729EMemTypeFs == G729_FAST_MEMORY)
        {
            /* Check for priority and memory description can be added here */
            psMem->pvAPPEBasePtr = alloc_fast(psMem->s32G729ESize);
#ifdef MEASURE_HEAP_USAGE
			s32TotalMallocBytes +=  psMem->s32G729ESize ;
#endif
        }
        else
        {
            psMem->pvAPPEBasePtr = alloc_slow(psMem->s32G729ESize);
#ifdef MEASURE_HEAP_USAGE
			s32TotalMallocBytes +=  psMem->s32G729ESize ;
#endif
        }
    }

#ifdef  DEBUG_G729
	psEncConfig->filetw_func = filetw_function ;
	psEncConfig->filefw_func = filefw_function ;
	psEncConfig->filesw_func = filesw_function ;
	psEncConfig->fileinit_func = fileinit_function ;
	psEncConfig->sprintf_ext = sprintf;
	psEncConfig->debugdir ="./";
#endif

    /* Initialize the G729 encoder */
    eRetVal = eG729EEncodeInit(psEncConfig);
    if(eRetVal != E_G729E_OK)
    {
        eG729EEncodeExit(f_speech, f_serial, psEncConfig, ps16InBuf, ps16OutBuf);
        return G729_FAILURE;
    }

    if((ps16InBuf = alloc_fast(G729_L_FRAME * sizeof(G729_S16))) == G729_NULL)
    {
        eG729EEncodeExit(f_speech, f_serial, psEncConfig, ps16InBuf, ps16OutBuf);
        return G729_FAILURE;
    }

    if((ps16OutBuf = alloc_fast((G729_CODEC_SIZE) * sizeof(G729_S16))) == G729_NULL)
    {
        eG729EEncodeExit(f_speech, f_serial, psEncConfig, ps16InBuf, ps16OutBuf);
        return G729_FAILURE;
    }

	//ps8InputFileBuf = alloc_fast(TEST_BUF_SIZE);

	fseek(f_speech, 0, SEEK_END);
	TotalLen = ftell(f_speech);
	fseek(f_speech, 0, SEEK_SET);

    if((ps8InputFileBuf = alloc_fast(TotalLen)) == G729_NULL)
    {
        eG729EEncodeExit(f_speech, f_serial, psEncConfig, ps16InBuf, ps16OutBuf);
        printf("Too large input file: %d Bytes!\n", TotalLen);
        return G729_FAILURE;
    }

	//TotalLen = 0 - TotalLen;
	//fseek(f_speech,TotalLen,SEEK_END);
	//TotalLen = 0 - TotalLen;
	if((G729_S32)fread(ps8InputFileBuf,1,TotalLen,f_speech)!=TotalLen)
	{
        eG729EEncodeExit(f_speech, f_serial, psEncConfig, ps16InBuf, ps16OutBuf);
		mem_free(ps8InputFileBuf);
		exit(-1);
	}
	ps8InFlBufBak =  ps8InputFileBuf;
	ps16InputFileBuf = (G729_S16 *)ps8InputFileBuf;

	while( 1 )
    {
        /* Process all the input file */
		TotalLen -= G729_L_FRAME*2;
		if (TotalLen<0)
		{
			break;
		}
		for(s32i=0;s32i<G729_L_FRAME;s32i++)
		{
			ps16InBuf[s32i] = *ps16InputFileBuf++;
		}

        psEncConfig->u8APPEVADFlag = vad_enable;
#ifdef	ARM_MIPS_TEST_WINCE						/* for taking timing on wince platform*/
		INTERRUPTS_SET(0xDF);  						/*disable interrupt*/
		Flag=QueryPerformanceFrequency(&lpFrequency);
		Flag=QueryPerformanceCounter(&lpPerformanceCountBegin);

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
#ifdef TIME_PROFILE_RVDS_ARM9
		dummyCall1();
#endif
        eRetVal = eG729EEncodeFrame(psEncConfig, ps16InBuf, ps16OutBuf);

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

#ifdef MEASURE_STACK_USAGE
        GET_STACK_USAGE (ps32BaseSP, ps32SP, s32StackCount, s32StackValue);
		if (s32PeakStack < s32StackValue)
            s32PeakStack = s32StackValue;
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
        if (eRetVal != E_G729E_OK)
        {
            /* free all the dynamic memory and exit */
            eG729EEncodeExit(f_speech, f_serial, psEncConfig, ps16InBuf, ps16OutBuf);
            exit (-1);
        }

#ifdef OUTPUT_DUMP
		nb_words = ps16OutBuf[1]/16 +  (G729_S16)2;

		fwrite(ps16OutBuf, sizeof(G729_S16), nb_words, f_serial);
#endif

        FrCnt ++ ;

#ifdef G729_DEBUG_PRINT
        if(!s32Quiet)
        {
            fprintf( stdout, "Done : %6ld %3ld\r", FrCnt, FrCnt*100/FlLen ) ;
            fflush(stdout);
        }
#endif
    }

#ifdef TIME_PROFILE_RVDS

#ifdef MEASURE_STACK_USAGE
#ifdef MEASURE_HEAP_USAGE
	printf("\n\n %s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t%ld\t%ld\t \n\n",chname,psArgv[3],frame_max_clk * 64,frame_min_clk * 64,s32Frame,frame_max_no,frame_min_no,total_clk * 64,s32PeakStack,s32TotalMallocBytes);
#endif
#endif

#ifndef MEASURE_STACK_USAGE
#ifndef MEASURE_HEAP_USAGE
	printf("\n\n %s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t \n\n",chname,psArgv[3],\
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
	fprintf(stdout,"\n%s\t%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname,psArgv[1],psArgv[3],
		maxFrameTime,minFrameTime,numFrame,maxFrameNumber, minFrameNumber,totalTime);

      	fprintf (fSysTime,"\n|\tAudio Format\t|\tPlat\t|\tSamplerate\t|\tCh\t|\tBiterate\t|\tBit\t|\tPerf(MIPS)\t|\tAudio file name\n");
	performance = (double)totalTime * 0.000001*cpu_freq*8000/numFrame/G729_L_FRAME;
       fprintf(fSysTime, "|\tG729 Encoder\t|\tARM%d\t|\t%9d\t|\t%d\t|\t%8d\t|\t%d\t|\t%.8f\t|%s\n", platform, 8000, 1, bit_rate, 16, performance, psArgv[2]);
//	fprintf(fSysTime,"\n%s\t%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname,psArgv[1],psArgv[3],
//		maxFrameTime,minFrameTime,numFrame,maxFrameNumber, minFrameNumber,totalTime);
		/*fprintf(fSysTime,"InputFile= %s  OutputFile= %s RateFile= %s\n",psArgv[4],psArgv[5],psArgv[2]);
		fprintf(fSysTime,"\tPeak Frame no: %ld,  Peak Time: %ld\n", maxFrameNumber, maxFrameTime);
		fprintf(fSysTime,"\tMin  Frame no: %ld,  Min Time: %ld\n", minFrameNumber, minFrameTime);
		fprintf(fSysTime,"\tTotal Frame : %ld, Average Time: %ld\n", (FrameNo*64), FrameNo?totalTime/numFrame:0);
    fprintf(fSysTime, "----------------------------------------\n\n");*/
    fclose(fSysTime);
#endif

#ifdef	ARM_MIPS_TEST_WINCE
	printf("\nEncoder");
	printf("\n%s\t%s\t%s\t%ld",chname,psArgv[1],psArgv[3],maxFrameTime);
	printf("\t%ld",minFrameTime);
	printf("\t%d\t%d\t%d\t%ld\t",numFrame,maxFrameNumber,minFrameNumber,totalTime);
	fprintf(fSysTime,"\n%s\t%s\t%s\t%ld",chname,psArgv[1],psArgv[3],maxFrameTime);
	fprintf(fSysTime,"\t%ld",minFrameTime);
	fprintf(fSysTime,"\t%d\t%d\t%d\t%ld\t",numFrame,maxFrameNumber,minFrameNumber,totalTime);
	fclose(fSysTime);
#endif

    eG729EEncodeExit(f_speech, f_serial, psEncConfig, ps16InBuf, ps16OutBuf);
    if(G729_NULL!=ps8InFlBufBak)
	{
		mem_free(ps8InFlBufBak);
		ps8InFlBufBak = G729_NULL;
	}

    return G729_SUCCESS;
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
