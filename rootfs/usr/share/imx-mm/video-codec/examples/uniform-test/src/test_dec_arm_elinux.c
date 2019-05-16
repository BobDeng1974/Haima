/*
 *  Copyright (c) 2005-2013, Freescale Semiconductor Inc.,
 *  All Rights Reserved.
 *
 *  The following programs are the sole property of Freescale Semiconductor Inc.,
 *  and contain its proprietary and confidential information.
 *
 */

#include "stdio.h"

#ifdef __WINCE
#include <windows.h>
#elif RVDS
//
#else
#include <sys/time.h>
#endif
#include "dut_test_api.h"

/* APP version information */
#define APP_NAME "test_dec_arm_elinux"
#define VERSION_NUM "1.0"
#define VERSION_STR_SUFFIX ""
#define SEPARATOR " "
#define APP_VERSION_STR \
    (APP_NAME ":" VERSION_NUM \
     SEPARATOR VERSION_STR_SUFFIX \
     SEPARATOR "build on" \
     SEPARATOR __DATE__ SEPARATOR __TIME__)

int g_last_width=0;	//eagle for size change
int g_last_height=0;



#define APP_DEBUG_PRINTF printf
#define NULL  (void*)0
/* data type definition */
typedef void            Void;
typedef long            I32;
typedef unsigned long   U32;
typedef short           I16;
typedef unsigned short  U16;
typedef char            I8;
typedef unsigned char   U8;
typedef unsigned char * String;
typedef float           Float;
typedef double          Double;
typedef int             Bool;
typedef unsigned long   Time;

typedef  T_DEC_RETURN_DUT  ( VideoTestDecInitFunc )     ( void ** _ppDecObj, T_DEC_CONTXT_DUT * _psDecContxt ); 
typedef  T_DEC_RETURN_DUT  ( VideoTestDecFrameFunc )    ( void * _pDecObj, T_DEC_CONTXT_DUT * _psDecContxt ); 
typedef  T_DEC_RETURN_DUT  ( VideoTestDecReleaseFunc )  ( void * _pDecObj, T_DEC_CONTXT_DUT * _psDecContxt ); 
typedef  T_DEC_RETURN_DUT  ( VideoTestDecSetProbesFunc )( void * _pDecObj, PROBE_DUT_FUNC * _pProbe ); 

typedef struct
{
    VideoTestDecInitFunc      * ptrVideoTestDecInit;
    VideoTestDecFrameFunc     * ptrVideoTestDecFrame;
    VideoTestDecReleaseFunc   * ptrVideoTestDecRelease;
    VideoTestDecSetProbesFunc * ptrVideoTestDecSetProbes;
}
T_DEC_FUNC;

#define CASE_FIRST(x)   if (strncmp(argv[0], x, strlen(x)) == 0)
#define CASE(x)         else if (strncmp(argv[0], x, strlen(x)) == 0)
#define DEFAULT         else


#define MAX_STRLEN              200
#define INPUT_BUF_LEN           512

typedef struct
{
	char 	infile[MAX_STRLEN];
	char 	outfile[MAX_STRLEN];
	char 	dutlib[MAX_STRLEN];
	char 	resfile[MAX_STRLEN];
	char 	logfile[MAX_STRLEN];

	int 	wp_argc;
	char   *wp_argv[64];
	char 	wp_options[3*MAX_STRLEN];
	
    int     saveYUV;
    int     maxnum;
    int     display;

	int     putLog;
	int 	saveResult;
	int 	memFlag;
	int 	libVer;
}
IOParams;

static IOParams g_ioParams;

#ifndef RVDS
#define STACK_MEASURE
#define HEAP_MEASURE
#endif

#ifdef STACK_MEASURE
//#define STACK_PRINTF printf
 
//============== structure definition ================//
typedef struct
{
 unsigned char *pSP;       // temporary sp value for stack bottom
 unsigned char *pBaseSP;         // sp value for stack top 
 unsigned long TmpCount;        // temporary count
 unsigned long TmpValue;        // temporary value for stack depth
 unsigned long Peak;       // peak value of stack
} sStackMeasure;
 
typedef struct 
{
 unsigned char* pBaseSP;  // current sp value before entering callback function
 unsigned char* pBeforeBotSP; // sp value for stack bottom before entering callback function
 unsigned char* pAfterBotSP; // sp value for stack bottom after returning from callback function 
}sStackCallBack;
 
sStackMeasure g_StackMeasure;
sStackCallBack g_StackCallBack;
 
#define MAX_STACK_SIZE             4096     /* stack size painted */
#define PROBE_STACK_SIZE           0x20     /* stack size to check */
#define EMPTY_STACK_MARKER         0x55  /* stack marker */
 
//================ platform macro ==========================//
#ifdef __WINCE
static void * GET_SP()
{
	int SP_Base;
	return (void*)&SP_Base;
}
#define GET_CURRENT_SP(base) { \
  base=GET_SP(); \
 }
#else
//#define GET_CURRENT_SP(base) base=0;
#define GET_CURRENT_SP(base) { \
  __asm ( \
    "MOV %0, R13": "=r" (base) \
  ); \
 }
#endif
 
//============== common macro ========================//
#define STACK_LOOPUP_BOT(base) { \
    g_StackMeasure.TmpCount= 0; \
    g_StackMeasure.TmpValue = 0;      \
    g_StackMeasure.pSP= base; \
    while (g_StackMeasure.TmpCount < MAX_STACK_SIZE) \
    { \
       g_StackMeasure.pSP--; \
       if (*g_StackMeasure.pSP == EMPTY_STACK_MARKER) \
            g_StackMeasure.TmpValue++;  \
        else  \
            g_StackMeasure.TmpValue= 0;  \
        if (g_StackMeasure.TmpValue>= PROBE_STACK_SIZE) \
            break;  \
        g_StackMeasure.TmpCount++; \
    }  \
    g_StackMeasure.pSP += g_StackMeasure.TmpValue; \
  }
 
#define STACK_FILL_TAG(from,to) { \
  unsigned char* top=from; \
  unsigned char* bot=to; \
  while(top >= bot) \
  { \
   *top=EMPTY_STACK_MARKER; \
   top--; \
  } \
 }
 
//=============== basic macro ===================//
#define STACK_INIT() {g_StackMeasure.Peak=0;}
 
/* Macro to paint the stack area */
#define STACK_TAG()       {  \
 GET_CURRENT_SP(g_StackMeasure.pBaseSP); \
 STACK_FILL_TAG(g_StackMeasure.pBaseSP-1, g_StackMeasure.pBaseSP-MAX_STACK_SIZE); \
 }
 
/* make sure that the arguments are not complicated */
#define STACK_UPDATE()        { \
 STACK_LOOPUP_BOT(g_StackMeasure.pBaseSP); \
    /*STACK_PRINTF("sp base: 0x%X, sp: 0x%X , tmpvalue: %d \r\n",g_StackMeasure.pBaseSP,g_StackMeasure.pSP,g_StackMeasure.TmpValue);*/ \
    g_StackMeasure.TmpValue = (g_StackMeasure.pBaseSP-g_StackMeasure.pSP); \
    if (g_StackMeasure.TmpValue > g_StackMeasure.Peak) \
  g_StackMeasure.Peak=g_StackMeasure.TmpValue; \
    /*STACK_PRINTF("value: %d, peak: %d \r\n",g_StackMeasure.TmpValue,g_StackMeasure.Peak);*/ \
  }
 
#define STACK_PRINTF() {printf("stack peak: %d \r\n",g_StackMeasure.Peak);}
 

#define STACK_CALLBACK_TAG() { \
 GET_CURRENT_SP(g_StackCallBack.pBaseSP); \
 STACK_LOOPUP_BOT(g_StackCallBack.pBaseSP); \
 g_StackCallBack.pBeforeBotSP=g_StackMeasure.pSP; \
 }
 
#define STACK_CALLBACK_RESTORE() { \
 STACK_LOOPUP_BOT(g_StackCallBack.pBaseSP); \
 g_StackCallBack.pAfterBotSP=g_StackMeasure.pSP; \
 if (g_StackCallBack.pAfterBotSP < g_StackCallBack.pBeforeBotSP) \
 { \
  STACK_FILL_TAG((g_StackCallBack.pBeforeBotSP-1),g_StackCallBack.pAfterBotSP); \
 } \
  }
 
#else  // STACK_MEASURE
#define STACK_INIT() {;}
#define STACK_TAG() {;}
#define STACK_UPDATE() {;}
#define STACK_PRINTF() {;}
 
#define STACK_CALLBACK_TAG() {;}
#define STACK_CALLBACK_RESTORE() {;}
 
#endif  // STACK_MEASURE 

#ifdef HEAP_MEASURE
typedef struct
{
 unsigned int TotalSize;
} sHeapMeasure;
 
sHeapMeasure g_HeapMeasure;
#define HEAP_INIT() {g_HeapMeasure.TotalSize=0;}
#define HEAP_INCREASE(size) {g_HeapMeasure.TotalSize+=size;}
#define HEAP_DECREASE(size) {g_HeapMeasure.TotalSize-=size;}
#define HEAP_PRINTF() {printf("heap size: %d \r\n",g_HeapMeasure.TotalSize);}
#else
#define HEAP_INIT() {;}
#define HEAP_INCREASE(size) {;}
#define HEAP_DECREASE(size) {;}
#define HEAP_PRINTF() {;}
#endif


const char * App_CodecVersionInfo()
{
	return (const char *)APP_VERSION_STR;
}


static void usage(char*program)
{
		printf("\nUsage: %s [options] -l dynamic_library -i bitstream_file\n", program);
		printf("options:\n"
			   "	-o <file_name>		   : Save decoded output in YUV 4:2:0 format "
			   "[default: no save]\n"
			   "	-n <frame_num>				  : decode max <frame_num> frames\n"
			   "[default: all frames will be decoded]\n"
			   "    -t <frame time log>           : if specified, produce every frame decoding time in log file.\n"
			   "    -r <report file>              : if specified, produce test information in report_file.\n" 
			   "	-d							  : if specified, LCD render enabled.\n"
			   "	-m							  : if specified, print stack and heap information.\n"
			   "	-w,[wrapper options]		  : if specified, pass options to wrapper.\n"
			   "	-v                  		  : if specified, print library version.\n"
			   );
		exit(0);
}

#define NAME_SIZE 256

static void GetUserInput(IOParams *pIO, int argc, char *argv[])
{
    int     bitFileDone = 0;
	char	**wp_argv = NULL;
	char    *wp_options = NULL;
	char temp[NAME_SIZE];

    // Defaults
    pIO->putLog = 0;
    pIO->saveYUV = 0;
    pIO->maxnum = 0;
    pIO->display = 0;
	
	pIO->wp_argc = 0;
	pIO->memFlag = 0;
	pIO->libVer = 0;

	pIO->saveResult = 0;
	
    pIO->infile[0] = 0;
    pIO->outfile[0] = 0;
	pIO->dutlib[0] = 0;
	pIO->resfile[0] = 0;
	pIO->logfile[0] = 0;
	
	memset(&pIO->wp_argv[0],0, sizeof(pIO->wp_argv));
	memset(&pIO->wp_options[0],0, sizeof(pIO->wp_options));

    argc--;
    argv++;

	wp_argv = pIO->wp_argv;
	wp_options = pIO->wp_options;
	
    while (argc)
    {
        if (argv[0][0] == '-')
        {
            CASE_FIRST("-i")
            {
                argc--;
                argv++;
                if (argv[0] != NULL)
                {
#ifdef __WINCE                			
					fnGetHomeDir(temp);
					strcat(temp,argv[0]);
					strcpy((char *)pIO->infile, temp);
#else
					strcpy((char *)pIO->infile, argv[0]);
#endif
					bitFileDone = 1;
                }
            }
			CASE("-l")
            {
                argc--;
                argv++;
                if (argv[0] != NULL)
					strcpy((char *)pIO->dutlib, argv[0]);
            }
			CASE("-o")
            {
                argc--;
                argv++;
                if (argv[0] != NULL)
				{
#ifdef __WINCE                			
					fnGetHomeDir(temp);
					strcat(temp,argv[0]);
					strcpy((char *)pIO->outfile, temp);
#else				
					strcpy((char *)pIO->outfile, argv[0]);
#endif
					pIO->saveYUV = 1;
				}
            }
			CASE("-t")
            {
                argc--;
                argv++;
                if (argv[0] != NULL)
				{
#ifdef __WINCE                			
					fnGetHomeDir(temp);
					strcat(temp,argv[0]);
					strcpy((char *)pIO->logfile, temp);
#else
					strcpy((char *)pIO->logfile, argv[0]);
#endif
					pIO->putLog = 1;
				}
            }
			CASE("-r")
            {
                argc--;
                argv++;
                if (argv[0] != NULL)
				{
#ifdef __WINCE                			
					fnGetHomeDir(temp);
					strcat(temp,argv[0]);
					strcpy((char *)pIO->resfile, temp);
#else
					strcpy((char *)pIO->resfile, argv[0]);
#endif
					pIO->saveResult = 1;
				}
            }
            CASE("-n")
            {
                argc--;
                argv++;
                if (argv[0] != NULL)
                    sscanf(argv[0], "%d", &pIO->maxnum);
            }
            
            CASE("-d")
            {
                pIO->display = 1;
            }
			CASE("-m")
            {
                pIO->memFlag = 1;
            }
			CASE("-v")
            {
                pIO->libVer = 1;
            }
            CASE("-w,")
            {
                if (argv[0] != NULL)
                {
                    sscanf(argv[0], "-w,%s", wp_options);
					*wp_argv = wp_options;
					
					wp_argv++;
					pIO->wp_argc++;

					wp_options+=strlen(wp_options);
					wp_options++;
                }

            }
            DEFAULT                             // Has to be last
            {
                APP_DEBUG_PRINTF("Unsupported option %s\n", argv[0]);
                usage(pIO->infile);
            }
        }
        else
        {
#ifdef RVDS //fix argument bug in RVDS
	        //argc--;
      		 // argv++;		
#else
            APP_DEBUG_PRINTF("Unsupported option %s\n", argv[0]);
                usage(pIO->infile);
#endif				
            
        }
        argc--;
        argv++;
    }
    if (!bitFileDone)
        usage(pIO->infile);
}



#ifdef __WINCE


void* dlsym(HMODULE handle,char *name) 
{
	char wchar_argv[NAME_SIZE];

	mbstowcs(wchar_argv,name,NAME_SIZE);

	return ((void*)GetProcAddress(handle,wchar_argv));
};

void * dlopen(char* file, int mode)
{
	char wchar_argv[NAME_SIZE];

	mbstowcs(wchar_argv,file,NAME_SIZE);

	return ((void*)LoadLibrary((LPCTSTR)wchar_argv));
}

#define dlclose(handle) FreeLibrary(handle);

#define dlerror() ((void*)("not support"))

int fnGetHomeDir( unsigned char * _lpszDirctory )   
{   
    LPTSTR     String;   
    U32        uiResult;   
    TCHAR      achBuf[NAME_SIZE];   

    memset( achBuf, 0, NAME_SIZE );
    if( uiResult = GetModuleFileName(NULL, achBuf, sizeof(achBuf)/sizeof(TCHAR)) )   
    {   
        String = achBuf + uiResult * sizeof(TCHAR);   
        while( *String != L'\\' )  
        {
            String--;   
        }
        *(++String) = L'\0';   
        if(WideCharToMultiByte(CP_ACP, 0, achBuf, -1, _lpszDirctory, MAX_PATH, NULL, NULL))   
        {   
            return 0;   
        }   
    }
    
    return 1;   
}

#elif RVDS
T_DEC_RETURN_DUT VideoTestDecInit_dut( void **ppDecWrapperObj, T_DEC_CONTXT_DUT *_psDecAppContxt);
T_DEC_RETURN_DUT VideoTestDecFrame_dut( void * _pDecWrapperObj,T_DEC_CONTXT_DUT * _psDecContxt);
T_DEC_RETURN_DUT VideoTestDecRelease_dut( void * pDecWrapperObj, T_DEC_CONTXT_DUT * psDecContxt );
T_DEC_RETURN_DUT VideoTestDecSetProbes_dut(void *pDecWrapperObj, PROBE_DUT_FUNC *psProbe);
#define dlclose(handle) 

#endif

static FILE *g_fdut = NULL;
static int g_lcd_open = 0;

static Void CB_fnWriteFile( FILE * _fpDstFile, U8 * _puchSrc, I32 _iWidth, I32 _iHeight, I32 _iStride )
{
    FILE * fpDstFile = _fpDstFile;
    U8 * puchSrc = _puchSrc;
    I32 iWidth = _iWidth;
    I32 iHeight = _iHeight;
    I32 iStride = _iStride;
    I32 iIndex;

    for ( iIndex = 0; iIndex < iHeight; iIndex++ )
    {
        fwrite( puchSrc, iWidth, sizeof(U8), fpDstFile );
        puchSrc += iStride;
    }
}

#include "render_lcd.h"

static int open_lcd(int total_width, int total_height,int left,int top,int width,int height)
{
	return init_lcd_video(FORMAT_4_2_0, total_width, total_height, left,top,width,height,1);
}

static int input_lcd(void *addr, int width, int height)
{
	return output_frame(addr, width, height);
}

#ifdef __WINCE
#elif RVDS
#elif FHW
#else
static int input_lcd_dr(void *addr, int width, int height)
{
	return output_frame_dr(addr, width, height);
}
#endif
static int close_lcd()
{
	return deinit_lcd_video();
}
static Void CB_fnStoreOutFrmDut( U8 * _puchLumY, 
                   U8 * _puchChrU, 
                   U8 * _puchChrV, 
				   I32 _iFrmWidth, 
                   I32 _iFrmHeight, 
                   I32 _iStrideLX, 
                   I32 _iStrideLY,
                   I32 _iStrideUV,
				   I32 _iTopOffset,
				   I32 _iLeftOffset)
{
	U8 *puchLumY;
	U8 *puchChrU;
	U8 *puchChrV;
    I32 iWidthY = _iFrmWidth;
    I32 iHeightY = _iFrmHeight;
    I32 iWidthUV = iWidthY >> 1;
    I32 iHeightUV = iHeightY >> 1;

	if(g_ioParams.saveYUV && g_fdut)
	{
		puchLumY = _puchLumY + (_iTopOffset * _iStrideLX + _iLeftOffset);
		puchChrU = _puchChrU + (_iTopOffset * _iStrideUV / 2 + _iLeftOffset / 2);
		puchChrV = _puchChrV + (_iTopOffset * _iStrideUV / 2 + _iLeftOffset / 2);

		CB_fnWriteFile( g_fdut, puchLumY, iWidthY, iHeightY, _iStrideLX);
		CB_fnWriteFile( g_fdut, puchChrU, iWidthUV, iHeightUV, _iStrideUV);
		CB_fnWriteFile( g_fdut, puchChrV, iWidthUV, iHeightUV, _iStrideUV);
	}
	if(g_ioParams.display)
	{
		if(!g_lcd_open)
		{
			//open lcd
			//printf("xstride: %d, ystride: %d, left: %d, top: %d, width: %d, height: %d \r\n",_iStrideLX, _iStrideLY,_iLeftOffset,_iTopOffset,iWidthY,iHeightY);
			if(-1==open_lcd(_iStrideLX, _iStrideLY,_iLeftOffset,_iTopOffset,iWidthY,iHeightY)) return;
			g_lcd_open = 1;
		}
#if 1		
		//g_last_width and g_last_height !=0
		if((iWidthY!=g_last_width)||(iHeightY!=g_last_height)) //eagle for size change !!
		{
			printf("size change !!! \r\n");
			deinit_lcd_video();
			if(-1==open_lcd(_iStrideLX, _iStrideLY,_iLeftOffset,_iTopOffset,iWidthY,iHeightY))
			{
				printf("size change and re-init LCD fail \r\n ");
				return;
			}
		}
#endif		
#ifdef __WINCE
		input_lcd_crop(_puchLumY, _iStrideLX, _iStrideLY,_iLeftOffset,_iTopOffset,iWidthY,iHeightY);
#else
		input_lcd(_puchLumY, _iStrideLX, _iStrideLY);
#endif
	}
}

#ifdef __WINCE
#elif RVDS
#elif FHW
#else
static Void CB_fnStoreOutFrmDut_DR( U8 * _puchLumY, 
                   U8 * _puchChrU, 
                   U8 * _puchChrV, 
				   I32 _iFrmWidth, 
                   I32 _iFrmHeight, 
                   I32 _iStrideLX, 
                   I32 _iStrideLY,
                   I32 _iStrideUV,
				   I32 _iTopOffset,
				   I32 _iLeftOffset)
{
	U8 *puchLumY;
	U8 *puchChrU;
	U8 *puchChrV;
    I32 iWidthY = _iFrmWidth;
    I32 iHeightY = _iFrmHeight;
    I32 iWidthUV = iWidthY >> 1;
    I32 iHeightUV = iHeightY >> 1;

	if(g_ioParams.saveYUV && g_fdut)
	{
		puchLumY = _puchLumY + (_iTopOffset * _iStrideLX + _iLeftOffset);
		puchChrU = _puchChrU + (_iTopOffset * _iStrideUV / 2 + _iLeftOffset / 2);
		puchChrV = _puchChrV + (_iTopOffset * _iStrideUV / 2 + _iLeftOffset / 2);

		CB_fnWriteFile( g_fdut, puchLumY, iWidthY, iHeightY, _iStrideLX);
		CB_fnWriteFile( g_fdut, puchChrU, iWidthUV, iHeightUV, _iStrideUV);
		CB_fnWriteFile( g_fdut, puchChrV, iWidthUV, iHeightUV, _iStrideUV);
	}
	if(g_ioParams.display)
	{
		if(!g_lcd_open)
		{
			printf("Error : LCD is not open for DR\n");
			return;
		}
		//g_last_width and g_last_height !=0
		if((iWidthY!=g_last_width)||(iHeightY!=g_last_height)) //eagle for size change !!
		{
			printf("size change , re-init LCD \r\n");
			deinit_lcd_video();
			usleep(10000);
			if(-1==open_lcd(_iStrideLX, _iStrideLY,_iLeftOffset,_iTopOffset,iWidthY,iHeightY))
			{
				printf("size change and re-init LCD fail \r\n ");
				return;
			}
		}
		
#ifndef __WINCE
		input_lcd_dr(_puchLumY, _iStrideLX, _iStrideLY);
#endif
	}
}
#endif
static int g_total_bits_len = 0;

static Void CB_fnBitsUnitlen(bits_len)
{
 	g_total_bits_len +=bits_len;
}

static FILE *g_flog = NULL;

#ifdef __WINCE         // for taking timing on wince platform

    LARGE_INTEGER lpFrequency1 ;
    LARGE_INTEGER lpPerformanceCount1;
    LARGE_INTEGER lpPerformanceCount2;
    __int64 TotalDecTime=0;
    __int64 Temp, FrameTime=0;

static void timer_init()
{
	FrameTime = 0;
	TotalDecTime = 0;
}

static void timer_start()
{
    //  INTERRUPTS_SET(0xDF) ; 	//disable interrupt
    QueryPerformanceFrequency(&lpFrequency1);
    QueryPerformanceCounter(&lpPerformanceCount1);
}

static void timer_stop(int more_flag)
{
     QueryPerformanceCounter(&lpPerformanceCount2);
     Temp=(((lpPerformanceCount2.QuadPart - lpPerformanceCount1.QuadPart)*1000000)/(lpFrequency1.QuadPart));
     TotalDecTime += Temp;
	 FrameTime += Temp;
	 if(g_flog && g_ioParams.putLog && !more_flag)
	 {
	 	fprintf(g_flog, "%d\r\n",FrameTime);
		FrameTime= 0;
	 }
//	 INTERRUPTS_SET(0x1F) ;  //enable interrupt
}

static void timer_report()
{
    printf("Decoding Time: %ld us\n", TotalDecTime);
}

static unsigned long get_decoding_time()
{
	return TotalDecTime;
}

#elif RVDS

int prev_clk, curr_clk, clk,frame_no=0;
int clk_pf[150];
int cbk_clk1, cbk_clk2;
int total_clk =0,max_clk=0,avg_clk=0,frame=0;
int cycle_measure=1;
int init_cache=0; 
extern int prev_cycles(void);
extern int curr_cycles(void);

static void timer_init()
{
	total_clk=0;
}

static void timer_start()
{
	prev_clk = prev_cycles();
}

static void timer_stop(int more_flag)
{
	curr_clk = curr_cycles();
	clk = (curr_clk-prev_clk);  /* clk gives the total core cycles per	frame call */
	total_clk += clk;	
	if(clk > max_clk)
	{
		max_clk = clk;
	}
}

static void timer_report()
{
	printf("total_clk: %d, total_us(total_clk*64/532): %d , max_clk: %d \r\n",total_clk,total_clk*64/532,max_clk);
}

static unsigned long get_decoding_time()
{
	// unit: us
	return (total_clk*64/532);
}
#else

//struct timeval tv_prof1, tv_prof2,tv_prof3, tv_prof4;
//long time_frame;    // frame duration (until a new frame arrive)
//struct timeval tv_prog_1, tv_prog_2; // program duration
//long time_before=0, time_after=0,Time=0, Time_NAL=0;

// profiling total time - from program start to program end
struct timeval tm_prog1, tm_prog2;
static unsigned long duration_prog = 0;
// profiling decoding time
struct timeval tm_dec1, tm_dec2;
static unsigned long duration_dec = 0;
// profiling NAL reading time
struct timeval tm_nal1, tm_nal2;
static unsigned long duration_nal = 0;

// init all timers
static void timer_init()
{
    duration_prog = 0;
    duration_dec = 0;
    duration_nal = 0;
}

static void timer_start()
{
    gettimeofday(&tm_dec1, 0);
}

static void timer_stop(int more_flag)
{
    unsigned long tm_1, tm_2;

    gettimeofday(&tm_dec2, 0);
	
    tm_1 = tm_dec1.tv_sec * 1000000 + tm_dec1.tv_usec;
    tm_2 = tm_dec2.tv_sec * 1000000 + tm_dec2.tv_usec;
    duration_dec = duration_dec + (tm_2-tm_1);
	duration_nal += (tm_2-tm_1);
	if(g_flog && g_ioParams.putLog && !more_flag)
	{
	    fprintf(g_flog, "%d\r\n", duration_nal);
		duration_nal = 0;
	}
}

static void timer_report()
{
    printf("Decoding Time: %ld us\n", duration_dec);
}

static unsigned long get_decoding_time()
{
	return duration_dec;
}

#endif

static Void CB_fnStoreBeginTime()
{
 	timer_start();
}
static Void CB_fnStoreEndTime(int more_flag)
{
 	timer_stop(more_flag);
}
static Void CB_fnStackTag()
{
	if(!g_ioParams.memFlag) return;
 	STACK_TAG();
}
static Void CB_fnStackUpdate()
{
	if(!g_ioParams.memFlag) return;
 	STACK_UPDATE();
}
static Void CB_fnStackCallbackTag()
{
	if(!g_ioParams.memFlag) return;
 	STACK_CALLBACK_TAG();
}

static Void CB_fnStackCallbackUpdate()
{
	if(!g_ioParams.memFlag) return;
 	STACK_CALLBACK_RESTORE();
}

static Void CB_fnHeapIncrease(int size)
{
	if(!g_ioParams.memFlag) return;
 	HEAP_INCREASE(size);
}

static Void CB_fnHeapDecrease(int size)
{
	if(!g_ioParams.memFlag) return;
	HEAP_DECREASE(size);
}
#ifdef __WINCE
#elif RVDS
#elif FHW
#else

int g_buffer_number =0;
static Void CB_fnInitLCD(int video_width, int video_height, int left,int top,int width,int height,int frame_number)
{
#ifndef __WINCE 
	//open lcd
	g_buffer_number = frame_number;
	if(-1==open_lcd(video_width, video_height,left,top,width,height)) return;
	g_lcd_open = 1;
#endif
}

static long CB_fnGetBuffer()
{
#ifndef __WINCE
	return (long)get_buffer_dr();
#else
	return 0;
#endif
}

static void CB_fnReleaseBuffer(void *buffer)
{
#ifndef __WINCE
	release_buffer_dr(buffer);
#endif
}

static long CB_fnGetPhyAddr(void *buffer)
{
#ifndef __WINCE
	return (long)get_physical_addr(buffer);
#else
	return 0;
#endif
}

#endif

PROBE_DUT_FUNC fnTestDut_prb;

long fnTestDut_prb(T_PROBE_TYPE prb_type, void *para)
{	
	T_PROBE_INIT_LCD *pPrbInitLcd;
	T_PROBE_INIT_BUFFER *pPrbInitBuf;
	T_PROBE_PUT_FRAME *pPrbPutFrm;
	
	switch(prb_type)
	{
	case T_PUT_FRAME:
		pPrbPutFrm = (T_PROBE_PUT_FRAME *)para;
		if(pPrbPutFrm)
		CB_fnStoreOutFrmDut(pPrbPutFrm->puchLumY,
			                pPrbPutFrm->puchChrU, 
			                pPrbPutFrm->puchChrV, 
							pPrbPutFrm->iFrmWidth, 
			                pPrbPutFrm->iFrmHeight, 
			                pPrbPutFrm->iStrideLX, 
			                pPrbPutFrm->iStrideLY,
			                pPrbPutFrm->iStrideUV,
							pPrbPutFrm->iTopOffset,
							pPrbPutFrm->iLeftOffset);
		break;
	case T_BITS_UNIT_LEN:
		CB_fnBitsUnitlen((int)para);
		break;
	case T_DECODE_BEGIN:
		CB_fnStoreBeginTime();
		break;
	case T_DECODE_END:
		CB_fnStoreEndTime((int)para);
		break;
	case T_STACK_TAG:
		CB_fnStackTag();
		break;
	case T_STACK_UPDATE:
		CB_fnStackUpdate();
		break;
	case T_STACK_CB_TAG:
		CB_fnStackCallbackTag();
		break;
	case T_STACK_CB_UPDATE:
		CB_fnStackCallbackUpdate();
		break;
	case T_HEAP_INCREASE:
		CB_fnHeapIncrease((int)para);
		break;
	case T_HEAP_DECREASE:
		CB_fnHeapDecrease((int)para);
		break;
#ifdef __WINCE
#elif RVDS
#elif FHW
#else

	case T_INIT_LCD:
		pPrbInitLcd = (T_PROBE_INIT_LCD*)para;
		if(pPrbInitLcd)
		{
			CB_fnInitLCD(pPrbInitLcd->video_width, pPrbInitLcd->video_height,0,0, pPrbInitLcd->video_width, pPrbInitLcd->video_height,pPrbInitLcd->frame_number);
		}
		break;
	case T_INIT_BUFFER:	
		pPrbInitBuf = (T_PROBE_INIT_BUFFER*)para;
		if(pPrbInitBuf)
		{
			CB_fnInitLCD(pPrbInitBuf->total_width, pPrbInitBuf->total_height,pPrbInitBuf->left,pPrbInitBuf->top,pPrbInitBuf->width,pPrbInitBuf->height,pPrbInitBuf->frame_number);
		}
		break;		
	case T_REJ_BUFFER:	// reject buffer
		//do nothing
		break;		
	case T_FREE_BUFFER:	//free all buffers
		//do nothing 
		break;
	case T_GET_BUFFER:
		return CB_fnGetBuffer();
		break;
	case T_REL_BUFFER:
		CB_fnReleaseBuffer((void*)para);
		break;
	case T_GET_PHY_ADDR:
		return CB_fnGetPhyAddr((void*)para);
		break;
	case T_PUT_FRAME_DR:
		pPrbPutFrm = (T_PROBE_PUT_FRAME *)para;
		if(pPrbPutFrm)
		CB_fnStoreOutFrmDut_DR(pPrbPutFrm->puchLumY,
							pPrbPutFrm->puchChrU, 
							pPrbPutFrm->puchChrV, 
							pPrbPutFrm->iFrmWidth, 
							pPrbPutFrm->iFrmHeight, 
							pPrbPutFrm->iStrideLX, 
							pPrbPutFrm->iStrideLY,
							pPrbPutFrm->iStrideUV,
							pPrbPutFrm->iTopOffset,
							pPrbPutFrm->iLeftOffset);
		break;
#endif
	default:
		break;
	}
};

static Void * gpDllDut = NULL;
static T_DEC_FUNC DecFuncDut;
static int g_frame_number = 0;
static int decode_stream(T_DEC_CONTXT_DUT *pDecContxt)
{
    void *pDecObj = NULL;
    T_DEC_RETURN_DUT iRetVal;

	fprintf(stderr, "\r[%d]", pDecContxt->uiFrameNum);
    fflush(stderr);
	
	iRetVal = DecFuncDut.ptrVideoTestDecInit( &pDecObj, pDecContxt);
	if(iRetVal != T_DEC_INIT_OK_DUT) return -1;

	iRetVal = DecFuncDut.ptrVideoTestDecSetProbes(pDecObj, fnTestDut_prb);
	if(iRetVal != T_SET_PROB_OK_DUT) return -1;

    while (iRetVal != T_DEC_ALLOUT_DUT && pDecContxt->uiFrameNum < g_frame_number)
    {   
        iRetVal = DecFuncDut.ptrVideoTestDecFrame( pDecObj, pDecContxt);
		fprintf(stderr, "\r[%d]", pDecContxt->uiFrameNum);
        fflush(stderr);
    }
	
    iRetVal = DecFuncDut.ptrVideoTestDecRelease( pDecObj, pDecContxt );
	if(iRetVal != T_DEC_REL_OK_DUT) return -1;
	
    return 0;
}

/* Macro that sets all the decoder api funtion pointers */
#ifdef RVDS
#define SET_DEC_FUN_PTR( pDll, pDecFunc, LibName, DecInit, DecFrame, DecRelease, SetProbes ) \
{ \
	(pDecFunc)->ptrVideoTestDecInit = DecInit; \
	(pDecFunc)->ptrVideoTestDecFrame =  DecFrame; \
	(pDecFunc)->ptrVideoTestDecRelease =  DecRelease; \
	(pDecFunc)->ptrVideoTestDecSetProbes = SetProbes; \
}

#else
#define SET_DEC_FUN_PTR( pDll, pDecFunc, LibName, DecInit, DecFrame, DecRelease, SetProbes ) \
{ \
	pDll = dlopen(LibName, 1); \
	if(!pDll) \
	{ \
		printf("can't load %s: %s.\n", LibName, dlerror()); \
		return -1; \
	} \
	(pDecFunc)->ptrVideoTestDecInit =      dlsym(pDll, #DecInit); \
	(pDecFunc)->ptrVideoTestDecFrame =     dlsym(pDll, #DecFrame); \
	(pDecFunc)->ptrVideoTestDecRelease =   dlsym(pDll, #DecRelease); \
	(pDecFunc)->ptrVideoTestDecSetProbes = dlsym(pDll, #SetProbes); \
}
#endif

#define FREE_DEC_FUN_PTR( pDll, pDecFunc, DecInit, DecFrame, DecRelease, SetProbes) \
{\
	dlclose(pDll);\
	(pDecFunc)->ptrVideoTestDecInit =      NULL; \
	(pDecFunc)->ptrVideoTestDecFrame =     NULL; \
	(pDecFunc)->ptrVideoTestDecRelease =   NULL; \
	(pDecFunc)->ptrVideoTestDecSetProbes = NULL; \
}

#define STACK_REVISION_VAL 84

int main(int argc, char **argv)
{
	String strLibName = NULL;
	char outfile[NAME_SIZE];
	T_DEC_CONTXT_DUT DecContxt;

	if(argc < 5)
	{
		usage(argv[0]);
		return -1;
	}

	GetUserInput(&g_ioParams, argc, argv);

	// output the APP Version Info
      printf("%s \n", App_CodecVersionInfo());

	timer_init();

	memset(&DecFuncDut, 0, sizeof(DecFuncDut));

	strLibName = g_ioParams.dutlib;

    printf( " DUT library:                \t%s\n", strLibName );

    /* open library and set function pointers */
    SET_DEC_FUN_PTR( gpDllDut, &DecFuncDut, strLibName, VideoTestDecInit_dut, 
        VideoTestDecFrame_dut, VideoTestDecRelease_dut, VideoTestDecSetProbes_dut);

	if(g_ioParams.memFlag)
	{
		HEAP_INIT();
		STACK_INIT();
	}

	if(g_ioParams.saveYUV)
	{
		g_fdut = fopen(g_ioParams.outfile, "wb");
		if(!g_fdut)
		{
			printf("can not open output file %s.\n", g_ioParams.outfile);
			return -1;
		}
	}

	if(g_ioParams.putLog)
	{
		g_flog = fopen(g_ioParams.logfile, "wb");
		if(!g_flog)
		{
			printf("can not open time log file %s.\n", g_ioParams.logfile);
			return -1;
		}
	}

	//printf(" input bitstream : %s.\n",g_ioParams.infile);
	//printf(" frame_number : %d, display: %d.\n",g_ioParams.maxnum, g_ioParams.display);
	//printf(" memFlag : %d, saveYUV : %d\n",g_ioParams.memFlag,g_ioParams.saveYUV);

	DecContxt.argc = g_ioParams.wp_argc;
	DecContxt.argv = g_ioParams.wp_argv;
	
	DecContxt.strInFile = g_ioParams.infile;
	
	DecContxt.uiHeight = 0;
	DecContxt.uiWidth = 0;
	DecContxt.uiBitRate = 0;
	DecContxt.uiFrameRate = 0;
	DecContxt.uiFrameNum = 0;

	g_frame_number = g_ioParams.maxnum;
	if(!g_frame_number) g_frame_number = 0x7FFFFFFF;

	g_total_bits_len = 0;

	decode_stream(&DecContxt);

    /* free library and  function pointers */
    FREE_DEC_FUN_PTR( gpDllDut, &DecFuncDut, VideoTestDecInit_dut, 
        VideoTestDecFrame_dut, VideoTestDecRelease_dut, VideoTestDecSetProbes_dut);
	
	if(g_ioParams.display)
	{
		if(g_lcd_open)
		{
			close_lcd();
			g_lcd_open = 0;
		}
	}
	
#ifdef STACK_MEASURE	
	g_StackMeasure.Peak += STACK_REVISION_VAL;
#endif

	if(!DecContxt.uiBitRate)
	{
		if(DecContxt.uiFrameNum)
		{
			DecContxt.uiBitRate = (unsigned int)((double)g_total_bits_len * (double)DecContxt.uiFrameRate /((double)DecContxt.uiFrameNum));
		}
			
	}
		
	//timer_report();
	printf("FrameWidth = %d\r\n",DecContxt.uiWidth);
	printf("FrameHeight = %d\r\n",DecContxt.uiHeight);
	printf("BitRate = %d bps\r\n",DecContxt.uiBitRate);
	printf("FrameRate = %d fps\r\n",DecContxt.uiFrameRate);
	
	printf("FrameNumber = %d\r\n",DecContxt.uiFrameNum);
	printf("DecodeTime = %d us\r\n",get_decoding_time());


#ifdef STACK_MEASURE		
	if(g_ioParams.memFlag)
	{
	   // STACK_PRINTF();
		//HEAP_PRINTF();
		printf("StackPeak = %d bytes\r\n", g_StackMeasure.Peak);
	    printf("HeapSize = %d bytes\r\n", g_HeapMeasure.TotalSize);
	}
#endif

	if(g_ioParams.saveYUV)
	{
		if(g_fdut) fclose(g_fdut);
		g_fdut = NULL;
	}

	if(g_ioParams.putLog)
	{
		if(g_flog) fclose(g_flog);
		g_flog = NULL;
	}

	if(g_ioParams.saveResult)
	{
		FILE *fres = fopen(g_ioParams.resfile, "wb");
		if(!fres)
		{
			printf(" can not open file %s.\n", g_ioParams.resfile);
		}
		else
		{
			fprintf(fres,"FrameWidth = %d\r\n",DecContxt.uiWidth);
			fprintf(fres,"FrameHeight = %d\r\n",DecContxt.uiHeight);
			fprintf(fres,"BitRate = %d bps\r\n",DecContxt.uiBitRate);
			fprintf(fres,"FrameRate = %d fps\r\n",DecContxt.uiFrameRate);
			
			fprintf(fres,"FrameNumber = %d\r\n",DecContxt.uiFrameNum);
			fprintf(fres,"DecodeTime = %d us\r\n",get_decoding_time());
#ifdef STACK_MEASURE
			if(g_ioParams.memFlag)
			{
				fprintf(fres,"StackPeak = %d bytes\r\n", g_StackMeasure.Peak);
				fprintf(fres,"HeapSize = %d bytes\r\n", g_HeapMeasure.TotalSize);
			}
#endif			
			fclose(fres); 
		}

	}

	return 0;
}

#ifdef __WINCE

int _tmain(int argc,_TCHAR *argv[])
{
	char* argv_char[NAME_SIZE];
	int argc_size,i;

#if( _WIN32_WCE == 500 ) //xianzhong	//1set control to enable unaligned access: c1:bit22(U bit =1,enable) bit1(A bit=0, disable check)
{
	unsigned long cp15_c1;
	cp15_c1=CONTROL_REG_READ();	
	//printf("cp15= 0x%X \r\n",cp15_c1);
	cp15_c1=cp15_c1&0xFFFFFFFD;		// clear bit 1 (A bit)
	cp15_c1=cp15_c1|0x00400000;		// set bit 22 (U bit)
	CONTROL_REG_SET(cp15_c1);
	cp15_c1=CONTROL_REG_READ();
	//printf("cp15= 0x%X \r\n",cp15_c1);	
}
#endif

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
