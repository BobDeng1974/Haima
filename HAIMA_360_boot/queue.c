#include "queue.h"

void CQueueInit(Queue *const q)
{
    q->mQueueCtr = 0;        
	q->mQueueInPtr = &q->mQueueBuf[0];
	q->mQueueOutPtr = &q->mQueueBuf[0];
}


void Clear(Queue *const q)
{
	q->mQueueCtr = 0;
	q->mQueueInPtr = &q->mQueueBuf[0];
	q->mQueueOutPtr = &q->mQueueBuf[0];
}
    
    
	
uint GetElemNum(const Queue *const q)
{
	return (q->mQueueCtr);
}

TBOOL IsEmpty(const Queue *const q)
{

    if (0u < q->mQueueCtr)
	{
		return (bFALSE);
	}
	else
	{
		return (bTRUE);
	}
}
    
	
TBOOL IsFull(const Queue *const q)
{
      
	if (QUEUE_SIZE > q->mQueueCtr)
	{
		return (bFALSE);
	}
	else
	{
		return (bTRUE);
	}
}
    
/*--------------------------------------------------------------------------
*  Function:
*  CQueue.EnQueue2
* 
*  Parameters:
*  TType *- ָ��ָ���Ԫ��
* 
*  Returns value:
*  TBOOL -���TType *ָ��ָ���Ԫ�ؽ����гɹ����򷵻�bTRUE,���򷵻�bFALSE.
* 
*  Description:
*  ��ָ����Ԫ�ؽ����У�����ɹ�����bTRUE,ʧ�ܷ���bFALSE,��������������ȫ�ģ�
*  ֻ�����ж���ʹ�ã����ߵ�����ʹ�á�
* 
*--------------------------------------------------------------------------*/
TBOOL EnQueue(Queue *const q, const OneFrameData *const inElemPtr)
{
    if(NULL == inElemPtr)
	{
		return (bFALSE);
	}
    else
    {

    }
    
	if (q->mQueueCtr < QUEUE_SIZE)
	{
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
		return (bTRUE);
	}
    else
    {

    }
	return (bFALSE);
}



TBOOL GetHead(const Queue *const q, OneFrameData *const outElemPtr)
{
	if(NULL == outElemPtr)
	{
		return (bFALSE);
	}
    else
    {

    }
    
	if (0u == q->mQueueCtr)
	{
		return (bFALSE);
	}
	else
	{
		*outElemPtr = *q->mQueueOutPtr;  
	}
    
	return (bTRUE);
}





TBOOL DeQueue(Queue *const q,OneFrameData *const outElemPtr)
{
    
	if(NULL == outElemPtr)
	{
		return (bFALSE);
	}
    else
    {

    }
    
	if (0u == q->mQueueCtr)
	{
		return (bFALSE);
	}
	else
	{
		q->mQueueCtr--;
		*outElemPtr = *q->mQueueOutPtr;
		q->mQueueOutPtr++;
		if (q->mQueueOutPtr == &q->mQueueBuf[QUEUE_SIZE])
		{
			q->mQueueOutPtr = q->mQueueBuf;
		}          
        else
        {

        }
	}
    
	return (bTRUE);
}
