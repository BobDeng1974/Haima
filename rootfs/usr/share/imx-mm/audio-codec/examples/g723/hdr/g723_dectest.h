
/*****************************************************************************
 *
 * Motorola Inc.
 * (c) Copyright 2004 Motorola, Inc.
 * ALL RIGHTS RESERVED.
 *
 *****************************************************************************
 ************************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc.
 ************************************************************************
 * File Name: g723_dectest.h
 *
 * Description: This is a header file for g723_dectest.c
 *
 ****************************** Change History********************************
 *
 *   DD/MMM/YYYY     Code Ver     Description                   Author
 *   -----------     --------     -----------                   ------
 *   18/Oct/2004     0.1          File created                  Tommy Tang
 *    9/May/2005     0.2          Add table definition          Tommy Tang
 *****************************************************************************/
#ifndef G723_DECTEST_H
#define G723_DECTEST_H
/******************************************************************************/


#ifndef OS_VRTX
#define alloc_fast(A)        malloc(A)
#define alloc_slow(A)        malloc(A)
#define mem_free(A)          free(A)
#else
#define mem_free(A)
#endif

#ifdef G723_DEC_TABRELOC_TST
extern const G723_S32 gas32L_bseg[];
extern const G723_S32 gas32CombinatorialTable[];

#if ((defined(G723_ARM9E_VERSION) || defined(G723_ARM11_VERSION)) && !defined(GCC_ALIGN))
extern __align(4) const G723_S16 gas16AcbkGainTable085[];
#elif defined(GCC_ALIGN)
extern const G723_S16 gas16AcbkGainTable085[] __attribute__ ((aligned(4)));
#else
extern const G723_S16 gas16AcbkGainTable085[];
#endif

extern const G723_S16 gas16AcbkGainTable170[];
extern const G723_S16 gas16epsi170[];
extern const G723_S16 gas16gain170[];
extern const G723_S16 gas16LspDcTable[];
extern const G723_S16 gas16BandInfoTable[];
extern const G723_S16 gas16BandTb8[];
extern const G723_S16 gas16CosineTable[];
extern const G723_S16 gas16fact[];
extern const G723_S16 gas16base[];
extern const G723_S16 gas16Nb_puls[];
extern const G723_S16 gas16FcbkGainTable[];

extern const G723_S16 gas16PostFiltZeroTable[];
extern const G723_S16 gas16PostFiltPoleTable[];
extern const G723_S32 gas32MaxPosTable[];
extern const G723_S16 gas16LpfConstTable[];

#endif
/************************************ END OF FILE *****************************/
#endif
