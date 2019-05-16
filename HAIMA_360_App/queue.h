#ifndef QUEUE_H
#define QUEUE_H
#include <pthread.h>

#include "types.h"


#define QUEUE_SIZE    1000u
#define COM_DATA_SIZE  128u  /*单帧最大长度*/

typedef struct cRecMsg{
	uint Len;
	char data[COM_DATA_SIZE];
}RecMsg;


typedef struct cMsg{
	uint MsgType;
    uint MsgData;
	uint MsgData2;
	RecMsg SerialData;
}Msg;




typedef struct CQueue{
    uint mQueueCtr;
    Msg *mQueueInPtr;
    Msg *mQueueOutPtr;
    Msg mQueueBuf[QUEUE_SIZE];
	pthread_mutex_t f_lock;
}Queue;


extern	void CQueueInit(Queue *const q);
extern	void Clear(Queue *const q);
extern	BOOL IsEmpty(const Queue *const q);
extern	BOOL IsFull(const Queue *const q);
extern	BOOL EnQueue(Queue *const q, const Msg *const inElemPtr);
extern	BOOL DeQueue(Queue *const q,Msg *const outElemPtr);
#endif
