#pragma once

#include "ECanVci.h"
#include "RingBuf.h"
#pragma warning(disable:4430)
#include <map>
#include "afxcmn.h"
using namespace std;

#define	CAN_UNCONNECT							0	//δ����״̬
#define	CAN_CONNECT_OK							1	//��������
#define CAN_CONNECT_CHECK_CODE_ERR				2	//���������
#define CAN_CONNECT_SHEILD_CODE_FORMAT_ERR		3	//�������ʽ����
#define	CAN_CONNECT_TIMER0_FORMAT_ERR			4	//��ʱ��0��ʽ����
#define	CAN_CONNECT_TIMER1_FORMAT_ERR			5	//��ʱ��1��ʽ����
#define	CAN_CONNECT_OPEN_DRIVER_ERR				6	//�豸��ʧ��
#define	CAN_CONNECT_INIT_DRIVER_ERR				7	//�豸��ʼ��ʧ��
#define CAN_CONNECT_START_ERR					8	//�豸��ʼʧ��

#define CAN_RESET_OK							0	//can������λ�ɹ�
#define	CAN_RESET_FAILED						1	//can������λʧ��
#define CAN_RESET_NOT_OPEN						2	//can����û�д�

#define	CAN_NO									0
#define	CAN_YES									1

#define	MAX_DEV_TYPE_NUM						16
#define MAX_LARGE_SEND_ID						32
#define MAX_LARGE_SIZE							4096

#define CAN_LARGE_NO							0
#define	CAN_LARGE_SEND							1
#define	CAN_LARGE_RECEIVE						2

typedef struct __CAN_OBJ
{
	int				id;
	int				len;
	unsigned char	buf[MAX_LARGE_SIZE];
}CAN_OBJ;

typedef struct __CAN_LARGE_DATA
{
	int				id;			//��ǰIDֵ
	int				ack;		//CAN_NO����ʾ��ǰ�����ݷ���δ�յ����Ӧ��
								//CAN_YES����ʾ��ǰ�����ݽ��յ������Ӧ��
	int				len;		//���ͻ��߽������ݵĳ���
	int				flag;		//��ǰid�Ƿ�����ʹ��
	int				start;		//��ǰ���ݿ�ʼλ��
	int				rx_id;		//����ID
	int				tx_id;		//����ID
	int				serial;		//��ˮ��
	unsigned char	data[MAX_LARGE_SIZE];	//���ݴ��
}CAN_LARGE_DATA;

class ECanDriver
{
public:
	ECanDriver(void);
	~ECanDriver(void);

	/* �豸�������Ƽ��ͺ� */
	map<CString,int>			m_mapDevtype;
	int							m_devtype;



	/* �豸���ƴ��� */
	LPCTSTR						dev_type_str_list[MAX_DEV_TYPE_NUM];
	int							dev_type_str_num;
	int							dev_type_select;
	CString						dev_type_str;

	/* canͨ������ */
	CString						dev_can_channel_str_list[MAX_DEV_TYPE_NUM];
	int							dev_can_channel_str_num;
	int							dev_can_channel_select;

	/* ������ */
	CString						dev_can_idx_str_list[MAX_DEV_TYPE_NUM];
	int							dev_can_idx_str_num;
	int							dev_can_idx_select;

	/* ������ */
	CString						dev_can_bitrate_str_list[MAX_DEV_TYPE_NUM];
	int							dev_can_bitrate_str_num;
	int							dev_can_bitrate_select;

	/* �˲� */
	CString						dev_can_fliter_str_list[MAX_DEV_TYPE_NUM];
	int							dev_can_fliter_str_num;
	int							dev_can_fliter_select;

	/* ģʽ */
	CString						dev_can_mode_str_list[MAX_DEV_TYPE_NUM];
	int							dev_can_mode_str_num;
	int							dev_can_mode_select;

	/* ������ */
	CString						unmask_code;
	/* ������ */
	CString						mask_code;
	/* can������������� */
	HANDLE						h_mutex;
	/* ���ݽ���ringbuf */
	RingBuf						can_receive_buf;

	/* ������������ʱʹ�ã�����֡ͷ��Ϣ�ɹ��󣬵ȴ��ն˽���Ӧ�� */
	int							send_head_ack;

	/* �����������б�־ */
	int							large_data_sending;
	/* �����ݷ���ID */
	int							large_data_send_id;
	/* ��������������͵�ID�б� */
	CAN_LARGE_DATA				large_data_id_list[MAX_LARGE_SEND_ID];
	/* �����������ݷ��͵�ID���� */
	int							large_data_id_num;
	/* can����״̬ */
	int		m_can_link_status;

	/* CAN���ݽ����߳� */
	static UINT ReceiveThread(void *param);

	/* CAN�������ݷ����߳� */
	static UINT TransmitHeartBeat(void *param);

	/* ��ʼ��can�������� */
	int		CanDriverInit();
	/* �ر�can���� */
	int		CanDriverClose();
	/* can�������� */
	int		CanDriverConnect();
	/* �Ͽ�can���� */
	int		CanDriverUnconnect();
	/* ��λcan���� */
	int		CanDriverReset();
	/* ��õ�ǰcan����״̬ */
	int		GetCanDriverStatus();
	/* �������� */
	int		TransmitData(unsigned int id, unsigned char* p_data, int len);
	/* ������������ݷ���ID */
	int		AddLargeSendCanId(int tx_id,int rx_id);
	/* �����������ݷ���ID */
	int		ClearLargeSendCanId(void);
	/* ͨ��can id��ȡ��Ӧ��ͬ��ID */
	int		GetCanTxRxId(int tx_id, int rx_id);
protected:
	/* ����N֡���ݵ�CAN������ */
	int		TransmitDataSub(PVCI_CAN_OBJ pSend,int frame);
	/* ͨ��tx id����rx id��ȡ��ǰ�����Ĵ����ݽṹ */
	CAN_LARGE_DATA * GetCanLargePdata(int tx_id, int rx_id);
};

