/*
 ***********************************************************************
 * Copyright 2005-2008 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*!
 ***********************************************************************
 * Header file of deinterlace abstract layer
 *
 *
 * History
 *   Date          Changed                                Changed by
 *   Sep. 5, 2007  Create                                 Zhenyong Chen
 *   Sep. 14, 2007 Add member dynamic_params to DEINTER   Zhenyong Chen
 *   Oct. 10, 2007 Modify Deinterlace                     Zhenyong Chen
 ***********************************************************************
 */


#ifndef __DEINTERLACE_H__
#define __DEINTERLACE_H__

#include "Deinterlace_types.h"

/*
 * Method infomation related APIs
 */
/* \Function
 *   InitDeinterlace
 * \Brief
 *   Init deinterlace algorithms, and get information of implemented 
 *   algorithms
 * \Return value
 *   None
 * \Parameters
 *   None
 * \See also
 *   InitDeinterlaceSafe, InitDeinterlaceUnSafe
 */
void InitDeinterlace(void);

/* \Function
 *   GetMethodCount
 * \Brief
 *   Get total methods implemented 
 * \Return value
 *   Count of methods
 * \Parameters
 *   None
 * \See also
 *   N/A
 */
int GetMethodCount(void);

/* \Function
 *   GetMethodName
 * \Brief
 *   Get name of the deinterlacing method
 * \Return value
 *   Method name
 * \Parameters
 *   method  [in] which name is required
 * \Remark
 *   The returned name string is readonly (const), don't try modify it
 * \See also
 *   N/A
 */
const char * GetMethodName(unsigned int method);

/* \Function
 *   MethodFromPosition
 * \Brief
 *   Get method from position
 * \Return value
 *   Method ID
 * \Parameters
 *   position  [in] where the method is located. Range from 0 to GetMethodCount()
 * \See also
 *   GetMethodCount
 */
unsigned int MethodFromPosition(int position);

/* \Function
 *   GetMethodPosition
 * \Brief
 *   Get method position
 * \Return value
 *   Method position
 * \Parameters
 *   method  [in] method which position is required
 * \See also
 *   MethodFromPosition
 */
int GetMethodPosition(unsigned int method);

/* \Function
 *   IsMethodNeedPrevFrame
 * \Brief
 *   Check whether previous frame need loading when deinterlacing current
 *   frame. Useful for memory bandwidth decrease.
 * \Return value
 *   TRUE if this method need previous frame loading; FALSE if needn't.
 * \Parameters
 *   method  [in] method for query
 * \See also
 *   IsMethodNeedNextFrame
 */
BOOL IsMethodNeedPrevFrame(unsigned int method);

/* \Function
 *   IsMethodNeedNextFrame
 * \Brief
 *   Check whether next frame need loading when deinterlacing current
 *   frame. Useful for memory bandwidth decrease.
 * \Return value
 *   TRUE if this method need next frame loading; FALSE if needn't.
 * \Parameters
 *   method  [in] method for query
 * \See also
 *   IsMethodNeedPrevFrame
 */
BOOL IsMethodNeedNextFrame(unsigned int method);


/* \Function
 *   Deinterlace
 * \Brief
 *   Deinterlace an interlace frame, and output a  progressive frame in 
 *   same place as current frame
 * \Return value
 *   Whether current frame can be reused for other frame deinterlacing
 *      0 -- current frame can be reusable (can save memory bandwidth for reverse play)
 *      1 -- current frame will not be reusable
 * \Parameters
 *   pDeinterInfo  [inout] See definition of DEINTER
 * \See also
 *   Document ...
 */
int Deinterlace(DEINTER *pDeinterInfo);

/* \Function
 *   GetMotionBlockTrackFlag
 * \Brief
 *   Check status of motion-block-tracking mode
 * \Return value
 *   Status of motion block tracking
 *      0 -- no motion block tracking
 *      1 -- has motion block tracking
 * \Parameters
 *   None
 * \Remark
 *   For algorithm research use
 * \See also
 *   SetMotionBlockTrackFlag
 */
int GetMotionBlockTrackFlag(void);

/* \Function
 *   SetMotionBlockTrackFlag
 * \Brief
 *   Set flag of motion-block-tracking mode
 * \Return value
 *   Previous status of motion block tracking
 * \Parameters
 *   newval   [in] flag to enable/disable motion-block-tracking mode.
 *                 0 -- disable motion block tracking
 *                 1 -- enable motion block tracking
 * \Remark
 *   For algorithm research use
 * \See also
 *   GetMotionBlockTrackFlag
 */
int SetMotionBlockTrackFlag(int newval);


#endif /* __DEINTERLACE_H__ */

