
#ifndef __PROTOCAL_API_H__
#define __PROTOCAL_API_H__

#include "protocolapi.h"
// uart message related
//#define USART_MSG_HEAD			0xff

/*================ϵͳ��Ϣ======================*/

///������Ϣ
//for ARM:none
//for MCU:{}
//ARM֪ͨMCU�������
#define CM_ARM_READY				0x81ul
/*================ʱ��ͬ��======================*/

///MCU ��ARM �ϱ�ʱ��,���para[0] ~para[5] ȫΪ0��ʾʱ��ֵ����Ч��
//for MCU:{para[0] = time>>7,para[1] = time>>6��para[2] =..��}
//for ARM:{}
//MCU ��ARM �ϱ�ʱ���ȡʱ��
#define CM_GET_TIME				0x01ul//���������Ӧ

#define CM_ACC_OFF				0x02ul//���������Ӧ

#define	CM_GENERAL_ACK			0x00


#define	CM_SEND_KEY				0x03  // ����״̬��Ϣ

// ������Ϣ
// for ARM: {para, 7�ֽ�= ( para[0]: ������Ϣ}
// 
// for MCU: none
// MAU֪ͨARM��ǰ������Ϣ

#define	CM_SEND_VEHICLE_INFO				0x04	

/* FROM HMI ID:0x31  event  */
#define CM_SET_MODE				0x05  // 1:2D 2:3D��ʾ�л�
#define	CM_SET_ViewType			0x0E  //������ʾ��ͼ

//#define CM_SET_AVM_SW     		0x03  // AVM���ذ����ź�
#define	CM_GET_TURN_STAT	   	0x90
#define  CM_SET_CAR_COLOUR      0x11 //SET_CAR_COLOUR


//��ȡ��ʾ��ʽ
// for ARM: {para, 1�ֽ�= ( para[0]: ��ʾ��ʽ}
// 1:2D 2:3D
// for MCU: none
// MAU֪ͨARM��ǰ������Ϣ

#define	CM_GET_MODE				0x06

//��ѯ��ǰ��ͼ���ͣ�������
#define	CM_GET_DISP_TYPE				0x07

//�����뻽��
#define	CM_SET_WAKE				0x08


//mcu��������
#define CM_ARM_UPGRADE				0x09

//mcu�ظ�APP �汾
#define CM_ARM_APP_VER				0x0A

//mcu�ظ�BOOT �汾
#define CM_ARM_BOOT_VER				0x0B
//get arm app ver 
#define	CM_ARM_APP_VERSION			0x0C
//��ȡ�㷨�汾
#define	CM_ARM_ARI_VERSION			0x0D	

//������ʾ��ͼ
#define	CM_SET_DISP_TYPE				0x0E

//ARM��ӦMCU��ǰ��ʾ��ͼ
#define	CM_SET_DISP_TYPE_ACK	0x9A       
//һ���궨
#define CM_VIDEO_CALIBRATION   0x0F

//���ó�ģ��ɫ
#define  CM_SET_CAR_COLOUR   0x11

//����AVM�ܿ���
#define	CM_SET_AVM_ALL_SWITCH				0x12

//���ó����߿���״̬
#define	CM_SET_CAR_SUBLINE				0x13

//���ø�����ʻ���ܿ���
#define	CM_SET_ADAS_FUNCTION_SWITCH			0x14

//���ø�����ʻ���ܿ����ٶ�
#define	CM_SET_ADAS_FUNCTION_SPEED				0x15

//��ȡAVM���ò���
#define	CM_GET_AVM_INFO				0x10

//��ȡAVM���ò�����Ӧ
#define	CM_GET_AVM_INFO_ACK				0x8E

//����ת�򿪹�
//#define CM_SET_TURN_ENABLE					0x17

//����ת�򿪹ػ�Ӧ
#define	CM_GET_TURN_STAT				0x90

//������ʾ��ͼ��Ӧ
//#define	CM_SET_DISP_TYPE_ACK				0x8C

//ͨ��Ӧ��
#define	CM_GENERAL_ACK				0x00

//SD������
#define	CM_TEST_SD_CARD				0x18

//SD������Ӧ��
#define CM_TEST_SD_CARD_ACK			   0x91


//������ͨ��MCUת������ʾ�豸�����л�
#define	CM_UNVTRANS_TO_ARM				0x16
#define	CM_UNVTRANS_TO_MCU				0x110
#define	CM_GET_MODE_ACK				0x82
/*================������Ϣ======================*/


#define	CM_SET_AVM_DISP					0x84//arm�л�AVM ����MCU

#define CM_AVM_CalibrationStatus       0x40 
#define CM_AVM_CamCaliFault       		0x41
#define CM_VIDEO_CALIBRATION            0x0F
#define CM_VIDEO_CALIBRATION_ACK          0x8C
#define  CM_SEND_CAMERA_FAULT            0x99


///������Ϣ
//for ARM:{para[0] = (0:����boot, 1:����App),para[1] = (0:arm �����������1:MCU���������)}
//for MCU:{para[0] = (0:����boot, 1:����App),para[1] = (0:arm �����������1:MCU���������)}
//֪ͨ�Է�MCU����ģʽ
#define CM_MCU_UPGRADE				0x85
#define	CM_MCU_TO_ARM_UPGRADE	    		0x09

///ARM��ȡMCU BOOT�汾��Ϣ�� 01 00 03 00����ʾ�汾�ţ�3.0.1��
//for ARM:{para[0] = ..,para[1] = ..��para[2] =..��para[3] =00}
//for MCU:{}
//��ȡMCU Boot �汾��
#define CM_MCU_BOOT_VERSION			0x87

///ARM��ȡMCU APP�汾��Ϣ�� 01 00 03 00����ʾ�汾�ţ�3.0.1��
//for ARM:{para[0] = ..,para[1] = ..��para[2] =..��para[3] =00}
//for MCU:{}
//��ȡMCU App �汾��
#define CM_MCU_APP_VERSION			0x86

//arm ����λ
#define CM_RESET_SYSTEM            0x88
//��ȡARM���APP�汾��Ϣ
#define	CM_ARM_APP_VERSION_ACK							0x89	
// ��ȡARM����㷨�汾��Ϣ
#define	CM_ARM_ARI_VERSION_ACK					0x8A	
#define	CM_HEARTBEAT				0x8B
/*================Ӳ��ģ�������Ϣ======================*/
///��������Ϣ
///���������ݸ�ʽ: ��16�ֽڣ����е�0�ֽڱ�ʾ��Ŀ���[0x00 ~ 0x10]
//for MCU: ��������: para[0] = ��Ŀ���, д������: para[0:15]
//for ARM: �ϱ�������: para[0:15]
#define ITEM_TEST_ID_MAX			0x10ul
#define ITEM_TEST_ITEM_SIZE		0x10ul
#define CM_ITEM_TEST				0x98ul

#define CM_HOST_ACK_MSG			0x90ul


/************UDS   �������  **************/
#define    CM_UPDATE_ARM_READY 					   0x19ul
#define    CM_UPDATE_ARM_READY_ACK 				   0x92ul

#define    CM_UPDATE_ARM_START 					    0x1Aul
#define    CM_UPDATE_ARM_START_ACK 					   0x93ul

#define    CM_UPDATE_ARM_DATATRANS 					   0x1Bul
#define    CM_UPDATE_ARM_DATATRANS_ACK 			   0x94ul

#define    CM_UPDATE_ARM_DATATRANS_END 			   0x1Cul
#define    CM_UPDATE_ARM_DATATRANS_END_ACK 			   0x95ul

#define    CM_UPDATE_ARM_FINISH							 0x96ul
#define    CM_UPDATE_ARM_ABORT 							 0x1Dul
/************End UDS �������  **************/
#define	CM_SET_TURN_ENABLE				                   0x1E
#define	CM_GET_TURN_STATUS       				          0x1F
#define	CM_GET_TURN_STATUS_ACK      				            0x97
#define CM_PRESENT_TIME										   0x20
/**cycle from ARM��ʾ��ͼ**/
#define	CM_SEND_DISP_TYPE				                    0x98

#define	CM_GET_DISP_TYPE_ACK			  	0x83

#define	CM_CYCLE_SEND_HAIMA_AVM_INFO			                  0x9C

//����ת��USB CHECK
#define    CM_CHECK_USB 								  0x21
#define    CM_CHECK_USB_ACK 								 0x9B


/*================PC�Զ���������Ϣ======================*/

//�˳��Զ�������
#define	CM_PC_ARM_STOP				0x21	

//ͨ��Ӧ��ȷ��MUC or PC�Ƿ��յ�ָ��
#define	CM_PC_GENERAL_ACK				0x00

//PC���MUC�Ƿ�����
#define	CM_PC_ARM_READY				0x01

//PC��MUC���ͳ�����Ϣ
#define	CM_PC_SEND_VEHICLE_INFO				0x02

//PC��MCU�����/�ر�AVM��Ƶ���
#define	CM_PC_SET_AVM_ENABLE				0x03

//PC�����ȡMUC���APP�汾��Ϣ
#define	CM_PC_MCU_APP_VERSION								0x04

//MCU��PC����MCU_APP�汾��Ϣ
#define	CM_PC_MCU_APP_VERSION_ACK							0x05

//PC�����ȡMUC���boot�汾��Ϣ
#define	CM_PC_MCU_BOOT_VERSION						0x06	

//MCU��PC����MCU_boot�汾��Ϣ
#define	CM_PC_MCU_BOOT_VERSION_ACK					0x07

//PC��������ϵͳ
#define	CM_PC_RESET_SYSTEM				0x08

//PC�����ȡARM_APP�汾��Ϣ
#define	CM_PC_ARM_APP_VERSION								0x09

//MCU��PC����ARM_APP�汾��Ϣ
#define	CM_PC_ARM_APP_VERSION_ACK							0x0A

//PC�����ȡARM�㷨�汾��Ϣ
#define	CM_PC_ARM_ARI_VERSION						0x0B	

//MCU��PC����ARM�㷨�汾��Ϣ
#define	CM_PC_ARM_ARI_VERSION_ACK					0x0C	

//PC����������ʾ��ͼ
#define	CM_PC_SET_DISP_TYPE				0x0D

//MCU��PC����������ʾ��ͼ���
#define	CM_PC_SET_DISP_TYPE_ACK				0x0E

//PC�����ѯ��ǰ��ͼ��ʾ����
#define	CM_PC_GET_DISP_TYPE				0x0F

//MCU��PC���͵�ǰ��ͼ��ʾ����
#define	CM_PC_GET_DISP_TYPE_ACK				0x10

//PC����һ���궨
#define	CM_PC_VIDEO_CALIBRATION				0x11

//PC�������ó�ģ��ɫ
#define	CM_PC_SET_CAR_COLOUR				0x12

//PC��������AVM����
#define	CM_PC_SET_AVM_ALL_SWITCH				0x13

//PC�������ó����߿���״̬
#define	CM_PC_SET_CAR_SUBLINE				0x14

//PC�������ø�����ʻ���ܿ���
#define	CM_PC_SET_ADAS_FUNCTION_SWITCH			0x15

//PC�������ø�����ʻ���ܿ����ٶ�
#define	CM_PC_SET_ADAS_FUNCTION_SPEED				0x16

//PC�����ȡAVM���ò���
#define	CM_PC_GET_AVM_INFO				0x17

//MCU��PC����AVM���ò���
#define	CM_PC_GET_AVM_INFO_ACK				0x18

//PC��������ת�򿪹أ�Ϋ��
#define	CM_PC_SET_TURN_ENABLE					0x19

//MCU��PC����ת�򿪹��������
#define	CM_PC_GET_TURN_STAT				0x1A

//��λ��д����������
#define	CM_PC_SET_SerialNumber				0x1B 

//��λ��������������
#define	CM_PC_GET_SerialNumber				0x1C   

//MCU�ظ���λ����������
#define	CM_PC_GET_SerialNumber_Ack				0x1D    

//��λ��д���豸ID
#define	CM_PC_SET_DeviceID				    0x1E    

//��λ����ȡ�豸ID
#define	CM_PC_GET_DeviceID				    0x1F    

//MCUӦ���ȡ�豸ID
#define	CM_PC_GET_DeviceID_Ack				    0x20   

//2D/3D��ʾ�л�
#define CM_PC_SET_MODE                        0x22

//SD������
#define    CM_PC_TEST_SD_CARD			   0x23

//SD������Ӧ��
#define	CM_PC_TEST_SD_CARD_ACK				0x24

//MCU��������ָ��
#define    CM_PC_MCU_SLEEP			   0x25

//��ѹ���ָ��
#define    CM_PC_MCU_VOLTAGE           0X26

//��ѹ���ָ��Ӧ��
#define    CM_PC_MCU_VOLTAGE_ACK           0X27

//���USB
#define	CM_PC_USB				                           0x28u
#define	CM_PC_USB_ACK				                  0x29u

//#define	CM_CRT_YMODEM_YPDATE				            0x88u

/**********End auto test define*************/

#endif /* end of __PROTOCAL_API_H__ */

