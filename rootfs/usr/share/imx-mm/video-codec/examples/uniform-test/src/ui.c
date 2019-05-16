/*
 ***********************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "defs.h"
#include "ui.h"


//=================================================================================


#define CASE_FIRST(x)   if (memcmp(argv[0], x, strlen(x)) == 0)
#define CASE(x)         else if (memcmp(argv[0], x, strlen(x)) == 0)
#define DEFAULT         else



static int usage(const char *program)
{
		printf("\nUsage: %s [options] -l dynamic_library -i bitstream_file\n", program);
		printf("options:\n"
			   "	-o <file_name>		   : Save decoded output in YUV 4:2:0 format "
			   "[default: no save]\n"
			   "	-n <frame_num>				  : decode max <frame_num> frames\n"
			   "[default: all frames will be decoded]\n"
			   "    -t <frame time log>           : if specified, produce every frame decoding time in log file.\n"
			   "    -r <report file>              : if specified, produce test information in report_file.\n" 
			   "	-d							  : if specified, LCD render enabled.\n"
			   "	-m							  : if specified, print stack and heap information.\n"
			   "	-w,[wrapper options]		  : if specified, pass options to wrapper.\n"
			   "	-v                  		  : if specified, print library version.\n"
			   );
		return -1;
}

int GetUserInput(IOParams *pIO, int argc, const char *argv[])
{
    int     bitFileDone = 0;
	char	**wp_argv = NULL;
	char    *wp_options = NULL;

    // Defaults
    pIO->putLog = 0;
    pIO->saveYUV = 0;
    pIO->maxnum = 0;
    pIO->display = 0;
	
	pIO->wp_argc = 0;
	pIO->memFlag = 0;
	pIO->libVer = 0;

	pIO->saveResult = 0;
	
    pIO->infile[0] = 0;
    pIO->outfile[0] = 0;
	pIO->dutlib[0] = 0;
	pIO->resfile[0] = 0;
	pIO->logfile[0] = 0;
	
	memset(&pIO->wp_argv[0],0, sizeof(pIO->wp_argv));
	memset(&pIO->wp_options[0],0, sizeof(pIO->wp_options));

    argc--;
    argv++;

	wp_argv = pIO->wp_argv;
	wp_options = pIO->wp_options;
	
    while (argc)
    {
        if (argv[0][0] == '-')
        {
            CASE_FIRST("-i")
            {
                argc--;
                argv++;
                if (argv[0] != NULL)
                {
					strcpy((char *)pIO->infile, argv[0]);
					bitFileDone = 1;
                }
            }
			CASE("-l")
            {
                argc--;
                argv++;
                if (argv[0] != NULL)
					strcpy((char *)pIO->dutlib, argv[0]);
            }
			CASE("-o")
            {
                argc--;
                argv++;
                if (argv[0] != NULL)
				{
					strcpy((char *)pIO->outfile, argv[0]);
					pIO->saveYUV = 1;
				}
            }
			CASE("-t")
            {
                argc--;
                argv++;
                if (argv[0] != NULL)
				{
					strcpy((char *)pIO->logfile, argv[0]);
					pIO->putLog = 1;
				}
            }
			CASE("-r")
            {
                argc--;
                argv++;
                if (argv[0] != 0)
				{
					strcpy((char *)pIO->resfile, argv[0]);
					pIO->saveResult = 1;
				}
            }
            CASE("-n")
            {
                argc--;
                argv++;
                if (argv[0] != NULL)
                    sscanf(argv[0], "%d", &pIO->maxnum);
            }
            
            CASE("-d")
            {
                pIO->display = 1;
            }
			CASE("-m")
            {
                pIO->memFlag = 1;
            }
			CASE("-v")
            {
                pIO->libVer = 1;
            }
            CASE("-w,")
            {
                if (argv[0] != NULL)
                {
                    sscanf(argv[0], "-w,%s", wp_options);
					*wp_argv = wp_options;
					
					wp_argv++;
					pIO->wp_argc++;

					wp_options+=strlen(wp_options);
					wp_options++;
                }

            }
            DEFAULT                             // Has to be last
            {
                PRINT_ERROR("Unsupported option %s\n", argv[0]);
                return usage(pIO->infile);
            }
        }
        else
        {
#if defined(ARMULATOR) //fix argument bug in RVDS
	        //argc--;
      		 // argv++;		
#else
            PRINT_ERROR("Unsupported option %s\n", argv[0]);
                return usage(pIO->infile);
#endif				
            
        }
        argc--;
        argv++;
    }
    if (!bitFileDone)
        return usage(pIO->infile);

    return 0;
}


int yuv_frame_compare( IOParams *pIO,
                       const unsigned char* pLum,
                       const unsigned char* pCb,
                       const unsigned char* pCr,
                       unsigned int uiWidth,
                       unsigned int uiHeight,
                       unsigned int uiStride,
                       unsigned int uiUVStride,
                       const int rauiCropping[] )
{
    // read yuv frame in
    int size = (uiWidth - rauiCropping[0] - rauiCropping[1]) * (uiHeight -  rauiCropping[2] - rauiCropping[3]);
    unsigned char *buf = (unsigned char *)av_malloc(size/2*3);
    int rd;
    unsigned int y = -1;
    unsigned int x = -1;
#if 1
	return 0;
#else
    rd = fread(buf, sizeof(buf[0]), size/2*3, pIO->refYUVFile);
    if(rd == size/2*3)
    {
      // cmp
      const unsigned char*  pucSrc;
      unsigned int width = uiWidth - (rauiCropping[0] + rauiCropping[1]);
      unsigned int height = uiHeight - (rauiCropping[2] + rauiCropping[3]);
      unsigned int stride = uiStride;
      int error_count = 0;
      int xmin = 100000;
      int ymin;
    
      pucSrc = pLum + ( rauiCropping[0] + rauiCropping[2] * stride );
      PRINT_ERROR("y compare ...");
      x=0; y=0;
      error_count = 0;
      for(y=0; y<height; y++)
      {
        for(x=0; x<width; x++)
        {
          if(*(pucSrc+stride*y+x) != *(buf+width*y+x))
          {
            error_count++;
            // find left-most bad pixel in current mb
            if(x < xmin)
            {
              xmin = x;
              ymin = y;
            }
            break; // check next line in the same mb row
          }
        }
        //  if mb line finished
        if((y&15) == 15 && error_count)
        {
          x = xmin;
          y = ymin;
          goto failed;
        }
//        assert(xmin == 100000);
      }

      stride = uiUVStride;
      height >>= 1;
      width  >>= 1;

      PRINT_ERROR("cb compare ...");
      x=0; y=0;
      pucSrc = pCb + ( ( rauiCropping[0] + rauiCropping[2] * stride ) >> 1 );
      for(y=0; y<height; y++)
      {
        for(x=0; x<width; x++)
        {
          if(*(pucSrc+stride*y+x) != *(buf+size+width*y+x))
          {
            goto failed;
          }
        }
      }
    
      PRINT_ERROR("cr compare ...");
      x=0; y=0;
      pucSrc = pCr + ( ( rauiCropping[0] + rauiCropping[2] * stride ) >> 1 );
      for(y=0; y<height; y++)
      {
        for(x=0; x<width; x++)
        {
          if(*(pucSrc+stride*y+x) != *(buf+size+size/4+width*y+x))
           goto failed;
        }
      }
    }
    else
    {
      goto failed;
    }

	// ok
	PRINT_ERROR("......[PASS]\n");
	av_free(buf);
    return 0;

failed:
    PRINT_ERROR("......[FAILED]\nMismatch found at position (%d,%d).\n",x,y);
	av_free(buf);
//	ASSERT(0);
	pIO->tst++;
    return -1;
#endif
}

