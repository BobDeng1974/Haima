/******************************************************************************

                  ��Ȩ���� (C), 2017-2018, ���񵼺��Ƽ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : can_net_layer.c
  �� �� ��   : ����
  ��    ��   : xz
  ��������   : 2018��7��17��
  ����޸�   :
  ��������   : 
  �����б�   :
*
*       1.                can_link_receive_data
*       2.                can_net_layer_send_flow_control
*       3.                can_net_layer_send_send
*       4.                can_net_send
*       5.                check_frame_sequence_number
*       6.                diag_receive_data
*       7.                flow_control_frame_dealwith
*       8.                init_net_layer_parameer
*       9.                join_can_send_queue
*       10.                judge_rev_less_mintime
*       11.                judge_rev_over_maxtime
*       12.                send_consecutive_frame_first_frame
*       13.                send_consecutive_frame_second_frame
*
#define CAN_NEY_LAYER_GLOBALS
  �޸���ʷ   :
  1.��    ��   : 2018��7��17��
    ��    ��   : xz
    �޸�����   : �����ļ�

******************************************************************************/

#include  <string.h>
#include  "can_net_layer.h"
#include  "can_diag_protocol.h" 
#include  "iflash.h"
#include  "can.h"
#include  "api.h"
#include  "usart.h"
#include  "miscdef.h"
#include  "protocolapi.h"
#include "timer.h"
#include "nm.h"


#define dbg_msg(dev, str)   {}// ����dbg_msgΪ��

/*can���ķ��ࣺdiag �� ����*/
static DIAG_REC_TYPE         diag_rev;
static FLOW_FRAME_PARA_TYPE  rev_flow_frame_para;//���յ���������    �������
static FLOW_FRAME_PARA_TYPE  send_flow_frame_para;//������������     �ͻ��� 


//��������㷢�� �ṹ��
CAN_NET_LAYER_SEND_TYPE    can_net_layer_send;


#define DIAG_MODE 1


/*
*----------------------------------------------------------------------------------------------------------------------------------------
* ����ԭ�ͣ�void init_diag_parameter(void)
* �������ܣ���ʼ��diag��ز���
  ����˵����
  
* �����������
* �� �� ֵ����
  ���ߣ�    qdh
*----------------------------------------------------------------------------------------------------------------------------------------
*/

void init_net_layer_parameer(void)
{
	can_net_layer_send.valid   = 0;
	can_net_layer_send.step    = STOP_STEP;
	send_flow_frame_para.FS    = 0;
	send_flow_frame_para.BS    = 0;
	send_flow_frame_para.STmin = 100;//100ms
	//can_send_queue.num=0;//shield by lsr 20170912
	//rev_flow_frame_para.FS=
	//rev_flow_frame_para.BS=
	//rev_flow_frame_para.STmin=
}



//******************************************************************************
// DESCRIPTION:����㷢��Ӧ�ò�Ĵ���Դ��ָʾ
// ARUGMENT   :
// RETURN     :
// AUTHOR     :
//******************************************************************************
void dealwith_linklayer_error(device_info_t *dev,u8  N_Result)
{
  /*here,pad the function of dealwith any error of the link layer*/
    switch(N_Result)
    {
	 	case N_WAIT_NEXT_FC:     //��Է��������ϲ��ָʾ
	      dbg_msg(dev,"reset timer,wait for next FC\r\n");
	      break;
	    case N_BUFFER_OVFLW:     //��Է��������ϲ��ָʾ
	      dbg_msg(dev,"ERR: buff of receiver over flow\r\n");
	      break;
	    case N_INVALID_FS:
	      dbg_msg(dev,"ERR: FS is invalid\r\n");
	      break;
	    case N_CF_TIMEOUT:
	      dbg_msg(dev,"ERR: CF is timeout\r\n");
	      break;
	    case N_FLOW_OVER:
	      dbg_msg(dev,"ERR: the data is too long\r\n");    //��Խ��������ϲ��ָʾ
	      break;
	    case N_WRONG_SN:

				dbg_msg(dev, "ERR: SN is wrong\r\n");  
			
	      break;  
			default:
			   break;
    }
}


/*
*----------------------------------------------------------------------------------------------------------------------------------------
* ����ԭ�ͣ�void send_consecutive_frame_first_frame(void)
* �������ܣ����Ͷ�֡(ֻ����֡)
  ����˵����Data Ϊ��֡��ȥPCI ����
  
* �����������  
* �� �� ֵ����
  ���ߣ�    
*----------------------------------------------------------------------------------------------------------------------------------------
*/
void diag_send_data(device_info_t *dev, u8 *Data, u16 len )
{
	u8 buff[8]={0};
    //���巢��    
 	can_net_layer_send.had_send_len=0x00;
	can_net_layer_send.consecutive_frame_serial=0x00; //SN  ���
	//can_net_layer_send.len = len ;
	buff[0] = CONSECUTIVE_FRAME_FIRST_HEAD_0X10|((u8)(can_net_layer_send.len&0xff00)>>8);
 	buff[1] = can_net_layer_send.len&0xFF;
	memcpy(&buff[2],Data,6);//��Ӧ��Ϸ����SID����Ӧ�ظ�
	//dbg_msgv(dev, "������֡����DAT: %x %x %x %x %x %x %x %x������\r\n",buff[0],buff[1],buff[2],buff[3],buff[4],buff[5],buff[6],buff[7]);
	can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)buff);//������֡  �ȴ�����֡�����͵ڶ�֡���ж��ڽ���
	can_net_layer_send.had_send_len = 6;
	//������ʱ
	start_timer(TIMER_DEV3,WAINT_FLOW_CONTROL_FRAME_TIM);//�ȴ�����������Ʒ��� 150ms
//	printf("open TIM4, wait for ����\r\n");
    can_net_layer_send.step = CONSECUTIVE_FRAME_SECOND_FRAME_STEP;        //���ڶ���״̬������֡��һ֡
}

/*
*----------------------------------------------------------------------------------------------------------------------------------------
* ����ԭ�ͣ�void send_consecutive_frame_second_frame(void)
* �������ܣ������ں�8���������ݣ��ڶ���
  ����˵����  
* �����������
* �� �� ֵ����
  ���ߣ�    qdh
*----------------------------------------------------------------------------------------------------------------------------------------
*/
//	
void send_consecutive_frame_second_frame(device_info_t *dev)    
{
    u8   send_data[8]={0};
    u16    len = 0; 
    len = can_net_layer_send.len - can_net_layer_send.had_send_len;
    if(len>7)
    {
		len = 7;
    }
    can_net_layer_send.consecutive_frame_serial = (can_net_layer_send.consecutive_frame_serial+1)%0x10;
    send_data[0] = CONSECUTIVE_FRAME_SECOND_HEAD_0X20 | can_net_layer_send.consecutive_frame_serial;
    memcpy(&send_data[1],&can_net_layer_send.buff[can_net_layer_send.had_send_len],len);       
	//��������
	can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);
	//dbg_msg(dev, "��������֡�ڶ�֡\r\n");
	//�����ѷ��Ĳ���   
    can_net_layer_send.had_send_len = can_net_layer_send.had_send_len + len;
}


/*
*----------------------------------------------------------------------------------------------------------------------------------------
* ����ԭ�ͣ�void flow_control_frame_dealwith(void)
* �������ܣ���ʼ��diag��ز���, ���������ƻصĲ����������õȵ����� flow_dealwith
  ����˵������������ǻظ��� ������֡
  
* �����������
* �� �� ֵ����
  ���ߣ�    
*----------------------------------------------------------------------------------------------------------------------------------------
*/
void flow_control_frame_dealwith(device_info_t *dev,can_msg_t  *can_msg_temp,u8 mode)
{
	 uint8_t g_N_Result=0; 
	// dbg_msg(dev, "AVM��������֡���д���\r\n");
	  //����֡  ���Ȳ���
	 if( mode == DIAG_MODE )
	 {
		 if(get_diag_address_type() == DIAG_F_ADDRESS) //����Ѱַ��֧��	
			 return;
	 }
	 if(can_msg_temp->dlc!=0x08)
	 {
	 	dbg_msg(dev,"FC CAN.DLC is wrong,return\r\n");
		return ;
	 	}
	if(((can_msg_temp->dat.b[0]>>4) == 0x03)&&(can_net_layer_send.step==CONSECUTIVE_FRAME_SECOND_FRAME_STEP)) //CONSECUTIVE_FRAME_SECOND_FRAME_STEP ���ڶ���״̬  ������
  	{
        stop_timer(TIMER_DEV3);//Ӳ����ʱ��
      //  printf("��֡�ȴ������أ�close TIM4 �����ڶ�֡\r\n");
        can_net_layer_send.step   = STOP_STEP;
        rev_flow_frame_para.FS    = can_msg_temp->dat.b[0]&0x0f;
        rev_flow_frame_para.BS    = can_msg_temp->dat.b[1];
        rev_flow_frame_para.STmin = can_msg_temp->dat.b[2]; 

        if(rev_flow_frame_para.FS>0x02)        return;
        if(rev_flow_frame_para.BS>0xFF)       return;
        if(rev_flow_frame_para.STmin>0xFF)     return;    

			
        //rev_flow_frame_para.STmin �����ʱ��ת����us
//        rev_flow_frame_para.STmin=rev_flow_frame_para.STmin*1000;//STmin��λΪms
	//	printf("ST=%d\r\n",rev_flow_frame_para.STmin);
		  
        //�жϽ���������FS״̬��������ECU��������֡     
        if(rev_flow_frame_para.FS == 0)
        {  
         //���ڶ�֡
         send_consecutive_frame_second_frame(dev);            
				if(can_net_layer_send.had_send_len>=can_net_layer_send.len)
				{          
				can_net_layer_send.valid  = 0; //����ȫ������
				}
				else
				{
				can_net_layer_send.step = CONSECUTIVE_FRAME_THIRD_AND_AFTER_FRAME_STEP;//�����巢�����壨�������Ժ����״̬

				start_timer(TIMER_DEV3,(u16)rev_flow_frame_para.STmin);
	           
				g_N_Result = N_FC_RIGHT;
			}
        }
        //�ȴ���һ��������(��һ������)
        else if(rev_flow_frame_para.FS == 1)     //if the FS=1,restart the timer 
        {   
            //re-count time and wait for Another fc_frame

			start_timer(TIMER_DEV3,WAINT_FLOW_CONTROL_FRAME_TIM);//�ȴ���һ������
           
            can_net_layer_send.step = CONSECUTIVE_FRAME_SECOND_FRAME_STEP;  //���ڶ���״̬
            g_N_Result  = N_WAIT_NEXT_FC;
        }     
       //���������� 
        else if(rev_flow_frame_para.FS >= 2)     //the data would be sent is too long to receive for the tester
        {  
            can_net_layer_send.step = STOP_STEP;           
            can_net_layer_send.valid  = 0; 
            diag_rev.consecutive_frame_state = CONSECUTIVE_FRAME_STATE_FINISH;
            g_N_Result = N_BUFFER_OVFLW;  
            dealwith_linklayer_error(dev,g_N_Result);
            return;
        }
        dealwith_linklayer_error(dev,g_N_Result);
     	}
}



/*
*----------------------------------------------------------------------------------------------------------------------------------------
* ����ԭ�ͣ�ajust_rev_over_maxtime
* �������ܣ� �жϷ��ͷ�����N_Cr�����Է�����̫����
  			if the ECU is receiving diag data,and it didn't receive
			  any thing in 250ms.do report the error cf-timeout and set
			  the receive state to 0
  ����˵���� 
* ���������
* �� �� ֵ��            
  ���ߣ�    qdh
*----------------------------------------------------------------------------------------------------------------------------------------
*/
void judge_rev_over_maxtime(device_info_t *dev)
{
//	dbg_msgv(dev,"dev->diag_t.cf_curret_time=%d\r\n",dev->diag_t.cf_curret_time);
	if(( dev->diag_t.cf_curret_time - dev->diag_t.cf_last_time)> (REV_CONSECUTIVE_FRAME_OVER_TIM))//N_Cr ���շ���������֡�ĵȴ�ʱ��
	{
		dev->diag_t.cf_time_flag =0;
		if(diag_rev.consecutive_frame_state == CONSECUTIVE_FRAME_STATE_REVCEVING)
		{                
			diag_rev.consecutive_frame_state = CONSECUTIVE_FRAME_STATE_FINISH;
			dealwith_linklayer_error(dev,N_CF_TIMEOUT);            
		}
	}
	dev->diag_t.cf_curret_time=0;		
}
  

/*
*----------------------------------------------------------------------------------------------------------------------------------------
* ����ԭ�ͣ�ajust_rev_less_mintime
* �������ܣ� �жϷ��ͷ�С����С����ڣ���̫����
  ����˵���� 
* ���������
* �� �� ֵ��            
  ���ߣ�    qdh
*----------------------------------------------------------------------------------------------------------------------------------------
*/
void judge_rev_less_mintime(void)
{
	u32 curret_time;
	 //Ҫ��Է�����ٶȣ��Ͳ��ж�
	if(send_flow_frame_para.STmin==STMIN_0MS) 
	{
		return ;
	}
	if((curret_time-diag_rev.CF_last_frame_time)<= send_flow_frame_para.STmin)
	{
		if(diag_rev.consecutive_frame_state == CONSECUTIVE_FRAME_STATE_REVCEVING)
	    {                  
	        //diag_rev.consecutive_frame_state = CONSECUTIVE_FRAME_STATE_FINISH; //��ʱ��
	        //��ʾ�Է����ʹ����
	        //dealwith_linklayer_error(N_CF_TIMEOUT);            
	    }
	}

}



/*
*----------------------------------------------------------------------------------------------------------------------------------------
* ����ԭ�ͣ�void can_net_layer_send_flow_control(uint8_t FS,uint8_t BS,uint8_t STmin)
* �������ܣ� ���������Ĳ�����������������
  ����˵���� 
* �����������������ֵ�ֱ��ӦΪFS  BS STIM
* �� �� ֵ��
            
  ���ߣ�    qdh
*----------------------------------------------------------------------------------------------------------------------------------------
*/

void can_net_layer_send_flow_control(device_info_t *dev,u32 id,u8 FS,u8 BS,u8 STmin)
{
	u8 send_data[8] = {0};
	u8 send_enable  =  DISABLE;

	if( id >= 0x700 && id <= 0x7ff )
	{   //���ID������֡����
		if( id == AVM_Phy_Resp )
		{
			send_enable = ENABLE;
		}
	}
	else
	{   //��ͨID������֡����x
		send_enable = ENABLE;
		dbg_msg(dev, "read others flow control enable\r\n");
	}

	if( send_enable == 1 )
	{	//�����Ƹ�ʽ
		send_data[0] = 0x30|FS;
		send_data[1] = BS;
		send_data[2] = STmin;
		send_data[3] = FIX_NO_USE_DATA;
		send_data[4] = FIX_NO_USE_DATA;
		send_data[5] = FIX_NO_USE_DATA;	 
		send_data[6] = FIX_NO_USE_DATA;
		send_data[7] = FIX_NO_USE_DATA;	 
	//	dbg_msg(dev, "AVM��������֡\r\n");
		can_msg_Send_SWJ_Up(dev,id,(u32*)send_data);	 
	}
}

/*
*----------------------------------------------------------------------------------------------------------------------------------------
* ����ԭ�ͣ�check_frame_sequence_number(uint8_t current_frame_sn,uint8_t last_frame_sn) 
* �������ܣ� ����������Ϊ 1X~21~2F 20~2F,//�ж�����������Ƿ���ȷ
  ����˵���� 
* ���������current_frame_sn    ��ǰ������ last_frame_sn ��һ������
* �� �� ֵ��error:return 0
            right:return 1
  ���ߣ�    qdh
*----------------------------------------------------------------------------------------------------------------------------------------
*/
uint8_t check_frame_sequence_number(uint8_t current_frame_sn,uint8_t last_frame_sn)
{ 
	if((last_frame_sn >=0x10) &&(last_frame_sn < 0x20))	
    {
		if(current_frame_sn != 0x21) //����֡��һ��SN=1  �����ΪF
			return 0;
		else                            
			return 1;
    }
    else if((last_frame_sn >= 0x20)&&(last_frame_sn<0x2f))
    {
        if(current_frame_sn != last_frame_sn + 1)
        	return 0;
        else
        	return 1;
    }
    else if(last_frame_sn == 0x2f)
    {
        if(current_frame_sn != 0x20)
        	return 0;
        else
          return 1;       
    }
    else
      return 0;
}


/*
*----------------------------------------------------------------------------------------------------------------------------------------
* ����ԭ�ͣ�void diag_receive_data(void)
* �������ܣ� ������������ ����,��Ҫ������ ���������壬������ӵ����������������
  ����˵���� 
* �����������
* �� �� ֵ����
  ���ߣ�    xz
*----------------
�˺�����ò��Ӵ�ӡ��Ϣ*/
void diag_receive_data(device_info_t *dev,can_msg_t  *p_msg)
{  
	//���ض������
	if(NULL == p_msg)
	{
		return ;
	}
	/*
	if(!dev->ci->host_ready)
	{
		dbg_msg(dev, "������can msg recv when host not ready, skip������\r\n");
		return ;
	}*/
	if(p_msg->id == AVM_Phy_Req)//����Ѱַ
	{		 
		set_diag_address_type(DIAG_P_ADDRESS);
	}
	else if(p_msg->id == FUNCATIONAL_REQ_ADDR)
	{
		set_diag_address_type(DIAG_F_ADDRESS);		
	}
	else
	{
		return ;
	}
	//dbg_msgv(dev, "DIAG-ID: 0x%x, DAT: %08x_%08x������\r\n", p_msg->id, p_msg->dat.w[1], p_msg->dat.w[0]);

	if( p_msg->dlc!=0x08) //������ϱ���dlc=8������8���0x00
	{
		return;    
		}

	//�����
    if(p_msg->dat.b[0] < 0x08)//Data[0]Ϊ���ݳ��ȣ��� �ж�Ϊ��֡������             
    {
        if((can_net_layer_send.step == CONSECUTIVE_FRAME_SECOND_FRAME_STEP)||(can_net_layer_send.step == CONSECUTIVE_FRAME_THIRD_AND_AFTER_FRAME_STEP))
        {
          return;
        }   
        if(p_msg->dat.b[0] == 0)                                
				{
					return;
				}
        if((p_msg->dat.b[0] > 0x07) &&(p_msg->dat.b[0]<0x0f))//��֡Data[0]����λΪ0 ������λΪ���ݳ���
				{
					return;
				}
				diag_rev.consecutive_frame_sum = 0;//����
        memcpy(diag_rev.buf, &p_msg->dat.b[1], 7);//���ݵڶ����ֽڵ�ǰ7λ����buf     
        diag_rev.total_len=p_msg->dat.b[0];  //���ݱ��ĵ�һ���ֽڴ洢���ݳ���
        app_diagnose(dev,diag_rev.buf,diag_rev.total_len); //��֡��������㴦��  �������������Э�鴦��    
			}
		
    //��������֡
    else if((p_msg->dat.b[0] >= 0x10) &&(p_msg->dat.b[0] < 0x20))  //receive first frame ==0x10
    {	
		if((can_net_layer_send.step == CONSECUTIVE_FRAME_SECOND_FRAME_STEP)||(can_net_layer_send.step == CONSECUTIVE_FRAME_THIRD_AND_AFTER_FRAME_STEP))
		{
			return;//�ڷ�������֡״̬���ܽ��ܶ�֡
		}   
		//�ܳ���
		if(get_diag_address_type()==DIAG_F_ADDRESS) //����Ѱַ�˳�
		{
			return;
		}
		diag_rev.total_len = (p_msg->dat.b[0]&0x0f)<<8;
		diag_rev.total_len = p_msg->dat.b[1] | diag_rev.total_len;//����DATA�ܳ��ȣ���ȥPCI���ܳ��ȣ�����SID��             
		//dbg_msgv(dev, "��������֡����ΪLEN=%d\r\n",diag_rev.total_len);
		if(diag_rev.total_len < 0x08)  //len_can_data
		{
			return;
		}
        //�ж��ܳ��Ⱥ�����
     if(diag_rev.total_len > DIAG_REV_BUFFER_MAX)  //�����Ʒ����ж��� 
      {           
			//���������� ���ԽӼ�������������                     
			send_flow_frame_para.FS=FS_OVLW;//���
			send_flow_frame_para.BS=BS_0;//BS����Ϊ0 ����ָʾ�������ڲ�����ݵķ����ڼ�������֡���ٷ���������֡
			if(get_diag_sesion_mode( ) == DIAG_PR_MODE)//��̻Ựģʽ
			{  
			    send_flow_frame_para.STmin=STMIN_0MS;//����֡��֡���  ����Ϊ0
			}
			else                             
			{
			    send_flow_frame_para.STmin = STMIN_DEFAULT_20MS;
			}						
			can_net_layer_send_flow_control(dev,AVM_Phy_Resp,send_flow_frame_para.FS,send_flow_frame_para.BS,send_flow_frame_para.STmin); //���ط����� ��������Ϣ			  
			dealwith_linklayer_error(dev,N_FLOW_OVER);//�������̫��
			return;
        }
				
        //���½���������Ĳ���
        diag_rev.consecutive_frame_state = CONSECUTIVE_FRAME_STATE_REVCEVING;  //means ECU is receiving the diagnose data
        diag_rev.consecutive_frame_sum = (diag_rev.total_len - 6)/7; //  ÿ�����ĺ�1��������� ����֡ռ6���ֽ�	
		if((diag_rev.total_len - 6)%7)
        {
			diag_rev.consecutive_frame_sum++;
        }		
		//dbg_msgv(dev, "����֡�ܸ���Ϊ:%d\r\n",diag_rev.consecutive_frame_sum);
        diag_rev.last_frame_sn = p_msg->dat.b[0];                          
        diag_rev.num=0;	//����
        memcpy(&diag_rev.buf[diag_rev.num], &p_msg->dat.b[2], 6);//Data��������ĵ�ַ�� 6���ֽڸ�buf�׵�ַ           
		diag_rev.num+=6;//��֡���ݴ洢
	
		//����Ѱַ�����ǵ�֡  ������Ѱַ�����·���������
		if(get_diag_address_type() == DIAG_P_ADDRESS)             
		{                          
            //���������� overflow��ʾ���ĳ��ȳ������շ�������㻺������С				
			send_flow_frame_para.FS=FS_SEND;
            send_flow_frame_para.BS=BS_0;	//����Ҫ�ȴ�����  ֱ�ӷ�������֡
            if(get_diag_sesion_mode( ) == DIAG_PR_MODE)
            {  
                send_flow_frame_para.STmin=STMIN_0MS;
            }		
            else                             
            {
            
                send_flow_frame_para.STmin = STMIN_DEFAULT_20MS;
            }
		//	printf("�������أ�STmin =%d \r\n", send_flow_frame_para.STmin);
		    can_net_layer_send_flow_control(dev,AVM_Phy_Resp,send_flow_frame_para.FS,send_flow_frame_para.BS,send_flow_frame_para.STmin);   	                        	
		 }
        else
        {
            diag_rev.consecutive_frame_sum = 0;  //mul-frame function address,no response
        } 
    }

	
    //20~2F������Ľ���
    else if((p_msg->dat.b[0] >= 0x20)&&(p_msg->dat.b[0] < 0x30))/*receive continuous frame  ����֡��Χ*/ 
    {
		judge_rev_over_maxtime(dev)	;
	
        if((can_net_layer_send.step == CONSECUTIVE_FRAME_SECOND_FRAME_STEP)||(can_net_layer_send.step == CONSECUTIVE_FRAME_THIRD_AND_AFTER_FRAME_STEP))
        {
            return;//�ڷ�������֡״̬���ܽ��ܶ�֡
        }  
		if(diag_rev.consecutive_frame_state == CONSECUTIVE_FRAME_STATE_FINISH)
		{                
			return;           
		}
        /*check the frame_head is right or not*/
        if(check_frame_sequence_number(p_msg->dat.b[0],diag_rev.last_frame_sn) == 0)//ͨ���ж���������Ƿ������ж������Ƿ�����ȷ
        {  
            diag_rev.consecutive_frame_state = CONSECUTIVE_FRAME_STATE_FINISH;     //ECU is not in the receiving mul-frame
            dealwith_linklayer_error(dev,N_WRONG_SN);
            return ;
        }
        //
        diag_rev.consecutive_frame_sum--;//�˴����ڽ���һ������֡��Ӧ�ü�ȥ1.
        //���½��ջ�����     
        memcpy(&diag_rev.buf[diag_rev.num],&p_msg->dat.b[1], 7); 
        diag_rev.num+=7;//
        if(diag_rev.consecutive_frame_sum==0)   //���һ��
        {
          if(diag_rev.num<diag_rev.total_len)  //   if((diag_rev.num + p_msg->DLC) < diag_rev.total_len)
            {
                return;    
            }
			 dev->diag_t.cf_time_flag =0;/**/
            diag_rev.consecutive_frame_state = CONSECUTIVE_FRAME_STATE_FINISH;     //ECU is not in the receiving mul-frame
            diag_rev.last_frame_sn = 0;
			//dbg_msg(dev, "��֡������ϣ�������ϲ㴦�� \r\n");
            app_diagnose(dev,diag_rev.buf,diag_rev.total_len);//����֡û������ ���͸�app_diag����
        }
        else
        {            
            diag_rev.last_frame_sn = p_msg->dat.b[0];//�������һ������֡  

					/*���շ���������֡�ĵȴ�ʱ�� N_Cr<150ms��������ʱ*/
			dev->diag_t.cf_last_time=0;/*������ܴ�CF��ʱ��Ϊ0 */
			dev->diag_t.cf_time_flag = 1;/*�����δ��ʱ 10ms ��λ*/
        } 
    }    
	
    //������������    ���Ͷ�֡��DTC��DID�ȣ�                  
    else if((p_msg->dat.b[0]>>4) == 0x03)/*receive flow control frame*/
    {
		flow_control_frame_dealwith(dev,p_msg,DIAG_MODE);
    }			
}

/**
**@brief:can��Ϣ������̣�����can�����Ա���  ������Ϣ��������Ϣ
**
**
**************/
u32 Body_Can_proc(device_info_t *dev, can_msg_t *cmsg)
{	
	static u8 Veh_info_buff[8]={0};//���붨��Ϊ��̬�ֲ���������Ȼÿ�ε��û����³�ʼ�����ᱣ��Ϊ��һ���޸ĵ�ֵ
	static u8 Veh_info_buff_last[8]={0};//
	#if ENABLE_HAIMA
	static body_car_rec_info_t  body_car_rec_data={0};
	u8  buf[8]={0};
	u8  send_arm=0;
	#endif
	//dbg_msgv(dev, "[Body] ID: 0x%x, DAT: %08x_%08x������\r\n", cmsg->id, cmsg->dat.w[1], cmsg->dat.w[0]);
	if(!dev->ci->host_ready)
	{
		dbg_msg(dev, "������can msg recv when host not ready, skip������\r\n");
		return 1;
	}
	//CANID  ���� 
	switch(cmsg->id)
	{
	#if ENABLE_HAIMA
	case 0x1F7:
		
		if(dev->ipc->miss_avm0==0)
		{
			stimer_stop(dev, FLAG_BODYCAN_MISS0);
			stimer_start(dev, FLAG_BODYCAN_MISS0, TO_CNT_1S);
			}
		else{
			dev->ipc->miss_avm0=0;
			stimer_start(dev, FLAG_BODYCAN_MISS0, TO_CNT_1S);
			}	

	//��λ
		body_car_rec_data.GearSelDisp = cmsg->dat.b[3]&0x0F;
		if(body_car_rec_data.GearSelDisp ==0x00)
		{
			Veh_info_buff[5] =0x05;	//p	
			//printf("P \r\n");
			}
  		else if(body_car_rec_data.GearSelDisp ==0x05)
		{
			Veh_info_buff[5] =0x04;//D
			//	printf("D \r\n");
			}
		else if(body_car_rec_data.GearSelDisp ==0x06)
		{
			Veh_info_buff[5] =0x00;//N
			//	printf("N \r\n");
			}
		else if(body_car_rec_data.GearSelDisp ==0x07)
		{
			Veh_info_buff[5] =0x01;//R
				//printf("R \r\n");
			}
		else {
			dbg_msg(dev,"��λֵ����\r\n");
			}
		
	//���� ����OK
		body_car_rec_data.VehSpdVd = (cmsg->dat.b[6]>>5)&0x01;
		if(body_car_rec_data.VehSpdVd)
		{
			body_car_rec_data.VehSpd = ((cmsg->dat.b[6]&0x1F)<<8)|(cmsg->dat.b[7]);
			if(body_car_rec_data.VehSpd > 0x1900)
			{
				break;
				}
			body_car_rec_data.VehSpd *=0.05625;
		//	dbg_msgv(dev, "vel-speed=%d\r\n",body_car_rec_data.VehSpd);
			Veh_info_buff[0] &= 0x00;//km/h
			Veh_info_buff[0] |=(u8)(body_car_rec_data.VehSpd&0x0F)<<4;
			Veh_info_buff[1]  =(u8)(body_car_rec_data.VehSpd>>4)&0xffu;
			}
		else
		{
			dbg_msg(dev,"������Ч \r\n");
			}
		 break;

	case 0x22C: 
		
		if(dev->ipc->miss_avm1==0)
		{
			stimer_stop(dev, FLAG_BODYCAN_MISS1);
			stimer_start(dev, FLAG_BODYCAN_MISS1, TO_CNT_500MS);
			}
		else{
			dev->ipc->miss_avm1=0;
			stimer_start(dev, FLAG_BODYCAN_MISS1, TO_CNT_500MS);
			}	

		//ת��ƿ���״̬
		body_car_rec_data.DirLampLeftSwitchSts  =(cmsg->dat.b[3]>>3)&0x01 ;
		body_car_rec_data.DirLampRightSwitchSts =(cmsg->dat.b[1]>>5)&0x01 ;
		if((body_car_rec_data.DirLampLeftSwitchSts == 0x00)&&(body_car_rec_data.DirLampRightSwitchSts ==0x00))
		{
			Veh_info_buff[4] &=0x0f;//��������
		//	printf("normal\r\n");
			}
		else if((body_car_rec_data.DirLampLeftSwitchSts == 0x00)&&(body_car_rec_data.DirLampRightSwitchSts ==0x01))
		{
			Veh_info_buff[4] &=0x0f;//
			Veh_info_buff[4] |=0x20;//��ת
		//	printf("right\r\n");
			}
		else if((body_car_rec_data.DirLampLeftSwitchSts == 0x01)&&(body_car_rec_data.DirLampRightSwitchSts ==0x00))
		{
			Veh_info_buff[4] &=0x0f;//
			Veh_info_buff[4] |=0x10;//��ת
			//printf("left\r\n");
			}
		else if((body_car_rec_data.DirLampLeftSwitchSts == 0x01)&&(body_car_rec_data.DirLampRightSwitchSts ==0x01))
		{
			Veh_info_buff[4] &=0x0f;//
			Veh_info_buff[4] |=0x30;//˫��
			//printf("left-right\r\n");
			}
				
		break;
	
	case  0x3D4: 

		if(dev->ipc->miss_avm2==0)
		{
			stimer_stop(dev, FLAG_BODYCAN_MISS2);
			stimer_start(dev, FLAG_BODYCAN_MISS2, TO_CNT_200MS);
			}
		else{
			dev->ipc->miss_avm2=0;
			stimer_start(dev, FLAG_BODYCAN_MISS2, TO_CNT_200MS);
			}
	
		body_car_rec_data.StrWhlAngleSign = (cmsg->dat.b[2]>>7)&0x01;
		body_car_rec_data.StrWhlAngle  = ((cmsg->dat.b[2]&0x7f)<<8)|cmsg->dat.b[3];
		if(body_car_rec_data.StrWhlAngle>0x1E78u)
		{
			dbg_msgv(dev, "SAS_SteeringAngleVD is invalid\r\n");
			break;
			}	
		if(body_car_rec_data.StrWhlAngleSign)
		{
			body_car_rec_data.StrWhlAngle = -body_car_rec_data.StrWhlAngle/170;
			}
		else
		{
			body_car_rec_data.StrWhlAngle = body_car_rec_data.StrWhlAngle/170;
				}
		Veh_info_buff[2] = (body_car_rec_data.StrWhlAngle & 0xffu);
		Veh_info_buff[3] =((body_car_rec_data.StrWhlAngle >> 8)&0xffu);
		break;

	case  0x539:
		
		if(dev->ipc->miss_avm3==0)
		{
			stimer_stop(dev, FLAG_BODYCAN_MISS3);
			stimer_start(dev, FLAG_BODYCAN_MISS3, TO_CNT_4S);
			}
		else{
			dev->ipc->miss_avm3=0;
			stimer_start(dev, FLAG_BODYCAN_MISS3, TO_CNT_4S);
			}

		// set avm 2d 3d
		if(body_car_rec_data.Avm2D3DSwitch != ((cmsg->dat.b[3]>>7)&0x01))
		{
			body_car_rec_data.Avm2D3DSwitch = (cmsg->dat.b[3]>>7)&0x01;
			send_arm = body_car_rec_data.Avm2D3DSwitch+1;
			comm_message_send(dev, CM_SET_MODE, 0, &send_arm, 1);
			}

		//���ó����߿���״̬
		if(body_car_rec_data.AvmGuidesSet!=((cmsg->dat.b[2]>>4)&0x03))
		{
			body_car_rec_data.AvmGuidesSet = (cmsg->dat.b[2]>>4)&0x03;
			if(body_car_rec_data.AvmGuidesSet == 0x02)
			{
				send_arm = 0x0b;
				comm_message_send(dev, CM_SET_CAR_SUBLINE, 0, &send_arm, 1);
				}
			if(body_car_rec_data.AvmGuidesSet == 0x01)
			{
				send_arm = 0x1b;
				comm_message_send(dev, CM_SET_CAR_SUBLINE, 0, &send_arm, 1);
					}
			}

		//����ƫ�빦��			
		if(body_car_rec_data.AVM_LDW_SET!=((cmsg->dat.b[2]>>4)&0x03))
		{
			body_car_rec_data.AVM_LDW_SET = (cmsg->dat.b[2]>>4)&0x03;
			if(body_car_rec_data.AVM_LDW_SET == 0x02)
			{
				send_arm = 0x0b;
				comm_message_send(dev, CM_SET_ADAS_FUNCTION_SPEED, 0, &send_arm, 1);
				}
			if(body_car_rec_data.AVM_LDW_SET == 0x01)
			{
				send_arm = 0x1b;
				comm_message_send(dev, CM_SET_ADAS_FUNCTION_SPEED, 0, &send_arm, 1);
					}
			}			

		//set car color
		if(body_car_rec_data.SET_CAR_color !=((cmsg->dat.b[3]>>4)&0x07))
		{
			body_car_rec_data.SET_CAR_color =((cmsg->dat.b[3]>>4)&0x07);
			if(body_car_rec_data.SET_CAR_color==0x00)
			{
				dbg_msg(dev,"set color invalid\r\n");
				}
			else
			{
				comm_message_send(dev, CM_SET_CAR_COLOUR, 0, &body_car_rec_data.SET_CAR_color, 1);	
				}
			}
		
	//��������2
			body_car_rec_data.TouScrPreKey2 =(cmsg->dat.b[1]&0x1F);
			if(body_car_rec_data.TouScrPreKey2 ==0x00)
			{
		//		dbg_msg(dev,"touch 2 invalid\r\n");
				}
			else if((0<body_car_rec_data.TouScrPreKey2)&&(body_car_rec_data.TouScrPreKey2<0x10))
			{
				comm_message_send(dev, CM_SET_ViewType, 0, &body_car_rec_data.TouScrPreKey2, 1);
				}
			else if(body_car_rec_data.TouScrPreKey2 ==0x10)
			{
				send_arm=1;
				comm_message_send(dev, CM_SEND_KEY, 0,&send_arm, 1);
				}
			else if(body_car_rec_data.TouScrPreKey2 ==0x11)
			{
				send_arm=2;
				comm_message_send(dev, CM_SEND_KEY, 0,&send_arm, 1);
				}
			else if(body_car_rec_data.TouScrPreKey2 ==0x12)
			{
				//comm_message_send(dev, CM_SET_MODE, 0,NULL, 0);//�ظ�����
				}
		break;	
		
	case 0x6c2:

		if(dev->ipc->miss_avm4==0)
		{
			stimer_stop(dev, FLAG_BODYCAN_MISS4);
			stimer_start(dev, FLAG_BODYCAN_MISS4, TO_CNT_10S);
			}
		else{
			dev->ipc->miss_avm4=0;
			stimer_start(dev, FLAG_BODYCAN_MISS4, TO_CNT_10S);
			}	
		body_car_rec_data.AVMOneKeyCaltionVd = cmsg->dat.b[3]&0x01;
		if(body_car_rec_data.AVMOneKeyCaltionVd)
		{
			body_car_rec_data.AVMOneKeyCaltion = (cmsg->dat.b[4]<<24)|(cmsg->dat.b[5]<<16)|(cmsg->dat.b[6]<<8)|cmsg->dat.b[7];
			if(body_car_rec_data.AVMOneKeyCaltion==0xAA55AA55u)
			{
				send_arm = 0x00;
				//printf("open caltion \r\n");
				}
			else if(body_car_rec_data.AVMOneKeyCaltion==0x55AA55AAu)
			{
				send_arm = 0x01;
			//	printf("close caltion \r\n");
					}
			else
				{break;}
			comm_message_send(dev, CM_VIDEO_CALIBRATION, 0,&send_arm, 1);
			}
		else{}
		break;
	#endif 
	
	#if ENABLE_autotest
	    case 0x652:
		if(dev->ipc->autotest_flag == 1)
		{
			   buf[0] = 0x65;
			   buf[1] = 0x02;
			   buf[2] = 0x01;
			   buf[3] = 0x55;
			   buf[4] = 0x55;
			   buf[5] = 0x55;
			   buf[6] = 0x55;
			   buf[7] = 0x55;
			   can_msg_Send_SWJ(dev, 0x653,(u32*)buf); 
			}
	    break;
	#endif
	
	#if ENABLE_TEST
		case GET_MCU_VER_ID:
			buf[0] = 0;
			buf[1] = get_app_version()  >> 16;
			buf[2] = (get_app_version() >> 8) & 0xFF;
			buf[3] = get_app_version()  & 0xFF;
			buf[4] = 0;
			buf[5] = dev->ipc->boot_version>> 16;
			buf[6] = (dev->ipc->boot_version >> 8) & 0xff;
			break;
		case GET_ARM_VER_ID:
			di.data = buf;
			di.len = 0;
			comm_message_send1(dev, CM_ARM_APP_VERSION, 0, &di, 1);
			for( i= 0;i<2000;i++);
			comm_message_send1(dev, CM_ARM_ARI_VERSION, 0, &di, 1);
			break;
		case SET_VIDEO_VIEW_ID:
			buf[0] = cmsg->dat.b[0];
			di.data = buf;
			di.len = 1;
			comm_message_send1(dev, CM_SET_ViewType, 0, &di, 1);
			break;
	#endif
		default:break;
	}

	//��������Ϣ ���� ��Veh_info_buff ƥ����ARM��ͨѶЭ��  added 20180706��memcmp�ж���û�и��£����оͷ��ͣ���󽫴˴θ��µ�copy�� last����
	if(memcmp(Veh_info_buff,Veh_info_buff_last,8) != 0)//�����Ϊ��0
	{
	//	di.data = Veh_info_buff;
//		di.len = 8;
		comm_message_send(dev, CM_SEND_VEHICLE_INFO, 0, Veh_info_buff, 8);
//	 	dbg_msgv(dev, "send to ARM: data0:%x data1:%x data2:%x data3:%x data4:%x data5:%x  \r\n", Veh_info_buff[0],Veh_info_buff[1],Veh_info_buff[2],Veh_info_buff[3],Veh_info_buff[4],Veh_info_buff[5]);
	}
	memcpy(Veh_info_buff_last,Veh_info_buff ,8);
	return 1;
}


//extern const ECU_READ_DTC_CMD_CONST_TYPE   ecu_read_dtc_cmd_const[ECU_READ_DTC_MAX];


/*******************************************************************************
* Description: can_link_receive_data
* Argument   :  ����CANID ��Χ�������CAN������Ϣ 1������ 2��DIAG 3��NM
* Return     :     -1 ���ݳ��ȴ��󣬲���12��������
*                   0 ������ȡ��
*                   1 ȡ����Ч����
* Author     :
********************************************************************************/
void can_link_receive_data(device_info_t * dev)
{	
	can_msg_t  *can_msg_tmp=NULL;	
	can_msg_tmp  = car_can_recv(dev);
	dev=get_device_info();
	if(NULL == can_msg_tmp)
	{
		return ;
	}
	// ����ID�����䴦��;����1������ 2��NM����(��ѡ) 3��DIAG   С�˷��ͣ�morotola lsb ����
 	if( can_msg_tmp->id >= 0x700 && can_msg_tmp->id <= 0x7ff )//diag ����
  	{
			dev->diag_t.diag_rec_sts = 1;
			diag_receive_data(dev,can_msg_tmp);		
    	}
	else if((can_msg_tmp->id > 0x17f) && (can_msg_tmp->id <= 0x700))
	{
		if(get_diag_communication_state()==DIAG_COMM_RX_AND_TX_ENABLE)
			{
				Body_Can_proc(dev,can_msg_tmp);
		}
		else {
			dbg_msg(dev,"��ֹ������ͨѶ\r\n");
			return;
		}
	}
	else if((can_msg_tmp->id > 0xff) && (can_msg_tmp->id <= 0x17f))//for NM
	{
		dbg_msgv(dev, "NM_ID: 0x%x, DAT: %02x_%02x������\r\n", can_msg_tmp->id, can_msg_tmp->dat.b[0], can_msg_tmp->dat.b[1]);
		if(NM_Rx_Judge_ID_inrange(&our_netid,can_msg_tmp->id,((can_msg_tmp->id&(our_netid.nmBaseAddr))+can_msg_tmp->dat.b[0])))//�յ����籨��
		{
		 NMPduType kk_cnd={
								 0x17,
								 0x01,
								 {0x12}  
							 };

			 kk_cnd.Source = can_msg_tmp->id &0x000000FFul;
			 kk_cnd.Destination = can_msg_tmp->dat.b[0];
			 kk_cnd.OpCode.b  = can_msg_tmp->dat.b[1];
			 NM_RxIndication(&our_netid,&kk_cnd);
		}
	}
}

/*****************************************************************************
 �� �� ��  : TIM4_TRQHandler
 ��������  : �ȴ������ƶ�ʱ��
 �������  : i
             i
             i
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��7��20��
    ��    ��   : xz
    �޸�����   : �����ɺ���

*****************************************************************************/
void TIM4_IRQHandler( void )
{
	u16 len;
	u8 send_data[8]={0};
	// clear pending flag
	TIM4->SR = 0;//״̬�Ĵ�����Ӳ����1 ������� 
	//printf("TIM4  \r\n");
	//�ȴ��������峬ʱ  
 if(can_net_layer_send.step == CONSECUTIVE_FRAME_SECOND_FRAME_STEP)  //���ڶ���״̬
  {   
		stop_timer(TIMER_DEV3);
		can_net_layer_send.step = STOP_STEP;
		can_net_layer_send.valid  = 0;
		dbg_msg(get_device_info(),"wait FC failed, cancel send\r\n");
    } 
	  else if(can_net_layer_send.step == CONSECUTIVE_FRAME_THIRD_AND_AFTER_FRAME_STEP) //�����巢�����壨�������Ժ����״̬
    {
		//�ж���û�з��� the data had been sent over���ɲ���Ҫ���ж�
		if(can_net_layer_send.had_send_len>=can_net_layer_send.len)
		{
			stop_timer(TIMER_DEV3);
			can_net_layer_send.step = STOP_STEP;           
			can_net_layer_send.valid  = 0; 
		} 
		//������
		else
		{
			len = can_net_layer_send.len - can_net_layer_send.had_send_len;
			if(len >= 7u)
			{
				len = 7;
			}
			can_net_layer_send.consecutive_frame_serial = (can_net_layer_send.consecutive_frame_serial+1u)%0x10u;//1 .2 3 ~15 .0
			send_data[0]= CONSECUTIVE_FRAME_SECOND_HEAD_0X20 | can_net_layer_send.consecutive_frame_serial;
			memcpy(&send_data[1],&can_net_layer_send.buff[can_net_layer_send.had_send_len],len); //len=7         
			//��������
			can_msg_Send_SWJ_Up(get_device_info(), AVM_Phy_Resp,(u32*)send_data);         
			//���²���
			can_net_layer_send.had_send_len = can_net_layer_send.had_send_len + len;
			if(can_net_layer_send.had_send_len>=can_net_layer_send.len)
			{
				stop_timer(TIMER_DEV3);//����رն�ʱ��
				can_net_layer_send.step = STOP_STEP;           
				can_net_layer_send.valid  = 0;
				/*������� �����㷢��buff*/
				memset(&can_net_layer_send.buff[0],0,CAN_NET_LAYER_SEND_BUFF_MAX);
//				can_net_layer_send.buff[CAN_NET_LAYER_SEND_BUFF_MAX]={0};
				can_net_layer_send.len  = 0;
			}
		} 
    } 	
}

