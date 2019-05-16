/*
 ***********************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc.
 ***********************************************************************
 */
 /***************************************************************************** 
** H264_dec_dut.c 
** 
** 
** Description: Contains the API functions that are invoked by VTS.
**
** Revision History: 
** ----------------- 
** 1.0  XXXX  XianZhong   create this file
** 2.0  05/20/2010  Eagle Zhou  sync wrapper version 
*****************************************************************************/ 

//Do not move this

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef USE_DLL
#define IMPORT_DLL
#endif
#include "avcd_dec_api.h"
#ifdef USE_DLL
#undef IMPORT_DLL
#endif
#include "dirUtils.h"
#include "io.h"
#include "dut_probes_vts.h"
#include "dut_api_vts.h"
#include "dut_test_api.h"

#ifdef TGT_OS_WINCE	//eagle
//#define inline __inline
//#define NULL	0
#include <malloc.h>
//extern void INTERRUPTS_ON();
//extern void INTERRUPTS_OFF();
//extern void INTERRUPTS_SET(unsigned long);
//extern unsigned long INTERRUPTS_STATE();
#define INTERRUPTS_ON()
#define INTERRUPTS_OFF()
#define INTERRUPTS_SET(val)
//#define INTERRUPTS_STATE() 0xDF

#define VTS_PRINTF()

#else
#define VTS_PRINTF(...)
#endif

#define DEBUG_APP 0

#define MEMCPY memcpy
#define MEMSET memset
#define MALLOC malloc
#define CALLOC calloc
#define REALLOC realloc
#define EXIT   exit

#if defined(SUPPORT_MSE) && (SUPPORT_MSE == 0)
//#  include <math.h>
#endif

#define ENABLE_HW_DEBLOCK

#if defined (SW_DEBLOCK) || (defined(HW_DEBLOCK) && (HW_DEBLOCK==0))
#undef ENABLE_HW_DEBLOCK
#endif


//#ifdef ENABLE_TIME_PROFILE
//#undef USE_DISPLAY
//#undef SAVE_KEV
//#undef SAVE_RGB
//#undef SAVE_UYVY
//#include <sys/time.h>
//#endif
#ifdef TGT_OS_WINCE
#include <windows.h>
#endif

#define DEBUG_APP     0

#ifdef _FSL_VTS
#include "dut_api_vts.h"
#endif

#include "dut_test_api.h"

#define BUFFER_NUM 17
#define MAX_NAL_LEN  2100000
typedef struct
{
	int argc;
	char **argv;
    IOParams ioPars;
    sAVCDecoderConfig vdec;
    eAVCDRetType eStatus;
    int ff_flag;
    int ff_flag_prev;
    int ffcnt;
    int ff_taken;
	int ffglobal, ff_enable;
	int s32NumBytesGlobal;
	unsigned char *pAppBuffer;

// Application buffers 2 NAL
	UCHAR NALBuffer[2][MAX_NAL_LEN];
	int current_nal_index;// = -1;//Which NALBuffer is used
	int NALBufferBytes[2];//Length of NALBuffer
	int NALDeStuffed[2];//NAL destuffed flag

	int video_width;
	int video_height;

	int frame_number;

#if defined(ENABLE_HW_DEBLOCK) && defined(TGT_OS_WINCE) && ( _WIN32_WCE == 600 )
//	HANDLE pf;
#endif

	int last_width;
	int last_height;
	int last_bufNumber;

	FuncProbeDut * pfnVtsProbes;
	long (*fnPrbDutFunc)(T_PROBE_TYPE prb_type, void *para);

	UCHAR* Buffers[BUFFER_NUM];
	unsigned long PhyBuffer[BUFFER_NUM];
	int bufferIndex;

	int gBufferNum;

}H264_DEC_OBJ;

#define BUF_BITS_LEN_CB(size)	{if(pH264Obj->fnPrbDutFunc) pH264Obj->fnPrbDutFunc(T_BITS_UNIT_LEN, size);}

#define DEC_TIMER_BEGIN() {if(pH264Obj->fnPrbDutFunc) pH264Obj->fnPrbDutFunc(T_DECODE_BEGIN, 0);}
#define DEC_TIMER_END() {if(pH264Obj->fnPrbDutFunc) pH264Obj->fnPrbDutFunc(T_DECODE_END, 0);}


#define STACK_TAG() {if(pH264Obj->fnPrbDutFunc) pH264Obj->fnPrbDutFunc(T_STACK_TAG, 0);}
#define STACK_UPDATE() {if(pH264Obj->fnPrbDutFunc) pH264Obj->fnPrbDutFunc(T_STACK_UPDATE, 0);}

#define STACK_CALLBACK_TAG()
#define STACK_CALLBACK_UPDATE()

#define HEAP_INCREASE(size) {if(pH264Obj->fnPrbDutFunc) pH264Obj->fnPrbDutFunc(T_HEAP_INCREASE, size);}
#define HEAP_DECREASE(size) {if(pH264Obj->fnPrbDutFunc) pH264Obj->fnPrbDutFunc(T_HEAP_DECREASE, size);}


static void GetUserInput(H264_DEC_OBJ *pH264Obj,IOParams *pIO, Int argc, char *argv[]);
void AppAllocMemory(H264_DEC_OBJ * pH264Obj,sAVCDMemAllocInfo *psMemPtr);
void AppAllocMemory_1(H264_DEC_OBJ * pH264Obj,sAVCDMemAllocInfo *psMemPtr);
void AllocateFrameMemory(sAVCDYCbCrStruct *psFrame, sAVCDConfigInfo *pConfig, H264_DEC_OBJ * pH264Obj);
void FreeDecoderMemory(sAVCDMemAllocInfo *psMemPtr);
void InitalizeAppMemory(sAVCDMemAllocInfo *psMemPtr);
void FreeFrameMemory(sAVCDYCbCrStruct *psFrame);
int cbkAppBufRead(unsigned char* pBuf, int s32BufSize, int *last, void *pAppContext); //DSPhl27777
void Usage(H264_DEC_OBJ *pH264Obj);

void init_NAL_buffers(H264_DEC_OBJ *pH264Obj)
{
    pH264Obj->current_nal_index = 0;
    pH264Obj->NALBufferBytes[0] = pH264Obj->NALBufferBytes[1] = 0;
    pH264Obj->NALDeStuffed[0] = pH264Obj->NALDeStuffed[1] = 0;
}

void read_nal(H264_DEC_OBJ *pH264Obj, IOParams *pIOPars, int index)
{
	int bytes = 0;

    /* VTS : record bitstream input start time */
    if ( NULL != pH264Obj->pfnVtsProbes )
    {
        pH264Obj->pfnVtsProbes( E_INPUT_STARTTIME, NULL );
    }
    bytes = IO_GetNalUnitAnnexB(pIOPars, pH264Obj->NALBuffer[index], MAX_NAL_LEN);

    pH264Obj->NALDeStuffed[index] = 0;
	pH264Obj->NALBufferBytes[index] = bytes;

    /* VTS : record bitstream input end time and bistream unit length */
    if ( NULL != pH264Obj->pfnVtsProbes )
    {
        pH264Obj->pfnVtsProbes( E_INPUT_ENDTIME, NULL );
        pH264Obj->pfnVtsProbes( E_INPUT_LENGTH, &bytes );
    }

	BUF_BITS_LEN_CB((void *)(bytes * 8));

}
/***************************************************/

#if defined(ENABLE_HW_DEBLOCK)
void* BufRecord[17];
unsigned int BufPhyAddr[17];

#if defined(TGT_OS_WINCE) && ( _WIN32_WCE == 600 )
#include <windows.h>
#include <Devload.h>
#include <strsafe.h>
#include <ceddk.h>
#include <stdlib.h>
#include <tchar.h>
#include <bsp.h>
#include <pf.h>
pfAllocMemoryParams pfParams[17];
#endif
#endif

void* getBuffer(void* AppContext);
void rejectBuffer(void* buffer, void* AppContext);
int initSWBuffer(int width,int height,H264_DEC_OBJ * pH264Obj)
{
	int i,size;
	size = width*height*1.5;
	for (i = 0; i < pH264Obj->gBufferNum; i ++)
	{
		if(pH264Obj->Buffers[i]) free(pH264Obj->Buffers[i]);

		if (NULL == (pH264Obj->Buffers[i] = malloc(size)))
		{
			return 0;
		}
	}
	return 1;

}

int initHWBuffer(int width,int height,H264_DEC_OBJ * pH264Obj)
{
#if defined(TGT_OS_WINCE) && defined(ENABLE_HW_DEBLOCK)
	int i, size;

	size = (width+32) * (height+32) * 3/2;

	for (i=0;i< pH264Obj->gBufferNum;i++)
	{

#if( _WIN32_WCE == 500 ) //xianzhong
		unsigned long PhysAddr;
		BufRecord[i] = (UINT32 *) AllocPhysMem(size, PAGE_EXECUTE_READWRITE, 0, 0, (ULONG *) &PhysAddr);
		// Update the buffer to be cached, write-through
		VirtualSetAttributes(BufRecord[i], size, 0x8, 0xC, NULL);

#elif( _WIN32_WCE == 600 )
/*		BOOL bOK = PFAllocPhysMem(pH264Obj->pf, size, &pfParams[i]);
		if(!bOK)
		{
				 printf("Can not alloc physical memory for frame buffers[%d]!\n", i);
				 return -1;
		}
		BufRecord[i]= pfParams[i].userVirtAddr;
*/
#endif
	 //	   printf("BufRecord[%d]:%x\n",i, BufRecord[i]);

	}
#endif
    return 1;
}

int initBuffers(sAVCDecoderConfig *psAVCDec, int width,int height,int bufferNum, H264_DEC_OBJ * pH264Obj)
{
	int i,size;
	pH264Obj->gBufferNum = bufferNum;

	if (E_AVCD_SW_DEBLOCK == AVCDGetDeblockOption(psAVCDec))
	{
		return initSWBuffer(width,height,pH264Obj);
	}
	else
	{
		return initHWBuffer(width,height,pH264Obj);
	}
}
void releaseBuffers(sAVCDecoderConfig *psAVCDec,H264_DEC_OBJ * pH264Obj)
{
	int i;
	if (E_AVCD_HW_DEBLOCK == AVCDGetDeblockOption(psAVCDec))
	{
#if defined(TGT_OS_WINCE) && defined(ENABLE_HW_DEBLOCK)
	for (i=0;i< pH264Obj->gBufferNum;i++)
	{
#if( _WIN32_WCE == 500 )//xianzhong
		FreePhysMem(BufRecord[i]);
#elif( _WIN32_WCE == 600 )
        	PFFreePhysMem(pH264Obj->pf, pfParams[i]);
#endif
	}
#endif
	}
	else
	{
		for (i = 0; i< pH264Obj->gBufferNum; i ++)
		{
			if ( NULL != pH264Obj->Buffers[i])
			{
				free(pH264Obj->Buffers[i]);
			}
		}
	}
	//printf("@@@@total %d buffers are provided to decoder\n",pH264Obj->bufferIndex);
}


void* getBuffer(void* AppContext)
{
	void* temp=NULL;
	H264_DEC_OBJ * pH264Obj = (H264_DEC_OBJ *)AppContext;

#ifdef DR_LINUX_TEST
		if(pH264Obj->fnPrbDutFunc)
		{
			return pH264Obj->fnPrbDutFunc(T_GET_BUFFER, NULL);
		}
#else
#ifdef ENABLE_HW_DEBLOCK
	if (E_AVCD_HW_DEBLOCK == AVCDGetDeblockOption(&pH264Obj->vdec))
	{
		temp = BufRecord[pH264Obj->bufferIndex%pH264Obj->gBufferNum];
	}
	else
#endif
	{
		temp = pH264Obj->Buffers[pH264Obj->bufferIndex%pH264Obj->gBufferNum];
	}

	pH264Obj->bufferIndex+=1;
#endif

	return temp;

}

void rejectBuffer(void* buffer, void* AppContext)
{
VTS_PRINTF("rejected %x\n", buffer);
}

void releaseBuffer(void* buffer, void* AppContext)
{
	int i;
	H264_DEC_OBJ * pH264Obj = (H264_DEC_OBJ *)AppContext;

	VTS_PRINTF("released %x\n", buffer);

#ifdef DR_LINUX_TEST
		if(pH264Obj->fnPrbDutFunc)
		{
			pH264Obj->fnPrbDutFunc(T_REL_BUFFER, buffer);
			return;
		}
#else
#ifdef ENABLE_HW_DEBLOCK
	if (E_AVCD_HW_DEBLOCK == AVCDGetDeblockOption(&pH264Obj->vdec))
	{
		for(i=0;i<pH264Obj->gBufferNum; i++)
		{
			if(BufRecord[i] == buffer) break;
		}
	}
	else
#endif
	{
		for(i=0;i<pH264Obj->gBufferNum; i++)
		{
			if(pH264Obj->Buffers[i] == buffer) break;
		}
	}

	if(i >= pH264Obj->gBufferNum)
	{
		PRINT_ERROR("invalid buffer 0x%x\n",buffer);
		return;
	}
#endif
	//just for conformance test
	//memset(buffer, 0, (pH264Obj->video_width * pH264Obj->video_height * 3) /2);
}

unsigned int queryPhyAddress(void* buffer, void* AppContext)
{
	int i;
	H264_DEC_OBJ * pH264Obj = (H264_DEC_OBJ *)AppContext;

	VTS_PRINTF("queryPhyAddress %x\n", buffer);

#ifdef DR_LINUX_TEST
	if(pH264Obj->fnPrbDutFunc)
	{
		return (unsigned int)pH264Obj->fnPrbDutFunc(T_GET_PHY_ADDR, buffer);
	}
#else
#ifdef ENABLE_HW_DEBLOCK
	for(i=0;i<pH264Obj->gBufferNum; i++)
	{
		if(BufRecord[i] == buffer)
		{
			return BufPhyAddr[i];
		}
	}

	PRINT_ERROR("queryPhyAddress failed 0x%x\n", buffer);
#endif
#endif
	return 0;
}


#define CASE_FIRST(x)   if (strncmp(argv[0], x, strlen(x)) == 0)
#define CASE(x)         else if (strncmp(argv[0], x, strlen(x)) == 0)
#define DEFAULT         else

//#ifndef OS_VRTX
static void GetUserInput(H264_DEC_OBJ *pH264Obj,IOParams *pIO, Int argc, char *argv[])
{
    pIO->ffFlag = 0;

    pIO->maxnum = 0;
    pIO->display = 0;
#ifndef ENABLE_HW_DEBLOCK
	pIO->hwdblock = 0;
#else
	pIO->hwdblock = 1;
#endif

    pIO->tst = 0;

    while (argc)
    {
        if (argv[0][0] == '-')
        {
            CASE_FIRST("-ff")
            {
                if (argv[1] != NULL)
                    pH264Obj->ffglobal = atoi(argv[1]);
                pH264Obj->ff_enable = 1;
                pIO->ffFlag = 1;
            }
            CASE("-n")
            {
                argc--;
                argv++;
                if (argv[0] != NULL)
                    sscanf(argv[0], "%d", &pIO->maxnum);
            }
            CASE("-s")
            {
                pIO->hwdblock = 0;
            }
            DEFAULT                             // Has to be last
            {
                PRINT_ERROR("Unsupported option %s\n", argv[0]);
            }
        }
        argc--;
        argv++;
    }
}

void InitalizeAppMemory(sAVCDMemAllocInfo *psMemPtr)
{
    int s32Count, maxNumReqs;

    maxNumReqs = psMemPtr->s32NumReqs;

    for (s32Count = 0; s32Count < maxNumReqs; s32Count++)
    {
        psMemPtr->asMemBlks[s32Count].pvBuffer = NULL;
    }
}



void AppAllocMemory(H264_DEC_OBJ * pH264Obj, sAVCDMemAllocInfo *psMemPtr)
{
    int s32Count, maxNumReqs;

    maxNumReqs = psMemPtr->s32NumReqs;

    for (s32Count = 0; s32Count < maxNumReqs; s32Count++)
    {
        if (psMemPtr->asMemBlks[s32Count].s32Allocate == 1)
        {
            psMemPtr->asMemBlks[s32Count].pvBuffer = \
                MALLOC(psMemPtr->asMemBlks[s32Count].s32Size);

			HEAP_INCREASE((void *)psMemPtr->asMemBlks[s32Count].s32Size);
        }
    }
}

void AppAllocMemory_1(H264_DEC_OBJ * pH264Obj,sAVCDMemAllocInfo *psMemPtr)
{
    int s32Count, maxNumReqs;

    maxNumReqs = psMemPtr->s32NumReqs;

    for (s32Count = 0; s32Count < maxNumReqs; s32Count++)
    {
        if (psMemPtr->asMemBlks[s32Count].s32SizeDependant == 1 &&
            psMemPtr->asMemBlks[s32Count].s32Allocate == 1)
        {
            if ((psMemPtr->asMemBlks[s32Count].pvBuffer != NULL) &&
                (psMemPtr->asMemBlks[s32Count].s32Copy == 1))
            {
                psMemPtr->asMemBlks[s32Count].pvBuffer =
                    REALLOC(psMemPtr->asMemBlks[s32Count].pvBuffer,
                            psMemPtr->asMemBlks[s32Count].s32Size);
            }
            else
            {
		if(psMemPtr->asMemBlks[s32Count].pvBuffer)
		{
             		  free(psMemPtr->asMemBlks[s32Count].pvBuffer);
		}
                psMemPtr->asMemBlks[s32Count].pvBuffer = \
                    MALLOC(psMemPtr->asMemBlks[s32Count].s32Size);

				HEAP_INCREASE((void *)psMemPtr->asMemBlks[s32Count].s32Size);
            }
        }
    }
}


void AllocateFrameMemory(sAVCDYCbCrStruct *psFrame, sAVCDConfigInfo *pConfig, H264_DEC_OBJ * pH264Obj)
{
    int s32Xsize, s32Ysize, s32Cysize, s32Cxsize;

	if(psFrame->eOutputFormat != E_AVCD_420_PLANAR_PADDED)
	{
	    if (psFrame->pu8y != NULL)
	        free(psFrame->pu8y);
	    if (psFrame->pu8cb != NULL)
	        free(psFrame->pu8cb);
	    if (psFrame->pu8cr != NULL)
	        free(psFrame->pu8cr);
	}
    // Allocate memory to the frames
    switch (psFrame->eOutputFormat)
    {
	case E_AVCD_420_PLANAR:
		psFrame->s16Xsize = (pConfig->s16FrameWidth); //  + 16); // 192 (extra buffer space not required);
		pH264Obj->video_width = s32Xsize = psFrame->s16Xsize;
		pH264Obj->video_height = s32Ysize = (pConfig->s16FrameHeight); // + 16); // 160 (extra buffer space not required);
		psFrame->s16CxSize = psFrame->s16Xsize >> 1;
		s32Cysize = s32Ysize >> 1;
		s32Cxsize = psFrame->s16CxSize;

        psFrame->pu8y =(unsigned char*) MALLOC(s32Xsize * s32Ysize
    				* sizeof (char));
        psFrame->pu8cb =(unsigned char*) MALLOC(s32Cxsize * s32Cysize
    				* sizeof (char));
        psFrame->pu8cr =(unsigned char*) MALLOC(s32Cxsize * s32Cysize
    				* sizeof (char));
		if(psFrame->pu8y==NULL)
		{
			PRINT_ERROR("Failed to allocate output buffer(Y)\n");
			   exit(0);
		}
		if(psFrame->pu8cb==NULL)
		{
			PRINT_ERROR("Failed to allocate output buffer(U)\n");
			   exit(0);
		}
		if(psFrame->pu8cr==NULL)
		{
			PRINT_ERROR("Failed to allocate output buffer(V)\n");
			   exit(0);
		}
		break;
    case E_AVCD_420_PLANAR_PADDED:
		psFrame->s16Xsize = (pConfig->s16FrameWidth); //  + 16); // 192 (extra buffer space not required);
		pH264Obj->video_width = s32Xsize = psFrame->s16Xsize;
		pH264Obj->video_height = s32Ysize = (pConfig->s16FrameHeight); // + 16); // 160 (extra buffer space not required);
		psFrame->s16CxSize = psFrame->s16Xsize >> 1;
		s32Cysize = s32Ysize >> 1;
		s32Cxsize = psFrame->s16CxSize;

                       psFrame->pu8y  = NULL;
                       psFrame->pu8cb = NULL;
                       psFrame->pu8cr = NULL;

        pH264Obj->video_width = (pH264Obj->video_width+15)/16*16+32;
        pH264Obj->video_height = (pH264Obj->video_height+15)/16*16+32;
        break;
	case E_AVCD_422_UYVY:
		psFrame->s16Xsize = (pConfig->s16FrameWidth)*2;
		s32Xsize = psFrame->s16Xsize;
		s32Ysize = (pConfig->s16FrameHeight);
		psFrame->pu8y =(unsigned char*) malloc(s32Xsize * s32Ysize* sizeof (char));
		if(psFrame->pu8y==NULL)
		{
			printf("Failed to allocate output buffer(YUV)\n");
			exit(0);
		}

        psFrame->pu8cb = NULL;
        psFrame->pu8cr = NULL;

		break;
	}
}


void FreeDecoderMemory(sAVCDMemAllocInfo *psMemPtr)
{
    int s32Count, maxNumReqs = psMemPtr->s32NumReqs;

    for (s32Count = 0; s32Count < maxNumReqs; s32Count++)
    {
        if (psMemPtr->asMemBlks[s32Count].pvBuffer != NULL)
        {
            free(psMemPtr->asMemBlks[s32Count].pvBuffer);
        }
    }
}

void FreeFrameMemory(sAVCDYCbCrStruct *psFrame)
{
   if ( ( psFrame->eOutputFormat == E_AVCD_422_UYVY ) || ( psFrame->eOutputFormat == E_AVCD_420_PLANAR ) )
   {
    if (psFrame->pu8y != NULL)
        free(psFrame->pu8y);
    if (  psFrame->eOutputFormat == E_AVCD_420_PLANAR )
    {
        if (psFrame->pu8cb != NULL)
         free(psFrame->pu8cb);
        if (psFrame->pu8cr != NULL)
         free(psFrame->pu8cr);
    }
   }
}

//static int H264DecInit_internal( void **_ppDecObj, DEC_INIT_CONTXT_DUT *_psInitContxt, T_DEC_CONTXT_DUT *_psDecContxt)
static int H264DecInit_internal( void **_ppDecObj, DUT_INIT_CONTXT_2_1 *_psInitContxt, T_DEC_CONTXT_DUT *_psDecContxt)
{
    IOParams *pIOPars = NULL;
    sAVCDecoderConfig *pVDec = NULL;
    H264_DEC_OBJ * pH264Obj = NULL;

	T_DEC_CONTXT_DUT *psH264DecContxt = (T_DEC_CONTXT_DUT *)_psDecContxt;
    DUT_INIT_CONTXT_2_1 * psH264InitContxt = (DUT_INIT_CONTXT_2_1 *)_psInitContxt;

	printf("%s\n", H264DCodecVersionInfo());

    pH264Obj = (H264_DEC_OBJ *)malloc( sizeof(H264_DEC_OBJ) );
    if ( pH264Obj == NULL )
    {
        return E_DEC_INIT_ERROR_DUT;
    }
    memset( pH264Obj, 0, sizeof( H264_DEC_OBJ ) );
    *_ppDecObj = pH264Obj;
    /* set the VTS probe */
    if(psH264InitContxt) pH264Obj->pfnVtsProbes = psH264InitContxt->pfProbe;

    pIOPars = &pH264Obj->ioPars;
    pVDec = &pH264Obj->vdec;

    pVDec->s32FrameNumber = 0;
    pVDec->sFrameData.s32FrameNumber = 0;
    pVDec->sFrameData.pu8y = NULL;
    pVDec->sFrameData.pu8cb = NULL;
    pVDec->sFrameData.pu8cr = NULL;
    pVDec->sConfig.s16FrameWidth = 0;
    pVDec->sConfig.s16FrameHeight = 0;
    //Application data set by the application
    pVDec->pAppContext    = (void*) pH264Obj; //DSPhl27777
    pVDec->cbkAVCDBufRead = NULL;//&cbkAppBufRead;
    pH264Obj->ffglobal = 0;
	pH264Obj->current_nal_index =-1;

	pH264Obj->frame_number = 0;
	pH264Obj->video_width = 0;
	pH264Obj->video_height = 0;

	pH264Obj->bufferIndex = 0;
	pH264Obj->gBufferNum = 17;

	pH264Obj->last_width = 0;
	pH264Obj->last_height= 0;
	pH264Obj->last_bufNumber= 0;

#ifndef ENABLE_HW_DEBLOCK
	pIOPars->hwdblock = 0;
#else
    /* configure hardware */
    pIOPars->hwdblock = 1;
#endif
    //pIOPars->display = 0;

	// Get settings from command options
	if(!psH264DecContxt) GetUserInput(pH264Obj, &pH264Obj->ioPars, 0, NULL);
	else GetUserInput(pH264Obj, &pH264Obj->ioPars, psH264DecContxt->argc,psH264DecContxt->argv);

	pVDec->sFrameData.eOutputFormat = E_AVCD_420_PLANAR_PADDED;  // no memory alloced, uses librar

    /* copy input file and output file name to the decoder used date structure */
	if(psH264DecContxt)strncpy(pIOPars->bitFileName, psH264DecContxt->strInFile, MAX_STRLEN);
	if(psH264InitContxt)strncpy(pIOPars->bitFileName, psH264InitContxt->strInFile, MAX_STRLEN);

	// Initialize I/O based on user input. Located in io.c
    IO_Init(pIOPars);


    init_NAL_buffers(pH264Obj);

    // Allocate Memory for input bit buffer
//    vdec.s32InBufferLength = 2100000;
//    vdec.pvInBuffer = MALLOC(vdec.s32InBufferLength);
    // buffering 2 NAL
    read_nal(pH264Obj,pIOPars,0);
//    read_nal(1);
//    ASSERT(NALBufferBytes[0] != 0);
    pH264Obj->current_nal_index = 1;


	STACK_TAG();
    pH264Obj->eStatus = eAVCDInitQueryMem (&pVDec->sMemInfo);// [zhenyong] tell me(app) memories you want
    STACK_UPDATE();

    InitalizeAppMemory(&pVDec->sMemInfo);

    AppAllocMemory(pH264Obj,&pVDec->sMemInfo);// [zhenyong] app do some allocation (only those determined)

    // Initialize the decoder
    STACK_TAG();
    pH264Obj->eStatus = eAVCDInitVideoDecoder(pVDec);
	STACK_UPDATE();
    if (pH264Obj->eStatus != E_AVCD_INIT)
    {
        return E_DEC_INIT_ERROR_DUT;
    }

    //AVCD_HW_flag_settings = 0;
    STACK_TAG();
    AVCDSetDeblockOption(pVDec, E_AVCD_SW_DEBLOCK);
	STACK_UPDATE();
    if(pIOPars->hwdblock)
    {
    	STACK_TAG();
        AVCDSetDeblockOption(pVDec, E_AVCD_HW_DEBLOCK);
		STACK_UPDATE();
   	}
    //if(ioPars.display) AVCD_HW_flag_settings |= 2;

    VTS_PRINTF("IPU deblock settings: %d\n", AVCDGetDeblockOption(pVDec));

#if defined(ENABLE_HW_DEBLOCK) && defined(TGT_OS_WINCE) && ( _WIN32_WCE == 600 )
/*
	pH264Obj->pf = PFOpenHandle();
	if (pH264Obj->pf == INVALID_HANDLE_VALUE)
	{
		return E_DEC_INIT_ERROR_DUT;
	}
*/
#endif

    return E_DEC_INIT_OK_DUT;
}

/*-------------------------------------------------------------------------------------*/
/******************************* API functions for VTS *********************************/
/*-------------------------------------------------------------------------------------*/
//DEC_RETURN_DUT _FSL_EXPORT_C VideoDecInit_dut( void **_ppDecObj, DEC_INIT_CONTXT_DUT *_psInitContxt)
DEC_RETURN_DUT DLL_EXPORTS VideoDecInit( void **_ppDecObj, void *_psInitContxt)
{
	return H264DecInit_internal(_ppDecObj, _psInitContxt, NULL);
}

T_DEC_RETURN_DUT _FSL_EXPORT_C VideoTestDecInit_dut( void **_ppDecObj, T_DEC_CONTXT_DUT *_psDecContxt)
{
	DEC_RETURN_DUT eDecRet;
	eDecRet = H264DecInit_internal(_ppDecObj, NULL, _psDecContxt);
	return (eDecRet == E_DEC_INIT_OK_DUT) ? T_DEC_INIT_OK_DUT : T_DEC_INIT_ERROR_DUT;
}

DEC_RETURN_DUT DecodeOneFrame( void *pDecObj, void *pParam )
{
    H264_DEC_OBJ * pH264Obj = (H264_DEC_OBJ *)pDecObj;
	T_DEC_CONTXT_DUT * psH264DecContxt = (T_DEC_CONTXT_DUT *)pParam;
    IOParams *pIOPars = NULL;
    sAVCDecoderConfig *pVDec = NULL;
    eAVCDRetType ffState;
    unsigned char u8Invalid;
    int buffer_changed_flag;

    pIOPars = &pH264Obj->ioPars;
    pVDec = &pH264Obj->vdec;

	if ((pH264Obj->eStatus == E_AVCD_CHANGE_SERVICED)||(pH264Obj->eStatus == E_AVCD_FLUSH_STATE))
    {// [zhenyong] so don't read in next NAL
     // [zhenyong, 2007-03-12] FIXME! the buffer will be passed to decoder again,
     // but needn't destuff again. And length may changed!
            buffer_changed_flag = 0;//Bitstream no change
    }
    else
    {// [zhenyong] decoder wants following NAL
        int next_nal_index;
        if(pH264Obj->current_nal_index == 0)
            next_nal_index = 1;
        else
            next_nal_index = 0;

        // check NALBuffer
        //
        pVDec->pvInBuffer = pH264Obj->NALBuffer[next_nal_index];
        pVDec->s32InBufferLength = MAX_NAL_LEN;
        pVDec->s32NumBytes = pH264Obj->NALBufferBytes[next_nal_index];

        if(pH264Obj->NALDeStuffed[next_nal_index])
        {
            buffer_changed_flag = 2;//Bitstream reset, but needn't destuff
        }
        else
        {
            buffer_changed_flag = 1;//Bitstream reset, and destuff
        }

        // fetch next NAL

        read_nal(pH264Obj, pIOPars,pH264Obj->current_nal_index);

        pH264Obj->current_nal_index = next_nal_index;
    }

    pH264Obj->pAppBuffer = pVDec->pvInBuffer;
    pH264Obj->s32NumBytesGlobal = pVDec->s32NumBytes;
    if (pVDec->s32NumBytes == 0)
    {
        return E_DEC_FINISH_DUT;
    }
    else
    {
        if (pH264Obj->ff_flag)
            ffState = E_AVCD_FF;
        else
            ffState = pH264Obj->eStatus;


		STACK_TAG();
		DEC_TIMER_BEGIN();

        //***********************************************
        pH264Obj->eStatus = eAVCDecodeNALUnit(pVDec, pH264Obj->ff_flag);
        //***********************************************


        DEC_TIMER_END();
        STACK_UPDATE();



        //printf("eStatus: %d\n",eStatus);

        if (E_AVCD_NOMEM == pH264Obj->eStatus)
        {

		VTS_PRINTF("not enough of frame buffer\n");}

        if (pIOPars->ffFlag == 1)
        {
            if ((pVDec->s32FrameNumber == pH264Obj->ffglobal) && !pH264Obj->ff_taken && pH264Obj->ff_enable)
            {
                pH264Obj->ff_flag = 1;
                pH264Obj->ff_taken = 1;
            }
        }

        if (pH264Obj->eStatus == E_AVCD_NULL_POINTER)
        {
            EXIT(pH264Obj->eStatus);
        }
        pH264Obj->ffcnt ++;
        if ((ffState == E_AVCD_FF) && (pH264Obj->eStatus == E_AVCD_NOERROR))
        {
            pH264Obj->ff_flag = 0;
        }
        if (pH264Obj->eStatus == E_AVCD_SEQ_CHANGE)
        {
        	int width, height,FrameBufNumber;

        	STACK_TAG();
            pH264Obj->eStatus = eAVCDReQueryMem(pVDec);
			STACK_UPDATE();

            AppAllocMemory_1(pH264Obj, &pVDec->sMemInfo);

            width = pVDec->sConfig.s16FrameWidth;
            height = pVDec->sConfig.s16FrameHeight;
			FrameBufNumber = pVDec->sMemInfo.s32MinFrameBufferNum;

			if((pH264Obj->last_width != width || pH264Obj->last_height != height || pH264Obj->last_bufNumber != FrameBufNumber))
            {
		        {
		        	int size;

				if(psH264DecContxt)
				{
					psH264DecContxt->uiWidth = width;
					psH264DecContxt->uiHeight= height;
				}
                size = (width+32)*(height+32)*1.5;
                //printf("Frame ref number:%d\n",vdec.sMemInfo.s32MinFrameBufferNum);getchar();
                //printf("Frame ref number:%d\n",(AVCStruct*)(vdec.pvAVCData)->nBuffers);//getchar();

#ifdef DR_LINUX_TEST
					if(pH264Obj->last_width && pH264Obj->last_height && pH264Obj->last_bufNumber)
						{
							printf("need enhance DUT wrapper ...\n");
							return E_DEC_ERROR_DUT;
						}
						if(pH264Obj->fnPrbDutFunc)
						{
						T_PROBE_INIT_BUFFER prbBuf;
						prbBuf.total_width = (width+32);
						prbBuf.total_height = (height+32);

						prbBuf.left=16;
						prbBuf.top=16;
						prbBuf.width=width;
						prbBuf.height=height;

						prbBuf.frame_number = FrameBufNumber;
						pH264Obj->fnPrbDutFunc(T_INIT_BUFFER,(void*)&prbBuf);
					}
#else
					{
                initBuffers(pVDec, width+32,height+32,
                     pVDec->sMemInfo.s32MinFrameBufferNum,pH264Obj);
                VTS_PRINTF("init Frame Mgr\n");
                //AVCDSetIPUBufferNum(&vdec,3);
            }
#endif

					pH264Obj->last_width = width;
					pH264Obj->last_height = height;
					pH264Obj->last_bufNumber = FrameBufNumber;
		        }
            {
                AVCD_FrameManager frameMgr;
                frameMgr.BfGetter = getBuffer;
                frameMgr.BfRejector = rejectBuffer;

				STACK_TAG();
                AVCDSetBufferManager(pVDec, &frameMgr);
				STACK_UPDATE();

				STACK_TAG();
					H264SetAdditionalCallbackFunction(pVDec,E_RELEASE_FRAME, releaseBuffer);
				STACK_UPDATE();

				STACK_TAG();
					H264SetAdditionalCallbackFunction(pVDec,E_QUERY_PHY_ADDR, queryPhyAddress);
				STACK_UPDATE();

                VTS_PRINTF("Set Frame Mgr\n");
            }

                if ((pVDec->sConfig.s16FrameWidth != 0) ||
                    (pVDec->sConfig.s16FrameHeight != 0))
                {
                    AllocateFrameMemory(&pVDec->sFrameData, &pVDec->sConfig, pH264Obj);
                    pH264Obj->ff_flag = 0;
                }
            }
        }
    }
    u8Invalid = (pH264Obj->eStatus != E_AVCD_DEMO_PROTECT) &&(pH264Obj->eStatus != E_AVCD_BAD_DATA)
				&& (pH264Obj->eStatus != E_AVCD_NOERROR) && (pH264Obj->eStatus != E_AVCD_CHANGE_SERVICED)
				&& (pH264Obj->eStatus != E_AVCD_NO_OUTPUT) &&(pH264Obj->eStatus != E_AVCD_FLUSH_STATE)
			    && (pH264Obj->eStatus != E_NO_PICTURE_PAR_SET_NAL) &&(pH264Obj->eStatus != E_NO_SEQUENCE_PAR_SET_NAL);
    if (u8Invalid)
    {
    	printf("%s %d, Exit with eStatus : %d\n", __FUNCTION__, __LINE__, pH264Obj->eStatus);
      //  EXIT(pH264Obj->eStatus);
		return E_DEC_ERROR_DUT;
    }
#ifdef SUPPORT_OUTPUT_FRAME_EARLY

	eAVCDGetFrameEarly(pVDec);

	if(pVDec->sFrameData.pu8y && pVDec->sFrameData.pu8cb && pVDec->sFrameData.pu8cr)
	{
    	T_PROBE_PUT_FRAME prbFrm;
    	long TopOffset, LeftOffset;
    	char *pu8y = pVDec->sFrameData.pu8y;
		char *pu8cb = pVDec->sFrameData.pu8cb;
		char *pu8cr = pVDec->sFrameData.pu8cr;

		TopOffset= 16;
		LeftOffset = 16;

		pH264Obj->frame_number++;

		if(psH264DecContxt)	psH264DecContxt->uiFrameNum = pH264Obj->frame_number;

    	if(pVDec->sFrameData.eOutputFormat == E_AVCD_420_PLANAR_PADDED)
    	{
    		pu8y -=((pVDec->sFrameData.s16Xsize * 16) + 16);
			pu8cb-=((pVDec->sFrameData.s16CxSize * 8) + 8);
			pu8cr-=((pVDec->sFrameData.s16CxSize * 8) + 8);
			TopOffset= 16 + pVDec->sFrameData.cropTop_display * 2;
			LeftOffset = 16 + pVDec->sFrameData.cropLeft_display * 2;
    	}

		prbFrm.puchLumY = pu8y;
		prbFrm.puchChrU = pu8cb;
		prbFrm.puchChrV = pu8cr;
		prbFrm.iFrmWidth = pVDec->sFrameData.s16FrameWidth;
		prbFrm.iFrmHeight = pVDec->sFrameData.s16FrameHeight;
		prbFrm.iStrideLX = pVDec->sFrameData.s16Xsize;
		prbFrm.iStrideLY = pH264Obj->video_height;
		prbFrm.iStrideUV = pVDec->sFrameData.s16CxSize;
		prbFrm.iTopOffset = TopOffset;
		prbFrm.iLeftOffset = LeftOffset;
#ifdef DR_LINUX_TEST
        pH264Obj->fnPrbDutFunc(T_PUT_FRAME_DR, (void*)&prbFrm);
#else
		pH264Obj->fnPrbDutFunc(T_PUT_FRAME, (void*)&prbFrm);
#endif
	}

#else
    if ((pH264Obj->eStatus == E_AVCD_NOERROR) || (pH264Obj->eStatus == E_AVCD_FLUSH_STATE)
		||(pH264Obj->eStatus == E_AVCD_BAD_DATA)||(pH264Obj->eStatus == E_AVCD_DEMO_PROTECT))
    {
        /* VTS : record frame output start time */
        if ( NULL != pH264Obj->pfnVtsProbes )
        {
            pH264Obj->pfnVtsProbes( E_OUTPUT_STARTTIME, NULL );
        }

        if(1)
        {
            if(pVDec->sFrameData.eOutputFormat != E_AVCD_420_PLANAR_PADDED)
            {
            	STACK_TAG();
                eAVCDGetFrame ( pVDec );
				STACK_UPDATE();
            }
       //     WriteOutput(pH264Obj->eStatus, pIOPars, pVDec);
        }

        pH264Obj->frame_number++;

		if(psH264DecContxt)	psH264DecContxt->uiFrameNum = pH264Obj->frame_number;

        /* for test application*/
        if(pH264Obj->fnPrbDutFunc)
        {
        	T_PROBE_PUT_FRAME prbFrm;
        	long TopOffset, LeftOffset;
        	char *pu8y = pVDec->sFrameData.pu8y;
			char *pu8cb = pVDec->sFrameData.pu8cb;
			char *pu8cr = pVDec->sFrameData.pu8cr;

			TopOffset= 16;
			LeftOffset = 16;

        	if(pVDec->sFrameData.eOutputFormat == E_AVCD_420_PLANAR_PADDED)
        	{
        		pu8y -=((pVDec->sFrameData.s16Xsize * 16) + 16);
				pu8cb-=((pVDec->sFrameData.s16CxSize * 8) + 8);
				pu8cr-=((pVDec->sFrameData.s16CxSize * 8) + 8);
				TopOffset= 16 + pVDec->sFrameData.cropTop_display * 2;
				LeftOffset = 16 + pVDec->sFrameData.cropLeft_display * 2;
        	}

			prbFrm.puchLumY = pu8y;
			prbFrm.puchChrU = pu8cb;
			prbFrm.puchChrV = pu8cr;
			prbFrm.iFrmWidth = pVDec->sFrameData.s16FrameWidth;
			prbFrm.iFrmHeight = pVDec->sFrameData.s16FrameHeight;
			prbFrm.iStrideLX = pVDec->sFrameData.s16Xsize;
			prbFrm.iStrideLY = pH264Obj->video_height;
			prbFrm.iStrideUV = pVDec->sFrameData.s16CxSize;
			prbFrm.iTopOffset = TopOffset;
			prbFrm.iLeftOffset = LeftOffset;
#ifdef DR_LINUX_TEST
            pH264Obj->fnPrbDutFunc(T_PUT_FRAME_DR, (void*)&prbFrm);
#else
			pH264Obj->fnPrbDutFunc(T_PUT_FRAME, (void*)&prbFrm);
#endif
        }

		/* store output frame here */
        if ( pH264Obj->pfnVtsProbes )
	   {
	   	    char *pu8y = pVDec->sFrameData.pu8y;
			char *pu8cb = pVDec->sFrameData.pu8cb;
			char *pu8cr = pVDec->sFrameData.pu8cr;
            FRAME_COPY_INFO sFrmInfo;

		    if(pVDec->sFrameData.eOutputFormat == E_AVCD_420_PLANAR_PADDED)
		    {

			    pu8y += ((pVDec->sFrameData.cropTop_display * 2 * pVDec->sFrameData.s16Xsize) + (pVDec->sFrameData.cropLeft_display * 2));
			    pu8cb += ((pVDec->sFrameData.cropTop_display * pVDec->sFrameData.s16CxSize) + pVDec->sFrameData.cropLeft_display);
			    pu8cr += ((pVDec->sFrameData.cropTop_display * pVDec->sFrameData.s16CxSize) + pVDec->sFrameData.cropLeft_display);
		   }

            sFrmInfo.puchLumY = pu8y;
            sFrmInfo.puchChrU = pu8cb;
            sFrmInfo.puchChrV = pu8cr;
            sFrmInfo.iFrmWidth = pVDec->sFrameData.s16FrameWidth;
            sFrmInfo.iFrmHeight = pVDec->sFrameData.s16FrameHeight;
            sFrmInfo.iBufStrideY = pVDec->sFrameData.s16Xsize;
            sFrmInfo.iBufStrideUV = pVDec->sFrameData.s16CxSize;
            pH264Obj->pfnVtsProbes( E_OUTPUT_FRAME, &sFrmInfo );
	   }

        /* VTS : record frame output end time */
        if ( NULL != pH264Obj->pfnVtsProbes )
	   {
            pH264Obj->pfnVtsProbes( E_OUTPUT_ENDTIME, NULL );
	   }

    }
    else if ((pH264Obj->eStatus == E_AVCD_FF))
    {
            //fprintf(stderr, "FF[%d]\r", pH264Obj->n++);
            //fflush(stderr);
    }
#endif
    if(pIOPars->maxnum && pH264Obj->frame_number > pIOPars->maxnum)
    {
        return E_DEC_FINISH_DUT;
    }

    return E_DEC_FRAME_DUT;
}

static DEC_RETURN_DUT H264DecFrame( void * _pDecObj, void * _pParam )
{
    H264_DEC_OBJ * pH264Obj = (H264_DEC_OBJ *)_pDecObj;
	T_DEC_CONTXT_DUT * psH264DecContxt = (T_DEC_CONTXT_DUT *)_pParam;
    sAVCDecoderConfig *pVDec = NULL;
    IOParams *pIOPars = NULL;
    DEC_RETURN_DUT eRetVal = E_DEC_FRAME_DUT;

    pVDec = &pH264Obj->vdec;
    pIOPars = &pH264Obj->ioPars;

    eRetVal = DecodeOneFrame( _pDecObj, _pParam);

    // output frames in POC order
    if ( eRetVal == E_DEC_FINISH_DUT )
    {
#ifdef SUPPORT_OUTPUT_FRAME_EARLY

		eAVCDGetFrameEarly(pVDec);

		if(pVDec->sFrameData.pu8y && pVDec->sFrameData.pu8cb && pVDec->sFrameData.pu8cr)
		{
			T_PROBE_PUT_FRAME prbFrm;
			long TopOffset, LeftOffset;
			char *pu8y = pVDec->sFrameData.pu8y;
			char *pu8cb = pVDec->sFrameData.pu8cb;
			char *pu8cr = pVDec->sFrameData.pu8cr;

			TopOffset= 16;
			LeftOffset = 16;

			pH264Obj->frame_number++;

			if(psH264DecContxt)	psH264DecContxt->uiFrameNum = pH264Obj->frame_number;

			if(pVDec->sFrameData.eOutputFormat == E_AVCD_420_PLANAR_PADDED)
			{
				pu8y -=((pVDec->sFrameData.s16Xsize * 16) + 16);
				pu8cb-=((pVDec->sFrameData.s16CxSize * 8) + 8);
				pu8cr-=((pVDec->sFrameData.s16CxSize * 8) + 8);
				TopOffset= 16 + pVDec->sFrameData.cropTop_display * 2;
				LeftOffset = 16 + pVDec->sFrameData.cropLeft_display * 2;
			}

			prbFrm.puchLumY = pu8y;
			prbFrm.puchChrU = pu8cb;
			prbFrm.puchChrV = pu8cr;
			prbFrm.iFrmWidth = pVDec->sFrameData.s16FrameWidth;
			prbFrm.iFrmHeight = pVDec->sFrameData.s16FrameHeight;
			prbFrm.iStrideLX = pVDec->sFrameData.s16Xsize;
			prbFrm.iStrideLY = pH264Obj->video_height;
			prbFrm.iStrideUV = pVDec->sFrameData.s16CxSize;
			prbFrm.iTopOffset = TopOffset;
			prbFrm.iLeftOffset = LeftOffset;
#ifdef DR_LINUX_TEST
			pH264Obj->fnPrbDutFunc(T_PUT_FRAME_DR, (void*)&prbFrm);
#else
			pH264Obj->fnPrbDutFunc(T_PUT_FRAME, (void*)&prbFrm);
#endif
		}

#else

    	pH264Obj->eStatus = E_AVCD_NOERROR;
    	while(pH264Obj->eStatus == E_AVCD_NOERROR)
    	{
    		STACK_TAG();
    		pH264Obj->eStatus=eAVCDecoderFlushAll(pVDec);
			STACK_UPDATE();

    		if(pH264Obj->eStatus == E_AVCD_NOERROR)
    		{
                /* VTS : record frame output start time */
                if ( NULL != pH264Obj->pfnVtsProbes )
                {
                    pH264Obj->pfnVtsProbes( E_OUTPUT_STARTTIME, NULL );
                }

                pH264Obj->frame_number++;

				if(psH264DecContxt)	psH264DecContxt->uiFrameNum = pH264Obj->frame_number;

                if(1)
                {
                    if(pVDec->sFrameData.eOutputFormat != E_AVCD_420_PLANAR_PADDED)
                    {
                    	STACK_TAG();
                        eAVCDGetFrame ( pVDec );
						STACK_UPDATE();
                    }
                }

                 /* for test application*/
		        if(pH264Obj->fnPrbDutFunc)
		        {
		        	T_PROBE_PUT_FRAME prbFrm;
		        	long TopOffset, LeftOffset;
		        	char *pu8y = pVDec->sFrameData.pu8y;
					char *pu8cb = pVDec->sFrameData.pu8cb;
					char *pu8cr = pVDec->sFrameData.pu8cr;

					TopOffset= 16;
					LeftOffset = 16;

		        	if(pVDec->sFrameData.eOutputFormat == E_AVCD_420_PLANAR_PADDED)
		        	{
		        		pu8y -=((pVDec->sFrameData.s16Xsize * 16) + 16);
						pu8cb-=((pVDec->sFrameData.s16CxSize * 8) + 8);
						pu8cr-=((pVDec->sFrameData.s16CxSize * 8) + 8);
						TopOffset= 16 + pVDec->sFrameData.cropTop_display * 2;
						LeftOffset = 16 + pVDec->sFrameData.cropLeft_display * 2;
		        	}

					prbFrm.puchLumY = pu8y;
					prbFrm.puchChrU = pu8cb;
					prbFrm.puchChrV = pu8cr;
					prbFrm.iFrmWidth = pVDec->sFrameData.s16FrameWidth;
					prbFrm.iFrmHeight = pVDec->sFrameData.s16FrameHeight;
					prbFrm.iStrideLX = pVDec->sFrameData.s16Xsize;
					prbFrm.iStrideLY = pH264Obj->video_height;
					prbFrm.iStrideUV = pVDec->sFrameData.s16CxSize;
					prbFrm.iTopOffset = TopOffset;
					prbFrm.iLeftOffset = LeftOffset;

#ifdef DR_LINUX_TEST
					pH264Obj->fnPrbDutFunc(T_PUT_FRAME_DR, (void*)&prbFrm);
#else
					pH264Obj->fnPrbDutFunc(T_PUT_FRAME, (void*)&prbFrm);
#endif


		        }

				/* store output frame here */
			   if ( pH264Obj->pfnVtsProbes )
			   {
			   	    char *pu8y = pVDec->sFrameData.pu8y;
					char *pu8cb = pVDec->sFrameData.pu8cb;
					char *pu8cr = pVDec->sFrameData.pu8cr;
                    FRAME_COPY_INFO sFrmInfo;

				    if(pVDec->sFrameData.eOutputFormat == E_AVCD_420_PLANAR_PADDED)
				    {

					    pu8y += ((pVDec->sFrameData.cropTop_display * 2 * pVDec->sFrameData.s16Xsize) + (pVDec->sFrameData.cropLeft_display * 2));
					    pu8cb += ((pVDec->sFrameData.cropTop_display * pVDec->sFrameData.s16CxSize) + pVDec->sFrameData.cropLeft_display);
					    pu8cr += ((pVDec->sFrameData.cropTop_display * pVDec->sFrameData.s16CxSize) + pVDec->sFrameData.cropLeft_display);
				   }

                    sFrmInfo.puchLumY = pu8y;
                    sFrmInfo.puchChrU = pu8cb;
                    sFrmInfo.puchChrV = pu8cr;
                    sFrmInfo.iFrmWidth = pVDec->sFrameData.s16FrameWidth;
                    sFrmInfo.iFrmHeight = pVDec->sFrameData.s16FrameHeight;
                    sFrmInfo.iBufStrideY = pVDec->sFrameData.s16Xsize;
                    sFrmInfo.iBufStrideUV = pVDec->sFrameData.s16CxSize;
                    pH264Obj->pfnVtsProbes( E_OUTPUT_FRAME, &sFrmInfo );

			   }

                /* VTS : record frame output end time */
                if ( NULL != pH264Obj->pfnVtsProbes )
                {
                    pH264Obj->pfnVtsProbes( E_OUTPUT_ENDTIME, NULL );
                }

                //fprintf(stderr, "[%d]\n", pH264Obj->n++);
                //fflush(stderr);
    		}
    	}
#endif
        eRetVal = E_DEC_ALLOUT_DUT;
    }

    return eRetVal;
}

DEC_RETURN_DUT _FSL_EXPORT_C VideoDecFrame_dut( void * _pDecObj, void * _pParam)
{
	return H264DecFrame(_pDecObj, NULL);
}

T_DEC_RETURN_DUT _FSL_EXPORT_C VideoTestDecFrame_dut( void * _pDecObj,T_DEC_CONTXT_DUT * _psDecContxt)
{
	DEC_RETURN_DUT eDecRet;

	eDecRet = H264DecFrame(_pDecObj, _psDecContxt);

	switch(eDecRet)
	{
	case E_DEC_FRAME_DUT:
		return T_DEC_FRAME_DUT;
		break;
	case E_DEC_FINISH_DUT:
		return T_DEC_FINISH_DUT;
		break;
	case E_DEC_ALLOUT_DUT:
		return T_DEC_ALLOUT_DUT;
		break;
	default:
		return T_DEC_FINISH_DUT;
		break;
	}
}


static void H264DecRelease( void * _pDecObj, void * _pParam )
{
    H264_DEC_OBJ * pH264Obj = (H264_DEC_OBJ *)_pDecObj;
    IOParams *pIOPars = NULL;
    sAVCDecoderConfig *pVDec = NULL;

    pVDec = &pH264Obj->vdec;
    pIOPars = &pH264Obj->ioPars;

#if defined(ENABLE_HW_DEBLOCK) && defined(TGT_OS_WINCE) && ( _WIN32_WCE == 600 )
//	if(pH264Obj->pf) PFCloseHandle(pH264Obj->pf);
#endif

	//printf("\nEnd of AVC decoding\n");
    // Close all IOs
    IO_Close(pIOPars);

	STACK_TAG();
    eAVCDFreeVideoDecoder(pVDec);
	STACK_UPDATE();

	releaseBuffers(pVDec,pH264Obj);
    FreeDecoderMemory(&pVDec->sMemInfo);
    FreeFrameMemory(&pVDec->sFrameData);

	free(pH264Obj); pH264Obj=NULL;

}

DEC_RETURN_DUT _FSL_EXPORT_C VideoDecRelease_dut( void * _pDecObj )
{
	H264DecRelease(_pDecObj, NULL);

    return E_DEC_REL_OK_DUT;
}

T_DEC_RETURN_DUT _FSL_EXPORT_C VideoTestDecRelease_dut( void * _pDecObj, T_DEC_CONTXT_DUT * _psDecContxt )
{
	H264DecRelease(_pDecObj, NULL);

	_psDecContxt->uiFrameRate = 25;

    return T_DEC_REL_OK_DUT;
}

#if ( 21 > WRAPPER_API_VERSION )
DEC_RETURN_DUT _FSL_EXPORT_C VideoDecSetProbes_dut( void * _pDecObj, FuncProbeDut * _pProbe)
{
	H264_DEC_OBJ * pH264Obj = (H264_DEC_OBJ *)_pDecObj;

    pH264Obj->pfnVtsProbes = _pProbe;

	return E_SET_PROB_OK_DUT;
}
#endif

T_DEC_RETURN_DUT _FSL_EXPORT_C VideoTestDecSetProbes_dut( void * _pDecObj, PROBE_DUT_FUNC * _pProbe )
{
	H264_DEC_OBJ * pH264Obj = (H264_DEC_OBJ *)_pDecObj;

	pH264Obj->fnPrbDutFunc = _pProbe;

	return T_SET_PROB_OK_DUT;
}

//support new VTS
#if ( 21 == WRAPPER_API_VERSION )
DEC_RETURN_DUT DLL_EXPORTS QueryAPIVersion( long * _piAPIVersion )
{
    *_piAPIVersion = WRAPPER_API_VERSION;
    
    return E_GET_VER_OK_DUT;
}
#else
DEC_RETURN_DUT _FSL_EXPORT_C QueryAPIVersion_dut( unsigned char _auchVer[] )
{
	strcpy( _auchVer, DUT_API_HEADER_VERSION );
	return E_GET_VER_OK_DUT;
}
#endif

