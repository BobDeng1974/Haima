/*
 * wrbmp.h
 *
 * Copyright (C) 1994-1997, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 */
/****************************************************************************
 *
 * (C) 2003 MOTOROLA INDIA ELECTRONICS LTD.
 *
 *   CHANGE HISTORY
 *   dd/mm/yy   Code Ver    Description                         Author
 *   --------   -------     -----------                         ------
 *   01/06/04   01          Changed filename to wrbmp.h         B.Venkatarao
 *                          Removed library related
 *                          includes, remove unnecessay code.
 *   07/01/05   02          Prefix all the data types, defines  B.Venkatarao
 *                           exposed to API
 *
 ****************************************************************************/
	 
	 /************************************************************************
	  * Copyright 2005-2010 by Freescale Semiconductor, Inc.
	  * All modifications are confidential and proprietary information
	  * of Freescale Semiconductor, Inc. 
	  ************************************************************************/
	 


#define JMETHOD(type,methodname,arglist)  type (*methodname) arglist

/*
 * Object interface for djpeg's output file encoding modules
 */
typedef struct djpeg_dest_struct * djpeg_dest_ptr;

struct djpeg_dest_struct {
  /* start_output is called after jpeg_start_decompress finishes.
   * The color map will be ready at this time, if one is needed.
   */
  JMETHOD(void, start_output, (JPEGD_Decoder_Object *dec_obj,
			       djpeg_dest_ptr dinfo));
  /* Emit the specified number of pixel rows from the buffer. */
  JMETHOD(void, put_pixel_rows, (JPEGD_Decoder_Object *dec_obj,
				 djpeg_dest_ptr dinfo,
				 int rows_supplied));
  /* Finish up at the end of the image. */
  JMETHOD(void, finish_output, (JPEGD_Decoder_Object *dec_obj,
				djpeg_dest_ptr dinfo));

  /* Target file spec; filled in by djpeg.c after object is created. */
  FILE * output_file;

  /* Output pixel-row buffer.  Created by module init or start_output.
   * Width is cinfo->output_width * cinfo->output_components;
   * height is buffer_height.
   */
  JPEGD_UINT8 **buffer;
  int buffer_height;
};

/* Global function prototypes */
extern djpeg_dest_ptr jinit_write_bmp (JPEGD_Decoder_Object *dec_obj,
                                       int is_os2);
