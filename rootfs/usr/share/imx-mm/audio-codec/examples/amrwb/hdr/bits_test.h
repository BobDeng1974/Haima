
 /************************************************************************
  * Copyright 2005-2010 by Freescale Semiconductor, Inc.
  * All modifications are confidential and proprietary information
  * of Freescale Semiconductor, Inc.
  ************************************************************************/

#ifndef _BITS_TEST_H
#define _BITS_TEST_H

#include "wbamr_common_interface.h"

#define MRDTX         10
#define NUM_OF_MODES  11        /* see bits.h for bits definition             */


#define NBBITS_7k     132                  /* 6.60k  */
#define NBBITS_9k     177                  /* 8.85k  */
#define NBBITS_12k    253                  /* 12.65k */
#define NBBITS_14k    285                  /* 14.25k */
#define NBBITS_16k    317                  /* 15.85k */
#define NBBITS_18k    365                  /* 18.25k */
#define NBBITS_20k    397                  /* 19.85k */
#define NBBITS_23k    461                  /* 23.05k */
#define NBBITS_24k    477                  /* 23.85k */

#define NBBITS_SID    35				   /* sid frame */
#define NB_BITS_MAX   NBBITS_24k


#define WBAMR_SIZE_MAX  		(3+NB_BITS_MAX)          /* serial size max */
#define TX_FRAME_TYPE 	(WBAMR_S16)0x6b21
#define RX_FRAME_TYPE 	(WBAMR_S16)0x6b20

#define TX_SPEECH 0
#define TX_SID_FIRST 1
#define TX_SID_UPDATE 2
#define TX_NO_DATA 3

#define RX_SPEECH_GOOD 0
#define RX_SPEECH_PROBABLY_DEGRADED 1
#define RX_SPEECH_LOST 2
#define RX_SPEECH_BAD 3
#define RX_SID_FIRST 4
#define RX_SID_UPDATE 5
#define RX_SID_BAD 6
#define RX_NO_DATA 7

#define MRSID 9

#define CRC_POLYNOMIAL  0x171     /* 101110001: CRC Poly = D8 + D6 + D5 + D4 + 1 */
#define EOR_CRC_VAL     0x71      /* 01110001 : Value to be XORed;excludes D8    */

#endif

