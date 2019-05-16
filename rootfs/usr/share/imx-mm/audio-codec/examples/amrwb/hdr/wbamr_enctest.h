
/**********************************************************************
*
* Motorola Inc.
* (c) Copyright 2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
***********************************************************************
************************************************************************
* Copyright 2005-2010 by Freescale Semiconductor, Inc.
* All modifications are confidential and proprietary information
* of Freescale Semiconductor, Inc.
  **********************************************************************
* File Name			: WBAMR_ENC_TEST.H
*
* Description		: WBAMR Encoder interface for Testing.
*
* Functions Included: --NONE--
*
****************************** Change History**************************
*
*    DD/MM/YYYY     Code Ver     Description      Author
*    -----------     --------     -----------      ------
*	 27/01/2005		1.0			Initial version		Shashi/Naganna
**********************************************************************/


#ifndef _WBAMR_ENC_TEST_H_
#define	_WBAMR_ENC_TEST_H_

#ifdef	WBAMR_ENCODER_CONT_TST
#define	WBAMR_EXTRASTATE	500*4
#define	WBAMR_EXTRASCRATCH	500*4
#else
#define	WBAMR_EXTRASTATE	0
#define	WBAMR_EXTRASCRATCH	0
#endif

#ifdef WBAMR_ENCODER_TABLE_RELOCATION_TST
#define ENCODER_TABLE_SIZE	(1054)
#define COMMON_TABLE_SIZE	(11566 + 44)
#endif

#ifdef CYCLE_MEASUREMENT
int mystrcmp(const char* pcStr1, const char* pcStr2);
#endif

#endif
