/*
 ***********************************************************************
 * Copyright 2005-2008 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*!
 ***********************************************************************
 * Implementation file of picture management
 *
 *
 * History
 *   Date          Changed                                Changed by
 *   Sep. 5, 2007  Create                                 Zhenyong Chen
 *   Sep. 17, 2007 Add lock for buffer allocation         Zhenyong Chen
 *   Oct. 10, 2007 Add InvalidPicture					  Zhenyong Chen
 *   Oct. 18, 2007 Modify API to support WinCE            Zhenyong Chen
 *   Oct. 19, 2007 Add APIs to lock/unlock a buffer       Zhenyong Chen
 ***********************************************************************
 */


#include "common.h"
#include "PictureManagement.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPictureManagement::CPictureManagement()
{
    int i;
    for(i=0; i<MAX_PICTURES; i++)
    {
        pictures[i].y = pictures[i].cb = pictures[i].cr = NULL;
        pictures[i].poc = -1;
        pictures[i].nlock = 0;
    }
    m_bInternAllocated = FALSE;
}

CPictureManagement::~CPictureManagement()
{
    DestroyAllPictures();
}
void CPictureManagement::DestroyAllPictures()
{
    int i;
    for(i=0; i<MAX_PICTURES; i++)
    {
        if(m_bInternAllocated)
        {
        if(pictures[i].y)
            delete pictures[i].y;
        if(pictures[i].cb)
            delete pictures[i].cb;
        if(pictures[i].cr)
            delete pictures[i].cr;
        }
        pictures[i].y = pictures[i].cb = pictures[i].cr = NULL;
        pictures[i].poc = -1;
        pictures[i].nlock = 0;
    }

    m_bInternAllocated = FALSE;
}
int CPictureManagement::CreateAllPictures(int ysize, int csize)
{
    int i;
    // Check valid
    if(ysize > 2*1024*1024 || csize > ysize)
        return -2;

    m_bInternAllocated = TRUE;

    for(i=0; i<MAX_PICTURES; i++)
    {
        ASSERT(pictures[i].poc == -1);
        ASSERT(pictures[i].nlock == 0);
        ASSERT(pictures[i].y == NULL);
        ASSERT(pictures[i].cb == NULL);
        ASSERT(pictures[i].cr == NULL);
        pictures[i].y = new BYTE [ysize];
        pictures[i].cb = new BYTE [csize];
        pictures[i].cr = new BYTE [csize];
        if(pictures[i].y == NULL || pictures[i].cb == NULL || pictures[i].cr == NULL)
        {
            goto alloc_failed;
        }
    }
    return 0;
alloc_failed:
    DestroyAllPictures();
    return -1;
}
PICTUREENTRY *CPictureManagement::Find(int poc)
{
    int i;
    if(poc < 0)
        return NULL;
    for(i=0; i<MAX_PICTURES; i++)
    {
        if(pictures[i].poc == poc)
            return &pictures[i];
    }
    return NULL;
}
PICTUREENTRY *CPictureManagement::AllocPicture(int min_is_oldest)
{
    PICTUREENTRY *pic = NULL;
    // Search empty or oldest
    int min = 65536;
    int max = -1;
    int i_min = -1;
    int i_max = -1;
    int i;
    for(i=0; i<MAX_PICTURES; i++)
    {
        if(pictures[i].nlock)
            continue;
        if(pictures[i].poc == -1)
        {
            pic = &pictures[i];
            goto find_pic;
        }
        if(pictures[i].poc < min)
        {
            min = pictures[i].poc;
            i_min = i;
        }
        if(pictures[i].poc > max)
        {
            max = pictures[i].poc;
            i_max = i;
        }
    }
    ASSERT(i_min != -1 && i_max != -1);
    if(min_is_oldest)
    {
        pic = &pictures[i_min];
    }
    else
    {
        pic = &pictures[i_max];
    }
find_pic:
    pic->poc = -1; // Mark unused
    return pic;
}
void CPictureManagement::InvalidPicture(int poc)
{
    int i;
    if(poc < 0)
        return;
    for(i=0; i<MAX_PICTURES; i++)
    {
        if(pictures[i].poc == poc)
        {
            pictures[i].poc = -1;
            return;
        }
    }
}


BOOL CPictureManagement::SetBuffers(BYTE **y, BYTE **u, BYTE **v, int count)
{
    if(count != MAX_PICTURES && m_bInternAllocated)
        return FALSE;
    int i;
    for(i=0; i<MAX_PICTURES; i++)
    {
        pictures[i].poc = -1;
        pictures[i].nlock = 0;
        pictures[i].y = y[i];
        pictures[i].cb = u[i];
        pictures[i].cr = v[i];
    }

    return TRUE;
}

BOOL CPictureManagement::LockPicture(BYTE *pY)
{
    int i;
    if(pY == NULL)
        return TRUE;

    for(i=0; i<MAX_PICTURES; i++)
    {
        if(pictures[i].y == pY)
        {
            pictures[i].nlock++;
            return TRUE;
        }
    }

    return FALSE;// Not found
}

BOOL CPictureManagement::UnLockPicture(BYTE *pY)
{
    int i;
    if(pY == NULL)
        return TRUE;

    for(i=0; i<MAX_PICTURES; i++)
    {
        if(pictures[i].y == pY)
        {
            pictures[i].nlock--;
            return TRUE;
        }
    }
    return FALSE;// Not found
}

