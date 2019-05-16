/*
 ***********************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */


 /*
****************************************************************************  
 * Freescale ShangHai Video Codec Team Change History

  Version    Date               Author		     CRs          Comments
  01         19/Nov/2008        Chen Zhenyong                 Support unified GUI
****************************************************************************  
*/


#ifndef _UI_H_
#define _UI_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_STRLEN              200
#define INPUT_BUF_LEN           512


typedef struct
{
	char 	infile[MAX_STRLEN];
	char 	outfile[MAX_STRLEN];
	char 	dutlib[MAX_STRLEN];
	char 	resfile[MAX_STRLEN];
	char 	logfile[MAX_STRLEN];

	int 	wp_argc;
	char   *wp_argv[64];
	char 	wp_options[3*MAX_STRLEN];
	
    int     saveYUV;
    int     maxnum;
    int     display;

	int     putLog;
	int 	saveResult;
	int 	memFlag;
	int 	libVer;

	int     tst;
}
IOParams;


int GetUserInput(IOParams *pIO, int argc, const char *argv[]);

int yuv_frame_compare( IOParams *pIO,
                       const unsigned char* pLum,
                       const unsigned char* pCb,
                       const unsigned char* pCr,
                       unsigned int uiWidth,
                       unsigned int uiHeight,
                       unsigned int uiStride,
                       unsigned int uiUVStride,
                       const int rauiCropping[] );


#ifdef __cplusplus
}
#endif

#endif /* _UI_H_ */

