/*
 ***********************************************************************
 * Copyright 2005-2008 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

/*!
 ***********************************************************************
 * Header file of picture management
 *
 *
 * History
 *   Date          Changed                                Changed by
 *   Sep. 5, 2007  Create                                  Zhenyong Chen
 *   Sep. 17, 2007 Add lock for buffer allocation          Zhenyong Chen
 *   Oct. 10, 2007 Add InvalidPicture					   Zhenyong Chen
 *   Oct. 18, 2007 Modify API to support WinCE             Zhenyong Chen
 *   Oct. 19, 2007 Add APIs to lock/unlock a buffer        Zhenyong Chen
 ***********************************************************************
 */


#ifndef __PICTUREMANAGEMENT_H__
#define __PICTUREMANAGEMENT_H__

typedef struct tagPICTUREENTRY
{
    BYTE *y;
    BYTE *cb;
    BYTE *cr;
    int  nlock; // If nlock > 0, then this buffer cannot be overwritten (it is used),
                // regardless poc.
    int  poc; // ID of this picture
}PICTUREENTRY;

#define MAX_PICTURES 4

class CPictureManagement  
{
private:
    PICTUREENTRY pictures[MAX_PICTURES];
    BOOL m_bInternAllocated; // Whether pictures are created internally

public:
    CPictureManagement();
    virtual ~CPictureManagement();

    //@ Lock picture with specified y buffer, prevent overwriting
    BOOL LockPicture(BYTE *pY);
    //@ Unlock picture with specified y buffer, allow overwriting
    BOOL UnLockPicture(BYTE *pY);

    //@ Buffers are allocated by outside, and pass them to picture management
    //@ Conflict with CreateAllPictures
    BOOL SetBuffers(BYTE **y, BYTE **u, BYTE **v, int count);

    //@ Create all pictures internally
    //@ Conflict with SetBuffers
    int CreateAllPictures(int ysize, int csize);

    //@ Destroy all pictures
    void DestroyAllPictures();

    //@ Find a picture with ID "poc"
    PICTUREENTRY *Find(int poc);

    //@ Allocate a picture (if no, delete oldest picture)
    PICTUREENTRY *AllocPicture(int min_is_oldest);

    //@ Invalidate a picture (content is not reusable)
    void InvalidPicture(int poc);
};

#endif /* __PICTUREMANAGEMENT_H__ */

