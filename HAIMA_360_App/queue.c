#include "queue.h"



void CQueueInit(Queue *const q)
{
    q->mQueueCtr = 0;        
	q->mQueueInPtr = &q->mQueueBuf[0];
	q->mQueueOutPtr = &q->mQueueBuf[0];
	pthread_mutex_init(&q->f_lock,NULL);
}


void Clear(Queue *const q)
{
	q->mQueueCtr = 0;
	q->mQueueInPtr = &q->mQueueBuf[0];
	q->mQueueOutPtr = &q->mQueueBuf[0];
	pthread_mutex_init(&q->f_lock,NULL);
}
    
    
	
uint GetElemNum(const Queue *const q)
{
	return (q->mQueueCtr);
}

BOOL IsEmpty(const Queue *const q)
{

    if (0u < q->mQueueCtr)
	{
		return (FALSE);
	}
	else
	{
		return (TRUE);
	}
}
    
	
BOOL IsFull(const Queue *const q)
{
      
	if (QUEUE_SIZE > q->mQueueCtr)
	{
		return (FALSE);
	}
	else
	{
		return (TRUE);
	}
}
    
/*--------------------------------------------------------------------------
*  Function:
*  CQueue.EnQueue2
* 
*  Parameters:
*  TType *- 指针指向的元素
* 
*  Returns value:
*  TBOOL -如果TType *指针指向的元素进队列成功，则返回bTRUE,否则返回bFALSE.
* 
*  Description:
*  将指定的元素进队列，如果成功返回bTRUE,失败返回bFALSE,本函数不是任务安全的，
*  只能在中断中使用，或者单任务使用。
* 
*--------------------------------------------------------------------------*/
BOOL EnQueue(Queue *const q, const Msg *const inElemPtr)
{
	
    if(NULL == inElemPtr)
	{
		return (FALSE);
	}
    else
    {
		
    }
    
	if (q->mQueueCtr < QUEUE_SIZE)
	{
		pthread_mutex_lock(&q->f_lock);
		q->mQueueCtr++;
		*q->mQueueInPtr = *inElemPtr;
		q->mQueueInPtr++;
		if (q->mQueueInPtr == &q->mQueueBuf[QUEUE_SIZE])
		{
			q->mQueueInPtr = q->mQueueBuf;
		}
        else
        {

        }
		pthread_mutex_unlock(&q->f_lock);
		return (TRUE);
	}
    else
    {
		
    }
	return (FALSE);
}



BOOL GetHead(const Queue *const q, Msg *const outElemPtr)
{
	if(NULL == outElemPtr)
	{
		return (FALSE);
	}
    else
    {

    }
    
	if (0u == q->mQueueCtr)
	{
		return (FALSE);
	}
	else
	{
		*outElemPtr = *q->mQueueOutPtr;  
	}
    
	return (TRUE);
}





BOOL DeQueue(Queue *const q,Msg *const outElemPtr)
{
    
	if(NULL == outElemPtr)
	{
		return (FALSE);
	}
    else
    {

    }
    
	if (0u == q->mQueueCtr)
	{
		return (FALSE);
	}
	else
	{	
		pthread_mutex_lock(&q->f_lock);
		q->mQueueCtr--;
		*outElemPtr = *q->mQueueOutPtr;  /*结构体赋值 浅拷贝赋值*/
		q->mQueueOutPtr++;
		if (q->mQueueOutPtr == &q->mQueueBuf[QUEUE_SIZE])
		{
			q->mQueueOutPtr = q->mQueueBuf;
		}          
        else
        {

        }
		pthread_mutex_unlock(&q->f_lock);
	}
    
	return (TRUE);
}
