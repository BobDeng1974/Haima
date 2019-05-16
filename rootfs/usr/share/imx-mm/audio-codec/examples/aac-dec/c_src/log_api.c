/*
 ***********************************************************************
 * Copyright 2005-2008 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */
 /*******************************************************************************
 *
 *   CHANGE HISTORY
 *   dd/mm/yy   Code Ver     Description                        Author
 *   02/02/2007       02     Level 4 Warnings removal           rks05c
 *                           Defect ID: TLSbo89610
 ******************************************************************************/

#include <stdio.h>
#include  <string.h>
#include "log_api.h"
// structure for log record. Currently uses p2k log struct
// so that we can use the existing p2k parsers.

//tlsbo89610
#if defined(WINCE_WARNLVL4_DISABLE) && defined(__WINCE)
	// disable some level-4 warnings, use #pragma warning(enable:###) to re-enable
	#pragma warning(disable:4100) // warning C4100: unreferenced formal parameter
#endif //#if defined(WINCE_WARNLVL4_DISABLE) && defined(__WINCE)

typedef struct
{
    unsigned short int sync;
    unsigned short int size;
    unsigned int timestamp;
    unsigned int msgId;
} LOG_HEADER;
typedef struct
{
    LOG_HEADER hdr;
    void *data;
} LOG_RECORD;

#define DEBUG_FILE "debug.bin"
#define MAX_TEXT_LENGTH 400
#define END_MSGIDS 500
FILE * log_fp=NULL;

static int initlogger()
{
  if(log_fp==NULL)
    log_fp=fopen(DEBUG_FILE,"w");
  if(log_fp==NULL)
    return(-1);
  return(0);  
}

#ifndef VC_PLUS_PLUS
int DebugLogText(short int msgid,char *fmt,...)
#else
int DebugLogText(int dummy1,short int msgid,char *fmt,...)
#endif

{
  LOG_HEADER hdr;
  va_list ap;
  char logString[MAX_TEXT_LENGTH];
  
  if(initlogger()==-1)
    return(-1);

  if (msgid > END_MSGIDS)
    return(-1);
	
  if(strlen(fmt) > MAX_TEXT_LENGTH)
    { return(-1); }

  va_start(ap,fmt);
  vsprintf(logString,fmt,ap);
  va_end(ap);
  hdr.sync=0xB5C7;
  hdr.timestamp=0;
  hdr.size=(unsigned short)(sizeof(hdr)+strlen(logString)+1);          //tlsbo89610
  hdr.msgId=msgid;
  fwrite(&hdr,1,sizeof(hdr),log_fp);
  fprintf(log_fp, "\n");
  fwrite(logString,(strlen(logString)+1),1,log_fp);
  fprintf(log_fp, "\n");
  fflush(log_fp);    
  //printf("%5d:%s\n",msgid,logString);
  return(1);
}

#ifndef VC_PLUS_PLUS
int DebugLogData(short int msgid,void *ptr,int size)
#else
int DebugLogData(int id1,short int msgid,void *ptr,int size)
#endif
{
  int i;
  int *pptr;

  pptr = (int *)ptr;
  if(initlogger()==-1)
    return(-1);

  if (msgid > END_MSGIDS)
    return(-1);
	

  //fwrite(ptr,size,1,log_fp);
  for (i=0; i<size; i++)
	  fprintf(log_fp, "%04x\n", *pptr++); 

  fflush(log_fp);      
  return(1);
}

int dummytext(short int msgid,char *fmt,...)
{ return(1);}
int dummydata(short int msgid,void *ptr,int size)
{ return(1); }
