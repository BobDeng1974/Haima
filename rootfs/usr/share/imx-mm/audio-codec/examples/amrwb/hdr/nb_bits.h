
 /************************************************************************
  * Copyright 2005-2010 by Freescale Semiconductor, Inc.
  * All modifications are confidential and proprietary information
  * of Freescale Semiconductor, Inc.
  ************************************************************************/

#ifndef _NB_BITS_H
#define _NB_BITS_H

#include "bits_test.h"

static const WBAMR_S16 nb_of_bits[NUM_OF_MODES] = {
    NBBITS_7k,
    NBBITS_9k,
    NBBITS_12k,
    NBBITS_14k,
    NBBITS_16k,
    NBBITS_18k,
    NBBITS_20k,
    NBBITS_23k,
    NBBITS_24k,
    NBBITS_24k,
	NBBITS_SID
};

/* size of packed frame for each mode, excluding TOC byte */
static WBAMR_S16 packed_size[16] = {17, 23, 32, 36, 40, 46, 50, 58,
                                 60,  5,  0,  0,  0,  0,  0,  0};

static WBAMR_S16 if2_packed_size[16] = {18, 23, 33, 37, 41, 47, 51, 59,
                                 61,  6,  1,  1,  1,  1,  1,  1};


//In AMR WB IF1 formt, the SID frame size is corrected to 8 bytes.
//TLSbo60278
static WBAMR_S16 if1_packed_size[16] = {20, 26, 35, 39, 43, 49, 53, 61,
                                 63,  8,  1,  1,  1,  1,  1,  1};
#endif
