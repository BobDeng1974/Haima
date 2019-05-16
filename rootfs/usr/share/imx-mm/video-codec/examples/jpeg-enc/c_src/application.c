/*##############################################################################
             Copyright(C) 2001-2002, Motorola India Electronics Pvt Ltd.
                       Motorola Confidential Proprietary.
                              All rights reserved.
--------------------------------------------------------------------------------
Filename    : application.c

Description : This is a sample application code. This file is not a part of
                JPEG encoder libraray. This file will help an
                application programmer understand how a JPEG encoder is
                to be called.

Author(s)   : Harsha Deeph G (harshad@miel.mot.com)

Context     :

Caution     :

**********************************************************************
 *
 * (C) 2004 MOTOROLA INDIA ELECTRONICS LTD.
 *
 * CHANGE HISTORY
 *
 * dd/mm/yy   Description                                Author
 * --------   -----------                                ------
 * 16/03/04   Created.                                   Harsha Deeph G
 *
 * 18/03/04   Added init and other functions             Harsha Deeph G
 *
 * 21/03/04   Added a setup to open different            Ganesh Kumar C
 *            input files and accordingly
 *            allocate buffer for input
 *
 * 22/03/04   Removed interlaved from params             Harsha Deeph G
 *
 * 10/05/04   Made the application to take               Ganesh Kumar C
 *            command line arguments. Also
 *            added code for basic error checks
 *            and to print the usage of this
 *            application
 * 18/11/04   Added code for parsing 'progressive'      Ganesh Kumar C
 *            switch and updating the  mode in
 *            param structure
 *
 * 09/11/04   Added exif file format support.            Gauri Deshpande
 *            Added fill_exif_params() to initialise the
 *            parameters to be passed to codec
 * 04/12/04   Streaming o/p changes and MCU row          Harsha Deeph G
 *            level functions called for base line
 *            and progressive
 * 10/12/04   Added support for thumbnail                Harsha Deeph G
 * 26/12/04   Added JFIF params                          Harsha Deeph G
 * 26/12/04   Frame level API added                      Harsha Deeph G
 * 16/11/06   Modified for 								 Manjunath H S
 *            profile on RVDS
 *            and ELinux:TLSbo83403
 * 05/01/07   Measure Heap and Stack usage				 Manjunath H S
 *			  TLSbo87093
 * 09/02/07   Removed the output writing                 Abhishek Mehrotra
 *            when TIME_PROFILE is enabled
 ***********************************************************************/

/*
***********************************************************************
* Copyright 2005-2011 by Freescale Semiconductor, Inc.
* All modifications are confidential and proprietary information
* of Freescale Semiconductor, Inc.
* 15/12/08   ENGR00102600: add cropping and          Eagle Zhou
*                 raw data output
* 01/11/10   ENGR00133257: allow user add some non-mandatory tag info into exif header   Eagle Zhou
* 03/28/11                            add tags: orientation/flash/whitebalance 	Eagle Zhou
***********************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "jpeg_enc_interface.h"

#define TEST_RAW_DATA_OUTPUT
#define TEST_GPS_INFO
#define TEST_NON_MANDATORY_TAGS
//by anirudh_r@infosys.com to get the timing detail from the board
#ifdef TIME_PROFILE
#include <sys/time.h>
#endif

#ifdef MEASURE_STACK_USAGE
#include "mm_codec_common.h"
#endif

#ifdef MEASURE_HEAP_USAGE
    unsigned int     s32TotalMallocBytes = 0;
#endif

 extern int initlogger();
 extern int exitlogger();

JPEG_ENC_UINT8 push_output(JPEG_ENC_UINT8 ** out_buf_ptrptr,JPEG_ENC_UINT32 *out_buf_len_ptr,
    JPEG_ENC_UINT8 flush, void * context, JPEG_ENC_MODE enc_mode);


JPEG_ENC_UINT8 no_output(JPEG_ENC_UINT8 ** out_buf_ptrptr,JPEG_ENC_UINT32 *out_buf_len_ptr,
    JPEG_ENC_UINT8 flush, void * context, JPEG_ENC_MODE enc_mode);

static int
keymatch (char * arg, const char * keyword, int minchars)
/* Case-insensitive matching of (possibly abbreviated) keyword switches. */
/* keyword is the constant keyword (must be lower case already), */
/* minchars is length of minimum legal abbreviation. */
{
  register int ca, ck;
  register int nmatched = 0;

  while ((ca = *arg++) != '\0') {
    if ((ck = *keyword++) == '\0')
      return 0;			/* arg longer than keyword, no good */
    if (isupper(ca))		/* force arg to lcase (assume ck is already) */
      ca = tolower(ca);
    if (ca != ck)
      return 0;			/* no good */
    nmatched++;			/* count matched characters */
  }

  if ((ck = *keyword) != '\0')
      return 0;			/* arg shorter than keyword, no good */
  
  /* reached end of argument; fail if it's too short for unique abbrev */
  if (nmatched < minchars)
    return 0;
  return 1;			/* A-OK */
}

unsigned int outfileindx;
unsigned int raw_jpg_output;

/* The batch file that uses this executable will have to be changed if the
 * enum data type for 'mode' and 'yuv_type' is changed */
void parse_switches(jpeg_enc_parameters * params,int argc, char **argv,FILE *file_arr[5])
{
   int argn;
   char * arg;
   int qt,rm;
   int yuv_type, mode;
   int y_width,y_height,u_width,u_height,v_width,v_height, exif,temp;
   int y_left,y_top,y_total_width,y_total_height;
   int u_left,u_top,u_total_width,u_total_height;
   int v_left,v_top,v_total_width,v_total_height;

    /*Default parameters*/
    params->compression_method = JPEG_ENC_SEQUENTIAL;
    params->mode = JPEG_ENC_MAIN_ONLY;
    params->quality = 75;
    params->restart_markers = 0;
    params->y_width = 0;
    params->y_height = 0;
    params->u_width = 0;
    params->u_height = 0;
    params->v_width = 0;
    params->v_height = 0;
    params->primary_image_height = 640;
    params->primary_image_width = 480;
    params->yuv_format = JPEG_ENC_YUV_420_NONINTERLEAVED;
    params->exif_flag = 0;

    params->y_left = 0;
    params->y_top = 0;
    params->y_total_width = 0;
    params->y_total_height = 0;
    params->raw_dat_flag= 0;	

#ifdef TEST_RAW_DATA_OUTPUT
    raw_jpg_output=0;	
#endif		
    /*Parameters updated from command line*/

    for (argn = 1; argn < argc; argn++)
    {
       arg = argv[argn];

       if (keymatch(arg, "-md", 1))
       {
           sscanf(argv[++argn],"%d",&mode);
           params->mode = mode;
       }
       else if (keymatch(arg, "-prg", 1))
       {
           params->compression_method = JPEG_ENC_PROGRESSIVE;
       }
       else if (keymatch(arg, "-q", 1))
       {
           sscanf(argv[++argn],"%d",&qt);
           params->quality = qt;

       }
       else if(keymatch(arg, "-rm", 1))
       {
           sscanf(argv[++argn],"%d",&rm);
           params->restart_markers = rm;

       }
       else if(keymatch(arg, "-yt", 1))
       {
           sscanf(argv[++argn],"%d",&yuv_type);
           params->yuv_format = yuv_type;

       }
       else if(keymatch(arg, "-prw", 1))
       {
           sscanf(argv[++argn],"%d",&temp);
           params->primary_image_width = temp;
       }
       else if(keymatch(arg, "-prh", 1))
       {
           sscanf(argv[++argn],"%d",&temp);
           params->primary_image_height = temp;
       }
       else if(keymatch(arg, "-yw", 1))
       {
           sscanf(argv[++argn],"%d",&y_width);
           params->y_width = y_width;
       }
       else if(keymatch(arg, "-yh", 1))
       {
           sscanf(argv[++argn],"%d",&y_height);
           params->y_height = y_height;

       }
       else if(keymatch(arg, "-uw", 1))
       {
           sscanf(argv[++argn],"%d",&u_width);
           params->u_width = u_width;

       }
       else if(keymatch(arg, "-uh", 1))
       {
           sscanf(argv[++argn],"%d",&u_height);
           params->u_height = u_height;

       }
       else if(keymatch(arg, "-vw", 1))
       {
           sscanf(argv[++argn],"%d",&v_width);
           params->v_width = v_width;

       }
       else if(keymatch(arg, "-vh", 1))
       {
           sscanf(argv[++argn],"%d",&v_height);
           params->v_height = v_height;

       }
       else if(keymatch(arg, "-yleft", 1))
       {
           sscanf(argv[++argn],"%d",&y_left);
           params->y_left = y_left;
       }
       else if(keymatch(arg, "-ytop", 1))
       {
           sscanf(argv[++argn],"%d",&y_top);
           params->y_top = y_top;

       }
       else if(keymatch(arg, "-ytw", 1))
       {
           sscanf(argv[++argn],"%d",&y_total_width);
           params->y_total_width = y_total_width;		   

       }
       else if(keymatch(arg, "-yth", 1))
       {
           sscanf(argv[++argn],"%d",&y_total_height);
           params->y_total_height = y_total_height;

       }
       else if(keymatch(arg, "-raw", 1))
       {
           params->raw_dat_flag= 1;

       }	   
#ifdef TEST_RAW_DATA_OUTPUT	   
       else if(keymatch(arg, "-jpg", 1))
       {
           //application will write header(.jpg output) for raw data
           raw_jpg_output=1;
       }	   
#endif
#ifndef RVDS_TEST
       else if(keymatch(arg, "-ni", 1))
       {
          file_arr[0] = fopen(argv[++argn],"rb");
          file_arr[1] = fopen(argv[++argn],"rb");
          file_arr[2] = fopen(argv[++argn],"rb");
       }
#else
	   else if(keymatch(arg, "-ni", 1))
       {
       	  char yname[100],uname[100],vname[100];
       	  strcpy (yname, argv[++argn]);
       	  strcat (yname, ".ycomp");
       	  strcpy (uname, argv[argn]);
       	  strcat (uname, ".ucomp");
       	  strcpy (vname, argv[argn]);
       	  strcat (vname, ".vcomp");
          file_arr[0] = fopen(yname,"rb");
          file_arr[1] = fopen(uname,"rb");
          file_arr[2] = fopen(vname,"rb");
       }
#endif
       else if(keymatch(arg, "-i", 1))
       {
          file_arr[3] = fopen(argv[++argn],"rb");
       }
       else if(keymatch(arg, "-o", 1))
       {
          file_arr[4] = fopen(argv[++argn],"wb");
          outfileindx = argn;

       }
       else if(keymatch(arg, "-ex", 1))
       {
           sscanf(argv[++argn], "%d", &exif);
           params->exif_flag = exif;
       }
       else if(keymatch(arg, "-h", 1))
       {
#ifndef RVDS_TEST
          printf("Elinux Usage : jenc -yt=yuv_type <val 0..6> -q=quality <val 0..100> -rm=restart_markers <val 0..1> -yw=y_width <val> -yh=y_height <val> -uw=u_width <val> -uh=u_height <val>  -vw=v_width <val> -vh=v_height <val> -yleft=y_left <val> -ytop=y_top <val> -ytw=y_total_width <val> -yth=y_total_height <val> -ni/-i=non_inter/inter <yfilename> <ufilename> <vfilename> / <ifilename> -o=outfile <outputfilename> -ex=exif <val 0/1> -h=help usage\n");
#else
		  printf("RVDS Usage : jenc -yt=yuv_type <val 0..6> -q=quality <val 0..100> -rm=restart_markers <val 0..1> -yw=y_width <val> -yh=y_height <val> -uw=u_width <val> -uh=u_height <val>  -vw=v_width <val> -vh=v_height <val> -yleft=y_left <val> -ytop=y_top <val> -ytw=y_total_width <val> -yth=y_total_height <val> -ni/-i=non_inter/inter <filename without extensions> / <ifilename> -o=outfile <outputfilename> -ex=exif <val 0/1> -h=help usage\n");
#endif
          printf("Refer to 'jpeg_enc_interface.h' for yuv types\n");
          printf("Example Usage \n");
          printf("\njenc -yt 2 -q 75 -rm 0 -yw 480 -yh 640  -uw 240 -uh 320  -vw 240 -vh 320 -yleft 10 -ytop 10 -ytw 500 -yth 660 -ni y.dat u.dat v.dat -o test_bike.jpg -ex 1 \n");
          exit(-1);

       }


    }

    // check cropping ?	
    // by now, we simplify usage, information about u and v will be computed according some assumption, user need to modify their value for special case !! 
    if(params->y_total_width==0)
    {
        params->y_left=0;
	  params->u_left=0;
	  params->v_left=0;
    	  params->y_total_width=params->y_width;  // no cropping
    	  params->u_total_width=params->u_width;  // no cropping
    	  params->v_total_width=params->v_width;  // no cropping
    }
    else
    {
    	 if (params->yuv_format == JPEG_ENC_YUV_444_NONINTERLEAVED)  // 4:4:4
    	 {
    	 	params->u_left=params->y_left;
		params->v_left=params->y_left;	
		params->u_total_width=params->y_total_width;
		params->v_total_width=params->y_total_width;
    	 }
	 else		// 4:2:2 or 4:2:0
	 {
    	 	params->u_left=params->y_left>>1;
		params->v_left=params->y_left>>1;	 
		params->u_total_width=params->y_total_width>>1;
		params->v_total_width=params->y_total_width>>1;
	 }
    }

    if(params->y_total_height==0)
    {
        params->y_top=0;
	  params->u_top=0;
	  params->v_top=0;		
    	  params->y_total_height=params->y_height; // no cropping
    	  params->u_total_height=params->u_height; // no cropping
    	  params->v_total_height=params->v_height; // no cropping
    }
    else
    {
    	 if ((params->yuv_format == JPEG_ENC_YUV_444_NONINTERLEAVED)||(params->yuv_format == JPEG_ENC_YUV_422_NONINTERLEAVED))
    	 {	// 4:4:4 or 4:2:2
    	 	params->u_top=params->y_top;
		params->v_top=params->y_top;	
		params->u_total_height=params->y_total_height;
		params->v_total_height=params->y_total_height;
    	 }
	 else	// 4:2:0
	 {
    	 	params->u_top=params->y_top>>1;
		params->v_top=params->y_top>>1;	 
		params->u_total_height=params->y_total_height>>1;
		params->v_total_height=params->y_total_height>>1;
	 }
    }
	
}

void fill_exif_params(jpeg_enc_exif_parameters *params)
{
    /* tags values required from application */
    const JPEG_ENC_UINT32 XResolution[2] = { 72, 1};
    const JPEG_ENC_UINT32 YResolution[2] = { 72, 1};
    const JPEG_ENC_UINT16 ResolutionUnit = 2;
    const JPEG_ENC_UINT16 YCbCrPositioning = 1;

    /* IFD0 params */
    params->IFD0_info.x_resolution[0] = XResolution[0];
    params->IFD0_info.x_resolution[1] = XResolution[1];
    params->IFD0_info.y_resolution[0] = YResolution[0];
    params->IFD0_info.y_resolution[1] = YResolution[1];
    params->IFD0_info.resolution_unit = ResolutionUnit;
    params->IFD0_info.ycbcr_positioning = YCbCrPositioning;

    /* IFD1 params */
    params->IFD1_info.x_resolution[0] = XResolution[0];
    params->IFD1_info.x_resolution[1] = XResolution[1];
    params->IFD1_info.y_resolution[0] = YResolution[0];
    params->IFD1_info.y_resolution[1] = YResolution[1];
    params->IFD1_info.resolution_unit = ResolutionUnit;

}

#ifdef TEST_RAW_DATA_OUTPUT

#ifdef TEST_GPS_INFO
#define NUM_OF_TAGS_IFD0 6
#define NUM_OF_TAGS_GPS 7
#else
#define NUM_OF_TAGS_IFD0 5
#endif

#define NUM_OF_TAGS_EXIF_IFD 6

typedef enum
{
    XRESOLUTION_TAG = 0x011a ,
    YRESOLUTION_TAG = 0x011b ,
    RESOLUTIONUNIT_TAG = 0x0128 ,
    YCBCRPOSITIONING_TAG = 0x0213 ,
    EXIFIFDPOINTER_TAG = 0x8769 ,
    GPSIFDPOINTER_TAG =0x8825	// #ifdef  TEST_GPS_INFO
} IFD0_TAGS ;

typedef enum
{
    EXIFVERSION_TAG              =  0x9000,
    COMPONENTSCONFIGURATION_TAG  =  0x9101,
    FLASHPIXVERSION_TAG          =  0xA000,
    COLORSPACE_TAG               =  0xA001,
    PIXELXDIMENSION_TAG          =  0xA002,
    PIXELYDIMENSION_TAG          =  0xA003

} EXIFIFD_TAGS ;

#ifdef TEST_GPS_INFO
typedef enum
{
    GPSVERSION_TAG              =  0x0,
    GPSLATITUDEREF_TAG  =  0x1,
    GPSLATITUDE_TAG          =  0x2,
    GPSLONGTITUDEREF_TAG               =  0x3,
    GPSLONGTITUDE_TAG               =  0x4,
    GPSTIMESTAMP_TAG          =  0x7,
    GPSDATESTAMP_TAG          =  0x1D
} GPSIFD_TAGS ;
#endif


//enum
typedef enum 
{
    EXIF_INVALID   = 0,
    EXIF_BYTE      = 1,
    EXIF_ASCII     = 2,
    EXIF_SHORT     = 3,
    EXIF_LONG      = 4,
    EXIF_RATIONAL  = 5,
    EXIF_UNDEFINED = 7,
    EXIF_SLONG     = 9,
    EXIF_SRATIONAL = 10,
} EXIF_DATATYPES;

#define SIZE_OF_EXIF_BYTE       1
#define SIZE_OF_EXIF_ASCII      1
#define SIZE_OF_EXIF_SHORT      2
#define SIZE_OF_EXIF_LONG       4
#define SIZE_OF_EXIF_RATIONAL   8   
#define SIZE_OF_EXIF_UNDEFINED  1
#define SIZE_OF_EXIF_SLONG      4
#define SIZE_OF_EXIF_SRATIONAL  8

#define write_byte(fp, val) fputc((val),fp)


int tags_offset;	
void write_2bytes(FILE* fp, int val)
{
	/* Emit a 2-byte integer these are always MSB first in JPEG files  */
	write_byte(fp, (val >> 8) & 0xFF);
	write_byte(fp, val & 0xFF);
}

void write_4bytes(FILE* fp, int val)
{
	/* Emit a 4-byte integer; these are always MSB first in JPEG files */
	write_byte(fp, (val >> 24) & 0xFF);
	write_byte(fp, (val >>16 ) & 0xFF);
	write_byte(fp, (val >> 8) & 0xFF);
	write_byte(fp, val & 0xFF);
}

void write_marker(FILE* fp, int val)
{
	/* Emit a marker code */
	write_byte(fp, 0xFF);
	write_byte(fp, val);
}

void write_jfif_app0(FILE* fp,jpeg_enc_parameters *params)
{
	/* Emit a JFIF-compliant APP0 marker */
	/*
	* Length of APP0 block	(2 bytes)
	* Block ID			(4 bytes - ASCII "JFIF")
	* Zero byte			(1 byte to terminate the ID string)
	* Version Major, Minor	(2 bytes - major first)
	* Units			(1 byte - 0x00 = none, 0x01 = inch, 0x02 = cm)
	* Xdpu			(2 bytes - dots per unit horizontal)
	* Ydpu			(2 bytes - dots per unit vertical)
	* Thumbnail X size		(1 byte)
	* Thumbnail Y size		(1 byte)
	*/

	write_marker(fp, 0xe0); //M_APP0

	write_2bytes(fp, 2 + 4 + 1 + 2 + 1 + 2 + 2 + 1 + 1); /* length */

	write_byte(fp, 0x4A);	/* Identifier: ASCII "JFIF" */
	write_byte(fp, 0x46);
	write_byte(fp, 0x49);
	write_byte(fp, 0x46);

	write_byte(fp, 0);

	write_byte(fp, 1); /* major Version fields :  Default JFIF version = 1.01 */
	write_byte(fp, 1); /* minor version */

	write_byte(fp, params->jfif_params.density_unit); /* Pixel size information */

	write_2bytes(fp, params->jfif_params.X_density); /*X: Pixel aspect ratio */
	write_2bytes(fp, params->jfif_params.Y_density); /*Y: Pixel aspect ratio */

	write_byte(fp, 0);		/* No thumbnail image */
	write_byte(fp, 0);
}


void write_exif_app1_header(FILE* fp)
{
	int size_app1_main_only;
	write_marker(fp, 0xe1); //M_APP1

	/* 'size_of_app1' is the total size of the APP1 marker when
	* there is no thumbnail. Observe that this value keeps
	* changing as and when we add more tags */
#ifdef TEST_GPS_INFO
	size_app1_main_only = 2+6+8+2+12*NUM_OF_TAGS_IFD0+4+4*2+4*2+2+12*NUM_OF_TAGS_EXIF_IFD+4+(2+12*NUM_OF_TAGS_GPS+4+3*3*SIZE_OF_EXIF_RATIONAL+11);
#else
	size_app1_main_only = 2+6+8+2+12*NUM_OF_TAGS_IFD0+4+4*2+4*2+2+12*NUM_OF_TAGS_EXIF_IFD+4;
#endif	
	write_2bytes(fp, size_app1_main_only);

	write_byte(fp, 0x45);            /* Identifier: Exif (ASCII) */
	write_byte(fp, 0x78);
	write_byte(fp, 0x69);
	write_byte(fp, 0x66);
	write_byte(fp, 0x00);
	write_byte(fp, 0x00);

	/* TIFF header */
	write_byte(fp, 0x4D);            /* 2 bytes define endiannes: 4D4d - big endian */
	write_byte(fp, 0x4D);
	write_byte(fp, 0x00);            /* 2bytes: 42 (fixed) */
	write_byte(fp, 0x2A);
	write_byte(fp, 0x00);            /* 4 bytes: offset to 0th IFD */
	write_byte(fp, 0x00);
	write_byte(fp, 0x00);
	write_byte(fp, 0x08);

}

void write_exif_IFD0(FILE* fp,jpeg_enc_exif_parameters* exif_params)
{

	jpeg_enc_IFD0_appinfo *ifd0_info = &(exif_params->IFD0_info);
	unsigned int i;
	//int tags_offset;	

	// sizeof tiffheader (8) + 2 (2bytes to store num of tags)
	// + size of all tags + 4 (4B to store Next_IFD_offset
	tags_offset = 8 + 2 + 12 * NUM_OF_TAGS_IFD0 + 4 ;

	write_2bytes (fp, NUM_OF_TAGS_IFD0) ;

	// XResolution
	write_2bytes (fp, XRESOLUTION_TAG) ; // jpeg_enc_tag
	write_2bytes (fp, EXIF_RATIONAL) ;    // data type
	write_4bytes (fp, 1) ; // count
	write_4bytes (fp, tags_offset) ;
	tags_offset +=  SIZE_OF_EXIF_RATIONAL;

	// NOTE: extra data (data greater than 4 bytes is written after writing all tags

	// YResolution
	write_2bytes (fp, YRESOLUTION_TAG) ; // jpeg_enc_tag
	write_2bytes (fp, EXIF_RATIONAL) ;    // data type
	write_4bytes (fp, 1) ; // count
	write_4bytes (fp, tags_offset) ;
	tags_offset +=  SIZE_OF_EXIF_RATIONAL;

	// ResolutionUnit
	write_2bytes (fp, RESOLUTIONUNIT_TAG) ; // jpeg_enc_tag
	write_2bytes (fp, EXIF_SHORT) ;    // data type
	write_4bytes (fp, 1) ; // count
	write_2bytes (fp, ifd0_info->resolution_unit) ;
	write_2bytes (fp, 0) ;

	// YCbCrPositioning
	write_2bytes (fp, YCBCRPOSITIONING_TAG) ; // jpeg_enc_tag
	write_2bytes (fp, EXIF_SHORT) ;    // data type
	write_4bytes (fp, 1) ; // count
	write_2bytes (fp, ifd0_info->ycbcr_positioning) ;
	write_2bytes (fp, 0) ;

	// ExifIfdPointer
	write_2bytes (fp, EXIFIFDPOINTER_TAG) ; // jpeg_enc_tag
	write_2bytes (fp, EXIF_LONG) ;    // data type
	write_4bytes (fp, 1) ; // count
	write_4bytes (fp, tags_offset);  // exifIFD follows after IFD0 data.

#ifdef TEST_GPS_INFO
	tags_offset +=  2+12*NUM_OF_TAGS_EXIF_IFD+4; //+ rational data ?
	// GPSPointer
	write_2bytes (fp, GPSIFDPOINTER_TAG) ; // jpeg_enc_tag
	write_2bytes (fp, EXIF_LONG) ;    // data type
	write_4bytes (fp, 1) ; // count
	write_4bytes (fp, tags_offset);  // exifIFD follows after IFD0 data.
	tags_offset -=  2+12*NUM_OF_TAGS_EXIF_IFD+4; //keep same with jpeg_enc_emit_exifIFD()
#endif

	write_4bytes (fp, 0) ; /* No Thumbnail and no IFD1 */

	// write data greater than 4 bytes here
	// x_resolution 
	for (i = 0 ; i < 2 ; i++)
	{
		write_4bytes (fp, ifd0_info->x_resolution[i]) ;
	}

	// y_resolution 
	for (i = 0 ; i < 2 ; i++)
	{
		write_4bytes (fp, ifd0_info->y_resolution[i]) ;
	}

}


void write_exifIFD(FILE* fp, jpeg_enc_parameters * params)
{
	/* 
	* jpeg_enc_exifIFD_appinfo *exififd_info = &(cinfo->params->exif_params.exififd_info);
	* Not used now, maybe required later 
	*/
	unsigned char exif_version[] = {0x30, 0x32, 0x32, 0x30};     // ASCII: 02.20
	unsigned char flashpix_version[] = {0x30, 0x31, 0x30, 0x30}; // ASCII: 01.00 
	unsigned char comp_config[] = {1, 2, 3, 0};   // Y, Cb, Cr, doesn't exist
	unsigned short COLORSPACE = 1;
	//int tags_offset;

	/* Current EXIf IFD does not use 'tags_offset. Future versions
	* may use as more tags get added. This will be used in
	* IFD1 */
	tags_offset += 2 + 12*NUM_OF_TAGS_EXIF_IFD + 4 ;
	// 2 (2bytes to store num of tags)
	// + size of all tags + 4 (4B to store Next_IFD_offset

	write_2bytes (fp, NUM_OF_TAGS_EXIF_IFD) ;

	// ExifVersion
	write_2bytes (fp, EXIFVERSION_TAG) ; // jpeg_enc_tag
	write_2bytes (fp, EXIF_UNDEFINED) ;    // data type
	write_4bytes (fp, 4) ; // count
	write_byte (fp, exif_version[0]) ;
	write_byte (fp, exif_version[1]) ;
	write_byte (fp, exif_version[2]) ;
	write_byte (fp, exif_version[3]) ;

	// COMPONENTSCONFIGURATION_TAG
	write_2bytes (fp, COMPONENTSCONFIGURATION_TAG) ; // jpeg_enc_tag
	write_2bytes (fp, EXIF_UNDEFINED) ;    // data type
	write_4bytes (fp, 4) ; // count
	write_byte (fp, comp_config[0]) ;
	write_byte (fp, comp_config[1]) ;
	write_byte (fp, comp_config[2]) ;
	write_byte (fp, comp_config[3]) ;

	// FLASHPIXVERSION_TAG
	write_2bytes (fp, FLASHPIXVERSION_TAG) ; // jpeg_enc_tag
	write_2bytes (fp, EXIF_UNDEFINED) ;    // data type
	write_4bytes (fp, 4) ; // count
	write_byte (fp, flashpix_version[0]) ;
	write_byte (fp, flashpix_version[1]) ;
	write_byte (fp, flashpix_version[2]) ;
	write_byte (fp, flashpix_version[3]) ;

	// COLORSPACE_TAG             
	write_2bytes (fp, COLORSPACE_TAG) ; // jpeg_enc_tag
	write_2bytes (fp, EXIF_SHORT) ;    // data type
	write_4bytes (fp, 1) ; // count
	write_2bytes (fp, COLORSPACE) ;
	write_2bytes (fp, 0) ;

	// PIXELXDIMENSION_TAG
	// EXIF Image Width of Primary Image
	write_2bytes (fp, PIXELXDIMENSION_TAG) ; // jpeg_enc_tag
	write_2bytes (fp, EXIF_SHORT) ;    // data type
	write_4bytes (fp, 1) ; // count
	write_2bytes(fp, params->primary_image_width);
	write_2bytes (fp, 0) ;

	// PIXELYDIMENSION_TAG
	// EXIF Image Height of Primary Image
	write_2bytes (fp, PIXELYDIMENSION_TAG) ; // jpeg_enc_tag
	write_2bytes (fp, EXIF_SHORT) ;    // data type
	write_4bytes (fp, 1) ; // count
	write_2bytes(fp, params->primary_image_height);
	write_2bytes (fp, 0) ;

	write_4bytes(fp, 0) ; // offset to next IFD. NO IFD is pointed by exif ifd
}

#ifdef TEST_GPS_INFO
void write_GPSIFD(FILE* fp)
{

    unsigned int version;
    unsigned char latitude_ref,longtitude_ref;	
    int latitude_degree,latitude_minute,latitude_second;
    int longtitude_degree,longtitude_minute,longtitude_second;
    int hour,minute,seconds;
    int year,month,day;	
    //int tags_offset;

    version = 0x02000000;		
    latitude_ref='N';
    longtitude_ref='E';
    latitude_degree=108;
    latitude_minute=8;
    latitude_second=10;
    longtitude_degree=210;
    longtitude_minute=12;
    longtitude_second=20;
    hour=13;
    minute=14;
    seconds=15;
    year=2008;
    month=12;
    day=04;		
    /* Current GPS does not use 'tags_offset. Future versions
     * may use as more tags get added. This will be used in
     * IFD1 */
    tags_offset += 2 + 12*NUM_OF_TAGS_GPS + 4 ;
                    // 2 (2bytes to store num of tags)
                    // + size of all tags + 4 (4B to store Next_IFD_offset)

    write_2bytes (fp, NUM_OF_TAGS_GPS) ;

    // GPSVersionID
    write_2bytes (fp, GPSVERSION_TAG) ; // jpeg_enc_tag
    write_2bytes (fp, EXIF_BYTE) ;    // data type
    write_4bytes (fp, 4) ; // count
    write_4bytes (fp, version) ;

    //GPSLatitudeRef		
    write_2bytes (fp, GPSLATITUDEREF_TAG) ; // jpeg_enc_tag
    write_2bytes (fp, EXIF_ASCII) ;    // data type
    write_4bytes (fp, 2) ; // count
    write_byte (fp,latitude_ref) ;
    write_byte (fp, 0) ;
    write_2bytes (fp, 0) ;
	
    //GPSLatitude
    write_2bytes (fp, GPSLATITUDE_TAG) ; // jpeg_enc_tag
    write_2bytes (fp, EXIF_RATIONAL) ;    // data type
    write_4bytes (fp, 3) ; // count
    write_4bytes (fp, tags_offset) ;
   tags_offset += 3* SIZE_OF_EXIF_RATIONAL;

    //GPSLongtitudeRef
    write_2bytes (fp, GPSLONGTITUDEREF_TAG) ; // jpeg_enc_tag
    write_2bytes (fp, EXIF_ASCII) ;    // data type
    write_4bytes (fp, 2) ; // count
    write_byte (fp, longtitude_ref) ;
    write_byte (fp, 0) ;
    write_2bytes (fp, 0) ;

    //GPSLongitude
    write_2bytes (fp, GPSLONGTITUDE_TAG) ; // jpeg_enc_tag
    write_2bytes (fp, EXIF_RATIONAL) ;    // data type
    write_4bytes (fp, 3) ; // count
    write_4bytes (fp, tags_offset) ;
    tags_offset += 3* SIZE_OF_EXIF_RATIONAL;    

    //GPSTimeStamp
    write_2bytes (fp, GPSTIMESTAMP_TAG) ; // jpeg_enc_tag
    write_2bytes (fp, EXIF_RATIONAL) ;    // data type
    write_4bytes (fp, 3) ; // count
    write_4bytes (fp, tags_offset) ;
    tags_offset += 3* SIZE_OF_EXIF_RATIONAL;

    //GPSDateStamp
    write_2bytes (fp, GPSDATESTAMP_TAG) ; // jpeg_enc_tag
    write_2bytes (fp, EXIF_ASCII) ;    // data type
    write_4bytes (fp, 11) ; // count
    write_4bytes (fp, tags_offset) ;
    tags_offset += 11;//3* SIZE_OF_EXIF_RATIONAL;	

    write_4bytes(fp, 0) ; // offset to next IFD. NO IFD is pointed by gps ifd

    //write GPSLatitude	: 3*rational_size = 24 bytes : dd/1,mm/1,ss/1
    write_4bytes(fp,latitude_degree);
    write_4bytes(fp,1);
    write_4bytes(fp,latitude_minute);
    write_4bytes(fp,1);
    write_4bytes(fp,latitude_second);
    write_4bytes(fp,1);
    
    //write GPSLongitude : 3*rational_size = 24 bytes : ddd/1,mm/1,ss/1	
    write_4bytes(fp,longtitude_degree);
    write_4bytes(fp,1);
    write_4bytes(fp,longtitude_minute);
    write_4bytes(fp,1);
    write_4bytes(fp,longtitude_second);
    write_4bytes(fp,1);

    //write GPSTimeStamp : 3*rational_size = 24 bytes : hh/1,mm/1,ss/1
    write_4bytes(fp,hour);
    write_4bytes(fp,1);
    write_4bytes(fp,minute);
    write_4bytes(fp,1);
    write_4bytes(fp,seconds);
    write_4bytes(fp,1);    

    //write GPSDateStamp : 11 bytes:  "YYYY:MM:DD "
    //write_4bytes(fp,year);
    write_byte(fp,'2');
    write_byte(fp,'0');	
    write_byte(fp,'0');	
    write_byte(fp,'8');	
    write_byte(fp,':');
    //write_2bytes(fp,month);
    write_byte(fp,'1');
    write_byte(fp,'2');
    write_byte(fp,':');
    //write_2bytes(fp,day);
    write_byte(fp,'0');
    write_byte(fp,'4');    
    write_byte(fp,0);    
	
}

#endif

void write_exif_app1(FILE* fp,jpeg_enc_parameters * params)
{
	/* Emit a EXIF-compliant APP1 marker */
	write_exif_app1_header(fp);
	write_exif_IFD0(fp,&params->exif_params);
	write_exifIFD(fp,params);
#ifdef TEST_GPS_INFO
	write_GPSIFD(fp);
#endif
	//no IFD1(No thumbnail image)
}

write_jfif_file_header (FILE* fp,jpeg_enc_parameters * params)
{
	write_marker(fp, 0xd8);	/* SOI */
	write_jfif_app0(fp,params);
}

write_exif_file_header (FILE* fp,jpeg_enc_parameters * params)
{
	write_marker(fp, 0xd8);	/* SOI */
	write_exif_app1(fp,params);        
}


void write_file_trailer (FILE* fp)
{
	write_marker(fp, 0xd9); //EOI
}

#endif //TEST_RAW_DATA_OUTPUT


#ifdef TEST_NON_MANDATORY_TAGS
void test_non_mandatory_tags(jpeg_enc_object * obj_ptr)
{
	jpeg_enc_make_info make_info;
	jpeg_enc_makernote_info makernote_info;
	jpeg_enc_model_info model_info;
	jpeg_enc_datetime_info datetime_info;
	jpeg_enc_focallength_info focallength_info;
	jpeg_enc_gps_info gps_info;
	unsigned char orientation_info;
	unsigned char flash_info;
	unsigned char whitebalance_info;

	//set makernote info
	make_info.make_bytes=8;
	memcpy(&make_info.make,"fsl_make",make_info.make_bytes);
	//set makernote info
	makernote_info.makernote_bytes=13;
	memcpy(&makernote_info.makernote,"fsl_makernote",makernote_info.makernote_bytes);
	//set model info
	model_info.model_bytes=9;
	memcpy(&model_info.model,"fsl_model",model_info.model_bytes);
	//set datetime
	memcpy(&datetime_info.datetime,"2010:11:12 18:19:20 " ,20);  //"YYYY:MM:DD HH:MM:SS"
	//set focallength info
	focallength_info.numerator=1;
	focallength_info.denominator=1000;
	//set gps info
	gps_info.version=0x02020000;	//default: 2.2.0.0
	memcpy(&gps_info.latitude_ref,"N ",2);
	memcpy(&gps_info.longtitude_ref,"E ",2);
	//latitude: dd/1,mm/1,ss/1
	gps_info.latitude_degree[0]=100;
	gps_info.latitude_degree[1]=1;
	gps_info.latitude_minute[0]=101;
	gps_info.latitude_minute[1]=1;	
	gps_info.latitude_second[0]=102;
	gps_info.latitude_second[1]=1;	
	//longtitude: dd/1,mm/1,ss/1
	gps_info.longtitude_degree[0]=200;
	gps_info.longtitude_degree[1]=1;
	gps_info.longtitude_minute[0]=201;
	gps_info.longtitude_minute[1]=1;
	gps_info.longtitude_second[0]=202;	
	gps_info.longtitude_second[1]=1;	
	//altitude(meters): aa/1
	gps_info.altitude_ref=0;		// 0: up sea level; 1: below sea level
	gps_info.altitude[0]=1000;
	gps_info.altitude[1]=1;
	//timestamp: hh/1,mm/1,ss/1
	gps_info.hour[0]=19;
	gps_info.hour[1]=1;
	gps_info.minute[0]=20;
	gps_info.minute[1]=1;
	gps_info.seconds[0]=21;
	gps_info.seconds[1]=1;
	//gps_info.processmethod_bytes=21;
	//memcpy(&gps_info.processmethod,"fsl_processing_method",gps_info.processmethod_bytes);	
	gps_info.processmethod_bytes=32;
	memcpy(&gps_info.processmethod,"GPS NETWORK HYBRID ARE ALL FINE.",gps_info.processmethod_bytes);
	memcpy(&gps_info.datestamp,"2011:12:13 ",11); //"YYYY:MM:DD "

	orientation_info=5;		/*5 = The 0th row is the visual left-hand side of the image, and the 0th column is the visual top.*/
	flash_info=9;			/*0009.H = Flash fired, compulsory flash mode*/
	whitebalance_info=1;	/*1 = Manual white balance*/

	jpeg_enc_set_exifheaderinfo(obj_ptr, JPEGE_ENC_SET_HEADER_MAKE, (unsigned int)(&make_info));	
	jpeg_enc_set_exifheaderinfo(obj_ptr, JPEGE_ENC_SET_HEADER_MAKERNOTE, (unsigned int)(&makernote_info));
	jpeg_enc_set_exifheaderinfo(obj_ptr, JPEGE_ENC_SET_HEADER_MODEL, (unsigned int)(&model_info));
	jpeg_enc_set_exifheaderinfo(obj_ptr, JPEGE_ENC_SET_HEADER_DATETIME, (unsigned int)(&datetime_info));
	jpeg_enc_set_exifheaderinfo(obj_ptr, JPEGE_ENC_SET_HEADER_FOCALLENGTH, (unsigned int)(&focallength_info));
	jpeg_enc_set_exifheaderinfo(obj_ptr, JPEGE_ENC_SET_HEADER_GPS, (unsigned int)(&gps_info));

	jpeg_enc_set_exifheaderinfo(obj_ptr, JPEGE_ENC_SET_HEADER_ORIENTATION, (unsigned int)(&orientation_info));
	jpeg_enc_set_exifheaderinfo(obj_ptr, JPEGE_ENC_SET_HEADER_FLASH, (unsigned int)(&flash_info));
	jpeg_enc_set_exifheaderinfo(obj_ptr, JPEGE_ENC_SET_HEADER_WHITEBALANCE, (unsigned int)(&whitebalance_info));

	return;
}
#endif

FILE *fp_out;

int main(int argc, char **argv)
{

	//by anirudh_r@infosys.com to get the timing detail from the board
	/* this structure has been declared to get the timing deatil while
	                   running on board */

	#ifdef MEASURE_STACK_USAGE
       unsigned int           *ps32SP, *ps32BaseSP;
       unsigned int           s32StackCount, s32StackValue;
       unsigned int           s32PeakStack = 0;
	#endif

	#ifdef TIME_PROFILE
		struct timeval StartTime, EndTime;
		unsigned long TotalDecTimeUs = 0;
		unsigned int n_frames = 1,Max_time = 0,Min_time = 0,f_Max_time = 0,f_Min_time = 0;
		unsigned char chname[] = "[PROFILE-INFO]";
	#endif

#ifdef TIME_PROFILE_RVDS
	int prev_clk, curr_clk,clk,n_frames=1;
	int total_clk =0,max_clk=0,min_clk=0,f_max_clk=0,f_min_clk=0;
	extern int prev_cycles(void);
	extern int curr_cycles(void);
	unsigned char chname[] = "[PROFILE-INFO]";
#endif


    JPEG_ENC_UINT8 * i_buff;
    JPEG_ENC_UINT8 * y_buff;
    JPEG_ENC_UINT8 * u_buff;
    JPEG_ENC_UINT8 * v_buff;
    JPEG_ENC_RET_TYPE return_val;
    jpeg_enc_parameters * params;
    FILE *Y,*U,*V,*I;
    FILE *file_arr[5];// =  {Y,U,V,I,fp_out};
    JPEG_ENC_UINT16 i;
    jpeg_enc_object * obj_ptr;
    JPEG_ENC_UINT8 number_mem_info;
    jpeg_enc_memory_info * mem_info;
    JPEG_ENC_UINT32 *offset_tbl_ptr;
    JPEG_ENC_UINT8 *value_tbl_ptr;
    JPEG_ENC_UINT8 num_entries;

#ifdef MEASURE_HEAP_USAGE
    s32TotalMallocBytes = 0;
#endif

    initlogger();

    /* --------------------------------------------
     * Allocate memory for Encoder Object
     * -------------------------------------------*/
    obj_ptr = (jpeg_enc_object *) malloc(sizeof(jpeg_enc_object));

    /* Assign the function for streaming output */


    obj_ptr->jpeg_enc_push_output = push_output;

    #ifdef TIME_PROFILE_RVDS
    obj_ptr->jpeg_enc_push_output = no_output;
    #endif

    #ifdef TIME_PROFILE
    obj_ptr->jpeg_enc_push_output = no_output;
    #endif
    obj_ptr->context=NULL;   //user can put private variables into it
    /* --------------------------------------------
     * Fill up the parameter structure of JPEG Encoder
     * -------------------------------------------*/
    params = &(obj_ptr->parameters);

   file_arr[0] = NULL;
   file_arr[1] = NULL;
   file_arr[2] = NULL;
   file_arr[3] = NULL;
   file_arr[4] = NULL;

   parse_switches(params, argc, argv, file_arr);

   if (params->exif_flag)
   {
       fill_exif_params(&(params->exif_params));
   }
   else
   {
       /* Pixel size is unknown by default */
       params->jfif_params.density_unit = 0;
       /* Pixel aspect ratio is square by default */
       params->jfif_params.X_density = 1;
       params->jfif_params.Y_density = 1;
   }

   Y = file_arr[0];
   U = file_arr[1];
   V = file_arr[2];
   I = file_arr[3];
   fp_out = file_arr[4];


    if( (params->y_width == 0)  || (params->u_width == 0) || (params->v_width == 0)
        || (params->y_height == 0) || (params->u_height == 0) || (params->v_height == 0) )
    {
        printf("Invalid width or/and height specified for components\n");
        exit(-1);
    }


    /* --------------------------------------------
     * Allocate memory for Input and Output Buffers
     * -------------------------------------------*/

    if( (params->yuv_format == JPEG_ENC_YUV_444_NONINTERLEAVED) ||
        (params->yuv_format == JPEG_ENC_YUV_420_NONINTERLEAVED) ||
        (params->yuv_format == JPEG_ENC_YUV_422_NONINTERLEAVED) )
    {
        if( (Y == NULL) || (U == NULL) || (V == NULL) )
        {
            printf("Invalid non interleaved input files specified\n");
            exit(-1);
        }
        y_buff = (JPEG_ENC_UINT8 *) malloc(sizeof(JPEG_ENC_UINT8)*params->y_total_width*params->y_total_height);
        u_buff = (JPEG_ENC_UINT8 *) malloc(sizeof(JPEG_ENC_UINT8)*params->u_total_width*params->u_total_height);
        v_buff = (JPEG_ENC_UINT8 *) malloc(sizeof(JPEG_ENC_UINT8)*params->v_total_width*params->v_total_height);
        fread(y_buff,1,params->y_total_width*params->y_total_height,Y);
        fread(u_buff,1,params->u_total_width*params->u_total_height,U);
        fread(v_buff,1,params->v_total_width*params->v_total_height,V);
        i_buff = NULL;
    }
    else
    {
        if( I == NULL)
        {
            printf("Invalid interleaved input file specified\n");
            exit(-1);
        }
        i_buff = (JPEG_ENC_UINT8 *) malloc(sizeof(JPEG_ENC_UINT8)*(params->y_total_width + params->u_total_width +
                                           params->v_total_width)*(params->y_total_height));

        fread(i_buff,1,(params->y_total_width + params->u_total_width + params->v_total_width)
                       *(params->y_total_height),I);
        y_buff = u_buff = v_buff = NULL;
    }

    if( fp_out == NULL)
    {
        printf("Unable to open outputfile specified\n");
        exit(-1);
    }

    //printf api version 	
    printf("version: %s\n", jpege_CodecVersionInfo());	

#ifdef TEST_RAW_DATA_OUTPUT
    // write file header for verification of raw data output
    if((params->raw_dat_flag==1)&&(raw_jpg_output==1))
    {
         if(params->exif_flag)
         {
         	write_exif_file_header(fp_out, params);
         }
	   else
	   {
	   	write_jfif_file_header(fp_out, params);
	   }
    }
#endif	
    /* --------------------------------------------
     * QUERY MEMORY REQUIREMENTS
     * -------------------------------------------*/
    return_val = jpeg_enc_query_mem_req(obj_ptr);

    if(return_val != JPEG_ENC_ERR_NO_ERROR)
    {
        printf("JPEG encoder returned an error when jpeg_enc_query_mem_req was called \n");
        printf("Return Val %d\n",return_val);
        exit(-1);
    }

    /* --------------------------------------------
     * ALLOCATE MEMORY REQUESTED BY CODEC
     * -------------------------------------------*/

    number_mem_info = obj_ptr->mem_infos.no_entries;

    for(i = 0; i < number_mem_info; i++)
    {
        /* This example code ignores the 'alignment' and
         * 'memory_type', but some other applications might want
         * to allocate memory based on them */
        mem_info = &(obj_ptr->mem_infos.mem_info[i]);
        mem_info->memptr = (void *) malloc(mem_info->size);
		if(mem_info->memptr==NULL)
		{
			printf("Malloc error after query\n");
			return -1;
		}
		else
		{
			#ifdef MEASURE_HEAP_USAGE
		         s32TotalMallocBytes+=(mem_info->size);
	        #endif

		}
    }

    /* --------------------------------------------
     * Call encoder Init routine
     * -------------------------------------------*/

#ifdef MEASURE_STACK_USAGE
    PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
#endif

    return_val = jpeg_enc_init(obj_ptr);

#ifdef MEASURE_STACK_USAGE
    GET_STACK_USAGE (ps32BaseSP, ps32SP, s32StackCount, s32PeakStack);
#endif

    if(return_val != JPEG_ENC_ERR_NO_ERROR)
    {
        printf("JPEG encoder returned an error when jpeg_enc_init was called \n");
        printf("Return Val %d\n",return_val);
        exit(-1);
    }

    /* Temporary and later this will be removed once streaming output
     * is implemented */
    /* obj_ptr->out_buff = out_buff_ref; */

#ifdef TEST_NON_MANDATORY_TAGS
	test_non_mandatory_tags(obj_ptr);
#endif

#if 1
    /* --------------------------------------------
     * CALL JPEG ENCODER : Frame Level API
     * -------------------------------------------*/

	 #ifdef MEASURE_STACK_USAGE
             PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
     #endif

     //by anirudh_r@infosys.com to get the timing detail from the board
	 #ifdef TIME_PROFILE
	 		gettimeofday(&StartTime, NULL);
	 #endif

	 #ifdef TIME_PROFILE_RVDS
		prev_clk = prev_cycles();
	 #endif


    return_val = jpeg_enc_encodeframe(obj_ptr, i_buff,
                                      y_buff, u_buff, v_buff);


    //by anirudh_r@infosys.com to get the timing detail from the board
	#ifdef TIME_PROFILE
		 gettimeofday(&EndTime, NULL);
		 TotalDecTimeUs += (EndTime.tv_usec - StartTime.tv_usec) + (EndTime.tv_sec - StartTime.tv_sec)*1000000L;
	#endif

	#ifdef TIME_PROFILE_RVDS
		curr_clk = curr_cycles();
		clk = (curr_clk-prev_clk);      /* clk gives the total core cycles per frame call */
		total_clk += clk;
		if(clk > max_clk)
	     	max_clk = clk;
	#endif

	#ifdef MEASURE_STACK_USAGE
             GET_STACK_USAGE (ps32BaseSP, ps32SP, s32StackCount, s32StackValue);
             if(s32PeakStack < s32StackValue)
                 s32PeakStack = s32StackValue;
    #endif


    if(return_val == JPEG_ENC_ERR_ENCODINGCOMPLETE)
    {
        printf("Encoding of Image completed\n");
    }
    else
    {
        printf("JPEG encoder returned an error in jpeg_enc_encodeframe \n");
        printf("Return Val %d\n",return_val);
        exit(-1);
    }
#else
{

    JPEG_ENC_UINT8 * i_buff_ptr;
    JPEG_ENC_UINT8 * y_buff_ptr;
    JPEG_ENC_UINT8 * u_buff_ptr;
    JPEG_ENC_UINT8 * v_buff_ptr;
    JPEG_ENC_UINT8 mult_factor;

    i_buff_ptr = i_buff;
    y_buff_ptr = y_buff;
    u_buff_ptr = u_buff;
    v_buff_ptr = v_buff;

    if(params->yuv_format == JPEG_ENC_YUV_420_NONINTERLEAVED)
    {
        mult_factor = 2;
    }
    else
    {
        mult_factor = 1;
    }


    /*--------------------------------------------
     * CALL JPEG ENCODER.
     * -------------------------------------------*/

	 #ifdef MEASURE_STACK_USAGE
             PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
     #endif

     //by anirudh_r@infosys.com to get the timing detail from the board
	 #ifdef TIME_PROFILE
	 		gettimeofday(&StartTime, NULL);
	 #endif

	 #ifdef TIME_PROFILE_RVDS
		prev_clk = prev_cycles();
	 #endif


    while(1)
    {
        return_val = jpeg_enc_encodemcurow(obj_ptr, i_buff_ptr,
                                           y_buff_ptr, u_buff_ptr, v_buff_ptr);
        if(return_val != JPEG_ENC_ERR_NO_ERROR)
        {
            break;
        }
        if(i_buff_ptr == NULL)
        {
            y_buff_ptr += params->y_width*mult_factor*8;
            u_buff_ptr += params->u_width*8;
            v_buff_ptr += params->v_width*8;
        }
        else
        {
            i_buff_ptr += params->y_width*2*8;
        }
    }

     //by anirudh_r@infosys.com to get the timing detail from the board
	#ifdef TIME_PROFILE
			gettimeofday(&EndTime, NULL);
			TotalDecTimeUs += (EndTime.tv_usec - StartTime.tv_usec) + (EndTime.tv_sec - StartTime.tv_sec)*1000000L;
	#endif

	#ifdef TIME_PROFILE_RVDS
		curr_clk = curr_cycles();
		clk = (curr_clk-prev_clk);      /* clk gives the total core cycles per frame call */
		total_clk += clk;
		if(clk > max_clk)
	     	max_clk = clk;
	#endif

	#ifdef MEASURE_STACK_USAGE
             GET_STACK_USAGE (ps32BaseSP, ps32SP, s32StackCount, s32StackValue);
             if(s32PeakStack < s32StackValue)
                 s32PeakStack = s32StackValue;
    #endif


    if(return_val == JPEG_ENC_ERR_ENCODINGCOMPLETE)
    {
        printf("Encoding of Image completed\n");
    }
    else
    {
        printf("JPEG encoder returned an error when  jpeg_enc_encodemcurowas called \n");
        printf("Return Val %d\n",return_val);
        exit(-1);

    }

    if(params->compression_method == JPEG_ENC_PROGRESSIVE)
    {
        while(1)
        {
            return_val = jpeg_enc_encodepassmcurow(obj_ptr);

            if(return_val != JPEG_ENC_ERR_NO_ERROR)
            {
                break;
            }
        }

        if(return_val == JPEG_ENC_ERR_ENCODINGCOMPLETE)
        {
            printf("Encoding of Progressive Image completed\n");
        }
        else
        {
            printf("JPEG encoder returned an error when jpeg_enc_encodepassmcurow was called \n");
            printf("Return Val %d\n",return_val);
            exit(-1);
        }
    }

}
#endif

    if((params->mode == JPEG_ENC_THUMB)&&(params->raw_dat_flag==0))
    {

        /* --------------------------------------------
         * Incase of thumbnail, we have to fseek to an
         * appropriate place and write bytes. Allocate
         * memory for the offset and the val tables
         * -------------------------------------------*/

        /* Allocate memory for the offset and value tables */
        offset_tbl_ptr = (JPEG_ENC_UINT32 *)malloc(sizeof(JPEG_ENC_UINT32)*JPEG_ENC_NUM_OF_OFFSETS);
        value_tbl_ptr = (JPEG_ENC_UINT8 *)malloc(sizeof(JPEG_ENC_UINT8)*JPEG_ENC_NUM_OF_OFFSETS);
        return_val = jpeg_enc_find_length_position(obj_ptr, offset_tbl_ptr,
                                                   value_tbl_ptr,&num_entries);
        if(return_val != JPEG_ENC_ERR_NO_ERROR)
        {
            printf("Encoder returned an error when jpeg_enc_find_length_position was called \n");
            printf("Return val %d \n",return_val);
        }

        /* --------------------------------------------
         * Fseek and write bytes
         * -------------------------------------------*/
        for(i = 0; i < num_entries; i++)
        {
            if(fseek(fp_out,offset_tbl_ptr[i],SEEK_SET))
            {
                printf("fseek failed\n");
            }
            /* Overwrite at the appropriate location */
            fputc(value_tbl_ptr[i],fp_out);
        }

        /* --------------------------------------------
         * Free memory for the offset and the val tables
         * -------------------------------------------*/
        free(offset_tbl_ptr);
        free(value_tbl_ptr);
    }
#ifdef TEST_RAW_DATA_OUTPUT	
    else if((params->raw_dat_flag==1)&&(raw_jpg_output==1)) 
    {
         // write file header for verification of raw data output
         write_file_trailer(fp_out);
    }
#endif	
    /* Close Logger */
    exitlogger();

    /* --------------------------------------------
     * FREE MEMORY REQUESTED BY CODEC
     * -------------------------------------------*/
    number_mem_info = obj_ptr->mem_infos.no_entries;
    for(i = 0; i < number_mem_info; i++)
    {
        mem_info = &(obj_ptr->mem_infos.mem_info[i]);
        free(mem_info->memptr);
    }




    /* --------------------------------------------
     * Free the Input and Output buffers
     * -------------------------------------------*/
    if( (params->yuv_format == JPEG_ENC_YUV_444_NONINTERLEAVED) ||
        (params->yuv_format == JPEG_ENC_YUV_420_NONINTERLEAVED) ||
        (params->yuv_format == JPEG_ENC_YUV_422_NONINTERLEAVED) )
    {
        free(y_buff);
        free(u_buff);
        free(v_buff);
        fclose(Y);
        fclose(U);
        fclose(V);
    }
    else
    {
       free(i_buff);
       fclose(I);
    }




	//by anirudh_r@infosys.com to get the timing detail from the board
	#ifdef TIME_PROFILE
	        //printf("\n\n\n\t\tTotal Decode Time [microseconds] = %ld\n", TotalDecTimeUs);
			printf("\n%s\t%s\t %dx%d\t %dx%d\t %d\t %d\t%d\t%d\t%d\t%d\t%d\n",chname,argv[outfileindx],\
			params->y_width, params->y_height, params->y_width, params->y_height,params->quality, \
			Max_time,Min_time,n_frames,f_Max_time,f_Min_time,TotalDecTimeUs);

			//writing to file
			//fprintf(fp_out_timefile,"%ld\n",TotalDecTimeUs);

			//closing decode_time
			//fclose(fp_out_timefile);
	#endif
#ifdef __SYMBIAN32__
#ifdef MEASURE_HEAP_USAGE
			printf("\n Stack and Heap are  %d\t%d\t",s32PeakStack,s32TotalMallocBytes);
#endif
#endif

	#ifdef TIME_PROFILE_RVDS

		#ifndef MEASURE_STACK_USAGE
        #ifndef MEASURE_HEAP_USAGE

	        printf("\n%s\t%s\t %dx%d\t %dx%d\t %d\t %d\t%d\t%d\t%d\t%d\t%d\n",chname,argv[outfileindx],\
			params->y_width, params->y_height, params->y_width, params->y_height,params->quality, \
			max_clk,min_clk,n_frames,f_max_clk,f_min_clk,(total_clk*64));

		#endif
 		#endif

		#ifdef MEASURE_STACK_USAGE
        #ifdef MEASURE_HEAP_USAGE

	        printf("\n%s\t%s\t %dx%d\t %dx%d\t %d\t %d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",chname,argv[outfileindx],\
			params->y_width, params->y_height, params->y_width, params->y_height,params->quality, \
			max_clk,min_clk,n_frames,f_max_clk,f_min_clk,(total_clk*64),s32PeakStack,s32TotalMallocBytes);

		#endif
 		#endif

	#endif


    //free(out_buff_ref);
    free(obj_ptr);
    fclose(fp_out);
    return 1;
}

/* This function is an example implementation of call back
 * function 'jpeg_enc_push_output'. This function reads the
 * data from the buffer and writes the content into an
 * output file */

#define APP_OUT_BUFFER_SIZE 1000
JPEG_ENC_UINT8 APP_OUT_BUFFER[APP_OUT_BUFFER_SIZE];

JPEG_ENC_UINT8 no_output(JPEG_ENC_UINT8 ** out_buf_ptrptr,JPEG_ENC_UINT32 *out_buf_len_ptr,
    JPEG_ENC_UINT8 flush, void * context, JPEG_ENC_MODE enc_mode)
{
   JPEG_ENC_UINT32 i;
    if(*out_buf_ptrptr == NULL)
    {
        /* This function is called for the 1'st time from the
         * codec */
        *out_buf_ptrptr = APP_OUT_BUFFER;
        *out_buf_len_ptr = APP_OUT_BUFFER_SIZE;
    }

}


JPEG_ENC_UINT8 push_output(JPEG_ENC_UINT8 ** out_buf_ptrptr,JPEG_ENC_UINT32 *out_buf_len_ptr,
    JPEG_ENC_UINT8 flush, void * context, JPEG_ENC_MODE enc_mode)
{
    JPEG_ENC_UINT32 i;
    if(*out_buf_ptrptr == NULL)
    {
        /* This function is called for the 1'st time from the
         * codec */
        *out_buf_ptrptr = APP_OUT_BUFFER;
        *out_buf_len_ptr = APP_OUT_BUFFER_SIZE;
    }

    else if(flush == 1)
    {
        /* Flush the buffer*/
        /* This example code flushes the buffer into a file */
        /* File Write to be commented while profiling */

        for(i = 0; i < *out_buf_len_ptr;i++)
        {
            fputc(*(*out_buf_ptrptr + i),fp_out);
        }

    }
    else
    {

       /* File Write to be commented while profiling */

        /* This example code flushes the buffer into a file */

        for(i = 0; i < APP_OUT_BUFFER_SIZE ;i++)
        {
            fputc(*(*out_buf_ptrptr + i),fp_out);
        }

        /* Now provide a new buffer */
        *out_buf_ptrptr = APP_OUT_BUFFER;
        *out_buf_len_ptr = APP_OUT_BUFFER_SIZE;
    }

    return(1); /* Success */
}


/////////////////////////////   main entry /////////////////////////////
#ifndef WINDOWS_PLATFORM	//eagle: windows version
#ifdef __WINCE
#include "windows.h"
#define NAME_SIZE 255

int _tmain(int argc,_TCHAR *argv[])
{
	char* argv_char[NAME_SIZE];
	int argc_size,i;

	//printf("=================");
	for(i=0;i < argc; i++)
	{
		argv_char[i] = (char *) malloc(sizeof(char)*NAME_SIZE);
		argc_size=wcstombs(argv_char[i],argv[i],NAME_SIZE);
	}
	main(argc,argv_char);

	for(i=0;i < argc; i++)
	{
		free(argv_char[i]);
		argv_char[i]=NULL;
	}
    return 0;
}
#endif
#endif   //WIN32

