
/************************* MPEG-2 NBC Audio Decoder **************************
 *                                                                           *
"This software module was originally developed by
AT&T, Dolby Laboratories, Fraunhofer Gesellschaft IIS
and edited by
Yoshiaki Oikawa (Sony Corporation),
Mitsuyuki Hatanaka (Sony Corporation)
Mike Coleman (Five Bats Research)
in the course of development of the MPEG-2 NBC/MPEG-4 Audio standard ISO/IEC 13818-7,
14496-1,2 and 3. This software module is an implementation of a part of one or more
MPEG-2 NBC/MPEG-4 Audio tools as specified by the MPEG-2 NBC/MPEG-4
Audio standard. ISO/IEC  gives users of the MPEG-2 NBC/MPEG-4 Audio
standards free license to this software module or modifications thereof for use in
hardware or software products claiming conformance to the MPEG-2 NBC/MPEG-4
Audio  standards. Those intending to use this software module in hardware or
software products are advised that this use may infringe existing patents.
The original developer of this software module and his/her company, the subsequent
editors and their companies, and ISO/IEC have no liability for use of this software
module or modifications thereof in an implementation. Copyright is not released for
non MPEG-2 NBC/MPEG-4 Audio conforming products.The original developer
retains full right to use the code for his/her  own purpose, assign or donate the
code to a third party and to inhibit third party from using the code for non
MPEG-2 NBC/MPEG-4 Audio conforming products. This copyright notice must
be included in all copies or derivative works."
Copyright(c)1996,1997.
 *                                                                           *
 ****************************************************************************/
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
 *   dd/mm/yy   Code Ver     Description                        Author
 *   --------   -------      -----------                        ------
 *   08/03/2000  0.01        Added this template                Nanda
 *                                                              Kishore A.S
 *   08/03/2000  0.02        Integration changes                Nanda
 *                                                              Kishore A.S
 *   14/04/2000  0.03        Clean up, error codes              Nanda
 *                                                              Kishore A.S
 *   03/05/2000  0.04        Memory design changes              B.Venkatarao
 *   18/07/2003    05        New window-ola design              B.Venkatarao
 *   16/04/2004    06        Relocatability changes             Vishala
 *   08/07/2004    07        Modified proto-type of
 *                           AACD_writeout                      S.Nachiappan
 *   08/07/2004    08        Output-file suffix is set          S.Nachiappan
 *                           to ".hex" if TARGET_FILEIO is
 *                           defined. (for ARM ADS)
 *   23/07/2004    09        Modifications for 24 bit output    Pradeep V
 *   19/08/2004    10        Removed, all stream-parsing        S.Nachiappan
 *                           calls, since these will be done
 *                           outside of the decoder.
 *   02/02/2007    11        Level 4 Warnings removal           rks05c
 *                           Defect ID: TLSbo89610
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bsacd_dec_interface.h"
//DSPhl28187 Library header files are removed other than the aac_Dec_interface.h file

//tlsbo89610
#if defined(WINCE_WARNLVL4_DISABLE) && defined(__WINCE)
	// disable some level-4 warnings, use #pragma warning(enable:###) to re-enable
	#pragma warning(disable:4100) // warning C4100: unreferenced formal parameter
#endif //#if defined(WINCE_WARNLVL4_DISABLE) && defined(__WINCE)


#define PATH_LEN    128

FILE *outfd[CHANS];

//DSPhl28187
void AACD_closefiles(void);
int AACD_open_output_files(AACD_Decoder_Config *dec_config, char *outpath);

/*******************************************************************************
 *
 *   FUNCTION NAME - write_fext
 *
 *   DESCRIPTION
 *       This function forms appropriate file extensions depending on the
 *       output channel type.
 *
 *   ARGUMENTS
 *       cip - Channel info
 *       c   - Output channel type
 *       n   - Output channel number
 *       ptr - Global structure pointer
 *
 *   RETURN VALUE
 *       None.
 *
 ******************************************************************************/
static void write_fext(AACD_Ch_Info * cip, char c, int n)	//DSPhl28187
{
    sprintf((cip->fext), "_%c%02d", c, n);
}

/*******************************************************************************
 *
 *   FUNCTION NAME - AACD_open_output_files
 *
 *   DESCRIPTION
 *       This function opens output PCM files depending on the type of output
 *       channel.
 *
 *   ARGUMENTS
 *
 *   RETURN VALUE
 *       Success : 0
 *        Error  : -1
 ******************************************************************************/
int AACD_open_output_files(AACD_Decoder_Config *dec_config, char *outpath)		 //DSPhl28187
{
    int             i,
                    j;
    char            file[PATH_LEN];

#ifdef USE_AIFF
    if (ptr->AACD_useAIFF)
    {
        ptr->AACD_AIFF_MC_Info = mip;
        ptr->AACD_AIFF_sampleRate = ptr->tbl_ptr_AACD_samp_rate_info[mip->sampling_rate_idx].samp_rate;
    }
#endif
#ifdef DUMP_CODE_REF
    /* Open only one file */
    {
        if (mip->ch_info[0].file_opened != 1)
        {
           // strcpy(file, output_path);
            strcpy(file, outpath);
            strcat(file, ".pcm");
            outfd[0] = creat(file, 0666);

            if (outfd[0] < 0)
            {
	      //myexit(AAC_ERROR_FILEIO, ptr);
	      return(-1);
            }
            else
            {
                mip->ch_info[0].file_opened = 1;
            }
        }
    }
#else

    j = 0;
    /* if FCenter == 1 (center speaker present)
     *	position    index   extension
     *	left	    1	    01
     *	center	    0	    00
     *	right	    2	    02
     * if FCenter == 0 (center speaker absent)
     *	position    index   extension
     *	left	    0	    01
     *	right	    1	    02
     */
    for (i = ((FCenter==1) ? 0 : 1); i < FCHANS; i++)
    {
        write_fext(&dec_config->ch_info[j], 'f', i);	//DSPhl28187
        j++;
    }
	for (i = 0; i < SCHANS; i++)
    {
        write_fext(&dec_config->ch_info[j], 's', i);		//DSPhl28187
        j++;
    }
    /* if BFCenter == 1 (center speaker present) position    index
     * extension left       3       00 center       5       02 right 4 01
     * if BCenter == 0 (center speaker absent) position index extension
     * left     3       00 right        4       01 */
    for (i = 0; i < BCHANS; i++)
    {
        write_fext(&dec_config->ch_info[j], 'b', i);		//DSPhl28187
        j++;
    }
    for (i = 0; i < LCHANS; i++)
    {
        write_fext(&dec_config->ch_info[j], 'l', i);		//DSPhl28187
        j++;
    }

    for (i = 0; i < CHANS; i++)
    {
	    if (!(*(dec_config->ch_info[i].present)) || (*(dec_config->ch_info[i].file_opened) == 1))	//DSPhl28187
            continue;

     //   strcpy(file, output_path);
        strcpy (file, outpath);
        strcat(file,(dec_config->ch_info[i].fext));		//DSPhl28187

#ifdef USE_AIFF
        if (!(ptr->AACD_useAIFF))
#endif

            strcat(file, ".hex");

#ifdef USE_AIFF
        else
            strcat(file, ".aif");
#endif

        outfd[i] = fopen(file, "w");
        if (outfd[i] == NULL)
        {
	  //myexit(AAC_ERROR_FILEIO, ptr);
	  return(-1);
        }
        else
        {
	  //printf("Opening file for channel %d\n", i);
            *(dec_config->ch_info[i].file_opened) = 1;		//DSPhl28187
        }

#ifdef USE_AIFF
        if (ptr->AACD_useAIFF)
        {
            /* write dummy header and position for audio data */
            if (AACD_WriteAIFFHeader(outfd[i], ptr->AACD_AIFF_channels, ptr->AACD_AIFF_frames, ptr->AACD_AIFF_wordsize, ptr->AACD_AIFF_sampleRate) != 0)
            {
	      //myexit(AAC_ERROR_FILEIO, ptr);
	      return(-1);
            }
        }
#endif
    }
#endif
    return (0);
}


/*******************************************************************************
 *
 *   FUNCTION NAME - AACD_writeout
 *
 *   DESCRIPTION
 *       This function is used to write the output data to a file. 16 or 32 bit
 *       output is written.
 *
 *   ARGUMENTS
 *       data - Pointer to the output data for all channels
 *
 *   RETURN VALUE
 *       Success : 0
 *        Error  : -1
 ******************************************************************************/
//DSPhl28187
int AACD_writeout(AACD_OutputFmtType data[][AAC_FRAME_SIZE], AACD_Decoder_Config *dec_config,
                   int* ShouldOpen, int* ShouldClose,char *outpath)
{
    int             i;
    int             size,
                    num_chans;
#ifdef DUMP_CODE_REF
    int             j;
#endif

    if (AACD_open_output_files(dec_config, outpath)!=0)		//DSPhl28187
      return(-1);

    num_chans = CHANS;

    if (*ShouldClose == 1)
    {
        for (i=0; i < num_chans; i++)
        {
            if (*(dec_config->ch_info[i].present))		//DSPhl28187
            {
                fclose(outfd[i]);
            }
         }
         *ShouldClose = 0;
         return(0);
     }

    size = LN2*sizeof(short);

    for (i = 0; i < num_chans; i++)
    {
        int j;
        if (!(*(dec_config->ch_info[i].present)))		//DSPhl28187
            continue;
        #ifdef __SYMBIAN32__
        fwrite(&(data[i][0]),4,size/2,outfd[i]);
        #else
        for (j = 0; j < size/2; j++)
        {
           fprintf(outfd[i], "%08x\n", (AACD_OutputFmtType)data[i][j]);
        }
        #endif
    }

#ifdef USE_AIFF
    if (ptr->AACD_useAIFF)
        ptr->AACD_AIFF_frames += LN2;
#endif
    return(0);
}


/*******************************************************************************
 *
 *   FUNCTION NAME - myexit
 *
 *   DESCRIPTION
 *       This function is used to perform an exit from the decoder, returning
 *       a proper error code.
 *
 *   ARGUMENTS
 *       errornum - A number denoting the error which occurred
 *
 *   RETURN VALUE
 *       None.
 *
 ******************************************************************************/
void myexit(int errornum) //DSPhl28187
{
#ifdef USE_AIFF
    if (ptr->AACD_useAIFF)
        AACD_closefiles(ptr->AACD_AIFF_MC_Info, ptr);
#endif
    exit(errornum);
}


/*******************************************************************************
 *
 *   FUNCTION NAME - AACD_closefiles
 *
 *   DESCRIPTION
 *       This function closes opened AIFF files.
 *
 *   ARGUMENTS
 *		 None
 *
 *   RETURN VALUE
 *       None.
 *
 ******************************************************************************/

//DSPhl28187
void AACD_closefiles(void)
{

#ifdef USE_AIFF
    int             i;

    if (mip && ptr->AACD_useAIFF)
    {
        for (i = 0; i < CHANS; i++)
        {
            if (!(mip->ch_info[i].present))
                continue;
            AACD_WriteAIFFHeader(outfd[i], ptr->AACD_AIFF_channels, ptr->AACD_AIFF_frames, ptr->AACD_AIFF_wordsize, ptr->AACD_AIFF_sampleRate);
            close(outfd[i]);
        }
    }
#endif
}

/* End of file */
