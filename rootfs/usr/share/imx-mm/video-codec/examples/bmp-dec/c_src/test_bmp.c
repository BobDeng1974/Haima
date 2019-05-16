
/******************************************************************************
 *
 *   MOTOROLA GENERAL BUSINESS USE
 *
 *
 *   (C) 2004 MOTOROLA INDIA ELECTRONICS PVT. LTD.
 *
 *   FILENAME        - test_bmp.c
 *   ORIGINAL AUTHOR - Rajesh Gupta
 *
 
 /*
  ***********************************************************************
  * Copyright 2005-2010 by Freescale Semiconductor, Inc.
  * All modifications are confidential and proprietary information
  * of Freescale Semiconductor, Inc. 
  ***********************************************************************
  */
/*******************************************************************************
 *
 *   CHANGE HISTORY
 *   dd/mm/yy        Code Ver      Description                 Author
 *   --------        --------      -----------                 ------
 *   10/03/2004      0.1            Initial version          Rajesh Gupta
 *
 *   04/01/2007      0.2           Durgaprasad S.Bilagi   Stack & Heap measurement 
 *                                                          included:tlsbo87093
 *   08/10/2007      1.0            Ported on VRTX          Satish K Singh
 *   17/04/2008      0.2            add 32 bit color          Eagle Zhou : related CRs:ENGR00073179
 *******************************************************************************
 *
 *   DESCRIPTION
 *   Example file to test bmp decoding function on host (windows/unix) platform
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "bmp_interface.h"

#define INPUT_IMAGE_BUFFER_SIZE 8192



#ifdef TIME_PROFILE
#include <sys/time.h>
#endif

#ifdef MEASURE_STACK_USAGE
#include "mm_codec_common.h"
#endif 

#ifdef MEASURE_HEAP_USAGE
    unsigned int     s32TotalMallocBytes = 0;
#endif


//#define TEST_SUSPENSION 1

char buff[INPUT_IMAGE_BUFFER_SIZE];

FILE *fp_input_image;

FILE *pftest;


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

int get_outbit_depth(int input_dep,bmp_output_format outfmt)
{
	int dep;
	if((input_dep==32)&&(outfmt==E_BMP_OUTPUTFORMAT_RGB888))	// 32:r/g/b/a
	{
		dep=32;
	}
	else if (outfmt==E_BMP_OUTPUTFORMAT_RGB888)  // 24:r/g/b
	{
		dep=24;
	}
	else if (outfmt==E_BMP_OUTPUTFORMAT_RGB565)  // 16:r/g/b
	{
		dep=16;
	}
	else if (outfmt==E_BMP_OUTPUTFORMAT_RGB555)  // 16:r/g/b
	{
		dep=16;
	}
	else if (outfmt==E_BMP_OUTPUTFORMAT_RGB666)  //24: r/g/b
	{
		dep=24;
	}	
	return dep;
}

int write_ppm_alpha_file(char* stream_name,char* out_data, int width, int height,int dep)
{

	FILE* fp_ppm;
	char filename_ppm[OUTPUT_FILENAME_SIZE];
	char *filename_temp;
	char filename_bak[OUTPUT_FILENAME_SIZE];
	char filename_delim[]=".bmp";
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

	if(dep==8)
	{
		ppm_size=width*height; // gray
	}
	else if(dep==16)
	{
		ppm_size=width*height*2; // rgb
	}
	else if (dep==24)
	{
		ppm_size=width*height*3; // rgb
	}
	else if(dep==32)
	{
		ppm_size=width*height*3; // rgb
	}
	else
	{
		//unkown
	}

	alpha_size=width*height;   // alpha		
	// malloc data buffer
	alphabuf=malloc(alpha_size);
	if(!alphabuf)
	{
		printf("alloc alpha data fail \r\n");
	}

	ppmbuf=malloc(ppm_size);
	if(!ppmbuf)
	{
		printf("alloc ppmbuf data fail \r\n");
	}		

	// create filename
	strcpy(filename_bak,stream_name);
	
	//filename_temp=strtok(filename_bak,filename_delim);
	strlower(filename_bak); // => upper case
	filename_temp=(char*)strstr(filename_bak,filename_delim);
	filename_temp[0]='\0';
	filename_temp=filename_bak;
	//fprintf(fp_log,"filename_temp: %s \r\n",filename_temp);
	sprintf(filename_ppm,"%s_%d.ppm",filename_temp,dep);

	// open files
	if(dep==32)
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

	
	// write .ppm/.alpha file header
	if(dep==8)
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

	if(dep==32)
	{
		fprintf(fp_alpha, "%s\n", "P5");
		fprintf(fp_alpha, "%d %d\n", width,height);
		fprintf(fp_alpha, "%d\n", 255);
		fflush(fp_alpha);			
	}

	// copy data
	dat=out_data;
	dst_p=ppmbuf;
	dst_a=alphabuf;
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			if(dep==8)
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
			if(dep==32)
			{
				*dst_a++=*dat++; //alpha
			}				
		}
	}
	
	// write data into files
	fwrite(ppmbuf, sizeof(char), ppm_size, fp_ppm);
	if(dep==32)
	{
		fwrite(alphabuf, sizeof(char), alpha_size, fp_alpha);
		fclose(fp_alpha);
	}
								
	fclose(fp_ppm);		
	free(alphabuf);
	free(ppmbuf);
	return 1;
}
#endif

/*******************************************************************************
 *
 *   FUNCTION NAME - test_start
 *
 *   ARGUMENTS     - none
 *
 *   RETURN VALUE  - none
 *
 *******************************************************************************
 *
 *   DESCRIPTION
 *   Function opens a test file which is used to dump debug data
 ******************************************************************************/
void test_start()
{
#ifndef __SYMBIAN32__
	pftest = fopen("test_file.unix", "wb+");
#else
	pftest = fopen("D:\\test_file.unix", "wb+");
#endif
}

/*******************************************************************************
 *
 *   FUNCTION NAME - test_end
 *
 *   ARGUMENTS     - none
 *
 *   RETURN VALUE  - none
 *
 *******************************************************************************
 *
 *   DESCRIPTION
 *   Function closes the test file which is opened by an earlier
 *   function
 ******************************************************************************/
void test_end()
{
	fclose(pftest);
}

/*******************************************************************************
 *
 *   FUNCTION NAME - dump_data
 *
 *   ARGUMENTS     - str - Pointer to char string containing the data
 *                         to be dumped
 *                   num - number of bytes to be dumped
 *
 *   RETURN VALUE  - none
 *
 *******************************************************************************
 *
 *   DESCRIPTION
 *   Function used to dump certain number of debug bytes in test file
 ******************************************************************************/
void dump_data(char *str, int num)
{
	fwrite(str, sizeof(char), num, pftest);
}

/*******************************************************************************
 *
 *   FUNCTION NAME - BMP_get_new_data
 *
 *   ARGUMENTS     - new_buf_ptr : address of new buffer pointer
 *                 - new_buf_len : UINT32 type pointer to numbytes read.
 *                 - dec_object    :  BMP decoder object
 *
 *   RETURN VALUE  - int : Error value
 *
 *******************************************************************************
 *
 *   DESCRIPTION
 *   Read file bytes in buffer.
 ******************************************************************************/
BMP_error_type BMP_get_new_data (BMP_UINT8 **new_buf_ptr, BMP_UINT32 *new_buf_len, BMP_Decoder_Object * dec_object)
{
    int bytes_read;

#ifdef TEST_SUSPENSION
	static int counter = 0;
#endif
#ifdef TEST_SUSPENSION
	counter++;

	if (counter == 4)
	{
		return BMP_ERR_SUSPEND;
	}
#endif

    bytes_read = fread((void*)buff, sizeof(char), INPUT_IMAGE_BUFFER_SIZE, fp_input_image);
    if(bytes_read)
    {
		*new_buf_len = bytes_read;
		*new_buf_ptr = buff;
        return 0;
    }
    else
    {
        return BMP_ERR_EOF;
    }
}

/*******************************************************************************
 *
 *   FUNCTION NAME - BMP_seek_file
 *
 *   ARGUMENTS     - dec_object    :  BMP decoder object
 *                 - numbytes        : number of bytes to seek
 *                 - start_or_current: flag to specify whether to seek
 *                                     from start or end.
 *
 *   RETURN VALUE  - int : Error value
 *
 *******************************************************************************
 *
 *   DESCRIPTION
 *   Seek numbytes in the file from start or current position
 ******************************************************************************/
BMP_error_type BMP_seek_file(BMP_Decoder_Object *dec_object, BMP_INT32 numbytes, BMP_Seek_File_Position start_or_current)
{
//    printf("num bytes to seek = %d\n", numbytes);

    if(start_or_current == BMP_SEEK_FILE_CURR_POSITION)
    {
        if(fseek(fp_input_image, numbytes, SEEK_CUR) != 0)
        {
            return BMP_ERR_ERROR;
        }

    }
    else if (start_or_current == BMP_SEEK_FILE_START)
    {
        if(fseek(fp_input_image, numbytes, SEEK_SET) != 0)
        {
            return BMP_ERR_ERROR;
        }
    }
    return BMP_ERR_NO_ERROR;
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
 *   ample decoding function to test bmp decoder on unix/windows
 ******************************************************************************/
#ifndef OS_VRTX
int main(int argc, char *argv[])
#else
int BMP_dec_main(int argc, char *mm_argv)
#endif
{
    FILE *fp_write;
    BMP_Decoder_Object BmpDecObject;
    BMP_error_type bmp_err;
    int index, row_index;
    char *out_buf, *out_image;
	int mem_out_pix;
	int mult_factor=3;
	int outbuf_offset=0;
	
	
    
#ifdef TEST_SUSPENSION
	int suspended = 0;
	int temp=0;
#endif

#ifdef MEASURE_STACK_USAGE
       unsigned int           *ps32SP, *ps32BaseSP;
       unsigned int           s32StackCount, s32StackValue;
       unsigned int           s32PeakStack = 0;
#endif



#ifdef TIME_PROFILE
	struct timeval StartTime, EndTime;
	unsigned long TotalDecTimeUs = 0;
	unsigned int n_frames = 1,Max_time = 0,Min_time = 0,f_Max_time = 0,f_Min_time = 0,TotalCoreCycles = 0;
    unsigned char chname[] = "[PROFILE-INFO]";
#endif


#ifdef TIME_PROFILE_RVDS
	int prev_clk, curr_clk,clk,n_frames=1;
	int total_clk =0,max_clk=0,min_clk=0,avg_clk=0,f_max_clk=0,f_min_clk=0;
	int init_cache=0; 
	extern int prev_cycles(void);
	extern int curr_cycles(void);
	unsigned char chname[] = "[PROFILE-INFO]";
#endif

#ifdef MEASURE_HEAP_USAGE
       s32TotalMallocBytes = 0;
#endif

#ifdef OS_VRTX
	char *argv[10];
	char *abc = mm_argv;

	abc += strlen(abc)+1;
	argv[0] = abc;
	abc += strlen(abc)+1;
	argv[1] = abc;
	abc += strlen(abc)+1;
	argv[2] = abc;
	abc += strlen(abc)+1;
	argv[3] = abc;	
	abc += strlen(abc)+1;
	argv[4] = abc;	
	abc += strlen(abc)+1;
	argv[5] = abc;	
	abc += strlen(abc)+1;
	argv[6] = abc;	
	abc += strlen(abc)+1;
	argv[7] = abc;	
	
	argc -= 1; 
#endif

	/*Check for correct count of program arguments*/
	if(argc!=7)
	{
		printf("Number of arguments should be equal to 6 \n");
		printf("Usage :<input-file> <output-file> <output-format> \
<output-width> <output-height> <scaling-mode>");

		return -1;
	}

    fp_input_image = fopen(argv[1], "rb");
    if (fp_input_image == NULL)
    {
        printf("Error opening input image file \n");
        return 2;
    }

#ifndef OUT_PPM_FILE
    fp_write = fopen(argv[2], "wb+");
    if (fp_write == NULL)
    {
        printf("Error opening output image file \n");
        return -1;
    }
#endif	
    test_start();

	/*Configure output parameters*/

	BmpDecObject.dec_param.out_format    = atoi(argv[3]);
	BmpDecObject.dec_param.output_width  = atoi(argv[4]);
    BmpDecObject.dec_param.output_height = atoi(argv[5]);
    BmpDecObject.dec_param.scale_mode    = atoi(argv[6]);
	BmpDecObject.BMP_get_new_data=BMP_get_new_data;
	BmpDecObject.BMP_seek_file=BMP_seek_file;

	/*Validation of configured parameters*/

	if(BmpDecObject.dec_param.out_format<0 || BmpDecObject.dec_param.out_format>3)
	{
		printf("Configured output Format is invalid");
		return -1;
	}
	if(BmpDecObject.dec_param.scale_mode<0 || BmpDecObject.dec_param.scale_mode>1)
	{
		printf("Configured scaling mode is invalid");
		return -1;
	}
	if(BmpDecObject.dec_param.output_width<=0)
	{
		printf("Configured width is invalid ");
		return -1;
	}

	if(BmpDecObject.dec_param.output_height<=0)
	{
		printf("Configured height is invalid ");
		return -1;
	}

    /*Query for memory requirements*/

    bmp_err = BMP_query_dec_mem(&BmpDecObject);

    if(bmp_err != BMP_ERR_NO_ERROR)
    {
        printf("error BMP mem query \n");
        return -1;
    }

	/*Allocate memory after the querying for memory requirements is done*/

    for(index = 0; index < BmpDecObject.mem_info.num_reqs; index++)
    {
        BmpDecObject.mem_info.mem_info_sub[index].ptr
            = (void*)malloc(sizeof(char) * BmpDecObject.mem_info.mem_info_sub[index].size);
        if(BmpDecObject.mem_info.mem_info_sub[index].ptr == NULL)
        {
            printf(" malloc error after query mem \n");
            return -1;
        }
        else
        {
          #ifdef MEASURE_HEAP_USAGE
             s32TotalMallocBytes+=(BmpDecObject.mem_info.mem_info_sub[index].size);
         #endif

        }

    }

#ifdef MEASURE_STACK_USAGE
    PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
#endif

    /*Initialize the BMP decoder*/

	bmp_err = BMP_decoder_init(&BmpDecObject);

#ifdef MEASURE_STACK_USAGE    
    GET_STACK_USAGE (ps32BaseSP, ps32SP, s32StackCount, s32PeakStack);
#endif

	if(bmp_err != BMP_ERR_NO_ERROR)
	{
        printf("error BMP_decoder_init \n");
		return -1;
	}

	/*Set the multiplication factor depending upon the
	  output format specified.This mutiplication factor
	  is useful for allocation and modification of output buffer
	*/

	if ((BmpDecObject.dec_param.out_format == E_BMP_OUTPUTFORMAT_RGB555)
			      || (BmpDecObject.dec_param.out_format == E_BMP_OUTPUTFORMAT_RGB565))
	{
		mult_factor=2;
	}
	/*For RGB666 and RGB888*/
#if 1 //eagle
	else if(BmpDecObject.dec_info_init.bit_cnt==32)
	{
		printf("32 bit color (alpha channel) ! \r\n");
		mult_factor=4;
	}
#endif
	else
	{
		mult_factor=3;
	}

	/*Allocate memory for output buffer*/
	mem_out_pix = BmpDecObject.dec_info_init.output_width
                             * BmpDecObject.dec_info_init.output_height
                             * mult_factor;

    out_image = (char*) calloc(mem_out_pix,sizeof(char));
    if(out_image == NULL)
    {
        printf(" out_image memory allocation error \n");
		return -1;
    }

  	if(BmpDecObject.dec_info_init.image_height>0)
  	{
    		out_buf = (out_image + mem_out_pix - (int)(BmpDecObject.dec_info_init.output_width * mult_factor));
		outbuf_offset=0-(BmpDecObject.dec_info_init.output_width*mult_factor);
  	}
	else
	{
		printf("negative height ! \r\n");
		out_buf= out_image;
		outbuf_offset=BmpDecObject.dec_info_init.output_width*mult_factor;
	}


	/*Decode BMP input stream row by row*/

    for(row_index = 0; row_index < BmpDecObject.dec_info_init.output_height; row_index++)
    {

#ifdef TEST_SUSPENSION
		if (suspended == 1)
		{
			suspended = 0;
			temp = 0 - BmpDecObject.num_byte_read_in_row ;

			printf("File pointer before seek = %d \n", ftell(fp_input_image));
            if(fseek(fp_input_image, temp, SEEK_CUR) != 0)
			{
                return -1;
			}
            printf("file pointer before seek = %d \n", ftell(fp_input_image));
		}
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
       bmp_err = BMP_decode_row_pp(&BmpDecObject, out_buf);

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

#ifdef MEASURE_STACK_USAGE
    if (s32PeakStack < s32StackValue)
        s32PeakStack = s32StackValue;
#endif     


        dump_data((char*)out_buf, BmpDecObject.dec_info_init.output_width*mult_factor);
		if (bmp_err == BMP_ERR_SUSPEND)
		{

#ifdef TEST_SUSPENSION
			suspended = 1;
			row_index--;
			printf("Number of bytes in the row = %d\n", BmpDecObject.num_byte_read_in_row );
#endif
			printf("Data not yet ready\n");
			continue;
		}
        if(bmp_err != BMP_ERR_NO_ERROR)
        {
            printf("BMP decode error \n");
            return -1;
        }

		//out_buf -= (BmpDecObject.dec_info_init.output_width*mult_factor);
		out_buf += outbuf_offset;
    }

    printf("decoded successfully ! \n");


#if 1 //eagle
#ifdef OUT_PPM_FILE
	{
		int dep;
		//dep=BmpDecObject.dec_info_init.bit_cnt==32? 32:24;
		dep=get_outbit_depth(BmpDecObject.dec_info_init.bit_cnt,BmpDecObject.dec_param.out_format);
		write_ppm_alpha_file(argv[1],out_image,BmpDecObject.dec_info_init.output_width,BmpDecObject.dec_info_init.output_height,dep);
	}
#endif
#else
	/*Write the decoded output data to a file*/
	fprintf(fp_write, "%s\n", "P6");
	fprintf(fp_write, "%d %d\n", BmpDecObject.dec_info_init.output_width,
		BmpDecObject.dec_info_init.output_height);

	fprintf(fp_write, "%d\n", 255);
      fflush(fp_write);

	fwrite(out_image, sizeof(char), mem_out_pix, fp_write);
	fclose(fp_write);
#endif

	fclose(fp_input_image);

#ifdef TIME_PROFILE
           
			//printf("Total Decode Time [microseconds] = %ld\n", TotalDecTimeUs);
			printf("\n%s\t%s\t%dx%d\t%d\t%dx%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",chname,argv[1],\
			        BmpDecObject.dec_info_init.output_height,BmpDecObject.dec_info_init.output_width,BmpDecObject.dec_info_init.bit_cnt,\
			        BmpDecObject.dec_param.output_height,BmpDecObject.dec_param.output_width,BmpDecObject.dec_param.out_format, \
					Max_time,Min_time,n_frames,f_Max_time,f_Min_time,TotalDecTimeUs,TotalCoreCycles);
#endif

#ifdef __SYMBIAN32__
#ifdef MEASURE_HEAP_USAGE
			printf("\n Stack and Heap are  %d\t%d\t",s32PeakStack,s32TotalMallocBytes);
#endif
#endif

#ifdef TIME_PROFILE_RVDS   
        
        #ifdef MEASURE_STACK_USAGE  
        #ifdef MEASURE_HEAP_USAGE
             printf("\n[PROFILE-INFO]\t%s\t0\t0\t%d\t0\t0\t%ld\t0\t%d x %d\t0\t0\t0\t0\t%d\t%d\n",argv[1],n_frames,total_clk,BmpDecObject.dec_info_init.output_width,BmpDecObject.dec_info_init.output_height,s32PeakStack,s32TotalMallocBytes);  	
 		#endif
 		#endif

        #ifndef MEASURE_STACK_USAGE  
        #ifndef MEASURE_HEAP_USAGE
             printf("\n[PROFILE-INFO]\t%s\t0\t0\t%d\t0\t0\t%ld\t0\t%d x %d\t0\t0\t0\t0\n",argv[1],n_frames,total_clk,BmpDecObject.dec_info_init.output_width,BmpDecObject.dec_info_init.output_height);  	
 		#endif
 		#endif

#endif

	/*Free the allocated memory*/
    free(out_image);
    for(index = 0; index < BmpDecObject.mem_info.num_reqs; index++)
    {
        free(BmpDecObject.mem_info.mem_info_sub[index].ptr);
    }
	test_end();
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

