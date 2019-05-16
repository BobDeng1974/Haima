

/****************************************************************************
 *
 * (C) 2004 MOTOROLA INDIA ELECTRONICS LTD.
 *
 *   CHANGE HISTORY
 *   dd/mm/yy   Code Ver    Description                         Author
 *   --------   -------     -----------                         ------
 *   16/12/04   01          Created.                            B.Venkatarao
 *
 ****************************************************************************/
 	 /************************************************************************
	  * Copyright 2005-2010 by Freescale Semiconductor, Inc.
	  * All modifications are confidential and proprietary information
	  * of Freescale Semiconductor, Inc. 
	  ************************************************************************/
#include <stdio.h>
#include "log_api.h"


/***************************************************************************
 *
 *   FUNCTION NAME - DebugLogData
 *
 *   DESCRIPTION
 *      This function logs data
 *
 *   ARGUMENTS
 *      msgid   -   Log message id
 *      ptr     -   Pointer to the data
 *      size    -   Size in bytes
 *
 *   RETURN VALUE
 *      0
 *
 ***************************************************************************/
int DebugLogData(short int msgid,void *ptr,int size)
{
    int i;
    unsigned char *p = (unsigned char *) ptr;

    fprintf (stdout, "%d ", msgid);
    for (i = size; i > 0; i--)
        fprintf (stdout, "%02x", *p++);
    fprintf (stdout, "\n");
    return 0;
}

/***************************************************************************
 *
 *   FUNCTION NAME - DebugLogText
 *
 *   DESCRIPTION
 *      This function logs text.
 *
 *   ARGUMENTS
 *      msgid   -   Log message id
 *      fmt     -   Format of the text
 *      ...     -   Variable number of arguments
 *
 *   RETURN VALUE
 *      0
 *
 ***************************************************************************/
int DebugLogText(short int msgid,char *fmt,...)
{
    va_list arg_list;

    va_start (arg_list, fmt);
    fprintf (stdout, "%d ", msgid);
    vfprintf (stdout, fmt, arg_list);
    fprintf (stdout, "\n");
    va_end (arg_list);

    return 0;
}
