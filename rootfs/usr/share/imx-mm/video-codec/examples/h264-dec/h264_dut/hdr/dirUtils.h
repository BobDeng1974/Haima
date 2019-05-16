
/*
 ***********************************************************************
 *
 *                         Motorola Labs
 *         Multimedia Communications Research Lab (MCRL)
 *
 *              H.264/MPEG-4 Part 10 AVC Video Codec
 *
 *          This code is the property of MCRL, Motorola.
 *  (C) Copyright 2002-04, MCRL Motorola Labs. All Rrights Reserved.
 *
 *  M O T O R O L A   C O N F I D E N T I A L   P R O P R I E T A R Y
 *
 ***********************************************************************
 */
/*
 ***********************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc.
 ***********************************************************************
 */

 /*
****************************************************************************
 * Freescale ShangHai Video Codec Team Change History

  Version    Date               Author		     CRs          Comments
  01         19/Nov/2008        Chen Zhenyong                 Remove unused macros (some depend on un-disclosed APIs)
****************************************************************************
*/

#ifndef _UTILS_H
#define _UTILS_H

// Data Types Required by Application.	//DSPhl28494
typedef unsigned char UCHAR;
typedef signed char SCHAR;
typedef unsigned long int U32;
typedef signed long int S32;
typedef unsigned short int U16;
typedef signed short int S16;

// size of "int" is platform specific - it could be 2 bytes or 4 bytes
// Use  the typedef Int only when the size does not matter.
typedef int Int;

#endif /* _UTILS_H */

