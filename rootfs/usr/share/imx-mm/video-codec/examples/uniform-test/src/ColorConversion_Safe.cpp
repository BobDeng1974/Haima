/*
 ***********************************************************************
 * Copyright 2006-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*!
 ***********************************************************************
 * Implementation file of color space conversion
 *   Currently YUV (4:2:0, 4:2:2, 4:4:4) to rgb32 is implemented. 
 &
 * Support conversion:
 *   1. YUV420
 *   2. YUV422
 *   3. YUV444
 *   4. Only Y or U or V channel
 *
 *
 * History
 *   Date          Changed                                Changed by
 *   Jun. 16, 2008 Create                                 Zhenyong Chen
 ***********************************************************************
 */


#define ZERO_Y 16
#define ZERO_U 128
#define ZERO_V 128

#define FORMRGB(alpha,r,g,b) ((alpha)<<24) | ((r)<<16) | ((g)<<8) | (b)

#define GET_ALPHA(rgb) (((rgb)>>24) & 0xff)
#define GET_R(rgb) (((rgb)>>16) & 0xff)
#define GET_G(rgb) (((rgb)>>8)  & 0xff)
#define GET_B(rgb) (((rgb)>>0)  & 0xff)

#define CLIPCOLOR(v) ((v)<0?0:((v)>255?255:(v)))

/****************************************************************
 * YCbCr to RGB
 ***************************************************************/


/* all stride values are in _pixels_ */
static void YUV420toRGB32_fast(
    const unsigned char *puc_y,
    int stride_y, 
    const unsigned char *puc_u,
    const unsigned char *puc_v,
    int stride_uv, 
    unsigned char *puc_out,
    int width_y,
    int height_y,
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

	width_y &= ~1;
	height_y &= ~1;

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
            int ival;
            int rgby, ru, rv, gu, gv, bu, bv;
            int R, G, B;

            if(mask_clrchannel & 1)
                ival = *pV;
            else
                ival = ZERO_V;

            rv =  409 * (ival - 128);
            gv = -208 * (ival - 128);
            bv =    0 * (ival - 128);

            pV++;

            if(mask_clrchannel & 2)
                ival = *pU;
            else
                ival = ZERO_U;

            ru =    0 * (ival - 128);
            gu = -100 * (ival - 128);
            bu =  516 * (ival - 128);

            pU++;

            // top-left pixel
            if(mask_clrchannel & 4)
                ival = *pY;
            else
                ival = ZERO_Y;

            rgby = 298 * (ival - 16);
            pY++;

            R = (rgby+ru+rv+128)>>8;
            G = (rgby+gu+gv+128)>>8;
            B = (rgby+bu+bv+128)>>8;
            out = FORMRGB(0xFF, CLIPCOLOR(R), CLIPCOLOR(G), CLIPCOLOR(B));
            *(unsigned int *)puc_out = out;
            puc_out += 4;

            // top-right pixel
            if(mask_clrchannel & 4)
                ival = *pY;
            else
                ival = ZERO_Y;

            rgby = 298 * (ival - 16);
            pY++;

            R = (rgby+ru+rv+128)>>8;
            G = (rgby+gu+gv+128)>>8;
            B = (rgby+bu+bv+128)>>8;
            out = FORMRGB(0xFF, CLIPCOLOR(R), CLIPCOLOR(G), CLIPCOLOR(B));
            *(unsigned int *)puc_out = out;
            puc_out += 4;

            // bottom-left
            if(mask_clrchannel & 4)
                ival = *pY1;
            else
                ival = ZERO_Y;

            rgby = 298 * (ival - 16);
            pY1++;

            R = (rgby+ru+rv+128)>>8;
            G = (rgby+gu+gv+128)>>8;
            B = (rgby+bu+bv+128)>>8;
            out = FORMRGB(0xFF, CLIPCOLOR(R), CLIPCOLOR(G), CLIPCOLOR(B));
            *(unsigned int *)pOut2 = out;
            pOut2 += 4;

            // bottom-right
            if(mask_clrchannel & 4)
                ival = *pY1;
            else
                ival = ZERO_Y;

            rgby = 298 * (ival - 16);
            pY1++;

            R = (rgby+ru+rv+128)>>8;
            G = (rgby+gu+gv+128)>>8;
            B = (rgby+bu+bv+128)>>8;
            out = FORMRGB(0xFF, CLIPCOLOR(R), CLIPCOLOR(G), CLIPCOLOR(B));
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

	width_y &= ~1;

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
            int ival;
            int rgby, ru, rv, gu, gv, bu, bv;
            int R, G, B;

            if(mask_clrchannel & 1)
                ival = *pV;
            else
                ival = ZERO_V;

            rv =  409 * (ival - 128);
            gv = -208 * (ival - 128);
            bv =    0 * (ival - 128);

            pV++;

            if(mask_clrchannel & 2)
                ival = *pU;
            else
                ival = ZERO_U;

            ru =    0 * (ival - 128);
            gu = -100 * (ival - 128);
            bu =  516 * (ival - 128);

            pU++;

            // left pixel
            if(mask_clrchannel & 4)
                ival = *pY;
            else
                ival = ZERO_Y;

            rgby = 298 * (ival - 16);
            pY++;

            R = (rgby+ru+rv+128)>>8;
            G = (rgby+gu+gv+128)>>8;
            B = (rgby+bu+bv+128)>>8;

            out = FORMRGB(0xFF, CLIPCOLOR(R), CLIPCOLOR(G), CLIPCOLOR(B));
            *(unsigned int *)puc_out = out;
            puc_out += 4;

            // right pixel
            if(mask_clrchannel & 4)
                ival = *pY;
            else
                ival = ZERO_Y;

            rgby = 298 * (ival - 16);
            pY++;

            R = (rgby+ru+rv+128)>>8;
            G = (rgby+gu+gv+128)>>8;
            B = (rgby+bu+bv+128)>>8;

            out = FORMRGB(0xFF, CLIPCOLOR(R), CLIPCOLOR(G), CLIPCOLOR(B));
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
            int ival;
            int rgby, ru, rv, gu, gv, bu, bv;
            int R, G, B;

            if(mask_clrchannel & 1)
                ival = *pV;
            else
                ival = ZERO_V;

            rv =  409 * (ival - 128);
            gv = -208 * (ival - 128);
            bv =    0 * (ival - 128);

            pV++;

            if(mask_clrchannel & 2)
                ival = *pU;
            else
                ival = ZERO_U;

            ru =    0 * (ival - 128);
            gu = -100 * (ival - 128);
            bu =  516 * (ival - 128);

            pU++;

            if(mask_clrchannel & 4)
                ival = *pY;
            else
                ival = ZERO_Y;

            rgby = 298 * (ival - 16);
            pY++;

            R = (rgby+ru+rv+128)>>8;
            G = (rgby+gu+gv+128)>>8;
            B = (rgby+bu+bv+128)>>8;

            out = FORMRGB(0xFF, CLIPCOLOR(R), CLIPCOLOR(G), CLIPCOLOR(B));
            *(unsigned int *)puc_out = out;
            puc_out += 4;

        }

        puc_y   += stride_y;
        puc_u   += stride_uv;
        puc_v   += stride_uv;
        puc_out += stride_diff;
    }
}


void YUVToRGB32(
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

/****************************************************************
 * RGB to YCbCr
 ***************************************************************/

static void RGBtoYUV420_fast(
    unsigned char *puc_y,
    int stride_y,
    unsigned char *puc_u,
    unsigned char *puc_v,
    int stride_uv,
    const int *rgbbuf,
    int width_y,
    int height_y,
    int stride_rgb)
{
    int x, y;

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
        unsigned char *pY, *pY2;
        unsigned char *pU;
        unsigned char *pV;
        int *rgbout, *rgbout2;
        int rgb;

        pY =  puc_y;
        pU =  puc_u;
        pV =  puc_v;
        rgbout = (int *)rgbbuf;

        pY2 = pY + stride_y;
        rgbout2 = rgbout + stride_rgb;

        for (x=0; x<width_y; x+=2)
        {
            int R, G, B;
            int Y, U, V;

            // top-left
            rgb = *rgbout++;

            R = GET_R(rgb);
            G = GET_G(rgb);
            B = GET_B(rgb);

            Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
            *pY++ = (unsigned char)Y;
            U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
            V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;

            // top-right
            rgb = *rgbout++;

            R = GET_R(rgb);
            G = GET_G(rgb);
            B = GET_B(rgb);

            Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
            *pY++ = (unsigned char)Y;
            U += ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
            V += ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;

            // bottom-left
            rgb = *rgbout2++;

            R = GET_R(rgb);
            G = GET_G(rgb);
            B = GET_B(rgb);

            Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
            *pY2++ = (unsigned char)Y;
            U += ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
            V += ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;

            // bottom-right
            rgb = *rgbout2++;

            R = GET_R(rgb);
            G = GET_G(rgb);
            B = GET_B(rgb);

            Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
            *pY2++ = (unsigned char)Y;
            U += ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
            V += ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;

            *pU++ = (unsigned char)((U+2)>>2);
            *pV++ = (unsigned char)((V+2)>>2);
        }

        puc_y   += (stride_y<<1);
        puc_u   += stride_uv;
        puc_v   += stride_uv;
        rgbbuf  += (stride_rgb<<1);
    }
}

static void RGBtoYUV422_fast(
    unsigned char *puc_y,
    int stride_y,
    unsigned char *puc_u,
    unsigned char *puc_v,
    int stride_uv,
    const int *rgbbuf,
    int width_y,
    int height_y,
    int stride_rgb)
{
    int x, y;

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
        int *rgbout;
        int rgb;

        pY =  puc_y;
        pU =  puc_u;
        pV =  puc_v;
        rgbout = (int *)rgbbuf;

        for (x=0; x<width_y; x+=2)
        {
            int R, G, B;
            int Y, U, V;

            // left
            rgb = *rgbout++;

            R = GET_R(rgb);
            G = GET_G(rgb);
            B = GET_B(rgb);

            Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
            *pY++ = (unsigned char)Y;

            U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
            V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;

            // right
            rgb = *rgbout++;

            R = GET_R(rgb);
            G = GET_G(rgb);
            B = GET_B(rgb);

            Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
            *pY++ = (unsigned char)Y;

            U += ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
            V += ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;

            *pU++ = (unsigned char)((U+1)>>1);
            *pV++ = (unsigned char)((V+1)>>1);
        }

        puc_y   += stride_y;
        puc_u   += stride_uv;
        puc_v   += stride_uv;
        rgbbuf  += stride_rgb;
    }
}

static void RGBtoYUV444_fast(
    unsigned char *puc_y,
    int stride_y,
    unsigned char *puc_u,
    unsigned char *puc_v,
    int stride_uv,
    const int *rgbbuf,
    int width_y,
    int height_y,
    int stride_rgb)
{
    int x, y;

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
        int *rgbout;
        int rgb;

        pY =  puc_y;
        pU =  puc_u;
        pV =  puc_v;
        rgbout = (int *)rgbbuf;

        for (x=0; x<width_y; x++)
        {
            int R, G, B;
            int Y, U, V;

            rgb = *rgbout++;

            R = GET_R(rgb);
            G = GET_G(rgb);
            B = GET_B(rgb);

            Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;

            *pY++ = (unsigned char)Y;

            U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
            V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;

            *pU++ = (unsigned char)U;
            *pV++ = (unsigned char)V;
        }

        puc_y   += stride_y;
        puc_u   += stride_uv;
        puc_v   += stride_uv;
        rgbbuf  += stride_rgb;
    }
}

void RGB32ToYUV(
    const int *rgb,
    int stride_rgb,
    int width,
    int height,
    unsigned char *y,
    unsigned char *u,
    unsigned char *v,
    int stride_y,
    int stride_uv,
    int chrom_fmt)
{
    switch(chrom_fmt)
    {
    case 0:
        RGBtoYUV420_fast(y, stride_y, u, v, stride_uv, rgb, width, height, stride_rgb>>2);
        break;
    case 1:
        RGBtoYUV422_fast(y, stride_y, u, v, stride_uv, rgb, width, height, stride_rgb>>2);
        break;
    case 2:
        RGBtoYUV444_fast(y, stride_y, u, v, stride_uv, rgb, width, height, stride_rgb>>2);
        break;
    default:
        break;
    }
}

#if 0
#include <stdlib.h>
#include <memory.h>
#include "ColorConversion_Unsafe.h"

#define UNITST_WIDTH 8
#define UNITST_HEIGHT 1
static int clrbuf[UNITST_WIDTH * UNITST_HEIGHT];
static int clrbufref[UNITST_WIDTH * UNITST_HEIGHT];
static unsigned char y[UNITST_WIDTH * UNITST_HEIGHT];
static unsigned char u[UNITST_WIDTH * UNITST_HEIGHT];
static unsigned char v[UNITST_WIDTH * UNITST_HEIGHT];
static unsigned char yref[UNITST_WIDTH * UNITST_HEIGHT];
static unsigned char uref[UNITST_WIDTH * UNITST_HEIGHT];
static unsigned char vref[UNITST_WIDTH * UNITST_HEIGHT];
static int chroma_fmt = 2;


void unit_test()
{
    int i,j;

    // init color buffer
    for(i=0; i<UNITST_HEIGHT; i++)
    {
        for(j=0; j<UNITST_WIDTH; j++)
        {
			unsigned char t;
			t = rand();
            y[i*UNITST_WIDTH+j] = (t);

			t = rand();
            u[i*UNITST_WIDTH+j] = (t);

			t = rand();
            v[i*UNITST_WIDTH+j] = (t);
            if((j&1)==1)
            {
 //               y[i*UNITST_WIDTH+j] = 0xeb;
                u[i*UNITST_WIDTH+j] = u[i*UNITST_WIDTH+j-1];
                v[i*UNITST_WIDTH+j] = v[i*UNITST_WIDTH+j-1];
            }
        }
    }

    y[0] = 0x3f;
    y[1] = 0xf1;
    u[0] = 0x2a;
    u[1] = 0x2a;
    v[0] = 0xe5;
    v[1] = 0xe5;
/*    y[2] = 0x3f;
    y[3] = 0xf1;
    u[2] = 0x2a;
    u[3] = 0x2a;
    v[2] = 0xe5;
    v[3] = 0xe5;*/

    memcpy(yref, y, sizeof(y[0])*UNITST_WIDTH * UNITST_HEIGHT);
    memcpy(uref, u, sizeof(u[0])*UNITST_WIDTH * UNITST_HEIGHT);
    memcpy(vref, v, sizeof(v[0])*UNITST_WIDTH * UNITST_HEIGHT);
#if 0
    InitYUV2RGBConversion();
    ConvertYUVToRGB32(
                (unsigned char *)clrbufref,
                UNITST_WIDTH*4,
                y,
                u,
                v,
                UNITST_WIDTH,
                UNITST_WIDTH,
                chroma_fmt,
                UNITST_WIDTH,
                UNITST_HEIGHT,
                7);

    // test YUVToRGB32
    YUVToRGB32(
            (unsigned char *)clrbuf,
            UNITST_WIDTH*4,
            y,
            u,
            v,
            UNITST_WIDTH,
            UNITST_WIDTH,
            chroma_fmt,
            UNITST_WIDTH,
            UNITST_HEIGHT,
            7);

    // compare
    for(i=0; i<UNITST_HEIGHT; i++)
    {
        for(j=0; j<UNITST_WIDTH; j++)
        {
            unsigned char *p1, *p2;
            p1 = (unsigned char *)&clrbuf[i*UNITST_WIDTH+j];
            p2 = (unsigned char *)&clrbufref[i*UNITST_WIDTH+j];
            int diff;
            int sum = 0;

            diff = *p1 - *p2;
            diff = ABS(diff);
            sum |= diff;
            p1++;
            p2++;

            diff = *p1 - *p2;
            diff = ABS(diff);
            sum |= diff;
            p1++;
            p2++;

            diff = *p1 - *p2;
            diff = ABS(diff);
            sum |= diff;
            p1++;
            p2++;

            diff = *p1 - *p2;
            diff = ABS(diff);
            sum |= diff;
            p1++;
            p2++;

            if(sum > 8)
            {
                _printf("Bit mismatch found at (%d,%d). 0x%08x is desired but 0x%08x is found.\r\n",
                    j, i, clrbufref[i*UNITST_WIDTH+j], clrbuf[i*UNITST_WIDTH+j]);
                return;
            }
        }
    }

    _printf("YUV to rgb test     [pass]\r\nTesting rgb to YUV ...\r\n");
#endif
    int loopcount = 2;
    for(i=0; i<loopcount; i++)
    {
        // convert to rgb buffer
        YUVToRGB32(
                (unsigned char *)clrbuf,
                UNITST_WIDTH*4,
                y,
                u,
                v,
                UNITST_WIDTH,
                UNITST_WIDTH,
                chroma_fmt,
                UNITST_WIDTH,
                UNITST_HEIGHT,
                7);

        // convert to yuv buffers
        RGB32ToYUV(
                clrbuf,
                UNITST_WIDTH*4,
                UNITST_WIDTH,
                UNITST_HEIGHT,
                y,
                u,
                v,
                UNITST_WIDTH,
                UNITST_WIDTH,
                chroma_fmt);
        if(0)//i==0)
        {
            memcpy(yref, y, sizeof(y[0])*UNITST_WIDTH * UNITST_HEIGHT);
            memcpy(uref, u, sizeof(u[0])*UNITST_WIDTH * UNITST_HEIGHT);
            memcpy(vref, v, sizeof(v[0])*UNITST_WIDTH * UNITST_HEIGHT);
        }
    }
    // compare
    for(i=0; i<UNITST_HEIGHT; i++)
    {
        for(j=0; j<UNITST_WIDTH; j++)
        {
            unsigned char *p1, *p2;
            p1 = (unsigned char *)&y[i*UNITST_WIDTH+j];
            p2 = (unsigned char *)&yref[i*UNITST_WIDTH+j];
            int diff;
            int sum = 0;

            diff = *p1 - *p2;
            diff = ABS(diff);
            sum |= diff;

            if(sum > 16)
            {
                _printf("Bit mismatch found at (%d,%d). 0x%08x is desired but 0x%08x is found.\r\n",
                    j, i, yref[i*UNITST_WIDTH+j], y[i*UNITST_WIDTH+j]);
                if(chroma_fmt == 2)
                    _printf("input: y=0x%2x, u=0x%2x, v=0x%2x\r\n", yref[i*UNITST_WIDTH+j], uref[i*UNITST_WIDTH+j], vref[i*UNITST_WIDTH+j]);
                else if(chroma_fmt == 1)
                    _printf("input: y=0x%2x, u=0x%2x, v=0x%2x\r\n", yref[i*UNITST_WIDTH+j], uref[i*UNITST_WIDTH+j/2], vref[i*UNITST_WIDTH+j/2]);
                return;
            }
        }
    }

    _printf("rgb to YUV test     [pass]\r\n");

}
#endif

