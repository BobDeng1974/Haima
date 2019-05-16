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
 ***********************************************************************
 */
#include "defs.h"
#include "YUVDisplayer.h"
#include <stdio.h> // printf

#if defined(TGT_OS_WIN32) || defined(TGT_OS_WINCE)

//#include "macros.h"
#include "ColorConversion_Safe.h"

//===============================================================================
// Display window class definition

//
// FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
// PURPOSE: Processes messages for the main window.
//
// WM_COMMAND - process the application menu
// WM_PAINT - Paint the main window
// WM_DESTROY - post a quit message and return
//
//
static YUVDisplayer *pContext = NULL;
void OnDraw(HWND hWnd, HDC hDC)
{
    if(pContext == NULL || pContext->m_hVideo == NULL)
        return;

    HDC hBmpDC;
    hBmpDC = CreateCompatibleDC(hDC);
    HBITMAP hbmpold = (HBITMAP)SelectObject(hBmpDC, pContext->m_hVideo);
    BitBlt(hDC,
			0,
			0,
			0+pContext->m_in_width,
			0+pContext->m_in_height,
			hBmpDC,
			0,
			0,
			SRCCOPY);
    SelectObject(hBmpDC, hbmpold);
    DeleteDC(hBmpDC);
}

#define WM_THREAD_QUIT (WM_USER+500)
#include <Windowsx.h>
#include <stdio.h>

static int info_box_x = 0;
static int info_box_y = 0;

// UNICODE version
void OnMouseMove(HWND hWnd, int xpos, int ypos)
{
  // display information
  HDC hdc = GetDC(hWnd);
  TCHAR txt[100];
  wsprintf(txt, TEXT("x=%04d,y=%04d, mb_x=%03d,mb_y=%03d"), xpos, ypos, xpos>>4, ypos>>4);
#if defined(TGT_OS_WIN32)
  TextOut(hdc, info_box_x, info_box_y, txt, wcslen(txt));
#elif defined(TGT_OS_WINCE)
  RECT rc;
  rc.left = info_box_x;
  rc.top = info_box_y;
  rc.right = info_box_x+200;
  rc.bottom = info_box_y+20;
  DrawText(hdc, txt, wcslen(txt), &rc, DT_LEFT);
#endif
  ReleaseDC(hWnd, hdc);
}

LRESULT CALLBACK YUVWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message) 
    {
    case WM_COMMAND:
        wmId = LOWORD(wParam); 
        wmEvent = HIWORD(wParam); 
        // Parse the menu selections:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        OnDraw(hWnd, hdc);
        EndPaint(hWnd, &ps);
        break;
    case WM_MOUSEMOVE:
        OnMouseMove(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); 
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_CLOSE: // close window operation is not allowed
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


// Create a thread to keep window alive
DWORD WINAPI MsgLoop(void *param)
{
	YUVDisplayer *dis = (YUVDisplayer *)param;
	// get application instance handle
	dis->m_hInstance = GetModuleHandle(NULL);
#if defined(TGT_OS_WIN32)
	// register window class
    WNDCLASSEX wcx; 
 
    wcx.cbSize = sizeof(wcx);          // size of structure 
    wcx.style = CS_HREDRAW | CS_VREDRAW;                    // redraw if size changes 
    wcx.lpfnWndProc = YUVWndProc;     // points to window procedure 
    wcx.cbClsExtra = 0;                // no extra class memory 
    wcx.cbWndExtra = 0;                // no extra window memory 
    wcx.hInstance = dis->m_hInstance;         // handle to instance 
    wcx.hIcon = LoadIcon(NULL,IDI_APPLICATION);       // predefined app. icon 
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);        // predefined arrow 
    wcx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);  // black background brush 
    wcx.lpszMenuName =  NULL;    // name of menu resource 
    wcx.lpszClassName = TEXT("YUVWClass");  // name of window class 
    wcx.hIconSm = NULL; 
 
    RegisterClassEx(&wcx); 
#else // TGT_OS_WINCE
	WNDCLASS wcx;
 
	wcx.style = CS_HREDRAW | CS_VREDRAW;                    // redraw if size changes 
    wcx.lpfnWndProc = YUVWndProc;     // points to window procedure 
    wcx.cbClsExtra = 0;                // no extra class memory 
    wcx.cbWndExtra = 0;                // no extra window memory 
    wcx.hInstance = dis->m_hInstance;         // handle to instance 
    wcx.hIcon = NULL;       // predefined app. icon 
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);        // predefined arrow 
    wcx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);  // black background brush 
    wcx.lpszMenuName =  NULL;    // name of menu resource 
    wcx.lpszClassName = TEXT("YUVWClass");  // name of window class 
    RegisterClass(&wcx); 
#endif

    pContext = dis; // data to show for this window

    info_box_y = dis->m_nHeight; // divide the window to picture area and information area

	 // create window
	dis->m_hWnd = CreateWindow(
		TEXT("YUVWClass"),      // registered class name
		TEXT("Freescale video viewer"),     // window name
		WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX, // top-level window
		CW_USEDEFAULT,       // default horizontal position 
		CW_USEDEFAULT,       // default vertical position
		dis->m_in_width,       // default width
		dis->m_in_height+60,   // extra height to show some information
		(HWND) NULL,         // no owner window
		(HMENU) NULL,        // use class menu
		dis->m_hInstance,           // handle to application instance
		(LPVOID) NULL);      // no window-creation data
	if(dis->m_hWnd == NULL)
	{
		// Set event
		SetEvent(dis->m_hEvent);
		return -1;
	}

	// show
	ShowWindow(dis->m_hWnd, SW_SHOW);
	UpdateWindow(dis->m_hWnd);
	
	MSG msg;
	while(GetMessage(&msg, (HWND)NULL, 0, 0) != 0) 
    {
        // not window message, handle directly. zhenyong
        if(msg.message == WM_THREAD_QUIT)
            break;
        TranslateMessage(&msg); 
        DispatchMessage(&msg); 
    }

	// WM_QUIT

	DestroyWindow(dis->m_hWnd);
	dis->m_hWnd = NULL;

    pContext = NULL;

	// Set event
	SetEvent(dis->m_hEvent);
	return 0;
}



//===============================================================================

YUVDisplayer::YUVDisplayer() : m_hWnd(NULL), m_pDisplayBuffer(NULL), m_hInstance(NULL)
{
    _ThreadID = 0;
    _hThread = NULL;
	m_hEvent = NULL;
    m_hVideo = NULL;
}

YUVDisplayer::~YUVDisplayer()
{
	Cleanup();
}

void YUVDisplayer::Cleanup()
{
    if(!m_bInited)
        return;

	// if thread is alive, send message to close
	if(_hThread)
	{
		PostThreadMessage(_ThreadID, WM_THREAD_QUIT, 1, 0); // exit code is 1
	}

	// wait until thread closed
	WaitForSingleObject(m_hEvent, INFINITE);
//		MsgWaitForMultipleObjectsEx (1, &_hThread , &interval,  );

	// continue cleanup
	CloseHandle ( _hThread );
	_hThread = NULL;
	_ThreadID = 0;
	CloseHandle(m_hEvent);
	m_hEvent = NULL;

	m_hWnd = NULL;

    if(m_hVideo)
    {
        DeleteObject(m_hVideo);
        m_hVideo = NULL;
    }

    baseYUVDisplayer::Cleanup();
}

int YUVDisplayer::OpenDisplayer(int width,
            int height,
            int crop_left/*=0*/,
            int crop_top/*=0*/,
            int crop_right/*=0*/,
            int crop_bottom/*=0*/,
            void *p/*=0*/)
{
    if(baseYUVDisplayer::OpenDisplayer(width,height,crop_left,crop_top,crop_right,crop_bottom,p) == -1)
        return -1;

    // Create new bmp
    HDC hdc;
    hdc = ::GetDC(NULL);
    BITMAPINFO bmih;
    bmih.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmih.bmiHeader.biWidth = m_in_width;
    bmih.bmiHeader.biHeight = -m_in_height; // top-down
    bmih.bmiHeader.biPlanes = 1;
    bmih.bmiHeader.biBitCount = 32;
    bmih.bmiHeader.biCompression = BI_RGB;
    bmih.bmiHeader.biSizeImage = 0;
    bmih.bmiHeader.biXPelsPerMeter = 0;
    bmih.bmiHeader.biYPelsPerMeter = 0;
    bmih.bmiHeader.biClrUsed = 0;
    bmih.bmiHeader.biClrImportant = 0;
    m_hVideo = CreateDIBSection(hdc,
                    &bmih,
                    DIB_RGB_COLORS,
                    (void **)&m_pDisplayBuffer,
                    NULL,
                    0);
    ::ReleaseDC(NULL, hdc);

    if(m_hVideo == NULL)
    {
		Cleanup();
		return -1;
    }

	m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	// create a thread to manage the window messages
	_hThread = CreateThread(NULL, 0, MsgLoop, this, CREATE_SUSPENDED, &_ThreadID);
	if(_ThreadID == NULL)
	{
		Cleanup();
		return -1;
	}

	// improve priority for display
	SetThreadPriority(_hThread, THREAD_PRIORITY_ABOVE_NORMAL);

	// resume
	ResumeThread(_hThread);

	// wait 2 seconds for window display
//	Sleep(2000);
	do
	{
		int ws = WaitForSingleObject(m_hEvent, 200);
		if(ws != WAIT_TIMEOUT || (IsWindow((volatile HWND)m_hWnd) && IsWindowVisible((volatile HWND)m_hWnd))) // thread exit or error or window opened
			break;
	}while(1);

	return 0;
}
/* To display a yuv picture
 * Parameters
 * y,cb,cr [in] YUV buffers for the picture to display, including extension and cropping part
 *
 */
int YUVDisplayer::DisplayYUVPicture(const unsigned char *y,
						  const unsigned char *cb,
						  const unsigned char *cr,
						  int ystride,
						  int cstride)
{
    if(!IsWindow(m_hWnd))
        return -1;
	ASSERT(m_pDisplayBuffer != NULL);
    // offset y, cb and cr
    // fill black color for cropped part
    int top = (m_vb_ext_top+m_crop_top);
    int left = (m_vb_ext_left+m_crop_left);
    y += top*ystride+left;
    top >>= 1;
    left >>= 1;
    cb += top*cstride+left;
    cr += top*cstride+left;
	YUVToRGB32((unsigned char *)m_pDisplayBuffer,
        m_in_width*4,
        y,
        cb,
        cr,
        ystride,
        cstride,
        0,
        m_in_width,
        m_in_height,
        7);
	// post thread message or update target window directly
	// Seems thead message for WM_PAINT not work
	InvalidateRect(m_hWnd, NULL, FALSE);

	return 0;
}

#elif defined(TGT_OS_ELINUX)
// elinux
#define linux
#include "render_lcd.h"

YUVDisplayer::YUVDisplayer()
{
}

YUVDisplayer::~YUVDisplayer()
{
	Cleanup();
}

void YUVDisplayer::Cleanup()
{
    deinit_lcd_video();
    baseYUVDisplayer::Cleanup();
}

/* To open display
 * Parameters
 * width   [in] Frame width
 * height  [in] Frame height
 * crop_left, crop_top, crop_right, crop_bottom
 *         [in] Cropped part of display buffer
 * Remarks
 *   LCD mallocs frame buffer with size of(video_buffer_width,video_buffer_height),
 *   not (width,height). The cropping is applied on visible part (widthxheight), not
 *   frame buffer.
 *   Call SetVideoBufferBoundary() first to notify the video buffer boundary size
 */
int YUVDisplayer::OpenDisplayer(int width,
            int height,
            int crop_left/*=0*/,
            int crop_top/*=0*/,
            int crop_right/*=0*/,
            int crop_bottom/*=0*/,
            void *p/*=0*/)
{
    if(baseYUVDisplayer::OpenDisplayer(width,height,crop_left,crop_top,crop_right,crop_bottom,p) == -1)
      return -1;
    return init_lcd_video(FORMAT_4_2_0,
         m_nWidth+m_vb_ext_left+m_vb_ext_right,
         m_nHeight+m_vb_ext_top+m_vb_ext_bot,
         m_vb_ext_left+m_crop_left,
         m_vb_ext_top+m_crop_top,
         m_in_width,
         m_in_height, 1);
    }
/* To display a yuv picture
 * Parameters
 * y,cb,cr [in] YUV buffers for the picture to display, including extension and cropping part
 *
 */
int YUVDisplayer::DisplayYUVPicture(const unsigned char *y,
						  const unsigned char *cb,
						  const unsigned char *cr,
						  int ystride,
						  int cstride)
{
    // source yuv picture is of same size of frame buffer
    ASSERT(ystride == (m_in_width+m_vb_ext_left+m_crop_left+m_vb_ext_right+m_crop_right));
    output_frame_ex(y,
         cb,
         cr,
         ystride,
         cstride,
         m_in_width,
         m_in_height,
         m_vb_ext_left+m_crop_left,
         m_vb_ext_top+m_crop_top,
         m_vb_ext_right+m_crop_right,
         m_vb_ext_bot+m_crop_bottom);
	return 0;
}

#elif defined (TGT_OS_UNIX) && defined(ENABLE_X11)

//#include "macros.h"
#include "ColorConversion_Safe.h"
#include <malloc.h>
//===============================================================================
// Display window class definition

//
// FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
// PURPOSE: Processes messages for the main window.
//
// WM_COMMAND - process the application menu
// WM_PAINT - Paint the main window
// WM_DESTROY - post a quit message and return
//
//
static YUVDisplayer *pContext = NULL;
void OnDraw(Window hWnd, GC hDC)
{
    if(pContext == NULL || pContext->m_hVideo == NULL || hWnd != pContext->m_hWnd)
        return;

    XPutImage(pContext->display,
        pContext->m_hWnd,
        hDC,
        pContext->m_hVideo,
        0,
        0,
        0,
        0,
        pContext->m_in_width,
        pContext->m_in_height); 
    XFlush(pContext->display);
}

#define WM_THREAD_QUIT (WM_USER+500)
//#include <Windowsx.h>
#include <stdio.h>

static int info_box_x = 0;
static int info_box_y = 0;

// UNICODE version
void OnMouseMove(Window hWnd, int xpos, int ypos)
{
/*
  // display information
  HDC hdc = GetDC(hWnd);
  TCHAR txt[100];
  wsprintf(txt, TEXT("x=%04d,y=%04d, mb_x=%03d,mb_y=%03d"), xpos, ypos, xpos>>4, ypos>>4);
  TextOut(hdc, info_box_x, info_box_y, txt, wcslen(txt));
  ReleaseDC(hWnd, hdc);
*/
}

int YUVWndProc(YUVDisplayer *dis, XEvent *event, int wParam, int lParam)
{
  GC gc;
  switch(event->type)
  {
  case Expose:
    gc = DefaultGC(dis->display, dis->screen_num);
    OnDraw(dis->m_hWnd, gc);
    break;
  default:
    break;
  }

  return 0;

/*
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message) 
    {
    case WM_COMMAND:
        wmId = LOWORD(wParam); 
        wmEvent = HIWORD(wParam); 
        // Parse the menu selections:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        OnDraw(hWnd, hdc);
        EndPaint(hWnd, &ps);
        break;
    case WM_MOUSEMOVE:
        OnMouseMove(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); 
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_CLOSE: // close window operation is not allowed
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }*/
    return 0;
}


// Create a thread to keep window alive
//static  Atom MyAtom;

void *MsgLoop(void *param)
{
	YUVDisplayer *dis = (YUVDisplayer *)param;

    pContext = dis; // data to show for this window

    info_box_y = dis->m_nHeight; // divide the window to picture area and information area

	 // create window
    dis->m_hWnd = XCreateSimpleWindow(
        dis->display,
        RootWindow(dis->display, dis->screen_num),
        0,
        0,
        dis->m_in_width,
        dis->m_in_height+60,
        0,
        BlackPixel(dis->display, dis->screen_num),
        BlackPixel(dis->display, dis->screen_num));

	if(dis->m_hWnd == None)
	{
		// Set event
	    sem_post(&dis->m_hEvent);
		return 0;
	}

    XSelectInput(dis->display,
        dis->m_hWnd,
        ExposureMask|StructureNotifyMask|KeyPressMask);

	// show
    XMapWindow(dis->display, dis->m_hWnd); 
    XFlush(dis->display);


    for (;;)
    {
      XEvent ev;

      XNextEvent(dis->display, &ev);
#if 1
      if(ev.type == DestroyNotify)
      {
        break;
      }
#else
      if(ev.type == ClientMessage)
      {
        XClientMessageEvent *evt;
        evt = (XClientMessageEvent *)&ev;
        // Is it my custom message?
        if (evt->message_type == MyAtom)
        {
           break;
        }
        // Here I would do something about my custom message
      }
#endif
      // merge other Expose events
      if(ev.type == Expose)
      {
        int remain = XPending(dis->display);
        while(remain > 0)
        {
          XEvent nev;
          XNextEvent(dis->display, &nev);
          if(nev.type != Expose)
          {
            XPutBackEvent(dis->display, &nev);
            break;
          }
        }
      }
      if(YUVWndProc(dis, &ev, 0, 0) == -1)
        break;
    } 

	// WM_QUIT

	XDestroyWindow(dis->display, dis->m_hWnd);
	dis->m_hWnd = None;
    pContext = NULL;

	// Set event
	sem_post(&dis->m_hEvent);
	return 0;
}



//===============================================================================

YUVDisplayer::YUVDisplayer() : m_hWnd(None), m_pDisplayBuffer(NULL)
{
    display = NULL;
    m_hVideo = NULL;
}

YUVDisplayer::~YUVDisplayer()
{
	Cleanup();
}

void YUVDisplayer::Cleanup()
{
    if(!m_bInited)
        return;

    if(m_hWnd != None)
    {
#if 1
      XDestroyWindowEvent event;
      event.type = DestroyNotify;
      event.serial = 0;
      event.send_event = 1;
      event.display = display;
      event.event = m_hWnd;
      event.window = m_hWnd;
      XSendEvent(display, event.window, 0, StructureNotifyMask,
                  (XEvent *)&event);
//      XFlush(display);
//      XSync(display, True);
#else
      // Following code is not supported for multi-threaded case!!!

      XClientMessageEvent xevent;
      // Get an Atom to use for my custom message. Arbitrarily name the atom
      // "MyAtom"
      MyAtom = XInternAtom(display, "MyAtom", 0);
      xevent.type = ClientMessage;
      // Use the Atom we got for our custom message
      xevent.message_type = MyAtom;
      
      // I don't really use these fields, but here's an example
      // of setting them
      xevent.format = 32;
      xevent.data.l[0] = 0;
      
      // Send the ClientMessage event
      XSendEvent(display, m_hWnd, 0, 0, (XEvent *)&xevent);
#endif
    }

	// wait until thread closed
    sem_wait(&m_hEvent);

	// continue cleanup
    pthread_join(_ThreadID, NULL);

    sem_destroy(&m_hEvent);


    if(display)
    {
      XCloseDisplay(display); 
      display = NULL;
    }

    if(m_hVideo)
    {
      XDestroyImage(m_hVideo);
      m_hVideo = NULL;
    }

    baseYUVDisplayer::Cleanup();
}

int YUVDisplayer::OpenDisplayer(int width,
            int height,
            int crop_left/*=0*/,
            int crop_top/*=0*/,
            int crop_right/*=0*/,
            int crop_bottom/*=0*/,
            void *p/*=0*/)
{
    Visual *visual;
	int depth;
	int pad;

    if(baseYUVDisplayer::OpenDisplayer(width,height,crop_left,crop_top,crop_right,crop_bottom,p) == -1)
        return -1;

    XVisualInfo visual_info;

    display = XOpenDisplay(0);
    screen_num = DefaultScreen(display);
    depth = DisplayPlanes(display, screen_num);
    XMatchVisualInfo(display, screen_num, depth, TrueColor, &visual_info);
    visual = visual_info.visual;

    // Create new bmp
    if(depth != 24 && depth != 32)
    {
      printf("Bit depth %d is not support!\n", depth);
      Cleanup();
      return -1;
    }

    // assert bit depth is 24 or 32
    ASSERT(depth == 24 || depth == 32);
    pad = 32;

    m_pDisplayBuffer = (char *)malloc(m_in_width * m_in_height * 4);
    if(m_pDisplayBuffer == 0)
    {
      printf("Error malloc.\n");
      Cleanup();
      return -1;
    }
    m_hVideo = XCreateImage(display, visual, depth, ZPixmap, 0, m_pDisplayBuffer, m_in_width, m_in_height, pad, 0);
    if(m_hVideo == 0)
    {
      printf("Error XCreateImage.\n");
      Cleanup();
      return -1;
    }

    sem_init(&m_hEvent, 0, 0);

    pthread_create(&_ThreadID, NULL, MsgLoop, this);

	// wait 2 seconds for window display
//	Sleep(2000);
	do
	{
	    struct timespec ts;
        ts.tv_sec = time(NULL)+1;
		int ws = sem_timedwait(&m_hEvent, &ts);
		if(ws != -1 || (m_hWnd != None)) // thread exit or error or window opened
			break;
	}while(1);

	return 0;
}

int YUVDisplayer::DisplayYUVPicture(const unsigned char *y,
						  const unsigned char *cb,
						  const unsigned char *cr,
						  int ystride,
						  int cstride)
{
    if(m_hWnd == None)
        return -1;
	ASSERT(m_pDisplayBuffer != NULL);
    // offset y, cb and cr
    // fill black color for cropped part
    int top = (m_vb_ext_top+m_crop_top);
    int left = (m_vb_ext_left+m_crop_left);
    y += top*ystride+left;
    top >>= 1;
    left >>= 1;
    cb += top*cstride+left;
    cr += top*cstride+left;
	YUVToRGB32((unsigned char *)m_pDisplayBuffer,
        m_in_width*4,
        y,
        cb,
        cr,
        ystride,
        cstride,
        0,
        m_in_width,
        m_in_height,
        7);
	// post thread message or update target window directly
	// Seems thead message for WM_PAINT not work
//    GC gc = DefaultGC(display, screen_num); 
//    XPutImage(display, m_hWnd, gc, m_hVideo, 0, 0, 0, 0, m_nWidth, m_nHeight); 
//    XFlush(display);
    XExposeEvent event;
    event.type = Expose;
    event.serial = 0;
    event.send_event = 1;
    event.display = display;
    event.window = m_hWnd;
    event.x = 0;
    event.y = 0;
    event.width = m_in_width;
    event.height = m_in_height;
    event.count = 0;
    XSendEvent(display, m_hWnd, 1, ExposureMask, (XEvent *)&event);
//    XFlush(display);
	return 0;
}

#else // dummy displayer
YUVDisplayer::YUVDisplayer()
{
}

YUVDisplayer::~YUVDisplayer()
{
}

#endif

