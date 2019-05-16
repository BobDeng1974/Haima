/*
 ***********************************************************************
 * Copyright 2008-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*!
 ***********************************************************************
 * YUVDisplayer
 *
 * A displayer to view YUV picture output by video decoder.
 *
 * History
 *   Date          Changed                                Changed by
 *   Aug. 14, 2008 Create                                 Zhenyong Chen
 *   Nov. xx, 2008 Add X11 displayer                      Zhenyong Chen
 *   Nov. 28, 2008 Add a dummy display for no displayer   Zhenyong Chen
 *                 case
 *   Dec. 04, 2008 Extract base class; redefine interface Zhenyong Chen
 ***********************************************************************
 */
#ifndef _YUVDISPLAYER_H_
#define _YUVDISPLAYER_H_

#include <stdio.h>

class baseYUVDisplayer
{
public:
    baseYUVDisplayer()
	{
	  m_nWidth = 0;
	  m_nHeight = 0;
	  m_crop_left = 0;
	  m_crop_top = 0;
	  m_crop_right = 0;
	  m_crop_bottom = 0;
      m_vb_ext_left = 16;
      m_vb_ext_top = 16;
      m_vb_ext_right = 16;
      m_vb_ext_bot = 16;
      m_bInited = 0;
	}
    virtual ~baseYUVDisplayer()
	{
	}
	virtual int OpenDisplayer(int width,
	               int height,
	               int crop_left=0,
	               int crop_top=0,
	               int crop_right=0,
	               int crop_bottom=0,
	               void *p=0)
    {
      if(m_bInited == 1)
      {
        printf("ERROR! cannot open again!\n");
        return -1;
      }
      else
      {
        m_nWidth = width;
        m_nHeight = height;
        m_crop_left = crop_left;
        m_crop_top = crop_top;
        m_crop_right = crop_right;
        m_crop_bottom = crop_bottom;
        m_in_width = m_nWidth - m_crop_left - m_crop_right;
        m_in_height = m_nHeight - m_crop_top - m_crop_bottom;
        m_bInited = 1;
        return 0;
      }
    }
    virtual void Cleanup()
    {
	  m_nWidth = m_nHeight = 0;
	  m_bInited = 0;
    }
    int DisplayYUVPicture(const unsigned char *y,
						  const unsigned char *cb,
						  const unsigned char *cr,
						  int ystride,
						  int cstride)
    {
	  return 0;
    }
    void SetVideoBufferBoundary(int ext_left, int ext_top, int ext_right, int ext_bot)
    {
      m_vb_ext_left = ext_left;
      m_vb_ext_top = ext_top;
      m_vb_ext_right = ext_right;
      m_vb_ext_bot = ext_bot;
    }

public:
	int m_nWidth;  // picture dimension
	int m_nHeight;

	int m_crop_left; // cropping visible part (m_nWidth x m_nHeight)
	int m_crop_top;
	int m_crop_right;
	int m_crop_bottom;


    int m_vb_ext_left; // boundary extension for yuv frame buffer
    int m_vb_ext_top;
    int m_vb_ext_right;
    int m_vb_ext_bot;
public:
    int m_bInited;
public:
    int m_in_width; // m_nWidth - m_crop_left - m_crop_right
    int m_in_height;
};

#if defined(TGT_OS_WIN32) || defined(TGT_OS_WINCE)
#include <windows.h>

class YUVDisplayer : public baseYUVDisplayer
{
public:
	YUVDisplayer();
	virtual ~YUVDisplayer();
	virtual int OpenDisplayer(int width,
	               int height,
	               int crop_left=0,
	               int crop_top=0,
	               int crop_right=0,
	               int crop_bottom=0,
	               void *p=0);
	int DisplayYUVPicture(const unsigned char *y,
						  const unsigned char *cb,
						  const unsigned char *cr,
						  int ystride,
						  int cstride);
	virtual void Cleanup();


//private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;
	HBITMAP  m_hVideo;
	COLORREF  *m_pDisplayBuffer; // content to draw on screen
	unsigned long _ThreadID;
	HANDLE _hThread;
	HANDLE m_hEvent;
};


#elif defined(TGT_OS_ELINUX)
//#include "typedefs.h"
#include "render_lcd.h"
typedef int BOOL;

class YUVDisplayer : public baseYUVDisplayer
{
public:
	YUVDisplayer();
	virtual ~YUVDisplayer();
	virtual int OpenDisplayer(int width,
	               int height,
	               int crop_left=0,
	               int crop_top=0,
	               int crop_right=0,
	               int crop_bottom=0,
	               void *p=0);
	int DisplayYUVPicture(const unsigned char *y,
						  const unsigned char *cb,
						  const unsigned char *cr,
						  int ystride, // should be equal to video_buffer_width
						  int cstride);
	virtual void Cleanup();
};

#elif defined (TGT_OS_UNIX) && defined(ENABLE_X11)
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <semaphore.h>
#include <time.h>

#include <pthread.h>

typedef unsigned int UINT;
typedef unsigned int DWORD;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif


class YUVDisplayer : public baseYUVDisplayer
{
public:
	YUVDisplayer();
	virtual ~YUVDisplayer();
	virtual int OpenDisplayer(int width,
	               int height,
	               int crop_left=0,
	               int crop_top=0,
	               int crop_right=0,
	               int crop_bottom=0,
	               void *p=0);
	int DisplayYUVPicture(const unsigned char *y,
						  const unsigned char *cb,
						  const unsigned char *cr,
						  int ystride,
						  int cstride);
	virtual void Cleanup();
//private:
//	HINSTANCE m_hInstance;
    Display *display;
    int screen_num;
    Window m_hWnd;
    XImage *m_hVideo;
    char   *m_pDisplayBuffer;
	pthread_t _ThreadID;
	sem_t m_hEvent;
};
#else
// define a dummy displayer
class YUVDisplayer : public baseYUVDisplayer
{
public:
	YUVDisplayer();
	virtual ~YUVDisplayer();
};

#endif

#endif /* _YUVDISPLAYER_H_ */

