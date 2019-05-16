
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
 * 21/02/06    01          initial revision                   Sripathi Kamath
 *
 * 24/02/06    02          updated with review comments       Sripathi Kamath
 *
 * 21/03/06    06          added wave read functions          Sripathi Kamath
 *****************************************************************************

 * DESCRIPTION
 *   This file contains the prototypes of the functions used in test_mp3enc.c
 *   and the default values used for mode, input format, sampling frequency
 *   and bitrate.
 *
 ******************************************************************************/

#ifndef MP3ENCODER_H
#define MP3ENCODER_H

/*Set default stereo mode of operation
  'j' - joint stereo mode
  'm' - mono stereo mode
 */
#define DFLT_MOD   'j'


/*Set default sampling frequency;
  possible values are 32000, 44100, 48000
 */
#define DFLT_SFQ   44100


/*Set default bitrate;
  possible values are 32, 40, 48, 56, 64, 80,
  96, 112, 128, 160, 192, 224, 256, 320
 */
#define DFLT_BR    128


/*Set default input format
  'i' - input samples are L/R interleaved
  'l' - input samples are with contiguous L samples,
  followed by contiguous R samples
 */
#define DFLT_INTLV 'i'

/*Set the default configuration
 *  's' - Optimized for speed (Lower quality, lower MIPS)
 *  'q' - Optimized for quality (Best quality,higher MIPS)
 */
#define DFLT_CONFIG 'q'

/* Returns encoding parameters from the specifications of the
 * command line.
 */
#ifdef TWO_INSTANCES
void parse_args(int argc, char** argv, int *mode, int *sfreq, int *bitrate, char *inPath, char *outPath,
                                       int *mode1,int *sfreq1,int *bitrate1,char *inPath1,char *outPath1  );
#else
void parse_args(int argc, char** argv, int *mode, int *sfreq, int *bitrate, char *inPath, char *outPath);
#endif

void encoder_mem_info_alloc(MP3E_Encoder_Config *enc_config);

/* Gives the command line syntax
 */
void usage(char *programName);

/* output mp3 file */
void output_mp3_file(MP3E_Encoder_Config *enc_config,MP3E_INT8 *outbuf);
#ifdef TWO_INSTANCES
void output_mp3_file1(MP3E_Encoder_Config *enc_config1,MP3E_INT8 *outbuf1);
#endif

#endif
