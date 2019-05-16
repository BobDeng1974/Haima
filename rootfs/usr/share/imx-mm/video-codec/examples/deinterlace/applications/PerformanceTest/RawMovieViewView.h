/*
 ***********************************************************************
 * Copyright 2005-2008 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*!
 ***********************************************************************
 * Header file of "View" class
 *
 * History
 *   Date          Changed                                Changed by
 *   Nov. 15, 2007 Create and port from DeintApp.cpp      Zhenyong Chen
 ***********************************************************************
 */

#ifndef __RAWMOVIEVIEWVIEW_H__
#define __RAWMOVIEVIEWVIEW_H__

#include "ProcCtrl.h"

class CRawMovieViewView
{
public: // create from serialization only
    CRawMovieViewView();
// Attributes
public:
    CProcCtrl m_ProcCtrl;

private:
    COLORREF *clrbuf[2]; // Buffer in RGB space - to display
    int  m_nDisplayWidth;
    int  m_nDisplayHeight;

// Operations
public:

// Overrides
    void OnDraw();  // overridden to draw this view


// Implementation
private:
    void ShowInformation();
    void CloseOldStream();
    BOOL InitDisplay(STREAMINFO *psi);
    void CleanupDisplay();
    void DisplayPicture(int n, BOOL MinIsOldest);
    BYTE * m_pBufferLocked;

public:
    BOOL m_bOpened;
    void SetDeinterlacingProcessControl(int param1, int param2, int param3);
    void UpdateDisplay();
    virtual ~CRawMovieViewView();
    int  OnSaveResult(const char *save_yuv);

// Generated message map functions
public:
    void OpenFile(const char *sPathName);
    void OnNext();
    void OnPrev();
    void OnHome();
    void OnEnd();
    void OnGotopicture();
};

#endif /* __RAWMOVIEVIEWVIEW_H__ */
