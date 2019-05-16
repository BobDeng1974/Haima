/*
********************************************************************************
**									CHINA-SZ 
**
** Project	360pro
**
** File 	CAN_DIAG_PROTOCOL.c 
** 
** Terms:
**	  
** Brief: diag protocol for app program
**
** Version	   Author	  Date		 changed	 Description
** ========   =======	 ========	 ========	 ===========   
**	V1.0	   xz        2018523  
**	 
**
********************************************************************************
*/
#define CAN_DIAG_GLOBALS

#include "stdlib.h"
#include <string.h>
#include "can_did.h"
#include "can_diag_protocol.h"
#include "protocolapi.h"
#include "can.h"
#include "can_net_layer.h"
#include "usart.h"
#include "api.h"
#include "hw_config.h"
#include "usart.h"
#include "config.h"
#include "iflash.h"
#include "can_dtc.h"
#include "timer.h"
#include "diag_upgrade.h"

CAN_DIAG_TYPE diag; //diag �����ṹ��
SERVER_APPLICATION_LAYER_TIM_TYPE   server_application_layer_tim;//Ӧ�ò�ʱ�����
CLIENT_APPLICATION_LAYER_TIM_TYPE   client_application_layer_tim;//Ӧ�ò�Ự�����ʱ��

/*
*----------------------------------------------------------------------------------------------------------------------------------------
* ����ԭ�ͣ�void init_diag_protocol_parameter(void)
* �������ܣ���ʼ��diag��ز���
  ����˵����
* �����������
* �� �� ֵ����
  ���ߣ�    qdh
*----------------------------------------------------------------------------------------------------------------------------------------
*/
void init_diag_protocol_parameer(void)
{
 	diag.address_type=DIAG_P_ADDRESS;//����Ѱַģʽ
	diag.sesion_mode=DIAG_DE_MODE;//�ϵ缴������Ĭ�ϻỰģʽ
	diag.security_level1_state=DIAG_SECURITY_LEVEL_STATE_LOCK; //��ȫ״̬
	diag.security_level3_state=DIAG_SECURITY_LEVEL_STATE_LOCK;//��ȫ״̬
	diag.dtc_function_enable=DIAG_DTC_FUNCTION_ON;
	diag.communication_enable=DIAG_COMM_RX_AND_TX_ENABLE;
	diag.SAFC_reach_3num_mcu_resetbit = DIAG_ECU_HAD_NOT_RESET;//ȡ1λ
	diag.SAFC_1=0x00;
	diag.SAFC_3=0x00;
	diag.time_SAFC_3=0x00;
	diag.time_SAFC_1=0x00;//һ�㰲ȫ����ʧ�ܼ���3��ʱ���
	diag.security_level1_access_step=DIAG_NOT_REQ_SEED;//δ����Seed
	diag.security_level3_access_step=DIAG_NOT_REQ_SEED;//δ����Seed
	//SAFC_1  SAFC_3  ��flash������
	//Ӧ�ò�ʱ�����
	server_application_layer_tim.P2can=50*OS_1MS;
	server_application_layer_tim.P2_asterisk_can=5000*OS_1MS;
	server_application_layer_tim.S3_sever=5000*OS_1MS;
	client_application_layer_tim.P2can=50*OS_1MS;//ECU�յ���Ϸ�����Ӧʱ��Ҫ��
	client_application_layer_tim.P2_asterisk_can=5000*OS_1MS;
	client_application_layer_tim.P3_can_phy=100*OS_1MS;
	client_application_layer_tim.P3_can_func=100*OS_1MS;
	client_application_layer_tim.S3_client=5000;
	client_application_layer_tim.S3_Tester=4000;
  	 //diag.rev_frame_tim=0x00; ��������֡��ʱ��
    init_did_data(get_device_info());
	init_dtc_flash();
}
/*
*----------------------------------------------------------------------------------------------------------------------------------------
* ����ԭ�ͣ�void init_diag_protocol_parameter(void)
* �������ܣ���ʼ��diag��ز���
  ����˵����
* �����������
* �� �� ֵ����
  ���ߣ�    qdh
*----------------------------------------------------------------------------------------------------------------------------------------
*/
void init_diag_protocol_parameer_boot(void)
{
 	diag.address_type=DIAG_P_ADDRESS;//����Ѱַģʽ
	diag.sesion_mode=DIAG_PR_MODE;//�ϵ缴������Ĭ�ϻỰģʽ
	diag.security_level1_state=DIAG_SECURITY_LEVEL_STATE_LOCK; //��ȫ״̬
	diag.security_level3_state=DIAG_SECURITY_LEVEL_STATE_LOCK;//��ȫ״̬
	diag.dtc_function_enable=DIAG_DTC_FUNCTIONOFF;
	diag.communication_enable=DIAG_ENABLE_RX_AND_TX_DISABLE;
	diag.SAFC_reach_3num_mcu_resetbit = DIAG_ECU_HAD_NOT_RESET;//ȡ1λ
	diag.SAFC_1=0x00;
	diag.SAFC_3=0x00;
	diag.time_SAFC_3=0x00;
	diag.time_SAFC_1=0x00;//һ�㰲ȫ����ʧ�ܼ���3��ʱ���
	diag.security_level1_access_step=DIAG_NOT_REQ_SEED;//δ����Seed
	diag.security_level3_access_step=DIAG_NOT_REQ_SEED;//δ����Seed
	//SAFC_1  SAFC_3  ��flash������
	//Ӧ�ò�ʱ�����
	server_application_layer_tim.P2can=50*OS_1MS;
	server_application_layer_tim.P2_asterisk_can=5000*OS_1MS;
	server_application_layer_tim.S3_sever=5000*OS_1MS;
	client_application_layer_tim.P2can=50*OS_1MS;//ECU�յ���Ϸ�����Ӧʱ��Ҫ��
	client_application_layer_tim.P2_asterisk_can=5000*OS_1MS;
	client_application_layer_tim.P3_can_phy=100*OS_1MS;
	client_application_layer_tim.P3_can_func=100*OS_1MS;
	client_application_layer_tim.S3_client=2000*OS_1MS;
	client_application_layer_tim.S3_Tester=4000;
}


//******************************************************************************
// DESCRIPTION:
// ARUGMENT   :��ȡcommunication_enable��״̬
// RETURN     :
// AUTHOR     :
//******************************************************************************
  u8  get_diag_communication_state(void)
{
	return diag.communication_enable;
}


u8  get_diag_dtc_function_state(void)
{
	return diag.dtc_function_enable;
}

//******************************************************************************
// DESCRIPTION:
// ARUGMENT   :sesion_mode  ״̬
// RETURN     :
// AUTHOR     :
//******************************************************************************
u8  get_diag_sesion_mode(void)
{
	return diag.sesion_mode;
}

//******************************************************************************
// DESCRIPTION:
// ARUGMENT   :address_type  ��ǰѰַģʽ
// RETURN     :
// AUTHOR     :
//******************************************************************************
u8  get_diag_address_type(void)
{
	return diag.address_type;
}

//******************************************************************************
// DESCRIPTION:����Ѱַģʽ
// ARUGMENT   :
// RETURN     :
// AUTHOR     :
//******************************************************************************
void set_diag_address_type(uint8_t type)
{
	diag.address_type=type;
}
void set_diag_session(uint8_t type)
{
	diag.sesion_mode=type;
}
unsigned int create_seed(void)
{
	unsigned int Seed=0;
	unsigned char i,seed[4]={0};
    srand(get_systick_value());
 	for(i=0;i<4;i++)
 	{
    	seed[i]=rand()%256;
			}
	Seed = (unsigned int)seed[3];
	Seed |=((unsigned int)seed[2])<<8;
	Seed |=((unsigned int)seed[1])<<16;
	Seed |=((unsigned int)seed[0])<<24;
    return Seed;
}

/*****************************************************************************
 �� �� ��  : SecuriyAlgorithmFBL
 ��������  : boot��ȫ�㷨
 �������  : _int32 wSeed  
             _int32 MASK   
 �������  : ��
 �� �� ֵ  : u32
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��12��04��
    ��    ��   : xz
    �޸�����   : �����ɺ���

*****************************************************************************/
//һ�����ʡ�ֻ��Ҫһ����������---���������к�
static u32  key1[4]={0x00,0x0,0x00,0x00};
static u32  key2[4]={0x00,0x00,0x00,0x0};

void  get_key_level1(void)
{
	u8 key[4]={0};
	STMFLASH_Read(ADDR_ECUSerialNumber+6,(u16*)key, 2);
//	printf("%x-%x-%x-%x\r\n",key[0],key[1],key[2],key[3]);
	key1[0] = key[0];
	key1[1] |= key[1];
	key1[2] |= key[2];
	key1[3] |= key[3];
	}

void get_key_level2(void)
{
	u8 key[4]={0};	
	STMFLASH_Read(ADDR_ECUSerialNumber+6,(u16*)key, 2);
//	printf("%x-%x-%x-%x\r\n",key[0],key[1],key[2],key[3]);
	key[0]=(key[0]>>3)|((key[0]&0x07)<<5);
	key[1]=(key[1]>>3)|((key[1]&0x07)<<5);
	key[2]=(key[2]>>3)|((key[2]&0x07)<<5);
	key[3]=(key[3]>>3)|((key[3]&0x07)<<5);
//	printf("%x-%x-%x-%x\r\n",key[0],key[1],key[2],key[3]);
	key2[0] = key[0];
	key2[1] |= key[1];
	key2[2] |= key[2];
	key2[3] |= key[3];	
	}

unsigned int encipher (unsigned int num_rounds,unsigned int v[2],unsigned int  k[4])	// num_rounds=2  Ϊ��ȫ�ȼ����
{
	unsigned int i;
	unsigned int v0=v[0],v1=v[1],sum=0,delta=0x9E3779B9;
	for(i=0;i<num_rounds;i++)
	{
		v0 +=(((v1<<4)^(v1>>5))+v1)^(sum+k[sum&3]);
		sum +=delta;
		v1 +=(((v0<<4)^(v0>>5))+v0)^(sum+k[(sum>>11)&3]);
		}
	v[0]=v0;v[1]=v1;
	return v[0];
}


//******************************************************************************
// DESCRIPTION:��Ϸ���$10 �Ựģʽ for app
// ARUGMENT   : //bufferΪsid��ʼ�������� lenΪsid��ʼ�����ݳ��ȣ�֡�ṹ��
// RETURN     :
// AUTHOR     :
//******************************************************************************
u8  SID_10_fun(device_info_t *dev,u8 *buffer,u16 len)
{
	u8 send_data[8]={0};
	u8 responce_result=NO_ERR;
	if(len!=0x01)//�жϳ���
	{
		responce_result=FOR_ERR;
                         		dbg_msg(dev, "$10 service ����ȷ����Ϣ���ȡ�����\r\n");
		can_send_NRC(DIAG_SESSION_SID,FOR_ERR);
		return  responce_result;
	}
	switch(buffer[0])//��������  ��һ���ֽ�buffer[0]�����ӷ���ѡ��
	{
		case 0x01:				  
		  	if(diag.sesion_mode == DIAG_PR_MODE)
		  	{
		  	//	responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//����������NRC=22 
			//	can_send_NRC(DIAG_SESSION_SID,CONDITION_ERR);
			//	return responce_result;	
			}
		    diag.sesion_mode = DIAG_DE_MODE;//���ý���Ĭ�ϻỰģʽ
			diag.security_level1_state = DIAG_SECURITY_LEVEL_STATE_LOCK;//��ȫ״̬����
		    diag.security_level3_state = DIAG_SECURITY_LEVEL_STATE_LOCK;//��ȫ״̬����
		    diag.dtc_function_enable = DIAG_DTC_FUNCTION_ON;//ʹ��DTC  
		    if(diag.communication_enable == DIAG_COMM_RX_AND_TX_DISABLE)//�ж�ͨ�ſ���ʹ��
		    {
		      diag.communication_enable   = DIAG_COMM_RX_AND_TX_ENABLE;
			//	diag_comm_enable( );//init_can(0);  //test
		    }
			
			//dbg_msg(dev, "Enter $10 01 service success,Need positive response\r\n");
		    //responce contect   $10���������Ӧ��ʽ
			send_data[0] = 0x06;//����
			send_data[1] = 0x50;
			send_data[2] = 0x01;//���� ���� 01 �ӷ���diagnostic session type��	    	    
			send_data[3] = (client_application_layer_tim.P2can&0xFF00)>>8; //��Ÿ��ֽ�
			send_data[4] = client_application_layer_tim.P2can&0xFF;//���ֽ�
			send_data[5] =(client_application_layer_tim.P2_asterisk_can&0xFF00)>>8;// 
			send_data[6] =client_application_layer_tim.P2_asterisk_can&0xFF;//���ֽ� 
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);
		break;
			
		case 0x02:
			if((diag.address_type == DIAG_F_ADDRESS)||(diag.sesion_mode == DIAG_DE_MODE))//��̻Ự����������Ѱַģʽ		  
			{
				responce_result=NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//DIAG_PR_MODE��֧�ֹ���Ѱַ
				can_send_NRC(DIAG_SESSION_SID,CONDITION_ERR);
				return responce_result;					
			}
//			if(check1_result==0)
//			{
//				dbg_msgv(dev,"Don`t receive $31 01 02 02,\r\n");
//				send_data[0] = 0x04 ;
//				send_data[1] = 0x7f;
//				send_data[2] = 0x10;
//				send_data[3] = 0x02;
//				send_data[4] = 0x22;//condition not correct
//				can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//���ط���Ӧ
//				return NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;
//			}

			
			diag.sesion_mode =DIAG_PR_MODE;
			diag.address_type =DIAG_F_ADDRESS;
		 	diag.security_level1_state = DIAG_SECURITY_LEVEL_STATE_LOCK;
		 	diag.security_level3_state = DIAG_SECURITY_LEVEL_STATE_LOCK;
			dbg_msg(dev,"jump to boot\r\n");		
			send_data[0] = 0x02;
			send_data[1] = 0x50;
			send_data[2] = 0x02;
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//������ģʽ�Ŀ϶���Ӧ
			//comm_message_send(dev,CM_UPDATE_ARM_READY, 0,NULL , 0);//֪ͨARM����uboot
			dev->ipc->app_jump = 1;//������־λ ��1	
			stop_timer(TIMER_DEV7); 
			systick_stop();
			device_deinit(dev);
			__disable_irq();
			process_switch(dev->ipc->boot_entry, dev->ipc->boot_stack);//��ת
			break;
			
		case 0x03:
			if(diag.sesion_mode == DIAG_PR_MODE)//	��Ĭ��ģʽת��	  
			{
				responce_result=NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//DIAG_PR_MODE��֧�ֹ���Ѱַ
				return responce_result; 				
			}
			diag.sesion_mode = DIAG_EX_MODE;
		  	diag.security_level1_state = DIAG_SECURITY_LEVEL_STATE_LOCK;
		    diag.security_level3_state = DIAG_SECURITY_LEVEL_STATE_LOCK;	
		   //responce contect
		 	dbg_msg(dev,"$10 03 for PR mode, checkPreconditon sucessed\r\n");
			send_data[0] = 0x06;//����
			send_data[1] = 0x50;
		    send_data[2] = 0x03;//���� ���� 01 �ӷ���diagnostic session type��	    	    
		    send_data[3] = (client_application_layer_tim.P2can&0xFF00)>>8; //��Ÿ��ֽ�
		    send_data[4] = client_application_layer_tim.P2can&0xFF;//���ֽ�
		    send_data[5] =(client_application_layer_tim.P2_asterisk_can&0xFF00)>>8;// 
		    send_data[6] =client_application_layer_tim.P2_asterisk_can&0xFF;//���ֽ� 
		   	can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);
		break;

		case 0x81 :
			if(diag.sesion_mode == DIAG_PR_MODE)
		  	{
		  		responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//����������NRC=22 
				can_send_NRC(DIAG_SESSION_SID,CONDITION_ERR);
				return responce_result;	
			}
		    diag.sesion_mode = DIAG_DE_MODE;//���ý���Ĭ�ϻỰģʽ
			diag.security_level1_state = DIAG_SECURITY_LEVEL_STATE_LOCK;//��ȫ״̬����
		    diag.security_level3_state = DIAG_SECURITY_LEVEL_STATE_LOCK;//��ȫ״̬����
		    diag.dtc_function_enable = DIAG_DTC_FUNCTION_ON;//ʹ��DTC  
		    if(diag.communication_enable == DIAG_COMM_RX_AND_TX_DISABLE)//�ж�ͨ�ſ���ʹ��
		    {
		      diag.communication_enable   = DIAG_COMM_RX_AND_TX_ENABLE;
			//	diag_comm_enable( );//init_can(0);  //test
		    }
			
			dbg_msg(dev, "Enter $10 81 service success,No Positive Response Required\r\n");
			
		break;

		case 0x82 :
			if(diag.address_type == DIAG_F_ADDRESS)//��̻Ự����������Ѱַģʽ		  
			{
				responce_result=NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//DIAG_PR_MODE��֧�ֹ���Ѱַ
				can_send_NRC(DIAG_SESSION_SID,CONDITION_ERR);
				return responce_result;					
			}
//			if(check1_result==0)
//			{
//				dbg_msgv(dev,"Don`t receive $31 01 02 02,\r\n");
//				send_data[0] = 0x04 ;
//				send_data[1] = 0x7f;
//				send_data[2] = 0x10;
//				send_data[3] = 0x02;
//				send_data[4] = 0x22;//condition not correct
//				can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//���ط���Ӧ
//				return NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;
//			}
			dbg_msg(dev,"$10 82 for PR mode, checkPreconditon sucessed\r\n");
			send_data[0] = 0x02;
			send_data[1] = 0x50;
			send_data[2] = 0x02;
	     //	can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//������ģʽ�Ŀ϶���Ӧ
			dev->ipc->app_jump = 1;//������־λ ��1
			process_switch(dev->ipc->boot_entry, dev->ipc->boot_stack);//��ת
		break;

		case 0x83 :
			if(diag.sesion_mode == DIAG_PR_MODE)//	��Ĭ��ģʽת��  ��ȫ״̬�л�������״̬����ͨ�ſ��Ʒ���Ϳ���DTC���÷����״̬����Ӱ��	  
			{
 				diag.security_level1_state=DIAG_SECURITY_LEVEL_STATE_LOCK; //��ȫ״̬����
				diag.security_level3_state=DIAG_SECURITY_LEVEL_STATE_LOCK;//��ȫ״̬����
			}
			 diag.sesion_mode = DIAG_EX_MODE;
		  	diag.security_level1_state = DIAG_SECURITY_LEVEL_STATE_LOCK;
		    diag.security_level3_state = DIAG_SECURITY_LEVEL_STATE_LOCK;	
		   //responce contect
		  	dbg_msg(dev,"$10 83 for PR mode,No Positive Response Required\r\n");
		break;
			
		default:
				dbg_msg(dev,"��֧���ӷ���\r\n");
				responce_result=SUB_ERR;//buffer[0]ֵ����Ӧ01 02 03�ӷ���֧��
			 //	can_send_NRC(DIAG_SESSION_SID,SUB_ERR);
			break;			
	}	
	return responce_result;		
}


//******************************************************************************
// DESCRIPTION: $11  ECUReset  ֧��01 hardReset,03 softReset����Ҫ����Ӧ�ظ�
// ARUGMENT   : bufferΪsid��ʼ�������� lenΪsid��ʼ�����ݳ���
// RETURN     :
// AUTHOR     :
//******************************************************************************
u8 SID_11_fun(device_info_t *dev,u8 *buffer,u16 len)
{
	u8 send_data[8]={0};
	u8 responce_result=NO_ERR;
	
	if(len!=0x01)//�жϳ���
	{
		responce_result=FOR_ERR;
		dbg_msg(dev, "$11 service ����ȷ����Ϣ���ȡ�����\r\n");
		can_send_NRC(DIAG_ECU_RESET_SID,FOR_ERR);
		return  responce_result;
	}

	switch(buffer[0])
	{
		case 0x01:  //hardReset
			if((diag.sesion_mode == DIAG_PR_MODE)&&(diag.address_type==DIAG_F_ADDRESS))
		  	{
		  		responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//����������NRC=22 
				can_send_NRC(DIAG_ECU_RESET_SID,CONDITION_ERR);
				return responce_result;	
			}
			diag.sesion_mode = DIAG_EX_MODE;
			dbg_msg(dev, "HardReset������\r\n");
			send_data[0] = 2;
			send_data[1] = 0x51;//$11���������Ӧ��ʽ
			send_data[2] = 0x01;	
		 	can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);
			mdelay(1000);//�ȴ�����
			/*PMIC��*/
			IO_SET(GPIOA, 3, LOW);
			mdelay(10);
			software_reset(); 
			break;
	
		case 0x03: //softReset	   
			if((diag.sesion_mode == DIAG_PR_MODE)&&(diag.address_type==DIAG_F_ADDRESS))
		  	{
		  		responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//����������NRC=22 
				can_send_NRC(DIAG_ECU_RESET_SID,CONDITION_ERR);
				return responce_result;	
			}	
			dbg_msg(dev, "SoftReset������\r\n");
			send_data[0] = 2;
			send_data[1] = 0x51;//$11���������Ӧ��ʽ
		    send_data[2] = 0x03;
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);	
			IO_SET(GPIOA, 3, LOW);
			mdelay(10);
			software_reset(); // never return 
			break;
			
		case 0x81:	//hardReset		
			diag.sesion_mode = DIAG_EX_MODE;
			dbg_msg(dev, "Enter $11 81 service success,N0 positive response\r\n");
			mdelay(10);
			IO_SET(GPIOA, 3, LOW);
			mdelay(10);
			software_reset(); 	
			break;
			
		case 0x83: //softReset	   
			if(diag.sesion_mode == DIAG_PR_MODE)
			{
				responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//����������NRC=22 
				can_send_NRC(DIAG_SESSION_SID,CONDITION_ERR);
				return responce_result; 
			}
			dbg_msg(dev, "Enter $11 81 service success,N0 positive response\r\n");
			mdelay(10);
			software_reset(); 
		break;
		
		default:
			dbg_msg(dev,"��֧���ӷ���\r\n");
			responce_result=SUB_ERR;//buffer[0]ֵ����Ӧ01 02 03�ӷ���֧��
			can_send_NRC(DIAG_ECU_RESET_SID,SUB_ERR);
		break;			
	}
  return responce_result;
	
}

//******************************************************************************
// DESCRIPTION:$14���� ClearDiagnosticInformation �ӷ���$FFFFFF(All groups)  ���е���λ��� 
// RETURN     :

// AUTHOR     :
//******************************************************************************
u8 SID_14_fun(device_info_t *dev,u8 *buffer,u16 len)
{
    u8 send_data[8] = {0};
    u8 responce_result=NO_ERR;
    if(len != 0x03)             
    {
		responce_result=FOR_ERR;
		dbg_msg(dev, "$14 service ����ȷ����Ϣ���ȡ�����\r\n");
		can_send_NRC(DIAG_CLR_DTC_SID,FOR_ERR);
		return  responce_result;
    }
		
    if((buffer[0]==0xff)&&(buffer[1]==0xff)&&(buffer[2]==0xff))//�ߡ��С���λ��ΪFF ����  
    {
		clear_dtc() ;   // ��� ������ ,��������Ϊ0x00����������
    }
	else
	{
		dbg_msg(dev,"��֧���ӷ���\r\n");
		responce_result=OUTRANGE_ERR;//buffer[0]ֵ����Ӧ01 02 03�ӷ���֧��
		can_send_NRC(DIAG_CLR_DTC_SID,OUTRANGE_ERR);
		return responce_result;
	}
	dbg_msg(dev, "Enter $14 service success,Need positive response\r\n");
	send_data[0] = 0x01;
    send_data[1] = 0x54;  //$14 ����Ӧ�ظ�Value 54 
    can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);

	//save_dtc( );
	dev->ci->write_dtc_flag=1;//дflash��־
	 
    return responce_result;    
}

//******************************************************************************
// DESCRIPTION:$19 read DTC information
// ARUGMENT   : //bufferΪsid��ʼ�������� lenΪsid��ʼ�����ݳ���  
// RETURN     :֧��01 02 03 04 0A
// AUTHOR     :
//******************************************************************************
u8 SID_19_fun(device_info_t *dev,u8 *buffer,u16 len)
{
 	u8 send_data[AVM_DTC_MAX*4+3]={0};  //DTC ��
	u8 responce_result=NO_ERR;
	u8 tester_mask;
	u16 dtc_count=0;
  if(len != 0x02)             
  {
		responce_result=FOR_ERR;
		dbg_msg(dev, "$19 ����ȷ����Ϣ���ȡ�����\r\n");
		can_send_NRC(DIAG_R_DTC_SID,FOR_ERR);
		return  responce_result;
    }
	
	switch(buffer[0])
	{
		case 0x01:		//ͨ��״̬���뱨����������
			tester_mask = buffer[1];
			if(diag.sesion_mode == DIAG_PR_MODE) //��̻Ựʱ  
			{
   			  	responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//����������NRC=22 
				can_send_NRC(DIAG_R_DTC_SID,NOT_SUPPERT_SUB_IN_ACTIVE_SESSION);
				return responce_result;	
			}
		//	dbg_msg(dev, "Enter $19 01 service success,Need positive response\r\n");
			dtc_count = read_dtc_01_02_sub_id(send_data,0x01,tester_mask);
			send_data[0] = 0x06;
			send_data[1] = 0x59;//����Ӧ��ʽ
			send_data[2] = 0x01;//request type
			send_data[3] = ECU_SUPPORT_MSAK;//
			send_data[4] = 0x00;//ISO15031-6DTCformat Ϊ0 DTC_Format_Identifier
			send_data[5] = dtc_count>>8u;//DTC_Count ���ֽ�
			send_data[6] = dtc_count&0x0F;//���ֽ�
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);	
			break;
			
		case 0x02://ͨ��״̬���뱨�������  ��֡����
			tester_mask = buffer[1];
			if(diag.sesion_mode == DIAG_PR_MODE)  //��֧�ֱ��ģʽ
			{
   				responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//����������NRC=22 
				can_send_NRC(DIAG_R_DTC_SID,CONDITION_ERR);
				return responce_result;	
			}
//			dbg_msg(dev, "Enter $19 02 service success,Need positive response\r\n");
			send_data[0] = 0x59;//request type
			send_data[1] = 0x02;//�ӷ���
			send_data[2] =ECU_SUPPORT_MSAK ;//DTCstatusavailabilityMask.
			dtc_count = read_dtc_01_02_sub_id(&send_data[3],0x02,tester_mask);
			dbg_msgv(dev, "dtc_count : %d\r\n", dtc_count);
			can_net_layer_send.len=dtc_count*4+3;
			memcpy(&can_net_layer_send.buff[0],&send_data[0],can_net_layer_send.len);
			//�˴���֡���ͣ�DTC��Ϣ��֡���͡�
			if(can_net_layer_send.len<0x08)
			{
				can_msg_Send_SWJ(dev,AVM_Phy_Resp, (u32*)send_data);
				}
			else{
				diag_send_data(dev,can_net_layer_send.buff,can_net_layer_send.len);
				}
			break;
		
		default:
			dbg_msg(dev,"��֧���ӷ���\r\n");
			responce_result=SUB_ERR;//buffer[0]ֵ����Ӧ01 02 03�ӷ���֧��
			can_send_NRC(DIAG_R_DTC_SID,SUB_ERR);
		break;				
	}

	//��ȡ$19  DTC ��������
  return responce_result;
}


//******************************************************************************
// DESCRIPTION:$22  ͨ����ʶ����ĳ��DID���ݷ��� for app  
// ARUGMENT   : bufferΪsid��ʼ�������� lenΪsid��ʼ�����ݳ��� 
// RETURN     :
// AUTHOR     :
//******************************************************************************
u8 SID_22_fun(device_info_t *dev,u8 *buffer,u16 len)
{
	u8 responce_result=NO_ERR;
	typedef union union60
	{
		u8 b[60];
		u16 h[30];
	}union60_t;
	union60_t  send_did_data={{0}};
	u16 DID = 0;//DID value  2���ֽ�   
	if(len!=0x02)//�жϳ���
	{
		responce_result=FOR_ERR;
		dbg_msg(dev, "$22 service ����ȷ����Ϣ���ȡ�����\r\n");
		can_send_NRC(DIAG_R_DID_SID,FOR_ERR);
		return  responce_result;
	}
 	if(diag.sesion_mode==DIAG_PR_MODE)//��̻Ự��֧��
	{
		responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//����������NRC=22 
		can_send_NRC(DIAG_R_DID_SID,CONDITION_ERR);
		return responce_result;
	}		
	DID = buffer[0]<<8;//DID MSB
	DID += buffer[1];//DID LSB
	switch( DID)
	{	//�ظ���Ϊ>8�ֽ�
		
		case DID_CURRENT_DIAG_SESSION :
			send_did_data.b[0]=get_diag_sesion_mode();
			can_net_layer_send.len = LEN_CURRENT_DIAG_SESSION;//+3 ����22�����DID�����ֽ�
			responce_result=NO_ERR;
		break;

		case DID_Part_Number: // ��Ӧ�̴��� 9���ֽ�
			STMFLASH_Read( ADDR_Part_Number,send_did_data.h,LEN_Part_Number/2+1);
			can_net_layer_send.len = LEN_Part_Number;
			responce_result=NO_ERR;
		break;
		
		case DID_ECUSoftwareVersion : 
			STMFLASH_Read( ADDR_ECUSoftwareVersion,send_did_data.h,LEN_ECUSoftwareVersion/2);
		//	printf("ver=%x-%x\r\n",send_did_data.b[0],send_did_data.b[1]);
			can_net_layer_send.len = LEN_ECUSoftwareVersion;//+3 ����22�����DID�����ֽ�
			responce_result=NO_ERR;
		break;

		case DID_Manufacture_Date: //  4���ֽ�
			STMFLASH_Read( ADDR_Manufacture_Date,send_did_data.h,LEN_Manufacture_Date/2);
			can_net_layer_send.len = LEN_Manufacture_Date;
			responce_result=NO_ERR;
		break;

		case DID_ECUSerialNumber:			
			STMFLASH_Read( ADDR_ECUSerialNumber,send_did_data.h,LEN_SerialNumber/2);
			can_net_layer_send.len = LEN_SerialNumber;
			responce_result=NO_ERR;
		break;
		
		case DID_Supported_units:  // ��Ӧ�̵��ӿ��Ƶ�ԪӲ���汾��  3���ֽ�
			STMFLASH_Read( ADDR_Supported_units,send_did_data.h,LEN_Supported_units);
			can_net_layer_send.len = LEN_Supported_units;
			responce_result=NO_ERR;
		break;
		
		case DID_VINDataIdentifier: // 	
			STMFLASH_Read( ADDR_VINDataIdentifier,send_did_data.h,LEN_VINDataIdentifier/2+1);
			can_net_layer_send.len = LEN_VINDataIdentifier;
			responce_result=NO_ERR;
		break;
		
		case DID_System_name: // 	
			STMFLASH_Read( ADDR_System_name,send_did_data.h,LEN_System_name/2);
			can_net_layer_send.len = LEN_System_name;
			responce_result=NO_ERR;
		break;
		
		case DID_Install_date: // 	
			STMFLASH_Read( ADDR_Install_date,send_did_data.h,LEN_Install_date/2);
			can_net_layer_send.len = LEN_Install_date;
			responce_result=NO_ERR;
		break;
		
		case DID_BAT_Vol: // 	
			send_did_data.b[0]=(u8)dev->adc_t.main_pwr_volt;//B+ zhengshu
			send_did_data.b[1]=(u8)(100*(dev->adc_t.main_pwr_volt-send_did_data.b[0]));//B+ zhengshu
			can_net_layer_send.len = LEN_BAT_Vol;
			responce_result=NO_ERR;
		break;

		case DID_Fingerprint: // 	
			STMFLASH_Read( ADDR_Fingerprint,send_did_data.h,LEN_Fingerprint/2);
			can_net_layer_send.len = LEN_Fingerprint;
			responce_result=NO_ERR;
		break;
		
		case DID_MCU_APP_VER: // 	
			//STMFLASH_Read( ADDR_MCU_APP_VER,send_did_data.h,LEN_MCU_APP_VER/2+1);
		//	dbg_msg(dev, "UDS get app ver\r\n");
			can_net_layer_send.len = LEN_MCU_APP_VER+3;
			can_net_layer_send.buff[0] = can_net_layer_send.len;
			can_net_layer_send.buff[1] = 0x62;
			can_net_layer_send.buff[2] = buffer[0];
			can_net_layer_send.buff[3] = buffer[1];
			can_net_layer_send.buff[4] = (get_app_version()>>16)&0xff;
			can_net_layer_send.buff[5] = (get_app_version()>>8)&0xff;
			can_net_layer_send.buff[6] = get_app_version()&0xff;
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp, (u32*)can_net_layer_send.buff);
			responce_result=NO_ERR;
			return 1;
		
		case DID_MCU_BOOT_VER: // 	
			//STMFLASH_Read( ADDR_MCU_BOOT_VER,send_did_data.h,LEN_MCU_BOOT_VER/2+1);
		//	dbg_msg(dev, "UDS get boot ver\r\n");
			can_net_layer_send.len = LEN_MCU_BOOT_VER+3;
			can_net_layer_send.buff[0] = can_net_layer_send.len;
			can_net_layer_send.buff[1] = 0x62;
			can_net_layer_send.buff[2] = buffer[0];
			can_net_layer_send.buff[3] = buffer[1];
			can_net_layer_send.buff[4] = (get_boot_version()>>16)&0xff;
			can_net_layer_send.buff[5] = (get_boot_version()>>8)&0xff;
			can_net_layer_send.buff[6] = get_boot_version()&0xff;
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp, (u32*)can_net_layer_send.buff);
			responce_result=NO_ERR;
			return 1;
		
		case DID_ARM_APP_VER: // 	
			//STMFLASH_Read( ADDR_ARM_APP_VER,send_did_data.h,LEN_ARM_APP_VER/2+1);
			comm_message_send(dev, CM_ARM_APP_VERSION, 0, NULL, 0);
			//can_net_layer_send.len = LEN_ARM_APP_VER;
			responce_result=NO_ERR;
			return 1;
//		break;
		
		case DID_ARM_SF_VER: // 	
			//STMFLASH_Read( ADDR_ARM_SF_VER,send_did_data.h,LEN_ARM_SF_VER/2+1);
			comm_message_send(dev, CM_ARM_ARI_VERSION, 0, NULL, 0);
		//	can_net_layer_send.len = LEN_ARM_SF_VER;
			responce_result=NO_ERR;
		return 1;
//		break;

		case DID_Supplier_ID: // 	
			STMFLASH_Read( ADDR_Supplier_ID,send_did_data.h,LEN_Supplier_ID/2);
			can_net_layer_send.len = LEN_Supplier_ID;
			responce_result=NO_ERR;
		break;
				
		default:
			dbg_msg(dev,"$22��֧�ֵ�DID\r\n");
			responce_result=OUTRANGE_ERR;//buffer[0]ֵ����Ӧ01 02 03�ӷ���֧��
			can_send_NRC(DIAG_ECU_RESET_SID,OUTRANGE_ERR);
		break;
		}
		if(responce_result==NO_ERR)
		{	
			memset(&can_net_layer_send.buff[0],0,CAN_NET_LAYER_SEND_BUFF_MAX);
			can_net_layer_send.len +=3;	
			if(can_net_layer_send.len>=8)
			{
				can_net_layer_send.buff[0] = 0x62;
				can_net_layer_send.buff[1] = buffer[0];
				can_net_layer_send.buff[2] = buffer[1];
				memcpy(&can_net_layer_send.buff[3],send_did_data.b,can_net_layer_send.len);
				diag_send_data(dev,can_net_layer_send.buff, can_net_layer_send.len);//��֡����
				}
			else
			{
				can_net_layer_send.buff[0] = can_net_layer_send.len;
				can_net_layer_send.buff[1] = 0x62;
				can_net_layer_send.buff[2] = buffer[0];
				can_net_layer_send.buff[3] = buffer[1];
				memcpy(&can_net_layer_send.buff[4],send_did_data.b,can_net_layer_send.len-3);
				can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp, (u32*)can_net_layer_send.buff);
				}
		}
	return  responce_result;
}

//******************************************************************************
// DESCRIPTION:$27 SecurityAccess 01RequestSeed 02Sendkey ��07 08��Ӧ
// ARUGMENT   : //bufferΪsid��ʼ�������� lenΪsid��ʼ�����ݳ���  
// RETURN     :��֪����ȫ�㷨Ϊ��
// AUTHOR     :
//******************************************************************************

u8 SID_27_fun(device_info_t *dev,u8 *buffer,u16 len)
{
	u8 send_data[8]={0};
	u8 responce_result=NO_ERR;
	unsigned int v[2],key,PassWord;
	switch(buffer[0])
	{
	//for App Seed and key
	case 0x01:	//app���� ��01 02 ����չ�Ự ����Ѱַ�� ����			  	   							   						
			//���ȺϷ����ж�
		if(len!=0x01)//�жϳ���
		{
			responce_result=FOR_ERR;
			//dbg_msg(dev, "$27 service ����ȷ����Ϣ���ȡ�����\r\n");
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,FOR_ERR);
			return  responce_result;
			}  
		if((diag.sesion_mode ==DIAG_DE_MODE)||(diag.address_type ==DIAG_F_ADDRESS ))    
		{
			responce_result = CONDITION_ERR;//����������NRC=22 
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,CONDITION_ERR);
			return responce_result; 
			}
		if((diag.SAFC_1>=2)&&(diag.SAFC_reach_3num_mcu_resetbit == DIAG_ECU_HAD_NOT_RESET))                     
		{
			responce_result = DELAY_NOT_ARRIVAL_ERR;//
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,DELAY_NOT_ARRIVAL_ERR);
			return responce_result; 
			}
		if(diag.security_level1_state == DIAG_SECURITY_LEVEL_STATE_UNLOCK)  //if having been unlock,send datas 00 00 00 00���Ѿ�������ȫ����
		{	
			send_data[0] = 0x06;
            send_data[1] = 0x67;//����Ӧ��ʽ
            send_data[2] = 0x01;//����Ӧ��ʽ
            can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)send_data);
		}
		else
		{
         	diag.security_seed=create_seed();
			send_data[0] = 0x06;//����
			send_data[1] = 0x67;
            send_data[2] = 0x01;
			//4���ֽڵ����ӷ���send_data
            send_data[3] = diag.security_seed>>24;
            send_data[4] = (diag.security_seed >>16)&0xFF;
            send_data[5] = (diag.security_seed>>8)&0xFF;
            send_data[6] = diag.security_seed &0xFF;
            can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)send_data);//����seed
            diag.security_level1_access_step = DIAG_HAD_REQ_SEED;//ȷ�ϰ�ȫ���ʲ���
		}			
		break;

	case 0x03:	//app���� ��01 02 ����չ�Ự ����Ѱַ�� ����			  	   							   						
			//���ȺϷ����ж�
		if(len!=0x01)//�жϳ���
		{
			responce_result=FOR_ERR;
			//dbg_msg(dev, "$27 service ����ȷ����Ϣ���ȡ�����\r\n");
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,FOR_ERR);
			return  responce_result;
			}  
		if((diag.sesion_mode ==DIAG_DE_MODE)||(diag.address_type ==DIAG_F_ADDRESS )  )    
		{
			responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//����������NRC=22 
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,CONDITION_ERR);
			return responce_result; 
			}
		if((diag.SAFC_3>=2)&&(diag.SAFC_reach_3num_mcu_resetbit == DIAG_ECU_HAD_NOT_RESET))                     
		{
			responce_result = DELAY_NOT_ARRIVAL_ERR;//
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,DELAY_NOT_ARRIVAL_ERR);
			return responce_result; 
			}
		if(diag.security_level3_state == DIAG_SECURITY_LEVEL_STATE_UNLOCK)  //if having been unlock,send datas 00 00 00 00���Ѿ�������ȫ����
		{	
			send_data[0] = 0x06;
            send_data[1] = 0x67;//����Ӧ��ʽ
            send_data[2] = 0x03;
            can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)send_data);
		}
		else
		{
         	diag.security_seed=create_seed();
			send_data[0] = 0x06;//����
			send_data[1] = 0x67;
            send_data[2] = 0x03;
			//4���ֽڵ����ӷ���send_data
            send_data[3] = diag.security_seed>>24;
            send_data[4] = (diag.security_seed >>16)&0xFF;
            send_data[5] = (diag.security_seed>>8)&0xFF;
            send_data[6] = diag.security_seed &0xFF;
            can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)send_data);//����seed
            diag.security_level3_access_step = DIAG_HAD_REQ_SEED;//ȷ�ϰ�ȫ���ʲ���
		}			
		break;
		
	case 0x02://���� key  
		if((diag.sesion_mode ==DIAG_DE_MODE)||(diag.sesion_mode ==DIAG_PR_MODE)||(diag.address_type ==DIAG_F_ADDRESS )	)	 
		{
			responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//����������NRC=22 
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,CONDITION_ERR);
			return responce_result; 
			}
		if((diag.security_level1_state == DIAG_SECURITY_LEVEL_STATE_UNLOCK)||(diag.security_level1_access_step != DIAG_HAD_REQ_SEED))    
			{
			responce_result = SEQUENCE_ERROR;//����������NRC=22 
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,SEQUENCE_ERROR);
			return responce_result; 
			}
		if(diag.security_level3_state == DIAG_SECURITY_LEVEL_STATE_UNLOCK)  //if having been unlock,send datas 00 00 00 00���Ѿ�������ȫ����
		{	
			send_data[0] = 0x06;
            send_data[1] = 0x67;//����Ӧ��ʽ
            send_data[2] = 0x02;
            can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)send_data);	
			break ;
		}

		else
			diag.security_level1_access_step = DIAG_NOT_REQ_SEED; 
		//obtain the key from tester��ȡKey from �����
		PassWord = buffer[1]<<24;
		PassWord = PassWord+(buffer[2]<<16);
		PassWord = PassWord+(buffer[3]<<8);
		PassWord = PassWord+buffer[4];
		v[0]=diag.security_seed;
		v[1]=~v[0]; 
		get_key_level1();
		key=encipher(2,v,key1);
		//dbg_msgv(dev,"Security_key = %08x\r\n",Security_key[0]);
		//compare the PassWord with Security_key
		if(PassWord^key)  
		{ 
            diag.SAFC_1++;
            if(diag.SAFC_1 <= 2)
            { 
                if(diag.SAFC_1==2)
                {
                    diag.time_SAFC_1 = 0; //OSTimeGet(&os_err); //test
                    diag.SAFC_reach_3num_mcu_resetbit = DIAG_ECU_HAD_NOT_RESET;
					 responce_result=OVER_TRY_ERR;
                }
                else
                {
                     responce_result=INVALID_KEY_ERR;
                }
            }
            else
            {  
                diag.SAFC_1 = 2;
                responce_result=OVER_TRY_ERR;
            }
			can_send_NRC(DIAG_SECURITY_ACCESS_SID , responce_result);
			return responce_result;
		}
		
		else
		{
			diag.SAFC_reach_3num_mcu_resetbit = DIAG_ECU_HAD_NOT_RESET;
			diag.SAFC_1 = 0;
			diag.security_level1_state = DIAG_SECURITY_LEVEL_STATE_UNLOCK; //ƥ��������־λ�����ͽ����־λ
		}
		send_data[0] = 0x02;
		send_data[1] = 0x67;
		send_data[2] = 0x02;
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)send_data);	
		diag.security_level1_access_step = DIAG_NOT_REQ_SEED;//�ڶ�����������־λ����
		break;

	case 0x04://���� key  
		if((diag.sesion_mode ==DIAG_DE_MODE)||(diag.address_type ==DIAG_F_ADDRESS )	)	 
		{
			responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//����������NRC=22 
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,CONDITION_ERR);
			return responce_result; 
			}
		if(diag.security_level3_state == DIAG_SECURITY_LEVEL_STATE_UNLOCK)  //if having been unlock,send datas 00 00 00 00���Ѿ�������ȫ����
		{	
			send_data[0] = 0x06;
            send_data[1] = 0x67;//����Ӧ��ʽ
            send_data[2] = 0x04;
            can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)send_data);
			break ;
		}

		else
			diag.security_level3_access_step = DIAG_NOT_REQ_SEED; 
		//obtain the key from tester��ȡKey from �����
		PassWord = buffer[1]<<24;
		PassWord = PassWord+(buffer[2]<<16);
		PassWord = PassWord+(buffer[3]<<8);
		PassWord = PassWord+buffer[4];
		v[0]=diag.security_seed;
		v[1]=~v[0];
		get_key_level2();
	//	printf("%x~%x~%x~%x\r\n",key2[0],key2[1],key2[2],key2[3]);
		key=encipher(2,v,key2);
		dbg_msgv(dev,"Security_key = %08x,PassWord= %08x\r\n",key,PassWord);
		//compare the PassWord with Security_key
		if(PassWord^key)  
		{ 
            diag.SAFC_3++;
            if(diag.SAFC_3 <= 2)
            { 
                if(diag.SAFC_3==2)
                {
                    diag.time_SAFC_3 = 0; //OSTimeGet(&os_err); //test
                    diag.SAFC_reach_3num_mcu_resetbit = DIAG_ECU_HAD_NOT_RESET;
					 responce_result=OVER_TRY_ERR;
                }
                else
                {
                     responce_result=INVALID_KEY_ERR;
                }
            }
            else
            {  
                diag.SAFC_3 = 2;
                responce_result=OVER_TRY_ERR;
            }
			can_send_NRC(DIAG_SECURITY_ACCESS_SID , responce_result);
			return responce_result;
		}
		
		else
		{
			diag.SAFC_reach_3num_mcu_resetbit = DIAG_ECU_HAD_NOT_RESET;
			diag.SAFC_3 = 0;
			diag.security_level3_state = DIAG_SECURITY_LEVEL_STATE_UNLOCK; //ƥ��������־λ�����ͽ����־λ
		}
		send_data[0] = 0x02;
		send_data[1] = 0x67;
		send_data[2] = 0x04;
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)send_data);	
		diag.security_level3_access_step = DIAG_NOT_REQ_SEED;//�ڶ�����������־λ����
		break;
	default:
		responce_result=SUB_ERR;
		can_send_NRC(DIAG_SECURITY_ACCESS_SID , responce_result);
		break;			
	}
	return responce_result;
}



//******************************************************************************
// DESCRIPTION:
// ARUGMENT   : bufferΪsid��ʼ�������� lenΪsid��ʼ�����ݳ���  
// RETURN     :
// AUTHOR     :
//******************************************************************************
uint8_t   SID_28_fun(device_info_t *dev,uint8_t *buffer,uint16_t len)
{
	uint8_t send_data[8]={0};
	uint8_t responce_result=NO_ERR;
	//���ȺϷ����ж�
	if(len!=0x02)
	{
		responce_result=FOR_ERR;  
		can_send_NRC(DIAG_COMM_CONTROL_SID,FOR_ERR);
		return responce_result; 
	}

	if(diag.sesion_mode!=DIAG_EX_MODE)
	{
		//can_send_NRC(DIAG_COMM_CONTROL_SID,NOT_SUPPERT_SUB_IN_ACTIVE_SESSION);
		return (NOT_SUPPERT_SUB_IN_ACTIVE_SESSION);      			
		}
	
	switch(buffer[0])
	{
		case 0x00:				    
			if(buffer[1]!= 0x01)
			{  
				can_send_NRC(DIAG_COMM_CONTROL_SID,OUTRANGE_ERR);
				return (OUTRANGE_ERR);				
			}	
			send_data[0] = 0x02;
			send_data[1] = 0x68;	
			send_data[2] = DIAG_COMM_RX_AND_TX_ENABLE;
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);		   	 
			diag.communication_enable = DIAG_COMM_RX_AND_TX_ENABLE;								   
			break;
			
		case 0x01:
			if(buffer[1]!= 0x01)
			{    
				can_send_NRC(DIAG_COMM_CONTROL_SID,OUTRANGE_ERR);
				return (OUTRANGE_ERR);				
			}				  	  	
			send_data[0] = 0x02;
			send_data[1] = 0x68;	
			send_data[2] = DIAG_ENABLE_RX_AND_TX_DISABLE;
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);		   	 
			diag.communication_enable = DIAG_ENABLE_RX_AND_TX_DISABLE;					
			//diag_comm_enable( );//test			   
			break;	
		case 0x03:		 
			if(buffer[1]!= 0x03)
			{    
				can_send_NRC(DIAG_COMM_CONTROL_SID,OUTRANGE_ERR);
				return (OUTRANGE_ERR);				
			}										
			send_data[0] = 0x02;
			send_data[1] = 0x68;
			send_data[2] = DIAG_COMM_RX_AND_TX_DISABLE;
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);  
			diag.communication_enable = DIAG_COMM_RX_AND_TX_DISABLE;						
			break;  

		default:
		//	can_send_NRC(DIAG_COMM_CONTROL_SID,SUB_ERR);
			responce_result=SUB_ERR;
			break;			
	}
	return responce_result;
	}

u8 SID_31_fun(device_info_t *dev,u8 *buffer,u16 len)  
{
	u8 send_data[8]={0};
	u8 responce_result=NO_ERR;	
	u16 routine_id;//�������� ID
	u8 Erase_result,routine_control_type;//routine_control_type=start routine(01�ڶ����ֽ�)
   //���ݳ����ж�  ���ȱ������3����ͬ�ӷ������ݳ��Ȳ�һ��
	if(len<3)
	{
		responce_result=FOR_ERR; 
		can_send_NRC(DIAG_ROUTE_CONTROL_SID,FOR_ERR);
		return responce_result; 
	}
	routine_control_type=buffer[0];	                	
	routine_id=buffer[1];
	routine_id=(routine_id<<8)+buffer[2];
	
	if((routine_control_type== 0x01)&&(routine_id==0xFF00)&&(buffer[3]==0xff))//31  01�ӷ���
	{
		send_data[0]=0x03;
		send_data[1]=0x7F;
		send_data[2]=0x31;//Routine Control Type
		send_data[3]=0x78;
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//����78��ʾ����ǵȴ�����Ӧ
		//
		Erase_result = upgrade_earseMemory(dev);
		send_data[0]=0x05;
		send_data[1]=0x71;
		send_data[2]=0x01;//Routine Control Type
		send_data[3]=0xff;
		send_data[4]=0x00;
		send_data[5]=Erase_result;//�ɹ�01 ʧ��02
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);
		}
	else if((routine_control_type== 0x01)&&(routine_id==0xff00)&&(buffer[3]==0x01))//����boot
	{
		dbg_msg(dev,"uds upgrade mcu_boot,jump to app\r\n");
		send_data[0]=0x03;
		send_data[1]=0x7F;
		send_data[2]=0x31;//Routine Control Type
		send_data[3]=0x78;
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//����78��ʾ����ǵȴ�����Ӧ
		//Erase_result = upgrade_earseMemory(dev);
		
		dev->ipc->upgrade_mcu_boot = 1;//����mcu_boot 	
		systick_stop();
		device_deinit(dev);
		__disable_irq();
		process_switch(dev->ipc->app_entry, dev->ipc->app_stack);//��ת
		return 1;
		}
	else if((routine_control_type== 0x01)&&(routine_id==0xff01)&&(buffer[3]==0xff))
	{
		send_data[0]=0x03;
		send_data[1]=0x7F;
		send_data[2]=0x31;//Routine Control Type
		send_data[3]=0x78;
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//����78��ʾ����ǵȴ�����Ӧ
		upgrade_loadFile_check1(dev,&buffer[0],len);
		}
	else 
	{
		can_send_NRC(DIAG_ROUTE_CONTROL_SID,CONDITION_ERR);
		return responce_result=CONDITION_ERR;
		}
	return responce_result;
}
//******************************************************************************
// DESCRIPTION:
// ARUGMENT   : bufferΪsid��ʼ�������� lenΪsid��ʼ�����ݳ���  
// RETURN     :
// AUTHOR     :
//******************************************************************************
u8 SID_2E_fun(device_info_t *dev,uint8_t *buffer,uint16_t len)
{
	u8 data[LEN_DID_MAX]={0};
	u8 send_data[8]={0};
	u8 responce_result=NO_ERR;	
	u16 DID;
	u32 addr,LEN;
	DID=buffer[0];
	DID=(DID<<8)+buffer[1]; //  
	
	if(diag.address_type  == DIAG_F_ADDRESS)  
	{
		can_send_NRC(DIAG_W_DID_SID, NOT_SUPPERT_SUB_IN_ACTIVE_SESSION);
		return (NOT_SUPPERT_SUB_IN_ACTIVE_SESSION); 
		}
	if(diag.security_level3_state != DIAG_SECURITY_LEVEL_STATE_UNLOCK)//�谲ȫ����
	{	
		//can_send_NRC(DIAG_W_DID_SID,SECURITY_ERR );
	//	return (SECURITY_ERR);    
		}
	memcpy(data,&buffer[2],len-2);//did����Ҫд�������
	switch(DID)
	{
		case DID_Part_Number:
			//dbg_msg(dev,"write  part number\r\n");
			addr=ADDR_Part_Number;
			LEN=LEN_Part_Number/2+1;
			//STMFLASH_Write(ADDR_Part_Number,(u16*)data,LEN_Part_Number/2+1);
			break;
		case DID_ECUSoftwareVersion:
			addr=ADDR_ECUSoftwareVersion;
			LEN=LEN_ECUSoftwareVersion/2;
			//STMFLASH_Write(ADDR_ECUSoftwareVersion,(u16*)data,LEN_ECUSoftwareVersion/2);
			break;	
		case DID_Supported_units:
			addr=ADDR_Supported_units;
			len=LEN_Supported_units/2+1;
		//	STMFLASH_Write(ADDR_Supported_units,(u16*)data,LEN_Supported_units/2+1);
			break;
		case DID_VINDataIdentifier:
			addr=ADDR_VINDataIdentifier;
			LEN=LEN_VINDataIdentifier/2+1;
		//	STMFLASH_Write(ADDR_VINDataIdentifier,(u16*)data,LEN_VINDataIdentifier/2+1);
			break;
		case DID_System_name:
			addr=ADDR_System_name;
			LEN=LEN_System_name/2;

	//		STMFLASH_Write(ADDR_System_name,(u16*)data,LEN_System_name/2);
			break;
		case DID_Install_date:
			addr=ADDR_Install_date;
			LEN=LEN_Install_date/2;
		//	STMFLASH_Write(ADDR_Install_date,(u16*)data,LEN_Install_date/2);
			break;
		case DID_Fingerprint:
			addr=ADDR_Fingerprint;
			LEN=LEN_Fingerprint/2;
			//dbg_msg(dev,"write time\r\n");
		//	STMFLASH_Write(ADDR_Fingerprint,(u16*)data,LEN_Fingerprint/2);
			break;
		case DID_MCU_APP_VER:
			addr=ADDR_MCU_APP_VER;
			LEN=2;
		//	STMFLASH_Write(ADDR_MCU_APP_VER,(u16*)data,2);
			break;
		case DID_MCU_BOOT_VER:
			addr=ADDR_MCU_BOOT_VER;
			LEN=2;
		//	STMFLASH_Write(ADDR_MCU_BOOT_VER,(u16*)data,2);
			break;
		case DID_ARM_APP_VER:
			addr=ADDR_ARM_APP_VER;
			LEN=2;
		//	STMFLASH_Write(ADDR_ARM_APP_VER,(u16*)data,2);
			break;
		case DID_ARM_SF_VER:
			addr=ADDR_ARM_SF_VER;
			LEN=2;
		//	STMFLASH_Write(ADDR_ARM_SF_VER,(u16*)data,2);
			break;
		default:
			responce_result=SUB_ERR;//buffer[0]ֵ����Ӧ01 02 03�ӷ���֧��
			can_send_NRC(DIAG_W_DID_SID,SUB_ERR);
			return responce_result;
			}

	memcpy(dev->did_t.buf,&buffer[2] ,2*LEN);
	dev->did_t.LEN =LEN;
	dev->did_t.ADDR = addr;
			
	send_data[0] = 0x03;
	send_data[1] = 0x6E;
	send_data[2] = buffer[0];
	send_data[3] = buffer[1];
	can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//�ӳٵ���־λΪ1��д���ȱ���DID�ͳ���
	if(dev->ci->write_did_flag)//����дdid
	{
	//	dbg_msg(dev, "time is right \r\n");
		STMFLASH_Write(dev->did_t.ADDR,(u16*)&buffer[2],dev->did_t.LEN);
		}
	else {	

		dev->did_t.flag = 1;
		}

	return responce_result;
}


/*****************************************************************************
* Description: 
* Argument   :  for ARM  upgrade  from  can 
* Return     :
* Author     :  20180706
*****************************************************************************/
//uint8_t RequestDownload(uint8_t *buffer, uint16_t len)
u8 SID_34_fun(device_info_t *dev,uint8_t *buffer, uint16_t len)
{
	u8 update_type;
	//if((diag.sesion_mode != DIAG_PR_MODE)||(diag.address_type != DIAG_P_ADDRESS))
	//{
	//	can_send_NRC(DIAG_REQUEST_DOWNLOAD_SID,NOT_SUPPERT_SUB_IN_ACTIVE_SESSION);
	//	return  0;
	//}
	
	if ( diag.security_level3_state != DIAG_SECURITY_LEVEL_STATE_UNLOCK )
	{
	    can_send_NRC(DIAG_REQUEST_DOWNLOAD_SID,SECURITY_ERR);
		return  0;
	}
	diag.file_trans_mode = 0x02; //����ģʽ
	update_type = buffer[10];//��������
	if((update_type==0x10u)||(update_type==0x20u))
	{
		dbg_msg(dev,"upgrade mcu\r\n");	
		upgrade_requestFileDownload(dev,buffer, len);//����������������������ݳ���
		}
	else
	{	
		dbg_msg(dev,"��������ARM��������\r\n");
		upgrade_requestFileDownload_ARM(dev,buffer, len);//����������������������ݳ���
		comm_message_send(dev,CM_UPDATE_ARM_READY, 0, NULL , 0);
		/*��ʼ��ʱ���೤ʱ��δ�յ�ARM��Ӧ����ʾ����ʧ��*/
		dev->ipc->usart_normal = 0;
		start_timer(TIMER_DEV1,5000);
		}
		return NO_ERR;
}


//******************************************************************************
// DESCRIPTION:
// ARUGMENT   : //bufferΪsid��ʼ�������� lenΪsid��ʼ�����ݳ���  
// RETURN     :
// AUTHOR     :
//******************************************************************************
u8 SID_36_fun(device_info_t *dev,u8 *buffer,u16 len)
{
	u8 send_data[8]={0};
    u8 responce_result=NO_ERR;
	//if((diag.sesion_mode != DIAG_PR_MODE)|(diag.address_type != DIAG_P_ADDRESS))
	//{
	//	can_send_NRC(DIAG_TRANSFER_DATA_SID,NOT_SUPPERT_SUB_IN_ACTIVE_SESSION);
	//	return  0;
	//}
	
	if ( diag.security_level3_state != DIAG_SECURITY_LEVEL_STATE_UNLOCK )
	{
	    can_send_NRC(DIAG_TRANSFER_DATA_SID,SECURITY_ERR);
		return  0;
	}

 	if(diag.file_trans_mode==0x02)
	{
		//�Ӳ���02Ϊ����ָ��
		send_data[0]=0x03;
		send_data[1]=0x7F;
		send_data[2]=0x36;//Routine Control Type
		send_data[3]=0x78;
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//����78��ʾ����ǵȴ�����Ӧ
		upgrade_transferData(dev,buffer,len);
	}
	else
	{
		can_send_NRC(DIAG_TRANSFER_DATA_SID,SUB_ERR);
		responce_result=SUB_ERR;
	}
	return responce_result;
}

uint8_t SID_37_fun(device_info_t *dev,uint8_t *buffer, uint16_t len)
{
	u8 send_data[8] = {0x03,0x7F,0x37,0x22};
//	if((diag.sesion_mode != DIAG_PR_MODE)|(diag.address_type != DIAG_P_ADDRESS))
	//{
	//	can_send_NRC(DIAG_REQUEST_TRANSFER_EXIT_SIG,NOT_SUPPERT_SUB_IN_ACTIVE_SESSION);
	//	return  0;
//	}
	if ( diag.security_level3_state != DIAG_SECURITY_LEVEL_STATE_UNLOCK )
	{
	    can_send_NRC(DIAG_REQUEST_TRANSFER_EXIT_SIG,SECURITY_ERR);
		return  0;
	}
	
	if(diag.file_trans_mode == 0x02)
	{
		upgrade_requestTransferExit(dev, buffer, len);
		dbg_msg(dev,"���ݴ�����ɣ��˳��������\r\n");
		}
	else 
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);
	return NO_ERR;
}


//******************************************************************************
// DESCRIPTION:
// ARUGMENT   : //bufferΪsid��ʼ�������� lenΪsid��ʼ�����ݳ���  
// RETURN     :
// AUTHOR     :
//******************************************************************************
u8 SID_3E_fun(device_info_t *dev,uint8_t *buffer,uint16_t len)
{
	u8 send_data[8]={0};
	u8 responce_result=NO_ERR;
	//���ȺϷ����ж�
	if(len!=0x01)//����Ϊ1
	{
		responce_result=FOR_ERR; 
		can_send_NRC(DIAG_TESTER_PRESENT_SID,FOR_ERR);
		return responce_result; 
	}
	dbg_msg(dev, "Enter $3E service success\r\n");

	switch(buffer[0])
	{
		case 0x00:	//���������
			send_data[0] = 0x02;
			send_data[1] = 0x7E;
			send_data[2] = 0x00;
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);	  				
			break;
		case 0x80://ʲô�����÷���
			  //return (NO_ERR);//no need response to the tester 		   	    	   
			break;		
		default:
			responce_result=SUB_ERR;
			//can_send_NRC(DIAG_TESTER_PRESENT_SID,SUB_ERR); 
			break;			
	}
	return responce_result;
}

//******************************************************************************
// DESCRIPTION: bufferΪsid��ʼ�������� lenΪsid��ʼ�����ݳ���  
// ARUGMENT   :DTC���� EX�Ựģʽ
// RETURN     :
// AUTHOR     :
//******************************************************************************
u8 SID_85_fun(device_info_t *dev,u8 *buffer,u8 len)
{
	u8 send_data[8]={0};
	u8 responce_result=NO_ERR;
	//���ȺϷ����ж�
	if(len!=0x01)
	{
	    responce_result=FOR_ERR; 
		can_send_NRC(DIAG_DTC_CONTTOL_SID,FOR_ERR);
	    return responce_result; 
		}
	if(diag.sesion_mode==DIAG_PR_MODE)
	{
		can_send_NRC(DIAG_DTC_CONTTOL_SID,CONDITION_ERR);
		return (NOT_SUPPERT_SUB_IN_ACTIVE_SESSION);	  
		}
	dbg_msg(dev, "Enter $85 service success\r\n");

	switch(buffer[0])
	{
		case 0x01:	//on 		
			diag.dtc_function_enable = DIAG_DTC_FUNCTION_ON;	
			send_data[0] = 0x02;
			send_data[1] = 0xC5;
			send_data[2] = 0x01;
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);		  			
			break;
		case 0x02://			
			diag.dtc_function_enable = DIAG_DTC_FUNCTIONOFF;
			send_data[0] = 0x02;
			send_data[1] = 0xC5;
			send_data[2] = 0x02;
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);		  						
			break;
		case 0x81:	//on 		
			diag.dtc_function_enable = DIAG_DTC_FUNCTION_ON;	
			break;
		case 0x82://			
			diag.dtc_function_enable = DIAG_DTC_FUNCTIONOFF;		  						
			break;
						
		default:
			responce_result=SUB_ERR;
	//		can_send_NRC(DIAG_TESTER_PRESENT_SID,SUB_ERR); 
			break;			
	}
	return responce_result;
}


/*
*----------------------------------------------------------------------------------------------------------------------------------------
* ����ԭ�ͣ�void app_diagnose(void)
* �������ܣ���ʼ��diag��ز���
  �������˵����  buffer:sid(��)��ʼ��������һ�����ݣ�len:sid(��)�������ܳ���
* �� �� ֵ����
  ���ߣ�    qdh
*----------------------------------------------------------------------------------------------------------------------------------------
*/
u32 app_diagnose(device_info_t *dev,uint8_t *buffer,uint16_t len)
{	
	uint8_t sid=buffer[0];              //sid  ����ID
	uint8_t response_result=NO_ERR;     //��ʼ ��ȷ���
//	uint8_t send_data[7];	
	switch(sid)  //buffer[1]Ϊsid��ʼ��������
    {
	    case DIAG_SESSION_SID:  //�Ựģʽ  10
	        response_result=SID_10_fun(dev,&buffer[1],len-1);                         
	        break;
	    case DIAG_ECU_RESET_SID: //��λ  11
	    	response_result=SID_11_fun(dev,&buffer[1],len-1);                                          
	        break;
	    case DIAG_CLR_DTC_SID:   //�������  14
	        response_result=SID_14_fun(dev,&buffer[1],len-1);                                                           
	        break;
	    case DIAG_R_DTC_SID:  //��DTC
	        response_result=SID_19_fun(dev,&buffer[1],len-1);            	            	
	        break;
	    case DIAG_R_DID_SID:   //����ʶ�� 
	        response_result=SID_22_fun(dev,&buffer[1],len-1);       
	        break;
	    case DIAG_SECURITY_ACCESS_SID:  //��ȫ����  
	        response_result=SID_27_fun(dev,&buffer[1],len-1);                                  	               	   
	        break;
	    case DIAG_COMM_CONTROL_SID: //ͨ�ſ���
	        response_result=SID_28_fun(dev,&buffer[1],len-1);                                  	               	               	            	 
	        break;
	    case  DIAG_W_DID_SID://д��ʶ��:  
			response_result=SID_2E_fun(dev,&buffer[1],len-1);      
	        break;               
	    case  DIAG_R_DIR_DID://��ȡ�ļ�Ŀ¼�б�
	       // response_result = SID_38_fun(&buffer[1],len-1);

	        break;
	    case DIAG_TRANSFER_DATA_SID :
	        response_result = SID_36_fun(dev,&buffer[0], len);
	        break;
	    case DIAG_TESTER_PRESENT_SID:
	        response_result = SID_3E_fun(dev,&buffer[1],len-1);            	
	        break;
	    case DIAG_DTC_CONTTOL_SID:
	        response_result = SID_85_fun(dev,&buffer[1],len-1);            	            	            
	        break;			
	    case DIAG_ROUTE_CONTROL_SID:   //���̿���
	         response_result = SID_31_fun(dev,&buffer[1],len-1); 
	        break;
	    case DIAG_REQUEST_DOWNLOAD_SID:
	       	 response_result = SID_34_fun(dev,&buffer[1], len-1);
	        break;
	    case DIAG_REQUEST_TRANSFER_EXIT_SIG:
	       	 response_result = SID_37_fun(dev,&buffer[1], len-1);
	        break;

	    default:
	   		dbg_msg(dev, "SID is not support������\r\n");
	        response_result = SID_ERR;//����֧��
			can_send_NRC( sid, SID_ERR); 
	        break;
    }                	
	return response_result;
}

/*
 *@brief :   Ӧ�ó��������ر���ִ��Ԥ��̻������
*autor:  xz  
*Time �� 20180524
*/
void check_s3_client(device_info_t *dev)
{
	static u8 current_sts=0; 
	if((get_diag_sesion_mode()!=DIAG_DE_MODE)&&(current_sts==0))//����Ĭ�ϻỰ���򿪶�ʱ��5s
	{
		current_sts=1;
		dev->diag_t.diag_rec_sts=0;
		stimer_start(dev, FLAG_S3_CLIENT,TO_CNT_5S);
		}

	if((dev->diag_t.diag_rec_sts==1)&&(current_sts==1))
	{
		current_sts=0;
		dev->diag_t.diag_rec_sts=0;
	//	dbg_msg(dev,"reset S3_clien timer\r\n");
		stimer_stop(dev, FLAG_S3_CLIENT);
		stimer_start(dev, FLAG_S3_CLIENT,TO_CNT_5S);
		}

	if(get_flag(dev, FLAG_S3_CLIENT))
	{
		stimer_stop(dev, FLAG_S3_CLIENT);
		current_sts=0;
		dev->diag_t.diag_rec_sts=0;
		set_diag_session(DIAG_DE_MODE);
		dbg_msg(dev,"back to de_session\r\n");
		}
	}

