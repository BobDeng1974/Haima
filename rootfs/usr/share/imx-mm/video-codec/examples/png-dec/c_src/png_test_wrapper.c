/******************************************************************************
 ***********************************************************************
 *
 *   MOTOROLA GENERAL BUSINESS USE
 *
 *
 *   (C) 2004 MOTOROLA INDIA ELECTRONICS PVT. LTD.
 *
 *   FILENAME        - png_test_wrapper.c
 *   ORIGINAL AUTHOR - Sameer P.Rapate
 *
 *******************************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. 
 ***********************************************************************
 *
 *   CHANGE HISTORY
 *   dd/mm/yy        Code Ver       Author              Description
 *   --------        --------      -----------           ----------
 *   12/10/2004        0.1        Sameer P.Rapate       Initial version
 *
 *   16/11/2006        0.2        Manjunath H S         Modified for
 *                                                      profile on RVDS 
 *                                                      and ELinux:TLSbo83403
 *   05/12/2006        0.3        Durgaprasad S.Bilagi  16-bit support
 *                                                      implementation:tlsbo81719
 *   04/01/2007        0.4        Durgaprasad S.Bilagi  Stack & Heap measurement 
 *                                                      included:tlsbo87093
 *   29/04/2008        0.5        Eagle Zhou: modify ppm file output for auto-test
 *   10/12/2008        0.6        Eagle Zhou: ENGR00102074: add BGR support
 *                                                        add rgb888_ppm output to simplify verification
 *   04/03/2009        0.7        Eagle Zhou: ENGR00108864: add frame decode
 *******************************************************************************
 *
 *   DESCRIPTION
 *   Example file to test png decoding function on host (windows/unix) platform
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "png_dec_interface.h"
#include "log_api.h"
#ifdef TIME_PROFILE
#include <sys/time.h>
#endif

#ifdef MEASURE_STACK_USAGE
#include "mm_codec_common.h"
#endif 

#define USE_DECODE_FRAME

#ifdef MEASURE_HEAP_USAGE
    PNG_UINT32     s32TotalMallocBytes=0;
#endif

FILE *fp_input_image;

#define PRINT_APP_CONTEXT 0 //DSPhl27779
//#define TEST_SUSPENSION


#define OUT_PPM_FILE	//1eagle
#ifdef OUT_PPM_FILE
#define OUTPUT_FILENAME_SIZE 255

char *strlower(char *string)
{
   char *p;

   for (p=string; *p; p++)
      *p = tolower(*p);

   return string;
}

int get_outbit_depth(int input_dep,png_output_format outfmt)
{
	int dep;

	if(input_dep>16)
	{
		printf("unsupported depth: %d \r\n",input_dep);
	}

	if((outfmt==E_PNG_OUTPUTFORMAT_RGB888)||(outfmt==E_PNG_OUTPUTFORMAT_BGR888))	// 24:r/g/b
	{
		dep=24;
	}
	else if ((outfmt==E_PNG_OUTPUTFORMAT_RGB565)||(outfmt==E_PNG_OUTPUTFORMAT_BGR565))  // 16:r/g/b
	{
		dep=16;
	}
	else if ((outfmt==E_PNG_OUTPUTFORMAT_RGB555)||(outfmt==E_PNG_OUTPUTFORMAT_BGR555))  // 16:r/g/b
	{
		dep=16;
	}
	else if ((outfmt==E_PNG_OUTPUTFORMAT_RGB666)||(outfmt==E_PNG_OUTPUTFORMAT_BGR666))  // 24:r/g/b
	{
		dep=24;
	}
	else if ((outfmt==E_PNG_OUTPUTFORMAT_ARGB)||(outfmt==E_PNG_OUTPUTFORMAT_BGRA))  //32: r/g/b/a
	{
		dep=32;
	}	
	else if (outfmt==E_PNG_OUTPUTFORMAT_AG)  //16: gray/a
	{
		dep=16;
	}
	else if (outfmt==E_PNG_OUTPUTFORMAT_G)  //8: gray
	{
		dep=8;
	}
	else
	{
		//error
		printf("unsupported output format \r\n");
	}
	return dep;
}

void convert_rgb888(unsigned char* inbuffer,unsigned char* outbuffer_rgb,unsigned char* outbuffer_a,png_output_format format,unsigned int width, unsigned int height)
{
	// input format : 	BGR or RGB 555/565/666/888; ARGB/BGRA; AG/G
	// output format:	outbuffer_rgb: RGB888 or G
	//				outbuffer_a: A
	// overwrite from tail to end in order use the same buffer when (inbuffer==outbuffer)
	int i,total_pixels;
	unsigned char * src8;
	unsigned short * src16;
	unsigned char * dst;
	unsigned char * dst_a;
	unsigned char temp8;
	unsigned short temp16;

	total_pixels=width*height;
	if(format==E_PNG_OUTPUTFORMAT_RGB888)
	{
		if(inbuffer==outbuffer_rgb)
		{
			//do nothing
		}
		else
		{
			src8=inbuffer+total_pixels*3;
			dst=outbuffer_rgb+total_pixels*3;
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
	else if (format==E_PNG_OUTPUTFORMAT_BGR888)
	{
		src8=inbuffer+total_pixels*3;
		dst=outbuffer_rgb+total_pixels*3;
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
	else if (format==E_PNG_OUTPUTFORMAT_RGB666)
	{
		if(inbuffer==outbuffer_rgb)
		{
			//do nothing
		}
		else
		{
			src8=inbuffer+total_pixels*3;
			dst=outbuffer_rgb+total_pixels*3;
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
	else if (format==E_PNG_OUTPUTFORMAT_BGR666)
	{
		src8=inbuffer+total_pixels*3;
		dst=outbuffer_rgb+total_pixels*3;
		for(i=0;i<total_pixels;i++)
		{
			src8-=3;
			dst-=3;
			temp8=src8[0];	
			dst[0]=src8[2];
			dst[1]=src8[1];
			dst[2]=temp8;	
		}	}
	else if (format==E_PNG_OUTPUTFORMAT_RGB565)
	{
		src16=(unsigned short*)(inbuffer+total_pixels*2);
		dst=outbuffer_rgb+total_pixels*3;
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
	else if (format==E_PNG_OUTPUTFORMAT_BGR565)
	{
		src16=(unsigned short*)(inbuffer+total_pixels*2);
		dst=outbuffer_rgb+total_pixels*3;
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
	else if (format==E_PNG_OUTPUTFORMAT_RGB555)
	{
		src16=(unsigned short*)(inbuffer+total_pixels*2);
		dst=outbuffer_rgb+total_pixels*3;
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
	else if (format==E_PNG_OUTPUTFORMAT_BGR555)
	{
		src16=(unsigned short*)(inbuffer+total_pixels*2);
		dst=outbuffer_rgb+total_pixels*3;
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
	else if (format==E_PNG_OUTPUTFORMAT_ARGB)
	{
		src8=inbuffer;
		dst=outbuffer_rgb;
		dst_a=outbuffer_a;
		for(i=0;i<total_pixels;i++)
		{
			dst_a[0]=src8[0];	//A
			dst[0]=src8[1];	//R
			dst[1]=src8[2];	//G
			dst[2]=src8[3];	//B
			src8+=4;
			dst+=3;
			dst_a+=1;
		}				
	}
	else if (format==E_PNG_OUTPUTFORMAT_BGRA)
	{
		src8=inbuffer;
		dst=outbuffer_rgb;
		dst_a=outbuffer_a;
		for(i=0;i<total_pixels;i++)
		{
			dst_a[0]=src8[3];	//A
			temp8=src8[0];
			dst[0]=src8[2];	//R
			dst[1]=src8[1];	//G
			dst[2]=temp8;	//B
			src8+=4;
			dst+=3;
			dst_a+=1;
		}
	}
	else if (format==E_PNG_OUTPUTFORMAT_AG)
	{
		src8=inbuffer;
		dst=outbuffer_rgb;
		dst_a=outbuffer_a;
		for(i=0;i<total_pixels;i++)
		{
			dst[0]=src8[0];	//G
			dst_a[0]=src8[1];	//A
			src8+=2;
			dst+=1;
			dst_a+=1;
		}
	}	
	else if (format==E_PNG_OUTPUTFORMAT_G)
	{
		if(inbuffer==outbuffer_rgb)
		{
			//do nothing
		}
		else
		{
			src8=inbuffer;
			dst=outbuffer_rgb;
			for(i=0;i<total_pixels;i++)
			{
				dst[0]=src8[0];	//G
				src8+=1;
				dst+=1;
			}
		}
	}
	else
	{
		printf("unknown output format ! \r\n");
	}
	
	return;
}

int write_ppm_alpha_file(char* stream_name,char* out_data, int width, int height,int dep,png_output_format outfmt,int wrapper_format)
{

	FILE* fp_ppm;
	char filename_ppm[OUTPUT_FILENAME_SIZE];
	char *filename_temp;
	char filename_bak[OUTPUT_FILENAME_SIZE];
	char filename_delim[]=".png";
	//char filename_delim[]=".";
	FILE* fp_alpha;
	char filename_alpha[OUTPUT_FILENAME_SIZE];

	int i,j;
	int ppm_size,alpha_size;
	
	char * ppmbuf;
	char*  alphabuf;
	char * dat;
	char * dst_p;
	char * dst_a;


	if((outfmt==E_PNG_OUTPUTFORMAT_RGB888)||(outfmt==E_PNG_OUTPUTFORMAT_BGR888))	// 24:r/g/b
	{
		ppm_size=width*height*3; // rgb
		alpha_size=0;
	}
	else if ((outfmt==E_PNG_OUTPUTFORMAT_RGB565)||(outfmt==E_PNG_OUTPUTFORMAT_BGR565))  // 16:r/g/b
	{
		ppm_size=width*height*2; // rgb
		alpha_size=0;
	}
	else if ((outfmt==E_PNG_OUTPUTFORMAT_RGB555)||(outfmt==E_PNG_OUTPUTFORMAT_BGR555))  // 16:r/g/b
	{
		ppm_size=width*height*2; // rgb
		alpha_size=0;
	}
	else if ((outfmt==E_PNG_OUTPUTFORMAT_RGB666)||(outfmt==E_PNG_OUTPUTFORMAT_BGR666))  // 24:r/g/b
	{
		ppm_size=width*height*3; // rgb
		alpha_size=0;
	}
	else if ((outfmt==E_PNG_OUTPUTFORMAT_ARGB)||(outfmt==E_PNG_OUTPUTFORMAT_BGRA))  //32: r/g/b/a
	{
		ppm_size=width*height*3; // rgb
		alpha_size=width*height;   // alpha
	}	
	else if (outfmt==E_PNG_OUTPUTFORMAT_AG)  //16: gray/a
	{
		ppm_size=width*height; // gray
		alpha_size=width*height;   // alpha
	}
	else if (outfmt==E_PNG_OUTPUTFORMAT_G)  //8: gray
	{
		ppm_size=width*height; // gray
		alpha_size=0;
	}
	else
	{
		//error
		printf("unsupported output format \r\n");
		return 0;
	}

	// malloc data buffer
	if(alpha_size>0)
	{
		alphabuf=malloc(alpha_size);
		if(!alphabuf)
		{
			printf("alloc alpha data fail \r\n");
			return 0;
		}
	
	}
	ppmbuf=malloc(ppm_size);
	if(!ppmbuf)
	{
		printf("alloc ppmbuf data fail,size: %d \r\n",ppm_size);
		return 0;
	}		

	// create filename
	strcpy(filename_bak,stream_name);
	
	//filename_temp=strtok(filename_bak,filename_delim);
	strlower(filename_bak); // => slower case
	filename_temp=(char*)strstr(filename_bak,filename_delim);
	filename_temp[0]='\0';
	filename_temp=filename_bak;
	//fprintf(fp_log,"filename_temp: %s \r\n",filename_temp);
	sprintf(filename_ppm,"%s_%d.ppm",filename_temp,dep);

	// open files
	if(alpha_size>0)
	{
		sprintf(filename_alpha,"%s_%d_alpha.ppm",filename_temp,dep);
		fp_alpha=fopen(filename_alpha,"wb");
		if(!fp_alpha)
		{
			printf("open file fail: %s \r\n",filename_alpha);
			return 0;
		}
	}
	
	fp_ppm=fopen(filename_ppm,"wb");
	if(!fp_ppm)
	{
		printf("open file fail: %s \r\n",filename_ppm);
		return 0;
	}

	if(wrapper_format==0)
	{
		// write .ppm/.alpha file header
		if((dep==8)||(outfmt==E_PNG_OUTPUTFORMAT_AG))
		{
			fprintf(fp_ppm, "%s\n", "P5");
		}
		else
		{
			fprintf(fp_ppm, "%s\n", "P6");
		}
		fprintf(fp_ppm, "%d %d\n", width,height);
		fprintf(fp_ppm, "%d\n", 255);
		fflush(fp_ppm);

		if(alpha_size>0)
		{
			fprintf(fp_alpha, "%s\n", "P5");
			fprintf(fp_alpha, "%d %d\n", width,height);
			fprintf(fp_alpha, "%d\n", 255);
			fflush(fp_alpha);			
		}

	}

	// copy data
	dat=out_data;
	dst_p=ppmbuf;
	dst_a=alphabuf;
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			// first alpha when ARGB
			if((outfmt!=E_PNG_OUTPUTFORMAT_AG))
			{
				if(alpha_size>0)
				{
					*dst_a++=*dat++; //alpha
				}
			}

			// then r/g/b
			if((dep==8)||(outfmt==E_PNG_OUTPUTFORMAT_AG))
			{
				dst_p[0]=dat[0]; // gray
				dst_p+=1;
				dat+=1;
			}
			else if (dep==16)
			{
				dst_p[0]=dat[0]; // 
				dst_p[1]=dat[1]; // 
				dst_p+=2;
				dat+=2;
			}
			else if (dep==24)
			{
				dst_p[0]=dat[0]; // r
				dst_p[1]=dat[1]; // g
				dst_p[2]=dat[2]; // b
				dst_p+=3;
				dat+=3;
			}
			else
			{
				dst_p[0]=dat[0]; // r
				dst_p[1]=dat[1]; // g
				dst_p[2]=dat[2]; // b
				dst_p+=3;
				dat+=3;
			}

			// then alpha when AG
			if((outfmt==E_PNG_OUTPUTFORMAT_AG))
			{
				if(alpha_size>0)
				{
					*dst_a++=*dat++; //alpha
				}
			}
		}
	}
	
	// write data into files
	//printf("begin write data \r\n");
	fwrite(ppmbuf, sizeof(char), ppm_size, fp_ppm);
	if(alpha_size>0)
	{
		fwrite(alphabuf, sizeof(char), alpha_size, fp_alpha);
		fclose(fp_alpha);
		free(alphabuf);
	}
								
	fclose(fp_ppm);		
	free(ppmbuf);
	return 1;
}

int write_ppm_alpha_rgb888_file(char* stream_name,char* out_data, int width, int height,int dep,png_output_format outfmt,int wrapper_format,int scale_factor)
{

	FILE* fp_ppm;
	char filename_ppm[OUTPUT_FILENAME_SIZE];
	char *filename_temp;
	char filename_bak[OUTPUT_FILENAME_SIZE];
	char filename_delim[]=".png";
	//char filename_delim[]=".";
	FILE* fp_alpha;
	char filename_alpha[OUTPUT_FILENAME_SIZE];
	char outformat[OUTPUT_FILENAME_SIZE];

	int i,j;
	int ppm_size,alpha_size;
	
	char * ppmbuf;
	char*  alphabuf;
	char * dat;
	char * dst_p;
	char * dst_a;


	if((outfmt==E_PNG_OUTPUTFORMAT_RGB888))	// 24:r/g/b
	{
		ppm_size=width*height*3; // rgb
		alpha_size=0;
		sprintf(outformat,"rgb888");
	}
	else if((outfmt==E_PNG_OUTPUTFORMAT_BGR888))// 24:b/g/r
	{
		ppm_size=width*height*3; // rgb
		alpha_size=0;
		sprintf(outformat,"bgr888");
	}
	else if ((outfmt==E_PNG_OUTPUTFORMAT_RGB565))  // 16:r/g/b
	{
		ppm_size=width*height*3; // rgb
		alpha_size=0;
		sprintf(outformat,"rgb565");
	}
	else if ((outfmt==E_PNG_OUTPUTFORMAT_BGR565))  // 16:b/g/r
	{
		ppm_size=width*height*3; // rgb
		alpha_size=0;
		sprintf(outformat,"bgr565");
	}
	else if ((outfmt==E_PNG_OUTPUTFORMAT_RGB555))  // 16:r/g/b
	{
		ppm_size=width*height*3; // rgb
		alpha_size=0;
		sprintf(outformat,"rgb555");
	}
	else if ((outfmt==E_PNG_OUTPUTFORMAT_BGR555))  // 16:b/g/r
	{
		ppm_size=width*height*3; // rgb
		alpha_size=0;
		sprintf(outformat,"bgr555");
	}	
	else if ((outfmt==E_PNG_OUTPUTFORMAT_RGB666))  // 24:r/g/b
	{
		ppm_size=width*height*3; // rgb
		alpha_size=0;
		sprintf(outformat,"rgb666");
	}
	else if ((outfmt==E_PNG_OUTPUTFORMAT_BGR666))  // 24:b/g/r
	{
		ppm_size=width*height*3; // rgb
		alpha_size=0;
		sprintf(outformat,"bgr666");
	}
	else if ((outfmt==E_PNG_OUTPUTFORMAT_ARGB))  //32: a/r/g/b
	{
		ppm_size=width*height*3; // rgb
		alpha_size=width*height;   // alpha
		sprintf(outformat,"argb");
	}	
	else if ((outfmt==E_PNG_OUTPUTFORMAT_BGRA))  //32: a/b/g/r
	{
		ppm_size=width*height*3; // rgb
		alpha_size=width*height;   // alpha
		sprintf(outformat,"bgra");
	}		
	else if (outfmt==E_PNG_OUTPUTFORMAT_AG)  //16: gray/a
	{
		ppm_size=width*height; // gray
		alpha_size=width*height;   // alpha
		sprintf(outformat,"ag");
	}
	else if (outfmt==E_PNG_OUTPUTFORMAT_G)  //8: gray
	{
		ppm_size=width*height; // gray
		alpha_size=0;
		sprintf(outformat,"g");
	}
	else
	{
		//error
		printf("unsupported output format \r\n");
		return 0;
	}

	// malloc data buffer
	if(alpha_size>0)
	{
		alphabuf=malloc(alpha_size);
		if(!alphabuf)
		{
			printf("alloc alpha data fail \r\n");
			return 0;
		}
	
	}
	ppmbuf=malloc(ppm_size);
	if(!ppmbuf)
	{
		printf("alloc ppmbuf data fail,size: %d \r\n",ppm_size);
		return 0;
	}		

	// create filename
	strcpy(filename_bak,stream_name);
	
	//filename_temp=strtok(filename_bak,filename_delim);
	strlower(filename_bak); // => slower case
	filename_temp=(char*)strstr(filename_bak,filename_delim);
	filename_temp[0]='\0';
	filename_temp=filename_bak;
	//fprintf(fp_log,"filename_temp: %s \r\n",filename_temp);
	if (scale_factor>1)
	{
		sprintf(filename_ppm,"%s_scale(%d)_%d_%s.ppm",filename_temp,scale_factor,dep,outformat);
	}
	else
	{
		sprintf(filename_ppm,"%s_%d_%s.ppm",filename_temp,dep,outformat);
	}

	// open files
	if(alpha_size>0)
	{
		if (scale_factor>1)
		{
			sprintf(filename_alpha,"%s_scale(%d)_%d_%s_alpha.ppm",filename_temp,scale_factor,dep,outformat);
		}
		else
		{
			sprintf(filename_alpha,"%s_%d_%s_alpha.ppm",filename_temp,dep,outformat);
		}
		fp_alpha=fopen(filename_alpha,"wb");
		if(!fp_alpha)
		{
			printf("open file fail: %s \r\n",filename_alpha);
			return 0;
		}
	}
	
	fp_ppm=fopen(filename_ppm,"wb");
	if(!fp_ppm)
	{
		printf("open file fail: %s \r\n",filename_ppm);
		return 0;
	}

	if(wrapper_format==0)
	{
		// write .ppm/.alpha file header
		if((dep==8)||(outfmt==E_PNG_OUTPUTFORMAT_AG))
		{
			fprintf(fp_ppm, "%s\n", "P5");
		}
		else
		{
			fprintf(fp_ppm, "%s\n", "P6");
		}
		fprintf(fp_ppm, "%d %d\n", width,height);
		fprintf(fp_ppm, "%d\n", 255);
		fflush(fp_ppm);

		if(alpha_size>0)
		{
			fprintf(fp_alpha, "%s\n", "P5");
			fprintf(fp_alpha, "%d %d\n", width,height);
			fprintf(fp_alpha, "%d\n", 255);
			fflush(fp_alpha);			
		}

	}

	convert_rgb888(out_data,ppmbuf,alphabuf,outfmt,width,height);
	
	// write data into files
	//printf("begin write data \r\n");
	fwrite(ppmbuf, sizeof(char), ppm_size, fp_ppm);
	if(alpha_size>0)
	{
		fwrite(alphabuf, sizeof(char), alpha_size, fp_alpha);
		fclose(fp_alpha);
		free(alphabuf);
	}
								
	fclose(fp_ppm);		
	free(ppmbuf);
	return 1;
}

#endif


/*******************************************************************************
 *
 *   FUNCTION NAME - PNG_app_read_data
 *
 *   ARGUMENTS
 *
 *
 *   RETURN VALUE  -  void
 *
 *******************************************************************************
 *
 *   DESCRIPTION
 *
 ******************************************************************************/
PNGD_RET_TYPE PNG_app_read_data(void *input_ptr, PNG_UINT8 *input_data,
					   PNG_UINT32 length_requested, PNG_UINT32 length_returned, void *pAppContext) //DSPhl27779
{

#ifdef TEST_SUSPENSION
	static int counter=0;
#endif

#if PRINT_APP_CONTEXT
	{
		FILE *fp = fopen("AppContextLog.txt","a");
		if(fp)
		{
			fprintf(fp, "\nApplication Context in PNG_app_read_data is %x", pAppContext);
			fclose(fp);
		}
	}	
#endif

	if(input_ptr==NULL)
	{
		input_ptr = fp_input_image;
	}
	length_returned = fread((void*)input_data, sizeof(PNG_UINT8), length_requested,
							(FILE *)input_ptr);

#ifdef TEST_SUSPENSION
	counter++;

	if (counter == 15)
	{
	    printf("suspended \n");
	    return PNGD_ERR_SUSPEND;
	}
#endif

	if(length_returned!=length_requested)
	{
		printf("\nReturned length is not equal to requested length\n");
	}
	return PNGD_OK;
}
/*******************************************************************************
 *
 *   FUNCTION NAME - PNG_app_free
 *
 *   ARGUMENTS
 *
 *   RETURN VALUE  - int : Error value
 *
 *******************************************************************************
 *
 *   DESCRIPTION
 *
 ******************************************************************************/
void PNG_app_free(void *ptr, void *pAppContext)
{
#if PRINT_APP_CONTEXT
	{
		FILE *fp = fopen("AppContextLog.txt","a");
		if(fp)
		{
			fprintf(fp, "\nApplication Context in PNG_app_free is %x", pAppContext);
			fclose(fp);
		}
	}	
#endif

	free(ptr);
}

/*******************************************************************************
 *
 *   FUNCTION NAME - PNG_app_free
 *
 *   ARGUMENTS
 *
 *   RETURN VALUE  - int : Error value
 *
 *******************************************************************************
 *
 *   DESCRIPTION
 *
 ******************************************************************************/
void * PNG_app_malloc(void *ptr, PNG_UINT32 size, void *pAppContext)
{

	
#if PRINT_APP_CONTEXT
	{
		FILE *fp = fopen("AppContextLog.txt","a");
		if(fp)
		{
			fprintf(fp, "\nApplication Context in PNG_app_free is %x", pAppContext);
			fclose(fp);
		}
	}	
#endif


	ptr= (PNG_UINT8 *)calloc(size,sizeof(PNG_UINT8));


    if(ptr==NULL)
	{
		printf("Memory allocation failed");
	}
    else
    {
        #ifdef MEASURE_HEAP_USAGE
            s32TotalMallocBytes+=size;
        #endif
    }


	return ptr;
}

/*******************************************************************************
 *
 *   FUNCTION NAME - main
 *
 *   ARGUMENTS     - None
 *
 *   RETURN VALUE  - int : Error value
 *
 *******************************************************************************
 *
 *   DESCRIPTION
 *   Sample decoding function to test png decoder on unix/windows
 ******************************************************************************/
int main(int argc, char *argv[])
{
    
    PNG_Decoder_Object png_dec_object;
    PNGD_RET_TYPE png_err;
    PNG_UINT32  row_index;
    PNG_UINT8 *out_buf, *out_image;
	int pass=0,mult_factor=3,mem_out_pix=0;

#ifdef SAVE_OUTPUT   
    FILE *fp_write;
    char *unwrapped_ptr;
    int unwrapped_format;
#endif
    
    #ifdef MEASURE_STACK_USAGE
       PNG_UINT32           *ps32SP, *ps32BaseSP;
       PNG_UINT32           s32StackCount, s32StackValue;
       PNG_UINT32           s32PeakStack = 0;
    #endif

    #ifdef TIME_PROFILE
		struct timeval StartTime, EndTime;
		unsigned long TotalDecTimeUs = 0;
		unsigned int n_frames = 1,Max_time = 0,Min_time = 0,f_Max_time = 0,f_Min_time = 0;
		unsigned char chname[] = "[PROFILE-INFO]";
		FILE *fp_decode_time;
#ifndef __SYMBIAN32__
		fp_decode_time=fopen("decode_time","a");
#else
		fp_decode_time=fopen("D:\\decode_time","a");
#endif
	#endif
	
	#ifdef TIME_PROFILE_RVDS
		int prev_clk, curr_clk,clk,n_frames=1;
		int total_clk =0,max_clk=0,min_clk=0,f_max_clk=0,f_min_clk=0;
		extern int prev_cycles(void);
		extern int curr_cycles(void);
		unsigned char chname[] = "[PROFILE-INFO]";
	#endif
	
#ifdef TEST_SUSPENSION
    int suspend_flag;
#endif

     #ifdef MEASURE_HEAP_USAGE
        s32TotalMallocBytes = 0;
     #endif

    //application data
	png_dec_object.pAppContext = (void*) 0x123456;

	png_dec_object.PNG_app_malloc = PNG_app_malloc;
	png_dec_object.PNG_app_free = PNG_app_free;
	png_dec_object.PNG_app_read_data=PNG_app_read_data;

	/*Check for correct count of program arguments*/
	if(argc!=8)
	{
		printf("Number of arguments should be equal to 7 \n");
		printf("Usage :<input-file> <output-file> <output-format> \
        <output-width> <output-height> <scaling-mode> <raw-format>");

		return -1;
	}

    fp_input_image = fopen(argv[1], "rb");
    if (fp_input_image == NULL)
    {
        printf("Error opening input image file \n");
        return 2;
    }


	/*Set the configurable paramteres*/

    png_dec_object.dec_param.outformat     = atoi(argv[3]);
	png_dec_object.dec_param.output_width  = atoi(argv[4]);
    png_dec_object.dec_param.output_height = atoi(argv[5]);
    png_dec_object.dec_param.scale_mode    = atoi(argv[6]);
    
#ifdef SAVE_OUTPUT   

    unwrapped_format=atoi(argv[7]);

   if(unwrapped_format==0)
   { 
	fp_write = fopen(argv[2], "wb+");
   }
   else
   {
        unwrapped_ptr=argv[1];
        strcat(unwrapped_ptr,".noheader");
	    fp_write = fopen(unwrapped_ptr, "wb+");
   }

	if (fp_write == NULL)
	{
		printf("Error opening output image file \n");
		return -1;
	}
#endif

	/*Validation of configured parameters*/

	if(png_dec_object.dec_param.outformat<0 || png_dec_object.dec_param.outformat>=E_PNG_LAST_OUTPUT_FORMAT)
	{
		printf("Configured output Format is invalid");
		return -1;
	}
	if(png_dec_object.dec_param.scale_mode<0 || png_dec_object.dec_param.scale_mode>1)
	{
		printf("Configured scaling mode is invalid");
		return -1;
	}
	if(png_dec_object.dec_param.output_width<=0)
	{
		printf("Configured width is invalid ");
		return -1;
	}

	if(png_dec_object.dec_param.output_height<=0)
	{
		printf("Configured height is invalid ");
		return -1;
	}

	// check api version 
	printf("version: %s\n", PNGD_CodecVersionInfo());


#ifdef TEST_SUSPENSION
    suspend_flag = 1;
#endif

	/*Initialization Routine*/
#ifdef TEST_SUSPENSION
    while(suspend_flag)
    {

	suspend_flag = 0;
#endif

#ifdef MEASURE_STACK_USAGE
    PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
#endif

PNG_INIT:	
    png_err=PNG_dec_init(&png_dec_object);

#ifdef MEASURE_STACK_USAGE    
    GET_STACK_USAGE (ps32BaseSP, ps32SP, s32StackCount, s32PeakStack);
#endif


#ifdef TEST_SUSPENSION
	if (png_err == PNGD_ERR_SUSPEND)
	{
	    printf("suspended from init\n");
	    suspend_flag = 1;
	    if (fseek(fp_input_image, 0, SEEK_SET) != 0)
	    {
		printf("problem in seeking the file to the beginning\n");
		return -1;
	    }
	    continue;
	}
#endif

	if(png_err != PNGD_OK)
	{
		if(png_err==PNG_DEC_INVALID_OUTFORMAT)
		{
#if 1
			//use recommended output format by decoder, and re-init decoder
			printf("output format is invalid, and use new output format %d \r\n",png_dec_object.dec_param.outformat);
			//png_dec_object.dec_param.outformat has already modified by decoder
			if (fseek(fp_input_image, 0, SEEK_SET) == 0)
			{
				goto PNG_INIT;
			}
			printf("Configured output Format is invalid\n");
			return -1;
#else
			// eagle : for auto test
			static png_output_format outfmt=E_PNG_OUTPUTFORMAT_ARGB;		
			if(outfmt<E_PNG_LAST_OUTPUT_FORMAT)
			{
				printf("Configured output Format is invalid: %d \n",png_dec_object.dec_param.outformat);
				png_dec_object.dec_param.outformat=outfmt++;
				if (fseek(fp_input_image, 0, SEEK_SET) == 0)
				{
					goto PNG_INIT;
				}
			}		
			printf("Configured output Format is invalid\n");
			return -1;
#endif			
		}
		else
		{
			printf("Error in PNG_decoder_init \n");
			return -1;
		}
	}

	/*Set the multiplication factor depending upon the
	  output format specified.This mutiplication factor
	  is useful for allocation and modification of output buffer
	*/



//dsphl28117
	if ((png_dec_object.dec_param.outformat == E_PNG_OUTPUTFORMAT_RGB555)|| (png_dec_object.dec_param.outformat == E_PNG_OUTPUTFORMAT_RGB565)||
	    (png_dec_object.dec_param.outformat == E_PNG_OUTPUTFORMAT_BGR555)|| (png_dec_object.dec_param.outformat == E_PNG_OUTPUTFORMAT_BGR565))
	{
		mult_factor=2;
	}
//dsphl28117
	else if ((png_dec_object.dec_param.outformat==E_PNG_OUTPUTFORMAT_ARGB)||(png_dec_object.dec_param.outformat==E_PNG_OUTPUTFORMAT_BGRA))
	{
		mult_factor=4;
	}
	else if(png_dec_object.dec_param.outformat==E_PNG_OUTPUTFORMAT_G)
	{
		mult_factor=1;
	}
	else if(png_dec_object.dec_param.outformat==E_PNG_OUTPUTFORMAT_AG)
	{
		mult_factor=2;
	}
	/*For RGB888 and RGB666*/
	else
	{
		mult_factor=3;
	}

    if(png_dec_object.dec_info_init.bit_depth==16)
    {       mem_out_pix=png_dec_object.dec_info_init.output_width*
            png_dec_object.dec_info_init.output_height*mult_factor*2;
    }
    else
    {
	mem_out_pix=png_dec_object.dec_info_init.output_width*
		png_dec_object.dec_info_init.output_height*mult_factor;
    }

	/*Allocate memory for output image*/
	out_image = (PNG_UINT8 *)calloc(mem_out_pix,sizeof(PNG_UINT8));
	out_buf=out_image;

	/*Allocate memory for row pixels*/
//dsphl28117
	if((png_dec_object.dec_param.outformat== E_PNG_OUTPUTFORMAT_ARGB)||(png_dec_object.dec_param.outformat== E_PNG_OUTPUTFORMAT_BGRA))
	{
        if(png_dec_object.dec_info_init.bit_depth==16)
        {   png_dec_object.pixels = (PNG_INT32*)calloc(8 *
			png_dec_object.dec_info_init.output_width,sizeof(int));
	}
	else
        {   png_dec_object.pixels = (PNG_INT32*)calloc(4 *
            png_dec_object.dec_info_init.output_width,sizeof(int));
        }
    }
    else
    {
        if(png_dec_object.dec_info_init.bit_depth==16)
        {   png_dec_object.pixels = (PNG_INT32*)calloc(6 *
            png_dec_object.dec_info_init.output_width,sizeof(int));
        }
        else
        {   png_dec_object.pixels = (PNG_INT32*)calloc(3 *
            png_dec_object.dec_info_init.output_width,sizeof(int));
        }
	}
    if(out_image == NULL ||png_dec_object.pixels==NULL)
    {
        printf(" Memory Allocation for output buffer failed\n");
		return -1;
    }

#ifndef USE_DECODE_FRAME
	/*Decode row by row*/

	/*For interlaced images passes=7, else number of passes=1*/

	for (pass = 0; pass < png_dec_object.dec_info_init.number_passes; pass++)
	{
		for(row_index = 0; row_index < png_dec_object.dec_info_init.output_height; row_index++)
        {

			out_buf = out_image +(row_index*png_dec_object.dec_info_init.output_width*mult_factor);
#endif
            #ifdef MEASURE_STACK_USAGE
                PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
            #endif
		
        	#ifdef TIME_PROFILE
				gettimeofday(&StartTime, NULL);
			#endif

			#ifdef TIME_PROFILE_RVDS
				prev_clk = prev_cycles();
			#endif    

#ifndef USE_DECODE_FRAME
			png_err = PNG_decode_row(&png_dec_object,out_buf);
#else
			//printf("call decode frame \r\n");
			png_err = PNG_decode_frame(&png_dec_object,out_image);
#endif

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
            #endif /* MEASURE_STACK_USAGE */

#ifdef TEST_SUSPENSION
			if (png_err == PNGD_ERR_SUSPEND)
			{
			    printf("suspended from decode row in row no %d\n", row_index);
			    suspend_flag = 1;

		            if (fseek(fp_input_image, 0, SEEK_SET) != 0)
			    {
				printf("problem in seeking the file\n");
				return -1;
			    }
			    break;
			}
#endif

			if(png_err != PNGD_OK)
			{
				printf("PNG decode error \n");
				return -1;
			}
		
          #ifdef MEASURE_STACK_USAGE
             if (s32PeakStack < s32StackValue)
                 s32PeakStack = s32StackValue;
          #endif     
        
#ifndef USE_DECODE_FRAME        
        }

#ifdef TEST_SUSPENSION
		if (png_err == PNGD_ERR_SUSPEND)
		{
			free(png_dec_object.pixels);
			free(out_image);
		    break;
		}
#endif
		png_dec_object.rows_decoded=0;
		png_dec_object.dec_info_init.pass++;
	}
#endif
#ifdef TEST_SUSPENSION
    }
#endif

	/*Cleanup Routine*/

    png_err=PNG_cleanup(&png_dec_object);
#ifdef TEST_SUSPENSION
     if(png_err==PNGD_RD_END_ERROR)
     {
        printf("Suspended from read end.Ignoring\n");
     }

#endif

         if(png_err!=PNGD_OK && png_err!=PNGD_RD_END_ERROR)
         {
           printf("Error in cleanup");
           return -1;

         }
	free(png_dec_object.pixels);

	printf("decoded successfully \n");

	/*Write decoded data to the PPM file*/
#ifdef SAVE_OUTPUT   
	//printf("mem_out_pix: %d ,bit_dep: %d \r\n",mem_out_pix,png_dec_object.dec_info_init.bit_depth);
	//printf("width: %d, height: %d \r\n",png_dec_object.dec_info_init.output_width,png_dec_object.dec_info_init.output_height);
#ifdef OUT_PPM_FILE
	{
		int dep;			
		dep=get_outbit_depth(png_dec_object.dec_info_init.bit_depth,png_dec_object.dec_param.outformat);
		//write_ppm_alpha_file(argv[1],out_image,png_dec_object.dec_info_init.output_width,png_dec_object.dec_info_init.output_height,dep,png_dec_object.dec_param.outformat,unwrapped_format);
		write_ppm_alpha_rgb888_file(argv[1],out_image,png_dec_object.dec_info_init.output_width,png_dec_object.dec_info_init.output_height,dep,png_dec_object.dec_param.outformat,unwrapped_format,png_dec_object.dec_info_init.scaling_factor);
	}	

#else
    if(unwrapped_format==0)
    {
     if((png_dec_object.dec_param.outformat==E_PNG_OUTPUTFORMAT_G) || (png_dec_object.dec_param.outformat==E_PNG_OUTPUTFORMAT_AG))
      fprintf(fp_write, "%s\n", "P5");
     else
	fprintf(fp_write, "%s\n", "P6");
      
	fprintf(fp_write, "%d %d\n", png_dec_object.dec_info_init.output_width,
		png_dec_object.dec_info_init.output_height);
	fprintf(fp_write, "%d\n", 255);
    fflush(fp_write);
    }


//printf("png_dec_object.dec_info_init.bit_depth: %d \r\n",png_dec_object.dec_info_init.bit_depth);
    if(png_dec_object.dec_info_init.bit_depth<=8)
	{
#if 1 //eagle
		if(png_dec_object.dec_param.outformat==E_PNG_OUTPUTFORMAT_ARGB)
		{		
			int i;
			unsigned char * src;
			unsigned char * dst;
			src=out_image;
			dst=out_image;
			printf("skip alpha channel for argb format: 8 bit depth \r\n");
			for (i=0;i<mem_out_pix/4;i++)
			{
				dst[0]=src[1];	//R
				dst[1]=src[2];	//G
				dst[2]=src[3];	//B
				dst+=3;
				src+=4;
			}
			fwrite(out_image, sizeof(char),mem_out_pix*3/4, fp_write);
		}
		else if(png_dec_object.dec_param.outformat==E_PNG_OUTPUTFORMAT_BGRA)
		{		
			int i;
			unsigned char * src;
			unsigned char * dst;
			src=out_image;
			dst=out_image;
			printf("skip alpha channel for bgra format: 8 bit depth \r\n");
			for (i=0;i<mem_out_pix/4;i++)
			{
				dst[0]=src[2];	//R
				dst[1]=src[1];	//G
				dst[2]=src[0];	//B
				dst+=3;
				src+=4;
			}
			fwrite(out_image, sizeof(char),mem_out_pix*3/4, fp_write);
		}
		else
		{
			fwrite(out_image, sizeof(char),mem_out_pix, fp_write);
		}
#else
		fwrite(out_image, sizeof(char),mem_out_pix, fp_write);
#endif
	
	
    }
    else
    	{
#if 1 //eagle
		if(png_dec_object.dec_param.outformat==E_PNG_OUTPUTFORMAT_ARGB)
		{		
			int i;
			unsigned char * src;
			unsigned char * dst;
			src=out_image;
			dst=out_image;
			printf("skip alpha channel for argb format: 16 bit depth \r\n");
			for (i=0;i<mem_out_pix/2/4;i++)
			{
				dst[0]=src[1];
				dst[1]=src[2];
				dst[2]=src[3];
				dst+=3;
				src+=4;
			}
			fwrite(out_image, sizeof(char),mem_out_pix/2*3/4, fp_write);
		}
		else if(png_dec_object.dec_param.outformat==E_PNG_OUTPUTFORMAT_BGRA)
		{		
			int i;
			unsigned char * src;
			unsigned char * dst;
			src=out_image;
			dst=out_image;
			printf("skip alpha channel for argb format: 16 bit depth \r\n");
			for (i=0;i<mem_out_pix/2/4;i++)
			{
				dst[0]=src[2];
				dst[1]=src[1];
				dst[2]=src[0];
				dst+=3;
				src+=4;
			}
			fwrite(out_image, sizeof(char),mem_out_pix/2*3/4, fp_write);
		}		
		else
		{
			fwrite(out_image, sizeof(char),mem_out_pix/2, fp_write);
		}
#else
    	fwrite(out_image, sizeof(char),mem_out_pix/2, fp_write);
#endif
    	}

	fclose(fp_write);
#endif	
#endif  

    fclose(fp_input_image);
    free(out_image);


    #ifdef TIME_PROFILE
			//printf("Total Decode Time [microseconds] = %ld\n\n", TotalDecTimeUs);
            printf("\n%s\t%s\t%dx%d\t%dx%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",chname,argv[1], \
            png_dec_object.dec_info_init.width, png_dec_object.dec_info_init.height, \
            png_dec_object.dec_param.output_width,png_dec_object.dec_param.output_height, png_dec_object.dec_param.outformat, \
            Max_time,Min_time,n_frames,f_Max_time,f_Min_time,TotalDecTimeUs);

			fprintf(fp_decode_time,"%ld\n",TotalDecTimeUs);

			fclose(fp_decode_time);
    #endif
    #ifdef TIME_PROFILE_RVDS
        #ifdef MEASURE_STACK_USAGE  
        #ifdef MEASURE_HEAP_USAGE
            //printf("Total Decode Time [microseconds] = %ld\n\n", TotalDecTimeUs);
            printf("\n%s\t%s\t%dx%d\t%dx%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",chname,argv[1], \
            png_dec_object.dec_info_init.width, png_dec_object.dec_info_init.height, \
            png_dec_object.dec_param.output_width,png_dec_object.dec_param.output_height, png_dec_object.dec_param.outformat, \
            max_clk,min_clk,n_frames,f_max_clk,f_min_clk,(total_clk*64) ,s32PeakStack,s32TotalMallocBytes);
 		#endif
 		#endif

        #ifndef MEASURE_STACK_USAGE  
        #ifndef MEASURE_HEAP_USAGE
			//printf("Total Decode Time [microseconds] = %ld\n\n", TotalDecTimeUs);
            printf("\n%s\t%s\t%dx%d\t%dx%d\t%d\t%d\t%d\t%d\t%d\n",chname,argv[1], \
            png_dec_object.dec_info_init.width, png_dec_object.dec_info_init.height, \
            png_dec_object.dec_param.output_width,png_dec_object.dec_param.output_height, png_dec_object.dec_param.outformat, \
            max_clk,min_clk,n_frames,f_max_clk,f_min_clk,(total_clk*64));
 		#endif
 		#endif
    #endif
#ifdef __SYMBIAN32__
#ifdef MEASURE_HEAP_USAGE
			printf("\n Stack and Heap are  %d\t%d\t",s32PeakStack,s32TotalMallocBytes);
#endif
#endif

	return 0;
}

/////////////////////////////   main entry /////////////////////////////
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
/******************** End of file ****************************/

