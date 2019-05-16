#pragma once

#define FILE_MAX_SIZE						0x100000000			//�ļ��ռ����� -> 4GB
//#define FILE_MAX_SIZE						0x400000			//�ļ��ռ����� -> 4MB
#define KB_1								0x400				//1kb	
#define UNLIMIT								0xffffffff			//�����ʹ���ֵΪ0xffffffffʱ�����ʹ���������	
#define CAN_INFO_NUM						0x27				//����CAN��Ϣ��ĿΪ36+3
#define GET_FILE_DIRECTORY					"SGMW_GetFiles"		//tbc�ļ�Ŀ¼
#define PRINT_LOG							"print.log"			//��ӡ��־


/* ����֡��Ϣ���� */
#define NO_ERR								0x00	//��ȷ
#define SID_ERR								0x11	//����֧��
#define SUB_ERR								0x12	//��֧���ӹ���
#define FOR_ERR								0x13	//����ȷ����Ϣ���Ⱥ���Ч�ĸ�ʽ
#define CONDITION_ERR						0x22	//��������ȷ
#define SEQUENCE_ERROR						0x24	//�������д���
#define OUTRANGE_ERR						0x31	//���󳬳���Χ
#define SECURITY_ERR						0x33	//��ȫ���ʾܾ�
#define INVALID_KEY_ERR						0x35   	//��Կ��Ч
#define OVER_TRY_ERR          				0x36    //�������Դ���
#define DELAY_NOT_ARRIVAL_ERR 				0x37    //����ʱ���ӳ�δ��
#define UPLOAD_DOWNLOAD_NOT_ACCEPTED        0x70	//�������ϴ�����
#define TRANSFER_DATA_SUSPENDED				0x71	//���ݴ�����ͣ
#define PROGRAMME_ERR         				0x72	//һ����ʧ��
#define WRONG_BLOCK_SEQUENCE_COUNTER		0x73	//��������ݿ����м�����
#define REQ_CORRECTLY_REC_RESP_PENDING  	0x78 	//��ȷ����������Ϣ-�ȴ���Ӧ
#define SUB_FUNC_NOT_SUPPORT_ACTIVE_SES 	0x7E 	//����Ự��֧�ָ��ӷ���
#define NOT_SUPPERT_SUB_IN_ACTIVE_SESSION   0x7F	//����Ự��֧�ָ÷���
#define BUFF_OVER_FLOW						0xFF	//the buff is overflow
#define PROGRAMME_ERR         				0x72	//һ����ʧ��


/* ISO14429Э�������� */
#define DIAGNOSITIC_SESSION_CONTROL			0x10
#define ECU_RESET							0x11
#define CLEAR_DIAGNOSTIC_INFORMATION		0x14
#define READ_DTC_INFORMATION				0x19
#define READ_DATA_BY_IDENTIFIER				0x22
#define SECURITY_ACCESS						0x27
#define COMMUNICATION_CONTROL				0x28
#define WRITE_DATA_BY_IDENTIFIER			0x2E
#define INPUT_OUTPUT_CONTROL_BY_IDENTIFIER	0x2F
#define ROUTINE_CONTROL						0x31
#define TESTER_PRESENT						0x3E
#define CONTROL_DTC_SETTING					0x85
#define ERR_DTC_ID							0x7F


/* ����д��״̬�� */
#define WRITE_TO_TBOX_NULL					0		//�޶���
#define WRITE_TO_TBOX_EXTERN_MODE			1		//������չģʽ
#define WRITE_TO_TBOX_WAIT_EXTERN_MODE		2		//�ȴ�������չģʽ
#define WRITE_TO_TBOX_READ_SN				3		//��SN
#define WRITE_TO_TBOX_WAIT_READ_SN			4		//�ȴ���ȡSN
#define WRITE_TO_TBOX_READ_SEED				5		//������
#define WRITE_TO_TBOX_WAIT_READ_SEED		6		//�ȴ���ȡ����
#define WRITE_TO_TBOX_READ_PASSWORD			7		//����Կ
#define WRITE_TO_TBOX_WAIT_READ_PASSWORD	8		//�ȴ���ȡ��Կ
#define WRITE_TO_TBOX_WRITE_DATA			9		//д������
#define WRITE_TO_TBOX_WAIT_WRITE_DATA		10		//�ȴ�����д�����



/* �ļ�����״̬�� */
#define TRANS_FROM_TBOX_NULL				0		//�޶���
#define TRANS_FROM_TBOX_EXTERN_MODE			1		//������չģʽ
#define TRANS_FROM_TBOX_WAIT_EXTERN_MODE	2		//�ȴ�������չģʽ
#define TRANS_FROM_TBOX_READ_SN				3		//��SN
#define TRANS_FROM_TBOX_WAIT_READ_SN		4		//�ȴ���ȡSN
#define TRANS_FROM_TBOX_READ_SEED			5		//������
#define TRANS_FROM_TBOX_WAIT_READ_SEED		6		//�ȴ���ȡ����
#define TRANS_FROM_TBOX_READ_PASSWORD		7		//����Կ
#define TRANS_FROM_TBOX_WAIT_READ_PASSWORD	8		//�ȴ���ȡ��Կ
#define TRANS_FROM_TBOX_REQUEST_READDIR		9		//�ļ��б��ȡ����
#define TRANS_FROM_TBOX_TRANSFER_DIRDATA	10		//�ļ��б���
#define TRANS_FROM_TBOX_TRANSFER_DIREXIT	11		//�ļ��б����˳�
#define TRANS_FROM_TBOX_REQUEST_READFILE	12		//�ļ���ȡ����
#define TRANS_FROM_TBOX_TRANSFER_FILEDATA	13		//�ļ����ݴ���
#define TRANS_FROM_TBOX_TRANSFER_FILEEXIT   14		//�ļ������˳�
#define TRANS_FROM_TBOX_TRANSFER_CHECK	    15		//�ļ�CRCУ��
#define TRANS_FROM_TBOX_REQUEST_DELETE      16		//�ļ�ɾ������
#define TRANS_FROM_TBOX_TRANSFER_END		17		//�������


/* �������� */
#define NULL_TYPE							0		//������
#define UPDATE_TYPE							1		//��������
#define TRANSFER_TYPE						2		//��������

/* �������� */
#define TRANS_NULL							0		//�޴���
#define TRANS_LIST							1		//�ļ��б���
#define TRANS_FILE							2		//�ļ�����
#define DELETE_FILE							3		//ɾ���ļ�


#define GET_BYTE_24(n)						((unsigned char)((n>>24) & 0xff))
#define GET_BYTE_16(n)						((unsigned char)((n>>16) & 0xff))
#define GET_BYTE_8(n)						((unsigned char)((n>>8) & 0xff))
#define GET_BYTE_0(n)						((unsigned char)((n>>0) & 0xff))


#define GET_5WORD(p_temp) ((p_temp[0]<<32) + (p_temp[1]<<24) + (p_temp[2]<<16) + (p_temp[3]<<8) + (p_temp[4]))
#define GET_DWORD(p_temp) ((p_temp[0]<<24) + (p_temp[1]<<16) + (p_temp[2]<<8) + (p_temp[3]))
#define GET_MWORD(p_temp) ((p_temp[0]<<16) + (p_temp[1]<<8) + (p_temp[2]))
#define GET_WORD(p_temp)  ((p_temp[0]<<8) + (p_temp[1]))


typedef struct __tbox_read_info
{
	unsigned char		read_id;		//������ID
	unsigned char		write_id;		//д����ID

	unsigned long		len;			//���ݳ���
	unsigned long		act_mode;		//��дģʽ
	unsigned long		did;			//didi

	unsigned char		param0;			//����0
	unsigned char		param1;			//����1
	unsigned char		param2;			//����2
	unsigned char		param3;			//����3
	unsigned char		param4;			//����4

	CString				str;			//��������
}TBOX_READ_INFO;


typedef struct __tbox_file_list
{
	CString file_name;
	DWORD file_serial;
	DWORD file_length;
	unsigned char file_buf[8];
}TBOX_FILE_LIST;


typedef struct __tbox_write_info
{
	DWORD param0;
	DWORD param1;
	DWORD param2;
	DWORD param3;
}TBOX_WRITE_INFO;


typedef struct __tbox_can_id_info
{
	WORD can_id;
	DWORD send_period;
	DWORD send_cnt;

}TBOX_CAN_ID_INFO;


typedef struct __tbox_wait_time
{
	DWORD time;		//�ȴ�ʱ��
	bool status;	//�ȴ�״̬

}TBOX_WAIT_TIME;


#define AVM_FUN_ID              0x7DF          //Ϋ����Ѱַ��ַ

typedef enum {

	 /* �ļ�����״̬�� */
	 UPDATE_TO_TBOX_NULL = 0,				    //�޶���
	 UPDATA_TO_AVM_FUN_PREPROGRAM,                  //����ѰַԤ��̽׶�
	 UPDATA_TO_AVM_FUN_AFTER_PROGRAM,               //����Ѱַ��̺�׶�
	 UPDATA_TO_AVM_FUN_NEGATIVE_RESPONSE,           //����Ѱַ����Ӧ����׶�

	 UPDATA_TO_AVM_ENTER_EXTMODE,               //������չģʽ
	 UPDATA_TO_AVM_WAIT_ENTER_EXTMODE,          //�ȴ�������չģʽ
	 UPDATA_TO_AVM_STOP_SET_DTC,                //ֹͣ�趨DTC
	 UPDATE_TO_AVM_COMMUNICATE_CONTROL,		    //ͨ�ſ���
	 UPDATE_TO_AVM_READ_SN,                    //�����к�
	 UPDATE_TO_AVM_PROGRAM_MODE,				//������ģʽ
	 UPDATE_TO_AVM_WAIT_PROGRAM_MODE,			//�ȴ�������ģʽ
	 UPDATE_TO_AVM_READ_SEED,					//������
	 UPDATE_TO_AVM_WAIT_READ_SEED,				//�ȴ���ȡ����
	 UPDATE_TO_AVM_READ_PASSWORD,				//����Կ
	 UPDATE_TO_AVM_WAIT_READ_PASSWORD,			//�ȴ���ȡ��Կ
	 UPDATE_TO_AVM_WRITE_FINGERPRINT,           //дָ��
	 UPDATE_TO_AVM_EARSE_MEMORY,				//�ڴ����
	 UPDATE_TO_AVM_DOWNLOAD_FLASH_DRV,			//����flash����
	 UPDATE_TO_AVM_REQUEST_DOWNLOAD,			//��������
	 UPDATE_TO_AVM_TRANSFER_DATA,				//�����ݴ���
	 UPDATE_TO_AVM_TRANSFER_DATA_EXIT,		    //�˳����ݴ���
	 UPDATE_TO_AVM_UPDATE_CHECK,				//�ļ�����У��1
	 UPDATE_TO_AVM_WRITE_DATA,                  //д����
	 UPDATE_TO_AVM_WRITE_VER,                  //д����汾
	 UPDATE_TO_AVM_UPDATE_END,					//������ɣ��ն˸�λ��
	 UPDATE_TO_AVM_RESET,					    //�ն˸�λ
	 UPDATA_TO_AVM_ENTER_EXTMODE_AGAIN,         //�ٴν�����չģʽ
	 UPDATE_TO_AVM_RECOVER_COMMUNICATE_CONTROL, //�ָ�ͨ�п���
	 UPDATA_TO_AVM_START_SET_DTC,               //�����趨DTC
	 UPDATA_TO_AVM_ENTER_DEFMODE,               //����Ĭ�ϻỰģʽ
	 UPDATA_TO_AVM_CLEAR_DTC,                   //���DTC
	 UPDATA_TO_AVM_UPDATE_END,                  //�������
}update_stat;