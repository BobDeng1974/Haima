/*******************************************************************************
* Motorola Inc.
* (c) Copyright 2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
***********************************************************************
* Copyright 2005-2010 by Freescale Semiconductor, Inc.
* All modifications are confidential and proprietary information
* of Freescale Semiconductor, Inc.
***********************************************************************
********************************************************************************
* File Name: log_api.c
*
* Description: Sample file for testing debug log implementation
*
* Functions Included: - DebugLogText
*                     - DebugLogData
****************************** Change History***********************************
*   DD/MMM/YYYY     Code Ver     Description                            Author
*   -----------     --------     -----------                            ------
*   15/Jan/2005     0.1          File created                           Ashok
*   15/Jul/2008     0.2          Remove warnings                        Tao Jun
*
*******************************************************************************/
#ifndef OS_VRTX
#include <stdio.h>
#include <stdarg.h>
#include  <string.h>
#endif
#include "nb_amr_common_api.h"
#include "nb_amr_dec_api.h"
#include "nb_amr_enc_api.h"
#include "log_api.h"
/******************************************************************************/

#define DEBUG_FILE "debug.bin"
#define MAX_TEXT_LENGTH 256

#ifndef OS_VRTX
FILE * log_fp=NULL;

/* static int initlogger()
 * {
 *    if(log_fp==NULL)
 *        log_fp=fopen(DEBUG_FILE,"w");
 *    if(log_fp==NULL)
 *        return(-1);
 *    return(0);
 * }
 */
int DebugLogText(short int msgid,char *fmt,...)
{
    va_list ap;
    char logString[MAX_TEXT_LENGTH];

    if (((msgid > NBAMRE_BEGIN_DBGLOG_MSGID) && (msgid > NBAMRE_END_DBGLOG_MSGID))
       ||((msgid > NBAMRD_BEGIN_DBGLOG_MSGID) && (msgid > NBAMRD_END_DBGLOG_MSGID)))
    {
        if(strlen(fmt) > MAX_TEXT_LENGTH)
        {
            return(-1);
        }
        va_start(ap,fmt);
        vsprintf(logString,fmt,ap);
        va_end(ap);
        fwrite(logString,(strlen(logString)+1),1,log_fp);
        fflush(log_fp);
        printf("%d:%s\n",msgid,logString);
    }
    else
    {
        return (-1);
    }
    return (1);
}

int DebugLogData(short int msgid,void *ptr,int size)
{
    if (((msgid > NBAMRE_BEGIN_DBGLOG_MSGID) && (msgid > NBAMRE_END_DBGLOG_MSGID))
            ||((msgid > NBAMRD_BEGIN_DBGLOG_MSGID) && (msgid > NBAMRD_END_DBGLOG_MSGID)))
    {
        fwrite(ptr,size,1,log_fp);

        fflush(log_fp);
    }
    else
    {
        return (-1);
    }
    return(1);
}
/*
 * static int exitlogger()
 * {
 *     fclose(log_fp);
 *     return(0);
 * }
 */
#endif

