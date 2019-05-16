#include "StdAfx.h"
#include "ECanDriver.h"

#include "SGMW_diag.h"
#include "SGMW_diagDlg.h"

#include "winTest.h"

#pragma comment(lib,"ECanVci.lib")

static bool bStream = true;//接收到流控制帧标志
CString avmLogStr;

const CString disp_diag_list[10] = {
	"0","1","2","3","4","5","6","7","8","9",
};

const CString disp_bitrate_list[13] = {
	"1000Kbps","800Kbps","666Kbps","500Kbps","400Kbps","250Kbps","200kbs","125Kbps","100Kbps","80Kbps","50Kbps","40Kbps","20Kbps",
};

const CString disp_fliter_list[2] = {
	"双滤波","单滤波",
};

const CString disp_mode_list[2] = {
	"正常模式","只听模式",
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

	//设置设备类型
	m_mapDevtype["VCI_USBCAN1"]	= VCI_USBCAN1;
	m_mapDevtype["VCI_USBCAN2"]	= VCI_USBCAN2;
	m_devtype					= VCI_USBCAN1;

	m_can_link_status			= CAN_UNCONNECT;

	//设置设备类型字符串
	ltr = m_mapDevtype.begin();
	for( dev_type_str_num = 0 ; dev_type_str_num < MAX_DEV_TYPE_NUM && ltr!=m_mapDevtype.end() ; dev_type_str_num++ )
	{
		dev_type_str_list[dev_type_str_num] = ltr->first;
		ltr++;
	}

	/* can通道处理 */
	for( dev_can_channel_str_num = 0 ; dev_can_channel_str_num < 4 ; dev_can_channel_str_num++ )
		dev_can_channel_str_list[dev_can_channel_str_num] = disp_diag_list[dev_can_channel_str_num];

	/* 索引号 */
	for( dev_can_idx_str_num = 0 ; dev_can_idx_str_num < 8 ; dev_can_idx_str_num++ )
		dev_can_idx_str_list[dev_can_idx_str_num] = disp_diag_list[dev_can_idx_str_num];

	/* 波特率 */
	for( dev_can_bitrate_str_num = 0 ; dev_can_bitrate_str_num < 13 ; dev_can_bitrate_str_num++ )
		dev_can_bitrate_str_list[dev_can_bitrate_str_num] = disp_bitrate_list[dev_can_bitrate_str_num];

	/* 滤波 */
	for( dev_can_fliter_str_num = 0 ; dev_can_fliter_str_num < 2 ; dev_can_fliter_str_num++ )
		dev_can_fliter_str_list[dev_can_fliter_str_num] = disp_fliter_list[dev_can_fliter_str_num];

	/* 模式 */
	for( dev_can_mode_str_num = 0 ; dev_can_mode_str_num < 2 ; dev_can_mode_str_num++ )
		dev_can_mode_str_list[dev_can_mode_str_num] = disp_mode_list[dev_can_mode_str_num];

	return 0;

}
/* 发送N帧数据到CAN总线上 */
int ECanDriver::TransmitDataSub(PVCI_CAN_OBJ pSend,int frame)
{
	int	res		= 0;
	int	retry	= 5;
	int	start	= 0;

	while(retry)
	{
		//DWORD dwErr = WaitForSingleObject(h_mutex,INFINITE);	//获取信号量
  //      if (dwErr != WAIT_OBJECT_0)
  //      {
  //           TRACE("[%d]VCI_Transmit  WaitForSingleObject(h_mutex error) %x\r\n",dwErr); 
  //      }
        //Sleep(1);
		res = VCI_Transmit(m_devtype , dev_can_idx_select , dev_can_channel_select , pSend + start , frame);
        //Sleep(100);
		//ReleaseMutex(h_mutex);					//释放信号量
        Sleep(1);
        if (res == 0xFFFFFFFF )
        {
            VCI_ERR_INFO errinfo = {0};

            DWORD dwErr =  VCI_ReadErrInfo(m_devtype,dev_can_idx_select,dev_can_channel_select,&errinfo);
            //TRACE("[%d]VCI_Transmit ERROR:%d  dwErrCode:%08x\r\n",GetTickCount(),dwErr,errinfo.ErrCode);
            if (dwErr == 0x4000)
            {
                CString str;
                str.Format("CAN 执行失败（%04x），请重新启动CAN。",errinfo.ErrCode);
		        p_dlg->ShowInfo(str, 1);
                return 0;
            }
        }else
        {
             //TRACE("[%d]VCI_Transmit OK,return:%d  frame:%d\r\n",GetTickCount(),res,frame);
        }
		if( (res+start) == frame )
			return frame;						//发送成功，直接退出
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
	frameinfo.SendType	= 0;		//0,正常发送；1，单次发送；2，自发自收；3，单次自发自收
	frameinfo.TimeFlag	= 0;		//no used
	frameinfo.TimeStamp	= 0;		//no used
	frameinfo.RemoteFlag	= 0;		//远程帧表示，0，数据帧；1，远程帧
	frameinfo.ExternFlag	= 0;		//帧类型格式，0，标准帧；1，扩展帧
	memset(frameinfo.Data, 0x00, 8);
	frameinfo.Data[0] = 0x02;
	frameinfo.Data[1] = 0x3e;
	frameinfo.Data[2] = 0x80;

	int res = p_diag->can_driver.TransmitDataSub(&frameinfo, 1);

	//打印
	CString str;
	str.Format("打印 --> send: %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo.Data[0], 
		frameinfo.Data[1], frameinfo.Data[2], frameinfo.Data[3], frameinfo.Data[4],
		frameinfo.Data[5], frameinfo.Data[6], frameinfo.Data[7]);
	p_diag->ShowInfo(str, 1);


	/* 判断发送的数据帧是否为1帧，如果是，返回成功；否则，返回失败 */
	if( res == 1 )
		return STATUS_OK;
	return STATUS_ERR;
}


/*
发送数据到CAN总线上
VCI_Transmit: 返回为-1，表示发送失败，发送超时时间为2秒
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
	{	/* 发送数据小于等于8，直接发送，不需要分包 */
		frameinfo[0].ID			= id;		//id
		frameinfo[0].DataLen	= 8;		//len
		frameinfo[0].SendType	= 0;		//0,正常发送；1，单次发送；2，自发自收；3，单次自发自收
		frameinfo[0].TimeFlag	= 0;		//no used
		frameinfo[0].TimeStamp	= 0;		//no used
		frameinfo[0].RemoteFlag	= 0;		//远程帧表示，0，数据帧；1，远程帧
		frameinfo[0].ExternFlag	= 0;		//帧类型格式，0，标准帧；1，扩展帧
		memcpy(&frameinfo[0].Data,p_data,len);

		/* 发送一帧数据 */
		res = TransmitDataSub(frameinfo,1);

#if 1
		if(p_dlg->handle_flag)
		{
			//打印
			CString str;
			CString strTime = p_dlg->comm.GetTime();
			str.Format("打印 -- [%s] --> send: %02x %02x %02x %02x %02x %02x %02x %02x", strTime, frameinfo[0].Data[0], 
				frameinfo[0].Data[1], frameinfo[0].Data[2], frameinfo[0].Data[3], frameinfo[0].Data[4],
				frameinfo[0].Data[5], frameinfo[0].Data[6], frameinfo[0].Data[7]);
			p_dlg->ShowInfo(str, 1);
		}
#else
		//打印	--	测试
		CString str;
		str.Format("打印 -- %04x --> send: %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo[0].ID, frameinfo[0].Data[0], 
			frameinfo[0].Data[1], frameinfo[0].Data[2], frameinfo[0].Data[3], frameinfo[0].Data[4],
			frameinfo[0].Data[5], frameinfo[0].Data[6], frameinfo[0].Data[7]);
		p_dlg->ShowInfo(str, 1);
#endif

		
		/* 判断发送的数据帧是否为1帧，如果是，返回成功；否则，返回失败 */
		if( res == 1 )
			return STATUS_OK;
		return STATUS_ERR;
	}
	else
	{	/* 发送批量的数据，需要对数据进行分包操作 */
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
		WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//获取信号量
		p_dlg->log.FlushLog(log_str);
		ReleaseMutex(p_dlg->h_log_mutex);					//释放信号量

		/* 获取大数据发送缓冲指针 */
		p_can = GetCanLargePdata(id,0);	
		if( p_can == NULL )
			return STATUS_ERR;
		if( p_can->flag )
			return STATUS_ERR;

		/* 发送开始帧 */
		frameinfo[0].ID 		= id;		//id
		frameinfo[0].DataLen	= 8;		//len
		frameinfo[0].SendType	= 0;		//0,正常发送；1，单次发送；2，自发自收；3，单次自发自收
		frameinfo[0].TimeFlag	= 0;		//no used
		frameinfo[0].TimeStamp	= 0;		//no used
		frameinfo[0].RemoteFlag = 0;		//远程帧表示，0，数据帧；1，远程帧
		frameinfo[0].ExternFlag = 0;		//帧类型格式，0，标准帧；1，扩展帧

		/* 设置帧头+数据长度,帧头: bit4-bit7;数据长度:(bit0-bit4)+(bit8-bit15) */
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
				//打印
				CString str;
				CString strTime = p_dlg->comm.GetTime();
				str.Format("打印 -- [%s] --> send: %02x %02x %02x %02x %02x %02x %02x %02x", strTime, frameinfo[i].Data[0], 
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
				//打印
				CString str;
				CString strTime = p_dlg->comm.GetTime();
				str.Format("打印 -- [%s] --> send: %02x %02x %02x %02x %02x %02x %02x %02x", strTime, frameinfo[i].Data[0], 
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
		//WaitForSingleObject(h_mutex,INFINITE);	//获取信号量
		//while(p_can->ack == CAN_NO && timeout--)
		//log_str.Format("begin wait frame(0x30) : %d",GetTickCount());
		//WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//获取信号量
		//p_dlg->log.FlushLog(log_str);
		//ReleaseMutex(p_dlg->h_log_mutex);					//释放信号量

		while( (p_can->ack == CAN_NO) && (timeout--) && (bStream) )
			Sleep(1);	

		//str.Format("finish wait frame(0x30) : %d - %d",GetTickCount(),timeout);
		//p_dlg->ShowInfo(str, 1);

		//log_str.Format("finish wait frame(0x30) : %d - %d",GetTickCount(),timeout);
		//WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//获取信号量
		//p_dlg->log.FlushLog(log_str);
		//ReleaseMutex(p_dlg->h_log_mutex);					//释放信号量
		////ReleaseMutex(h_mutex);					//释放信号量

		if( timeout )
		{	/* 接收到终端应答数据,开始批量发送数据 */
			if(len <= 256)
			{
				p_data   			+= 7;		/* 重新定位数据开始位置 */
				data_len 			-= 7;		/* 重新计算需要发送的数据长度 */
			}
			else
			{
				p_data   			+= 8;		/* 重新定位数据开始位置 */
				data_len 			-= 8;		/* 重新计算需要发送的数据长度 */
			}
			
			failed_err			 = 0;		/* 初始化失败此处 */
			serial_number 		 = 0;		/* 初始化流水号信息 */
			large_data_sending	 = CAN_YES;	/* 大数据量发送标志 */

			/* 计算剩余数据需要发送多少帧 */
			fram_cnt  = data_len / 7;
			if( data_len % 7 )
				fram_cnt++;
			//TRACE("[%d]TransmitData,Total fram_cnt=%d \r\n",GetTickCount(),fram_cnt);
            
			/* 按照帧数据格式发送数据 */
			//while( data_len && failed_err < 10 )
			//{
			/* 计算一次最多发送的帧数 */
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

			/* 将数据放入frame中,一次发送cnt个frame */
			for( i = 0 ; i < send_frame_cnt ; i++ )
			{
				frameinfo[i].ID 		= id;		//id
				frameinfo[i].DataLen	= 8;		//len
				frameinfo[i].SendType	= 0;		//0,正常发送；1，单次发送；2，自发自收；3，单次自发自收
				frameinfo[i].TimeFlag	= 0;		//no used
				frameinfo[i].TimeStamp	= 0;		//no used
				frameinfo[i].RemoteFlag = 0;		//远程帧表示，0，数据帧；1，远程帧
				frameinfo[i].ExternFlag = 0;		//帧类型格式，0，标准帧；1，扩展帧
					
				/* (serial_number + i) & 0xf 不能为0 */
		/*		if( ((serial_number + i) & 0xf) == 0 )
					serial_number++;*/

		/*		if( (i & 0xf) == 0 )
					serial_number++;*/

				//if( ((i&0x0f) == 0x0f) && (first_flag == 1) )
				//	first_flag = 0;

				///* 首帧跟着的第一串流数据由0x21～0x2f，再之后接着的流数据由0x20～0x2f，以此类推 */
				//if(first_flag)
				//{
				//	/* 设置帧头+数据长度,帧头: bit4-bit7;数据长度:(bit0-bit4)+(bit8-bit15) */
				//	frameinfo[i].Data[0]	= 0x20 + ((serial_number+i)&0x0f);
				//	memcpy(&frameinfo[i].Data[1],p_data + (i * 7),7);
				//}
				//else
				//{
					/* 设置帧头+数据长度,帧头: bit4-bit7;数据长度:(bit0-bit4)+(bit8-bit15) */
					frameinfo[i].Data[0]	= 0x20 + ((i+1)&0x0f);
					memcpy(&(frameinfo[i].Data[1]), &p_data[i * 7], 7);
			/*	}*/


					/* CAN帧与帧之间延时 */
					//for(int i = 0; i < 20000; i++)	;

					
				////打印
			/*	if(!p_dlg->handle_flag)
				{
					CString str;
					str.Format("打印 --> send: %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo[i].Data[0], 
						frameinfo[i].Data[1], frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
						frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
					p_dlg->ShowInfo(str, 1);
				}*/
			}

			//打印
			//if(p_dlg->handle_flag)
			//{
			//	CString str;
			//	CString strTime = p_dlg->comm.GetTime();
			//	str.Format("打印 -- [%s] --> send:", strTime);
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

			///* 发送send_frame_cnt帧数据，返回send_frame表示发送成功的帧数 */
   //         TRACE(" * ");

			//send_frame	= TransmitDataSub(frameinfo,send_frame_cnt);
   //            
			//if( send_frame )
			//{
			//	/* 去掉发送出去的数据，重新初始化指针与数据长度 */
			//	send_data	 	 = send_frame * 7;	/* 计算已经发送出去的数据长度 */
			//	p_data			+= send_data;		/* 重新定位数据开始位置 */
			//	fram_cnt		-= send_frame;		/* 重新计算剩余帧数 */
			////	serial_number	+= send_frame;		/* 重新计算流水号 */			//?

			//	/* 计算剩余数据长度 */
			//	if( data_len >= send_data )
			//		data_len -= send_data;
			//	else
			//		data_len = 0;

			//	/* 清除错误重试标志 */
			//	failed_err = 0;
			//}
			//else	/* 数据发送失败 */
			//{
			//	failed_err++;
			//	large_data_sending = CAN_NO;
			//	if( failed_err > 10 )
			//	{
			//		log_str.Format("can 数据发送失败，重试10次，退出");
			//		//WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//获取信号量
			//		//p_dlg->log.FlushLog(log_str);
			//		//ReleaseMutex(p_dlg->h_log_mutex);					//释放信号量

			//		p_can->flag = CAN_LARGE_NO;
			//		return STATUS_ERR;
			//	}
   //                 TRACE("\r\n");


			//}
			//}
			p_can->flag = CAN_LARGE_NO;
			large_data_sending = CAN_NO;
			log_str.Format("can 数据发送成功，退出");
			//WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//获取信号量
			//p_dlg->log.FlushLog(log_str);
			//ReleaseMutex(p_dlg->h_log_mutex);					//释放信号量
			return STATUS_OK;
		}
		else
		{
			log_str.Format("can 数据未接收到流控帧，超时退出");
			//WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//获取信号量
			//p_dlg->log.FlushLog(log_str);
			//ReleaseMutex(p_dlg->h_log_mutex);					//释放信号量
			p_can->flag = CAN_LARGE_NO;
			large_data_sending = CAN_NO;
			return STATUS_ERR;
		}
	}
	large_data_sending = CAN_NO;
	return STATUS_ERR;
}
/* 增加允许大数据发送ID */
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

/* 清除允许大数据发送ID */
int ECanDriver::ClearLargeSendCanId(void)
{
	large_data_id_num = 0;
	return STATUS_OK;
}

/* 通过tx id或者rx id获取当前操作的大数据结构 */
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
		/* 断开连接后，直接退出线程 */
		if( p_can->m_can_link_status != CAN_CONNECT_OK )
			break;

		/* 测试模式时，直接退出线程 */
		if( p_dlg->m_check_var.GetCheck() )
			break;

		WaitForSingleObject(p_can->h_mutex,INFINITE);	//获取信号量
        
		frame_cnt = VCI_Receive(p_can->m_devtype,p_can->dev_can_idx_select,p_can->dev_can_channel_select,frameinfo,250,200);
        //TRACE("[%d]Get CAN Data.\r\n",GetTickCount());
		ReleaseMutex(p_can->h_mutex);					//释放信号量	
				
		//if( frame_cnt )
		//{
		//	avmLogStr.Format("[AVM Debug Log | %s:%d] 共接收到 %d 个数据帧：", __FUNCTION__, __LINE__, frame_cnt);
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
			{	/* 允许保存当前帧 */

				//p_dlg->ShowInfo(avmLogStr, 1);

				save_flag = CAN_YES;
				if( frameinfo[i].ID != p_dlg->pc_id )	//过滤CAN_ID
				{
					save_flag = CAN_NO;
					continue;
				}

				if( p_dlg->handle_flag )	//升级or传输流程
				{

					//avmLogStr.Format("[AVM Debug Log | %s:%d] 进入升级or传输流程分支...", __FUNCTION__, __LINE__);
					//p_dlg->ShowInfo(avmLogStr, 1);

					p_can_data = p_can->GetCanLargePdata(0,frameinfo[i].ID);
					if( p_can_data )
					{	/* 此ID可以接收大数据 */

						if( p_can_data->flag == CAN_LARGE_SEND )
						{	/* 此ID正在发送大数据 */
							if( frameinfo[i].Data[0] == 0x30 && p_can_data->ack == CAN_NO )
							//if( (frameinfo[i].Data[0]>>4 & 0xff) == 0x03 && p_can_data->ack == CAN_NO )	//接收到流控制帧
							{	/* 接收到大数据允许发送应答,此数据不保存到ringbuf中 */
								p_can_data->ack = CAN_YES;
								save_flag		= CAN_NO;
								//log_str.Format("update find 0x30 frame -  %d\r\n",GetTickCount());
								//WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//获取信号量
								//p_dlg->log.FlushLog(log_str);
								//ReleaseMutex(p_dlg->h_log_mutex);					//释放信号量
								bStream = false;
								
								//打印
#if 0
								if(frameinfo[i].DataLen)
								{
									CString str;
									CString strTime = p_dlg->comm.GetTime();
									str.Format("打印 -- [%s] --> recv: %02x %02x %02x %02x %02x %02x %02x %02x", strTime, frameinfo[i].Data[0], 
										frameinfo[i].Data[1],frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
										frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
									p_dlg->ShowInfo(str, 1);
								}
#endif
							}
						}
						else if( p_can_data->flag == CAN_LARGE_RECEIVE )
						{	/* 此ID正在接收大数据 */

							if( (frameinfo[i].Data[0] & 0xf0) == 0x10 )
							{	/* 大数据发送第一帧,需要发送相对应的应答帧 */
								memset(p_can_data->data,0,4096);

								p_can_data->start = 0;
								p_can_data->flag  = CAN_LARGE_RECEIVE;
								p_can_data->len   = ((frameinfo[i].Data[0] & 0x0f) << 8) + frameinfo[i].Data[1] + 2;
								frameinfo[i].Data[0] = frameinfo[i].Data[0] & 0x0f;
								memcpy(p_can_data->data + p_can_data->start, &frameinfo[i].Data[0], 8);
								p_can_data->start  = 8;
								p_can_data->serial = 1;


								/* 发送流控制帧 */
								memset(can_data.buf, 0x00, 8);
								can_data.buf[0] = 0x30;
								can_data.buf[1] = 0x00;
								can_data.buf[2] = 0x00;
								p_can->TransmitData(p_can_data->tx_id,can_data.buf,8);

								/* 数据不保存 */
								save_flag = CAN_NO;
							}
							else if( (frameinfo[i].Data[0] & 0xf0) == 0x20 )
							{	/* 保存中间数据 */
								if( p_can_data->serial == (frameinfo[i].Data[0] & 0x0f) )
								{	/* 流水号正确 */
									if( p_can_data->len > p_can_data->start )
									{
										if( (p_can_data->len - p_can_data->start) > 7 )
										{	/* 中间帧 */
											memcpy(p_can_data->data + p_can_data->start, &frameinfo[i].Data[1],7);
											p_can_data->start  += 7;
											p_can_data->serial ++;
											if( 16 == p_can_data->serial )
												p_can_data->serial = 0;
										}
										else
										{	/* 最后一帧 */
											memcpy(p_can_data->data + p_can_data->start, &frameinfo[i].Data[1],p_can_data->len - p_can_data->start);
											p_can_data->flag = CAN_LARGE_NO;

											/* 将数据保存到ringbuf中 */
											can_data.id  = frameinfo[i].ID;
											can_data.len = p_can_data->len;
											memcpy(can_data.buf,p_can_data->data,p_can_data->len );

											/* 将CAN数据存放到ringbuf中 */
											p_can->can_receive_buf.put_ringbuf((unsigned char*)&can_data);
										}
									}
								}
								else
								{	/* 流水号错误，不保存数据 */
									p_can_data->flag = CAN_LARGE_NO;
								}

								/* 数据不保存 */
								save_flag = CAN_NO;
							}
						}
						else
						{	/* 判断此ID是否是大数据开始帧 */
							if( (frameinfo[i].Data[0] & 0xf0) == 0x10 )
							{	/* 大数据发送第一帧,需要发送相对应的应答帧 */
								memset(p_can_data->data,0,4096);

								p_can_data->start = 0;
								p_can_data->flag  = CAN_LARGE_RECEIVE;
								p_can_data->len   = ((frameinfo[i].Data[0] & 0x0f) << 8) + frameinfo[i].Data[1] + 2;
								frameinfo[i].Data[0] = frameinfo[i].Data[0] & 0x0f;
								memcpy(p_can_data->data + p_can_data->start, &frameinfo[i].Data[0], 8);
								p_can_data->start  = 8;
								p_can_data->serial = 1;

								/* 发送应答数据 */
								memset(can_data.buf, 0x00, 8);
								can_data.buf[0] = 0x30;
								can_data.buf[1] = 0x00;
								can_data.buf[2] = 0x00;
								p_can->TransmitData(p_can_data->tx_id,can_data.buf,8);

								/* 数据不保存 */
								save_flag = CAN_NO;
							}
						}
					}

				}
				else	//正常流程
				{

					//avmLogStr.Format("[AVM Debug Log | %s:%d] 进入正常流程分支...", __FUNCTION__, __LINE__);
					//p_dlg->ShowInfo(avmLogStr, 1);
					/* 判断是否是分包数据 */
					p_can_data = p_can->GetCanLargePdata(0,frameinfo[i].ID);
					if( p_can_data )
					{	/* 此ID可以接收大数据 */

						//avmLogStr.Format("[AVM Debug Log | %s:%d] 进入接收大数据分支...", __FUNCTION__, __LINE__);
						//p_dlg->ShowInfo(avmLogStr, 1);
						//if( (frameinfo[i].data[0] == 0x02) && (frameinfo[i].data[1] == 0x7e) )//
						//{	//链接包
						//	/* 数据保存 */
						//	save_flag = can_yes;
						//}
						//else 
						if( p_can_data->flag == CAN_LARGE_SEND )
						{	/* 此ID正在发送大数据 */
						
							if( frameinfo[i].Data[0] == 0x30 && p_can_data->ack == CAN_NO )
							//if( (frameinfo[i].Data[0]>>4 & 0xff) == 0x03 && p_can_data->ack == CAN_NO )	//接收到流控制帧
							{	/* 接收到大数据允许发送应答,此数据不保存到ringbuf中 */
								p_can_data->ack = CAN_YES;
								save_flag		= CAN_NO;

								log_str.Format("find 0x30 frame -  %d\r\n",GetTickCount());
								//WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//获取信号量
								//p_dlg->log.FlushLog(log_str);
								//ReleaseMutex(p_dlg->h_log_mutex);					//释放信号量
#if 0
								//打印
								if(frameinfo[i].DataLen)
								{
									CString str;
									CString strTime = p_dlg->comm.GetTime();
									str.Format("打印 -- [%s] --> recv: %02x %02x %02x %02x %02x %02x %02x %02x", strTime, frameinfo[i].Data[0], 
										frameinfo[i].Data[1],frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
										frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
									p_dlg->ShowInfo(str, 1);
								}
#endif
								bStream = false;
							}

						}
						else if( p_can_data->flag == CAN_LARGE_RECEIVE )
						{	/* 此ID正在接收大数据 */
							if( (frameinfo[i].Data[0] & 0xf0) == 0x10 )
							{	/* 大数据发送第一帧,需要发送相对应的应答帧 */
								memset(p_can_data->data,0,4096);

								p_can_data->start = 0;
								p_can_data->flag  = CAN_LARGE_RECEIVE;
								p_can_data->len   = ((frameinfo[i].Data[0] & 0x0f) << 8) + frameinfo[i].Data[1] + 1;
								memcpy(p_can_data->data + p_can_data->start, &frameinfo[i].Data[1],7);
								p_can_data->start  = 7;
								p_can_data->serial = 1;

								//avmLogStr.Format("[AVM Debug Log | %s:%d] 接收到FF，数据长度：%d...", __FUNCTION__, __LINE__, p_can_data->len);
								//p_dlg->ShowInfo(avmLogStr, 1);

								//打印
						/*		if(frameinfo[i].DataLen)
								{
									CString str;
									str.Format("打印 --> recv: %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo[i].Data[0], 
										frameinfo[i].Data[1],frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
										frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
									p_dlg->ShowInfo(str, 1);
								}*/


								/* 发送流控制帧 */
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

								//avmLogStr.Format("[AVM Debug Log | %s:%d] 确认FF，发送FC...", __FUNCTION__, __LINE__);
								//p_dlg->ShowInfo(avmLogStr, 1);
								#endif


								/* 数据不保存 */
								save_flag = CAN_NO;
							}
							else if( (frameinfo[i].Data[0] & 0xf0) == 0x20 )
							{	/* 保存中间数据 */

								//avmLogStr.Format("[AVM Debug Log | %s:%d] 接收到CF,编号：%d...", __FUNCTION__, __LINE__, frameinfo[i].Data[0] & 0x0f);
								//p_dlg->ShowInfo(avmLogStr, 1);

								/*avmLogStr.Format("[CF Data Value] %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo[i].Data[0], 
										frameinfo[i].Data[1],frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
										frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
								p_dlg->ShowInfo(avmLogStr, 1);*/

								if( p_can_data->serial == (frameinfo[i].Data[0] & 0x0f) )
								{	/* 流水号正确 */
									if( p_can_data->len > p_can_data->start )
									{
										if( (p_can_data->len - p_can_data->start) > 7 )
										{	/* 中间帧 */
											memcpy(p_can_data->data + p_can_data->start, &frameinfo[i].Data[1],7);
											p_can_data->start  += 7;
											p_can_data->serial ++;

											if( 16 == p_can_data->serial )
												p_can_data->serial = 0;

											//打印
									/*		if(frameinfo[i].DataLen)
											{
												CString str;
												str.Format("打印 --> recv: %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo[i].Data[0], 
													frameinfo[i].Data[1],frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
													frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
												p_dlg->ShowInfo(str, 1);
											}*/
										}
										else
										{	/* 最后一帧 */

											//avmLogStr.Format("[AVM Debug Log | %s:%d] 接收到最后一帧，保存数据到buf...", __FUNCTION__, __LINE__);
											//p_dlg->ShowInfo(avmLogStr, 1);

											memcpy(p_can_data->data + p_can_data->start, &frameinfo[i].Data[1],p_can_data->len - p_can_data->start);
											p_can_data->flag = CAN_LARGE_NO;

											/* 将数据保存到ringbuf中 */
											can_data.id  = frameinfo[i].ID;
											can_data.len = p_can_data->len;
											memcpy(can_data.buf,p_can_data->data,p_can_data->len );

											//打印
							/*				if(frameinfo[i].Data[0])
											{
												CString str;
												str.Format("打印 --> recv: %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo[i].Data[0], 
													frameinfo[i].Data[1],frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
													frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
												p_dlg->ShowInfo(str, 1);
											}*/

											/* 将CAN数据存放到ringbuf中 */
											p_can->can_receive_buf.put_ringbuf((unsigned char*)&can_data);
										}
									}
								}
								else
								{	/* 流水号错误，不保存数据 */
									p_can_data->flag = CAN_LARGE_NO;
								}

								/* 数据不保存 */
								save_flag = CAN_NO;
							}
							else
							{
								/*avmLogStr.Format("[AVM Debug Log | %s:%d] 未知数据？？？", __FUNCTION__, __LINE__, p_can_data->len);
								p_dlg->ShowInfo(avmLogStr, 1);*/
							}
						}
						else
						{	/* 判断此ID是否是大数据开始帧 */
							if( (frameinfo[i].Data[0] & 0xf0) == 0x10 )
							{	/* 大数据发送第一帧,需要发送相对应的应答帧 */
								memset(p_can_data->data,0,4096);

								p_can_data->start = 0;
								p_can_data->flag  = CAN_LARGE_RECEIVE;
								p_can_data->len   = ((frameinfo[i].Data[0] & 0x0f) << 8) + frameinfo[i].Data[1] + 1;
								memcpy(p_can_data->data + p_can_data->start, &frameinfo[i].Data[1],7);
								p_can_data->start  = 7;
								p_can_data->serial = 1;


								//avmLogStr.Format("[AVM Debug Log | %s:%d] 接收到FF，数据长度：%d...", __FUNCTION__, __LINE__, p_can_data->len);
								//p_dlg->ShowInfo(avmLogStr, 1);

								#if 0
								//打印
								if(frameinfo[i].DataLen)
								{
									CString str;
									str.Format("打印 --> recv: %02x %02x %02x %02x %02x %02x %02x %02x", frameinfo[i].Data[0], 
										frameinfo[i].Data[1],frameinfo[i].Data[2], frameinfo[i].Data[3], frameinfo[i].Data[4],
										frameinfo[i].Data[5], frameinfo[i].Data[6], frameinfo[i].Data[7]);
									p_dlg->ShowInfo(str, 1);
								}
								#endif

								/* 发送流控制帧 */
								#if 0
								memset(can_data.buf,0,8);

								p_can->TransmitData(p_can_data->tx_id,can_data.buf,8);
								#else
								memset(can_data.buf, 0x00, 8);
								can_data.buf[0] = 0x30;
								can_data.buf[1] = 0x00;
								can_data.buf[2] = 0x0A;
								p_can->TransmitData(p_can_data->tx_id,can_data.buf,8);

								//avmLogStr.Format("[AVM Debug Log | %s:%d] 确认FF，发送FC...", __FUNCTION__, __LINE__);
								//p_dlg->ShowInfo(avmLogStr, 1);

								#endif


								/* 数据不保存 */
								save_flag = CAN_NO;
							}
							else
							{
								/*avmLogStr.Format("[AVM Debug Log | %s:%d] 未知数据？？？", __FUNCTION__, __LINE__, p_can_data->len);
								p_dlg->ShowInfo(avmLogStr, 1);*/
							}
						}
					}

				}


				if( save_flag == CAN_YES )
				{	/* 直接按保存到ringbuf中 */

					memset(can_data.buf,0,4096);
					can_data.id  = frameinfo[i].ID;
					can_data.len = frameinfo[i].DataLen;
					memcpy(can_data.buf,frameinfo[i].Data,frameinfo[i].DataLen);

					/* 将CAN数据存放到ringbuf中 */
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

	/* 已经连接CAN总线，直接退出 */
	if( m_can_link_status == CAN_CONNECT_OK )
		return CAN_CONNECT_OK;

	m_can_link_status = CAN_UNCONNECT;

	m_devtype = m_mapDevtype[dev_type_str];

	/* 获取验收码 */
	code = _ttoi(unmask_code);
	/* 获取屏蔽码 */
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

	/* 打开设备 */
	res = VCI_OpenDevice(m_devtype,dev_can_idx_select,0);
	if( res !=STATUS_OK)
		return CAN_CONNECT_OPEN_DRIVER_ERR;

	/* 初始化设备 */
	res = VCI_InitCAN(m_devtype,dev_can_idx_select,dev_can_channel_select,&init_config);
	if( res != STATUS_OK )
		return CAN_CONNECT_INIT_DRIVER_ERR;

	/* 开始使用can设备 */
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
