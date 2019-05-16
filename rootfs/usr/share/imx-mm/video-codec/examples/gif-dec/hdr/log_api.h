///////////////////////////////////////////////////////////////////////////
//
//              Motorola India Electornics Limited
//              Copyright 1999 Motorola Inc.
//
//  File Description:   Logging API's defined
//
//  Author(s):          (1) Vijay PY (vijaypy@motorola.com)
//
//  Version History:    29/Sep/2004 - Vijay PY Created
//
//
///////////////////////////////////////////////////////////////////////////

 /************************************************************************
  * Copyright 2005-2010 by Freescale Semiconductor, Inc.
  * All modifications are confidential and proprietary information
  * of Freescale Semiconductor, Inc.
  ************************************************************************/

#ifndef LOG_API_H
#define LOG_API_H
#include <stdarg.h>


// message id classifications
//codec1: range = 0-500 with its level0,level1,... rage of 0-99,100-199,...so on.
//codec2: range = 500-1000 with its level0,level1,... rage of 500-599,600-699,...so on.
// codec3.....
//codec4... so on.



//APIS for logging.
// All the codec libraries will use the below function name symbols to
// implement debug logs. These function symbols will be implemented
// by the logger library or task.

int DebugLogData(short int msgid,void *ptr,int size);
int DebugLogText(short int msgid,char *fmt,...);

#ifdef VC_PLUS_PLUS

int DebugLogTextVC(int id1, int id2,char *fmt,...);

#endif

#endif

/**********************End of File*************************/
