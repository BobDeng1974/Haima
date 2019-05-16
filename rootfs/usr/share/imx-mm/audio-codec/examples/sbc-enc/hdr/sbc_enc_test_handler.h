
/*
 ***********************************************************************
 * Copyright 2005-2008 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */
/*==================================================================================================

  File Name:  sbc_enc_test_handler.h

  General Description: Test handler header for sbc encoder

  ====================================================================================================
  Revsion History:
  Modification     Tracking
  Author                          Date          Number     Description of Changes
  -------------------------   ------------    ----------   -------------------------------------------
                                                               Initial Creation
  Tao  Jun                    09/04/2008      engr71841    1. Add file header
                                                           2. Change CIRC_BUF_SIZE

  ====================================================================================================*/

#ifndef SBC_ENC_TEST_HANDLER_H
#define SBC_ENC_TEST_HANDLER_H 
#include "stdio.h"
#include "sbc_api.h"

#undef NO_MIPS_TEST

#ifndef TIME_PROFILE
#ifndef ARM11_MIPS_TEST_LERVDS	
#ifndef ARM9_MIPS_TEST_LERVDS
#ifndef ARM_MIPS_TEST_WINCE

#define NO_MIPS_TEST

#endif
#endif
#endif
#endif

#ifdef NO_MIPS_TEST
#define CIRC_BUF_SIZE		    3072
#else
#define CIRC_BUF_SIZE		    578560	/* 565K, larget test bitstream size is 563K */
#endif

#ifdef RE_ENTRANCY

#define MAX_INST 10
typedef int (*TestAppMainfunc)(int argc, char *argv[]);
int RunMultipleInstances(TestAppMainfunc CompMainFunc, int argc, char **argv);
typedef struct
{
	int nArgCnt;
	char **sArgList;
	void *AppThreadId;
}TestAppMainFuncParam;

#endif

typedef struct tInputBitStream {
	char*	filename;
	FILE*	infp;
}InputBitStream;

typedef struct tOutputFile {
	char*	filename;
	FILE*	outfp;
}OutputFile;



typedef struct tSbcEncApp{

	uint8	last_frame;
	uint8	end_of_buffer;
	uint32	frame_no;

	uint8	*out_buf;
	uint8	*in_buf;
	uint8	*circular_buffer;

	uint8 *read_ptr;
        uint8 *write_ptr;
        uint8 *circ_buf_start;
        uint8 *circ_buf_end;

	InputBitStream          inputBS;
	OutputFile              outFile;

	struct sbc_enc_input_struct sbc_enc_input;
	struct sbc_output_struct sbc_enc_output;

	struct sbc_data_struct		sbc_enc_data;

	SbcEncConfig	sbc_enc_config;
}SbcEncApp;



#endif


