
#include <can.h>
#include <diag_upgrade.h>
#include <usart.h>
#include <api.h>
#include <protocolapi.h>
#include "can_net_layer.h"
#include "can_diag_protocol.h"
#include "timer.h"

static void sys_stopmode(void)
{	
	SCB->SCR |= 1ul<<2;
	PWR->CR &=0ul<<1;
	PWR ->CR |=1ul<<0;
	__WFI();
} 

/*停止模式是通过WFI(等待中断方式进入)，所以进入后有外部中断时
就会进入中断程序继续执行程序  然后返回停止模式程序下条语句执行。*/
static void Enter_stopmode( void)
{
	systick_stop();
	sys_stopmode();
}



void stop_mode_proc(device_info_t *dev)
{	 
		watchdog_feed(dev);
	
		clock_switch(dev, FALSE);
	
		device_deinit(dev);    //关闭串口，在关闭之前向上位机发送应答指令
		
		device_low_power_enter();	//GPIO ABCDE配置成模拟输入（在停机模式?/笽O零消耗

		__disable_irq();
	
		ACC_EXTI_Init();
			
		CAN_RX_EXTI_Init();

		Enter_stopmode();
				
	 	memset((u32 *)(SRAM_BASE+0x400ul),0,(SRAM_SIZE-0x400)); //防止启动不正常，最好重启之前重置SRAM，执行cold启动
	
		software_reset(); // never return
}


static u32 comm_booting_process(device_info_t *dev, message_t *msg)
{
	switch(msg->id)
	{
		case CM_ARM_READY: // host uart ready

			dbg_msg(dev, "Host is ready!\r\n");

	//		confirm_upgrading(dev, UPD_TYPE_APP, UPD_FROM_MCU);


			dev->ci->state = HOST_RUNNING; // Exit this state

			break;

		case CM_MCU_UPGRADE:

			dbg_msg(dev, "Host confirm upgrading!\r\n");

			dev->ci->state = HOST_RUNNING; // Exit this state

			goto exit0;

		case CM_HOST_ACK_MSG:

			dbg_msg(dev, ">>>>> RECV HOST uboot ACK\r\n");

		//	stimer_stop(dev, FLAG_HOST_ACK);

			break;

		default:

			dbg_msgv(dev, "Unknown msg id %d, skip\r\n", msg->id);

			break;
	}

	return 0;

exit0:
	return 1;
}





//for ARM upgrade by diag can
static u32 comm_upgrading_process(device_info_t *dev, message_t *msg)
{
	
	u32 hid = msg->hid.w;
	u8 response_data[8]={0};
//	data_info_t di;
	switch(msg->id)
	{
		case CM_UPDATE_ARM_READY_ACK : 

			dbg_msg(dev, "\r\n---ARM ready for upgrade by diag can---\r\n");
			dev->ipc->usart_normal=1;
			if(stUpgradeParam.update_type==0x10)
			{
				return 0;		
				}
			response_data[0]= stUpgradeParam.update_type;
			dbg_msgv(dev, "\r\n-stUpgradeParam.update_type=%x-\r\n",stUpgradeParam.update_type);
			response_data[1]= (stUpgradeParam.fileTotal&0xff000000)>>24;
			response_data[2]= (stUpgradeParam.fileTotal&0xff0000)>>16;
			response_data[3]= (stUpgradeParam.fileTotal&0xff00)>>8;
			response_data[4]= stUpgradeParam.fileTotal&0xff;
			response_data[5] = 0x02;//0x01 代表1个包1K，0x02代表2K 
		//	di.data = &response_data[0];
	//		di.len = 6;
			comm_message_send(dev, CM_UPDATE_ARM_START, hid, response_data, 6);
			break;

		case CM_UPDATE_ARM_START_ACK:

			dbg_msg(dev, "\r\n---start  upgrade---\r\n");
			if(msg->par.b[0]==0x01)
			{
				response_data[0]  = 0x04;
				response_data[1]  = 0x74;
				response_data[2]  = 0x20;
				response_data[3]  = 0x08;
				response_data[4]  = 0x04;
				can_msg_Send_SWJ(dev, AVM_Phy_Resp,(u32*) response_data);//回复34正响应
				}
			else if (msg->par.b[0]==0x02)
			{
				//等待多少秒依然回复这个就中止升级
				}

			break;
		
		case CM_UPDATE_ARM_DATATRANS_ACK:

		   if(msg->par.b[0]==0x02)
		   {
			if(stUpgradeParam.fileRecvCnt <= stUpgradeParam.fileTotal)
	  		{
				if(0xFFFFFF == stUpgradeParam.bagSeq)
					stUpgradeParam.bagSeq = 1;
				else
					stUpgradeParam.bagSeq++;
	 		  }	
		   }
		
		   else if(msg->par.b[0]==0x01)
		   	{
		   	dbg_msg(dev,"bagNum error,back\r\n");	
		   		}
		   else {
		   	}
 			break;

		case CM_UPDATE_ARM_DATATRANS_END_ACK:			

			if(msg->par.b[0]==0x01)
			{
				dbg_msg(dev, "watiting for ARM updating...\r\n");
				}
			else
			{
				response_data[0]  = 0x04;
				response_data[1]  = 0x71;
				response_data[2]  = 0x02;
				response_data[3]  = 0x02;
				response_data[4]  = 0x00;
				can_msg_Send_SWJ_Up(dev,AVM_Phy_Resp,(u32*)response_data);
				dbg_msg(dev, "file checked failure\r\n");
				}
			break;

		case CM_UPDATE_ARM_FINISH:
				response_data[0]  = 0x05;
				response_data[1]  = 0x71;
				response_data[2]  = 0x01;
				response_data[3]  = 0x02;
				response_data[4]  = 0x02;
				response_data[5]  = msg->par.b[0];
				can_msg_Send_SWJ_Up(dev,AVM_Phy_Resp,(u32*)response_data);
				stUpgradeParam.optStep = OPT_REQ_CHECK1_OK;
				if(msg->par.b[0]==0x01)
				{
					dbg_msg(dev, "update ARM successed,ready to reboot  \r\n");					
					}
				else{
					dbg_msg(dev, "update ARM failed\r\n");
					}
			break;
		
		case CM_GENERAL_ACK:
		//	dbg_msg(dev, "ARM normal ACK\r\n");
		break;	

		default:

//			dbg_msgv(dev, "msg id 0x%02x is not support, skip!\r\n", msg->id);
			
		break;
	}

	return 0;

}


static void stage_booting_process(device_info_t *dev, message_t *msg, message_t *dmsg)
{
	dbg_msg(dev, "\r\n -----BOOTING------- \r\n");
	
	while(HOST_BOOTING == dev->ci->state)
	{
		watchdog_feed(dev);
	
		dbg_uart_msg_process(dev, dmsg);

		can_link_receive_data(dev);
	
		//if(get_flag(dev, FLAG_UART_COMM_DMAT_END))
		//{
	//		usart_on_dma_end(dev->comm_uart);
		//}
		
		if(TRUE == comm_message_poll(dev, msg))
		{
			if(comm_booting_process(dev, msg))
			{
				break;
			}
		}
	}
}




static void stage_running_process(device_info_t *dev, message_t *msg, message_t *dmsg)
{
//	int i=0;
	dbg_msg(dev, "\r\n ------UPGRADING------- \r\n");
	
	while(HOST_RUNNING == dev->ci->state)
	{
		watchdog_feed(dev);
	
		dbg_uart_msg_process(dev, dmsg);

		can_link_receive_data(dev);

		check_s3_client(dev);
		
		if(stUpgradeParam.update_type!=0x10)
		{
		if(TRUE == comm_message_poll(dev, msg))
		{
			if(comm_upgrading_process(dev, msg))
			{
				break;
			}
		  }
		}
	}
}





static void stage_upgrading_process(device_info_t *dev, message_t *msg, message_t *dmsg)
{
	dbg_msg(dev, "\r\n ------UPGRADING------- \r\n");
	
	while(HOST_UPGRADING== dev->ci->state)
	{
		
	}
}


static void stage_accoff_process(device_info_t *dev, message_t *msg, message_t *dmsg)
{
	#if ENABLE_CAN_WEKAUP
	
	can_msg_t *can_msg_tmp=NULL;
	
	dbg_msgf(dev, "\r\n--- Enter ACC OFF stage ---\r\n");
	
	while (HOST_ACCOFF==dev->ci->state  )
	{	
		watchdog_feed(dev);

		dbg_uart_msg_process(dev, dmsg);

	    can_msg_tmp  = car_can_recv(dev);

		if(get_flag(dev, FLAG_ACC_LINE_ON))//
		{			
			
			dev->ci->state = HOST_RUNNING;

			break;
			}


		if(dev->ci->wait_canMsg_time)
		{
			dev->ci->wait_canMsg_time=0; 

			if(can_msg_tmp==NULL)
			{				
				stop_mode_proc(dev);

				break;
			}
			else
			{	
				if( can_msg_tmp->id >= 0x700 && can_msg_tmp->id <= 0x7ff )//diag 报文
  				{	
  					diag_receive_data(dev,can_msg_tmp);		
    				}
				else if((can_msg_tmp->id > 0x00) && (can_msg_tmp->id <= 0x700))
				{
					Body_Can_proc(dev,can_msg_tmp);
					}	
				
				dev->ipc->can_wait_count=0;//处理完后继续计时
				}
			}
		else
		{
			if(can_msg_tmp!=NULL)
			{		
				 if( can_msg_tmp->id >= 0x700 && can_msg_tmp->id <= 0x7ff )//diag 报文
  				{	
  					diag_receive_data(dev,can_msg_tmp);		
    				}
				 else if((can_msg_tmp->id > 0x00) && (can_msg_tmp->id <= 0x700))
				 {
						Body_Can_proc(dev,can_msg_tmp);
						}
				 dev->ipc->can_wait_count=0;
				}	
			}
		

		if(get_flag(dev, FLAG_UART_COMM_DMAT_END))
		{
			usart_on_dma_end(dev->comm_uart);
			}
	
		if(TRUE == comm_message_poll(dev, msg)) 
		{		
			if(comm_running_process(dev, msg))
			{
				break;
				}
			}
		}
	#else
		dev->ci->wait_canMsg_time=0;
		dbg_msgf(dev, "\r\n--- Enter ACC OFF stage ---\r\n");
		while (HOST_ACCOFF==dev->ci->state)
		{
			dbg_uart_msg_process(dev, dmsg);

			watchdog_feed(dev);

			if(get_flag(dev, FLAG_ACC_LINE_ON))//鍒ゆ柇ACC鏄惁鎵撳紑
			{
				dbg_msg(dev, "ACC ON in running, skip!\r\n");
			
				dev->ci->state = HOST_RUNNING;

				break;
			}
			
			if(dev->ci->wait_canMsg_time)
			{
				dbg_msg(dev, "time out,ready to sleep\r\n");

				dev->ipc->can_wait_count=0;
				
				dev->ci->wait_canMsg_time=0; 

				device_deinit(dev);    //关闭串口，在关闭之前向上位机发送应答指令
		

				device_low_power_enter();	//GPIO ABCDE配置成模拟输入（在停机模式?/笽O零消耗
	
				ACC_EXTI_Init();
			
				Enter_stopmode();

				IO_SET(GPIOA, 3, LOW);
	
				mdelay(10);

				software_reset();
				
				break;
				}	
			}
	#endif			
//	wait_for_interrupt();
}


// order according to enum HOST_XXXX  函数指针数组存储函数指针
static state_func_t sfunc[] =
{
	stage_booting_process,
	stage_upgrading_process,
	stage_running_process,
	NULL,
};

state_func_t *get_stage_func_array(void)
{
	return sfunc;
}


