/*
 ***********************************************************************
 * Copyright 2005-2008 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*!
 ***********************************************************************
 * Implementaion file of raw yuv video loader
 *
 * Support file format
 *    YUV file, with chroma format 420, 422, or 444
 *    CbYCrY file, chroma format is 422
 *    KEV file, with chroma format 420, 422, or 444
 *
 *
 * History
 *   Date          Changed                                Changed by
 *   Sep. 1, 2007  Create                                 Zhenyong Chen
 *   Sep. 7, 2007  Support CbYCrY storage format (VQEG)   Zhenyong Chen
 *   Sep. 11, 2007 Support separate luma, chroma color    Zhenyong Chen
 *                 space conversion
 *   Nov. 14, 2007 Remove color space conversion code to  Zhenyong Chen
 *                 file ColorConversion_UnSafe.cpp
 *   Nov. 28, 2007 Disk file access: using bigger buffer  Zhenyong Chen
 *                 and improve performace greatly.
 ***********************************************************************
 */


#include "common.h"
#include "RawMovieFile.h"
#include "Deinterlace.h"

/////////////////////////////////////////////////////////////////////////////
// CRawMovie
CRawMovie::CRawMovie()
{
    m_nChromFormat = CHROM_FMT_420;
    m_nPictures = 0;
}
CRawMovie::~CRawMovie()
{
}
void CRawMovie::GetDimension(int *width, int *height)
{
    *width = m_nWidth;
    *height = m_nHeight;
}
int CRawMovie::GetChromaFormat()
{
    return m_nChromFormat;
}

BOOL CRawMovie::IsPicturePositionValid(int pic)
{
    return (pic >= 0 && pic < m_nPictures);
}
int CRawMovie::GetPictureCount()
{
    return m_nPictures;
}

int GetFileLength(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    return ftell (fp);
}
/////////////////////////////////////////////////////////////////////////////
// CYUVMovie
CYUVMovie::CYUVMovie()
{
    m_fMovie = NULL;
    m_bOpen = FALSE;
    m_nStoreFmt = STREAM_YUV;
    m_bTopFirst = TRUE;
    m_tmpBuffer = NULL;
}
CYUVMovie::~CYUVMovie()
{
    if(m_bOpen)
    {
        fclose(m_fMovie);
        m_fMovie = NULL;
        m_bOpen = FALSE;
        if(m_tmpBuffer)
        {
            FREE(m_tmpBuffer);
            m_tmpBuffer = NULL;
        }
    }
}
int CYUVMovie::OpenMovie(const char *strFile, int width, int height, int chromFmt, STREAMFORMAT storeFmt)
{
    // Close old file
    if(m_bOpen)
    {
        fclose(m_fMovie);
        m_fMovie = NULL;
        m_bOpen = FALSE;
        if(m_tmpBuffer)
        {
            FREE(m_tmpBuffer);
            m_tmpBuffer = NULL;
        }
    }
    // Check parameters
    if(width <= 0 || height <= 0)
        return -1;

    // Open new file
    m_fMovie = fopen(strFile, "rb");
    if(m_fMovie == NULL)
    {
        return -1;
    }
    // Setting parameter
    m_nWidth = width;
    m_nHeight = height;
    m_nChromFormat = chromFmt;
    m_nStoreFmt = storeFmt;
    m_bOpen = TRUE;

    if(m_nStoreFmt == STREAM_CbYCrY)
        m_nChromFormat = CHROM_FMT_422;

    int ysize = m_nWidth * m_nHeight;
    int csize;

    switch(m_nChromFormat)
    {
    case CHROM_FMT_420:
        csize = ysize >> 2;
        break;
    case CHROM_FMT_422:
        csize = ysize >> 1;
        break;
    case CHROM_FMT_444:
        csize = ysize;
        break;
    default:
        return -1;//not support
    }

    m_tmpBuffer = (BYTE *)MALLOC(ysize + csize * 2);
    if(m_tmpBuffer == NULL)
        return -1;

    m_nPictures = GetFileLength(m_fMovie) / (ysize + (csize << 1));

    return 0;
}

int CYUVMovie::ReadPicture(int pic, BYTE *buf_y, BYTE *buf_cb, BYTE *buf_cr, int stride_y, int stride_c)
{
    ASSERT(m_bInterleaved);

    if(!m_bInterleaved)
        return -1; // Not support
    if(m_bOpen)
    {
        if(!IsPicturePositionValid(pic))
            return -2;

        // calc position
        int ysize = m_nWidth * m_nHeight;
        int csize;
        int cwidth, cheight;

        switch(m_nChromFormat)
        {
        case CHROM_FMT_420:
            cwidth = m_nWidth >> 1;
            cheight = m_nHeight >> 1;
            csize = ysize >> 2;
            break;
        case CHROM_FMT_422:
            cwidth = m_nWidth >> 1;
            cheight = m_nHeight;
            csize = ysize >> 1;
            break;
        case CHROM_FMT_444:
            cwidth = m_nWidth;
            cheight = m_nHeight;
            csize = ysize;
            break;
        default:
            return -1;//not support
        }

        if(m_nStoreFmt == STREAM_YUV)
        {
            int picsize = ysize + (csize << 1);
            DWORD base;

            base = pic * picsize;

            // Locate
            fseek(m_fMovie, base, SEEK_SET);

            // start fetching
            if(buf_y != NULL)
            {
                if(stride_y == m_nWidth)
                {
                    fread(buf_y, sizeof(char), m_nWidth * m_nHeight, m_fMovie);
                }
                else
                {
                    fread(m_tmpBuffer, sizeof(char), m_nWidth * m_nHeight, m_fMovie);
                    BYTE *pSrc = m_tmpBuffer;
                    BYTE *pDst = buf_y;
                    int i;
                    for(i=0; i<m_nHeight; i++)
                    {
                        MEMCPY(pDst, pSrc, m_nWidth);
                        pSrc += m_nWidth;
                        pDst += stride_y;
                    }
                }
            }
            if(buf_cb != NULL)
            {
                if(stride_c == cwidth)
                {
                    fread(buf_cb, sizeof(char), cwidth * cheight, m_fMovie);
                }
                else
                {
                    fread(m_tmpBuffer, sizeof(char), cwidth * cheight, m_fMovie);
                    BYTE *pSrc = m_tmpBuffer;
                    BYTE *pDst = buf_cb;
                    int i;
                    for(i=0; i<cheight; i++)
                    {
                        MEMCPY(pDst, pSrc, m_nWidth);
                        pSrc += cwidth;
                        pDst += stride_c;
                    }
                }
            }
            if(buf_cr != NULL)
            {
                if(stride_c == cwidth)
                {
                    fread(buf_cr, sizeof(char), cwidth * cheight, m_fMovie);
                }
                else
                {
                    fread(m_tmpBuffer, sizeof(char), cwidth * cheight, m_fMovie);
                    BYTE *pSrc = m_tmpBuffer;
                    BYTE *pDst = buf_cr;
                    int i;
                    for(i=0; i<cheight; i++)
                    {
                        MEMCPY(pDst, pSrc, m_nWidth);
                        pSrc += cwidth;
                        pDst += stride_c;
                    }
                }
            }
        }
        else if(m_nStoreFmt == STREAM_CbYCrY)
        {
            ASSERT(m_nChromFormat == CHROM_FMT_422);
            int LineLen = m_nWidth * 2;
            BYTE *buf;
            int i,j;
            int picsize = ysize + (csize << 1);
            DWORD base;

            base = pic * picsize;

            fseek(m_fMovie, base, SEEK_SET);

            fread(m_tmpBuffer, sizeof(BYTE), picsize, m_fMovie);
            buf = m_tmpBuffer;

            for(i=0; i<m_nHeight; i++)
            {
                // y
                BYTE *p = buf + 1;
                for(j=0; j<m_nWidth; j++)
                {
                    buf_y[j] = *p;
                    p += 2;
                }
                buf_y += stride_y;
                // cb
                p = buf;
                for(j=0; j<m_nWidth/2; j++)
                {
                    buf_cb[j] = *p;
                    p += 4;
                }
                buf_cb += stride_c;
                // cr
                p = buf+2;
                for(j=0; j<m_nWidth/2; j++)
                {
                    buf_cr[j] = *p;
                    p += 4;
                }
                buf_cr += stride_c;

                buf += LineLen; // Next row
            }
        }
        else
        {
            return -1;
        }
        return 0;
    }
    else
    {
        return -1;
    }
}
/////////////////////////////////////////////////////////////////////////////
// CKEVMovie
CKEVMovie::CKEVMovie()
{
    m_fY = NULL;
    m_fCb = NULL;
    m_fCr = NULL;
    m_bOpen = FALSE;
    m_tmpBuffer = NULL;
}
CKEVMovie::~CKEVMovie()
{
    if(m_bOpen)
    {
        fclose(m_fY);
        m_fY = NULL;
        fclose(m_fCb);
        m_fCb = NULL;
        fclose(m_fCr);
        m_fCr = NULL;
        m_bOpen = FALSE;
        if(m_tmpBuffer)
        {
            FREE(m_tmpBuffer);
            m_tmpBuffer = NULL;
        }
    }
}
int CKEVMovie::OpenMovie(const char *strFile, int width, int height, int chromFmt, STREAMFORMAT storeFmt)
{
    char filepathname[_MAX_PATH+1];
    // Close old file
    if(m_bOpen)
    {
        fclose(m_fY);
        m_fY = NULL;
        fclose(m_fCb);
        m_fCb = NULL;
        fclose(m_fCr);
        m_fCr = NULL;
        m_bOpen = FALSE;
        if(m_tmpBuffer)
        {
            FREE(m_tmpBuffer);
            m_tmpBuffer = NULL;
        }
    }
    // Open new file
    int len = strlen(strFile);
    if(len > _MAX_PATH)
        return -1;
    strcpy(filepathname, strFile);
    // Remove appending '/' or '\\'
    if(filepathname[len-1] == '/' || filepathname[len-1] == '\\')
    {
        filepathname[len-1] = 0;
        len--;
    }
    strcat(filepathname, "/y/data");
    m_fY = fopen(filepathname, "rb");
    if(m_fY == NULL)
        return -1;
    filepathname[len] = 0;
    strcat(filepathname, "/cb/data");
    m_fCb = fopen(filepathname, "rb");
    if(m_fCb == NULL)
    {
        fclose(m_fY);
        m_fY = NULL;
        return -1;
    }
    filepathname[len] = 0;
    strcat(filepathname, "/cr/data");
    m_fCr = fopen(filepathname, "rb");
    if(m_fCr == NULL)
    {
        fclose(m_fY);
        m_fY = NULL;
        fclose(m_fCb);
        m_fCb = NULL;
        return -1;
    }

    m_bOpen = TRUE;

    // Setting parameter
    m_nWidth = width;
    m_nHeight = height;
    m_nChromFormat = chromFmt;
    m_nStoreFmt = storeFmt;

    int ysize = m_nWidth * m_nHeight;
    int csize;

    switch(m_nChromFormat)
    {
    case CHROM_FMT_420:
        csize = ysize >> 2;
        break;
    case CHROM_FMT_422:
        csize = ysize >> 1;
        break;
    case CHROM_FMT_444:
        csize = ysize;
        break;
    default:
        return -1;//Not support
    }

    m_tmpBuffer = (BYTE *)MALLOC(ysize + csize * 2);
    if(m_tmpBuffer == NULL)
        return -1;


    int picy = GetFileLength(m_fY) / ysize;
    int piccb = GetFileLength(m_fCb) / csize;
    int piccr = GetFileLength(m_fCr) / csize;
    m_nPictures = min(picy, min(piccb, piccr));

    return 0;
}

int CKEVMovie::ReadPicture(int pic, BYTE *buf_y, BYTE *buf_cb, BYTE *buf_cr, int stride_y, int stride_c)
{
    if(m_bOpen)
    {
        if(!IsPicturePositionValid(pic))
            return -2;

        // Calc position
        int ysize = m_nWidth * m_nHeight;
        int csize;
        int cwidth, cheight;

        switch(m_nChromFormat)
        {
        case CHROM_FMT_420:
            cwidth = m_nWidth >> 1;
            cheight = m_nHeight >> 1;
            csize = ysize >> 2;
            break;
        case CHROM_FMT_422:
            cwidth = m_nWidth >> 1;
            cheight = m_nHeight;
            csize = ysize >> 1;
            break;
        case CHROM_FMT_444:
            cwidth = m_nWidth;
            cheight = m_nHeight;
            csize = ysize;
            break;
        default:
            return -1;//Not support
        }
        DWORD yoff = pic * ysize;
        DWORD cboff = pic * csize;
        DWORD croff = pic * csize;

        if(!m_bInterleaved) // Frame is split into 2 seperate fields
        {
            // Start fetching
            if(buf_y != NULL)
            {
                int i;
                BYTE *ydst;
                BYTE *ysrc;
                fseek(m_fY, yoff, SEEK_SET);
                fread(m_tmpBuffer, sizeof(BYTE), ysize, m_fY);
                ysrc = m_tmpBuffer;
                // First field
                if(m_bTopFirst)
                    ydst = buf_y;
                else
                    ydst = buf_y + stride_y;

                for(i=0; i<m_nHeight; i+=2)
                {
                    MEMCPY(ydst, ysrc, m_nWidth);
                    ysrc += m_nWidth;
                    ydst += stride_y*2;
                }
                // Second field
                if(m_bTopFirst)
                    ydst = buf_y + stride_y;
                else
                    ydst = buf_y;

                for(i=0; i<m_nHeight; i+=2)
                {
                    MEMCPY(ydst, ysrc, m_nWidth);
                    ysrc += m_nWidth;
                    ydst += stride_y*2;
                }
            }
            if(buf_cb != NULL)
            {
                int i;
                BYTE *cbdst;
                BYTE *cbsrc;
                fseek(m_fCb, cboff, SEEK_SET);
                fread(m_tmpBuffer, sizeof(BYTE), csize, m_fCb);
                cbsrc = m_tmpBuffer;
                // First field
                if(m_bTopFirst)
                    cbdst = buf_cb;
                else
                    cbdst = buf_cb + stride_c;
                for(i=0; i<cheight; i+=2)
                {
                    MEMCPY(cbdst, cbsrc, cwidth);
                    cbsrc += cwidth;
                    cbdst += stride_c*2;
                }
                // Second field
                if(m_bTopFirst)
                    cbdst = buf_cb + stride_c;
                else
                    cbdst = buf_cb;

                for(i=0; i<cheight; i+=2)
                {
                    MEMCPY(cbdst, cbsrc, cwidth);
                    cbsrc += cwidth;
                    cbdst += stride_c*2;
                }
            }
            if(buf_cr != NULL)
            {
                int i;
                BYTE *crdst;
                BYTE *crsrc;
                fseek(m_fCr, croff, SEEK_SET);
                fread(m_tmpBuffer, sizeof(BYTE), csize, m_fCr);
                crsrc = m_tmpBuffer;
                // First field
                if(m_bTopFirst)
                    crdst = buf_cr;
                else
                    crdst = buf_cr + stride_c;
                for(i=0; i<cheight; i+=2)
                {
                    MEMCPY(crdst, crsrc, cwidth);
                    crsrc += cwidth;
                    crdst += stride_c*2;
                }
                // Second field
                if(m_bTopFirst)
                    crdst = buf_cr + stride_c;
                else
                    crdst = buf_cr;

                for(i=0; i<cheight; i+=2)
                {
                    MEMCPY(crdst, crsrc, cwidth);
                    crsrc += cwidth;
                    crdst += stride_c*2;
                }
            }
        }
        else
        {
            // Start fetching
            if(buf_y != NULL)
            {
                fseek(m_fY, yoff, SEEK_SET);
                if(stride_y == m_nWidth)
                {
                    fread(buf_y, sizeof(char), m_nWidth * m_nHeight, m_fY);
                }
                else
                {
                    fread(m_tmpBuffer, sizeof(char), m_nWidth * m_nHeight, m_fY);
                    BYTE *pSrc = m_tmpBuffer;
                    BYTE *pDst = buf_y;
                    int i;
                    for(i=0; i<m_nHeight; i++)
                    {
                        MEMCPY(pDst, pSrc, m_nWidth);
                        pSrc += m_nWidth;
                        pDst += stride_y;
                    }
                }
            }
            if(buf_cb != NULL)
            {
                fseek(m_fCb, cboff, SEEK_SET);
                if(stride_c == cwidth)
                {
                    fread(buf_cb, sizeof(char), cwidth * cheight, m_fCb);
                }
                else
                {
                    fread(m_tmpBuffer, sizeof(char), cwidth * cheight, m_fCb);
                    BYTE *pSrc = m_tmpBuffer;
                    BYTE *pDst = buf_cb;
                    int i;
                    for(i=0; i<cheight; i++)
                    {
                        MEMCPY(pDst, pSrc, m_nWidth);
                        pSrc += cwidth;
                        pDst += stride_c;
                    }
                }
            }
            if(buf_cr != NULL)
            {
                fseek(m_fCr, croff, SEEK_SET);
                if(stride_c == cwidth)
                {
                    fread(buf_cr, sizeof(char), cwidth * cheight, m_fCr);
                }
                else
                {
                    fread(m_tmpBuffer, sizeof(char), cwidth * cheight, m_fCr);
                    BYTE *pSrc = m_tmpBuffer;
                    BYTE *pDst = buf_cr;
                    int i;
                    for(i=0; i<cheight; i++)
                    {
                        MEMCPY(pDst, pSrc, m_nWidth);
                        pSrc += cwidth;
                        pDst += stride_c;
                    }
                }
            }
        }
        return 0;
    }
    else
    {
        return -1;
    }
}


