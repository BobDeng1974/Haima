////////////////////////////////////////////////////////////////////////////////
//
// MPEG-4/2 Visual Decoder developed by IPRL Chicago
// Copyright 1999 Motorola Inc.
//
// File Description:   Definitions for DCT function
//
// Author(s):
//
// Version History:
//                  Jun/24/2004-Vijay : Review comments incorporated
//
////////////////////////////////////////////////////////////////////////////////
 /************************************************************************
  * Copyright 2005-2010 by Freescale Semiconductor, Inc.
  * All modifications are confidential and proprietary information
  * of Freescale Semiconductor, Inc.
  ************************************************************************/

#ifndef MPEG4TYPES_H
#define MPEG4TYPES_H

typedef unsigned char UCHAR;
typedef signed char SCHAR;
typedef unsigned long int U32;
typedef signed long int S32;

#if defined(_WINDOWS)
typedef int U16;
typedef int S16;

#else
typedef unsigned short U16;
typedef signed short S16;

#endif

typedef int Int;

// Data type for some look-up tables that need only 8 bits
// Some platforms give better performance using 16 bits

typedef unsigned char UCHAR_T;
typedef signed char SCHAR_T;


typedef enum
{
    Y, CR, CB
}
plane_t;

#endif
