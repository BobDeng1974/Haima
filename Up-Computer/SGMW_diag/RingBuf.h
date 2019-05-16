#pragma once
#pragma warning(disable:4430)

#define	RINGBUF_ERR				0	//ringbuf建立失败
#define	RINGBUF_OK				1	//ringbuf建立成功
#define	RINGBUF_MUTEX_EXIST		2	//ringbuf互斥量已经建立
#define RINGBUF_MALLOC_FAILED	3	//ringbuf内存申请失败
#define RINGBUF_MUTEX_ERR		4	//ringbuf互斥量错误
#define RINGBUF_NULL			5	//ringbuf中无数据

class RingBuf
{
public:
	RingBuf(void);
	~RingBuf(void);

	int		create_ringbuf(int id,unsigned int block_num,unsigned int block_size);
	int		delete_ringbuf(void);
	int		get_ringbuf(unsigned char* p_buf);
	int		put_ringbuf(unsigned char* p_buf);

	HANDLE			h_mutex;

	unsigned int	flag;
	unsigned char	*p_ringbuf;
	unsigned int	read_index;
	unsigned int	write_index;
	unsigned int	max_block_num;
	unsigned int	max_block_size;

};

