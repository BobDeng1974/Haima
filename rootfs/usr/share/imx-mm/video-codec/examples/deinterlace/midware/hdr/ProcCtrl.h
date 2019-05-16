/*
 ***********************************************************************
 * Copyright 2005-2008 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*!
 ***********************************************************************
 * Header file of deinterlacing system process management
 * The entire process consists of:
 *     1. Raw yuv data loading (through class CRawMovie)
 *     2. Picture buffer management (through class CPictureManagement)
 *     3. Deinterlace (through API Deinterlace)
 *     4. Saving output video
 *     5. Color space conversion
 *     6. Display (on off-screen)
 *
 * History
 *   Date          Changed                                Changed by
 *   Sep. 5, 2007  Create                                 Zhenyong Chen
 *   Sep. 5, 2007  Create                                 Zhenyong Chen
 *   Sep. 14, 2007 Support direct interfering of de-      Zhenyong Chen
 *                 interlacing process
 *   Oct. 18, 2007 Modify API to support WinCE            Zhenyong Chen
 *   Jan. 22, 2008 Add support for double frames output   Zhenyong Chen
 ***********************************************************************
 */


#ifndef __PROCCTRL_H__
#define __PROCCTRL_H__

#include "RawMovieFile.h"
#include "PictureManagement.h"
#include "Deinterlace.h"

/* \Name
 *   YUVTORGB
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
 *   ConvertYUVToRGB32
 */
typedef 
void (* YUVTORGB)(
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

typedef struct tagSTREAMINFO
{
    //@ Path name of stream file
    char sPathName[_MAX_PATH];
    //@ Store format
    STREAMFORMAT StreamFormat;
    BOOL bFieldsInterleaved;
    //@ Chroma format, should be one of CHROM_FMT_420, CHROM_FMT_422, and CHROM_FMT_444
    int nChromaFormat;
    //@ Temporal order of odd and even fields
    BOOL bTopFirst;
    //@ Picture dimension, in pixels
    int nWidth;
    int nHeight;
    //@ Whether postprocess (deinterlace) is needed or not
    BOOL bDeinterlace;
    //@ Method of deinterlace if bDeinterlace is TRUE; otherwise meaningless
    unsigned int nDeintMethodID;
//  int nField2Deint; // Which field to deinterlace
    // Only deinterlace second field in a frame
}STREAMINFO;

class CProcCtrl  
{
public:
    //@ Raw yuv information
    STREAMINFO m_StreamInfo;
    //@ Raw yuv data loader
    CRawMovie *m_pRawMovie;
    //@ Picture buffer management
    CPictureManagement picture_mgmnt;
    //@ Deinterlacing process control (research use)
    DYNAMIC dynamic_params;

    //@ Get luma buffer stride
    int GetYStride(){ return m_y_stride; }
    //@ Get chroma buffer stride
    int GetUVStride() { return m_uv_stride; }

    //@ RGB buffers. ProcCtrl may perform color space translation to 
    //@ these buffers
    //@ Double buffer mechanism. One is onscreen, another is offscreen. 
    //@ These buffers are allocated by outside
    void *clrbuf[2];

    //@ Color convertor
    YUVTORGB YUVToRGBConvertor;
private:
    //@ Buffer parameters
    int   m_y_stride;
    int   m_uv_stride;
    int   m_ysize;
    int   m_csize;

    //@ Y buffer of newly decoded frame
    BYTE *m_current_y;

    //@ Display management
    int  m_nOnDisplay;         // Index of on-screen buffer in clrbuf[]
    int  m_nPictureOffDisplay; // Picture in off-screen buffer
    int  m_nPictureOnDisplay;  // Picture in on-screen buffer
    BOOL m_bOffscreenDirty;    // Dirty flag for off-screen
private:
    int GetNextPicture(int forward); // Calculate following picture number according
                                     // to play order
public:
    CProcCtrl();
    virtual ~CProcCtrl();

    //@ Init
    void InitProcCtrl();

    //@ Create and open a stream
    int CreateStream(STREAMINFO *si, BOOL bCreateBuffers);
    //@ Close a stream
    void DeleteStream();

    //@ Get y buffer of newly decoded frame
    BYTE * GetNewlyDecodedPictureY();

    //@ Calculate buffer parameters (uv stride, y size, uv size) according 
    //@ to y stride
    void UpdateBufferInfo(int y_stride);

    //@ Set picture buffers externally
    BOOL SetBuffers(BYTE **y, BYTE **u, BYTE **v, int count, int y_stride);

    //@ Get total pictures in current stream
    int GetPictureCount();
    //@ Get index of on-screen buffer in clrbuf[]
    int  GetOnScreenBufferIndex();
    //@ Get POC currently dislayed
    int  GetPocInOnScreen();
    //@ Check whether off-screen has new data and is ready for display
    BOOL IsOffscreenUpdated();

    //@ Decoding the next picture (in play sequence)
    int DecodingNextPicture(int forward);
    //@ Decoding a picture with specified POC
    int DecodingPicture(int n, BOOL MinIsOldest);
    //@ Hook for Flip operaton (swap offscreen and onscreen)
    void OnFlip();

    //@ Color spcae conversion
    void SetRGBSpaceMask(int mask);
    int  GetRGBSpaceMask(){return m_nMaskColorChannel;}
    void EnableColorConvert(BOOL bEnable);

    //@ Save result
    BOOL OpenOutput(const char *sPathName, int format); // 0 - Don't save; 1 - Save to default YUV (chroma lossless); 2 - Save to YV12 (possible chroma lossy); Other - Not support
    void CloseOutput();

private:
    PICTUREENTRY m_extra_deinterlaced;// Some methods will 
                        // deinterlace two frames simultaneously
                        // This variable records the extra de-
                        // interlaced frame number
    //@ Save deinterlaced yuv frame into file
    int SaveYUVToFile(BYTE *bufy, BYTE *bufcb, BYTE *bufcr);
    //@ Enable/disable color space conversion
    BOOL m_bConvert2RGB;
    //@ Color channel for conversion
    unsigned int m_nMaskColorChannel;
    //@ Save result to disk
    int  m_nSaveResult; // 0 - Don't save; 1 - Save to default YUV (chroma lossless); 2 - Save to YV12 (possible chroma lossy); Other - Not support
    FILE *m_fpYUV;     // File handle to save YUV file
};

#endif /* __PROCCTRL_H__ */

