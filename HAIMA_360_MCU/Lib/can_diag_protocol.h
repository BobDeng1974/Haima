
#ifndef _CAN_DIAG_PROTOCOL_H
#define _CAN_DIAG_PROTOCOL_H
#include "stdint.h"
#include "miscdef.h"



#ifdef	CAN_DIAG_GLOBALS
#define	CAN_DIAG_EXT     //��CAN_DIAG_GLOBALS�ж����.c ģ��ʱ��������ʹCAN_DIAG_EXTǿ��Ϊ��
#else
#define	CAN_DIAG_EXT	extern//����������δ����CAN_DIAG_GLOBALS��.cʱ������CAN_DIAG_EXTΪextern(�ⲿȫ������) �ɹ�����ģ�����
#endif

//����ID����
#define AVM_Phy_Req           (0x752) //����豸������������������CAN��ʶ��
#define FUNCATIONAL_REQ_ADDR  (0x7DF) //����豸����Ѱַ����CAN��ʶ��	
#define AVM_Phy_Resp          (0x753) //������������豸��������ӦCAN��ʶ��

#define FIX_NO_USE_DATA    0x00  //��������һ��������Ч�����������8�ֽڣ�δʹ���ֽ���0x00��� 

  /*****************address_type  start**********************/
  #define DIAG_P_ADDRESS                       0x01
#define DIAG_F_ADDRESS                       0x02
/*****************address_type  start*end*****************/

	
/*****************sid����  start**********************/
#define DIAG_SESSION_SID                0x10 //�Ựģʽ
#define DIAG_ECU_RESET_SID              0x11 //��λ
#define DIAG_CLR_DTC_SID                0x14 //������ϡ�
#define DIAG_SECURITY_ACCESS_SID        0x27 //��ȫ����
#define DIAG_COMM_CONTROL_SID           0x28 //ͨ�ſ���
#define DIAG_R_DIR_DID                  0x38 //��ȡ�ļ�Ŀ¼�б�
#define DIAG_TESTER_PRESENT_SID         0x3e //����������
#define DIAG_W_DID_SID                  0x2e //д��ʶ��
#define DIAG_R_DID_SID                  0x22 //����ʶ��
#define DIAG_R_DTC_SID                  0x19 //��DTC
#define DIAG_DTC_CONTTOL_SID            0x85 //DTC����
#define DIAG_ROUTE_CONTROL_SID          0x31 //���̿���

#define DIAG_REQUEST_DOWNLOAD_SID      (0X34)   // add by xzb 20170303
#define DIAG_TRANSFER_DATA_SID         (0X36)   // add by xzb 20170303
#define DIAG_REQUEST_TRANSFER_EXIT_SIG (0X37)   // add by xzb 20170303

/* �Ựack  sid(����ӦӦ��ֵΪsid+0x40)   add by taoqh 20170427 */
#define DIAG_SESSION_ACK_SID                (0x10|0x40) //�Ựģʽ
#define DIAG_ECU_RESET_ACK_SID              (0x11|0x40) //��λ
#define DIAG_CLR_DTC_ACK_SID                (0x14|0x40) //������ϡ�
#define DIAG_SECURITY_ACCESS_ACK_SID        (0x27|0x40) //��ȫ����
#define DIAG_COMM_CONTROL_ACK_SID           (0x28|0x40) //ͨ�ſ���
#define DIAG_TESTER_PRESENT_ACK_SID         (0x3e|0x40) //����������
#define DIAG_W_DID_ACK_SID                  (0x2e|0x40) //д��ʶ��
#define DIAG_R_DID_ACK_SID                  (0x22|0x40) //����ʶ��
#define DIAG_R_DTC_ACK_SID                  (0x19|0x40) //��DTC
#define DIAG_DTC_CONTTOL_ACK_SID            (0x85|0x40) //DTC����
#define DIAG_ROUTE_CONTROL_ACK_SID          (0x31|0x40) //���̿���
#define DIAG_REQUEST_DOWNLOAD_ACK_SID       (0X34|0x40) //��ʼ��������
#define DIAG_TRANSFER_DATA_ACK_SID          (0X36|0x40) //��������
#define DIAG_REQUEST_TRANSFER_EXIT_ACK_SIG  (0X37|0x40) //���ݴ������
/*****************sid����  end**********************/

/**************sesion_mode �Ựģʽ start******************/
#define DIAG_DE_MODE       0x01
#define DIAG_PR_MODE       0x02
#define DIAG_EX_MODE       0x03
/**************sesion_mode �Ựģʽ end******************/

/*****************��ȫ����״̬ start***********************/
#define	DIAG_SECURITY_LEVEL_STATE_UNLOCK    	0  //Ҫ��ǰ׺����ֹ����Ҳ������ͬ
#define	DIAG_SECURITY_LEVEL_STATE_LOCK		    1
/*****************��ȫ����״̬ end***********************/

/*************dtc_function_enabled start*********************/
#define DIAG_DTC_FUNCTION_ON    0x01
#define DIAG_DTC_FUNCTIONOFF    0x02
/*************dtc_function_enabled end*********************/

/***************communication_enable start*******************/
#define DIAG_COMM_RX_AND_TX_ENABLE   0x00
#define DIAG_ENABLE_RX_AND_TX_DISABLE  0x01
#define DIAG_COMM_RX_AND_TX_DISABLE  0x03
/***************communication_enable end*******************/

/*************Ҫ����Ĳ��� start******************************/
//#define SAFC_1_add1   0x00
//#define SAFC_3_add1  (0x00+1)   
/*************Ҫ����Ĳ��� end******************************/

/**********************����Ӧ start**************************/
#define NO_ERR                             0x00    //��ȷ
#define SID_ERR                            0x11    //����֧��
#define SUB_ERR                            0x12    //��֧���ӹ���
#define FOR_ERR                            0x13    //����ȷ����Ϣ���Ⱥ���Ч�ĸ�ʽ
#define CONDITION_ERR                      0x22    //��������ȷ
#define SEQUENCE_ERROR                     0x24    //�������д���
#define OUTRANGE_ERR                       0x31    //���󳬳���Χ
#define SECURITY_ERR                       0x33    //��ȫ���ʾܾ�
#define INVALID_KEY_ERR                    0x35    //��Կ��Ч
#define OVER_TRY_ERR                       0x36    //�������Դ���
#define DELAY_NOT_ARRIVAL_ERR              0x37    //����ʱ���ӳ�δ��
#define UPLOAD_DOWNLOAD_NOT_ACCEPTED       0x70    //�������ϴ�����
#define TRANSFER_DATA_SUSPENDED            0x71    //���ݴ�����ͣ
#define PROGRAMME_ERR                      0x72    //һ����ʧ��
#define WRONG_BLOCK_SEQUENCE_COUNTER       0x73    //��������ݿ����м�����
#define REQ_CORRECTLY_REC_RESP_PENDING     0x78    //��ȷ����������Ϣ-�ȴ���Ӧ
#define SUB_FUNC_NOT_SUPPORT_ACTIVE_SES    0x7E    //����Ự��֧�ָ��ӷ���
#define NOT_SUPPERT_SUB_IN_ACTIVE_SESSION  0x7F    //����Ự��֧�ָ÷���
#define BUFF_OVER_FLOW                     0xFF    //the buff is overflow 
/**********************����Ӧ end**************************/



/**************SAFC_reach_3num_mcu_resetbit SAFC �ﵽ���θ�λ��־ start*************/
#define DIAG_ECU_HAD_RESET     0x01
#define DIAG_ECU_HAD_NOT_RESET 0x00
/**************SAFC_reach_3num_mcu_resetbit SAFC �ﵽ���θ�λ��־ end*************/

/***********security_level3_access_step  security_level1_access_step start***********/
#define DIAG_HAD_REQ_SEED   0x01
#define DIAG_NOT_REQ_SEED   0x02

/***********security_level3_access_step security_level1_access_step end***********/
//
#define ECU_SUPPORT_MSAK       				0x09 


// ��ȫ�������� ��ECU_MASK

#define  ECU_Mask_Boot  	(0x6a7c52e7) 
#define  ECU_Mask_APP      	(0x52c36b19)


typedef struct{
	
	uint8_t 	address_type;//��ַ���͡�g_address_type���ֹ���Ѱַ(�㲥)������Ѱַ(������ĳһ��ECU)
	uint8_t 	sesion_mode; //�Ựģʽ     g_session_model
	uint8_t 	security_level1_state; //һ�㰲ȫ״̬ ��Ӧ server_lock_state_1
	//uint8_t 	security_level2_state; //������ȫ״̬  no use
	uint8_t 	security_level3_state; //��̰�ȫ״̬  server_lock_state_2
	uint8_t 	dtc_function_enable;  //DTC����ʹ�ܡ�dtc_setting_type	  ��ĵط��õ�disable_send_can_data��dtc_function_enable���
	uint8_t 	communication_enable; //ͨ�ſ��� Type  comm_control_type
	uint32_t    security_seed; //���ӡ�����Seed_App   Seed_Bootloader
	uint32_t    security_ECU_Mask;//���� ECU_Mask_App   SECU_Mask_Bootloader    
	                    //ʧ�����ﵽ3�κ��ӳ������ӣ������Ӳ��ܷ��ʣ��������ӳ����SAFC���,����ﵽ3�Σ����������ӿ��Գ���������ʱ����ʾ���ʴ�������������ʧ��
	uint8_t 	SAFC_1; //һ�㰲ȫ ��ȫ����ʧ�ܼ���  SAFC_1 
	uint32_t 	time_SAFC_1; //һ�㰲ȫ ��ȫ����ʧ�ܼ����ﵽ����ʱ��� timing_SAFC_1
	uint8_t     security_level1_access_step; //��ȫ���ʲ��衡req_seed_state_1
	uint8_t     security_level3_access_step; //��ȫ���ʲ��衡req_seed_state_2
	uint8_t 	SAFC_3;  //��̰�ȫ״̬ ��ȫ����ʧ�ܼ���  SAFC_2
	uint32_t 	time_SAFC_3; //��̰�ȫ ��ȫ����ʧ�ܼ����ﵽ����ʱ��� timing_SAFC_2
	uint8_t     SAFC_reach_3num_mcu_resetbit:1; //SAFC �ﵽ���θ�λ��־ g_ecu_had_reset_just_now  #define ECU_HAD_RESET     0x01 #define ECU_HAD_NOT_RESET 0x00
	uint8_t     file_trans_mode; //04 �ļ����ݴ��� 05 �ļ��б���
	uint32_t    file_len;        //�ļ����ļ��б���
	uint32_t 	rev_frame_tim; //���յ��κ���ʱ��
	//u8          update_type;//0����MCU_APP  ��0 ����ARM
}CAN_DIAG_TYPE;
extern CAN_DIAG_TYPE diag; 

//Ӧ�ò�ʱ�����
#define OS_1MS     1
typedef struct
{
	uint16_t P2can;//����ǣ�PC��������ڳɹ���������������ĺ󣬵ȴ�ECU���������Ӧ�ĳ�ʱʱ��ISO 15765-3�иò�����Ϊ��P2CAN_Client
	uint16_t P2_asterisk_can; //ms P2*can;���������ڷ��ͷ���Ӧ��Ϊ0x78 �ķ���Ӧ�󣬵�������������Ӧ��Ϣʱ�������Ҫ��
	uint16_t S3_sever; //��ʱ����˲�����ms �������Ķ�ʱ�����������ڷ�Ĭ�ϻỰģʽ����S3Serverʱ���ڣ����������û�н��յ��κ���������ģ����˳���Ĭ�ϻỰģʽ������Ĭ�ϻỰģʽ 
}SERVER_APPLICATION_LAYER_TIM_TYPE; //������ ʱ�����


typedef struct
{
	uint16_t P2can;//AVM���յ�������Ϣ������Ӧ��ʱ�������Ҫ�� ms
	uint16_t P2_asterisk_can; //ms P2*can;���������ڷ��ͷ���Ӧ��Ϊ0x78 �ķ���Ӧ�󣬵�������������Ӧ��Ϣʱ�������Ҫ��
	uint16_t P3_can_phy;/*�ͻ��˳ɹ���������Ѱַ������Ϣ�����Ҹ�������Ϣ����Ҫ������������Ӧ�󣬿ͻ����ٴη���
                        ��һ������Ѱַ������Ϣ����С���ʱ�� */
	uint16_t P3_can_func; /*�ͻ��˳ɹ������깦��Ѱַ������Ϣ�����ٴη�����һ������Ѱַ������Ϣʱ����С���ʱ�䣬
                        �����������һ��Ϊ�ͻ��˷��͵�������Ҫ��������Ӧ��һ��Ϊ���ɲ���֧�ָ�����ķ�����
                        ��Ӧ�������ڲ��ַ�������������Ӧ�� */
	uint16_t S3_client; //ms �������Ķ�ʱ�����������ڷ�Ĭ�ϻỰģʽ����S3Serverʱ���ڣ����������û�н��յ��κ���������ģ����˳���Ĭ�ϻỰģʽ������Ĭ�ϻỰģʽ 
	uint16_t S3_Tester;//����Ƿ�����֡���������(3E hex)������ʹECU�����ڷ�Ĭ�ϻỰģʽ�µ����ʱ����
}CLIENT_APPLICATION_LAYER_TIM_TYPE; //Ӧ�ò�Ự�����ʱ��


//��������Ϣ
typedef struct body_car_rec_info
{	//from 0x1f7
	u8 GearSelDisp;//��λѡ��
	u16 VehSpd;//����
	u8 VehSpdVd;//�����Ƿ���Ч
	//0x22C
	u8 DirLampLeftSwitchSts;//��ת��
	u8 DirLampRightSwitchSts; //��ת��
	u8 RsvGearSwitchSts;//�ֶ������������ź�
	//0x3D4
	s16 StrWhlAngle; //��ǰ������ת���
	u8 StrWhlAngleSign;//������ת������
	u16 StrWhlSpd;//������ת���ٶ�

	//0x539
	u8 AvmGuidesSet; //ȫ������������
	u8 AVM_LDW_SET;//360 ����ƫ�빦��״̬����
	u8 SET_CAR_color; //��ɫ
	u8 TouScrPreKey2;//��������λ��2
	u8 Avm2D3DSwitch;//
	//0x6C2
	u32 AVMOneKeyCaltion;//����һ���궨 ���ڳ������߱궨
	u8 AVMOneKeyCaltionVd;//һ���궨��־λ
}body_car_rec_info_t;

//���ͱ���
typedef struct body_car_send_info
{	
	u8	AVM_SET_AVM_DISP;
	u8	AVM_CalibrationStatus;
	u8  AVM_CamCaliFault;

	u8  AVM_ActivationStatus;
	u8	AVM_DispViewStatus;
	
}body_car_send_info_t;


CAN_DIAG_EXT u32 app_diagnose(	device_info_t *dev,uint8_t *buffer,uint16_t len);
CAN_DIAG_EXT uint8_t get_diag_sesion_mode(void);
CAN_DIAG_EXT uint8_t get_diag_address_type(void);
CAN_DIAG_EXT uint8_t get_diag_communication_state(void);
CAN_DIAG_EXT uint8_t get_diag_dtc_function_state(void);

CAN_DIAG_EXT void set_diag_address_type(uint8_t type);
CAN_DIAG_EXT void init_diag_protocol_parameer(void);
CAN_DIAG_EXT void init_diag_protocol_parameer_boot(void);
CAN_DIAG_EXT void diag_send_data(device_info_t *dev, u8 *Data, u16 len );
CAN_DIAG_EXT void check_s3_client(device_info_t *dev);


//#define ROUTINE_RESULT_SUCESS  0x01  //���ɹ�
//#define ROUTINE_RESULT_DOING   0x02  //�����ڽ���
//#define ROUTINE_RESULT_FAIL    0x03  //��ʧ��
#endif
