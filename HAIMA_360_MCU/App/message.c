

#include <usart.h>
#include <api.h>
#include <protocolapi.h>
#include <can_diag_protocol.h>
#include <can.h>
#include <can_dtc.h>
#include <miscdef.h>
#include <can_net_layer.h>
#include <timer.h>
#include <can_did.h>	

/*
*解决潍柴通信测试 no ack 
*	
*/
static void can_tx_no_ack_check(void)
{
	device_info_t *dev=get_device_info();
	static u8 times = 0,no_ack_recovery_stat = 0;
	u8 buff[8]={0};

	if((dev->ci->BusOffQuick == 1)||(dev->ci->BusOffSlow == 1)){//在busoff状态不存在NoAck
		times = 0;
		return;
	}
	
	if(((CAN1->ESR&0x70) >> 4) == 0x03)//no ack
	{ 
		if(times < 15)
		{
	
			if(get_flag(dev,FLAG_TIMEOUT_NO_ACK_10MS)){
				//发送失败会自动重传 不需要操作
				times++;
			}
		}
		else if(times == 15){  //150MS timeout  
			dbg_msgf(dev, "no ack recovery\r\n");
			car_can_reinit(dev);
			enable_err_retrans_mode(0);//禁止重发
			dev->ci->can_no_tx=0x01;//禁止发送报文 150ms
			no_ack_recovery_stat = 1;  //进入recovery状态 
			times++; //16
		}
		else if(times > 15){
			if(get_flag(dev,FLAG_TIMEOUT_NO_ACK_10MS)){
				times++;
			}
			if(times > 30){
				times = 0;
				dev->ci->can_no_tx=0x00;
				enable_err_retrans_mode(1);//重发
				no_ack_recovery_stat = 0; //退出recovery状态 
				buff[0]=dev->ipc->work_state &0x03;	
				buff[0]|=dev->ipc->avm_disviewsts<<4;
				body_can_send_fill(dev, 0x520,(u32*)buff,1,"BC520");//立即发送
			}
		}
		
		
	}
	else{ //退出no ack
		times = 0;
		if(no_ack_recovery_stat == 1){ //退出no ack  以免与busoff冲突
			dev->ci->can_no_tx=0x00;
			enable_err_retrans_mode(1);//重发
			no_ack_recovery_stat = 0;
		}
		
	}
}

/*
*解决潍柴通信 快慢busoff恢复策略
*	
*/

static void can_bufoff_recovery(device_info_t *dev)
{
	static u8  quick_times = 0; //快恢复次数计数
	static u8  slow_times = 0; //快恢复次数计数
	u8 buff[8]={0};
	u32 mailbox;
	if((get_busoff_sts()==0x01) &&(dev->ci->BusOffQuick == 0)&&(dev->ci->BusOffSlow == 0)){
		dbg_msg(dev, "busoff quick recovery\r\n");
		dev->ci->BusOffQuick=0x01;//进入  恢复  T[BusOffQuick]=100m	
		car_can_reinit(dev);
		enable_err_retrans_mode(0);//禁止重发
		dev->ci->can_no_tx=0x01;//禁止发送报文
		quick_times = 0;
		start_timer(TIMER_DEV2,50);//T[BusOffQuick]=100m		
	}



	if(dev->ci->BusOffQuick == 0x02){ //快恢复定时器时间到
		dev->ci->BusOffQuick=0x01;
		stop_timer(TIMER_DEV2);

     	//尝试恢复
		dev->ci->can_no_tx=0x00;//发送报文
		car_can_reinit(dev);
		enable_err_retrans_mode(1);//重发模式
		//body_can_send_fill(dev, 0x0b1,NULL,1,"BC0B1");// 1->立即发送
		//can_msg_send_handle(dev);// body can 发送处理函数
		mailbox = body_can_send_by_name(NULL,"SVM_591");
		mdelay(1);
		if((get_busoff_sts()==0x01)  || (CAN_TransmitStatus(CAN1,mailbox) != CAN_TxStatus_Ok)){
			quick_times++;
			dbg_msgv(dev, "busoff quick recovery failed:%d\r\n",quick_times);
			if(quick_times == 5){
				dev->ci->BusOffQuick =0x00;
				dev->ci->BusOffSlow=0x01;//进入慢恢复 
				dbg_msg(dev, "busoff slow recovery\r\n");
				car_can_reinit(dev);
				enable_err_retrans_mode(0);//禁止重发
				dev->ci->can_no_tx=0x01;//禁止发送报文
				start_timer(TIMER_DEV2,200);//T[BusOffSlow]=1000ms
				return;
			}
			car_can_reinit(dev);
			enable_err_retrans_mode(0);//禁止重发
			dev->ci->can_no_tx=0x01;//禁止发送报文
			start_timer(TIMER_DEV2,50);//进入下一次恢复
		}
		else{//退出离线状态
			dbg_msg(dev, "busoff quick recovery ok\r\n");
			dev->ci->BusOffQuick=0x00;
			dev->dtc_t.busoff_dtc_flag=0;
		   //所有报文发一遍
		   buff[0]=dev->ipc->work_state &0x03;	
			buff[0]|=dev->ipc->avm_disviewsts<<4;
			set_flag( dev,  FLAG_TIMEOUT_400MS);
		//	body_can_send_fill(dev, 0x591,(u32*)buff,0,"SVM_591");//立即发送
		//	body_can_send_fill(dev, 0x0b1,NULL,0,"BC0B1");//默认5 发送5次
		//	body_can_send_fill_event_count_by_name("BC0B1",4);//修改为4 再发送4次 已经成功发送过一次了
		//	body_can_send_fill_cur_time_by_name("BC0B1",0);//微调时间
	//		body_can_send_fill(dev, 0x0b3,NULL,0,"BC0B3");//立即发送
		}

		
		
	}

	if(dev->ci->BusOffSlow == 0x02){//慢恢复定时器时间到
		slow_times++;
		if(slow_times==0x03)
		{
			dev->dtc_t.busoff_dtc_flag=1;
			}
		dev->ci->BusOffSlow = 0x01;
		stop_timer(TIMER_DEV2);

     	//尝试恢复
		dev->ci->can_no_tx=0x00;//发送报文
		car_can_reinit(dev);
		enable_err_retrans_mode(1);//重发模式
		//body_can_send_fill(dev, 0x0b1,NULL,1,"BC0B1");// 1->立即发送
		//can_msg_send_handle(dev);// body can 发送处理函数
		mailbox = body_can_send_by_name(NULL,"SVM_591");
		mdelay(1);
		if((get_busoff_sts()==0x01)  || (CAN_TransmitStatus(CAN1,mailbox) != CAN_TxStatus_Ok)){
			dbg_msg(dev, "busoff slow recovery failed\r\n");
			car_can_reinit(dev);
			enable_err_retrans_mode(0);//禁止重发
			dev->ci->can_no_tx=0x01;//禁止发送报文
			start_timer(TIMER_DEV2,200);//进入下一次恢复
		}
		else{//退出离线状态
			dbg_msg(dev, "busoff slow recovery ok\r\n");
			slow_times=0;
			dev->ci->BusOffSlow=0x00;
			dev->dtc_t.busoff_dtc_flag=0;
		   //所有报文发一遍
		  	 buff[0]=dev->ipc->work_state &0x03;	
			buff[0]|=dev->ipc->avm_disviewsts<<4;
			set_flag( dev,  FLAG_TIMEOUT_400MS);
			//body_can_send_fill(dev, 0x591,(u32*)buff,0,"SVM_591");//立即发送
		//	body_can_send_fill(dev, 0x0b1,NULL,0,"BC0B1");//默认5 发送5次
		//	body_can_send_fill_event_count_by_name("BC0B1",4);//修改为4 再发送4次 已经成功发送过一次了
		//	body_can_send_fill_cur_time_by_name("BC0B1",0);//微调时间
	//		body_can_send_fill(dev, 0x0b3,NULL,0,"BC0B3");//立即发送

		}
	}

	return;
		
}

void enter_NM_sleep(device_info_t *dev)
{
	
	dev->ipc->stop_module_flag=1;
	
	IO_SET(GPIOA, 3, LOW);
	
	mdelay(10);
		
	software_reset(); // never return

	}

//应用报文在Alive发送100ms后才允许发送
u32 body_can591_data[8]={0};
void send_591_flame(device_info_t *dev)
{
	//dbg_msg(dev,"send 591 data\r\n");
	//mdelay(100);
	can_msg_Send_SWJ_Up(dev, 0x591,body_can591_data);
}

//休眠NM管理
void IOSwitchSample(void)
{		
//	dbg_msgv(get_device_info(), "ACC_sts=%d\r\n",GetACCStatus());

    if(GetACCStatus())
  	{
  		GotoMode(&our_netid,NM_Awake);
  	}
  	else
  	{
  		//dbg_msg(get_device_info(),"sleep  \r\n");
  		GotoMode(&our_netid,NM_BusSleep);
  	}

	//休眠，进入低功耗模式 
   if((NM_stBusSleep==our_netid.nmState)&&(GetACCStatus()==0))
	{
		//dbg_msg(get_device_info(),"sleep2  \r\n");
    	enter_NM_sleep(get_device_info());
		}
	
    return;
}

static void check_flame_miss(device_info_t *dev)
{
	
		if(get_flag(dev,FLAG_BODYCAN_MISS0))
		{
	//			dbg_msg(dev, "0x316 \r\n");
			dev->ipc->miss_avm0=1;
			stimer_stop(dev,FLAG_BODYCAN_MISS0);
			}
	
		if(get_flag(dev,FLAG_BODYCAN_MISS1))
		{
	//			dbg_msg(dev, "0x316 \r\n");
			dev->ipc->miss_avm1=1;
			stimer_stop(dev,FLAG_BODYCAN_MISS1);
			}
		if(get_flag(dev,FLAG_BODYCAN_MISS2))
		{
			dev->ipc->miss_avm2=1;
			stimer_stop(dev,FLAG_BODYCAN_MISS2);
			}
		if(get_flag(dev,FLAG_BODYCAN_MISS3))
		{
			dev->ipc->miss_avm3=1;
			stimer_stop(dev,FLAG_BODYCAN_MISS3);
			}
		if(get_flag(dev,FLAG_BODYCAN_MISS4))
		{
			dev->ipc->miss_avm4=1;
			stimer_stop(dev,FLAG_BODYCAN_MISS4);
			}
}

static u32 comm_booting_process(device_info_t *dev, message_t *msg)
{
	u8  buff[6];
	u32 hid = msg->hid.w;
	switch(msg->id)
	{
		case CM_ARM_READY: // host uart ready
	
			dbg_msg(dev, "Host is ready!\r\n");
			dev->ipc->dtc_arm_com_sate=0;
			dev->ipc->usart_normal=1;
			dev->ipc->work_state =0x02;
			dev->ci->host_ready = 1;
			dev->ci->state = HOST_RUNNING; // Exit this state
			//USB_Master_Slave(ON);
		
			buff[0]=19;
			buff[1]=02;
			buff[2]=00;
			buff[3]=00;
			buff[4]=01;
			buff[5]=01;

			comm_message_send(dev, CM_GET_TIME, hid, buff, 6);

			goto exit0;
		
		default:
	
			dbg_msgv(dev, "Unknown msg id 0x%02x, skip\r\n", msg->id);
	
			break;
	}
	
	return 0;
	
exit0:
	return 1;
}
static u32 comm_running_process(device_info_t *dev, message_t *msg)
{
	
	u32 i, ver, hid = msg->hid.w;
	u8 buff[8]={0};
	static body_car_send_info_t  body_car_send_date={0};
	switch(msg->id)
	{
		case CM_ARM_READY: // host uart ready, host may reboot for(para-reset, power shake)

			dbg_msg(dev, "\r\n----------- HOST READY ----------\r\n");
			dev->ipc->usart_normal=1;
			dev->ci->host_ready = 1; 	
			dev->ipc->work_state = 0x02;
			//初始化DTC
			dev->ipc->dtc_arm_com_sate=0;
			dev->ipc->can_wait_count=0;//
			buff[0]=17;
			buff[1]=02;
			buff[2]=14;
			buff[3]=11;
			buff[4]=05;
			buff[5]=30;
			comm_message_send(dev, CM_GET_TIME, hid, buff, 6);
			break;

		#if 1
		case CM_MCU_UPGRADE:
			dbg_msgv(dev, "start %s code upgrading[%d]...\r\n", msg->par.b[0] ? "MCU APP" : "MCU BOOT", msg->par_cnt);
			if(2 ^ msg->par_cnt)
			{
				dbg_msgv(dev, "invalid para cnt %d for mcu upgrade, skip\r\n", msg->par_cnt);
				break;
			}

			if(UPD_FROM_ARM ^ msg->par.b[1])
			{
				dbg_msgv(dev, "invalid upgrading DIR %d, not from ARM\r\n", msg->par.b[1]);
				break;
			}
			
			car_can_deinit_simple(dev);
			
			car_can_sleep(dev);

			if(UPD_TYPE_BOOT == msg->par.b[0])
			{
				dev->ci->upgrade_type = UPD_TYPE_BOOT;

				dev->ci->host_ready=0;

				confirm_upgrading(dev, UPD_TYPE_BOOT, UPD_FROM_ARM);

				car_can_sleep(dev);

				stop_timer(TIMER_DEV7);

				iflash_unlock();

				upgrading_process(dev,msg);
			}

			break;
		#endif
			
		case CM_MCU_BOOT_VERSION:

			dbg_msg(dev, "query mcu boot ver\r\n");

			ver = dev->ipc->boot_version; // (u32 *)buf may cause 'align fault'

			comm_message_send(dev, CM_ARM_BOOT_VER, hid, (u8 *)&ver, 4);
			
			break;

		case CM_MCU_APP_VERSION:
	//		dbg_msg(dev, "query mcu app ver\r\n");
			ver = get_app_version(); // (u32 *)buf may cause 'align fault'
			comm_message_send(dev, CM_ARM_APP_VER, hid, (u8 *)&ver, 4);
			break;

		case CM_SET_AVM_DISP:
		//	dbg_msg(dev,"rec set acm display\r\n");
			if(body_car_send_date.AVM_SET_AVM_DISP !=msg->par.b[0])
			{
				body_car_send_date.AVM_SET_AVM_DISP = msg->par.b[0];
				buff[0]= body_car_send_date.AVM_SET_AVM_DISP;
				buff[1]= (body_car_send_date.AVM_CalibrationStatus & 0x03)|\
						(body_car_send_date.AVM_CamCaliFault<<4);	
				can_msg_Send_SWJ_Up(dev, 0x0B1,(u32*)buff);
				}
			break;

		case CM_VIDEO_CALIBRATION_ACK:
			//ARM周期发送0xB1 有变化才MCU转发
			dbg_msg(dev,"rec video cal\r\n");
			if((body_car_send_date.AVM_CalibrationStatus!= msg->par.b[0])||(body_car_send_date.AVM_CamCaliFault!= msg->par.b[1]))
			{
				body_car_send_date.AVM_CalibrationStatus = msg->par.b[0];
				body_car_send_date.AVM_CamCaliFault = msg->par.b[1];
				buff[0]= body_car_send_date.AVM_SET_AVM_DISP;
				buff[1]= (body_car_send_date.AVM_CalibrationStatus & 0x03)|\
						(body_car_send_date.AVM_CamCaliFault<<4);	
				can_msg_Send_SWJ_Up(dev, 0x0B1,(u32*)buff);

				}
			break;	

		case CM_SEND_CAMERA_FAULT:

			dev->ipc->dtc_camera_state=msg->par.b[0];
			break;

		case CM_GET_TIME:

			dbg_msg(dev, "query time info\r\n");
			break;
		

		case CM_GET_MODE_ACK:
			buff[0]=1;
			buff[1]=msg->par.b[0];	
			can_msg_Send_SWJ_Up(dev, 0x0B3,(u32*)buff);
			break;

		case CM_GET_TURN_STATUS_ACK	:
				
      		if(dev->ipc->autotest_flag == 1)
				{ 
						buff[0] = msg->par.b[0];
		        debug_message_send(dev, CM_PC_GET_TURN_STAT, hid, buff, 1);
            }
			buff[0]=0x05;
			buff[1]=msg->par.b[0];
			can_msg_Send_SWJ_Up(dev, 0x0B3,(u32*)buff);
			break;	

		case CM_ARM_APP_VERSION_ACK:
			dev->ipc->arm_app_ver = (msg->par.b[1] << 16) | (msg->par.b[2] << 8)|(msg->par.b[3]);
			//dbg_msgv(dev, "arm_app_ver[%d,%d,%d]\r\n", msg->par.b[0],msg->par.b[1],msg->par.b[2]);
			ver = dev->ipc->arm_app_ver;
            if(dev->ipc->autotest_flag == 1)
			{
			    debug_message_send(dev, CM_PC_ARM_APP_VERSION_ACK, hid, (u8 *)&ver, 4); 
            	}
			buff[0]=0x06;
			buff[1]=0x62;
			buff[2]=DID_ARM_APP_VER>>8;
			buff[3]=DID_ARM_APP_VER&0xff;
			buff[4]=(dev->ipc->arm_app_ver & 0xFF0000)>>16;
			buff[5]=(dev->ipc->arm_app_ver & 0xFF00)>>8;
			buff[6]=dev->ipc->arm_app_ver&0xFF;
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)buff);
			break;
		
		case CM_ARM_ARI_VERSION_ACK:

            if(dev->ipc->autotest_flag == 1)
			{
				ver = dev->ipc->arm_ari_ver;
		        debug_message_send(dev, CM_PC_ARM_ARI_VERSION_ACK, hid, (u8 *)&ver, 4);
            }

			dev->ipc->arm_ari_ver = (msg->par.b[1] << 16) | (msg->par.b[2] << 8)|(msg->par.b[3]);
			//dbg_msgv(dev, "arm_ari_ver[%d,%d,%d]\r\n", msg->par.b[0],msg->par.b[1],msg->par.b[2]);
			buff[0]=0x06;
			buff[1]=0x62;
			buff[2]=DID_ARM_SF_VER>>8;
			buff[3]=DID_ARM_SF_VER&0xff;
			buff[4]=(dev->ipc->arm_ari_ver & 0xFF0000)>>16;
			buff[5]=(dev->ipc->arm_ari_ver & 0xFF00)>>8;
			buff[6]=dev->ipc->arm_ari_ver&0xFF;
			can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)buff);
			break;

		case CM_SEND_DISP_TYPE:
		//	buff[0]=dev->ipc->work_state &0x03;
		//	dbg_msgv(dev, "%x_%x  ",dev->ipc->work_state,msg->par.b[0]);
		//	dbg_msg(dev,"rec avm_view\r\n");
			dev->ipc->avm_disviewsts=msg->par.b[0];
		//	can_msg_Send_SWJ_Up(dev, 0x520,(u32*)buff);//放在 500msprocess发送
			break;
			
		case CM_RESET_SYSTEM:

			dbg_msg(dev, "request reset system\r\n");

			if(!msg->par.b[0])
			{
				dbg_msgf(dev, "do reset HOST\r\n");

				stimer_stop_all(dev);
				
		//		peripheral_power_ctrl(OFF);

			}
			else
			{
				dbg_msgf(dev, "do whole reset\r\n");
				
				car_can_deinit_simple(dev);
				
				car_can_sleep(dev);
				
				device_deinit(dev);

			//	power_on_gpio_ctrl(OFF);

				IO_SET(GPIOA, 3, LOW);
				mdelay(10);
				
				software_reset(); // never return
			}			

		break;	
		
		case CM_HEARTBEAT:
			dev->ipc->heart_count = 0;	

			dev->ipc->dtc_arm_com_sate=0;
		//	dbg_msg(dev, "heart beat\r\n");
			dev->ipc->work_state = 0x02;
			break;

		case CM_GENERAL_ACK:
           
     	  if(dev->ipc->autotest_flag == 1)
		  {					
			if(CM_SET_CAR_COLOUR == msg->par.b[0])
			{
                 buff[0] = CM_PC_SET_CAR_COLOUR;
              }
            else if(CM_VIDEO_CALIBRATION == msg->par.b[0])
			{
                 buff[0] = CM_PC_VIDEO_CALIBRATION;
              }
            else if(CM_SET_AVM_ALL_SWITCH == msg->par.b[0])
			{
                 buff[0] = CM_PC_SET_AVM_ALL_SWITCH;
              }
            else if(CM_SET_CAR_SUBLINE == msg->par.b[0])
			{
                 buff[0] = CM_PC_SET_CAR_SUBLINE;
              }
            else if(CM_SET_ADAS_FUNCTION_SWITCH == msg->par.b[0])
			{
                 buff[0] = CM_PC_SET_ADAS_FUNCTION_SWITCH;
              }
            else if(CM_SET_ADAS_FUNCTION_SPEED == msg->par.b[0])
			{    
                 buff[0] = CM_PC_SET_ADAS_FUNCTION_SPEED;
              }
            else if(CM_SEND_VEHICLE_INFO == msg->par.b[0])
			{
                 buff[0] = CM_PC_SEND_VEHICLE_INFO;
              }
            else if(CM_SEND_KEY == msg->par.b[0])
			{      
                 buff[0] = CM_PC_SET_AVM_ENABLE;
              }
            else if(CM_SET_CAR_COLOUR == msg->par.b[0])
			{
                 buff[0] = CM_PC_SET_CAR_COLOUR;
              }
			else if(CM_SET_MODE == msg->par.b[0])
			{
                 buff[0] = CM_PC_SET_MODE;
			  }
	
			buff[1] = 0x01;

        	debug_message_send(dev, CM_PC_GENERAL_ACK, hid, buff, 2);
            }
		  
			else
			{
				//dbg_msg(dev, "ARM normal ACK···\r\n");
			}
			break;
			
		case CM_SET_DISP_TYPE_ACK: // 自动化测试设置视图显示回应
			
 			buff[0] = msg->par.b[0];
    		if(dev->ipc->autotest_flag == 1)
			{
				
				debug_message_send(dev, CM_PC_SET_DISP_TYPE_ACK, hid, &buff[0], 1);
            	} 
			
		    break;
			
		case CM_GET_DISP_TYPE_ACK: // 自动化测试获取视图显示回应
		
		    buff[0] = msg->par.b[0];

			if(dev->ipc->autotest_flag == 1)
			{
	        	debug_message_send(dev, CM_PC_GET_DISP_TYPE_ACK, hid,  &buff[0], 1);
       		 }
	   		 break;
		
		case CM_GET_AVM_INFO_ACK:
		
        if(dev->ipc->autotest_flag == 1)
				{  
					for(i=0;i<11;i++)
					{
						buff[i] = msg->par.b[i];
					}
					
		        debug_message_send(dev, CM_PC_GET_AVM_INFO_ACK, hid, buff, 11);
        	}
		buff[0]=0x04;
		buff[1]=msg->par.b[1];
		can_msg_Send_SWJ_Up(dev, 0x0B3,(u32*)buff);
		
		break;
		
		case CM_GET_TURN_STAT:			
		//for  test
        if(dev->ipc->autotest_flag == 1)
				{ 
						buff[0] = msg->par.b[0];
		        debug_message_send(dev, CM_PC_GET_TURN_STAT, hid, buff, 1);
            }

		    break;

		case CM_TEST_SD_CARD_ACK:
			
			buff[0] = msg->par.b[0];

			if(dev->ipc->autotest_flag == 1)
			{
		        debug_message_send(dev, CM_PC_TEST_SD_CARD_ACK, hid, buff, 1);
            }
            
            break;



		case  CM_CHECK_USB_ACK:
			
			buff[0] = msg->par.b[0];

			if(dev->ipc->autotest_flag == 1)
			{
		        debug_message_send(dev, CM_PC_USB_ACK, hid, buff, 1);
            }
			break;

		case CM_CYCLE_SEND_HAIMA_AVM_INFO:
			//400ms cycle
			body_can591_data[0]=msg->par.w[0];
			body_can591_data[1]=msg->par.w[1];
			//can_msg_Send_SWJ_Up(dev, 0x591,msg->par.w);
			 break;

		default:

		//	dbg_msgv(dev, "msg id 0x%02x is not support, skip!\r\n", msg->id);
			
			break;
	}
		
	return 0;

//exit0:
//	return 1;
}

static void stage_booting_process(device_info_t *dev, message_t *msg, message_t *dmsg)
{
//	u8 buff[8]={0};
	dbg_msg(dev, "\r\n--- Enter BOOTING stage ---\r\n");
//	buff[0]=dev->ipc->work_state &0x03;	
//	start_timer(TIMER_DEV1,5000);
//	body_can_send_fill(dev, 0x520,(u32*)buff,1,"BC520");//kl15 on send immediately 
	while(HOST_BOOTING == dev->ci->state)
	{
		watchdog_feed(dev);
	
		dbg_uart_msg_process(dev, dmsg);

		IOSwitchSample();
			
		if(get_flag(dev, FLAG_ACC_LINE_ON))//鍒ゆ柇ACC鏄惁鎵撳紑
		{
			dbg_msg(dev, "ACC ON in booting, skip!\r\n");
			
			dev->ci->acc_line = ON;//ON=1  ACC鎵撳紑
		}	
		
		if(get_flag(dev, FLAG_ACC_LINE_OFF))//杩愯鏃惰幏鍙栨爣蹇椾綅涓篴cc off
		{
			dbg_msg(dev, "ACC OFF detected when running!\r\n");

			dev->ci->acc_line = OFF;
			
			dev->ipc->work_state =0x01;
	
			dev->ipc->can_wait_count =0x00;

		//	dev->ci->state = HOST_ACCOFF;
			
			break;	
		}

		/*check miss avm flame*/
		check_flame_miss(dev);


		if(get_flag(dev, FLAG_TIMEOUT_1S))
		{
			if(get_diag_dtc_function_state()==DIAG_DTC_FUNCTION_ON)
			{
	 		  camera_front_error_check(dev);
	 		  camera_back_error_check( dev);
	 		  camera_left_error_check(dev);
	 		  camera_right_error_check(dev);
			}
			}
		/*dtc  check 500ms*/	
		if(get_flag(dev,FLAG_TIMEOUT_500MS))
		{	
			if(get_diag_dtc_function_state()==DIAG_DTC_FUNCTION_ON)
				{

				arm_comm_fault_proc(dev);

				}
	
			}	



		if(get_flag(dev,FLAG_TIMEOUT_WD))//50ms  TEST cycle 
		{
			if(dev->dtc_t.votlage_dtc_flag)//KL15 ON=2s v>16v or <9v
			{
				if(get_diag_dtc_function_state()==DIAG_DTC_FUNCTION_ON)
				{
					Batt_adc_message_process(dev);//
					}
				}
			}


		if(get_flag(dev,FLAG_TIMEOUT_400MS))
		{
			
			send_591_flame(dev);
			}


		if(get_flag(dev,FLAG_TIMEOUT_20MS))//20ms  TEST cycle  busoff
		{
			if(get_diag_dtc_function_state()==DIAG_DTC_FUNCTION_ON)
			{
				can_buf_off_check(dev);
				}
			}

		if(get_flag(dev,FLAG_TIMEOUT_10MS))
		{
			if((get_diag_dtc_function_state()==DIAG_DTC_FUNCTION_ON)&&(dev->ci->flame_miss_dtc==0))
			{
				can_flame_missing_check(dev);//miss avm flame check
				}
			}

		if(dev->ci->arm_sleep==0x01)
		{//
		//	dbg_msg(dev,"reset 1\r\n");
	//		IO_SET(GPIOA, 3, LOW);
	
	//		mdelay(10);
				
	//		software_reset(); // never return 
			}
			
		can_link_receive_data(dev);  //can 消息处理

		can_tx_no_ack_check();
		
		can_bufoff_recovery(dev);
		
		check_s3_client(dev);
			
		can_msg_send_handle(dev);// body can send poll

		
		if(TRUE == comm_message_poll(dev, msg))
		{
			if(comm_booting_process(dev, msg))
			{
				break;
			}
		}

		wait_for_interrupt(); // for ADC process
	}
}
static void stage_running_process(device_info_t *dev, message_t *msg, message_t *dmsg)
{

	dbg_msg(dev, "\r\n--- Enter RUNNING stage ---\r\n");//鎵撳嵃杩涘叆running_process淇℃伅
		
	while(HOST_RUNNING == dev->ci->state)//HOST_RUNNING==2,  host normal stage
	{
		watchdog_feed(dev);//璁剧疆鐪嬮棬鐙楋紙杩欓噷鏈缃級

		dbg_uart_msg_process(dev, dmsg);//鎵撳嵃dbg淇℃伅锛堟棤锛?

		IOSwitchSample();

		if(dev->ci->arm_sleep==0x01)
		{
			
//			dbg_msg(dev,"reset 1\r\n");
			IO_SET(GPIOA, 3, LOW);
	
			mdelay(10);
				
			software_reset(); // never return 
			}
		

		if(get_flag(dev, FLAG_ACC_LINE_ON))//鍒ゆ柇ACC鏄惁鎵撳紑
		{
			dbg_msg(dev, "ACC ON in running, skip!\r\n");
			
			dev->ci->acc_line = ON;//ON=1  ACC鎵撳紑
		}
		
		
		if(get_flag(dev, FLAG_ACC_LINE_OFF))//杩愯鏃惰幏鍙栨爣蹇椾綅涓篴cc off
		{
			dbg_msg(dev, "ACC OFF detected when running!\r\n");

			dev->ci->acc_line = OFF;

			dev->ipc->work_state =0x01;

			dev->ipc->can_wait_count = 0;

			//dev->ci->state =HOST_ACCOFF;
			
			break;		
			}

	
		/*check miss avm*/
		check_flame_miss(dev);
		
		if(get_flag(dev, FLAG_TIMEOUT_1S))
		{
			if(get_diag_dtc_function_state()==DIAG_DTC_FUNCTION_ON)
			{
			 camera_front_error_check(dev);
			 camera_back_error_check( dev);
			 camera_left_error_check(dev);
			 camera_right_error_check(dev);
				}
			}
		
		/*dtc  check 500ms*/	
		if(get_flag(dev,FLAG_TIMEOUT_500MS))
		{	
			if(get_diag_dtc_function_state()==DIAG_DTC_FUNCTION_ON)
			{
				arm_comm_fault_proc(dev);
			}
	
		}

		if(get_flag(dev,FLAG_TIMEOUT_400MS))
		{
			
			send_591_flame(dev);
			}

		
		if(get_flag(dev,FLAG_TIMEOUT_WD))//50ms  TEST cycle 
		{
			if(dev->dtc_t.votlage_dtc_flag)//KL15 ON=2s v>16v or <9v
			{
				if(get_diag_dtc_function_state()==DIAG_DTC_FUNCTION_ON)
				{
					Batt_adc_message_process(dev);//
					}
				}
			}

		if(get_flag(dev,FLAG_TIMEOUT_20MS))//20ms  TEST cycle  busoff 
		{
			if(get_diag_dtc_function_state()==DIAG_DTC_FUNCTION_ON)
			{
				can_buf_off_check(dev);
				}
			}

		if(get_flag(dev,FLAG_TIMEOUT_10MS))
		{
			if((get_diag_dtc_function_state()==DIAG_DTC_FUNCTION_ON)&&(dev->ci->flame_miss_dtc==0))
			{
				can_flame_missing_check(dev);//miss avm flame check
				}
			}	

		if(get_flag(dev,FLAG_USART_ORE))
			{
				dbg_msg(dev,"usart rec Overrun error\r\n");
				}

	//	if(get_flag(dev, FLAG_UART_COMM_DMAT_END))
	//	{
//			usart_on_dma_end(dev->comm_uart);
	//	}
		
		can_link_receive_data(dev);//can处理

		can_tx_no_ack_check();
		
		can_bufoff_recovery(dev);

		check_s3_client(dev);

		can_msg_send_handle(dev);// body can
		
		if(TRUE == comm_message_poll(dev, msg))//judge 
		{		
			if(comm_running_process(dev, msg))
			{
				break;
			}
		}

		wait_for_interrupt(); 
	}
}

static void stage_upgrading_process(device_info_t *dev, message_t *msg, message_t *dmsg)
{
	dbg_msg(dev, "\r\n--- Enter upgrade mcu_boot  stage ---\r\n");//鎵撳嵃杩涘叆running_process淇℃伅
	while (HOST_UPGRADING==dev->ci->state  )
	{	
		watchdog_feed(dev);

		dbg_uart_msg_process(dev, dmsg);
	
		can_link_receive_data(dev);

		check_s3_client(dev);		

	/*	if(TRUE == comm_message_poll(dev, msg))
		{
			if(comm_upgrading_process(dev, msg))
			{
				break;
			}
		}*/
	}
}

static void stage_accoff_process(device_info_t *dev, message_t *msg, message_t *dmsg)
{	
	can_msg_t *can_msg_tmp=NULL;

	dev->ci->can_no_tx =1;//
	
	dbg_msgf(dev, "\r\n--- Enter ACC OFF stage ---\r\n");
	enable_err_retrans_mode(0); //
	
	while (HOST_ACCOFF==dev->ci->state  )
	{	
		watchdog_feed(dev);

		dbg_uart_msg_process(dev, dmsg);

	    can_msg_tmp  = car_can_recv(dev);
		
		if(dev->ci->wait_canMsg_time==0x01)//10s计时到
		{
			if(get_flag(dev, FLAG_ACC_LINE_ON))//
			{			

				dev->ci->can_no_tx =0;//
			
				dev->ci->state = HOST_RUNNING;
				enable_err_retrans_mode(1); 
				break;
			}

			if(can_msg_tmp==NULL)
			{				
				dev->ci->wait_canMsg_time=0x02; 
			//	dbg_msg(dev,"open timer\r\n");
				comm_message_send(dev, CM_ACC_OFF, 0, NULL, 0);//通知ARM ACC

				stimer_start(dev,FLAG_HOST_ACK,300);//3s定时器
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
				dev->ci->wait_canMsg_time=0x00;
				}
			}
		else if(dev->ci->wait_canMsg_time==0x00)
		{
			if(get_flag(dev, FLAG_ACC_LINE_ON))//
			{			
				dev->ci->can_no_tx =0;//
			
				dev->ci->state = HOST_RUNNING;
				enable_err_retrans_mode(1); 
				break;
			}
			
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
				 dev->ci->wait_canMsg_time=0x00;
				}	
			}
		else if(dev->ci->wait_canMsg_time==0x02)
		{
			if(get_flag(dev, FLAG_HOST_ACK))
			{
			//	dbg_msg(dev,"rec  timer\r\n");
				//stop_mode_proc(dev);
				dev->ipc->stop_module_flag=1;
			
				IO_SET(GPIOA, 3, LOW);

				mdelay(10);
				
				software_reset(); // never return
				
				break;
				}
			if(get_flag(dev, FLAG_ACC_LINE_ON))//
			{	

				dbg_msg(dev,"reset on \r\n");
				IO_SET(GPIOA, 3, LOW);

				mdelay(10);
				
				software_reset(); // never return

				break;
				}	
			if(can_msg_tmp!=NULL)
			{		
				if(can_msg_tmp->id >= 0x700 && can_msg_tmp->id <= 0x7ff )//diag 报文
  				{	
  					diag_receive_data(dev,can_msg_tmp);		
    				}
				else if((can_msg_tmp->id > 0x00) && (can_msg_tmp->id <= 0x700))
				{
					Body_Can_proc(dev,can_msg_tmp);
					}
				 dbg_msg(dev,"stop  timer\r\n");
				 dev->ipc->can_wait_count=0;
				 dev->ci->wait_canMsg_time=0x00;
				 get_flag(dev, FLAG_HOST_ACK);//清零
				 stimer_stop(dev,FLAG_HOST_ACK);
				}

			}

	//	if(get_flag(dev, FLAG_UART_COMM_DMAT_END))
		//{
		//	usart_on_dma_end(dev->comm_uart);
		//	}
	
		if(TRUE == comm_message_poll(dev, msg)) 
		{		
			if(comm_running_process(dev, msg))
			{
				break;
				}
			}

		}
			
//	wait_for_interrupt();
}


#if 0
static void stage_wakeup_process(device_info_t *dev, message_t *msg, message_t *dmsg)
{
//	u32 ret=0;

	dbg_msg(dev, "\r\n--- Enter WAKEUP stage ---\r\n");

	while(HOST_WAKEUP == dev->ci->state)
	{
		#if 0
		dbg_uart_msg_process(dev, dmsg);

		watchdog_feed(dev);

		if(get_flag(dev, FLAG_SHUTDN_TIMEOUT))
		{
			//stimer_stop_all(dev);
		
			//systick_stop();
			
			//wakeup2off_process(dev, HOST_SLEEP);
			
			//systick_start(dev);
			
			dbg_msg(dev, "shutdown !\r\n");
			{
				dbg_msgf(dev, "do whole reset\r\n");
					
				car_can_deinit_simple(dev);
				
				car_can_sleep(dev);
				
				device_deinit(dev);

				power_on_gpio_ctrl(OFF);

				software_reset(); // never return
			}

			break;
		}

		
		if(get_flag(dev, FLAG_ACC_LINE_ON))
		{
			dev->ci->acc_line = ON;

			if(stimer_stop(dev, FLAG_SHUTDN_TIMEOUT))//这里有问题
			{
				dbg_msg(dev, "delay-shutdown cancel, reset host!\r\n");

				//stimer_stop_all(dev);

				//gpio_timer_init();
				
				//other_line_gpio_init(dev);//
			
				//reset_host_process(dev);
				{
					dbg_msgf(dev, "do whole reset\r\n");
						
					car_can_deinit_simple(dev);
					
					car_can_sleep(dev);
					
					device_deinit(dev);

					power_on_gpio_ctrl(OFF);

					software_reset(); // never return
				}
				
				break;
			}
			else
			{
			    dbg_msg(dev, "ACC ON detected when wakeup, switch back!\r\n");

				stimer_stop(dev, FLAG_INA_TIMEOUT); // stop again
				
				systick_stop();
				
				dev->ci->state =  dev->ci->host_ready ? HOST_RUNNING : HOST_BOOTING;
				
				systick_start(dev);
				break;

			}
		}
		

		if(get_flag(dev, FLAG_ACC_LINE_OFF))
		{
			dbg_msg(dev, "ACC OFF detected when wakeup, skip\r\n");

			dev->ci->acc_line = OFF;
		}

		if(get_flag(dev, FLAG_B_LINE_ON))
		{
			dbg_msg(dev, "B+ ON detected when wakeup, skip!\r\n");

			dev->ci->b_line = ON;
		}

		if(get_flag(dev, FLAG_B_LINE_OFF))
		{
			dbg_msg(dev, "B+ OFF detected when sleep!\r\n");

			dev->ci->b_line = OFF;

			stimer_stop_all(dev);

			systick_stop();

			wakeup2off_process(dev, HOST_DEEP_SLEEP);

			systick_start(dev);

			break;
		}

		if(get_flag(dev, FLAG_INA_TIMEOUT))
		{
			dbg_msg(dev, "activity timer timeout(20s), start delay-power-off\r\n");
			
			peripheral_power_ctrl(OFF);

			stimer_stop_all(dev);

			if(!dev->ci->host_ready)
			{
				dbg_msg(dev, "host is not ready, shutdown directly!\r\n");
		
				systick_stop();
			
				wakeup2off_process(dev, HOST_SLEEP);
			
				systick_start(dev);

				break;
			}
			else
			{
				dbg_msg(dev, "notify host delay-power-off msg sent out\r\n");
				
				comm_message_send(dev, CM_ACC_OFF_DET, 0, NULL, 0);
				
				stimer_start(dev, FLAG_SHUTDN_TIMEOUT, TO_CNT_20S);
			}
		}


		#if IMX6_NOT_RESET
		if(get_flag(dev, FLAG_HOST_REST))
		{
			dbg_msg(dev, "Host reset finish\r\n");

			gpio_reset_host_leave();

			stimer_start(dev, FLAG_HOST_ACK, TO_CNT_20S);
		}

		if(get_flag(dev, FLAG_HOST_ACK))
		{
			dbg_msg(dev, "HOST BOOTUP ACK timeout, reboot!\r\n");

			stimer_stop_all(dev);
			
			comm_uart_deinit(dev);
			
			comm_uart_init(dev);
			
			gpio_reset_host_enter();
			
			stimer_start(dev, FLAG_HOST_REST, TO_CNT_300MS);
		}
		#endif

	//	Batt_adc_message_process(dev);

		
		can_link_receive_data(dev);//can处理
		
//		ret=Can_msg_proc(dev);
//		
//		if(ret)
//		{

//			dbg_msg(dev, "Refresh INA time 20s !\r\n");

//			stimer_restart(dev, FLAG_INA_TIMEOUT, TO_CNT_20S);
//		}
		

	//	if(get_flag(dev, FLAG_UART_COMM_DMAT_END))
	//	{
//			usart_on_dma_end(dev->comm_uart);
	//	}
	
		if(TRUE == comm_message_poll(dev, msg))
		{
			if(comm_wakeup_process(dev, msg))
			{
				break;
			}
		}

		wait_for_interrupt();
		#endif
	}
}
static void stage_sleep_process(device_info_t *dev, message_t *msg, message_t *dmsg)
{
	dbg_msg(dev, "\r\n--- Enter SLEEP stage ---\r\n");
	
	while(HOST_SLEEP == dev->ci->state)
	{
		dbg_uart_msg_process(dev, dmsg);

		watchdog_feed(dev);
		
		if(get_flag(dev, FLAG_ACC_LINE_ON))
	
			{
			dbg_msgf(dev, "ACC ON detected when sleep!\r\n");

			dev->ci->acc_line = ON;

			dev->ci->state = HOST_BOOTING;
				
		//	stimer_restart(dev, FLAG_HOST_ACK, TO_CNT_20S);//等待20s ARM响应
	
			break;
		}
		
		
		if(get_flag(dev, FLAG_ACC_LINE_OFF))
		{
			dbg_msg(dev, "ACC OFF detected when sleep, skip\r\n");

			dev->ipc->work_state =0x01;

			dev->ci->state =HOST_ACCOFF;

//			stop_mode_proc(dev);

//			mdelay(10);	
		
//			dbg_msg(dev,"\r\wake up, to reset mcu \r\n");
			
//			IO_SET(GPIOA, 3, LOW);
	
//			mdelay(10);
				
//			software_reset(); // never return	
			
			break;

		}


		#if 0
		if(get_flag(dev, FLAG_B_LINE_ON))
		{
			dbg_msg(dev, "B+ ON detected when sleep, skip!\r\n");

			dev->ci->b_line = ON;
		}

		if(get_flag(dev, FLAG_B_LINE_OFF))
		{
			dbg_msg(dev, "B+ OFF detected when sleep!\r\n");
			
			dev->ci->b_line = OFF;

			dev->ci->state = HOST_DEEP_SLEEP;

			break;
		}
				//远程唤醒 can
		if(get_flag(dev, FLAG_CARCAN_ON))
		{
			dbg_msgf(dev, "CAR CAN wakeup\r\n");

			dev->ci->carcan_wake = CAN_REMOTE_WAKEUP; // sos function is also remote wakeup msg

			systick_stop();
			
			wakeup_from_off_process(dev);
			
			systick_start(dev);

			break;
		}
		#endif
		wait_for_interrupt();
	}
}
static void stage_deep_sleep_process(device_info_t *dev, message_t *msg, message_t *dmsg)
{
	dbg_msg(dev, "\r\n--- Enter DEEP SLEEP stage ---\r\n");
	

	while(HOST_DEEP_SLEEP == dev->ci->state)
	{
		#if 0
		dbg_uart_msg_process(dev, dmsg);

		watchdog_feed(dev);

//		Batt_adc_message_process(dev);

		
		if(get_flag(dev, FLAG_ACC_LINE_ON))
		{
			dbg_msg(dev, "ACC ON in deep-sleep stage, waiting B+\r\n");
		
			dev->ci->acc_line = ON;	
		}
		
		
		if(get_flag(dev, FLAG_ACC_LINE_OFF))
		{
			dbg_msg(dev, "ACC OFF detected when deep sleep, skip!\r\n");
		
			dev->ci->acc_line = OFF;
		}

		if(get_flag(dev, FLAG_B_LINE_ON))
		{
			dbg_msg(dev, "B+ ON detected when deep sleep!\r\n");

			dev->ci->b_line = ON;

			if(ON == dev->ci->acc_line)
			{
				systick_stop();
			
				pwron_host_process(dev);

				systick_start(dev);
			}
			else
			{
				dev->ci->state = HOST_SLEEP;
			}

			break;
		}

		if(get_flag(dev, FLAG_B_LINE_OFF))
		{
			dbg_msg(dev, "B+ OFF detected when deep sleep, skip\r\n");

			dev->ci->b_line = OFF;
		}
		#endif
	}
}

static void stage_upgrading_process(device_info_t *dev, message_t *msg, message_t *dmsg)
{
	dbg_msgf(dev, "\r\n--- Enter UPGRADING stage ---\r\n");
	while(HOST_UPGRADING == dev->ci->state)
	{
		}
}
#endif



// order according to enum HOST_XXXX
static state_func_t sfunc[] =
{
	stage_booting_process,
	stage_upgrading_process,
	stage_running_process,
//	stage_wakeup_process,
	stage_accoff_process,
//	stage_sleep_process,
//	stage_deep_sleep_process,
};

state_func_t *get_stage_func_array(void)
{
	return sfunc;
}


//用于NM中断 1ms 中断一次
void TIM7_IRQHandler(void)/*定时器最大定时6500ms*/
{ 
//	dbg_msg(get_device_info(),"TIM7\r\n");
	NM_MainTask(&our_netid);
	
 	TIM7->SR = 0;//状态寄存器，硬件置1 软件清零
}



