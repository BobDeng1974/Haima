/*
 *  Copyright (c) 2005-2013, Freescale Semiconductor Inc.,
 *  All Rights Reserved.
 *
 *  The following programs are the sole property of Freescale Semiconductor Inc.,
 *  and contain its proprietary and confidential information.
 *
 */

#ifdef FHW
typedef enum _CHROMA_FORMAT
{
	FORMAT_RESERVED_CHROMA=0,
	FORMAT_4_2_0 ,
    	FORMAT_4_2_2 ,
    	FORMAT_4_4_4 
} CHROMA_FORMAT; /* Y Cb Cr sample format of a video stream -- in mpeg4 & divx , 0x01 for 4:2:0, 0x10 for 4:2:2, 0x10 for 4:4:4 */

int init_lcd_video(CHROMA_FORMAT fmt, int width, int height,int left,int top,int in_width,int in_height, int gso)
{
    return 0;//ok
}

int deinit_lcd_video()
{
    return 0;
}

int output_frame(void *frm, int width, int height)
{
    return 0;
}

#else
#ifdef linux

#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/compiler.h>
#include <linux/videodev.h>
#include <sys/mman.h>
#include <linux/videodev.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/time.h>
#include <malloc.h>
#endif //__arm

#ifdef WINCE	//1eagle
#define inline __inline
#define NULL	0
#include "malloc.h"
#include "windows.h"
#include "Winbase.h"

#define SCREEN_LEFT		0
#define SCREEN_TOP		0
#define PAD_SIZE	0
#define SCREEN_WIDTH 440//220//240//176	//352
#define SCREEN_HEIGHT 360//180//196//144	//288
#define FRAME_INTERVAL	33	// 30 frame/seconds
HWND g_hwndDis;
HDC g_hdc;
BITMAPINFO g_bmih;
HBITMAP g_m_hVideo;
//HDC g_hBmpDC;
//HBITMAP g_hbmpold;
BYTE * g_rgbbuf;
#endif

#ifdef RVDS
#define inline __inline
#define NULL 0
#endif

extern int g_last_width;
extern int g_last_height;

#define MMF_STRLEN(str) strlen(str)
#define MMF_MALLOC(size) malloc(size)
#define MMF_FREE(ptr) free(ptr)
#define MMF_MEMCPY(dst, src, size) memcpy((dst), (src), (size))

inline static char *
MMF_STRDUP (const char *str)
{
  	char *new_str;
  	unsigned int length;

 	 if(str){
	 	
      		length = MMF_STRLEN(str)+1; //strlen not including the terminating NULL
      		new_str = (char *)MMF_MALLOC (length);
		if(new_str)
      			MMF_MEMCPY(new_str, str, length);  
    	}
	 else
    		new_str = NULL;
  	return new_str;
}

typedef enum _CHROMA_FORMAT
{
	FORMAT_RESERVED_CHROMA=0,
	FORMAT_4_2_0 ,
    	FORMAT_4_2_2 ,
    	FORMAT_4_4_4 
} CHROMA_FORMAT; /* Y Cb Cr sample format of a video stream -- in mpeg4 & divx , 0x01 for 4:2:0, 0x10 for 4:2:2, 0x10 for 4:4:4 */

typedef enum
{
    MMF_FAILURE = -1,
    MMF_SUCCESS = 0,
    MMF_ERR_BAD_PARAM,      /* wrong parameters */
    MMF_ERR_NOT_SUPPORT,    /* feature not supported */
    MMF_ERR_DEVICE_UNAVAILABLE,    /* card is removed or deactived */
    MMF_ERR_CANCELLED,      /* operation is cancelled by user */
    MMF_ERR_TIMEOUT         /* timeout when transfer data */
} MMF_ERROR;

#define VO_V4L2_BUFFER_COUNT    21

typedef unsigned long MMF_UINT32;
typedef unsigned long long MMF_UINT64;

typedef struct
{
    MMF_UINT32 offset;
    char *address;
    int length;
    int flag;
    MMF_UINT32 crop_offset;	//eagle : for dut cropping
}image_buf;


typedef struct
{
    char *device_name;

    int fd_v4l2;
    image_buf buffers[VO_V4L2_BUFFER_COUNT];
    MMF_UINT64 buf_count;
    MMF_UINT64 vd_count;

    int inited;
}mmf_video_out_v4l2;

static char gso_video_device[] = {"/dev/v4l/video16"};
static char dto_video_device[] = {"/dev/video16"};
#ifdef FSL_MX27	
int g_output = 0;
#define FIX_MX27_LIMITATION
#else
int g_output = 3; // MX31
#endif

// video device
static mmf_video_out_v4l2 vo;
static int video_size = 0;
static int last_id = 0;

#if 0
int fb_setup(void)
{
        char fbdev[] = "/dev/fb0";
        struct fb_var_screeninfo fb_var;
        struct fb_fix_screeninfo fb_fix;
        struct mxcfb_color_key color_key;
        struct mxcfb_gbl_alpha alpha;
        int retval = -1;
        int fd_fb;
        unsigned short * fb0;
        __u32 screen_size;
        int h, w;

        if ((fd_fb = open(fbdev, O_RDWR, 0)) < 0)
        {
                printf("Unable to open %s\n", fbdev);
                retval = -1;
                goto err0;
        }

        if ( ioctl(fd_fb, FBIOGET_VSCREENINFO, &fb_var) < 0) {
                goto err1;
        }
        if (g_display_height == 0)
                g_display_height = fb_var.yres;

        if (g_display_width == 0)
                g_display_width = fb_var.xres;

        alpha.alpha = 255;
        alpha.enable = 1;
        if ( ioctl(fd_fb, MXCFB_SET_GBL_ALPHA, &alpha) < 0) {
                retval = 0;
                goto err1;
        }

        color_key.color_key = 0x00080808;
        color_key.enable = 1;
        if ( ioctl(fd_fb, MXCFB_SET_CLR_KEY, &color_key) < 0) {
                retval = 0;
                goto err1;
        }

        if ( ioctl(fd_fb, FBIOGET_FSCREENINFO, &fb_fix) < 0) {
                goto err1;
        }
        screen_size = fb_var.xres * fb_var.yres * fb_var.bits_per_pixel / 8;

        /* Map the device to memory*/
        fb0 = (unsigned short *)mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);
        if ((int)fb0 == -1)
        {
                printf("\nError: failed to map framebuffer device 0 to memory.\n");
                retval = -1;
                goto err1;
        }


        if (fb_var.bits_per_pixel == 16) {
                for (h = 0; h < g_display_height; h++) {
                        for (w = 0; w < g_display_width; w++) {
                                fb0[(h*fb_var.xres) + w] = 0x0841;
                        }
                }
        }
        else if (fb_var.bits_per_pixel == 24) {
                for (h = 0; h < g_display_height; h++) {
                        for (w = 0; w < g_display_width; w++) {
                                unsigned char * addr = (unsigned char *)fb0 + ((h*fb_var.xres) + w) * 3;
                                *addr++ = 8;
                                *addr++ = 8;
                                *addr++ = 8;
                        }
                }
        }
        else if (fb_var.bits_per_pixel == 32) {
                for (h = 0; h < g_display_height; h++) {
                        for (w = 0; w < g_display_width; w++) {
                                unsigned char * addr = (unsigned char *)fb0 + ((h*fb_var.xres) + w) * 4;
                                *addr++ = 8;
                                *addr++ = 8;
                                *addr++ = 8;
                        }
                }
        }

        retval = 0;
        munmap(fb0, screen_size);
err1:
        close(fd_fb);
err0:
        return retval;
}
#endif

#ifdef WINCE		//1eagle
typedef struct
{
		LARGE_INTEGER frequency;
		LARGE_INTEGER startCount;
		LARGE_INTEGER endCount;
		LARGE_INTEGER interCount;
		int sleep;	// mseconds
}ClockSync_t;

static ClockSync_t clocksync;

#define RESET_SYNC_CLOCK         \
{                           \
	clocksync.startCount.QuadPart = 0;\
	clocksync.endCount.QuadPart   = 0;\
	clocksync.interCount.QuadPart   = 0;\
	QueryPerformanceFrequency(&clocksync.frequency); \
}

#define START_SYNC_CLOCK	\
{								\
	QueryPerformanceFrequency(&clocksync.frequency); \
	QueryPerformanceCounter(&clocksync.startCount);	\
}

#define STOP_SYNC_CLOCK	\
{	\
	QueryPerformanceCounter(&clocksync.endCount);	\
	if(clocksync.frequency.QuadPart!=0) \
	{ \
       	clocksync.interCount.QuadPart =((clocksync.endCount.QuadPart - clocksync.startCount.QuadPart)*1000000/clocksync.frequency.QuadPart);	\
	      clocksync.sleep=FRAME_INTERVAL-(clocksync.interCount.QuadPart/1000); \
        	if(clocksync.sleep>0) \
	     {	\
		   /*printf("will sleep : (%d) ms \n",clocksync.sleep);*/ \
		   Sleep(clocksync.sleep); \
	      } \
	} \
	else \
	{ \
		/*printf("frequency==0 \r\n");*/ \
	}\
}


#define CLIP_16_235(X) (((X) > 235) ? 235 : ((X) < 16) ? 16 : (X))
#define CLIP_16_240(X) (((X) > 240) ? 240 : ((X) < 16) ? 16 : (X))
 
struct lookuptable
{
    int m_plY[256];
    int m_plRV[256];
    int m_plGV[256];
    int m_plGU[256];
    int m_plBU[256];
    unsigned char clip[768];
};
 
static struct lookuptable lut;
 
/* Setup lookup table
 */
void InitYUV2RGBConversion(void)
{
    int i;
 
    // clipper
    for(i=-256; i<512; i++)
        lut.clip[i+256] = (unsigned char)((i<0)?0:(i>255)?255:i);
 
    for(i=0; i<256; i++)
    {
        // Y
        if(i >= 16)
        {
            if(i > 235)
                lut.m_plY[i] = lut.m_plY[235];
            else
                lut.m_plY[i] = (int)((299.3*(i-16))/256);
        }
        else
        {
            lut.m_plY[i] = 0;
        }
        //UV
        if((i >= 16) && (i <= 240))
        {
            lut.m_plRV[i] = (int)((410.1 *(i-128))/256 + 0.5);
            lut.m_plGV[i] = (int)((-208.9*(i-128))/256 + 0.5);
            lut.m_plGU[i] = (int)((-100.5*(i-128))/256 + 0.5);
            lut.m_plBU[i] = (int)((518.5 *(i-128))/256 + 0.5);
        }
        else if(i < 16)
        {
            lut.m_plRV[i] = (int)((410.1 *(16-128))/256 + 0.5);
            lut.m_plGV[i] = (int)((-208.9*(16-128))/256 + 0.5);
            lut.m_plGU[i] = (int)((-100.5*(16-128))/256 + 0.5);
            lut.m_plBU[i] = (int)((518.5 *(16-128))/256 + 0.5);
        }
        else
        {
            lut.m_plRV[i] = lut.m_plRV[240];
            lut.m_plGV[i] = lut.m_plGV[240];
            lut.m_plGU[i] = lut.m_plGU[240];
            lut.m_plBU[i] = lut.m_plBU[240];
        }
    }
}
 
/******************************************************************************
 * Functions to convert YUV to RGB
 *
 * To convert YUV420, YUV422 or YUV444 to RGB.
 *
 * Return value
 *     None
 *
 * Parameters
 *     puc_y, puc_u, puc_v - y, u, v buffers.
 *     stride_y, stride_uv - stride of luma and chroma buffers.
 *     puc_out             - RGB buffer.
 *     _stride_out         - stride of RGB buffer.
 *     width_y, height_y   - width and height of luma.
 *     mask_clrchannel     - indicator or y, u or v buffer.
 *                           Bit 2 - y
 *                           Bit 1 - u
 *                           Bit 0 - v
 *                           Value '1' stands for display the corresponding channel,
 *                           while value '0' disables the channel.
 */
#define ZERO_Y 16
#define ZERO_U 128
#define ZERO_V 128
#define FORMRGB(alpha,r,g,b) ((alpha)<<24) | ((r)<<16) | ((g)<<8) | (b)
#define CLIPCOLOR(v) lut.clip[(v)+256]
 
/* all stride values are in _pixels_ */
static void YUV420toRGB32_fast(const unsigned char *puc_y, int stride_y, 
                const unsigned char *puc_u, const unsigned char *puc_v, int stride_uv, 
                unsigned char *puc_out, int width_y, int height_y,
                unsigned int _stride_out,
                unsigned int mask_clrchannel) 
{
 
    int x, y;
    int stride_diff = 8 * _stride_out - 4 * width_y;
 
    if (height_y < 0) 
    {
        /* we are flipping our output upside-down */
        height_y  = -height_y;
        puc_y    += (height_y   - 1) * stride_y ;
        puc_u    += (height_y/2 - 1) * stride_uv;
        puc_v    += (height_y/2 - 1) * stride_uv;
        stride_y  = -stride_y;
        stride_uv = -stride_uv;
    }
 
    for (y=0; y<height_y; y+=2) 
    {
        unsigned char* pY;
        unsigned char* pY1;
        unsigned char* pU;
        unsigned char* pV;
        unsigned int  out;
        unsigned char* pOut2;
        pY   = (unsigned char*) puc_y;
        pY1  = (unsigned char*) puc_y+stride_y;
        pU   = (unsigned char*) puc_u;
        pV   = (unsigned char*) puc_v;
        pOut2= (unsigned char*) puc_out+4*_stride_out;
 
        for (x=0; x<width_y; x+=2)
        {
            int R, G, B;
            int Y;
 
            unsigned char val;
            int ival;
 
            if(mask_clrchannel & 1)
                ival = *pV;
            else
                ival = ZERO_V;
 
            val = (unsigned char)CLIP_16_240(ival);
            R = lut.m_plRV[val];
            G = lut.m_plGV[val];
            pV++;
 
            if(mask_clrchannel & 2)
                ival = *pU;
            else
                ival = ZERO_U;
 
            val = (unsigned char)CLIP_16_240(ival);
            G += lut.m_plGU[val];
            B  = lut.m_plBU[val];
            pU++;
 
            // top-left pixel
            if(mask_clrchannel & 4)
                Y = *pY;
            else
                Y = ZERO_Y;
 
            Y = CLIP_16_235(Y);
            Y = lut.m_plY[Y];
            pY++;
            out = FORMRGB(0xFF, CLIPCOLOR(R+Y), CLIPCOLOR(G+Y), CLIPCOLOR(B+Y));
            *(unsigned int *)puc_out = out;
            puc_out += 4;
 
            // top-right pixel
            if(mask_clrchannel & 4)
                Y = *pY;
            else
                Y = ZERO_Y;
 
            Y = CLIP_16_235(Y);
            Y = lut.m_plY[Y];
            pY++;
            out = FORMRGB(0xFF, CLIPCOLOR(R+Y), CLIPCOLOR(G+Y), CLIPCOLOR(B+Y));
            *(unsigned int *)puc_out = out;
            puc_out += 4;
 
            // bottom-left
            if(mask_clrchannel & 4)
                Y = *pY1;
            else
                Y = ZERO_Y;
 
            Y = CLIP_16_235(Y);
            Y = lut.m_plY[Y];
            pY1++;
            out = FORMRGB(0xFF, CLIPCOLOR(R+Y), CLIPCOLOR(G+Y), CLIPCOLOR(B+Y));
            *(unsigned int *)pOut2 = out;
            pOut2 += 4;
 
            // bottom-right
            if(mask_clrchannel & 4)
                Y = *pY1;
            else
                Y = ZERO_Y;
 
            Y = CLIP_16_235(Y);
            Y = lut.m_plY[Y];
            pY1++;
            out = FORMRGB(0xFF, CLIPCOLOR(R+Y), CLIPCOLOR(G+Y), CLIPCOLOR(B+Y));
            *(unsigned int *)pOut2 = out;
            pOut2 += 4;
        }
 
        puc_y   += 2*stride_y;
        puc_u   += stride_uv;
        puc_v   += stride_uv;
        puc_out += stride_diff;
    }
}
 
static void YUV422toRGB32_fast(const unsigned char *puc_y, int stride_y, 
                const unsigned char *puc_u, const unsigned char *puc_v, int stride_uv, 
                unsigned char *puc_out, int width_y, int height_y,
                unsigned int _stride_out,
                unsigned int mask_clrchannel) 
{
 
    int x, y;
    int stride_diff = 4 * _stride_out - 4 * width_y;
 
    if (height_y < 0)
    {
        /* we are flipping our output upside-down */
        height_y  = -height_y;
        puc_y    += (height_y   - 1) * stride_y ;
        puc_u    += (height_y/2 - 1) * stride_uv;
        puc_v    += (height_y/2 - 1) * stride_uv;
        stride_y  = -stride_y;
        stride_uv = -stride_uv;
    }
 
    for (y=0; y<height_y; y++) 
    {
        unsigned char* pY;
        unsigned char* pU;
        unsigned char* pV;
        unsigned int  out;
        pY = (unsigned char*) puc_y;
        pU = (unsigned char*) puc_u;
        pV = (unsigned char*) puc_v;
 
        for (x=0; x<width_y; x+=2)
        {
            int R, G, B;
            int Y;
 
            unsigned char val;
            int ival;
 
            if(mask_clrchannel & 1)
                ival = *pV;
            else
                ival = ZERO_V;
 
            val = (unsigned char)CLIP_16_240(ival);
            R = lut.m_plRV[val];
            G = lut.m_plGV[val];
            pV++;
 
            if(mask_clrchannel & 2)
                ival = *pU;
            else
                ival = ZERO_U;
 
            val = (unsigned char)CLIP_16_240(ival);
            G += lut.m_plGU[val];
            B  = lut.m_plBU[val];
            pU++;
 
            // left pixel
            if(mask_clrchannel & 4)
                Y = *pY;
            else
                Y = ZERO_Y;
            Y = CLIP_16_235(Y);
            Y = lut.m_plY[Y];
            pY++;
            out = FORMRGB(0xFF, CLIPCOLOR(R+Y), CLIPCOLOR(G+Y), CLIPCOLOR(B+Y));
            *(unsigned int *)puc_out = out;
            puc_out += 4;
 
            // right pixel
            if(mask_clrchannel & 4)
                Y = *pY;
            else
                Y = ZERO_Y;
            Y = CLIP_16_235(Y);
            Y = lut.m_plY[Y];
            pY++;
            out = FORMRGB(0xFF, CLIPCOLOR(R+Y), CLIPCOLOR(G+Y), CLIPCOLOR(B+Y));
            *(unsigned int *)puc_out = out;
            puc_out += 4;
        }
 
        puc_y   += stride_y;
        puc_u   += stride_uv;
        puc_v   += stride_uv;
        puc_out += stride_diff;
    }
}
static void YUV444toRGB32_fast(const unsigned char *puc_y, int stride_y, 
                const unsigned char *puc_u, const unsigned char *puc_v, int stride_uv, 
                unsigned char *puc_out, int width_y, int height_y,
                unsigned int _stride_out,
                unsigned int mask_clrchannel) 
{
 
    int x, y;
    int stride_diff = 4 * _stride_out - 4 * width_y;
 
    if (height_y < 0)
    {
        /* we are flipping our output upside-down */
        height_y  = -height_y;
        puc_y    += (height_y   - 1) * stride_y ;
        puc_u    += (height_y/2 - 1) * stride_uv;
        puc_v    += (height_y/2 - 1) * stride_uv;
        stride_y  = -stride_y;
        stride_uv = -stride_uv;
    }
 
    for (y=0; y<height_y; y++) 
    {
        unsigned char* pY;
        unsigned char* pU;
        unsigned char* pV;
        unsigned int  out;
        pY = (unsigned char*) puc_y;
        pU = (unsigned char*) puc_u;
        pV = (unsigned char*) puc_v;
 
        for (x=0; x<width_y; x++)
        {
            int R, G, B;
            int Y;
 
            unsigned char val;
            int ival;
 
            if(mask_clrchannel & 1)
                ival = *pV;
            else
                ival = ZERO_V;
 
            val = (unsigned char)CLIP_16_240(ival);
            R = lut.m_plRV[val];
            G = lut.m_plGV[val];
            pV++;
 
            if(mask_clrchannel & 2)
                ival = *pU;
            else
                ival = ZERO_U;
 
            val = (unsigned char)CLIP_16_240(ival);
            G += lut.m_plGU[val];
            B  = lut.m_plBU[val];
            pU++;
 
            if(mask_clrchannel & 4)
                Y = *pY;
            else
                Y = ZERO_Y;
 
            Y = CLIP_16_235(Y);
            Y = lut.m_plY[Y];
            pY++;
            out = FORMRGB(0xFF, CLIPCOLOR(R+Y), CLIPCOLOR(G+Y), CLIPCOLOR(B+Y));
            *(unsigned int *)puc_out = out;
            puc_out += 4;
 
        }
 
        puc_y   += stride_y;
        puc_u   += stride_uv;
        puc_v   += stride_uv;
        puc_out += stride_diff;
    }
}
void ConvertYUVToRGB32(
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
    unsigned int mask_clrchannel)
{
    switch(chrom_fmt)
    {
    case 0:
        YUV420toRGB32_fast(y, stride_y, u, v, stride_uv, rgb, width, height, stride_rgb>>2, mask_clrchannel);
        break;
    case 1:
        YUV422toRGB32_fast(y, stride_y, u, v, stride_uv, rgb, width, height, stride_rgb>>2, mask_clrchannel);
        break;
    case 2:
        YUV444toRGB32_fast(y, stride_y, u, v, stride_uv, rgb, width, height, stride_rgb>>2, mask_clrchannel);
        break;
    default:
        break;
    }
}
int init_lcd_video(CHROMA_FORMAT fmt, int width, int height,int left,int top,int in_width,int in_height, int gso)
{
	WNDCLASS   wcex;   
	HANDLE hInstance =(HINSTANCE)GetModuleHandle(0);
	static int count=0;

	count++;
	g_last_width=in_width;
      g_last_height=in_height;
	width = ((unsigned int)(width+15))&0xFFFFFFF0;
	height =((unsigned int)(height+15))&0xFFFFFFF0;

	width+=PAD_SIZE;
	width+=PAD_SIZE;
	
	//wcex.cbSize                   =   sizeof(WNDCLASSEX);     
	wcex.style                     =   0;   
	wcex.lpfnWndProc         =   (WNDPROC)CallWindowProc;   
	wcex.cbClsExtra           =   0;   
	wcex.cbWndExtra           =   0;   
	wcex.hInstance             =   hInstance;//(HINSTANCE)GetModuleHandle(0);   
	wcex.hIcon                     =  NULL;// LoadIcon   (0,   IDI_APPLICATION);   
	wcex.hCursor                 =   LoadCursor   (0,IDC_ARROW);   
	wcex.hbrBackground     =   (HBRUSH)(COLOR_WINDOW+1);   
	wcex.lpszMenuName       =   0;   
	wcex.lpszClassName     =   TEXT("ce");   
	//wcex.hIconSm                 =   LoadIcon(0,   IDI_APPLICATION);   

	if   ((count==1)&&(!RegisterClass(&wcex)))   
	{   
		printf("register window class fail \n");   
		return   -1;   
	}   

	g_hwndDis =  CreateWindow(
		TEXT("ce"),
		TEXT("Display"),
		WS_VISIBLE|WS_CAPTION|WS_BORDER,
		SCREEN_LEFT, SCREEN_LEFT, SCREEN_WIDTH, SCREEN_HEIGHT,
		NULL, NULL, hInstance, NULL);

	if (!g_hwndDis)
	{
		printf("creat windows fail \n");
		return -1;
	}

	SetWindowPos(g_hwndDis, HWND_TOPMOST, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SWP_NOMOVE | SWP_NOSIZE);
	ShowWindow(g_hwndDis, SW_SHOW);

	InitYUV2RGBConversion();

	g_hdc = GetWindowDC(g_hwndDis);
	if(g_hdc == NULL)
	{
		printf("get window dc fail \n");
		DestroyWindow(g_hwndDis);
		return -1;
	}
	g_bmih.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	g_bmih.bmiHeader.biWidth = width;
	g_bmih.bmiHeader.biHeight =-height;//top-down
	g_bmih.bmiHeader.biPlanes = 1;
	g_bmih.bmiHeader.biBitCount = 32;
	g_bmih.bmiHeader.biCompression = BI_RGB;
	g_bmih.bmiHeader.biSizeImage = 0;
	g_bmih.bmiHeader.biXPelsPerMeter = 0;
	g_bmih.bmiHeader.biYPelsPerMeter = 0;
	g_bmih.bmiHeader.biClrUsed = 0;
	g_bmih.bmiHeader.biClrImportant = 0;
	g_m_hVideo = CreateDIBSection(g_hdc,
		&g_bmih,
		DIB_RGB_COLORS,
		(void **)&g_rgbbuf,
		NULL,
		0);
	if(g_m_hVideo == NULL)
	{
		printf("create dib section fail \n");
		DestroyWindow(g_hwndDis);
		return -1;
	}

	RESET_SYNC_CLOCK;

	return 0; //ok
}

int deinit_lcd_video()
{
	ReleaseDC(g_hwndDis, g_hdc);
	DeleteObject(g_m_hVideo);
	DestroyWindow(g_hwndDis);	
	return 0;
}

int input_lcd_crop(void *frm, int width, int height,int left,int top,int valid_width,int valid_height)
{
	HDC hBmpDC;
	HBITMAP hbmpold;
	unsigned char* y;
	unsigned char* u;
	unsigned char* v;
	unsigned int LumaSize;

	STOP_SYNC_CLOCK;
	
	hBmpDC = CreateCompatibleDC(g_hdc);
	if(hBmpDC == NULL)
	{
		printf("create compatible dc fail \n");
		return -1;
	}

	LumaSize =  (((unsigned short int)width+15)&0xFFFFFFF0) * ((height+15)&0xFFFFFFF0);

#if 1
	y=(unsigned char*) frm;
	u=y+LumaSize+(LumaSize>>2);
	v=y+LumaSize;

	// 4:2:0 format
	y+=top*width+left;
	u+=(top>>1)*(width>>1)+(left>>1);
	v+=(top>>1)*(width>>1)+(left>>1);	
	ConvertYUVToRGB32(g_rgbbuf+top*4*width+4*left,4*width,y,u,v,width,width>>1,0,valid_width,valid_height,0x7);
#else
	y=(unsigned char*) frm;
	u=y+LumaSize+(LumaSize>>2);
	v=y+LumaSize;

	// 4:2:0 format
	ConvertYUVToRGB32(g_rgbbuf,4*width,y,u,v,width,width>>1,0,width,height,0x7);
#endif

	hbmpold = (HBITMAP)SelectObject(hBmpDC, g_m_hVideo);
#if 0	
	StretchBlt(
		g_hdc, 
		0, 
		0, 
		SCREEN_WIDTH, 
		SCREEN_HEIGHT, 
		hBmpDC, 
		0, 
		0, 
		width, 
		height, 
		SRCCOPY); 
#else
	StretchBlt(
		g_hdc, 
		0, 
		0, 
		SCREEN_WIDTH, 
		SCREEN_HEIGHT, 
		hBmpDC, 
		left, 
		top, 
		valid_width, 
		valid_height, 
		SRCCOPY);
#endif
	SelectObject(hBmpDC, hbmpold);
	DeleteDC(hBmpDC);

	START_SYNC_CLOCK;
	return 0;
}


int output_frame(void *frm, int width, int height)
{
	HDC hBmpDC;
	HBITMAP hbmpold;
	unsigned char* y;
	unsigned char* u;
	unsigned char* v;
	unsigned int LumaSize;

	STOP_SYNC_CLOCK;
	
	hBmpDC = CreateCompatibleDC(g_hdc);
	if(hBmpDC == NULL)
	{
		printf("create compatible dc fail \n");
		return -1;
	}

	LumaSize =  (((unsigned short int)width+15)&0xFFFFFFF0) * ((height+15)&0xFFFFFFF0);

	y=(unsigned char*) frm;
	u=y+LumaSize+(LumaSize>>2);
	v=y+LumaSize;

	// 4:2:0 format
	ConvertYUVToRGB32(g_rgbbuf,4*width,y,u,v,width,width>>1,0,width,height,0x7);


	hbmpold = (HBITMAP)SelectObject(hBmpDC, g_m_hVideo);
#if 0
	BitBlt(g_hdc,
		0, 
		0, 
		width, 
		height, 
		hBmpDC, 
		0, 
		0, 
		SRCCOPY);
#else
	StretchBlt(
		g_hdc, 
		0, 
		0, 
		SCREEN_WIDTH, 
		SCREEN_HEIGHT, 
		hBmpDC, 
		0, 
		0, 
		width, 
		height, 
		SRCCOPY); 
#endif
	SelectObject(hBmpDC, hbmpold);
	DeleteDC(hBmpDC);

	START_SYNC_CLOCK;
	return 0;
}

#elif RVDS
int init_lcd_video(CHROMA_FORMAT fmt, int width, int height,int left,int top,int in_width,int in_height, int gso)
{
    return 0;//ok
}

int deinit_lcd_video()
{
    return 0;
}

int output_frame(void *frm, int width, int height)
{
    return 0;
}

#else // linux

static int g_lcd_buf_count = 0;
extern int g_buffer_number;
#define UNUSED_FLAG  0
#define HOLD_FLAG    1
#define DQ_FLAG      2
static int mmf_video_out_v4l2_inst_init (mmf_video_out_v4l2 * that, int gso)
{
    int i;

	g_buffer_number += 5;
	if(g_buffer_number > VO_V4L2_BUFFER_COUNT)
	{
		printf("too many buffers [%d] to be prepared by V4L2\n",g_buffer_number);
		return MMF_FAILURE;
	}
	
	g_lcd_buf_count = g_buffer_number ? g_buffer_number : VO_V4L2_BUFFER_COUNT;

    that->fd_v4l2 = 0;
    if(gso)
        that->device_name = gso_video_device;
    else
        that->device_name = dto_video_device;

    for(i=0; i<g_lcd_buf_count; i++)
    {
        that->buffers[i].offset = 0;
        that->buffers[i].address = NULL;
        that->buffers[i].length = 0;
        that->buffers[i].flag= UNUSED_FLAG;
	  that->buffers[i].crop_offset=0;
    }
    that->buf_count = 0;
    that->vd_count = 0;

    last_id = 0;

    return 0;
};

static MMF_ERROR mmf_video_out_v4l2_init (mmf_video_out_v4l2 * that,
        CHROMA_FORMAT pixelfmt, int width, int height, int left,int top,int rect_width,int rect_height,int gso)
{
    int i;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_control ctrl;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers buf_req;
    struct v4l2_buffer buf;

    that->fd_v4l2 = open(that->device_name, O_RDWR, 0);
    if(that->fd_v4l2 == -1)
    {
        printf("cannot open %s!\n", that->device_name);
        return MMF_FAILURE;
    }

    if(gso)
    {
        if (ioctl(that->fd_v4l2, VIDIOC_S_OUTPUT, &g_output) < 0)
        {
            printf("set output failed\n");
            return MMF_FAILURE;
        }
    }

    memset(&cropcap, 0, sizeof(cropcap));
    cropcap.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    if(ioctl(that->fd_v4l2, VIDIOC_CROPCAP, &cropcap) < 0)
    {
        printf("failed to get crop caps!\n");
        return MMF_FAILURE;
    }
/*    M_MESSAGE(LEVEL_DEBUG, "  cropcap.bounds.width = %d\n  cropcap.bound.height = %d\n" \
            "  cropcap.defrect.width = %d\n  cropcap.defrect.height = %d\n",
            cropcap.bounds.width, cropcap.bounds.height,
            cropcap.defrect.width, cropcap.defrect.height);
*/
    crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    crop.c.top = 0;
    crop.c.left = 0;
    crop.c.width = cropcap.defrect.width;
    crop.c.height = cropcap.defrect.height;

#ifdef FIX_MX27_LIMITATION	// input resolution have limitation on MX27 BSP
	// width	
	if(crop.c.width>rect_width) // screen > image : full image content and shrink screen (1:1)
	{
		crop.c.width=rect_width;
	}	
	else if(rect_width>4*crop.c.width) // image > 4* screen : cut image content and full screen (4:1 down)
	{
		rect_width=4*crop.c.width;
	}	
	else if(rect_width>2*crop.c.width) //4*screen> image > 2* screen : cut image content and full screen (2:1 down)
	{
		rect_width=2*crop.c.width;
	}
	else // 2*screen>= image >= screen  or image== 4* screen: full image content and full screen (scale: 1 to 2, fixed 4)
	{
		//OK
	}
	// height	
	if(crop.c.height>rect_height)
	{
		crop.c.height=rect_height;
	}
	else if(rect_height>4*crop.c.height)
	{
		rect_height=4*crop.c.height;
	}	
	else if(rect_height>2*crop.c.height)
	{
		rect_height=2*crop.c.height;
	}
	else
	{
		//OK
	}
	printf("screen width: %d, height: %d \r\n",crop.c.width,crop.c.height);

#endif

    if(ioctl(that->fd_v4l2, VIDIOC_S_CROP, &crop) < 0)
    {
        printf("failed to set crop!\n");
        return MMF_FAILURE;
    }

#ifdef FSL_MX27
	// do not support rotate !
#else
    /* rotate 90-right for imx31ads lcd */
    ctrl.id = V4L2_CID_PRIVATE_BASE; //V4L2_CID_MXC_ROTATE;
    ctrl.value = 4;//IPU_ROTATE_90_RIGHT;
    if(ioctl(that->fd_v4l2, VIDIOC_S_CTRL, &ctrl) < 0)
    {
        printf("failed to rotate!\n");
        //return MMF_FAILURE;
    }
#endif

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    //fmt.fmt.pix.width = width;
    //fmt.fmt.pix.height = height;

#if 1	// eagle for dut cropping
{
	#define CROP_ALIGN	8
	#ifdef FSL_MX27  // for mx27
	struct v4l2_rect {
	int   left;
	int   top;
	int   width;
	int   height;
	};
	struct v4l2_rect off;
	#else // for mx31
	struct v4l2_mxc_offset {
		unsigned long u_offset;
		unsigned long v_offset;
	};
	struct v4l2_mxc_offset off;
	#endif	

	int in_width,in_height,in_width_chroma,in_height_chroma;
	int cr_left,cr_right,cr_bottom,cr_top;
	int crop_left_chroma,crop_right_chroma,crop_bottom_chroma,crop_top_chroma;

	//printf("width: %d, height: %d \r\n",width,height);
	in_width=rect_width/CROP_ALIGN*CROP_ALIGN;
	in_height=rect_height/CROP_ALIGN*CROP_ALIGN;
	cr_left=left;
	cr_top=top;
	cr_right=width-in_width-cr_left;
	cr_bottom=height-in_height-cr_top;
	in_width_chroma = in_width / 2;
	in_height_chroma = in_height / 2;
	crop_left_chroma = cr_left / 2;
	crop_right_chroma = cr_right / 2;
	crop_top_chroma = cr_top / 2;
	crop_bottom_chroma = cr_bottom / 2;
	//printf("%d,%d,%d,%d \r\n",cr_left,cr_right,cr_top,cr_bottom);

	#ifdef FSL_MX27 // for mx27
	off.left = cr_left;
	off.top  = cr_top;
	off.width= in_width;
	off.height=in_height;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height =height;
	#else  // for mx31
	off.u_offset = ((cr_left + cr_right + in_width) * (in_height + cr_bottom)) - cr_left + (crop_top_chroma * (in_width_chroma + crop_left_chroma + crop_right_chroma))+ crop_left_chroma;
	off.v_offset = off.u_offset +	(crop_left_chroma + crop_right_chroma + in_width_chroma) * ( in_height_chroma  + crop_bottom_chroma + crop_top_chroma);
	fmt.fmt.pix.width = in_width;
	fmt.fmt.pix.height = in_height;
	#endif

	fmt.fmt.pix.bytesperline =width;// in_width + cr_left + cr_right;
	fmt.fmt.pix.priv = (unsigned long) & off;
	fmt.fmt.pix.sizeimage = width*height*3/2; //(in_width + cr_left + cr_right)* (in_height + cr_top + cr_bottom)*3/2;
}	
#endif


    switch(pixelfmt)
    {
        case FORMAT_4_2_0:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
            video_size = width * height * 3 / 2;
            break;
        case FORMAT_4_2_2:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YYUV;
            break;
        default:
            printf("unknown format!\n");
            return MMF_FAILURE;
    }
    if(ioctl(that->fd_v4l2, VIDIOC_S_FMT, &fmt) < 0)
    {
        printf("failed to set format!\n");
        return MMF_FAILURE;
    }

    memset(&buf_req, 0, sizeof(buf_req));
    buf_req.count = g_lcd_buf_count;
    buf_req.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buf_req.memory = V4L2_MEMORY_MMAP;
    if(ioctl(that->fd_v4l2, VIDIOC_REQBUFS, &buf_req) < 0)
    {
        printf("failed to request buffers!\n");
        return MMF_FAILURE;
    }

    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buf.memory = V4L2_MEMORY_MMAP;
    for(i=0; i<g_lcd_buf_count; i++)
    {
        buf.index = i;
        if(ioctl(that->fd_v4l2, VIDIOC_QUERYBUF, &buf) < 0)
        {
            printf("failed to query this buffer (%d)!\n", buf.index);
            return MMF_FAILURE;
        }

        that->buffers[buf.index].length = buf.length;
        that->buffers[buf.index].offset = buf.m.offset;
        that->buffers[buf.index].address = mmap(NULL, buf.length,
                PROT_READ | PROT_WRITE, MAP_SHARED,
                that->fd_v4l2, buf.m.offset);

	//eagle for dut cropping
#ifdef FSL_MX27
	//
#else 
	that->buffers[buf.index].crop_offset= buf.m.offset+ (top * width)+ left;	
#endif		

    }

    that->inited = 1;

    return MMF_SUCCESS;
}

static int mmf_video_out_v4l2_get_buf(void *vo)
{
    int i;
    struct v4l2_buffer buf;
    mmf_video_out_v4l2 *that = (mmf_video_out_v4l2*)vo;

    // direct render requires at least 3 frames vacant
    if(g_lcd_buf_count - that->buf_count < 3)
    {
        buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        buf.memory = V4L2_MEMORY_MMAP;

again:
        if(ioctl(that->fd_v4l2, VIDIOC_DQBUF, &buf) < 0)
        {
            // seed to fast, delay ...
            // FIXME!
            printf("hold!");
            usleep(1000);
            goto again;
        }
        // 
        for(i=0; i<g_lcd_buf_count; i++)
        {
        	//if(that->buffers[i].offset == buf.m.offset)
            if((that->buffers[i].offset == buf.m.offset)||(that->buffers[i].crop_offset== buf.m.offset))
            {
                that->buf_count--;
                that->buffers[i].flag &= ~DQ_FLAG;
                break;
            }
        }
    }

    for(i=last_id+1; i<g_lcd_buf_count; i++)
    {
        if(that->buffers[i].flag == UNUSED_FLAG)
        {
            last_id = i;
            return i;
        }
    }
    for(i=0; i<last_id+1; i++)
    {
        if(that->buffers[i].flag == UNUSED_FLAG)
        {
            last_id = i;
            return i;
        }
    }

    last_id = 0;

    return -1;
}

static void mmf_video_out_v4l2_receive (void *context, int buffindex)
{
    int i;
    mmf_video_out_v4l2 *that = (mmf_video_out_v4l2*)context;
    int type;
    struct v4l2_buffer buf;

    buf.index = buffindex;

    if(buf.index >= g_lcd_buf_count || buf.index < 0)
    {
        printf("error buffer index!\n");
        return;
    }
    buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buf.memory = V4L2_MEMORY_MMAP;
    if(ioctl(that->fd_v4l2, VIDIOC_QUERYBUF, &buf) < 0)
    {
        printf("no such buffer!\n");
        return;
    }

	//eagle for dut cropping
#ifdef FSL_MX27
	//
#else
	buf.m.offset=that->buffers[buf.index].crop_offset;
#endif


    if(ioctl(that->fd_v4l2, VIDIOC_QBUF, &buf) < 0)
    {
        printf("cannot queue this buffer!\n");
        return;
    }

    that->buf_count++;

    if(that->vd_count++ == 1)
    {
        type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        if(ioctl(that->fd_v4l2, VIDIOC_STREAMON, &type) < 0)
        {
            printf("output video failed!\n");
            return;
        }
        else
        {
            printf("output video now!\n");
        }
    }
}

static MMF_ERROR mmf_video_out_v4l2_de_init (mmf_video_out_v4l2 * that)
{
    int i;
    int type;

    type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

    if(ioctl(that->fd_v4l2, VIDIOC_STREAMOFF, &type) < 0)
    {
        printf("warning: streamoff error\n");
    }

    for(i=0; i<g_lcd_buf_count; i++)
    {
        munmap(that->buffers[i].address, that->buffers[i].length);
    }

    close(that->fd_v4l2);

//    MMF_FREE(that->device_name);

    return MMF_SUCCESS;
}

//------------------------------------------------------
//interface
int init_lcd_video(CHROMA_FORMAT fmt, int width, int height,int left,int top,int in_width,int in_height, int gso)
{
    // init vo
    mmf_video_out_v4l2_inst_init(&vo, gso);

   g_last_width=in_width;
   g_last_height=in_height;
    // init hardware
    if(MMF_FAILURE == mmf_video_out_v4l2_init(&vo, fmt, width, height,left,top,in_width,in_height, gso))
    {
        if(vo.fd_v4l2)
        {
            close(vo.fd_v4l2);
        }
        return -1;
    }

    return 0;//ok
}

int deinit_lcd_video()
{
    if(vo.inited)
    {
        mmf_video_out_v4l2_de_init(&vo);
        vo.inited = 0;
    }

    return 0;
}

int output_frame(void *frm, int width, int height)
{
    if(vo.inited)
    {
        void *buffer;
        int index = mmf_video_out_v4l2_get_buf(&vo);
        if(index == -1)
        {
            printf("no buffer available!\n");
            return -1;
        }
        buffer = vo.buffers[index].address;
        vo.buffers[index].flag |= DQ_FLAG;

#if 1
        memcpy(buffer, frm, video_size);
#else
        {
            #define min(x,y) ((x)<(y)?(x):(y))
            // cache buffer "frm"
            int i;
            int tmp = frm;
            const int cachesize = 4*1024;
            int size = min(cachesize, video_size);
//            for(i=0; i<((size>>5)>>4); i++)
            for(i=0; i<50; i++)
            {
                __asm__ __volatile__
                (
                    "PLD [%0, #0];\n\r"
                    "PLD [%1, #0];\n\r"
                    "PLD [%2, #0];\n\r"
                    "PLD [%3, #0];\n\r"
                    "PLD [%4, #0];\n\r"
                    "PLD [%5, #0];\n\r"
                    "PLD [%6, #0];\n\r"
                    "PLD [%7, #0];\n\r"
                    :
                    :"r"(tmp),"r"(tmp+32),"r"(tmp+64),"r"(tmp+96),"r"(tmp+128),"r"(tmp+160),"r"(tmp+192),"r"(tmp+224)
                );
                tmp += (32*8);
                __asm__ __volatile__
                (
                    "PLD [%0, #0];\n\r"
                    "PLD [%1, #0];\n\r"
                    "PLD [%2, #0];\n\r"
                    "PLD [%3, #0];\n\r"
                    "PLD [%4, #0];\n\r"
                    "PLD [%5, #0];\n\r"
                    "PLD [%6, #0];\n\r"
                    "PLD [%7, #0];\n\r"
                    :
                    :"r"(tmp),"r"(tmp+32),"r"(tmp+64),"r"(tmp+96),"r"(tmp+128),"r"(tmp+160),"r"(tmp+192),"r"(tmp+224)
                );

                tmp += (32*8);
            }
        }
//        memcpy(buffer, frm, video_size);

#endif
        mmf_video_out_v4l2_receive(&vo, index);

        return 0;
    }

    return -1;
}
//------------------------------------------------------
//interface to direct render

static int mmf_video_out_v4l2_get_buf_dr(void *vo)
{
    int i;
    struct v4l2_buffer buf;
    mmf_video_out_v4l2 *that = (mmf_video_out_v4l2*)vo;

	 for(i=last_id+1; i<g_lcd_buf_count; i++)
    {
        if(that->buffers[i].flag == UNUSED_FLAG)
        {
            last_id = i;
            return i;
        }
    }
    for(i=0; i<last_id+1; i++)
    {
        if(that->buffers[i].flag == UNUSED_FLAG)
        {
            last_id = i;
            return i;
        }
    }

    // direct render requires at least 3 frames vacant
    if(g_lcd_buf_count - that->buf_count < 3)
    {
        buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        buf.memory = V4L2_MEMORY_MMAP;

again:
        if(ioctl(that->fd_v4l2, VIDIOC_DQBUF, &buf) < 0)
        {
            // seed to fast, delay ...
            // FIXME!
            printf("hold!");
            usleep(1000);
            goto again;
        }
        // 
        for(i=0; i<g_lcd_buf_count; i++)
        {
            //if(that->buffers[i].offset == buf.m.offset)
            if ((that->buffers[i].offset == buf.m.offset)||(that->buffers[i].crop_offset== buf.m.offset))
            {
                that->buf_count--;
                that->buffers[i].flag &= ~DQ_FLAG;
                break;
            }
        }
    }

    for(i=last_id+1; i<g_lcd_buf_count; i++)
    {
        if(that->buffers[i].flag == UNUSED_FLAG)
        {
            last_id = i;
            return i;
        }
    }
    for(i=0; i<last_id+1; i++)
    {
        if(that->buffers[i].flag == UNUSED_FLAG)
        {
            last_id = i;
            return i;
        }
    }

    last_id = 0;

    return -1;
}

void *get_buffer_dr(void)
{
    int index = mmf_video_out_v4l2_get_buf_dr(&vo);
    if(index == -1)
    {
        printf("no buffer available!\n");
        return NULL;
    }
	vo.buffers[index].flag |= HOLD_FLAG;
	vo.buf_count++;
    return vo.buffers[index].address;
}

void release_buffer_dr(void *buffer)
{
	int i;
    for(i=0; i<g_lcd_buf_count; i++)
    {
        if(vo.buffers[i].address == buffer)
        {
            vo.buffers[i].flag &= ~HOLD_FLAG;
        }
    }
}

long get_physical_addr(void *buffer)
{
	int i;
    for(i=0; i<g_lcd_buf_count; i++)
    {
        if(vo.buffers[i].address == buffer)
        {
            return vo.buffers[i].offset;
        }
    }
	return 0;
}

static void mmf_video_out_v4l2_receive_dr (void *context, int buffindex)
{
    int i;
    mmf_video_out_v4l2 *that = (mmf_video_out_v4l2*)context;
    int type;
    struct v4l2_buffer buf;

    buf.index = buffindex;

    if(buf.index >= g_lcd_buf_count || buf.index < 0)
    {
        printf("error buffer index!\n");
        return;
    }
    buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buf.memory = V4L2_MEMORY_MMAP;
    if(ioctl(that->fd_v4l2, VIDIOC_QUERYBUF, &buf) < 0)
    {
        printf("no such buffer!\n");
        return;
    }

	//eagle for dut cropping
#ifdef FSL_MX27
	//
#else
	buf.m.offset=that->buffers[buf.index].crop_offset;
#endif

    if(ioctl(that->fd_v4l2, VIDIOC_QBUF, &buf) < 0)
    {
        printf("cannot queue this buffer!\n");
        return;
    }

 //   that->buf_count++;

    if(that->vd_count++ == 1)
    {
        type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        if(ioctl(that->fd_v4l2, VIDIOC_STREAMON, &type) < 0)
        {
            printf("output video failed!\n");
            return;
        }
        else
        {
            printf("output video now!\n");
        }
    }
}

int output_frame_dr(void *frm)
{
    int i;
    for(i=0; i<g_lcd_buf_count; i++)
    {
        if(vo.buffers[i].address == frm)
        {
            vo.buffers[i].flag |= DQ_FLAG;
            mmf_video_out_v4l2_receive_dr(&vo, i);
            break;
        }
    }

}

#endif
#endif
