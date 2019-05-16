/*
 ***********************************************************************
 * Copyright (c) 2005-2010, 2012, Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*==================================================================================================

    Module Name:  gG711_decode_test.c

    General Description: test the G711 decoder.

====================================================================================================

Revision History:
                            Modification     Tracking
Author                          Date          Number     Description of Changes
-------------------------   ------------    ----------   -------------------------------------------
Vineet Golchha              Apr/20/2006        1.0          Initial Version
Neha Srivastava             24/01/2007         1.1      Elinux Ported Version
Satish K Singh              Sep/12/2007        1.2       Modified for CARMEL VRTX
Qiu Cunshou                 May/22/2008     engr77040    Add API for version
====================================================================================================
                                        INCLUDE FILES
==================================================================================================*/
#ifdef __WINCE
#include <windows.h>
#include <winbase.h>
#endif
#include <stdio.h>
#include <string.h>
#include "g711_dec_api.h"

#ifdef TIME_PROFILE
#include <sys/time.h>
#endif

#ifdef MEASURE_STACK_USAGE
#include "mm_codec_common.h"
#endif

#define OUTPUT_DUMP
#if defined(TIME_PROFILE) || defined(TIME_PROFILE_RVDS) || defined(ARM_MIPS_TEST_WINCE)
#undef OUTPUT_DUMP
#endif
/*==============================<INCLUDE_FILES END>===============================================*/

/*==================================================================================================
                                     LOCAL CONSTANTS
==================================================================================================*/

#define BUFFER_SIZE     64
#define G711_ARGUMENTS_OK 0
#define G711_ARGUMENTS_FAIL !G711_ARGUMENTS_OK
#include "process_cmdline.h"
int frame_no;

#ifdef	READ_FULL_INPUTFILE
#define TIME_PROFILE_BUFFER_SIZE    100*1024
#endif

/*==================================================================================================
                                     LOCAL FUNCTIONS
==================================================================================================*/


#ifdef BIG_ENDIAN
   static void dec_vSwapBytes(short *ps16Buf, int s32Cnt);
#endif

/************************************ TEST CODE PARAMETERS *************************************/

/***********************************************************************************************/

/*==================================================================================================
                                     GLOBAL VARIABLES
==================================================================================================*/




#ifdef TIME_PROFILE
    struct timeval StartTime,EndTime;
    long time_before =0, time_after=0;
    int retVal = -1;
    int minFrameNumber=0, maxFrameNumber=0;
    unsigned int numFrame = 0;
    long minFrameTime=0x7FFFFFFF;
    long maxFrameTime =0, totalTime =0;
    long timediff=0;
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
    double performance = 0.0;
#endif

#ifdef TIME_PROFILE_RVDS
    long     prev_clk, curr_clk, clk;
    long     total_clk = 0;
    unsigned int     frame_min_clk = 0xffffffff;
    long     frame_max_clk = 0;
    long     frame_min_no = 0;
    long     frame_max_no = 0;
    unsigned int nframe=0;
    unsigned char chname[] = "[PROFILE-INFO]";
    extern int prev_cycles(void);
    extern int curr_cycles(void);
#endif

#ifdef DUMMYCALL
    unsigned int uiframe;

void dummyCall1(void)
{
    uiframe++;
}

void dummyCall2(void)
{
    uiframe++;
}
#endif

#ifdef	MEASURE_STACK_USAGE
	int index_temp;
	int stack_usage=0;
	int *stack_ptr=NULL;
	int *base_ptr=NULL;
	int peak_stack_usage=0;
	unsigned int stack_ptr_value=0;
#endif


#ifdef MEASURE_HEAP_USAGE
  int peak_heap_usage=0;
#endif


#ifdef ARM_MIPS_TEST_WINCE        		 /* for taking timing on wince platform*/
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

/*
#if defined(TIME_PROFILE_RVDS) || defined(TIME_PROFILE)
    FILE    *pfCycleCount = 0;
#endif
*/
    void g711decodeTest(FILE *pInFile, FILE *pOutFile, unsigned char mode);

/*==================================================================================================
                                       GLOBAL FUNCTIONS
==================================================================================================*/

/*=================================================================================================

FUNCTION: main

DESCRIPTION:
   test starting point.

ARGUMENTS PASSED:
   None.

RETURN VALUE:
   None

PRE-CONDITIONS:
   None

POST-CONDITIONS:
   None

IMPORTANT NOTES:
   None

==================================================================================================*/

#ifndef REENTRANCY_TEST

#ifndef OS_VRTX
#ifdef __WINCE
int _tmain(int argc, char *argv[])
#else
int main( int argc, char *argv[] )
#endif
#else
int G711_dec_main(int argc, char *mm_argv)
#endif
{
	char InputFilename[255],OutputFilename[255];
	unsigned char mode = 0;
	FILE *pInFile=NULL, *pOutFile=NULL;
	const char *version;

#if defined(TIME_PROFILE_RVDS) || defined(TIME_PROFILE)
    FILE    *pfCycleCount = 0;
#endif

#ifdef DUMMYCALL
    uiframe = 0;
#endif

#ifdef OS_VRTX
	char *argv[5];
	char *abc = mm_argv;

	abc += strlen(abc)+1;
	argv[0] = abc;
	abc += strlen(abc)+1;
	argv[1] = abc;
	abc += strlen(abc)+1;
	argv[2] = abc;
	abc += strlen(abc)+1;
	argv[3] = abc;
	argc -= 1;
#endif

#ifdef __WINCE
	_TCHAR *arg_word;
	char *arg_byte;
	int count;
#endif

#ifdef __WINCE
	for (count = 1; count < argc; count++)
	{
		arg_word =(_TCHAR *)argv[count];
		arg_byte = argv[count];

		while(*(arg_word) != '\0')
		{
			*arg_byte++=(char)*arg_word++;
		}
			*arg_byte=(char)'\0';
	}
#endif

    version = G711D_get_version_info();
    printf("Running %s\n",version);

#if defined(TIME_PROFILE_RVDS) || defined(TIME_PROFILE) || defined(ARM_MIPS_TEST_WINCE)
 #ifndef __SYMBIAN32__
    pfCycleCount = fopen ("../audio_performance.txt", "a");
#else
    pfCycleCount = fopen ("D:\\g711_dec_cycles.txt", "a");
#endif
    if(!pfCycleCount)
    {
		printf("\nUnable to open log file g711_dec_cycles.txt\n");
		goto exit;
	}    
#endif //#if defined(TIME_PROFILE_RVDS) || defined(TIME_PROFILE) || defined(ARM_MIPS_TEST_WINCE)

	if ( G711ProcessCmdLineOptions(argc, argv, &mode) )
	{
		return 0;
	}
    if (mode >= 2)
    {
        printf ("\n G.711 decoder don't support converting between A-law and Mu-law\n");
        printf ("\n G.711 encoder supports converting between A-law and Mu-law\n");
        printf ("\n A/M                : select A for A-law and M for Mu-law\n");
        return 0;
    }

	strcpy( InputFilename, argv[2] );
    strcpy( OutputFilename, argv[3] );

	pInFile   = fopen(InputFilename,"rb");
	pOutFile  = fopen(OutputFilename,"wb");

	if(!pInFile || !pOutFile)
	{
		if(!pInFile)
		{
			printf("\nUnable to open input file : %s\n", InputFilename);
		}

		if(!pOutFile)
		{
			printf("\nUnable to open output file : %s\n", OutputFilename);
		}

		goto exit;
	}

	g711decodeTest(pInFile, pOutFile, mode);

#ifdef TIME_PROFILE_RVDS

	fprintf(pfCycleCount,"\tPeak Frame no: %ld,  clk: %ld\n", frame_max_no,
																frame_max_clk);
	fprintf(pfCycleCount,"\tMin  Frame no: %ld,  clk: %ld\n", frame_min_no,
																frame_min_clk);

	fprintf(pfCycleCount,"\t Total clk : %ld\n", total_clk);

	fprintf(pfCycleCount,"\tTotal Frame: %ld, average clk: %ld\n",
									   nframe, nframe?total_clk/nframe:0);
	fprintf(pfCycleCount, "------------------------------------------\n\n");

	{
		int count = 0;
		int tot_char = 0;
		int n = 0;
		int ch = 0;
		char fileName[300]="";
		char file[100]="";

		strcpy(fileName,InputFilename);
		tot_char = strlen(fileName);
		count = tot_char - 1;
		while (fileName[count] != '/')
		{
			count --;

			if(count==0)
			{
				if(fileName[count]!='/')
				{
					count--;
				}
				break;
			}
		}

		n=count+1;

		while (n != tot_char)
		{
		 file[ch ++] = fileName [n ++];
		}

#if defined(MEASURE_STACK_USAGE) && defined(MEASURE_HEAP_USAGE)
		fprintf(pfCycleCount,"\n\n%s\t%s\t%ld\t%ld\t%d\t%d\t%ld\t%d\t%d\t\n\n",chname, file,
					(frame_max_clk*64),(frame_min_clk*64),frame_max_no,frame_min_no,(total_clk*64), peak_stack_usage, peak_heap_usage);

		fprintf(stderr,"\n\n%s\t%s\t%ld\t%ld\t%d\t%d\t%ld\t%d\t%d\t\n\n",chname, file,
					(frame_max_clk*64),(frame_min_clk*64),frame_max_no,frame_min_no,(total_clk*64), peak_stack_usage, peak_heap_usage);
#else
		fprintf(pfCycleCount,"\n\n%s\t%s\t%ld\t%ld\t%d\t%d\t%ld\t\n\n",chname, file,
					(frame_max_clk*64),(frame_min_clk*64),frame_max_no,frame_min_no,(total_clk*64));


		fprintf(stderr,"\n\n%s\t%s\t%ld\t%ld\t%d\t%d\t%ld\t\n\n",chname, file,
					(frame_max_clk*64),(frame_min_clk*64),frame_max_no,frame_min_no,(total_clk*64));

#endif //#if defined(MEASURE_STACK_USAGE) && defined(MEASURE_HEAP_USAGE)

	}
#endif //TIME_PROFILE_RVDS

#ifdef TIME_PROFILE
	printf("\nDencoder");
	printf("\n%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t\n",chname, InputFilename,
						maxFrameTime,minFrameTime,numFrame,maxFrameNumber,minFrameNumber,totalTime);
	
	fprintf (pfCycleCount,"\n|\tAudio Format\t|\tPlat\t|\tSamplerate\t|\tCh\t|\tBiterate\t|\tBit\t|\tPerf(MIPS)\t|\tAudio file name\n");
	performance = (double)totalTime * 0.000001*cpu_freq*8000/numFrame/BUFFER_SIZE;
       fprintf(pfCycleCount, "|\tG.711 Decoder\t|\tARM%d\t|\t%9d\t|\t%d\t|\t%8d\t|\t%d\t|\t%.8f\t|%s\n", platform, 8000, 1, 0, 16, performance, InputFilename);

//	fprintf(pfCycleCount,"\n%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname, InputFilename,
//						maxFrameTime,minFrameTime,numFrame,maxFrameNumber,minFrameNumber,totalTime);
	
#endif
#ifdef __SYMBIAN32__
#ifdef MEASURE_STACK_USAGE
			printf("\n Stack n Heap - %d\t%d\t",peak_stack_usage,peak_heap_usage);
#endif
#endif

#ifdef	ARM_MIPS_TEST_WINCE
	printf("\nDencoder");
	printf("\n%s\t%s\t%ld",chname,InputFilename,maxFrameTime);
	printf("\t%ld",minFrameTime);
	printf("\t%d\t%d\t%d\t%ld\t",numFrame,maxFrameNumber,minFrameNumber,totalTime);
	fprintf(pfCycleCount,"\n%s\t%s\t%ld",chname,InputFilename,maxFrameTime);
	fprintf(pfCycleCount,"\t%ld",minFrameTime);
	fprintf(pfCycleCount,"\t%d\t%d\t%d\t%ld\t",numFrame,maxFrameNumber,minFrameNumber,totalTime);
#endif

exit:

	if(pInFile)
	{
		fclose(pInFile), pInFile=0;
	};

	if(pOutFile)
	{
		fclose(pOutFile), pOutFile=0;
	};



#if defined(TIME_PROFILE_RVDS) || defined(TIME_PROFILE) || defined(ARM_MIPS_TEST_WINCE)
	if(pfCycleCount)
	{
		fclose(pfCycleCount), pfCycleCount=0;
	}
#endif //  defined(TIME_PROFILE_RVDS) || defined(TIME_PROFILE) || defined(ARM_MIPS_TEST_WINCE)


    return(0);
}//END OF MAIN

#else //REENTRANCY_TEST

#include <pthread.h>

typedef struct ReentrancyParams
{
	FILE *ifp;
	FILE *ofp;
	unsigned char mode;
}ReentrancyParams;

int DecThreadProc(void *thread)
{
	ReentrancyParams *pRentrancyParams = (ReentrancyParams*) thread;

	g711decodeTest(pRentrancyParams->ifp, pRentrancyParams->ofp, pRentrancyParams->mode);

	return 0;
}

int main( int argc, char *argv[] )
{
	ReentrancyParams  rentrancyParams1;
	ReentrancyParams  rentrancyParams2;

	rentrancyParams1.ifp = 0;
	rentrancyParams1.ofp = 0;
	rentrancyParams2.ifp = 0;
	rentrancyParams2.ofp = 0;

	pthread_t threadid1, threadid2;
	long thread_stat1,thread_stat2;

	if(argc<7)
	{
		printf("\nG711 reentrancy usage: <exe> <mode> <in file1> <out file1> <mode> <in file2> <out file2>");
		printf ("\n mode : select A for A-law and M for Mu-law\n");
		goto exit;
	}

	if ( strcmp( argv[1], "A" ) == 0 )
	{
		rentrancyParams1.mode = 0;
	}
	else if ( strcmp( argv[1], "M" ) == 0 )
	{
		rentrancyParams1.mode = 1;
	}
	else
	{
		printf("\nG711 reentrancy usage: <exe> <mode> <in file1> <out file1> <mode> <in file2> <out file2>");
		printf ("\n mode : select A for A-law and M for Mu-law\n");
		goto exit;
	}

	if ( strcmp( argv[4], "A" ) == 0 )
	{
		rentrancyParams2.mode = 0;
	}
	else if ( strcmp( argv[4], "M" ) == 0 )
	{
		rentrancyParams2.mode = 1;
	}
	else
	{
		printf("\nG711 reentrancy usage: <exe> <mode> <in file1> <out file1> <mode> <in file2> <out file2>");
		printf("\n mode : select A for A-law and M for Mu-law\n");
		goto exit;
	}

	rentrancyParams1.ifp = fopen(argv[2],"rb");
	rentrancyParams2.ifp = fopen(argv[5],"rb");

	rentrancyParams1.ofp = fopen(argv[3],"wb");
	rentrancyParams2.ofp = fopen(argv[6],"wb");

	if(!rentrancyParams1.ifp || !rentrancyParams2.ifp || !rentrancyParams1.ofp || !rentrancyParams2.ofp)
	{
		printf("\nFailed to open one or more files specified!!!\n");
		goto exit;
	}

	printf("\n\tCreating 2 Threads, one for each decoder Object");

	thread_stat1 = pthread_create(&threadid1,NULL,(void *)&DecThreadProc,(void *)&rentrancyParams1);

	if (thread_stat1 !=0 )
	{
		printf("\nThread Creation Failed : %ld",thread_stat1);
		goto exit;
	}

	thread_stat2 = pthread_create(&threadid2,NULL,(void *)&DecThreadProc,(void *)&rentrancyParams2);

	if (thread_stat2 !=0)
	{
		printf("\nThread Creation Failed : %ld",thread_stat2);
		goto exit;
	}

	printf("\n\tWaitForMultipleObjects Thread Execution...");

	pthread_join( threadid1, NULL);
	pthread_join( threadid2, NULL);

	printf("\n\tBoth Decodes Complete.\n");

exit:
	if(rentrancyParams1.ifp)
	{
		fclose(rentrancyParams1.ifp), rentrancyParams1.ifp=0;
	};

	if(rentrancyParams2.ifp)
	{
		fclose(rentrancyParams2.ifp), rentrancyParams2.ifp=0;
	};

	if(rentrancyParams1.ofp)
	{
		fclose(rentrancyParams1.ofp), rentrancyParams1.ofp=0;
	};

	if(rentrancyParams2.ofp)
	{
		fclose(rentrancyParams2.ofp), rentrancyParams2.ofp=0;
	};

    return(0);
}//END OF MAIN

#endif //REENTRANCY_TEST

void g711decodeTest(FILE *pInFile, FILE *pOutFile, unsigned char mode)
{
	short pcm_samples[BUFFER_SIZE];
	unsigned char coded_samples[BUFFER_SIZE];

#ifdef READ_FULL_INPUTFILE
	short *k1;
#else
	short k1[BUFFER_SIZE];
#endif
    int readCount = 0;
	//unsigned int buffer_count = 0;
	int buffer_count = 0;

#ifdef BIG_ENDIAN
    unsigned char lower_byte,upper_byte;
#endif
#ifdef READ_FULL_INPUTFILE
	int		readCountAll = 0;
	int		offset = 0;
	short   kAll[TIME_PROFILE_BUFFER_SIZE];
#endif

	if ( mode == 0 )    // selecting A law
	{


#ifdef READ_FULL_INPUTFILE
		readCountAll = fread ( kAll, sizeof(short), 1024*64, pInFile);
		readCount = (readCountAll > BUFFER_SIZE) ? BUFFER_SIZE : readCountAll;
		readCountAll -= readCount;
		k1 = kAll;
#else
		//readCount = fread ( k1, sizeof(short), BUFFER_SIZE, pInFile);
		readCount = fread ( coded_samples, sizeof(unsigned char), BUFFER_SIZE, pInFile);
#endif

      while(readCount)// == ( BUFFER_SIZE  ) )
	  {

		  buffer_count = 0;

#ifdef BIG_ENDIAN
		  dec_vSwapBytes( k1, BUFFER_SIZE );
#endif
/*
			while ( buffer_count < readCount)
		   {
			 coded_samples[ buffer_count ] = (unsigned char) k1[ buffer_count ];
			 buffer_count++;
		   }
*/


#ifdef TIME_PROFILE
		  retVal = gettimeofday(&StartTime, NULL);
		  if (retVal == 0)
		  {
			  time_before = StartTime.tv_sec * 1000000 + StartTime.tv_usec;
			  timeFlag = 1;
		  }
#endif

#ifdef	MEASURE_STACK_USAGE
			PAINT_STACK (base_ptr, stack_ptr, index_temp);
#endif

#ifdef TIME_PROFILE_RVDS
           prev_clk=prev_cycles();
#endif

#ifdef	ARM_MIPS_TEST_WINCE						/* for taking timing on wince platform*/
		INTERRUPTS_SET(0xDF);  						/*disable interrupt*/
		Flag=QueryPerformanceFrequency(&lpFrequency);
		Flag=QueryPerformanceCounter(&lpPerformanceCountBegin);
#endif
#ifdef DUMMYCALL
        dummyCall1();
#endif

			g711AlawDecode(BUFFER_SIZE, coded_samples, pcm_samples);

#ifdef DUMMYCALL
        dummyCall2();
#endif


#ifdef TIME_PROFILE_RVDS
            curr_clk = curr_cycles();
            clk = (curr_clk-prev_clk);    /* clk gives the total core cycles per
                                             frame call */
            total_clk += clk;
            nframe++;
            if(clk > frame_max_clk)
            {
                frame_max_clk = clk;
                frame_max_no = nframe;
            }
            if(clk < frame_min_clk)
            {
                frame_min_clk = clk;
                frame_min_no = nframe;
            }
#endif

#ifdef  MEASURE_STACK_USAGE
			GET_STACK_USAGE (base_ptr, stack_ptr, index_temp, stack_ptr_value);
#endif

#ifdef MEASURE_STACK_USAGE
			if (peak_stack_usage < stack_ptr_value)
			peak_stack_usage = stack_ptr_value;
#endif
#ifdef TIME_PROFILE
            if (timeFlag == 1)
            {
                retVal = gettimeofday(&EndTime, NULL);
                if (retVal == 0)
                {
                    time_after = EndTime.tv_sec * 1000000 + EndTime.tv_usec;
                    timediff = time_after - time_before;
                    numFrame++;
                    if (timediff > maxFrameTime)
                    {
                        maxFrameTime = timediff;
                        maxFrameNumber = numFrame;
                    }
                    else if (timediff < minFrameTime)
                    {
                        minFrameTime = timediff;
                        minFrameNumber = numFrame;
                    }
                    totalTime += timediff;
                    /*
                    fprintf(pfCycleCount,"\t FrameNo: %d", numFrame);
                    fprintf(pfCycleCount,"\t Total clk : %ld\n", timediff);
                    */
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

		  buffer_count = 0;

		  while ( buffer_count < readCount)
		   {

#ifdef BIG_ENDIAN
			lower_byte = (unsigned char) pcm_samples[ buffer_count ];
			upper_byte = (unsigned char) (pcm_samples[ buffer_count ] >> 8);
			pcm_samples[ buffer_count ] = (unsigned short)( lower_byte << 8 ) | upper_byte ;
#endif
#ifdef OUTPUT_DUMP
			fwrite( &pcm_samples[ buffer_count ],sizeof(short),1,pOutFile );
#endif

			buffer_count++;
		   }
#ifdef READ_FULL_INPUTFILE
			offset += readCount;
			k1 = &kAll[offset];
			readCount = (readCountAll > BUFFER_SIZE) ? BUFFER_SIZE : readCountAll;
			readCountAll -= readCount;
#else
		  //readCount = fread ( k1, sizeof(short), BUFFER_SIZE, pInFile)	;
		  readCount = fread ( coded_samples, sizeof(unsigned char), BUFFER_SIZE, pInFile);
#endif
		}
	 }
	else                    // selecting Mu law
	{

		//readCount = fread ( k1, sizeof(short), BUFFER_SIZE, pInFile );
		readCount = fread ( coded_samples, sizeof(unsigned char), BUFFER_SIZE, pInFile);
        while(readCount)// == ( BUFFER_SIZE  ) )
		{

		  buffer_count = 0;

#ifdef BIG_ENDIAN
		  dec_vSwapBytes( k1, BUFFER_SIZE );
#endif
/*
		  while ( buffer_count < readCount)
		   {
			 coded_samples[ buffer_count ] = (unsigned char) k1[ buffer_count ];
			 buffer_count++;
		   }
*/




#ifdef TIME_PROFILE
		   retVal = gettimeofday(&StartTime, NULL);
		  if (retVal == 0)
		  {
			  time_before = StartTime.tv_sec * 1000000 + StartTime.tv_usec;
			  timeFlag = 1;
		  }
#endif


#ifdef MEASURE_STACK_USAGE
		 PAINT_STACK (base_ptr, stack_ptr, index_temp);
#endif

#ifdef TIME_PROFILE_RVDS
		   prev_clk=prev_cycles();
#endif

#ifdef	ARM_MIPS_TEST_WINCE						/* for taking timing on wince platform*/
		INTERRUPTS_SET(0xDF);  						/*disable interrupt*/
		Flag=QueryPerformanceFrequency(&lpFrequency);
		Flag=QueryPerformanceCounter(&lpPerformanceCountBegin);
#endif
#ifdef DUMMYCALL
        dummyCall1();
#endif

		g711MulawDecode(BUFFER_SIZE, coded_samples, pcm_samples);

#ifdef DUMMYCALL
        dummyCall2();
#endif


#ifdef TIME_PROFILE_RVDS
            curr_clk = curr_cycles();
            clk = (curr_clk-prev_clk);    /* clk gives the total core cycles per
                                             frame call */
            total_clk += clk;
            nframe++;
            if(clk > frame_max_clk)
            {
                frame_max_clk = clk;
                frame_max_no = nframe;
            }
            if(clk < frame_min_clk)
            {
                frame_min_clk = clk;
                frame_min_no = nframe;
            }
#endif

#ifdef	MEASURE_STACK_USAGE
		GET_STACK_USAGE (base_ptr, stack_ptr, index_temp, stack_ptr_value);
#endif

#ifdef MEASURE_STACK_USAGE
		if (peak_stack_usage < stack_ptr_value)
		peak_stack_usage = stack_ptr_value;
#endif
#ifdef TIME_PROFILE
            if (timeFlag == 1)
            {
                retVal = gettimeofday(&EndTime, 0);
                if (retVal == 0)
                {
                    time_after = EndTime.tv_sec * 1000000 + EndTime.tv_usec;
                    timediff = time_after - time_before;
                    numFrame++;
                    if (timediff > maxFrameTime)
                    {
                        maxFrameTime = timediff;
                        maxFrameNumber = numFrame;
                    }
                    else if (timediff < minFrameTime)
                    {
                        minFrameTime = timediff;
                        minFrameNumber = numFrame;
                    }
                    totalTime += timediff;
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

		  buffer_count = 0;

		  while ( buffer_count < readCount )
		   {
#ifdef BIG_ENDIAN
			lower_byte = (unsigned char) pcm_samples[ buffer_count ];
			upper_byte = (unsigned char) (pcm_samples[ buffer_count ] >> 8);
			pcm_samples[ buffer_count ] = (unsigned short)( lower_byte << 8 ) | upper_byte ;
#endif
#ifdef OUTPUT_DUMP
			fwrite( &pcm_samples[ buffer_count ],sizeof(short),1,pOutFile );
#endif
			buffer_count++;
		   }
		  //readCount = fread ( k1, sizeof(short), BUFFER_SIZE, pInFile );
		  readCount = fread ( coded_samples, sizeof(unsigned char), BUFFER_SIZE, pInFile);
		}
	}
}

/*==================================================================================================

FUNCTION: dec_vSwapBytes
DESCRIPTION:

ARGUMENTS PASSED:
   short *ps16Buf :
   int s32Cnt  :


RETURN VALUE:
   None

PRE-CONDITIONS:
   None

POST-CONDITIONS:
   None

IMPORTANT NOTES:
   None

==================================================================================================*/
void dec_vSwapBytes(short *ps16Buf, int s32Cnt)
{
    char   s8ch;
    char   *s8p;

    while(--s32Cnt >= 0)
    {
         s8p = (char *)(ps16Buf+s32Cnt);
         s8ch = s8p[0];
         s8p[0] = s8p[1];
         s8p[1] = s8ch;
    }
}



