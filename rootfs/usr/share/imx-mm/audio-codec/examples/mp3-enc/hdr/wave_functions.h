
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
 * 02/06/08    02          Add support of my fread
 *                         to bypass the file rw
 *                         issues on arm12 platform           Lionel Xu
 * 10/07/08    03          Add flag ENABLE_MY_FREAD           Lionel Xu
 ****************************************************************************

 * DESCRIPTION
 *   This file contains the prototypes of functions defined in
 *   wave_functions.c
 *
 ******************************************************************************/

#ifndef WAVEFUNCTIONS_H
#define WAVEFUNCTIONS_H

/* Parses the input file header and verifies whether it is a
 * wave file
 */

#ifdef ENABLE_MY_FREAD

size_t my_fread(void *, size_t, size_t, void *);
int    my_fseek(void *, long long, int);
int    my_fclose(void *);
long   my_ftell(void *);

//For input buffer
typedef struct
{
	long file_size;
	int bytes_read;
	char *buff;
	char *buff_head;
}struct_input_data;

#endif


#ifdef ENABLE_MY_FREAD
void my_afopen(int *, int *, void *);

/* Function to read the pcm samples from a wave file
 */
int my_afread(short int *, int, int, void *);
#ifdef TWO_INSTANCES
int my_afread1(short int *, int, int, void *);
#endif

#else
FILE *afopen(char *, int *, int *);

/* Function to read the pcm samples from a wave file
 */
int afread(short int *, int, int, FILE *);
#ifdef TWO_INSTANCES
int afread1(short int *, int, int, FILE *);
#endif

#endif

#endif
