
/******************************************************************************
 *
 *   MOTOROLA GENERAL BUSINESS USE
 *
 *
 *   (C) 2004 MOTOROLA INDIA ELECTRONICS PVT. LTD.
 *
 *   FILENAME        - gif_test.c
 *   ORIGINAL AUTHOR - M.Intiyas Pasha
 *
 *******************************************************************************
 ***********************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc.
 ***********************************************************************
 *
 *   CHANGE HISTORY
 *   dd/mm/yy        Code Ver       Author				Description
 *   --------        --------      -------	            ------------
 *   27/06/2004      0.1          M.Intiyas Pasha  		Initial version
 *   19/12/2004      0.2          Sameer P. Rapate  	Modified version
 *   16/11/2006      0.3          Manjunath H S         Modified for
 *                                                      profile on RVDS
 *                                                      and ELinux:TLSbo83403
 *   04/01/2007      0.4          Durgaprasad S.Bilagi  Stack & Heap measurement
 *                                                      included:tlsbo87093
 *  15/04/2008      0.5           Eagle Zhou           	adjust strategy for transpanrent color/chang size/....
			 *                                        		related CRs: ENGR69242/ENGR70265/ENGR70654
 *  17/11/2008      0.6           Eagle Zhou			ENGR00099100: add BGR format
 *******************************************************************************
 *
 *   DESCRIPTION
 *   Example file to test gif decoding function on host (windows/unix) platform
 *****************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"gif_dec_interface.h"


#define INPUT_IMAGE_BUFFER_SIZE 4096

/* Include the header file for the function used */

#ifdef TIME_PROFILE
#include <sys/time.h>
#endif

#ifdef __WINCE
	#include<windows.h>
#endif

#ifdef MEASURE_STACK_USAGE
#include "mm_codec_common.h"
#endif

#ifdef MEASURE_HEAP_USAGE
    unsigned int     s32TotalMallocBytes = 0;
#endif

//#define TEST_SUSPENSION 1

FILE *fp_input_gif;
char buff[INPUT_IMAGE_BUFFER_SIZE];

char *filename_temp;
char filename_bak[255];
char filename_delim[]=".";


EXPORT_C int fill_background(GIF_Decoder_Object *gif_dec_obj,unsigned char* image_addr/*colormap bkgnd??*/)
{
	int i,j;
	int width = gif_dec_obj->dec_info_init.glob_out_width;
	int height = gif_dec_obj->dec_info_init.glob_out_height;

	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char * filladdr;

	if((gif_dec_obj->dec_param.out_format == E_GIF_OUTPUTFORMAT_BGR888)
	   ||(gif_dec_obj->dec_param.out_format == E_GIF_OUTPUTFORMAT_BGR555)
	   ||(gif_dec_obj->dec_param.out_format == E_GIF_OUTPUTFORMAT_BGR565)
	   ||(gif_dec_obj->dec_param.out_format == E_GIF_OUTPUTFORMAT_BGR666))
	{
		red=0xd8;	//B
		green=0xe9;	//G
		blue=0xec;	//R
	}
	else
	{
		red=0xec;		//1 decided by system?
		green=0xe9;
		blue=0xd8;
	}

	filladdr=image_addr;

	if((gif_dec_obj->dec_param.out_format == E_GIF_OUTPUTFORMAT_RGB888)
	  ||(gif_dec_obj->dec_param.out_format == E_GIF_OUTPUTFORMAT_BGR888))
	{
		for (i = 0; i < height; ++i)
		{
			for(j=0;j<width;j++)
			{
				filladdr[0] = red;
				filladdr[1] = green;
				filladdr[2] = blue;
				filladdr += 3;
			}
		}
	}
	else if ((gif_dec_obj->dec_param.out_format == E_GIF_OUTPUTFORMAT_RGB666)
	  ||(gif_dec_obj->dec_param.out_format == E_GIF_OUTPUTFORMAT_BGR666))
	{
		for (i = 0; i < height; ++i)
		{
			for(j=0;j<width;j++)
			{
				filladdr[0] = red & 0xFFFC;
				filladdr[1] = green & 0xFFFC;
				filladdr[2] = blue & 0xFFFC;
				filladdr += 3;
			}
		}
	}

	else if ((gif_dec_obj->dec_param.out_format == E_GIF_OUTPUTFORMAT_RGB565)
	  ||(gif_dec_obj->dec_param.out_format == E_GIF_OUTPUTFORMAT_BGR565))
	{
		GIF_UINT16 out_pixel, *out_pix_buff;
		out_pix_buff = (GIF_UINT16*)filladdr;
		for (i = 0; i < height; ++i)
		{
			for(j=0;j<width;j++)
			{
			        out_pixel = (GIF_UINT16)((((red >> 3) & 0x001F) << 11) & 0xFFFF);
			        out_pixel|= (GIF_UINT16)((((green >> 2)& 0x003F) << 5) & 0xFFFF);
			        out_pixel|= (GIF_UINT16)(((blue >> 3) & 0x001F) & 0xFFFF);
				*out_pix_buff++=out_pixel;
			}
		}
	}
	else if ((gif_dec_obj->dec_param.out_format == E_GIF_OUTPUTFORMAT_RGB555)
	  ||(gif_dec_obj->dec_param.out_format == E_GIF_OUTPUTFORMAT_BGR555))
	{
		GIF_UINT16 out_pixel, *out_pix_buff;
		out_pix_buff = (GIF_UINT16*)filladdr;
		for (i = 0; i < height; ++i)
		{
			for(j=0;j<width;j++)
			{
				out_pixel = (GIF_UINT16)((((red >> 3) & 0x001F) << 11) & 0xFFFF);
				out_pixel|= (GIF_UINT16)((((green >> 3)& 0x001F) << 6) & 0xFFFF);
				out_pixel|= (GIF_UINT16)((((blue>> 3) & 0x001F) << 1) & 0xFFFF);
				*out_pix_buff++=out_pixel;
			}
		}
	}
	else
	{
		//
	}


}


void convert_ppm(unsigned char* inbuffer,unsigned char* outbuffer,gif_output_format format,unsigned int width, unsigned int height)
{
	// input format : 	BGR or RGB 555/565/666/888
	// output format:	RGB888
	// overwrite from tail to end in order use the same buffer when (inbuffer==outbuffer)
	int i,total_pixels;
	unsigned char * src8;
	unsigned short * src16;
	unsigned char * dst;
	unsigned char temp8;
	unsigned short temp16;

	total_pixels=width*height;
	if(format==E_GIF_OUTPUTFORMAT_RGB888)
	{
		if(inbuffer==outbuffer)
		{
			//do nothing
		}
		else
		{
			src8=inbuffer+total_pixels*3;
			dst=outbuffer+total_pixels*3;
			for(i=0;i<total_pixels;i++)
			{
				src8-=3;
				dst-=3;
				dst[0]=src8[0];
				dst[1]=src8[1];
				dst[2]=src8[2];
			}
		}
	}
	else if (format==E_GIF_OUTPUTFORMAT_BGR888)
	{
		src8=inbuffer+total_pixels*3;
		dst=outbuffer+total_pixels*3;
		for(i=0;i<total_pixels;i++)
		{
			src8-=3;
			dst-=3;
			temp8=src8[0];
			dst[0]=src8[2];
			dst[1]=src8[1];
			dst[2]=temp8;
		}
	}
	else if (format==E_GIF_OUTPUTFORMAT_RGB666)
	{
		if(inbuffer==outbuffer)
		{
			//do nothing
		}
		else
		{
			src8=inbuffer+total_pixels*3;
			dst=outbuffer+total_pixels*3;
			for(i=0;i<total_pixels;i++)
			{
				src8-=3;
				dst-=3;
				dst[0]=src8[0];
				dst[1]=src8[1];
				dst[2]=src8[2];
			}
		}
	}
	else if (format==E_GIF_OUTPUTFORMAT_BGR666)
	{
		src8=inbuffer+total_pixels*3;
		dst=outbuffer+total_pixels*3;
		for(i=0;i<total_pixels;i++)
		{
			src8-=3;
			dst-=3;
			temp8=src8[0];
			dst[0]=src8[2];
			dst[1]=src8[1];
			dst[2]=temp8;
		}	}
	else if (format==E_GIF_OUTPUTFORMAT_RGB565)
	{
		src16=(unsigned short*)(inbuffer+total_pixels*2);
		dst=outbuffer+total_pixels*3;
		for(i=0;i<total_pixels;i++)
		{
			src16-=1;
			dst-=3;
			temp16=*src16;
			dst[0]=((temp16>>11)&0x1F)<<3;	//R
			dst[1]=((temp16>>5)&0x3F)<<2;	//G
			dst[2]=((temp16)&0x1F)<<3;	//B
		}
	}
	else if (format==E_GIF_OUTPUTFORMAT_BGR565)
	{
		src16=(unsigned short*)(inbuffer+total_pixels*2);
		dst=outbuffer+total_pixels*3;
		for(i=0;i<total_pixels;i++)
		{
			src16-=1;
			dst-=3;
			temp16=*src16;
			dst[0]=((temp16)&0x1F)<<3;	//R
			dst[1]=((temp16>>5)&0x3F)<<2;	//G
			dst[2]=((temp16>>11)&0x1F)<<3;	//B
		}
	}
	else if (format==E_GIF_OUTPUTFORMAT_RGB555)
	{
		src16=(unsigned short*)(inbuffer+total_pixels*2);
		dst=outbuffer+total_pixels*3;
		for(i=0;i<total_pixels;i++)
		{
			src16-=1;
			dst-=3;
			temp16=*src16;
			dst[0]=((temp16>>11)&0x1F)<<3;	//R
			dst[1]=((temp16>>6)&0x1F)<<3;	//G
			dst[2]=((temp16>>1)&0x1F)<<3;	//B
		}
	}
	else if (format==E_GIF_OUTPUTFORMAT_BGR555)
	{
		src16=(unsigned short*)(inbuffer+total_pixels*2);
		dst=outbuffer+total_pixels*3;
		for(i=0;i<total_pixels;i++)
		{
			src16-=1;
			dst-=3;
			temp16=*src16;
			dst[0]=((temp16>>1)&0x1F)<<3;	//R
			dst[1]=((temp16>>6)&0x1F)<<3;	//G
			dst[2]=((temp16>>11)&0x1F)<<3;	//B
		}
	}
	else
	{
		printf("unknown output format ! \r\n");
	}

	return;
}

/*******************************************************************************
 *
 *   FUNCTION NAME - main
 *
 *   ARGUMENTS     - argc  :  Argument Count
 *                   argv[]:  Array of pointer to strings .
 *                            Filenames provided as argument to decoder.
 *
 *   RETURN VALUE  - int : Error value
 *
 *******************************************************************************
 *
 *   DESCRIPTION
 *   Sample decoding function to test bmp decoder on unix/windows/eLinux on ARM
 ******************************************************************************/
int main(int argc,char *argv[])
{
	GIF_Decoder_Object gif_dec_obj;
	GIFD_RET_TYPE gif_query_err,gif_init_err,gif_dec_err;
	int memreq,mem_out_pix;
	char *out_image,*out_buf;
	int glob_mem_index=0,index=0,mult_factor=3;

#ifdef SAVE_OUTPUT
    FILE *fp_decoded_gif;
#endif
    char out_str[255];
	int counter=0;
#ifdef TEST_SUSPENSION
	int suspended = 0;
	int temp=0;
#endif

#ifdef __WINCE         // for taking timing on wince platform
	BOOL Flag;
	LARGE_INTEGER lpFrequency1 ;
	LARGE_INTEGER lpPerformanceCount1;
	LARGE_INTEGER lpPerformanceCount2;
	__int64 TotalDecTime=0;
	__int64 Temp;
#endif

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

	unsigned char* image_buf1;
	unsigned char* image_buf2;
	unsigned int image_size;
	unsigned int screen_width;
	unsigned int screen_height;

#ifdef MEASURE_HEAP_USAGE
       s32TotalMallocBytes = 0;
#endif

	/*Check for correct count of program arguments*/
	if(argc!=7)
	{
		printf("Number of arguments should be equal to 6 \n");
		printf("Usage :<input-file> <output-file> <output-format> \
<output-width> <output-height> <scaling-mode>");

		return -1;
	}

	fp_input_gif=fopen(argv[1],"rb");
	if(fp_input_gif==NULL)
	{
		printf("Error Opening input GIF file\n");
		return -1;
	}
	strcpy(filename_bak,argv[1]);

	/*Parameter initialization*/

	gif_dec_obj.dec_param.out_format=atoi(argv[3]);
	gif_dec_obj.dec_param.output_width=atoi(argv[4]);
	gif_dec_obj.dec_param.output_height=atoi(argv[5]);
	gif_dec_obj.dec_param.scale_mode=atoi(argv[6]);

	/*Validation of configured parameters*/

	if(gif_dec_obj.dec_param.out_format<0 || gif_dec_obj.dec_param.out_format>=E_GIF_LAST_OUTPUT_FORMAT)
	{
		printf("Configured output Format is invalid \r\n");
		return -1;
	}
	if(gif_dec_obj.dec_param.scale_mode<0 || gif_dec_obj.dec_param.scale_mode>1)
	{
		printf("Configured scaling mode is invalid \r\n");
		return -1;
	}
	if(gif_dec_obj.dec_param.output_width<=0)
	{
		printf("Configured width is invalid  \r\n");
		return -1;
	}

	if(gif_dec_obj.dec_param.output_height<=0)
	{
		printf("Configured height is invalid \r\n ");
		return -1;
	}

	// check api version
	printf("version: %s\n", GIFD_CodecVersionInfo());

	/*Initialization of callback Function pointer */
	gif_dec_obj.GIF_get_new_data=GIF_get_new_data;

	/*Set the multiplication factor depending upon the
	  output format specified.This mutiplication factor
	  is useful for allocation of output buffer
	*/
//dsphl28117
	if ((gif_dec_obj.dec_param.out_format == E_GIF_OUTPUTFORMAT_RGB555)
			      || (gif_dec_obj.dec_param.out_format == E_GIF_OUTPUTFORMAT_RGB565)
			      || (gif_dec_obj.dec_param.out_format == E_GIF_OUTPUTFORMAT_BGR555)
			      || (gif_dec_obj.dec_param.out_format == E_GIF_OUTPUTFORMAT_BGR565))
	{
		mult_factor=2;
	}
	/*For RGB888 and RGB666*/
	else
	{
		mult_factor=3;
	}

	/* Query global memory requirements of the decoder*/
	gif_query_err=GIF_query_dec_mem (&gif_dec_obj);

	if(gif_query_err!=GIF_ERR_NO_ERROR)
	{
		printf("Memory Query error \n");
		return -1;
	}

	for(memreq=0;memreq<gif_dec_obj.mem_info.num_reqs;memreq++)
	{
		gif_dec_obj.mem_info.mem_info_sub[memreq].ptr
			=(void *)calloc((gif_dec_obj.mem_info.mem_info_sub[memreq].size),sizeof(char));
		if(gif_dec_obj.mem_info.mem_info_sub[memreq].ptr==NULL)
		{
			printf("Malloc error after query\n");
			return -1;
		}
        else
        {
          #ifdef MEASURE_HEAP_USAGE
             s32TotalMallocBytes+=(gif_dec_obj.mem_info.mem_info_sub[memreq].size);
         #endif

        }
	}
	/*Save the mem requests count*/
	glob_mem_index=gif_dec_obj.mem_info.num_reqs;

	/*Seek to the start of the file befor calling init*/

    if(fseek(fp_input_gif, 0, SEEK_SET) != 0)
    {
        return -1;
    }

	/*Initialization of decoder for global data*/
#ifdef MEASURE_STACK_USAGE
    PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
#endif

    gif_init_err=GIF_decoder_init(&gif_dec_obj);

	screen_width=gif_dec_obj.dec_info_init.glob_out_width;
	screen_height=gif_dec_obj.dec_info_init.glob_out_height;
	image_size=screen_width*screen_height*mult_factor;
	image_buf1=malloc(image_size*2);
	if(!image_buf1)
	{
		printf("malloc image buffer fail \r\n");
		return -1;
	}
	printf("image: width: %d, height: %d \r\n",screen_width,screen_height);
	image_buf2=image_buf1+image_size;
      #ifdef MEASURE_HEAP_USAGE
           s32TotalMallocBytes+=(image_size*2);
      #endif

	// fill default back ground !!!
	//1memset(image_buf1,0xFF,image_size*2);
	fill_background(&gif_dec_obj,image_buf1);
	fill_background(&gif_dec_obj,image_buf2);


#ifdef MEASURE_STACK_USAGE
    GET_STACK_USAGE (ps32BaseSP, ps32SP, s32StackCount, s32PeakStack);
#endif

	if(gif_init_err != GIF_ERR_NO_ERROR)
	{
        printf("error GIF_decoder_init \n");
		return -1;
	}

	/*Decode frame by frame.Break from the loop
	  when Terminator block is reached */
	while(1)
	{
#ifdef TEST_SUSPENSION
		if (suspended == 1)
		{
			suspended = 0;
			temp = 0 - gif_dec_obj.bytes_read_in_a_frame ;

			printf("File pointer before seek = %ld \n", ftell(fp_input_gif));
            if(fseek(fp_input_gif, temp, SEEK_CUR) != 0)
			{
                return -1;
			}
            printf("File pointer after seek = %ld \n", ftell(fp_input_gif));
		}
#endif

		/*Query for memory requirements of a frame*/
		gif_query_err=GIF_query_dec_mem_frame(&gif_dec_obj);

#ifdef TEST_SUSPENSION
		if (gif_query_err == GIFD_SUSPEND)
		{
			suspended = 1;
			printf("Number of bytes read in the frame= %d\n", gif_dec_obj.bytes_read_in_a_frame);

			printf("Data not yet ready\n");
			continue;
		}
#endif

		if (gif_query_err==GIF_ERR_TERMINATOR_REACHED)
		{
			printf("End of GIF file reached.\n");
			break;
		}
		else if(gif_query_err!=GIF_ERR_NO_ERROR)
		{
			printf("Error in querying memory requirments for a frame");
			return -1;

		}

		for(memreq=glob_mem_index;memreq<gif_dec_obj.mem_info.num_reqs;memreq++)
		{
			gif_dec_obj.mem_info.mem_info_sub[memreq].ptr
				=(void *)calloc((gif_dec_obj.mem_info.mem_info_sub[memreq].size),sizeof(char));
			if(gif_dec_obj.mem_info.mem_info_sub[memreq].ptr==NULL)
			{
				printf("Malloc error after query\n");
				return -1;
			}
            else
            {
        #ifdef MEASURE_HEAP_USAGE
            s32TotalMallocBytes+=(gif_dec_obj.mem_info.mem_info_sub[memreq].size);
        #endif

            }
		}
		/*Initializiation for a frame*/
       #ifdef MEASURE_STACK_USAGE
             PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
       #endif

		gif_init_err=GIF_decoder_init_frame(&gif_dec_obj);

       #ifdef MEASURE_STACK_USAGE
             GET_STACK_USAGE (ps32BaseSP, ps32SP, s32StackCount, s32StackValue);
       #endif /* MEASURE_STACK_USAGE */
       #ifdef MEASURE_STACK_USAGE
             if (s32PeakStack < s32StackValue)
                 s32PeakStack = s32StackValue;
       #endif

#ifdef TEST_SUSPENSION
		if (gif_init_err == GIFD_SUSPEND)
		{
			suspended = 1;
			printf("Number of bytes in the frame= %d\n", gif_dec_obj.bytes_read_in_a_frame);

			printf("Data not yet ready\n");
			for(index = glob_mem_index; index < gif_dec_obj.mem_info.num_reqs; index++)
			{
				free(gif_dec_obj.mem_info.mem_info_sub[index].ptr);
			}
			continue;
		}
#endif


		/*Application can access the spatial and timing info
		  for a frame here by reading relevant decoder info init
		  members.*/
		if(gif_init_err!=GIF_ERR_NO_ERROR)
		{
			printf("Error in initialization of frame data");
			return -1;
		}


		mem_out_pix = gif_dec_obj.dec_info_init.out_image_width
                       * gif_dec_obj.dec_info_init.out_image_height
                       * mult_factor;
#if 0
		out_image = (char*) calloc(mem_out_pix,sizeof(char));
		out_buf=out_image;
		if(out_image == NULL)
		{
			printf(" out_image memory allocation error \n");
			return -1;
		}

#ifdef MEASURE_STACK_USAGE
        PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
#endif
#endif
	out_image=image_buf1;
	//memcpy(image_buf1,image_buf2,image_size);
	if (gif_dec_obj.dec_info_init.disposal_method==3 ) //GIF_DISPOSE_RESTORE
	{
		memcpy(image_buf2,image_buf1,image_size);	// back up previous image
	}

#ifdef TIME_PROFILE
		gettimeofday(&StartTime, NULL);
#endif

#ifdef TIME_PROFILE_RVDS
	prev_clk = prev_cycles();
#endif

#ifdef __WINCE
       Flag=QueryPerformanceFrequency(&lpFrequency1);
	   Flag=QueryPerformanceCounter(&lpPerformanceCount1);
#endif
		/*Decode a frame*/

		gif_dec_err=GIF_decode(&gif_dec_obj,(unsigned char*)out_image);

#ifdef __WINCE
		Flag=QueryPerformanceCounter(&lpPerformanceCount2);
		Temp=(((lpPerformanceCount2.QuadPart - lpPerformanceCount1.QuadPart)*1000000)/(lpFrequency1.QuadPart));
		TotalDecTime += Temp;
#endif

#ifdef TIME_PROFILE_RVDS
	curr_clk = curr_cycles();
	clk = (curr_clk-prev_clk);      /* clk gives the total core cycles per frame call */
	total_clk += clk;
	if(clk > max_clk)
	     max_clk = clk;
#endif

#ifdef TIME_PROFILE
				gettimeofday(&EndTime, NULL);
				TotalDecTimeUs += (EndTime.tv_usec - StartTime.tv_usec) + (EndTime.tv_sec - StartTime.tv_sec)*1000000L;
#endif

#ifdef MEASURE_STACK_USAGE
     GET_STACK_USAGE (ps32BaseSP, ps32SP, s32StackCount, s32StackValue);
#endif /* MEASURE_STACK_USAGE */


#ifdef TEST_SUSPENSION
		if (gif_dec_err == GIFD_SUSPEND)
		{
			suspended = 1;
			printf("Number of bytes in the frame= %d\n", gif_dec_obj.bytes_read_in_a_frame);

			printf("Data not yet ready\n");
			for(index = glob_mem_index; index < gif_dec_obj.mem_info.num_reqs; index++)
			{
				free(gif_dec_obj.mem_info.mem_info_sub[index].ptr);
			}
			free(out_buf);
			continue;
		}
#endif
		if(gif_dec_err!=GIF_ERR_DECODING_COMPLETE)
		{
			printf("Error in decoding frame data");
			return -1;
		}

		printf("Decoding done successfully \n");

#ifdef MEASURE_STACK_USAGE
    if (s32PeakStack < s32StackValue)
       s32PeakStack = s32StackValue;
#endif

		/*Write decoded output of the frame to a file.
		  The output filename(corresponding to each frame)
		  is prefixed with the frame number*/
#ifdef SAVE_OUTPUT

#if 1	// ignore outputfile option: argv[2]
	filename_temp=strtok(filename_bak,filename_delim);
	if(gif_dec_obj.dec_param.out_format == E_GIF_OUTPUTFORMAT_RGB888)
	{
		sprintf(out_str,"%s_%d_rgb888.ppm",filename_temp,counter);
	}
	else if(gif_dec_obj.dec_param.out_format == E_GIF_OUTPUTFORMAT_BGR888)
	{
		sprintf(out_str,"%s_%d_bgr888.ppm",filename_temp,counter);
	}
	else if(gif_dec_obj.dec_param.out_format == E_GIF_OUTPUTFORMAT_RGB565)
	{
		sprintf(out_str,"%s_%d_rgb565.ppm",filename_temp,counter);
	}
	else if(gif_dec_obj.dec_param.out_format == E_GIF_OUTPUTFORMAT_BGR565)
	{
		sprintf(out_str,"%s_%d_bgr565.ppm",filename_temp,counter);
	}
	else if(gif_dec_obj.dec_param.out_format == E_GIF_OUTPUTFORMAT_RGB555)
	{
		sprintf(out_str,"%s_%d_rgb555.ppm",filename_temp,counter);
	}
	else if(gif_dec_obj.dec_param.out_format == E_GIF_OUTPUTFORMAT_BGR555)
	{
		sprintf(out_str,"%s_%d_bgr555.ppm",filename_temp,counter);
	}
	else if(gif_dec_obj.dec_param.out_format == E_GIF_OUTPUTFORMAT_RGB666)
	{
		sprintf(out_str,"%s_%d_rgb666.ppm",filename_temp,counter);
	}
	else if(gif_dec_obj.dec_param.out_format == E_GIF_OUTPUTFORMAT_BGR666)
	{
		sprintf(out_str,"%s_%d_bgr666.ppm",filename_temp,counter);
	}
	else
	{
		sprintf(out_str,"%s_%d_rgb.ppm",filename_temp,counter);
	}
	fp_decoded_gif=fopen(out_str,"wb");
#else
		sprintf(out_str,"%d_%s",counter,argv[2]);
#ifdef __SYMBIAN32__
        fp_decoded_gif=fopen(argv[2],"wb");
#else
#ifdef __WINCE
		fp_decoded_gif=fopen(argv[2],"wb");
#else
        fp_decoded_gif=fopen(out_str,"wb");
#endif
#endif
#endif
		if(fp_decoded_gif==NULL)
		{
			printf("Error Opening decoded file \n");
			return -1;
		}

		fprintf(fp_decoded_gif, "%s\n", "P6");

		fprintf(fp_decoded_gif, "%d %d\n", gif_dec_obj.dec_info_init.out_image_width,
			gif_dec_obj.dec_info_init.out_image_height);

		fprintf(fp_decoded_gif, "%d\n", 255);
		fflush(fp_decoded_gif);
		//printf("image size: %d \r\n", mem_out_pix);
#if 1
		{
			unsigned char* temp_output;
			unsigned int totalsize;
			totalsize=gif_dec_obj.dec_info_init.out_image_width*gif_dec_obj.dec_info_init.out_image_height*3;
			temp_output=malloc(totalsize);
			if(temp_output)
			{
				//we can not use the same output buffer since some transparent pixels may be referenced by following images !!
				convert_ppm(out_image,temp_output,gif_dec_obj.dec_param.out_format, gif_dec_obj.dec_info_init.out_image_width,gif_dec_obj.dec_info_init.out_image_height);
				fwrite(temp_output, sizeof(char), (unsigned int)(totalsize), fp_decoded_gif);
				free(temp_output);
			}
			else
			{
				printf("get output buffer null ! \r\n");
			}
		}
#else
		fwrite(out_image, sizeof(char), (unsigned int)(mem_out_pix), fp_decoded_gif);
#endif
		fclose(fp_decoded_gif);
#endif
		//1free(out_buf);


		// Prepare second image with next starting
		if (gif_dec_obj.dec_info_init.disposal_method== 2) //GIF_DISPOSE_BACKGND
		{
			//TRACE("*** GIF_DISPOSE_BACKGND ***\n");
			printf("postprocess method: dispose back ground !! \r\n");
			//to avoid add api interface: we move the disposition into library, so application need not care this operation.
			//GIF_dispose_background(&gif_dec_obj,image_buf1/*colormap background ??*/);
		}
		else if (gif_dec_obj.dec_info_init.disposal_method == 3) //GIF_DISPOSE_RESTORE
		{
			// restore previous image
			printf("postprocess method: dispose restore ! \r\n");
			memcpy(image_buf1,image_buf2,image_size);
		}
		else
		{
			// do nothing
		}


        /*Free up the local memory info pointers*/
		for(index = glob_mem_index; index < gif_dec_obj.mem_info.num_reqs; index++)
		{
			free(gif_dec_obj.mem_info.mem_info_sub[index].ptr);
		}
		counter++;

	}

	/*Free up the global memory info pointers*/
    for(index = 0; index < glob_mem_index; index++)
    {
        free(gif_dec_obj.mem_info.mem_info_sub[index].ptr);
    }

	// free image buffers
	free(image_buf1);

	fclose(fp_input_gif);
#ifdef TIME_PROFILE
		//printf("Total Decode Time [microseconds] = %ld\n", TotalDecTimeUs);
		printf("\n%s\t%s\t%dx%d\t%dx%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",chname,argv[1],
			gif_dec_obj.dec_info_init.image_width, gif_dec_obj.dec_info_init.image_height,
			gif_dec_obj.dec_param.output_width, gif_dec_obj.dec_param.output_height,gif_dec_obj.dec_param.out_format,
			Max_time,Min_time,n_frames,f_Max_time,f_Min_time,TotalDecTimeUs);
#endif

#ifdef TIME_PROFILE_RVDS
        #ifdef MEASURE_STACK_USAGE
        #ifdef MEASURE_HEAP_USAGE
        printf("\n%s\t%s\t%dx%d\t%dx%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",chname,argv[1],
			gif_dec_obj.dec_info_init.image_width, gif_dec_obj.dec_info_init.image_height,
			gif_dec_obj.dec_param.output_width, gif_dec_obj.dec_param.output_height,gif_dec_obj.dec_param.out_format,
			max_clk,min_clk,n_frames,f_max_clk,f_min_clk,(total_clk*64),s32PeakStack,s32TotalMallocBytes);
 		#endif
 		#endif

        #ifndef MEASURE_STACK_USAGE
        #ifndef MEASURE_HEAP_USAGE
        printf("\n%s\t%s\t%dx%d\t%dx%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",chname,argv[1],
			gif_dec_obj.dec_info_init.image_width, gif_dec_obj.dec_info_init.image_height,
			gif_dec_obj.dec_param.output_width, gif_dec_obj.dec_param.output_height,gif_dec_obj.dec_param.out_format,
			max_clk,min_clk,n_frames,f_max_clk,f_min_clk,(total_clk*64));
 		#endif
 		#endif

#endif

#ifdef __WINCE
	printf("\n lpPerformanceCount in us =%ld\n",TotalDecTime);
#endif


#ifdef __SYMBIAN32__
#ifdef MEASURE_HEAP_USAGE
			printf("\n Stack and Heap are  %d\t%d\t",s32PeakStack,s32TotalMallocBytes);
#endif
#endif

	return 0;
}


/*******************************************************************************
 *
 *   FUNCTION NAME - GIF_get_new_data
 *
 *   ARGUMENTS     - new_buf_ptr : address of new buffer pointer
 *                 - new_buf_len : GIF_UINT32 type pointer to numbytes read.
 *                 - gif_dec_obj : GIF decoder object
 *
 *   RETURN VALUE  - GIFD_RET_TYPE : Error value
 *
 *******************************************************************************
 *
 *   DESCRIPTION
 *   Read bytes from the file of the input buffer size in buffer.
 ******************************************************************************/

GIFD_RET_TYPE GIF_get_new_data (GIF_UINT8 **new_buf_ptr,GIF_UINT32 *new_buf_len,GIF_Decoder_Object *gif_dec_obj)
{
	int bytes_read;

#ifdef TEST_SUSPENSION
	static int counter = 0;
#endif
#ifdef TEST_SUSPENSION
	counter++;

	if (counter == 4)
	{
		return GIFD_SUSPEND;
	}
#endif

	bytes_read = fread(buff,sizeof(char),INPUT_IMAGE_BUFFER_SIZE, fp_input_gif);

    if(bytes_read)
    {
		*new_buf_len = bytes_read;
		*new_buf_ptr = buff;
        return 0;
    }
    else
    {
        return GIF_ERR_EOF;
    }
}


/////////////////////////////   main entry /////////////////////////////
#ifdef __WINCE
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


/**********************End of File*************************/
