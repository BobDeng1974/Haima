#ifndef QUEUE_H
#define QUEUE_H

#include "types.h"


#define QUEUE_SIZE    6000u



typedef struct cWriteFileInfo{
   const uint8_t*  filename;
   int fd;
   uint pos;
}WriteFileInfo;

typedef struct cOneFrameData{
   uchar data[2048];
}OneFrameData;

typedef struct CQueue{
    uint mQueueCtr;
    OneFrameData *mQueueInPtr;
    OneFrameData *mQueueOutPtr;
    OneFrameData mQueueBuf[QUEUE_SIZE];
}Queue;


extern	void CQueueInit(Queue *const q);
extern	void Clear(Queue *const q);
extern	TBOOL IsEmpty(const Queue *const q);
extern	TBOOL IsFull(const Queue *const q);
extern	TBOOL EnQueue(Queue *const q, const OneFrameData *const inElemPtr);
extern	TBOOL DeQueue(Queue *const q,OneFrameData *const outElemPtr);
#endif
