/*
 * jpeg_dec_app.c
 *
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains a command-line user interface for the JPEG decompressor.
 * It should work on any system with Unix- or MS-DOS-style command lines.
 *
 * Two different command line styles are permitted, depending on the
 * compile-time switch TWO_FILE_COMMANDLINE:
 *	djpeg [options]  inputfile outputfile
 *	djpeg [options]  [inputfile]
 * In the second style, output is always to standard output, which you'd
 * normally redirect to a file or pipe to some other program.  Input is
 * either from a named file or from standard input (typically redirected).
 * The second style is convenient on Unix but is unhelpful on systems that
 * don't support pipes.  Also, you MUST use the first style if your system
 * doesn't do binary I/O to stdin/stdout.
 * To simplify script writing, the "-outfile" switch is provided.  The syntax
 *	djpeg [options]  -outfile outputfile  inputfile
 * works regardless of which command line style is used.
 */
/****************************************************************************
 *
 * (C) 2003 MOTOROLA INDIA ELECTRONICS LTD.
 *
 *   CHANGE HISTORY
 *   dd/mm/yy   Code Ver    Description                         Author
 *   --------   -------     -----------                         ------
 *   26/12/03   01          Make MCU based processing.          B.Venkatarao
 *   16/01/04   02          API changes                         B.Venkatarao
 *   28/01/04   03          Error handling                      B.Venkatarao
 *   28/01/04   04          Removed unnecessary code from       B.Venkatarao
 *                          jdmaster.c
 *   29/01/04   05          Decoder instance-id is added to     B.Venkatarao
 *                          the interface of jpegd_get_new_data
 *   29/01/04   06          BMP support added.                  B.Venkatarao
 *   29/01/04   07          DCT scaling added.                  B.Venkatarao
 *   18/02/04   08          DCT scaling disabled and            B.Venkatarao
 *                          image down-scaling added.
 *   25/02/04   09          Support for RGB 16-bit output       B.Venkatarao
 *                           added.
 *   27/02/04   10          Interface changes done.             B.Venkatarao
 *   20/03/04   11          output_pixel_size is changed        B.Venkatarao
 *                           to output_format and enum is
 *                           added
 *   22/03/04   12          API changes for giving output       B.Venkatarao
 *                           format same as encoded image
 *                           format
 *   10/04/04   13          Change stride var type              B.Venkatarao
 *   10/04/04   14          Seperate function for writing       B.Venkatarao
 *                              outputs
 *   15/04/04   15          Changes to start IDCT optimization  B.Venkatarao
 *   26/04/04   16          Templating done                     B.Venkatarao
 *   11/05/04   17          Review rework for API changes       B.Venkatarao
 *   20/05/04   18          Error for wrong ofmt switch in      B.Venkatarao
 *                           parsing arguments
 *   01/06/04   19          Changes for not including library   B.Venkatarao
 *                          specific h-files
 *   07/06/04   20          Return actual size instead of       B.Venkatarao
 *                           extended size, for YUV output
 *                           format
 *   16/11/04   21          Added support for suspension        B.Venkatarao
 *   11/12/04   22          API changes, structures modified    B.Venkatarao
 *   17/12/04   23          While running on target,            B.Venkatarao
 *                          (rand() * MAX_RAND_SIZE) is
 *                          overflowing
 *   14/12/04   24          Added exif and thumbnail related    Gauri Deshpande
 *                           changes
 *   27/12/04   25          Review rework for exif and          Gauri Deshpande
 *                           thumbnail
 *   06/01/05   26          Made changes to run both primary    Gauri Deshpande
 *                           and thumbnail in one go.
 *   07/01/05   27          Prefix all the data types, defines  B.Venkatarao
 *                           exposed to API. Also modified
 *                           mem_info structure
 *   10/01/05   28          In case of RGB_565 output format,   B.Venkatarao
 *                           outputs are written into a file
 *                           byte by byte to make it
 *                           independent of endianness
 *   12/01/05   29          Return type changed                 B.Venkatarao
 *   16/11/06   30          Modified for 						H S Manjunath
 *            				profile on RVDS 
 *            				and ELinux:TLSbo83403
 *   05/01/07   31          Modified for Stack And Heap usage   H S Manjunath
 *							TLSbo87093
 ****************************************************************************/
 /*
 ***********************************************************************
 * Copyright (c) 2005-2012, Freescale Semiconductor Inc.,
 *********************************************************************** 
 *  History :
 *  Date             Author       Version    Description
 *  Apr,2007        Jogesh        1.0        Level 4 Warnings Removed
 *  14/05/08   32      add ppm output format                      Eagle Zhou
 *  30/July/2008 	 33 	 Add version query				  Wang Zening
 *  12/11/08   34          add bgr format:ENGR00098581		Eagle Zhou 
 *  02/03/09   35          add frame api:  ENGR00108642        Eagle Zhou
 *  14/05/09   36          add VPU Linux support: ENGR00112433	Eagle Zhou 
 *  14/10/09   37          add VPU Wince support: ENGR00117211     Eagle Zhou
 *  15/10/09   38          refine: simplify the whole work flow		Eagle Zhou
 *  11/06/10   39           add vpu crop : ENGR00123796             Eagle Zhou
 *  01/07/10   40          add input buffer size macro to 
 *                         show how to get better performance       Eagle Zhou 
****************************************************************************/	  


#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <ctype.h>
#include "jpeg_dec_interface.h"

#ifdef __WINCE
	#include<windows.h>
#else
#ifdef WIN32 //eagle	
	#include<windows.h>
	#define gettimeofday
#else
	#include <sys/time.h>
#endif
#endif

#ifdef VPU_SUPPORT
#include "vpu_lib.h"
#include "vpu_io.h"
#endif

#if defined(WINCE_WARNLVL4_DISABLE) && defined(__WINCE)
	// disable some level-4 warnings, use #pragma warning(enable:###) to re-enable
	#pragma warning(disable:4100) // warning C4100: unreferenced formal parameter
	#pragma warning(disable:4201) // warning C4201: nonstandard extension used : nameless struct/union
	#pragma warning(disable:4214) // warning C4214: nonstandard extension used : bit field types other than int
	#pragma warning(disable:4115) // warning C4115: named type definition in parentheses
#endif //#if defined(WINCE_WARNLVL4_DISABLE) && defined(__WINCE)

#define MAX_FILE_NAME		255
#define USE_DECODE_FRAME		// call decode frame api, otherwise, decode row by row

#define VPU_BUFF_ALIGN		1024
#define VPU_BUFF_UNIT		256
/*
   Tip for getting better performance:
   (1) in current design, file will be feed to decoder repeatedly, please refer to begin_flag in callback function
   	jpegd_get_file_info:	will parser file from file header
	jpegd_query_dec_mem:will parser file from file header
	jpegd_decoder_init:	will parser file from file header
   (2) in init step: only file header info is needed for callback, so we should better set smaller value for input size(CALLBACK_INPUT_LEN_INIT)
        in decode step: to decrease the calling numbers of callback, we should better set bigger value for input size(CALLBACK_INPUT_LEN_DEC)
*/
#define CALLBACK_INPUT_LEN_INIT	(4*1024) // callback length for file_init/query memory/decode_init/
#define CALLBACK_INPUT_LEN_DEC	(256*1024) // callback length for decode_frame
#define INPUT_BUF_MAX  ((CALLBACK_INPUT_LEN_DEC>=CALLBACK_INPUT_LEN_INIT)?CALLBACK_INPUT_LEN_DEC:CALLBACK_INPUT_LEN_INIT)//max(CALLBACK_INPUT_LEN_INIT,CALLBACK_INPUT_LEN_DEC)

#ifdef USE_DECODE_FRAME
#define CONVERT_YUV_RGB888	// convert .bin(YUV) to .ppm(RGB)
#endif
#define DEF_SCALE	1	//default scale value
/*
 * This list defines the known output image formats
 * (not all of which need be supported by a given version).
 * You can change the default output format by defining DEFAULT_FMT;
 * indeed, you had better do so if you undefine PPM_SUPPORTED.
 */

typedef enum {
	//FMT_BMP,		/* BMP format (Windows flavor) */
	//FMT_OS2,		/* BMP format (OS/2 flavor) */
	FMT_BIN,         /* Plain binary file without headers*/
	FMT_PPM         /* pnm*/
} IMAGE_FORMATS;

#define DEFAULT_FMT	FMT_BIN


FILE * input_file;
//FILE * output_file;
char * outfilename;	/* for -outfile switch */
//static char tfilename[200];

unsigned int input_buf_size;
unsigned char* JPEGD_input_buffer_ptr; //buffer used in call back function

IMAGE_FORMATS requested_fmt;

int mode;
int scale_mode;
JPEGD_UINT8 scale_val;


#ifdef VPU_SUPPORT
#define MAX_PHY_MEM	(16*1024*1024)	//max system physical memory
unsigned int g_vpu_phy_start;
unsigned int g_vpu_size;
unsigned int g_vpu_offset;
unsigned int g_vpu_virt_start;
unsigned int g_vpu_cpu_start;

int vpu_mode;
#endif

unsigned int g_test_stride_y_pad; //for crop test
    
char* YUV_temp[4];			// temporary buffer for YUV output (rows by rows)
char* pt_YUV_temp[4];		// temporary buffer for YUV output (rows by rows)
int Buffer_size[4];

JPEGD_Decoder_Object *p_dec_obj;

unsigned int callback_time_removed;  // for soft, we should remove callback time, for vpu, we should not remove callback time

/*Changes Made for Call back*/
JPEGD_UINT8 jpegd_get_new_data (JPEGD_UINT8 **ppBuf, JPEGD_UINT32 *pLen,
                          JPEGD_UINT32 mcu_offset, JPEGD_UINT8 begin_flag,
                          void *obj_ptr);
/*Changes Made for Call back*/

/***************************************************************************
 *
 *   FUNCTION NAME - usage
 *
 *   DESCRIPTION
 *      This function prints the Program usage.
 *
 *   ARGUMENTS
 *      None
 *
 *   RETURN VALUE
 *      None
 *
 ***************************************************************************/
void usage (void)
{
    fprintf(stderr, "usage: [switches] ");
    fprintf(stderr, "[inputfile]\n");

    fprintf(stderr, "Switches (names may be abbreviated):\n");
    //fprintf(stderr, "  -outsize HxV   Output image size eg, 176x144\n");
    fprintf(stderr, "  -ofmt n          Output format: \n");
    fprintf(stderr, "                                0 - enc-image-format (default)\n");
    fprintf(stderr, "                                1 - rgb_888\n");
    fprintf(stderr, "                                2 - rgb_565\n");
    fprintf(stderr, "                                3 - bgr_888\n");
    fprintf(stderr, "                                4 - bgr_565\n");	
    //fprintf(stderr, "  -ppm        Select PPM output format\n");	
    //fprintf(stderr, "  -bin         Select Raw output format%s\n",
    //        (DEFAULT_FMT == FMT_BIN ? " (default)" : ""));

    fprintf(stderr, "  -vpu 0/1        Select the vpu mode, 0:off (default); 1: on\n");	
    fprintf(stderr, "  -mode 0/1/2   Select the decoding mode: \n");
    fprintf(stderr, "                                 0 - JPEGD_PRIMARY\n");
    fprintf(stderr, "                                 1 - JPEGD_THUMBNAIL\n");
    fprintf(stderr, "                                 2 - BOTH (default)\n");
    fprintf(stderr, "  -exif_info 0/1  exif_info required: \n");
    fprintf(stderr, "                                 0 - Not Required (default)\n");
    fprintf(stderr, "                                 1 - Required\n");
    fprintf(stderr, "  -outfile name  Specify name for output file\n");

    fprintf(stderr, "Switches for advanced users:\n");
    fprintf(stderr, "  -dct int          Use integer DCT method\n");
    fprintf(stderr, "  -dct fast        Use fast integer DCT with less accurate (default)  \n");
    fprintf(stderr, "  -dct float       Use floating-point DCT method\n");

    fprintf(stderr, "  -per_vpu T,U,W  =>total bitsteam buffer size; unit size every call back; wait time(ms)\n");
    fprintf(stderr, "                           T and U had better be %d bytes alignment \n",VPU_BUFF_ALIGN);	
    fprintf(stderr, "                           T had better be integer times of U \n");		
    fprintf(stderr, "                           T should be bigger than (U+%d) \n",VPU_BUFF_UNIT);		
    fprintf(stderr, "  -per_soft L        =>bitstreaml buffer size(unit size every call back) \n");

    fprintf(stderr, "  -scale 1/2/4/8   Select the downscale value: \n");
    fprintf(stderr, "                                 1 - no scale (default) \n");
    fprintf(stderr, "                                 2 - 1/(2x2) downscale \n");
    fprintf(stderr, "                                 4 - 1/(4x4) downscale \n");	
    fprintf(stderr, "                                 8 - 1/(8x8) downscale \n");	
	
    exit(EXIT_FAILURE);
}

/*
 * Case-insensitive matching of possibly-abbreviated keyword switches.
 * keyword is the constant keyword (must be lower case already),
 * minchars is length of minimum legal abbreviation.
 */

int keymatch (char * arg, const char * keyword, int minchars)
{
  register int ca, ck;
  register int nmatched = 0;

  while ((ca = *arg++) != '\0') {
    if ((ck = *keyword++) == '\0')
      return 0;		/* arg longer than keyword, no good */
    if (isupper(ca))		/* force arg to lcase (assume ck is already) */
      ca = tolower(ca);
    if (ca != ck)
      return 0;		/* no good */
    nmatched++;			/* count matched characters */
  }
  /* reached end of argument; fail if it's too short for unique abbrev */
  if (nmatched < minchars)
    return 0;
  return 1;			/* A-OK */
}

/*
 * Argument-parsing code.
 * The switch parser is designed to be useful with DOS-style command line
 * syntax, ie, intermixed switches and file names, where only the switches
 * to the left of a given file name affect processing of that file.
 * The main program in this file doesn't actually use this capability...
 */

/***************************************************************************
 *
 *   FUNCTION NAME - parse_switches
 *
 *   DESCRIPTION
 *      This function parses optional switches.
 *      Returns argv[] index of first file-name argument
 *      (== argc if none). Any file names with indexes <= last_file_arg_seen
 *      are ignored; they have presumably been processed in a previous
 *      iteration. (Pass 0 for last_file_arg_seen on the first or only
 *      iteration.) for_real is 0 on the first (dummy) pass; we may
 *      skip any expensive processing.
 *
 *   ARGUMENTS
 *      dec_param           - Pointer to decoder parameters
 *      argc                - Argument count
 *      argv                - Pointer to arguments
 *      last_file_arg_seen  - ?
 *      for_real            - ?
 *
 *   RETURN VALUE
 *      Index of next argument
 *
 ***************************************************************************/
int parse_switches (JPEGD_Decoder_Params *dec_param, int argc, char **argv,
                int last_file_arg_seen, int for_real)
{
  int argn;
  char * arg;
  int exif_info;

  /* Set up default JPEG parameters. */
  requested_fmt = DEFAULT_FMT;	/* set default output file format */
  outfilename = NULL;
  //cinfo->err->trace_level = 0;

  /* Scan command line options, adjust parameters */

  for (argn = 1; argn < argc; argn++) {
    arg = argv[argn];
    if (*arg != '-') {
      /* Not a switch, must be a file name argument */
      if (argn <= last_file_arg_seen) {
	outfilename = NULL;	/* -outfile applies to just one input file */
	continue;		/* ignore this name if previously processed */
      }
      break;			/* else done parsing switches */
    }
    arg++;			/* advance past switch marker character */

     if (keymatch(arg, "dct", 2)) {
      /* Select IDCT algorithm. */
      if (++argn >= argc)	/* advance to next argument */
	usage();
      if (keymatch(argv[argn], "int", 1)) {
	dec_param->dct_method = JPEGD_IDCT_SLOW;
      }
      else if (keymatch(argv[argn], "fast", 2)) {
	dec_param->dct_method = JPEGD_IDCT_FAST;
      }
      else if (keymatch(argv[argn], "float", 2)) {
	dec_param->dct_method = JPEGD_IDCT_FLOAT;
      }
      else
	usage();

    }
     else if (keymatch(arg, "outfile", 4)) {
      /* Set output file name. */
      if (++argn >= argc)	/* advance to next argument */
	usage();
      outfilename = argv[argn];	/* save it away for later use */

    }
#if 0
    else if (keymatch(arg, "outsize", 5)) {
      /* The output image size HxV. */
      if (++argn >= argc)	/* advance to next argument */
	usage();
      {
          int h,v;
          if (sscanf(argv[argn], "%dx%d", &h,&v) != 2)
              usage();
          dec_param->desired_output_width = (JPEGD_UINT16)h;
          dec_param->desired_output_height = (JPEGD_UINT16)v;
      }

    }
#endif	
    else if (keymatch(arg, "per_vpu", 7)) {
      if (++argn >= argc)	/* advance to next argument */
	usage();
      {
          int t,u,w;
          if (sscanf(argv[argn], "%d,%d,%d", &t,&u,&w) != 3)
              usage();
          dec_param->vpu_bitstream_buf_size= (JPEGD_UINT32)t;
          dec_param->vpu_fill_size= (JPEGD_UINT32)u;
          dec_param->vpu_wait_time= (JPEGD_UINT32)w;	
	    printf("bitstream buf size: %d, fill size: %d, wait time(ms): %d \r\n",dec_param->vpu_bitstream_buf_size,dec_param->vpu_fill_size,dec_param->vpu_wait_time);	  
	    if(dec_param->vpu_bitstream_buf_size%VPU_BUFF_ALIGN !=0) 
			printf("Warning: vpu bitstream buffer had better be %d bytes alignment \r\n",VPU_BUFF_ALIGN);
	    if((dec_param->vpu_bitstream_buf_size%dec_param->vpu_fill_size !=0)) 
			printf("Warning: vpu bitstream buffer had better be interger times of vpu fill size \r\n");
	    if(dec_param->vpu_bitstream_buf_size<=(dec_param->vpu_fill_size + VPU_BUFF_UNIT))	
	    {
	        printf("Error: vpu bitstream buffer should be bigger than (vpu fill size+%d) \r\n",VPU_BUFF_UNIT);
		 usage(); 	
	    }
		
      }

    }	
    else if (keymatch(arg, "per_soft", 8)) {
      if (++argn >= argc)	/* advance to next argument */
	usage();
      {
          int l;
          if (sscanf(argv[argn], "%d", &l) != 1)
              usage();
          input_buf_size=l;		  
          printf("bitstream buf size: \r\n",input_buf_size);		  
      }

    }
#if 0	 
    else if (keymatch(arg, "ppm", 3)) {
      /* PPM output format */
      requested_fmt = FMT_PPM;
    }
    else if (keymatch(arg, "bin", 1)) {
      /* Plain binary file without headers. */
      requested_fmt = FMT_BIN;

    }	
#endif		
    else if (keymatch(arg, "ofmt", 3)) {
      /* The output image size HxV. */
      if (++argn >= argc)	/* advance to next argument */
	usage();
      {
          int n;
          if (sscanf(argv[argn], "%d", &n) != 1)
              usage();
          switch (n)
          {
          case 0:
              dec_param->output_format = JPEGD_OFMT_ENC_IMAGE_FMT;
		 requested_fmt = FMT_BIN;	  
              break;
          case 2:
              dec_param->output_format = JPEGD_OFMT_RGB_565;
		 requested_fmt = FMT_PPM;	  
              break;
          case 1:
              dec_param->output_format = JPEGD_OFMT_RGB_888;
		 requested_fmt = FMT_PPM;	  
              break;
          case 4:
              dec_param->output_format = JPEGD_OFMT_BGR_565;
		 requested_fmt = FMT_PPM;	  
              break;
          case 3:
              dec_param->output_format = JPEGD_OFMT_BGR_888;
		 requested_fmt = FMT_PPM;	  
              break;
          default:
              usage();
          }
      }

    }

    else if (keymatch(arg, "exif_info", 4)) {
        if (++argn >= argc)	/* advance to next argument */
    	    usage();
        sscanf(argv[argn], "%d", &exif_info);
        dec_param->exif_info_needed = (JPEGD_UINT8)exif_info;
    }
    else if (keymatch(arg, "mode", 4)) {
        if (++argn >= argc)	/* advance to next argument */
          	usage();
        sscanf(argv[argn], "%d", &mode);
        if (mode == 1)
            dec_param->decoding_mode = JPEGD_THUMBNAIL;
    }
#ifdef VPU_SUPPORT	
    else if (keymatch(arg, "vpu", 3)) {
        if (++argn >= argc)	/* advance to next argument */
          	usage();
        sscanf(argv[argn], "%d", &vpu_mode);
        if (vpu_mode == 1)
            dec_param->vpu_enable= 1;
    }
#endif	
    else if (keymatch(arg, "scale", 5)) {	 
	  int scale;	
	  if (++argn >= argc)	/* advance to next argument */    	    
	  	usage();		
	  sscanf(argv[argn], "%d", &scale);
	  if(scale !=1 && scale !=2 && scale !=4 && scale !=8)
	  	usage();
	  //dec_param->scale=(JPEGD_UINT8)scale;
	  //printf("scale: %d \r\n",dec_param->scale);    
	  scale_val=(JPEGD_UINT8)scale;
	  scale_mode=1;
	  printf("scale: %d \r\n",scale_val);    
     }
    else if (keymatch(arg, "pad", 3)) {	 
	  int scale;	
	  if (++argn >= argc)	/* advance to next argument */    	    
	  	usage();		
	  sscanf(argv[argn], "%d", &g_test_stride_y_pad);
	  printf("internal pad test: %d \r\n",g_test_stride_y_pad);    
     }
	else {
      usage();			/* bogus switch */
    }
  }

  return argn;			/* return index of next arg (file name) */
}

#ifdef CONVERT_YUV_RGB888
typedef unsigned char JSAMPLE;
#define GETJSAMPLE(value)  ((int) (value))

#define MAXJSAMPLE	255
#define CENTERJSAMPLE	128

#define RIGHT_SHIFT(x,shft)	((x) >> (shft))

#define SCALEBITS	16	/* speediest right-shift on some machines */
#define ONE_HALF	((int) 1 << (SCALEBITS-1))
#define FIX(x)		((int) ((x) * (1L<<SCALEBITS) + 0.5))
#define ALIGN_2(x)	((x+1)/2*2)

typedef struct {
	JPEGD_INT16 * Cr_r_tab;		/* => table for Cr to R conversion */
	JPEGD_INT16 * Cb_b_tab;		/* => table for Cb to B conversion */
	JPEGD_INT16 * Cr_g_tab;		/* => table for Cr to G conversion */
	JPEGD_INT16 * Cb_g_tab;		/* => table for Cb to G conversion */

	JSAMPLE * sample_range_limit; /* table for fast range-limiting */
} Color_deconverter;


void prepare_range_limit_table_app (Color_deconverter* cconvert)
{
	JSAMPLE * table;
	int i;

	cconvert->sample_range_limit += (MAXJSAMPLE+1);	/* allow negative subscripts of simple table */
	table = cconvert->sample_range_limit;
	/* First segment of "simple" table: limit[x] = 0 for x < 0 */
	memset(table - (MAXJSAMPLE+1),0, (MAXJSAMPLE+1) * sizeof(JSAMPLE));
	/* Main part of "simple" table: limit[x] = x */
	for (i = 0; i <= MAXJSAMPLE; i++)
	{
		table[i] = (JSAMPLE) i;
	}
	table += CENTERJSAMPLE;	/* Point to where post-IDCT table starts */
	/* End of simple table, rest of first half of post-IDCT table */
	for (i = CENTERJSAMPLE; i < 2*(MAXJSAMPLE+1); i++)
	{
		table[i] = MAXJSAMPLE;
	}
	/* Second half of post-IDCT table */
	memset(table + (2 * (MAXJSAMPLE+1)),0, (2 * (MAXJSAMPLE+1) - CENTERJSAMPLE) * sizeof(JSAMPLE));
	memcpy(table + (4 * (MAXJSAMPLE+1) - CENTERJSAMPLE), cconvert->sample_range_limit, CENTERJSAMPLE * sizeof(JSAMPLE));

}
void build_yuv_rgb_table (Color_deconverter* cconvert)
{
	int i;
	int x;

	for (i = 0, x = -CENTERJSAMPLE; i <= MAXJSAMPLE; i++, x++)
	{
		/* i is the actual input pixel value, in the range 0..MAXJSAMPLE */
		/* The Cb or Cr value we are thinking of is x = i - CENTERJSAMPLE */
		/* Cr=>R value is nearest int to 1.40200 * x */
		cconvert->Cr_r_tab[i] = (JPEGD_INT16)RIGHT_SHIFT(FIX(1.40200) * x + ONE_HALF, SCALEBITS);
		/* Cb=>B value is nearest int to 1.77200 * x */
		cconvert->Cb_b_tab[i] = (JPEGD_INT16)RIGHT_SHIFT(FIX(1.77200) * x + ONE_HALF, SCALEBITS);
		/* Cr=>G value is scaled-up -0.71414 * x */
		cconvert->Cr_g_tab[i] = (JPEGD_INT16)RIGHT_SHIFT((- FIX(0.71414)) * x, SCALEBITS);
		/* Cb=>G value is scaled-up -0.34414 * x */
		/* We also add in ONE_HALF so that need not do it in inner loop */
		cconvert->Cb_g_tab[i] = (JPEGD_INT16)RIGHT_SHIFT((- FIX(0.34414)) * x + ONE_HALF, SCALEBITS);
	}

}

void yuv420_rgb888_convert (FILE* fp,Color_deconverter* cconvert, JPEGD_UINT8** input_buf,int valid_width,int valid_height,int* stride)
{
	int y, cred, cgreen, cblue;
	int cb, cr;
	JPEGD_UINT8* outptr0, *outptr1, *outptr_p;
	JPEGD_UINT8* inptr0_y, *inptr1_y, *inptr_cb, *inptr_cr;
	JPEGD_UINT8* inptr0_y_p, *inptr1_y_p, *inptr_cb_p, *inptr_cr_p;
	int row, col;
	int col_width;
	/* copy these pointers into registers if possible */
	JSAMPLE * range_limit = cconvert->sample_range_limit;
	JPEGD_INT16 * Crrtab = cconvert->Cr_r_tab;
	JPEGD_INT16 * Cbbtab = cconvert->Cb_b_tab;
	JPEGD_INT16 * Crgtab = cconvert->Cr_g_tab;
	JPEGD_INT16 * Cbgtab = cconvert->Cb_g_tab;

	//printf("valid_width: %d, valid_height: %d \r\n",valid_width,valid_height);
	//printf("stride[0,1,2]= %d, %d, %d\r\n",stride[0],stride[1],stride[2]);
	//printf("input[0,1,2]= 0x%X, 0x%X, 0x%X \r\n",input_buf[0],input_buf[1],input_buf[2]);

	outptr_p = (JPEGD_UINT8*)malloc(3*valid_width*2);
	if(outptr_p==NULL)
	{
		return;
	}		
	inptr0_y_p = input_buf[0];
	inptr1_y_p = inptr0_y_p + stride[0];
	inptr_cb_p = input_buf[1];
	inptr_cr_p = input_buf[2];
	col_width = 3 * valid_width;
	/* Loop for each group of output pixels */
	for (row = 0; row < valid_height / 2; row++)
	{
		outptr0 = outptr_p;
		outptr1 = outptr_p + col_width;

		inptr0_y = inptr0_y_p ;
		inptr1_y = inptr1_y_p;
		inptr_cb =inptr_cb_p;
		inptr_cr = inptr_cr_p;

		for (col = 0; col < valid_width / 2; col++)
		{
			/* Do the chroma part of the calculation */
			cb = GETJSAMPLE(*inptr_cb++);
			cr = GETJSAMPLE(*inptr_cr++);
			cred = Crrtab[cr];
			cgreen =  Cbgtab[cb] + Crgtab[cr];
			cblue = Cbbtab[cb];
			/* Fetch 4 Y values and emit 4 pixels */
			y  = GETJSAMPLE(*inptr0_y++);
			*outptr0++ =   range_limit[y + cred];
			*outptr0++ = range_limit[y + cgreen];
			*outptr0++ =  range_limit[y + cblue];

			y  = GETJSAMPLE(*inptr0_y++);
			*outptr0++ =   range_limit[y + cred];
			*outptr0++ = range_limit[y + cgreen];
			*outptr0++ =  range_limit[y + cblue];

			y  = GETJSAMPLE(*inptr1_y++);
			*outptr1++ =   range_limit[y + cred];
			*outptr1++ = range_limit[y + cgreen];
			*outptr1++ =  range_limit[y + cblue];

			y  = GETJSAMPLE(*inptr1_y++);
			*outptr1++ =   range_limit[y + cred];
			*outptr1++ = range_limit[y + cgreen];
			*outptr1++ =  range_limit[y + cblue];
		}

		/* If odd width */
		if (valid_width & 1)
		{
			/* Do the chroma part of the calculation */
			cb = GETJSAMPLE(*inptr_cb++);
			cr = GETJSAMPLE(*inptr_cr++);
			cred = Crrtab[cr];
			cgreen =  Cbgtab[cb] + Crgtab[cr];
			cblue = Cbbtab[cb];
			/* Fetch 4 Y values and emit 4 pixels */
			y  = GETJSAMPLE(*inptr0_y++);
			*outptr0++ =   range_limit[y + cred];
			*outptr0++ = range_limit[y + cgreen];
			*outptr0++ =  range_limit[y + cblue];

			inptr0_y++;

			y  = GETJSAMPLE(*inptr1_y++);
			*outptr1++ =   range_limit[y + cred];
			*outptr1++ = range_limit[y + cgreen];
			*outptr1++ =  range_limit[y + cblue];

			inptr1_y++;
			col++;
		}

		inptr0_y_p += 2*stride[0] ;
		inptr1_y_p = inptr0_y_p + stride[0];
		inptr_cb_p += stride[1];
		inptr_cr_p += stride[2];
		fwrite(outptr_p,1,3*valid_width,fp);		
		fwrite(outptr_p + col_width,1,3*valid_width,fp);		


	}

	/* If odd height */
	if (valid_height & 1)
	{
		outptr0 = outptr_p;
		//outptr1 = outptr_p + col_width;
		for (col = 0; col < valid_width / 2; col++)
		{
			/* Do the chroma part of the calculation */
			cb = GETJSAMPLE(*inptr_cb++);
			cr = GETJSAMPLE(*inptr_cr++);
			cred = Crrtab[cr];
			cgreen =  Cbgtab[cb] + Crgtab[cr];
			cblue = Cbbtab[cb];
			/* Fetch 4 Y values and emit 4 pixels */
			y  = GETJSAMPLE(*inptr0_y++);
			*outptr0++ =   range_limit[y + cred];
			*outptr0++ = range_limit[y + cgreen];
			*outptr0++ =  range_limit[y + cblue];

			y  = GETJSAMPLE(*inptr0_y++);
			*outptr0++ =   range_limit[y + cred];
			*outptr0++ = range_limit[y + cgreen];
			*outptr0++ =  range_limit[y + cblue];

			//inptr1_y++;
			//inptr1_y++;
		}

		/* If odd width */
		if (valid_width & 1)
		{
			/* Do the chroma part of the calculation */
			cb = GETJSAMPLE(*inptr_cb++);
			cr = GETJSAMPLE(*inptr_cr++);
			cred = Crrtab[cr];
			cgreen =  Cbgtab[cb] + Crgtab[cr];
			cblue = Cbbtab[cb];
			/* Fetch 4 Y values and emit 4 pixels */
			y  = GETJSAMPLE(*inptr0_y++);
			*outptr0++ =   range_limit[y + cred];
			*outptr0++ = range_limit[y + cgreen];
			*outptr0++ =  range_limit[y + cblue];

			inptr0_y++;

			//inptr1_y++;
			//inptr1_y++;
			col++;
		}
		fwrite(outptr_p,1,3*valid_width,fp);			
	}

	free(outptr_p);
}

void yuv422h_rgb888_convert (FILE* fp,Color_deconverter* cconvert, JPEGD_UINT8** input_buf,int valid_width,int valid_height,int* stride)
{
	int y, cred, cgreen, cblue;
	int cb, cr;
	JSAMPLE * outptr, *outptr_p;
	JPEGD_UINT8* inptr_y, *inptr_cb, *inptr_cr;
	JPEGD_UINT8* inptr_y_p, *inptr_cb_p, *inptr_cr_p;	
	int row, col;

	JSAMPLE * range_limit = cconvert->sample_range_limit;
	JPEGD_INT16 * Crrtab = cconvert->Cr_r_tab;
	JPEGD_INT16 * Cbbtab = cconvert->Cb_b_tab;
	JPEGD_INT16 * Crgtab = cconvert->Cr_g_tab;
	JPEGD_INT16 * Cbgtab = cconvert->Cb_g_tab;

	outptr_p = (JPEGD_UINT8*)malloc(3*valid_width);
	if(outptr_p==NULL)
	{
		return;
	}	
	inptr_y_p = input_buf[0];
	inptr_cb_p = input_buf[1];
	inptr_cr_p = input_buf[2];

	/* Loop for each group of output pixels */
	for (row = 0; row < valid_height; row++)
	{
		outptr = outptr_p;
		inptr_y=inptr_y_p;
		inptr_cb=inptr_cb_p;
		inptr_cr=inptr_cr_p;

		for (col = 0; col < valid_width / 2; col++)
		{
			/* Do the chroma part of the calculation */
			cb = GETJSAMPLE(*inptr_cb++);
			cr = GETJSAMPLE(*inptr_cr++);
			cred = Crrtab[cr];
			cgreen =  Cbgtab[cb] + Crgtab[cr];
			cblue = Cbbtab[cb];
			/* Fetch 4 Y values and emit 4 pixels */
			y  = GETJSAMPLE(*inptr_y++);
			*outptr++ =   range_limit[y + cred];
			*outptr++ = range_limit[y + cgreen];
			*outptr++ =  range_limit[y + cblue];

			y  = GETJSAMPLE(*inptr_y++);
			*outptr++ =   range_limit[y + cred];
			*outptr++ = range_limit[y + cgreen];
			*outptr++ =  range_limit[y + cblue];
		}

		/* If odd width */
		if (valid_width & 1)
		{
			/* Do the chroma part of the calculation */
			cb = GETJSAMPLE(*inptr_cb++);
			cr = GETJSAMPLE(*inptr_cr++);
			cred = Crrtab[cr];
			cgreen =  Cbgtab[cb] + Crgtab[cr];
			cblue = Cbbtab[cb];
			/* Fetch 4 Y values and emit 4 pixels */
			y  = GETJSAMPLE(*inptr_y++);
			*outptr++ =   range_limit[y + cred];
			*outptr++ = range_limit[y + cgreen];
			*outptr++ =  range_limit[y + cblue];
			inptr_y++;
			col++;
		}
		
		inptr_y_p += stride[0] ;
		inptr_cb_p += stride[1];
		inptr_cr_p += stride[2] ;		
		fwrite(outptr_p,1,3*valid_width,fp);	
	}

	free(outptr_p);

}


void yuv422v_rgb888_convert (FILE* fp,Color_deconverter* cconvert, JPEGD_UINT8** input_buf,int valid_width,int valid_height,int* stride)
{
	int y, cred, cgreen, cblue;
	int cb, cr;
	JPEGD_UINT8* outptr0, *outptr1, *outptr_p;
	JPEGD_UINT8* inptr0_y, *inptr1_y, *inptr_cb, *inptr_cr;
	JPEGD_UINT8* inptr0_y_p, *inptr1_y_p, *inptr_cb_p, *inptr_cr_p;
	int row, col;
	int col_width;

	JSAMPLE * range_limit = cconvert->sample_range_limit;
	JPEGD_INT16 * Crrtab = cconvert->Cr_r_tab;
	JPEGD_INT16 * Cbbtab = cconvert->Cb_b_tab;
	JPEGD_INT16 * Crgtab = cconvert->Cr_g_tab;
	JPEGD_INT16 * Cbgtab = cconvert->Cb_g_tab;

	outptr_p = (JPEGD_UINT8*)malloc(3*valid_width*2);
	if(outptr_p==NULL)
	{
		return;
	}		
	inptr0_y_p = input_buf[0];
	inptr1_y_p = inptr0_y_p + stride[0];
	inptr_cb_p = input_buf[1];
	inptr_cr_p = input_buf[2];
	col_width = 3 * valid_width;
	/* Loop for each group of output pixels */
	for (row = 0; row < valid_height / 2; row++)
	{
		outptr0 = outptr_p;
		outptr1 = outptr_p + col_width;

		inptr0_y = inptr0_y_p ;
		inptr1_y = inptr1_y_p;
		inptr_cb =inptr_cb_p;
		inptr_cr = inptr_cr_p;

		for (col = 0; col < valid_width; col++)
		{
			/* Do the chroma part of the calculation */
			cb = GETJSAMPLE(*inptr_cb++);
			cr = GETJSAMPLE(*inptr_cr++);
			cred = Crrtab[cr];
			cgreen =  Cbgtab[cb] + Crgtab[cr];
			cblue = Cbbtab[cb];
			/* Fetch 4 Y values and emit 4 pixels */
			y  = GETJSAMPLE(*inptr0_y++);
			*outptr0++ =   range_limit[y + cred];
			*outptr0++ = range_limit[y + cgreen];
			*outptr0++ =  range_limit[y + cblue];

			y  = GETJSAMPLE(*inptr1_y++);
			*outptr1++ =   range_limit[y + cred];
			*outptr1++ = range_limit[y + cgreen];
			*outptr1++ =  range_limit[y + cblue];
		}

		inptr0_y_p += 2*stride[0] ;
		inptr1_y_p = inptr0_y_p + stride[0];
		inptr_cb_p += stride[1];
		inptr_cr_p += stride[2];
		fwrite(outptr_p,1,3*valid_width,fp);		
		fwrite(outptr_p + col_width,1,3*valid_width,fp);		


	}

	/* If odd height */
	if (valid_height & 1)
	{
		outptr0 = outptr_p;
		//outptr1 = outptr_p + col_width;
		for (col = 0; col < valid_width; col++)
		{
			/* Do the chroma part of the calculation */
			cb = GETJSAMPLE(*inptr_cb++);
			cr = GETJSAMPLE(*inptr_cr++);
			cred = Crrtab[cr];
			cgreen =  Cbgtab[cb] + Crgtab[cr];
			cblue = Cbbtab[cb];
			/* Fetch 4 Y values and emit 4 pixels */
			y  = GETJSAMPLE(*inptr0_y++);
			*outptr0++ =   range_limit[y + cred];
			*outptr0++ = range_limit[y + cgreen];
			*outptr0++ =  range_limit[y + cblue];

		}

		fwrite(outptr_p,1,3*valid_width,fp);			
	}
	
	free(outptr_p);

}

void yuv444_rgb888_convert(FILE* fp,Color_deconverter* cconvert, JPEGD_UINT8** input_buf,int valid_width,int valid_height,int* stride) 
{
	int y, cb, cr;
	JPEGD_UINT8* outptr, *outptr_p;
	JPEGD_UINT8* inptr_y, *inptr_cb, *inptr_cr;
	int row,col;

	JPEGD_INT16 * Crrtab = cconvert->Cr_r_tab;
	JPEGD_INT16 * Cbbtab = cconvert->Cb_b_tab;
	JPEGD_INT16 * Crgtab = cconvert->Cr_g_tab;
	JPEGD_INT16 * Cbgtab = cconvert->Cb_g_tab;
	JSAMPLE * range_limit = cconvert->sample_range_limit;

	inptr_y = input_buf[0];
	inptr_cb = input_buf[1];
	inptr_cr = input_buf[2];
	outptr_p = (JPEGD_UINT8*)malloc(3*valid_width);
	if(outptr_p==NULL)
	{
		return;
	}

	for (row = 0; row < valid_height; row++)
	{
		outptr = outptr_p;
		for (col = 0; col < valid_width; col++)
		{
			y  = GETJSAMPLE(*inptr_y++);
			cb = GETJSAMPLE(*inptr_cb++);
			cr = GETJSAMPLE(*inptr_cr++);

			/* Range-limiting is essential due to noise introduced by DCT losses. */
			*outptr++ =   range_limit[y + Crrtab[cr]];
			*outptr++ = range_limit[y + Cbgtab[cb] + Crgtab[cr]];
			*outptr++ =  range_limit[y + Cbbtab[cb]];
		}
		inptr_y += (stride[0] - valid_width);
		inptr_cb += (stride[1] - valid_width);
		inptr_cr += (stride[2] - valid_width);
		fwrite(outptr_p,1,3*valid_width,fp);	
	}

	free(outptr_p);
}

void yuv400_gray_convert(FILE* fp,JPEGD_UINT8** input_buf,int valid_width,int valid_height,int* stride)
{
	int row;
	JPEGD_UINT8* out;

	out=input_buf[0];
	for (row = 0; row < valid_height; row++)
	{
		fwrite(out,1,valid_width,fp);
		out += stride[0];
	}
}


void convert_yuv_rgb888 (JPEGD_Decoder_Object *dec_obj,JPEGD_UINT8 **output_buf, JPEGD_INT32 *out_stride_width)
{
	JPEGD_Decoder_Info  *dec_info;
	JPEGD_Component_Info *compptr;
	FILE* fp;
	Color_deconverter cconvert;
	JPEGD_UINT8 * temp;
	char tfilename[MAX_FILE_NAME];
	JPEGD_UINT16 image_width;
	JPEGD_UINT16 image_height;

	dec_info = &dec_obj->dec_info;
	compptr = dec_info->comp_info;

	image_width=dec_info->actual_output_width;
	image_height=dec_info->actual_output_height;
	
	//write ppm file header
	strcpy (tfilename, outfilename);
	if (dec_obj->dec_param.decoding_mode == JPEGD_THUMBNAIL)
	{
		strcat (tfilename,"_thumb");
	}
	
	if(dec_obj->dec_param.vpu_enable==1)
	{
		//strcat (tfilename, "_vpu_rgb888.ppm");
		strcat (tfilename, ".ppm");
	}
	else
	{
		//strcat (tfilename, "_soft_rgb888.ppm");
		strcat (tfilename, ".soft.ppm");
	}

	if ((fp = fopen(tfilename, "wb")) == NULL)
	{
		return;
	}

	if(dec_info->num_components==1)
	{
		fprintf(fp, "%s\n", "P5");
	}
	else
	{
		fprintf(fp, "%s\n", "P6");
	}
	fprintf(fp, "%d %d\n", image_width,image_height);
	fprintf(fp, "%d\n", 255);
	fflush(fp);


	// malloc color convert table
	temp = (JPEGD_UINT8 *)malloc((MAXJSAMPLE+1) * sizeof(JPEGD_INT16)*4+
		(5 * (MAXJSAMPLE+1) + CENTERJSAMPLE) * sizeof(JSAMPLE));
	if(temp==NULL)
	{
		fclose(fp);
		return;
	}
	cconvert.Cr_r_tab=(JPEGD_INT16*)temp;
	cconvert.Cb_b_tab =(JPEGD_INT16*)(temp +(MAXJSAMPLE+1) * sizeof(JPEGD_INT16));
	cconvert.Cr_g_tab = (JPEGD_INT16*)(temp +2*(MAXJSAMPLE+1) * sizeof(JPEGD_INT16));
	cconvert.Cb_g_tab = (JPEGD_INT16*)(temp +3*(MAXJSAMPLE+1) * sizeof(JPEGD_INT16));
	cconvert.sample_range_limit=(JSAMPLE*)(temp +4*(MAXJSAMPLE+1) * sizeof(JPEGD_INT16));

	//init conver table
	build_yuv_rgb_table(&cconvert);
	prepare_range_limit_table_app(&cconvert);

	//convert and write file
	if(dec_info->num_components==1)
	{
		//4:0:0
		printf("4:0:0 convert \r\n");
		yuv400_gray_convert(fp,output_buf,image_width,image_height,(int*)out_stride_width);
	}
	else if (dec_info->num_components==3)
	{
		if((ALIGN_2(dec_info->comp_info[0].actual_output_width)==(dec_info->comp_info[1].actual_output_width<<1))
			&&(ALIGN_2(dec_info->comp_info[0].actual_output_width)==(dec_info->comp_info[2].actual_output_width<<1)))
		{
			if((ALIGN_2(dec_info->comp_info[0].actual_output_height)==(dec_info->comp_info[1].actual_output_height<<1))
				&&(ALIGN_2(dec_info->comp_info[0].actual_output_height)==(dec_info->comp_info[2].actual_output_height<<1)))			
			{
				//4:2:0
				printf("4:2:0 convert \r\n");
				yuv420_rgb888_convert(fp,&cconvert,output_buf,image_width,image_height,(int*)out_stride_width);
				
			}
			else if((dec_info->comp_info[0].actual_output_height==dec_info->comp_info[1].actual_output_height)
				&&(dec_info->comp_info[0].actual_output_height==dec_info->comp_info[2].actual_output_height))
			{
				//4:2:2 hor
				printf("4:2:2 hor convert \r\n");
				yuv422h_rgb888_convert(fp,&cconvert,output_buf,image_width,image_height,(int*)out_stride_width);
			}
			else
			{
				//unknown format
				printf("unknown yuv format \r\n");
			}
			
		}
		else if((dec_info->comp_info[0].actual_output_width==dec_info->comp_info[1].actual_output_width)
			&&(dec_info->comp_info[0].actual_output_width==dec_info->comp_info[2].actual_output_width))
		{
			if((ALIGN_2(dec_info->comp_info[0].actual_output_height)==(dec_info->comp_info[1].actual_output_height<<1))
				&&(ALIGN_2(dec_info->comp_info[0].actual_output_height)==(dec_info->comp_info[2].actual_output_height<<1)))			
			{
				//4:2:2 ver
				printf("4:2:2 ver convert \r\n");
				yuv422v_rgb888_convert(fp,&cconvert,output_buf,image_width,image_height,(int*)out_stride_width);
			}
			else if((dec_info->comp_info[0].actual_output_height==dec_info->comp_info[1].actual_output_height)
				&&(dec_info->comp_info[0].actual_output_height==dec_info->comp_info[2].actual_output_height))
			{
				//4:4:4
				printf("4:4:4 convert \r\n");
				yuv444_rgb888_convert(fp,&cconvert,output_buf,image_width,image_height,(int*)out_stride_width);
			}
			else
			{
				//unknown format
				printf("unknown yuv format \r\n");
			}			
		}
		else
		{
			//unknown format
			printf("unknown yuv format \r\n");
		}
	}
	else
	{
		//unknown format 
		printf("unknown yuv format \r\n");
	}

	//release resource
	free(cconvert.Cr_r_tab);
	fclose(fp);
	
}
#endif
/***************************************************************************
 *
 *   FUNCTION NAME - write_outputs
 *
 *   DESCRIPTION
 *      This function writes the JPEG decoder outputs to a file.
 *
 *   ARGUMENTS
 *      dec_obj             - Pointer to decoder object
 *      output_buf          - Pointer to output buffer pointer
 *      dest_mgr            - Pointer to output format manager
 *                              used only for the formats other than BIN.
 *
 *   RETURN VALUE
 *      None
 *
 ***************************************************************************/
void write_outputs (JPEGD_Decoder_Object *dec_obj,
               JPEGD_UINT8 **output_buf,
               JPEGD_INT32 *out_stride_width,
               FILE* output_file)
{
    int ci, num_scanlines;
    int size, stride_size;
    JPEGD_Decoder_Info  *dec_info;
    JPEGD_Component_Info *compptr;

    dec_info = &dec_obj->dec_info;

    num_scanlines = dec_info->num_lines;
    /* Write out the decoded scanlines */
    {
        JPEGD_UINT8 *outptr;
        int row;

        if (dec_obj->dec_param.output_format == JPEGD_OFMT_ENC_IMAGE_FMT)
        {
            int num_cols;

            /* Write component by component */
            for (ci = 0, compptr = dec_info->comp_info;
                 ci < dec_info->num_components; ci++, compptr++)
            {
                outptr = output_buf[ci];
                stride_size = out_stride_width[ci];
                num_cols = compptr->actual_output_width; // FIX ME: in fact, it had better use original width and height 
                //printf("stride_size: %d \r\n",stride_size);
                //printf("num_cols: %d \r\n",num_cols);
                //printf("compptr->num_lines: %d \r\n",compptr->num_lines);		
                for (row = 0; row < compptr->num_lines; row++)
                {
                    //fwrite (outptr,1,num_cols,output_file);
                    memcpy(pt_YUV_temp[ci], outptr, (num_cols-num_cols%2));
                    outptr += stride_size;
					pt_YUV_temp[ci] += (num_cols-num_cols%2);
					Buffer_size[ci] += (num_cols-num_cols%2);			
                }
            }
#if 0   // test VPU Stride
{
	FILE* fp;
	fp=fopen("temp.yuv","wb");
	for(ci = 0, compptr = dec_info->comp_info;
                 ci < dec_info->num_components; ci++, compptr++)
       {
	printf("write: 0x%X , stride: %d , height: %d \r\n",output_buf[ci],out_stride_width[ci],compptr->num_lines);
	fwrite (output_buf[ci],1,compptr->num_lines*out_stride_width[ci],fp);

                 	}
	fclose(fp);
}

#endif
#ifdef CONVERT_YUV_RGB888		   
            convert_yuv_rgb888(dec_obj,output_buf,out_stride_width);
#endif
				 
        }
        else
        {
            int output_pixel_size = 3;
            if ((dec_obj->dec_param.output_format == JPEGD_OFMT_RGB_565)||(dec_obj->dec_param.output_format == JPEGD_OFMT_BGR_565))
            {
                output_pixel_size = 2;
            }
            outptr = output_buf[0];
            size = output_pixel_size * dec_info->actual_output_width;
            stride_size = output_pixel_size * out_stride_width[0];
            if (requested_fmt == FMT_PPM)
            {
                /* For RGB_565, outputs are written as half words,
                 *  So while writing into a file we need to treat
                 *  the outputs as half-words to take care of
                 *  endianness
                 */
                if (output_pixel_size == 2)
                {
                    for (row = 0; row < num_scanlines; row++)
                    {
                        int i;
                        JPEGD_UINT16 *outptr1;

                        outptr1 = (JPEGD_UINT16 *)outptr;
                        for (i = 0; i < size; i += 2)
                        {
                            JPEGD_UINT16 tmp;
                            JPEGD_UINT8 tmp1;
                            tmp = *outptr1++;
                            tmp1 = (JPEGD_UINT8)(tmp & 0xFF);
                            putc(tmp1, output_file);

                            tmp1 = (JPEGD_UINT8)((tmp >> 8) & 0xFF);
                            putc(tmp1, output_file);
						
                        }                  
                        outptr += stride_size;
                    }
                }
                else
                {
                    for (row = 0; row < num_scanlines; row++)
                    {
                        fwrite (outptr,1,size,output_file);
                        outptr += stride_size;
                    }
                }
            }
        }
    }

    return;
}

#ifdef VPU_SUPPORT
//here, we get physical memory through vpu interface !!
int InitPhyDevice()
{
	RetCode vpu_ret;
	int ret=1;
	
#ifdef __WINCE
	VPUMemAlloc buff;
	int buf_size;
	
	vpu_ret=vpu_Init(NULL);
	if(vpu_ret!=RETCODE_SUCCESS) //failure
	{
		printf("phy device failure !!!\r\n");
		ret=0;
	}

	buf_size=MAX_PHY_MEM;
	vpu_ret=vpu_AllocPhysMem(buf_size,&buff);
	if(vpu_ret)
	{
		printf("1:allocate physical memory failure: %d \r\n",buf_size);
		ret=0;
	}

	if(ret==1)
	{
		//printf("phy: 0x%X , virt: 0x%X , size: 0x%X \r\n",buff.phy_addr,buff.virt_uaddr,buff.size);	
		g_vpu_phy_start=buff.PhysAdd;
		g_vpu_virt_start=buff.VirtAdd;
		g_vpu_offset=0;
		g_vpu_size=buf_size;
	}	
	vpu_Deinit();	//Wince Driver do not support re-entry of vpu_init(), so we call vpu_deinit() before decoder calling vpu_init().
	//it seems that we still can use physical memory after vpu_Deinit().

#else
	vpu_mem_desc buff;
	
	vpu_ret=vpu_Init(NULL);
	if(vpu_ret!=RETCODE_SUCCESS) //failure
	{
		printf("phy device failure !!!\r\n");
		ret=0;
	}

	buff.size=MAX_PHY_MEM;
	vpu_ret=IOGetPhyMem(&buff);
	if(vpu_ret)
	{
		printf("2:allocate physical memory failure: %d \r\n",buff.size);
		ret=0;
	}
	vpu_ret=IOGetVirtMem(&buff);
	if(vpu_ret<=0)
	{
		printf("get virtual memory failure \r\n");
		ret=0;
	}
	if(ret==1)
	{
		//printf("phy: 0x%X , virt: 0x%X , size: 0x%X \r\n",buff.phy_addr,buff.virt_uaddr,buff.size);	
		g_vpu_phy_start=buff.phy_addr;
		g_vpu_virt_start=buff.virt_uaddr;
		g_vpu_offset=0;
		g_vpu_size=buff.size;
		g_vpu_cpu_start=buff.cpu_addr;
	}
	//vpu_UnInit();		
#endif	
	return ret;
}
int DeinitPhyDevice()
{
	//In fact, vpu_UnInit() already been called in decoder !!
	//So, we need not do anything
#ifndef __WINCE
	vpu_mem_desc buff;
	buff.phy_addr=g_vpu_phy_start;
	buff.virt_uaddr=g_vpu_virt_start;
	buff.cpu_addr=g_vpu_cpu_start;
	buff.size=g_vpu_size;
	IOFreeVirtMem(&buff);
	IOFreePhyMem(&buff);
	vpu_UnInit();
#endif	
}

int GetPhyVirtMem(vpu_mem_desc * buff)
{
	RetCode vpu_ret;
	int ret=0;
	if(g_vpu_offset+buff->size>g_vpu_size)
	{
		printf("memory overflow: %d\r\n",buff->size);
		return 1;	//overflow
	}
	
	buff->phy_addr=g_vpu_phy_start+g_vpu_offset;

	buff->virt_uaddr=g_vpu_virt_start+g_vpu_offset;

	g_vpu_offset+=buff->size;
	//printf("get %d bytes physical memory \r\n",buff->size);

	return ret;
}

int FreePhyVirtMem(vpu_mem_desc * buff)
{
	int ret=0;
	//ret=IOFreeVirtMem(buff);
	//ret=IOFreePhyMem(buff);	
	return ret;	
}
#endif

void check_exif_info(JPEGD_Decoder_Object* dec_obj)
{
	JPEGD_exif_info * exif;
	JPEGD_IFD0_appinfo* ifd0_info;
	JPEGD_exifIFD_appinfo* exififd_info;
	JPEGD_IFD1_appinfo* ifd1_info;	

	if((dec_obj->dec_param.exif_info_needed==0)||(dec_obj->dec_param.decoding_mode==JPEGD_THUMBNAIL))
	{
		return;
	}
	
	exif=&dec_obj->exif_info;
	ifd0_info=&dec_obj->exif_info.ifd0_info;
	ifd1_info=&dec_obj->exif_info.ifd1_info;
	exififd_info=&dec_obj->exif_info.exififd_info;
	
	printf("exif ifd flag: %d \r\n",exif->ExifIFD_flag);
	printf("exif ifd 0 flag: %d \r\n",exif->IFD0_flag);
	printf("exif ifd 1 flag: %d \r\n",exif->IFD1_flag);

	printf("IFD0 : X: %d, Y: %d \r\n",ifd0_info->x_resolution[0],ifd0_info->y_resolution[0]);
	printf("IFD1 : X: %d, Y: %d \r\n",ifd1_info->x_resolution[1],ifd1_info->y_resolution[1]);

	printf("IFD0 : orientation: %d \r\n",ifd0_info->orientation);
	printf("IFD1 : orientation: %d \r\n",ifd1_info->orientation);

	printf("EXIF version: %d.%d.%d.%d  \r\n",exififd_info->exif_version[0],exififd_info->exif_version[1],exififd_info->exif_version[2],exififd_info->exif_version[3]);
	printf("EXIF IFD : X: %d, Y: %d \r\n",exififd_info->pixel_x_dimension,exififd_info->pixel_y_dimension);

	
	
}


#ifdef __WINCE         // for taking timing on wince platform

LARGE_INTEGER lpFrequency1 ;
LARGE_INTEGER lpPerformanceCount1;
LARGE_INTEGER lpPerformanceCount2;
__int64 TotalDecTime=0;

static void timer_init()
{
	TotalDecTime = 0;
}

static void timer_start()
{
    //  INTERRUPTS_SET(0xDF) ; 	//disable interrupt
    QueryPerformanceFrequency(&lpFrequency1);
    QueryPerformanceCounter(&lpPerformanceCount1);
}

static void timer_stop()
{
	__int64 Temp;
     QueryPerformanceCounter(&lpPerformanceCount2);
     Temp=(((lpPerformanceCount2.QuadPart - lpPerformanceCount1.QuadPart)*1000000)/(lpFrequency1.QuadPart));
     TotalDecTime += Temp;
}

static void timer_report()
{
    printf("Decoding Time: %ld us \n", TotalDecTime);

}

static unsigned long get_decoding_time()
{
	return TotalDecTime;
}

#else	//Linux 

struct timeval tm_dec1, tm_dec2;
static unsigned long duration_dec = 0;

// init all timers
static void timer_init()
{
    duration_dec = 0;
}

static void timer_start()
{
    gettimeofday(&tm_dec1, 0);
}

static void timer_stop()
{
    unsigned long tm_1, tm_2;

    gettimeofday(&tm_dec2, 0);
	
    tm_1 = tm_dec1.tv_sec * 1000000 + tm_dec1.tv_usec;
    tm_2 = tm_dec2.tv_sec * 1000000 + tm_dec2.tv_usec;
    duration_dec = duration_dec + (tm_2-tm_1);
}

static void timer_report()
{
    printf("Decoding Time: %ld us\n", duration_dec);
}

static unsigned long get_decoding_time()
{
	return duration_dec;
}

#endif

void print_vpu_version()
{
#ifdef  __WINCE
	unsigned int ver;
	unsigned short pn;
	unsigned short version;
	unsigned char  ipprjnum;

	InitPhyDevice();
	if(RETCODE_SUCCESS!=vpu_GetVersionInfo(&ver))
	{
		printf("get vpu version failure \r\n");
	}
	else
	{
		pn = (unsigned short)(ver>>16);
		version = (unsigned short)ver;
		ipprjnum = (unsigned char)(pn);
		printf("VPU(num:%d) version: %04d.%04d.%08d", ipprjnum,(version>>(12))&0x0f, (version>>(8))&0x0f, (version)&0xff );
	}
#else
    	 vpu_versioninfo ver;
	 InitPhyDevice();
	 if(RETCODE_SUCCESS!=vpu_GetVersionInfo(&ver))
	 {
	 	printf("get vpu version failure \r\n");
	 }
	 else
	 {
	 	printf("VPU FW: [major.minor.release]=[%d.%d.%d] \r\n",ver.fw_major,ver.fw_minor,ver.fw_release);
		printf("VPU LIB: [major.minor.release]=[%d.%d.%d] \r\n",ver.lib_major,ver.lib_minor,ver.lib_release);
	 }
#endif	 	
}

/***************************************************************************
 *
 *   FUNCTION NAME - main
 *
 *   DESCRIPTION
 *      This function is the main application routine which runs the
 *      JPEG decoder.
 *
 *   ARGUMENTS
 *      argc                - Program argument count
 *      argv                - Pointer to arguments
 *
 *   RETURN VALUE
 *      0 always
 *
 ***************************************************************************/
int main (int argc, char **argv)
{

    JPEGD_Decoder_Object dec_obj;
    JPEGD_Mem_Alloc_Info *mem_info = &dec_obj.mem_info;
    int file_index;

    JPEGD_RET_TYPE ret;
    JPEGD_UINT8 *output_buf[JPEGD_MAX_NUM_COMPS] = {NULL};
    JPEGD_Decoder_Info  *dec_info;
    JPEGD_Component_Info *compptr;
    JPEGD_INT32 out_stride_width[JPEGD_MAX_NUM_COMPS];
    int ci;
    JPEGD_THUMBNAIL_TYPE thumbnail_type;
    JPEGD_UINT8 file_format;
    JPEGD_UINT32 min_size_exif=0;
    char tfilename[MAX_FILE_NAME];
    FILE* output_file=NULL;

    unsigned int pad; 	
	
#ifdef VPU_SUPPORT
	vpu_mem_desc bitstream_mem_desc;// for vpu bitstream buffer
	vpu_mem_desc frame_buffer; //for vpu output buffer
#endif

    dec_info = &dec_obj.dec_info;

    p_dec_obj = &dec_obj;


   //get version 	
   {
	char* version;
	version = (char*)jpegd_CodecVersionInfo();
	printf("version: %s\n", version);
   }


    //set default value
    dec_obj.dec_param.output_format = JPEGD_OFMT_ENC_IMAGE_FMT;
    dec_obj.dec_param.dct_method = JPEGD_IDCT_FAST;
    //dec_obj.dec_param.desired_output_width = 0; //set 0 to disable scale
    //dec_obj.dec_param.desired_output_height = 0;
    dec_obj.dec_param.exif_info_needed = 0;
    dec_obj.dec_param.decoding_mode = JPEGD_PRIMARY;
    dec_obj.dec_param.scale=DEF_SCALE;	//=1,2,4,8
	
    dec_obj.dec_param.vpu_enable=0;
    input_buf_size=CALLBACK_INPUT_LEN_INIT;    		    //set default value
    g_test_stride_y_pad=0;    // default, no crop test
    
#ifdef VPU_SUPPORT	
    bitstream_mem_desc	.size=0;
    frame_buffer.size=0;

    dec_obj.dec_param.vpu_bitstream_buf_size=0;  //set 0 to use internal default value
    dec_obj.dec_param.vpu_fill_size=0;                 //set 0 to use internal default value
    dec_obj.dec_param.vpu_wait_time=0;              //set 0 to use internal default value
    //InitPhyDevice();
    vpu_mode=0;
#endif

    mode = 2; // By default run both primary and thumbnail
    scale_mode=0; //use default scale

    file_index = parse_switches(&dec_obj.dec_param, argc, argv, 0, 0);

#ifdef VPU_SUPPORT
    if(dec_obj.dec_param.vpu_enable==1)
    {
	  print_vpu_version();
    }
#endif

    //malloc bitstream buffer
    JPEGD_input_buffer_ptr=malloc(INPUT_BUF_MAX);
    if(JPEGD_input_buffer_ptr==NULL)
    {
    	 printf("malloc call back buffer failure: %d \r\n",input_buf_size);
	 exit(EXIT_FAILURE);	 
    }
		

    /* Unix style: expect zero or one file name */
    if (file_index < argc-1) {
        fprintf(stderr, "only one input file\n");
        usage();
    }

    /* Open the input file. */
    if (file_index < argc)
    {
        if ((input_file = fopen(argv[file_index], "rb")) == NULL)
        {
            fprintf(stderr, " can't open %s\n", argv[file_index]);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        fprintf(stderr, "input file must be specified\n");
        usage();
    }

restart:

	if((input_buf_size==CALLBACK_INPUT_LEN_INIT)||(input_buf_size==CALLBACK_INPUT_LEN_DEC))
	{
		//user do not set specified value for it through command option, so we can modify it to get better performance
		printf("init the callback input size (smaller value): %d \r\n",CALLBACK_INPUT_LEN_INIT); 
		input_buf_size=CALLBACK_INPUT_LEN_INIT;   // set input data length for callback, the value is set for init api
	}
#ifdef VPU_SUPPORT	
	printf("vpu_enable: %d \r\n",dec_obj.dec_param.vpu_enable);
	if(bitstream_mem_desc.size!=0)
	{
		FreePhyVirtMem(&bitstream_mem_desc);
		bitstream_mem_desc.size=0;
	}
	if(frame_buffer.size!=0)
	{
		FreePhyVirtMem(&frame_buffer);
		frame_buffer.size=0;
	}
	if((dec_obj.dec_param.vpu_enable==1)&&(requested_fmt==FMT_PPM))
	{
		printf("use YUV output: since vpu do not support RGB \r\n");
		requested_fmt=FMT_BIN;	//vpu do not support RGB
	}
#endif

    /* Open the output file. */
    if (outfilename != NULL)
    {
        strcpy (tfilename, outfilename);
        if (dec_obj.dec_param.decoding_mode == JPEGD_THUMBNAIL)
            strcat (tfilename,"_thumb");
        if (requested_fmt == FMT_BIN)
            strcat (tfilename, ".bin");
	 else //(requested_fmt == FMT_PPM)
            strcat (tfilename, ".ppm");	
	 
        if ((output_file = fopen(tfilename, "wb")) == NULL) {
            fprintf(stderr, " can't open %s\n", outfilename);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        //fprintf(stderr, "output file must be specified\n");
        //usage();
    }

	/*Changes Made for Call back*/
	ret = jpegd_register_jpegd_get_new_data(jpegd_get_new_data,&dec_obj);
	if (ret != JPEGD_ERR_NO_ERROR)
	{
	   exit (1);
	}
	/*Changes Made for Call back*/

    timer_init();
    timer_start();

    /* get file information: JFIF/EXIF, thumbnail present, and min_size_exif */
    ret = jpegd_get_file_info (&dec_obj, &file_format,
                               &thumbnail_type,
                               &min_size_exif);
    if (ret != JPEGD_ERR_NO_ERROR)
    {
        exit (1);
    }

    if(dec_obj.dec_param.exif_info_needed && dec_info->file_format != JPEGD_FILE_IS_EXIF)
    {
       printf("not exif file \r\n");
       dec_obj.dec_param.exif_info_needed=0;
    }

    /* Query for memory required */
    ret = jpegd_query_dec_mem (&dec_obj);

    if (ret != JPEGD_ERR_NO_ERROR)
    {
    	if(dec_obj.dec_param.vpu_enable==1)
    	{
    		printf("vpu query failure: %d \r\n",ret);
    		dec_obj.dec_param.vpu_enable=0;
		goto restart;	
    	}
        exit (1);
    }

    /* Allocate required memory */
    {
        int i;
        for (i = 0; i < mem_info->num_reqs; i++)
        {
#ifdef VPU_SUPPORT        
        	if(mem_info->mem_info_sub[i].mem_type_usage==JPEGD_PHY_SUCCESSIVE_MEMORY)
        	{
        		RetCode vpu_ret;        		
			bitstream_mem_desc.size = mem_info->mem_info_sub[i].size;
			//printf("bitstream size: %d \r\n",bitstream_mem_desc.size);
			vpu_ret = GetPhyVirtMem(&bitstream_mem_desc);
			if (vpu_ret) {
				printf("Unable to obtain physical mem: %d \n",bitstream_mem_desc.size);
				dec_obj.dec_param.vpu_enable=0;
				bitstream_mem_desc.size=0;
				goto restart;	
			}
			//printf("virt addr: 0x%X , size: %d \r\n",bitstream_mem_desc.virt_uaddr,bitstream_mem_desc.size);
			mem_info->mem_info_sub[i].ptr=(void*)(bitstream_mem_desc.virt_uaddr);
			//printf("phy addr: 0x%X , size: %d \r\n",bitstream_mem_desc.phy_addr,bitstream_mem_desc.size);
			mem_info->mem_info_sub[i].phy_ptr=(void*)(bitstream_mem_desc.phy_addr);
        	}
		else
#endif			
		{
            	   mem_info->mem_info_sub[i].ptr =
                (void *) malloc (mem_info->mem_info_sub[i].size);
		}
		if(mem_info->mem_info_sub[i].ptr==NULL)
		{
			printf("Malloc error after query\n");
			return -1;
		}
        }
    }


    if(scale_mode==1)
    {
        printf("original [width,height]: [%d,%d] \r\n",dec_obj.dec_info.original_image_width,dec_obj.dec_info.original_image_height);
    	  dec_obj.dec_param.scale=scale_val;  //use scale value set by user
    }

    /* Initialize the decoder */
    ret = jpegd_decoder_init (&dec_obj);

    if (ret != JPEGD_ERR_NO_ERROR)
    {
      	if(dec_obj.dec_param.vpu_enable==1)
    	{
    		printf("vpu init failure: %d \r\n",ret);
    		dec_obj.dec_param.vpu_enable=0;
		goto restart;	
    	}
	else
	{
          printf("init failure: %d \r\n",ret);
	 }
        exit (1);
    }

    check_exif_info(&dec_obj);

    // Initialize the output module 
    if(output_file)
    {
	    switch (requested_fmt) {
	    case FMT_PPM:
		//if((dec_info->num_components==1)&&(dec_obj.dec_param.output_format == JPEGD_OFMT_GRAY))
		//	fprintf(output_file, "%s\n", "P5");
		//else	
			fprintf(output_file, "%s\n", "P6");
		fprintf(output_file, "%d %d\n", dec_obj.dec_info.actual_output_width,dec_obj.dec_info.actual_output_height);
		fprintf(output_file, "%d\n", 255);
		fflush(output_file);        
	        break;		
	    case FMT_BIN:
	        break;
	    default:
	        printf ("Unsupported format\n");
	        exit(1);
	        break;
	    }
    }

    /* Allocate output buffer */
    if (dec_obj.dec_param.output_format == JPEGD_OFMT_ENC_IMAGE_FMT)
    {
#ifdef VPU_SUPPORT
		if(dec_obj.dec_param.vpu_enable==1)
		{
			int ret;
			//printf("actual width: %d \r\n",dec_info->comp_info[0].actual_output_width);
			frame_buffer.size=0;
			frame_buffer.phy_addr=0;
			for (ci = 0, compptr = dec_info->comp_info;ci < dec_info->num_components; ci++, compptr++)
			{
			      //FIXME: for 420: uv_pad=y_pad/2, but for 444: uv_pad=y_pad !!
				pad= ((ci==0)?g_test_stride_y_pad:(g_test_stride_y_pad/2)); 
				frame_buffer.size+=dec_info->comp_info[ci].actual_output_height * (dec_info->comp_info[ci].actual_output_width+pad);
				out_stride_width[ci] = dec_info->comp_info[ci].actual_output_width+pad;
				//pt_YUV_temp[ci] = YUV_temp[ci] = (JPEGD_UINT8 *)malloc(dec_info->original_image_width * dec_info->original_image_height * sizeof(JPEGD_UINT8));
				pt_YUV_temp[ci] = YUV_temp[ci] = (JPEGD_UINT8 *)malloc((dec_info->actual_output_width+pad) * dec_info->actual_output_height * sizeof(JPEGD_UINT8));
				Buffer_size[ci] = 0;

			}
			ret = GetPhyVirtMem(&frame_buffer);
			if (ret)
			{
				printf("Unable to obtain physical mem\n");
    				dec_obj.dec_param.vpu_enable=0;
				frame_buffer.size=0;
				goto restart;					
			}

			if(out_stride_width[0]%16!=0) printf("error: vpu stride[0] must be aligned by 16 \r\n");
			
			output_buf[0]=(JPEGD_UINT8 *)(frame_buffer.phy_addr);
			output_buf[1]=output_buf[0]+dec_info->comp_info[0].actual_output_height * (dec_info->comp_info[0].actual_output_width+g_test_stride_y_pad);
			output_buf[2]=output_buf[1]+dec_info->comp_info[1].actual_output_height * (dec_info->comp_info[1].actual_output_width+(g_test_stride_y_pad/2));	
			//printf("malloc output: phy:0x%X, virt: 0x%X \r\n",frame_buffer.phy_addr,frame_buffer.virt_uaddr);
			//printf("virt: 0x%X, 0x%X, 0x%X \r\n",frame_buffer.virt_uaddr,
			//	frame_buffer.virt_uaddr+dec_info->comp_info[0].actual_output_height * (dec_info->comp_info[0].actual_output_width+g_test_stride_y_pad),
			//	frame_buffer.virt_uaddr+dec_info->comp_info[0].actual_output_height * (dec_info->comp_info[0].actual_output_width+g_test_stride_y_pad)+dec_info->comp_info[1].actual_output_height * (dec_info->comp_info[1].actual_output_width+(g_test_stride_y_pad/2)));
		}
		else
#endif	//VPU_SUPPORT
		{
		        for (ci = 0, compptr = dec_info->comp_info;
		             ci < dec_info->num_components; ci++, compptr++)
		        {
		            pad= ((ci==0)?g_test_stride_y_pad:(g_test_stride_y_pad/2)); 
#ifdef USE_DECODE_FRAME        
		            output_buf[ci] = (JPEGD_UINT8 *)malloc
		                (compptr->actual_output_height * (compptr->actual_output_width+pad) * sizeof(JPEGD_UINT8));
#else
		            output_buf[ci] = (JPEGD_UINT8 *)malloc
		                (compptr->max_lines * (compptr->actual_output_width+pad) * sizeof(JPEGD_UINT8));
#endif
		            out_stride_width[ci] = compptr->actual_output_width+pad;
					//pt_YUV_temp[ci] = YUV_temp[ci] = (JPEGD_UINT8 *)malloc(dec_info->original_image_width * dec_info->original_image_height * sizeof(JPEGD_UINT8));
					pt_YUV_temp[ci] = YUV_temp[ci] = (JPEGD_UINT8 *)malloc((dec_info->actual_output_width+pad) * dec_info->actual_output_height * sizeof(JPEGD_UINT8));
					Buffer_size[ci] = 0;
		        }			
		}

    }
    else	//RGB
    {
        int output_pixel_size = 3;
        if ((dec_obj.dec_param.output_format == JPEGD_OFMT_RGB_565)||(dec_obj.dec_param.output_format == JPEGD_OFMT_BGR_565))
        {
            output_pixel_size = 2;
        }
	  pad=g_test_stride_y_pad;
#ifdef USE_DECODE_FRAME    		
        output_buf[0] = (JPEGD_UINT8 *)malloc (output_pixel_size *
                                         dec_info->actual_output_height *
                                         (dec_info->actual_output_width+pad) *
                                         sizeof(JPEGD_UINT8));
#else
        output_buf[0] = (JPEGD_UINT8 *)malloc (output_pixel_size *
                                         dec_info->max_lines *
                                         (dec_info->actual_output_width+pad) *
                                         sizeof(JPEGD_UINT8));
#endif
        out_stride_width[0] = dec_info->actual_output_width+pad;
    }

	if((input_buf_size==CALLBACK_INPUT_LEN_INIT)||(input_buf_size==CALLBACK_INPUT_LEN_DEC))
	{
		//user do not set specified value for it through command option, so we can modify it to get better performance
		printf("reset the callback input size (bigger value): %d \r\n",CALLBACK_INPUT_LEN_DEC);  
    	input_buf_size=CALLBACK_INPUT_LEN_DEC;   // set input data length for callback, the value is set for decode frame api
	}
    //printf("will decode frame \r\n");
	timer_init();

    if(dec_obj.dec_param.vpu_enable==1)
    {
    	  callback_time_removed=0;
    }
    else
    {
    	  callback_time_removed=1;
    }


        /* Decoder loop to generate the YUV/RGB outputs */
        do
        {
            /* Decode few lines */
            /* out_stride_width[] should be set by the application such that
             *  it is >= output width returned by the decoder (initialization)
             */
		timer_start();
#ifdef USE_DECODE_FRAME
            ret = jpegd_decode_frame (&dec_obj, output_buf, out_stride_width);
#else
            ret = jpegd_decode_mcu_row (&dec_obj, output_buf, out_stride_width);
#endif
            timer_stop();

            if (ret != JPEGD_ERR_NO_ERROR)
            {
            		if(dec_obj.dec_param.vpu_enable==1)
            		{
            			printf("vpu decode failure: %d \r\n",ret);
            			dec_obj.dec_param.vpu_enable=0;
				goto restart;		
            		}
                   break;
            }

	     //printf("decode OK \r\n");
		 

#ifdef VPU_SUPPORT
		if(dec_obj.dec_param.vpu_enable==1)
		 {
			JPEGD_UINT8* base;
			base=(JPEGD_UINT8*)(frame_buffer.virt_uaddr);
		      for (ci = 0, compptr = dec_info->comp_info;ci < dec_info->num_components; ci++, compptr++)
        		{
        		      //FIXME: for 444: y_pad=uv_pad !!!
        		      pad= ((ci==0)?g_test_stride_y_pad:(g_test_stride_y_pad/2)); 
				output_buf[ci]=base;
				base+=compptr->actual_output_height*(compptr->actual_output_width+pad);
             	}
		}
#endif
		if(output_file)
		{
		   //if(g_test_stride_y_pad !=0)	   printf("PAD: %d, output stride: %d !!!!!! \r\n",g_test_stride_y_pad,out_stride_width[0]);
                write_outputs(&dec_obj, output_buf, out_stride_width, output_file);
		}

        } while (dec_info->output_scanline < dec_info->actual_output_height);


	printf("decode finish: width: %d, height: %d  \r\n",dec_info->actual_output_width, dec_info->actual_output_height);
	timer_report();	
	
    /* finish output */
    if (output_file)
    {
	fflush(output_file);
	if (dec_obj.dec_param.output_format == JPEGD_OFMT_ENC_IMAGE_FMT)
	{
		for (ci = 0, compptr = dec_info->comp_info; ci < dec_info->num_components; ci++, compptr++)
		{
			fwrite (YUV_temp[ci],1,Buffer_size[ci],output_file);
			free(YUV_temp[ci]);
		}	
	}

	fclose(output_file);
	output_file=NULL;	
    }
	
    /* Free the allocated memory */
    {
        int i;
        for (i = 0; i < mem_info->num_reqs; i++)
        {
#ifdef VPU_SUPPORT	        
		if(mem_info->mem_info_sub[i].mem_type_usage!=JPEGD_PHY_SUCCESSIVE_MEMORY)
#endif
            free (mem_info->mem_info_sub[i].ptr);
        }
    }

	
    /* Run the decoder again in thumbnail mode */
    if ((mode == 2) &&
        (dec_obj.dec_param.decoding_mode == JPEGD_PRIMARY) &&
        (thumbnail_type == JPEGD_THUMBNAIL_JPEG))
    {
        dec_obj.dec_param.decoding_mode = JPEGD_THUMBNAIL;
        fseek(input_file, 0, SEEK_SET);
#ifdef VPU_SUPPORT	
	  if(vpu_mode==1)
	  {
	  	dec_obj.dec_param.vpu_enable=1;
	  }

	  if(frame_buffer.size!=0)
	  {
	  	//release at restart
	  }
	  else
#endif	
         {
		/* Free the output buffer */
		for (ci = 0; ci < JPEGD_MAX_NUM_COMPS; ci++)
		{
			if(output_buf[ci])
				free (output_buf[ci]);
			output_buf[ci]=NULL;
		}
	  }
        goto restart;
    }

    if (input_file)
        fclose(input_file);

#ifdef VPU_SUPPORT	
	//release bitstream buffer
	if(bitstream_mem_desc.size!=0)
	{
		FreePhyVirtMem(&bitstream_mem_desc);
	}

	//release frame output buffer
	if(frame_buffer.size!=0)
	{
		FreePhyVirtMem(&frame_buffer);
	}
	else
#endif		
	{
		/* Free the output buffer */
		for (ci = 0; ci < JPEGD_MAX_NUM_COMPS; ci++)
		{
			if(output_buf[ci]) free (output_buf[ci]);
		}
	}

#ifdef VPU_SUPPORT	
	DeinitPhyDevice();
#endif

    //free bitstream buffer
    free(JPEGD_input_buffer_ptr);

    /* All done. */
    return 0;
}

/***************************************************************************
 *
 *   FUNCTION NAME - jpegd_get_new_data
 *
 *   DESCRIPTION
 *      This function gets new input data for the given decoder object.
 *      The buffer should hold minimum of one MCU worth data to facilitate
 *       the suspension.
 *
 *   ARGUMENTS
 *      ppBuf               - Pointer to buffer pointer
 *      pLen                - Pointer to buffer length
 *      mcu_offset          - Backward offset to the recent MCU
 *      begin_flag          - 1, provide bitstream from the begining
 *                            0, provide from the current position
 *      obj_ptr             - Decoder object pointer
 *
 *   RETURN VALUE
 *      JPEGD_SUCCESS
 *      JPEGD_END_OF_FILE - End of input data
 *
 ***************************************************************************/
JPEGD_UINT8 jpegd_get_new_data(JPEGD_UINT8 **ppBuf, JPEGD_UINT32 *pLen, JPEGD_UINT32 mcu_offset,
                   JPEGD_UINT8 begin_flag, void *obj_ptr)
{
    int  iBytesRead = 0;
    unsigned int removed_time=callback_time_removed;

 
    if (obj_ptr != (JPEGD_Decoder_Object *)p_dec_obj)
    {
        fprintf (stderr, "Wrong decoder object pointer\n");
        exit(1);
    }

	if(1==removed_time)
	{
		timer_stop();
	}

	/* Seek to the begining of the file again */
    if (begin_flag == 1)
    {
    	 //printf("seek bistream to header \r\n");
        fseek(input_file, 0, SEEK_SET);
    }

    if(*ppBuf	==NULL)	// application decide the buffer address 
    {
	    iBytesRead = fread (JPEGD_input_buffer_ptr,1,input_buf_size,input_file);

	    *pLen = iBytesRead;
           if(iBytesRead>0)
	    {	
	         *ppBuf = JPEGD_input_buffer_ptr;
           }
    }
    else	// application copy data into the address which is pointed by decoder
    {
		iBytesRead = fread (*ppBuf,1,*pLen, input_file);

		if (iBytesRead != *pLen) {
			*pLen = iBytesRead;
		}    	
		//printf("copy data: 0x%X, size: %d \r\n",*ppBuf,*pLen);
    }


	if(1==removed_time)
	{
		timer_start();
	}

    if (iBytesRead <= 0)
    {
        iBytesRead = 0;
        //*ppBuf = NULL;
        //*pLen = iBytesRead; 
        return JPEGD_END_OF_FILE;
    }


    return JPEGD_SUCCESS;
}

#ifdef __WINCE
#define NAME_SIZE 255
int _tmain(int argc,_TCHAR *argv[])
{

            char* argv_char[NAME_SIZE];

            int argc_size,i;

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

/* End of file */
