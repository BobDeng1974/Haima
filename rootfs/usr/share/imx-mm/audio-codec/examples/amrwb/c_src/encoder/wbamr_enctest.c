
/**********************************************************************
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
*
*  AMR Wideband Codec 3GPP TS26.190 / ITU-T G.722.2, April 30, 2003.
*
***********************************************************************
*
* File Name			: amrwb_enctest.c
*
* Description		: test frame work for WBAMR Encoder.
*
* Functions Included: main()
*					: stack_fill()
*					: check_stack_depth()
****************************** Change History**************************
*
*    DD/MM/YYYY     Code Ver     Description                      Author
*    -----------     --------     -----------                     ------
*    03/11/2004	    v1.0         Initial version                  Naganna/Shashi
*    13/12/2006     v1.1         Added profiling code for RVDS    Shyam Krishnan M
*    28/07/2008     v1.2         Added profiling code for ARM9    r66286
**********************************************************************/


/********************************************************************
 *                         INCLUDE FILES
 *******************************************************************/
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

#include "wbamr_enc_interface.h"
#include "wbamr_enctest.h"

#ifdef __WINCE
#include<windows.h>
#endif

#define OUTPUT_DUMP
#if defined TIME_PROFILE || defined TIME_PROFILE_RVDS || defined TIME_PROFILE_RVDS_ARM9|| defined ARM_MIPS_TEST_WINCE
#undef OUTPUT_DUMP
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

#if WBAMR_ENCODER_STACKDEPTH_TST
WBAMR_S32 stack_fill(WBAMR_S32 pattern);
WBAMR_S32 check_stack_depth(WBAMR_S32 stack_ptr, WBAMR_U8 pattern);
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

#ifdef MEASURE_STACK_USAGE
#include "mm_codec_common.h"
#endif

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

//extern WBAMR_S16 a_hp_wsp[];

/********************************************************************
 *        MAIN PROGRAM
 *******************************************************************/

int main (int argc, char *argv[])
{
  	WBAMR_S16 *in_buf;						/* Pointer to new speech data*/
	WBAMR_S16 *out_buf;						/* Output bitstream buffer */
	WBAMRE_RET_TYPE retval;
    WBAMR_U8 bitstreamformat = 0;
	WBAMR_S16 nr;

	/* disable encoder DTX */

  	WBAMR_S32 i;
  	WBAMRE_Mem_Alloc_Info_Sub *mem;
  	WBAMRE_Encoder_Config *enc_config;


	WBAMR_S16 allow_dtx, mode_file;
	WBAMR_U16 nb_bits;
	WBAMR_S32 mode = 0;
    WBAMR_S16 mode_in=0;
    FILE *f_speech;                        /* File of speech data                   */
    FILE *f_serial;                        /* File of serial bits for transmission  */
    FILE *f_mode = NULL;                   /* File of modes for each frame          */

    unsigned long infilesize = 0;
#ifdef MEASURE_STACK_USAGE
    int           *ps32SP, *ps32BaseSP;
    int           s32StackCount, s32StackValue;
    int           s32PeakStack = 0;
#endif
#ifdef MEASURE_HEAP_USAGE
    int      s32TotalMallocBytes=0;
#endif
#ifdef WBAMR_ENCODER_CONT_TST
	WBAMR_S32 *scratchMem,maxScr = 0,scrCount = 0;
#endif
#ifdef WBAMR_ENCODER_STACKDEPTH_TST
	WBAMR_U32 stackTestPtr;
	WBAMR_S32 maxStack = 0, curFrameStackUsage;
#endif
#if defined(WBAMR_ARM1136) &&  defined(TIME_PROFILE_RVDS )
    WBAMR_S32 prev_clk, curr_clk, clk,max_frame_no,min_frame_no;
    WBAMR_S32 total_clk,max_clk,min_clk,avg_clk,frame;
    FILE *fp_cycle;
    unsigned char chname[] = "[PROFILE-INFO]";
    fp_cycle=fopen("wbamr_enc_cycle.txt", "a");
#endif
#ifdef ALIGNMENT_CHECK
    int cp_ctrl_data = 0;       /* to store CP15 control register data */
#endif

#ifdef TIME_PROFILE
    FILE    *pfCycleCount, *ph_mips;
    struct timeval tv_prof;
    long time_before =0, time_after=0;
    int retVal = -1;
    int minFrameNumber=0, maxFrameNumber=0, numFrame =0,Frame=0;
    long minFrameTime=0x7FFFFFFF;
    long maxFrameTime =0, totalTime =0;
    long timeVal=0;
    unsigned char timeFlag =0;
    unsigned char chname[] = "[PROFILE-INFO]";
    int bit_rate[9] = {6600, 8850, 12650, 14250, 15850, 18250, 19850, 23050, 23850};
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
#ifdef __SYMBIAN32__
    pfCycleCount = fopen ("D:\\wbamr_enc_cycle.txt", "a");
#else //__SYMBIAN32__
	pfCycleCount = fopen ("wbamr_enc_cycle.txt", "a");
       ph_mips = fopen("../audio_performance.txt","a");
#endif //__SYMBIAN32__
#endif

#ifdef ARM_MIPS_TEST_WINCE
    FILE    *fSysTime;
    fSysTime = fopen ("wb_amr_enc_sys_time.txt", "a");
    if(!fSysTime)
    {
		printf("\nUnable to open log file g723_enc_sys_time.txt\n");
		exit(1);
	}
#endif

    /*----- Report Current Version   ---------------------------------*/
    printf("\nRunning %s\n",WBAMR_get_version_info());

    /*-------------------------------------------------------------------------*
     * Open speech file and result file (output serial bit stream)             *
     *-------------------------------------------------------------------------*/

     if ((argc < 4) || (argc > 6))
    {
        fprintf(stderr, "Usage : coder  (-dtx) (-itu | -mime | -if1 | -if2) mode speech_file  bitstream_file \n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Format for speech_file:\n");
        fprintf(stderr, "  Speech is read form a binary file of 16 bits data.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Format for bitstream_file (default):\n");
        fprintf(stderr, "  One word (2-byte) to indicate type of frame type.\n");
        fprintf(stderr, "  One word (2-byte) to indicate frame type.\n");
        fprintf(stderr, "  One word (2-byte) to indicate mode.\n");
        fprintf(stderr, "  N words (2-byte) containing N bits (bit 0 = 0xff81, bit 1 = 0x007f).\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "  if option -itu defined:\n");
		fprintf(stderr, "  One word (2-byte) for sync word (0x6b21)\n");
		fprintf(stderr, "  One word (2-byte) for frame length N.\n");
		fprintf(stderr, "  N words (2-byte) containing N bits (bit 0 = 0x007f, bit 1 = 0x0081).\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "  if option -mime defined:\n");
		fprintf(stderr, "  AMR-WB MIME/storage format, see RFC 3267 (sections 5.1 and 5.3) for details.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "  if option -if1 defined:\n");
		fprintf(stderr, "  Except the magic word, this is same as mime, includes CRC.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "  if option -if2 defined:\n");
		fprintf(stderr, "  Except the magic word, this is same as mime.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "mode: 0 to 8 (9 bits rates) or\n");
        fprintf(stderr, "      -modefile filename\n");
        fprintf(stderr, " ===================================================================\n");
        fprintf(stderr, " mode   :  (0)  (1)   (2)   (3)   (4)   (5)   (6)   (7)   (8)     \n");
        fprintf(stderr, " bitrate: 6.60 8.85 12.65 14.25 15.85 18.25 19.85 23.05 23.85 kbit/s\n");
        fprintf(stderr, " ===================================================================\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "-dtx if DTX is ON, default is OFF\n");
        fprintf(stderr, "\n");
        exit(0);
    }
    allow_dtx = 0;

#ifndef TIME_PROFILE_RVDS
    if (strcmp(argv[1], "-dtx") == 0)
#else
    if (mystrcmp(argv[1], "-dtx") == 0)
#endif
    {
        allow_dtx = 1;
        argv++;
    }

	bitstreamformat = 0;

#ifndef TIME_PROFILE_RVDS
    if (strcmp(argv[1], "-itu") == 0)
#else
    if (mystrcmp(argv[1], "-itu") == 0)
#endif
	{
	    bitstreamformat = 1;
	    argv++;
		fprintf(stderr, "Input bitstream format: ITU\n");
    } else
	{
#ifndef TIME_PROFILE_RVDS
		if (strcmp(argv[1], "-mime") == 0)
#else
		if (mystrcmp(argv[1], "-mime") == 0)
#endif
		{
			bitstreamformat = 2;
			argv++;
			fprintf(stderr, "Input bitstream format: MIME\n");
		}
#ifndef TIME_PROFILE_RVDS
        else if (strcmp(argv[1], "-if2") == 0)
#else
        else if (mystrcmp(argv[1], "-if2") == 0)
#endif
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

	mode_file = 0;
#ifndef TIME_PROFILE_RVDS
    if (strcmp(argv[1], "-modefile") == 0)
#else
    if (mystrcmp(argv[1], "-modefile") == 0)
#endif
    {
        mode_file = 1;
        argv++;
        if ((f_mode = fopen(argv[1], "r")) == NULL)
        {
            fprintf(stderr, "Error opening input file  %s !!\n", argv[1]);
            exit(0);
        }
        fprintf(stderr, "Mode file:  %s\n", argv[1]);
    } else
    {
        mode = (WBAMR_S32)atoi(argv[1]);
        if ((mode < 0) || (mode > 8))
        {
            fprintf(stderr, " error in bit rate mode %d: use 0 to 8\n", mode);
            exit(0);
        }
        /*else
            nb_bits = nb_of_bits[mode];
        */
    }

    if ((f_speech = fopen(argv[2], "rb")) == NULL)
    {
        fprintf(stderr, "Error opening input file  %s !!\n", argv[2]);
        exit(0);
    }
    fprintf(stderr, "Input speech file:  %s\n", argv[2]);

    if ((f_serial = fopen(argv[3], "wb")) == NULL)
    {
        fprintf(stderr, "Error opening output bitstream file %s !!\n", argv[3]);
        exit(0);
    }
    fprintf(stderr, "Output bitstream file:  %s\n", argv[3]);



  	/* allocate memory for encoder configuration structure */

  	enc_config=(WBAMRE_Encoder_Config *)alloc_fast(sizeof(WBAMRE_Encoder_Config));


  	enc_config->wbappe_initialized_data_start = NULL ; //TLSbo86158

													/* Initialize config structure memory to NULL;*/
	for (i=0; i< WBAMR_MAX_NUM_MEM_REQS; i++)
	{
		enc_config->wbamre_mem_info.mem_info_sub[i].wbappe_base_ptr =NULL;
	}
 	retval = wbamre_query_enc_mem(enc_config);		/* Query for memory */
  	if (retval != WBAMRE_OK)
	{
		free(enc_config);							/* deallocate memory allocated for encoder config */
		return WBAMR_SUCCESS;
 	}

   	nr = enc_config->wbamre_mem_info.wbamre_num_reqs;	/* Number of memory chunk requests by the encoder */

 													/* allocate memory requested by the encoder*/
	mem = &(enc_config->wbamre_mem_info.mem_info_sub[0]);
	if (mem->wbamre_type_fs == WBAMR_FAST_MEMORY)
  	{
		mem->wbappe_base_ptr = alloc_fast(mem->wbamre_size + WBAMR_EXTRASTATE);
		#ifdef MEASURE_HEAP_USAGE
			s32TotalMallocBytes +=  (mem->wbamre_size + WBAMR_EXTRASTATE);
		#endif
	}
	else
	{
		mem->wbappe_base_ptr = alloc_slow(mem->wbamre_size + WBAMR_EXTRASTATE);
		#ifdef MEASURE_HEAP_USAGE
			s32TotalMallocBytes +=  (mem->wbamre_size + WBAMR_EXTRASTATE);
		#endif
	}

	mem = &(enc_config->wbamre_mem_info.mem_info_sub[1]);
	if (mem->wbamre_type_fs == WBAMR_FAST_MEMORY)
  	{
		mem->wbappe_base_ptr = alloc_fast(mem->wbamre_size + WBAMR_EXTRASCRATCH);
		#ifdef MEASURE_HEAP_USAGE
			s32TotalMallocBytes +=  (mem->wbamre_size + WBAMR_EXTRASCRATCH);
		#endif
	}
	else
	{
		mem->wbappe_base_ptr = alloc_slow(mem->wbamre_size + WBAMR_EXTRASCRATCH);
		#ifdef MEASURE_HEAP_USAGE
			s32TotalMallocBytes +=  (mem->wbamre_size + WBAMR_EXTRASCRATCH);
		#endif
	}
      #ifdef TGT_CPU_ARM9
	 enc_config->wbappe_initialized_data_start = (WBAMR_U8 *)BEGIN_WBAMRE_DATA;
      #endif
	enc_config->wbappe_mode = (WBAMR_S16 *)&mode_in;	/* set encoding mode here */
	enc_config->wbappe_dtx_flag = allow_dtx;			/* enable DTX mode here */
	enc_config->wbamre_output_size = (WBAMR_U16*)&nb_bits;
	enc_config->wbamre_output_format =bitstreamformat;

	retval = wbamre_encode_init (enc_config);			/* initialize encoder */

	if (WBAMRE_OK != retval)
	{
		for (i=0; i<nr; i++)
		{
			free (enc_config->wbamre_mem_info.mem_info_sub[i].wbappe_base_ptr);
		}
		free (enc_config);							/* free all the memory allocated for encoder config param */
		return WBAMR_FAILURE;
	}
													/* allocate memory for input buffer */
	if ((in_buf = alloc_fast((WBAMR_L_FRAME*sizeof (WBAMR_S16)))) == NULL)
	{
		for (i=0; i<nr; i++)
		{
			free (enc_config->wbamre_mem_info.mem_info_sub[i].wbappe_base_ptr);
		}
		free (enc_config);							/*free all the memory allocated for encoder config param*/
		return WBAMR_FAILURE;
	}

		out_buf = alloc_fast ((WBAMR_SERIAL_FRAMESIZE*sizeof(WBAMR_S16)));
	//	out_buf = alloc_fast (MAX_PACKED_SIZE * sizeof (WBAMR_U8));

													/* allocate memory for output buffer (unpacked) */
	if (out_buf == NULL)
	{
		for (i=0; i<nr; i++)
		{
			free (enc_config->wbamre_mem_info.mem_info_sub[i].wbappe_base_ptr);
		}
		free (enc_config);							/* free all the memory allocated for encoder config param */
		free(in_buf);
		return WBAMR_FAILURE;
	};

#ifdef WBAMR_ENCODER_CONT_TST
	/* Fill the Extra State Mem buffer with 55's to check the usage of extra scratch.
	*/
	scratchMem = (WBAMR_S32 *)enc_config->wbamre_mem_info.mem_info_sub[0].wbappe_base_ptr;
	scratchMem += ((enc_config->wbamre_mem_info.mem_info_sub[0].wbamre_size)/sizeof(WBAMR_S32));
	for (i = 0; i < (WBAMR_EXTRASTATE/sizeof(WBAMR_S32)); i++)
	{
		scratchMem[i] = 0x44444444;
	}

	/* Fill the Extra Scratch buffer with 55's to check the usage of extra scratch.
	*/
	scratchMem = (WBAMR_S32 *)enc_config->wbamre_mem_info.mem_info_sub[1].wbappe_base_ptr;
	scratchMem += ((enc_config->wbamre_mem_info.mem_info_sub[1].wbamre_size)/sizeof(WBAMR_S32));
	for (i = 0; i < (WBAMR_EXTRASCRATCH/sizeof(WBAMR_S32)); i++)
	{
		scratchMem[i] = 0x55555555;
	}

#endif


 	/* If MIME/storage format selected, write the magic number
	 * at the beginning of the bitstream file
 	 */
    if (bitstreamformat == 2)
    {
        fwrite("#!AMR-WB\n", sizeof(char), 9, f_serial);
    }

#if defined(WBAMR_ARM1136) &&  defined(TIME_PROFILE_RVDS )
	frame=0;
	total_clk = max_clk = avg_clk = max_frame_no= min_frame_no=0;
	min_clk=0x7fffffff;
#endif

	/*************************************************************
     *	 Encode speech frame by frame till end of frame			 *
	 *************************************************************/
	while (fread(in_buf, sizeof(WBAMR_S16), WBAMR_L_FRAME, f_speech) == WBAMR_L_FRAME)
	{
		infilesize +=WBAMR_L_FRAME;
		if (mode_file)
		{
			if (fscanf(f_mode, "%hd", &mode) == EOF)
			{
				mode = *enc_config->wbappe_mode;
				fprintf(stderr, "\n end of mode control file reached\n");
				fprintf(stderr, " From now on using mode: %hd\n", mode);
				mode_file = 0;
			}
#ifdef WBAMR_BIG_ENDIAN
			mode >>= 16;
#endif
		}

		if ((mode < 0) || (mode > 8))
		{
			fprintf(stderr, " error in bit rate mode %hd: use 0 to 8\n", mode);
			exit(0);
		}


		*enc_config->wbappe_mode = (WBAMR_S16)mode;				/* set encoding mode here */

		for (i = 0; i < WBAMR_SERIAL_FRAMESIZE; i++)
			out_buf[i] = 0;							/* zero flags and parameter bits */

#ifdef WBAMR_ENCODER_CONT_TST
		/* Get the scratch pointer. */
		scratchMem = (WBAMR_S32 *)enc_config->wbamre_mem_info.mem_info_sub[1].wbappe_base_ptr;
		/* Contaminate the Encode Scratch with 55's.....
		*/
		for(i=0;i<(enc_config->wbamre_mem_info.mem_info_sub[1].wbamre_size)/sizeof(WBAMR_S32);i++)
			scratchMem[i]=0x55555555;
#endif

#ifdef WBAMR_ENCODER_STACKDEPTH_TST
		stackTestPtr = stack_fill(0x31313131);
#endif
#ifdef WBAMR_BIG_ENDIAN
		byteswap(in_buf, WBAMR_L_FRAME);
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
#ifdef MEASURE_STACK_USAGE
        PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
#endif

#if defined(WBAMR_ARM1136) &&  defined(TIME_PROFILE_RVDS )
	    frame++;

        //fprintf(stderr, " Frames processed:			%hd\r", frame);
		prev_clk = prev_cycles();
#endif

#ifdef TIME_PROFILE_RVDS_ARM9
		dummyCall1();
#endif
#ifdef	ARM_MIPS_TEST_WINCE						/* for taking timing on wince platform*/
		INTERRUPTS_SET(0xDF);  						/*disable interrupt*/
		Flag=QueryPerformanceFrequency(&lpFrequency);
		Flag=QueryPerformanceCounter(&lpPerformanceCountBegin);
#endif

		/* encode speech */
		retval=wbamre_encode_frame(enc_config, in_buf, out_buf);

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

#ifdef WBAMR_ENCODER_STACKDEPTH_TST

     	curFrameStackUsage = check_stack_depth(stackTestPtr, 0x31);

     	if (maxStack < curFrameStackUsage)
     	{
     		maxStack = curFrameStackUsage;
     	}
#endif

		if (retval != WBAMRE_OK)
		{
			free (out_buf);
      		free (in_buf);
			for (i=0; i<nr; i++)
			{
				free (enc_config->wbamre_mem_info.mem_info_sub[i].wbappe_base_ptr);
			}
      		free (enc_config);
			exit (-1);
		}

#ifdef WBAMR_ENCODER_CONT_TST
		scrCount = 0;
		scratchMem = (WBAMR_S32 *)enc_config->wbamre_mem_info.mem_info_sub[1].wbappe_base_ptr;
		/* Contaminate the Encode Scratch with 55's.....	*/
		for(i=0;i<(enc_config->wbamre_mem_info.mem_info_sub[1].wbamre_size)/sizeof(WBAMR_S32); i++)
			if (scratchMem[i] != 0x55555555)
				scrCount++;
		if (scrCount > maxScr)
			maxScr = scrCount;
#endif
        /* if1 format */
		if(enc_config->wbamre_output_format == 4)
        {
            /* IF1 frame format returns number of bits based on the
             * mode used. This
             * includes the header part also
             * Following calculation is done to get the number of
             * packed bytes to be
             * written into the file. 1 is added for the remainder bits.
             */

//In AMR WB IF1 formt, the SID frame size is corrected to 8 bytes.
//TLSbo60278
            if(*enc_config->wbamre_output_size %8)
            *enc_config->wbamre_output_size =\
                    (*enc_config->wbamre_output_size >> 3) + 1;
             else
            *enc_config->wbamre_output_size =\
                    (*enc_config->wbamre_output_size >> 3);
        }

		if(enc_config->wbamre_output_format<2)
		{
#ifdef WBAMR_BIG_ENDIAN
			byteswap(out_buf, *enc_config->wbamre_output_size);
#endif

#ifdef OUTPUT_DUMP
			fwrite(out_buf, sizeof(WBAMR_S16), *enc_config->wbamre_output_size, f_serial);
#endif
		}
		else
	    {
#ifdef OUTPUT_DUMP
			fwrite((WBAMR_U8 *)out_buf, sizeof(WBAMR_U8), *enc_config->wbamre_output_size, f_serial);
#endif
        }

        }

#ifdef WBAMR_ENCODER_STACKDEPTH_TST
	/* Print the maximum stack used by the current function  */
	fprintf(stderr, "  MAXIMUM STACK USAGE: %d bytes   \n", maxStack);
#endif

#ifdef WBAMR_ENCODER_CONT_TST

	fprintf(stderr, "Scratch = %d bytes\n", maxScr*sizeof(WBAMR_S32));
  	/* Check the contents of the extra State nem for the possibility of Overwrite...*/
	scratchMem = (WBAMR_S32 *)enc_config->wbamre_mem_info.mem_info_sub[0].wbappe_base_ptr;
	scratchMem += ((enc_config->wbamre_mem_info.mem_info_sub[0].wbamre_size)/sizeof(WBAMR_S32));
	for (i = 0; i < (WBAMR_EXTRASTATE/sizeof(WBAMR_S32)); i++)
	{
		if (0x44444444 != scratchMem[i])
		{
			static int Error = 0;
			if (Error == 0)
			{
				fprintf(stderr, "WARNING::StateMem over usage...!\n");
				Error++;
			}
		}
	}
  	/* Check the contents of the extra scratch for the possibility of Overwrite....*/
	scratchMem = (WBAMR_S32 *)enc_config->wbamre_mem_info.mem_info_sub[1].wbappe_base_ptr;
	scratchMem += ((enc_config->wbamre_mem_info.mem_info_sub[1].wbamre_size)/sizeof(WBAMR_S32));
	for (i = 0; i < (WBAMR_EXTRASCRATCH/sizeof(WBAMR_S32)); i++)
	{
		if (0x55555555 != scratchMem[i])
		{
			static int Warning = 0;
			if (Warning == 0)
			{
				fprintf(stderr, "WARNING::Scratch over usage...!\n");
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
     fprintf(fp_cycle,"Input File: %s and MODE: %d\n",  argv[2], mode);
     fprintf(fp_cycle," MAX_MCPS = %f,AVG_MCPS = %f,in frame: %d\n ",(float)max_clk/20000,(float)avg_clk/20000,max_frame_no);
     fprintf(fp_cycle, "\n");
     fclose (fp_cycle);
#endif

 #ifdef TIME_PROFILE
 	printf("\nEncoder");
 	printf("\n%s\t%s\t%ld\t%d\t%ld\t%ld\t%d\t%d\t%ld\t%ld\t",chname, argv[2],2*infilesize,mode,
						maxFrameTime,minFrameTime,noOfFrames,maxFrameNumber,minFrameNumber,totalTime);
	
	fprintf (ph_mips,"\n|\tAudio Format\t|\tPlat\t|\tSamplerate\t|\tCh\t|\tBiterate\t|\tBit\t|\tPerf(MIPS)\t|\tAudio file name\n");
	performance = (double)totalTime * 0.000001*cpu_freq*50/numFrame;	//totalTime * cpu_freq / (numFrame *0.02)
       fprintf(ph_mips, "|\tWB_AMR Enc  \t|\tARM%d\t|\t%9d\t|\t%d\t|\t%8d\t|\t%d\t|\t%.8f\t|%s\n", platform, 16000, 1, mode<9?bit_rate[mode]:0, 16, performance, argv[3]);

	fprintf(pfCycleCount,"\n%s\t%s\t%ld\t%d\t%ld\t%ld\t%d\t%d\t%ld\t%ld\t",chname, argv[2],2*infilesize,mode,
						maxFrameTime,minFrameTime,noOfFrames,maxFrameNumber,minFrameNumber,totalTime);

    /*fprintf(pfCycleCount,"\tPeak Frame no: %ld,  Peak Time: %ld\n",
                maxFrameNumber, maxFrameTime);
    fprintf(pfCycleCount,"\tMin Frame no: %ld,  Min Time: %ld\n",
                minFrameNumber, minFrameTime);
    fprintf(pfCycleCount,"\tTotal Frame : %ld, Average Time: %ld\n",
            Frame, Frame?totalTime/numFrame:0);
    fprintf(pfCycleCount, "----------------------------------------\n\n");*/
    fclose(pfCycleCount);
    fclose(ph_mips);
#endif
 #ifdef __SYMBIAN32__
#ifdef MEASURE_HEAP_USAGE
			printf("\n Stack and Heap are  %d\t%d\t",s32PeakStack,s32TotalMallocBytes);
#endif
#endif
 	/*************************************************************
  	 * Closedown speech coder
  	 ************************************************************/
  	free (out_buf);
  	free (in_buf);

  	for (i=0; i<nr; i++)
	{
		free (enc_config->wbamre_mem_info.mem_info_sub[i].wbappe_base_ptr);
	}
	free (enc_config);

	/*************************************************************
	 * Close down all files.
	 *************************************************************
	 */
	if(f_mode != NULL)
	{
		fclose(f_mode);
	}
	fclose(f_speech);
	fclose(f_serial);

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

#if WBAMR_ENCODER_STACKDEPTH_TST

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

	WBAMR_S32 count=0, i;
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

#ifdef TIME_PROFILE_RVDS
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
