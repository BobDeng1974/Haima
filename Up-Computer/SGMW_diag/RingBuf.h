#pragma once
#pragma warning(disable:4430)

#define	RINGBUF_ERR				0	//ringbuf����ʧ��
#define	RINGBUF_OK				1	//ringbuf�����ɹ�
#define	RINGBUF_MUTEX_EXIST		2	//ringbuf�������Ѿ�����
#define RINGBUF_MALLOC_FAILED	3	//ringbuf�ڴ�����ʧ��
#define RINGBUF_MUTEX_ERR		4	//ringbuf����������
#define RINGBUF_NULL			5	//ringbuf��������

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

