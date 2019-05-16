/*
 ***********************************************************************
 * Copyright 2005-2008 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*!
 ***********************************************************************
 * Implementation file of deinterlacing system process management
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
 *   Sep. 14, 2007 Support direct interfering of de-      Zhenyong Chen
 *                 interlacing process
 *   Sep. 17, 2007 Bug fixing: buffer should be protected Zhenyong Chen
 *                 during function DecodingPicture() -
 *                 prevent allocating same buffer
 *   Oct. 18, 2007 Modify API to support WinCE            Zhenyong Chen
 *   Jan. 22, 2008 Add support for double frames output   Zhenyong Chen
 ***********************************************************************
 */


#include "common.h"
#include "ProcCtrl.h"

#include "Deinterlace_Safe.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProcCtrl::CProcCtrl()
{
    m_pRawMovie = NULL;

    clrbuf[0] = clrbuf[1] = NULL;
    YUVToRGBConvertor = NULL;

    m_nOnDisplay = -1;
    m_nPictureOnDisplay = -1;
    m_nPictureOffDisplay = -1;
    m_bOffscreenDirty = FALSE;

    m_StreamInfo.nWidth = 352;
    m_StreamInfo.nHeight = 288;
    m_StreamInfo.bDeinterlace = FALSE;
    m_StreamInfo.nChromaFormat = CHROM_FMT_420;
    m_StreamInfo.StreamFormat = STREAM_YUV;
    m_StreamInfo.nDeintMethodID = (unsigned int)-1;
    m_StreamInfo.sPathName[0] = 0;

    m_bConvert2RGB = TRUE;
    m_nMaskColorChannel = 7; // Y, U, and V are enabled
    m_nSaveResult = 0; // Not save by default
    m_fpYUV = NULL;

    m_current_y = NULL;

    m_extra_deinterlaced.poc = -1;
}

CProcCtrl::~CProcCtrl()
{
    DeleteStream();
}

int CProcCtrl::CreateStream(STREAMINFO *si, BOOL bCreateBuffers)
{
    // Delete old stream
//  DeleteStream();

    // Create new stream
    switch(si->StreamFormat)
    {
    case STREAM_YUV:
    case STREAM_CbYCrY:
        m_pRawMovie = new CYUVMovie;
        break;
    case STREAM_KEV:
        m_pRawMovie = new CKEVMovie;
        break;
    default:
        break;
    }
    if(!m_pRawMovie)
        goto failed;

    m_StreamInfo = *si;

    m_pRawMovie->m_bInterleaved = m_StreamInfo.bFieldsInterleaved;
    m_pRawMovie->m_bTopFirst = m_StreamInfo.bTopFirst;

    if(m_pRawMovie->OpenMovie(si->sPathName, si->nWidth, si->nHeight, si->nChromaFormat, si->StreamFormat) == 0)
    {
        // Get dimension and chroma format from m_pRawMovie because the value may
        // be reset by descriptor file
        m_pRawMovie->GetDimension(&m_StreamInfo.nWidth, &m_StreamInfo.nHeight);
        m_StreamInfo.nChromaFormat = m_pRawMovie->GetChromaFormat();

        if(bCreateBuffers)
        {
            // Fixme! width should be 16x, height should be 2x.
            UpdateBufferInfo(((m_StreamInfo.nWidth+15)/16) * 16);

        // Create buffer - stride is row size
        if(picture_mgmnt.CreateAllPictures(m_ysize, m_csize) == -1)
            goto failed;
        }

        *si = m_StreamInfo;
        return 0;
    }
failed:
    DeleteStream();
    return -1;
}

void CProcCtrl::DeleteStream()
{
/*  if(m_fpYUV)
    {
        fclose(m_fpYUV);
        m_fpYUV = NULL;
    }*/
    if(m_pRawMovie)
    {
        delete m_pRawMovie;
        m_pRawMovie = NULL;
    }
    picture_mgmnt.DestroyAllPictures();

    clrbuf[0] = clrbuf[1] = NULL;

    m_nOnDisplay = -1;
}

int CProcCtrl::GetNextPicture(int forward)
{
    if(forward)
        return m_nPictureOnDisplay+1; // Forward
    else
        return m_nPictureOnDisplay-1;
}

int CProcCtrl::DecodingNextPicture(int forward)
{
//  if(clrbuf[!m_nOnDisplay])
    {
        int n = GetNextPicture(forward);
        if(DecodingPicture(n, forward) == 0)
        {
            return 0;
        }
    }
    return -1;
}
// 0 - Don't save; 1 - Save to default YUV (chroma lossless); 2 - Save to YV12 (possible chroma lossy); Other - Not support
BOOL CProcCtrl::OpenOutput(const char *sPathName, int format)
{
    if(m_fpYUV)
        fclose(m_fpYUV);
    m_fpYUV = fopen(sPathName, "ab");
    if(m_fpYUV)
    {
        m_nSaveResult = format;
        return TRUE;
    }
    else
        return FALSE;
}
void CProcCtrl::CloseOutput()
{
    if(m_fpYUV)
    {
        fflush(m_fpYUV);
        fclose(m_fpYUV);
    }
    m_fpYUV = NULL;
}
int CProcCtrl::SaveYUVToFile(BYTE *bufy, BYTE *bufcb, BYTE *bufcr)
{
    if(m_fpYUV == NULL)
        return -2;

    if(m_nSaveResult==1)
    {
        int i;
        int cwidth, cheight;
        BYTE *pSrc;
        switch(m_StreamInfo.nChromaFormat)
        {
        case CHROM_FMT_420:
            cwidth = m_StreamInfo.nWidth>> 1;
            cheight = m_StreamInfo.nHeight >> 1;
            break;
        case CHROM_FMT_422:
            cwidth = m_StreamInfo.nWidth >> 1;
            cheight = m_StreamInfo.nHeight;
            break;
        case CHROM_FMT_444:
            cwidth = m_StreamInfo.nWidth;
            cheight = m_StreamInfo.nHeight;
            break;
        default:
            cwidth = 0;
            cheight = 0;
            break;
        }

        pSrc = bufy;
        for(i=0; i<m_StreamInfo.nHeight; i++)
        {
            fwrite(pSrc, sizeof(BYTE), m_StreamInfo.nWidth, m_fpYUV);
            pSrc += m_y_stride;
        }
        pSrc = bufcb;
        for(i=0; i<cheight; i++)
        {
            fwrite(pSrc, sizeof(BYTE), cwidth, m_fpYUV);
            pSrc += m_uv_stride;
        }
        pSrc = bufcr;
        for(i=0; i<cheight; i++)
        {
            fwrite(pSrc, sizeof(BYTE), cwidth, m_fpYUV);
            pSrc += m_uv_stride;
        }

        return 0;
    }
    else if(m_nSaveResult == 2) // YV12
    {
        int i;
        int cwidth, cheight;
        BYTE *pSrc;
        BOOL downsample_h = 0;
        BOOL downsample_v = 0;
        cwidth = m_StreamInfo.nWidth >> 1;
        cheight = m_StreamInfo.nHeight >> 1;
        switch(m_StreamInfo.nChromaFormat)
        {
        case CHROM_FMT_420:
            break;
        case CHROM_FMT_422:
            downsample_v = 1;
            break;
        case CHROM_FMT_444:
            downsample_v = 1;
            downsample_h = 1;
            break;
        default:
            cwidth = 0;
            cheight = 0;
            break;
        }

        pSrc = bufy;
        for(i=0; i<m_StreamInfo.nHeight; i++)
        {
            fwrite(pSrc, sizeof(BYTE), m_StreamInfo.nWidth, m_fpYUV);
            pSrc += m_y_stride;
        }
        if(downsample_h == 0)
        {
            pSrc = bufcb;
            for(i=0; i<cheight/2; i++)
            {
                fwrite(pSrc, sizeof(BYTE), cwidth, m_fpYUV);
                pSrc += m_uv_stride;
                fwrite(pSrc, sizeof(BYTE), cwidth, m_fpYUV);
                pSrc += m_uv_stride;
                if(downsample_v)
                    pSrc += (m_uv_stride<<1);
            }
            pSrc = bufcr;
            for(i=0; i<cheight/2; i++)
            {
                fwrite(pSrc, sizeof(BYTE), cwidth, m_fpYUV);
                pSrc += m_uv_stride;
                fwrite(pSrc, sizeof(BYTE), cwidth, m_fpYUV);
                pSrc += m_uv_stride;
                if(downsample_v)
                    pSrc += (m_uv_stride<<1);
            }
        }
        else
        {
            BYTE *pTmp = (BYTE *)MALLOC(cwidth * sizeof(BYTE));
            if(pTmp)
            {
				int j;
                pSrc = bufcb;
                for(i=0; i<cheight/2; i++)
                {
                    for(j=0; j<cwidth; j++)
                    {
                        pTmp[j] = pSrc[j<<1];
                    }
                    fwrite(pTmp, sizeof(BYTE), cwidth, m_fpYUV);
                    pSrc += m_uv_stride;
                    for(j=0; j<cwidth; j++)
                    {
                        pTmp[j] = pSrc[j<<1];
                    }
                    fwrite(pTmp, sizeof(BYTE), cwidth, m_fpYUV);
                    pSrc += m_uv_stride;
                    if(downsample_v)
                        pSrc += (m_uv_stride<<1);
                }
                pSrc = bufcr;
                for(i=0; i<cheight/2; i++)
                {
                    for(j=0; j<cwidth; j++)
                    {
                        pTmp[j] = pSrc[j<<1];
                    }
                    fwrite(pTmp, sizeof(BYTE), cwidth, m_fpYUV);
                    pSrc += m_uv_stride;
                    for(j=0; j<cwidth; j++)
                    {
                        pTmp[j] = pSrc[j<<1];
                    }
                    fwrite(pTmp, sizeof(BYTE), cwidth, m_fpYUV);
                    pSrc += m_uv_stride;
                    if(downsample_v)
                        pSrc += (m_uv_stride<<1);
                }

                FREE(pTmp);
            }
        }

        return 0;
    }

    return -1;
}
// Return: -1 -- Bad chroma format
//         -2 -- File error
//         0  -- OK
#define NEXT_POC(n, forward) ((forward)?((n)+1):((n)-1))
#define PREV_POC(n, forward) ((forward)?((n)-1):((n)+1))
#define IS_VALID_POC(n) ((n) >= 0 && (n) < m_pRawMovie->GetPictureCount())
int CProcCtrl::DecodingPicture(int n, BOOL MinIsOldest)
{
    PICTUREENTRY *picture = NULL;
    PICTUREENTRY *pic_prev = NULL;
    PICTUREENTRY *pic_next = NULL;
    int ret = -2;
    int pic_reusable = 0;
    BYTE *bufy = NULL;
    BYTE *bufcb = NULL;
    BYTE *bufcr = NULL;

    if(m_pRawMovie != NULL && IS_VALID_POC(n))
    {
        // Fetch yuv data
        int stride_rgb = m_StreamInfo.nWidth << 2; // 32-bit

        // Check whether the frame is already deinterlaced
        if(m_extra_deinterlaced.poc == n)
        {
            bufy = m_extra_deinterlaced.y;
            bufcb = m_extra_deinterlaced.cb;
            bufcr = m_extra_deinterlaced.cr;
            pic_reusable = -1;
            goto postprocess;
        }


        // Lock picture n
        picture = picture_mgmnt.Find(n);
        if(picture)
            picture->nlock++;
        if(m_StreamInfo.bDeinterlace)
        {
            if(IsMethodNeedPrevFrame(m_StreamInfo.nDeintMethodID))
            {
                pic_prev = picture_mgmnt.Find(PREV_POC(n, MinIsOldest));
                if(pic_prev)
                    pic_prev->nlock++;
            }
            if(IsMethodNeedNextFrame(m_StreamInfo.nDeintMethodID))
            {
                pic_next = picture_mgmnt.Find(NEXT_POC(n, MinIsOldest));
                if(pic_next)
                    pic_next->nlock++;
            }
        }

        if(picture == NULL)
        {
            picture = picture_mgmnt.AllocPicture(MinIsOldest);
            if(picture)
            {
                if(m_pRawMovie->ReadPicture(n, picture->y, picture->cb, picture->cr, m_y_stride, m_uv_stride) == 0)
                {
                    picture->poc = n;
                    picture->nlock++;
                }
                else
                    picture = NULL;
            }
        }

        // Current field ready
        if(picture)
        {
            if(m_StreamInfo.bDeinterlace)
            {
                if(IsMethodNeedPrevFrame(m_StreamInfo.nDeintMethodID) && pic_prev == NULL)
                {
                    BOOL bNeedFetchPrev = TRUE;
                    if(IS_VALID_POC(PREV_POC(n, MinIsOldest)))
                    {
                        pic_prev = picture_mgmnt.Find(PREV_POC(n, MinIsOldest));
                        if(pic_prev)
                            bNeedFetchPrev = FALSE;
                        else
                            pic_prev = picture_mgmnt.AllocPicture(MinIsOldest);
                    }

                    BOOL bFetchedPrev = !bNeedFetchPrev;
                    if(pic_prev && bNeedFetchPrev)
                    {
                        if(m_pRawMovie->ReadPicture(PREV_POC(n, MinIsOldest), pic_prev->y, pic_prev->cb, pic_prev->cr, m_y_stride, m_uv_stride) == 0)
                        {
                            pic_prev->poc = PREV_POC(n, MinIsOldest);
                            pic_prev->nlock++;
                            bFetchedPrev = TRUE;
                        }
                    }

                    if(pic_prev && !bFetchedPrev)
                    {
                        ret = -2;
                        goto end;
                    }
                }
                
                if(IsMethodNeedNextFrame(m_StreamInfo.nDeintMethodID) && pic_next == NULL)
                {
                    BOOL bNeedFetchNext = TRUE;
                    if(IS_VALID_POC(NEXT_POC(n, MinIsOldest)))
                    {
                        pic_next = picture_mgmnt.Find(NEXT_POC(n, MinIsOldest));
                        if(pic_next)
                            bNeedFetchNext = FALSE;
                        else
                            pic_next = picture_mgmnt.AllocPicture(MinIsOldest);
                    }

                    BOOL bFetchedNext = !bNeedFetchNext;
                    if(pic_next && bNeedFetchNext)
                    {
                        if(m_pRawMovie->ReadPicture(NEXT_POC(n, MinIsOldest), pic_next->y, pic_next->cb, pic_next->cr, m_y_stride, m_uv_stride) == 0)
                        {
                            pic_next->poc = NEXT_POC(n, MinIsOldest);
                            pic_next->nlock++;
                            bFetchedNext = TRUE;
                        }
                    }

                    if(pic_next && !bFetchedNext)
                    {
                        ret = -2;
                        goto end;
                    }
                }

                // Calc
                DEINTER di;
                if(pic_prev)
                {
                    di.frame[0].y = pic_prev->y;
                    di.frame[0].cb = pic_prev->cb;
                    di.frame[0].cr = pic_prev->cr;
                }
                else
                {
                    di.frame[0].y = NULL;
                    di.frame[0].cb = NULL;
                    di.frame[0].cr = NULL;
                }
                if(picture)
                {
                    di.frame[1].y = picture->y;
                    di.frame[1].cb = picture->cb;
                    di.frame[1].cr = picture->cr;
                }
                else
                {
                    di.frame[1].y = NULL;
                    di.frame[1].cb = NULL;
                    di.frame[1].cr = NULL;
                }
                if(pic_next)
                {
                    di.frame[2].y = pic_next->y;
                    di.frame[2].cb = pic_next->cb;
                    di.frame[2].cr = pic_next->cr;
                }
                else
                {
                    di.frame[2].y = NULL;
                    di.frame[2].cb = NULL;
                    di.frame[2].cr = NULL;
                }
                di.chrom_fmt = m_StreamInfo.nChromaFormat;
                di.y_stride = m_y_stride;
                di.uv_stride = m_uv_stride;
                di.width = m_StreamInfo.nWidth;
                di.height = m_StreamInfo.nHeight;
                di.top_first = m_StreamInfo.bTopFirst;
//              di.top_deint = (m_StreamInfo.nField2Deint==0);
                di.method = m_StreamInfo.nDeintMethodID;
                di.dynamic_params = &dynamic_params;
#ifdef TIME_PROFILE
                extern void deinterlace_start();
                deinterlace_start();
#endif
                pic_reusable = Deinterlace(&di);
                // Record extra frame deinterlaced
                if(di.method == DEINTMETHOD_BLOCK_VT_GROUP_FRAMESAD_4TAP ||
                   di.method == DEINTMETHOD_BLOCK_VT_GROUP_FRAMESAD_BOB)
                {
                    m_extra_deinterlaced.y = di.frame[2].y;
                    m_extra_deinterlaced.cb = di.frame[2].cb;
                    m_extra_deinterlaced.cr = di.frame[2].cr;
                    if(m_extra_deinterlaced.y != NULL)
                        m_extra_deinterlaced.poc = NEXT_POC(n, MinIsOldest);
                    else
                        m_extra_deinterlaced.poc = -1;
                }
#ifdef TIME_PROFILE
                extern void deinterlace_stop();
                deinterlace_stop();
#endif
            } // if(m_StreamInfo.bDeinterlace)

            bufy = picture->y;
            bufcb = picture->cb;
            bufcr = picture->cr;
        } // if(picture)
postprocess:
        if(bufy)
        {
            if(m_fpYUV)
                SaveYUVToFile(bufy, bufcb, bufcr);

            // Convert yuv to rgb
            if(m_bConvert2RGB && clrbuf[!m_nOnDisplay] && YUVToRGBConvertor)
            {
                YUVToRGBConvertor((BYTE *)clrbuf[!m_nOnDisplay], 
                                        stride_rgb, 
                                        (const BYTE *)bufy, 
                                        (const BYTE *)bufcb, 
                                        (const BYTE *)bufcr, 
                                        m_y_stride, 
                                        m_uv_stride,
                                        m_pRawMovie->m_nChromFormat,
                                        m_StreamInfo.nWidth,
                                        m_StreamInfo.nHeight,
                                        m_nMaskColorChannel);
            }

            m_nPictureOffDisplay = n;

            m_bOffscreenDirty = TRUE;

            ret = 0;
            goto end;
        }
    }

end:
    // Free
    if(picture)
        picture->nlock--;
    if(pic_prev)
        pic_prev->nlock--;
    if(pic_next)
        pic_next->nlock--;
    if(pic_reusable == 0)
    {
        picture_mgmnt.InvalidPicture(n);
        if(m_StreamInfo.nDeintMethodID == DEINTMETHOD_BLOCK_VT_GROUP_FRAMESAD_4TAP ||
           m_StreamInfo.nDeintMethodID == DEINTMETHOD_BLOCK_VT_GROUP_FRAMESAD_BOB)
        {
            picture_mgmnt.InvalidPicture(NEXT_POC(n, MinIsOldest));
        }
    }
    // expose bufy for direct data access
    if(bufy)
        m_current_y = bufy;
    else
        m_current_y = NULL;

    return ret;
}
void CProcCtrl::OnFlip()
{
    int tmp;
    // Flip offscreen to onscreen
    m_nOnDisplay = !m_nOnDisplay;
    tmp = m_nPictureOnDisplay;
    m_nPictureOnDisplay = m_nPictureOffDisplay;
    m_nPictureOffDisplay = tmp;

    // Mark offscreen clean
    m_bOffscreenDirty = FALSE;
}
BOOL CProcCtrl::IsOffscreenUpdated()
{
    return m_bOffscreenDirty;
}
int CProcCtrl::GetOnScreenBufferIndex()
{
    return m_nOnDisplay;
}
int CProcCtrl::GetPocInOnScreen()
{
    return m_nPictureOnDisplay;
}

void CProcCtrl::InitProcCtrl()
{
    m_nPictureOnDisplay = -1;
    m_nPictureOffDisplay = -1;
}

int CProcCtrl::GetPictureCount()
{
    if(m_pRawMovie)
        return m_pRawMovie->GetPictureCount();
    else
        return 0;
}
BOOL CProcCtrl::SetBuffers(BYTE **y, BYTE **u, BYTE **v, int count, int y_stride)
{
    UpdateBufferInfo(y_stride);
    return picture_mgmnt.SetBuffers(y, u, v, count);
}

// Calculate uv_stride, ysize and csize
void CProcCtrl::UpdateBufferInfo(int y_stride)
{
    m_y_stride = y_stride;

    switch(m_StreamInfo.nChromaFormat)
    {
    case CHROM_FMT_420:
    case CHROM_FMT_422:
        m_uv_stride = m_y_stride >> 1;
        break;
    case CHROM_FMT_444:
        m_uv_stride = m_y_stride;
        break;
    default:
        m_uv_stride = 0;
    }

    m_ysize = m_y_stride * ((m_StreamInfo.nHeight+1)/2) * 2;
    switch(m_StreamInfo.nChromaFormat)
    {
    case CHROM_FMT_420:
        m_csize = m_ysize >> 2;
        break;
    case CHROM_FMT_422:
        m_csize = m_ysize >> 1;
        break;
    case CHROM_FMT_444:
        m_csize = m_ysize;
        break;
    default:
        break;
    }
}

BYTE * CProcCtrl::GetNewlyDecodedPictureY()
{
    return m_current_y;
}

void CProcCtrl::SetRGBSpaceMask(int mask)
{
    m_nMaskColorChannel = mask;
    m_bConvert2RGB = TRUE;
}

void CProcCtrl::EnableColorConvert(BOOL bEnable)
{
    m_bConvert2RGB = bEnable;
}

