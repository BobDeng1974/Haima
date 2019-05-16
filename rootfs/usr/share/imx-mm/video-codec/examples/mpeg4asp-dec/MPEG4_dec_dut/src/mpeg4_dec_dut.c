/***************************************************************************
** mpeg4_dec_dut.c
** Copyright 2005-2008 by Freescale Semiconductor, Inc.
** All modifications are confidential and proprietary information
** of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
**
** Description: Contains all functions for both DUT 
**              and reference decoder.
**
** Author:
**     Jacky Xu   <b05407@freescale.com>
**      
** Revision History: 
** ----------------- 
** 1.0  02/21/2008  Jacky Xu   create this file
*****************************************************************************/ 


/***************************************************************************** 
 * <Includes> 
 *****************************************************************************/ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <malloc.h>
#include "mpeg4types.h"
#include "mpeg4_dec_api.h"
#include "dut_api_vts.h"
#include "dut_test_probes.h"
#include "dut_test_api.h"


/***************************************************************************** 
 * <Macros> 
 *****************************************************************************/ 
#define TRUE 1
#define FALSE 0
#define MPEG4_ERROR -1
#define MPEG4_SUCCESS 0
#define BIT_BUFFER_SIZE 1024
#define BUFFER_NUM 8

#define CHKEXPEXIT(expr, message)           \
do \
{                             \
    if ((expr) == 0)           \
    {                         \
        goto exit;            \
    }                         \
}while(0)

/***************************************************************************** 
 * <Typedefs> 
 *****************************************************************************/

/* MPEG-4 DUT object structure */
typedef struct
{
    FILE                  *psInputFh;
    unsigned int          *offset;
    unsigned int           frame_num;
    VTS_PROBES_DUT        sVTSProbes;
    unsigned char          *Buffers[BUFFER_NUM];
    int                    bufferIndex;
    long                  (*fnPrbDutFunc)(T_PROBE_TYPE prb_type, void *para);
    int                    bufLen;
}sMpeg4DutObject;


/*******************************************************************************
 * Extern functions
 *******************************************************************************/
void* getBuffer(void* AppContext);
void rejectBuffer(void * buffer,void* AppContext);
void releaseBuffer(void* buffer,void* AppContext);
int initBuffers(void* AppContext, int size);
void releaseBuffers(void* AppContext);

int s32AllocateMem4Decoder (sMpeg4DecObject* psMpeg4DecObject);
void vFreeMemAllocated4Dec (sMpeg4DecObject* psMpeg4DecObject);
void vFreeMemAllocated4App(sMpeg4DecObject* psMpeg4DecObject);
void vInitailizeMpeg4DecObject(sMpeg4DecObject* psMpeg4DecObject);


/*******************************************************************************
 * Extern functions
 *******************************************************************************/
#define DEC_TIMER_BEGIN() {if(psMpeg4DutObj->fnPrbDutFunc) psMpeg4DutObj->fnPrbDutFunc(T_DECODE_BEGIN, 0);}
#define DEC_TIMER_END() {if(psMpeg4DutObj->fnPrbDutFunc) psMpeg4DutObj->fnPrbDutFunc(T_DECODE_END, 0);}

#define STACK_TAG() {if(psMpeg4DutObj->fnPrbDutFunc) psMpeg4DutObj->fnPrbDutFunc(T_STACK_TAG, 0);}
#define STACK_UPDATE() {if(psMpeg4DutObj->fnPrbDutFunc) psMpeg4DutObj->fnPrbDutFunc(T_STACK_UPDATE, 0);}

#define HEAP_INCREASE(size) {if(psMpeg4DutObj->fnPrbDutFunc) psMpeg4DutObj->fnPrbDutFunc(T_HEAP_INCREASE, (void*)size);}
#define HEAP_DECREASE(size) {if(psMpeg4DutObj->fnPrbDutFunc) psMpeg4DutObj->fnPrbDutFunc(T_HEAP_DECREASE, size);}


/***********************************************************************
 * -Function:
 *    DEC_RETURN_DUT VideoDecInit_dut( void ** ppDecObj, 
 *                                      void *  pFileParseContxt )
 *
 * -Description:
 *    This function initializes the MPEG-4 DUT by using the infomation from VTS app.
 *
 * -Input Param
 *    *pFileParseContxt       pointer of structure contain dut init infomation. 
 *                            It should be DEC_INIT_CONTXT_DUT for MPEG-4 DUT
 *
 * -Output Param
 *    **ppDecObj              pointer of decoder object pointer
 *
 * -Return
 *    E_MPG4_INIT_OK_DUT      decoder is initialized successfully
 *    E_MPG4_INIT_ERROR_DUT   encounter errer during initializing
 ***********************************************************************
 */



static DEC_RETURN_DUT Mpeg4DecInit(void ** ppDecObj, DEC_INIT_CONTXT_DUT * pInitContxt, T_DEC_CONTXT_DUT *psDecContxt)
{
    sMpeg4DutObject       *psMpeg4DutObj = NULL;
    sMpeg4DecObject       *psMpeg4DecObject = NULL;
    sMpeg4DecMemAllocInfo *psMemAllocInfo = NULL;
    unsigned char         *pu8BitBuffer = NULL;
    unsigned char          *pInputFd;
    int                    s32BitBufferLen = 0;
    int                    s32BytesRead = 0;
    eMpeg4DecRetType       eDecRetVal = E_MPEG4D_FAILURE;
    int                    i_loop = 0;
    int                    flag = 1;
    int                    count = 0;
    int                    mpeg4_h263_flag = 1;
    unsigned char          pattern[4];
    int                    cnt = 0;

    if(pInitContxt)
    {
        pInputFd = pInitContxt->strInFile;
    } 
    else if(psDecContxt)
    {
        pInputFd = psDecContxt->strInFile;
    }
    else
    {
        CHKEXPEXIT( FALSE, "DUT_CONTXT ERROR!" );
    }

   
    psMpeg4DecObject = (sMpeg4DecObject *)malloc(sizeof (sMpeg4DecObject));
    if (psMpeg4DecObject == NULL)
    {
        CHKEXPEXIT( FALSE, "Unable to allocate memory for Mpeg4 Decoder structure" );
    }
    else
    {
        vInitailizeMpeg4DecObject(psMpeg4DecObject);
    }

    psMpeg4DutObj = (sMpeg4DutObject *)malloc(sizeof(sMpeg4DutObject));
    if (psMpeg4DutObj == NULL)
    {
        CHKEXPEXIT( FALSE, "Unable to allocate memory for psMpg4DutObj" );
    }
    memset(psMpeg4DutObj, 0, sizeof(sMpeg4DutObject));


    psMpeg4DutObj->offset = (unsigned int *)malloc(sizeof(unsigned int)*20000);
    if (psMpeg4DutObj->offset == NULL)
    {
        CHKEXPEXIT( FALSE, "Unable to allocate memory for offset" );
    }   
    memset(psMpeg4DutObj->offset,0,sizeof(unsigned int)*20000);

    /* open the input file */
    if ((psMpeg4DutObj->psInputFh= fopen (pInputFd,"rb")) == NULL)
    {
        CHKEXPEXIT( FALSE, "Error : Unable to open Input Bitstream file" );
    }

    //psMpeg4DutObj->sVTSProbes = {NULL,NULL};
    psMpeg4DutObj->frame_num = 0;
    psMpeg4DutObj->bufferIndex = 0;
    
    /* Now parse the whole clip */
    pattern[0] = 0xff;
    pattern[1] = 0xff;
    pattern[2] = 0xff;
    pattern[3] = 0xff;
    for(i_loop=0;i_loop<20000;i_loop++)
        psMpeg4DutObj->offset[i_loop]=0;
    /* Looking for mpeg4 Clip flag: VIDEO_OBJECT_LAYER_START_CODE_PREFIX: 0x12, 28bits*/
    while (flag)
    {
        int tmp;
        if(pattern[3]==0x00 && pattern[2]==0x00 && pattern[1]==0x01 && ((pattern[0]&0xf0)==0x20)) 
        {
            mpeg4_h263_flag = 1;
            printf("Test Vector is MPEG4 SP Clip!\n");
            flag=0;
            break;
        }
        pattern[3]=pattern[2];
        pattern[2]=pattern[1];
        pattern[1]=pattern[0];

        tmp = getc(psMpeg4DutObj->psInputFh);
        if( tmp != EOF )
        {   
            pattern[0] = (unsigned char)tmp;
        }
        else            
        {
            break;
        }
    }    
    fseek (psMpeg4DutObj->psInputFh,0, SEEK_SET);
    pattern[0] = 0xff;
    pattern[1] = 0xff;
    pattern[2] = 0xff;
    pattern[3] = 0xff;

    /* Looking for H263 Clip flag: SHORT_VIDEO_START_MARKER: 0x20, 22bits*/
    while (flag)
    {
        int tmp;
        if(pattern[2]==0x00 && pattern[1]==0x00 && ((pattern[0]&0xfc)==0x80)) 
        {
            mpeg4_h263_flag = 2;
            flag=0;
            //printf("This is H263(Short Head) Clip!\n");
            break;
        }

        pattern[2]=pattern[1];
        pattern[1]=pattern[0];

        tmp = getc(psMpeg4DutObj->psInputFh);
        if( tmp != EOF )
        {   
            pattern[0] = (unsigned char)tmp;
        }
        else            
        {
            break;
        }
    }        
    fseek (psMpeg4DutObj->psInputFh,0, SEEK_SET);
    pattern[0] = 0xff;
    pattern[1] = 0xff;
    pattern[2] = 0xff;
    pattern[3] = 0xff;

    flag = 1;        
    /* This bit stream is MPEG4 SP Clip!*/
    if(mpeg4_h263_flag == 1) {
        while (flag)
        {
            int tmp;
            
            if(pattern[3]==0x00 && pattern[2]==0x00 && pattern[1]==0x01 && pattern[0]==0xB6) 
            {
                psMpeg4DutObj->offset[cnt]=count-4;
                count=4;
                cnt++;
            }

            pattern[3]=pattern[2];
            pattern[2]=pattern[1];
            pattern[1]=pattern[0];

            tmp = getc(psMpeg4DutObj->psInputFh);
            if( tmp != EOF )
            {   
                pattern[0] = (unsigned char)tmp;
                count++;
            }
            else            
            {
                psMpeg4DutObj->offset[cnt]=count;
                flag=0;
                break;
            }
        }    
    }
    pattern[0] = 0xff;
    pattern[1] = 0xff;
    pattern[2] = 0xff;
    pattern[3] = 0xff;   
    /* This bit stream is H263(Short Head) Clip!*/
    if(mpeg4_h263_flag == 2) {
        while (flag)
        {
            int tmp;
                
            if(pattern[2]==0x00 && pattern[1]==0x00 && ((pattern[0]&0xfc)==0x80)) 
            {
                   psMpeg4DutObj->offset[cnt]=count-3;
                count=3;
                cnt++;
            }

            pattern[2]=pattern[1];
            pattern[1]=pattern[0];

            tmp = getc(psMpeg4DutObj->psInputFh);
            if( tmp != EOF )
            {   
                pattern[0] = (unsigned char)tmp;
                count++;
            }
            else            
            {
                psMpeg4DutObj->offset[cnt]=count;
                flag=0;
                break;
            }
        }    
    }
    fseek(psMpeg4DutObj->psInputFh,0, SEEK_SET);
    /* Finish parse clip and get all offset */


    psMpeg4DecObject->pvAppContext = (void *)psMpeg4DutObj;
    *ppDecObj = psMpeg4DecObject;


    /*!
    *   Calling Query Mem Function
    */
    s32BitBufferLen = BIT_BUFFER_SIZE;
    pu8BitBuffer = (unsigned char *) malloc (sizeof(unsigned char)*s32BitBufferLen);
    if (pu8BitBuffer == NULL)
    {
        CHKEXPEXIT( FALSE, "Error : Unable to open Input Bitstream file" );
        //vFreeMemAllocated4App (psMpeg4DecObject);
    }
    s32BytesRead = fread(pu8BitBuffer, sizeof(unsigned char), s32BitBufferLen,
                          psMpeg4DutObj->psInputFh);

    if (s32BytesRead < s32BitBufferLen)
    {
       /*! Let Decoder handle the Error */
       //printf("ERROR - Insufficient bytes in encoded Bitstream file\n");
    }
    fseek(psMpeg4DutObj->psInputFh, 0, SEEK_SET);

    psMpeg4DecObject->sDecParam.sOutputBuffer.eOutputFormat = E_MPEG4D_420_YUV_PADDED;
    eDecRetVal =  eMPEG4DQuerymem (psMpeg4DecObject, pu8BitBuffer, s32BytesRead);

    {
  	    int width,height;
  	    width = ((psMpeg4DecObject->sDecParam.u16FrameWidth+15)>>4)*16+32;
  	    height = ((psMpeg4DecObject->sDecParam.u16FrameHeight+15)>>4)*16+32;
  	    initBuffers(psMpeg4DutObj,(int)(width*height*1.5));
        psMpeg4DutObj->bufLen = (int)(width*height*1.5);

        if(psDecContxt)
        {
            psDecContxt->uiWidth = psMpeg4DecObject->sDecParam.u16FrameWidth;
            psDecContxt->uiHeight = psMpeg4DecObject->sDecParam.u16FrameHeight;
        }
  	}

    if (pu8BitBuffer != NULL)
    {
        free (pu8BitBuffer);
        pu8BitBuffer = NULL;
    }

    if (eDecRetVal != E_MPEG4D_SUCCESS)
    {
        /*! Freeing Memory allocated by the Application */
        //vFreeMemAllocated4App (psMpeg4DecObject, psDecodeParams);
        CHKEXPEXIT( FALSE, "Initialize MPEG-4 DUT failed" );
    }

    /*!
    *   Allocating Memory for Output Buffer
    */
    psMpeg4DecObject->sDecParam.sOutputBuffer.pu8YBuf = (unsigned char *)
        malloc (psMpeg4DecObject->sDecParam.sOutputBuffer.s32YBufLen *
        sizeof(unsigned char));
    if (psMpeg4DecObject->sDecParam.sOutputBuffer.pu8YBuf == NULL)
    {
        CHKEXPEXIT( FALSE, "Unable to allocate memory for Output Buffer" );
    }

    psMpeg4DecObject->sDecParam.sOutputBuffer.pu8CbBuf = (unsigned char *)
        malloc (psMpeg4DecObject->sDecParam.sOutputBuffer.s32CbBufLen *
        sizeof(unsigned char));
    if (psMpeg4DecObject->sDecParam.sOutputBuffer.pu8CbBuf == NULL)
    {
        CHKEXPEXIT( FALSE, "Unable to allocate memory for Output Buffer" );
    }

    psMpeg4DecObject->sDecParam.sOutputBuffer.pu8CrBuf = (unsigned char *)
        malloc (psMpeg4DecObject->sDecParam.sOutputBuffer.s32CrBufLen *
        sizeof(unsigned char));
    if (psMpeg4DecObject->sDecParam.sOutputBuffer.pu8CrBuf == NULL)
    {
        CHKEXPEXIT( FALSE, "Unable to allocate memory for Output Buffer" );
    }


    /* Allocate memory to hold the quant values, make sure that we round it
     * up in the higher side, as non-multiple of 16 will be extended to
     * next 16 bits value
     */
    psMpeg4DecObject->sDecParam.p8MbQuants = (signed char*)
        malloc (((psMpeg4DecObject->sDecParam.u16FrameWidth +15) >> 4) *
                ((psMpeg4DecObject->sDecParam.u16FrameHeight +15) >> 4) *
                sizeof (signed char));

    if (psMpeg4DecObject->sDecParam.p8MbQuants == NULL)
    {
        CHKEXPEXIT( FALSE, "Unable to allocate memory for quant values" );
    }

    /*!
    *   Allocating Memory for MPEG4 Decoder
    */
    psMemAllocInfo = &(psMpeg4DecObject->sMemInfo);
    if (s32AllocateMem4Decoder (psMpeg4DecObject) == MPEG4_ERROR)
    {
        CHKEXPEXIT( FALSE, "Unable to allocate memory for Mpeg4 Decoder" );
    }

    psMpeg4DecObject->s32EnableErrorConceal = 1;

    /*!
    *   Calling MPEG4 Decoder Init Function
    */
    STACK_TAG()
    eDecRetVal = eMPEG4DInit (psMpeg4DecObject);
    STACK_UPDATE()

    /*register frame manager for direct rendering*/
    {
        //MPEG4DSP_FrameManager manager;
        //manager.BfGetter = getBuffer;
        //manager.BfRejector = rejectBuffer;
        STACK_TAG()
        //MPEG4DSetBufferManager(psMpeg4DecObject, &manager);
        eMPEG4DSetAdditionalCallbackFunction (psMpeg4DecObject, E_GET_FRAME, (void*)getBuffer);
        eMPEG4DSetAdditionalCallbackFunction (psMpeg4DecObject, E_REJECT_FRAME, (void*)rejectBuffer);
        eMPEG4DSetAdditionalCallbackFunction (psMpeg4DecObject, E_RELEASE_FRAME, (void*)releaseBuffer);
        STACK_UPDATE()
    }

    if ((eDecRetVal != E_MPEG4D_SUCCESS) &&
        (eDecRetVal != E_MPEG4D_ENDOF_BITSTREAM))
    {
        CHKEXPEXIT( FALSE, "Initialize MPEG-4 DUT failed" );
    }
    
    return E_DEC_INIT_OK_DUT;
exit:
    /* Take care to free all memory due to error happen */
    vFreeMemAllocated4App (psMpeg4DecObject);
    if (pu8BitBuffer != NULL)
    {
        free(pu8BitBuffer);
        pu8BitBuffer = NULL;
    }
    return E_DEC_INIT_ERROR_DUT;
}


DEC_RETURN_DUT VideoDecInit_dut(void ** ppDecObj, DEC_INIT_CONTXT_DUT * pInitContxt)
{
	printf("%s \n", MPEG4DCodecVersionInfo());
    return Mpeg4DecInit(ppDecObj, pInitContxt, NULL); 
}

T_DEC_RETURN_DUT VideoTestDecInit_dut(void ** ppDecObj, T_DEC_CONTXT_DUT *psDecContxt)
{
    
    DEC_RETURN_DUT eRetVal;
	printf("%s \n", MPEG4DCodecVersionInfo());
    eRetVal = Mpeg4DecInit(ppDecObj, NULL, psDecContxt);
    
    if (eRetVal == E_DEC_INIT_OK_DUT)
    {
        return T_DEC_INIT_OK_DUT;
    }
    else
    {
        return T_DEC_INIT_ERROR_DUT;
    }
}

/*!
 ***********************************************************************
 * -Function:
 *    DEC_RETURN_DUT VideoDecFrame_dut( void * pDecObj, 
 *                                       void * pParam )
 *
 * -Description:
 *    This function calls the MPEG-4 decoder API to decode the video data in stream buffer.
 *    When it returns, it outputs one frame and store it to the buffer array if necessary.
 *
 * -Input Param
 *    *pDecObj                pointer of decoder object
 *    *pParam                 pointer of the structure contains input buffer infomation, 
 *                            or pointer of callback fuction which get video bitstreams.
 *                            for MPEG-4 DUT, the type is forced to MPG4_STREAM_BUF_DUT.
 *
 * -Output Param
 *    none
 *
 * -Return
 *    E_MPG4_DEC_FRAME_DUT    finish output one or more frame
 *    E_MPG4_DEC_FINISH_DUT   finish decoding all the bitstream
 *    E_MPG4_DED_ALLOUT_DUT   all decoded frames have been output
 *    E_MPG4_DEC_ERROR_DUT    encounter error during decoding
 ***********************************************************************
 */


DEC_RETURN_DUT Mpeg4DecFrame(void * pDecObj, void * pParam)
{
    
    eMpeg4DecRetType       eDecRetVal = E_MPEG4D_FAILURE;
    DEC_RETURN_DUT         eRetVal = E_DEC_FRAME_DUT;
    sMpeg4DecObject        *psMpeg4DecObject = pDecObj;
    sMpeg4DutObject        *psMpeg4DutObj = psMpeg4DecObject->pvAppContext;
    unsigned char          *input_buf_bitstream = NULL;
    long                   *u32BytesRead = (long *)malloc(sizeof(long));
    T_DEC_CONTXT_DUT      *psDecContxt=NULL;

    if (pParam !=NULL)
        psDecContxt = (T_DEC_CONTXT_DUT *)pParam;
    
    if(u32BytesRead == NULL){
        //printf("\33[1;34m %s, %d: Memory allocation fail!\n \33[0m", __FILE__, __LINE__);
    }
    else
      *u32BytesRead = 0;
     /* Prepare for one fram data for decoding */
     {
          unsigned int  s32EncStrBufLen;
          unsigned int s32EncStrBufLenMalloc;

        if((psMpeg4DutObj->offset)[psMpeg4DutObj->frame_num+1]==0){
            eDecRetVal = E_MPEG4D_ENDOF_BITSTREAM;
            goto DQ_BREAK;
        }

         if(psMpeg4DutObj->frame_num>0)
         {
               s32EncStrBufLen=(psMpeg4DutObj->offset)[psMpeg4DutObj->frame_num+1];
               if(s32EncStrBufLen < 8)
                    s32EncStrBufLenMalloc = 8; //To void error when doing get_bits
               else
                    s32EncStrBufLenMalloc = s32EncStrBufLen;
               input_buf_bitstream = (unsigned char *) malloc(s32EncStrBufLenMalloc);
               *u32BytesRead = fread (input_buf_bitstream, sizeof(unsigned char), s32EncStrBufLen,psMpeg4DutObj->psInputFh);             
         }
         else if(psMpeg4DutObj->frame_num==0)
         {
                fseek (psMpeg4DutObj->psInputFh,0, SEEK_SET);
                s32EncStrBufLen=(psMpeg4DutObj->offset)[0]+(psMpeg4DutObj->offset)[1];
                if(input_buf_bitstream != NULL)
                   free(input_buf_bitstream);
                if(s32EncStrBufLen < 8)
                    s32EncStrBufLenMalloc = 8; //To void error when doing get_bits
                else
                    s32EncStrBufLenMalloc = s32EncStrBufLen;    
                input_buf_bitstream = (unsigned char *) malloc(s32EncStrBufLenMalloc);
                *u32BytesRead = fread (input_buf_bitstream, sizeof(unsigned char), s32EncStrBufLen,psMpeg4DutObj->psInputFh);
        }                 
       
        /* get bitstream unit removal time for VTS */
        if ( psMpeg4DutObj->sVTSProbes.pfStoreBitsRmTime != NULL )
        {
            psMpeg4DutObj->sVTSProbes.pfStoreBitsRmTime();
        }

        /* get bitstream unit length in unit of bytes for VTS */
        if ( psMpeg4DutObj->sVTSProbes.pfStoreBitsUnitLen != NULL )
        {
            psMpeg4DutObj->sVTSProbes.pfStoreBitsUnitLen(s32EncStrBufLen);
        }
     }

     STACK_TAG()
     DEC_TIMER_BEGIN()
     eDecRetVal = eMPEG4DDecode (psMpeg4DecObject,input_buf_bitstream,u32BytesRead);
     DEC_TIMER_END()
     STACK_UPDATE()

		 if (E_MPEG4D_OUT_OF_MEMORY == eDecRetVal) {
        //printf("frame buffer not enough!\n");
        return E_DEC_ERROR_DUT;
     }

     if (psDecContxt != NULL)
        psDecContxt->uiFrameNum = psMpeg4DutObj->frame_num;
     
     psMpeg4DutObj->frame_num++;

DQ_BREAK:                 
     if(u32BytesRead != NULL) 
        free(u32BytesRead);

     /* success to decoded and output a frame */ 
    if ((eDecRetVal == E_MPEG4D_SUCCESS )||(eDecRetVal == E_MPEG4D_ERROR_CONCEALED))
    {
        STACK_TAG()
        eDecRetVal = eMPEG4DGetOutputFrame(psMpeg4DecObject);
        STACK_UPDATE()
        
        /* store output frame for VTS */
        if ( psMpeg4DutObj->sVTSProbes.pfStoreDispFrm != NULL )
        {
            UCHAR * puchLumY = psMpeg4DecObject->sDecParam.sOutputBuffer.pu8YBuf
                + 16*((psMpeg4DecObject->sDecParam.u16FrameWidth+15)/16*16+32)+16;
            UCHAR * puchChrU = psMpeg4DecObject->sDecParam.sOutputBuffer.pu8YBuf
              	+ ((psMpeg4DecObject->sDecParam.u16FrameWidth+15)/16*16+32)*((psMpeg4DecObject->sDecParam.u16FrameHeight+15)/16*16+32)
	              + 16*((psMpeg4DecObject->sDecParam.u16FrameWidth+15)/16*16+32)/4+8;
            UCHAR * puchChrV = psMpeg4DecObject->sDecParam.sOutputBuffer.pu8YBuf
                + (((psMpeg4DecObject->sDecParam.u16FrameWidth+15)/16*16+32)*((psMpeg4DecObject->sDecParam.u16FrameHeight+15)/16*16+32)*5)/4
	              + 16*((psMpeg4DecObject->sDecParam.u16FrameWidth+15)/16*16+32)/4+8;
            U32 uiPicWidth = psMpeg4DecObject->sDecParam.u16FrameWidth;
            U32 uiPicHeight = psMpeg4DecObject->sDecParam.u16FrameHeight;
            U32 uiBufStrideY = (psMpeg4DecObject->sDecParam.u16FrameWidth+15)/16*16+32;
            U32 uiBufStrideUV = ((psMpeg4DecObject->sDecParam.u16FrameWidth+15)/16*16+32) >> 1;
        
            psMpeg4DutObj->sVTSProbes.pfStoreDispFrm( puchLumY, puchChrU, puchChrV, 
                                       uiPicWidth, uiPicHeight, 
                                       uiBufStrideY, uiBufStrideUV );

        }
        /* get output time for VTS */
        if ( psMpeg4DutObj->sVTSProbes.pfStoreFrmOutTime != NULL )
        {
            psMpeg4DutObj->sVTSProbes.pfStoreFrmOutTime();
        }

        /* for test application*/
		    if(psMpeg4DutObj->fnPrbDutFunc)
		    {
		        T_PROBE_PUT_FRAME prbFrm;
		        long TopOffset, LeftOffset;
					  TopOffset= 16;
					  LeftOffset = 16;

  					prbFrm.puchLumY = psMpeg4DecObject->sDecParam.sOutputBuffer.pu8YBuf;
  					prbFrm.puchChrU = psMpeg4DecObject->sDecParam.sOutputBuffer.pu8CbBuf;
  					prbFrm.puchChrV = psMpeg4DecObject->sDecParam.sOutputBuffer.pu8CrBuf;
  					prbFrm.iFrmWidth = psMpeg4DecObject->sDecParam.u16FrameWidth;
  					prbFrm.iFrmHeight = psMpeg4DecObject->sDecParam.u16FrameHeight;
  					prbFrm.iStrideLX = (psMpeg4DecObject->sDecParam.u16FrameWidth+15)/16*16+32;
  					prbFrm.iStrideLY = (psMpeg4DecObject->sDecParam.u16FrameHeight+15)/16*16+32;
  					prbFrm.iStrideUV = ((psMpeg4DecObject->sDecParam.u16FrameWidth+15)/16*16+32) >> 1;
  					prbFrm.iTopOffset = TopOffset;
  					prbFrm.iLeftOffset = LeftOffset;
					
		        psMpeg4DutObj->fnPrbDutFunc(T_PUT_FRAME, (void*)&prbFrm);
		    }

#if 0
        UCHAR * puchLumY = psMpeg4DecObject->sDecParam.sOutputBuffer.pu8YBuf;
        UCHAR * puchChrU = psMpeg4DecObject->sDecParam.sOutputBuffer.pu8CbBuf;
        UCHAR * puchChrV = psMpeg4DecObject->sDecParam.sOutputBuffer.pu8CrBuf;
        U32 uiPicWidth = psMpeg4DecObject->sDecParam.u16FrameWidth;
        U32 uiPicHeight = psMpeg4DecObject->sDecParam.u16FrameHeight;
        U32 uiBufStrideY = psMpeg4DecObject->sDecParam.u16FrameWidth;
        U32 uiBufStrideUV = psMpeg4DecObject->sDecParam.u16FrameWidth >> 1;
#endif

        eRetVal = E_DEC_FRAME_DUT;
    }
    else if ( eDecRetVal == E_MPEG4D_ENDOF_BITSTREAM )/* reach end of bitstream */
    {
        eRetVal = E_DEC_ALLOUT_DUT;
    }
    else /* encount some error */
    {
        //printf("Error eDecRetVal = %d\n",eDecRetVal);
        eRetVal = E_DEC_ERROR_DUT;
    }

    return eRetVal;
}


DEC_RETURN_DUT VideoDecFrame_dut(void * pDecObj, void * pParam)
{
    return Mpeg4DecFrame(pDecObj, NULL);
}

T_DEC_RETURN_DUT VideoTestDecFrame_dut(void * pDecObj, T_DEC_CONTXT_DUT *psDecContxt)
{
    DEC_RETURN_DUT eRetVal;
    eRetVal = Mpeg4DecFrame(pDecObj, psDecContxt);
    switch(eRetVal)
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


/*!
 ***********************************************************************
 * -Function:
 *    void VideoDecRelease_dut( void *pDecObj )
 *
 * -Description:
 *    This function releases all the memory used by MPEG-4 DUT, and exit the decoder.
 *
 * -Input Param
 *    *pDecObj                pointer of decoder object
 *
 * -Output Param
 *    none
 *
 * -Return
 *    none
 ***********************************************************************
 */

static DEC_RETURN_DUT Mpeg4DecRelease(void *pDecObj, void *pParam)
{
    sMpeg4DecObject       *psMpeg4DecObject = pDecObj;
    sMpeg4DutObject       *psMpeg4DutObj = psMpeg4DecObject->pvAppContext;
    eMpeg4DecRetType      eDecRetVal;
    T_DEC_CONTXT_DUT      *psDecContxt=NULL;

    //printf("Now free\n");
    if (pParam !=NULL)
        psDecContxt = (T_DEC_CONTXT_DUT *)pParam;
    
    STACK_TAG()
    eDecRetVal = eMPEG4DFree (psMpeg4DecObject);
    STACK_UPDATE()
    /* Freeing resources used by MPEG4 Decoder */
    //printf("Now free app\n");
    /*! Freeing Memory allocated by the Application */
    vFreeMemAllocated4App (psMpeg4DecObject);
    releaseBuffers(psMpeg4DutObj);

    return E_DEC_REL_OK_DUT;
}

DEC_RETURN_DUT VideoDecRelease_dut(void *pDecObj)
{
    Mpeg4DecRelease(pDecObj, NULL);
    return E_DEC_REL_OK_DUT;
}

T_DEC_RETURN_DUT VideoTestDecRelease_dut(void *pDecObj, T_DEC_CONTXT_DUT *psDecContxt)
{
    Mpeg4DecRelease(pDecObj, psDecContxt);
    return T_DEC_REL_ERROR_DUT;
}


/*!
 ***********************************************************************
 * -Function:
 *    DEC_RETURN_DUT VideoDecSetProbes_dut( void * pDecObj, 
 *                                void * pProbe )
 *
 * -Description:
 *    This function set the callback function pointers used by MPEG-4 DUT.
 *
 * -Input Param
 *    *pDecObj                pointer of decoder object
 *    *pProbe                 pointer of callback function probes structure
 *                            for MPEG-4 DUT, it is forced to MPG4_CONFTEST_PROBES_DUT
 *
 * -Output Param
 *    none
 *
 * -Return
 *    none
 ***********************************************************************
 */
DEC_RETURN_DUT VideoDecSetProbes_dut(void *pDecObj, VTS_PROBES_DUT *pProbe)
{
    sMpeg4DecObject       *psMpeg4DecObject = pDecObj;
    sMpeg4DutObject       *psMpeg4DutObj = psMpeg4DecObject->pvAppContext;
    memcpy(&(psMpeg4DutObj->sVTSProbes), pProbe, sizeof(VTS_PROBES_DUT));
    return E_SET_PROB_OK_DUT;
}


T_DEC_RETURN_DUT VideoTestDecSetProbes_dut( void * pDecObj, PROBE_DUT_FUNC * pProbe)
{
    sMpeg4DecObject       *psMpeg4DecObject = pDecObj;
    sMpeg4DutObject       *psMpeg4DutObj = psMpeg4DecObject->pvAppContext;
	  psMpeg4DutObj->fnPrbDutFunc = pProbe;
	  return T_SET_PROB_OK_DUT;
}



#if 0
DEC_RETURN_DUT QueryAPIVersion_dut( unsigned char _auchVer[] )
{
    strcpy( _auchVer, DUT_API_HEADER_VERSION );

    return E_GET_VER_OK_DUT;
}
#endif


/***************************************************************************
 * DUT Malloc and Free functions
 ***************************************************************************/

void * pvAllocateFastMem (int size, int align)
{
    return malloc(size);
}

void * pvAllocateSlowMem (int size, int align)
{
    return malloc(size);
}


int s32AllocateMem4Decoder (sMpeg4DecObject* psMpeg4DecObject)
{
    int s32MemBlkCnt = 0;
    sMpeg4DecMemBlock  *psMemInfo;
    sMpeg4DecMemAllocInfo *psMemAllocInfo = &(psMpeg4DecObject->sMemInfo);
    sMpeg4DutObject *psMpeg4DutObj = psMpeg4DecObject->pvAppContext;

    for (s32MemBlkCnt = 0; s32MemBlkCnt < psMemAllocInfo->s32NumReqs;
         s32MemBlkCnt++)
    {
        psMemInfo = &psMemAllocInfo->asMemBlks[s32MemBlkCnt];

        if (MPEG4D_IS_FAST_MEMORY (psMemInfo->s32Type))
            psMemInfo->pvBuffer = pvAllocateFastMem (psMemInfo->s32Size,
                                                     psMemInfo->s32Align);
        else
            psMemInfo->pvBuffer = pvAllocateSlowMem (psMemInfo->s32Size,
                                                     psMemInfo->s32Align);

        if (psMemInfo->pvBuffer == NULL)
        {
            return MPEG4_ERROR;
        }
        //printf("size: %d\n",psMemInfo->s32Size);
        //HEAP_INCREASE((psMemInfo->s32Size))
    }
    return MPEG4_SUCCESS;
}


void vFreeMemAllocated4Dec (sMpeg4DecObject* psMpeg4DecObject)
{
    int s32MemBlkCnt = 0;
    sMpeg4DecMemAllocInfo *psMemAllocInfo = &(psMpeg4DecObject->sMemInfo);
    sMpeg4DutObject *psMpeg4DutObj = psMpeg4DecObject->pvAppContext;
    
    for (s32MemBlkCnt = 0; s32MemBlkCnt < psMemAllocInfo->s32NumReqs;
         s32MemBlkCnt++)
    {
        if (psMemAllocInfo->asMemBlks[s32MemBlkCnt].pvBuffer != NULL)
        {
            HEAP_INCREASE((psMemAllocInfo->asMemBlks[s32MemBlkCnt].s32Size))
            free(psMemAllocInfo->asMemBlks[s32MemBlkCnt].pvBuffer);
            psMemAllocInfo->asMemBlks[s32MemBlkCnt].pvBuffer = NULL;
        }
    }

}

void vFreeMemAllocated4App (sMpeg4DecObject* psMpeg4DecObject)
{
    sMpeg4DecMemAllocInfo   *psMemAllocInfo;
    sMpeg4DutObject *psMpeg4DutObj = psMpeg4DecObject->pvAppContext;

    psMemAllocInfo = &(psMpeg4DecObject->sMemInfo);

    /* Freeing Memory Allocated by the Application for Decoder */
    vFreeMemAllocated4Dec (psMpeg4DecObject);
    psMemAllocInfo = NULL;
#if 0 // For DR
    if (psMpeg4DecObject->sDecParam.sOutputBuffer.pu8YBuf != NULL)
    {
        free (psMpeg4DecObject->sDecParam.sOutputBuffer.pu8YBuf);
        psMpeg4DecObject->sDecParam.sOutputBuffer.pu8YBuf = NULL;
    }
    if (psMpeg4DecObject->sDecParam.sOutputBuffer.pu8CbBuf != NULL)
    {
        free (psMpeg4DecObject->sDecParam.sOutputBuffer.pu8CbBuf);
        psMpeg4DecObject->sDecParam.sOutputBuffer.pu8CbBuf = NULL;
    }
    if (psMpeg4DecObject->sDecParam.sOutputBuffer.pu8CrBuf != NULL)
    {
        free (psMpeg4DecObject->sDecParam.sOutputBuffer.pu8CrBuf);
        psMpeg4DecObject->sDecParam.sOutputBuffer.pu8CrBuf = NULL;
    }
#endif
    if (psMpeg4DecObject->pvAppContext != NULL)
    {
        if (psMpeg4DutObj->offset != NULL)
        {
            free(psMpeg4DutObj->offset);
            psMpeg4DutObj->offset = NULL;
        }
        psMpeg4DutObj->psInputFh = NULL;
        psMpeg4DecObject->pvMpeg4Obj = NULL;
        psMpeg4DecObject->pvAppContext = NULL;

    }   
    if (psMpeg4DecObject != NULL)
    {
        free(psMpeg4DecObject);
        psMpeg4DecObject = NULL;
    }
}

void vInitailizeMpeg4DecObject(sMpeg4DecObject* psMpeg4DecObject)
{
	int S32Count;
	// Memory info initialization
	psMpeg4DecObject->sMemInfo.s32NumReqs = 0;
	for(S32Count= 0; S32Count < MAX_NUM_MEM_REQS;S32Count++)
	{
  		psMpeg4DecObject->sMemInfo.asMemBlks[S32Count].s32Size = 0;
  		psMpeg4DecObject->sMemInfo.asMemBlks[S32Count].s32Type = 0;
  		psMpeg4DecObject->sMemInfo.asMemBlks[S32Count].s32Priority = 0;
  		psMpeg4DecObject->sMemInfo.asMemBlks[S32Count].s32Align = E_MPEG4D_ALIGN_NONE;
  		psMpeg4DecObject->sMemInfo.asMemBlks[S32Count].pvBuffer = NULL;
	}

    //sMpeg4DecYCbCrBuffer sOutputBuffer;
  	psMpeg4DecObject->sDecParam.sOutputBuffer.pu8YBuf  = NULL;
  	psMpeg4DecObject->sDecParam.sOutputBuffer.pu8CbBuf = NULL;
  	psMpeg4DecObject->sDecParam.sOutputBuffer.pu8CrBuf = NULL;
  	psMpeg4DecObject->sDecParam.sOutputBuffer.s32YBufLen = 0;
  	psMpeg4DecObject->sDecParam.sOutputBuffer.s32CbBufLen= 0;
  	psMpeg4DecObject->sDecParam.sOutputBuffer.s32CrBufLen= 0;

    psMpeg4DecObject->sDecParam.u16FrameWidth  = 0;
    psMpeg4DecObject->sDecParam.u16FrameHeight = 0;
    psMpeg4DecObject->sDecParam.u16DecodingScheme = MPEG4D_START_DECODE_AT_IFRAME;
    psMpeg4DecObject->sDecParam.u16TicksPerSec = 0;

	  //sMpeg4DecTime        sTime;
	  psMpeg4DecObject->sDecParam.sTime.s32Seconds;
    psMpeg4DecObject->sDecParam.sTime.s32MilliSeconds;
    psMpeg4DecObject->sDecParam.s32TimeIncrementInTicks = 1;
    psMpeg4DecObject->sDecParam.u8VopType = 'I';
	  psMpeg4DecObject->sDecParam.p8MbQuants = NULL;


    //sMpeg4DecVisualSceneParams  sVisParam;
	  psMpeg4DecObject->sVisParam.s32NumberOfVos = 1;
  	for(S32Count=0;S32Count<MAX_VIDEO_OBJECTS;S32Count++)
  	{
  		  psMpeg4DecObject->sVisParam.as32NumberOfVols[S32Count] = 1;
  	}

    psMpeg4DecObject->pvMpeg4Obj	= NULL;
    psMpeg4DecObject->pvAppContext	= NULL;
    psMpeg4DecObject->eState		= E_MPEG4D_INVALID;
    psMpeg4DecObject->ptr_cbkMPEG4DBufRead = NULL;
}


int initBuffers(void* AppContext, int size)
{
    sMpeg4DutObject *psMpeg4DutObj = AppContext;
  	int i;
  	for (i = 0; i < BUFFER_NUM; i ++)
  	{
    		if (NULL == ((psMpeg4DutObj->Buffers)[i] = malloc(size)))
    		{
    			return 0;
    		}
  	}
  	return 1;
}
void releaseBuffers(void* AppContext)
{
    sMpeg4DutObject *psMpeg4DutObj = AppContext;
  	int i;
  	for (i = 0; i< BUFFER_NUM; i ++)
  	{
  		  if ( NULL != (psMpeg4DutObj->Buffers)[i])
  			    free((psMpeg4DutObj->Buffers)[i]);
  	}
  	printf("@@@@total %d buffers are provided to decoder\n",psMpeg4DutObj->bufferIndex);
}
void* getBuffer(void* AppContext)
{
    sMpeg4DutObject *psMpeg4DutObj = AppContext;
  	void* temp = (psMpeg4DutObj->Buffers)[(psMpeg4DutObj->bufferIndex)%BUFFER_NUM];
  	psMpeg4DutObj->bufferIndex++;//printf("buffer count:%d\n",bufferIndex-1);
  	//if (5 == bufferIndex) return NULL;
  	//printf("got %x with Appcontext %x\n",temp,AppContext);
  	return temp;
}

void rejectBuffer(void* buffer,void* AppContext)
{
}

void releaseBuffer(void* buffer,void* AppContext)
{
    sMpeg4DutObject *psMpeg4DutObj = AppContext;
    //printf("psMpeg4DutObj->bufLen = %d\n",psMpeg4DutObj->bufLen);
    memset((char*)buffer,0,psMpeg4DutObj->bufLen);
    return;
}