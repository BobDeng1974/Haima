

/****************************************************************************
 *
 * (C) 2004 MOTOROLA INDIA ELECTRONICS LTD.
 *
 *   CHANGE HISTORY
 *   dd/mm/yy   Code Ver    Description                         Author
 *   --------   -------     -----------                         ------
 *   08/12/04   01          Created to test suspension          B.Venkatarao
 *
 ****************************************************************************/
 	 /************************************************************************
	  * Copyright 2005-2010 by Freescale Semiconductor, Inc.
	  * All modifications are confidential and proprietary information
	  * of Freescale Semiconductor, Inc. 
	  ************************************************************************/
#ifdef CHECK_SUSPENSION
enum
{
    SUSP_TYPE_NONE = 0,
    SUSP_TYPE_GET_FILE_INFO,        // Once in get_file_info (from get_new_data())
    SUSP_TYPE_QUERY_MEM,            // Once in query_mem (from get_new_data())
    SUSP_TYPE_INIT,                 // Once in decoder_init (from get_new_data())
    SUSP_TYPE_DEC_ONCE,             // Once in decode_mcu_row (from get_new_data())
    SUSP_TYPE_DEC_RANDOM,           // Randomly in decode_mcu_row (from get_new_data())
    SUSP_TYPE_DEC_EVERY_MCU,        // After every MCU
    SUSP_TYPE_DEC_MARKER,           /* Every marker (restart marker, all other markers after
                                        1st scan */
    SUSP_TYPE_DEC_MARKER_BODY,      /* Every marker body (all markers after
                                        1st scan */
    SUSP_TYPE_DEC_FIRST_LOOP,       // Suspend in the first-loop (MCU row loop, in jpeg_read_image_data)
    SUSP_TYPE_DEC_SECOND_LOOP,      // Suspend in the second-loop (loop to skip last few MCUs in case of scaling, in jpeg_read_image_data)
    SUSP_TYPE_DEC_BOTH_LOOPS,       // Suspend in the first and second loop
};

enum
{
    SUSP_STATE_NONE,        // Can not suspend
    SUSP_STATE_START,       // Can suspend now
    SUSP_STATE_SUSPENDED,   // Suspended
    SUSP_STATE_END,         // End of Suspension
};

extern int susp_type;
extern int susp_state;
extern int susp_flag;
extern int susp_eof;
extern JPEGD_UINT32 susp_target_bytes;
#endif
