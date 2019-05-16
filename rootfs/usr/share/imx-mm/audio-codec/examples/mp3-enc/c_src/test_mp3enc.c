/****************************************************************************
 * (C) 2006 ALLGO EMBEDDED SYSTEMS PVT. LTD                                 *
 *                                                                          *
 * ORIGINAL AUTHOR: SRIPATHI KAMATH                                         *
 ****************************************************************************
 ***********************************************************************
 * Copyright (c) 2006-2010, 2012, 2013, Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc.
 ***********************************************************************
 * CHANGE HISTORY
 *
 * dd/mm/yy   Code Ver     Description                        Author
 * --------   -------      -----------                        ------
 * 21/02/06    01          initial revision                   Sripathi Kamath
 *
 * 24/02/06    02          updated with review comments       Sripathi Kamath
 *
 * 21/03/06    03          added wave read functions          Sripathi Kamath
 * 07/09/07    04          made changes to accomodate
 *                         new api functions                  Wang Qinling
 * 21/05/08    05          Update api info function           Huang Shen
 * 02/06/08    06          Bypass the file w/r issues
 *                         on arm12 platform                  Lionel Xu
 * 10/07/08    07          use fseek,ftell get filesize       Lionel Xu
 * 15/07/08    08          add compile flag to irq related    Lionel Xu

 ****************************************************************************
 * DESCRIPTION
 *   This file contains the main function of the test application that takes
 *   input from pcm or wave file and generates the encoded mp3 file.
 *
 ******************************************************************************/
#ifdef __WINCE
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mp3_enc_interface.h"
#include "test_mp3enc.h"
#include "wave_functions.h"

#define NUM_SAMPLES 1152             /* 1152 samples per channel */
#ifdef TWO_INSTANCES
#define MAXIMUM_INSTANCES 2          /* maximum number of instances supported*/
#define MAX_STACK_SIZE  0x1000
#define EMPTY_STACK_MARKER  0x55555555
#else
#define MAXIMUM_INSTANCES 1          /* maximum number of instances supported*/
#endif

#ifdef TIME_PROFILE_RVDS_ARM9
void dummyCall1(void);
void dummyCall2(void);
int  uiframe = 0;
#endif
#ifdef __ARM12
#ifndef __WINCE
#define TIMER_BASE       0x0A800000U
#define TIMER_LOAD       (TIMER_BASE + 0x000U)
#define TIMER_CONTROL    (TIMER_BASE + 0x008U)
#define TIMER_CLEAR      (TIMER_BASE + 0x00CU)
#define TIMER_VALUE      (0xFFF/2)

#define IRQCT_BASE       0x0A801000U
#define IRQCT_ENABLESET  (IRQCT_BASE + 0x008U)
#define IRQCT_ENABLECLR  (IRQCT_BASE + 0x00CU)
#endif
#endif

/*****************************************************************************
 *
 * globals
 *
 *****************************************************************************/
#ifdef __ARM12
#ifndef __WINCE
// Registers in the peripherals.
static volatile unsigned int * const irq_enable_set = (volatile unsigned int *)IRQCT_ENABLESET;
static volatile unsigned int * const irq_enable_clr = (volatile unsigned int *)IRQCT_ENABLECLR;
static volatile unsigned int * const timer_load     = (volatile unsigned int *)TIMER_LOAD;
static volatile unsigned int * const timer_control  = (volatile unsigned int *)TIMER_CONTROL;
static volatile unsigned int * const timer_clear    = (volatile unsigned int *)TIMER_CLEAR;
#endif
#endif

char* W1[MAXIMUM_INSTANCES];
char* W2[MAXIMUM_INSTANCES];
char* W3[MAXIMUM_INSTANCES];
char* W4[MAXIMUM_INSTANCES];
char* W5[MAXIMUM_INSTANCES];
char* W6[MAXIMUM_INSTANCES];


MP3E_Encoder_Parameter params;        /* Structure to pass input parameters
                                        to the encoder */
#ifdef TWO_INSTANCES
MP3E_Encoder_Parameter params1;        /* Structure to pass input parameters
                                        to the encoder */
#endif

FILE *ofp;                           /* Output file pointer */
#ifdef TWO_INSTANCES
FILE *ofp1;
int instanceID;
#endif

FILE *fout;                          /* Output file pointer for MIPS test*/

#ifdef TIME_PROFILE
#include <sys/time.h>
struct timeval StartTime, EndTime;
signed long TotalDecTimeUs = 0,MinTime = 500000,MaxTime = 0,Minframe = 0,Maxframe = 0,CurDecTimeUs = 0;
int nframe = 0;
#ifdef MPEG2_LSF
double den = 576000000.0;
#else
double den = 1152000000.0;
#endif
double curr_mhz;
int MHZ =532;
/*double a1, a2, a3;*/
unsigned char chname[] = "[PROFILE-INFO]";
void Elinux_measure_begin();
void Elinux_measure_end();
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
FILE *ph_mips = NULL;
double performance = 0.0;
int channel = 0;
int totalsamples = 0;
#endif


#ifdef MHZ_MEASURE
void RVDS_measure_begin();
void RVDS_measure_end(int num_samples);
int curr_clk,prev_clk,clk,max_clk,max_frm,max_sf;
int frame_Num=0;
double den, curr_mhz;
#endif

#ifdef __WINCE         /* for taking timing on wince platform*/
BOOL Flag;
LARGE_INTEGER lpFrequency;
LARGE_INTEGER lpPerformanceCountBegin;
LARGE_INTEGER lpPerformanceCountEnd;
__int64 TotalDecTime=0;
__int64 Temp,MaxDecTime=0,MinDecTime=0;
int max_frame = 0 ;
int min_frame = 0 ;
int frame_len = 0;
unsigned int sample_rate = 0, bit_rate = 0;
int max_sf;
int frame_Num=0;

#ifdef MPEG2_LSF
double den = 576000000.0;
#else
double den = 1152000000.0;
#endif

int MHZ =532;
void WinCE_measure_begin();
void WinCE_measure_end();
#endif


#ifdef ENABLE_MY_FREAD


struct_input_data input_raw_data;

char test_case[20*1024*1024];    //Lionel used to store the input wav/pcm


#ifdef TWO_INSTANCES
struct_input_data input_raw_data1;
char test_case1[20*1024*1024];    //Lionel used to store the input wav/pcm
#endif


#endif
/*****************************************************************************
 *
 * main
 *
 * DESCRIPTION
 *   This function calls the encoder through the api functions described in
 *   mp3enc_ARM_API.doc
 *
 *****************************************************************************/
#ifdef __WINCE
int _tmain (int argc, char** argv)
#else
int main (int argc, char** argv)
#endif
{
    FILE *ifp;                       /* Input file pointer */
#ifdef TWO_INSTANCES
    FILE *ifp1;
#endif
    MP3E_INT16 inbuf[NUM_SAMPLES*2];
#ifdef TWO_INSTANCES
    MP3E_INT16 inbuf1[NUM_SAMPLES*2];
#endif
    int mode = 0;
#ifdef TWO_INSTANCES
    int mode1=0;
#endif
    int sfreq = 0;
#ifdef TWO_INSTANCES
    int sfreq1 = 0;
#endif
    int bitrate = 0;
#ifdef TWO_INSTANCES
    int bitrate1 = 0;
#endif
    char inPath[80];
#ifdef TWO_INSTANCES
    char inPath1[80];
#endif
    char outPath[80];
#ifdef TWO_INSTANCES
    char outPath1[80];
#endif

#ifdef TWO_INSTANCES
    int filetype, filetype1, samp_ret, samp_ret1, num_channels, num_channels1;
#else
    int filetype, samp_ret, num_channels;
#endif
    char *type;
#ifdef TWO_INSTANCES
    char *type1;
#endif
    int num_samples;
#ifdef TWO_INSTANCES
    int num_samples1;
#endif
    int frame_count =0; /*record frame count*/
    MP3E_INT8 *outbuf;
#ifdef TWO_INSTANCES
    MP3E_INT8 *outbuf1;
#endif
#ifdef TWO_INSTANCES
    MP3E_RET_VAL  val, val1, rflag, rflag1;
#else
    MP3E_RET_VAL  val, rflag;
#endif
    MP3E_Encoder_Config enc_config;
#ifdef TWO_INSTANCES
    MP3E_Encoder_Config enc_config1;
#endif

#ifdef __WINCE
    _TCHAR *arg_word;
    char * arg_byte;
    int count;
#endif

#ifdef __ARM12
#ifdef __WINCE
        EnableRunFast();
#endif
#endif

#if defined(TIME_PROFILE)
    ph_mips = fopen("../audio_performance.txt", "a");
#endif

    enc_config.instance_id = 0;
#ifdef TWO_INSTANCES
    enc_config1.instance_id = 1;
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
    /* memory setup*/
    rflag = mp3e_query_mem(&enc_config);
#ifdef TWO_INSTANCES
    rflag1 = mp3e_query_mem(&enc_config1);
#endif
    encoder_mem_info_alloc (&enc_config);
#ifdef TWO_INSTANCES
    encoder_mem_info_alloc (&enc_config1);
#endif

	// Output the MP3 Encoder Version Info
	printf("%s \n", MP3ECodecVersionInfo());


    /* Parse the command line arguments to receive encoding parameters */
#ifdef TWO_INSTANCES
    parse_args(argc, argv, &mode, &sfreq, &bitrate, inPath, outPath,
                           &mode1,&sfreq1,&bitrate1,inPath1,outPath1);
#else
    parse_args(argc, argv, &mode, &sfreq, &bitrate, inPath, outPath);
#endif

    /* Setting the input and the output file pointers */
#ifdef ENABLE_MY_FREAD
    input_raw_data.buff = test_case;
    input_raw_data.buff_head = input_raw_data.buff;
    input_raw_data.file_size = 0;
	input_raw_data.bytes_read = 0;

    ifp = fopen(inPath, "rb");

    if(ifp == NULL)
    {
        fprintf(stderr,"Input file does not exist\n");
        exit(0);
    }

        fseek(ifp, 0, SEEK_END);
        input_raw_data.file_size = ftell(ifp);

	fseek(ifp, 0, SEEK_SET);
    fread(input_raw_data.buff,sizeof(char),input_raw_data.file_size,ifp);
/*    fclose(ifp);*/

#ifdef TWO_INSTANCES
	input_raw_data1.buff = test_case1;
	 input_raw_data1.buff_head = input_raw_data1.buff;
	 input_raw_data1.file_size = 0;
	 input_raw_data1.bytes_read = 0;

	ifp1 = fopen(inPath1, "rb");

    if(ifp1 == NULL)
    {
        fprintf(stderr,"Input file does not exist\n");
        exit(0);
    }

        fseek(ifp1, 0, SEEK_END);
        input_raw_data1.file_size = ftell(ifp1);

	fseek(ifp1, 0, SEEK_SET);
    fread(input_raw_data1.buff,sizeof(char),input_raw_data1.file_size,ifp1);
/*    fclose(ifp1);*/
#endif
#endif


#ifdef ENABLE_MY_FREAD
     my_afopen(&num_channels, &filetype, (void *)&input_raw_data);

#ifdef TWO_INSTANCES
	 my_afopen(&num_channels1,&filetype1, (void *)&input_raw_data1);
#endif

#else

    ifp = afopen (inPath, &num_channels, &filetype);
#ifdef TWO_INSTANCES
    ifp1= afopen (inPath1,&num_channels1, &filetype1);
    if(ifp1 == NULL)
    {
        fprintf(stderr,"Input1 file does not exist\n");
        exit(0);
    }
#endif
    if(ifp == NULL)
    {
        fprintf(stderr,"Input file does not exist\n");
        exit(0);
    }
#endif


    ofp = fopen (outPath,"wb");
#ifdef TWO_INSTANCES
    ofp1= fopen(outPath1,"wb");
#endif
    if(ofp == NULL)
    {
        fprintf(stderr,"Output file could not be created\n");
        exit(0);
    }
#ifdef TWO_INSTANCES
    if(ofp1 == NULL)
    {
        fprintf(stderr,"Output1 file could not be created\n");
        exit(0);
    }
#endif
    type = filetype?"wav":"pcm";

    params.app_sampling_rate = sfreq;   /* set sampling rate */
    params.app_bit_rate = bitrate;      /* set bit rate */
    params.app_mode = mode;             /* set mode */

#ifdef TWO_INSTANCES
    type1 = filetype1?"wav":"pcm";

     params1.app_sampling_rate =sfreq1 ;   /* set sampling rate */
    params1.app_bit_rate = bitrate1;      /* set bit rate */
    params1.app_mode = mode1;
#endif

    /* Initialize the encoder */
    val = mp3e_encode_init (&params,&enc_config);

#ifdef TWO_INSTANCES
    val1 = mp3e_encode_init (&params1,&enc_config1);
#endif
    /* Initialization error checks */
#ifdef TWO_INSTANCES
    if ((val == MP3E_ERROR_INIT_BITRATE)||(val1 == MP3E_ERROR_INIT_BITRATE))
#else
    if (val == MP3E_ERROR_INIT_BITRATE)
#endif
    {
        fprintf(stderr,"Invalid bitrate initialization\n");
        fprintf(stderr,"Possible bit rates are:\n");
        fprintf(stderr,"For MPEG1: 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 kbps\n");
#ifdef MPEG2_LSF
        fprintf(stderr,"For MPEG2-LSF: 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160 kbps\n");
#endif
        exit(0);
    }
#ifdef TWO_INSTANCES
    if ((val == MP3E_ERROR_INIT_SAMPLING_RATE)|| (val1 == MP3E_ERROR_INIT_SAMPLING_RATE))
#else
    if (val == MP3E_ERROR_INIT_SAMPLING_RATE)
#endif
    {
        fprintf(stderr,"Invalid sampling rate initialization\n");
        fprintf(stderr,"Possible sampling rates are:\n");
        fprintf(stderr,"32000, 44100, 48000\n");
#ifdef MPEG2_LSF
        fprintf(stderr,"16000, 22050, 24000\n");
#endif
        exit(0);
    }

#ifdef TWO_INSTANCES
    if ((val == MP3E_ERROR_INIT_MODE) || (val1 == MP3E_ERROR_INIT_MODE))
#else
    if (val == MP3E_ERROR_INIT_MODE)
#endif
    {
        fprintf(stderr,"Invalid mode initialization\n");
        fprintf(stderr,"Possible modes are: j or m\n");
        exit(0);
    }

#ifdef TWO_INSTANCES
    if ((val == MP3E_ERROR_INIT_FORMAT) || (val1 == MP3E_ERROR_INIT_FORMAT))
#else
    if (val == MP3E_ERROR_INIT_FORMAT)
#endif
    {
        fprintf(stderr,"Invalid input format type\n");
        fprintf(stderr,"Possible formats are: i or l\n");
        exit(0);
    }

#ifdef TWO_INSTANCES
    if ((val == MP3E_ERROR_INIT_QUALITY) || (val1 == MP3E_ERROR_INIT_QUALITY))
#else
    if (val == MP3E_ERROR_INIT_QUALITY)
#endif
    {
        fprintf(stderr,"Invalid configuration value\n");
        fprintf(stderr,"Possible values are: c or s\n");
        exit(0);
    }
    /* On successful initialization, read 1152*2 (576*2) samples from
     * input file and pass it to the encode_frame_mp3e function.
     * Note that if the last frame does not contain 1152*2 (576*2) samples,
     * it should be filled in with zeros before passing to the function
     */
    num_samples = (params.app_sampling_rate <=24000) ? 576 : 1152;
    /*number of samples differ for MPEG1 or MPEG2-LSF */
#ifdef TWO_INSTANCES
    num_samples1 = (params1.app_sampling_rate <=24000) ? 576 : 1152;
#endif
	fout = fopen("output.log","w");          /*log MIPS result*/

    if(type == "wav")
    {
       /* If the input file is a wav file, use afread function to read the
        * samples
        */
        if(num_channels == 1)              /*mono wave file*/
        {

            int i;

#ifdef ENABLE_MY_FREAD

#ifdef TWO_INSTANCES
            samp_ret = my_afread (inbuf, sizeof (short int), num_samples, (void *)&input_raw_data);
            samp_ret1 = my_afread1 (inbuf1, sizeof (short int), num_samples1, (void *)&input_raw_data1);
            while ((samp_ret == num_samples) || (samp_ret1== num_samples1))
#else
            while ((samp_ret = my_afread (inbuf, sizeof (short int), num_samples, (void *)&input_raw_data)) ==  num_samples)
#endif

#else

#ifdef TWO_INSTANCES
            samp_ret = afread (inbuf, sizeof (short int), num_samples, ifp);
            samp_ret1 = afread1 (inbuf1, sizeof (short int), num_samples1, ifp1);
            while ((samp_ret == num_samples) || (samp_ret1== num_samples1))
#else
            while ((samp_ret = afread (inbuf, sizeof (short int), num_samples, ifp)) ==  num_samples)
#endif

#endif

            {
#ifdef TWO_INSTANCES
               if(samp_ret==num_samples)
               {
#endif


#ifdef TWO_INSTANCES
                instanceID=enc_config.instance_id;
#endif
                outbuf =(char *) malloc (params.mp3e_outbuf_size);/* get the next
                                                            empty buffer */
#ifdef MHZ_MEASURE
                RVDS_measure_begin();
#endif

#ifdef TIME_PROFILE_RVDS_ARM9
                dummyCall1();
#endif

#ifdef TIME_PROFILE
                gettimeofday(&StartTime, NULL);
#endif

#ifdef __WINCE
                WinCE_measure_begin();
#endif

                mp3e_encode_frame (inbuf,&enc_config,outbuf);
                /* This function does encoding of one frame. It
                   internally calls swap_output_buf_mp3e() when
                   one full MP3 output buffer is available. */
#ifdef MHZ_MEASURE
                RVDS_measure_end(num_samples);
#endif
#ifdef TIME_PROFILE_RVDS_ARM9
            dummyCall2();
#endif
#ifdef TIME_PROFILE
                totalsamples += num_samples;
                Elinux_measure_end();
#endif
#ifdef __WINCE
                WinCE_measure_end();
#endif
#ifndef TIME_PROFILE
                output_mp3_file(&enc_config, outbuf);
#endif
                free (outbuf);      /* free the used buffer */
#ifdef TWO_INSTANCES
#ifdef ENABLE_MY_FREAD
                samp_ret = my_afread (inbuf, sizeof (short int), num_samples, (void *)&input_raw_data);
#else
                samp_ret = afread (inbuf, sizeof (short int), num_samples, ifp);
#endif

#endif
            }
#ifdef TWO_INSTANCES
               if(samp_ret1==num_samples1)
               {

                instanceID=enc_config1.instance_id;
                outbuf1 =(char *) malloc (params1.mp3e_outbuf_size);
                mp3e_encode_frame (inbuf1,&enc_config1,outbuf1);
                output_mp3_file1(&enc_config1, outbuf1);

                free (outbuf1);      /* free the used buffer */

#ifdef ENABLE_MY_FREAD
                samp_ret1 = my_afread (inbuf1, sizeof (short int), num_samples1,(void *)&input_raw_data1);
#else
                samp_ret1 = afread (inbuf1, sizeof (short int), num_samples1, ifp1);
#endif
               }
            }
#endif
            /* Fill out the last frame with zeros before passing
             * it to the encoder
             */
            if(samp_ret>0 && samp_ret != num_samples)
            {


                for(i=0; i<(num_samples-samp_ret); i++)
                    inbuf[samp_ret+i]=0;    /*Fill remaining part of
                                                 the frame with 0*/
#ifdef TWO_INSTANCES
                instanceID=enc_config.instance_id;
#endif
                outbuf =(char *) malloc (params.mp3e_outbuf_size);/* get the next
                                                            empty buffer */
#ifdef MHZ_MEASURE
                RVDS_measure_begin();
#endif
#ifdef TIME_PROFILE_RVDS_ARM9
                dummyCall1();
#endif
#ifdef TIME_PROFILE
                gettimeofday(&StartTime, NULL);
#endif
#ifdef __WINCE
                WinCE_measure_begin();
#endif
                mp3e_encode_frame (inbuf,&enc_config,outbuf);

#ifdef MHZ_MEASURE
                RVDS_measure_end(num_samples);
#endif
#ifdef TIME_PROFILE_RVDS_ARM9
            dummyCall2();
#endif
#ifdef TIME_PROFILE
                totalsamples += num_samples;
                Elinux_measure_end();
#endif
#ifdef __WINCE
                WinCE_measure_end();
#endif
#ifndef TIME_PROFILE
                output_mp3_file(&enc_config, outbuf);
#endif
                free (outbuf);      /* free the used buffer */
            }
#ifdef TWO_INSTANCES
            if(samp_ret1>0 && samp_ret1 != num_samples1)
            {

                for(i=0; i<(num_samples1-samp_ret1); i++)
                    inbuf1[samp_ret1+i]=0;    /*Fill remaining part of the frame with 0*/

                instanceID=enc_config1.instance_id;
                outbuf1 =(char *) malloc (params1.mp3e_outbuf_size);/* get the next
                                                            empty buffer */
                mp3e_encode_frame (inbuf1,&enc_config1,outbuf1);
                output_mp3_file1(&enc_config1, outbuf1);
                free (outbuf1);      /* free the used buffer */
            }
#endif
        }
#ifdef TWO_INSTANCES
        else if (num_channels == 2 && num_channels1==2)          /*stereo wave file*/
#else
        else if (num_channels == 2)          /*stereo wave file*/
#endif
        {
#ifdef ENABLE_MY_FREAD


#ifdef TWO_INSTANCES
             samp_ret = my_afread (inbuf, sizeof (short int), num_samples*2, (void *)&input_raw_data);
             samp_ret1 = my_afread1 (inbuf1, sizeof (short int), num_samples1*2, (void *)&input_raw_data1);

            while((samp_ret == num_samples*2) || (samp_ret1== num_samples1*2))
#else
            while ((samp_ret = my_afread (inbuf, sizeof (short int), num_samples*2, (void *)&input_raw_data)) ==  num_samples*2)
#endif



#else


#ifdef TWO_INSTANCES
             samp_ret = afread (inbuf, sizeof (short int), num_samples*2, ifp);
             samp_ret1 = afread1 (inbuf1, sizeof (short int), num_samples1*2, ifp1);

            while((samp_ret == num_samples*2) || (samp_ret1== num_samples1*2))
#else
            while ((samp_ret = afread (inbuf, sizeof (short int), num_samples*2, ifp)) ==  num_samples*2)
#endif


#endif
            {
#ifdef TWO_INSTANCES
            	if(samp_ret==num_samples*2)
            	{

            	instanceID=enc_config.instance_id;
#endif
                outbuf =(char *) malloc (params.mp3e_outbuf_size);/* get the next
                                                            empty buffer */
#ifdef MHZ_MEASURE
                RVDS_measure_begin();
#endif
#ifdef TIME_PROFILE_RVDS_ARM9
                dummyCall1();
#endif
#ifdef TIME_PROFILE
                gettimeofday(&StartTime, NULL);
#endif
#ifdef __WINCE
                WinCE_measure_begin();
#endif
                mp3e_encode_frame (inbuf,&enc_config,outbuf);
                /* This function does encoding of one frame. It
                   internally calls swap_output_buf_mp3e() when
                   one full MP3 output buffer is available. */
#ifdef MHZ_MEASURE
                RVDS_measure_end(num_samples);
#endif
#ifdef TIME_PROFILE_RVDS_ARM9
            dummyCall2();
#endif
#ifdef TIME_PROFILE
                totalsamples += num_samples;
                Elinux_measure_end();
#endif
#ifdef __WINCE
                WinCE_measure_end();
#endif
#ifndef TIME_PROFILE
                output_mp3_file(&enc_config, outbuf);
#endif
                /*frame_count++;*/
                /*fprintf (fout, "FrameNumber = %5d, FrameLenth = %5d, output_num_bytes = %5d\n", frame_count, params.mp3e_outbuf_size, enc_config.num_bytes);*/
                /*printf ("FrameNumber = %5d, FrameLenth = %5d, output_num_bytes = %5d\n", frame_count, params.mp3e_outbuf_size, enc_config.num_bytes);*/
                free (outbuf);      /* free the used buffer */
#ifdef TWO_INSTANCES
#ifdef ENABLE_MY_FREAD
                samp_ret = my_afread (inbuf, sizeof (short int), num_samples*2, (void *)&input_raw_data);
#else
                samp_ret = afread (inbuf, sizeof (short int), num_samples*2, ifp);
#endif
	            }
                 if(samp_ret1==num_samples1*2)
                 {

                  instanceID=enc_config1.instance_id;
                  outbuf1 =(char *) malloc (params1.mp3e_outbuf_size);
                  /* get the next empty buffer */
            	  mp3e_encode_frame (inbuf1,&enc_config1,outbuf1);
                  output_mp3_file1(&enc_config1, outbuf1);
                  free (outbuf1);      /* free the used buffer */
#ifdef ENABLE_MY_FREAD
                  samp_ret1 = my_afread1 (inbuf1, sizeof (short int), num_samples1*2, (void *)&input_raw_data1);
#else
            	  samp_ret1 = afread1 (inbuf1, sizeof (short int), num_samples1*2, ifp1);
#endif

                 }
#endif

            }
            /* Fill out the last frame with zeros before passing it
             * to the encoder
             */
            if(samp_ret>0 && samp_ret != num_samples*2)
            {
                int i;
                for(i=0; i<(num_samples*2-samp_ret); i++)
                    inbuf[samp_ret+i]=0;
#ifdef TWO_INSTANCES
                instanceID=enc_config.instance_id;
#endif
                outbuf =(char *) malloc (params.mp3e_outbuf_size);/* get the next
                                                            empty buffer */
#ifdef MHZ_MEASURE
                RVDS_measure_begin();
#endif
#ifdef TIME_PROFILE_RVDS_ARM9
                dummyCall1();
#endif
#ifdef TIME_PROFILE
                gettimeofday(&StartTime, NULL);
#endif
#ifdef __WINCE
                WinCE_measure_begin();
#endif
                mp3e_encode_frame (inbuf,&enc_config,outbuf);
#ifdef MHZ_MEASURE
                RVDS_measure_end(num_samples);
#endif
#ifdef TIME_PROFILE_RVDS_ARM9
            dummyCall2();
#endif
#ifdef TIME_PROFILE
                totalsamples += num_samples;
                Elinux_measure_end();
#endif
#ifdef __WINCE
                WinCE_measure_end();
#endif
#ifndef TIME_PROFILE
                output_mp3_file(&enc_config, outbuf);
#endif
                /*frame_count++;*/
                /*fprintf (fout, "FrameNumber = %5d, FrameLenth = %5d, output_num_bytes = %5d\n", frame_count, params.mp3e_outbuf_size, enc_config.num_bytes);*/
                /*printf ("FrameNumber = %5d, FrameLenth = %5d, output_num_bytes = %5d\n", frame_count, params.mp3e_outbuf_size, enc_config.num_bytes);*/
                free (outbuf);      /* free the used buffer */
            }

#ifdef TWO_INSTANCES
            if(samp_ret1>0 && samp_ret1 != num_samples1*2)
            {
                int i;
                for(i=0; i<(num_samples1*2-samp_ret1); i++)
                    inbuf1[samp_ret1+i]=0;
                instanceID=enc_config1.instance_id;
                outbuf1 =(char *) malloc (params1.mp3e_outbuf_size); /* get the next
                                                            empty buffer */
                mp3e_encode_frame (inbuf1,&enc_config1,outbuf1);
                output_mp3_file1(&enc_config1, outbuf1);
                free (outbuf1);      /* free the used buffer */
            }
#endif
        }
        else
            fprintf(stderr,"More than 2 channel input not supported\n");
    }
#ifndef TWO_INSTANCES
    else
    {
        /* If the input file is a pcm file, use fread function to read the
         * samples
         */
#ifdef ENABLE_MY_FREAD
        while ((samp_ret = my_fread (inbuf, sizeof (short int), num_samples*2, (void *)&input_raw_data)) ==  num_samples*2)
#else
        while ((samp_ret = fread (inbuf, sizeof (short int), num_samples*2, ifp)) ==  num_samples*2)
#endif
        {
            outbuf =(char *) malloc (params.mp3e_outbuf_size);/* get the next
                                                            empty buffer */
#ifdef MHZ_MEASURE
            RVDS_measure_begin();
#endif
#ifdef TIME_PROFILE_RVDS_ARM9
                dummyCall1();
#endif
#ifdef TIME_PROFILE
            gettimeofday(&StartTime, NULL);
#endif
#ifdef __WINCE
            WinCE_measure_begin();
#endif
            mp3e_encode_frame (inbuf,&enc_config,outbuf);
            /* This function does encoding of one frame. It
               internally calls swap_output_buf_mp3e() when
               one full MP3 output buffer is available. */
#ifdef MHZ_MEASURE
            RVDS_measure_end(num_samples);
#endif
#ifdef TIME_PROFILE_RVDS_ARM9
            dummyCall2();
#endif
#ifdef TIME_PROFILE
            totalsamples += num_samples;
            Elinux_measure_end();
#endif
#ifdef __WINCE
            WinCE_measure_end();
#endif
#ifndef TIME_PROFILE
            output_mp3_file(&enc_config, outbuf);
#endif
                /*frame_count++;*/
                /*fprintf (fout, "FrameNumber = %5d, FrameLenth = %5d, output_num_bytes = %5d\n", frame_count, params.mp3e_outbuf_size, enc_config.num_bytes);*/
            free (outbuf);      /* free the used buffer */
        }
        /* Fill out the last frame with zeros before passing it to the encoder
         */
        if(samp_ret>0 && samp_ret != num_samples*2)
        {
            int i;
            for(i=0; i<(num_samples*2-samp_ret); i++)
                inbuf[samp_ret+i]=0;

            outbuf =(char *) malloc (params.mp3e_outbuf_size);/* get the next
                                                            empty buffer */
#ifdef MHZ_MEASURE
            RVDS_measure_begin();
#endif
#ifdef TIME_PROFILE_RVDS_ARM9
                dummyCall1();
#endif
#ifdef TIME_PROFILE
            gettimeofday(&StartTime, NULL);
#endif
#ifdef __WINCE
            WinCE_measure_begin();
#endif
            mp3e_encode_frame (inbuf,&enc_config,outbuf);
#ifdef MHZ_MEASURE
            RVDS_measure_end(num_samples);
#endif
#ifdef TIME_PROFILE_RVDS_ARM9
            dummyCall2();
#endif
#ifdef TIME_PROFILE
            totalsamples += num_samples;
            Elinux_measure_end();
#endif
#ifdef __WINCE
            WinCE_measure_end();
#endif
#ifndef TIME_PROFILE
            output_mp3_file(&enc_config, outbuf);
#endif
                /*frame_count++;*/
                /*fprintf (fout, "FrameNumber = %5d, FrameLenth = %5d, output_num_bytes = %5d\n", frame_count, params.mp3e_outbuf_size, enc_config.num_bytes);*/
                /*printf ("FrameNumber = %5d, FrameLenth = %5d, output_num_bytes = %5d\n", frame_count, params.mp3e_outbuf_size, enc_config.num_bytes);*/
            free (outbuf);      /* free the used buffer */
        }
    }
#endif
#ifdef TWO_INSTANCES
    instanceID=enc_config.instance_id;
#endif

    outbuf =(char *) malloc (params.mp3e_outbuf_size);/* get the next
                                                            empty buffer */
    mp3e_flush_bitstream(&enc_config, outbuf);   /* flush any pending output bytes in the encoder */

    output_mp3_file(&enc_config, outbuf);
#ifdef TWO_INSTANCES
    instanceID=enc_config1.instance_id;
    outbuf1 =(char *) malloc (params1.mp3e_outbuf_size);/* get the next
                                                            empty buffer */
    mp3e_flush_bitstream(&enc_config1, outbuf1);   /* flush any pending output bytes in the encoder */

    output_mp3_file1(&enc_config1, outbuf1);
#endif

    /*frame_count++;*/
    /*fprintf (fout, "FrameNumber = %5d, FrameLenth = %5d, output_num_bytes = %5d\n", frame_count, params.mp3e_outbuf_size, enc_config.num_bytes);*/
    /*printf ("FrameNumber = %5d, FrameLenth = %5d, output_num_bytes = %5d\n", frame_count, params.mp3e_outbuf_size, enc_config.num_bytes);*/
    /* Free the input and the output file pointers */
#if defined(TIME_PROFILE)
	fprintf (ph_mips,"\n|\tAudio Format\t|\tPlat\t|\tSamplerate\t|\tCh\t|\tBiterate\t|\tBit\t|\tPerf(MIPS)\t|\tAudio file name\n");
       if(0 == (params.app_mode & 3))
       	channel = 2;
	else if(1 == (params.app_mode & 3))
		channel = 1;
	performance = (double)TotalDecTimeUs * 0.000001*cpu_freq* params.app_sampling_rate /totalsamples;	//
       fprintf(ph_mips, "|\tMP3 Encoder \t|\tARM%d\t|\t%9d\t|\t%d\t|\t%8d\t|\t%d\t|\t%.8f\t|%s\n", platform, params.app_sampling_rate, channel, params.app_bit_rate*1000, 16, performance, outPath);
       fclose(ph_mips);
#endif

    fclose(fout);
    fclose (ifp);
    fclose (ofp);
#ifdef TWO_INSTANCES
    fclose (ifp1);
    fclose (ofp1);
#endif
    free (outbuf);      /* free the used buffer */
    free (W1[0]);
    free (W2[0]);
    free (W3[0]);
    free (W4[0]);
    free (W5[0]);
    free (W6[0]);
#ifdef TWO_INSTANCES
    free (outbuf1);      /* free the used buffer */
    free (W1[1]);
    free (W2[1]);
    free (W3[1]);
    free (W4[1]);
    free (W5[1]);
    free (W6[1]);
#endif
    return 0;
}

/*****************************************************************************
 *
 * Output MP3 file
 *
 * DESCRIPTION
 *   This is a fuction to output MP3 file
 *
 *****************************************************************************/
void output_mp3_file(MP3E_Encoder_Config *enc_config,MP3E_INT8 *outbuf)
{
    if (enc_config->num_bytes != 0)
       {
           int i;
           MP3E_INT8 *outbuf_ptr;
           outbuf_ptr = outbuf;
           fwrite (outbuf, sizeof (char), enc_config->num_bytes, ofp); /* write the filled output buf onto the output file */
           for(i=0;i<enc_config->num_bytes;i++)
           *outbuf_ptr++=0;
        }
}
/*****************************************************************************
 *
 * Output MP3 file
 *
 * DESCRIPTION
 *   This is a fuction to output MP3 file
 *
 *****************************************************************************/
#ifdef TWO_INSTANCES
void output_mp3_file1(MP3E_Encoder_Config *enc_config1,MP3E_INT8 *outbuf1)
{
    if (enc_config1->num_bytes != 0)
       {
           int i;
           MP3E_INT8 *outbuf_ptr;
           outbuf_ptr = outbuf1;
           fwrite (outbuf1, sizeof (char), enc_config1->num_bytes, ofp1); /* write the filled output buf onto the output file */
           for(i=0;i<enc_config1->num_bytes;i++)
           *outbuf_ptr++=0;
        }
}
#endif
/*****************************************************************************
 *
 * RVDS_MEASURE_BEGIN
 *
 * DESCRIPTION
 *   This is a fuction to begin record current cycle value
 *
 *****************************************************************************/
#ifdef MHZ_MEASURE
void RVDS_measure_begin()
{
   __asm
   {
       mrc p15, 0, prev_clk, c15, c12, 0;  /* Read the Monitor conrol register */
       orr prev_clk, prev_clk, 0xf;
       mcr p15, 0, prev_clk, c15, c12, 0;   /* write rge Monitor control regsiter */
       mrc p15, 0, prev_clk, c15, c12, 1;  /* Read count register 0 */
   }
}

/*****************************************************************************
 *
 * RVDS_MEASURE_end
 *
 * DESCRIPTION
 *   This is a fuction to record current cycle value and calculate the time that ecoder process
 *
 *****************************************************************************/
void RVDS_measure_end(int num_samples)
{
   __asm
   {
       mrc p15, 0, curr_clk, c15, c12, 1;  /* Read cycle count register 0 */
   }
             frame_Num++;
             clk = (curr_clk-prev_clk);
            if ((curr_clk-prev_clk) > max_clk)
            {
                max_clk = curr_clk-prev_clk;
                max_frm = frame_Num;
                /*max_sf = params->app_sampling_rate;*/
            }
            den = (double)(num_samples * 1000000.0);
            curr_mhz= (double)((double)(64.0*clk*(params.app_sampling_rate))/den);
            fprintf (fout, "FrameNumber = %10d, max_clk = %10d, max_frm = %5d, Current frame Mhz = %16f\n", frame_Num, max_clk, max_frm, curr_mhz);
}
#endif
/*****************************************************************************
 *
 * Elinux_measure_end
 *
 * DESCRIPTION
 *   This is a fuction to record current cycle value and calculate the time that ecoder process
 *
 *****************************************************************************/
#ifdef TIME_PROFILE
void Elinux_measure_end()
{
        gettimeofday(&EndTime, NULL);
        nframe++;
        CurDecTimeUs = (EndTime.tv_usec - StartTime.tv_usec) + (EndTime.tv_sec - StartTime.tv_sec)*1000000L;
        TotalDecTimeUs += CurDecTimeUs;
        if(CurDecTimeUs > MaxTime)
        {
            MaxTime = CurDecTimeUs;
            Maxframe = nframe;
        }
        if(CurDecTimeUs < MinTime)
        {
            MinTime = CurDecTimeUs;
            Minframe = nframe;
        }

        /*a1 = MHZ*CurDecTimeUs;
        a2 = a1*params.app_sampling_rate;
        a3 = a2/den;*/

        curr_mhz= ((MHZ*(params.app_sampling_rate))/den)*CurDecTimeUs;
#if 0
        fprintf (fout, "nframe =\t %d\t MaxTime =\t %d\t MinTime =\t %d\t Maxframe =\t %d\t Minframe =\t %d\t CurDecTimeUs =\t %d\t TotalDecTimeUs =\t %d\t Current frame Mhz = \t %16f\n", nframe, MaxTime, MinTime, Maxframe, Minframe, CurDecTimeUs, TotalDecTimeUs,curr_mhz);
#endif
}
#endif

/*****************************************************************************
 *
 * WinCE_MEASURE_BEGIN
 *
 * DESCRIPTION
 *   This is a fuction to begin record current cycle value
 *
 *****************************************************************************/
#ifdef __WINCE
void WinCE_measure_begin()
{
  /* for taking timing on wince platform*/
     /*INTERRUPTS_SET(0xDF); */ /*disable interrupt*/
     Flag=QueryPerformanceFrequency(&lpFrequency);
     Flag=QueryPerformanceCounter(&lpPerformanceCountBegin);
}

/*****************************************************************************
 *
 * WinCE_MEASURE_end
 *
 * DESCRIPTION
 *   This is a fuction to record current cycle value and calculate the time that ecoder process
 *
 *****************************************************************************/
void WinCE_measure_end()
{
    Flag=QueryPerformanceCounter(&lpPerformanceCountEnd);
    Temp=(((lpPerformanceCountEnd.QuadPart - lpPerformanceCountBegin.QuadPart)*1000000)/(lpFrequency.QuadPart));//this is the duration
    TotalDecTime += Temp;
    frame_Num++;
    if(MaxDecTime<Temp){
        MaxDecTime = Temp;
        max_frame = frame_Num;
        }
    if((MinDecTime == 0)||(MinDecTime>Temp)){
        MinDecTime = Temp;
        min_frame = frame_Num;
        }
    /*INTERRUPTS_SET(0x1F) ;  *//*enable interrupt*/
    fprintf (fout, "FrameNumber = %10d, min_frame = %10d, max_frame = %5d, Current frame Time = %16d\n", frame_Num, min_frame, max_frame, Temp);
}
#endif
/************************************************************************
 *
 * parse_args
 *
 * DESCRIPTION
 *   Returns encoding parameters from the specifications of
 *   the command line.  Default settings are used for parameters
 *   not specified in the command line.
 *
 * The command line is parsed according to the following syntax:
 *
 * +m  is followed by the mode
 * +s  is followed by the sampling rate
 * +b  is followed by the total bitrate
 * +f  is followed by the input format
 *
 * The input and output filenames are read into "inPath" and "outPath".
 *
 ************************************************************************/
#ifdef TWO_INSTANCES
void parse_args(int argc, char** argv, int *mode, int *sfreq, int *bitrate, char *inPath, char *outPath,
                                       int *mode1,int *sfreq1,int *bitrate1,char *inPath1,char *outPath1 )
{
    int   err = 0, i = 0;
    int quality;
    int  quality1;
    char *programName = argv[0];

    /* preset defaults */
    inPath[0] = '\0';      /* inPath and outPath are set to '\0' */
    inPath1[0]='\0';
    outPath[0] = '\0';
    outPath1[0]='\0';

    *sfreq = DFLT_SFQ;     /* sfreq is set to default sampling frequency */
    *sfreq1= DFLT_SFQ;
    *bitrate = DFLT_BR;    /* bitrate is set to default bitrate */
    *bitrate1=DFLT_BR;

    if(DFLT_MOD == 'j')    /* mode flag is set to default stereo mode */
       {

        *mode = 0;
        *mode1=0;
       }
    else if(DFLT_MOD == 'm')
    {


        *mode = 1;
        *mode1=1;
    }
    else
    {


        *mode = 2;         /* Error in mode setting */
        *mode1=2;
    }

    if (DFLT_INTLV == 'i') /* mode flag is updated with default input format */
    {

        *mode += 0;
        *mode1 +=0;
    }
    else if (DFLT_INTLV == 'l')
    {


        *mode += 256;
        *mode1 +=256;
    }
    else
    {


        *mode += 512;      /* Error in mode setting */
        *mode1 +=512;
    }

    if (DFLT_CONFIG=='q')
    {


            quality = 1;
            quality1=1;
    }
    else if (DFLT_CONFIG == 's')
    {


            quality = 0;
            quality1=0;
    }
    else
    {


            quality = 2;
            quality1=2;
    }


    /* command line is parsed to set the parameters */
    while(++i<argc && err == 0)
    {
        char c, *token, *arg, *nextArg;
        int  argUsed;

        token = argv[i];
        if(*token++ == '+')
        {
           argUsed = 0;
           while( (c = *token++) )
           {
              switch(c)
              {


              case 'm':
                          argUsed = 1;
                          i++;
                          arg=argv[i];
                           if ((DFLT_MOD == 'j') && (*arg == 'm'))
                               *mode += 1;
                           else if ((DFLT_MOD == 'm') && (*arg == 'j'))
                               *mode -= 1;
                           else if ((*arg != 'j') && (*arg != 'm'))
                               *mode |= 2;
                           i++;
                           arg=argv[i];
                           if ((DFLT_MOD == 'j') && (*arg == 'm'))
                               *mode1 += 1;
                           else if ((DFLT_MOD == 'm') && (*arg == 'j'))
                               *mode1 -= 1;
                           else if ((*arg != 'j') && (*arg != 'm'))
                               *mode1 |= 2;
                           break;
                           /* Change the sampling frequency depending on input*/
                case 's':  argUsed = 1;
                           i++;
                           arg=argv[i];
                           *sfreq = atoi( arg );
                           i++;
                           arg=argv[i];
                           *sfreq1 = atoi( arg );
                           break;
                           /* Change the bitrate depending on input*/
                case 'b':  argUsed = 1;
                           i++;
                           arg=argv[i];
                           *bitrate = atoi(arg);
                           i++;
                           arg=argv[i];
                           *bitrate1 = atoi(arg);
                           break;
                           /* Change the mode flag depending on input*/
                case 'f':  argUsed = 1;
                            i++;
                            arg=argv[i];
                           if ((DFLT_INTLV == 'l') && (*arg == 'i'))
                               *mode -= 256;
                           else if ((DFLT_INTLV == 'i') && (*arg == 'l'))
                               *mode += 256;
                           else if ((*arg != 'l') && (*arg != 'i'))
                               *mode |= 512;
                           i++;
                            arg=argv[i];
                           if ((DFLT_INTLV == 'l') && (*arg == 'i'))
                               *mode1 -= 256;
                           else if ((DFLT_INTLV == 'i') && (*arg == 'l'))
                               *mode1 += 256;
                           else if ((*arg != 'l') && (*arg != 'i'))
                               *mode1 |= 512;
                           break;
                case 'c':  argUsed = 1;
                           i++;
                            arg=argv[i];
                           if(*arg == 's') quality = 0;
                           else if (*arg == 'q') quality = 1;
                           else quality = 2;
                           i++;
                            arg=argv[i];
                           if(*arg == 's') quality1 = 0;
                           else if (*arg == 'q') quality1 = 1;
                           else quality1 = 2;
                           break;
                           /* If any other options other than
                            * m,f,s,b are given */
                default:   fprintf(stderr,"%s: unrecognized option %c\n",programName, c);
                           err = 1;
                           break;
              }
                }
                if(argUsed)
                {
                    if(arg == token)
                        token = "";

                    arg = "";
                    argUsed = 0;
                }
            }

        else
        {   /* Read the input and the output filenames from arguments*/
            if(inPath[0] == '\0')       strcpy(inPath, argv[i]);
            else if(inPath1[0] == '\0')       strcpy(inPath1, argv[i]);
            else if(outPath[0] == '\0') strcpy(outPath, argv[i]);
            else if(outPath1[0] == '\0') strcpy(outPath1, argv[i]);
            else
            { /*Error if any excess arguments are passed*/
                fprintf(stderr,"%s: excess arg %s\n", programName, argv[i]);
                err = 1;
            }
        }
    }

        *mode |= (quality<<16);
        *mode1 |= (quality1<<16);

#ifdef MHZ_MEASURE_SOC
	strcpy(inPath, "castanets.wav");
	strcpy(outPath, "castanets.mp3");

	strcpy(inPath1, "fatboy.wav");
	strcpy(outPath1, "fatboy.mp3");
#endif

    if(err || inPath[0] == '\0' || outPath[0] == '\0'||inPath1[0] == '\0' || outPath1[0] == '\0')
        usage(programName);  /* never returns */

}
#else
void parse_args(int argc, char** argv, int *mode, int *sfreq, int *bitrate, char *inPath, char *outPath)
{
    int   err = 0, i = 0;
    int quality;
    char *programName = argv[0];

    /* preset defaults */
    inPath[0] = '\0';      /* inPath and outPath are set to '\0' */
    outPath[0] = '\0';

    *sfreq = DFLT_SFQ;     /* sfreq is set to default sampling frequency */
    *bitrate = DFLT_BR;    /* bitrate is set to default bitrate */

    if(DFLT_MOD == 'j')    /* mode flag is set to default stereo mode */
        *mode = 0;
    else if(DFLT_MOD == 'm')
        *mode = 1;
    else
        *mode = 2;         /* Error in mode setting */

    if (DFLT_INTLV == 'i') /* mode flag is updated with default input format */
        *mode += 0;
    else if (DFLT_INTLV == 'l')
        *mode += 256;
    else
        *mode += 512;      /* Error in mode setting */

    if (DFLT_CONFIG=='q')
            quality = 1;
    else if (DFLT_CONFIG == 's')
            quality = 0;
    else
            quality = 2;


    /* command line is parsed to set the parameters */
    while(++i<argc && err == 0)
    {
        char c, *token, *arg, *nextArg;
        int  argUsed;

        token = argv[i];
        if(*token++ == '+')
        {
            if(i+1 < argc) nextArg = argv[i+1];
            else           nextArg = "";
            argUsed = 0;
            while( (c = *token++) )
            {
                if(*token)
                    arg = token;
                else
                    arg = nextArg;
                switch(c)
                {          /* Change the mode flag depending on input*/
                case 'm':  argUsed = 1;
                           if ((DFLT_MOD == 'j') && (*arg == 'm'))
                               *mode += 1;
                           else if ((DFLT_MOD == 'm') && (*arg == 'j'))
                               *mode -= 1;
                           else if ((*arg != 'j') && (*arg != 'm'))
                               *mode |= 2;
                           break;
                           /* Change the sampling frequency depending on input*/
                case 's':  argUsed = 1;
                           *sfreq = atoi( arg );
                           break;
                           /* Change the bitrate depending on input*/
                case 'b':  argUsed = 1;
                           *bitrate = atoi(arg);
                           break;
                           /* Change the mode flag depending on input*/
                case 'f':  argUsed = 1;
                           if ((DFLT_INTLV == 'l') && (*arg == 'i'))
                               *mode -= 256;
                           else if ((DFLT_INTLV == 'i') && (*arg == 'l'))
                               *mode += 256;
                           else if ((*arg != 'l') && (*arg != 'i'))
                               *mode |= 512;
                           break;
                case 'c':  argUsed = 1;
                           if(*arg == 's') quality = 0;
                           else if (*arg == 'q') quality = 1;
                           else quality = 2;
                           break;
                           /* If any other options other than
                            * m,f,s,b are given */
                default:   fprintf(stderr,"%s: unrecognized option %c\n",programName, c);
                           err = 1;
                           break;
                }
                if(argUsed)
                {
                    if(arg == token)
                        token = "";
                    else
                        ++i;
                    arg = "";
                    argUsed = 0;
                }
            }
        }
        else
        {   /* Read the input and the output filenames from arguments*/
            if(inPath[0] == '\0')       strcpy(inPath, argv[i]);
            else if(outPath[0] == '\0') strcpy(outPath, argv[i]);
            else
            { /*Error if any excess arguments are passed*/
                fprintf(stderr,"%s: excess arg %s\n", programName, argv[i]);
                err = 1;
            }
        }
    }

        *mode |= (quality<<16);

#ifdef MHZ_MEASURE_SOC
        strcpy(inPath, "castanets.wav");
	strcpy(outPath, "castanets.mp3");
#endif

    if(err || inPath[0] == '\0' || outPath[0] == '\0')
        usage(programName);  /* never returns */

}
#endif
/*****************************************************************************
 *
 * usage
 *
 * DESCRIPTION
 *   Writes command line syntax to the file specified by "stderr"
 *
 *****************************************************************************/
#ifdef TWO_INSTANCES
void usage(char *programName)  /* print syntax & exit */
{
    fprintf(stderr,
            "Usage: %s [+m mode][+s sfrq][+b br][+f fmt][+c config] input outMP3\n",
            programName);
    fprintf(stderr,"where\n");
    fprintf(stderr," +m mode   stereo mode (m=mono ,j=joint-stereo)        : j/m  (dflt %5c)\n",DFLT_MOD);
    fprintf(stderr," +s sfrq   input sample rate in Hz (32000, 44100,48000)      (dflt %5d)\n",DFLT_SFQ);
    fprintf(stderr," +b br     total bitrate in kbps(32,48,64,96,128,256,320)       (dflt %5d)\n",DFLT_BR);
    fprintf(stderr," +f fmt    input format         (i=L/R interleaved, l=contiguous L samples+contiguous R samples):  (dflt %5c)\n",DFLT_INTLV);
    fprintf(stderr," +c config input configuration  (q=Optimized for quality, s=Optimized for speed): (dflt %5c)\n",DFLT_CONFIG);
    fprintf(stderr," input     input WAVE file\n");
    fprintf(stderr," outMP3    output MP3 encoded audio\n");
    exit(0);
}
#else
void usage(char *programName)  /* print syntax & exit */
{
    fprintf(stderr,
            "Usage: %s [+m mode][+s sfrq][+b br][+f fmt][+c config] input outMP3\n",
            programName);
    fprintf(stderr,"where\n");
    fprintf(stderr," +m mode   channel mode         : j/m  (dflt %5c)\n",DFLT_MOD);
    fprintf(stderr," +s sfrq   input smpl rate in Hz       (dflt %5d)\n",DFLT_SFQ);
    fprintf(stderr," +b br     total bitrate in kbps       (dflt %5d)\n",DFLT_BR);
    fprintf(stderr," +f fmt    input format         : i/l  (dflt %5c)\n",DFLT_INTLV);
    fprintf(stderr," +c config input configuration  : q/s  (dflt %5c)\n",DFLT_CONFIG);
    fprintf(stderr," input     input PCM/WAVE file\n");
    fprintf(stderr," outMP3    output MP3 encoded audio\n");
    exit(0);
}
#endif

#ifdef __ARM12
#ifndef __WINCE
#ifndef TGT_OS_ELINUX
__irq void IRQ_Handler(void)            //Lyon 070731
{


   *timer_clear = 1;
   *irq_enable_clr = 0x02;
   *irq_enable_set = 0x02;
}
#endif
#endif
#endif


/*****************************************************************************
 *

 * PROTOTYPE
 *
 * void encoder_mem_info_alloc(enc_object_mp3e *enc_config)
 *
 * DESCRIPTION
 * This function allocates the required memory as given by
 * query_mem_mp3e() API call.  Memory blocks of required sizes are
 * allocated and the corresponding memory block pointers are assigned
 * to mem_info[].ptr
 *****************************************************************************/

void encoder_mem_info_alloc (MP3E_Encoder_Config *enc_config)
{

int instance_id = enc_config->instance_id;

W1[instance_id] = (char *)malloc(sizeof(char)*enc_config->mem_info[0].size);
enc_config->mem_info[0].ptr =
    (int*)((unsigned int )(&W1[instance_id][0] +
               enc_config->mem_info[0].align - 1 )
               & (0xffffffff ^ (enc_config->mem_info[0].align - 1 )));
            /* includes Global variable structure*/

W2[instance_id] = (char *)malloc(sizeof(char)*enc_config->mem_info[1].size);
enc_config->mem_info[1].ptr =
    (int *)((unsigned int )(&W2[instance_id][0]
                            + enc_config->mem_info[1].align - 1 )
			& (0xffffffff ^ (enc_config->mem_info[1].align - 1 )));

W3[instance_id] = (char *)malloc(sizeof(char)*enc_config->mem_info[2].size);
enc_config->mem_info[2].ptr =
    (int *)((unsigned int )(&W3[instance_id][0]
                            + enc_config->mem_info[2].align - 1 )
			& (0xffffffff ^ (enc_config->mem_info[2].align - 1 )));

W4[instance_id] = (char *)malloc(sizeof(char)*enc_config->mem_info[3].size);
enc_config->mem_info[3].ptr =
    (int *)((unsigned int )(&W4[instance_id][0]
                            + enc_config->mem_info[3].align - 1 )
			& (0xffffffff ^ (enc_config->mem_info[3].align - 1 )));

W5[instance_id] = (char *)malloc(sizeof(char)*enc_config->mem_info[4].size);
enc_config->mem_info[4].ptr =
    (int *)((unsigned int )(&W5[instance_id][0]
                            + enc_config->mem_info[4].align - 1 )
			& (0xffffffff ^ (enc_config->mem_info[4].align - 1 )));

W6[instance_id] = (char *)malloc(sizeof(char)*enc_config->mem_info[5].size);
enc_config->mem_info[5].ptr =
    (int *)((unsigned int )(&W6[instance_id][0]
                            + enc_config->mem_info[5].align - 1 )
			& (0xffffffff ^ (enc_config->mem_info[5].align - 1 )));

}

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
