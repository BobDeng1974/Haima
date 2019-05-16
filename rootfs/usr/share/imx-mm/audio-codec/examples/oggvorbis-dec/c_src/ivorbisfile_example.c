/*  Copyright 2010, Xiph.Org Foundation
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    Neither the name of the Xiph.org Foundation nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.
  This software is provided by the copyright holders and contributors ¡°as is¡±
  and any express or implied warranties, including, but not limited to, the
  implied warranties of merchantability and fitness for a particular purpose are
  disclaimed. In no event shall the foundation or contributors be liable for any
  direct, indirect, incidental, special, exemplary, or consequential damages
  (including, but not limited to, procurement of substitute goods or services;
  loss of use, data, or profits; or business interruption) however caused and on
  any theory of liability, whether in contract, strict liability, or tort
  (including negligence or otherwise) arising in any way out of the use of this
  software, even if advised of the possibility of such damage.
*/
/*
 ***********************************************************************
 * Copyright (c) 2005-2009, 2012, Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc.
 ***********************************************************************
 */

/********************************************************************

 function: simple example decoder using vorbis decoder

********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oggvorbis_dec_api.h" //DSPhl28176

#ifdef TIME_PROFILE
#include <sys/time.h>
#endif

#ifdef __WINCE
#include <windows.h>
#include <winbase.h>
#endif

#ifdef TIME_PROFILE_WINCE         /* for taking timing on wince platform*/
extern void INTERRUPTS_SET(int k);
#endif

#define OUT_DUMP
#if defined(TIME_PROFILE) || defined(TIME_PROFILE_RVDS) || defined(TIME_PROFILE_WINCE)
#undef OUT_DUMP
#endif
#ifdef MEASURE_STACK_USAGE //TLSbo88602
#include "mm_codec_common.h"
#endif

#define CHUNK_SIZE 8192*6*2 //max 8192 sample * max channel * sizeof(short)
/* Packet Types */
#define INFORMATION_HEADER  0x01
#define COMMENT_HEADER      0x03
#define CODEC_SETUP_HEADER  0x05

typedef struct {
    FILE *fp;
    int file_size;
    int bytes_read;
    char *buff;
    char *buff_head;
}struct_input_file;

//Callbacks
size_t my_fread(void *, size_t, size_t, void *);
int my_fseek(void *, ogg_int64_t, int);
int my_fclose(void *);
long my_ftell(void *);

void OggDecode(char *argv[]);
void Usage(void);

#ifdef RELOCATION
#undef REENTRANCY
#endif

#ifdef REENTRANCY
#undef RELOCATION
#endif

#define INPUT_BUFFER_SIZE 8*1024
char input_buffer[INPUT_BUFFER_SIZE]; //hold input buffer

#ifdef __WINCE
int  _tmain(int argc, char *argv[])
#else
int main(int argc,char *argv[])
#endif
{
#ifdef REENTRANCY
    char *argv1[3],*argv2[3];
    int i=0,j=0;

#if defined (__GNUC__)
    pthread_t thread1, thread2;
    int  ret1, ret2;
#endif
#endif
    if(argc < 4)
    {
        Usage();
        exit (0);
    }

	/*Reentrancy*/
#ifdef __WINCE
	{
		_TCHAR *arg_word;
		char * arg_byte,i;
		for (i = 0; i < argc; i++)
		{
			arg_word =(_TCHAR *)argv[i];
			arg_byte = argv[i];

			while(*(arg_word) != '\0')
			{
				*arg_byte++=(char)*arg_word++;
			}
			*arg_byte=(char)'\0';
		}
	}
#endif
#ifdef REENTRANCY
    if(argc < 7)
    {
        Usage();
        exit (0);
    }

    for(i=1,j=1;i <= 3;i++)
    {
        argv1[i]=argv[j++];

    }
    i=1;
    while(j != argc)
    {
        argv2[i]=argv[j++];
        i++;
    }
#if defined (__GNUC__)
    // Re-entrancy Test : Create independent threads each of which will execute the decoder function
    ret1 = pthread_create( &thread1, NULL, (void*) OggDecode, (void*) argv1);
    ret2 = pthread_create( &thread2, NULL, (void*) OggDecode, (void*) argv2);

    pthread_join( thread1, NULL);
    pthread_join( thread2, NULL);
#else
    OggDecode(argv);
#endif

#else
    OggDecode(argv);
#endif
    return 0;
}

void OggDecode(char *argv[])
{
    long ret =0;
    sOggVorbisDecObj sOVDecObj;
    unsigned char *old_buffer;
    struct_input_file *input_file;
    long size=0;
    char *pcmout=NULL;
    FILE *fpin=NULL,*fpout=NULL,*fperr=NULL;
    int eof=0;

#ifdef RELOCATION
    int relocate = 0;
    int noofreloc= 10;
    unsigned char *base = NULL;
#endif

#if (defined TIME_PROFILE) || defined(TIME_PROFILE_RVDS)
    unsigned int first_frame = 1;
#endif

#ifdef TIME_PROFILE
    struct timeval StartTime, EndTime;
    unsigned long TotalDecTimeUs = 0;
    unsigned long prev_DecTimeUs = 0;
    unsigned long min_DecTimeUs = 0;
    unsigned long cur_DecTimeUs = 0;
    int max_frame = 0 ;
    int min_frame = 0 ;
    int frame_no = 0;
    unsigned char chname[] = "[PROFILE-INFO]";
    FILE *fp_mhz;
    int outputfilesize = 0;
#ifdef ARMPLAT
    int platform = ARMPLAT;
#else
    int platform = 12;
#endif
#ifdef CPUFREQ
    int cpu_freq = CPUFREQ;
#else    
    int cpu_freq = 1000;
#endif
    int bit_len = 16;
    double performance = 0.0;
#endif

#ifdef TIME_PROFILE_RVDS                    //TLSbo85204
    typedef struct
    {
        unsigned long prev_clk;
        unsigned long curr_clk;
    }ClockLog_t;
    long totalClock=0;
    int  max_clk=0,min_clk=99999999,nframe=0,max_frame=0,min_frame=0,clk=0;
    unsigned char       chname[] = "[PROFILE-INFO]";                                    //TLSbo85204
    extern int prev_cycles(void);
    extern int curr_cycles(void);
    FILE *fp_mhz;
    ClockLog_t clockLog;
    int outputfilesize = 0;
#endif

//TLSbo88602
#ifdef MEASURE_STACK_USAGE
    int           *ps32SP, *ps32BaseSP;
    int           s32StackCount, s32StackValue;
    int           s32PeakStack = 0;
#endif

#ifdef MEASURE_HEAP_USAGE
    int      s32TotalMallocBytes=0;
#endif

#ifdef __WINCE         // for taking timing on wince platform
#ifdef TIME_PROFILE_WINCE
	BOOL Flag;
	LARGE_INTEGER lpFrequency1 ;
	LARGE_INTEGER lpPerformanceCount1;
	LARGE_INTEGER lpPerformanceCount2;
        int  TotalDecTimeUs = 0;
        int  prev_DecTimeUs = 0;
        int  cur_DecTimeUs = 0;
        int  min_DecTimeUs = 0;
        int max_frame = 0 ;
        int min_frame = 0;
 	int frame_no = 0;
        unsigned char chname[] = "[PROFILE-INFO]";
        FILE *fp_mhz;
        int outputfilesize = 0;
#endif
#endif

    // Output the version information of FLAC decoder.
    printf( "%s\n", OggVorbisVerInfo() );

//TLSbo88602
    fpin = fopen(argv[1],"rb");
    if(fpin == NULL)
    {
        printf("Error opening i/p file %s\n",argv[1]);
        return ;
    }

    input_file = (struct_input_file *)malloc(sizeof(struct_input_file));
    if(input_file == NULL)
    {
        printf("Error Allocating memory \n");
        return;
    }

    input_file->fp = fpin;
    input_file->file_size = 0;
    input_file->bytes_read = 0;


    fseek(fpin, 0, SEEK_END);
    /*get the stream to the buffer*/
    input_file->file_size = ftell(fpin);
    fseek(fpin, 0, SEEK_SET);

    input_file->buff = (char *)malloc(sizeof(char)*input_file->file_size);
    input_file->buff_head = input_file->buff;
    fread(input_file->buff,sizeof(char),input_file->file_size,fpin);

#ifdef OUT_DUMP
    fpout = fopen(argv[2],"wb");
    if(fpout == NULL)
    {
        printf("Error opening o/p file %s\n",argv[2]);
        return ;
    }
#endif
    fperr = fopen(argv[3],"wb");
    if(fperr == NULL)
    {
        printf("Error opening err file %s\n",argv[3]);
        return ;
    }
#if (defined TIME_PROFILE || defined TIME_PROFILE_RVDS || defined TIME_PROFILE_WINCE)
    fp_mhz = fopen("../audio_performance.txt","a");
    if (fp_mhz == NULL)
        printf("Couldn't open cycles file\n");
    fprintf (fp_mhz,"\n|\tAudio Format\t|\tPlat\t|\tSamplerate\t|\tCh\t|\tBiterate\t|\tBit\t|\tPerf(MIPS)\t|\tAudio file name\n");
#endif
    /*Allocating aligned memory for output buffer*/
    pcmout = (char *)malloc(sizeof(char)*CHUNK_SIZE + 4);
    pcmout = (char *)(((unsigned int)pcmout + 3) & ~0x3);

    /* Initialization of the decoder object */
    sOVDecObj.datasource = (void *)input_file->buff;
    sOVDecObj.read_func = my_fread;
    sOVDecObj.seek_func = my_fseek;
    sOVDecObj.close_func = my_fclose;
    sOVDecObj.tell_func = my_ftell;
    sOVDecObj.pcmout = pcmout;
    sOVDecObj.output_length = CHUNK_SIZE;
    sOVDecObj.buffer_length = input_file->file_size;

    size = OggVorbisQueryMem(&sOVDecObj);
    //printf("QueryMem Size Required %ld\n",size);



    sOVDecObj.pvOggDecObj = (void *)malloc(sOVDecObj.OggDecObjSize);


#ifdef MEASURE_HEAP_USAGE
                s32TotalMallocBytes +=  sOVDecObj.OggDecObjSize;
#endif

    memset(sOVDecObj.pvOggDecObj,0,sOVDecObj.OggDecObjSize);
    sOVDecObj.buf_size = size;

    /*decoder internal buffer allocation*/
#ifdef RELOCATION
    sOVDecObj.decoderbuf = (unsigned char*)malloc(noofreloc*(size+4));
    base = sOVDecObj.decoderbuf;
#else
    sOVDecObj.decoderbuf = (unsigned char*)malloc(size);
#endif

    if(!sOVDecObj.decoderbuf)
    {
        printf("Internal Decoder buffer allocation failed!!");
        return ;
    }
    memset(sOVDecObj.decoderbuf,0,size);

    sOVDecObj.datasource = (void *)input_file;

#ifndef RAW_DATA_INPUT
    if(OggVorbisDecoderInit(&sOVDecObj) != 0) {
        fprintf(fperr,"Input does not appear to be an Ogg bitstream.\n");
        printf("Input does not appear to be an Ogg bitstream.\n");
        exit(1);
    }
    while(!eof){

#ifdef TIME_PROFILE
        gettimeofday(&StartTime, NULL);
#endif
#ifdef TIME_PROFILE_RVDS                                //TLSbo85204
        clockLog.prev_clk = prev_cycles();
#endif

#ifdef MEASURE_STACK_USAGE
        PAINT_STACK (ps32BaseSP, ps32SP, s32StackCount);
#endif

#ifdef __WINCE
#ifdef TIME_PROFILE_WINCE
       INTERRUPTS_SET(0xDF);  /*disable interrupt*/
       Flag=QueryPerformanceFrequency(&lpFrequency1);
       Flag=QueryPerformanceCounter(&lpPerformanceCount1);
#endif
#endif

        ret = OggVorbisDecode(&sOVDecObj);      //Decode Call

#ifdef TIME_PROFILE
        gettimeofday(&EndTime, NULL);
        frame_no++;
        cur_DecTimeUs = (EndTime.tv_usec - StartTime.tv_usec) + (EndTime.tv_sec - StartTime.tv_sec)*1000000L;

        if(!first_frame)
            if(cur_DecTimeUs > prev_DecTimeUs)
                prev_DecTimeUs = cur_DecTimeUs ;
        first_frame = 0;
        TotalDecTimeUs += cur_DecTimeUs ;
#endif
#ifdef TIME_PROFILE_RVDS
        clockLog.curr_clk = curr_cycles();
        /* considering the devide by 64 counter measurements*/
        clk = (clockLog.curr_clk - clockLog.prev_clk)*64;
        totalClock +=  clk;
        nframe ++;

        if(!first_frame) {
               if (clk > max_clk){
                   max_clk = clk;
                   max_frame = nframe;
               }
               if (clk < min_clk) {
                   min_clk = clk;
                   min_frame = nframe;
               }
        }
        first_frame = 0;
#endif

#ifdef MEASURE_STACK_USAGE
        GET_STACK_USAGE (ps32BaseSP, ps32SP, s32StackCount, s32StackValue);
        if (s32PeakStack < s32StackValue)
            s32PeakStack = s32StackValue;
#endif

#ifdef __WINCE
#ifdef TIME_PROFILE_WINCE
	frame_no++;
	Flag=QueryPerformanceCounter(&lpPerformanceCount2);
	cur_DecTimeUs=(((lpPerformanceCount2.QuadPart - lpPerformanceCount1.QuadPart)*1000000)/(lpFrequency1.QuadPart));
        INTERRUPTS_SET(0x1F) ;  /*enable interrupt*/
	TotalDecTimeUs = TotalDecTimeUs + cur_DecTimeUs;
	if(cur_DecTimeUs > prev_DecTimeUs)
	{
	    prev_DecTimeUs = cur_DecTimeUs;
	    max_frame = frame_no;
	}
#endif
#endif

        if (ret == 0) {
            /* EOF */
            eof=1;
        } else if (ret < 0) {
            /* Application specific Error Handling */
        } else {
#ifdef OUT_DUMP
            fwrite(pcmout,1,ret,fpout);
#endif
#if(defined TIME_PROFILE || defined TIME_PROFILE_RVDS || defined TIME_PROFILE_WINCE)
            outputfilesize += ret;
#endif
        }

#ifdef RELOCATION
        if(relocate < noofreloc)
        {
            old_buffer=sOVDecObj.decoderbuf;
            sOVDecObj.decoderbuf = base + size * relocate;
            relocate++;
            sOVDecObj.decoderbuf = (unsigned char *)(((unsigned int)sOVDecObj.decoderbuf + 3) & ~0x3);
            /*copy the whole buffer upfront. The pointer will be updated later
             Note that the new location and the old location could not overlap*/
            memcpy(sOVDecObj.decoderbuf,old_buffer,sOVDecObj.buf_size);
            OggVorbisDecoderReLocate(&sOVDecObj);
            relocate = relocate % noofreloc;
        }
#endif
    }
#else
    OggVorbisDecoderInit(&sOVDecObj);
    {
        int inputSize, byte, output_sample,offset,packtype, data_left, i;
        char buffer[6];
        int byte_consumed = 0;
        inputSize = my_fread(input_buffer, 1, INPUT_BUFFER_SIZE, sOVDecObj.datasource);
        sOVDecObj.initial_buffer = input_buffer;
        while (inputSize>0) {
            /* when decode 3 header packet,in case byte_consumed is
                    not equal to packet size, seeking for "vorbis" sync word  */
            if (sOVDecObj.mPacketCount<3){
                offset = 0;
                data_left = inputSize;
                while(data_left>0){
                    packtype = input_buffer[offset];
                    if (packtype == INFORMATION_HEADER ||
                        packtype == COMMENT_HEADER ||
                        packtype == CODEC_SETUP_HEADER){
                        byte = offset+1;
                        for (i=0; i<6; i++)
                            buffer[i] = input_buffer[byte++];
                        if(memcmp(buffer,"vorbis",6)==0){
                            if(offset!=0) {
                                memmove(input_buffer, input_buffer+offset, inputSize-offset);
                                byte = my_fread(input_buffer+inputSize-offset, 1, offset,sOVDecObj.datasource);
                                inputSize = byte + inputSize - offset;
                            }
                            break;
                        }
                    }
                    offset++;
                    data_left--;
                    if(data_left==0){
                        inputSize = my_fread(input_buffer, 1, INPUT_BUFFER_SIZE, sOVDecObj.datasource);
                        if (inputSize==0)
                            break;
                        data_left = inputSize;
                    }
                }
            }
            if (inputSize==0)
                break;
            sOVDecObj.buffer_length = inputSize;
    #ifdef TIME_PROFILE
        gettimeofday(&StartTime, NULL);
    #endif
    #ifdef TIME_PROFILE_RVDS
        clockLog.prev_clk = prev_cycles();
    #endif
    #ifdef __WINCE
    #ifdef TIME_PROFILE_WINCE
       INTERRUPTS_SET(0xDF);  /*disable interrupt*/
       Flag=QueryPerformanceFrequency(&lpFrequency1);
       Flag=QueryPerformanceCounter(&lpPerformanceCount1);
    #endif
    #endif

            ret = OggVorbisDecode(&sOVDecObj); //, inputSize, &byte_consumed);
    #ifdef TIME_PROFILE
        gettimeofday(&EndTime, NULL);
        frame_no++;
        cur_DecTimeUs = (EndTime.tv_usec - StartTime.tv_usec) + (EndTime.tv_sec - StartTime.tv_sec)*1000000L;
        if(!first_frame)
            if(cur_DecTimeUs > prev_DecTimeUs)
                 prev_DecTimeUs = cur_DecTimeUs ;
        first_frame = 0;
        TotalDecTimeUs += cur_DecTimeUs ;
        outputfilesize += sOVDecObj.outNumSamples * sOVDecObj.NoOfChannels *2;
    #endif
    #ifdef TIME_PROFILE_RVDS
            clockLog.curr_clk = curr_cycles();
            /* considering the devide by 64 counter measurements*/
            clk = (clockLog.curr_clk - clockLog.prev_clk)*64;
            totalClock +=  clk;
            nframe ++;
            if(!first_frame) {
                if (clk > max_clk){
                    max_clk = clk;
                    max_frame = nframe;
                }
                if (clk < min_clk) {
                    min_clk = clk;
                    min_frame = nframe;
                }
            }
            first_frame = 0;
            outputfilesize += sOVDecObj.outNumSamples * sOVDecObj.NoOfChannels *2;
    #endif
    #ifdef __WINCE
    #ifdef TIME_PROFILE_WINCE
	    frame_no++;
	    Flag=QueryPerformanceCounter(&lpPerformanceCount2);
	    cur_DecTimeUs=(((lpPerformanceCount2.QuadPart - lpPerformanceCount1.QuadPart)*1000000)/(lpFrequency1.QuadPart));
            INTERRUPTS_SET(0x1F) ;  /*enable interrupt*/
	    TotalDecTimeUs = TotalDecTimeUs + cur_DecTimeUs;
	    if(cur_DecTimeUs > prev_DecTimeUs)
	    {
	        prev_DecTimeUs = cur_DecTimeUs;
	        max_frame = frame_no;
	    }
            outputfilesize += sOVDecObj.outNumSamples * sOVDecObj.NoOfChannels *2;
    #endif
    #endif
            if(ret) {printf("decode error!\nret == %d\n", ret); break;}
            output_sample = sOVDecObj.outNumSamples;
    #ifdef OUT_DUMP
            if (output_sample)
                fwrite(pcmout, output_sample, sizeof(short)*(sOVDecObj.NoOfChannels), fpout);
    #endif
            byte_consumed = sOVDecObj.byteConsumed;
            memmove(input_buffer, input_buffer+byte_consumed,  inputSize-byte_consumed );
            byte = my_fread(input_buffer+(inputSize-byte_consumed), 1, byte_consumed, sOVDecObj.datasource);
            inputSize = byte + (inputSize-byte_consumed);
            //printf("packet number: %d\n", sOVDecObj.mPacketCount);
        }

    }
#endif



#ifdef TIME_PROFILE
    printf("\n%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t%ld\n",chname,argv[1],\
        prev_DecTimeUs,min_DecTimeUs,frame_no,max_frame,min_frame,TotalDecTimeUs,outputfilesize);
//    fprintf(fp_mhz,"\n%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t%ld",chname,argv[1],
//        prev_DecTimeUs,min_DecTimeUs,frame_no,max_frame,min_frame,TotalDecTimeUs,outputfilesize);
        performance = (double)TotalDecTimeUs * 0.000001*cpu_freq*sOVDecObj.SampleRate * sOVDecObj.NoOfChannels *2/outputfilesize;
        fprintf(fp_mhz, "|\tOggvorbis Dec\t|\tARM%d\t|\t%9d\t|\t%d\t|\t%8d\t|\t%d\t|\t%.8f\t|%s\n", platform, sOVDecObj.SampleRate, sOVDecObj.NoOfChannels, sOVDecObj.ave_bitrate, 16, performance, argv[1]);

    /*fprintf(fp_mhz,"Input File = %s\n", argv[1]);
    fprintf(fp_mhz,"Total Decode Time [microseconds] = %ld\n", TotalDecTimeUs);*/
    fclose(fp_mhz);
#endif
#ifdef TIME_PROFILE_RVDS
    {
        int tot_char = 0 , n = 0, count = 0, ch = 0;
        char fileName[300]="";
        char file[100]="";
        strcpy(fileName,argv[1]);
        tot_char = strlen(fileName);
        count = tot_char - 1;
        while (fileName[count] != '/')
            {
                count --;
            }

        n=count+1;
        while (n != tot_char)
            {
                file[ch ++] = fileName [n ++];

            }

#ifdef MEASURE_STACK_USAGE
#ifdef MEASURE_HEAP_USAGE
                printf("%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t\n",chname,file,\
                        max_clk,min_clk,nframe,max_frame,min_frame,totalClock,outputfilesize,s32PeakStack,s32TotalMallocBytes);
                fprintf(fp_mhz,"%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t\n",chname,file,\
                        max_clk,min_clk,nframe,max_frame,min_frame,totalClock,outputfilesize,s32PeakStack,s32TotalMallocBytes);

#endif
#endif


#ifndef MEASURE_STACK_USAGE
#ifndef MEASURE_HEAP_USAGE

        printf("%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t\n",chname,file,\
            max_clk,min_clk,nframe,max_frame,min_frame,totalClock,outputfilesize);
        fprintf(fp_mhz,"%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t\n",chname,file,\
            max_clk,min_clk,nframe,max_frame,min_frame,totalClock,outputfilesize);

#endif
#endif

        fclose(fp_mhz);
    }

#endif
#if __WINCE
#ifdef TIME_PROFILE_WINCE
//	printf("\n%s\t%s\t%d\t%d\t%ld\t%ld\t%d\t%d\t%d\t%ld\t%f\t",chname,opt->infileName,(int)(opt->sampRate*opt->nFrameBits/opt->nSamples),opt->nChannels,\
//		prev_DecTimeUs,min_DecTimeUs,frame_no,max_frame,min_frame,TotalDecTimeUs,((float)opt->sampRate/(float)opt->nSamples));

    printf("\n%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t%ld\n",chname,argv[1],\
        prev_DecTimeUs,min_DecTimeUs,frame_no,max_frame,min_frame,TotalDecTimeUs,outputfilesize);
    fprintf(fp_mhz,"\n%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%ld\t%ld",chname,argv[1],
        prev_DecTimeUs,min_DecTimeUs,frame_no,max_frame,min_frame,TotalDecTimeUs,outputfilesize);
	/*printf("Total Decode Time [microseconds] = %ld\n", TotalDecTimeUs);
	printf("Decode Time per Frame [microseconds] = %ld\n", TotalDecTimeUs/nFrames);*/
    fclose(fp_mhz);
#endif
#endif

    /* cleanup */
    OggVorbisCleanup(&sOVDecObj);

    fprintf(fperr,"Done.\n");
    printf("Done.\n"); //fprintf(fperr,"Done.\n");

    fclose(fpin);
#ifdef OUT_DUMP
    fclose(fpout);
#endif
    fclose(fperr);

#ifdef RELOCATION
    free(base);
#else
    free(sOVDecObj.decoderbuf);
#endif
    free(sOVDecObj.pvOggDecObj);
    free(pcmout);
    return ;
}

size_t my_fread(void *buff, size_t unit, size_t size, void *fp)
{
    int bytes_read;
    struct_input_file *input_file=(struct_input_file *)fp;

    bytes_read = unit*size;

    if(bytes_read+input_file->bytes_read<=input_file->file_size)
    {
        memcpy(buff,input_file->buff,bytes_read);
        input_file->bytes_read += bytes_read;
        input_file->buff += bytes_read;
    }
    else if(input_file->file_size-input_file->bytes_read<bytes_read)
    {
        bytes_read = input_file->file_size-input_file->bytes_read;
        memcpy(buff,input_file->buff,bytes_read);
        input_file->bytes_read += bytes_read;
        input_file->buff += bytes_read;
    }
    return bytes_read;
}

int my_fseek(void *fp, ogg_int64_t unit, int size)
{
    int bytes_read = (int)(unit+size);
    struct_input_file *input_file=(struct_input_file *)fp;
    if(bytes_read==1)
    {
        input_file->bytes_read = 0;
        input_file->buff = input_file->buff_head;
    }
    else
    {
        input_file->bytes_read = bytes_read;
        input_file->buff = input_file->buff_head+bytes_read;
    }
    return 0;
}

int my_fclose(void *fp)
{
    struct_input_file *input_file=(struct_input_file *)fp;
    free(input_file->buff_head);
    free(fp);
    return 0;
}

long my_ftell(void *fp)
{
    struct_input_file *input_file=(struct_input_file *)fp;
    return input_file->bytes_read;
}

void Usage()
{
    printf("USAGE:Exename i/p-File o/p-File errFile");
    return;
}
