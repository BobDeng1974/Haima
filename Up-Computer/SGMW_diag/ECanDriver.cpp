#include "StdAfx.h"
#include "ECanDriver.h"

#include "SGMW_diag.h"
#include "SGMW_diagDlg.h"

#include "winTest.h"

#pragma comment(lib,"ECanVci.lib")

static bool bStream = true;//���յ�������֡��־
CString avmLogStr;

const CString disp_diag_list[10] = {
	"0","1","2","3","4","5","6","7","8","9",
};

const CString disp_bitrate_list[13] = {
	"1000Kbps","800Kbps","666Kbps","500Kbps","400Kbps","250Kbps","200kbs","125Kbps","100Kbps","80Kbps","50Kbps","40Kbps","20Kbps",
};

const CString disp_fliter_list[2] = {
	"˫�˲�","���˲�",
};

const CString disp_mode_list[2] = {
	"����ģʽ","ֻ��ģʽ",
};


const unsigned char bitrate_to_time_list[] ={
	0x00,0x14,		//1000kbps
	0x00,0x16,		//800kbps
	0x80,0x86,		//666kbps
	0x00,0x1c,		//500kbps
	0x80,0xfa,		//400kbps
	0x01,0x1c,		//250kbps
	0x81,0xfa,		//200kbps
	0x03,0x1c,		//125kbps
	0x43,0x2f,		//100kbps
	0x83,0xff,		//80kbps
	0x47,0x2f,		//50kbps
	0x87,0xff,		//40kbps
	0x53,0x2f,		//20kbps
};

ECanDriver::ECanDriver(void)
{
	large_data_id_num = 0;
}


ECanDriver::~ECanDriver(void)
{
}
int ECanDriver::GetCanDriverStatus(void)
{
	return m_can_link_status;
}
int ECanDriver::CanDriverInit(void)
{
	map<CString,int>::iterator	ltr;
	int i = 0;

	//�����豸����
	m_mapDevtype["VCI_USBCAN1"]	= VCI_USBCAN1;
	m_mapDevtype["VCI_USBCAN2"]	= VCI_USBCAN2;
	m_devtype					= VCI_USBCAN1;

	m_can_link_status			= CAN_UNCONNECT;

	//�����豸�����ַ���
	ltr = m_mapDevtype.begin();
	for( dev_type_str_num = 0 ; dev_type_str_num < MAX_DEV_TYPE_NUM && ltr!=m_mapDevtype.end() ; dev_type_str_num++ )
	{
		dev_type_str_list[dev_type_str_num] = ltr->first;
		ltr++;
	}

	/* canͨ������ */
	for( dev_can_channel_str_num = 0 ; dev_can_channel_str_num < 4 ; dev_can_channel_str_num++ )
		dev_can_channel_str_list[dev_can_channel_str_num] = disp_diag_list[dev_can_channel_str_num];

	/* ������ */
	for( dev_can_idx_str_num = 0 ; dev_can_idx_str_num < 8 ; dev_can_idx_str_num++ )
		dev_can_idx_str_list[dev_can_idx_str_num] = disp_diag_list[dev_can_idx_str_num];

	/* ������ */
	for( dev_can_bitrate_str_num = 0 ; dev_can_bitrate_str_num < 13 ; dev_can_bitrate_str_num++ )
		dev_can_bitrate_str_list[dev_can_bitrate_str_num] = disp_bitrate_list[dev_can_bitrate_str_num];

	/* �˲� */
	for( dev_can_fliter_str_num = 0 ; dev_can_fliter_str_num < 2 ; dev_can_fliter_str_num++ )
		dev_can_fliter_str_list[dev_can_fliter_str_num] = disp_fliter_list[dev_can_fliter_str_num];

	/* ģʽ */
	for( dev_can_mode_str_num = 0 ; dev_can_mode_str_num < 2 ; dev_can_mode_str_num++ )
		dev_can_mode_str_list[dev_can_mode_str_num] = disp_mode_list[dev_can_mode_str_num];

	return 0;

}
/* ����N֡���ݵ�CAN������ */
int ECanDriver::TransmitDataSub(PVCI_CAN_OBJ pSend,int frame)
{
	int	res		= 0;
	int	retry	= 5;
	int	start	= 0;

	while(retry)
	{
		//DWORD dwErr = WaitForSingleObject(h_mutex,INFINITE);	//��ȡ�ź���
  //      if (dwErr != WAIT_OBJECT_0)
  //      {
  //           TRACE("[%d]VCI_Transmit  WaitForSingleObject(h_mutex error) %x\r\n",dwErr); 
  //      }
        //Sleep(1);
		res = VCI_Transmit(m_devtype , dev_can_idx_select , dev_can_channel_select , pSend + start , frame);
        //Sleep(100);
		//ReleaseMutex(h_mutex);					//�ͷ��ź���
        Sleep(1);
        if (res == 0xFFFFFFFF )
        {
            VCI_ERR_INFO errinfo = {0};

            DWORD dwErr =  VCI_ReadErrInfo(m_devtype,dev_can_idx_select,dev_can_channel_select,&errinfo);
            //TRACE("[%d]VCI_Transmit ERROR:%d  dwErrCode:%08x\r\n",GetTickCount(),dwErr,errinfo.ErrCode);
            if (dwErr == 0x4000)
            {
                CString str;
                str.Format("CAN ִ��ʧ�ܣ�%04x��������������CAN��",errinfo.ErrCode);
		        p_dlg->ShowInfo(str, 1);
                return 0;
            }
        }else
        {
             //TRACE("[%d]VCI_Transmit OK,return:%d  frame:%d\r\n",GetTickCount(),res,frame);
        }
		if( (res+start) == frame )
			return frame;						//���ͳɹ���ֱ���˳�
		else if( res > 0 )
		{
			start += res;
			res    = 0;
		}
        //TRACE("[%d]VCI_Transmit retry:%d\r\n",GetTickCount(),retry); 
		retry--;
	}
	return start;
}


UINT ECanDriver::TransmitHeartBeat(void *param)
{
	CSGMW_diagDlg *p_diag = (CSGMW_diagDlg*)param;

	VCI_CAN_OBJ frameinfo = {0};
	frameinfo.ID		  = 0x07df;		//id
	frameinfo.DataLen	= 8;		//len
	frameinfo.SendType	= 0;		//0,�������ͣ�1�����η��ͣ�2���Է����գ�3�������Է�����
	frameinfo.TimeFlag	= 0;		//no used
	frameinfo.TimeStamp	= 0;		//no used
	frameinfo.RemoteFlag	= 0;		//Զ��֡��ʾ��0������֡��1��Զ��֡
	frameinfo.ExternFlag	= 0;		//֡���͸�ʽ��0����׼֡��1����չ֡
	memset(frameinfo.Data, 0x00, 8);
	frameinfo.Data[0] = 0x02;
	frameinfo.Data[1] = 0x3e;
	frameinfo.Data[2] = 0x80;

	int res = p_diag->can_driver.TransmitDataSub(&frameinfo, 1);

	//��ӡ
	CString str;
	str.Format("��ӡ --> send: %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo.Data[0], 
		frameinfo.Data[1], frameinfo.Data[2], frameinfo.Data[3], frameinfo.Data[4],
		frameinfo.Data[5], frameinfo.Data[6], frameinfo.Data[7]);
	p_diag->ShowInfo(str, 1);


	/* �жϷ��͵�����֡�Ƿ�Ϊ1֡������ǣ����سɹ������򣬷���ʧ�� */
	if( res == 1 )
		return STATUS_OK;
	return STATUS_ERR;
}


/*
�������ݵ�CAN������
VCI_Transmit: ����Ϊ-1����ʾ����ʧ�ܣ����ͳ�ʱʱ��Ϊ2��
*/
int ECanDriver::TransmitData(unsigned int id, unsigned char* p_data, int len)
{
	int				res				=  0;
	//VCI_CAN_OBJ		frameinfo[64]	= {0};
	VCI_CAN_OBJ		frameinfo[1024]	= {0};
	CString			log_str;

	if( len > 4095 )
		return STATUS_ERR;
    
    //TRACE("[%d]TransmitData,Len=%d \r\n",GetTickCount(),len);

	if( len <= 8 )
	{	/* ��������С�ڵ���8��ֱ�ӷ��ͣ�����Ҫ�ְ� */
		frameinfo[0].ID			= id;		//id
		frameinfo[0].DataLen	= 8;		//len
		frameinfo[0].SendType	= 0;		//0,�������ͣ�1�����η��ͣ�2���Է����գ�3�������Է�����
		frameinfo[0].TimeFlag	= 0;		//no used
		frameinfo[0].TimeStamp	= 0;		//no used
		frameinfo[0].RemoteFlag	= 0;		//Զ��֡��ʾ��0������֡��1��Զ��֡
		frameinfo[0].ExternFlag	= 0;		//֡���͸�ʽ��0����׼֡��1����չ֡
		memcpy(&frameinfo[0].Data,p_data,len);

		/* ����һ֡���� */
		res = TransmitDataSub(frameinfo,1);

#if 1
		if(p_dlg->handle_flag)
		{
			//��ӡ
			CString str;
			CString strTime = p_dlg->comm.GetTime();
			str.Format("��ӡ -- [%s] --> send: %02x %02x %02x %02x %02x %02x %02x %02x", strTime, frameinfo[0].Data[0], 
				frameinfo[0].Data[1], frameinfo[0].Data[2], frameinfo[0].Data[3], frameinfo[0].Data[4],
				frameinfo[0].Data[5], frameinfo[0].Data[6], frameinfo[0].Data[7]);
			p_dlg->ShowInfo(str, 1);
		}
#else
		//��ӡ	--	����
		CString str;
		str.Format("��ӡ -- %04x --> send: %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo[0].ID, frameinfo[0].Data[0], 
			frameinfo[0].Data[1], frameinfo[0].Data[2], frameinfo[0].Data[3], frameinfo[0].Data[4],
			frameinfo[0].Data[5], frameinfo[0].Data[6], frameinfo[0].Data[7]);
		p_dlg->ShowInfo(str, 1);
#endif

		
		/* �жϷ��͵�����֡�Ƿ�Ϊ1֡������ǣ����سɹ������򣬷���ʧ�� */
		if( res == 1 )
			return STATUS_OK;
		return STATUS_ERR;
	}
	else
	{	/* �������������ݣ���Ҫ�����ݽ��зְ����� */
		int		i				= 0;
		int		data_len		= len;
		int		fram_cnt		= 0;
		int		timeout			= 250;//50;
		int		serial_number	= 0;
		int		send_frame_cnt	= 0;
		int		send_frame		= 0;
		int		send_data		= 0;
		int		failed_err		= 0;
		CAN_LARGE_DATA* p_can   = NULL;
		int		first_flag		= 0;

		log_str.Format("trans larg data: %x - %d",id,len);
		WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//��ȡ�ź���
		p_dlg->log.FlushLog(log_str);
		ReleaseMutex(p_dlg->h_log_mutex);					//�ͷ��ź���

		/* ��ȡ�����ݷ��ͻ���ָ�� */
		p_can = GetCanLargePdata(id,0);	
		if( p_can == NULL )
			return STATUS_ERR;
		if( p_can->flag )
			return STATUS_ERR;

		/* ���Ϳ�ʼ֡ */
		frameinfo[0].ID 		= id;		//id
		frameinfo[0].DataLen	= 8;		//len
		frameinfo[0].SendType	= 0;		//0,�������ͣ�1�����η��ͣ�2���Է����գ�3�������Է�����
		frameinfo[0].TimeFlag	= 0;		//no used
		frameinfo[0].TimeStamp	= 0;		//no used
		frameinfo[0].RemoteFlag = 0;		//Զ��֡��ʾ��0������֡��1��Զ��֡
		frameinfo[0].ExternFlag = 0;		//֡���͸�ʽ��0����׼֡��1����չ֡

		/* ����֡ͷ+���ݳ���,֡ͷ: bit4-bit7;���ݳ���:(bit0-bit4)+(bit8-bit15) */
		bStream     = true;
		p_can->flag = CAN_LARGE_SEND;
		if(len <= 256)
		{	
			first_flag = 1;
			frameinfo[0].Data[0]	= 0x10 ;
			memcpy(&frameinfo[0].Data[1],p_data,7);
			TransmitDataSub(frameinfo,1);

			if(p_dlg->handle_flag)
			{
				//��ӡ
				CString str;
				CString strTime = p_dlg->comm.GetTime();
				str.Format("��ӡ -- [%s] --> send: %02x %02x %02x %02x %02x %02x %02x %02x", strTime, frameinfo[i].Data[0], 
					frameinfo[i].Data[1], frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
					frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
				p_dlg->ShowInfo(str, 1);
			}
		}
		else
		{
			first_flag = 1;
			frameinfo[0].Data[0]	= 0x10 | (p_data[0] & 0xff);
			memcpy(&frameinfo[0].Data[1],p_data+1,7);
			TransmitDataSub(frameinfo,1);

			if(p_dlg->handle_flag)
			{
				//��ӡ
				CString str;
				CString strTime = p_dlg->comm.GetTime();
				str.Format("��ӡ -- [%s] --> send: %02x %02x %02x %02x %02x %02x %02x %02x", strTime, frameinfo[i].Data[0], 
					frameinfo[i].Data[1], frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
					frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
				p_dlg->ShowInfo(str, 1);
			}
		}
		
		p_can->ack  = CAN_NO;
		p_can->len  = len;
		timeout		= 200;

		CString str;
		//str.Format("begin wait frame(0x30) : %d",GetTickCount());
		//p_dlg->ShowInfo(str, 1);
		//WaitForSingleObject(h_mutex,INFINITE);	//��ȡ�ź���
		//while(p_can->ack == CAN_NO && timeout--)
		//log_str.Format("begin wait frame(0x30) : %d",GetTickCount());
		//WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//��ȡ�ź���
		//p_dlg->log.FlushLog(log_str);
		//ReleaseMutex(p_dlg->h_log_mutex);					//�ͷ��ź���

		while( (p_can->ack == CAN_NO) && (timeout--) && (bStream) )
			Sleep(1);	

		//str.Format("finish wait frame(0x30) : %d - %d",GetTickCount(),timeout);
		//p_dlg->ShowInfo(str, 1);

		//log_str.Format("finish wait frame(0x30) : %d - %d",GetTickCount(),timeout);
		//WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//��ȡ�ź���
		//p_dlg->log.FlushLog(log_str);
		//ReleaseMutex(p_dlg->h_log_mutex);					//�ͷ��ź���
		////ReleaseMutex(h_mutex);					//�ͷ��ź���

		if( timeout )
		{	/* ���յ��ն�Ӧ������,��ʼ������������ */
			if(len <= 256)
			{
				p_data   			+= 7;		/* ���¶�λ���ݿ�ʼλ�� */
				data_len 			-= 7;		/* ���¼�����Ҫ���͵����ݳ��� */
			}
			else
			{
				p_data   			+= 8;		/* ���¶�λ���ݿ�ʼλ�� */
				data_len 			-= 8;		/* ���¼�����Ҫ���͵����ݳ��� */
			}
			
			failed_err			 = 0;		/* ��ʼ��ʧ�ܴ˴� */
			serial_number 		 = 0;		/* ��ʼ����ˮ����Ϣ */
			large_data_sending	 = CAN_YES;	/* �����������ͱ�־ */

			/* ����ʣ��������Ҫ���Ͷ���֡ */
			fram_cnt  = data_len / 7;
			if( data_len % 7 )
				fram_cnt++;
			//TRACE("[%d]TransmitData,Total fram_cnt=%d \r\n",GetTickCount(),fram_cnt);
            
			/* ����֡���ݸ�ʽ�������� */
			//while( data_len && failed_err < 10 )
			//{
			/* ����һ����෢�͵�֡�� */
#if 0
			if( fram_cnt >= 16 )
				send_frame_cnt = 16;
			else
				send_frame_cnt = fram_cnt;
#elif 0
			if( fram_cnt >= 64 )
				send_frame_cnt = 64;
			else
				send_frame_cnt = fram_cnt;
#endif
			send_frame_cnt = fram_cnt;

			/* �����ݷ���frame��,һ�η���cnt��frame */
			for( i = 0 ; i < send_frame_cnt ; i++ )
			{
				frameinfo[i].ID 		= id;		//id
				frameinfo[i].DataLen	= 8;		//len
				frameinfo[i].SendType	= 0;		//0,�������ͣ�1�����η��ͣ�2���Է����գ�3�������Է�����
				frameinfo[i].TimeFlag	= 0;		//no used
				frameinfo[i].TimeStamp	= 0;		//no used
				frameinfo[i].RemoteFlag = 0;		//Զ��֡��ʾ��0������֡��1��Զ��֡
				frameinfo[i].ExternFlag = 0;		//֡���͸�ʽ��0����׼֡��1����չ֡
					
				/* (serial_number + i) & 0xf ����Ϊ0 */
		/*		if( ((serial_number + i) & 0xf) == 0 )
					serial_number++;*/

		/*		if( (i & 0xf) == 0 )
					serial_number++;*/

				//if( ((i&0x0f) == 0x0f) && (first_flag == 1) )
				//	first_flag = 0;

				///* ��֡���ŵĵ�һ����������0x21��0x2f����֮����ŵ���������0x20��0x2f���Դ����� */
				//if(first_flag)
				//{
				//	/* ����֡ͷ+���ݳ���,֡ͷ: bit4-bit7;���ݳ���:(bit0-bit4)+(bit8-bit15) */
				//	frameinfo[i].Data[0]	= 0x20 + ((serial_number+i)&0x0f);
				//	memcpy(&frameinfo[i].Data[1],p_data + (i * 7),7);
				//}
				//else
				//{
					/* ����֡ͷ+���ݳ���,֡ͷ: bit4-bit7;���ݳ���:(bit0-bit4)+(bit8-bit15) */
					frameinfo[i].Data[0]	= 0x20 + ((i+1)&0x0f);
					memcpy(&(frameinfo[i].Data[1]), &p_data[i * 7], 7);
			/*	}*/


					/* CAN֡��֮֡����ʱ */
					//for(int i = 0; i < 20000; i++)	;

					
				////��ӡ
			/*	if(!p_dlg->handle_flag)
				{
					CString str;
					str.Format("��ӡ --> send: %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo[i].Data[0], 
						frameinfo[i].Data[1], frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
						frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
					p_dlg->ShowInfo(str, 1);
				}*/
			}

			//��ӡ
			//if(p_dlg->handle_flag)
			//{
			//	CString str;
			//	CString strTime = p_dlg->comm.GetTime();
			//	str.Format("��ӡ -- [%s] --> send:", strTime);
			//	for( i = 0 ; i < send_frame_cnt ; i++ )
			//	{
			//		CString strCh;
			//		strCh.Format(" %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo[i].Data[0], 
			//			frameinfo[i].Data[1], frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
			//			frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);

			//		str += strCh;
			//	}
			//	p_dlg->ShowInfo(str, 1);
			//}

			TransmitDataSub(frameinfo,send_frame_cnt);

			///* ����send_frame_cnt֡���ݣ�����send_frame��ʾ���ͳɹ���֡�� */
   //         TRACE(" * ");

			//send_frame	= TransmitDataSub(frameinfo,send_frame_cnt);
   //            
			//if( send_frame )
			//{
			//	/* ȥ�����ͳ�ȥ�����ݣ����³�ʼ��ָ�������ݳ��� */
			//	send_data	 	 = send_frame * 7;	/* �����Ѿ����ͳ�ȥ�����ݳ��� */
			//	p_data			+= send_data;		/* ���¶�λ���ݿ�ʼλ�� */
			//	fram_cnt		-= send_frame;		/* ���¼���ʣ��֡�� */
			////	serial_number	+= send_frame;		/* ���¼�����ˮ�� */			//?

			//	/* ����ʣ�����ݳ��� */
			//	if( data_len >= send_data )
			//		data_len -= send_data;
			//	else
			//		data_len = 0;

			//	/* ����������Ա�־ */
			//	failed_err = 0;
			//}
			//else	/* ���ݷ���ʧ�� */
			//{
			//	failed_err++;
			//	large_data_sending = CAN_NO;
			//	if( failed_err > 10 )
			//	{
			//		log_str.Format("can ���ݷ���ʧ�ܣ�����10�Σ��˳�");
			//		//WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//��ȡ�ź���
			//		//p_dlg->log.FlushLog(log_str);
			//		//ReleaseMutex(p_dlg->h_log_mutex);					//�ͷ��ź���

			//		p_can->flag = CAN_LARGE_NO;
			//		return STATUS_ERR;
			//	}
   //                 TRACE("\r\n");


			//}
			//}
			p_can->flag = CAN_LARGE_NO;
			large_data_sending = CAN_NO;
			log_str.Format("can ���ݷ��ͳɹ����˳�");
			//WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//��ȡ�ź���
			//p_dlg->log.FlushLog(log_str);
			//ReleaseMutex(p_dlg->h_log_mutex);					//�ͷ��ź���
			return STATUS_OK;
		}
		else
		{
			log_str.Format("can ����δ���յ�����֡����ʱ�˳�");
			//WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//��ȡ�ź���
			//p_dlg->log.FlushLog(log_str);
			//ReleaseMutex(p_dlg->h_log_mutex);					//�ͷ��ź���
			p_can->flag = CAN_LARGE_NO;
			large_data_sending = CAN_NO;
			return STATUS_ERR;
		}
	}
	large_data_sending = CAN_NO;
	return STATUS_ERR;
}
/* ������������ݷ���ID */
int ECanDriver::AddLargeSendCanId(int tx_id,int rx_id)
{
	int i;
	for( i = 0 ; i < large_data_id_num ;i ++ )
	{
		if( large_data_id_list[i].rx_id == rx_id 
			&& large_data_id_list[i].tx_id == tx_id )
			return STATUS_OK;
	}

	if( large_data_id_num < MAX_LARGE_SEND_ID )
	{
		large_data_id_list[large_data_id_num].id     = large_data_id_num;
		large_data_id_list[large_data_id_num].len    = 0;
		large_data_id_list[large_data_id_num].flag   = 0;
		large_data_id_list[large_data_id_num].start  = 0;
		large_data_id_list[large_data_id_num].serial = 1;
		large_data_id_list[large_data_id_num].rx_id  = rx_id;
		large_data_id_list[large_data_id_num].tx_id  = tx_id;
		memset(large_data_id_list[large_data_id_num].data,0,MAX_LARGE_SIZE);

		large_data_id_num++;
		return STATUS_OK;
	}
	return STATUS_ERR;
}

/* �����������ݷ���ID */
int ECanDriver::ClearLargeSendCanId(void)
{
	large_data_id_num = 0;
	return STATUS_OK;
}

/* ͨ��tx id����rx id��ȡ��ǰ�����Ĵ����ݽṹ */
CAN_LARGE_DATA* ECanDriver::GetCanLargePdata(int tx_id, int rx_id)
{
	int i;
	if( tx_id )
	{
		for( i = 0 ; i < large_data_id_num ; i++ )
		{
			if(large_data_id_list[i].tx_id == tx_id )
				return &large_data_id_list[i];
		}
	}

	if( rx_id )
	{
		for( i = 0 ; i < large_data_id_num ; i++ )
		{
			if(large_data_id_list[i].rx_id == rx_id )
				return &large_data_id_list[i];
		}
	}
	return NULL;
}

int ECanDriver::GetCanTxRxId(int tx_id, int rx_id)
{
	int	i;
	if( tx_id )
	{
		for( i = 0 ; i < large_data_id_num ; i++ )
		{
			if(large_data_id_list[i].tx_id == tx_id )
				return large_data_id_list[i].rx_id;
		}
	}

	if( rx_id )
	{
		for( i = 0 ; i < large_data_id_num ; i++ )
		{
			if(large_data_id_list[i].rx_id == rx_id )
				return large_data_id_list[i].tx_id;
		}
	}

	return 0;
}

UINT ECanDriver::ReceiveThread(void *param)
{
	ECanDriver  *p_can			= (ECanDriver*)param;
	VCI_CAN_OBJ  frameinfo[250]	= {0};
	CAN_OBJ		 can_data		= {0};
	CAN_LARGE_DATA* p_can_data	= NULL;
	VCI_ERR_INFO errinfo		= {0};
	int			 frame_cnt		=  0;
	int			 save_flag		=  0;
	int			 i				=  0;
	CString		 log_str;

	while(1)
	{
		/* �Ͽ����Ӻ�ֱ���˳��߳� */
		if( p_can->m_can_link_status != CAN_CONNECT_OK )
			break;

		/* ����ģʽʱ��ֱ���˳��߳� */
		if( p_dlg->m_check_var.GetCheck() )
			break;

		WaitForSingleObject(p_can->h_mutex,INFINITE);	//��ȡ�ź���
        
		frame_cnt = VCI_Receive(p_can->m_devtype,p_can->dev_can_idx_select,p_can->dev_can_channel_select,frameinfo,250,200);
        //TRACE("[%d]Get CAN Data.\r\n",GetTickCount());
		ReleaseMutex(p_can->h_mutex);					//�ͷ��ź���	
				
		//if( frame_cnt )
		//{
		//	avmLogStr.Format("[AVM Debug Log | %s:%d] �����յ� %d ������֡��", __FUNCTION__, __LINE__, frame_cnt);
		//	p_dlg->ShowInfo(avmLogStr, 1);
		//	for( i = 0 ; i < frame_cnt ; i++ )
		//	{
		//		avmLogStr.Format("[Message[%d] Print] recv: %02x %02x %02x %02x %02x %02x %02x %02x", i, frameinfo[i].Data[0], frameinfo[i].Data[1]
		//						, frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4], frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
		//		p_dlg->ShowInfo(avmLogStr, 1);
		//	}
		//}

		if( frame_cnt )
		{
			for( i = 0 ; i < frame_cnt ; i++ )
			{	/* �����浱ǰ֡ */

				//p_dlg->ShowInfo(avmLogStr, 1);

				save_flag = CAN_YES;
				if( frameinfo[i].ID != p_dlg->pc_id )	//����CAN_ID
				{
					save_flag = CAN_NO;
					continue;
				}

				if( p_dlg->handle_flag )	//����or��������
				{

					//avmLogStr.Format("[AVM Debug Log | %s:%d] ��������or�������̷�֧...", __FUNCTION__, __LINE__);
					//p_dlg->ShowInfo(avmLogStr, 1);

					p_can_data = p_can->GetCanLargePdata(0,frameinfo[i].ID);
					if( p_can_data )
					{	/* ��ID���Խ��մ����� */

						if( p_can_data->flag == CAN_LARGE_SEND )
						{	/* ��ID���ڷ��ʹ����� */
							if( frameinfo[i].Data[0] == 0x30 && p_can_data->ack == CAN_NO )
							//if( (frameinfo[i].Data[0]>>4 & 0xff) == 0x03 && p_can_data->ack == CAN_NO )	//���յ�������֡
							{	/* ���յ�������������Ӧ��,�����ݲ����浽ringbuf�� */
								p_can_data->ack = CAN_YES;
								save_flag		= CAN_NO;
								//log_str.Format("update find 0x30 frame -  %d\r\n",GetTickCount());
								//WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//��ȡ�ź���
								//p_dlg->log.FlushLog(log_str);
								//ReleaseMutex(p_dlg->h_log_mutex);					//�ͷ��ź���
								bStream = false;
								
								//��ӡ
#if 0
								if(frameinfo[i].DataLen)
								{
									CString str;
									CString strTime = p_dlg->comm.GetTime();
									str.Format("��ӡ -- [%s] --> recv: %02x %02x %02x %02x %02x %02x %02x %02x", strTime, frameinfo[i].Data[0], 
										frameinfo[i].Data[1],frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
										frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
									p_dlg->ShowInfo(str, 1);
								}
#endif
							}
						}
						else if( p_can_data->flag == CAN_LARGE_RECEIVE )
						{	/* ��ID���ڽ��մ����� */

							if( (frameinfo[i].Data[0] & 0xf0) == 0x10 )
							{	/* �����ݷ��͵�һ֡,��Ҫ�������Ӧ��Ӧ��֡ */
								memset(p_can_data->data,0,4096);

								p_can_data->start = 0;
								p_can_data->flag  = CAN_LARGE_RECEIVE;
								p_can_data->len   = ((frameinfo[i].Data[0] & 0x0f) << 8) + frameinfo[i].Data[1] + 2;
								frameinfo[i].Data[0] = frameinfo[i].Data[0] & 0x0f;
								memcpy(p_can_data->data + p_can_data->start, &frameinfo[i].Data[0], 8);
								p_can_data->start  = 8;
								p_can_data->serial = 1;


								/* ����������֡ */
								memset(can_data.buf, 0x00, 8);
								can_data.buf[0] = 0x30;
								can_data.buf[1] = 0x00;
								can_data.buf[2] = 0x00;
								p_can->TransmitData(p_can_data->tx_id,can_data.buf,8);

								/* ���ݲ����� */
								save_flag = CAN_NO;
							}
							else if( (frameinfo[i].Data[0] & 0xf0) == 0x20 )
							{	/* �����м����� */
								if( p_can_data->serial == (frameinfo[i].Data[0] & 0x0f) )
								{	/* ��ˮ����ȷ */
									if( p_can_data->len > p_can_data->start )
									{
										if( (p_can_data->len - p_can_data->start) > 7 )
										{	/* �м�֡ */
											memcpy(p_can_data->data + p_can_data->start, &frameinfo[i].Data[1],7);
											p_can_data->start  += 7;
											p_can_data->serial ++;
											if( 16 == p_can_data->serial )
												p_can_data->serial = 0;
										}
										else
										{	/* ���һ֡ */
											memcpy(p_can_data->data + p_can_data->start, &frameinfo[i].Data[1],p_can_data->len - p_can_data->start);
											p_can_data->flag = CAN_LARGE_NO;

											/* �����ݱ��浽ringbuf�� */
											can_data.id  = frameinfo[i].ID;
											can_data.len = p_can_data->len;
											memcpy(can_data.buf,p_can_data->data,p_can_data->len );

											/* ��CAN���ݴ�ŵ�ringbuf�� */
											p_can->can_receive_buf.put_ringbuf((unsigned char*)&can_data);
										}
									}
								}
								else
								{	/* ��ˮ�Ŵ��󣬲��������� */
									p_can_data->flag = CAN_LARGE_NO;
								}

								/* ���ݲ����� */
								save_flag = CAN_NO;
							}
						}
						else
						{	/* �жϴ�ID�Ƿ��Ǵ����ݿ�ʼ֡ */
							if( (frameinfo[i].Data[0] & 0xf0) == 0x10 )
							{	/* �����ݷ��͵�һ֡,��Ҫ�������Ӧ��Ӧ��֡ */
								memset(p_can_data->data,0,4096);

								p_can_data->start = 0;
								p_can_data->flag  = CAN_LARGE_RECEIVE;
								p_can_data->len   = ((frameinfo[i].Data[0] & 0x0f) << 8) + frameinfo[i].Data[1] + 2;
								frameinfo[i].Data[0] = frameinfo[i].Data[0] & 0x0f;
								memcpy(p_can_data->data + p_can_data->start, &frameinfo[i].Data[0], 8);
								p_can_data->start  = 8;
								p_can_data->serial = 1;

								/* ����Ӧ������ */
								memset(can_data.buf, 0x00, 8);
								can_data.buf[0] = 0x30;
								can_data.buf[1] = 0x00;
								can_data.buf[2] = 0x00;
								p_can->TransmitData(p_can_data->tx_id,can_data.buf,8);

								/* ���ݲ����� */
								save_flag = CAN_NO;
							}
						}
					}

				}
				else	//��������
				{

					//avmLogStr.Format("[AVM Debug Log | %s:%d] �����������̷�֧...", __FUNCTION__, __LINE__);
					//p_dlg->ShowInfo(avmLogStr, 1);
					/* �ж��Ƿ��Ƿְ����� */
					p_can_data = p_can->GetCanLargePdata(0,frameinfo[i].ID);
					if( p_can_data )
					{	/* ��ID���Խ��մ����� */

						//avmLogStr.Format("[AVM Debug Log | %s:%d] ������մ����ݷ�֧...", __FUNCTION__, __LINE__);
						//p_dlg->ShowInfo(avmLogStr, 1);
						//if( (frameinfo[i].data[0] == 0x02) && (frameinfo[i].data[1] == 0x7e) )//
						//{	//���Ӱ�
						//	/* ���ݱ��� */
						//	save_flag = can_yes;
						//}
						//else 
						if( p_can_data->flag == CAN_LARGE_SEND )
						{	/* ��ID���ڷ��ʹ����� */
						
							if( frameinfo[i].Data[0] == 0x30 && p_can_data->ack == CAN_NO )
							//if( (frameinfo[i].Data[0]>>4 & 0xff) == 0x03 && p_can_data->ack == CAN_NO )	//���յ�������֡
							{	/* ���յ�������������Ӧ��,�����ݲ����浽ringbuf�� */
								p_can_data->ack = CAN_YES;
								save_flag		= CAN_NO;

								log_str.Format("find 0x30 frame -  %d\r\n",GetTickCount());
								//WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//��ȡ�ź���
								//p_dlg->log.FlushLog(log_str);
								//ReleaseMutex(p_dlg->h_log_mutex);					//�ͷ��ź���
#if 0
								//��ӡ
								if(frameinfo[i].DataLen)
								{
									CString str;
									CString strTime = p_dlg->comm.GetTime();
									str.Format("��ӡ -- [%s] --> recv: %02x %02x %02x %02x %02x %02x %02x %02x", strTime, frameinfo[i].Data[0], 
										frameinfo[i].Data[1],frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
										frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
									p_dlg->ShowInfo(str, 1);
								}
#endif
								bStream = false;
							}

						}
						else if( p_can_data->flag == CAN_LARGE_RECEIVE )
						{	/* ��ID���ڽ��մ����� */
							if( (frameinfo[i].Data[0] & 0xf0) == 0x10 )
							{	/* �����ݷ��͵�һ֡,��Ҫ�������Ӧ��Ӧ��֡ */
								memset(p_can_data->data,0,4096);

								p_can_data->start = 0;
								p_can_data->flag  = CAN_LARGE_RECEIVE;
								p_can_data->len   = ((frameinfo[i].Data[0] & 0x0f) << 8) + frameinfo[i].Data[1] + 1;
								memcpy(p_can_data->data + p_can_data->start, &frameinfo[i].Data[1],7);
								p_can_data->start  = 7;
								p_can_data->serial = 1;

								//avmLogStr.Format("[AVM Debug Log | %s:%d] ���յ�FF�����ݳ��ȣ�%d...", __FUNCTION__, __LINE__, p_can_data->len);
								//p_dlg->ShowInfo(avmLogStr, 1);

								//��ӡ
						/*		if(frameinfo[i].DataLen)
								{
									CString str;
									str.Format("��ӡ --> recv: %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo[i].Data[0], 
										frameinfo[i].Data[1],frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
										frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
									p_dlg->ShowInfo(str, 1);
								}*/


								/* ����������֡ */
								#if 0
								memset(can_data.buf,0,8);
								can_data.buf[0] = 0x30;
								p_can->TransmitData(p_can_data->tx_id,can_data.buf,8);
								#else
								memset(can_data.buf, 0x00, 8);
								can_data.buf[0] = 0x30;
								can_data.buf[1] = 0x00;
								can_data.buf[2] = 0x0A;
								p_can->TransmitData(p_can_data->tx_id,can_data.buf,8);

								//avmLogStr.Format("[AVM Debug Log | %s:%d] ȷ��FF������FC...", __FUNCTION__, __LINE__);
								//p_dlg->ShowInfo(avmLogStr, 1);
								#endif


								/* ���ݲ����� */
								save_flag = CAN_NO;
							}
							else if( (frameinfo[i].Data[0] & 0xf0) == 0x20 )
							{	/* �����м����� */

								//avmLogStr.Format("[AVM Debug Log | %s:%d] ���յ�CF,��ţ�%d...", __FUNCTION__, __LINE__, frameinfo[i].Data[0] & 0x0f);
								//p_dlg->ShowInfo(avmLogStr, 1);

								/*avmLogStr.Format("[CF Data Value] %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo[i].Data[0], 
										frameinfo[i].Data[1],frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
										frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
								p_dlg->ShowInfo(avmLogStr, 1);*/

								if( p_can_data->serial == (frameinfo[i].Data[0] & 0x0f) )
								{	/* ��ˮ����ȷ */
									if( p_can_data->len > p_can_data->start )
									{
										if( (p_can_data->len - p_can_data->start) > 7 )
										{	/* �м�֡ */
											memcpy(p_can_data->data + p_can_data->start, &frameinfo[i].Data[1],7);
											p_can_data->start  += 7;
											p_can_data->serial ++;

											if( 16 == p_can_data->serial )
												p_can_data->serial = 0;

											//��ӡ
									/*		if(frameinfo[i].DataLen)
											{
												CString str;
												str.Format("��ӡ --> recv: %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo[i].Data[0], 
													frameinfo[i].Data[1],frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
													frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
												p_dlg->ShowInfo(str, 1);
											}*/
										}
										else
										{	/* ���һ֡ */

											//avmLogStr.Format("[AVM Debug Log | %s:%d] ���յ����һ֡���������ݵ�buf...", __FUNCTION__, __LINE__);
											//p_dlg->ShowInfo(avmLogStr, 1);

											memcpy(p_can_data->data + p_can_data->start, &frameinfo[i].Data[1],p_can_data->len - p_can_data->start);
											p_can_data->flag = CAN_LARGE_NO;

											/* �����ݱ��浽ringbuf�� */
											can_data.id  = frameinfo[i].ID;
											can_data.len = p_can_data->len;
											memcpy(can_data.buf,p_can_data->data,p_can_data->len );

											//��ӡ
							/*				if(frameinfo[i].Data[0])
											{
												CString str;
												str.Format("��ӡ --> recv: %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo[i].Data[0], 
													frameinfo[i].Data[1],frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
													frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
												p_dlg->ShowInfo(str, 1);
											}*/

											/* ��CAN���ݴ�ŵ�ringbuf�� */
											p_can->can_receive_buf.put_ringbuf((unsigned char*)&can_data);
										}
									}
								}
								else
								{	/* ��ˮ�Ŵ��󣬲��������� */
									p_can_data->flag = CAN_LARGE_NO;
								}

								/* ���ݲ����� */
								save_flag = CAN_NO;
							}
							else
							{
								/*avmLogStr.Format("[AVM Debug Log | %s:%d] δ֪���ݣ�����", __FUNCTION__, __LINE__, p_can_data->len);
								p_dlg->ShowInfo(avmLogStr, 1);*/
							}
						}
						else
						{	/* �жϴ�ID�Ƿ��Ǵ����ݿ�ʼ֡ */
							if( (frameinfo[i].Data[0] & 0xf0) == 0x10 )
							{	/* �����ݷ��͵�һ֡,��Ҫ�������Ӧ��Ӧ��֡ */
								memset(p_can_data->data,0,4096);

								p_can_data->start = 0;
								p_can_data->flag  = CAN_LARGE_RECEIVE;
								p_can_data->len   = ((frameinfo[i].Data[0] & 0x0f) << 8) + frameinfo[i].Data[1] + 1;
								memcpy(p_can_data->data + p_can_data->start, &frameinfo[i].Data[1],7);
								p_can_data->start  = 7;
								p_can_data->serial = 1;


								//avmLogStr.Format("[AVM Debug Log | %s:%d] ���յ�FF�����ݳ��ȣ�%d...", __FUNCTION__, __LINE__, p_can_data->len);
								//p_dlg->ShowInfo(avmLogStr, 1);

								#if 0
								//��ӡ
								if(frameinfo[i].DataLen)
								{
									CString str;
									str.Format("��ӡ --> recv: %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo[i].Data[0], 
										frameinfo[i].Data[1],frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
										frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
									p_dlg->ShowInfo(str, 1);
								}
								#endif

								/* ����������֡ */
								#if 0
								memset(can_data.buf,0,8);

								p_can->TransmitData(p_can_data->tx_id,can_data.buf,8);
								#else
								memset(can_data.buf, 0x00, 8);
								can_data.buf[0] = 0x30;
								can_data.buf[1] = 0x00;
								can_data.buf[2] = 0x0A;
								p_can->TransmitData(p_can_data->tx_id,can_data.buf,8);

								//avmLogStr.Format("[AVM Debug Log | %s:%d] ȷ��FF������FC...", __FUNCTION__, __LINE__);
								//p_dlg->ShowInfo(avmLogStr, 1);

								#endif


								/* ���ݲ����� */
								save_flag = CAN_NO;
							}
							else
							{
								/*avmLogStr.Format("[AVM Debug Log | %s:%d] δ֪���ݣ�����", __FUNCTION__, __LINE__, p_can_data->len);
								p_dlg->ShowInfo(avmLogStr, 1);*/
							}
						}
					}

				}


				if( save_flag == CAN_YES )
				{	/* ֱ�Ӱ����浽ringbuf�� */

					memset(can_data.buf,0,4096);
					can_data.id  = frameinfo[i].ID;
					can_data.len = frameinfo[i].DataLen;
					memcpy(can_data.buf,frameinfo[i].Data,frameinfo[i].DataLen);

					/* ��CAN���ݴ�ŵ�ringbuf�� */
					p_can->can_receive_buf.put_ringbuf((unsigned char*)&can_data);
				}
			}
		}

		else
        {
			DWORD dw = VCI_ReadErrInfo(p_can->m_devtype,p_can->dev_can_idx_select,p_can->dev_can_channel_select,&errinfo);
           // TRACE (
         }

		Sleep(10);
	}
	return 0;
}

int ECanDriver::CanDriverReset(void)
{
	if( m_can_link_status != CAN_CONNECT_OK )
		return CAN_RESET_NOT_OPEN;

	if(VCI_ResetCAN(m_devtype,dev_can_idx_select,dev_can_channel_select)==1)
		return CAN_RESET_OK;
	else
		return CAN_RESET_FAILED;
}

int ECanDriver::CanDriverUnconnect(void)
{
	int res;

	Sleep(100);

	res = VCI_CloseDevice(m_devtype,dev_can_idx_select);
	if( res == STATUS_OK )
	{
		m_can_link_status = CAN_UNCONNECT;
		Sleep(100);
		if( h_mutex )
			CloseHandle(h_mutex);
		h_mutex			  = NULL;

		return 1;
	}
	return 0;
}

int ECanDriver::CanDriverConnect(void)
{
	VCI_INIT_CONFIG init_config;
	int				res			=  0;
	DWORD			code		=  0;
	DWORD			mask		=  0;

	/* �Ѿ�����CAN���ߣ�ֱ���˳� */
	if( m_can_link_status == CAN_CONNECT_OK )
		return CAN_CONNECT_OK;

	m_can_link_status = CAN_UNCONNECT;

	m_devtype = m_mapDevtype[dev_type_str];

	/* ��ȡ������ */
	code = _ttoi(unmask_code);
	/* ��ȡ������ */
	mask = _ttoi(mask_code);

	if( code < 0 || code > 0xffff )
		return CAN_CONNECT_CHECK_CODE_ERR;
	if(  mask < 0 || mask > 0xffff )
		return CAN_CONNECT_SHEILD_CODE_FORMAT_ERR;

	init_config.AccCode	= 0x00;			//code;
	init_config.AccMask	= 0xffffffff;	//mask;
	init_config.Filter	= 2;
	init_config.Mode	= 0;
	init_config.Timing0	= bitrate_to_time_list[dev_can_bitrate_select*2+0];
	init_config.Timing1	= bitrate_to_time_list[dev_can_bitrate_select*2+1];

	/* ���豸 */
	res = VCI_OpenDevice(m_devtype,dev_can_idx_select,0);
	if( res !=STATUS_OK)
		return CAN_CONNECT_OPEN_DRIVER_ERR;

	/* ��ʼ���豸 */
	res = VCI_InitCAN(m_devtype,dev_can_idx_select,dev_can_channel_select,&init_config);
	if( res != STATUS_OK )
		return CAN_CONNECT_INIT_DRIVER_ERR;

	/* ��ʼʹ��can�豸 */
	res = VCI_StartCAN(m_devtype,dev_can_idx_select,dev_can_channel_select);
	if( res != STATUS_OK )
		return CAN_CONNECT_START_ERR;

	if( can_receive_buf.flag == 0 )
		can_receive_buf.create_ringbuf(0,4096,MAX_LARGE_SIZE);

	m_can_link_status = CAN_CONNECT_OK;
	h_mutex = CreateMutex(NULL,FALSE,NULL);

	CWinThread *thrd = AfxBeginThread(ReceiveThread,this);
    thrd->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL );

	return CAN_CONNECT_OK;
}

int ECanDriver::CanDriverClose(void)
{
	if( m_can_link_status == CAN_CONNECT_OK )
	{
		m_can_link_status = CAN_UNCONNECT;
		Sleep(500);
		VCI_CloseDevice(m_devtype,dev_can_idx_select);
	}
	if( can_receive_buf.flag )
		can_receive_buf.delete_ringbuf();
	return STATUS_OK;
}
