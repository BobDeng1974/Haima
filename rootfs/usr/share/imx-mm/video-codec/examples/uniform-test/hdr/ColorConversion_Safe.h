/*
 ***********************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*!
 ***********************************************************************
 * Implementation file of color space conversion
 *   Currently YUV (4:2:0, 4:2:2, 4:4:4) to rgb32 is implemented. 
 &
 * Support conversion:
 *   1. YUV420
 *   2. YUV422
 *   3. YUV444
 *   4. Only Y or U or V channel
 *
 *
 * History
 *   Date          Changed                                Changed by
 *   Jun. 16, 2008 Create                                 Zhenyong Chen
 ***********************************************************************
 */


#ifndef __COLORCONVERSION_SAFE_H__
#define __COLORCONVERSION_SAFE_H__

/* \Function
 *   YUVToRGB32
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
void YUVToRGB32(
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

/* \Function
 *   RGB32ToYUV
 * \Brief
 *   Convert RGB32 buffers to YCbCr format
 * \Return value
 *   None
 * \Parameters
 *   rgb        [in]  Source buffer, in RGB32 format
 *   stride_rgb [in]  Stride of "rgb"
 *   y          [out] Y buffer
 *   u          [out] U buffer
 *   v          [out] V buffer
 *   stride_y   [in]  Stride of "y"
 *   stride_uv  [in]  Stride of "u" and "v".
 *   chrom_fmt  [in]  Chroma format. Must be one of following value:
 *                        0 - 4:2:0
 *                        1 - 4:2:2
 *                        2 - 4:4:4
 *   width      [in]  Width of picture. In pixel.
 *   height     [in]  Height of picture. In pixel.
 *
 * \See also
 *   N/A
 */
void RGB32ToYUV(
    const int *rgb,
    int stride_rgb,
    int width,
    int height,
    unsigned char *y,
    unsigned char *u,
    unsigned char *v,
    int stride_y,
    int stride_uv,
    int chrom_fmt);

#endif /* __COLORCONVERSION_SAFE_H__ */

