/*
***********************************************************************
* Copyright 2005-2010 by Freescale Semiconductor, Inc.
* All modifications are confidential and proprietary information
* of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
***********************************************************************
*/

#ifndef __RENDER_LCD_H__
#define __RENDER_LCD_H__


//#define __WINCE


typedef enum _CHROMA_FORMAT
{
    FORMAT_RESERVED_CHROMA=0,
    FORMAT_4_2_0 ,
    FORMAT_4_2_2 ,
    FORMAT_4_4_4 
} CHROMA_FORMAT; /* Y Cb Cr sample format of a video stream -- in mpeg4 & divx , 0x01 for 4:2:0, 0x10 for 4:2:2, 0x10 for 4:4:4 */

#if 1//def __WINCE

int init_lcd_video(CHROMA_FORMAT fmt, int width, int height,int left,int top,int in_width,int in_height, int gso);
int output_frame(void *frm, int width, int height);
int deinit_lcd_video();

#else

int v4l2_open(void);
int v4l2_close(void);

int v4l2_set_format_alloc_frames(int width, int height, int count);
/*ENGR00035098    clean H264 decoder API  start*/
//int ShareBuffersWithV4L2(ShareFrames *pShareFrames);
/*ENGR00035098    clean H264 decoder API  end*/
void *v4l2_get_buffer(void);

/*
To output a frame to display
Parameters:
    frm : [out] virtual address for this frame
Return value:
    0 for success, -1 for failure.
 */
int output_frame(void *vaddr, int directrender);

#define VIDIOC_QBUF_DIRECT   _IOW  ('V', 35, int) //zhenyong, display buffer right now !
#define VIDIOC_SETBUFS  _IOWR ('V', 16, ShareFrames) //zhenyong

#endif

#endif /* __RENDER_LCD_H__ */

