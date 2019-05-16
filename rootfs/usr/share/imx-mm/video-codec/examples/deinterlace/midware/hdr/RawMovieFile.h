/*
 ***********************************************************************
 * Copyright 2005-2008 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*!
 ***********************************************************************
 * Header file of raw yuv video loader
 *
 * Support file format
 *    YUV file, with chroma format 420, 422, or 444
 *    CbYCrY file, chroma format is 422
 *    KEV file, with chroma format 420, 422, or 444
 *
 *
 * History
 *   Date          Changed                                Changed by
 *   Sep.  1, 2007 Create                                 Zhenyong Chen
 *   Sep.  7, 2007 Support CbYCrY storage format (VQEG)   Zhenyong Chen
 *   Nov. 14, 2007 Remove color space conversion code to  Zhenyong Chen
 *                 file ColorConversion_UnSafe.cpp
 ***********************************************************************
 */

#ifndef __RAWMOVIEFILE_H__
#define __RAWMOVIEFILE_H__

// Input file format
typedef enum tagSTREAMFORMAT
{
    STREAM_YUV,    // Single file, non-packet mode
    STREAM_CbYCrY, // Single file, packet mode. Chroma format is 422
    STREAM_KEV,    // Multiple files
    STREAM_UNKNOWN // Unsupport
}STREAMFORMAT;

class CRawMovie
{
/***** data *****/
public:
    //@ Format of file
    STREAMFORMAT m_nStoreFmt;
    BOOL m_bInterleaved;// Fields are interleaved to form a frame
    //@ Video infomation
    int m_nWidth;
    int m_nHeight;      // Width & height of video (in pixel)
    int m_nChromFormat; // One of CHROM_FMT_420, CHROM_FMT_422 and CHROM_FMT_444
    BOOL m_bTopFirst;   // Top field is first or not
protected:
    int m_nPictures;    // Stores total pictures in this video

/***** methods *****/
public:
    CRawMovie();
    virtual ~CRawMovie();

    //@ Set movie format, and open it. Note: parameters passed in may be overridden
    //@ if movie file has self description
    virtual int OpenMovie(const char *strFile, int width, int height, int chromFmt, STREAMFORMAT storeFmt) = 0;
    //@ Get video dimension
    void GetDimension(int *width, int *height);
    //@ Get chroma format
    int GetChromaFormat();
    //@ Get pictures
    int GetPictureCount();
    //@ Read picture N. If buf_x == NULL, then skip reading this channel
    //@     0 -- ok
    //@    -1 -- not support
    //@    -2 -- end of file
    virtual int ReadPicture(int pic, BYTE *buf_y, BYTE *buf_cb, BYTE *buf_cr, int stride_y, int stride_c) = 0;
    //@ Check whether the picture is in current stream
    virtual BOOL IsPicturePositionValid(int pic);
};

//@ This class handles STREAM_YUV and STREAM_CbYCrY file format
class CYUVMovie : public CRawMovie
{
private:
    FILE *m_fMovie;
    BOOL  m_bOpen; // Record open status of video file
    BYTE *m_tmpBuffer; // Disk file performace is bottle-neck. Loading using
                       // bigger buffer will help improve file access performance
public:
    CYUVMovie();
    virtual ~CYUVMovie();

    virtual int OpenMovie(const char *strFile, int width, int height, int chromFmt, STREAMFORMAT storeFmt);
    virtual int ReadPicture(int pic, BYTE *buf_y, BYTE *buf_cb, BYTE *buf_cr, int stride_y, int stride_c);
};

//@ This class handles STREAM_KEV file format
class CKEVMovie : public CRawMovie
{
private:
    FILE *m_fY;
    FILE *m_fCb;
    FILE *m_fCr;
    BOOL  m_bOpen; // Record open status of video file
    BYTE *m_tmpBuffer; // Disk file performace is bottle-neck. Loading using
                       // bigger buffer will help improve file access performance
public:
    CKEVMovie();
    virtual ~CKEVMovie();

    virtual int OpenMovie(const char *strFile, int width, int height, int chromFmt, STREAMFORMAT storeFmt);
    virtual int ReadPicture(int pic, BYTE *buf_y, BYTE *buf_cb, BYTE *buf_cr, int stride_y, int stride_c);
};

#endif /* __RAWMOVIEFILE_H__ */

