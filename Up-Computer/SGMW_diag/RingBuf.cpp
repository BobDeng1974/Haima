#include "StdAfx.h"
#include "RingBuf.h"


RingBuf::RingBuf(void)
{
	flag			= 0;
	read_index		= 0;;
	write_index		= 0;
	max_block_num	= 0;
	max_block_size	= 0;
	p_ringbuf		= NULL;
	h_mutex			= NULL;
}


RingBuf::~RingBuf(void)
{
	if(p_ringbuf )
		delete[] p_ringbuf;
	if( h_mutex )
	{
		CloseHandle(h_mutex);
		h_mutex = NULL;
	}
}

RingBuf::delete_ringbuf(void)
{
	flag      = 0;
	if( p_ringbuf == NULL )
		return RINGBUF_ERR;
	if( h_mutex == NULL )
		return RINGBUF_MUTEX_ERR;

	delete[] p_ringbuf;
	CloseHandle(h_mutex);
	h_mutex   = NULL;
	p_ringbuf = NULL;
	flag      = 0;
	return RINGBUF_OK;
}

RingBuf::create_ringbuf(int id, unsigned int block_num,unsigned int block_size)
{
	CString str;
	str.Format("ringbuf%02d",id);
	h_mutex = CreateMutex(NULL,FALSE,str);
	if( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		CloseHandle(h_mutex);
		h_mutex = NULL;
		return RINGBUF_MUTEX_EXIST;
	}
	p_ringbuf = new unsigned char[block_num*block_size];
	if( p_ringbuf == NULL )
		return RINGBUF_MALLOC_FAILED;
	max_block_num   = block_num;
	max_block_size  = block_size;
	flag			= 1;
	return RINGBUF_OK;
}

RingBuf::get_ringbuf(unsigned char* p_buf)
{
	if( p_ringbuf == NULL )
		return RINGBUF_ERR;
	if( h_mutex == NULL )
		return RINGBUF_MUTEX_ERR;

	WaitForSingleObject(h_mutex,INFINITE); //获取信号量
	if( write_index != read_index )
	{
		memcpy(p_buf,&p_ringbuf[read_index * max_block_size],max_block_size);
		read_index++;
		if( read_index >= max_block_num )
			read_index = 0;
	}
	else
	{
		ReleaseMutex(h_mutex); //释放信号量
		return RINGBUF_NULL;
	}
	ReleaseMutex(h_mutex); //释放信号量
	return RINGBUF_OK;
}

RingBuf::put_ringbuf(unsigned char* p_buf)
{
	if( p_ringbuf == NULL )
		return RINGBUF_ERR;
	if( h_mutex == NULL )
		return RINGBUF_MUTEX_ERR;

	WaitForSingleObject(h_mutex,INFINITE); //获取信号量
	memcpy(p_ringbuf + write_index * max_block_size, p_buf, max_block_size);
	write_index++;
	if( write_index >= max_block_num )
		write_index = 0;
	ReleaseMutex(h_mutex); //释放信号量
	return RINGBUF_OK;
}
