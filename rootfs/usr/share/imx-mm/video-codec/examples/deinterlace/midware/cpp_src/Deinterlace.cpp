/*
 ***********************************************************************
 * Copyright 2005-2008,2011 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*!
 ***********************************************************************
 * Implementation file of deinterlace abstract layer
 *
 *
 * History
 *   Date          Changed                                Changed by
 *   Sep. 05, 2007 Create                                 Zhenyong Chen
 *   Sep. 10, 2007 Build Bob, Weave, VT Median, VT+Bob,   Zhenyong Chen
 *                 VT Avg, Bidirectional prediction
 *   Sep. 12, 2007 Bug fixing - macro MEDIAN wrongly      Zhenyong Chen
 *                 coded
 *   Sep. 13, 2007 Add Block-based VT                     Zhenyong Chen
 *   Sep. 14, 2007 Add control parameter to support       Zhenyong Chen
 *                 user direct interfering interpolating
 *                 process. This is useful to find the
 *                 best parameter setting.
 *   Sep. 17, 2007 Conbine VT Median and Block VT         Zhenyong Chen
 *   Sep. 28, 2007 New vertical filter (-1, 5, 5, -1) to  Zhenyong Chen
 *                 replace Bob in new block VT
 *   Sep. 30, 2007 Format of input to deinterlace: frame  Zhenyong Chen
 *   Oct. 10, 2007 Modify Deinterlace                     Zhenyong Chen
 ***********************************************************************
 */


#include "common.h"
#include "Deinterlace.h"

#include "Deinterlace_Safe.h"
#ifdef FULL_RELEASE
#include "Deinterlace_UnSafe.h"
#endif

/*
 * Deinterlace methods
 */
static DEINTMETHOD methods_info[32];
static int method_count = 0;
int GetMethodCount(void)
{
    return method_count;
}
BOOL IsMethodNeedPrevFrame(unsigned int method)
{
    int methodIndex = GetMethodPosition(method);
    if(methodIndex == -1)
        return FALSE;
    return methods_info[methodIndex].need_prev_frame;
}
BOOL IsMethodNeedNextFrame(unsigned int method)
{
    int methodIndex = GetMethodPosition(method);
    if(methodIndex == -1)
        return FALSE;
    return methods_info[methodIndex].need_next_frame;
}
static BOOL IsMethodSafe(unsigned int method)
{
    int methodIndex = GetMethodPosition(method);
    if(methodIndex == -1)
        return FALSE;
    return methods_info[methodIndex].safe;
}
const char * GetMethodName(unsigned int method)
{
    int methodIndex = GetMethodPosition(method);
    if(methodIndex == -1)
        return NULL;
    return methods_info[methodIndex].name;
}
unsigned int MethodFromPosition(int position)
{
    if(position == -1)
        return (unsigned int)-1;
    return methods_info[position].method;
}
int GetMethodPosition(unsigned int method)
{
    int i;
    for(i=0; i<method_count; i++)
    {
        if(methods_info[i].method == method)
            return i;
    }
    return -1;
}
void InitDeinterlace(void)
{
    int i;
    for(i=0; i<32; i++)
    {
        methods_info[i].method = (unsigned int)-1;
    }
    method_count = 0;
    // Register implemented methods

    // Bob
    methods_info[method_count].method = DEINTMETHOD_BOB;
    strcpy(methods_info[method_count].name, "Bob");
    methods_info[method_count].need_prev_frame = 0;
    methods_info[method_count].need_next_frame = 0;
    methods_info[method_count].safe = 1;
    method_count++;

    // Weave
    methods_info[method_count].method = DEINTMETHOD_WEAVE;
    strcpy(methods_info[method_count].name, "Weave");
    methods_info[method_count].need_prev_frame = 0;
    methods_info[method_count].need_next_frame = 0;
    methods_info[method_count].safe = 1;
    method_count++;

    int count;
    InitDeinterlaceSafe(&methods_info[method_count], &count);
    method_count += count;
#ifdef FULL_RELEASE
    InitDeinterlaceUnsafe(&methods_info[method_count], &count);
    method_count += count;
#endif
}


static int distinguish_block = 0;
int GetMotionBlockTrackFlag(void)
{
    return distinguish_block;
}
int SetMotionBlockTrackFlag(int newval)
{
    int old = distinguish_block;
    distinguish_block = newval;
    return old;
}

// Deinterlacing a frame
int Deinterlace(DEINTER *pDeinterInfo)
{
    int method = pDeinterInfo->method;
    int nFrameReUsable;

    if(method == DEINTMETHOD_WEAVE) // Do nothing
        return 0;


    // In-frame deinterlacing -
    // Following methods won't be affected
    // Bob, Weave, Repeat, VT_Median, Bob_Weave
    switch(method)
    {
    case DEINTMETHOD_BOB:
    case DEINTMETHOD_WEAVE:
        nFrameReUsable = 1;
        break;
    default:
        nFrameReUsable = IsFrameReusableSafe(method, distinguish_block);
        if(nFrameReUsable == -1)
        {
#ifdef FULL_RELEASE
            nFrameReUsable = IsFrameReusableUnsafe(method, distinguish_block);
#endif
        }
        if(nFrameReUsable == -1)
            nFrameReUsable = 0;
        break;
    }

    // Parameter check
    if(IsMethodNeedPrevFrame(method) && pDeinterInfo->frame[0].y == NULL)
        method = DEINTMETHOD_BOB;
    if(IsMethodNeedNextFrame(method) && pDeinterInfo->frame[2].y == NULL)
        method = DEINTMETHOD_BOB;

    DYNAMIC dnTmp;
    if(! pDeinterInfo->dynamic_params)
    {
        dnTmp.nParam1 = -1;
        dnTmp.nParam2 = -1;
        dnTmp.nParam3 = -1;
        pDeinterInfo->dynamic_params = &dnTmp;
    }
    pDeinterInfo->dynamic_params->bTrackMotionBlock = distinguish_block;

    // deinterlace begin ...
    pDeinterInfo->method = method;
    if(IsMethodSafe(method))
        DeinterlaceSafe(pDeinterInfo);
#ifdef FULL_RELEASE
    else
        DeinterlaceUnsafe(pDeinterInfo);
#endif
    // deinterlace end.

    return nFrameReUsable;
}

