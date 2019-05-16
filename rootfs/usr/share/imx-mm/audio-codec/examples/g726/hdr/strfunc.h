
 /************************************************************************
  * Copyright 2005-2010 by Freescale Semiconductor, Inc.
  * All modifications are confidential and proprietary information
  * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
  ************************************************************************/

/****************************** Change History********************************
 *
 *   DD/MMM/YYYY     Code Ver     Description      Author
 *   -----------     --------     -----------      ------
 *   22/05/2008      engr77332    Avoid Warning    Qiu Cunshou
 *****************************************************************************/

#ifndef STRFUNC_H
#define STRFUNC_H

#if defined CYCLE_MEASUREMENT || defined TIME_PROFILE_RVDS
extern int mystrcmp(const char*, const char*);
#endif

#endif
