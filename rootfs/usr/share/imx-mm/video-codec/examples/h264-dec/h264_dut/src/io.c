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
 ***********************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc.
 ***********************************************************************
 ***********************************************************************
 *  \file
 *      io.c
 *
 *  \brief
 *    This file handles I/O routines for the AVC application including
 *     KevFile I/O and bitstream file input.
 *
 *  \author
 *      Faisal Ishtiaq          <faisal@motorola.com> <BR>
 *      Raghavan Subramaniyan   <raghavan.s@motorola.com>
 *
 *  \version
 *      $Revision: 1.9 $ - $Date: 2004/06/28 21:08:17 $
 *
 ***********************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "io.h"
#include "dirUtils.h"


static void OpenInputFile(IOParams *p)
{
	unsigned char a[512];
	int i;
    //! We are reading from a file
//#ifndef OS_VRTX
    if ((p->inFile = fopen(p->bitFileName, "rb")) == NULL)
    {
		//DSPhl28494
		fprintf(stderr, "App IO2 :Error opening bitstream file %s for reading\n",	\
                 p->bitFileName);
		exit(3);
    }
//#endif
    // printf("File Name %s\n", p->bitFileName);
    //i = fread(a, 1, 512, p->inFile);
    p->nRead = 0;
    p->index = 0;
    p->endBitstream = 0;
}

void RewindInputFile(IOParams *p)
{

	#ifdef TGT_OS_WINCE
	fseek(p->inFile,0L,SEEK_SET);
	#else
    rewind(p->inFile);

	#endif

}

/*!
 ***********************************************************************
 * \brief
 *      Reads NALU from input source.
 *      Currently supports annex-B bytestream format
 * \param *p
 *      pointer to the IOParams structure
 * \return static Int
 *      Integer value of the data read
 * \author
 *      Raghavan Subramanian        <rags@labs.mot.com>
 * \date
 *      4/2/2003
 ***********************************************************************
 */
static Int GetByte(IOParams *p)
{
    Int     val;

    if (p->index >= p->nRead)
    {
        p->index = 0;

//#ifndef OS_VRTX
	if (p->inFile == NULL)
		PRINT_ERROR("Error in file pointer\n");

        p->nRead = fread(&(p->inputBuffer[0]), sizeof (unsigned char), INPUT_BUF_LEN, (FILE*)p->inFile);
//#else
//#endif

        if (p->nRead == 0)
        {
            p->endBitstream = 1;
            return -1;
        }
    }
    val = p->inputBuffer[p->index];
    p->index++;

    return val;
}

/*!
 ***********************************************************************
 * \brief
 *      Function retrieves a NAL unit from the AnnexB defined bitstream
 *      format, parses it to extract a NAL unit, removes the stuffing
 *      bytes, and drops it into the buffer from which the decoder reads.
 * \param p
 *      Pointer to IOParams
 * \param *buf
 *      Pointer to char
 * \param bufLength
 *      Length of the buffer into which the data is being put into
 * \return long
 *      Length of the NAL unit in bits w/o stuffing bits.
 * \author
 *      Raghavan Subramanian        <rags@labs.mot.com>
 * \date
 *      4/2/2003
 ***********************************************************************
 */
#define START_CODE              0x01
#define NUM_STUFF_BYTES         3
#define START_CODE_MASK         ((1 << (NUM_STUFF_BYTES * 8)) - 1)
#define STUFF_BYTE              0x03
long IO_GetNalUnitAnnexB(IOParams *p, UCHAR * buf, long bufLength)
{
    long    nBytes = 0, nTmp = 0;
    Int     x;
    unsigned long lastWord = 0xffffffff;

    while ((x = GetByte(p)) == 0)
        ;                                       // Skip leading zero-bytes

    if (p->endBitstream)
        return 0;

    if (x != START_CODE)
    {
        PRINT_ERROR("Error: Startcode not found in NALU\n");
    }

    while (1)
    {
        x = GetByte(p);

        if (p->endBitstream)
        {
            int     i;

            // Write out last 3 bytes
            for (i = 3 - nTmp; i < 3; i++)
            {
                buf[nBytes++] = (UCHAR) ((lastWord >> (16 - 8 * i)) &
                                         0xff);
            }
            break;
        }

        if (nBytes > bufLength)
        {
            PRINT_ERROR("End of bitstream %d %d\n", nBytes, bufLength);
            PRINT_ERROR("Error: NALU data overflows buffer[%ld]\n", bufLength);
            break;
        }

        lastWord = ((lastWord & 0x00ffffff) << 8) | (x & 0xff);

        // Check if the last 3 bytes are either startcode or 0s
        if ((lastWord & 0x00fffffe) == 0)
        {
            buf[nBytes++] = (UCHAR) ((lastWord >> 24) & 0xff);
            p->index--;
            break;
        }

        if ((nTmp >= 3))
        {
            buf[nBytes++] = (UCHAR) ((lastWord >> 24) & 0xff);
        }
        if (nTmp < 3)
        {
            // Wait until 4 bytes have been read in
            nTmp++;
        }

    }

	//DSPhl28494
    //printf("Read %ld bytes from NALU\n", nBytes);

    return nBytes;
}

void IO_Init(IOParams *p)
{
    OpenInputFile(p);
}

void IO_Close(IOParams *p)
{
    fclose(p->inFile);
}

