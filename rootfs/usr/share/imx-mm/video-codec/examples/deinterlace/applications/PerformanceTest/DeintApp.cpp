/*
 ***********************************************************************
 * Copyright 2005-2008,2011 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*
 * Application to test deinterlace on linux
 *
 * History
 *   Date          Changed                                Changed by
 *   Sep. 6, 2007  Create                                 Zhenyong Chen
 *   Sep. 18, 2007 Porting from VC project                Zhenyong Chen
 *   Jan. 30, 2008 Support version, and use new API       Zhenyong Chen
 *                 InitPlatformEx
 *   Mar. 28, 2008 Add wince support                      Zhenyong Chen
 *   Jun. 27, 2008 Print version information using new    Zhenyong Chen
 *                 API
 *   Jul. 23, 2008 Remove SoC related code; fix strcmp    Zhenyong Chen
 *                 bug (in function GetUserInput) using
 *                 strncmp, for armcc 3.1, arm11lervds
 */
#include "common.h"
#ifdef _LINUX
#include <sys/time.h>
#include <unistd.h>
#endif
#ifdef UNDER_CE
#include <winbase.h>
#endif
//#include "RawMovieFile.h"
//#include "ProcCtrl.h"
#include "Deinterlace_types.h"
#include "Deinterlace_Safe.h"
#include "Deinterlace_UnSafe.h"
#include "Deinterlace.h"

#ifdef FULL_RELEASE
#include "ColorConversion_UnSafe.h"
#endif

#include "RawMovieViewView.h"

// Version
static char sVersion[72];

// Video parameter set by user
STREAMINFO si;

//-------------------------------------------------------------------------
// IO
void Usage()
{
    int i;
    PRINTF("\nUsage: <prog> [options] bitstream_file\n");
    PRINTF("options:\n"
       "    -fmt <YUV|CbYCrY|KEV> : Input bitstream format, YUV, CbYCrY or KEV. Default to YUV\n"
       "    -notinterleaved : Fields are not interleaved\n"
       "    -w <width> : Width of input video, in pixel.\n"
       "    -h <height> : Height of input video, in pixel.\n"
       "    -chrom <chrom_format> : Chroma format. Can take 4:2:0, 4:2:2 or 4:4:4\n"
       "    -dint <deinterlace method> : Method of deinterlace.\n"
       "    -topfirst : Set if first field is top field. \n"
       "    -s <save_yuv> : Save processed YUV to file in YUV format.\n"
       "    -n <number> : Decode n pictures\n");
    PRINTF("De-interlacing methods:\n");

    for(i=0; i<GetMethodCount(); i++)
    {
        unsigned int id = MethodFromPosition(i);
        PRINTF("  %d : %s\n", id, GetMethodName(id));
    }
}

#define CASE_FIRST(x)   if (strncmp(argv[0], x, strlen(x)) == 0)
#define CASE(x)         else if (strncmp(argv[0], x, strlen(x)) == 0)
#define DEFAULT         else

static int picture_to_decode;
char save_yuv[_MAX_PATH+1];

// For Windows CE, path name may consist quote marks during
// command line parse. This function is to remove them
static void FormatPathName(char *sPathFile)
{
    int len;
    len = strlen(sPathFile);
    if(sPathFile[len-1] == '\'' || sPathFile[len-1] == '\"')
        sPathFile[len-1] = 0;
    if(sPathFile[0] == '\'' || sPathFile[0] == '\"')
    {
        len = strlen(sPathFile+1);
        memcpy(sPathFile, sPathFile+1, len+1); // including '\0'
    }
}

static int GetUserInput(STREAMINFO *psi, int argc, char *argv[])
{
    // Defaults
    memset(psi, 0, sizeof(STREAMINFO));
    psi->bTopFirst = FALSE;
    psi->bFieldsInterleaved = TRUE;
    psi->bDeinterlace = FALSE;
    picture_to_decode = 25;
    save_yuv[0] = 0;

    argc--;
    argv++;
    while (argc>0)
    {
        if (argv[0][0] == '-')
        {
            CASE_FIRST("-fmt")                    // Has to be first
            {
                argc--;
                argv++;
                if(argv[0] != NULL)
                {
                    // Walkround for armcpp
                    // Do not use strcmp(...) for this statement
                    if(strncmp(argv[0], "YUV", 3) == 0)
                        psi->StreamFormat = STREAM_YUV;
                    else if(strcmp(argv[0], "CbYCrY") == 0)
                        psi->StreamFormat = STREAM_CbYCrY;
                    else if(strcmp(argv[0], "KEV") == 0)
                        psi->StreamFormat = STREAM_KEV;
                    else
                        return -1;
                }
                else
                    return -1;
            }
            CASE("-notinterleaved")
            {
                psi->bFieldsInterleaved = FALSE;
            }
            CASE("-w")
            {
                argc--;
                argv++;
                if (argv[0] != NULL)
                    sscanf(argv[0], "%d", &psi->nWidth);
                else
                    return -1;
            }
            CASE("-h")
            {
                argc--;
                argv++;
                if (argv[0] != NULL)
                    sscanf(argv[0], "%d", &psi->nHeight);
                else
                    return -1;
            }
            CASE("-chrom")
            {
                argc--;
                argv++;
                if(argv[0] != NULL)
                {
                    // Walkround for armcpp
                    // Do not use strcmp(...) for this statement
                    if(strncmp(argv[0], "4:2:0", 5) == 0)
                        psi->nChromaFormat = CHROM_FMT_420;
                    else if(strcmp(argv[0], "4:2:2") == 0)
                        psi->nChromaFormat = CHROM_FMT_422;
                    else if(strcmp(argv[0], "4:4:4") == 0)
                        psi->nChromaFormat = CHROM_FMT_444;
                    else
                        return -1;
                }
                else
                    return -1;
            }
            CASE("-s")
            {
                argc--;
                argv++;
                if(argv[0] != NULL)
                {
                    strncpy(save_yuv, argv[0], _MAX_PATH);
                    save_yuv[_MAX_PATH-1] = 0;
                    FormatPathName(save_yuv);
                }
                else
                    return -1;
            }
            CASE("-dint")
            {
                argc--;
                argv++;
                psi->bDeinterlace = TRUE;
                if (argv[0] != NULL)
                {
                    sscanf(argv[0], "%d", &psi->nDeintMethodID);
                }
                else
                    return -1;
            }
            CASE("-topfirst")
            {
                psi->bTopFirst = TRUE;
            }
            CASE("-n")
            {
                argc--;
                argv++;
                if (argv[0] != NULL)
                    sscanf(argv[0], "%d", &picture_to_decode);
                else
                    return -1;
            }
            DEFAULT                             // Has to be last
            {
                PRINTF("Unsupported option %s\n", argv[0]);
                return -1;
            }
        }
        else
        {
            strncpy(psi->sPathName, argv[0], _MAX_PATH-1);
            psi->sPathName[_MAX_PATH-1] = 0;
            FormatPathName(psi->sPathName);
        }
        argc--;
        argv++;
    }
    if (psi->sPathName[0] == 0)
    {
        return -1;
    }
    else
        return 0;
}
//------------------------------------------------------------------
// Profile
#ifdef TIME_PROFILE

// Variables to store counter

#ifdef _LINUX
// profiling total time - from program start to program end
struct timeval tm_prog1, tm_prog2;
unsigned long duration_prog = 0;
// profiling decoding time
struct timeval tm_dec1, tm_dec2;
unsigned long duration_dec = 0;
#endif // _LINUX

#ifdef UNDER_CE
LARGE_INTEGER lpFrequency1 ;
LARGE_INTEGER lpPerformanceCount1;
LARGE_INTEGER lpPerformanceCount2;
unsigned long duration_dec = 0;
#endif // UNDER_CE

#endif // TIME_PROFILE

// init all timers
void timer_init()
{
#ifdef _LINUX
    duration_prog = 0;
    duration_dec = 0;
#endif
#ifdef UNDER_CE
    duration_dec = 0;
#endif
}

void timer_start(void *tm)
{
#ifdef _LINUX
    gettimeofday((struct timeval *)tm, 0);
#endif
#ifdef UNDER_CE
    QueryPerformanceFrequency(&lpFrequency1);
    QueryPerformanceCounter((LARGE_INTEGER *)tm);
#endif
}

void timer_stop(void *tmbefore, void *tmafter, unsigned long *total)
{
#ifdef _LINUX
    unsigned long tm_1, tm_2;
    gettimeofday((struct timeval *)tmafter, 0);
    tm_1 = ((struct timeval *)tmbefore)->tv_sec * 1000000 + ((struct timeval *)tmbefore)->tv_usec;
    tm_2 = ((struct timeval *)tmafter)->tv_sec * 1000000 + ((struct timeval *)tmafter)->tv_usec;
    *total = *total + (tm_2-tm_1);
#endif
#ifdef UNDER_CE
    int delta;
    QueryPerformanceCounter((LARGE_INTEGER *)tmafter);
    delta = (int)(((((LARGE_INTEGER *)tmafter)->QuadPart - ((LARGE_INTEGER *)tmbefore)->QuadPart)*1000000)/(lpFrequency1.QuadPart));
    *total = *total + delta;
#endif
}
void timer_report(char *who, unsigned long tm)
{
#ifdef _LINUX
    PRINTF("%s costs time: %ld us\n", who, tm);
#endif
#ifdef UNDER_CE
    PRINTF("%s costs time: %ld us\n", who, tm);
#endif
}

void deinterlace_start()
{
#ifdef _LINUX
    timer_start(&tm_dec1);
#endif
#ifdef UNDER_CE
    timer_start(&lpPerformanceCount1);
#endif
}
void deinterlace_stop()
{
#ifdef _LINUX
    timer_stop(&tm_dec1, &tm_dec2, &duration_dec);
#endif
#ifdef UNDER_CE
    timer_stop(&lpPerformanceCount1, &lpPerformanceCount2, &duration_dec);
#endif
}

int main(int argc, char *argv[])
{
    // display version information
    PRINTF(GetDeinterlaceVersionInfo());
    PRINTF("\r\n");

    STRCPY(sVersion, "unknown");

    PRINTF( "(C) Copyright 2007 Freescale semiconductor, SHA\n\n"
            "Program to deinterlace interlaced video to progressive video.\n");

    CRawMovieViewView view;
    int i;

    // Init color conversion and deinterlace
#ifdef FULL_RELEASE
    InitYUV2RGBConversion();
#endif
    InitDeinterlace();

    // Parse command line
    PRINTF("Input command is:\n");
    for(i=0; i<argc; i++)
        PRINTF("%s ", argv[i]);
    PRINTF("\n");
    if(GetUserInput(&si, argc, argv) == -1)
    {
        Usage();
        return -1;
    }

    // Reset timer value to zero
    timer_init();

    // Check whether output file is needed
    if(save_yuv[0] != 0)
    {
        view.OnSaveResult(save_yuv);
    }

    // File open
    view.OpenFile("dummy");

    // Deinterlace input video
    PRINTF("\n");
#ifdef _LINUX
    timer_start(&tm_prog1);
#endif

    if(view.m_bOpened)
    {
        for(i=0; i<picture_to_decode; i++)
        {
            view.OnNext();
        }
    }
    else
    {
        PRINTF("Failed to play.\n");
    }

#ifdef _LINUX
    timer_stop(&tm_prog1, &tm_prog2, &duration_prog);
#endif
    PRINTF("\n");

#ifdef _LINUX
    timer_report("Total time", duration_prog);
    timer_report("Decoding time", duration_dec);
#endif
#ifdef UNDER_CE
    timer_report("Decoding time", duration_dec);
#endif

    // Close file if opened
    view.OnSaveResult(NULL);

    return 0;
}

