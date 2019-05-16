
/*
********************************************************************************
* Copyright 2005-2010 by Freescale Semiconductor, Inc.
* All modifications are confidential and proprietary information
* of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
*
*******************************************************************************
*
*
* Description: Test Application for testing MPEG2 Decoder. This Test Application
*              exemplifies the usage of MPEG2 Decoder API's.
* Functions Included:    
* 1. int cbkMPEG2DBufRead (int s32EncStrBufLen, unsigned char *pu8EncStrBuf,
*                          int s32Offset, void *pvAppContext);
* 2. eMpeg2DecRetType eMPEG2Decode(sMpeg2DecObject *psMp2Obj,unsigned int *s32decodedBytes, void *pvAppContext)
* 3. int s32AllocateMem2Decoder (sMPEG2DecMemAllocInfo *psMemAllocInfo);
* 4. void vInitailizeMpeg2DecObject(sMpeg2DecObject* psMpeg2DecObject);
* 5. eMpeg2DecRetType eMPEG2DFree(sMpeg2DecObject *psMp2Obj);
* 6. int eMPEG2KevWrite(sIOFileParams file_info,sMpeg2DecObject *pMp2DObj); 
* 7. void eMPEG2KevClose (sIOFileParams file_info);
*
*    DD/MMM/YYYY   Code Ver      Description				Author
*    -----------      --------     -----------			------------
*    24/Oct/2007        01             Created				Eagle Zhou : add dut wrapper: ENGR00053833
*    09/Nov/2007        02            DR(ENGR00055417)           Wang Zening   
*    01/Feb/2008        03            ENGR00061975                 Eagle Zhou: modify for re-entrant
*    22/Feb/2008        04       							Eagle Zhou: add support for the newest interface(including test application)
*    21/May/2008        05	        engr00077044 			Eagle Zhou: porting stand alone (RVDS)
*    27/June/2008       06          ENGR00081514: 			Eagle Zhou: add API version and demo protection
*    11/Aug/2009        07          ENGR00115078                   Eagle Zhou: add dropping B frames
*    20/May/2010        08                                                Eagle Zhou: unify wrapper version
*************************************************************************************************/


#include "MPEG2DecTestApp.h"

#include "dut_api_vts.h"
#include "dut_test_api.h"
#include "common.h"

#define TEST_RELEASEBUF

#define EXIT_VALUE	0
#define MAX_PICTURE_SIZE (4*1024*1024)

#define ISO_END_CODE            0x1B9
#define PACK_START_CODE         0x1BA
#define SYSTEM_START_CODE       0x1BB

#define VIDEO_ELEMENTARY_STREAM 0x1e0

#define SEQUENCE_END_CODE       0x1B7

#ifdef TEST_RELEASEBUF
#define BUFFER_NUM 3 //4/5
#else
#define BUFFER_NUM 3
#endif

#define BUFF_VALID		1
#define BUFF_INVALID	0
#define BUFF_RELEASE	1
#define BUFF_UNRELEASE	0
#define BUFF_DISPLAY	1
#define BUFF_UNDISPLAY	0


typedef struct
{
	int g_cur;			// valid bytes already read in pes packet
	int g_max;			// total valid bytes in pes packet
	int g_system;		// whether pes packet is exist in stream?	
}sStreamPesPacketInfo;

typedef struct
{
	unsigned char* Buffers[BUFFER_NUM];
	unsigned char buff_flag_valid[BUFFER_NUM];  // it is set valid when decoder output this frame to application
	unsigned char buff_flag_release[BUFFER_NUM]; // it is set when decoder will not reference this frame
	unsigned char buff_flag_display[BUFFER_NUM]; // it is set when application output this frame after post-process
	int bufferIndex;
	int size;
}sBufManager;

typedef struct
{
	sMpeg2DecObject sMpeg2DecObject;

	unsigned char bits[MAX_PICTURE_SIZE];
	int giAllOut;
	sIOFileParams sIOFileParamsObj;
	int frame_num;
	sStreamPesPacketInfo sPesPacketInfo;
	sBufManager sBufManagerInfo;
#ifdef VTS_17
	VTS_PROBES_DUT sVTSProbes;// for vts17
#else	
	FuncProbeDut * pfnVtsProbes;  //for vts >1.8
#endif
	long (*fnPrbDutFunc)(T_PROBE_TYPE prb_type, void *para);		// for new test application
	
} sMpeg2DecWrapperObject;



#define DEC_TIMER_BEGIN() {if(psDecWrapperObj->fnPrbDutFunc) psDecWrapperObj->fnPrbDutFunc(T_DECODE_BEGIN, 0);}
#define DEC_TIMER_END() {if(psDecWrapperObj->fnPrbDutFunc) psDecWrapperObj->fnPrbDutFunc(T_DECODE_END, 0);}


#define STACK_TAG() {if(psDecWrapperObj->fnPrbDutFunc) psDecWrapperObj->fnPrbDutFunc(T_STACK_TAG, 0);}
#define STACK_UPDATE() {if(psDecWrapperObj->fnPrbDutFunc) psDecWrapperObj->fnPrbDutFunc(T_STACK_UPDATE, 0);}
 
#define STACK_CALLBACK_TAG() {if(psDecWrapperObj->fnPrbDutFunc) psDecWrapperObj->fnPrbDutFunc(T_STACK_CB_TAG, 0);}
#define STACK_CALLBACK_UPDATE() {if(psDecWrapperObj->fnPrbDutFunc) psDecWrapperObj->fnPrbDutFunc(T_STACK_CB_UPDATE, 0);}

#define HEAP_INCREASE(size) {if(psDecWrapperObj->fnPrbDutFunc) psDecWrapperObj->fnPrbDutFunc(T_HEAP_INCREASE, (void*)size);}
#define HEAP_DECREASE(size) {if(psDecWrapperObj->fnPrbDutFunc) psDecWrapperObj->fnPrbDutFunc(T_HEAP_DECREASE, (void*)size);}

#define BUF_BITS_LEN_CB(size)	{if(psDecWrapperObj->fnPrbDutFunc) psDecWrapperObj->fnPrbDutFunc(T_BITS_UNIT_LEN, (void*)size);}


#define BUF_MANAGER_INIT(para)  {if(psDecWrapperObj->fnPrbDutFunc) psDecWrapperObj->fnPrbDutFunc(T_INIT_BUFFER, para);}
#define BUF_MANAGER_GET(buf)  {if(psDecWrapperObj->fnPrbDutFunc) {buf=psDecWrapperObj->fnPrbDutFunc(T_GET_BUFFER, NULL);} \
							else {buf=NULL;}}
#define BUF_MANAGER_REJECT(buf)  {if(psDecWrapperObj->fnPrbDutFunc) psDecWrapperObj->fnPrbDutFunc(T_REJ_BUFFER, buf);}
#define BUF_MANAGER_RELEASE(buf)  {if(psDecWrapperObj->fnPrbDutFunc) psDecWrapperObj->fnPrbDutFunc(T_REL_BUFFER, buf);}
#define BUF_MANAGER_FREE(obj)  {if(psDecWrapperObj->fnPrbDutFunc) psDecWrapperObj->fnPrbDutFunc(T_FREE_BUFFER, obj);}

// macro for probe of vts
#ifdef VTS_17
#define VTS_INPUT_LENGTH(size) if (psDecWrapperObj->sVTSProbes.pfStoreBitsUnitLen != NULL ){psDecWrapperObj->sVTSProbes.pfStoreBitsUnitLen(size);}
#define VTS_INPUT_STARTTIME()
#define VTS_INPUT_ENDTIME()
#define VTS_OUTPUT_FRAME(pstruct)
#define VTS_OUTPUT_STARTTIME()
#define VTS_OUTPUT_ENDTIME() if (psDecWrapperObj->sVTSProbes.pfStoreFrmOutTime!= NULL )	{psDecWrapperObj->sVTSProbes.pfStoreFrmOutTime();}
#define VTS_STOREBIT_RMTIME() if (psDecWrapperObj->sVTSProbes.pfStoreBitsRmTime != NULL ){psDecWrapperObj->sVTSProbes.pfStoreBitsRmTime();}

#else
#define VTS_INPUT_LENGTH(psize) {if(psDecWrapperObj->pfnVtsProbes) psDecWrapperObj->pfnVtsProbes(E_INPUT_LENGTH,psize);}
#define VTS_INPUT_STARTTIME() {if(psDecWrapperObj->pfnVtsProbes) psDecWrapperObj->pfnVtsProbes(E_INPUT_STARTTIME,NULL);}
#define VTS_INPUT_ENDTIME() {if(psDecWrapperObj->pfnVtsProbes) psDecWrapperObj->pfnVtsProbes(E_INPUT_ENDTIME,NULL);}
#define VTS_OUTPUT_FRAME(pstruct) {if(psDecWrapperObj->pfnVtsProbes) psDecWrapperObj->pfnVtsProbes(E_OUTPUT_FRAME,pstruct);}
#define VTS_OUTPUT_STARTTIME() {if(psDecWrapperObj->pfnVtsProbes) psDecWrapperObj->pfnVtsProbes(E_OUTPUT_STARTTIME,NULL);}
#define VTS_OUTPUT_ENDTIME() {if(psDecWrapperObj->pfnVtsProbes) psDecWrapperObj->pfnVtsProbes(E_OUTPUT_ENDTIME,NULL);}
#define VTS_STOREBIT_RMTIME()
#endif


//#define TRACE_WRAPPER
#ifdef TRACE_WRAPPER
#define TRACE_PRINTF	printf
#else
#define TRACE_PRINTF
#endif


/*! Just to allocate a block of  memory. We have not taken 
 *  any extra step to align the memory to the required type. Assumption is 
 *  that the allocated memory is always word aligned. */

void * pvAllocateFastMem (int size)
{
    return (malloc(size));
}


/*!
*******************************************************************************
* Description: This is an application domain fucntion. It allocates memory
*              required by the Decoder.
*
* \param[in]  psMemAllocInfo - is a pointer to a structure which holds
*             allocated memory. This allocated memory is required by the
*             decoder.
*
* \return     returns the status of Memory Allocation, ERROR/ SUCCESS
* \remarks    
*             Global Variables: None
*
* \attention  Range Issues: None
* \attention  Special Issues: None
*
*
*/

int s32AllocateMem2Decoder (sMpeg2DecMemAllocInfo *psMemAllocInfo)
{
    int s32MemBlkCnt = 0, cnt;
    sMpeg2DecMemBlock  *psMemInfo;

    for (s32MemBlkCnt = psMemAllocInfo->s32BlkNum; s32MemBlkCnt < (psMemAllocInfo->s32NumReqs+psMemAllocInfo->s32BlkNum);
         s32MemBlkCnt++)
    {      
        psMemInfo = &psMemAllocInfo->asMemBlks[s32MemBlkCnt];

        psMemInfo->pvBuffer = pvAllocateFastMem (psMemInfo->s32Size);
		if (psMemInfo->pvBuffer == NULL)
        {              
			if(s32MemBlkCnt != 0)
			{
				for(cnt=0;cnt<(s32MemBlkCnt-1);cnt++)
				{
					psMemInfo = &psMemAllocInfo->asMemBlks[s32MemBlkCnt];
					free(psMemInfo->pvBuffer);
				}
			}
            return ERROR; 
        }
       else
       {
		  //HEAP_UPDATE(psMemInfo->s32Size);
       }

    }

    return SUCCESS;         
}

void vInitailizeMpeg2DecObject(sMpeg2DecObject* psMpeg2DecObject)
{
	int S32Count;
	// Memory info initialization
	psMpeg2DecObject->sMemInfo.s32NumReqs = 0;
	for(S32Count= 0; S32Count < MAX_NUM_MEM_REQS;S32Count++)
	{
		psMpeg2DecObject->sMemInfo.asMemBlks[S32Count].s32Size = 0;
		psMpeg2DecObject->sMemInfo.asMemBlks[S32Count].pvBuffer = NULL;
	}

   
	psMpeg2DecObject->sDecParam.sOutputBuffer.pu8YBuf  = NULL;
	psMpeg2DecObject->sDecParam.sOutputBuffer.s32YBufLen = 0;

    psMpeg2DecObject->sDecParam.u16FrameWidth  = 0;
    psMpeg2DecObject->sDecParam.u16FrameHeight = 0;
	psMpeg2DecObject->sDecParam.bitrate = 0;

   	psMpeg2DecObject->pvMpeg2Obj	= NULL;
    psMpeg2DecObject->pvAppContext	= NULL;
    psMpeg2DecObject->eState		= E_MPEG2D_INVALID;
    psMpeg2DecObject->ptr_cbkMPEG2DBufRead = NULL;
}

/*!
*******************************************************************************
* Description:It deallocates all the memory which was allocated by Application
*
* \param[in]  psMpeg2DecObject - is a pointer to Mpeg2 Decoder
*                                Configuration structure.
*
* \return     enum type
* \remarks    
*             Global Variables: None
*
* \attention  Range Issues: None
* \attention  Special Issues: None
*
* 
*/
eMpeg2DecRetType eMPEG2DFree(sMpeg2DecObject *psMp2Obj)
{ 
	sMpeg2DecMemAllocInfo *psMemAllocInfo;
    int s32MemBlkCnt = 0;
	psMemAllocInfo = &(psMp2Obj->sMemInfo);
    
     
    for (s32MemBlkCnt = 0; s32MemBlkCnt < MAX_NUM_MEM_REQS; s32MemBlkCnt++)
    {
        if (psMemAllocInfo->asMemBlks[s32MemBlkCnt].pvBuffer != NULL)
        {
            free (psMemAllocInfo->asMemBlks[s32MemBlkCnt].pvBuffer);
            psMemAllocInfo->asMemBlks[s32MemBlkCnt].pvBuffer = NULL;
        }      
    }        
     

    return(E_MPEG2D_SUCCESS);
}


int initFrmBuffers(int size,sMpeg2DecWrapperObject* psDecWrapperObj)
{
	int i;
	for (i = 0; i < BUFFER_NUM; i ++)
	{
		if (NULL == (psDecWrapperObj->sBufManagerInfo.Buffers[i] = malloc(size)))
		{
			return 0;
		}
	}
	return 1;
}
void releaseFrmBuffers(sMpeg2DecWrapperObject* psDecWrapperObj)
{
	int i;
	for (i = 0; i< BUFFER_NUM; i ++)
	{
		if ( NULL != psDecWrapperObj->sBufManagerInfo.Buffers[i])
			free(psDecWrapperObj->sBufManagerInfo.Buffers[i]);
	}
	//printf("@@@@total %d buffers are provided to decoder\n",psDecWrapperObj->sBufManagerInfo.bufferIndex);
}

void* getBuffer(void* pDecWrapperObj)
{
	sMpeg2DecWrapperObject* psDecWrapperObj=(sMpeg2DecWrapperObject*)pDecWrapperObj;
	void* temp;
	int i;
	static int count=0;
	for(i=psDecWrapperObj->sBufManagerInfo.bufferIndex;i<psDecWrapperObj->sBufManagerInfo.bufferIndex+BUFFER_NUM;i++)
	{
		if(psDecWrapperObj->sBufManagerInfo.buff_flag_valid[i%BUFFER_NUM]==BUFF_INVALID)
		{
			temp=psDecWrapperObj->sBufManagerInfo.Buffers[i%BUFFER_NUM];
			psDecWrapperObj->sBufManagerInfo.bufferIndex=i%BUFFER_NUM;
			//printf("[%d]get: 0x%X \r\n",count++,temp);
			return temp;
		}		
	}
	printf("error: buffer is full !!! \r\n");
	return NULL;
	
}
void rejectBuffer(void* buffer, void* pDecWrapperObj)
{
	static int count=0;
	sMpeg2DecWrapperObject* psDecWrapperObj=(sMpeg2DecWrapperObject*)pDecWrapperObj;
	psDecWrapperObj->sBufManagerInfo.bufferIndex++;
	//printf("[%d]rejected: 0x%X\n", count++,buffer);
}

void releaseRefBuffer(void* buffer, void* pDecWrapperObj)
{
	sMpeg2DecWrapperObject* psDecWrapperObj=(sMpeg2DecWrapperObject*)pDecWrapperObj;
	int i;
	static int count=0;	
#ifdef TEST_RELEASEBUF	
	//printf("[%d]release buffer: 0x%X , size: %d \r\n",count++,buffer,psDecWrapperObj->sBufManagerInfo.size);
	//memset(buffer,NULL,psDecWrapperObj->sBufManagerInfo.size);
#endif

	for(i=0;i<BUFFER_NUM;i++)
	{
		if(psDecWrapperObj->sBufManagerInfo.Buffers[i]==buffer)
		{
			psDecWrapperObj->sBufManagerInfo.buff_flag_release[i]=BUFF_RELEASE;
			return;
		}
	}
	printf("error: invalid buffer pointer !! \r\n");
	
	return;
}

/*this should be called after Re-Query memory, because it needs the frame size*/
void initFrameManager(sMpeg2DecObject *psMpeg2DecObject,sMpeg2DecWrapperObject* psDecWrapperObj)
{
	unsigned short int width, height;
	int size;
	width = ((psMpeg2DecObject->sDecParam.u16FrameWidth+15)/16)*16;
	height = ((psMpeg2DecObject->sDecParam.u16FrameHeight+15)/16)*16;
	size = width*height*1.5;
	initFrmBuffers(size,psDecWrapperObj);
	psDecWrapperObj->sBufManagerInfo.size=size;
}
void releaseFrameManager(sMpeg2DecWrapperObject* psDecWrapperObj)
{
	releaseFrmBuffers(psDecWrapperObj);
}



void eMPEG2FileClose (sIOFileParams file_info)
{
	if(file_info.fpInput)
		fclose(file_info.fpInput);
}

unsigned char Get_Byte(FILE* fp)
{
/*
  while(ld->Rdptr >= ld->Rdbfr+2048)
  {
    read(ld->Infile,ld->Rdbfr,2048);    
    ld->Rdptr -= 2048;
    ld->Rdmax -= 2048;
  }
  return *ld->Rdptr++;
  */
  unsigned char x;
  fread(&x, sizeof(unsigned char), 1, fp);
  //printf("x=0x%X \r\n",x);
  return x;
}

/* extract a 16-bit word from the bitstream buffer */
int Get_Word(FILE* fp)
{
#if 1
  int Val;

  Val = Get_Byte(fp);
  return (Val<<8) | Get_Byte(fp);
#else  
  unsigned short x;
  fread(&x, sizeof(unsigned short), 1, fp);
  printf("x=0x%X \r\n",x);
  return x; 
#endif
}

int Get_Long(FILE* fp)
{
#if 1
	int i;

  i = Get_Word(fp);
  return (i<<16) | Get_Word(fp);
#else
 
 int x;
 fread(&x, sizeof(unsigned int), 1, fp);
 printf("x=0x%X \r\n",x);
 return x;
#endif
}

int Next_Packet(FILE* fp,sStreamPesPacketInfo *psPesPacketInfo)
{
  unsigned int code;
  int l;

  for(;;)
  {
    code = Get_Long(fp);

	//printf("code: 0x%x \r\n",code);
    /* remove system layer byte stuffing */
    while ((code & 0xffffff00) != 0x100)
      code = (code<<8) | Get_Byte(fp);

    switch(code)
    {
    case PACK_START_CODE: /* pack header */
      /* skip pack header (system_clock_reference and mux_rate) */
      //ld->Rdptr += 8;
	fseek(fp, +8, SEEK_CUR);
      break;
    case VIDEO_ELEMENTARY_STREAM:   
      code = Get_Word(fp);             /* packet_length */
	  //printf("\n length: 0x%x \r\n",code);
      //ld->Rdmax = ld->Rdptr + code;
	psPesPacketInfo->g_max=code;  
	psPesPacketInfo->g_cur=1;

      code = Get_Byte(fp);
	  psPesPacketInfo->g_cur++;

      if((code>>6)==0x02)
      {
        //ld->Rdptr++;
        fseek(fp, +1, SEEK_CUR);
		psPesPacketInfo->g_cur++;
        code=Get_Byte(fp);  /* parse PES_header_data_length */
		psPesPacketInfo->g_cur++;
        //ld->Rdptr+=code;    /* advance pointer by PES_header_data_length */
        fseek(fp, 0+code, SEEK_CUR);
		psPesPacketInfo->g_cur+=code;
        printf("MPEG-2 PES packet\n");
        return 0;
      }
      else if(code==0xff)
      {
        /* parse MPEG-1 packet header */
        while((code=Get_Byte(fp))== 0xFF)
		{
			psPesPacketInfo->g_cur++;
		}
		psPesPacketInfo->g_cur++;
      }
       
      /* stuffing bytes */
      if(code>=0x40)
      {
        if(code>=0x80)
        {
          fprintf(stderr,"Error in packet header\n");
          exit(EXIT_VALUE);
        }
        /* skip STD_buffer_scale */
       // ld->Rdptr++;
	fseek(fp, +1, SEEK_CUR);
	psPesPacketInfo->g_cur++;
        code = Get_Byte(fp);
		psPesPacketInfo->g_cur++;
      }

      if(code>=0x30)
      {
        if(code>=0x40)
        {
          fprintf(stderr,"Error in packet header\n");
          exit(EXIT_VALUE);
        }
        /* skip presentation and decoding time stamps */
        //ld->Rdptr += 9;
        fseek(fp, +9, SEEK_CUR);
		psPesPacketInfo->g_cur+=9;
      }
      else if(code>=0x20)
      {
        /* skip presentation time stamps */
        //ld->Rdptr += 4;
        fseek(fp, +4, SEEK_CUR);
		psPesPacketInfo->g_cur+=4;
      }
      else if(code!=0x0f)
      {
        fprintf(stderr,"Error in packet header\n");
        exit(EXIT_VALUE);
      }
      return 0;
    case ISO_END_CODE: /* end */
      /* simulate a buffer full of sequence end codes */
  /*    l = 0;
      while (l<2048)
      {
        ld->Rdbfr[l++] = SEQUENCE_END_CODE>>24;
        ld->Rdbfr[l++] = SEQUENCE_END_CODE>>16;
        ld->Rdbfr[l++] = SEQUENCE_END_CODE>>8;
        ld->Rdbfr[l++] = SEQUENCE_END_CODE&0xff;
      }
      ld->Rdptr = ld->Rdbfr;
      ld->Rdmax = ld->Rdbfr + 2048;
      */
      return 1;
    default:
      if(code>=SYSTEM_START_CODE)
      {
        /* skip system headers and non-video packets*/
        code = Get_Word(fp);	
		//printf("code: 0x%x \r\n",code);
      //  ld->Rdptr += code;
      fseek(fp, 0+code, SEEK_CUR);
      }
      else
      {
        fprintf(stderr,"Unexpected startcode %08x in system layer\n",code);
        exit(EXIT_VALUE);
      }
      break;
    }
  }
  return 0;
}

void Init_pes_packet(sStreamPesPacketInfo *psPesPacketInfo)
{
	psPesPacketInfo->g_cur=0;	
	psPesPacketInfo->g_max=-1;
	psPesPacketInfo->g_system=0;	
}

void Init_Wrapper_Struct(sMpeg2DecWrapperObject* psDecWrapperObj)
{
	int i;
	for(i=0;i<BUFFER_NUM;i++)
	{
		psDecWrapperObj->sBufManagerInfo.buff_flag_valid[i]=BUFF_INVALID;
		psDecWrapperObj->sBufManagerInfo.buff_flag_release[i]=BUFF_UNRELEASE;
		psDecWrapperObj->sBufManagerInfo.buff_flag_display[i]=BUFF_UNDISPLAY;
	}
	psDecWrapperObj->sBufManagerInfo.bufferIndex=0;
	
	psDecWrapperObj->giAllOut=0;
	psDecWrapperObj->frame_num=-1;
	Init_pes_packet(&psDecWrapperObj->sPesPacketInfo);
#ifdef VTS_17	
	memset((void*)(&psDecWrapperObj->sVTSProbes),(int)NULL,sizeof(psDecWrapperObj->sVTSProbes));
#else
	memset((void*)(&psDecWrapperObj->pfnVtsProbes),(int)NULL,sizeof(psDecWrapperObj->pfnVtsProbes));	
#endif
	psDecWrapperObj->fnPrbDutFunc=NULL;
}


long IO_GetSegtion(FILE *fp, unsigned char * buf, long bufLength,sStreamPesPacketInfo *psPesPacketInfo)
{
    long    nBytes = 0, nTmp = 0;
    unsigned char     x;
    unsigned long lastWord = 0xffffffff;

//    while ((x = GetByte(p)) == 0)
//        ;                                       // Skip leading zero-bytes

    if (feof(fp))
        return 0;

//    if (x != START_CODE)
//    {
//        printf("Error: Startcode not found in NALU\n");
//    }

    while (1)
    {
    	if(psPesPacketInfo->g_system&&(psPesPacketInfo->g_cur>psPesPacketInfo->g_max))
    	{
    		if(Next_Packet(fp,psPesPacketInfo))
    		{
				buf[nBytes++] = (unsigned char) ((SEQUENCE_END_CODE >> 24) & 0xff);
				buf[nBytes++] = (unsigned char) ((SEQUENCE_END_CODE >> 16) & 0xff);
				buf[nBytes++] = (unsigned char) ((SEQUENCE_END_CODE >> 8) & 0xff);
				buf[nBytes++] = (unsigned char) ((SEQUENCE_END_CODE >> 0) & 0xff);
				fseek(fp, 0, SEEK_END);
				psPesPacketInfo->g_cur+=4;
				break;
				//return 0; // sequence end
    		}
    	}
        //fread(&x, sizeof(unsigned char), 1, fp);
        x = Get_Byte(fp);
		//printf("x: 0x%X \r\n",x);

        if (feof(fp))
        {
            int     i;

            printf("End of Bitstream\n");

            // Write out last 3 bytes
            for (i = 3 - nTmp; i < 3; i++)
            {
                buf[nBytes++] = (unsigned char) ((lastWord >> (16 - 8 * i)) &
                                         0xff);
				psPesPacketInfo->g_cur++;
            }
#if 1	// eagle 2008_02_22: add sequence end for those files which have no sequence end (such as BR00500K.MPG)
		buf[nBytes++] = (unsigned char) ((SEQUENCE_END_CODE >> 24) & 0xff);
		buf[nBytes++] = (unsigned char) ((SEQUENCE_END_CODE >> 16) & 0xff);
		buf[nBytes++] = (unsigned char) ((SEQUENCE_END_CODE >> 8) & 0xff);
		buf[nBytes++] = (unsigned char) ((SEQUENCE_END_CODE >> 0) & 0xff);
#endif			
            break;
        }

        if (nBytes > bufLength)
        {
#ifndef OS_VRTX
            printf("End of bitstream %d %d\n", nBytes, bufLength);
            printf("Error: NALU data overflows buffer[%ld]\n", bufLength);
#endif
            break;
        }

        lastWord = ((lastWord & 0x00ffffff) << 8) | (x & 0xff);
	//printf("lastWord=0x%x \r\n",lastWord);
	if((lastWord == 0x000001ba)||(lastWord == 0x000001e0))
	{
		psPesPacketInfo->g_system=1;
		fseek(fp, -4, SEEK_CUR);
		continue;
	}

        // check if end of sequence found

        if(lastWord == 0x000001b7)
        {
            buf[nBytes++] = (unsigned char) ((lastWord >> 24) & 0xff);
            buf[nBytes++] = (unsigned char) ((lastWord >> 16) & 0xff);
            buf[nBytes++] = (unsigned char) ((lastWord >> 8) & 0xff);
            buf[nBytes++] = (unsigned char) ((lastWord >> 0) & 0xff);
            fseek(fp, 0, SEEK_END);
			psPesPacketInfo->g_cur+=4;
            break;
        }
        // check if the lastWord is picture start code
        if(lastWord == 0x00000100 && nBytes > 0)
        {
            // set fp back 4 bytes
            fseek(fp, -4, SEEK_CUR);
            break;
        }

        // Check if the last 3 bytes are either startcode or 0s
//        if ((lastWord & 0x00fffffe) == 0)
//        {
//            buf[nBytes++] = (UCHAR) ((lastWord >> 24) & 0xff);
//            p->index--;
//            assert(p->index >= 0);
//            break;
//        }

        if ((nTmp >= 3))
        {
            buf[nBytes++] = (unsigned char) ((lastWord >> 24) & 0xff);
			psPesPacketInfo->g_cur++;
        }
        else
        {
            // Wait until 4 bytes have been read in
            nTmp++;
        }

    }
	//DSPhl28494
    //printf("Read %ld bytes from NALU\n", nBytes);

    return nBytes;
}

int cbkMPEG2DBufRead(int *s32EncStrBufLen,  unsigned char **pu8EncStrBuf,
                      int s32Offset, void *pDecWrapperObj)
{    static unsigned int   u32BytesRead=0;
	static unsigned int last_loc=0;
    sMpeg2DecWrapperObject* psDecWrapperObj=(sMpeg2DecWrapperObject*)pDecWrapperObj;
    sIOFileParams *psIOFileParamsObj =&(psDecWrapperObj->sIOFileParamsObj);
    unsigned char* pBits=psDecWrapperObj->bits;

	DEC_TIMER_END();
	STACK_CALLBACK_TAG();

	if(psDecWrapperObj->frame_num == 0)      
	{    	
		//fseek (psIOFileParamsObj->fpInput,0-u32BytesRead, SEEK_CUR);
		fseek (psIOFileParamsObj->fpInput,last_loc, SEEK_SET);
	}
	last_loc=ftell(psIOFileParamsObj->fpInput);
	
    u32BytesRead = IO_GetSegtion(psIOFileParamsObj->fpInput, pBits, MAX_PICTURE_SIZE,&(psDecWrapperObj->sPesPacketInfo));
    *s32EncStrBufLen = u32BytesRead;
    *pu8EncStrBuf = pBits;
    // sequence header will reparse ?    
    //if(psDecWrapperObj->frame_num == -1)
   //   fseek (psIOFileParamsObj->fpInput,0, SEEK_SET);
    psIOFileParamsObj->s32NumBytesRead += u32BytesRead;
      if (u32BytesRead == 0)    
	{
		if (feof (psIOFileParamsObj->fpInput) == 0)
		{
			STACK_CALLBACK_UPDATE();
			DEC_TIMER_BEGIN();
			return -1;
		} 
	}     	

	VTS_INPUT_ENDTIME();	
	/* get bitstream removal time for VTS */
	//if (psDecWrapperObj->sVTSProbes.pfStoreBitsRmTime != NULL )
	//{
	//  psDecWrapperObj->sVTSProbes.pfStoreBitsRmTime( );
	//}
	VTS_STOREBIT_RMTIME();
        
	/* get bitstream length in unit of bytes for VTS */
	//if (psDecWrapperObj->sVTSProbes.pfStoreBitsUnitLen != NULL )
	//{
	//  psDecWrapperObj->sVTSProbes.pfStoreBitsUnitLen( u32BytesRead );
	//}
	VTS_INPUT_LENGTH(u32BytesRead);

	// get bitstream length for test application
	BUF_BITS_LEN_CB((u32BytesRead*8));

	STACK_CALLBACK_UPDATE();	
	DEC_TIMER_BEGIN();
	
	return (u32BytesRead);
}


int GetUserInput(sMpeg2DecObject *psMp2Obj,int argc, char *argv[])
{
#define CASE_FIRST(x)   if (strncmp(argv[0], x, strlen(x)) == 0)
#define CASE(x)         else if (strncmp(argv[0], x, strlen(x)) == 0)
#define DEFAULT         else

	while (argc)
	{
		if (argv[0][0] == '-')
		{
			CASE_FIRST("-b")
			{
				MPEG2DEnableSkipBMode(psMp2Obj,1);
				printf("enable: drop B frame \r\n");
			}
			DEFAULT                             // Has to be last
			{
				printf("Unsupported option %s\n", argv[0]);
			}
		}
		else
		{
			printf("Please input '-' option \r\n");
		}
		argc--;
		argv++;
	}

	return 1;
}

//DEC_RETURN_DUT VideoDecInit_internal( void **ppDecWrapperObj, DEC_INIT_CONTXT_DUT *pFileVTSContxt, T_DEC_CONTXT_DUT *_psAPPContxt )
DEC_RETURN_DUT VideoDecInit_internal( void **ppDecWrapperObj, DUT_INIT_CONTXT_2_1 *pFileVTSContxt, T_DEC_CONTXT_DUT *_psAPPContxt )
{
    sMpeg2DecObject       *psMpeg2DecObject = NULL;
    sMpeg2DecWrapperObject * psDecWrapperObj = NULL;
    eMpeg2DecRetType       eDecRetVal = E_MPEG2D_FAILURE;
    sMpeg2DecMemAllocInfo *psMemAllocInfo;
	int flag=1;
    int bytes_read=0;
    int count=0;
    T_DEC_CONTXT_DUT *psMpegAppContxt = (T_DEC_CONTXT_DUT *)_psAPPContxt;
    DUT_INIT_CONTXT_2_1 * psMpegVTSContxt = (DUT_INIT_CONTXT_2_1 *)pFileVTSContxt;
	
    DEC_RETURN_DUT eRetVal = E_DEC_INIT_OK_DUT;

    //ASSERT((ppDecWrapperObj != NULL) && (psInitContxt!= NULL));

	// Output the WMV9 Decoder Version Info
      printf("%s \n", MPEG2DCodecVersionInfo());
	
	psDecWrapperObj = (sMpeg2DecWrapperObject *) malloc (sizeof (sMpeg2DecWrapperObject));
	if (psDecWrapperObj == NULL)
	{
		printf ("Unable to allocate memory for Mpeg2 Decoder structure\n");
		eMPEG2FileClose(psDecWrapperObj->sIOFileParamsObj);       
		return E_DEC_INIT_ERROR_DUT;
	}
	else
	{
		Init_Wrapper_Struct(psDecWrapperObj);
		psMpeg2DecObject=&psDecWrapperObj->sMpeg2DecObject;
		vInitailizeMpeg2DecObject(psMpeg2DecObject);
	}


    psDecWrapperObj->sIOFileParamsObj.s32NumBytesRead = 0; 

     if(psMpegAppContxt) psDecWrapperObj->sIOFileParamsObj.ps8EncStrFname = psMpegAppContxt->strInFile;
     if(psMpegVTSContxt) 
	{
	  psDecWrapperObj->sIOFileParamsObj.ps8EncStrFname = psMpegVTSContxt->strInFile;
#ifdef VTS_17		
		//
#else
		psDecWrapperObj->pfnVtsProbes=psMpegVTSContxt->pfProbe;
#endif
     	}

    if ((psDecWrapperObj->sIOFileParamsObj.fpInput = fopen (psDecWrapperObj->sIOFileParamsObj.ps8EncStrFname,"rb")) == NULL)
	{
		printf ("Error : Unable to open Input Bitstream file \"%s\" \n",psDecWrapperObj->sIOFileParamsObj.ps8EncStrFname);

        return E_DEC_INIT_ERROR_DUT;
	}


    /*Added for call back changes*/

	psMpeg2DecObject->pvAppContext = (void *)psDecWrapperObj;

	STACK_TAG();
	
	eDecRetVal =  eMPEG2DQuerymem (psMpeg2DecObject);

	STACK_UPDATE();
	
	if (eDecRetVal != E_MPEG2D_SUCCESS)
	{
		printf ("Function eMPEG2DQuerymem() resulted in failure\n");
		printf ("MPEG2D Error Type : %d\n", eDecRetVal);

		eMPEG2FileClose(psDecWrapperObj->sIOFileParamsObj);
	
        /*! Freeing Memory allocated by the Application */
	
		free(psMpeg2DecObject);
	
		return E_DEC_INIT_ERROR_DUT;
	} 
 
     /*!
    *   Allocating Memory for MPEG2 Decoder
    */

	psMemAllocInfo = &(psMpeg2DecObject->sMemInfo);
	if (s32AllocateMem2Decoder (psMemAllocInfo) == ERROR)
	{

	/*! Freeing Memory allocated by the Application */
		eMPEG2FileClose(psDecWrapperObj->sIOFileParamsObj);
		free (psMpeg2DecObject);
		return E_DEC_INIT_ERROR_DUT;
	}
    
	eDecRetVal = eMPEG2D_Init (psMpeg2DecObject);

    /*Added for call back changes*/

	Mpeg2_register_func(psMpeg2DecObject,cbkMPEG2DBufRead);

	Init_pes_packet(&psDecWrapperObj->sPesPacketInfo);

Retry_query:		
	//psDecWrapperObj->frame_num=-1;		

	STACK_TAG();
	eDecRetVal =  eMPEG2D_Re_Querymem(psMpeg2DecObject);

	STACK_UPDATE();	

	if (eDecRetVal != E_MPEG2D_SUCCESS)
	{	
		printf("requery fail \r\n");
		goto Retry_query;	
	}	

    psDecWrapperObj->frame_num++;
	initFrameManager(psMpeg2DecObject,psDecWrapperObj);

	if (eDecRetVal != E_MPEG2D_SUCCESS)
	{
		printf ("Function eMPEG2D_Re_Querymem() resulted in failure\n");
		printf ("MPEG2D Error Type : %d\n", eDecRetVal);
	/*! Freeing Memory allocated by the Application */
		
		eMPEG2FileClose(psDecWrapperObj->sIOFileParamsObj);
		eMPEG2DFree (psMpeg2DecObject);
		return E_DEC_INIT_ERROR_DUT;
	} 

	psMemAllocInfo = &(psMpeg2DecObject->sMemInfo);
	if (s32AllocateMem2Decoder (psMemAllocInfo) == ERROR)
	{
		eMPEG2FileClose(psDecWrapperObj->sIOFileParamsObj);
	/*! Freeing Memory allocated by the Application */
	    eMPEG2DFree (psMpeg2DecObject);
		return E_DEC_INIT_ERROR_DUT;
	}


    /*! 
    *   Calling MPEG2 Decoder Init Function
    */
	eDecRetVal = eMPEG2D_ReInit (psMpeg2DecObject);

	// set call back function
	{
		MPEG2D_FrameManager frameManager;		
		frameManager.BfGetter = getBuffer;
		frameManager.BfRejector = rejectBuffer;
		MPEG2DSetBufferManager(psMpeg2DecObject,&frameManager);
		MPEG2DSetAdditionalCallbackFunction(psMpeg2DecObject, E_RELEASE_FRAME,(void*)releaseRefBuffer);
	}


	if(psMpegAppContxt)	
	{
		if(!GetUserInput(psMpeg2DecObject, psMpegAppContxt->argc,psMpegAppContxt->argv))		
		{
			eDecRetVal=E_MPEG2D_INVALID_ARGUMENTS;
		}			
	}	

	if ((eDecRetVal != E_MPEG2D_SUCCESS) &&(eDecRetVal != E_MPEG2D_ENDOF_BITSTREAM))
	{
		eMPEG2FileClose(psDecWrapperObj->sIOFileParamsObj);
 
		/*!  Freeing Memory allocated by the Application */
		eMPEG2DFree (psMpeg2DecObject);
	
 		return E_DEC_INIT_ERROR_DUT;
	}

    *ppDecWrapperObj = psDecWrapperObj;
    Init_pes_packet(&psDecWrapperObj->sPesPacketInfo);    

    return E_DEC_INIT_OK_DUT;
}

//DEC_RETURN_DUT DLL_EXPORTS VideoDecInit_dut( void **ppDecWrapperObj, DEC_INIT_CONTXT_DUT *pFileVTSContxt )
DEC_RETURN_DUT DLL_EXPORTS VideoDecInit( void **_ppDecObj, void *_psInitContxt)
{
	return VideoDecInit_internal(_ppDecObj, _psInitContxt, NULL);
}

T_DEC_RETURN_DUT DLL_EXPORTS VideoTestDecInit_dut( void **ppDecWrapperObj, T_DEC_CONTXT_DUT *_psDecAppContxt)
{
	DEC_RETURN_DUT eDecRet;
	TRACE_PRINTF("[func, line]= [%s, %d] \r\n",__FUNCTION__, __LINE__);
	eDecRet = VideoDecInit_internal(ppDecWrapperObj, NULL,  _psDecAppContxt);
	TRACE_PRINTF("[func, line]= [%s, %d] \r\n",__FUNCTION__, __LINE__);
	return (eDecRet == E_DEC_INIT_OK_DUT) ? T_DEC_INIT_OK_DUT : T_DEC_INIT_ERROR_DUT;
}


DEC_RETURN_DUT VideoDecFrame( void *pDecWrapperObj, void *pParam )
{
	int                    s32decodedBytes=0;
	sMpeg2DecWrapperObject* psDecWrapperObj=(sMpeg2DecWrapperObject*)pDecWrapperObj;
	sMpeg2DecObject *psMpeg2DecObject = &(psDecWrapperObj->sMpeg2DecObject);
	eMpeg2DecRetType       eDecRetVal = E_MPEG2D_FAILURE;
	DEC_RETURN_DUT eRetVal = E_DEC_FRAME_DUT;
	T_DEC_CONTXT_DUT *	psDecContext=(T_DEC_CONTXT_DUT *)pParam;
	
    ASSERT(pDecWrapperObj != NULL);

    if ( psDecWrapperObj->giAllOut == 1 )
    {
        return E_DEC_ALLOUT_DUT;
    }

	STACK_TAG();
	DEC_TIMER_BEGIN();
	
	eDecRetVal = eMPEG2Decode(psMpeg2DecObject,&s32decodedBytes,psMpeg2DecObject->pvAppContext); 

	DEC_TIMER_END();
	STACK_UPDATE();

	if(eDecRetVal==E_MPEG2D_DEMO_PROTECT)
	{
		//printf("enter demo protection !!! \r\n");
		eDecRetVal=E_MPEG2D_FRAME_READY;
	}

    psDecWrapperObj->frame_num++;

	if((eDecRetVal==E_MPEG2D_FRAME_READY) ||(eDecRetVal==E_MPEG2D_ENDOF_BITSTREAM)
					||(eDecRetVal==E_MPEG2D_ERROR_STREAM)||(eDecRetVal==E_MPEG2D_FAILURE))
    {
        //printf("decoding frame %d ", frames_decoded);
        //printf("\tresolution: %d * %d \n", psMpeg2DecObject->sDecParam.u16FrameWidth, 
        //                                   psMpeg2DecObject->sDecParam.u16FrameHeight);

	if(psDecContext)
	{
		psDecContext->uiFrameNum++;
	}

        /* for test application*/
	if ( psDecWrapperObj->fnPrbDutFunc != NULL )
	{
		T_PROBE_PUT_FRAME prbFrm;
		int iFrameSize = ((psMpeg2DecObject->sDecParam.u16FrameWidth+15)&0xFFFFFFF0) *
		         ((psMpeg2DecObject->sDecParam.u16FrameHeight+15)&0xFFFFFFF0);
		unsigned char * puchLumY = psMpeg2DecObject->sDecParam.sOutputBuffer.pu8YBuf;
#ifndef YV12			
		unsigned char * puchChrU = puchLumY + iFrameSize;
		unsigned char * puchChrV = puchChrU + (iFrameSize >> 2);
#else	//YV12
		unsigned char * puchChrV = puchLumY + iFrameSize;
		unsigned char * puchChrU = puchChrV + (iFrameSize >> 2);	
#endif
		int iPicWidth = psMpeg2DecObject->sDecParam.u16FrameWidth;
		int iPicHeight = psMpeg2DecObject->sDecParam.u16FrameHeight;
		int iBufStrideY = (psMpeg2DecObject->sDecParam.u16FrameWidth+15)&0xFFFFFFF0;
		int iBufStrideUV = iBufStrideY >> 1;
		prbFrm.puchLumY = puchLumY;
		prbFrm.puchChrU = puchChrU;
		prbFrm.puchChrV = puchChrV;
		prbFrm.iFrmWidth = iPicWidth;
		prbFrm.iFrmHeight = iPicHeight;
		prbFrm.iStrideLX = iBufStrideY;
		prbFrm.iStrideLY = (((iPicHeight+15)>>4)<<4)+2*0;
		prbFrm.iStrideUV = iBufStrideUV;
		prbFrm.iTopOffset = 0;
		prbFrm.iLeftOffset = 0;

		psDecWrapperObj->fnPrbDutFunc(T_PUT_FRAME, (void*)&prbFrm);

	}

        /* store output frame for VTS */
#ifdef VTS_17
        if ( psDecWrapperObj->sVTSProbes.pfStoreDispFrm != NULL )
        {
#if 1     //1eagle : for unaligned width or height       
            int iFrameSize = ((psMpeg2DecObject->sDecParam.u16FrameWidth+15)&0xFFFFFFF0) *
                             ((psMpeg2DecObject->sDecParam.u16FrameHeight+15)&0xFFFFFFF0);
            unsigned char * puchLumY = psMpeg2DecObject->sDecParam.sOutputBuffer.pu8YBuf;
#ifndef YV12			
            unsigned char * puchChrU = puchLumY + iFrameSize;
            unsigned char * puchChrV = puchChrU + (iFrameSize >> 2);
#else	//YV12
            unsigned char * puchChrV = puchLumY + iFrameSize;
            unsigned char * puchChrU = puchChrV + (iFrameSize >> 2);	
#endif
            int iPicWidth = psMpeg2DecObject->sDecParam.u16FrameWidth;
            int iPicHeight = psMpeg2DecObject->sDecParam.u16FrameHeight;
            int iBufStrideY = (psMpeg2DecObject->sDecParam.u16FrameWidth+15)&0xFFFFFFF0;
            int iBufStrideUV = iBufStrideY >> 1;
#else
            int iFrameSize = psMpeg2DecObject->sDecParam.u16FrameWidth *
                             psMpeg2DecObject->sDecParam.u16FrameHeight;
            unsigned char * puchLumY = psMpeg2DecObject->sDecParam.sOutputBuffer.pu8YBuf;
            unsigned char * puchChrU = puchLumY + iFrameSize;
            unsigned char * puchChrV = puchChrU + (iFrameSize >> 2);
            int iPicWidth = psMpeg2DecObject->sDecParam.u16FrameWidth;
            int iPicHeight = psMpeg2DecObject->sDecParam.u16FrameHeight;
            int iBufStrideY = psMpeg2DecObject->sDecParam.u16FrameWidth;
            int iBufStrideUV = psMpeg2DecObject->sDecParam.u16FrameWidth >> 1;
#endif
            psDecWrapperObj->sVTSProbes.pfStoreDispFrm( puchLumY, puchChrU, puchChrV, 
                                       iPicWidth, iPicHeight, 
                                       iBufStrideY, iBufStrideUV );
        }
#else
	if ( psDecWrapperObj->pfnVtsProbes != NULL )
	{
		FRAME_COPY_INFO sFrmInfo;
		int iFrameSize = ((psMpeg2DecObject->sDecParam.u16FrameWidth+15)&0xFFFFFFF0) *
                             ((psMpeg2DecObject->sDecParam.u16FrameHeight+15)&0xFFFFFFF0);
		sFrmInfo.puchLumY = psMpeg2DecObject->sDecParam.sOutputBuffer.pu8YBuf;	
#ifndef YV12			
		sFrmInfo.puchChrU = sFrmInfo.puchLumY + iFrameSize;
		sFrmInfo.puchChrV = sFrmInfo.puchChrU+ (iFrameSize >> 2);
#else
		sFrmInfo.puchChrV = sFrmInfo.puchLumY + iFrameSize;
		sFrmInfo.puchChrU = sFrmInfo.puchChrV+ (iFrameSize >> 2);
#endif
		sFrmInfo.iFrmWidth =psMpeg2DecObject->sDecParam.u16FrameWidth;
		sFrmInfo.iFrmHeight= psMpeg2DecObject->sDecParam.u16FrameHeight;
		sFrmInfo.iBufStrideY = (psMpeg2DecObject->sDecParam.u16FrameWidth+15)&0xFFFFFFF0;
		sFrmInfo.iBufStrideUV =  sFrmInfo.iBufStrideY >> 1;
		VTS_OUTPUT_FRAME(&sFrmInfo);
	}	
#endif
	// VTS: after output frame
	VTS_OUTPUT_ENDTIME();

	  //printf("output one frame: 0x%X \r\n",psMpeg2DecObject->sDecParam.sOutputBuffer.pu8YBuf);
#ifdef TEST_RELEASEBUF
	{
		int i,mark;
		mark=0;
		for(i=0;i<BUFFER_NUM;i++)
		{
			if(psMpeg2DecObject->sDecParam.sOutputBuffer.pu8YBuf==psDecWrapperObj->sBufManagerInfo.Buffers[i])
			{	
				mark=1;						
			}
			if(psDecWrapperObj->sBufManagerInfo.buff_flag_release[i]==BUFF_RELEASE)
			{
				//printf("release buffer: 0x%X, size: %d \r\n",psDecWrapperObj->sBufManagerInfo.Buffers[i],psDecWrapperObj->sBufManagerInfo.size);
				memset(psDecWrapperObj->sBufManagerInfo.Buffers[i],NULL,psDecWrapperObj->sBufManagerInfo.size);
				psDecWrapperObj->sBufManagerInfo.buff_flag_release[i]=BUFF_UNRELEASE;
			}
		}
		if(mark==0)
		{
			printf("error: can not find buffer pointer in buffer pool !!! \r\n");	
		}
	}
#endif


        /* convert return value */
        eRetVal = E_DEC_FRAME_DUT;
        if (eDecRetVal==E_MPEG2D_ENDOF_BITSTREAM)
        {
            eRetVal = E_DEC_ALLOUT_DUT;
            psDecWrapperObj->giAllOut = 1;
        }

	}
	else if ((eDecRetVal==E_MPEG2D_PARTIAL_DECODE) || (eDecRetVal==E_MPEG2D_SUCCESS)
		||(eDecRetVal==E_MPEG2D_ERROR_STREAM)||(eDecRetVal==E_MPEG2D_FAILURE))
	{
	    eRetVal = E_DEC_FRAME_DUT;
	}
	else if(eDecRetVal==E_MPEG2D_FRAME_SKIPPED)
	{
		if(psDecContext)
		{
			psDecContext->uiFrameNum++;
		}
		eRetVal = E_DEC_FRAME_DUT;
	}
	else
	{
	    eRetVal = E_DEC_ALLOUT_DUT;
	}

	return eRetVal;
}


DEC_RETURN_DUT DLL_EXPORTS VideoDecFrame_dut( void * _pDecWrapperObj, void * _pParam )
{
	//return VideoDecFrame(_pDecObj, _pParam);
	return VideoDecFrame(_pDecWrapperObj,NULL);
}

T_DEC_RETURN_DUT DLL_EXPORTS VideoTestDecFrame_dut( void * _pDecWrapperObj,T_DEC_CONTXT_DUT * _psDecContxt)
{
	DEC_RETURN_DUT eDecRet;
	TRACE_PRINTF("[func, line]= [%s, %d] \r\n",__FUNCTION__, __LINE__);
	eDecRet = VideoDecFrame(_pDecWrapperObj,_psDecContxt);
	TRACE_PRINTF("[func, line]= [%s, %d]: ret: %d \r\n",__FUNCTION__, __LINE__,eDecRet);	
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

DEC_RETURN_DUT VideoDecRelease( void *pDecWrapperObj )
{
    sMpeg2DecWrapperObject* psDecWrapperObj=(sMpeg2DecWrapperObject*)pDecWrapperObj;
    sMpeg2DecObject *psMpeg2DecObject = &(psDecWrapperObj->sMpeg2DecObject);
    eMpeg2DecRetType       eDecRetVal = E_MPEG2D_FAILURE;

    ASSERT(pDecWrapperObj != NULL);
    
    printf("Bitrate=%8d ",psMpeg2DecObject->sDecParam.bitrate);

    	/*! 
    *   Closing file
    */
    eMPEG2FileClose (psDecWrapperObj->sIOFileParamsObj);

    /*! 
    *   Freeing resources used by MPEG2 Decoder
    */

    /* When does Decoder allocate its internal memory.
       Depending on whether its done at Init() stage /
       Query_Mem() state/ Decode() state, i will have
       call this function to synchronise with the
       allocation point
    */ 
    eDecRetVal = eMPEG2DFree (psMpeg2DecObject);
    releaseFrameManager(psDecWrapperObj);

    return E_DEC_REL_OK_DUT;
}

DEC_RETURN_DUT DLL_EXPORTS VideoDecRelease_dut( void * pDecWrapperObj )
{
	return VideoDecRelease(pDecWrapperObj);
}

T_DEC_RETURN_DUT DLL_EXPORTS VideoTestDecRelease_dut( void * pDecWrapperObj, T_DEC_CONTXT_DUT * psDecContxt )
{	
	sMpeg2DecWrapperObject* psDecWrapperObj=(sMpeg2DecWrapperObject*)pDecWrapperObj;
	sMpeg2DecObject *psMpeg2DecObject = &(psDecWrapperObj->sMpeg2DecObject);
	DEC_RETURN_DUT eDecRet;

	TRACE_PRINTF("[func, line]= [%s, %d] \r\n",__FUNCTION__, __LINE__);

	// compute heap size
	HEAP_INCREASE(sizeof (sMpeg2DecObject));
	{
		int s32MemBlkCnt = 0, cnt;
		sMpeg2DecMemAllocInfo *psMemAllocInfo;
		sMpeg2DecMemBlock  *psMemInfo;
		psMemAllocInfo = &(psMpeg2DecObject->sMemInfo);
		for (s32MemBlkCnt = psMemAllocInfo->s32BlkNum; s32MemBlkCnt < (psMemAllocInfo->s32NumReqs+psMemAllocInfo->s32BlkNum);s32MemBlkCnt++)
		{      
			psMemInfo = &psMemAllocInfo->asMemBlks[s32MemBlkCnt];

			if (psMemInfo->pvBuffer!= NULL)
			{     
				HEAP_INCREASE(psMemInfo->s32Size);
			}
		}
	}

	psDecContxt->uiWidth=psMpeg2DecObject->sDecParam.u16FrameWidth;
	psDecContxt->uiHeight=psMpeg2DecObject->sDecParam.u16FrameHeight;
	psDecContxt->uiBitRate=psMpeg2DecObject->sDecParam.bitrate;
	psDecContxt->uiFrameRate=0;// decoder skip the information currently, so we can not get the information
	
	eDecRet=VideoDecRelease(pDecWrapperObj);
	TRACE_PRINTF("[func, line]= [%s, %d] \r\n",__FUNCTION__, __LINE__);

	if (eDecRet!=E_DEC_REL_OK_DUT)
		return T_DEC_REL_ERROR_DUT;
	else
	    	return T_DEC_REL_OK_DUT;
}

#if ( 21 > WRAPPER_API_VERSION )
DEC_RETURN_DUT DLL_EXPORTS VideoDecSetProbes_dut(void *pDecWrapperObj, VTS_PROBES_DUT *psProbe)
{
	ASSERT((pDecWrapperObj != NULL) && (psProbe != NULL));
	memcpy(&(((sMpeg2DecWrapperObject*)pDecWrapperObj)->sVTSProbes), psProbe, sizeof(VTS_PROBES_DUT) );	
	return E_SET_PROB_OK_DUT;
	
}
#endif
T_DEC_RETURN_DUT DLL_EXPORTS VideoTestDecSetProbes_dut(void *pDecWrapperObj, PROBE_DUT_FUNC *psProbe)
{
	TRACE_PRINTF("[func, line]= [%s, %d] \r\n",__FUNCTION__, __LINE__);
	ASSERT((pDecWrapperObj != NULL));
	((sMpeg2DecWrapperObject*)pDecWrapperObj)->fnPrbDutFunc=psProbe;
	TRACE_PRINTF("[func, line]= [%s, %d] \r\n",__FUNCTION__, __LINE__);	
	return T_SET_PROB_OK_DUT;
}


#if ( 21 == WRAPPER_API_VERSION )
DEC_RETURN_DUT DLL_EXPORTS QueryAPIVersion( long * _piAPIVersion )
{
    *_piAPIVersion = WRAPPER_API_VERSION;
    
    return E_GET_VER_OK_DUT;
}
#else
DEC_RETURN_DUT QueryAPIVersion_dut( unsigned char _auchVer[] )
{
    strcpy( _auchVer, DUT_API_HEADER_VERSION );
	
    return E_GET_VER_OK_DUT;
}
#endif

