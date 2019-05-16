/*
 ***********************************************************************
 * Copyright 2005-2008 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*!
 ***********************************************************************
 * Implementation file of "View" class
 *
 * History
 *   Date          Changed                                Changed by
 *   Nov. 15, 2007 Create and port from DeintApp.cpp      Zhenyong Chen
 ***********************************************************************
 */

#include "common.h"
#include "RawMovieFile.h"
#include "RawMovieViewView.h"

void CRawMovieViewView::CleanupDisplay()
{
    int i;
    // Delete GDI bitmaps
    for(i=0; i<2; i++)
    {
        if(clrbuf[i])
        {
            delete clrbuf[i];
            clrbuf[i] = NULL;
        }
    }

    m_pBufferLocked = NULL;
}

BOOL CRawMovieViewView::InitDisplay(STREAMINFO *psi)
{
    int i;
    // Calc display dimension
    m_nDisplayWidth = psi->nWidth;
    m_nDisplayHeight = psi->nHeight;

    // Create new bmp
    for(i=0; i<2; i++)
        clrbuf[i] = new DWORD[m_nDisplayWidth * m_nDisplayHeight];
    if(clrbuf[0] == NULL || clrbuf[1] == NULL)
    {
        PRINTF("Cannot load new stream!");
        CloseOldStream();
        CleanupDisplay();
        return FALSE;
    }
    else
    {
        // Pass rgb buffers to CProcCtrl
        m_ProcCtrl.clrbuf[0] = clrbuf[0];
        m_ProcCtrl.clrbuf[1] = clrbuf[1];
        m_ProcCtrl.EnableColorConvert(FALSE);
        m_ProcCtrl.InitProcCtrl();
        m_ProcCtrl.YUVToRGBConvertor = NULL;
    }

    m_bOpened = TRUE;

    return TRUE;
}

CRawMovieViewView::CRawMovieViewView()
{
    // 1. Display
    m_nDisplayWidth = 352;//m_nWidth;
    m_nDisplayHeight = 288;//m_nHeight; // No deinterlace

    // GDI
    clrbuf[0] = clrbuf[1] = NULL;
    // 2. Others
    m_bOpened = FALSE;
    m_pBufferLocked = NULL;
}

CRawMovieViewView::~CRawMovieViewView()
{
    CloseOldStream();
    CleanupDisplay();
}

void CRawMovieViewView::OnDraw()
{
    int idx = m_ProcCtrl.GetOnScreenBufferIndex();

    if(idx >= 0 && idx <= 1)
    {
        // Display current picture number
        char str[32];
        sprintf(str, "%8d      ", m_ProcCtrl.GetPocInOnScreen());
        fprintf(stderr, str);
        fprintf(stderr, "\r");
        fflush(stderr);
    }
}

void CRawMovieViewView::ShowInformation()
{
    char text[2048];
    if(clrbuf[0] != NULL)
    {
        char tmp[1024];
        strcpy(tmp, "Video: ");
        strcat(tmp, m_ProcCtrl.m_StreamInfo.sPathName);
        strcat(tmp, "\n");
        strcpy(text, tmp);
        sprintf(tmp, "Width=%d, Height=%d\n", m_ProcCtrl.m_StreamInfo.nWidth, m_ProcCtrl.m_StreamInfo.nHeight);
        strcat(text, tmp);
        strcat(text, "File format: ");
        switch(m_ProcCtrl.m_StreamInfo.StreamFormat)
        {
        case STREAM_YUV:
            strcat(text, "YUV\n");
            break;
        case STREAM_CbYCrY:
            strcat(text, "CbYCrY\n");
            break;
        case STREAM_KEV:
            strcat(text, "KEV\n");
            break;
        default:
            strcat(text, "Unknown\n");
            break;
        }
        strcat(text, "Chroma format: ");
        switch(m_ProcCtrl.m_StreamInfo.nChromaFormat)
        {
        case CHROM_FMT_420:
            strcat(text, "4:2:0\n");
            break;
        case CHROM_FMT_422:
            strcat(text, "4:2:2\n");
            break;
        case CHROM_FMT_444:
            strcat(text, "4:4:4\n");
            break;
        default:
            strcat(text, "Unknown\n");
            break;
        }
        if(m_ProcCtrl.m_StreamInfo.bDeinterlace)
        {
            strcat(text, (m_ProcCtrl.m_StreamInfo.bTopFirst) ? "Top field first\n" : "Bottom field first\n");
            strcat(text, "Postprocess: deinterlace, method: ");
            strcat(text, GetMethodName(m_ProcCtrl.m_StreamInfo.nDeintMethodID));
            strcat(text, "\n");
        }
    }
    else
    {
        strcat(text, "Please open a video file.");
    }
    // display
    PRINTF(text);
}


void CRawMovieViewView::CloseOldStream()
{
    // Remove old stream
    m_ProcCtrl.DeleteStream();

    m_bOpened = FALSE;
}

void CRawMovieViewView::OpenFile(const char *sPathName)
{
    // Parameters passed by external variable
    extern STREAMINFO si;
//  STREAMINFO *psi = &m_ProcCtrl.m_StreamInfo;
    // Input parameter has been set in global value si
    // sPathName is ignored
#ifdef eLinux_iMX31
    if(access(si.sPathName, 4) == -1)
//#else
//    if(_access(si.sPathName, 4) == -1)
    {
        PRINTF("Cannot open file %s!\n", si.sPathName);
        return;
    }
#endif
    CloseOldStream();
    CleanupDisplay();
    if(m_ProcCtrl.CreateStream(&si, TRUE) == 0)
    {
        if(InitDisplay(&si))
        {
            // Display first picture
//          OnHome();
            // Print information
            ShowInformation();
        }
        else
        {
            PRINTF("Failed to init display!\n");
        }
    }
    else
    {
        PRINTF("Failed to create stream! Maybe file %s is missing.\n", si.sPathName);
    }
}

// Force redraw picture
void CRawMovieViewView::UpdateDisplay()
{
    // Force update
    BOOL bInvalidate = TRUE;

    // Get the just prepared off-screen buffer
    BYTE *pY = m_ProcCtrl.GetNewlyDecodedPictureY();
    // Show
    if(pY)
    {
        // Lock pBuffer and free previous buffer
        m_ProcCtrl.picture_mgmnt.LockPicture(pY);
        m_ProcCtrl.picture_mgmnt.UnLockPicture(m_pBufferLocked);
        m_pBufferLocked = pY;
    }

    // Notify ProcCtrl that surface is flipped
    m_ProcCtrl.OnFlip();

    if(bInvalidate)
        OnDraw();
}

// Decode a picture and display it
void  CRawMovieViewView::DisplayPicture(int n, BOOL MinIsOldest)
{
    if(m_ProcCtrl.DecodingPicture(n, MinIsOldest) == 0)
    {
        UpdateDisplay();
    }
}

void  CRawMovieViewView::OnNext()
{
    DisplayPicture(m_ProcCtrl.GetPocInOnScreen()+1, TRUE);
}

void  CRawMovieViewView::OnPrev() 
{
    if(m_ProcCtrl.GetPocInOnScreen()-1 >= 0)
        DisplayPicture(m_ProcCtrl.GetPocInOnScreen()-1, FALSE);
}

void  CRawMovieViewView::OnHome() 
{
    DisplayPicture(0, TRUE);
}

void  CRawMovieViewView::OnEnd() 
{
    if(m_ProcCtrl.m_pRawMovie != NULL)
    {
        DisplayPicture(m_ProcCtrl.m_pRawMovie->GetPictureCount()-1, FALSE);
    }
}
void CRawMovieViewView::SetDeinterlacingProcessControl(int param1, int param2, int param3)
{
    m_ProcCtrl.dynamic_params.nParam1 = param1;
    m_ProcCtrl.dynamic_params.nParam2 = param2;
    m_ProcCtrl.dynamic_params.nParam3 = param3;
    // Update
    DisplayPicture(m_ProcCtrl.GetPocInOnScreen(), TRUE);
}

void CRawMovieViewView::OnGotopicture() 
{
    int nPic;
    int readcnt;
    readcnt = scanf("%d", &nPic);

    if(readcnt == 0 || readcnt == EOF)
    {
        PRINTF("Please input destination picture count.");
    }
    else
    {
        DisplayPicture(nPic, TRUE);
    }
}

int CRawMovieViewView::OnSaveResult(const char *save_yuv) 
{
    BOOL bSaveFile;
    bSaveFile = (save_yuv != NULL);

    if(bSaveFile)
    {
#ifdef eLinux_iMX31
        if(access(save_yuv, 0) != -1)
        {
            PRINTF("File %s already exists. Please remove it first.\n",
                save_yuv);
            bSaveFile = FALSE;
            return -1;
        }
        else
#endif
        {
            if(!m_ProcCtrl.OpenOutput(save_yuv, 1))
            {
                PRINTF("\n\nCannot save yuv file.\n\n");
            }
        }
    }
    else
    {
        m_ProcCtrl.CloseOutput();
    }

    return 0;
}
