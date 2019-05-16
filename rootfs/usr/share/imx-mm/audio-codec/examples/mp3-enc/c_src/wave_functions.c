/****************************************************************************
 * (C) 2006 ALLGO EMBEDDED SYSTEMS PVT. LTD                                 *
 *                                                                          *
 * ORIGINAL AUTHOR: SRIPATHI KAMATH                                         *
 ****************************************************************************
 ***********************************************************************
 * Copyright 2006-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc.
 ***********************************************************************
 * CHANGE HISTORY
 *
 * dd/mm/yy   Code Ver     Description                        Author
 * --------   -------      -----------                        ------
 * 21/03/06    01          initial revision                   Sripathi Kamath
 * 02/06/08    02          Enable my fread to bypass
 *                         the file rw issues on ARM12 platfrom  Lionel Xu
 ****************************************************************************
 * DESCRIPTION
 *   This file contains functions for reading and writing wav files
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wave_functions.h"

#ifdef ENABLE_MY_FREAD

extern struct_input_data input_raw_data;

#ifdef TWO_INSTANCES
extern struct_input_data input_raw_data1;
#endif

int my_afread (short int *inbuf, int size, int no_of_elements, void *input_buff)
{
    static int count=0;
    int samp_read;
    static int num_samples, num_bytes;

    /* For the first call, read the no. of data bytes in the
     * wav file and store it in a static variable
     */
    if(count == 0)
    {
		my_fread(&num_bytes, sizeof (long int), 1, input_buff);
        num_samples = num_bytes/2;
    }

    /* If the no_of_elements asked for are available in the input,
     * that many elements are returned
     */
    if((count+no_of_elements) < num_samples)
    {
		samp_read = my_fread(inbuf, sizeof (short int), no_of_elements, input_buff);
        count += samp_read;
    }
    /* If the no_of_elements asked for are not available in the input,
     * only as many available are returned
     */
    else if(count < num_samples)
    {
		samp_read = my_fread(inbuf, sizeof (short int), num_samples-count, input_buff);
        count += samp_read;
    }
#ifdef TWO_INSTANCES
    /* Else return 0
     */
    else
        samp_read = 0;
    return(samp_read);
}
int my_afread1 (short int *inbuf, int size, int no_of_elements, void *input_buff)
{
    static int count1=0;
    int samp_read;
    static int num_samples1, num_bytes1;

    /* For the first call, read the no. of data bytes in the
     * wav file and store it in a static variable
     */
    if(count1 == 0)
    {
		my_fread(&num_bytes1, sizeof (long int), 1, input_buff);
        num_samples1 = num_bytes1/2;
    }

    /* If the no_of_elements asked for are available in the input,
     * that many elements are returned
     */
    if((count1+no_of_elements) < num_samples1)
    {
		samp_read =  my_fread(inbuf, sizeof (short int), no_of_elements, input_buff);
        count1 += samp_read;
    }
    /* If the no_of_elements asked for are not available in the input,
     * only as many available are returned
     */
    else if(count1 < num_samples1)
    {
		samp_read = my_fread(inbuf, sizeof (short int),num_samples1-count1, input_buff);
        count1 += samp_read;
    }
#endif
    /* Else return 0
     */
    else
        samp_read = 0;
    return(samp_read);
}


#else
/*****************************************************************************
 *
 * afread
 *
 * DESCRIPTION
 *   This function reads the pcm samples from a wave file and returns
 *   the no. of samples read
 *
 * INPUT
 *   inbuf          - pointer to buffer into which the samples are returned
 *   size           - the size of each PCM sample to be read
 *   no_of_elements - no. of PCM samples to be read
 *   ifp            - file pointer from which the inputs are read
 *
 * OUTPUT
 *   Returns the no. of samples read successfully
 *****************************************************************************/
int afread (short int *inbuf, int size, int no_of_elements, FILE *ifp)
{
    static int count=0;
    int samp_read;
    static int num_samples, num_bytes;

    /* For the first call, read the no. of data bytes in the
     * wav file and store it in a static variable
     */
    if(count == 0)
    {
        fread(&num_bytes, sizeof (long int), 1, ifp);
        num_samples = num_bytes/2;
    }

    /* If the no_of_elements asked for are available in the input,
     * that many elements are returned
     */
    if((count+no_of_elements) < num_samples)
    {
        samp_read = fread(inbuf, sizeof (short int), no_of_elements, ifp);
        count += samp_read;
    }
    /* If the no_of_elements asked for are not available in the input,
     * only as many available are returned
     */
    else if(count < num_samples)
    {
        samp_read = fread(inbuf, sizeof (short int), num_samples-count, ifp);
        count += samp_read;
    }
#ifdef TWO_INSTANCES
    /* Else return 0
     */
    else
        samp_read = 0;
    return(samp_read);
}
int afread1 (short int *inbuf, int size, int no_of_elements, FILE *ifp)
{
    static int count1=0;
    int samp_read;
    static int num_samples1, num_bytes1;

    /* For the first call, read the no. of data bytes in the
     * wav file and store it in a static variable
     */
    if(count1 == 0)
    {
        fread(&num_bytes1, sizeof (long int), 1, ifp);
        num_samples1 = num_bytes1/2;
    }

    /* If the no_of_elements asked for are available in the input,
     * that many elements are returned
     */
    if((count1+no_of_elements) < num_samples1)
    {
        samp_read = fread(inbuf, sizeof (short int), no_of_elements, ifp);
        count1 += samp_read;
    }
    /* If the no_of_elements asked for are not available in the input,
     * only as many available are returned
     */
    else if(count1 < num_samples1)
    {
        samp_read = fread(inbuf, sizeof (short int), num_samples1-count1, ifp);
        count1 += samp_read;
    }
#endif
    /* Else return 0
     */
    else
        samp_read = 0;
    return(samp_read);
}

#endif

#ifdef ENABLE_MY_FREAD

void my_afopen (int *num_channels, int *filetype, void * input_data)
{
    int data, fmt_length;
    int temp;
    short int s_data;

    *num_channels = 2;            /*default*/

	my_fread(&data, sizeof (long int), 1, input_data);
    /* Checks whether the first 4 bytes stands for RIFF
     */
    if(data != 0x46464952)
    {
        fprintf(stderr, "Warning: RIFF Header not found, Assuming raw PCM input\n");
        *filetype = 0;
		my_fseek(input_data, 0, SEEK_SET);
    }
    else
    {
		my_fseek(input_data, 4, SEEK_CUR);
        my_fread(&data, sizeof (long int), 1, input_data);
        /* Checks whether WAVE header is present
         */
        if(data != 0x45564157)
        {
            fprintf(stderr, "Warning: WAVE Header not found, Assuming raw PCM input\n");
            *filetype = 0;
           	my_fseek(input_data, 0, SEEK_SET);
        }
        else
        {
            *filetype = 1;
             my_fread(&data, sizeof (long int), 1, input_data);

            /* Seeks for format chunk in the file
             */
            while(data != 0x20746d66)
            {
                temp =  my_fread(&data, sizeof (long int), 1, input_data);
                if(temp == 0)
                {
                    fprintf(stderr, "Error: Format Chunk not found in the Wave File\n");
                    exit(0);
                }
                my_fseek(input_data, data, SEEK_CUR);
                my_fread(&data, sizeof (long int), 1, input_data);
            }
			my_fread(&fmt_length, sizeof (long int), 1, input_data);


            /* Checks whether the wav file consists of uncompressed PCM samples
             */
			my_fread(&s_data, sizeof (short int), 1, input_data);
            if(s_data != 1)
            {
                fprintf(stderr, "Error: Compressed Data Found. Only wave files with PCM data can be used for input\n");
                exit(0);
            }

            /* Read the no. of channels in the wave file
             */
			my_fread(num_channels, sizeof (short int), 1, input_data);

			my_fseek(input_data, fmt_length-4, SEEK_CUR);

			my_fread(&data, sizeof (long int), 1, input_data);

            /* Seeks for data chunk in the file
             */
            while(data != 0x61746164)
            {
				temp = my_fread(&data, sizeof (long int), 1, input_data);
                if(temp == 0)
                {
                    fprintf(stderr, "Error: Data Chunk not found in the Wave File\n");
                    exit(0);
                }
				my_fseek(input_data, data, SEEK_CUR);
				my_fread(&data, sizeof (long int), 1, input_data);
            }
        }
    }

}

#else
/*****************************************************************************
 *
 * afopen
 *
 * DESCRIPTION
 *   This function reads the header of the input file and checks
 *   whether it is a valid wav file. If the header is not of a wav file,
 *   input is assumed to be from a PCM file
 *
 * INPUT
 *   inPath       - path of the input file
 *   filtype      - pointer to address where the input filetype is returned.
 *                  This function writes 0 if it is a PCM file, 1 for wav file.
 *   num_channels - pointer to address where the no. of channels in the
 *                  wave file is returned.
 *
 * OUTPUT
 *   Returns the file pointer
 *****************************************************************************/
FILE *afopen (char *inPath, int *num_channels, int *filetype)
{
    FILE *ifp;
    int data, fmt_length;
    int temp;
    short int s_data;

    *num_channels = 2;            /*default*/
    ifp = fopen (inPath,"rb");
    fread(&data, sizeof (long int), 1, ifp);

    /* Checks whether the first 4 bytes stands for RIFF
     */
    if(data != 0x46464952)
    {
        fprintf(stderr, "Warning: RIFF Header not found, Assuming raw PCM input\n");
        *filetype = 0;
        fseek(ifp, 0, SEEK_SET);
    }
    else
    {
        fseek(ifp, 4, SEEK_CUR);
        fread(&data, sizeof (long int), 1, ifp);

        /* Checks whether WAVE header is present
         */
        if(data != 0x45564157)
        {
            fprintf(stderr, "Warning: WAVE Header not found, Assuming raw PCM input\n");
            *filetype = 0;
            fseek(ifp, 0, SEEK_SET);
        }
        else
        {
            *filetype = 1;
            fread(&data, sizeof (long int), 1, ifp);

            /* Seeks for format chunk in the file
             */
            while(data != 0x20746d66)
            {
                temp = fread(&data, sizeof (long int), 1, ifp);
                if(temp == 0)
                {
                    fprintf(stderr, "Error: Format Chunk not found in the Wave File\n");
                    exit(0);
                }
                fseek(ifp, data, SEEK_CUR);
                fread(&data, sizeof (long int), 1, ifp);
            }
            fread(&fmt_length, sizeof (long int), 1, ifp);

            /* Checks whether the wav file consists of uncompressed PCM samples
             */
            fread(&s_data, sizeof (short int), 1, ifp);
            if(s_data != 1)
            {
                fprintf(stderr, "Error: Compressed Data Found. Only wave files with PCM data can be used for input\n");
                exit(0);
            }

            /* Read the no. of channels in the wave file
             */
            fread(num_channels, sizeof (short int), 1, ifp);

            fseek(ifp,fmt_length-4,SEEK_CUR);
            fread(&data, sizeof (long int), 1, ifp);

            /* Seeks for data chunk in the file
             */
            while(data != 0x61746164)
            {
                temp = fread(&data, sizeof (long int), 1, ifp);
                if(temp == 0)
                {
                    fprintf(stderr, "Error: Data Chunk not found in the Wave File\n");
                    exit(0);
                }
                fseek(ifp, data, SEEK_CUR);
                fread(&data, sizeof (long int), 1, ifp);
            }
        }
    }
    /*Returns file pointer*/
    return ifp;
}


#endif


#ifdef ENABLE_MY_FREAD
size_t my_fread(void *buff, size_t unit, size_t size, void *fp)
{
	int bytes_read;
	struct_input_data *input_data=(struct_input_data *)fp;

	bytes_read = unit*size;

	if(bytes_read+input_data->bytes_read<=input_data->file_size)
	{
		memcpy(buff,input_data->buff,bytes_read);
		input_data->bytes_read += bytes_read;
		input_data->buff += bytes_read;
	}
	else if(input_data->file_size-input_data->bytes_read<bytes_read)
	{
		bytes_read = input_data->file_size-input_data->bytes_read;
		memcpy(buff,input_data->buff,bytes_read);
		input_data->bytes_read += bytes_read;
		input_data->buff += bytes_read;
	}

	bytes_read = bytes_read/unit;

	return bytes_read;
}

int my_fseek(void *fp, long long offset, int origin)
{
    struct_input_data *input_data=(struct_input_data *)fp;
    if (origin == SEEK_SET)
    	{
    	input_data->bytes_read = (int)offset;
		input_data->buff = input_data->buff_head + offset;
    	}

	else if (origin == SEEK_CUR)
		{
		input_data->bytes_read += (int)offset;
		input_data->buff += offset;
		}

	else if (origin == SEEK_END)
		{
		return 0;
		}
	else
		return 0;

	return 0;
}

int my_fclose(void *fp)
{
	struct_input_data *input_data=(struct_input_data *)fp;
	free(input_data->buff_head);
	free(fp);
	return 0;
}

long my_ftell(void *fp)
{
	struct_input_data *input_data=(struct_input_data *)fp;
	return input_data->bytes_read;
}

#endif //TEST_PERFORMANCE


