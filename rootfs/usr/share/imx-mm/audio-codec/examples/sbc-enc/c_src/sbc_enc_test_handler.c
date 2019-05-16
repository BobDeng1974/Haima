/*
 ***********************************************************************
 * Copyright (c) 2005-2008, 2012, Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */
/*==================================================================================================

  Module Name:  sbc_enc_test_handler.c

  General Description: Test handler for sbc encoder - file I/O, calls encoder engine

  ====================================================================================================
  Revsion History:
  Modification     Tracking
  Author                          Date          Number     Description of Changes
  -------------------------   ------------    ----------   -------------------------------------------
  Dusan Veselinovic           08/01/2004                   Initial Creation
  Tao  Jun                    19/02/2008      engr66159    Add MIPS test code for ARM11/ARM9
  Tao  Jun                    25/03/2008      engr69557    Build on different platforms
  Tao  Jun                    03/04/2008      engr70541    Some Changes for TIME_PROFILE
  Tao  Jun                    09/04/2008      engr71841    ADD MIPS test code for WinCE
  Tao  Jun	        20/05/2008      engr76823    Added version API
  ====================================================================================================
  INCLUDE FILES
  ==================================================================================================*/

#ifdef __WINCE
#include <windows.h>
#endif
#include "sbc_typedefs.h"
#include "sbc_defs.h"
#include "sbc_api.h"
#include "sbc_enc_test_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef RE_ENTRANCY
#include <pthread.h>
#endif

#ifdef TIME_PROFILE
#include <sys/time.h>
unsigned long totaltime = 0;
float mcps=0;
int32   frame_number = 0;
FILE *ph_mips;
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
int sample_rate = 0;
int channel = 0;
double performance = 0.0; 
#endif
#ifdef ARM11_MIPS_TEST_LERVDS
	int32 prev_clk, curr_clk, clk,maxclk = 0;
	FILE *fid; 
#endif
#ifdef ARM_MIPS_TEST_WINCE        		 /* for taking timing on wince platform*/
extern void INTERRUPTS_SET(int k);
		uint32 Flag;
        LARGE_INTEGER lpFrequency;
        LARGE_INTEGER lpPerformanceCountBegin;
        LARGE_INTEGER lpPerformanceCountEnd;
        int64 Temp;
        FILE *fid;
	int64 totaltime_wince = 0;
	int32   frame_number_wince = 0;
#endif

/*==================================================================================================
  GLOBAL VARIABLES
  ==================================================================================================*/


static char *help_text[] = {
	"",
	"Usage:  sbc_encode input_file options",
	"",
	"",
	" All options are required ecxept: ",
	" 1) Either bitpool or bitrate must be provided, but not both",
	" 2) Super frame size is optional. Encoder operates on data frame size that ",
	" is a product of the number of subbands and number of blocks. If input data ",
	" comes in frames of a certain predefined size, then the super_frame_size ",
	" quantity should be set to that size.",
	" 3) Psychoacoustic model may or may not be used ",
	"",
	"  input_file            Specify input file",
	"",
	"  -h                    Display this command line help and exit.",
	"",
	"  -l <blk_len>          block length (4,8,12 or 16)",
	"",
	"  -m <mode>             mode (0=mono, 1=dual_channel, 2=stereo, 3=joint stereo)",
	"",
	"  -o output_file        Specify output file.  Output suppressed if unspecified.",
	"",
	"  -n <subbands>         number of subbands (4 or 8)",
	"",
	"  -p                    Enable psycho-acoustic model [default is off]",
	"",
	"  -r <rate>             Bitrate in bps (cannot be combined with -b option)",
	"",
	"  -b <bitpool>          Bitpool value (2 to 250) (cannot be combined with -r option)",
	"",
	"  -s <sampling_freq>    Sampling Frequency (16000,32000,44100, or 48000 Hz)",
	"",
	"  -f <super_frame_size> Size of input data frame (e.g. 2048) - optional ",
	"",
	0
};

/*==================================================================================================
  LOCAL FUNCTIONS
  ==================================================================================================*/
void 	sbc_enc_hdlr(SbcEncApp *sbc_enc_app);
void	SBC_fill_buffer(SbcEncApp *sbc_enc_app);
uint8	SBC_get_byte(uint8 *byte, SbcEncApp *sbc_enc_app);
void	SBC_Get_Data(SbcEncApp *sbc_enc_app,uint8 *dst,uint32 num_bytes);
uint8 	SBC_app_init( SbcEncApp *sbc_enc_app);
#ifdef ARM9_MIPS_TEST_LERVDS
void 	dummyCall1(void);
void 	dummyCall2(void);
uint32  uiframe;
#endif

#ifdef RE_ENTRANCY
static TestAppMainFuncParam AppParam[MAX_INST];
static int nNumInstances = 0;

static TestAppMainfunc CompMainFunc;

int RunMultipleInstances(TestAppMainfunc CompMainFunc, int argc, char **argv)
{
	int nArgCnt = 1;
	int i,nArgCopied = 0;
	int nArgLeft = 0;
	nNumInstances = atoi(argv[nArgCnt]);
	nArgCnt++;
	if(nNumInstances > MAX_INST){
		return -1;
	}
	nArgLeft = argc - 2;

	for(i=1;i<=nNumInstances;i++)
	{
		nArgCopied = ParseArgs(nArgLeft,argv+nArgCnt,i);
		if(nArgCopied < 0)
			return -1;
		nArgLeft -= nArgCopied;
		nArgCnt+=nArgCopied;
	}

	for (i = 0; i < nNumInstances; i++)
	{
		if(pthread_create((pthread_t *)(&AppParam[i].AppThreadId),
					NULL,
					CompMainFunc,
					(void *)&AppParam[i]) != 0)
		{
			return -1;
		}
	}
	for (i = 0; i < nNumInstances; i++)
	{
		if(pthread_join((pthread_t)(AppParam[i].AppThreadId),NULL)!= 0)
		{
			return -1;
		}
	}

	return 0;
}


int ParseArgs(int nArgCnt,char **ArgList,int InstanceNum)
{
	int i = 0;
	int nArgTemp = 0;
	char Arg[100];
	static int inst = 0;

	memset(Arg,0,100);
	Arg[0]='-';
	sprintf(Arg+1,"%d",InstanceNum+1);

	if(InstanceNum == nNumInstances){
		AppParam[inst].nArgCnt=nArgCnt;
		AppParam[inst].sArgList=ArgList;
		return nArgCnt;
	}

	while(strcmp(ArgList[i],Arg))
		i++;

	AppParam[inst].nArgCnt=i;
	AppParam[inst].sArgList=ArgList;
	inst++;

	return i;
}
int SbcEnc_TestAppMain(void *CmndArg);
int main(int argc, char **argv)
{

	if(RunMultipleInstances(SbcEnc_TestAppMain, argc, argv) < 0)
		return -1;

	return 0;
}



#endif



/*==================================================================================================

FUNCTION: Usage() - Displays help message

==================================================================================================*/
void Usage(void)
{
	int i;
	for (i = 0; help_text[i]; i++)
		printf("%s\n", help_text[i]);
}

/*==================================================================================================

FUNCTION: parse_long() - Reads a number from command line

==================================================================================================*/
uint8 parse_long(char *str, int32 *val)
{
	char    *end_ptr;

	*val = strtol(str, &end_ptr, 0);
	if (*end_ptr)
	{
		fprintf(stderr, "ERROR: bad integer value '%s'\n", str);
		return FALSE;
	}

	return TRUE;
}

/*==================================================================================================

FUNCTION: parse_unsigned_long() - Reads a number from command line

==================================================================================================*/
uint8 parse_unsigned_long(char *str, uint32 *val)
{
	char    *end_ptr;

	*val = strtol(str, &end_ptr, 0);
	if (*end_ptr)
	{
		fprintf(stderr, "ERROR: bad integer value '%s'\n", str);
		return FALSE;
	}
	return TRUE;
}


/*==================================================================================================

FUNCTION: parse_command_line()

==================================================================================================*/
/* Parse the command line, filling in the global "options" variable. */
/* Return TRUE if all is well, else FALSE. */
uint8 parse_command_line(int argc, char *argv[], struct sbc_data_struct *sbc_enc_data,InputBitStream *inputBS,OutputFile *outFile)
{
	int     i;
	char   *option;
	/* default values */
	int32 bitpool =	0;
	int32 bitrate = 0;
	int32 subbands =	8;
	int32 blocklength= 16;
	int32 mode = 0;
	int32 sampling_frequency = 48000;
	int32 super_frame_size=0; /* if not specified, will be set to frame_size */
	uint8 psychoacoustics=FALSE;
	char *output_file_name=NULL;
	char *input_file_name=NULL;
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (argv[i][2])
			{
				/* Only single-letter options are supported */
				fprintf(stderr,"ERROR -- unrecognized option '%s'\n", argv[i]);
				return FALSE;
			}

			switch(argv[i][1])
			{
				case 'b':
					if (++i >= argc)
					{
						fprintf(stderr,"ERROR -- missing argument to option '%s'\n", argv[i-1]);
						return FALSE;
					}
					option = strtok(argv[i],",");
					if (!parse_long(option, &bitpool))
						return FALSE;
					if (bitpool < 2 || bitpool > 250){
						fprintf(stderr,"ERROR -- invalid bitpool value\n");
						return FALSE;
					}
					break;

				case 'r':
					if (++i >= argc)
					{
						fprintf(stderr,"ERROR -- missing argument to option '%s'\n", argv[i-1]);
						return FALSE;
					}
					option = strtok(argv[i],",");
					if (!parse_long(option, &bitrate))
						return FALSE;

					break;

				case 's':
					if (++i >= argc)
					{
						fprintf(stderr,"ERROR -- missing argument to option '%s'\n", argv[i-1]);
						return FALSE;
					}
					option = strtok(argv[i],",");
					if (!parse_long(option, &sampling_frequency))
						return FALSE;

					if(sampling_frequency != 16000 && sampling_frequency != 32000 && sampling_frequency != 44100 && sampling_frequency != 48000){
						fprintf(stderr,"ERROR -- sampling frequency invalid\n");
						return FALSE;
					}
					break;
				case 'f':
					if (++i >= argc)
					{
						fprintf(stderr,"ERROR -- missing argument to option '%s'\n", argv[i-1]);
						return FALSE;
					}
					option = strtok(argv[i],",");
					if (!parse_long(option, &super_frame_size))
						return FALSE;

					break;

				case 'l':
					if (++i >= argc)
					{
						fprintf(stderr,"ERROR -- missing argument to option '%s'\n", argv[i-1]);
						return FALSE;
					}
					option = strtok(argv[i],",");
					if (!parse_long(option, &blocklength))
						return FALSE;

					if(blocklength != 4 && blocklength!=8 && blocklength!= 12 && blocklength!=16){
						fprintf(stderr,"ERROR -- blocklength must be 4,8,12 or 16\n");
						return FALSE;
					}
					break;

				case 'n':
					if (++i >= argc)
					{
						fprintf(stderr,"ERROR -- missing argument to option '%s'\n", argv[i-1]);
						return FALSE;
					}
					option = strtok(argv[i],",");
					if (!parse_long(option, &subbands))
						return FALSE;

					if(subbands != 4 && subbands != 8){
						fprintf(stderr,"ERROR -- number of subbands must be 4 or 8\n");
						return FALSE;
					}
					break;

				case 'm':
					if (++i >= argc)
					{
						fprintf(stderr,"ERROR -- missing argument to option '%s'\n", argv[i-1]);
						return FALSE;
					}
					option = strtok(argv[i],",");
					if (!parse_long(option, &mode))
						return FALSE;

					if(mode != 0 && mode != 1 && mode!=2 && mode!=3){
						fprintf(stderr,"ERROR -- mode must be between 0 and 3\n");
						return FALSE;
					}
					break;

				case 'h':
					Usage();
					exit(0);
					break;

				case 'p':
					psychoacoustics = TRUE;
					break;

				case 'o':
					if (++i >= argc)
					{

						fprintf(stderr,"ERROR -- missing argument to option '%s'\n", argv[i-1]);
						return FALSE;
					}
					else if (!output_file_name)
						output_file_name = argv[i];
					else
					{
						fprintf(stderr,"ERROR -- only one output file name is allowed\n");
						return FALSE;
					}
					break;
				default:
					fprintf(stderr,"ERROR -- unrecognized option '%s'\n", argv[i]);
					return FALSE;
			}
		}
		else if (!input_file_name)
			input_file_name = argv[i];
		else
		{
			fprintf(stderr,"ERROR -- only one input file name is allowed\n");
			return FALSE;
		}
	}

	if (!input_file_name)
	{
		fprintf(stderr,"ERROR -- must specify an input file name\n");
		return FALSE;
	}

	if(!output_file_name){
		fprintf(stderr,"ERROR -- must specify an output file name\n");
		return FALSE;
	}

	if( (bitpool == 0 && bitrate == 0) || (bitpool !=0 && bitrate!=0) ){
		/* either bitpool or bitrate (but not both) must be specified*/
		fprintf(stderr,"ERROR -- must specify either bitpool or bitrate, but not both\n");
		return FALSE;
	}

	/* copy the data into the approrpriate structures */
	inputBS->filename = input_file_name;
	outFile->filename = output_file_name;

	sbc_enc_data->nrof_blocks	= blocklength;
	sbc_enc_data->channel_mode	= mode;
	sbc_enc_data->nrof_subbands	= subbands;

	if(sampling_frequency == 16000)
		sbc_enc_data->sampling_freq	= 0;
	else if(sampling_frequency == 32000)
		sbc_enc_data->sampling_freq = 1;
	else if(sampling_frequency == 44100)
		sbc_enc_data->sampling_freq = 2;
	else if(sampling_frequency == 48000)
		sbc_enc_data->sampling_freq = 3;

	sbc_enc_data->bitpool		= bitpool;
	sbc_enc_data->bitrate		= bitrate;

	if(psychoacoustics)
		sbc_enc_data->allocation_method = 0;
	else
		sbc_enc_data->allocation_method = 1;

	sbc_enc_data->frame_size		 = ( mode==0? blocklength * subbands : 2*blocklength*subbands);

	/* if super frame size is not specified, set it to be equal to the frame size */
	if(super_frame_size != 0){
		sbc_enc_data->super_frame_size= super_frame_size;
	}
	else{
		sbc_enc_data->super_frame_size = sbc_enc_data->frame_size;
	}

	if(super_frame_size % sbc_enc_data->frame_size != 0){
		fprintf(stderr,"ERROR -- there must be an integer number of frames in a superframe\n");
		return FALSE;
	}

	if(mode == 3){
		fprintf(stderr,"ERROR -- JOINT STEREO mode not supported at this time\n");
		return FALSE;
	}

	return TRUE;
}



/*==================================================================================================

FUNCTION: main() - Main function of test handler, file I/O

==================================================================================================*/
#ifdef RE_ENTRANCY
int SbcEnc_TestAppMain(void *CmndArg)
#else
#ifdef __WINCE
int _tmain(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
#endif //RE_ENTRANCY
{
	sbc_mem_allocinfo_subtype *psMem;
	SbcEncApp sbc_enc_app;
	SBC_RET_TYPE retVal;

	int nNumReqs;
	int nCounter;

#ifdef __WINCE
	_TCHAR *arg_word;
	int8 *arg_byte;
	int32 count;
#endif	
#ifdef RE_ENTRANCY
	TestAppMainFuncParam *pCmndArg = (TestAppMainFuncParam *)CmndArg;
	int argc = pCmndArg->nArgCnt;
	char **argv = pCmndArg->sArgList;
#endif //RE_ENTRANCY

#ifdef TIME_PROFILE
//	struct timeval tv_b,tv_a;
       ph_mips = fopen("../audio_performance.txt", "a");
#endif

#if _DEBUG
	fprintf(stdout,"\r\n  *** SBC Encoder Test Handler ***\r\n\r\n");
#endif
#ifndef TIME_PROFILE 
#ifndef ARM11_MIPS_TEST_LERVDS 
#ifndef ARM9_MIPS_TEST_LERVDS  	
#ifndef ARM_MIPS_TEST_WINCE
    	/* Get the version info of the sign-on SBC ENC */
    	fprintf(stderr,"Running %s\n",SBCEncVersionInfo());	
#endif
#endif
#endif
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
                        	
	if(!parse_command_line(argc,argv,&sbc_enc_app.sbc_enc_data,&sbc_enc_app.inputBS,&sbc_enc_app.outFile)){
		fprintf(stderr," ERROR - processing command line\n");
		Usage();
		exit(1);
	}
#ifdef   ARM11_MIPS_TEST_LERVDS		
		if ((fid = fopen("cycle_lervds.csv", "a")) == NULL)
		{
			fprintf(stderr, "FATAL ERROR: Open cycle_lervds.csv FAILURE!\n");
			exit(1);
		}
		fprintf(fid,"%s\n",sbc_enc_app.inputBS.filename);
#endif
/*
#ifdef ARM_MIPS_TEST_WINCE         // for taking timing on wince platform
	if ((fid = fopen("cycle_wince.csv", "a")) == NULL)        
 	{
		fprintf(stderr, "FATAL ERROR: Open cycle_wince.csv FAILURE!\n");
		exit(1);	
	}
#endif
*/
	sbc_enc_app.inputBS.infp = fopen(sbc_enc_app.inputBS.filename, "rb");
	if(sbc_enc_app.inputBS.infp == NULL){
#if _DEBUG
		fprintf(stderr,"! ERROR: Cannot open the specified input file %s.\r\n", sbc_enc_app.inputBS.filename);
#endif
		exit (1);

	}
/*#ifdef   ARM_MIPS_TEST_WINCE			
        	fprintf(fid,"%s\n",sbc_enc_app.inputBS.filename);
#endif  */
#ifndef TIME_PROFILE
#ifndef ARM11_MIPS_TEST_LERVDS	
#ifndef ARM9_MIPS_TEST_LERVDS
#ifndef ARM_MIPS_TEST_WINCE

	sbc_enc_app.outFile.outfp = fopen(sbc_enc_app.outFile.filename, "wb");

	if (NULL == sbc_enc_app.outFile.outfp) {
#if _DEBUG
		fprintf(stderr,"! ERROR: Unable to create RAW output file %s.\r\n", sbc_enc_app.outFile.filename);
#endif
		exit (2);

	}
#endif
#endif
#endif
#endif


	if(sbc_enc_query_mem(&sbc_enc_app.sbc_enc_config)!=SBC_OK)
	{
		printf("Query Memory Failed \n");
		exit(2);
	}

	/* allocate memory requested by the encoder*/
	nNumReqs = sbc_enc_app.sbc_enc_config.mem_info.nNumMemReqs;
	for(nCounter = 0;nCounter<nNumReqs;nCounter++)
	{
		psMem = &(sbc_enc_app.sbc_enc_config.mem_info.sMemInfoSub[nCounter]);
		psMem->pSBCBasePtr = malloc(psMem->nSBCSize);
	}
	/* INITIALIZE THE APPLICATION STRUCTURE*/
	SBC_app_init(&sbc_enc_app);

	/* INITIALIZE THE ENCODER */
	retVal= sbc_enc_init(&sbc_enc_app.sbc_enc_config);
	if(retVal !=SBC_OK)
	{
		printf("Encoder Initialization failed... Exiting \n");
		exit(2);
	}

	SBC_fill_buffer(&sbc_enc_app);
	/* ENCODE FRAMES */
#if _DEBUG
	fprintf(stdout, "\r\n----------------------------------------\r\n");
	fprintf(stdout, "  SBC Encoder  starts encoding %s\r\n",sbc_enc_app.inputBS.filename);
	fprintf(stdout, "----------------------------------------\r\n\r\n");
#endif

	while( sbc_enc_app.last_frame == FALSE ){
		sbc_enc_hdlr(&sbc_enc_app);
	}
#ifdef TIME_PROFILE
        switch(sbc_enc_app.sbc_enc_data.sampling_freq)
        {
        case 3:
		sample_rate = 48000;
		break;
	 case 2:
		sample_rate = 44100;
		break;
	 case 1:
		sample_rate = 32000;
		break;
	 case 0:
	 default:
		sample_rate = 16000;		
		break;
        }
    printf("[PROFILE-INFO]\t%s\t%d\t%d\t%s\t%d\t%d\t%s\t%d\t%ld\n",
        sbc_enc_app.inputBS.filename,
        sbc_enc_app.sbc_enc_data.nrof_subbands,
        sbc_enc_app.sbc_enc_data.nrof_blocks,
        sample_rate,
        sbc_enc_app.sbc_enc_data.bitpool,
        sbc_enc_app.sbc_enc_data.channel_mode,
        ((sbc_enc_app.sbc_enc_data.allocation_method==0)?"ENABLE":"DISABLE"),
        frame_number,
        totaltime); 
	switch(sbc_enc_app.sbc_enc_data.channel_mode)
	{
	case 0:
		channel = 1;
		break;
	case 1:
	case 2:
	case 3:
	default:
		channel = 2;
		break;
	}
	printf("\ntotal size: %d, super frame size:%d, frame size:%d, frame_number:%d, blocks:%d, subband:%d\n", 16260060, sbc_enc_app.sbc_enc_data.super_frame_size, sbc_enc_app.sbc_enc_data.frame_size, frame_number, sbc_enc_app.sbc_enc_data.nrof_blocks, sbc_enc_app.sbc_enc_data.nrof_subbands);
       fprintf (ph_mips,"\n|\tAudio Format\t|\tPlat\t|\tSamplerate\t|\tCh\t|\tBiterate\t|\tBit\t|\tPerf(MIPS)\t|\tAudio file name\n");       
	performance = (double)totaltime * 0.000001*cpu_freq* sample_rate  /frame_number/sbc_enc_app.sbc_enc_data.nrof_blocks/sbc_enc_app.sbc_enc_data.nrof_subbands;	//
       fprintf(ph_mips, "|\tSBC Encoder \t|\tARM%d\t|\t%9d\t|\t%d\t|\t%8d\t|\t%d\t|\t%.8f\t|%s\n", platform, sample_rate, channel, sbc_enc_app.sbc_enc_data.bitrate, 16, performance, sbc_enc_app.outFile.filename, sbc_enc_app.inputBS.filename);
    
#endif
#ifdef ARM_MIPS_TEST_WINCE  
    printf("[PROFILE-INFO]\t%s\t%d\t%d\t%s\t%d\t%d\t%s\t%d\t%ld\n",
        sbc_enc_app.inputBS.filename,
        sbc_enc_app.sbc_enc_data.nrof_subbands,
        sbc_enc_app.sbc_enc_data.nrof_blocks,
        (sbc_enc_app.sbc_enc_data.sampling_freq==3)?"48000":((sbc_enc_app.sbc_enc_data.sampling_freq==2)?"44100":((sbc_enc_app.sbc_enc_data.sampling_freq==1)?"32000":"16000")),
        sbc_enc_app.sbc_enc_data.bitpool,
        sbc_enc_app.sbc_enc_data.channel_mode,
        ((sbc_enc_app.sbc_enc_data.allocation_method==0)?"ENABLE":"DISABLE"),
        frame_number_wince,
        totaltime_wince);  
#endif


#if _DEBUG
	fprintf(stdout, "\r\n----------------------------------------\r\n");
	fprintf(stdout, "  SBC Encoder ends encoding frames\r\n");
	fprintf(stdout, "----------------------------------------\r\n\r\n");
#endif

//#if _DEBUG
//	fprintf(stdout, "\r\n----------------------------------------\r\n");
//	 fprintf(stdout, " SBC Encoder encoded %d frames \t\t          \r\n",sbc_enc_app.sbc_enc_config.frame_no); 
//	fprintf(stdout, "----------------------------------------\r\n\r\n");
//#endif



	for(nCounter = 0;nCounter<nNumReqs;nCounter++)
		free(sbc_enc_app.sbc_enc_config.mem_info.sMemInfoSub[nCounter].pSBCBasePtr);

	free(sbc_enc_app.in_buf);
	free(sbc_enc_app.out_buf);
	free(sbc_enc_app.circular_buffer);

	if(NULL != sbc_enc_app.inputBS.infp)
	{
		fclose(sbc_enc_app.inputBS.infp);
	}
#ifndef TIME_PROFILE
#ifndef ARM11_MIPS_TEST_LERVDS	
#ifndef ARM9_MIPS_TEST_LERVDS
#ifndef ARM_MIPS_TEST_WINCE
	if(NULL != sbc_enc_app.outFile.outfp)
	{
		fclose(sbc_enc_app.outFile.outfp);
	}
#endif
#endif
#endif
#endif
#ifdef	ARM11_MIPS_TEST_LERVDS	
        fprintf(stderr, "MAX NUMBER: %d\n", maxclk);	
        fclose(fid);
#endif    
/*#ifdef ARM_MIPS_TEST_WINCE    
        fclose(fid);    
#endif */
        exit(0);
}

/*==================================================================================================

FUNCTION: sbc_enc_hdlr() - Test handler - calls SBC encoder engine, fills input buffers etc.

==================================================================================================*/
void sbc_enc_hdlr(SbcEncApp *sbc_enc_app)
{
	int32 buffer_fullness;
	uint32 N;
	uint32 i;
#ifdef TIME_PROFILE
	struct timeval tv_b,tv_a;
#endif
	/* number of frames in a super frame */
	N = sbc_enc_app->sbc_enc_config.sbc_data->super_frame_size/sbc_enc_app->sbc_enc_config.sbc_data->frame_size;


	/* make sure there is enough data before calling encoder */
	buffer_fullness = (uint32) sbc_enc_app->write_ptr - (uint32) sbc_enc_app->read_ptr;
	if(buffer_fullness < 0) buffer_fullness += CIRC_BUF_SIZE;


	/* Copy the super frame data from circular buffer into input buffer */
	sbc_enc_app->sbc_enc_input.input_frame_buf = sbc_enc_app->in_buf;
	if(buffer_fullness < sizeof(int16) * sbc_enc_app->sbc_enc_config.sbc_data->super_frame_size){
		/* if there is not enough data in the file, zero pad the last frame*/
		SBC_Get_Data(sbc_enc_app,sbc_enc_app->sbc_enc_input.input_frame_buf, buffer_fullness);
		for( i=0; i<( sizeof(int16)*sbc_enc_app->sbc_enc_config.sbc_data->super_frame_size - buffer_fullness);i++){
			* ( (uint8 *) ((uint32) sbc_enc_app->sbc_enc_input.input_frame_buf + (uint32)buffer_fullness + (uint32) i) )=0;
		}
	}
	else{
		SBC_Get_Data(sbc_enc_app,sbc_enc_app->sbc_enc_input.input_frame_buf, sizeof(int16) * sbc_enc_app->sbc_enc_config.sbc_data->super_frame_size);
	}


	/* call the encoder N times ( N = super_frame_size/(nrof_blocks*nrof_channels*nrof_subbands)*/
	for(i=0;i<N;i++){

		sbc_enc_app->sbc_enc_output.out_buffer = sbc_enc_app->out_buf;

#ifdef TIME_PROFILE
		gettimeofday(&tv_b,NULL);
#endif
#ifdef ARM11_MIPS_TEST_LERVDS			
					__asm
					{
						mrc p15, 0, prev_clk, c15, c12, 0;			/* Read the Monitor conrol register */
						orr prev_clk, prev_clk, 0xf;
						mcr p15, 0, prev_clk, c15, c12, 0			/* write rge Monitor control regsiter */
						mrc p15, 0, prev_clk, c15, c12, 1;			/* Read count register 0 */
					}
#endif   
#ifdef ARM9_MIPS_TEST_LERVDS
		dummyCall1();
#endif
#ifdef ARM_MIPS_TEST_WINCE						/* for taking timing on wince platform*/
     INTERRUPTS_SET(0xDF);  						/*disable interrupt*/
     Flag=QueryPerformanceFrequency(&lpFrequency);
     Flag=QueryPerformanceCounter(&lpPerformanceCountBegin);
#endif
		if(sbc_encoder_encode(&sbc_enc_app->sbc_enc_config,&sbc_enc_app->sbc_enc_input,&sbc_enc_app->sbc_enc_output)!=SBC_OK)
		{
			printf("Encode failed \n");
			exit(2);
		}
#ifdef ARM_MIPS_TEST_WINCE
        Flag=QueryPerformanceCounter(&lpPerformanceCountEnd);
        Temp=(((lpPerformanceCountEnd.QuadPart - lpPerformanceCountBegin.QuadPart)*1000000)/(lpFrequency.QuadPart));/*this is the duration*/
        INTERRUPTS_SET(0x1F) ;  					/*enable interrupt*/
		frame_number_wince++;
		totaltime_wince+=Temp;
        //fprintf(fid,"%d\n",Temp);  
#endif			
#ifdef ARM9_MIPS_TEST_LERVDS		
		dummyCall2();
#endif

#ifdef TIME_PROFILE
		gettimeofday(&tv_a,NULL);
		totaltime += (tv_a.tv_sec -tv_b.tv_sec)*1000000 + (tv_a.tv_usec -tv_b.tv_usec);
		frame_number++;
#endif
#ifdef	ARM11_MIPS_TEST_LERVDS
				__asm
					{
							mrc p15, 0, curr_clk, c15, c12, 1;			/* Read cycle count register 0 */
					}
					clk = (curr_clk-prev_clk);
				    maxclk = (clk>maxclk)?clk:maxclk;
					fprintf(fid,"%d\n",clk);
#endif

		/* advance the input pointer to the next frame in a super_frame */
		sbc_enc_app->sbc_enc_input.input_frame_buf = (void *)((uint32)sbc_enc_app->sbc_enc_input.input_frame_buf + 2 * sbc_enc_app->sbc_enc_config.sbc_data->frame_size);

		if(sbc_enc_app->sbc_enc_config.sbc_enc_status != SBC_FAIL){

//#if _DEBUG
//			sbc_enc_app->sbc_enc_config.frame_no++;
//#endif

#ifndef TIME_PROFILE		/* Output the encoded data */
#ifndef ARM11_MIPS_TEST_LERVDS
#ifndef ARM9_MIPS_TEST_LERVDS
#ifndef ARM_MIPS_TEST_WINCE
			fwrite(sbc_enc_app->sbc_enc_output.out_buffer, 1, sbc_enc_app->sbc_enc_output.size , sbc_enc_app->outFile.outfp);
#endif
#endif
#endif
#endif
		}
		else{
//#if _DEBUG
//			fprintf(stdout, "\r\n----------------------------------------\r\n");
//			fprintf(stdout, "  SBC Enc failed at frame %d with code %x \r\n",sbc_enc_app->sbc_enc_config.frame_no,sbc_enc_app->sbc_enc_output.debug_info); 
//			fprintf(stdout, "----------------------------------------\r\n\r\n");
//#endif
			sbc_enc_app->sbc_enc_config.control_word = SBC_DISABLE;
			sbc_enc_app->last_frame = TRUE;
			break;
		}


	}

	/* replenish the circular buffer after encoding 1 super_frame */
	SBC_fill_buffer(sbc_enc_app);
}

/*==================================================================================================

FUNCTION: SBC_Get_Data() - Reads data from the circular input buffer to input buffer

==================================================================================================*/
void SBC_Get_Data(SbcEncApp *sbc_enc_app,uint8 *dst,uint32 num_bytes)
{
	uint32 i;
	for(i=0;i<num_bytes;i++){
		*dst++ = *(sbc_enc_app->read_ptr);
		sbc_enc_app->read_ptr++;
		if( (uint32) sbc_enc_app->read_ptr > (uint32) sbc_enc_app->circ_buf_end ){
			sbc_enc_app->read_ptr = sbc_enc_app->circ_buf_start;
		}
	}
}
/*==================================================================================================

FUNCTION: SBC_fill_buffer() - Fills the circular input buffer with data

==================================================================================================*/
void SBC_fill_buffer(SbcEncApp *sbc_enc_app)
{

	uint8 status = SBC_OK;
	uint8 byte;

	int32 delta;

	delta = (uint32) sbc_enc_app->write_ptr - (uint32) sbc_enc_app->read_ptr;

	if(sbc_enc_app->end_of_buffer == TRUE){
		if(delta == 0){
			sbc_enc_app->last_frame = TRUE;
		}
		return;
	}

	while( (delta >= 0 && delta < (CIRC_BUF_SIZE - 1)) || (delta < -1) )
	{
		status = SBC_get_byte(&byte ,sbc_enc_app);
		if(status != SBC_OK){
			sbc_enc_app->end_of_buffer = TRUE;
			break;
		}
		*(sbc_enc_app->write_ptr) = byte;
		if(++(sbc_enc_app->write_ptr) > sbc_enc_app->circ_buf_end){
			sbc_enc_app->write_ptr = sbc_enc_app->circ_buf_start;
		}

		delta = (uint32)sbc_enc_app->write_ptr - (uint32)sbc_enc_app->read_ptr;

	}

}

/*==================================================================================================

FUNCTION: SBC_get_byte() - Reads 1 byte from a file

==================================================================================================*/
uint8 SBC_get_byte(uint8 *byte, SbcEncApp *sbc_enc_app)
{
	*byte = fgetc(sbc_enc_app->inputBS.infp);

	if(feof(sbc_enc_app->inputBS.infp)){
		return SBC_FAIL;
	}
	else{
		return SBC_OK;
	}
}

/*==================================================================================================

FUNCTION: SBC_app_init() - Initialise the members of the application structure

==================================================================================================*/
uint8 SBC_app_init( SbcEncApp *sbc_enc_app)
{
	sbc_enc_app->in_buf = (uint8 *)malloc(SBC_ENC_IN_BUF_SIZE);
	sbc_enc_app->out_buf = (uint8 *)malloc(SBC_ENC_OUT_BUF_SIZE);
	sbc_enc_app->circular_buffer= (uint8 *)malloc(CIRC_BUF_SIZE);
	memset(sbc_enc_app->in_buf,0,SBC_ENC_IN_BUF_SIZE);
	memset(sbc_enc_app->out_buf,0,SBC_ENC_OUT_BUF_SIZE);
	memset(sbc_enc_app->circular_buffer,0,CIRC_BUF_SIZE);
	sbc_enc_app->last_frame = FALSE;
	sbc_enc_app->end_of_buffer = FALSE;
	sbc_enc_app->frame_no = 0;
	sbc_enc_app->circ_buf_start = sbc_enc_app->circular_buffer;
	sbc_enc_app->circ_buf_end   = sbc_enc_app->circular_buffer + CIRC_BUF_SIZE-1;
	sbc_enc_app->read_ptr = sbc_enc_app->circ_buf_start;
	sbc_enc_app->write_ptr = sbc_enc_app->circ_buf_start;
	sbc_enc_app->sbc_enc_input.input_frame_buf = sbc_enc_app->in_buf;
	sbc_enc_app->sbc_enc_output.out_buffer = sbc_enc_app->out_buf;
	sbc_enc_app->sbc_enc_config.sbc_data = &sbc_enc_app->sbc_enc_data;
	sbc_enc_app->sbc_enc_output.sbc_data = NULL;
	return SBC_OK;
}
#ifdef ARM9_MIPS_TEST_LERVDS
void dummyCall1(void)
{
	uiframe++;
}

void dummyCall2(void)
{
	uiframe++;
}
#endif


