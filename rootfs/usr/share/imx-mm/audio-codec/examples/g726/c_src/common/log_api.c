
/*******************************************************************************
* Motorola Inc.
* (c) Copyright 2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
********************************************************************************
 ***********************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc.
 ***********************************************************************
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
*    9/Mar/2005     0.2          Close file handle automatically        Tommy Tang
*******************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "g726_dec_api.h"
#include "g726_enc_api.h"
#include "log_api.h"
/******************************************************************************/
#ifdef	_FILEIO

#define DEBUG_FILE "debug.bin"
#define MAX_TEXT_LENGTH 1024

FILE * log_fp=NULL;

static void exitlogger()
{
    fprintf(log_fp, "-------------------End of Logging--------------------\n");
    fclose(log_fp);
    log_fp = NULL;
}

static void initlogger()
{
    if(NULL == log_fp)
    {
        log_fp=fopen(DEBUG_FILE,"w");
        if(log_fp)
        {
            /* store function, which is called by exit */
            if(atexit(exitlogger))
            {
                /* Release resource immediately if registry failed.*/
                fclose(log_fp);
                log_fp=NULL;
            }
            else
            {
                fprintf(log_fp, "-------------------Start Logging--------------------\n");
            }
        }
    }
}

int DebugLogText(short int msgid,char *fmt,...)
{
    va_list ap;
    char logString[MAX_TEXT_LENGTH];

    if (((msgid > G726E_BEGIN_DBG_MSGID) && (msgid < G726E_END_DBG_MSGID))
       ||((msgid > G726D_BEGIN_DBG_MSGID) && (msgid < G726D_END_DBG_MSGID)))
    {
        if(strlen(fmt) > MAX_TEXT_LENGTH)
        {
            return(-1);
        }

        if(NULL == log_fp)
        {
            initlogger();
        }

        va_start(ap,fmt);
        vsprintf(logString,fmt,ap);
        va_end(ap);

        strcat(logString, "\n");

        if(log_fp)
        {
            fwrite(logString,(strlen(logString)),1,log_fp);
            fflush(log_fp);
        }

        printf("%d:%s",msgid,logString);
    }
    else
    {
        return (-1);
    }
    return (1);
}

int DebugLogData(short int msgid,void *ptr,int size)
{
    if (((msgid > G726E_BEGIN_DBG_MSGID) && (msgid < G726E_END_DBG_MSGID))
            ||((msgid > G726D_BEGIN_DBG_MSGID) && (msgid < G726D_END_DBG_MSGID)))
    {
        if(NULL == log_fp)
        {
            initlogger();
        }

        if(log_fp)
        {
            fprintf(log_fp, "***Binary data (%d bytes)\n", size);
            fwrite(ptr,size,1,log_fp);
			fprintf(log_fp, "\n***End binary data\n");

            fflush(log_fp);
        }
    }
    else
    {
        return (-1);
    }
    return(1);
}
#endif /* _FILEIO */
