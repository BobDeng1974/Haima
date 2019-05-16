
 /************************************************************************
  * Copyright 2005-2010 by Freescale Semiconductor, Inc.
  * All modifications are confidential and proprietary information
  * of Freescale Semiconductor, Inc.
  ************************************************************************/

typedef unsigned char U8;
typedef int S32 ;
typedef short S16;
typedef unsigned short U16;
#define        CHNL_TAB_COL_SIZE    2       /* number of columns in chanl table */

#define NBAMR_COMMON_TABLE_START_INDEX 0
#ifndef TARGET_ARM11
#define NBAMR_DECODER_TABLE_START_INDEX 51
#define NBAMR_ENCODER_TABLE_START_INDEX 75
#else
#define NBAMR_DECODER_TABLE_START_INDEX 50
#define NBAMR_ENCODER_TABLE_START_INDEX 74
#endif
/***************************<GLOBAL_VARIABLES BEGIN>***************************/
typedef struct
{
    char *name; /* name string */
    int   id;   /* integer id  */
}sConvTableType;

/* Parameters defining the adaptive codebook search per mode */
typedef struct
{
    S16 s16MaxFracLag;     /* lag up to which fractional lags are used    */
    S16 s16Flag3;          /* enable 1/3 instead of 1/6 fract. resolution */
    S16 s16FirstFrac;      /* first fractional to check                   */
    S16 s16LastFrac;       /* last fractional to check                    */
    S16 s16DeltaIntLow;    /* integer lag below TO to start search from   */
    S16 s16DeltaIntRange;  /* integer range around T0                     */
    S16 s16DeltaFrcLow;    /* fractional below T0                         */
    S16 s16DeltaFrcRange;  /* fractional range around T0                  */
    S16 s16PitMin;         /* minimum pitch                               */
}sModeDepParmType;
/******************************<Table_Index>*******************************/

/******************************************************************************/
extern void table_info(void);
extern S16 tab_size[108];
/***************************<Common Tables >***********************************/

/***************************<GLOBAL_VARIABLES END>*****************************/
