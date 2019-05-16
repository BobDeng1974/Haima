

/*
 * wrbmp.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains routines to write output images in Microsoft "BMP"
 * format (MS Windows 3.x and OS/2 1.x flavors).
 * Either 8-bit colormapped or 24-bit full-color format can be written.
 * No compression is supported.
 *
 * These routines may need modification for non-Unix environments or
 * specialized applications.  As they stand, they assume output to
 * an ordinary stdio stream.
 *
 * This code contributed by James Arthur Boucher.
 */
/****************************************************************************
 *
 * (C) 2003 MOTOROLA INDIA ELECTRONICS LTD.
 *
 *   CHANGE HISTORY
 *   dd/mm/yy   Code Ver    Description                         Author
 *   --------   -------     -----------                         ------
 *   21/01/04   01          Memory manager cleanup              B.Venkatarao
 *   29/01/04   02          BMP support added.                  B.Venkatarao
 *   25/02/04   03          Support for RGB 16-bit output       B.Venkatarao
 *                           added.
 *   20/03/04   04          output_pixel_size is changed        B.Venkatarao
 *                           to output_format and enum is
 *                           added
 *   01/04/04   05          Remove warnings.                    B.Venkatarao
 *   11/05/04   06          Review rework for API changes       B.Venkatarao
 *   14/05/04   07          Free the allocated memory           B.Venkatarao
 *   01/06/04   08          Changes for not including library   B.Venkatarao
 *                          specific h-files
 *   11/12/04   09          API changes, structures modified    B.Venkatarao
 *   07/01/05   10          Prefix all the data types, defines  B.Venkatarao
 *                           exposed to API
 *   12/11/08   11          add bgr format:ENGR00098581		Eagle Zhou 
 ****************************************************************************/
	 /*
	  ***********************************************************************
	  * Copyright 2005-2010 by Freescale Semiconductor, Inc.
	  * All modifications are confidential and proprietary information
	  * of Freescale Semiconductor, Inc. 
	  ***********************************************************************
	  *  History :
	 
	  *  Date			  Author	   Version	  Description
	 
	  *  Apr,2007		 Jogesh 	   1.0		  Level 4 Warnings Removed
	 
	  */

#include <stdio.h>
#include <stdlib.h>
#include "jpeg_dec_interface.h"
#include "wrbmp.h"

#if defined(WINCE_WARNLVL4_DISABLE) && defined(__WINCE)
	// disable some level-4 warnings, use #pragma warning(enable:###) to re-enable
	#pragma warning(disable:4100) // warning C4100: unreferenced formal parameter
#endif //#if defined(WINCE_WARNLVL4_DISABLE) && defined(__WINCE)

#define JFREAD(file,buf,sizeofbuf)  \
  ((size_t) fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))
#define JFWRITE(file,buf,sizeofbuf)  \
  ((size_t) fwrite((const void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))

/*
 * To support 12-bit JPEG data, we'd have to scale output down to 8 bits.
 * This is not yet implemented.
 */

/*
 * Since BMP stores scanlines bottom-to-top, we have to invert the image
 * from JPEG's top-to-bottom order.  To do this, we save the outgoing data
 * in a virtual array during put_pixel_row calls, then actually emit the
 * BMP file during finish_output.  The virtual array contains one JSAMPLE per
 * pixel if the output is grayscale or colormapped, three if it is full color.
 */

/* Private version of data destination object */

typedef struct {
  struct djpeg_dest_struct pub;	/* public fields */

  int is_os2;		/* saves the OS2 format request flag */

  JPEGD_UINT8 **whole_image;      /* needed to reverse row order */
  int data_width;           /* JSAMPLEs per row */
  int row_width;            /* physical width of one row in the BMP file */
  int pad_bytes;            /* number of padding bytes needed per row */
  int cur_output_row;       /* next row# to write to virtual array */
} bmp_dest_struct;

typedef bmp_dest_struct * bmp_dest_ptr;


/* Forward declarations */
void write_colormap (JPEGD_Decoder_Object *dec_obj,
                     bmp_dest_ptr dest, int map_colors, int map_entry_size);

/*
 * Write some pixel data.
 * In this module rows_supplied will always be 1.
 */

void
put_pixel_rows_bmp (JPEGD_Decoder_Object *dec_obj, djpeg_dest_ptr dinfo,
                    int rows_supplied)
/* This version is for writing 24-bit pixels */
{
    JPEGD_Decoder_Params *dec_param = &dec_obj->dec_param;
    JPEGD_Decoder_Info  *dec_info = &dec_obj->dec_info;
    bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
    JPEGD_UINT8 **image_ptr;
    register JPEGD_UINT8 *outptr;
    register int col;
    int pad;

    /* Access next row in virtual array */
    image_ptr = dest->whole_image + dest->cur_output_row;

    dest->cur_output_row++;

    /* Transfer data.  Note destination values must be in BGR order
     * (even though Microsoft's own documents say the opposite).
     */
    outptr = image_ptr[0];
    if ((dec_param->output_format == JPEGD_OFMT_RGB_565)||(dec_param->output_format == JPEGD_OFMT_BGR_565))
    {
        register JPEGD_UINT16 *inptr, invalue;
        inptr = (JPEGD_UINT16 *)dest->pub.buffer[0];
        for (col = dec_info->actual_output_width; col > 0; col--) {
            invalue = *inptr++;
            outptr[0] = (JPEGD_UINT8)((invalue << 3) & 0xFF);
            invalue >>= 5;
            outptr[1] = (JPEGD_UINT8)((invalue << 2) & 0xFF);
            invalue >>= 6;
            outptr[2] = (JPEGD_UINT8)((invalue << 3) & 0xFF);
            outptr += 3;
        }
    }
    else
    {
        register JPEGD_UINT8 *inptr;
        inptr = dest->pub.buffer[0];
        for (col = dec_info->actual_output_width; col > 0; col--) {
            outptr[2] = *inptr++;
            outptr[1] = *inptr++;
            outptr[0] = *inptr++;
            outptr += 3;
        }
    }

    /* Zero out the pad bytes. */
    pad = dest->pad_bytes;
    while (--pad >= 0)
        *outptr++ = 0;
}

void
put_gray_rows_bmp (JPEGD_Decoder_Object *dec_obj, djpeg_dest_ptr dinfo,
                   int rows_supplied)
/* This version is for grayscale OR quantized color output */
{
    bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
    JPEGD_Decoder_Info  *dec_info = &dec_obj->dec_info;
    JPEGD_UINT8 **image_ptr;
    register JPEGD_UINT8 *inptr, *outptr;
    register int col;
    int pad;

    /* Access next row in virtual array */
    image_ptr = dest->whole_image + dest->cur_output_row;
    dest->cur_output_row++;

    /* Transfer data. */
    inptr = dest->pub.buffer[0];
    outptr = image_ptr[0];
    for (col = dec_info->actual_output_width; col > 0; col--) {
        *outptr++ = *inptr++;
    }

    /* Zero out the pad bytes. */
    pad = dest->pad_bytes;
    while (--pad >= 0)
        *outptr++ = 0;
}

/*
 * Finish up at the end of the file.
 *
 * Here is where we really output the BMP file.
 *
 * First, routines to write the Windows and OS/2 variants of the file header.
 */

void
write_bmp_header (JPEGD_Decoder_Object *dec_obj, bmp_dest_ptr dest)
/* Write a Windows-style BMP file header, including colormap if needed */
{
    JPEGD_Decoder_Info  *dec_info = &dec_obj->dec_info;
    char bmpfileheader[14];
    char bmpinfoheader[40];
#define PUT_2B(array,offset,value)  \
    (array[offset] = (char) ((value) & 0xFF), \
     array[offset+1] = (char) (((value) >> 8) & 0xFF))
#define PUT_4B(array,offset,value)  \
        (array[offset] = (char) ((value) & 0xFF), \
         array[offset+1] = (char) (((value) >> 8) & 0xFF), \
         array[offset+2] = (char) (((value) >> 16) & 0xFF), \
         array[offset+3] = (char) (((value) >> 24) & 0xFF))
        JPEGD_INT32 headersize, bfSize;
    int bits_per_pixel, cmap_entries;

    /* Compute colormap size and total file size */
    {
        /* Unquantized, full color RGB */
        bits_per_pixel = 24;
        cmap_entries = 0;
    }
    /* File size */
    headersize = 14 + 40 + cmap_entries * 4; /* Header and colormap */
    bfSize = headersize + (JPEGD_INT32) dest->row_width *
        (JPEGD_INT32) dec_info->actual_output_height;

    /* Set unused fields of header to 0 */
    memset(bmpfileheader, 0, sizeof(bmpfileheader));
    memset(bmpinfoheader, 0, sizeof(bmpinfoheader));

    /* Fill the file header */
    bmpfileheader[0] = 0x42;	/* first 2 bytes are ASCII 'B', 'M' */
    bmpfileheader[1] = 0x4D;
    PUT_4B(bmpfileheader, 2, bfSize); /* bfSize */
    /* we leave bfReserved1 & bfReserved2 = 0 */
    PUT_4B(bmpfileheader, 10, headersize); /* bfOffBits */

    /* Fill the info header (Microsoft calls this a BITMAPINFOHEADER) */
    PUT_2B(bmpinfoheader, 0, 40);	/* biSize */
    PUT_4B(bmpinfoheader, 4, dec_info->actual_output_width); /* biWidth */
    PUT_4B(bmpinfoheader, 8, dec_info->actual_output_height); /* biHeight */
    PUT_2B(bmpinfoheader, 12, 1);	/* biPlanes - must be 1 */
    PUT_2B(bmpinfoheader, 14, bits_per_pixel); /* biBitCount */
    /* we leave biCompression = 0, for none */
    /* we leave biSizeImage = 0; this is correct for uncompressed data */
#if 0
    if (dec_info->density_unit == 2) { /* if have density in dots/cm, then */
        PUT_4B(bmpinfoheader, 24, (JPEGD_INT32) (dec_info->X_density*100)); /* XPels/M */
        PUT_4B(bmpinfoheader, 28, (JPEGD_INT32) (dec_info->Y_density*100)); /* XPels/M */
    }
#endif
    PUT_2B(bmpinfoheader, 32, cmap_entries); /* biClrUsed */
    /* we leave biClrImportant = 0 */

    if (JFWRITE(dest->pub.output_file, bmpfileheader, 14) != (size_t) 14)
    {
        fprintf (stderr, "File write error\n");
        exit (1);
    }
    if (JFWRITE(dest->pub.output_file, bmpinfoheader, 40) != (size_t) 40)
    {
        fprintf (stderr, "File write error\n");
        exit (1);
    }

    if (cmap_entries > 0)
        write_colormap(dec_obj, dest, cmap_entries, 4);
}


void
write_os2_header (JPEGD_Decoder_Object *dec_obj, bmp_dest_ptr dest)
/* Write an OS2-style BMP file header, including colormap if needed */
{
    JPEGD_Decoder_Info  *dec_info = &dec_obj->dec_info;
    char bmpfileheader[14];
    char bmpcoreheader[12];
    JPEGD_INT32 headersize, bfSize;
    int bits_per_pixel, cmap_entries;

    /* Compute colormap size and total file size */
    {
        /* Unquantized, full color RGB */
        bits_per_pixel = 24;
        cmap_entries = 0;
    }
    /* File size */
    headersize = 14 + 12 + cmap_entries * 3; /* Header and colormap */
    bfSize = headersize + (JPEGD_INT32) dest->row_width *
        (JPEGD_INT32) dec_info->actual_output_height;

    /* Set unused fields of header to 0 */
    memset(bmpfileheader, 0, sizeof(bmpfileheader));
    memset(bmpcoreheader, 0, sizeof(bmpcoreheader));

    /* Fill the file header */
    bmpfileheader[0] = 0x42;	/* first 2 bytes are ASCII 'B', 'M' */
    bmpfileheader[1] = 0x4D;
    PUT_4B(bmpfileheader, 2, bfSize); /* bfSize */
    /* we leave bfReserved1 & bfReserved2 = 0 */
    PUT_4B(bmpfileheader, 10, headersize); /* bfOffBits */

    /* Fill the info header (Microsoft calls this a BITMAPCOREHEADER) */
    PUT_2B(bmpcoreheader, 0, 12);	/* bcSize */
    PUT_2B(bmpcoreheader, 4, dec_info->actual_output_width); /* bcWidth */
    PUT_2B(bmpcoreheader, 6, dec_info->actual_output_height); /* bcHeight */
    PUT_2B(bmpcoreheader, 8, 1);	/* bcPlanes - must be 1 */
    PUT_2B(bmpcoreheader, 10, bits_per_pixel); /* bcBitCount */

    if (JFWRITE(dest->pub.output_file, bmpfileheader, 14) != (size_t) 14)
    {
        fprintf (stderr, "File write error\n");
        exit (1);
    }
    if (JFWRITE(dest->pub.output_file, bmpcoreheader, 12) != (size_t) 12)
    {
        fprintf (stderr, "File write error\n");
        exit (1);
    }

    if (cmap_entries > 0)
        write_colormap(dec_obj, dest, cmap_entries, 3);
}


/*
 * Write the colormap.
 * Windows uses BGR0 map entries; OS/2 uses BGR entries.
 */

void
write_colormap (JPEGD_Decoder_Object *dec_obj, bmp_dest_ptr dest,
		int map_colors, int map_entry_size)
{
  FILE * outfile = dest->pub.output_file;
  int i;

  {
    /* If no colormap, must be grayscale data.  Generate a linear "map". */
    for (i = 0; i < 256; i++) {
      putc(i, outfile);
      putc(i, outfile);
      putc(i, outfile);
      if (map_entry_size == 4)
	putc(0, outfile);
    }
  }
  /* Pad colormap with zeros to ensure specified number of colormap entries */ 
  if (i > map_colors)
  {
      fprintf (stderr, "Too many color map entries for BMP format\n");
      exit (1);
  }
  for (; i < map_colors; i++) {
    putc(0, outfile);
    putc(0, outfile);
    putc(0, outfile);
    if (map_entry_size == 4)
      putc(0, outfile);
  }
}

/*
 * Startup: normally writes the file header.
 * In this module we may as well postpone everything until finish_output.
 */

void
start_output_bmp (JPEGD_Decoder_Object *dec_obj, djpeg_dest_ptr dinfo)
{
  /* no work here */
}

void
finish_output_bmp (JPEGD_Decoder_Object *dec_obj, djpeg_dest_ptr dinfo)
{
    JPEGD_Decoder_Info  *dec_info = &dec_obj->dec_info;
    bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
    register FILE * outfile = dest->pub.output_file;
    JPEGD_UINT8 **image_ptr;
    register JPEGD_UINT8 *data_ptr;
    int row;
    register int col;

    /* Write the header and colormap */
    if (dest->is_os2)
        write_os2_header(dec_obj, dest);
    else
        write_bmp_header(dec_obj, dest);

    /* Write the file body from our virtual array */
    for (row = dec_info->actual_output_height; row > 0; row--) {
        image_ptr = dest->whole_image + row-1;
        data_ptr = image_ptr[0];
        for (col = dest->row_width; col > 0; col--) {
            putc(*data_ptr, outfile);
            data_ptr++;
        }
    }
    
    /* Free the memory */
    free (dest->whole_image[0]);
    free (dest->whole_image);
    free (dest);

    /* Make sure we wrote the output file OK */
    fflush(outfile);
    if (ferror(outfile))
    {
        fprintf (stderr, "File write error\n");
        exit (1);
    }
}

/*
 * The module selection routine for BMP format output.
 */
djpeg_dest_ptr
jinit_write_bmp (JPEGD_Decoder_Object *dec_obj, int is_os2)
{
    JPEGD_Decoder_Params *dec_param = &dec_obj->dec_param;
    JPEGD_Decoder_Info  *dec_info = &dec_obj->dec_info;
    bmp_dest_ptr dest;
    int row_width;

    /* Create module interface object, fill in method pointers */
    dest = (bmp_dest_ptr) malloc (sizeof(bmp_dest_struct));
    dest->pub.start_output = start_output_bmp;
    dest->pub.finish_output = finish_output_bmp;
    dest->is_os2 = is_os2;

    if ((dec_param->output_format == JPEGD_OFMT_RGB_565) ||
        (dec_param->output_format == JPEGD_OFMT_RGB_888)||
        (dec_param->output_format == JPEGD_OFMT_BGR_565) ||
        (dec_param->output_format == JPEGD_OFMT_BGR_888))
    {
#if 0
        dest->pub.put_pixel_rows = put_gray_rows_bmp;
#endif
        dest->pub.put_pixel_rows = put_pixel_rows_bmp;
    }
    else
    {
        fprintf (stderr, "BMP output must be RGB\n");
        exit (1);
    }


    /* Determine width of rows in the BMP file (padded to 4-byte boundary). */
    row_width = dec_info->actual_output_width * 3;
    dest->data_width = row_width;
    while ((row_width & 3) != 0) row_width++;
    dest->row_width = row_width;
    dest->pad_bytes = (int) (row_width - dest->data_width);

    {
        JPEGD_UINT8 **result;
        JPEGD_UINT8 *workspace;
        size_t size;
        int row;

        result = (JPEGD_UINT8 **) malloc (dec_info->actual_output_height *
                                    sizeof(JPEGD_UINT8 *));
        dest->whole_image = result;
        size = (size_t) ((size_t)dec_info->actual_output_height *
                         (size_t)row_width * sizeof(JPEGD_UINT8));
        workspace = (JPEGD_UINT8 *) malloc (size);
        for (row = 0; row < dec_info->actual_output_height; row++)
        {
            result[row] = workspace;
            workspace += row_width;
        }
    }

    dest->cur_output_row = 0;

    return (djpeg_dest_ptr) dest;
}
