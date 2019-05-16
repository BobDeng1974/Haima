/*
 ***********************************************************************
 * Copyright 2005-2008 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*!
 ***********************************************************************
 * Implementation file of color space conversion
 *   Currently YUV (4:2:0, 4:2:2, 4:4:4) to rgb32 is implemented. This 
 *   file contains some unsafe code, please don't release it.
 * Support conversion:
 *   1. YUV420
 *   2. YUV422
 *   3. YUV444
 *   4. Only Y or U or V channel
 *
 *
 * History
 *   Date          Changed                                Changed by
 *   Nov. 14, 2007 Create and port from RawMovieFile.cpp  Zhenyong Chen
 ***********************************************************************
 */

/*
  The following transformation is based on ITU-Recommendation BT.470-2 System B,G and
  on SMPTE 170M ( modes 5 and 6 in MPEG-4 standard ) with Y 16...235 and UV 16...240

  /  2568      0   3343  \              
 |   2568  -0c92  -1a1e   | / 65536 * 8 
  \  2568   40cf      0  /              

    Y -= 16;
    U -= 128;
    V -= 128;

    R = (0x2568*Y + 0x0000*V + 0x3343*U) / 0x2000;
    G = (0x2568*Y - 0x0c92*V - 0x1a1e*U) / 0x2000;
    B = (0x2568*Y + 0x40cf*V + 0x0000*U) / 0x2000;

    R = R>255 ? 255 : R;
    R = R<0   ?   0 : R;

    G = G>255 ? 255 : G;
    G = G<0   ?   0 : G;

    B = B>255 ? 255 : B;
    B = B<0   ?   0 : B;

*/

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

