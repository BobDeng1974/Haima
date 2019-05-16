/*
 ***********************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */
/*******************************************************************************
 *
 *
 * *****************************************************************************
 *
 * File name: aac_aux.c
 *
 * Description: Contains functions which the application needs to parse headers.
 *
 *
 * Functions Included: App_bs_look_bits
 *                     App_FindFileType
 *                     App_bs_refill_buffer
 *                     App_bs_readinit
 *                     App_bs_read_bits
 *                     App_bs_byte_align
 *                     App_get_ele_list
 *                     App_get_prog_config
 *                     App_get_adif_header
 *                     App_get_adts_header
 *
 *******************************************************************************
 *
 *   CHANGE HISTORY
 *   dd/mm/yy   Code Ver     Description                        Author
 *   --------   -------      -----------                        ------
 *   19/08/2004       01     Created.                           S.Nachiappan
 *                           Has functions, which the
 *                           application needs to
 *                           parse headers.
 *
 *   02/02/2007       02     Level 4 Warnings removal           rks05c
 *                           Defect ID: TLSbo89610
 ******************************************************************************/
#ifdef  BSAC_DEC
#ifdef  BSAC_DEBUG_DUMP
#include <stdio.h>
#endif
#endif
#include <string.h>
#include "bsacd_dec_interface.h"
//DSPhl28187 Library header files are removed other than the aac_Dec_interface.h file
#ifdef UNIX
//#include <strings.h>
#endif

//tlsbo89610
#if defined(WINCE_WARNLVL4_DISABLE) && defined(__WINCE)
	// disable some level-4 warnings, use #pragma warning(enable:###) to re-enable
	#pragma warning(disable:4100) // warning C4100: unreferenced formal parameter
#endif //#if defined(WINCE_WARNLVL4_DISABLE) && defined(__WINCE)

#define BS_BUF_SIZE AACD_INPUT_BUFFER_SIZE
#define MAX_ENC_BUF_SIZE 400*BS_BUF_SIZE
#define LONG_BOUNDARY 4

extern int App_adif_header_present;
extern int App_adts_header_present;

extern char bitstream_buf[MAX_ENC_BUF_SIZE];
extern int bitstream_buf_index;
extern int bitstream_count;
extern int in_buf_done;
extern int bytes_supplied;

extern int BitsInHeader;

extern unsigned char App_ibs_buf[INTERNAL_BS_BUFSIZE];
extern BitstreamParam App_bs_param;

extern void* aacd_alloc_fast (int );

/***************************************************************
 *  FUNCTION NAME - App_bs_look_bits
 *
 *  DESCRIPTION
 *      Get the required number of bits from the bit-register
 *
 *  ARGUMENTS
 *      nbits - Number of bits required
 *
 *  RETURN VALUE
 *      Required bits
 *
 **************************************************************/
unsigned long App_bs_look_bits (int nbits)
{
    BitstreamParam *b = &(App_bs_param);
    return b->bit_register;
}


/***************************************************************
 *  FUNCTION NAME - App_FindFileType
 *
 *  DESCRIPTION
 *      Find if the file is of type, ADIF or ADTS
 *
 *  ARGUMENTS
 *      val     -   First 4 bytes in the stream
 *
 *  RETURN VALUE
 *      0 - Success
 *     -1 - Error
 *
 **************************************************************/
int App_FindFileType(int val)
{
  if (val == 0x41444946)
    {
      App_adif_header_present = 1;
    }
  else
    {
      if ((val & 0xFFF00000) == 0xFFF00000)
	App_adts_header_present = 1;
      else
	return(-1);
    }
  return(0);
}



/***************************************************************
 *  FUNCTION NAME - App_bs_refill_buffer
 *
 *  DESCRIPTION
 *      Fill the bitstream buffer with new buffer.
 *
 *  ARGUMENTS
 *      None
 *
 *  RETURN VALUE
 *      0  - success
 *      -1 - error
 *
 **************************************************************/

int App_bs_refill_buffer ()
{
    BitstreamParam *b = &(App_bs_param);
    int bytes_to_copy;
    unsigned int len;


    bytes_to_copy = b->bs_end_ext - b->bs_curr_ext;
    if (bytes_to_copy <= 0)
    {
      if (bitstream_count <=0)
	return(-1);
      else
	{
	  len = (bitstream_count > BS_BUF_SIZE) ? BS_BUF_SIZE : bitstream_count;

	  b->bs_curr_ext = (unsigned char *)(bitstream_buf + bitstream_buf_index);           //tlsbo89610
	  b->bs_end_ext = b->bs_curr_ext + len;
	  bytes_to_copy = len;

	  bitstream_buf_index += len;
	  bitstream_count -= len;
	  in_buf_done    += len;
	  bytes_supplied = len;

	  /* Set only if previous Seeking is done */
	  /*
	    if (b->bs_seek_needed == 0)
            b->bs_seek_needed = SeekFlag;
	  */
	}

    }

    if (bytes_to_copy > INTERNAL_BS_BUFSIZE)
      bytes_to_copy = INTERNAL_BS_BUFSIZE;

    b->bs_curr = App_ibs_buf;
    memcpy (b->bs_curr, b->bs_curr_ext, bytes_to_copy);
    b->bs_curr_ext += bytes_to_copy;
    b->bs_end = b->bs_curr + bytes_to_copy;

    return 0;

}


/***********************************************************************
 *
 *   FUNCTION NAME - App_bs_readinit
 *
 *   DESCRIPTION
 *      This module initializes the BitStream Parameteres.
 *
 *   ARGUMENTS
 *       buf       -  Buffer from which, data is to be copied
 *                    into internal-buffer and bit-register
 *       bytes     -  Size of the above buffer in bytes.
 *
 *   RETURN VALUE
 *      None
 **********************************************************************/
void App_bs_readinit(char *buf, int bytes)
{
    BitstreamParam *b = &(App_bs_param);
    unsigned int temp;
    int ret;

    b->bs_curr_ext = (unsigned char *)buf;         //tlsbo89610
    b->bs_end_ext = b->bs_curr_ext + bytes;
    /* internal buffer is not initialized yet */
    b->bs_curr = App_ibs_buf;
    b->bs_end = b->bs_curr;
    b->bs_eof = 0;
    b->bs_seek_needed = 0;

    b->bit_register = 0;
    b->bit_counter = BIT_COUNTER_INIT;

    while (b->bit_counter >= 0)
    {
        if (b->bs_curr >= b->bs_end)
        {
            ret = App_bs_refill_buffer();
            if (ret < 0)
                break;
        }
        temp = *b->bs_curr++;
        b->bit_register = b->bit_register | (temp << b->bit_counter);
        b->bit_counter -= 8;
    }
}

/***************************************************************
 *  FUNCTION NAME - App_bs_read_bits
 *
 *  DESCRIPTION
 *      Reads the given number of bits from the bit-register
 *
 *  ARGUMENTS
 *      nbits - Number of bits required
 *
 *  RETURN VALUE
 *      - Required bits
 *      - -1 in case of end of bitstream
 *
 **************************************************************/
unsigned long App_bs_read_bits (int nbits)
{
    BitstreamParam *b = &(App_bs_param);
    unsigned long temp,temp1,temp_bit_register;
    long temp_bit_counter;
    int ret;

    BitsInHeader += nbits;


    temp_bit_counter = b->bit_counter;
    temp_bit_register = b->bit_register;

    /* If more than available bits are requested,
     * return error
     */
    if ((MIN_REQD_BITS - temp_bit_counter) < nbits)
        return (unsigned long)-1;              //tlsbo89610


    temp = temp_bit_register >> (32 - nbits);
    temp_bit_register <<= nbits;
    temp_bit_counter += nbits;

    while (temp_bit_counter >= 0)
    {
        if (b->bs_curr >= b->bs_end)
        {
            ret = App_bs_refill_buffer();
            if (ret < 0)
            {
                b->bit_register = temp_bit_register;
                b->bit_counter = temp_bit_counter;

                return(temp);
            }
        }

        temp1 = *b->bs_curr++;
        temp_bit_register = temp_bit_register | (temp1 << temp_bit_counter);
        temp_bit_counter -= 8;
     }

     b->bit_register = temp_bit_register;
     b->bit_counter = temp_bit_counter;


    return(temp);
}

/*******************************************************************************
 *
 *   FUNCTION NAME - App_bs_byte_align
 *
 *   DESCRIPTION
 *       This function makes the number of bits in the bit register
 *       to be a multiple of 8.
 *
 *   ARGUMENTS
 *       None
 *
 *   RETURN VALUE
 *       number of bits discarded.
*******************************************************************************/
int App_bs_byte_align()
{
    BitstreamParam *b = &(App_bs_param);
    int             nbits;


    nbits = MIN_REQD_BITS - b->bit_counter;
    nbits = nbits & 0x7;    /* LSB 3 bits */

    BitsInHeader += nbits;


    b->bit_register <<= nbits;
    b->bit_counter += nbits;


    return (nbits);
}


/*******************************************************************************
 *
 *   FUNCTION NAME - App_get_ele_list
 *
 *   DESCRIPTION
 *       Gets a list of elements present in the current data block
 *
 *   ARGUMENTS
 *       p           -  Pointer to array of Elements.
 *       enable_cpe  _  Flag to indicate whether channel paired element is
 *                      present.
 *
 *   RETURN VALUE
 *       None
*******************************************************************************/
static void App_get_ele_list(AACD_EleList * p, int enable_cpe)
{
    int             i,
                    j;

    for (i = 0, j = p->num_ele; i < j; i++)
    {
        if (enable_cpe)
            p->ele_is_cpe[i] = App_bs_read_bits(LEN_ELE_IS_CPE);
        else
            p->ele_is_cpe[i] = 0;
        p->ele_tag[i] = App_bs_read_bits(LEN_TAG);
    }
}



/*******************************************************************************
 *
 *   FUNCTION NAME - App_get_prog_config
 *
 *   DESCRIPTION
 *       Read the program configuration element from the data block
 *
 *   ARGUMENTS
 *       p           -  Pointer to a structure to store the new program
 *                       configuration.
 *
 *   RETURN VALUE
 *         Success :  0
 *
 *         Error   : -1
 *
*******************************************************************************/
int App_get_prog_config(AACD_ProgConfig * p)
{
    int             i,
                    j;


    p->tag = App_bs_read_bits(LEN_TAG);

    p->profile = App_bs_read_bits(LEN_PROFILE);
    if (p->profile != 1)
    {
        return -1;
    }
    p->sampling_rate_idx = App_bs_read_bits(LEN_SAMP_IDX);
    if (p->sampling_rate_idx >= 0xc)
    {
        return -1;
    }
    p->front.num_ele = App_bs_read_bits(LEN_NUM_ELE);
    if (p->front.num_ele > FCHANS)
    {
        return -1;
    }
    p->side.num_ele = App_bs_read_bits(LEN_NUM_ELE);
    if (p->side.num_ele > SCHANS)
    {
        return -1;
    }
    p->back.num_ele = App_bs_read_bits(LEN_NUM_ELE);
    if (p->back.num_ele > BCHANS)
    {
        return -1;
    }
    p->lfe.num_ele = App_bs_read_bits(LEN_NUM_LFE);
    if (p->lfe.num_ele > LCHANS)
    {
        return -1;
    }
    p->data.num_ele = App_bs_read_bits(LEN_NUM_DAT);
    p->coupling.num_ele = App_bs_read_bits(LEN_NUM_CCE);
    if (p->coupling.num_ele > CCHANS)
    {
        return -1;
    }
    if ((p->mono_mix.present = App_bs_read_bits(LEN_MIX_PRES)) == 1)
        p->mono_mix.ele_tag = App_bs_read_bits(LEN_TAG);
    if ((p->stereo_mix.present = App_bs_read_bits(LEN_MIX_PRES)) == 1)
        p->stereo_mix.ele_tag = App_bs_read_bits(LEN_TAG);
    if ((p->matrix_mix.present = App_bs_read_bits(LEN_MIX_PRES)) == 1)
    {
        p->matrix_mix.ele_tag = App_bs_read_bits(LEN_MMIX_IDX);
        p->matrix_mix.pseudo_enab = App_bs_read_bits(LEN_PSUR_ENAB);
    }
    App_get_ele_list(&p->front, 1);
    App_get_ele_list(&p->side, 1);
    App_get_ele_list(&p->back, 1);
    App_get_ele_list(&p->lfe, 0);
    App_get_ele_list(&p->data, 0);
    App_get_ele_list(&p->coupling, 1);

    App_bs_byte_align();

    j = App_bs_read_bits(LEN_COMMENT_BYTES);


    /*
     * The comment bytes are overwritten onto the same location, to
     * save memory.
     */

    for (i = 0; i < j; i++)
        p->comments[0] = (AACD_INT8)App_bs_read_bits(LEN_BYTE);
    /* null terminator for string */
    p->comments[0] = 0;

    return 0;

}

/*******************************************************************************
 *
 *   FUNCTION NAME - App_get_mp4forbsac_header
 *
 *   DESCRIPTION
 *       Gets mp4forbsac header from the input bitstream.
 *
 *   ARGUMENTS
 *         params  -  place to store the mp4forbsac-header data
 *
 *   RETURN VALUE
 *         Success :  1
 *         Error   : -1
*******************************************************************************/
int App_get_mp4forbsac_header(AACD_Block_Params * params)
{

    int byte1,byte2,byte3,byte4;

    byte1 = App_bs_read_bits(8);
    byte2 = App_bs_read_bits(8);
    byte3 = App_bs_read_bits(8);
    byte4 = App_bs_read_bits(8);
    params->scalOutObjectType = byte1 + (byte2 <<8) + (byte3 <<16) + (byte4 <<24);

    byte1 = App_bs_read_bits(8);
    byte2 = App_bs_read_bits(8);
    byte3 = App_bs_read_bits(8);
    byte4 = App_bs_read_bits(8);
    params->scalOutNumChannels = byte1 + (byte2 <<8) + (byte3 <<16) + (byte4 <<24);

    byte1 = App_bs_read_bits(8);
    byte2 = App_bs_read_bits(8);
    byte3 = App_bs_read_bits(8);
    byte4 = App_bs_read_bits(8);
    params->sampleRate = byte1 + (byte2 <<8) + (byte3 <<16) + (byte4 <<24);


    if(params->sampleRate == 96000) params->SamplingFreqIndex = 0;
    else if( params->sampleRate == 88200) params->SamplingFreqIndex = 1;
    else if( params->sampleRate == 64000) params->SamplingFreqIndex = 2;
    else if( params->sampleRate == 48000) params->SamplingFreqIndex = 3;
    else if( params->sampleRate == 44100) params->SamplingFreqIndex = 4;
    else if( params->sampleRate == 32000) params->SamplingFreqIndex = 5;
    else if( params->sampleRate == 24000) params->SamplingFreqIndex = 6;
    else if( params->sampleRate == 22050) params->SamplingFreqIndex = 7;
    else if( params->sampleRate == 16000) params->SamplingFreqIndex = 8;
    else if( params->sampleRate == 12000) params->SamplingFreqIndex = 9;
    else if( params->sampleRate == 11025) params->SamplingFreqIndex = 10;
    else if( params->sampleRate ==  8000) params->SamplingFreqIndex = 11;
    else if( params->sampleRate ==  7350) params->SamplingFreqIndex = 12;
    else params->SamplingFreqIndex = 13;




    byte1 = App_bs_read_bits(8);
    byte2 = App_bs_read_bits(8);
    byte3 = App_bs_read_bits(8);
    byte4 = App_bs_read_bits(8);
    params->framelengthflag = byte1 + (byte2 <<8) + (byte3 <<16) + (byte4 <<24);


    return 1;
}

/*******************************************************************************
 *
 *   FUNCTION NAME - App_dump_bsac_init
 *
 *   DESCRIPTION
 *       Gets mp4forbsac header from the input bitstream.
 *
 *   ARGUMENTS
 *         index   -  index to which dump point
 *		      1 = " dump point after quantizition"
 *		      2 = " dump point after ms stereo "
 *                    3 = " dump point after pns "
 *                    4 = " dump point after intensity "
 *                    5 = " dump point after TNS "
 *                    6 = " dump point after IMDCT "
 *         pbuf    -  buffer pointer
 *         length  -  buffer length
 *
 *   RETURN VALUE
 *         Success :  1
 *         Error   : -1
*******************************************************************************/
#ifdef  BSAC_DEC
#ifdef  BSAC_DEBUG_DUMP
int App_dump_bsac_init(void)
{

    FILE *fileptr1;

    fileptr1 = fopen("data_bsac_1_0.txt", "w");    fclose(fileptr1);
    fileptr1 = fopen("data_bsac_2_0.txt", "w");    fclose(fileptr1);
    fileptr1 = fopen("data_bsac_3_0.txt", "w");    fclose(fileptr1);
    fileptr1 = fopen("data_bsac_4_0.txt", "w");    fclose(fileptr1);
    fileptr1 = fopen("data_bsac_5_0.txt", "w");    fclose(fileptr1);
    fileptr1 = fopen("data_bsac_6_0.txt", "w");    fclose(fileptr1);

    return 1;
}
#endif
#endif

/*******************************************************************************
 *
 *   FUNCTION NAME - App_dump_bsac_data
 *
 *   DESCRIPTION
 *       Gets mp4forbsac header from the input bitstream.
 *
 *   ARGUMENTS
 *         index   -  index to which dump point
 *		      1 = " dump point after quantizition"
 *		      2 = " dump point after ms stereo "
 *                    3 = " dump point after pns "
 *                    4 = " dump point after intensity "
 *                    5 = " dump point after TNS "
 *                    6 = " dump point after IMDCT "
 *         pbuf    -  buffer pointer
 *         length  -  buffer length
 *
 *   RETURN VALUE
 *         Success :  1
 *         Error   : -1
*******************************************************************************/
#ifdef  BSAC_DEC
#ifdef  BSAC_DEBUG_DUMP
void App_dump_bsac_data(int index, int* pbuf, int length)
{

    FILE *fileptr1;
    int i;
    int quant_scale = 1;

    if(index == 1)   fileptr1 = fopen("data_bsac_1_0.txt", "a");
    else if(index == 2)   fileptr1 = fopen("data_bsac_2_0.txt", "a");
    else if(index == 3)   fileptr1 = fopen("data_bsac_3_0.txt", "a");
    else if(index == 4)   fileptr1 = fopen("data_bsac_4_0.txt", "a");
    else if(index == 5)   fileptr1 = fopen("data_bsac_5_0.txt", "a");
    else if(index == 6)   fileptr1 = fopen("data_bsac_6_0.txt", "a");
    else return ;

    for( i=0; i<length; i++)
    {
	fprintf(fileptr1,"%08x\n", (int)(pbuf[i] * quant_scale));

    }

    fclose(fileptr1);

}
#endif
#endif
/*******************************************************************************
 *
 *   FUNCTION NAME - App_get_adif_header
 *
 *   DESCRIPTION
 *       Gets ADIF header from the input bitstream.
 *
 *   ARGUMENTS
 *         params  -  place to store the adif-header data
 *
 *   RETURN VALUE
 *         Success :  1
 *         Error   : -1
*******************************************************************************/
int App_get_adif_header(AACD_Block_Params * params)
{
    int             i,
                    n,
                    select_status;
    AACD_ProgConfig      *tmp_config;
    ADIF_Header     temp_adif_header;
    ADIF_Header*    p = &(temp_adif_header);

    /* adif header */
    for (i = 0; i < LEN_ADIF_ID; i++)
        p->adif_id[i] = (char) App_bs_read_bits(LEN_BYTE);
    p->adif_id[i] = 0;  /* null terminated string */

#ifdef UNIX
    /* test for id */
    if (strncmp(p->adif_id, "ADIF", 4) != 0)
        return -1;  /* bad id */
#else
    /* test for id */
    if (*((unsigned long *) p->adif_id) != *((unsigned long *) "ADIF"))
        return -1;  /* bad id */
#endif

    /* copyright string */
    if ((p->copy_id_present = App_bs_read_bits(LEN_COPYRT_PRES)) == 1)
    {
        for (i = 0; i < LEN_COPYRT_ID; i++)
            p->copy_id[i] = (char)App_bs_read_bits(LEN_BYTE);

        /* null terminated string */
        p->copy_id[i] = 0;
    }
    p->original_copy = App_bs_read_bits(LEN_ORIG);
    p->home = App_bs_read_bits(LEN_HOME);
    p->bitstream_type = App_bs_read_bits(LEN_BS_TYPE);
    p->bitrate = App_bs_read_bits(LEN_BIT_RATE);

    /* program config elements */
    select_status = -1;
    n = App_bs_read_bits(LEN_NUM_PCE) + 1;

    tmp_config = (AACD_ProgConfig*) aacd_alloc_fast (n*sizeof(AACD_ProgConfig));

    for (i = 0; i < n; i++)
    {
        tmp_config[i].buffer_fullness =
            (p->bitstream_type == 0) ? App_bs_read_bits(LEN_ADIF_BF) : 0;


        if (App_get_prog_config(&tmp_config[i]))
        {
            return -1;
        }

	select_status = 1;
    }

    App_bs_byte_align();

    /* Fill in the AACD_Block_Params struct now */

    params->num_pce = n;
    params->pce     = tmp_config;
    params->BitstreamType = p->bitstream_type;
    params->BitRate       = p->bitrate;
    params->ProtectionAbsent = 0;

    return select_status;
}



/*******************************************************************************
 *
 *   FUNCTION NAME - App_get_adts_header
 *
 *   DESCRIPTION
 *       Searches and syncs to ADTS header from the input bitstream. It also
 *       gets the full ADTS header once sync is obtained.
 *
 *   ARGUMENTS
 *       params   - Place to store the header data
 *
 *   RETURN VALUE
 *       Success : 0
 *         Error : -1
*******************************************************************************/
int App_get_adts_header(AACD_Block_Params * params)
{
    ADTS_Header App_adts_header;
    ADTS_Header *p = &(App_adts_header);

    int bits_used = 0;

#ifdef OLD_FORMAT_ADTS_HEADER
    int emphasis;
#endif


	//while (1)          //tlsbo89610
	for(;;)
	{
        /*
         * If we have used up more than maximum possible frame for finding
         * the ADTS header, then something is wrong, so exit.
         */
        if (bits_used > (LEN_BYTE * ADTS_FRAME_MAX_SIZE))
            return -1;

        /* ADTS header is assumed to be byte aligned */
        bits_used += App_bs_byte_align();

#ifdef CRC_CHECK
        /* Add header bits to CRC check */
        UpdateCRCStructBegin();
#endif

        p->syncword = App_bs_read_bits ((LEN_SYNCWORD - LEN_BYTE));
        bits_used += LEN_SYNCWORD - LEN_BYTE;

        /* Search for syncword */
        while (p->syncword != ((1 << LEN_SYNCWORD) - 1))
        {
            p->syncword = ((p->syncword << LEN_BYTE) | App_bs_read_bits(LEN_BYTE));
            p->syncword &= (1 << LEN_SYNCWORD) - 1;
            bits_used += LEN_BYTE;
            /*
             * If we have used up more than maximum possible frame for finding
             * the ADTS header, then something is wrong, so exit.
             */
            if (bits_used > (LEN_BYTE * ADTS_FRAME_MAX_SIZE))
                return -1;
	}

	p->id = App_bs_read_bits(LEN_ID);
	bits_used += LEN_ID;

	/*
	  Disabled version check in allow MPEG2/4 streams
	  0 - MPEG4
	  1 - MPEG2
	  if (!p->id)
	  {
	  continue;
	  }
	*/

	p->layer = App_bs_read_bits(LEN_LAYER);
	bits_used += LEN_LAYER;
	if (p->layer != 0)
	  {
	    continue;
	  }

	p->protection_abs = App_bs_read_bits(LEN_PROTECT_ABS);
	bits_used += LEN_PROTECT_ABS;

	p->profile = App_bs_read_bits(LEN_PROFILE);
        bits_used += LEN_PROFILE;
	if (p->profile != 1)
	  {
            continue;
	  }

	p->sampling_freq_idx = App_bs_read_bits(LEN_SAMP_IDX);
        bits_used += LEN_SAMP_IDX;
	if (p->sampling_freq_idx >= 0xc)
	  {
            continue;
	  }

	p->private_bit = App_bs_read_bits(LEN_PRIVTE_BIT);
        bits_used += LEN_PRIVTE_BIT;

        //temp_channel_config = p->channel_config;
	p->channel_config = App_bs_read_bits(LEN_CHANNEL_CONF);
        bits_used += LEN_CHANNEL_CONF;
        ///* Audio mode has changed, so config has to be built up again */
        //if (temp_channel_config != p->channel_config)
	//ptr->AACD_default_config = 1;

	p->original_copy = App_bs_read_bits(LEN_ORIG);
        bits_used += LEN_ORIG;

	p->home = App_bs_read_bits(LEN_HOME);
        bits_used += LEN_HOME;

#ifdef OLD_FORMAT_ADTS_HEADER
	params->Flush_LEN_EMPHASIS_Bits = 0;
	if (p->id == 0)
	  {
	    emphasis = App_bs_read_bits(LEN_EMPHASIS);
	    bits_used += LEN_EMPHASIS;
	    params->Flush_LEN_EMPHASIS_Bits = 1;
	  }

#endif

	p->copyright_id_bit = App_bs_read_bits(LEN_COPYRT_ID_ADTS);
        bits_used += LEN_COPYRT_ID_ADTS;

	p->copyright_id_start = App_bs_read_bits(LEN_COPYRT_START);
        bits_used += LEN_COPYRT_START;

	p->frame_length = App_bs_read_bits(LEN_FRAME_LEN);
        bits_used += LEN_FRAME_LEN;

	p->adts_buffer_fullness = App_bs_read_bits(LEN_ADTS_BUF_FULLNESS);
        bits_used += LEN_ADTS_BUF_FULLNESS;

	p->num_of_rdb = App_bs_read_bits(LEN_NUM_OF_RDB);
        bits_used += LEN_NUM_OF_RDB;

   /*
     Removed, constraint: num_of_rdb == 0 because we can support more than
     one raw data block in 1 adts frame
     if (p->num_of_rdb != 0)
     {
     continue;
     }
   */

        /*
         * If this point is reached, then the ADTS header has been found and
         * CRC structure can be updated.
         */
#ifdef CRC_CHECK
        /*
         * Finish adding header bits to CRC check. All bits to be CRC
         * protected.
         */
        UpdateCRCStructEnd(0);
#endif

        if (p->protection_abs == 0)
	  {
	    p->crc_check = App_bs_read_bits(LEN_CRC);
            bits_used += LEN_CRC;
	  }

        /* Full header successfully obtained, so get out of the search */
        break;
   }

#ifndef OLD_FORMAT_ADTS_HEADER
    bits_used += App_bs_byte_align();
#else
    if (p->id != 0) // MPEG-2 style : Emphasis field is absent
      {
	bits_used += App_bs_byte_align();
      }
    else //MPEG-4 style : Emphasis field is present; cancel its effect
      {
	BitsInHeader -= LEN_EMPHASIS;
      }
#endif

    /* Fill in the AACD_Block_Params struct now */

    params->num_pce = 0;
    params->ChannelConfig = p->channel_config;
    params->SamplingFreqIndex = p->sampling_freq_idx;
    params->BitstreamType = (p->adts_buffer_fullness == 0x7ff) ? 1 : 0;



    params->BitRate       = 0; /* Caution !*/

    /* The ADTS stream contains the value of buffer fullness in units
       of 32-bit words. But the application is supposed to deliver this
       in units of bits. Hence we do a left-shift */

    params->BufferFullness = (p->adts_buffer_fullness) << 5;
    params->ProtectionAbsent = p->protection_abs;
    params->CrcCheck = p->crc_check;
    params->frame_length = p->frame_length-7;


    //ptr->AACD_mc_info.profile = p->profile;
    //ptr->AACD_mc_info.sampling_rate_idx = p->sampling_freq_idx;

    //AACD_infoinit(&(ptr->tbl_ptr_AACD_samp_rate_info[ptr->AACD_mc_info.sampling_rate_idx]), ptr);

    return 0;

}

/* End of file */

