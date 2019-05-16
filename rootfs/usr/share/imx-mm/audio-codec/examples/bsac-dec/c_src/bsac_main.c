 
/************************* MPEG-2 NBC Audio Decoder **************************
 *                                                                           *
"This software module was originally developed by
AT&T, Dolby Laboratories, Fraunhofer Gesellschaft IIS
and edited by
Yoshiaki Oikawa (Sony Corporation),
Mitsuyuki Hatanaka (Sony Corporation)
in the course of development of the MPEG-2 NBC/MPEG-4 Audio standard ISO/IEC 13818-7,
14496-1,2 and 3. This software module is an implementation of a part of one or more
MPEG-2 NBC/MPEG-4 Audio tools as specified by the MPEG-2 NBC/MPEG-4
Audio standard. ISO/IEC  gives users of the MPEG-2 NBC/MPEG-4 Audio
standards free license to this software module or modifications thereof for use in
hardware or software products claiming conformance to the MPEG-2 NBC/MPEG-4
Audio  standards. Those intending to use this software module in hardware or
software products are advised that this use may infringe existing patents.
The original developer of this software module and his/her company, the subsequent
editors and their companies, and ISO/IEC have no liability for use of this software
module or modifications thereof in an implementation. Copyright is not released for
non MPEG-2 NBC/MPEG-4 Audio conforming products.The original developer
retains full right to use the code for his/her  own purpose, assign or donate the
code to a third party and to inhibit third party from using the code for non
MPEG-2 NBC/MPEG-4 Audio conforming products. This copyright notice must
be included in all copies or derivative works."
Copyright(c)1996.
 *                                                                           *
 ****************************************************************************/
/************************************************************************
 * Copyright (c) 2005-2010, 2012, Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc.
 ***********************************************************************
 */

/*******************************************************************************
 *
 *   CHANGE HISTORY
 *   dd/mm/yy   Code Ver     Description                        Author
 *   --------   -------      -----------                        ------
 *   13/04/2000 01           Created                            Nanda
 *                                                              Kishore A.S
 *   24/05/2004 02           Modified for relocatability        Vishala
 *   08/06/2004 03           Modifed for api changes            Vishala
 *   14/06/2004 04           Modified for table
 *                           relocatability                     Vishala
 *
 *   08/07/2004 02           Defined ARM_ADS, SINGLE_VECTOR     S.Nachiappan
 *                           to handle decoding
 *                           of single/multiple input files
 *
 *   19/08/2004 03           The decoder now, decodes only      S.Nachiappan
 *                           raw_data_blocks. The application
 *                           parses headers and passes them
 *                           to the decoder.
 *
 *                           To enable the application to
 *                           pass correct-pointer to start of
 *                           block, the decoder provides the
 *                           the number of bits it used from
 *                           the input-buffer, which the
 *                           application uses to update its
 *                           stream-status variables.
 *                           For getting this info from the
 *                           decoder, BOOK_KEEP has to be defined
 *                           while compiling decoding routines.
 *
 *                           app_swap_buffers_aac_dec has been simplified,
 *                           now that 'remaining_bytes' has been
 *                           removed. update_bitstream_status() has
 *                           been added - and it is called, after
 *                           every function, which parses the
 *                           bitstream.
 *
 *                           prepare_bitstream, computes number of bytes
 *                           available for any parsing-routine called
 *                           subsequently and also updates the stream
 *                           control variables.
 *
 *                           Since, both the application and the decoder
 *                           will be accessing the same stream in this
 *                           application-example, the following protocol
 *                           is followed:
 *                           (1) call prepare_stream
 *                           (2) Application should call App_bs_readinit
 *                               Decoder should call AACD_bs_readint, which
 *                               it does internally
 *                               This is required, so as to flush the internal
 *                               buffers.
 *                           (3) call update_bitstream_status
 *                               This will reclaim, any unused bytes in the
 *                               stream.
 *
 *                           The variable 'remaining_bytes' has
 *                           been removed. Instead, its value is
 *                           added to bitstream_count.
 *
 *
 *   03/09/2004 04           Removed BitsInBlock, BitCreditType          S.Nachiappan
 *
 *   09/09/2004 05           Added call to AACD_writeout(), when         S.Nachiappan
 *                           bitstream_count becomes negative, so
 *                           that the output file is closed properly
 *
 *   16/09/2004 06           Defined function App_display_error_message  S.Nachiappan
 *                           for printing appropriate error messages
 *                           based on error codes
 *   13/06/2005 07           enabled o/p generation for first 2 frames  Ashok Kumar
 *                           and code for alignment check
 *   02/02/2007 08           Level 4 Warnings removal                   rks05c
 *                           Defect ID: TLSbo89610
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bsacd_dec_interface.h"
//DSPhl28187 Library header files are removed other than the aac_Dec_interface.h file

//DSPhl28081 Removed the header include
//#include "aacd_tables.h"
//#include "aac_fix_proto.h"

//tlsbo89610
#if defined(WINCE_WARNLVL4_DISABLE) && defined(__WINCE)
	// disable some level-4 warnings, use #pragma warning(enable:###) to re-enable
	#pragma warning(disable:4100) // warning C4100: unreferenced formal parameter
	#pragma warning(disable:4201) // warning C4201: nonstandard extension used : nameless struct/union
	#pragma warning(disable:4214) // warning C4201: nonstandard extension used : bit field types other than int
	#pragma warning(disable:4115) // warning C4115: named type definition in parentheses
#endif //#if defined(WINCE_WARNLVL4_DISABLE) && defined(__WINCE)

#define OUT_DUMP
#if defined(TIME_PROFILE) || defined(TIME_PROFILE_RVDS) || defined(DISCARD_OUTPUT)
#undef OUT_DUMP
#endif
#ifdef ALIGNMENT_CHECK
    int cp_ctrl_data = 0;
#endif

#ifdef TIME_PROFILE
#include <sys/time.h>
#endif

#ifdef __WINCE
#include<windows.h>
#endif

#ifdef MEASURE_STACK_USAGE //TLSbo88602
#include "mm_codec_common.h"
#endif


#define BS_BUF_SIZE AACD_INPUT_BUFFER_SIZE
#define LONG_BOUNDARY 4


//DSPhl28187
int AACD_writeout(AACD_OutputFmtType data[][AAC_FRAME_SIZE], AACD_Decoder_Config *dec_config,
                   int* ShouldOpen, int* ShouldClose,char *outpath);

//DSPhl28187
void  myexit(int errornum);

void* aacd_alloc_fast (int );
void* aacd_alloc_slow (int );
char app_swap_buffers_aac_dec (AACD_UCHAR **new_buf_ptr,
                               AACD_UINT32 *new_buf_len,
                               void *global_struct); /* TLSbo84529 */
extern int App_get_adts_header(AACD_Block_Params * params);

void App_display_error_message(AACD_RET_TYPE rc);

#ifdef INTERLEAVE_OUTPUT
AACD_OutputFmtType outbuf[CHANS*AAC_FRAME_SIZE];
#else
AACD_OutputFmtType outbuf[CHANS][AAC_FRAME_SIZE];
#endif

FILE *fin;

#define MAX_ENC_BUF_SIZE 400*BS_BUF_SIZE*24

char          bitstream_buf[MAX_ENC_BUF_SIZE];
int           bitstream_buf_index      = 0;
unsigned int  bitstream_count = 0;

long in_buf_done;

//DSPhl28081 Removed the variable
//void *aptable[58];


void* aacd_alloc_fast (int size);
void* aacd_alloc_slow (int size);
void aacd_free(void* mem);



void          update_bitstream_status(int bytes_used);
int           prepare_bitstream(void);

unsigned long App_bs_look_bits (int nbits);
unsigned long App_bs_read_bits (int nbits);
int           App_bs_byte_align(void);
int           App_bs_refill_buffer(void);
void          App_bs_readinit(char *buf, int bytes);

int           FileType;
int           App_adif_header_present=0;
int           App_adts_header_present=0;
int           App_FindFileType(int val);
int           App_get_prog_config(AACD_ProgConfig * p);
int           App_get_adif_header(AACD_Block_Params * params);
#ifdef  BSAC_DEC
int	      App_get_mp4forbsac_header(AACD_Block_Params * params);
#ifdef  BSAC_DEBUG_DUMP
int	      App_dump_bsac_init(void);
void	      App_dump_bsac_data(int index, int* pbuf, int length);
#endif
#endif



BitstreamParam App_bs_param;
unsigned char  App_ibs_buf[INTERNAL_BS_BUFSIZE];


AACD_Block_Params BlockParams;

int BitsInHeader=0;
int bytes_supplied=0;


int CurrFrame=0;

#ifndef BSAC_DEC
//DSPhl28081 Removed the function definition of "aacd_init_tables();"
/*******************************************************************************
 *
 *   FUNCTION NAME - main
 *
 *   DESCRIPTION
 *       Main AAC decoder routine which calls the initialization function
 *       and then the decoder.
 *
 *   ARGUMENTS
 *       argc - number of arguments passed to the program.
 *       argv - pointer to the arguments.
 *
 *   RETURN VALUE
 *       Exit status of the program.
******************************************************************************/
int main(int argc, char *argv[])
{
    AACD_RET_TYPE rc;
    int Length;

    AACD_Decoder_Config *dec_config;
    AACD_Decoder_info dec_info;
    AACD_Mem_Alloc_Info_Sub   *mem;
    int                       nr;
    int                       rec_no;
    char opath[AACD_PATH_LEN];
    char InFileName[AACD_PATH_LEN];


#ifdef ARM_ADS
#if (!(defined SINGLE_VECTOR)) || (defined TIME_PROFILE_RVDS)
    int TestVectorId = 0;
#endif

#if defined(ARM_ADS) && defined(FLEXIBLE_VECTOR_PATH)
  char in_path[] = "";
  char out_path[] = "";

#else
   #ifdef OLD_MIEL_DIR

   char in_path[] = "../input";
    char out_path[] = "../out";

  #else
 #ifdef __SYMBIAN32__
  char in_path[] = "D:\\bitm\\aac_dec";
    char out_path[] = "D:\\bitm\\aac_dec";

 #else

  char in_path[] = "../../../test_vectors/input";
    char out_path[] = "../output";
  #endif
  #endif
#endif //defined(FLEXIBLE_VECTOR_PATH)



#endif

#ifdef TIME_PROFILE_RVDS
    unsigned int  prev_clk, curr_clk, max_frm, max_clk, min_frm, min_clk, clk,nframe, max_sf;
    unsigned int total_clk=0;
    unsigned char chname[] = "[PROFILE-INFO]";
	extern int prev_cycles(void);
	extern int curr_cycles(void);
    FILE *fp_mhz;
	int sampling_rate = 0;
    int bit_rate = 0;
    int num_chans = 0;
    int len = 0;
#endif

#ifdef TIME_PROFILE
    struct timeval StartTime, EndTime;
    unsigned long TotalDecTimeUs = 0;
    unsigned long prev_DecTimeUs = 0;
    unsigned long cur_DecTimeUs = 0;
    unsigned long min_DecTimeUs = 0;
    int max_frame = 0 ;
    int min_frame = 0;
    unsigned char chname[] = "[PROFILE-INFO]";
    FILE *fp_mhz;
    int sampling_rate = 0;
    int bit_rate = 0;
    int num_chans = 0;
    int len = 0;
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

#ifdef __WINCE         // for taking timing on wince platform
	BOOL Flag;
	LARGE_INTEGER lpFrequency1 ;
	LARGE_INTEGER lpPerformanceCount1;
	LARGE_INTEGER lpPerformanceCount2;
	__int64 TotalDecTime=0;
	__int64 Temp;
    FILE *fp_mhz;
#endif

#ifdef MEASURE_STACK_USAGE //TLSbo88602
    int           *ps32SP, *ps32BaseSP;
    int           s32StackCount, s32StackValue;
    int           s32PeakStack = 0;
#endif

#ifdef MEASURE_HEAP_USAGE
    int      s32TotalMallocBytes=0;
#endif


#ifdef ARM_ADS

#ifdef SINGLE_VECTOR
    char TestVectors[][AACD_PATH_LEN] = {"AL00_8000.adif", "_end_of_vectors_"};
#else /*For running multiple test-vectors */
    char TestVectors[][AACD_PATH_LEN] =
    {
        "AL00_11025.adif",
        "AL00_12000.adif",
        "AL00_16000.adif",
        "AL00_22050.adif",
        "AL00_24000.adif",
        "AL00_32000.adif",
        "AL00_44100.adif",
        "AL00_48000.adif",
        "AL00_8000.adif",
        "AL01_11025.adif",
        "AL01_12000.adif",
        "AL01_16000.adif",
        "AL01_22050.adif",
        "AL01_24000.adif",
        "AL01_32000.adif",
        "AL01_44100.adif",
        "AL01_48000.adif",
        "AL01_8000.adif",
        "AL02_11025.adif",
        "AL02_12000.adif",
        "AL02_16000.adif",
        "AL02_22050.adif",
        "AL02_24000.adif",
        "AL02_32000.adif",
        "AL02_44100.adif",
        "AL02_48000.adif",
        "AL02_8000.adif",
        "AL03_11025.adif",
        "AL03_12000.adif",
        "AL03_16000.adif",
        "AL03_22050.adif",
        "AL03_24000.adif",
        "AL03_32000.adif",
        "AL03_44100.adif",
        "AL03_48000.adif",
        "AL03_8000.adif",
        "AL04_11025.adif",
        "AL04_12000.adif",
        "AL04_16000.adif",
        "AL04_22050.adif",
        "AL04_24000.adif",
        "AL04_32000.adif",
        "AL04_44100.adif",
        "AL04_48000.adif",
        "AL04_8000.adif",
        "AL05_8000.adif",
        "AL05_11025.adif",
        "AL05_12000.adif",
        "AL05_16000.adif",
        "AL05_22050.adif",
        "AL05_24000.adif",
        "AL05_32000.adif",
        "AL05_44100.adif",
        "AL05_48000.adif",
        "AL06_8000.adif",
        "AL06_11025.adif",
        "AL06_12000.adif",
        "AL06_16000.adif",
        "AL06_22050.adif",
        "AL06_24000.adif",
        "AL06_32000.adif",
        "AL06_44100.adif",
        "AL06_48000.adif",
        "AL14_8000.adif",
        "AL14_11025.adif",
        "AL14_12000.adif",
        "AL14_16000.adif",
        "AL14_22050.adif",
        "AL14_24000.adif",
        "AL14_32000.adif",
        "AL14_44100.adif",
        "AL14_48000.adif",
        "AL17_8000.adif",
        "AL17_11025.adif",
        "AL17_12000.adif",
        "AL17_16000.adif",
        "AL17_22050.adif",
        "AL17_24000.adif",
        "AL17_32000.adif",
        "AL17_44100.adif",
        "AL17_48000.adif",
        "AL18_8000.adif",
        "AL18_11025.adif",
        "AL18_12000.adif",
        "AL18_16000.adif",
        "AL18_22050.adif",
        "AL18_24000.adif",
        "AL18_32000.adif",
        "AL18_44100.adif",
        "AL18_48000.adif",
        "AL19_8000.adif",
        "AL19_11025.adif",
        "AL19_12000.adif",
        "AL19_16000.adif",
        "AL19_22050.adif",
        "AL19_24000.adif",
        "AL19_32000.adif",
        "AL19_44100.adif",
        "AL19_48000.adif",
        "SIN2_11025.aac",
        "SIN2_12000.aac",
        "SIN2_16000.aac",
        "SIN2_22050.aac",
        "SIN2_24000.aac",
        "SIN2_32000.aac",
        "SIN2_44100.aac",
        "SIN2_48000.aac",
        "SIN2_8000.aac",
        "L1_11025.aac",
        "L1_12000.aac",
        "L1_16000.aac",
        "L1_22050.aac",
        "L1_24000.aac",
        "L1_32000.aac",
        "L1_44100.aac",
        "L1_48000.aac",
        "L1_8000.aac",
        "L2_11025.aac",
        "L2_12000.aac",
        "L2_16000.aac",
        "L2_22050.aac",
        "L2_24000.aac",
        "L2_32000.aac",
        "L2_44100.aac",
        "L2_48000.aac",
        "L2_8000.aac",
        "L4_11025.aac",
        "L4_12000.aac",
        "L4_16000.aac",
        "L4_22050.aac",
        "L4_24000.aac",
        "L4_32000.aac",
        "L4_44100.aac",
        "L4_48000.aac",
        "L4_8000.aac",
        "L5_11025.aac",
        "L5_12000.aac",
        "L5_16000.aac",
        "L5_22050.aac",
        "L5_24000.aac",
        "L5_32000.aac",
        "L5_44100.aac",
        "L5_48000.aac",
        "L5_8000.aac",
        "AL00_11025.adts",
        "AL00_12000.adts",
        "AL00_16000.adts",
        "AL00_22050.adts",
        "AL00_24000.adts",
        "AL00_32000.adts",
        "AL00_44100.adts",
        "AL00_48000.adts",
        "AL00_8000.adts",
        "AL01_11025.adts",
        "AL01_12000.adts",
        "AL01_16000.adts",
        "AL01_22050.adts",
        "AL01_24000.adts",
        "AL01_32000.adts",
        "AL01_44100.adts",
        "AL01_48000.adts",
        "AL01_8000.adts",
        "AL02_11025.adts",
        "AL02_12000.adts",
        "AL02_16000.adts",
        "AL02_22050.adts",
        "AL02_24000.adts",
        "AL02_32000.adts",
        "AL02_44100.adts",
        "AL02_48000.adts",
        "AL02_8000.adts",
        "AL03_11025.adts",
        "AL03_12000.adts",
        "AL03_16000.adts",
        "AL03_22050.adts",
        "AL03_24000.adts",
        "AL03_32000.adts",
        "AL03_44100.adts",
        "AL03_48000.adts",
        "AL03_8000.adts",
        "AL04_11025.adts",
        "AL04_12000.adts",
        "AL04_16000.adts",
        "AL04_22050.adts",
        "AL04_24000.adts",
        "AL04_32000.adts",
        "AL04_44100.adts",
        "AL04_48000.adts",
        "AL04_8000.adts",
        "AL05_8000.adts",
        "AL05_11025.adts",
        "AL05_12000.adts",
        "AL05_16000.adts",
        "AL05_22050.adts",
        "AL05_24000.adts",
        "AL05_32000.adts",
        "AL05_44100.adts",
        "AL05_48000.adts",
        "AL06_8000.adts",
        "AL06_11025.adts",
        "AL06_12000.adts",
        "AL06_16000.adts",
        "AL06_22050.adts",
        "AL06_24000.adts",
        "AL06_32000.adts",
        "AL06_44100.adts",
        "AL06_48000.adts",
        "AL14_8000.adts",
        "AL14_11025.adts",
        "AL14_12000.adts",
        "AL14_16000.adts",
        "AL14_22050.adts",
        "AL14_24000.adts",
        "AL14_32000.adts",
        "AL14_44100.adts",
        "AL14_48000.adts",
        "AL17_8000.adts",
        "AL17_11025.adts",
        "AL17_12000.adts",
        "AL17_16000.adts",
        "AL17_22050.adts",
        "AL17_24000.adts",
        "AL17_32000.adts",
        "AL17_44100.adts",
        "AL17_48000.adts",
        "AL18_8000.adts",
        "AL18_11025.adts",
        "AL18_12000.adts",
        "AL18_16000.adts",
        "AL18_22050.adts",
        "AL18_24000.adts",
        "AL18_32000.adts",
        "AL18_44100.adts",
        "AL18_48000.adts",
        "AL19_8000.adts",
        "AL19_11025.adts",
        "AL19_12000.adts",
        "AL19_16000.adts",
        "AL19_22050.adts",
        "AL19_24000.adts",
        "AL19_32000.adts",
        "AL19_44100.adts",
        "AL19_48000.adts",
        "_end_of_vectors_"
    };

#endif /*END - SINGLE_VECTOR*/
#endif /*END - ARM_ADS */

    int ShouldOpen; /* For managing file opening by AACD_writeout */
    int ShouldClose; /* For managing file opening by AACD_writeout */

    int  n_bytes;
#ifdef OUT_DUMP
#if INTERLEAVE_OUTPUT || TEST_INTERLEAVE_OUTPUT
	FILE *pfOutput;
#endif /*END - INTERLEAVE_OUTPUT*/
#endif /*END - OUT_DUMP */

    {
	// Output the AAC Decoder Version Info
	printf("%s \n", aacd_decode_versionInfo());
    }

#ifdef ARM_ADS
#ifdef SINGLE_VECTOR
    if (argc > 1) /* command-line arguments were given */
    {
        if (argc > 4)
        {
            printf("Usage : %s  (This runs only the first test-vector defined in TestVectors[]) \n", argv[0]);
            printf("      : %s <infile> <outfile> <output bits per sample>\n", argv[0]);
            exit(1);
        }
    }
#else /* To Run multiple test-vectors */
    if (argc != 1)
    {
        printf("Usage: %s", argv[0]);
        printf("This will run all test vectors at one go\n");
        exit (1);
    }
#endif /*END - SINGLE_VECTOR */

#else
    if (argc < 3)
    {
        printf ("Usage: %s <infile> <outfile> <output bits per sample>\n", argv[0]);
        exit (1);
    }
#endif /*END - ARM_ADS */

    printf("\nsize of struct = %d\n", sizeof(dec_config->aacd_mem_info.mem_info_sub[0].aacd_size));

    dec_config = (AACD_Decoder_Config *) aacd_alloc_fast (sizeof(AACD_Decoder_Config));

    if (dec_config == NULL)
        return 1;
//DSPhl28081 Removed the statement
//dec_config->aacd_initialized_data_start = (void *) aptable;
#ifdef ENABLE_DEBUG_LOG
    aacd_register_debug_funcptr(DebugLogText, DebugLogData);	//DSPhl28187
#endif


    if( aacd_query_dec_mem (dec_config) != AACD_ERROR_NO_ERROR)
    {
        printf("Failed to get the memory configuration for the decoder\n");
        return 1;
    }
//DSPhl28081 Removed the function call
//aacd_init_tables();
    /* Number of memory chunk requests by the decoder */
    nr = dec_config->aacd_mem_info.aacd_num_reqs;

    for(rec_no = 0; rec_no < nr; rec_no++)
    {
        mem = &(dec_config->aacd_mem_info.mem_info_sub[rec_no]);

        if (mem->aacd_type == AACD_FAST_MEMORY)
        {
            mem->app_base_ptr = aacd_alloc_fast (mem->aacd_size);

#ifdef MEASURE_HEAP_USAGE
    s32TotalMallocBytes +=  mem->aacd_size;
#endif


            if (mem->app_base_ptr == NULL)
                return 1;
        }
        else
        {
            mem->app_base_ptr = aacd_alloc_slow (mem->aacd_size);

#ifdef MEASURE_HEAP_USAGE
            s32TotalMallocBytes +=  mem->aacd_size;
#endif
            if (mem->app_base_ptr == NULL)
                return 1;
        }
		memset(dec_config->aacd_mem_info.mem_info_sub[rec_no].app_base_ptr,
			0, dec_config->aacd_mem_info.mem_info_sub[rec_no].aacd_size); //TLSbo74884

    }


#ifdef TIME_PROFILE_RVDS				//tlsbo85204
    fp_mhz = fopen("aac_dec.txt","a");
    if (fp_mhz == NULL)
    {
        printf("Couldn't open cycles file\n");
    }
#endif


#ifdef ARM_ADS
#ifndef SINGLE_VECTOR /* To run multiple test-vectors */
    /* Start loop here for running all tests*/
    TestVectorId = 0;
    for(TestVectorId = 0; ;TestVectorId++)
    {
        if (strcmp(TestVectors[TestVectorId],"_end_of_vectors_")==0)
            break;
        else
        {
#endif /*END - SINGLE_VECTOR */
#endif /*END - ARM _ADS */

#ifdef TIME_PROFILE_RVDS				//tlsbo85204
            max_clk = 0; max_frm = 0;
			min_clk = 0xffffffff; min_frm = 0;
            nframe=0;
#endif
            ShouldOpen = 1;
            ShouldClose = 0;


            bitstream_buf_index = 0;
            bitstream_count = 0;
            bytes_supplied = 0;
            BitsInHeader=0;
            App_adif_header_present = 0;
            App_adts_header_present = 0;

            dec_config->app_swap_buf = app_swap_buffers_aac_dec;

            dec_config->num_pcm_bits     = AACD_16_BIT_OUTPUT;
    		if (argc == 4)
    		{
				if(strcmp(argv[3],"16")==0)
        		{
            		dec_config->num_pcm_bits     = AACD_16_BIT_OUTPUT;
        		}
				else if(strcmp(argv[3],"24")==0)
				{
            		dec_config->num_pcm_bits     = AACD_24_BIT_OUTPUT;
				}
				else
				{
        			printf ("Usage: %s <infile> <outfile> <output bits per sample>\n", argv[0]);
        			printf ("<output bits per sample> is 16 or 24\n");
        			exit (1);
				}
    		}

#ifdef ARM_ADS
#ifdef FLEXIBLE_VECTOR_PATH
            InFileName[0] = '\0';
            opath[0] = '\0';
#else
            InFileName[0] = '\0';
            strcat(InFileName, in_path);
          #ifndef __SYMBIAN32__
            strcat(InFileName, "/");
          #else
          strcat(InFileName, "\\");
          #endif
            opath[0] = '\0';
            strcat(opath, out_path);
          #ifndef __SYMBIAN32__
            strcat(opath, "/");
          #else
            strcat(opath, "\\");
          #endif

#endif //FLEXIBLE_VECTOR_PATH
#ifdef SINGLE_VECTOR
            if (argc == 1) /* No command line arguments were given */
            {
                strcat(InFileName,TestVectors[0]);
                strcat(opath, TestVectors[0]);
            }
            else
            {
                strcat(InFileName, argv[1]);
                strcat(opath, argv[2]);
            }
#else
            strcat(InFileName,TestVectors[TestVectorId]);
            strcat(opath, TestVectors[TestVectorId]);
#endif /*END - SINGLE_VECTOR */
#else

            strcpy(InFileName, argv[1]);
            strcpy(opath, argv[2]);
#endif /* END - ARM_ADS */


            printf(" Initializing decoder..");
            rc = aacd_decoder_init(dec_config);


            printf("Done\n");

            fin = fopen (InFileName, "rb");
            if (fin == NULL)
            {
                printf ("Couldn't open input file %s\n", InFileName);
                exit (1);
            }

#ifdef OUT_DUMP
#if INTERLEAVE_OUTPUT  || TEST_INTERLEAVE_OUTPUT
    		pfOutput = fopen (opath, "wb");
    		if (pfOutput == NULL)
    		{
        		printf ("Couldn't open output file %s\n", opath);
        		return 1;
    		}
#endif
#endif

            in_buf_done = 0;

            if (strlen(opath) + strlen("_f00.pcm") + 1 > AACD_PATH_LEN)
            {
               myexit(AAC_ERROR_FILEIO);  //DSPhl28187
            }

            strcpy(dec_info.output_path,opath);
            fseek(fin, 0, SEEK_END);
            /*get the stream to the buffer*/
            bitstream_count = ftell(fin);

            if (bitstream_count > MAX_ENC_BUF_SIZE)
            {
                printf("Application Error: InputFileSize > InputBufferSize\n");
                printf("Application Error: \nInputFileSize=%d \nInputBufferSize=%d\n", bitstream_count, MAX_ENC_BUF_SIZE);
                exit(1);
            }

            //rewind(fin);
			fseek(fin, 0, SEEK_SET);
            n_bytes = fread(bitstream_buf, 1, bitstream_count, fin);
            if( n_bytes != (int)bitstream_count )           //tlsbo89610
            {
                printf("Bitstream not read correctly\n");
                return 1;
            }


            /******************  Decoding Begins ***************************/

             #ifdef TIME_PROFILE
                #ifdef __SYMBIAN32__
                fp_mhz = fopen("D:\\aac_mhz.txt","a");
                #else
                fp_mhz = fopen("aac_mhz.txt","a");
                #endif
                if (fp_mhz == NULL)
                {
                    printf("Couldn't open cycles file\n");
                }
            #endif

            printf(" Decoding %s\n", InFileName);

            Length = prepare_bitstream();
            App_bs_readinit(bitstream_buf+(bitstream_buf_index-Length), Length);

            FileType = App_bs_look_bits(32);

            if (App_FindFileType(FileType) != 0)
            {
                printf("InputFile is not AAC\n");
                exit(1);
            }

            update_bitstream_status(0);

            if (App_adif_header_present)
            {
                Length = prepare_bitstream();
                App_bs_readinit(bitstream_buf+bitstream_buf_index-Length, Length);
                BitsInHeader = 0;
                App_get_adif_header(&BlockParams);
                dec_config->params = &BlockParams;
                update_bitstream_status(BitsInHeader/8);
				dec_config->adts_format = AACD_FALSE;
            }


            /* Get ADTS-Header if present and start decoding */

            //while(1) //tlsbo89610
			for(;;)
            {
                if (bitstream_count <= 0)
                {
                    ShouldClose = 1;
                    //DSPhl28187
					break;
#ifdef OUT_DUMP
#ifdef INTERLEAVE_OUTPUT
    					if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
    					{
							if (fwrite(outbuf, 3, dec_info.aacd_len*dec_info.aacd_num_channels, pfOutput) != \
								dec_info.aacd_len*dec_info.aacd_num_channels)
							{
								printf("Application File I/O Error\n");
                    			break;
							}
    					}
						else
						{
							if (fwrite(outbuf, 2, dec_info.aacd_len*dec_info.aacd_num_channels, pfOutput) != \
								dec_info.aacd_len*dec_info.aacd_num_channels)
							{
								printf("Application File I/O Error\n");
                    			break;
							}
						}
#else
#if  TEST_INTERLEAVE_OUTPUT
		{
		int i,j;
        if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
        {

		for(i=0;i<dec_info.aacd_len;i++)
		{

			for(j=0;j<CHANS;j++)
			{
			if(*(dec_config->ch_info[j].present))
				{
				fwrite(&outbuf[j][i], 3, 1, pfOutput);
				if(1 == dec_info.aacd_num_channels)
					fwrite(&outbuf[j][i], 3, 1, pfOutput);
				}

			}
		}
	}
		else
	{
		for(i=0;i<dec_info.aacd_len;i++)
		{

			for(j=0;j<CHANS;j++)
			{
					if(*(dec_config->ch_info[j].present))
				{
					fwrite(&outbuf[j][i], 2, 1, pfOutput);
					if(1 == dec_info.aacd_num_channels)
					fwrite(&outbuf[j][i], 2, 1, pfOutput);
				}
			}
		}
	}
		}
#else

		    if (AACD_writeout(outbuf, dec_config, &ShouldOpen, &ShouldClose, opath)!=0)
                    {
                        printf("Application File I/O Error\n");
                        exit(1);
                    }
#endif  //TEST_INTERLEAVE_OUTPUT
#endif
#endif
                    //printf("End of Input\n");
                }

                if (App_adts_header_present)
                {
                    BitsInHeader = 0;
                    Length = prepare_bitstream();
                    App_bs_readinit(bitstream_buf+bitstream_buf_index-Length, Length);
                    App_get_adts_header(&BlockParams);
                    dec_config->params = &BlockParams;
                    update_bitstream_status(BitsInHeader/8);
                    dec_config->adts_format = AACD_TRUE;
                }

                Length = prepare_bitstream();

				/*{
				static int siCnt = 0;
				siCnt++;
				if (siCnt>=10 && siCnt<15) {
				dec_config->packet_loss = AACD_PACKET_LOSS;
				}
			}*/

#ifdef ALIGNMENT_CHECK
                __asm
                {
                    mrc p15, 0, cp_ctrl_data, c1, c0, 0;  /* Read the CP15 control                              register value in
                                                             cp_ctrl_data variable */
                    orr cp_ctrl_data, cp_ctrl_data, 0x2;   /* enable strict                                     alignment check by
                                                              resetting A bit
                                                              [bit number                                       2] of control register */
                    mcr p15, 0, cp_ctrl_data, c1, c0, 0   /* write back to control register */
                        mrc p15, 0, cp_ctrl_data, c1, c0, 0;  /* read control register for verification */
                }
#endif

#ifdef TIME_PROFILE
        gettimeofday(&StartTime, NULL);
#endif

#ifdef __WINCE
       Flag=QueryPerformanceFrequency(&lpFrequency1);
	   Flag=QueryPerformanceCounter(&lpPerformanceCount1);
#endif

#ifdef MEASURE_STACK_USAGE //TLSbo88602
        PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
#endif
#ifdef TIME_PROFILE_RVDS					//tlsbo85204
                prev_clk = prev_cycles();
#endif

                rc = aacd_decode_frame(dec_config,&dec_info,outbuf, bitstream_buf+bitstream_buf_index-Length, Length);
#ifdef TIME_PROFILE_RVDS
                nframe++;
				curr_clk = curr_cycles();

                clk = (curr_clk-prev_clk);               /* clk gives the total core cycles per decode frame call */

                if (clk > max_clk)
                {
                    max_clk = clk;
                    max_frm = nframe;
                    max_sf = dec_info.aacd_sampling_frequency;
                }

				if (clk < min_clk)
				{
					min_clk = clk;
					min_frm = nframe;

				}
				total_clk = total_clk + clk;

                printf ("Current clock = %d\n", curr_clk);
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
#ifdef __WINCE
		Flag=QueryPerformanceCounter(&lpPerformanceCount2);
		Temp=(((lpPerformanceCount2.QuadPart - lpPerformanceCount1.QuadPart)*1000000)/(lpFrequency1.QuadPart));
		TotalDecTime += Temp;
#endif


#ifdef TIME_PROFILE
        gettimeofday(&EndTime, NULL);
        cur_DecTimeUs = (EndTime.tv_usec - StartTime.tv_usec) + (EndTime.tv_sec - StartTime.tv_sec)*1000000L;

        if(cur_DecTimeUs > prev_DecTimeUs)
        {
           prev_DecTimeUs = cur_DecTimeUs ;
           max_frame = *(dec_config->AACD_bno);  //DSPhl28187
        }

        TotalDecTimeUs += cur_DecTimeUs ;

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

#ifdef INTERLEAVE_OUTPUT

					if (1 == dec_info.aacd_num_channels)
					{
						int i;
						char *pcBase = (char *)outbuf, *pcSrc, *pcDst;

    					if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
    					{
							pcSrc = pcBase;
							pcDst = pcBase + dec_info.aacd_len * 3 * 2;
							for (i = 0; i < dec_info.aacd_len; i ++)
							{
								*pcDst++ = *pcSrc++;
								*pcDst++ = *pcSrc++;
								*pcDst++ = *pcSrc++;
							}

							pcSrc = pcBase + dec_info.aacd_len * 3 * 2;
							pcDst = pcBase;
							for (i = 0; i < dec_info.aacd_len; i ++)
							{
								*pcDst++ = *pcSrc++;
								*pcDst++ = *pcSrc++;
								*pcDst++ = *pcSrc++;
								pcSrc -= 3;
								*pcDst++ = *pcSrc++;
								*pcDst++ = *pcSrc++;
								*pcDst++ = *pcSrc++;
							}
    					}
						else
						{
							pcSrc = pcBase;
							pcDst = pcBase + dec_info.aacd_len * 2 * 2;
							for (i = 0; i < dec_info.aacd_len; i ++)
							{
								*pcDst++ = *pcSrc++;
								*pcDst++ = *pcSrc++;
							}

							pcSrc = pcBase + dec_info.aacd_len * 2 * 2;
							pcDst = pcBase;
							for (i = 0; i < dec_info.aacd_len; i ++)
							{
								*pcDst++ = *pcSrc++;
								*pcDst++ = *pcSrc++;
								pcSrc -= 2;
								*pcDst++ = *pcSrc++;
								*pcDst++ = *pcSrc++;
							}
						}
						dec_info.aacd_num_channels = 2;
					}
#endif

                if (rc != AACD_ERROR_NO_ERROR)
                {
                    App_display_error_message(rc);
                    ShouldClose = 1;
                    //DSPhl28187
#ifdef OUT_DUMP
						if (rc == AACD_ERROR_EOF)
						{
#ifdef INTERLEAVE_OUTPUT
    					if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
    					{
							if (fwrite(outbuf, 3, dec_info.aacd_len*dec_info.aacd_num_channels, pfOutput) != \
								dec_info.aacd_len*dec_info.aacd_num_channels)
							{
								printf("Application File I/O Error\n");
                    			break;
							}
    					}
						else
						{
							if (fwrite(outbuf, 2, dec_info.aacd_len*dec_info.aacd_num_channels, pfOutput) != \
								dec_info.aacd_len*dec_info.aacd_num_channels)
							{
								printf("Application File I/O Error\n");
                    			break;
							}
						}
#else
#if  TEST_INTERLEAVE_OUTPUT
		{
		int i,j;
        if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
        {

		for(i=0;i<dec_info.aacd_len;i++)
		{

			for(j=0;j<CHANS;j++)
			{
			if(*(dec_config->ch_info[j].present))
				{
				fwrite(&outbuf[j][i], 3, 1, pfOutput);
				if(1 == dec_info.aacd_num_channels)
					fwrite(&outbuf[j][i], 3, 1, pfOutput);
				}

			}
		}
	}
		else
	{
		for(i=0;i<dec_info.aacd_len;i++)
		{

			for(j=0;j<CHANS;j++)
			{
					if(*(dec_config->ch_info[j].present))
				{
					fwrite(&outbuf[j][i], 2, 1, pfOutput);
					if(1 == dec_info.aacd_num_channels)
					fwrite(&outbuf[j][i], 2, 1, pfOutput);
				}
			}
		}
	}
		}
#else

		    if (AACD_writeout(outbuf, dec_config, &ShouldOpen, &ShouldClose, opath)!=0)
                    {
                        printf("Application File I/O Error\n");
                        exit(1);
                    }
#endif  //TEST_INTERLEAVE_OUTPUT
#endif
						}

#endif

					if (rc == AACD_ERROR_EOF)
						break;
                }

                update_bitstream_status(dec_info.BitsInBlock/8);

#ifndef CT_CONF
                if (*(dec_config->AACD_bno) > 3)        //DSPhl28187
#else
					if (*(dec_config->AACD_bno) > 1)		//DSPhl28187
#endif
                {
                    //ptr->out_ptr = opath; //TLSbo63933
                    //DSPhl28187
#ifdef OUT_DUMP
#ifdef INTERLEAVE_OUTPUT

    					if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
    					{
							if (fwrite(outbuf, 3, dec_info.aacd_len*dec_info.aacd_num_channels, pfOutput) != \
								dec_info.aacd_len*dec_info.aacd_num_channels)
							{
								printf("Application File I/O Error\n");
                    			break;
							}
    					}
						else
						{
							if (fwrite(outbuf, 2, dec_info.aacd_len*dec_info.aacd_num_channels, pfOutput) != \
								dec_info.aacd_len*dec_info.aacd_num_channels)
							{
								printf("Application File I/O Error\n");
                    			break;
							}
						}
#else
#if  TEST_INTERLEAVE_OUTPUT
		{
		int i,j;
        if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
        {

		for(i=0;i<dec_info.aacd_len;i++)
		{

			for(j=0;j<CHANS;j++)
			{
			if(*(dec_config->ch_info[j].present))
				{
				fwrite(&outbuf[j][i], 3, 1, pfOutput);
				if(1 == dec_info.aacd_num_channels)
					fwrite(&outbuf[j][i], 3, 1, pfOutput);
				}

			}
		}
	}
		else
	{
		for(i=0;i<dec_info.aacd_len;i++)
		{

			for(j=0;j<CHANS;j++)
			{
					if(*(dec_config->ch_info[j].present))
				{
					fwrite(&outbuf[j][i], 2, 1, pfOutput);
					if(1 == dec_info.aacd_num_channels)
					fwrite(&outbuf[j][i], 2, 1, pfOutput);
				}
			}
		}
	}
		}
#else

		    if (AACD_writeout(outbuf, dec_config, &ShouldOpen, &ShouldClose, opath)!=0)
                    {
                        printf("Application File I/O Error\n");
                        exit(1);
                    }
#endif  //TEST_INTERLEAVE_OUTPUT
#endif
#endif
                }

                if (rc == AACD_ERROR_EOF)
                {
                    App_display_error_message(rc);
                    break;
                }
                //DSPhl28187
#ifndef CT_CONF
					if (*(dec_config->AACD_bno) > 3)
#else
						if (*(dec_config->AACD_bno) > 1)		//DSPhl28187
#endif
						{
							if (*(dec_config->AACD_bno) == 4)   //DSPhl28187
                    {
                        // To get the parameters in Log file

                        #ifdef TIME_PROFILE
                        sampling_rate = dec_info.aacd_sampling_frequency;
                        bit_rate = dec_info.aacd_bit_rate;
                        num_chans = dec_info.aacd_num_channels;
                        len = dec_info.aacd_len;
                        #endif

#ifdef TIME_PROFILE_RVDS
		sampling_rate = dec_info.aacd_sampling_frequency;
		bit_rate = dec_info.aacd_bit_rate;
		num_chans = dec_info.aacd_num_channels;
		len = dec_info.aacd_len;
#endif


#ifdef TIME_PROFILE
                       /* fprintf (fp_mhz,"sampling_rate = %d\n", dec_info.aacd_sampling_frequency);
                        fprintf (fp_mhz,"bit_rate = %d\n", dec_info.aacd_bit_rate);
                        fprintf (fp_mhz,"num_chans = %d\n", dec_info.aacd_num_channels);
                        fprintf (fp_mhz,"len = %d\n", dec_info.aacd_len);*/

#endif
                    }

#ifndef TIME_PROFILE
#ifndef __WINCE
                    printf ("\r[%3d]\n", (int) *(dec_config->AACD_bno));  //DSPhl28187
#endif
#endif
                    fflush (stdout);
                }
              //  printf ("Frame No = [%3d]\n", (int) ptr->AACD_bno);
                fflush (stdout);
            } /*end-while*/

#ifdef TIME_PROFILE_RVDS
		{

            //den = (double)(1024.0 * 1000000.0);
            //curr_mhz = (double)((double)(64.0*max_clk*max_sf)/den);

			int	count=0,n=0,ch=0,tot_char=0;
			char fileName[300]="";
			char file[100]="";

			strcpy(fileName,InFileName);
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
#ifdef MEASURE_STACK_USAGE
#ifdef MEASURE_HEAP_USAGE

			printf("\n%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t",chname,file,sampling_rate,bit_rate,\
 			num_chans,max_clk,min_clk,nframe,max_frm,min_frm,total_clk,s32PeakStack,s32TotalMallocBytes);
			fprintf(fp_mhz, "\n%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t",chname,file,sampling_rate,bit_rate,\
 			num_chans,max_clk,min_clk,nframe,max_frm,min_frm,total_clk,s32PeakStack,s32TotalMallocBytes);

#endif
#endif

#ifndef MEASURE_STACK_USAGE
#ifndef MEASURE_HEAP_USAGE

			printf("\n%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t",chname,file,sampling_rate,bit_rate,\
 			num_chans,max_clk,min_clk,nframe,max_frm,min_frm,total_clk);
			fprintf(fp_mhz, "\n%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t",chname,file,sampling_rate,bit_rate,\
 			num_chans,max_clk,min_clk,nframe,max_frm,min_frm,total_clk);

#endif
#endif


		}
#endif

#ifdef TIME_PROFILE
	printf("\n%s\t%s\t%d\t%d\t%d\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname,InFileName,sampling_rate,bit_rate,\
 	        num_chans,prev_DecTimeUs,min_DecTimeUs,(int) *(dec_config->AACD_bno),max_frame,min_frame,TotalDecTimeUs);
 	fprintf(fp_mhz, "\n%s\t%s\t%d\t%d\t%d\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname,InFileName,sampling_rate,bit_rate,
 	        num_chans,prev_DecTimeUs,min_DecTimeUs,(int) *(dec_config->AACD_bno),max_frame,min_frame,TotalDecTimeUs);
    /*fprintf(fp_mhz,"Total Decode Time [microseconds] = %ld\n", TotalDecTimeUs);
    fprintf (fp_mhz,"Total frames = %3d\n", (int) *(dec_config->AACD_bno));  //DSPhl28187
    fprintf (fp_mhz,"Peak time = %ld\n", prev_DecTimeUs);*/
    fclose(fp_mhz);
#endif
#ifdef __SYMBIAN32__
#ifdef MEASURE_HEAP_USAGE
			printf("\n Stack and Heap are  %d\t%d\t",s32PeakStack,s32TotalMallocBytes);
#endif
#endif


            fclose(fin);

#ifdef OUT_DUMP
#ifdef INTERLEAVE_OUTPUT
			if (NULL != pfOutput)
			{
				fclose(pfOutput);
			}
#endif
#endif

#ifdef ARM_ADS
#ifndef SINGLE_VECTOR
        }
    }
#endif
#endif

#ifdef MHZ_MEASUREMENT
    fclose(fp_mhz);
#endif

    for(rec_no = 0; rec_no < nr; rec_no++)
    {
        mem = &(dec_config->aacd_mem_info.mem_info_sub[rec_no]);

        if (mem->app_base_ptr)
        {
            aacd_free(mem->app_base_ptr);
            mem->app_base_ptr = 0;
        }
    }
    aacd_free(dec_config);

#ifdef __WINCE
		fp_mhz = fopen("aac_dec.txt","a");

		_ftprintf(fp_mhz,TEXT("\n%s\t%I64d\t"),InFileName, TotalDecTime);

		fclose(fp_mhz);

#endif
    return 0;
}

#else /* is BSAC_DEC*/

/************************************************************************************
    used for BSAC DECODER


***********************************************************************************/
int main(int argc, char *argv[])
{
    AACD_RET_TYPE rc;
    int Length;

    AACD_Decoder_Config *dec_config;
    AACD_Decoder_info dec_info;
    AACD_Mem_Alloc_Info_Sub   *mem;
    int                       nr;
    int                       rec_no;
    char opath[AACD_PATH_LEN];
    char InFileName[AACD_PATH_LEN];


#ifdef TIME_PROFILE_RVDS
    unsigned int  prev_clk, curr_clk, max_frm, max_clk, min_frm, min_clk, clk,nframe, max_sf;
    unsigned int total_clk=0;
    unsigned char chname[] = "[PROFILE-INFO]";
    extern int prev_cycles(void);
    extern int curr_cycles(void);
    FILE *fp_mhz;
    int sampling_rate = 0;
    int bit_rate = 0;
    int num_chans = 0;
    int len = 0;
#endif

#ifdef TIME_PROFILE
    struct timeval StartTime, EndTime;
    unsigned long TotalDecTimeUs = 0;
    unsigned long prev_DecTimeUs = 0;
    unsigned long cur_DecTimeUs = 0;
    unsigned long min_DecTimeUs = 0;
    int max_frame = 0 ;
    int min_frame = 0;
    unsigned char chname[] = "[PROFILE-INFO]";
    FILE *fp_mhz;
    int sampling_rate = 0;
    int bit_rate = 0;
    int num_chans = 0;
    int len = 0;
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

#ifdef __WINCE         // for taking timing on wince platform
	BOOL Flag;
	LARGE_INTEGER lpFrequency1 ;
	LARGE_INTEGER lpPerformanceCount1;
	LARGE_INTEGER lpPerformanceCount2;
    int  TotalDecTimeUs = 0;
    int  prev_DecTimeUs = 0;
    int  cur_DecTimeUs = 0;
    int  min_DecTimeUs = 0;
    int max_frame = 0 ;
    int min_frame = 0;
    unsigned char chname[] = "[PROFILE-INFO]";
    FILE *fp_mhz;
    int sampling_rate = 0;
    int bit_rate = 0;
    int num_chans = 0;
    int len = 0;
#endif

#ifdef MEASURE_STACK_USAGE //TLSbo88602
    int           *ps32SP, *ps32BaseSP;
    int           s32StackCount, s32StackValue;
    int           s32PeakStack = 0;
#endif

#ifdef MEASURE_HEAP_USAGE
    int      s32TotalMallocBytes=0;
#endif


    int ShouldOpen; /* For managing file opening by AACD_writeout */
    int ShouldClose; /* For managing file opening by AACD_writeout */

    int  n_bytes;
#ifdef OUT_DUMP
//#ifdef INTERLEAVE_OUTPUT
	FILE *pfOutput;
//#endif /*END - INTERLEAVE_OUTPUT*/
#endif /*END - OUT_DUMP */

    if (argc < 3)
    {
        printf ("Usage: %s <infile> <outfile> <output bits per sample>\n", argv[0]);
        exit (1);
    }

    printf("\nsize of struct = %d\n", sizeof(dec_config->aacd_mem_info.mem_info_sub[0].aacd_size));

    dec_config = (AACD_Decoder_Config *) aacd_alloc_fast (sizeof(AACD_Decoder_Config));

    if (dec_config == NULL)
        return 1;
//DSPhl28081 Removed the statement
//dec_config->aacd_initialized_data_start = (void *) aptable;
#ifdef ENABLE_DEBUG_LOG
    aacd_register_debug_funcptr(DebugLogText, DebugLogData);	//DSPhl28187
#endif
    if( aacd_query_dec_mem (dec_config) != AACD_ERROR_NO_ERROR)
    {
        printf("Failed to get the memory configuration for the decoder\n");
        return 1;
    }
//DSPhl28081 Removed the function call
//aacd_init_tables();
    /* Number of memory chunk requests by the decoder */
    nr = dec_config->aacd_mem_info.aacd_num_reqs;

    for(rec_no = 0; rec_no < nr; rec_no++)
    {
        mem = &(dec_config->aacd_mem_info.mem_info_sub[rec_no]);

        if (mem->aacd_type == AACD_FAST_MEMORY)
        {
            mem->app_base_ptr = aacd_alloc_fast (mem->aacd_size);

#ifdef MEASURE_HEAP_USAGE
    s32TotalMallocBytes +=  mem->aacd_size;
#endif


            if (mem->app_base_ptr == NULL)
                return 1;
        }
        else
        {
            mem->app_base_ptr = aacd_alloc_slow (mem->aacd_size);

#ifdef MEASURE_HEAP_USAGE
            s32TotalMallocBytes +=  mem->aacd_size;
#endif
            if (mem->app_base_ptr == NULL)
                return 1;
        }
		memset(dec_config->aacd_mem_info.mem_info_sub[rec_no].app_base_ptr,
			0, dec_config->aacd_mem_info.mem_info_sub[rec_no].aacd_size); //TLSbo74884

    }


#ifdef TIME_PROFILE_RVDS				//tlsbo85204
    fp_mhz = fopen("bsac_dec.txt","a");
    if (fp_mhz == NULL)
    {
        printf("Couldn't open cycles file\n");
    }
#endif




#ifdef TIME_PROFILE_RVDS				//tlsbo85204
            max_clk = 0; max_frm = 0;
			min_clk = 0xffffffff; min_frm = 0;
            nframe=0;
#endif
            ShouldOpen = 1;
            ShouldClose = 0;


            bitstream_buf_index = 0;
            bitstream_count = 0;
            bytes_supplied = 0;
            BitsInHeader=0;
            App_adif_header_present = 0;
            App_adts_header_present = 0;

            dec_config->app_swap_buf = app_swap_buffers_aac_dec;
            dec_config->num_pcm_bits     = AACD_16_BIT_OUTPUT;
	    if (argc == 4)
	    {
		if(strcmp(argv[3],"16")==0)
		{
		    dec_config->num_pcm_bits     = AACD_16_BIT_OUTPUT;
		}
		else if(strcmp(argv[3],"24")==0)
		{
		    dec_config->num_pcm_bits     = AACD_24_BIT_OUTPUT;
		}
		else
		{
		    printf ("Usage: %s <infile> <outfile> <output bits per sample>\n", argv[0]);
		    printf ("<output bits per sample> is 16 or 24\n");
		    exit (1);
		}
	    }


            strcpy(InFileName, argv[1]);
            strcpy(opath, argv[2]);


            printf(" Initializing decoder..");
            rc = aacd_decoder_init(dec_config);


            printf("Done\n");

            fin = fopen (InFileName, "rb");
            if (fin == NULL)
            {
                printf ("Couldn't open input file %s\n", InFileName);
                exit (1);
            }

#ifdef OUT_DUMP
//#ifdef INTERLEAVE_OUTPUT
    		pfOutput = fopen (opath, "wb");
    		if (pfOutput == NULL)
    		{
        		printf ("Couldn't open output file %s\n", opath);
        		return 1;
    		}
//#endif
#endif

            in_buf_done = 0;

            if (strlen(opath) + strlen("_f00.pcm") + 1 > AACD_PATH_LEN)
            {
               myexit(AAC_ERROR_FILEIO);  //DSPhl28187
            }

            strcpy(dec_info.output_path,opath);
            fseek(fin, 0, SEEK_END);
            /*get the stream to the buffer*/
            bitstream_count = ftell(fin);

            if (bitstream_count > MAX_ENC_BUF_SIZE)
            {
                printf("Application Error: InputFileSize > InputBufferSize\n");
                printf("Application Error: \nInputFileSize=%d \nInputBufferSize=%d\n", bitstream_count, MAX_ENC_BUF_SIZE);
                exit(1);
            }

            //rewind(fin);
            fseek(fin, 0, SEEK_SET);
            n_bytes = fread(bitstream_buf, 1, bitstream_count, fin);
            if( n_bytes != (int)bitstream_count )           //tlsbo89610
            {
                printf("Bitstream not read correctly\n");
                return 1;
            }


            /******************  Decoding Begins ***************************/

#ifdef TIME_PROFILE
#ifdef __SYMBIAN32__
          fp_mhz = fopen("D:\\aac_mhz.txt","a");
#else
          fp_mhz = fopen("../audio_performance.txt","a");
#endif
          if (fp_mhz == NULL)
          {
                printf("Couldn't open cycles file\n");
          }
          fprintf (fp_mhz,"\n|\tAudio Format\t|\tPlat\t|\tSamplerate\t|\tCh\t|\tBiterate\t|\tBit\t|\tPerf(MIPS)\t|\tAudio file name\n");
#endif

#ifdef __WINCE
          fp_mhz = fopen("aac_mhz_wince.txt","a");
          if (fp_mhz == NULL)
          {
                printf("Couldn't open cycles file\n");
          }
#endif
            printf(" Decoding %s\n", InFileName);

            Length = prepare_bitstream();
            App_bs_readinit(bitstream_buf+(bitstream_buf_index-Length), Length);

            BitsInHeader = 0;
            App_get_mp4forbsac_header(&BlockParams);
            dec_config->params = &BlockParams;
            update_bitstream_status(BitsInHeader/8);

	    BlockParams.num_pce = 0;
	    BlockParams.iMulti_Channel_Support= 0;
	    BlockParams.bsacDecLayer= -1;

#ifdef BSAC_DEBUG_DUMP
	    App_dump_bsac_init();
	    dec_config->dump_bsac_data =  App_dump_bsac_data;
#endif
            /* Get ADTS-Header if present and start decoding */

            //while(1) //tlsbo89610
            for(;;)
            {
                if (bitstream_count <= 0)
                {
                    ShouldClose = 1;
                    break;

                    //DSPhl28187
#ifdef OUT_DUMP
#ifdef INTERLEAVE_OUTPUT
    					if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
    					{
							if (fwrite(outbuf, 3, dec_info.aacd_len*dec_info.aacd_num_channels, pfOutput) != \
								dec_info.aacd_len*dec_info.aacd_num_channels)
							{
								printf("Application File I/O Error\n");
                    			break;
							}
    					}
						else
						{
							if (fwrite(outbuf, 2, dec_info.aacd_len*dec_info.aacd_num_channels, pfOutput) != \
								dec_info.aacd_len*dec_info.aacd_num_channels)
							{
								printf("Application File I/O Error\n");
                    			break;
							}
						}
#else
                    if (AACD_writeout(outbuf, dec_config, &ShouldOpen, &ShouldClose, opath)!=0)
                    {
                        printf("Application File I/O Error\n");
                        exit(1);
                    }
#endif
#endif
                    //printf("End of Input\n");
                }


                Length = prepare_bitstream();


#ifdef ALIGNMENT_CHECK
                __asm
                {
                    mrc p15, 0, cp_ctrl_data, c1, c0, 0;  /* Read the CP15 control                              register value in
                                                             cp_ctrl_data variable */
                    orr cp_ctrl_data, cp_ctrl_data, 0x2;   /* enable strict                                     alignment check by
                                                              resetting A bit
                                                              [bit number                                       2] of control register */
                    mcr p15, 0, cp_ctrl_data, c1, c0, 0   /* write back to control register */
                        mrc p15, 0, cp_ctrl_data, c1, c0, 0;  /* read control register for verification */
                }
#endif

#ifdef TIME_PROFILE
        gettimeofday(&StartTime, NULL);
#endif

#ifdef __WINCE
       INTERRUPTS_SET(0xDF);  /*disable interrupt*/
       Flag=QueryPerformanceFrequency(&lpFrequency1);
	   Flag=QueryPerformanceCounter(&lpPerformanceCount1);
#endif

#ifdef MEASURE_STACK_USAGE //TLSbo88602
        PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
#endif
#ifdef TIME_PROFILE_RVDS					//tlsbo85204
                prev_clk = prev_cycles();
#endif

                rc = aacd_decode_frame(dec_config,&dec_info,outbuf, bitstream_buf+bitstream_buf_index-Length, Length);
#ifdef TIME_PROFILE_RVDS
                nframe++;
		curr_clk = curr_cycles();

                clk = (curr_clk-prev_clk);               /* clk gives the total core cycles per decode frame call */

                if (clk > max_clk)
                {
                    max_clk = clk;
                    max_frm = nframe;
                    max_sf = dec_info.aacd_sampling_frequency;
                }

		if (clk < min_clk)
		{
		    min_clk = clk;
		    min_frm = nframe;

		}
		total_clk = total_clk + clk;

                printf ("Current clock = %d\n", curr_clk);
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
#ifdef __WINCE
	Flag=QueryPerformanceCounter(&lpPerformanceCount2);
	cur_DecTimeUs=(((lpPerformanceCount2.QuadPart - lpPerformanceCount1.QuadPart)*1000000)/(lpFrequency1.QuadPart));
        INTERRUPTS_SET(0x1F) ;  /*enable interrupt*/
	TotalDecTimeUs = TotalDecTimeUs + cur_DecTimeUs;
	if(cur_DecTimeUs > prev_DecTimeUs)
	{
	    prev_DecTimeUs = cur_DecTimeUs;
	    max_frame = *(dec_config->AACD_bno);
	}

#endif


#ifdef TIME_PROFILE
        gettimeofday(&EndTime, NULL);
        cur_DecTimeUs = (EndTime.tv_usec - StartTime.tv_usec) + (EndTime.tv_sec - StartTime.tv_sec)*1000000L;

        if(cur_DecTimeUs > prev_DecTimeUs)
        {
           prev_DecTimeUs = cur_DecTimeUs ;
           max_frame = *(dec_config->AACD_bno);  //DSPhl28187
        }

        TotalDecTimeUs += cur_DecTimeUs ;

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




	if (rc != AACD_ERROR_NO_ERROR)
	{
	    App_display_error_message(rc);
	    ShouldClose = 1;
	    //DSPhl28187
#ifdef OUT_DUMP
	    if (rc == AACD_ERROR_EOF)
	    {
#ifdef INTERLEAVE_OUTPUT
		if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
		{
		    if (fwrite(outbuf, 3, dec_info.aacd_len*dec_info.aacd_num_channels, pfOutput) != \
			dec_info.aacd_len*dec_info.aacd_num_channels)
		    {
			printf("Application File I/O Error\n");
			break;
		    }
		}
		else
		{
		    if (fwrite(outbuf, 2, dec_info.aacd_len*dec_info.aacd_num_channels, pfOutput) != \
			dec_info.aacd_len*dec_info.aacd_num_channels)
		    {
			printf("Application File I/O Error\n");
			break;
		    }
		}
#else
		if (AACD_writeout(outbuf, dec_config, &ShouldOpen, &ShouldClose, opath)!=0)
		{
		    printf("Application File I/O Error\n");
		    exit(1);
		}
#endif
	    }

#endif

	    break;
	}

	update_bitstream_status(dec_info.BitsInBlock/8);

#ifndef CT_CONF
	if (*(dec_config->AACD_bno) > 0)        //DSPhl28187
#endif
	{
	    //ptr->out_ptr = opath; //TLSbo63933
	    //DSPhl28187
#ifdef OUT_DUMP
#ifdef INTERLEAVE_OUTPUT
	    if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
	    {
		if (fwrite(outbuf, 3, dec_info.aacd_len*dec_info.aacd_num_channels, pfOutput) != \
		    dec_info.aacd_len*dec_info.aacd_num_channels)
		{
		    printf("Application File I/O Error\n");
		    break;
		}
	    }
	    else
	    {
		if (fwrite(outbuf, 2, dec_info.aacd_len*dec_info.aacd_num_channels, pfOutput) != \
		    dec_info.aacd_len*dec_info.aacd_num_channels)
		{
		    printf("Application File I/O Error\n");
		    break;
		}
	    }
#else
#ifdef  TEST_INTERLEAVE_OUTPUT
		{
		int i,j;
        if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
        {

		for(i=0;i<dec_info.aacd_len;i++)
			for(j=0;j<dec_info.aacd_num_channels;j++)
			{
				fwrite(&outbuf[j][i], 3, 1, pfOutput);

			}

		}
		else
		{
			for(i=0;i<dec_info.aacd_len;i++)
				for(j=0;j<dec_info.aacd_num_channels;j++)
				{
					fwrite(&outbuf[j][i], 2, 1, pfOutput);

				}
		}
		}
#else
	    if (AACD_writeout(outbuf, dec_config, &ShouldOpen, &ShouldClose, opath)!=0)
	    {
		printf("Application File I/O Error\n");
		exit(1);
	    }
#endif  // BSAC_DEC
#endif
#endif
	}

	if (rc == AACD_ERROR_EOF)
	{
	    App_display_error_message(rc);
	    break;
	}
	//DSPhl28187
#ifndef CT_CONF
	if (*(dec_config->AACD_bno) > 2)
#endif
	{
	    if (*(dec_config->AACD_bno) == 3)   //DSPhl28187
	    {
		// To get the parameters in Log file

#ifdef TIME_PROFILE
		sampling_rate = dec_info.aacd_sampling_frequency;
		bit_rate = dec_info.aacd_bit_rate;
		num_chans = dec_info.aacd_num_channels;
		len = dec_info.aacd_len;
#endif

#ifdef TIME_PROFILE_RVDS
		sampling_rate = dec_info.aacd_sampling_frequency;
		bit_rate = dec_info.aacd_bit_rate;
		num_chans = dec_info.aacd_num_channels;
		len = dec_info.aacd_len;
#endif


#ifdef TIME_PROFILE
		/* fprintf (fp_mhz,"sampling_rate = %d\n", dec_info.aacd_sampling_frequency);
		fprintf (fp_mhz,"bit_rate = %d\n", dec_info.aacd_bit_rate);
		fprintf (fp_mhz,"num_chans = %d\n", dec_info.aacd_num_channels);
		fprintf (fp_mhz,"len = %d\n", dec_info.aacd_len);*/

#endif
#ifdef __WINCE
		sampling_rate = dec_info.aacd_sampling_frequency;
		bit_rate = dec_info.aacd_bit_rate;
		num_chans = dec_info.aacd_num_channels;
		len = dec_info.aacd_len;

#endif
	    }

#ifndef TIME_PROFILE
	    printf ("\r[%3d]\n", (int) *(dec_config->AACD_bno));  //DSPhl28187
#endif
	    fflush (stdout);
	}
	//  printf ("Frame No = [%3d]\n", (int) ptr->AACD_bno);
	fflush (stdout);
            } /*end-while*/

#ifdef TIME_PROFILE_RVDS
		{

		    //den = (double)(1024.0 * 1000000.0);
		    //curr_mhz = (double)((double)(64.0*max_clk*max_sf)/den);

		    int	count=0,n=0,ch=0,tot_char=0;
		    char fileName[300]="";
		    char file[100]="";

		    strcpy(fileName,InFileName);
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
#ifdef MEASURE_STACK_USAGE
#ifdef MEASURE_HEAP_USAGE

		    printf("\n%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t",chname,file,sampling_rate,bit_rate,\
			num_chans,max_clk,min_clk,nframe,max_frm,min_frm,total_clk,s32PeakStack,s32TotalMallocBytes);
		    fprintf(fp_mhz, "\n%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t",chname,file,sampling_rate,bit_rate,\
			num_chans,max_clk,min_clk,nframe,max_frm,min_frm,total_clk,s32PeakStack,s32TotalMallocBytes);

#endif
#endif

#ifndef MEASURE_STACK_USAGE
#ifndef MEASURE_HEAP_USAGE

		    printf("\n%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t",chname,file,sampling_rate,bit_rate,\
			num_chans,max_clk,min_clk,nframe,max_frm,min_frm,total_clk);
		    fprintf(fp_mhz, "\n%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t",chname,file,sampling_rate,bit_rate,\
			num_chans,max_clk,min_clk,nframe,max_frm,min_frm,total_clk);

#endif
#endif


		}
#endif

#ifdef TIME_PROFILE
		printf("\n%s\t%s\t%d\t%d\t%d\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname,InFileName,sampling_rate,bit_rate,\
		    num_chans,prev_DecTimeUs,min_DecTimeUs,(int) *(dec_config->AACD_bno),max_frame,min_frame,TotalDecTimeUs);
//		fprintf(fp_mhz, "\n%s\t%s\t%d\t%d\t%d\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname,InFileName,sampling_rate,bit_rate,
//		    num_chans,prev_DecTimeUs,min_DecTimeUs,(int) *(dec_config->AACD_bno),max_frame,min_frame,TotalDecTimeUs);
		    /*fprintf(fp_mhz,"Total Decode Time [microseconds] = %ld\n", TotalDecTimeUs);
//		    fprintf (fp_mhz,"Total frames = %3d\n", (int) *(dec_config->AACD_bno));  //DSPhl28187
//		fprintf (fp_mhz,"Peak time = %ld\n", prev_DecTimeUs);*/
	       performance = (double)TotalDecTimeUs * 0.000001*cpu_freq*sampling_rate/(int) *(dec_config->AACD_bno)/1024;
              fprintf(fp_mhz, "|\tBSAC Decoder\t|\tARM%d\t|\t%9d\t|\t%d\t|\t%8d\t|\t%d\t|\t%.8f\t|%s\n", platform, sampling_rate, num_chans, bit_rate, bit_len, performance, InFileName);

		fclose(fp_mhz);
#endif
#ifdef __SYMBIAN32__
#ifdef MEASURE_HEAP_USAGE
		printf("\n Stack and Heap are  %d\t%d\t",s32PeakStack,s32TotalMallocBytes);
#endif
#endif

#ifdef __WINCE
		printf("\n%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t",chname,InFileName,sampling_rate,bit_rate,\
		    num_chans,prev_DecTimeUs,min_DecTimeUs,(int) *(dec_config->AACD_bno),max_frame,min_frame,TotalDecTimeUs);
		fprintf(fp_mhz, "\n%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t",chname,InFileName,sampling_rate,bit_rate,
		    num_chans,prev_DecTimeUs,min_DecTimeUs,(int) *(dec_config->AACD_bno),max_frame,min_frame,TotalDecTimeUs);
		fclose(fp_mhz);
#endif

		fclose(fin);

#ifdef OUT_DUMP
#ifdef INTERLEAVE_OUTPUT
		if (NULL != pfOutput)
		{
		    fclose(pfOutput);
		}
#endif
#endif



#ifdef MHZ_MEASUREMENT
		fclose(fp_mhz);
#endif

		for(rec_no = 0; rec_no < nr; rec_no++)
		{
		    mem = &(dec_config->aacd_mem_info.mem_info_sub[rec_no]);

		    if (mem->app_base_ptr)
		    {
			aacd_free(mem->app_base_ptr);
			mem->app_base_ptr = 0;
		    }
		}
		aacd_free(dec_config);

		return 0;
}




#endif /* BSAC_DEC*/




/***************************************************************************
 *
 *   FUNCTION NAME - app_swap_buffers_aac_dec
 *
 *   DESCRIPTION
 *       This function is a callback by instance of AACD decoder requesting
 *       the application for a new input buffer. It returns the used buffer
 *       pointer to the application and expects a new buffer pointer and
 *       buffer length in return. If buffer is not available it returns
 *       a NULL pointer with zero length to the AACD decoder instance. This
 *       is an example code.
 *
 *       The variable bytes_supplied keeps track of the total number
 *       of bytes used by the decoder, since the last call to the decoder.
 *
 *       Also see, update_bitstream_status() function below.
 *
 *   ARGUMENTS
 *       new_buf_ptr       - Pointer to pointer of used buffer
 *       new_buf_len       - Pointer to the length of the input buffer var
 *       instance_id       - AACD decoder Instance ID
 *
 *   RETURN VALUE
 *          0              - If buffer allocation is successfull.
 *         -1              - Indicates 'End of Bitstream'.
 *
 ***************************************************************************/

AACD_INT8 app_swap_buffers_aac_dec (AACD_UCHAR **new_buf_ptr,
                               AACD_UINT32 *new_buf_len,
                               void *global_struct) /* TLSbo84529 */
{

  int len;

  if (bitstream_count <= 0)
    {
      *new_buf_ptr = NULL;
      *new_buf_len = 0;
      return (AACD_INT8)-1;
    }
  else
    {
      len = (bitstream_count > BS_BUF_SIZE) ? BS_BUF_SIZE : bitstream_count;
      *new_buf_len = len;
      *new_buf_ptr = (AACD_UCHAR*)(bitstream_buf + bitstream_buf_index);
      bitstream_buf_index += len;
      bitstream_count -= len;
      in_buf_done    += len;
      bytes_supplied += len;
    }
  return(0);
}


/***************************************************************************
 *
 *   FUNCTION NAME - update_bitstream_status
 *
 *   DESCRIPTION
 *       This function accepts the number of bytes used by the decoder
 *       since the last call to the decoder and updates
 *       the bitstream variables, so that any parsing-function will
 *       start reading from the first unused-byte in the stream.
 *
 *   ARGUMENTS
 *       bytes_used    -  number of bytes used by the decoder since the
 *                        last call to the decoder.
 *
 *   RETURN VALUE
 *       None.
 *
 ***************************************************************************/
void update_bitstream_status(int bytes_used)
{
  int unused_bytes = bytes_supplied - bytes_used;

  /* 'bytes_supplied' should be set to zero now, so as to begin
   * a fresh account of bytes_supplied to any routine which
   * wants to use the bitstream subsequently.
   */

  bytes_supplied = 0;


  bitstream_count += unused_bytes;
  bitstream_buf_index -= unused_bytes;
  in_buf_done -= unused_bytes;
}


/***************************************************************************
 *
 *   FUNCTION NAME - prepare_bitstream
 *
 *   DESCRIPTION
 *       This function, sets bitstream variables, so that any subsequent call
 *       to either app_swap_buffer_aac_dec() or update_bitstream_status()
 *       will work correctly.
 *
 *   ARGUMENTS
 *       None.
 *
 *   RETURN VALUE
 *       Number of bytes available in the buffer, for any subsequent call
 *       to any stream-parsing routine
 *
 ***************************************************************************/
int prepare_bitstream()
{
  int len;

  len = (bitstream_count > BS_BUF_SIZE) ? BS_BUF_SIZE : bitstream_count;
  bitstream_buf_index += len;
  bitstream_count     -= len;
  in_buf_done         += len;
  bytes_supplied      += len;

  return(len);
}


void aacd_free(void* mem)
{
    free(mem);
    return;
}
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
void* aacd_alloc_fast (int size)
{
    void* ptr = NULL;
    ptr = (void *)malloc (size+4);
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
void* aacd_alloc_slow (int size)
{
    void* ptr = NULL;
    ptr = (void *)malloc (size);
    ptr = (void *)(((long)ptr + (long)LONG_BOUNDARY-1) & (long)(~(LONG_BOUNDARY-1)));
    return ptr;
}


/***************************************************************************
 *
 *   FUNCTION NAME - App_display_error_message
 *
 *   DESCRIPTION
 *      Given a error code, this displays an appropriate message
 *
 *   ARGUMENTS
 *        rc  -  error code
 *
 *   RETURN VALUE
 *      None.
 *
 ***************************************************************************/
void App_display_error_message(AACD_RET_TYPE rc)
{
  switch(rc)
    {
    case AACD_ERROR_NO_ERROR:
      break;
    case AACD_ERROR_START_BLOCK:
      break;
    case AACD_ERROR_HUFFDECODE:
      printf("\nError in Huffman decoding. Possibly corrupted stream\n");
      break;
    case AACD_ERROR_GETCC:
      printf("\nError decoding Coupling Channel\n");
      break;
    case AACD_ERROR_DATA_CHN:
      printf("\nError decoding Data Stream Element\n");
      break;
    case AACD_ERROR_PROG_CONFIG:
      printf("Bad Prog Config Element\n");
      break;
    case AACD_ERROR_MCINFO:
      printf("\nError entering Multi-channel data\n:");
      break;
    case AACD_ERROR_END_BLOCK:
      printf("\nError : Unexpected end of block\n");
      break;
    case AACD_ERROR_HEADER_TYPES:
      printf("Input file is Not an AAC-stream\n");
      break;
    case AACD_ERROR_NOT_SUPPORTED_PCM_BITS:
      printf("\nError PCM Support: Only 16-bit of 24-bit PCM is supported\n");
      break;
    case AACD_ERROR_NOT_SUPPORTED_CHANNEL_CONFIG:
      printf("\nError ChannelConfig Support\n");
      break;
    case AACD_ERROR_NOT_SUPPORTED_CHANNEL_COUNT:
      printf("\nError Channel suppport : Only two channels are supported\n");
      break;
    case AACD_ERROR_SEEKING:
      printf("\nError Seeking Data\n");
      break;
    case AACD_ERROR_EOF:
      printf("End of Input\n");
      break;
    case AACD_ERROR_INIT:
      printf("\nError Initializing\n");
      break;
    case AACD_ERROR_INVALID_HEADER:
      printf("\nError : Corrupted Header data\n");
      break;
    case AACD_ERROR_PNS:
      printf("\nError : Corrupted Spectral data\n");
      break;
    case AACD_ERROR_TNS_COEF_RES:
      printf("\nError : Corrupted TNS data\n");
      break;
    case AACD_ERROR_BLOCK_TYPE:
      printf("\nError : IMDCT : Invalid Block Type\n");
      break;
    }
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

/* End of file */

