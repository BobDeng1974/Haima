/**********************************************************************
*
* Motorola Inc.
* (c) Copyright 2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
***********************************************************************
***********************************************************************
* Copyright 2005-2010 by Freescale Semiconductor, Inc.
* All modifications are confidential and proprietary information
* of Freescale Semiconductor, Inc.
***********************************************************************
*
* File Name			: DUMMYCALL.c
*
* Description		: Call to print the number of frames processed.
*
* Functions Included: dummyCall()
*
****************************** Change History**************************
*
*    DD/MM/YYYY     Code Ver     Description      Author
*    -----------     --------     -----------      ------
*	 03/11/2004		v1.0		 Initial version	Naganna/Shashi
**********************************************************************/


#include <stdio.h>

extern int noOfFrames;

void dummyCall(void)
{
	noOfFrames++;
	//printf("Frame :: %d\n", noOfFrames);
}
