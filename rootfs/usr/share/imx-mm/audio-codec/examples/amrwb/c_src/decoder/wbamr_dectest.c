/**********************************************************************
*
* Motorola Inc.
* (c) Copyright 2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
***********************************************************************
* Copyright (c) 2005-2010, 2012, 2013, Freescale Semiconductor, Inc.
* All modifications are confidential and proprietary information
* of Freescale Semiconductor, Inc.
***********************************************************************
*
*  AMR Wideband Codec 3GPP TS26.190 / ITU-T G.722.2, April 30, 2003.
*
***********************************************************************
*
* File Name			: amrwb_dectest.c
*
* Description		: Test framework for WBAMR Decoder.
*
* Functions Included: main()
*					: stack_fill()
*					: check_stack_depth()
****************************** Change History**************************
*
*    DD/MM/YYYY     Code Ver     Description                        Author
*    -----------     --------     -----------                       ------
*    03/11/2004     v1.0         Initial version                    Naganna/Shashi
*    13/12/2006     v1.1         Added profiling code for RVDS      Shyam Krishnan M
*    28/07/2008     v1.2         Added profiling code for ARM9      r66286
**********************************************************************/


/**********************************************************************
* Include Files
**********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef TIME_PROFILE
#ifdef __SYMBIAN32__
#include <sys/time.h>
#else //__SYMBIAN32__
#include <time.h>
#endif //__SYMBIAN32__
#endif

#include "wbamr_dec_interface.h"
#include "wbamr_dectest.h"
#include "bits_test.h"
#include "nb_bits.h"

#ifdef __WINCE
#include<windows.h>
#endif

#define OUTPUT_DUMP
#if defined TIME_PROFILE || defined TIME_PROFILE_RVDS ||defined TIME_PROFILE_RVDS_ARM9|| defined ARM_MIPS_TEST_WINCE
#undef OUTPUT_DUMP
#endif

#ifdef WBAMR_DECODER_STACKDEPTH_TST

WBAMR_S32 stack_fill(WBAMR_S32 pattern);
WBAMR_S32 check_stack_depth(WBAMR_S32 stack_ptr, WBAMR_U8 pattern);

#endif

#if defined(WBAMR_ARM1136) &&  defined(TIME_PROFILE_RVDS )
WBAMR_S16 rem_data[4]={0,0,0,0};
WBAMR_S16 size_rem_data = 0;
#endif

#ifdef MEASURE_STACK_USAGE
#include "mm_codec_common.h"
#endif

int noOfFrames;
void dummyCall(void);

#ifdef WBAMR_BIG_ENDIAN
void byteswap(WBAMR_S16 *ptr, WBAMR_S32 len);
#endif

#if defined(WBAMR_ARM1136) &&  defined(TIME_PROFILE_RVDS )
WBAMR_S32 prev_cycles();
WBAMR_S32 curr_cycles();
#endif

#ifdef TIME_PROFILE_RVDS_ARM9
void    dummyCall1(void);
void    dummyCall2(void);
WBAMR_S32  uiframe;
#endif
#ifdef ARM_MIPS_TEST_WINCE        		 /* for taking timing on wince platform*/
WBAMR_S8    *gps8InpFileName;
WBAMR_S8    *gps8OutFileName;
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

WBAMR_S16 Read_serial(FILE * fp, WBAMR_S16 *prms, WBAMR_U8 bitstreamformat);
unsigned long infilesize = 0;

#ifdef TGT_CPU_ARM12
#define TIMER_BASE	 0x0A800000U
#define TIMER_LOAD	 (TIMER_BASE + 0x000U)
#define TIMER_CONTROL    (TIMER_BASE + 0x008U)
#define TIMER_CLEAR      (TIMER_BASE + 0x00CU)
#define TIMER_VALUE	 (0xFFF/2)

#define IRQCT_BASE	 0x0A801000U
#define IRQCT_ENABLESET	 (IRQCT_BASE + 0x008U)
#define IRQCT_ENABLECLR	 (IRQCT_BASE + 0x00CU)

///////////////////////////////////////////////////////////////////////////// //Lyon 070731
// File Global Data
// Registers in the peripherals.
static volatile unsigned int * const irq_enable_set = (volatile unsigned int *)IRQCT_ENABLESET;
static volatile unsigned int * const irq_enable_clr = (volatile unsigned int *)IRQCT_ENABLECLR;
static volatile unsigned int * const timer_load     = (volatile unsigned int *)TIMER_LOAD;
static volatile unsigned int * const timer_control  = (volatile unsigned int *)TIMER_CONTROL;
static volatile unsigned int * const timer_clear    = (volatile unsigned int *)TIMER_CLEAR;
#endif

//extern WBAMR_S16 fir_7k[];
WBAMR_S16 mode=0;

/**********************************************************************
* Main Program
**********************************************************************/

int main (int argc, char *argv[])
{
  	WBAMR_S32 i;
	WBAMR_S16 nr;
	WBAMR_S16 bitstreamformat;
    FILE *f_serial;						/* File of serial bits for transmission  */
    FILE *f_synth;						/* File of speech data                   */

	WBAMRD_RET_TYPE retval;
	WBAMRD_Mem_Alloc_Info_Sub *mem;

  	WBAMRD_Decoder_Config *dec_config;

	WBAMR_S16 *in_buf; /* bitstream buffer */

	WBAMR_S16 *out_buf; /* decoded PCM sample buffer */
    char magic[10];		/* For Storing the Magic word in case
							of MIME format*/
#ifdef MEASURE_STACK_USAGE
    int           *ps32SP, *ps32BaseSP;
    int           s32StackCount, s32StackValue;
    int           s32PeakStack = 0;
#endif

#ifdef MEASURE_HEAP_USAGE
    int      s32TotalMallocBytes=0;
#endif

#ifdef WBAMR_DECODER_CONT_TST
	WBAMR_S32 *scratchMem,maxScr = 0,scrCount = 0;
#endif

#ifdef WBAMR_DECODER_STACKDEPTH_TST
	WBAMR_U32 stackTestPtr;
	WBAMR_S32 maxStack = 0, curFrameStackUsage;
#endif

#if defined(WBAMR_ARM1136) &&  defined(TIME_PROFILE_RVDS )
    WBAMR_S32 prev_clk, curr_clk, clk,max_frame_no,min_frame_no;
    WBAMR_S32 total_clk,max_clk,avg_clk,frame,min_clk;
    FILE *fp_cycle;
	unsigned char chname[] = "[PROFILE-INFO]";
    fp_cycle = fopen("wb_amr_dec_cycle.txt", "a");
#endif

#ifdef ALIGNMENT_CHECK
    int cp_ctrl_data = 0;       /* to store CP15 control register data */
#endif

#ifdef TIME_PROFILE
    FILE    *pfCycleCount, *ph_mips;
    struct timeval tv_prof;
    long time_before =0, time_after=0;
    int retVal = -1;
    int minFrameNumber=0, maxFrameNumber=0, numFrame =0 ,Frame=0;
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
    int bit_rate[9] = {6600, 8850, 12650, 14250, 15850, 18250, 19850, 23050, 23850};
    double performance = 0.0;    
#ifdef __SYMBIAN32__
    pfCycleCount = fopen ("D:\\wbamr_dec_cycle.txt", "a");
#else //__SYMBIAN32__
	pfCycleCount = fopen ("wbamr_dec_cycle.txt", "a");
    ph_mips =  fopen ("../audio_performance.txt", "a");
#endif //__SYMBIAN32__
#endif

#ifdef ARM_MIPS_TEST_WINCE
    FILE    *fSysTime;
    fSysTime = fopen ("wb_amr_dec_sys_time.txt", "a");
    if(!fSysTime)
    {
		printf("\nUnable to open log file g723_enc_sys_time.txt\n");
		exit(1);
	}
#endif

/*
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 */
    /*----- Report Current Version   ---------------------------------*/
    printf("\nRunning %s\n",WBAMR_get_version_info());
    /*-----------------------------------------------------------------*
     *           Read passed arguments and open in/out files           *
     *-----------------------------------------------------------------*/
//#ifndef TGT_CPU_ARM12
    if (argc != 3 && argc != 4)
    {
        fprintf(stderr, "Usage : decoder  (-itu | -mime | -if1 | -if2) bitstream_file  synth_file\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Format for bitstream_file: (default)\n");
        fprintf(stderr, "  One word (2-byte) to indicate type of frame type.\n");
        fprintf(stderr, "  One word (2-byte) to indicate frame type.\n");
        fprintf(stderr, "  One word (2-byte) to indicate mode.\n");
        fprintf(stderr, "  N words (2-byte) containing N bits (bit 0 = 0xff81, bit 1 = 0x007f).\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "  if option -itu defined:\n");
		fprintf(stderr, "  One word (2-byte) for sync word (good frames: 0x6b21, bad frames: 0x6b20)\n");
		fprintf(stderr, "  One word (2-byte) for frame length N.\n");
		fprintf(stderr, "  N words (2-byte) containing N bits (bit 0 = 0x007f, bit 1 = 0x0081).\n");
		fprintf(stderr, "\n");
        fprintf(stderr, "  if option -mime defined:\n");
		fprintf(stderr, "  AMR-WB MIME/storage format, see RFC 3267 (sections 5.1 and 5.3) for details.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "  if option -if1 defined:\n");
		fprintf(stderr, "  Except the magic word, this is same as mime, calculates CRC.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "  if option -if2 defined:\n");
		fprintf(stderr, "  Except the magic word, this is same as mime.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Format for synth_file:\n");
        fprintf(stderr, "  Synthesis is written to a binary file of 16 bits data.\n");
        fprintf(stderr, "\n");
        exit(0);
    }
// #endif

	bitstreamformat = 0;

#if !defined(WBAMR_ARM1136) ||  !defined(TIME_PROFILE_RVDS )
    if (strcmp(argv[1], "-itu") == 0)
	{
	    bitstreamformat = 1;
	    argv++;
		fprintf(stderr, "Input bitstream format: ITU\n");
    } else
	{
		if (strcmp(argv[1], "-mime") == 0)
		{
			bitstreamformat = 2;
			argv++;
			fprintf(stderr, "Input bitstream format: MIME\n");
		} else if (strcmp(argv[1], "-if2") == 0)
		{
			bitstreamformat = 3;
			argv++;
			fprintf(stderr, "Input bitstream format: IF2\n");
		} else if (strcmp(argv[1], "-if1") == 0)
		{
			bitstreamformat = 4;
			argv++;
			fprintf(stderr, "Input bitstream format: IF1\n");
		} else
		{
			fprintf(stderr, "Input bitstream format: Default\n");
		}
	}
#else
    if (mystrcmp(argv[1], "-itu") == 0)
	{
	    bitstreamformat = 1;
	    argv++;
		fprintf(stderr, "Input bitstream format: ITU\n");
    } else
	{
		if (mystrcmp(argv[1], "-mime") == 0)
		{
			bitstreamformat = 2;
			argv++;
			fprintf(stderr, "Input bitstream format: MIME\n");
		} else if (mystrcmp(argv[1], "-if2") == 0)
		{
			bitstreamformat = 3;
			argv++;
			fprintf(stderr, "Input bitstream format: IF2\n");
		} else
		{
			fprintf(stderr, "Input bitstream format: Default\n");
		}
	}
#endif

	/* Open file for synthesis and packed serial stream */
    if(argc>1)
    	f_serial = fopen(argv[1], "rb");
    else
     f_serial = fopen("test.bit", "rb");
    if (f_serial == NULL)
    {
        fprintf(stderr, "Input file '%s' does not exist !!\n", argv[1]);
        exit(0);
    } else
        fprintf(stderr, "Input bitstream file:   %s\n", argv[1]);

    if(argc>2)
    	f_synth = fopen(argv[2], "wb");
    else
    	f_synth = fopen("test.pcm", "wb");
    if(f_synth ==NULL)
    {
        fprintf(stderr, "Cannot open file '%s' !!\n", argv[2]);
        exit(0);
    } else
        fprintf(stderr, "Synthesis speech file:   %s\n", argv[2]);


 /*
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 */
  	/* allocate memory for decoder configuration structure */
  	dec_config=(WBAMRD_Decoder_Config *)
 				alloc_fast(sizeof(WBAMRD_Decoder_Config));


	dec_config->wbamrd_decode_info_struct_ptr =NULL;

	dec_config->wbappd_initialized_data_start =NULL; //TLSbo60277

	/* Initialize config structure memory to NULL; */
	for (i=0; i< WBAMR_MAX_NUM_MEM_REQS; i++)
	{
		dec_config->wbamrd_mem_info.mem_info_sub[i].wbappd_base_ptr =NULL;
	}

	dec_config->bitstreamformat = bitstreamformat;


 	/* Query decoder for its memory requirement */
  	retval = wbamrd_query_dec_mem(dec_config);
  	if (WBAMRD_OK != retval)
	{
		/* de-allocate memory allocated to enc_config */
		free(dec_config);
		return WBAMR_FAILURE;
	}

 	/* Number of memory chunk requested by the decoder */
   	nr = dec_config->wbamrd_mem_info.wbamrd_num_reqs;

 	/* allocate memory requested by the decoder*/
	mem = &(dec_config->wbamrd_mem_info.mem_info_sub[0]);
	if (mem->wbamrd_type_fs == WBAMR_FAST_MEMORY)
  	{
		mem->wbappd_base_ptr = alloc_fast(mem->wbamrd_size + WBAMR_EXTRASTATE);
		#ifdef MEASURE_HEAP_USAGE
			s32TotalMallocBytes +=  (mem->wbamrd_size + WBAMR_EXTRASTATE);
		#endif
	}
	else
	{
		mem->wbappd_base_ptr = alloc_slow(mem->wbamrd_size + WBAMR_EXTRASTATE);
		#ifdef MEASURE_HEAP_USAGE
			s32TotalMallocBytes +=  (mem->wbamrd_size + WBAMR_EXTRASTATE);
		#endif
	}

	mem = &(dec_config->wbamrd_mem_info.mem_info_sub[1]);
	if (mem->wbamrd_type_fs == WBAMR_FAST_MEMORY)
  	{
		mem->wbappd_base_ptr = alloc_fast(mem->wbamrd_size + WBAMR_EXTRASCRATCH);
		#ifdef MEASURE_HEAP_USAGE
			s32TotalMallocBytes +=  (mem->wbamrd_size + WBAMR_EXTRASCRATCH);
		#endif
	}
	else
	{
		mem->wbappd_base_ptr = alloc_slow(mem->wbamrd_size + WBAMR_EXTRASCRATCH);
		#ifdef MEASURE_HEAP_USAGE
			s32TotalMallocBytes +=  (mem->wbamrd_size + WBAMR_EXTRASCRATCH);
		#endif
	}

	for(i = 2; i < nr; i++)
	{
		mem = &(dec_config->wbamrd_mem_info.mem_info_sub[i]);
		if (mem->wbamrd_type_fs == WBAMR_FAST_MEMORY)
      	{
			mem->wbappd_base_ptr = alloc_fast(mem->wbamrd_size);
			#ifdef MEASURE_HEAP_USAGE
				s32TotalMallocBytes +=  mem->wbamrd_size ;
			#endif
		}
		else
		{
			mem->wbappd_base_ptr = alloc_slow(mem->wbamrd_size);
			#ifdef MEASURE_HEAP_USAGE
				s32TotalMallocBytes +=  mem->wbamrd_size ;
			#endif
		}
	}

     #ifdef TGT_CPU_ARM9
	dec_config->wbappd_initialized_data_start = (WBAMR_U8 *)BEGIN_WBAMRD_DATA;
    #endif
	/* initialize decoder */
	retval = wbamrd_decode_init(dec_config);
	if (WBAMRD_OK != retval)
	{
		for (i=0; i<nr; i++)
		{
			free (dec_config->wbamrd_mem_info.mem_info_sub[i].wbappd_base_ptr);
		}
      	free (dec_config);
		return WBAMR_FAILURE;
	}
	/* allocate memory for input buffer */
	in_buf = alloc_fast (WBAMR_SERIAL_FRAMESIZE *sizeof (WBAMR_S16));

	if (NULL == in_buf)
	{
		/* free all the memory allocated for decoder config param 			*/
		for (i = 0; i < nr; i++)
		{
			free (dec_config->wbamrd_mem_info.mem_info_sub[i].wbappd_base_ptr);
		}
		free (dec_config);
		return WBAMR_FAILURE;
	};

	/* allocate memory for output buffer */
	out_buf = alloc_fast ((WBAMR_L_FRAME+32)*sizeof (WBAMR_S16));
	if(((int)out_buf&15)!=0)
	{
	   out_buf = (WBAMR_S16*)((int)out_buf + 16 - ((int)out_buf&15));
	}
	if (NULL == out_buf)
	{
		/* free all the memory allocated for decoder config param 			*/
		for (i=0; i<nr; i++)
		{
			free (dec_config->wbamrd_mem_info.mem_info_sub[i].wbappd_base_ptr);
		}
		free (dec_config);
		free(in_buf);
		return WBAMR_FAILURE;
	};

#ifdef WBAMR_DECODER_CONT_TST

	/* Fill the Extra State Mem buffer with 55's to check the usage of extra scratch.*/
	scratchMem = (WBAMR_S32 *)dec_config->wbamrd_mem_info.mem_info_sub[0].wbappd_base_ptr;
	scratchMem += (dec_config->wbamrd_mem_info.mem_info_sub[0].wbamrd_size/sizeof(WBAMR_S32));
	for (i = 0; i < (WBAMR_EXTRASTATE/sizeof(WBAMR_S32)); i++)
	{
		scratchMem[i] = 0x44444444;
	}

	/* Fill the Extra Scratch buffer with 55's to check the usage of extra scratch.
	*/
	scratchMem = (WBAMR_S32 *)dec_config->wbamrd_mem_info.mem_info_sub[1].wbappd_base_ptr;
	scratchMem += (dec_config->wbamrd_mem_info.mem_info_sub[1].wbamrd_size/sizeof(WBAMR_S32));
	for (i = 0; i < (WBAMR_EXTRASCRATCH/sizeof(WBAMR_S32)); i++)
	{
		scratchMem[i] = 0x55555555;
	}
#endif

	/* read and verify magic number if MIME/storage format specified */
    if (bitstreamformat == 2)
    {
        fread(magic, sizeof(char), 9, f_serial);

        if (strncmp(magic, "#!AMR-WB\n", 9))
        {
            fprintf(stderr, "%s%s\n", "Invalid magic number: ", magic);
            fclose(f_serial);
            fclose(f_synth);
            exit(0);
        }
    }
#if defined(WBAMR_ARM1136) &&  defined(TIME_PROFILE_RVDS )
	total_clk = max_clk = avg_clk = max_frame_no= min_frame_no = 0;
	min_clk=0x7fffffff;
	frame=0;
#endif

	/*************************************************************
     * Decode bitstream to generate one frame of speech till end
	 * of bitstream
	 ************************************************************/
	while(Read_serial(f_serial, (WBAMR_S16 *)in_buf,bitstreamformat)!=0)

	{

#ifdef WBAMR_DECODER_CONT_TST
		/* Get the scratch pointer. */
		scratchMem = (WBAMR_S32 *)dec_config->wbamrd_mem_info.mem_info_sub[1].wbappd_base_ptr;
		/* Contaminate the Encode Scratch with 55's.....	*/
		for (i=0;i<(dec_config->wbamrd_mem_info.mem_info_sub[1].wbamrd_size/sizeof(WBAMR_S32));i++)
			scratchMem[i]=0x55555555;
#endif


#ifdef WBAMR_DECODER_STACKDEPTH_TST
		/* Fill the stack with the pattern 0xA1A1A1A1. */
		stackTestPtr = stack_fill(0xA1A1A1A1);
#endif

#ifdef WBAMR_BIG_ENDIAN
		//byteswap(in_buf, );
#endif
#ifdef MEASURE_STACK_USAGE
        PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
#endif
#if defined(WBAMR_ARM1136) &&  defined(TIME_PROFILE_RVDS )
	    frame++;
        //fprintf(fp_cycle, " Frames processed:			%hd\r", frame);
		prev_clk = prev_cycles();
#endif

#ifdef TIME_PROFILE
	 Frame++;
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
            mrc p15, 0, cp_ctrl_data, c1, c0, 0;  /* Read the CP15 control register value
                                                     in cp_ctrl_data variable */
            orr cp_ctrl_data, cp_ctrl_data, 0x2;   /* enable strict alignment check
                                                      by resetting A bit [bit number 2]
                                                      of control register */
            mcr p15, 0, cp_ctrl_data, c1, c0, 0   /* write back to control register */
            mrc p15, 0, cp_ctrl_data, c1, c0, 0;  /* read control register for verification */
        }
        fprintf(stderr, "Final config = %x\n",  cp_ctrl_data);
#endif

#ifdef TIME_PROFILE_RVDS_ARM9
		dummyCall1();
#endif
#ifdef	ARM_MIPS_TEST_WINCE						/* for taking timing on wince platform*/
		INTERRUPTS_SET(0xDF);  						/*disable interrupt*/
		Flag=QueryPerformanceFrequency(&lpFrequency);
		Flag=QueryPerformanceCounter(&lpPerformanceCountBegin);
#endif
		/* decode speech */
		retval=wbamrd_decode_frame(dec_config, in_buf, out_buf);

#ifdef TIME_PROFILE_RVDS_ARM9
		dummyCall2();
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

#ifdef ALIGNMENT_CHECK
        __asm
        {
            mrc p15, 0, cp_ctrl_data, c1, c0, 0;  /* Read the control register value */
            and cp_ctrl_data, cp_ctrl_data, 0xfffffffd;   /* disable alignment chect */
            mcr p15, 0, cp_ctrl_data, c1, c0, 0   /* write back control register data */
            mrc p15, 0, cp_ctrl_data, c1, c0, 0;  /* Read control register data for verification */
        }
        fprintf(stderr, "Final config = %x\n",  cp_ctrl_data);
#endif
#if defined(WBAMR_ARM1136) &&  defined(TIME_PROFILE_RVDS )
		curr_clk = curr_cycles();

	    clk = (curr_clk-prev_clk);    			/* clk gives the total core cycles per
                                                   frame call */
        if(max_clk<clk)
        {
			max_clk = clk;
			max_frame_no = frame;
		}

		if(min_clk>clk)
        {
			min_clk = clk;
			min_frame_no = frame;
		}

        total_clk += clk;
        //fprintf(fp_cycle," cycles = %d,total_cycles = %d \n ",clk,total_clk);
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
                        maxFrameNumber = Frame;
                    }
                    else if (timeVal < minFrameTime)
                    {
                        minFrameTime = timeVal;
                        minFrameNumber = Frame;
                    }
                    totalTime += timeVal;
                }
                timeFlag = 0;
            }
#endif
		dummyCall();
#ifdef WBAMR_DECODER_CONT_TST
		scrCount= 0;

		scratchMem = (WBAMR_S32 *)dec_config->wbamrd_mem_info.mem_info_sub[1].wbappd_base_ptr;
		for (i = 0; i < (dec_config->wbamrd_mem_info.mem_info_sub[1].wbamrd_size/sizeof(WBAMR_S32)); i++)
		{
			if (0x55555555 != scratchMem[i])
				scrCount++;
			if (scrCount > maxScr)
				maxScr = scrCount;
		}
#endif

#ifdef WBAMR_DECODER_STACKDEPTH_TST
		/* Check the Stack usage for the current frame	*/
     	curFrameStackUsage = check_stack_depth(stackTestPtr, 0xA1);
     	if (maxStack < curFrameStackUsage)
     	{
     		maxStack = curFrameStackUsage;
     	}
     	printf("%d\n", maxStack);
#endif

		if (retval != WBAMRD_OK)
		{
			free(in_buf);
			free(out_buf);
			for (i = 0; i < nr; i++)
			{
				free(dec_config->wbamrd_mem_info.mem_info_sub[i].wbappd_base_ptr);
			}
			exit(-1);
		}
#ifdef WBAMR_BIG_ENDIAN
		byteswap(out_buf, WBAMR_L_FRAME);
#endif

#ifdef OUTPUT_DUMP
		fwrite(out_buf, sizeof(WBAMR_S16), WBAMR_L_FRAME, f_synth);
#endif
  	}

#ifdef WBAMR_DECODER_STACKDEPTH_TST
  	/* Print the maximum stack used by the current function */
	fprintf(stderr, "   MAXIMUM STACK USAGE: %d BYTES       \n", maxStack);
#endif

#ifdef WBAMR_DECODER_CONT_TST
	fprintf(stderr, "Scratch = %d bytes\n", maxScr*sizeof(WBAMR_S32));
	/* Check the contents of the extra State nem for the possibility of Overwrite */
	scratchMem = (WBAMR_S32 *)dec_config->wbamrd_mem_info.mem_info_sub[0].wbappd_base_ptr;
	scratchMem += (dec_config->wbamrd_mem_info.mem_info_sub[0].wbamrd_size/sizeof(WBAMR_S32));
	for (i = 0; i < (WBAMR_EXTRASTATE/sizeof(WBAMR_S32)); i++)
	{
		if (0x44444444 != scratchMem[i])
		{
			static int Error = 0;
			//if (Error == 0)
			{
				fprintf(stderr, "WARNING::(%d)StateMem over usage...!\n", i);
				Error++;
			}
		}
	}
 	/* Check the contents of the extra scratch for the possibility of Overwrite */
	scratchMem = (WBAMR_S32 *)dec_config->wbamrd_mem_info.mem_info_sub[1].wbappd_base_ptr;
	scratchMem += (dec_config->wbamrd_mem_info.mem_info_sub[1].wbamrd_size/sizeof(WBAMR_S32));
	for (i = 0; i < (WBAMR_EXTRASCRATCH/sizeof(WBAMR_S32)); i++)
	{
		if (0x55555555 != scratchMem[i])
		{
			static int Warning = 0;
			//if (Warning == 0)
			{
				fprintf(stderr, "WARNING::(%d)Scratch over usage...!\n", i);
				Warning++;
			}
		}
	}

#endif

#if defined(WBAMR_ARM1136) &&  defined(TIME_PROFILE_RVDS )
        #ifdef MEASURE_STACK_USAGE
        #ifdef MEASURE_HEAP_USAGE
 			fprintf(stderr,"\n\n%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t%ld\t%ld\t\n\n",chname, argv[1],
						max_clk, min_clk,frame,max_frame_no,min_frame_no, total_clk*64,s32PeakStack,s32TotalMallocBytes);
 		#endif
 		#endif

        #ifndef MEASURE_STACK_USAGE
        #ifndef MEASURE_HEAP_USAGE
 			fprintf(stderr,"\n\n%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t\n\n",chname, argv[1],
						max_clk, min_clk,frame,max_frame_no,min_frame_no, total_clk*64);
 		#endif
 		#endif

	 avg_clk = (total_clk/frame)*64;
     max_clk = max_clk*64;
	 min_clk = min_clk*64;
     fprintf(fp_cycle, "=======================================================\n");
     fprintf(fp_cycle," Input File: %s Total Frame %d\n", argv[1], frame);
     fprintf(fp_cycle," MAX_MCPS = %f,AVG_MCPS = %f,in frame: %d\n ",(float)max_clk/20000,(float)avg_clk/20000,max_frame_no);
     fprintf(fp_cycle, "\n");
#endif

#ifdef TIME_PROFILE
	printf("\nDecoder");
	printf("\n%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname, argv[1],
						maxFrameTime, minFrameTime,noOfFrames,maxFrameNumber,minFrameNumber, totalTime);
	fprintf(pfCycleCount,"\n%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname, argv[1],
						maxFrameTime, minFrameTime,noOfFrames,maxFrameNumber,minFrameNumber, totalTime);

	fprintf (ph_mips,"\n|\tAudio Format\t|\tPlat\t|\tSamplerate\t|\tCh\t|\tBiterate\t|\tBit\t|\tPerf(MIPS)\t|\tAudio file name\n");
	performance = (double)totalTime * 0.000001*cpu_freq*50/numFrame;	//totalTime * cpu_freq / (numFrame *0.02)
       fprintf(ph_mips, "|\tWB_AMR Dec  \t|\tARM%d\t|\t%9d\t|\t%d\t|\t%8d\t|\t%d\t|\t%.8f\t|%s\n", platform, 16000, 1, mode<9?bit_rate[mode]:0, 16, performance, argv[2]);
	   
    /*fprintf(pfCycleCount,"\tPeak Frame no: %ld,  Peak Time: %ld\n",
                maxFrameNumber, maxFrameTime);
    fprintf(pfCycleCount,"\tMin  Frame no: %ld,  Min Time: %ld\n",
                minFrameNumber, minFrameTime);
    fprintf(pfCycleCount,"\tTotal Frame : %ld, Average Time: %ld\n",
            Frame,Frame?totalTime/numFrame:0);
    fprintf(pfCycleCount, "----------------------------------------\n\n");*/
    fclose(pfCycleCount);
    fclose(ph_mips);
#endif
#ifdef __SYMBIAN32__
#ifdef MEASURE_HEAP_USAGE
			printf("\n Stack and Heap are  %d\t%d\t",s32PeakStack,s32TotalMallocBytes);
#endif
#endif
  	/************************************************************
   	 * Closedown speech coder
   	 ***********************************************************/
    free (out_buf);
    free (in_buf);
    for (i=0; i<nr; i++)
	{
	  free(dec_config->wbamrd_mem_info.mem_info_sub[i].wbappd_base_ptr);
	}
    free (dec_config);

     /************************************************************
   	 * Free al memory allocated for TABLE RELOCATION TEST.
   	 ***********************************************************/

	/* Close all opened Files.  */
	fclose(f_serial);
	fclose(f_synth);

#ifdef	ARM_MIPS_TEST_WINCE
	printf("\nDencoder");
	printf("\n%s\t%s\t%ld",chname,argv[2],maxFrameTime);
	printf("\t%ld",minFrameTime);
	printf("\t%d\t%d\t%d\t%ld\t",numFrame,maxFrameNumber,minFrameNumber,totalTime);
	fprintf(fSysTime,"\n%s\t%s\t%ld",chname,argv[2],maxFrameTime);
	fprintf(fSysTime,"\t%ld",minFrameTime);
	fprintf(fSysTime,"\t%d\t%d\t%d\t%ld\t",numFrame,maxFrameNumber,minFrameNumber,totalTime);
	fclose(fSysTime);
#endif

    return WBAMR_SUCCESS;
}

#ifdef WBAMR_DECODER_STACKDEPTH_TST

extern WBAMR_S32 stackPtrValue(void);

WBAMR_S32 stack_fill(WBAMR_S32 pattern)
{
	WBAMR_S32 stack_ptr, *ptr;
	static WBAMR_S32 i;

	stack_ptr = stackPtrValue();

	ptr = (WBAMR_S32 *)(stack_ptr-1);

	for(i = 1028; i>0 ; i--)
		ptr[i-1028] = pattern;

	return(stack_ptr+8);
}



WBAMR_S32 check_stack_depth(
					WBAMR_S32 stack_ptr,
					WBAMR_U8 pattern)
{

	WBAMR_S32 count=0, i ;
	WBAMR_U8 *ptr1;

	ptr1 = (WBAMR_U8 *)stack_ptr;

	for(i=4096;i>0;i--)
	{
		if(ptr1[i-4096] != pattern)
		{
			count++;
		}

	}

	return(count);

}

#endif

#ifdef WBAMR_BIG_ENDIAN
void byteswap(WBAMR_S16 *ptr, WBAMR_S32 len)
{
	WBAMR_S16 a0, a1, i;
	for (i = 0; i < len; i++)
	{
		a0 = ptr[i];
		a1 = a0;
		a1 <<= 8;
		a1 &= 0xFF00;
		a0 >>= 8;
		a0 &= 0x00FF;
		ptr[i] = (a1 | a0);
	}
	return;
}
#endif


#if !defined(WBAMR_ARM1136) ||  !defined(TIME_PROFILE_RVDS )
WBAMR_S16 Read_serial(FILE * fp, WBAMR_S16 *prms, WBAMR_U8 bitstreamformat)
{
    WBAMR_S16 n, n1, type_of_frame_type, coding_mode, datalen, i;
    WBAMR_U8 toc, q, temp, *packet_ptr, packet[64];
    WBAMR_S16 frame_type,*prms_ptr;
    static WBAMR_S16 framecounter=0;

    prms_ptr = prms;
    prms+=2;

    if(bitstreamformat == 0)				/* default file format */
    {
        n = (WBAMR_S16) fread(&type_of_frame_type, sizeof(WBAMR_S16), 1, fp);
        n = (WBAMR_S16) (n + fread(&frame_type, sizeof(WBAMR_S16), 1, fp));
        n = (WBAMR_S16) (n + fread(&mode, sizeof(WBAMR_S16), 1, fp));
        if (0 == framecounter)
        {
            framecounter++;
        }
        if(n!=3)return 0;

#ifdef WBAMR_BIG_ENDIAN
        byteswap(&type_of_frame_type, 1);
        byteswap(&frame_type, 1);
        byteswap(&mode, 1);
#endif
        coding_mode = mode;
        if(mode < 0 || mode > NUM_OF_MODES-1)
        {
            fprintf(stderr, "Invalid mode received: %d (check file format).\n", mode);
            exit(-1);
        }

        if (type_of_frame_type == TX_FRAME_TYPE)
        {
            switch (frame_type)
            {
                case TX_SPEECH:
                    frame_type = RX_SPEECH_GOOD;
                    break;
                case TX_SID_FIRST:
                    frame_type = RX_SID_FIRST;
                    break;
                case TX_SID_UPDATE:
                    frame_type = RX_SID_UPDATE;
                    break;
                case TX_NO_DATA:
                    frame_type = RX_NO_DATA;
                    break;
            }
        } else if (type_of_frame_type != RX_FRAME_TYPE)
        {
            fprintf(stderr, "Wrong type of frame type:%d.\n", type_of_frame_type);
        }

        if ((frame_type == RX_SID_FIRST) | (frame_type == RX_SID_UPDATE) | (frame_type == RX_NO_DATA) | (frame_type == RX_SID_BAD))
        {
            coding_mode = MRDTX;
        }
        n = (WBAMR_S16) fread(prms, sizeof(WBAMR_S16), nb_of_bits[coding_mode], fp);
#ifdef WBAMR_BIG_ENDIAN
        byteswap(prms, n);
#endif
        if (n != nb_of_bits[coding_mode])
            n = 0;

        prms_ptr[0] = frame_type;
        prms_ptr[1] = mode;
        return (n);
    }
    else
    {
        if (bitstreamformat == 1)		/* ITU file format */
        {
            n = (WBAMR_S16) fread(&type_of_frame_type, sizeof(WBAMR_S16), 1, fp);
            n = (WBAMR_S16)(n+fread(&datalen, sizeof(WBAMR_S16), 1, fp));
            if(n!=2)return 0;
#ifdef WBAMR_BIG_ENDIAN
            byteswap(&type_of_frame_type, 1);
            byteswap(&datalen, 1);
#endif

            if (datalen != 0)
            {
                coding_mode = -1;
                for(i=NUM_OF_MODES-1; i>=0; i--)
                {
                    if(datalen == nb_of_bits[i])
                    {
                        coding_mode = i;
                    }
                }
                if(coding_mode == -1)
                {
                    fprintf(stderr, "\n\n ERROR: Invalid number of data bits received [%d]\n\n", datalen);
                    exit(-1);
                }
            }

            n1 = fread(prms, sizeof(WBAMR_S16), datalen, fp);
#ifdef WBAMR_BIG_ENDIAN
            byteswap(prms, n1);
#endif
            prms_ptr[0] = type_of_frame_type;
            prms_ptr[1] = datalen;
            return(n);

        }
        else if(bitstreamformat==2)			/* MIME/storage file format */
        {
            /* read ToC byte, return immediately if no more data available */
            if (fread(&toc, sizeof(WBAMR_U8), 1, fp) == 0)
            {
                return 0;
            }

            /* extract q and mode from ToC */
            q  = (toc >> 2) & 0x01;
            mode = (toc >> 3) & 0x0F;

            /* read speech bits, return with empty frame if mismatch between mode info and available data */
            if ((WBAMR_S16)fread(packet, sizeof(WBAMR_U8), packed_size[mode], fp) != packed_size[mode])
            {
                return 0;
            }

            packet_ptr = (WBAMR_U8 *) prms_ptr;


            *packet_ptr++ = q;
            *packet_ptr++ = (WBAMR_U8) mode;

            for (i = 0; i < packed_size[mode]; i++)
            {
                *packet_ptr++ = packet[i];
            }

            /* return 1 to indicate succesfully parsed frame */
            return 1;
        }

        /* if IF1 format or IF2 format.. */
        else
        {
            /* read ToC byte, return immediately if no more data available */
            if (fread(&temp, sizeof(WBAMR_U8), 1, fp) == 0)
            {
                return 0;
            }

            /* extract mode from ToC */
            mode = (temp >> 4) & 0x0F;		/* frame_type */

            if (bitstreamformat == 3)
            {
                /* read speech bits, return with empty frame if mismatch between mode info and available data */
                if ((WBAMR_S16)fread(packet, sizeof(WBAMR_U8), if2_packed_size[mode]-1, fp) != if2_packed_size[mode]-1)
                {
                    return 0;
                }
            }
            else
            {
                if ((WBAMR_S16)fread(packet, sizeof(WBAMR_U8), if1_packed_size[mode]-1, fp) != if1_packed_size[mode]-1)
                {
                    return 0;
                }
            }

            packet_ptr = (WBAMR_U8 *) prms_ptr;

            *packet_ptr++ = temp;
            if (bitstreamformat == 3)
            {
                *packet_ptr++ = (WBAMR_U8) mode;

                for (i = 0; i < if2_packed_size[mode]-1; i++)
                {
                    *packet_ptr++ = packet[i];
                }
            }
            else
            {
                for (i = 0; i < if1_packed_size[mode]-1; i++)
                {
                    *packet_ptr++ = packet[i];
                }
            }

            /* return 1 to indicate succesfully parsed frame */
            return 1;

        }
#undef MRSID
    }
}
#else
WBAMR_S16 Read_serial(FILE * fp, WBAMR_S16 *prms, WBAMR_U8 bitstreamformat)
{
    WBAMR_S16 n, type_of_frame_type, coding_mode;
    WBAMR_S16 frame_type,*prms_ptr;

    WBAMR_S16 size_to_read = 0;
    WBAMR_S16 prms_index = 0;
    WBAMR_S16 index = 0;

    prms_ptr = prms;
    prms+=2;

    if (bitstreamformat == 0)
    {
        if (size_rem_data == 3)
        {
            type_of_frame_type = rem_data[1];
            frame_type = rem_data[2];
            mode = rem_data[3];
        }
        else if (size_rem_data == 2)
        {
            type_of_frame_type = rem_data[2];
            frame_type = rem_data[3];
            n = (WBAMR_S16) fread(&rem_data[0], sizeof(WBAMR_S16), 4, fp);
            if (n != 4)
                return 0;
            mode = rem_data[0];
            prms[0] = rem_data[1];
            prms[1] = rem_data[2];
            prms[2] = rem_data[3];
            prms_index = 3;
        }
        else if (size_rem_data == 1)
        {
            type_of_frame_type = rem_data[3];
            n = (WBAMR_S16) fread(&rem_data[0], sizeof(WBAMR_S16), 4, fp);
            if (n != 4)
                return 0;
            frame_type = rem_data[0];
            mode = rem_data[1];
            prms[0] = rem_data[2];
            prms[1] = rem_data[3];
            prms_index = 2;
        }
        else
        {
            n = (WBAMR_S16) fread(&rem_data[0], sizeof(WBAMR_S16), 4, fp);
            if (n != 4)
                return 0;
            type_of_frame_type = rem_data[0];
            frame_type = rem_data[1];
            mode = rem_data[2];

            prms[0] = rem_data[3];
            prms_index = 1;
        }
        size_rem_data = 0;

#ifdef WBAMR_BIG_ENDIAN
        byteswap(&type_of_frame_type, 1);
        byteswap(&frame_type, 1);
        byteswap(&mode, 1);
#endif
        coding_mode = mode;
        if(mode < 0 || mode > NUM_OF_MODES-1)
        {
            fprintf(stderr, "Invalid mode received: %d (check file format).\n", mode);
            exit(-1);
        }

        if (type_of_frame_type == TX_FRAME_TYPE)
        {
            switch (frame_type)
            {
                case TX_SPEECH:
                    frame_type = RX_SPEECH_GOOD;
                    break;
                case TX_SID_FIRST:
                    frame_type = RX_SID_FIRST;
                    break;
                case TX_SID_UPDATE:
                    frame_type = RX_SID_UPDATE;
                    break;
                case TX_NO_DATA:
                    frame_type = RX_NO_DATA;
                    break;
            }
        }
        else if (type_of_frame_type != RX_FRAME_TYPE)
        {
            fprintf(stderr, "Wrong type of frame type:%d.\n", type_of_frame_type);
        }

        if ((frame_type == RX_SID_FIRST) | (frame_type == RX_SID_UPDATE) | (frame_type == RX_NO_DATA) | (frame_type == RX_SID_BAD))
        {
            coding_mode = MRDTX;
        }

        size_rem_data = (nb_of_bits[coding_mode] - prms_index) % 4;

        size_to_read = (nb_of_bits[coding_mode] -prms_index) - size_rem_data;

        n = (WBAMR_S16) fread(&prms[prms_index], sizeof(WBAMR_S16), size_to_read, fp);

        if ( n == size_to_read)
        {
            if (size_rem_data != 0)
            {
                n = (WBAMR_S16) fread(&rem_data[0], sizeof(WBAMR_S16), 4, fp);
                if (n >= size_rem_data)
                {
                    for (index = 0; index <size_rem_data; index++)
                    {
                        prms[size_to_read+prms_index+index] = rem_data[index];
                    }
                    size_rem_data = 4 - index;
                }
            }
            n = nb_of_bits[coding_mode];
        }
        prms_index = 0;
#ifdef WBAMR_BIG_ENDIAN
        byteswap(prms, n);
#endif
        if (n != nb_of_bits[coding_mode])
            n = 0;

        prms_ptr[0] = frame_type;
        prms_ptr[1] = mode;
        return (n);
    }
    return 0;
}
#endif


#if defined(WBAMR_ARM1136) &&  defined(TIME_PROFILE_RVDS )
int mystrcmp(const char* pcStr1, const char* pcStr2)
{
    const char* pcLocal1 = pcStr1;
    const char* pcLocal2 = pcStr2;
    WBAMR_S32   diff;

    /* TEDebug ("Comparing strings %s and %s\n", pcStr1, pcStr2); */

    do
    {
        diff = (WBAMR_S32)(*pcLocal1 - *pcLocal2);

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
#endif

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

#ifdef TGT_CPU_ARM12
#ifndef TGT_OS_ELINUX
__irq void IRQ_Handler(void)    	//Lyon 070731
{


   *timer_clear	= 1;
   *irq_enable_clr = 0x02;
   *irq_enable_set = 0x02;
}
#endif
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
