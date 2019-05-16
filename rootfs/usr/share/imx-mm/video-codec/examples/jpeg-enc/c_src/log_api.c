

/**********************************************************************
 * 
 * (C) 2004 MOTOROLA INDIA ELECTRONICS LTD.
 * 
 * CHANGE HISTORY
 *
 * dd/mm/yy   Description                         Author
 * --------  ------------                         ------
 * 15/12/04   Created.                            Ganesh Kumar C
 *
 *********************************************************************/
 /*
 ***********************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc.
 ***********************************************************************
 */
#include <stdio.h>
#include  <string.h>
#include <stdarg.h>

#define DEBUG_FILE "debug.bin"
#define MAX_TEXT_LENGTH 256
FILE * log_fp=NULL;

int initlogger()
{
  if(log_fp==NULL)
    log_fp=fopen(DEBUG_FILE,"w");
  if(log_fp==NULL)
    return(-1);
  return(0);  
}

int DebugLogText(short int msgid,char *fmt,... )
{
  va_list ap;
  char logString[MAX_TEXT_LENGTH];
 
  va_start(ap,fmt);
  vsprintf(logString,fmt,ap);
  va_end(ap);
  fprintf(log_fp,"\n%5d  :  %s",msgid,logString);

  return 1;
}

/* This is a custom implementaition of Debug Log Data
 * for JPEG encoder. Here the pointer passed in interpreted
 * as a U16 pointer */
int DebugLogData(short int msgid,void *ptr,int size)
{
    int i,j;
    /* Assuimg that unsigned short takes 16 bits */
    unsigned short *ptr1 = (unsigned short *) ptr;
    
    fprintf (log_fp, "\n%5d  :  ",msgid);
   
    j = 0;
    
    /* The size passed is in the number of bytes */
    for (i = size/2; i > 0; i--)
    {
       fprintf (log_fp, "%04x  ", *ptr1++);
       j++;
        if(j == 8)
        {
            fprintf(log_fp,"\n          ");
            j = 0;
        }
    }
  
    return 1;
}

int exitlogger()
{
  if(log_fp==NULL)
  {
    return(-1);
  }
  else
  {
      fclose(log_fp);
  }
  return(1);  
}


