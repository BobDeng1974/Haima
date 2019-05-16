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
/*
 ***********************************************************************
 * Copyright (c) 2005-2010, 2012-2014, Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc.
 ***********************************************************************
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aacd_dec_interface.h"
#ifdef ALSA
#include <errno.h>
#include <poll.h>
#include <alsa/asoundlib.h>
#endif
/* Bitstream parameters structure */
typedef struct
{
  long bit_counter;
  unsigned long bit_register;
  unsigned char *bs_curr;
  unsigned char *bs_end;
  unsigned char *bs_curr_ext;
  unsigned char *bs_end_ext;
  unsigned int bs_eof;
  unsigned int bs_seek_needed;
} BitstreamParam;
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


#ifdef WAV_OUT
#include <windows.h>
#include <mmsystem.h>


/*  some good values for block size and count */
#define BLOCK_SIZE  8192*2
#define BLOCK_COUNT 4
typedef unsigned char U8;




/* module level variables */
static CRITICAL_SECTION waveCriticalSection;
static WAVEHDR*         waveBlocks;
static volatile int     waveFreeBlockCount;
static int              waveCurrentBlock;
int buffer_wav_out[8192];
HWAVEOUT hWaveOut; /* device handle */


static void writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size)
{
    WAVEHDR* current;
    int remain;

    current = &waveBlocks[waveCurrentBlock];

    while(size > 0) {
        /*
         * first make sure the header we're going to use is unprepared
         */
        if(current->dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(hWaveOut, current, sizeof(WAVEHDR));

        if(size < (int)(BLOCK_SIZE - current->dwUser)) {
            memcpy(current->lpData + current->dwUser, data, size);
            current->dwUser += size;
            break;
        }

        remain = BLOCK_SIZE - current->dwUser;
        memcpy(current->lpData + current->dwUser, data, remain);
        size -= remain;
        data += remain;
        current->dwBufferLength = BLOCK_SIZE;

        waveOutPrepareHeader(hWaveOut, current, sizeof(WAVEHDR));
        waveOutWrite(hWaveOut, current, sizeof(WAVEHDR));

        EnterCriticalSection(&waveCriticalSection);
        waveFreeBlockCount--;
        LeaveCriticalSection(&waveCriticalSection);

        /*
         * wait for a block to become free
         */
        while(!waveFreeBlockCount)
            Sleep(10);

        /*
         * point to the next block
         */
        waveCurrentBlock++;
        waveCurrentBlock %= BLOCK_COUNT;

        current = &waveBlocks[waveCurrentBlock];
        current->dwUser = 0;
    }
}

static WAVEHDR* allocateBlocks(int size, int count)
{
    unsigned char* buffer;
    int i;
    WAVEHDR* blocks;
    DWORD totalBufferSize = (size + sizeof(WAVEHDR)) * count;

    /*
     * allocate memory for the entire set in one go
     */
    if((buffer = HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        totalBufferSize
    )) == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        ExitProcess(1);
    }

    /*
     * and set up the pointers to each bit
     */
    blocks = (WAVEHDR*)buffer;
    buffer += sizeof(WAVEHDR) * count;
    for(i = 0; i < count; i++) {
        blocks[i].dwBufferLength = size;
        blocks[i].lpData = buffer;
        buffer += size;
    }

    return blocks;
}

static void freeBlocks(WAVEHDR* blockArray)
{
    /*
     * and this is why allocateBlocks works the way it does
     */
    HeapFree(GetProcessHeap(), 0, blockArray);
}

static void CALLBACK waveOutProc(
    HWAVEOUT hWaveOut,
    UINT uMsg,
    DWORD dwInstance,
    DWORD dwParam1,
    DWORD dwParam2
)
{
    int* freeBlockCounter = (int*)dwInstance;
    /*
     * ignore calls that occur due to openining and closing the
     * device.
     */
    if(uMsg != WOM_DONE)
        return;

    EnterCriticalSection(&waveCriticalSection);
    (*freeBlockCounter)++;
    LeaveCriticalSection(&waveCriticalSection);
}
#endif

#ifdef CORTEX_A8
#ifndef __WINCE
#ifndef TGT_OS_ELINUX

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
// Registers in the peripherals.
static volatile unsigned int * const irq_enable_set = (volatile unsigned int *)IRQCT_ENABLESET;
static volatile unsigned int * const irq_enable_clr = (volatile unsigned int *)IRQCT_ENABLECLR;
static volatile unsigned int * const timer_load     = (volatile unsigned int *)TIMER_LOAD;
static volatile unsigned int * const timer_control  = (volatile unsigned int *)TIMER_CONTROL;
static volatile unsigned int * const timer_clear    = (volatile unsigned int *)TIMER_CLEAR;
#endif
#endif
#endif

#define BS_BUF_SIZE AACD_INPUT_BUFFER_SIZE
#define LONG_BOUNDARY 4
#ifdef ALSA
#define WAKE_UP_LEN 1024
#endif


//DSPhl28187
int AACD_writeout(AACD_OutputFmtType data[][AAC_FRAME_SIZE], AACD_Decoder_Config *dec_config,
                   int* ShouldOpen, int* ShouldClose,char *outpath);

//DSPhl28187
void  myexit(int errornum);

void* aacd_alloc_fast (int );
void* aacd_alloc_slow (int );

extern int App_get_adts_header(AACD_Block_Params * params, int buff_len);

void App_display_error_message(AACD_RET_TYPE rc);

AACD_OutputFmtType outbuf[CHANS*AAC_FRAME_SIZE];


#ifdef ALSA
short outbuf_alsa[CHANS*AAC_FRAME_SIZE];
#endif

FILE *fin;

#define MAX_ENC_BUF_SIZE 100*BS_BUF_SIZE*24 *2

char          bitstream_buf[MAX_ENC_BUF_SIZE];
int           bitstream_buf_index      = 0;
unsigned int  bitstream_count = 0;
 /*used to count the bytes to see if a copy needed*/
int           bitstream_count_copy_once = 0;
//char          input_buff[BS_BUF_SIZE];
char         *input_buff=NULL;
char         *pinput_buff;

long in_buf_done;

//DSPhl28081 Removed the variable
//void *aptable[58];


void* aacd_alloc_fast (int size);
void* aacd_alloc_slow (int size);
void aacd_free(void* mem);



void          update_bitstream_status(int bytes_used);
int           prepare_bitstream(void);
int           prepare_bitstream_adts(AACD_Block_Params * params);

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


BitstreamParam App_bs_param;
unsigned char  App_ibs_buf[INTERNAL_BS_BUFSIZE];


AACD_Block_Params BlockParams;

int BitsInHeader=0;
int bytes_supplied=0;


int CurrFrame=0;
#define RAW_INPUT
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
    int Length, temp_len;

    AACD_Decoder_Config *dec_config;
    AACD_Decoder_info dec_info;
    AACD_Mem_Alloc_Info_Sub   *mem;
    int                       nr;
    int                       rec_no;
    char opath[AACD_PATH_LEN];
    char InFileName[AACD_PATH_LEN];
    int ret = 0;

#if WAV_OUT

		WAVEFORMATEX wfx_out;

#endif


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
    char TestVectors[][AACD_PATH_LEN] = {"test.aac", "_end_of_vectors_"};
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
#if TEST_INTERLEAVE_OUTPUT
	FILE *pfOutput;
#endif /*END - INTERLEAVE_OUTPUT*/
#endif /*END - OUT_DUMP */
#ifdef ALSA
	snd_pcm_t *playback_handle;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	snd_pcm_sframes_t frames_to_deliver;
	int nfds;
	int err;
	struct pollfd *pfds;
#endif
//#define ARM12DEBUG
#ifdef ARM12DEBUG 
    argc = 3;
    argv[1] = "AAC_HE_ADTS_22_48_WishI-48kSBRPS.aac";
    argv[2] = "AAC_HE_ADTS_22_48_WishI-48kSBRPS.pcm";
#endif

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
            if ((argc == 1)||(argc == 0)) /* No command line arguments were given */
            {
                strcat(InFileName,"test.aac");
                strcat(opath, TestVectors[0]);
		strcat(opath, "test.pcm");
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
#if TEST_INTERLEAVE_OUTPUT
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
		input_buff = (char *)malloc(BS_BUF_SIZE);
		if(input_buff == NULL)
		{
			printf("Not Enough Memory!\n");
			exit(1);
		}
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

            printf(" Decoding %s\n", InFileName);

            /* skip ID3 tag */
            while(!memcmp(bitstream_buf+bitstream_buf_index, "ID3", 3))
            {
                int tagsize = 0;
                char *pBuff = bitstream_buf+bitstream_buf_index;
                tagsize = (pBuff[6]<<21) | (pBuff[7]<<14) | (pBuff[8]<<7) | pBuff[9];
                tagsize += 10;

                bitstream_buf_index += tagsize;
                bitstream_count     -= tagsize;
                in_buf_done         += tagsize;
                bytes_supplied      += tagsize;
            }

            Length = prepare_bitstream();
            App_bs_readinit(bitstream_buf+(bitstream_buf_index-Length), Length);

            FileType = App_bs_look_bits(32);

            if (App_FindFileType(FileType) != 0)
            {
#ifndef RAW_INPUT
                printf("InputFile is not AAC\n");
                exit(1);
#else //RAW_INPUT
    { // test
                        update_bitstream_status(0);

                        Length = prepare_bitstream();
                        App_bs_readinit(bitstream_buf+bitstream_buf_index-Length, Length);
                        BitsInHeader = 0;
                        //App_get_adif_header(&BlockParams);
                        BlockParams.num_pce = 0;
                        BlockParams.ChannelConfig = 2;
                        BlockParams.SamplingFreqIndex = 4;    /* 44 */
                        BlockParams.BitstreamType = 0;
                        BlockParams.BitRate       = 0; /* Caution !*/
                        BlockParams.BufferFullness = 0;
                        BlockParams.ProtectionAbsent = 1;
                        BlockParams.CrcCheck = 0;

                        dec_config->params = &BlockParams;
                        update_bitstream_status(BitsInHeader/8);
                        Length = prepare_bitstream_adif();
#ifdef NO_STD_LIB
                        myMemCopy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#else
                        memcpy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#endif
                  //      bitstream_count_copy_once =0;
                        pinput_buff = input_buff;
                        temp_len = Length;
                  //      dec_info.BitsInBlock = 0;
                  //      dec_config->adts_format = AACD_FALSE;
    } //lyon test

#endif //RAW_INPUT
            }
#ifndef RAW_INPUT
            update_bitstream_status(0);
#else
            if (App_adif_header_present ==1 || App_adts_header_present ==1 )
            update_bitstream_status(0);
#endif
            if (App_adif_header_present)
            {
                Length = prepare_bitstream();
                App_bs_readinit(bitstream_buf+bitstream_buf_index-Length, Length);
                BitsInHeader = 0;
                App_get_adif_header(&BlockParams);
                dec_config->params = &BlockParams;
                update_bitstream_status(BitsInHeader/8);
				Length = prepare_bitstream_adif();
#ifdef NO_STD_LIB
				myMemCopy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#else
				memcpy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#endif

				bitstream_count_copy_once =0;
                pinput_buff = input_buff;
				temp_len = Length;
				dec_info.BitsInBlock = 0;
				dec_config->adts_format = AACD_FALSE;
            }

#ifdef RAW_INPUT
        if (App_adif_header_present ==0 && App_adts_header_present ==0 )
            App_adif_header_present = 1;
#endif
#ifdef ALSA
			for(;;)
			{

				if (App_adts_header_present)
               			{
                    			BitsInHeader = 0;
                   			Length = prepare_bitstream();
                    			App_bs_readinit(bitstream_buf+bitstream_buf_index-Length, Length);
                                        if(-2 == App_get_adts_header(&BlockParams, Length))
                                            break;
                    			dec_config->params = &BlockParams;
                    			update_bitstream_status(BitsInHeader/8);
                    			dec_config->adts_format = AACD_TRUE;
                		}

                 		if (App_adts_header_present)
                 		{
                 			Length = prepare_bitstream_adts(&BlockParams);

#ifdef NO_STD_LIB
	               		 	myMemCopy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#else
	                		memcpy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#endif
                    			pinput_buff = input_buff;
                    			temp_len = Length;

                 		}else if ((App_adif_header_present)&&(*(dec_config->AACD_bno) > 0))
				 {
				 	int bytes_left;
				 	Length = prepare_bitstream_adif();
					bytes_left = Length - bitstream_count_copy_once;

					if (bytes_left < AACD_6CH_FRAME_MAXLEN)
					{
#ifdef NO_STD_LIB
					    myMemCopy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#else
					    memcpy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#endif
                       			    bitstream_count_copy_once = 0;
                        		    pinput_buff = input_buff;
					    temp_len = Length;
					}
                    			else
                    			{
                    				pinput_buff += dec_info.BitsInBlock/8;
						temp_len     -= dec_info.BitsInBlock/8;
                    			}

				 }


				rc = aacd_decode_frame(dec_config,&dec_info,outbuf, pinput_buff, temp_len);

				update_bitstream_status(dec_info.BitsInBlock/8);

				if (rc == AACD_ERROR_NO_ERROR)
				{
					break;
				}

				if (rc == AACD_ERROR_EOF)
				{
					App_display_error_message(rc);
					break;
				}
				fflush (stdout);
			} /*end-while*/

			if ((err = snd_pcm_open (&playback_handle, "plughw:0,0", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
				fprintf (stderr, "cannot open audio device %s (%s)\n",
					"plughw:0,0",
					snd_strerror (err));
				exit (1);
			}
			if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
				fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
					snd_strerror (err));
				exit (1);
			}
			if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
				fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
					snd_strerror (err));
				exit (1);
			}
			if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
				fprintf (stderr, "cannot set access type (%s)\n",
					snd_strerror (err));
				exit (1);
			}
			if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
				fprintf (stderr, "cannot set sample format (%s)\n",
					snd_strerror (err));
				exit (1);
			}
			if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &dec_info.aacd_sampling_frequency, 0)) < 0) {
				fprintf (stderr, "cannot set sample rate (%s)\n",
					snd_strerror (err));
				exit (1);
			}
			if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, dec_info.aacd_num_channels)) < 0) {
				fprintf (stderr, "cannot set channel count (%s)\n",
					snd_strerror (err));
				exit (1);
			}
			if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
				fprintf (stderr, "cannot set parameters (%s)\n",
					snd_strerror (err));
				exit (1);
			}
			snd_pcm_hw_params_free (hw_params);
			/* tell ALSA to wake us up whenever WAKE_UP_LEN or more frames
			of playback data can be delivered. Also, tell
			ALSA that we'll start the device ourselves.
			*/
			if ((err = snd_pcm_sw_params_malloc (&sw_params)) < 0) {
				fprintf (stderr, "cannot allocate software parameters structure (%s)\n",
					snd_strerror (err));
				exit (1);
			}
			if ((err = snd_pcm_sw_params_current (playback_handle, sw_params)) < 0) {
				fprintf (stderr, "cannot initialize software parameters structure (%s)\n",
					snd_strerror (err));
				exit (1);
			}
			if ((err = snd_pcm_sw_params_set_avail_min (playback_handle, sw_params, WAKE_UP_LEN)) < 0) {
				fprintf (stderr, "cannot set minimum available count (%s)\n",
					snd_strerror (err));
				exit (1);
			}
			if ((err = snd_pcm_sw_params_set_start_threshold (playback_handle, sw_params, 0U)) < 0) {
				fprintf (stderr, "cannot set start mode (%s)\n",
					snd_strerror (err));
				exit (1);
			}
			if ((err = snd_pcm_sw_params (playback_handle, sw_params)) < 0) {
				fprintf (stderr, "cannot set software parameters (%s)\n",
					snd_strerror (err));
				exit (1);
			}
			/* the interface will interrupt the kernel every WAKE_UP_LEN frames, and ALSA
			will wake up this program very soon after that.
			*/
			if ((err = snd_pcm_prepare (playback_handle)) < 0) {
				fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
					snd_strerror (err));
				exit (1);
			}

			/* find out how much space is available for playback data */
			if ((frames_to_deliver = snd_pcm_avail_update (playback_handle)) < 0) {
				if (frames_to_deliver == -EPIPE) {
					fprintf (stderr, "an xrun occured\n");
				} else {
					fprintf (stderr, "unknown ALSA avail update return value (%d)\n",
						frames_to_deliver);
				}
			}

			frames_to_deliver = frames_to_deliver > dec_info.aacd_len ? dec_info.aacd_len : frames_to_deliver;

			{
				int i,j;
				short *psOutbuf_alsa = outbuf_alsa;
				if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
				{

					for(i=0;i<dec_info.aacd_len;i++)
					{

						for(j=0;j<CHANS;j++)
						{
							if(*(dec_config->ch_info[j].present))
							{
								*psOutbuf_alsa++ = outbuf[i]>>8;
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
								*psOutbuf_alsa++  = outbuf[i];
							}
						}
					}
				}
			}

			/* deliver the data */
			if (snd_pcm_writei (playback_handle, outbuf, frames_to_deliver) != frames_to_deliver) {
				fprintf (stderr, "write failed (%s)\n", snd_strerror (err));
			}

			/* wait till the interface is ready for data, or 1 second has elapsed. */
			if ((err = snd_pcm_wait (playback_handle, 1000)) < 0)
			{
				fprintf (stderr, "poll failed (%s)\n", strerror (errno));
			}

#endif
#if	WAV_OUT

			for(;;)
			{
				if (App_adts_header_present)
               			{
                    			BitsInHeader = 0;
                   			Length = prepare_bitstream();
                    			App_bs_readinit(bitstream_buf+bitstream_buf_index-Length, Length);
                                        if(-2 == App_get_adts_header(&BlockParams, Length))
                                            break;
                    			dec_config->params = &BlockParams;
                    			update_bitstream_status(BitsInHeader/8);
                    			dec_config->adts_format = AACD_TRUE;
                		}

                 		if (App_adts_header_present)
                 		{
                 			Length = prepare_bitstream_adts(&BlockParams);

#ifdef NO_STD_LIB
	               		 	myMemCopy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#else
	                		memcpy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#endif
                    			pinput_buff = input_buff;
                    			temp_len = Length;

                 		}else if ((App_adif_header_present)&&(*(dec_config->AACD_bno) > 0))
				 {
				 	int bytes_left;
				 	Length = prepare_bitstream_adif();
					bytes_left = Length - bitstream_count_copy_once;

					if (bytes_left < AACD_6CH_FRAME_MAXLEN)
					{
#ifdef NO_STD_LIB
					    myMemCopy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#else
					    memcpy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#endif
                       			    bitstream_count_copy_once = 0;
                        		    pinput_buff = input_buff;
					    temp_len = Length;
					}
                    			else
                    			{
                    				pinput_buff += dec_info.BitsInBlock/8;
						temp_len     -= dec_info.BitsInBlock/8;
                    			}

				 }


				rc = aacd_decode_frame(dec_config,&dec_info,outbuf, pinput_buff, temp_len);

				update_bitstream_status(dec_info.BitsInBlock/8);

				if (rc == AACD_ERROR_NO_ERROR)
				{
					break;
				}

				if (rc == AACD_ERROR_EOF)
				{
					App_display_error_message(rc);
					break;
				}
				fflush (stdout);
			} /*end-while*/

			{

			/*
			* initialise the module variables
				*/
				waveBlocks		   = allocateBlocks(BLOCK_SIZE, BLOCK_COUNT);
				waveFreeBlockCount = BLOCK_COUNT;
				waveCurrentBlock   = 0;

				InitializeCriticalSection(&waveCriticalSection);
				/*
				* set up the WAVEFORMATEX structure.
				*/
				wfx_out.nSamplesPerSec	= dec_info.aacd_sampling_frequency;  /* sample rate */
				wfx_out.wBitsPerSample	= dec_config->num_pcm_bits;  /* sample size */
				wfx_out.nChannels		= dec_info.aacd_num_channels;	   /* channels	  */
				wfx_out.cbSize			= 0;	  /* size of _extra_ info */
				wfx_out.wFormatTag		= WAVE_FORMAT_PCM;
				wfx_out.nBlockAlign 	= (wfx_out.wBitsPerSample * wfx_out.nChannels) >> 3;
				wfx_out.nAvgBytesPerSec = wfx_out.nBlockAlign * wfx_out.nSamplesPerSec;
				if(waveOutOpen(
					&hWaveOut,
					WAVE_MAPPER,
					&wfx_out,
					(DWORD_PTR)waveOutProc,
					(DWORD_PTR)&waveFreeBlockCount,
					CALLBACK_FUNCTION
					) != MMSYSERR_NOERROR) {
					//fprintf(stderr, "%s: unable to open wave mapper device\n", argv[0]);
					printf("unable to open wave mapper device\n");
					ExitProcess(1);
				}

			}

			{
				int i,j;
				if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
				{

					for(i=0;i<dec_info.aacd_len*dec_info.aacd_num_channels;i++)
					{

						fwrite(&outbuf[i], 3, 1, pfOutput);
#ifdef OUT2CHANNEL  
						if(1 == dec_info.aacd_num_channels)
							fwrite(&outbuf[i], 3, 1, pfOutput);
#endif
					}
				}
				else
				{
					for(i=0;i<dec_info.aacd_len*dec_info.aacd_num_channels;i++)
					{

						writeAudio(hWaveOut, (U8*) &outbuf[i], 2);
#ifdef OUT2CHANNEL                          
						if(1 == dec_info.aacd_num_channels)
							writeAudio(hWaveOut, (U8*) &outbuf[i], 2);
#endif
					}
				}
			}

#endif

            /* Get ADTS-Header if present and start decoding */

            //while(1) //tlsbo89610
			for(;;)
            {
                if ((bitstream_count <= 0) &&(*(dec_config->AACD_bno) > 0))
                {
                    ShouldClose = 1;
                    //DSPhl28187
					break;
#ifdef OUT_DUMP
#if  TEST_INTERLEAVE_OUTPUT
#ifdef ALSA

					/* find out how much space is available for playback data */
					if ((frames_to_deliver = snd_pcm_avail_update (playback_handle)) < 0) {
						if (frames_to_deliver == -EPIPE) {
							fprintf (stderr, "an xrun occured\n");
							break;
						} else {
							fprintf (stderr, "unknown ALSA avail update return value (%d)\n",
								frames_to_deliver);
							break;
						}
					}

					frames_to_deliver = frames_to_deliver > dec_info.aacd_len ? dec_info.aacd_len : frames_to_deliver;

					{
						int i,j;
						short *psOutbuf_alsa = outbuf_alsa;
						if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
						{

							for(i=0;i<dec_info.aacd_len*dec_info.aacd_num_channels;i++)
							{
								*psOutbuf_alsa++ = outbuf[i]>>8;

							}
						}
						else
						{
							for(i=0;i<dec_info.aacd_len*dec_info.aacd_num_channels;i++)
							{

									if(*(dec_config->ch_info[j].present))
									{
										*psOutbuf_alsa++  = outbuf[i];
									}

							}
						}
					}

					/* deliver the data */
					if (snd_pcm_writei (playback_handle, outbuf_alsa, frames_to_deliver) != frames_to_deliver) {
						fprintf (stderr, "write failed (%s)\n", snd_strerror (err));
						break;
					}

					/* wait till the interface is ready for data, or 1 second has elapsed. */
					if ((err = snd_pcm_wait (playback_handle, 1000)) < 0)
					{
						fprintf (stderr, "poll failed (%s)\n", strerror (errno));
						break;
					}
#else
					{
						int i,j;
						if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
                                                  {

			                               for(i=0;i<AAC_FRAME_SIZE*dec_info.aacd_num_channels;i++)
			                               {
				                           fwrite(&outbuf[i], 3, 1, pfOutput);
#ifdef OUT2CHANNEL  
				                           if(1 == dec_info.aacd_num_channels)
		  			                       fwrite(&outbuf[i],3, 1, pfOutput);
#endif
			                               }

						}
						else
						{
							for(i=0;i<dec_info.aacd_len*dec_info.aacd_num_channels;i++)
							{


#if WAV_OUT
							    writeAudio(hWaveOut, (U8*) &outbuf[i], 2);
#else
							    fwrite(&outbuf[i], 2, 1, pfOutput);
#endif
#ifdef OUT2CHANNEL
							    if(1 == dec_info.aacd_num_channels)  
#if WAV_OUT
								writeAudio(hWaveOut, (U8*) &outbuf[i], 2);
#else
							    fwrite(&outbuf[i], 2, 1, pfOutput);
#endif
#endif

							}
						}
					}
#endif
#else

		    if (AACD_writeout(outbuf, dec_config, &ShouldOpen, &ShouldClose, opath)!=0)
                    {
                        printf("Application File I/O Error\n");
                        exit(1);
                    }
#endif  //TEST_INTERLEAVE_OUTPUT
#endif
                    //printf("End of Input\n");
                }

                if (App_adts_header_present)
                {
                    BitsInHeader = 0;
                    Length = prepare_bitstream();
                    if(Length <= 0)
                    {
                        printf("End of stream! exit! 1\n");
                        break;
                    }
                    App_bs_readinit(bitstream_buf+bitstream_buf_index-Length, Length);
                    ret = App_get_adts_header(&BlockParams, Length);
                    if(-2 == ret)
                    {
                        printf("profile is not supported!\n");
                        break;
                    }
#ifdef RAW_INPUT
                    else if(-1 == ret)
                    {
                        if( bitstream_count <= 0)
                        {
                            printf("End of stream! exit! 2\n");
                            break;
                        }
                        /* haven't found adts header, consider the stream as RAW data input. */
                        printf("haven't found adts header, consider the stream as RAW data input!\n");
                        App_adif_header_present = 1;
                        App_adts_header_present = 0;
                        *(dec_config->AACD_bno) = 0;
                        dec_config->adts_format = AACD_FALSE;
                        update_bitstream_status(0);
#ifdef NO_STD_LIB
	                  myMemCopy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#else
	                  memcpy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#endif
                        pinput_buff = input_buff;
                        temp_len = Length;
                    }
#endif
                    else
                    {
                        dec_config->params = &BlockParams;
                        update_bitstream_status(BitsInHeader/8);
                        dec_config->adts_format = AACD_TRUE;
                    }
                }

                 if (App_adts_header_present)
                 	{
                 	Length = prepare_bitstream_adts(&BlockParams);
			if(Length<0 || Length >AACD_6CH_FRAME_MAXLEN)
			{
				printf("ADTS Header ERROR!\n");
				exit(1);
			}
                    if(Length == 0 && bitstream_count == 0)
                    {
                       printf("End of stream! exit! 3\n");
                       break;
                    }
                    if(Length < BlockParams.frame_length)
                    {
                        printf("The last frame is cut! End of stream! exit! 4\n");
                        break;
                    }
                    if ( bitstream_buf_index >=MAX_ENC_BUF_SIZE)
                    {
                               printf("bitstream_buf_index Error!\n");
                               exit(1);
                    }
#ifdef NO_STD_LIB
	                myMemCopy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#else
	                memcpy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#endif
                    pinput_buff = input_buff;
                    temp_len = Length;

                 	}
				 else if ((App_adif_header_present)&&(*(dec_config->AACD_bno) > 0))
				 	{
				 	int bytes_left;
				 	Length = prepare_bitstream_adif();
                                if(Length == 0 && bitstream_count == 0)
                                {
                                    printf("End of stream! exit! 5\n");
                                    break;
                                }
					bytes_left = Length - bitstream_count_copy_once;

					if (bytes_left < AACD_6CH_FRAME_MAXLEN)
						{
#ifdef NO_STD_LIB
					    myMemCopy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#else
					    memcpy (input_buff, bitstream_buf+bitstream_buf_index-Length, Length);
#endif
                        bitstream_count_copy_once = 0;
                        pinput_buff = input_buff;
						temp_len = Length;
						}
                    else
                    	{
                    	pinput_buff += dec_info.BitsInBlock/8;
						temp_len     -= dec_info.BitsInBlock/8;
                    	}

				 	}

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


#ifdef MEASURE_STACK_USAGE //TLSbo88602
        PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
#endif

#ifdef __WINCE
       Flag=QueryPerformanceFrequency(&lpFrequency1);
	   Flag=QueryPerformanceCounter(&lpPerformanceCount1);
#endif

#ifdef TIME_PROFILE_RVDS					//tlsbo85204
                prev_clk = prev_cycles();
#endif
#ifdef TIME_PROFILE
        gettimeofday(&StartTime, NULL);
#endif
                rc = aacd_decode_frame(dec_config,&dec_info,outbuf, pinput_buff, temp_len);

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

                //printf ("Current clock = %d\n", curr_clk);
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

                if (rc != AACD_ERROR_NO_ERROR)
                {
                    App_display_error_message(rc);
                    ShouldClose = 1;
                    //DSPhl28187
#ifdef OUT_DUMP
						if (rc == AACD_ERROR_EOF)
						{
#if  TEST_INTERLEAVE_OUTPUT
#ifdef ALSA

						/* find out how much space is available for playback data */
						if ((frames_to_deliver = snd_pcm_avail_update (playback_handle)) < 0) {
							if (frames_to_deliver == -EPIPE) {
								fprintf (stderr, "an xrun occured\n");
								break;
							} else {
								fprintf (stderr, "unknown ALSA avail update return value (%d)\n",
									frames_to_deliver);
								break;
							}
						}

						frames_to_deliver = frames_to_deliver > dec_info.aacd_len ? dec_info.aacd_len : frames_to_deliver;

						{
							int i,j;
							short *psOutbuf_alsa = outbuf_alsa;
							if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
							{

								for(i=0;i<dec_info.aacd_len*dec_info.aacd_num_channels;i++)
								{


									*psOutbuf_alsa++ = outbuf[i]>>8;


								}
							}
							else
							{
								for(i=0;i<dec_info.aacd_len*dec_info.aacd_num_channels;i++)
								{

									*psOutbuf_alsa++  = outbuf[i];


								}
							}
						}

						/* deliver the data */
						if (snd_pcm_writei (playback_handle, outbuf_alsa, frames_to_deliver) != frames_to_deliver) {
							fprintf (stderr, "write failed (%s)\n", snd_strerror (err));
							break;
						}

						/* wait till the interface is ready for data, or 1 second has elapsed. */
						if ((err = snd_pcm_wait (playback_handle, 1000)) < 0)
						{
							fprintf (stderr, "poll failed (%s)\n", strerror (errno));
							break;
						}
#else
						{
							int i,j;
							if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
							{

								for(i=0;i<dec_info.aacd_len*dec_info.aacd_num_channels;i++)
								{
				                                      fwrite(&outbuf[i], 3, 1, pfOutput);
#ifdef OUT2CHANNEL                                                        
				                                      if(1 == dec_info.aacd_num_channels)
					                                  fwrite(&outbuf[i], 3, 1, pfOutput);
#endif

			                                          }
		                                         	}
		                                          else
	                                                  {
		                                              for(i=0;i<dec_info.aacd_len*dec_info.aacd_num_channels;i++)
		                                              {
                                                        #if WAV_OUT
								writeAudio(hWaveOut, (U8*) &outbuf[i], 2);
                                                        #else
					                         fwrite(&outbuf[i], 2, 1, pfOutput);
                                                        #endif
#ifdef OUT2CHANNEL                                      
					                         if(1 == dec_info.aacd_num_channels)
                                                        #if WAV_OUT
						                      writeAudio(hWaveOut, (U8*) &outbuf[i], 2);
                                                        #else
						                     fwrite(&outbuf[i], 2, 1, pfOutput);
                                                        #endif
#endif
				                              }
			                                 }
		                                    }


#endif
#else

		    if (AACD_writeout(outbuf, dec_config, &ShouldOpen, &ShouldClose, opath)!=0)
                    {
                        printf("Application File I/O Error\n");
                        exit(1);
                    }
#endif  //TEST_INTERLEAVE_OUTPUT
						}
#endif

					if (rc == AACD_ERROR_EOF)
						break;
					if (App_adif_header_present)
						break;
                }

                update_bitstream_status(dec_info.BitsInBlock/8);
                if (App_adif_header_present)
                {
					bitstream_count_copy_once += dec_info.BitsInBlock/8;
                }

#ifndef CT_CONF
                if (*(dec_config->AACD_bno) > 3)        //DSPhl28187
#else
					if (*(dec_config->AACD_bno) > 1)		//DSPhl28187
#endif
                {
                    //ptr->out_ptr = opath; //TLSbo63933
                    //DSPhl28187
#ifdef OUT_DUMP
#if  TEST_INTERLEAVE_OUTPUT
#ifdef ALSA

						/* find out how much space is available for playback data */
						if ((frames_to_deliver = snd_pcm_avail_update (playback_handle)) < 0) {
							if (frames_to_deliver == -EPIPE) {
								fprintf (stderr, "an xrun occured\n");
								break;
							} else {
								fprintf (stderr, "unknown ALSA avail update return value (%d)\n",
									frames_to_deliver);
								break;
							}
						}

						frames_to_deliver = frames_to_deliver > dec_info.aacd_len ? dec_info.aacd_len : frames_to_deliver;

						{
							int i,j;
							short *psOutbuf_alsa = outbuf_alsa;
							if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
							{

								for(i=0;i<dec_info.aacd_len*dec_info.aacd_num_channels;i++)
								{

									*psOutbuf_alsa++ = outbuf[i]>>8;
								}
							}
							else
							{
								for(i=0;i<dec_info.aacd_len*dec_info.aacd_num_channels;i++)
								{

									*psOutbuf_alsa++  = outbuf[i];

								}
							}
						}

						/* deliver the data */
						if (snd_pcm_writei (playback_handle, outbuf_alsa, frames_to_deliver) != frames_to_deliver) {
							fprintf (stderr, "write failed (%s)\n", snd_strerror (err));
							break;
						}

						/* wait till the interface is ready for data, or 1 second has elapsed. */
						if ((err = snd_pcm_wait (playback_handle, 1000)) < 0)
						{
							fprintf (stderr, "poll failed (%s)\n", strerror (errno));
							break;
						}
#else
						{
							int i,j;
							if (AACD_24_BIT_OUTPUT == dec_config->num_pcm_bits)
							{

								for(i=0;i<dec_info.aacd_len*dec_info.aacd_num_channels;i++)
								{
				                                     fwrite(&outbuf[i], 3, 1, pfOutput);
#ifdef OUT2CHANNEL  
				                                     if(1 == dec_info.aacd_num_channels)
					                                   fwrite(&outbuf[i], 3, 1, pfOutput);
#endif
		                                                  }
	                                                  }
		                                          else
	                                                  {
		                                                  for(i=0;i<dec_info.aacd_len*dec_info.aacd_num_channels;i++)
		                                                  {
                                                                    #if WAV_OUT
						                    writeAudio(hWaveOut, (U8*) &outbuf[i], 2);
                                                                    #else
			         	                            fwrite(&outbuf[i], 2, 1, pfOutput);
                                                                    #endif
#ifdef OUT2CHANNEL  
					                            if(1 == dec_info.aacd_num_channels)
                                                                    #if WAV_OUT
					                             writeAudio(hWaveOut, (U8*) &outbuf[i], 2);
                                                                    #else
				    	                             fwrite(&outbuf[i], 2, 1, pfOutput);
                                                                    #endif
#endif
		                                                  }
	                                                  }
		}
#endif
#else

		    if (AACD_writeout(outbuf, dec_config, &ShouldOpen, &ShouldClose, opath)!=0)
                    {
                        printf("Application File I/O Error\n");
                        exit(1);
                    }
#endif  //TEST_INTERLEAVE_OUTPUT
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
//#ifndef __WINCE
 #ifndef WAV_OUT
  //                  if(*(dec_config->AACD_bno) >= 184)
  //                  printf ("\r[%3d]\n", (int) *(dec_config->AACD_bno));  //DSPhl28187                    
 #endif
//#endif
#endif
#ifndef WAV_OUT

                    fflush (stdout);
#endif
                }
              //  printf ("Frame No = [%3d]\n", (int) ptr->AACD_bno);
#ifndef WAV_OUT
                fflush (stdout);
#endif
            } /*end-while*/

#if WAV_OUT
 {
    int i;
    while(waveFreeBlockCount < BLOCK_COUNT)
        Sleep(10);

    /*
     * unprepare any blocks that are still prepared
     */
    for(i = 0; i < waveFreeBlockCount; i++)
        if(waveBlocks[i].dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(hWaveOut, &waveBlocks[i], sizeof(WAVEHDR));


    DeleteCriticalSection(&waveCriticalSection);
    freeBlocks(waveBlocks);
    waveOutClose(hWaveOut);

 }
#endif


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
	#ifdef MEASURE_STACK_USAGE
	#ifdef MEASURE_HEAP_USAGE
 	printf("\n Stack and Heap are  %d\t%d\t",s32PeakStack,s32TotalMallocBytes);
	#endif
	#endif
	printf("\n%s\t%s\t%d\t%d\t%d\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname,InFileName,sampling_rate,bit_rate,\
 	        num_chans,prev_DecTimeUs,min_DecTimeUs,(int) *(dec_config->AACD_bno),max_frame,min_frame,TotalDecTimeUs);
 //	fprintf(fp_mhz, "\n%s\t%s\t%d\t%d\t%d\t%ld\t%ld\t%d\t%d\t%d\t%ld\t",chname,InFileName,sampling_rate,bit_rate,
// 	        num_chans,prev_DecTimeUs,min_DecTimeUs,(int) *(dec_config->AACD_bno),max_frame,min_frame,TotalDecTimeUs);
       performance = (double)TotalDecTimeUs * 0.000001*cpu_freq*sampling_rate/(int) *(dec_config->AACD_bno)/1024;
       fprintf(fp_mhz, "|\tAAC Decoder \t|\tARM%d\t|\t%9d\t|\t%d\t|\t%8d\t|\t%d\t|\t%.8f\t|%s\n", platform, sampling_rate, num_chans, bit_rate, bit_len, performance, InFileName);
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


#ifdef ARM_ADS
#ifndef SINGLE_VECTOR
        }
    }
#endif
#endif

#ifdef MHZ_MEASUREMENT
    fclose(fp_mhz);
#endif
#ifdef ALSA
	snd_pcm_close (playback_handle);
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
    free(input_buff);

#ifdef __WINCE
		fp_mhz = fopen("aac_dec.txt","a");

		_ftprintf(fp_mhz,TEXT("\n%s\t%I64d\t"),InFileName, TotalDecTime);

		fclose(fp_mhz);

#endif
    return 0;
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


/***************************************************************************
 *
 *   FUNCTION NAME - prepare_bitstream_adts
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
int prepare_bitstream_adts(AACD_Block_Params * params)
{
  int len, frame_len;
  len = 0;
  frame_len = params->frame_length;
  if(frame_len<0)
	  return -1; //Error frame header
  len = (bitstream_count > frame_len) ? frame_len : bitstream_count;
  bitstream_buf_index += len;
  bitstream_count     -= len;
  in_buf_done         += len;
  bytes_supplied      += len;

  return(len);
}
/***************************************************************************
 *
 *   FUNCTION NAME - prepare_bitstream_adts
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
int prepare_bitstream_adif()
{
  int len, frame_len;
  len = 0;
  //frame_len = params->frame_length;
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
#ifdef CORTEX_A8
#ifndef __WINCE
#ifndef TGT_OS_ELINUX
__irq void IRQ_Handler(void)    	//Lyon 070731
{


   *timer_clear	= 1;
   *irq_enable_clr = 0x02;
   *irq_enable_set = 0x02;
}
#endif
#endif
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

/* End of file */

