
/*
 ***********************************************************************
 *
 *                         Motorola Labs
 *         Multimedia Communications Research Lab (MCRL)
 *
 *              H.264/MPEG-4 Part 10 AVC Video Codec
 *
 *          This code is the property of MCRL, Motorola.
 *  (C) Copyright 2002-03, MCRL Motorola Labs. All Rrights Reserved.
 *
 *       M O T O R O L A  I N T E R N A L   U S E   O N L Y
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

#ifndef _IO_H
#define _IO_H


#define MAX_STRLEN              200
#define INPUT_BUF_LEN           512

#define PRINT_ERROR printf

//! Input/Output parameters used by the encoder. An
//! alternate structure can be defined based upon the specific I/O

typedef struct
{
    //! Input Variables
//#ifndef OS_VRTX
    FILE   *inFile;                     //!< Bitstream file pointer
//#endif
    char    bitFileName[MAX_STRLEN];
    char    inputBuffer[INPUT_BUF_LEN]; //!< To buffer input data
    long    nRead;                      //!< Number of bytes read
    long    index;                      //!< Current read index
    int     endBitstream;               //!< Flag

    int     ffFlag;
    // [zhenyong, 2006/12/13] support frame numbers and display options
    int     maxnum;
    int     display;
    int     hwdblock;//hw deblock
    int     tst; // for test
}
IOParams;

//! Function prototpyes in io.c
void    IO_Init(IOParams *p);
void    IO_SetOutputDimensions(IOParams *p, int width, int height);
long    IO_GetNalUnitAnnexB(IOParams *p, unsigned char *buf,
                            long bufLength);
void    IO_Close(IOParams *p);

#endif
