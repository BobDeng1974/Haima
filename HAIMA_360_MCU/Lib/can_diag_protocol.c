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

CAN_DIAG_TYPE diag; //diag 相关项结构体
SERVER_APPLICATION_LAYER_TIM_TYPE   server_application_layer_tim;//应用层时间参数
CLIENT_APPLICATION_LAYER_TIM_TYPE   client_application_layer_tim;//应用层会话管理计时器

/*
*----------------------------------------------------------------------------------------------------------------------------------------
* 函数原型：void init_diag_protocol_parameter(void)
* 函数功能：初始化diag相关参数
  参数说明：
* 输入参数：无
* 返 回 值：无
  作者：    qdh
*----------------------------------------------------------------------------------------------------------------------------------------
*/
void init_diag_protocol_parameer(void)
{
 	diag.address_type=DIAG_P_ADDRESS;//物理寻址模式
	diag.sesion_mode=DIAG_DE_MODE;//上电即保持在默认会话模式
	diag.security_level1_state=DIAG_SECURITY_LEVEL_STATE_LOCK; //安全状态
	diag.security_level3_state=DIAG_SECURITY_LEVEL_STATE_LOCK;//安全状态
	diag.dtc_function_enable=DIAG_DTC_FUNCTION_ON;
	diag.communication_enable=DIAG_COMM_RX_AND_TX_ENABLE;
	diag.SAFC_reach_3num_mcu_resetbit = DIAG_ECU_HAD_NOT_RESET;//取1位
	diag.SAFC_1=0x00;
	diag.SAFC_3=0x00;
	diag.time_SAFC_3=0x00;
	diag.time_SAFC_1=0x00;//一般安全访问失败技术3次时间点
	diag.security_level1_access_step=DIAG_NOT_REQ_SEED;//未请求Seed
	diag.security_level3_access_step=DIAG_NOT_REQ_SEED;//未请求Seed
	//SAFC_1  SAFC_3  从flash读出来
	//应用层时间参数
	server_application_layer_tim.P2can=50*OS_1MS;
	server_application_layer_tim.P2_asterisk_can=5000*OS_1MS;
	server_application_layer_tim.S3_sever=5000*OS_1MS;
	client_application_layer_tim.P2can=50*OS_1MS;//ECU收到诊断发出回应时间要求
	client_application_layer_tim.P2_asterisk_can=5000*OS_1MS;
	client_application_layer_tim.P3_can_phy=100*OS_1MS;
	client_application_layer_tim.P3_can_func=100*OS_1MS;
	client_application_layer_tim.S3_client=5000;
	client_application_layer_tim.S3_Tester=4000;
  	 //diag.rev_frame_tim=0x00; 接受任意帧的时间
    init_did_data(get_device_info());
	init_dtc_flash();
}
/*
*----------------------------------------------------------------------------------------------------------------------------------------
* 函数原型：void init_diag_protocol_parameter(void)
* 函数功能：初始化diag相关参数
  参数说明：
* 输入参数：无
* 返 回 值：无
  作者：    qdh
*----------------------------------------------------------------------------------------------------------------------------------------
*/
void init_diag_protocol_parameer_boot(void)
{
 	diag.address_type=DIAG_P_ADDRESS;//物理寻址模式
	diag.sesion_mode=DIAG_PR_MODE;//上电即保持在默认会话模式
	diag.security_level1_state=DIAG_SECURITY_LEVEL_STATE_LOCK; //安全状态
	diag.security_level3_state=DIAG_SECURITY_LEVEL_STATE_LOCK;//安全状态
	diag.dtc_function_enable=DIAG_DTC_FUNCTIONOFF;
	diag.communication_enable=DIAG_ENABLE_RX_AND_TX_DISABLE;
	diag.SAFC_reach_3num_mcu_resetbit = DIAG_ECU_HAD_NOT_RESET;//取1位
	diag.SAFC_1=0x00;
	diag.SAFC_3=0x00;
	diag.time_SAFC_3=0x00;
	diag.time_SAFC_1=0x00;//一般安全访问失败技术3次时间点
	diag.security_level1_access_step=DIAG_NOT_REQ_SEED;//未请求Seed
	diag.security_level3_access_step=DIAG_NOT_REQ_SEED;//未请求Seed
	//SAFC_1  SAFC_3  从flash读出来
	//应用层时间参数
	server_application_layer_tim.P2can=50*OS_1MS;
	server_application_layer_tim.P2_asterisk_can=5000*OS_1MS;
	server_application_layer_tim.S3_sever=5000*OS_1MS;
	client_application_layer_tim.P2can=50*OS_1MS;//ECU收到诊断发出回应时间要求
	client_application_layer_tim.P2_asterisk_can=5000*OS_1MS;
	client_application_layer_tim.P3_can_phy=100*OS_1MS;
	client_application_layer_tim.P3_can_func=100*OS_1MS;
	client_application_layer_tim.S3_client=2000*OS_1MS;
	client_application_layer_tim.S3_Tester=4000;
}


//******************************************************************************
// DESCRIPTION:
// ARUGMENT   :获取communication_enable的状态
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
// ARUGMENT   :sesion_mode  状态
// RETURN     :
// AUTHOR     :
//******************************************************************************
u8  get_diag_sesion_mode(void)
{
	return diag.sesion_mode;
}

//******************************************************************************
// DESCRIPTION:
// ARUGMENT   :address_type  当前寻址模式
// RETURN     :
// AUTHOR     :
//******************************************************************************
u8  get_diag_address_type(void)
{
	return diag.address_type;
}

//******************************************************************************
// DESCRIPTION:设置寻址模式
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
 函 数 名  : SecuriyAlgorithmFBL
 功能描述  : boot安全算法
 输入参数  : _int32 wSeed  
             _int32 MASK   
 输出参数  : 无
 返 回 值  : u32
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年12月04日
    作    者   : xz
    修改内容   : 新生成函数

*****************************************************************************/
//一级访问。只需要一个常数数组---控制器序列号
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

unsigned int encipher (unsigned int num_rounds,unsigned int v[2],unsigned int  k[4])	// num_rounds=2  为安全等级深度
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
// DESCRIPTION:诊断服务$10 会话模式 for app
// ARUGMENT   : //buffer为sid后开始数据内容 len为sid后开始的数据长度（帧结构）
// RETURN     :
// AUTHOR     :
//******************************************************************************
u8  SID_10_fun(device_info_t *dev,u8 *buffer,u16 len)
{
	u8 send_data[8]={0};
	u8 responce_result=NO_ERR;
	if(len!=0x01)//判断长度
	{
		responce_result=FOR_ERR;
                         		dbg_msg(dev, "$10 service 不正确的消息长度···\r\n");
		can_send_NRC(DIAG_SESSION_SID,FOR_ERR);
		return  responce_result;
	}
	switch(buffer[0])//数据内容  第一个字节buffer[0]代表子服务选项
	{
		case 0x01:				  
		  	if(diag.sesion_mode == DIAG_PR_MODE)
		  	{
		  	//	responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//条件不满足NRC=22 
			//	can_send_NRC(DIAG_SESSION_SID,CONDITION_ERR);
			//	return responce_result;	
			}
		    diag.sesion_mode = DIAG_DE_MODE;//设置进入默认会话模式
			diag.security_level1_state = DIAG_SECURITY_LEVEL_STATE_LOCK;//安全状态锁定
		    diag.security_level3_state = DIAG_SECURITY_LEVEL_STATE_LOCK;//安全状态锁定
		    diag.dtc_function_enable = DIAG_DTC_FUNCTION_ON;//使能DTC  
		    if(diag.communication_enable == DIAG_COMM_RX_AND_TX_DISABLE)//判断通信控制使能
		    {
		      diag.communication_enable   = DIAG_COMM_RX_AND_TX_ENABLE;
			//	diag_comm_enable( );//init_can(0);  //test
		    }
			
			//dbg_msg(dev, "Enter $10 01 service success,Need positive response\r\n");
		    //responce contect   $10服务的正响应格式
			send_data[0] = 0x06;//长度
			send_data[1] = 0x50;
			send_data[2] = 0x01;//返回 进入 01 子服务（diagnostic session type）	    	    
			send_data[3] = (client_application_layer_tim.P2can&0xFF00)>>8; //存放高字节
			send_data[4] = client_application_layer_tim.P2can&0xFF;//低字节
			send_data[5] =(client_application_layer_tim.P2_asterisk_can&0xFF00)>>8;// 
			send_data[6] =client_application_layer_tim.P2_asterisk_can&0xFF;//低字节 
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);
		break;
			
		case 0x02:
			if((diag.address_type == DIAG_F_ADDRESS)||(diag.sesion_mode == DIAG_DE_MODE))//编程会话必须是物理寻址模式		  
			{
				responce_result=NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//DIAG_PR_MODE不支持功能寻址
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
//				can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//返回否定响应
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
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//进入编程模式的肯定响应
			//comm_message_send(dev,CM_UPDATE_ARM_READY, 0,NULL , 0);//通知ARM进入uboot
			dev->ipc->app_jump = 1;//升级标志位 置1	
			stop_timer(TIMER_DEV7); 
			systick_stop();
			device_deinit(dev);
			__disable_irq();
			process_switch(dev->ipc->boot_entry, dev->ipc->boot_stack);//跳转
			break;
			
		case 0x03:
			if(diag.sesion_mode == DIAG_PR_MODE)//	非默认模式转换	  
			{
				responce_result=NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//DIAG_PR_MODE不支持功能寻址
				return responce_result; 				
			}
			diag.sesion_mode = DIAG_EX_MODE;
		  	diag.security_level1_state = DIAG_SECURITY_LEVEL_STATE_LOCK;
		    diag.security_level3_state = DIAG_SECURITY_LEVEL_STATE_LOCK;	
		   //responce contect
		 	dbg_msg(dev,"$10 03 for PR mode, checkPreconditon sucessed\r\n");
			send_data[0] = 0x06;//长度
			send_data[1] = 0x50;
		    send_data[2] = 0x03;//返回 进入 01 子服务（diagnostic session type）	    	    
		    send_data[3] = (client_application_layer_tim.P2can&0xFF00)>>8; //存放高字节
		    send_data[4] = client_application_layer_tim.P2can&0xFF;//低字节
		    send_data[5] =(client_application_layer_tim.P2_asterisk_can&0xFF00)>>8;// 
		    send_data[6] =client_application_layer_tim.P2_asterisk_can&0xFF;//低字节 
		   	can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);
		break;

		case 0x81 :
			if(diag.sesion_mode == DIAG_PR_MODE)
		  	{
		  		responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//条件不满足NRC=22 
				can_send_NRC(DIAG_SESSION_SID,CONDITION_ERR);
				return responce_result;	
			}
		    diag.sesion_mode = DIAG_DE_MODE;//设置进入默认会话模式
			diag.security_level1_state = DIAG_SECURITY_LEVEL_STATE_LOCK;//安全状态锁定
		    diag.security_level3_state = DIAG_SECURITY_LEVEL_STATE_LOCK;//安全状态锁定
		    diag.dtc_function_enable = DIAG_DTC_FUNCTION_ON;//使能DTC  
		    if(diag.communication_enable == DIAG_COMM_RX_AND_TX_DISABLE)//判断通信控制使能
		    {
		      diag.communication_enable   = DIAG_COMM_RX_AND_TX_ENABLE;
			//	diag_comm_enable( );//init_can(0);  //test
		    }
			
			dbg_msg(dev, "Enter $10 81 service success,No Positive Response Required\r\n");
			
		break;

		case 0x82 :
			if(diag.address_type == DIAG_F_ADDRESS)//编程会话必须是物理寻址模式		  
			{
				responce_result=NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//DIAG_PR_MODE不支持功能寻址
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
//				can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//返回否定响应
//				return NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;
//			}
			dbg_msg(dev,"$10 82 for PR mode, checkPreconditon sucessed\r\n");
			send_data[0] = 0x02;
			send_data[1] = 0x50;
			send_data[2] = 0x02;
	     //	can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//进入编程模式的肯定响应
			dev->ipc->app_jump = 1;//升级标志位 置1
			process_switch(dev->ipc->boot_entry, dev->ipc->boot_stack);//跳转
		break;

		case 0x83 :
			if(diag.sesion_mode == DIAG_PR_MODE)//	非默认模式转换  安全状态切换到锁定状态，但通信控制服务和控制DTC设置服务的状态不受影响	  
			{
 				diag.security_level1_state=DIAG_SECURITY_LEVEL_STATE_LOCK; //安全状态锁定
				diag.security_level3_state=DIAG_SECURITY_LEVEL_STATE_LOCK;//安全状态锁定
			}
			 diag.sesion_mode = DIAG_EX_MODE;
		  	diag.security_level1_state = DIAG_SECURITY_LEVEL_STATE_LOCK;
		    diag.security_level3_state = DIAG_SECURITY_LEVEL_STATE_LOCK;	
		   //responce contect
		  	dbg_msg(dev,"$10 83 for PR mode,No Positive Response Required\r\n");
		break;
			
		default:
				dbg_msg(dev,"不支持子服务\r\n");
				responce_result=SUB_ERR;//buffer[0]值不对应01 02 03子服务不支持
			 //	can_send_NRC(DIAG_SESSION_SID,SUB_ERR);
			break;			
	}	
	return responce_result;		
}


//******************************************************************************
// DESCRIPTION: $11  ECUReset  支持01 hardReset,03 softReset，需要正响应回复
// ARUGMENT   : buffer为sid后开始数据内容 len为sid后开始的数据长度
// RETURN     :
// AUTHOR     :
//******************************************************************************
u8 SID_11_fun(device_info_t *dev,u8 *buffer,u16 len)
{
	u8 send_data[8]={0};
	u8 responce_result=NO_ERR;
	
	if(len!=0x01)//判断长度
	{
		responce_result=FOR_ERR;
		dbg_msg(dev, "$11 service 不正确的消息长度···\r\n");
		can_send_NRC(DIAG_ECU_RESET_SID,FOR_ERR);
		return  responce_result;
	}

	switch(buffer[0])
	{
		case 0x01:  //hardReset
			if((diag.sesion_mode == DIAG_PR_MODE)&&(diag.address_type==DIAG_F_ADDRESS))
		  	{
		  		responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//条件不满足NRC=22 
				can_send_NRC(DIAG_ECU_RESET_SID,CONDITION_ERR);
				return responce_result;	
			}
			diag.sesion_mode = DIAG_EX_MODE;
			dbg_msg(dev, "HardReset···\r\n");
			send_data[0] = 2;
			send_data[1] = 0x51;//$11服务的正响应格式
			send_data[2] = 0x01;	
		 	can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);
			mdelay(1000);//等待发完
			/*PMIC断*/
			IO_SET(GPIOA, 3, LOW);
			mdelay(10);
			software_reset(); 
			break;
	
		case 0x03: //softReset	   
			if((diag.sesion_mode == DIAG_PR_MODE)&&(diag.address_type==DIAG_F_ADDRESS))
		  	{
		  		responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//条件不满足NRC=22 
				can_send_NRC(DIAG_ECU_RESET_SID,CONDITION_ERR);
				return responce_result;	
			}	
			dbg_msg(dev, "SoftReset···\r\n");
			send_data[0] = 2;
			send_data[1] = 0x51;//$11服务的正响应格式
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
				responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//条件不满足NRC=22 
				can_send_NRC(DIAG_SESSION_SID,CONDITION_ERR);
				return responce_result; 
			}
			dbg_msg(dev, "Enter $11 81 service success,N0 positive response\r\n");
			mdelay(10);
			software_reset(); 
		break;
		
		default:
			dbg_msg(dev,"不支持子服务\r\n");
			responce_result=SUB_ERR;//buffer[0]值不对应01 02 03子服务不支持
			can_send_NRC(DIAG_ECU_RESET_SID,SUB_ERR);
		break;			
	}
  return responce_result;
	
}

//******************************************************************************
// DESCRIPTION:$14服务 ClearDiagnosticInformation 子服务$FFFFFF(All groups)  高中低三位组成 
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
		dbg_msg(dev, "$14 service 不正确的消息长度···\r\n");
		can_send_NRC(DIAG_CLR_DTC_SID,FOR_ERR);
		return  responce_result;
    }
		
    if((buffer[0]==0xff)&&(buffer[1]==0xff)&&(buffer[2]==0xff))//高、中、低位均为FF 激活  
    {
		clear_dtc() ;   // 清除 故障码 ,设置掩码为0x00，即可清零
    }
	else
	{
		dbg_msg(dev,"不支持子服务\r\n");
		responce_result=OUTRANGE_ERR;//buffer[0]值不对应01 02 03子服务不支持
		can_send_NRC(DIAG_CLR_DTC_SID,OUTRANGE_ERR);
		return responce_result;
	}
	dbg_msg(dev, "Enter $14 service success,Need positive response\r\n");
	send_data[0] = 0x01;
    send_data[1] = 0x54;  //$14 正响应回复Value 54 
    can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);

	//save_dtc( );
	dev->ci->write_dtc_flag=1;//写flash标志
	 
    return responce_result;    
}

//******************************************************************************
// DESCRIPTION:$19 read DTC information
// ARUGMENT   : //buffer为sid后开始数据内容 len为sid后开始的数据长度  
// RETURN     :支持01 02 03 04 0A
// AUTHOR     :
//******************************************************************************
u8 SID_19_fun(device_info_t *dev,u8 *buffer,u16 len)
{
 	u8 send_data[AVM_DTC_MAX*4+3]={0};  //DTC 码
	u8 responce_result=NO_ERR;
	u8 tester_mask;
	u16 dtc_count=0;
  if(len != 0x02)             
  {
		responce_result=FOR_ERR;
		dbg_msg(dev, "$19 不正确的消息长度···\r\n");
		can_send_NRC(DIAG_R_DTC_SID,FOR_ERR);
		return  responce_result;
    }
	
	switch(buffer[0])
	{
		case 0x01:		//通过状态掩码报告故障码个数
			tester_mask = buffer[1];
			if(diag.sesion_mode == DIAG_PR_MODE) //编程会话时  
			{
   			  	responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//条件不满足NRC=22 
				can_send_NRC(DIAG_R_DTC_SID,NOT_SUPPERT_SUB_IN_ACTIVE_SESSION);
				return responce_result;	
			}
		//	dbg_msg(dev, "Enter $19 01 service success,Need positive response\r\n");
			dtc_count = read_dtc_01_02_sub_id(send_data,0x01,tester_mask);
			send_data[0] = 0x06;
			send_data[1] = 0x59;//正响应格式
			send_data[2] = 0x01;//request type
			send_data[3] = ECU_SUPPORT_MSAK;//
			send_data[4] = 0x00;//ISO15031-6DTCformat 为0 DTC_Format_Identifier
			send_data[5] = dtc_count>>8u;//DTC_Count 高字节
			send_data[6] = dtc_count&0x0F;//低字节
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);	
			break;
			
		case 0x02://通过状态掩码报告故障码  多帧发送
			tester_mask = buffer[1];
			if(diag.sesion_mode == DIAG_PR_MODE)  //不支持编程模式
			{
   				responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//条件不满足NRC=22 
				can_send_NRC(DIAG_R_DTC_SID,CONDITION_ERR);
				return responce_result;	
			}
//			dbg_msg(dev, "Enter $19 02 service success,Need positive response\r\n");
			send_data[0] = 0x59;//request type
			send_data[1] = 0x02;//子服务
			send_data[2] =ECU_SUPPORT_MSAK ;//DTCstatusavailabilityMask.
			dtc_count = read_dtc_01_02_sub_id(&send_data[3],0x02,tester_mask);
			dbg_msgv(dev, "dtc_count : %d\r\n", dtc_count);
			can_net_layer_send.len=dtc_count*4+3;
			memcpy(&can_net_layer_send.buff[0],&send_data[0],can_net_layer_send.len);
			//此处多帧发送，DTC信息多帧发送。
			if(can_net_layer_send.len<0x08)
			{
				can_msg_Send_SWJ(dev,AVM_Phy_Resp, (u32*)send_data);
				}
			else{
				diag_send_data(dev,can_net_layer_send.buff,can_net_layer_send.len);
				}
			break;
		
		default:
			dbg_msg(dev,"不支持子服务\r\n");
			responce_result=SUB_ERR;//buffer[0]值不对应01 02 03子服务不支持
			can_send_NRC(DIAG_R_DTC_SID,SUB_ERR);
		break;				
	}

	//读取$19  DTC 后续动作
  return responce_result;
}


//******************************************************************************
// DESCRIPTION:$22  通过标识符读某个DID数据服务 for app  
// ARUGMENT   : buffer为sid后开始数据内容 len为sid后开始的数据长度 
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
	u16 DID = 0;//DID value  2个字节   
	if(len!=0x02)//判断长度
	{
		responce_result=FOR_ERR;
		dbg_msg(dev, "$22 service 不正确的消息长度···\r\n");
		can_send_NRC(DIAG_R_DID_SID,FOR_ERR);
		return  responce_result;
	}
 	if(diag.sesion_mode==DIAG_PR_MODE)//编程会话不支持
	{
		responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//条件不满足NRC=22 
		can_send_NRC(DIAG_R_DID_SID,CONDITION_ERR);
		return responce_result;
	}		
	DID = buffer[0]<<8;//DID MSB
	DID += buffer[1];//DID LSB
	switch( DID)
	{	//回复均为>8字节
		
		case DID_CURRENT_DIAG_SESSION :
			send_did_data.b[0]=get_diag_sesion_mode();
			can_net_layer_send.len = LEN_CURRENT_DIAG_SESSION;//+3 包含22服务和DID两个字节
			responce_result=NO_ERR;
		break;

		case DID_Part_Number: // 供应商代码 9个字节
			STMFLASH_Read( ADDR_Part_Number,send_did_data.h,LEN_Part_Number/2+1);
			can_net_layer_send.len = LEN_Part_Number;
			responce_result=NO_ERR;
		break;
		
		case DID_ECUSoftwareVersion : 
			STMFLASH_Read( ADDR_ECUSoftwareVersion,send_did_data.h,LEN_ECUSoftwareVersion/2);
		//	printf("ver=%x-%x\r\n",send_did_data.b[0],send_did_data.b[1]);
			can_net_layer_send.len = LEN_ECUSoftwareVersion;//+3 包含22服务和DID两个字节
			responce_result=NO_ERR;
		break;

		case DID_Manufacture_Date: //  4个字节
			STMFLASH_Read( ADDR_Manufacture_Date,send_did_data.h,LEN_Manufacture_Date/2);
			can_net_layer_send.len = LEN_Manufacture_Date;
			responce_result=NO_ERR;
		break;

		case DID_ECUSerialNumber:			
			STMFLASH_Read( ADDR_ECUSerialNumber,send_did_data.h,LEN_SerialNumber/2);
			can_net_layer_send.len = LEN_SerialNumber;
			responce_result=NO_ERR;
		break;
		
		case DID_Supported_units:  // 供应商电子控制单元硬件版本号  3个字节
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
			dbg_msg(dev,"$22不支持的DID\r\n");
			responce_result=OUTRANGE_ERR;//buffer[0]值不对应01 02 03子服务不支持
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
				diag_send_data(dev,can_net_layer_send.buff, can_net_layer_send.len);//多帧发送
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
// DESCRIPTION:$27 SecurityAccess 01RequestSeed 02Sendkey 和07 08对应
// ARUGMENT   : //buffer为sid后开始数据内容 len为sid后开始的数据长度  
// RETURN     :以知豆安全算法为例
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
	case 0x01:	//app程序 ：01 02 在扩展会话 物理寻址才 可用			  	   							   						
			//长度合法性判断
		if(len!=0x01)//判断长度
		{
			responce_result=FOR_ERR;
			//dbg_msg(dev, "$27 service 不正确的消息长度···\r\n");
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,FOR_ERR);
			return  responce_result;
			}  
		if((diag.sesion_mode ==DIAG_DE_MODE)||(diag.address_type ==DIAG_F_ADDRESS ))    
		{
			responce_result = CONDITION_ERR;//条件不满足NRC=22 
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,CONDITION_ERR);
			return responce_result; 
			}
		if((diag.SAFC_1>=2)&&(diag.SAFC_reach_3num_mcu_resetbit == DIAG_ECU_HAD_NOT_RESET))                     
		{
			responce_result = DELAY_NOT_ARRIVAL_ERR;//
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,DELAY_NOT_ARRIVAL_ERR);
			return responce_result; 
			}
		if(diag.security_level1_state == DIAG_SECURITY_LEVEL_STATE_UNLOCK)  //if having been unlock,send datas 00 00 00 00，已经解锁安全访问
		{	
			send_data[0] = 0x06;
            send_data[1] = 0x67;//正响应格式
            send_data[2] = 0x01;//正响应格式
            can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)send_data);
		}
		else
		{
         	diag.security_seed=create_seed();
			send_data[0] = 0x06;//长度
			send_data[1] = 0x67;
            send_data[2] = 0x01;
			//4个字节的种子放入send_data
            send_data[3] = diag.security_seed>>24;
            send_data[4] = (diag.security_seed >>16)&0xFF;
            send_data[5] = (diag.security_seed>>8)&0xFF;
            send_data[6] = diag.security_seed &0xFF;
            can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)send_data);//返回seed
            diag.security_level1_access_step = DIAG_HAD_REQ_SEED;//确认安全访问步骤
		}			
		break;

	case 0x03:	//app程序 ：01 02 在扩展会话 物理寻址才 可用			  	   							   						
			//长度合法性判断
		if(len!=0x01)//判断长度
		{
			responce_result=FOR_ERR;
			//dbg_msg(dev, "$27 service 不正确的消息长度···\r\n");
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,FOR_ERR);
			return  responce_result;
			}  
		if((diag.sesion_mode ==DIAG_DE_MODE)||(diag.address_type ==DIAG_F_ADDRESS )  )    
		{
			responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//条件不满足NRC=22 
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,CONDITION_ERR);
			return responce_result; 
			}
		if((diag.SAFC_3>=2)&&(diag.SAFC_reach_3num_mcu_resetbit == DIAG_ECU_HAD_NOT_RESET))                     
		{
			responce_result = DELAY_NOT_ARRIVAL_ERR;//
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,DELAY_NOT_ARRIVAL_ERR);
			return responce_result; 
			}
		if(diag.security_level3_state == DIAG_SECURITY_LEVEL_STATE_UNLOCK)  //if having been unlock,send datas 00 00 00 00，已经解锁安全访问
		{	
			send_data[0] = 0x06;
            send_data[1] = 0x67;//正响应格式
            send_data[2] = 0x03;
            can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)send_data);
		}
		else
		{
         	diag.security_seed=create_seed();
			send_data[0] = 0x06;//长度
			send_data[1] = 0x67;
            send_data[2] = 0x03;
			//4个字节的种子放入send_data
            send_data[3] = diag.security_seed>>24;
            send_data[4] = (diag.security_seed >>16)&0xFF;
            send_data[5] = (diag.security_seed>>8)&0xFF;
            send_data[6] = diag.security_seed &0xFF;
            can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)send_data);//返回seed
            diag.security_level3_access_step = DIAG_HAD_REQ_SEED;//确认安全访问步骤
		}			
		break;
		
	case 0x02://返回 key  
		if((diag.sesion_mode ==DIAG_DE_MODE)||(diag.sesion_mode ==DIAG_PR_MODE)||(diag.address_type ==DIAG_F_ADDRESS )	)	 
		{
			responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//条件不满足NRC=22 
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,CONDITION_ERR);
			return responce_result; 
			}
		if((diag.security_level1_state == DIAG_SECURITY_LEVEL_STATE_UNLOCK)||(diag.security_level1_access_step != DIAG_HAD_REQ_SEED))    
			{
			responce_result = SEQUENCE_ERROR;//条件不满足NRC=22 
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,SEQUENCE_ERROR);
			return responce_result; 
			}
		if(diag.security_level3_state == DIAG_SECURITY_LEVEL_STATE_UNLOCK)  //if having been unlock,send datas 00 00 00 00，已经解锁安全访问
		{	
			send_data[0] = 0x06;
            send_data[1] = 0x67;//正响应格式
            send_data[2] = 0x02;
            can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)send_data);	
			break ;
		}

		else
			diag.security_level1_access_step = DIAG_NOT_REQ_SEED; 
		//obtain the key from tester获取Key from 诊断仪
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
			diag.security_level1_state = DIAG_SECURITY_LEVEL_STATE_UNLOCK; //匹配后清零标志位，发送解除标志位
		}
		send_data[0] = 0x02;
		send_data[1] = 0x67;
		send_data[2] = 0x02;
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)send_data);	
		diag.security_level1_access_step = DIAG_NOT_REQ_SEED;//第二步？？？标志位？？
		break;

	case 0x04://返回 key  
		if((diag.sesion_mode ==DIAG_DE_MODE)||(diag.address_type ==DIAG_F_ADDRESS )	)	 
		{
			responce_result = NOT_SUPPERT_SUB_IN_ACTIVE_SESSION;//条件不满足NRC=22 
			can_send_NRC(DIAG_SECURITY_ACCESS_SID,CONDITION_ERR);
			return responce_result; 
			}
		if(diag.security_level3_state == DIAG_SECURITY_LEVEL_STATE_UNLOCK)  //if having been unlock,send datas 00 00 00 00，已经解锁安全访问
		{	
			send_data[0] = 0x06;
            send_data[1] = 0x67;//正响应格式
            send_data[2] = 0x04;
            can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)send_data);
			break ;
		}

		else
			diag.security_level3_access_step = DIAG_NOT_REQ_SEED; 
		//obtain the key from tester获取Key from 诊断仪
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
			diag.security_level3_state = DIAG_SECURITY_LEVEL_STATE_UNLOCK; //匹配后清零标志位，发送解除标志位
		}
		send_data[0] = 0x02;
		send_data[1] = 0x67;
		send_data[2] = 0x04;
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)send_data);	
		diag.security_level3_access_step = DIAG_NOT_REQ_SEED;//第二步？？？标志位？？
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
// ARUGMENT   : buffer为sid后开始数据内容 len为sid后开始的数据长度  
// RETURN     :
// AUTHOR     :
//******************************************************************************
uint8_t   SID_28_fun(device_info_t *dev,uint8_t *buffer,uint16_t len)
{
	uint8_t send_data[8]={0};
	uint8_t responce_result=NO_ERR;
	//长度合法性判断
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
	u16 routine_id;//请求例程 ID
	u8 Erase_result,routine_control_type;//routine_control_type=start routine(01第二个字节)
   //数据长度判断  长度必须大于3，不同子服务数据长度不一样
	if(len<3)
	{
		responce_result=FOR_ERR; 
		can_send_NRC(DIAG_ROUTE_CONTROL_SID,FOR_ERR);
		return responce_result; 
	}
	routine_control_type=buffer[0];	                	
	routine_id=buffer[1];
	routine_id=(routine_id<<8)+buffer[2];
	
	if((routine_control_type== 0x01)&&(routine_id==0xFF00)&&(buffer[3]==0xff))//31  01子服务
	{
		send_data[0]=0x03;
		send_data[1]=0x7F;
		send_data[2]=0x31;//Routine Control Type
		send_data[3]=0x78;
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//返回78提示诊断仪等待正响应
		//
		Erase_result = upgrade_earseMemory(dev);
		send_data[0]=0x05;
		send_data[1]=0x71;
		send_data[2]=0x01;//Routine Control Type
		send_data[3]=0xff;
		send_data[4]=0x00;
		send_data[5]=Erase_result;//成功01 失败02
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);
		}
	else if((routine_control_type== 0x01)&&(routine_id==0xff00)&&(buffer[3]==0x01))//升级boot
	{
		dbg_msg(dev,"uds upgrade mcu_boot,jump to app\r\n");
		send_data[0]=0x03;
		send_data[1]=0x7F;
		send_data[2]=0x31;//Routine Control Type
		send_data[3]=0x78;
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//返回78提示诊断仪等待正响应
		//Erase_result = upgrade_earseMemory(dev);
		
		dev->ipc->upgrade_mcu_boot = 1;//升级mcu_boot 	
		systick_stop();
		device_deinit(dev);
		__disable_irq();
		process_switch(dev->ipc->app_entry, dev->ipc->app_stack);//跳转
		return 1;
		}
	else if((routine_control_type== 0x01)&&(routine_id==0xff01)&&(buffer[3]==0xff))
	{
		send_data[0]=0x03;
		send_data[1]=0x7F;
		send_data[2]=0x31;//Routine Control Type
		send_data[3]=0x78;
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//返回78提示诊断仪等待正响应
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
// ARUGMENT   : buffer为sid后开始数据内容 len为sid后开始的数据长度  
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
	if(diag.security_level3_state != DIAG_SECURITY_LEVEL_STATE_UNLOCK)//需安全访问
	{	
		//can_send_NRC(DIAG_W_DID_SID,SECURITY_ERR );
	//	return (SECURITY_ERR);    
		}
	memcpy(data,&buffer[2],len-2);//did后面要写入的数据
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
			responce_result=SUB_ERR;//buffer[0]值不对应01 02 03子服务不支持
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
	can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//延迟到标志位为1在写，先保存DID和长度
	if(dev->ci->write_did_flag)//可以写did
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
	diag.file_trans_mode = 0x02; //下载模式
	update_type = buffer[10];//升级类型
	if((update_type==0x10u)||(update_type==0x20u))
	{
		dbg_msg(dev,"upgrade mcu\r\n");	
		upgrade_requestFileDownload(dev,buffer, len);//解析请求下载命令，包含数据长度
		}
	else
	{	
		dbg_msg(dev,"请求下载ARM升级数据\r\n");
		upgrade_requestFileDownload_ARM(dev,buffer, len);//解析请求下载命令，包含数据长度
		comm_message_send(dev,CM_UPDATE_ARM_READY, 0, NULL , 0);
		/*开始计时，多长时间未收到ARM回应，表示升级失败*/
		dev->ipc->usart_normal = 0;
		start_timer(TIMER_DEV1,5000);
		}
		return NO_ERR;
}


//******************************************************************************
// DESCRIPTION:
// ARUGMENT   : //buffer为sid后开始数据内容 len为sid后开始的数据长度  
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
		//子参数02为升级指令
		send_data[0]=0x03;
		send_data[1]=0x7F;
		send_data[2]=0x36;//Routine Control Type
		send_data[3]=0x78;
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);//返回78提示诊断仪等待正响应
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
		dbg_msg(dev,"数据传输完成，退出传输服务\r\n");
		}
	else 
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);
	return NO_ERR;
}


//******************************************************************************
// DESCRIPTION:
// ARUGMENT   : //buffer为sid后开始数据内容 len为sid后开始的数据长度  
// RETURN     :
// AUTHOR     :
//******************************************************************************
u8 SID_3E_fun(device_info_t *dev,uint8_t *buffer,uint16_t len)
{
	u8 send_data[8]={0};
	u8 responce_result=NO_ERR;
	//长度合法性判断
	if(len!=0x01)//长度为1
	{
		responce_result=FOR_ERR; 
		can_send_NRC(DIAG_TESTER_PRESENT_SID,FOR_ERR);
		return responce_result; 
	}
	dbg_msg(dev, "Enter $3E service success\r\n");

	switch(buffer[0])
	{
		case 0x00:	//诊断仪在线
			send_data[0] = 0x02;
			send_data[1] = 0x7E;
			send_data[2] = 0x00;
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);	  				
			break;
		case 0x80://什么都不用发，
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
// DESCRIPTION: buffer为sid后开始数据内容 len为sid后开始的数据长度  
// ARUGMENT   :DTC控制 EX会话模式
// RETURN     :
// AUTHOR     :
//******************************************************************************
u8 SID_85_fun(device_info_t *dev,u8 *buffer,u8 len)
{
	u8 send_data[8]={0};
	u8 responce_result=NO_ERR;
	//长度合法性判断
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
* 函数原型：void app_diagnose(void)
* 函数功能：初始化diag相关参数
  输入参数说明：  buffer:sid(含)开始的完整的一桢数据，len:sid(含)后数据总长度
* 返 回 值：无
  作者：    qdh
*----------------------------------------------------------------------------------------------------------------------------------------
*/
u32 app_diagnose(device_info_t *dev,uint8_t *buffer,uint16_t len)
{	
	uint8_t sid=buffer[0];              //sid  服务ID
	uint8_t response_result=NO_ERR;     //初始 正确结果
//	uint8_t send_data[7];	
	switch(sid)  //buffer[1]为sid后开始数据内容
    {
	    case DIAG_SESSION_SID:  //会话模式  10
	        response_result=SID_10_fun(dev,&buffer[1],len-1);                         
	        break;
	    case DIAG_ECU_RESET_SID: //复位  11
	    	response_result=SID_11_fun(dev,&buffer[1],len-1);                                          
	        break;
	    case DIAG_CLR_DTC_SID:   //清除故障  14
	        response_result=SID_14_fun(dev,&buffer[1],len-1);                                                           
	        break;
	    case DIAG_R_DTC_SID:  //读DTC
	        response_result=SID_19_fun(dev,&buffer[1],len-1);            	            	
	        break;
	    case DIAG_R_DID_SID:   //读标识符 
	        response_result=SID_22_fun(dev,&buffer[1],len-1);       
	        break;
	    case DIAG_SECURITY_ACCESS_SID:  //安全访问  
	        response_result=SID_27_fun(dev,&buffer[1],len-1);                                  	               	   
	        break;
	    case DIAG_COMM_CONTROL_SID: //通信控制
	        response_result=SID_28_fun(dev,&buffer[1],len-1);                                  	               	               	            	 
	        break;
	    case  DIAG_W_DID_SID://写标识符:  
			response_result=SID_2E_fun(dev,&buffer[1],len-1);      
	        break;               
	    case  DIAG_R_DIR_DID://读取文件目录列表
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
	    case DIAG_ROUTE_CONTROL_SID:   //例程控制
	         response_result = SID_31_fun(dev,&buffer[1],len-1); 
	        break;
	    case DIAG_REQUEST_DOWNLOAD_SID:
	       	 response_result = SID_34_fun(dev,&buffer[1], len-1);
	        break;
	    case DIAG_REQUEST_TRANSFER_EXIT_SIG:
	       	 response_result = SID_37_fun(dev,&buffer[1], len-1);
	        break;

	    default:
	   		dbg_msg(dev, "SID is not support···\r\n");
	        response_result = SID_ERR;//服务不支持
			can_send_NRC( sid, SID_ERR); 
	        break;
    }                	
	return response_result;
}

/*
 *@brief :   应用程序检测下载报文执行预编程环境检测
*autor:  xz  
*Time ： 20180524
*/
void check_s3_client(device_info_t *dev)
{
	static u8 current_sts=0; 
	if((get_diag_sesion_mode()!=DIAG_DE_MODE)&&(current_sts==0))//不在默认会话，打开定时器5s
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

