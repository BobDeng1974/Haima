
/***************************************************************************
* Property of Motorola, Motorola India Electronics Pvt Limited
****************************************************************************
* ANSI C source code
*
* Project Name : MP3 Decoder
*
* MOTOROLA CONFIDENTIAL PROPRIETARY
***************************************************************************/

/*
 ***********************************************************************
 * Copyright (c) 2005-2010, 2012, 2014, Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc.
 ***********************************************************************
 */

/**************************************************************************
 *   dd/mm/yy   Code Ver     Description                        Author
 *   --------   -------      -----------                        ------
 *   21/05/99   00           24-bit output instead of 16-bit    K.Srinivasan
 *   25/05/99   01           Removed warnings                   Arun Hiregange
 *   25/05/99   02           Added ifdefs for unused functions, Arun Hiregange
 *                           and layer 1/2/3 specific functions.
 *   26/05/99   03           Removed temporary arrays of size   Arun Hiregange
 *                           [SBLIMIT][SSLIMIT] by doing in-place
 *                           computations.
 *   26/05/99   04           Made a common memory for re[],     Arun Hiregange
 *                           is[] and is_pos[].
 *   27/05/99   05           Main data buffer reduced from      Arun Hiregange
 *                           4096 to 1536.
 *   28/05/99   06           Removed need for mem_alloc/free    Arun Hiregange
 *   28/05/99   07           Added ifdefs for printfs, file     Arun Hiregange
 *                           I/O related functions.
 *   31/05/99   08           Added print_stat() calls.          K.Srinivasan
 *   31/05/99   09           Made output array from long to int Arun Hiregange
 *   31/05/99   10           Moved coefficient array from       Arun Hiregange
 *                           stack to global.
 *   01/06/99   11           Added code for Onyx specific       Arun Hiregange
 *                           functionality.
 *   01/06/99   12           Corrected the common_buff to       Arun Hiregange
 *                           double from char to avoid bus error.
 *   07/06/99   13           Added this header.                 Arun Hiregange
 *   08/06/99   14           Added block normalization to       K.Srinivasan
 *                           III_dequantize_samples
 *   10/06/99   15           Merged with float version 1.11     Arun Hiregange
 *   14/06/99   16           Fixed pt conversion for III_stereo K.Srinivasan
 *   04/08/99   17           Removed float interfaces between   Arun Hiregange
 *                           functions.
 *   20/08/99   18           Merged with float 1.20             Darshak
 *                           (Updated for common.c changes)
 *   27/08/99   19           Moved parts of code to different   K.Srinivasan
 *                           files for reorganization into
 *                           smaller files for better
 *                           maintenance.
 *   09/09/99   20           Fixed point dequant done for CF    K.Srinivasan
 *   09/09/99   21           Fixed point changes for stereo     K.Srinivasan
 *   10/09/99   22           Fixed point changes for subband    Darshak V
 *                           synthesis
 *   14/09/99   23           Fixed point changes for antialias  K.Srinivasan
 *                           & reorder
 *   22/09/99   24           Fixed point changes for hybrid     Nanda
 *                                                              Kishore A.S
 *   13/10/99   25           Imdct globals added                Nanda
 *                                                              Kishore A.S
 *   27/10/99   26           Frequency inversion done in a      K.Srinivasan
 *                           separate function
 *   11/11/99   27           Changed interface for antialias    B.Venkatarao
 *   17/11/99   28           Cleanup of variable definitions    K.Srinivasan
 *   01/12/99   29           Moved find_lead_zeros from         Darshak
 *                           huffman.c
 *   25/02/00   30           MPEG-2 LSF changes                 K.Srinivasan
 *   18/06/02   31           Started with Onyx_ul fixed point   Padmagowri P.
 *                           code, added function calls to
 *                           Mp3_Decoder_Info & Mp3_Decoder_End.
 *                           Used storage specifiers to cross-
 *                           compile code on OnyxUL.
 *   20-06-2002 1.1          prev_bt variable is added for       A C Ashwin
 *                           for IMDCT state memory optimization
 *   19-07-2002 1.2          variables added for huffman changes Amit Sharma
 *   05-04-2004 1.3          Changed fileio for cross compiling  Sarang Akotkar
 *                           on ARM ADS 1.2.
 *   16-04-2004 1.3          Changed to make it relocatable      Mahesh D. S.
 *   26-04-2004 1.4          API changes completed.              Mahesh D. S.
 *   17-04-2004 1.5          API changes for ARM11 release       Mahesh D. S
 *   27-08-2004 1.6          Code cleanup                        Mahesh D.S.
 *   25-10-2004 1.7          Debug log changes                   Mahesh D.S
 *   14/02/2007 1.8          Level 4 Warnings removal            rks05c
 *                           Defect ID: TLSbo89611
 *   02/04/2007 1.9			 Removed 2 Level 4 warnings on WinCE rks05c
 *							 Defect ID: ENGR30447
 *   29-04-2007 1.9          1. correct wrong indication         Katherine Lu
 *                           ("unable to open file") in single
 *                           vector mode
 *                           2. modify audio_output_frame to
 *                           binary output.
 *                           3. add more test bitstreams for
 *                           ARM_ADS non-single vector test.
 *                           4. move get prev_cycles and
 *                           curr_cycles just before and after
 *                           decode frame.
 *                           5. add mhz output files for ARM_ADS
 *                           non-single vector test.
 *   31-01-2008  2.0         Code clean up                       Katherine Lu
 *   12-01-2008  3.0         Code clean up                       Baofeng Tian
  ****************************************************************************/

/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1991-1996)
 * ISO 11172-3 MPEG-1 Audio Codec
 * Revision 4.4 March 1996
 *
 * musicout.c
 **********************************************************************/
/**********************************************************************
 *   changes made since last update:                                  *
 *   date   programmers                comment                        *
 * 2/25/91  Douglas Wong        start of version 1.0 records          *
 * 3/06/91  Douglas Wong        rename setup.h to dedef.h             *
 *                              removed extraneous variables          *
 *                              removed window_samples (now part of   *
 *                              filter_samples)                       *
 * 3/07/91  Davis Pan           changed output file to "codmusic"     *
 * 5/10/91  Vish (PRISM)        Ported to Macintosh and Unix.         *
 *                              Incorporated new "out_fifo()" which   *
 *                              writes out last incomplete buffer.    *
 *                              Incorporated all AIFF routines which  *
 *                              are also compatible with SUN.         *
 *                              Incorporated user interface for       *
 *                              specifying sound file names.          *
 *                              Also incorporated user interface for  *
 *                              writing AIFF compatible sound files.  *
 * 27jun91  dpwe (Aware)        Added musicout and &sample_frames as  *
 *                              args to out_fifo (were glob refs).    *
 *                              Used new 'frame_params' struct.       *
 *                              Clean,simplify, track clipped output  *
 *                              and total bits/frame received.        *
 * 7/10/91  Earle Jennings      changed to floats to FLOAT            *
 *10/ 1/91  S.I. Sudharsanan,   Ported to IBM AIX platform.           *
 *          Don H. Lee,                                               *
 *          Peter W. Farrett                                          *
 *10/ 3/91  Don H. Lee          implemented CRC-16 error protection   *
 *                              newly introduced functions are        *
 *                              buffer_CRC and recover_CRC_error      *
 *                              Additions and revisions are marked    *
 *                              with "dhl" for clarity                *
 * 2/11/92  W. Joseph Carter    Ported new code to Macintosh.  Most   *
 *                              important fixes involved changing     *
 *                              16-bit ints to long or unsigned in    *
 *                              bit alloc routines for quant of 65535 *
 *                              and passing proper function args.     *
 *                              Removed "Other Joint Stereo" option   *
 *                              and made bitrate be total channel     *
 *                              bitrate, irrespective of the mode.    *
 *                              Fixed many small bugs & reorganized.  *
 *19 aug 92 Soren H. Nielsen    Changed MS-DOS file name extensions.  *
 * 8/27/93 Seymour Shlien,      Fixes in Unix and MSDOS ports,        *
 *         Daniel Lauzon, and                                         *
 *         Bill Truerniet                                             *
 *--------------------------------------------------------------------*
 * 4/23/92  J. Pineda           Added code for layer III.  LayerIII   *
 *          Amit Gulati         decoding is currently performed in    *
 *                              two-passes for ease of sideinfo and   *
 *                              maindata buffering and decoding.      *
 *                              The second (computation) pass is      *
 *                              activated with "decode -3 <outfile>"  *
 * 10/25/92 Amit Gulati         Modified usage() for layerIII         *
 * 12/10/92 Amit Gulati         Changed processing order of re-order- *
 *                              -ing step.  Fixed adjustment of       *
 *                              main_data_end pointer to exclude      *
 *                              side information.                     *
 *  9/07/93 Toshiyuki Ishino    Integrated Layer III with Ver 3.9.    *
 *--------------------------------------------------------------------*
 * 11/20/93 Masahiro Iwadare    Integrated Layer III with Ver 4.0.    *
 *--------------------------------------------------------------------*
 *  7/14/94 Juergen Koller      Bug fixes in Layer III code           *
 *--------------------------------------------------------------------*
 * 08/11/94 IIS                 Bug fixes in Layer III code           *
 *--------------------------------------------------------------------*
 * 11/04/94 Jon Rowlands        Prototype fixes                       *
 *--------------------------------------------------------------------*
 * 11/22/95 Heiko Purnhagen     skip ancillary data in bitstream      *
 **********************************************************************/
/******************************************************************************
 *		Freescale Semiconductor, Inc.
 *		=============================
 *   CHANGE HISTORY
 *   dd/mm/yy   Code Ver    Description								Author
 *   --------   -------     -----------								------
 *   05 Apr 07	1.0			Added MP2_output_frame and modified		Madhav Varma
 *							to support LayerII
 *	 05 Dec 07  1.1     Added Cortex vectors support  Katherine Lu
 ******************************************************************************/

#ifdef FILEIO
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#endif

/* API include */
#include   "mp3_dec_interface.h"
//DSPhl28081	Removed the Header decleration
//#include   "mp3d_tables.h"

//TLSbo89611
#if defined(WINCE_WARNLVL4_DISABLE) && defined(__WINCE)
	// disable some level-4 warnings, use #pragma warning(enable:###) to re-enable
	#pragma warning(disable:4201) // warning C4201: nonstandard extension used : nameless struct/union
	#pragma warning(disable:4214) // warning C4201: nonstandard extension used : bit field types other than int
	#pragma warning(disable:4127) // warning C4127: conditional expression is constant
	#pragma warning(disable:4100) // warning C4100: unreferenced formal parameter
	#pragma warning(disable:4115) // warning C4115: named type definition in parentheses # ENGR30447
#endif //#if defined(WINCE_WARNLVL4_DISABLE) && defined(__WINCE)
//TLSbo89611

//added by puneet_gulati@infosys.com to get the timing information from the board

#ifdef TIME_PROFILE
#include <sys/time.h>
#endif
///////////////////////////////////////////////////////////////////////////// //Lyon 070731
// Constants
#ifdef ARM12
#ifndef __WINCE

#define STRING_NUMBER    500
#define N                1000
#define LOG10_N          6
#define N_FORMAT         "%06d"

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
#endif

/////////////////////////////////////////////////////////////////////////////
#ifdef MEASURE_STACK_USAGE //tlsbo88602
#include "mm_codec_common.h"
#endif

#if defined(TIME_PROFILE)
#undef OUT_DUMP
#endif

#ifdef __WINCE //dsphl28536
#include <windows.h>
#endif


#define LONG_BOUNDARY   4
#define MAX_FILE_SIZE   100*1024*1024  /* file size in bytes to be decoded */


/****************************************************************
 *               SWAP BUFFER SPECIFIC DEFINITIONS               *
 * pInBuf   = pointer to input stream buffer.                   *
 * pUsedBuf = pointer to used buffer returned by MP3 decoder.   *
 *            The application must fill data from this location *
 *            onwards INPUT_BUFFER_SIZE many locations.         *
 ****************************************************************/
MP3D_UINT8     *pInBuf, *pUsedBuf;
MP3D_UINT32    InBufLen;  /* size of the file decoded so far */
MP3D_UINT32    uFileSize; /* size of input file */

//DSPhl28081 Removed the Variable
//void           *app_tables[70]; /* array of pointers to tables */

void audio_output_frame(MP3D_INT32 *, MP3D_Decode_Params*, FILE*);
//extern void *myMemSet(void *pDest, char data, unsigned int count);
//extern void *myMemCopy(void *pDest, const void *pSrc, unsigned int count);

void* alloc_fast (int size);
void* alloc_slow (int size);


//DSPhl28081 Removed the function definition
//init_tables ();

/*******************************************************
 * General Comment : When using File io on ADS, enable *
 *                   the option FILEIO and ARM_ADS     *
 *                                                     *
 *                 : When running on UNIX, enable only *
 *                   FILEIO                            *
 *******************************************************/

/* The main module starts here */
#ifdef __WINCE //dsphl28536
int main(int argc,_TCHAR *argv[])
#elif ARM_ADS
int main()
#else  /* This will be to run on UNIX */
int main(int argc, char **argv)
#endif
{
	#ifdef __WINCE   //dsphl28536
	wchar_t mode[]=L"rb";
	wchar_t mode_write[]=L"wb";
	#endif
    MP3D_INT32                *outbuf;
    MP3D_RET_TYPE             retval;
    MP3D_Mem_Alloc_Info_Sub   *mem;
    MP3D_Decode_Config        *dec_config;
    MP3D_Decode_Params        dec_param;
    MP3D_UINT8                *input_buffer;
    int                       nr;
    int                       i;
//added by puneet_gulati@infosys.com to get the timing details from the board
#ifdef TIME_PROFILE
	struct timeval StartTime, EndTime;
	unsigned long TotalDecTimeUs = 0,MaxTime = 0, MaxDecTimeUs = 0,MinTime = 0, Maxframe=0,Minframe=0;
	FILE	*fp_mhz;
	int nframe = 0;
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
    int bit_len = 16;
    double performance = 0.0;
#endif

#ifdef MEASURE_STACK_USAGE //TLSbo88602
    int           *ps32SP, *ps32BaseSP;
    int           s32StackCount, s32StackValue;
    int           s32PeakStack = 0;
#endif

#ifdef MEASURE_HEAP_USAGE
    int      s32TotalMallocBytes=0;
#endif


#ifdef __WINCE         // for taking timing on wince platform //dsphl28536
	BOOL Flag;
	LARGE_INTEGER lpFrequency1 ;
	LARGE_INTEGER lpPerformanceCount1;
	LARGE_INTEGER lpPerformanceCount2;
	__int64 TotalDecTime=0;
	__int64 Temp;
#endif

#ifdef PUSH_MODE
    const int sizeThreshold= SIZE_THRESHOLD;
    int leftLength;
	int leftInLength;
#endif

#ifdef FILEIO

    FILE       *in, *out;
    MP3D_INT32 words_read;

#ifndef PUSH_MODE
    MP3D_UINT8 mp3_input[MP3D_INPUT_BUF_SIZE];
#else
    MP3D_UINT8 mp3_input[MP3D_INPUT_BUF_PUSH_SIZE];
#endif

    int tagsize = 0;

    	{
	    // Output the MP3 Decoder Version Info
    	    printf("%s \n", mp3d_decode_versionInfo());
	    }
//detached for mp3 reference
#ifdef __WINCE	 //dsphl28536
		in = _wfopen(argv[1],mode);
#else

if(argc==0)
        in = fopen ("test.bit", "rb");
else
	      in = fopen (argv[1], "rb");
#endif
        if (in == NULL)
        {
#ifdef PRINT
            printf ("Unable to open file %s\n",argv[1]);
#endif
            exit (2);
        }
#ifdef OUT_DUMP
#ifdef __WINCE	 //dsphl28536
		out = _wfopen(argv[2],mode_write);
#else

if(argc==0)
        out = fopen("test.pcm", "wb");
else
	      out = fopen(argv[2],"wb");
#endif
		if (out == NULL)
        {
#ifdef PRINT
            printf ("Unable to open file %s\n",argv[2]);
#endif
            exit (2);
        }
#endif//OUT_DUMP
#endif// file IO
        {
            /* Allocate memory for decoder config structure */
            dec_config = (MP3D_Decode_Config *) alloc_fast (sizeof(MP3D_Decode_Config));

            if (dec_config == NULL)
                return 1;
//DSPhl28081 Removed the function call
//init_tables();
//DSPhl28081 Removed the statement
//dec_config->app_initialized_data_start = (void*) app_tables;

            /* Query for memory */
            if ((retval = mp3d_query_dec_mem (dec_config)) != MP3D_OK)
                return 1;

            /* Number of memory chunk requests by the decoder */
            nr = dec_config->mp3d_mem_info.mp3d_num_reqs;

            for(i = 0; i < nr; i++)
            {
                mem = &(dec_config->mp3d_mem_info.mem_info_sub[i]);

                if (mem->mp3d_type == MP3D_FAST_MEMORY)
                {
                    mem->app_base_ptr = alloc_fast (mem->mp3d_size);
                    memset(mem->app_base_ptr, 0xfe, mem->mp3d_size);
#ifdef MEASURE_HEAP_USAGE
					s32TotalMallocBytes +=  mem->mp3d_size;
#endif //#ifdef MEASURE_HEAP_USAGE
                    if (mem->app_base_ptr == NULL)
                        return 1;
                }
                else
                {
                    mem->app_base_ptr = alloc_slow (mem->mp3d_size);
#ifdef MEASURE_HEAP_USAGE
					s32TotalMallocBytes +=  mem->mp3d_size;
#endif //#ifdef MEASURE_HEAP_USAGE
                    if (mem->app_base_ptr == NULL)
                        return 1;
                }
            }

#ifdef FILEIO
            /* find the size of file which is reqired for allocation of
             * app_input_buffer */
            uFileSize = 0;
            while (MP3D_TRUE)
            {
                words_read = fread (mp3_input, 1, MP3D_INPUT_BUF_SIZE, in);

                uFileSize += words_read;
                if (words_read != MP3D_INPUT_BUF_SIZE)
                {
                    /* This happens at the end of file */
                    if (uFileSize == 0)
                    {
#ifdef PRINT
                        printf ("Empty Input file \n");
#endif
                        exit(2);
                    }
                    break;
                }
                if (uFileSize >= MAX_FILE_SIZE)
                   break;
            } /* end of while */
            fclose(in);
#endif

            /* allocate for input buffer */
            input_buffer = (MP3D_UINT8*) alloc_fast (uFileSize);
            if (input_buffer == NULL)
            {
                return 1;
            }
            pInBuf = input_buffer;

            /* allocate for output buffer */
            /* Size of short * channels * MP3D_FRAME_SIZE */
			 outbuf = (MP3D_INT32 *)alloc_fast ((sizeof(short))*2*MP3D_FRAME_SIZE);
            if (outbuf == NULL)
                return 1;
        }

        /* Swap buffer function pointer initializations */
#ifndef PUSH_MODE
        dec_config->app_swap_buf = app_swap_buffers_mp3_dec;
#endif
#ifndef PUSH_MODE
        /* initialize the mp3 decoder */
        if ((retval = mp3d_decode_init (dec_config, input_buffer, uFileSize)) != MP3D_OK)
            return retval;
#else
/* initialize the mp3 decoder */
        if ((retval = mp3d_decode_init (dec_config, NULL, 0)) != MP3D_OK)
        	return retval;

#endif


#ifdef FILEIO
#ifndef ARM_ADS
#ifdef __WINCE	//dsphl28536
		in = _wfopen(argv[1],mode);
#else
if(argc==0)
        in = fopen ("test.bit", "rb");
else
	      in = fopen(argv[1],"rb");
#endif
#endif

        fread(input_buffer,1,uFileSize,in);
#endif

#ifdef PUSH_MODE
        dec_config->pInBuf = (MP3D_INT8 *)mp3_input;
        InBufLen = 0;
        while(!memcmp(input_buffer+InBufLen, "ID3", 3))
        {
            char *pBuff = input_buffer+InBufLen;
            tagsize = (pBuff[6]<<21) | (pBuff[7]<<14) | (pBuff[8]<<7) | pBuff[9];
            tagsize += 10;
            leftInLength -= tagsize;
            InBufLen += tagsize;
        }

        if(uFileSize-InBufLen>MP3D_INPUT_BUF_PUSH_SIZE)
			leftInLength = MP3D_INPUT_BUF_PUSH_SIZE;
		else
			leftInLength = uFileSize-InBufLen;
        dec_config->inBufLen = leftInLength;
		dec_config->consumedBufLen =0;

        memcpy(mp3_input, input_buffer+InBufLen, leftInLength);
        InBufLen += leftInLength;
#endif


        /* Decoder loop. If the retval is a fatal error then only
		 * it should stop decoding frames. Fatal erroes starts from
		 * MP3D_ERROR_INIT (51). All other errors are recoverable.
		 ***/
        while (retval < MP3D_ERROR_INIT)
        {
//added by puneet_gulati@infosys.com to get the timing details from the board
#ifdef TIME_PROFILE
	   		gettimeofday(&StartTime, NULL);
#endif

#ifdef MEASURE_STACK_USAGE
           PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
#endif


#ifdef __WINCE //dsphl28536
       Flag=QueryPerformanceFrequency(&lpFrequency1);
	   Flag=QueryPerformanceCounter(&lpPerformanceCount1);
#endif

           retval = mp3d_decode_frame (dec_config, &dec_param, outbuf);


#ifdef __WINCE //dsphl28536
		Flag=QueryPerformanceCounter(&lpPerformanceCount2);
		Temp=(((lpPerformanceCount2.QuadPart - lpPerformanceCount1.QuadPart)*1000000)/(lpFrequency1.QuadPart));
		TotalDecTime += Temp;
#endif

//TLSbo88602
#ifdef MEASURE_STACK_USAGE
        GET_STACK_USAGE (ps32BaseSP, ps32SP, s32StackCount, s32StackValue);
#endif

#ifdef MEASURE_STACK_USAGE
        if (s32PeakStack < s32StackValue)
            s32PeakStack = s32StackValue;
#endif
//TLSbo88602


//added by puneet_gulati@infosys.com to get the timing details from the board
#ifdef TIME_PROFILE
		   		gettimeofday(&EndTime, NULL);
		   		nframe++;
				MaxDecTimeUs = (EndTime.tv_usec - StartTime.tv_usec) + (EndTime.tv_sec - StartTime.tv_sec)*1000000L;
		   		TotalDecTimeUs += (EndTime.tv_usec - StartTime.tv_usec) + (EndTime.tv_sec - StartTime.tv_sec)*1000000L;
		   		if (MaxDecTimeUs > MaxTime)
		   		{
		   				MaxTime = MaxDecTimeUs;
		   				Maxframe = nframe;
				}

#endif

            if (retval == MP3D_OK)
            {
                /* The output frame is ready for use. Decoding of the next frame
                 * should start only if the previous output frame has been fully
                 * output by the application. Presently we are writing the output in
                 * a file instead of output buffer.
                 */

	        if(dec_param.layer == 3 || dec_param.layer == 2 || dec_param.layer==1)
			{
				#ifdef OUT_DUMP
                audio_output_frame (outbuf, &dec_param, out);
				#endif
            }
		    else
		    {
			   printf(" Invalid Layer identified \n" );
			   retval = MP3D_ERROR_LAYER ;
			   break;
		    }
			}


            if (retval == MP3D_END_OF_STREAM)
            {
            	  printf("End of stream! \n" );
                break;
            }


#ifdef PUSH_MODE

           //printf("consumed:%d\n",dec_config->consumedBufLen);
           leftLength = dec_config->inBufLen - dec_config->consumedBufLen;
           //printf("Left:%d\n",leftLength);
           if(leftLength<0)
		   	       break;
           if(leftLength>sizeThreshold)
           {
             dec_config->pInBuf += dec_config->consumedBufLen;
		     dec_config->inBufLen -= dec_config->consumedBufLen;
			 dec_config->consumedBufLen =0;
           }
		   else
		   	{
			   memcpy(mp3_input, (dec_config->pInBuf+dec_config->consumedBufLen),leftLength);
            leftInLength = MP3D_INPUT_BUF_PUSH_SIZE - leftLength;
            if(leftInLength > uFileSize- InBufLen)
				leftInLength = uFileSize- InBufLen;

			if(InBufLen<uFileSize)
			{
			  memcpy(mp3_input+leftLength, (input_buffer+InBufLen),leftInLength);
			}

			InBufLen += leftInLength;
            dec_config->pInBuf = (MP3D_INT8 *)mp3_input;
			dec_config->inBufLen = leftLength + leftInLength;
			if(dec_config->inBufLen<0)
				 break;
			dec_config->consumedBufLen =0;
		   	}
#endif

        }



#ifdef FILEIO
        fclose (in);
#ifdef OUT_DUMP
        fclose (out);
#endif
#endif

		#ifdef MEASURE_STACK_USAGE
    #ifdef MEASURE_HEAP_USAGE
        		printf("%d,%d\n",s32PeakStack,s32TotalMallocBytes);
	  #endif
	  #endif



        /* free memory allocated by application */
        free (input_buffer);
        free (outbuf);
        for (i = 0; i < nr; i++)
        {
            free (dec_config->mp3d_mem_info.mem_info_sub[i].app_base_ptr);
        }
        free (dec_config);

#ifdef MEASURE_HEAP_USAGE
    s32TotalMallocBytes=0;
#endif

//added by puneet_gulati@infosys.com to get the timing details from the board
#ifdef TIME_PROFILE
	#ifdef __SYMBIAN32__
		fp_mhz = fopen("D:\mp3_mhz_gnu.txt","a+");
	#else
		fp_mhz = fopen("../audio_performance.txt","a+");
	#endif
	       if (fp_mhz == NULL)
              {
                  printf("Couldn't open cycles file\n");
              }
		fprintf (fp_mhz,"\n|\tAudio Format\t|\tPlat\t|\tSamplerate\t|\tCh\t|\tBiterate\t|\tBit\t|\tPerf(MIPS)\t|\tAudio file name\n");
		printf("\n%s\t%s\t%d\t%d\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname,argv[1],dec_param.mp3d_sampling_freq,dec_param.mp3d_num_channels,\
			MaxTime,MinTime,nframe,Maxframe,Minframe,TotalDecTimeUs);
//		fprintf(fp_mhz,"\n%s\t%s\t%d\t%d\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname,argv[1],dec_param.mp3d_sampling_freq,dec_param.mp3d_num_channels,
//			MaxTime,MinTime,nframe,Maxframe,Minframe,TotalDecTimeUs);
              performance = (double)TotalDecTimeUs * 0.000001*cpu_freq*dec_param.mp3d_sampling_freq/(nframe*576);
              fprintf(fp_mhz, "|\tMP3 Decoder \t|\tARM%d\t|\t%9d\t|\t%d\t|\t%8d\t|\t%d\t|\t%.8f\t|%s\n", platform, dec_param.mp3d_sampling_freq, dec_param.mp3d_num_channels, dec_param.mp3d_bit_rate*1000, bit_len, performance, argv[1]);
		/*fprintf(fp_mhz,"MHz for the stream %s\n",argv[1]);
		fprintf(fp_mhz,"Average Time [usec] = %ld\n", TotalDecTimeUs);
		fprintf(fp_mhz,"Peak Time = %ld\n\n", MaxTime);
		fprintf(fp_mhz, "---------------------------------------------------------\n");*/
		fclose(fp_mhz);
#endif
#ifdef __SYMBIAN32__
#ifdef MEASURE_HEAP_USAGE
			printf("\n Stack and Heap are  %d\t%d\t",s32PeakStack,s32TotalMallocBytes);
#endif
#endif
#ifdef __WINCE //dsphl28536
		printf("\n Input File =%s lpPerformanceCount in us =%ld",argv[1],TotalDecTime);
#endif

    return 0;
}

/***************************************************************************
 *
 *   FUNCTION NAME - audio_output_frame
 *
 *   DESCRIPTION
 *       This function sends the output samples to the output.
 *         Presently we are not saving output in output buffer instead we are
 *          adding to an output file.
 *
 *   ARGUMENTS
 *       outbuf               - array containing the samples to be outputted
 *       dec_param           - contains the decoder parameters required for
 *                             saving pcm output.
 *       outFile           - pointer to the file where output shall be written
 *
 *   RETURN VALUE
 *       None.
 *
 ***************************************************************************/
//#define PCM_OUTPUT

void audio_output_frame (MP3D_INT32 *outbuf, MP3D_Decode_Params *dec_param, FILE* outFile)
{
    int stereo = dec_param->mp3d_num_channels;
	MP3D_INT16 *out_ptr16;

	/* 16-bit pcm output */
	out_ptr16 = (MP3D_INT16 *)outbuf;
#ifdef FILEIO
	fwrite (out_ptr16,sizeof(short),stereo*576,outFile);
#endif

#ifdef FILEIO
    fflush(outFile);
#endif

}

#ifndef PUSH_MODE
/***************************************************************************
 *
 *   FUNCTION NAME - app_swap_buffers_mp3_dec
 *
 *   DESCRIPTION
 *       This function is a callback by instance of MP3 decoder requesting
 *       the application for a new input buffer. It returns the used buffer
 *       pointer to the application and expects a new buffer pointer and
 *       buffer length in return. If buffer is not available it returns
 *       a NULL pointer with zero length to the MP3 decoder instance. This
 *       is an example code.
 *
 *   ARGUMENTS
 *       new_buf_ptr       - Pointer to pointer of used buffer
 *       new_buf_len       - Pointer to the length of the input buffer
 *                           (In 16bit words)
 *       instance_id       - MP3 decoder Instance ID
 *
 *   Global Variable       - uFileSize - size of the input file.
 *                         - InBufLen  - size of the buffer decoded so far.
 *   RETURN VALUE
 *          0              - If buffer allocation is successfull.
 *         -1              - Indicates 'End of Bitstream'.
 *
 ***************************************************************************/
MP3D_INT8 app_swap_buffers_mp3_dec (MP3D_UINT8 **new_buf_ptr,
                                    MP3D_INT32 *new_buf_len,
                                    MP3D_Decode_Config *dec_config)
{
    if (InBufLen < uFileSize)
    {
        /* Bytes available */
        pUsedBuf = *new_buf_ptr;
        *new_buf_ptr = (pInBuf+InBufLen);
        if ((InBufLen + MP3D_INPUT_BUFFER_SIZE) > uFileSize)
            *new_buf_len = (MP3D_INT32) (uFileSize - InBufLen);
        else
            *new_buf_len = MP3D_INPUT_BUFFER_SIZE;
        InBufLen += MP3D_INPUT_BUFFER_SIZE;
        return 0;
    }
    else
    {
        /* Exhausted the buffer */
        *new_buf_ptr = NULL;
        *new_buf_len = 0;
        return -1;
    }
}
#endif

/***************************************************************************
 *
 *   FUNCTION NAME - alloc_fast
 *
 *   DESCRIPTION
 *          This function simulates to allocate memory in the internal
 *          memory. This function when used by the application should
 *          ensure that the memory address returned in always aligned
 *          to long boundry.
 *
 *   ARGUMENTS
 *       size              - Size of the memory requested.
 *
 *   RETURN VALUE
 *       base address of the memory chunck allocated.
 *
 ***************************************************************************/
void* alloc_fast (int size)
{
    void* ptr = NULL;
    ptr = malloc (size+4);
    ptr = (void *)(((long)ptr + (long)(LONG_BOUNDARY-1)) & (long)(~(LONG_BOUNDARY-1)));
    return ptr;
}

/***************************************************************************
 *
 *   FUNCTION NAME - alloc_slow
 *
 *   DESCRIPTION
 *          This function simulates to allocate memory in the external
 *          memory. This function when used by the application should
 *          ensure that the memory address returned in always aligned
 *          to long boundry.
 *
 *   ARGUMENTS
 *       size              - Size of the memory requested.
 *
 *   RETURN VALUE
 *       base address of the memory chunck allocated.
 *
 ***************************************************************************/
void* alloc_slow (int size)
{
    void* ptr = NULL;
    ptr = malloc (size);
    ptr = (void *)(((long)ptr + (long)LONG_BOUNDARY-1) & (long)(~(LONG_BOUNDARY-1)));
    return ptr;
}

#ifdef __WINCE //dsphl28536
int _tmain(int argc,_TCHAR *argv[])
{
	main(argc,argv);
    return 0;
}
#endif

#ifdef ARM12
#ifndef __WINCE
#ifndef LINUX
__irq void IRQ_Handler(void)    	//Lyon 070731
{


   *timer_clear	= 1;
   *irq_enable_clr = 0x02;
   *irq_enable_set = 0x02;
}
#endif
#endif
#endif
/* End of file */
