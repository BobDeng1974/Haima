
 /************************************************************************
  * Copyright 2005-2010 by Freescale Semiconductor, Inc.
  * All modifications are confidential and proprietary information
  * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
  ************************************************************************/

#ifndef MPEG2DECTESTAPP_H
#define MPEG2DECTESTAPP_H

/*! \file
*
* Description: Header file for the Test Application. It contains
*              structures, #defines, dependent files and function declarations
*              required by the Test Application.
* Functions Included:
* 1. int cbkMPEG2DBufRead (int s32EncStrBufLen, unsigned char *pu8EncStrBuf,
*                          int s32Offset, void *pvAppContext);
* 2. eMpeg2DecRetType eMPEG2Decode(sMpeg2DecObject *psMp2Obj,unsigned int *s32decodedBytes, void *pvAppContext);
*
* 3. int s32AllocateMem2Decoder (sMemAllocInfo *psMemAllocInfo);
*
*    DD/MMM/YYYY   Code Ver      Description                 Author
*    -----------   --------      -----------              ------------
*    12/Jun/2006     01           Created       MANOJ ARVIND & DURGAPRASAD S. BILAGI
*
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpeg2_dec_api.h"

#define ERROR               -1
#define SUCCESS              0

/*!
*******************************************************************************
*   Structures
*******************************************************************************
*/

typedef struct
{

    int      s32NumBytesRead;
    char     *ps8EncStrFname;
    char     *ps8DecStrFname;

    FILE     *fpInput;
    FILE     *fpOutY;
    FILE     *fpOutCb;
    FILE     *fpOutCr;

} sIOFileParams;


/*!
*******************************************************************************
*   Function Declarations
*******************************************************************************
*/

int cbkMPEG2DBufRead (int *s32EncStrBufLen, unsigned char **pu8EncStrBuf,int s32Offset, void *pvAppContext);
int s32AllocateMem2Decoder (sMpeg2DecMemAllocInfo *psMemAllocInfo);
void vInitailizeMpeg2DecObject(sMpeg2DecObject* psMpeg2DecObject);
void eMPEG2KevClose (sIOFileParams file_info);
int eMPEG2KevWrite(sIOFileParams file_info,sMpeg2DecObject *pMp2DObj);
eMpeg2DecRetType eMPEG2DFree(sMpeg2DecObject *psMp2Obj);

#endif

