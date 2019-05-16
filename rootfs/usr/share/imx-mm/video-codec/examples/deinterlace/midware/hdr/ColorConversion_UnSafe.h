/*
 ***********************************************************************
 * Copyright 2005-2008 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*!
 ***********************************************************************
 * Header file of color space conversion
 *   Currently YUV (4:2:0, 4:2:2, 4:4:4) to rgb32 is implemented. This 
 *   file contains some unsafe code, please don't release it.
 * Support conversion:
 *   1. YUV420
 *   2. YUV422
 *   3. YUV444
 *   4. Only Y or U or V channel
 *
 *
 * History
 *   Date          Changed                                Changed by
 *   Nov. 14, 2007 Create and port from RawMovieFile.cpp  Zhenyong Chen
 ***********************************************************************
 */

#ifndef __COLORCONVERSION_UNSAFE_H__
#define __COLORCONVERSION_UNSAFE_H__

/* \Function
 *   InitYUV2RGBConversion
 * \Brief
 *   Init color space conversion. Must be called before conversion started
 * \Return value
 *   None
 * \Parameters
 *   None
 * \See also
 *   N/A
 */
void InitYUV2RGBConversion(void);

/* \Function
 *   ConvertYUVRGB32
 * \Brief
 *   Convert YUV buffers to RGB32 format
 * \Return value
 *   None
 * \Parameters
 *   rgb        [out] Destination buffer, in RGB32 format
 *   stride_rgb [in]  Stride of "rgb"
 *   y          [in]  Y buffer
 *   u          [in]  U buffer
 *   v          [in]  V buffer
 *   stride_y   [in]  Stride of "y"
 *   stride_uv  [in]  Stride of "u" and "v".
 *   chrom_fmt  [in]  Chroma format. Must be one of following value:
 *                        0 - 4:2:0
 *                        1 - 4:2:2
 *                        2 - 4:4:4
 *   width      [in]  Width of picture. In pixel.
 *   height     [in]  Height of picture. In pixel.
 *   mask_clrchannel
 *              [in]  Indicate which channel(s) of Y, U, and V is(are) 
 *                    required.
 *                    bit 0  - V channel is valid if set.
 *                        1  - U channel is valid if set.
 *                        2  - Y channel is valid if set.
 * \See also
 *   N/A
 */
void ConvertYUVToRGB32(
    unsigned char *rgb,
    int stride_rgb,
    const unsigned char *y,
    const unsigned char *u,
    const unsigned char *v,
    int stride_y,
    int stride_uv,
    int chrom_fmt,
    int width,
    int height,
    unsigned int mask_clrchannel);

#endif /* __COLORCONVERSION_UNSAFE_H__ */

