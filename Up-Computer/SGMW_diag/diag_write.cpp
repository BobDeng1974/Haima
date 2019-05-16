#include "stdafx.h"
#include "SGMW_diag.h"
#include "SGMW_diagDlg.h"
#include "afxdialogex.h"
#include "diag.h"

#include "winTest.h"

#define TIMEOUT_WAIT_TIME	5000//1000


/* 向TBOX写入数据流程，需要安全认证 */
int CSGMW_diagDlg::WriteInfoToTbox(CAN_OBJ can_sour)
{
	int				res       =  0;
	unsigned char*	p_temp    = NULL;
	unsigned long   diag_seed =  0;
	unsigned long	diag_pas  =  0;
	CAN_OBJ			can_data  = {0};
	CString			str;

	int flag = 0;
	BYTE mod_key = 0x03;
	BYTE req_key = 0x03;
	static unsigned int SN;
	//if((diag_write.buf[2] == 0xF1) && (diag_write.buf[3] == 0x84))
	//{
	//	flag = 1;
	//	req_key = 0x05;
	//	mod_key = 0x02;
	//}
	//else
	//{
	//	flag = 0;
	//	req_key = 0x01;
	//	mod_key = 0x03;
	//}
		
	switch( diag_write_step )
	{
	case WRITE_TO_TBOX_NULL:
		/* 无操作 */
		break;
	case WRITE_TO_TBOX_EXTERN_MODE:
		/* 发送进入扩展模式指令 */
		can_data.id     = avm_id;
		can_data.len    = 8;
		can_data.buf[0] = 0x02;
		can_data.buf[1] = 0x10;
		//can_data.buf[2] = 0x03;		//扩展模式0x03	编程模式0x02
		can_data.buf[2] = mod_key;
		can_data.buf[3] = 0x00;
		can_data.buf[4] = 0x00;
		can_data.buf[5] = 0x00;
		can_data.buf[6] = 0x00;
		can_data.buf[7] = 0x00;

		can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
		//DebugPrintf(C2_DBG,"发送进入扩展模式指令",0,0,DBG_VAR_TYPE);
		if(!flag)
			DebugPrintf(C2_DBG,"发送进入扩展模式指令",0,0,DBG_VAR_TYPE);
		else
			DebugPrintf(C2_DBG,"发送进入编程模式指令",0,0,DBG_VAR_TYPE);

		res				= 1;
		diag_wait_time	= GetTickCount();
		diag_write_step = WRITE_TO_TBOX_WAIT_EXTERN_MODE;
		break;

	case WRITE_TO_TBOX_WAIT_EXTERN_MODE:
		/* 等待进入扩展模式应答，获取到应答后，发送读TBOX序列号指令 */
		if( (GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			//WriteToTboxErr("进入扩展模式指令未响应，写入数据失败!");
			if(!flag)
				WriteToTboxErr("进入扩展模式指令未响应，写入数据失败!");
			else
				WriteToTboxErr("进入编程模式指令未响应，写入数据失败!");
		}
		else
		{	/* 接收到数据 */
			//if( can_sour.buf[1] == 0x50 && can_sour.buf[2] == 0x03 )
			if( can_sour.buf[1] == 0x50 && can_sour.buf[2] == mod_key )
			{	/* 发送读序列号指令 */
				can_data.id = avm_id;
				can_data.len = 8;
				memset(can_data.buf, 0x00, 8);
				can_data.buf[0] = 0x03;
				can_data.buf[1] = 0x22;
				can_data.buf[2] = 0xF1;
				can_data.buf[3] = 0xC8;
				can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);

				res				= 1;
				diag_wait_time	= GetTickCount();
				diag_write_step = WRITE_TO_TBOX_WAIT_READ_SN;
				DebugPrintf(C2_DBG,"请求种子信息",0,0,DBG_VAR_TYPE);
			}
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == mod_key)
			{
				res = 1;
				WriteToTboxErr("进入扩展模式指令负相应，写入数据失败!");
			}
		}
		break;


	case WRITE_TO_TBOX_WAIT_READ_SN:
		if ((GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME)
		{	/* 等待超时 */
			WriteToTboxErr("读SN号超时，写入数据失败!");
		}
		else {
			/* 接收到数据 */
			if (can_sour.buf[1] == 0x62 && can_sour.buf[2] == 0xF1) {
				DebugPrintf(NORM_DBG, "读SN号成功", 0, 0, DBG_VAR_TYPE);
				p_temp = &can_sour.buf[4];
				SN = GET_DWORD(p_temp);
				/* 接收到应答,发送读序列号操作 */
				/* 请求种子数据 */
				can_data.id = avm_id;
				can_data.len = 8;
				can_data.buf[0] = 0x02;
				can_data.buf[1] = 0x27;
				//can_data.buf[2] = 0x01;
				can_data.buf[2] = req_key;
				can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);

				res = 1;
				diag_wait_time = GetTickCount();
				diag_write_step = WRITE_TO_TBOX_WAIT_READ_SEED;
				DebugPrintf(C2_DBG, "请求种子信息", 0, 0, DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x22) {
				str.Format("读SN号失败负相应");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
			}

		}
		break;




	case WRITE_TO_TBOX_WAIT_READ_SEED:
		/* 等待获取到种子信息,并且发送密钥 */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			WriteToTboxErr("未能获取到AVM加密种子，写入数据失败!");
		}
		else
		{	/* 发现TBOX上报的数据 */
			//if( can_sour.buf[1] == 0x67 && can_sour.buf[2] == 0x01 )
			if( can_sour.buf[1] == 0x67 && can_sour.buf[2] == req_key )
			{	/* 接收到应答,发送密钥 */
				/* 保存种子数据 */
				p_temp = &can_sour.buf[3];
				diag_seed = GET_DWORD(p_temp);
				DebugPrintf(C2_DBG,"found avm seed: 0x",(unsigned char*)&diag_seed,4,DBG_VAR_TYPE);

				if( diag_seed == 0  )
				{   /* 秘钥已经解锁，直接写入数据 */
					DebugPrintf(C2_DBG,"秘钥已解锁，直接写入数据",0,0,DBG_VAR_TYPE);
					can_driver.TransmitData(diag_write.id,diag_write.buf,diag_write.len);

					res				= 1;
					diag_wait_time	= GetTickCount();
					diag_write_step = WRITE_TO_TBOX_WAIT_WRITE_DATA;
					break;
				}
				/* 生成password */
				//diag_pas = safe.uds_calc_key(diag_seed,1);
				//if((diag_write.buf[2] == 0xF1) && (diag_write.buf[3] == 0x84))
				if(flag)
					diag_pas = safe.uds_calc_key(diag_seed,3,SN);
				else
					diag_pas = safe.uds_calc_key(diag_seed,2,SN);
					
				/* 发送密钥 */
				can_data.id     = avm_id;
				can_data.len    = 8;
				can_data.buf[0] = 0x06;
				can_data.buf[1] = 0x27;
				//can_data.buf[2] = 0x02;
				can_data.buf[2] = req_key + 1;
				can_data.buf[3] = GET_BYTE_24(diag_pas);
				can_data.buf[4] = GET_BYTE_16(diag_pas);
				can_data.buf[5] = GET_BYTE_8(diag_pas);
				can_data.buf[6] = GET_BYTE_0(diag_pas);
				can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);

				res				= 1;
				diag_wait_time	= GetTickCount();
				diag_write_step = WRITE_TO_TBOX_WAIT_READ_PASSWORD;
				DebugPrintf(C2_DBG,"发送秘钥",0,0,DBG_VAR_TYPE);
			}
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == 0x27 )
			{
				res = 1;
				WriteToTboxErr("无法获取AVM种子信息，写入数据失败!");
				str.Format("get tbox seed failed: %d",can_data.buf[3]);
				DebugPrintf(C2_DBG,str,0,0,DBG_VAR_TYPE);
			}
		}
		break;
	case WRITE_TO_TBOX_WAIT_READ_PASSWORD:
		/* 等待密钥发送应答，开始向TBOX写入数据 */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			WriteToTboxErr("未能获取到AVM密钥信息，写入数据失败!");
		}
		else
		{	/* 接收到数据 */
			//if( can_sour.buf[1] == 0x67 && can_sour.buf[2] == 0x02 )
			if( can_sour.buf[1] == 0x67 && can_sour.buf[2] == (req_key+1) )
			{	/* 接收到应答,发送写入信息给TBOX */
				can_driver.TransmitData(diag_write.id,diag_write.buf,diag_write.len);

				res				= 1;
				diag_wait_time	= GetTickCount();
				diag_write_step = WRITE_TO_TBOX_WAIT_WRITE_DATA;
				DebugPrintf(C2_DBG,"秘钥验证成功，开始写入数据",0,0,DBG_VAR_TYPE);
			}
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == 0x27 )
			{
				res = 1;
				WriteToTboxErr("密钥错误，写入数据失败!");

				str.Format("tbox password check failed: %d",can_sour.buf[3]);
				DebugPrintf(C1_DBG,str,0,0,DBG_VAR_TYPE);
			}
		}
		break;
	case WRITE_TO_TBOX_WAIT_WRITE_DATA:
		/* 等待向TBOX写入数据的应答 */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			WriteToTboxErr("写入AVM数据超时!");
		}
		else
		{	/* 接收到数据 */
			if( can_sour.buf[1] == 0x6e && can_sour.buf[2] ==  diag_write.buf[2] && can_sour.buf[3] == diag_write.buf[3])
			{
				res = 1;
				WriteToTboxErr("数据写入成功");
			} 
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == 0x2e )
			{
				res = 1;
				WriteToTboxErr("写入AVM数据验证错误");
				str.Format("write to avm data failed: %d",can_sour.buf[3]);
				DebugPrintf(C1_DBG,str,0,0,DBG_VAR_TYPE);
			}
		}
		break;
	default:
		WriteToTboxErr("发现错误的写入步骤");
		break;
	}
	return res;
}

	

/* 进入升级流程，向TBOX写入数据，需要安全认证 */
int CSGMW_diagDlg::UpdateToTbox(CAN_OBJ can_sour)
{
	int				res       =  0;
	unsigned char*	p_temp    = NULL;
	static unsigned int sN = 0;
	unsigned long   diag_seed =  0;
	unsigned long	diag_pas  =  0;
	CAN_OBJ			can_data  = {0};
	CString			str;

	int m_isDiff, m_codetype;
	m_isDiff = m_comb_diffud.GetCurSel();
	m_codetype = m_comb_fwtype.GetCurSel();

	//打印
	if(can_sour.len)
	{
		CString strTime = p_dlg->comm.GetTime();
		str.Format("打印 -- [%s] --> recv:", strTime);
		for(int i=0; i<can_sour.len; ++i)
		{
			CString strCh;
			strCh.Format(" %02x", can_sour.buf[i]);
			str += strCh;
		}
		ShowInfo(str, 1);
	}


	switch( diag_update_step )
	{
	case UPDATE_TO_TBOX_NULL:
		/* 无操作 停止升级*/
		OnBnClickedButtonStop();
		break;
	case UPDATA_TO_AVM_ENTER_EXTMODE:   //总是ui线程调用
		can_data.id = avm_id; 
		can_data.len = 8;
		memset(can_data.buf, 0x00, 8);
		can_data.buf[0] = 0x02;
		can_data.buf[1] = 0x10;
		can_data.buf[2] = 0x03;
		can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);
		//diag_update_step = UPDATA_TO_AVM_FUN_NEGATIVE_RESPONSE;
		DebugPrintf(NORM_DBG, "发送进入扩展模式指令", 0, 0, DBG_VAR_TYPE);
		diag_update_step = UPDATA_TO_AVM_STOP_SET_DTC;
		diag_wait_time = GetTickCount();
		
	break;
	case UPDATA_TO_AVM_STOP_SET_DTC:
		if ((GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME)
		{	/* 等待超时 */
			WriteToTboxErr("进入扩展模式指令未响应，写入数据失败!");
			OnBnClickedButtonStop();
		}
		else {
			/* 接收到数据 */
			if (can_sour.buf[1] == 0x50 && can_sour.buf[2] == 0x03) {
				DebugPrintf(NORM_DBG, "进入扩展模式成功", 0, 0, DBG_VAR_TYPE);

				/* 停止DTC */
				can_data.id = avm_id;
				can_data.len = 8;
				memset(can_data.buf, 0x00, 8);
				can_data.buf[0] = 0x02;
				can_data.buf[1] = 0x85;
				can_data.buf[2] = 0x02;
				can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);

				diag_wait_time = GetTickCount();
				diag_update_step = UPDATE_TO_AVM_COMMUNICATE_CONTROL;
				DebugPrintf(NORM_DBG, "停止设定DTC", 0, 0, DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x10) {
				str.Format("进入扩展模式失败，");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();
			}
		
		}
		break;
		
	case UPDATE_TO_AVM_COMMUNICATE_CONTROL:
		if ((GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME)
		{	/* 等待超时 */
			WriteToTboxErr("停止DTC指令未响应，写入数据失败!");
			OnBnClickedButtonStop();
		}
		else {
			/* 接收到数据 */
			if (can_sour.buf[1] == 0xc5 && can_sour.buf[2] == 0x02) {
				DebugPrintf(NORM_DBG, "停止DTC设置成功", 0, 0, DBG_VAR_TYPE);

				/* 发送通信控制指令 */
				can_data.id = avm_id;
				can_data.len = 8;
				memset(can_data.buf, 0x00, 8);
				can_data.buf[0] = 0x03;
				can_data.buf[1] = 0x28;
				can_data.buf[2] = 0x01;
				can_data.buf[3] = 0x01;
				can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);

				diag_wait_time = GetTickCount();
				diag_update_step = UPDATE_TO_AVM_READ_SN;
				DebugPrintf(NORM_DBG, "发送禁止通信控制", 0, 0, DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x85) {
				str.Format("停止DTC设置失败，");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();
			}

		}
		break;

	case UPDATE_TO_AVM_READ_SN:
		if ((GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME)
		{	/* 等待超时 */
			WriteToTboxErr("发送禁止通信控制超时，写入数据失败!");
			OnBnClickedButtonStop();
		}
		else {
			/* 接收到数据 */
			if (can_sour.buf[1] == 0x68 && can_sour.buf[2] == 0x01) {
				DebugPrintf(NORM_DBG, "发送禁止通信控制成功", 0, 0, DBG_VAR_TYPE);

				/* 发送读序列号指令 */
				can_data.id = avm_id;
				can_data.len = 8;
				memset(can_data.buf, 0x00, 8);
				can_data.buf[0] = 0x03;
				can_data.buf[1] = 0x22;
				can_data.buf[2] = 0xF1;
				can_data.buf[3] = 0x8C;
				can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);

				diag_wait_time = GetTickCount();
				diag_update_step = UPDATE_TO_AVM_PROGRAM_MODE;
				DebugPrintf(NORM_DBG, "发送读序列号指令", 0, 0, DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x28) {
				str.Format("发送禁止通信控制失败，");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();
			}

		}
		break;
	case UPDATE_TO_AVM_PROGRAM_MODE:
		if ((GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME)
		{	/* 等待超时 */
			WriteToTboxErr("发送读序列号指令超时，写入数据失败!");
			OnBnClickedButtonStop();
		}
		else {
			/* 接收到数据 */
			if (can_sour.buf[2] == 0x62 && can_sour.buf[3] == 0xF1 && can_sour.buf[4] == 0x8C) {
				DebugPrintf(NORM_DBG, "读序列号成功", 0, 0, DBG_VAR_TYPE);
				p_temp = &can_sour.buf[11];
			    sN = GET_DWORD(p_temp);
				/* 发送进入编程模式指令 */
				can_data.id = avm_id;
				can_data.len = 8;
				memset(can_data.buf, 0x00, 8);
				can_data.buf[0] = 0x02;
				can_data.buf[1] = 0x10;
				can_data.buf[2] = 0x02;
				can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);

				diag_wait_time = GetTickCount();
				diag_update_step = UPDATE_TO_AVM_WAIT_PROGRAM_MODE;
				DebugPrintf(NORM_DBG, "发送进入编程模式指令", 0, 0, DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x22) {
				str.Format("发送读序列号指令失败，");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();
			}

		}

		break;

	case UPDATE_TO_AVM_WAIT_PROGRAM_MODE:
		/* 等待进入编程模式应答，获取到应答后，发送读TBOX序列号指令 */
		if( (GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			WriteToTboxErr("发送进入编程模式指令超时!");
			OnBnClickedButtonStop();
		}
		else
		{	/* 接收到数据 */
			if( can_sour.buf[1] == 0x50 && can_sour.buf[2] == 0x02)
			{	/* 接收到应答, */
				DebugPrintf(NORM_DBG,"发送进入编程模式指令成功",0,0,DBG_VAR_TYPE);
				Sleep(1500);//等待下位机跳转
				/* 请求种子数据 */
				can_data.id  = avm_id;
				can_data.len = 8;
				memset(can_data.buf, 0x00, 8);
				can_data.buf[0] = 0x02;
				can_data.buf[1] = 0x27;
				can_data.buf[2] = 0x03;
				can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);

				diag_wait_time	= GetTickCount();
				diag_update_step = UPDATE_TO_AVM_WAIT_READ_SEED;
				DebugPrintf(NORM_DBG,"请求种子信息",0,0,DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x10) {
				if (can_sour.buf[3] == 0x78) {
					DebugPrintf(NORM_DBG, "NRC-78 等待...", 0, 0, DBG_VAR_TYPE);
					break;
				}

				str.Format("发送进入编程模式指令失败，");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();

			}

		}
		break;
	case UPDATE_TO_AVM_WAIT_READ_SEED:
		/* 等待获取到种子信息,并且发送密钥 */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			WriteToTboxErr("未能获取到AVM加密种子，写入数据失败!");
			OnBnClickedButtonStop();
		}
		else
		{	/* 发现TBOX上报的数据 */
			if( can_sour.buf[1] == 0x67 && can_sour.buf[2] == 0x03 )
			{	/* 接收到应答,发送密钥 */
				/* 保存种子数据 */

				p_temp = &can_sour.buf[3];
				diag_seed = GET_DWORD(p_temp);

				DebugPrintf(NORM_DBG,"found tbox seed: 0x",(unsigned char*)&diag_seed,4,DBG_VAR_TYPE);

				if( diag_seed == 0  )
				{   /* 秘钥已经解锁 */
					DebugPrintf(NORM_DBG,"秘钥已解锁，直接写入数据",0,0,DBG_VAR_TYPE);

					if((m_codetype == 0) || (m_codetype == 6))  // MCU 
					{
						/* 内存擦除 */
						can_data.id = avm_id;
						can_data.len = 8;
						memset(can_data.buf, 0x00, 8);
						can_data.buf[0] = 0x05;
						can_data.buf[1] = 0x31;
						can_data.buf[2] = 0x01;
						can_data.buf[3] = 0xFF;
						can_data.buf[4] = 0x00;
						if (m_codetype == 0) {
							can_data.buf[5] = 0xFF;
						}
						else {
							can_data.buf[5] = 0x01;
						}
						can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);
						DebugPrintf(NORM_DBG, "内存擦除..", 0, 0, DBG_VAR_TYPE);
						diag_wait_time = GetTickCount();
						diag_update_step = UPDATE_TO_AVM_EARSE_MEMORY;
					}
					else {
						/* 进入请求下载 ARM程序不用写指纹 与擦除 */
						DebugPrintf(NORM_DBG, "请求下载", 0, 0, DBG_VAR_TYPE);
						/* 请求文件下载 */
						can_data.id = avm_id;
						can_data.len = 12 + 1;// 发送长度+ 一个字节的长度（256> 2个字节）
						memset(can_data.buf, 0x00, 14);
						can_data.buf[0] = 0x0C;   //12
						can_data.buf[1] = 0x34;
						can_data.buf[2] = 0x00;
						can_data.buf[3] = 0x44;
						can_data.buf[4] = 0x00;
						can_data.buf[5] = 0x00;
						can_data.buf[6] = 0x00;
						can_data.buf[7] = 0x00;
						can_data.buf[8] = GET_BYTE_24(m_file_size);
						can_data.buf[9] = GET_BYTE_16(m_file_size);
						can_data.buf[10] = GET_BYTE_8(m_file_size);
						can_data.buf[11] = GET_BYTE_0(m_file_size);
						can_data.buf[12] = (((m_isDiff & 0x01) | ((m_codetype - 1) & 0x07) << 1));
						can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);
						str.Format("请求文件下载");
						DebugPrintf(NORM_DBG, str, 0, 0, DBG_VAR_TYPE);
						str.Format("update data type： %02x", can_data.buf[12]);
						DebugPrintf(NORM_DBG, str, 0, 0, DBG_VAR_TYPE);

						diag_wait_time = GetTickCount();
						diag_update_step = UPDATE_TO_AVM_REQUEST_DOWNLOAD;

					}
					
				}
				/* 生成password */
				diag_pas = safe.uds_calc_key(diag_seed,2,sN);
				DebugPrintf(NORM_DBG,"calculate key: 0x",(unsigned char*)&diag_pas,4,DBG_VAR_TYPE);
					
				/* 发送密钥 */
				can_data.id     = avm_id;
				can_data.len    = 8;
				memset(can_data.buf, 0x00, 8);
				can_data.buf[0] = 0x06;
				can_data.buf[1] = 0x27;
				can_data.buf[2] = 0x04;
				can_data.buf[3] = GET_BYTE_24(diag_pas);
				can_data.buf[4] = GET_BYTE_16(diag_pas);
				can_data.buf[5] = GET_BYTE_8(diag_pas);
				can_data.buf[6] = GET_BYTE_0(diag_pas);
				can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);

				diag_wait_time	= GetTickCount();
				diag_update_step = UPDATE_TO_AVM_WAIT_READ_PASSWORD;
				DebugPrintf(NORM_DBG,"发送秘钥",0,0,DBG_VAR_TYPE);
			}
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == 0x27 )
			{
				if (can_sour.buf[3] == 0x78) {
					DebugPrintf(NORM_DBG, "NRC-78 等待...", 0, 0, DBG_VAR_TYPE);
					break;
				}
				WriteToTboxErr("无法获取TBOX种子信息，写入数据失败!");
				str.Format("get tbox seed failed: %d", can_sour.buf[3]);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				OnBnClickedButtonStop();
			}
		}
		break;



	case UPDATE_TO_AVM_WAIT_READ_PASSWORD:
		/* 等待密钥发送应答，开始向TBOX写入数据 */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			WriteToTboxErr("未能获取到TBOX密钥信息，写入数据失败!");
			OnBnClickedButtonStop();
		}
		else
		{	/* 接收到数据 */	
#ifdef WIN_TEST
			if(1)
#else
			if( can_sour.buf[1] == 0x67 && can_sour.buf[2] == 0x04 )
#endif
			{	
				if ((m_codetype == 0) || (m_codetype == 6))   // MCU 
				{
					/* 接收到秘钥验证应答，发送擦除指令 */
					DebugPrintf(NORM_DBG, "秘钥验证成功，发送擦除指令", 0, 0, DBG_VAR_TYPE);

					/* 内存擦除 */
					can_data.id = avm_id;
					can_data.len = 8;
					memset(can_data.buf, 0x00, 8);
					can_data.buf[0] = 0x05;
					can_data.buf[1] = 0x31;
					can_data.buf[2] = 0x01;
					can_data.buf[3] = 0xFF;
					can_data.buf[4] = 0x00;
					if (m_codetype == 0) {
						can_data.buf[5] = 0xFF;
					}
					else {
						can_data.buf[5] = 0x01;
					}
					can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);
					DebugPrintf(NORM_DBG, "内存擦除..", 0, 0, DBG_VAR_TYPE);
					diag_wait_time = GetTickCount();
					diag_update_step = UPDATE_TO_AVM_EARSE_MEMORY;
				}
				else {
					/* 接收到秘钥验证应答，进入请求下载 ARM程序不用擦除 */
					DebugPrintf(NORM_DBG, "秘钥验证成功，请求下载", 0, 0, DBG_VAR_TYPE);
					/* 请求文件下载 */
					can_data.id = avm_id;
					can_data.len = 12 + 1;// 发送长度+ 一个字节的长度（256> 2个字节）
					memset(can_data.buf, 0x00, 14);
					can_data.buf[0] = 0x0C;   //12
					can_data.buf[1] = 0x34;
					can_data.buf[2] = 0x00;
					can_data.buf[3] = 0x44;
					can_data.buf[4] = 0x00;
					can_data.buf[5] = 0x00;
					can_data.buf[6] = 0x00;
					can_data.buf[7] = 0x00;
					can_data.buf[8] = GET_BYTE_24(m_file_size);
					can_data.buf[9] = GET_BYTE_16(m_file_size);
					can_data.buf[10] = GET_BYTE_8(m_file_size);
					can_data.buf[11] = GET_BYTE_0(m_file_size);
					can_data.buf[12] = (((m_isDiff & 0x01) | ((m_codetype - 1) & 0x07) << 1));
					can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);
					str.Format("请求文件下载");
					DebugPrintf(NORM_DBG, str, 0, 0, DBG_VAR_TYPE);
					str.Format("update data type： %02x", can_data.buf[12]);
					DebugPrintf(NORM_DBG, str, 0, 0, DBG_VAR_TYPE);

					diag_wait_time = GetTickCount();
					diag_update_step = UPDATE_TO_AVM_REQUEST_DOWNLOAD;
				
				}
				



			}
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == 0x27 )
			{

				WriteToTboxErr("密钥错误，写入数据失败!");

				str.Format("tbox password check failed: %d",can_sour.buf[3]);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				OnBnClickedButtonStop();
			}
		}
		break;


	case UPDATE_TO_AVM_EARSE_MEMORY:
		/* 等待内存擦除应答 */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
		
			WriteToTboxErr("未能获取到内存擦除响应，写入数据失败!");
			
			OnBnClickedButtonStop();
		}
		else
		{
			/* 接收到数据 */
			if(( can_sour.buf[1] == 0x71 ) && (can_sour.buf[2] ==  0x01))
			{	/* 接收到应答*/
				DebugPrintf(NORM_DBG,"内存擦除成功",0,0,DBG_VAR_TYPE);
			

				/* 请求文件下载 */	
				can_data.id     = avm_id;
				can_data.len    = 12 + 1 ;// 发送长度+ 一个字节的长度（256> 2个字节）
				memset(can_data.buf, 0x00, 14);
				can_data.buf[0] = 0x0C;   //12
				can_data.buf[1] = 0x34;
				can_data.buf[2] = 0x00;  
				can_data.buf[3] = 0x44;
				can_data.buf[4] = 0x00;
				can_data.buf[5] = 0x00;
				can_data.buf[6] = 0x00;
				can_data.buf[7] = 0x00;
				can_data.buf[8] = GET_BYTE_24(m_file_size);
				can_data.buf[9] = GET_BYTE_16(m_file_size);
				can_data.buf[10] = GET_BYTE_8(m_file_size);
				can_data.buf[11] = GET_BYTE_0(m_file_size);
				if (m_codetype == 0) {
					can_data.buf[12] = 0x10;
				}
				else if (m_codetype == 6) {
					can_data.buf[12] = 0x20;
				}
				else {
					can_data.buf[12] = (((m_isDiff & 0x01) | ((m_codetype - 1) & 0x07) << 1));
				}
				can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
				str.Format("请求文件下载");
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				str.Format("update data type： %02x", can_data.buf[12]);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				diag_wait_time	= GetTickCount();
				diag_update_step = UPDATE_TO_AVM_REQUEST_DOWNLOAD;
			}
			else if( (can_sour.buf[1] == 0x7f) && (can_sour.buf[2] == 0x31))
			{
				if (can_sour.buf[3] == 0x78) {
					DebugPrintf(NORM_DBG, "NRC-78 等待...", 0, 0, DBG_VAR_TYPE);
					break;
				}
				/* 等待内存擦除失败负响应 */
				DebugPrintf(NORM_DBG,"等待内存擦除失败",0,0,DBG_VAR_TYPE);
				OnBnClickedButtonStop();
				
			}
		}
		break;
	case UPDATE_TO_AVM_REQUEST_DOWNLOAD:
		/* 等待请求下载应答 */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			str.Format("未能获取到请求文件下载响应，写入数据失败!");
			WriteToTboxErr(str);
			OnBnClickedButtonStop();
		}
		else
		{
			/* 接收到数据 */
			if( can_sour.buf[1] == 0x74 && can_sour.buf[2] == 0x20)
			{	/* 接收到应答，请求下载成功 */
				str.Format("请求文件下载成功");
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				p_temp = &(can_sour.buf[3]);
				m_block_size = GET_WORD(p_temp);

				if(m_file_size % (m_block_size-4))
					m_block_num = m_file_size/(m_block_size-4) + 1;
				else
					m_block_num = m_file_size/(m_block_size-4);
			
				//前n-1块的CAN数据长度
				if((m_block_size + 1) % 7)
				{
					m_temp_len_before = ((m_block_size+1)/7 + 1) * 7;
				}
				else
				{
					m_temp_len_before = m_block_size + 1;
				}

				//第n块的CAN数据长度   最后一块数据长度
				m_remain_len = m_file_size - (m_block_num - 1)*(m_block_size - 4);
				if((m_remain_len + 4 + 1) % 7)
				{
					m_temp_len_last = ((m_remain_len+4+1)/7 + 1) * 7;
				}
				else
				{
					m_temp_len_last = m_remain_len + 4 + 1;
				}

				/* 发送块数据 */
				if(block_cnt < m_block_num)	//前n-1块
				{
					can_data.id			= avm_id;
					if(m_temp_len_before <= 256)
					{
						can_data.len    = m_temp_len_before;
						memset(can_data.buf, 0x00, m_temp_len_before);
						can_data.buf[0] = (unsigned char)m_block_size;
						can_data.buf[1] = 0x36;

						// 计算包序号，先低后高
						can_data.buf[2] = block_cnt%0x100;
						can_data.buf[3] = block_cnt/0x100;
						can_data.buf[4] = block_cnt/0x10000;

						pFile.Seek((block_cnt-1)*(m_block_size-4), CFile::begin);
						pFile.Read(can_data.buf+5,m_block_size-4);

						//memcpy(can_data.buf+5, m_file_data+(block_cnt-1)*(m_block_size-4), m_block_size-4);  
					}
					else
					{
						can_data.len    = m_temp_len_before + 1;
						memset(can_data.buf, 0x00, m_temp_len_before+1);
						can_data.buf[0] = (((WORD)(m_block_size) & 0x0f00)>>8) & 0xff;
						can_data.buf[1] = (WORD)(m_block_size) & 0xff;
						can_data.buf[2] = 0x36;

						// 计算包序号，先低后高
						can_data.buf[3] = block_cnt%0x100;
						can_data.buf[4] = block_cnt/0x100;
						can_data.buf[5] = block_cnt/0x10000;

						pFile.Seek((block_cnt-1)*(m_block_size-4), CFile::begin);
						pFile.Read(can_data.buf+6,m_block_size-4);

						//memcpy(can_data.buf+6, m_file_data+(block_cnt-1)*(m_block_size-4), m_block_size-4);  
					}
				}
				else if(block_cnt == m_block_num)			//第n块
				{
					can_data.id			= avm_id;
					if(m_temp_len_last <= 256)
					{
						can_data.len    = m_temp_len_last;
						memset(can_data.buf, 0x00, m_temp_len_last);
						can_data.buf[0] = m_remain_len + 4;
						can_data.buf[1] = 0x36;
						
						// 计算包序号，先低后高
						can_data.buf[2] = block_cnt%0x100;
						can_data.buf[3] = block_cnt/0x100;
						can_data.buf[4] = block_cnt/0x10000;

						pFile.Seek((block_cnt-1)*(m_block_size-4), CFile::begin);
						pFile.Read(can_data.buf+5,m_remain_len);

						//memcpy(can_data.buf+5, m_file_data+(block_cnt-1)*(m_block_size-4), m_remain_len);  
					}
					else
					{
						can_data.len    = m_temp_len_last + 1;
						memset(can_data.buf, 0x00, m_temp_len_last+1);
						can_data.buf[0] = (((WORD)(m_remain_len+4) & 0x0f00)>>8) & 0xff;
						can_data.buf[1] = (WORD)(m_remain_len+4) & 0xff;
						can_data.buf[2] = 0x36;
						
						// 计算包序号，先低后高
						can_data.buf[3] = block_cnt%0x100;
						can_data.buf[4] = block_cnt/0x100;
						can_data.buf[5] = block_cnt/0x10000;

						pFile.Seek((block_cnt-1)*(m_block_size-4), CFile::begin);
						pFile.Read(can_data.buf+6,m_remain_len);

						//memcpy(can_data.buf+6, m_file_data+(block_cnt-1)*(m_block_size-4), m_remain_len);  
					}
				}

				can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
				str.Format("发送第%d块数据", block_cnt);
				//DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				ShowInfo(str, 2);

				diag_wait_time	= GetTickCount();
				diag_update_step = UPDATE_TO_AVM_TRANSFER_DATA;
			}
			else if( can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x34)
			{
				str.Format("请求文件下载失败，");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();
			}
		}
		break;
	case UPDATE_TO_AVM_TRANSFER_DATA:
		/* 发送块数据响应 */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			str.Format("未能获取到第%d块数据发送响应，写入数据失败!", block_cnt);
			WriteToTboxErr(str);
			OnBnClickedButtonStop();
		}
		else
		{
			/* 接收到数据 */
			if( can_sour.buf[1] == 0x76 /*&& can_sour.buf[2] == block_cnt*/ )
			{	/* 接收到第i块数据发送应答，退出第i块数据传输 */     
				str.Format("发送第%d块数据成功", block_cnt);
				//DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				ShowInfo(str, 2);

				/* 文件进度条 */
				if(block_cnt < m_block_num)
					dwFilePos += m_block_size - 4;
				else
					dwFilePos += m_remain_len;
				m_cProgress.SetPos(dwFilePos);

				if(block_cnt < m_block_num)
				{
					block_cnt++;

					/* 发送块数据 */
					if(block_cnt < m_block_num)	//前n-1块
					{
						can_data.id			= avm_id;
						if(m_temp_len_before <= 256)
						{
							can_data.len    = m_temp_len_before;
							memset(can_data.buf, 0x00, m_temp_len_before);
							can_data.buf[0] = (unsigned char)m_block_size;
							can_data.buf[1] = 0x36;

							// 计算包序号，先低后高
							can_data.buf[2] = block_cnt%0x100;
							can_data.buf[3] = block_cnt/0x100;
							can_data.buf[4] = block_cnt/0x10000;

							pFile.Seek((block_cnt-1)*(m_block_size-4), CFile::begin);
							pFile.Read(can_data.buf+5,m_block_size-4);

							//memcpy(can_data.buf+5, m_file_data+(block_cnt-1)*(m_block_size-4), m_block_size-4);  
						}
						else
						{
							can_data.len    = m_temp_len_before + 1;
							memset(can_data.buf, 0x00, m_temp_len_before+1);
							can_data.buf[0] = (((WORD)(m_block_size) & 0x0f00)>>8) & 0xff;
							can_data.buf[1] = (WORD)(m_block_size) & 0xff;
							can_data.buf[2] = 0x36;

							// 计算包序号，先低后高
							can_data.buf[3] = block_cnt%0x100;
							can_data.buf[4] = block_cnt/0x100;
							can_data.buf[5] = block_cnt/0x10000;

							pFile.Seek((block_cnt-1)*(m_block_size-4), CFile::begin);
							pFile.Read(can_data.buf+6,m_block_size-4);

							//memcpy(can_data.buf+6, m_file_data+(block_cnt-1)*(m_block_size-4), m_block_size-4);  
						}
					}
					else if(block_cnt == m_block_num)			//第n块
					{
						can_data.id			= avm_id;
						if(m_temp_len_last <= 256)
						{
							can_data.len    = m_temp_len_last;
							memset(can_data.buf, 0x00, m_temp_len_last);
							can_data.buf[0] = m_remain_len + 4;
							can_data.buf[1] = 0x36;
						
							// 计算包序号，先低后高
							can_data.buf[2] = block_cnt%0x100;
							can_data.buf[3] = block_cnt/0x100;
							can_data.buf[4] = block_cnt/0x10000;

							pFile.Seek((block_cnt-1)*(m_block_size-4), CFile::begin);
							pFile.Read(can_data.buf+5,m_remain_len);

							//memcpy(can_data.buf+5, m_file_data+(block_cnt-1)*(m_block_size-4), m_remain_len);  
						}
						else
						{
							can_data.len    = m_temp_len_last + 1;
							memset(can_data.buf, 0x00, m_temp_len_last+1);
							can_data.buf[0] = (((WORD)(m_remain_len+4) & 0x0f00)>>8) & 0xff;
							can_data.buf[1] = (WORD)(m_remain_len+4) & 0xff;
							can_data.buf[2] = 0x36;
						
							// 计算包序号，先低后高
							can_data.buf[3] = block_cnt%0x100;
							can_data.buf[4] = block_cnt/0x100;
							can_data.buf[5] = block_cnt/0x10000;

							pFile.Seek((block_cnt-1)*(m_block_size-4), CFile::begin);
							pFile.Read(can_data.buf+6,m_remain_len);

							//memcpy(can_data.buf+6, m_file_data+(block_cnt-1)*(m_block_size-4), m_remain_len);  
						}
					}

					//str.Format("开始发送第%d块数据", block_cnt);
					//ShowInfo(str, 2);
					can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
					str.Format("发送第%d块数据", block_cnt);
					//DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
					ShowInfo(str, 2);

					diag_wait_time	= GetTickCount();
					diag_update_step = UPDATE_TO_AVM_TRANSFER_DATA;
				}
				else
				{
					can_data.id     = avm_id;
					can_data.len    = 8;
					memset(can_data.buf, 0x00, 8);
					can_data.buf[0] = 0x01;
					can_data.buf[1] = 0x37;
					can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
					diag_wait_time	= GetTickCount();
					diag_update_step = UPDATE_TO_AVM_TRANSFER_DATA_EXIT;
					str.Format("发送退出文件数据传输请求");
					DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				}
			}
			else if( can_sour.buf[1] == 0x7F)
			{

			if (can_sour.buf[3] == 0x78) {
				DebugPrintf(NORM_DBG, "NRC-78 等待...", 0, 0, DBG_VAR_TYPE);
				break;
			}
				/* 发送块数据失败 */
				str.Format("发送第%d块数据失败，", block_cnt);
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();
			}
		}
		break;
	case UPDATE_TO_AVM_TRANSFER_DATA_EXIT:
		/* 退出数据传输 */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			str.Format("未能获取到退出数据传输请求响应，写入数据失败!");
			WriteToTboxErr(str);
			OnBnClickedButtonStop();
		}
		else
		{
			/* 接收到数据 */

			if( can_sour.buf[1] == 0x77)
			{	/* 退出数据传输，进行文件下载校验 */
				str.Format("退出文件数据传输成功");
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				pFile.Close(); // 关闭文件

				can_data.id     = avm_id;
				can_data.len    = 10;
				memset(can_data.buf, 0x00, 10);
				can_data.buf[0] = 0x09;
				can_data.buf[1] = 0x31;
				can_data.buf[2] = 0x01;
				can_data.buf[3] = 0xFF;
				can_data.buf[4] = 0x01;
				can_data.buf[5] = 0xFF;
				can_data.buf[6] = GET_BYTE_24(m_dwCRC);
				can_data.buf[7] = GET_BYTE_16(m_dwCRC);
				can_data.buf[8] = GET_BYTE_8(m_dwCRC);
				can_data.buf[9] = GET_BYTE_0(m_dwCRC);
				can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);

				diag_wait_time	= GetTickCount();
				diag_update_step = UPDATE_TO_AVM_UPDATE_CHECK;
				DebugPrintf(NORM_DBG,"文件下载校验",0,0,DBG_VAR_TYPE);
			
			}
			else if( can_sour.buf[1] == 0x7F)
			{
				/* 退出数据传输失败 */
				str.Format("退出文件数据传输失败，");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();
			}
		}
		break;
	case UPDATE_TO_AVM_UPDATE_CHECK:
		/* 文件下载校验2 */
		/*if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )*/
		if (0)
		{	/* 等待超时 */
			str.Format("未能获取到文件下载校验响应，写入数据失败!");
			WriteToTboxErr(str);
			OnBnClickedButtonStop();
		}
		else
		{
			/* 接收到数据 */
			if( can_sour.buf[1] == 0x71 && can_sour.buf[5] == 0x01)
			{	
				DebugPrintf(NORM_DBG,"文件下载校验成功",0,0,DBG_VAR_TYPE);
				if (m_codetype == 0) {
					can_data.id = avm_id;
					can_data.len = 8;
					memset(can_data.buf, 0x00, 8);
					can_data.buf[0] = 0x07;
					can_data.buf[1] = 0x2E;
					can_data.buf[2] = 0xF1;
					can_data.buf[3] = 0x99;
					can_data.buf[4] = 0x20;
					can_data.buf[5] = (CTime::GetCurrentTime().GetYear() - 2000) / 10 << 4 | ((CTime::GetCurrentTime().GetYear() - 2000) % 10);
					can_data.buf[6] = (CTime::GetCurrentTime().GetMonth()) / 10 << 4 | ((CTime::GetCurrentTime().GetMonth()) % 10);
					can_data.buf[7] = (CTime::GetCurrentTime().GetDay()) / 10 << 4 | ((CTime::GetCurrentTime().GetDay()) % 10);
					can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);
					DebugPrintf(NORM_DBG, "写日期", 0, 0, DBG_VAR_TYPE);


					diag_wait_time = GetTickCount();
					diag_update_step = UPDATE_TO_AVM_WRITE_DATA;
				}
				else {
					can_data.id = avm_id;
					can_data.len = 8;
					memset(can_data.buf, 0x00, 8);
					can_data.buf[0] = 0x02;
					can_data.buf[1] = 0x11;
					can_data.buf[2] = 0x01;
					can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);

					diag_wait_time = GetTickCount();
					diag_update_step = UPDATE_TO_AVM_RESET;
					DebugPrintf(NORM_DBG, "终端复位", 0, 0, DBG_VAR_TYPE);
					
				}

			}
			else if (can_sour.buf[1] == 0x71 && can_sour.buf[5] == 0x00) {
				DebugPrintf(NORM_DBG, "文件校验失败", 0, 0, DBG_VAR_TYPE);
				OnBnClickedButtonStop();
			}
			else if(can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x31)
			{	
				if (can_sour.buf[3] == 0x78) {
					DebugPrintf(NORM_DBG, "NRC-78 等待...", 0, 0, DBG_VAR_TYPE);
					break;
				}
				DebugPrintf(NORM_DBG,"文件校验负响应",0,0,DBG_VAR_TYPE);
				OnBnClickedButtonStop();
			}
		}
		break;

	case UPDATE_TO_AVM_WRITE_DATA:
		if ((GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME)
		{	/* 等待超时 */
			WriteToTboxErr("发送写日期指令超时!");
			OnBnClickedButtonStop();
		}
		else
		{	/* 接收到数据 */
			if (can_sour.buf[1] == 0x6E && can_sour.buf[2] == 0xF1 && can_sour.buf[3] == 0x99)
			{	/* 接收到应答, */
				DebugPrintf(NORM_DBG, "发送写日期指令成功", 0, 0, DBG_VAR_TYPE);

	
				can_data.id = avm_id;
				can_data.len = 8;
				memset(can_data.buf, 0x00, 8);
				can_data.buf[0] = 0x05;
				can_data.buf[1] = 0x2E;
				can_data.buf[2] = 0xF1;
				can_data.buf[3] = 0x89;

				CString key1, key2;
				GetDlgItem(IDC_EDIT1)->GetWindowText(key1);
				GetDlgItem(IDC_EDIT4)->GetWindowText(key2);
				UpdateData(FALSE);

				
				can_data.buf[4] = _ttoi(key1);  
				can_data.buf[5] = _ttoi(key2);
				//BCD码存储  
				can_data.buf[4] = (can_data.buf[4]/10 <<4 )  | (can_data.buf[4]%10 & 0x0F);
				can_data.buf[5] = (can_data.buf[5]/10 << 4) | (can_data.buf[5] % 10 & 0x0F);

				can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);

				diag_wait_time = GetTickCount();
				diag_update_step = UPDATE_TO_AVM_WRITE_VER;
				DebugPrintf(NORM_DBG, "写软件版本", 0, 0, DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x2E) {

				str.Format("发送写日期指令失败，");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();

			}

		}
		break;

	case UPDATE_TO_AVM_WRITE_VER:
		if ((GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME)
		{	/* 等待超时 */
			WriteToTboxErr("写软件版本指令超时!");
			OnBnClickedButtonStop();
		}
		else
		{	/* 接收到数据 */
			if (can_sour.buf[1] == 0x6E && can_sour.buf[2] == 0xF1 && can_sour.buf[3] == 0x89)
			{	/* 接收到应答, */
				DebugPrintf(NORM_DBG, "写软件版本指令成功", 0, 0, DBG_VAR_TYPE);


				can_data.id = avm_id;
				can_data.len = 8;
				memset(can_data.buf, 0x00, 8);
				can_data.buf[0] = 0x02;
				can_data.buf[1] = 0x11;
				can_data.buf[2] = 0x01;
				can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);

				diag_wait_time = GetTickCount();
				diag_update_step = UPDATE_TO_AVM_RESET;
				DebugPrintf(NORM_DBG, "终端复位", 0, 0, DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x2E) {

				str.Format("写软件版本指令失败，");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();

			}

		}
		break;

	case UPDATE_TO_AVM_RESET:
		if ((GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME)
		{	/* 等待超时 */
			WriteToTboxErr("终端复位超时!");
			OnBnClickedButtonStop();
		}
		else
		{	/* 接收到数据 */
			if (can_sour.buf[1] == 0x51 && can_sour.buf[2] == 0x01)
			{	/* 接收到应答, */
				DebugPrintf(NORM_DBG, "终端复位指令成功", 0, 0, DBG_VAR_TYPE);

				Sleep(1500);//等待复位成功
				can_data.id = avm_id;
				can_data.len = 8;
				memset(can_data.buf, 0x00, 8);
				can_data.buf[0] = 0x04;
				can_data.buf[1] = 0x14;
				can_data.buf[2] = 0xFF;
				can_data.buf[3] = 0xFF;
				can_data.buf[4] = 0xFF;
				can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);

				diag_wait_time = GetTickCount();
				diag_update_step = UPDATA_TO_AVM_CLEAR_DTC;
				DebugPrintf(NORM_DBG, "清除DTC", 0, 0, DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x11) {

				str.Format("终端复位失败，");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();

			}

		}
		break;


	case UPDATA_TO_AVM_CLEAR_DTC:
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			str.Format("未能获取到清除DTC回应!");
			WriteToTboxErr(str);
			OnBnClickedButtonStop();
		}
		else {
			if (can_sour.buf[1] == 0x54) {
				DebugPrintf(NORM_DBG, "DTC 清除成功", 0, 0, DBG_VAR_TYPE);
				DebugPrintf(NORM_DBG, "升级流程完成", 0, 0, DBG_VAR_TYPE);
				OnBnClickedButtonStop();
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x14) {
				str.Format("DTC清除失败!");
				WriteToTboxErr(str);
				OnBnClickedButtonStop();
			}
		
		}

		break;
	default:
		WriteToTboxErr("发现错误的升级步骤");
		OnBnClickedButtonStop();
		break;
	}
	return res;
}








/* 进入传输流程，从TBOX读入数据，需要安全认证 */
int CSGMW_diagDlg::TransferFromTbox(CAN_OBJ can_sour)
{
	int				res       =  0;
	unsigned char*	p_temp    = NULL;
	unsigned long   diag_seed =  0;
	unsigned long	diag_pas  =  0;
	CAN_OBJ			can_data  = {0};
	CString			str;

	//打印
	if(can_sour.len)
	{
		CString strTime = p_dlg->comm.GetTime();
		str.Format("打印 -- [%s] --> recv:", strTime);
		for(int i=0; i<can_sour.len; ++i)
		{
			CString strCh;
			strCh.Format(" %02x", can_sour.buf[i]);
			str += strCh;
		}
		ShowInfo(str, 1);
	}

	
	switch( diag_transfer_step )
	{
	case TRANS_FROM_TBOX_NULL:
		/* 无操作 */
		break;
	case TRANS_FROM_TBOX_EXTERN_MODE:
		/* 发送进入扩展模式指令 */
		can_data.id     = avm_id;
		can_data.len    = 8;
		memset(can_data.buf, 0x55, 8);
		can_data.buf[0] = 0x02;
		can_data.buf[1] = 0x10;
		can_data.buf[2] = 0x03;

		can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
		DebugPrintf(NORM_DBG,"发送进入扩展模式指令",0,0,DBG_VAR_TYPE);

		res				= 1;
		diag_wait_time	= GetTickCount();
		diag_transfer_step = TRANS_FROM_TBOX_WAIT_EXTERN_MODE;
		break;
	case TRANS_FROM_TBOX_WAIT_EXTERN_MODE:
		/* 等待进入扩展模式应答，获取到应答后，发送读TBOX序列号指令 */
		if( (GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			WriteToTboxErr("进入扩展模式指令未响应，写入数据失败!");
		}
		else
		{	/* 接收到数据 */
			if( can_sour.buf[1] == 0x50 && can_sour.buf[2] == 0x03 )
			{	/* 接收到应答,发送读序列号操作 */
				DebugPrintf(NORM_DBG,"进入扩展模式成功",0,0,DBG_VAR_TYPE);

				/* 请求种子数据 */
				can_data.id  = avm_id;
				can_data.len = 8;
				memset(can_data.buf, 0x55, 8);
				can_data.buf[0] = 0x02;
				can_data.buf[1] = 0x27;
				can_data.buf[2] = 0x01;
				can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);

				res				= 1;
				diag_wait_time	= GetTickCount();
				diag_transfer_step = TRANS_FROM_TBOX_WAIT_READ_SEED;
				DebugPrintf(NORM_DBG,"请求种子信息",0,0,DBG_VAR_TYPE);
			}
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == 0x10 )
			{
				res = 1;
				str.Format("进入扩展模式失败，");
				str += CanErrHandle(can_sour.buf[3]);  
				WriteToTboxErr(str);
			}
		}
		break;
	case TRANS_FROM_TBOX_WAIT_READ_SEED:
		/* 等待获取到种子信息,并且发送密钥 */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			WriteToTboxErr("未能获取到TBOX加密种子，写入数据失败!");
		}
		else
		{	/* 发现TBOX上报的数据 */
			if( can_sour.buf[1] == 0x67 && can_sour.buf[2] == 0x01 )
			{	/* 接收到应答,发送密钥 */
				/* 保存种子数据 */
				p_temp = &can_sour.buf[3];
				diag_seed = GET_DWORD(p_temp);
				DebugPrintf(NORM_DBG,"found tbox sedd: 0x",(unsigned char*)&diag_seed,4,DBG_VAR_TYPE);

				if( diag_seed == 0  )
				{   /* 秘钥已经解锁，直接写入数据 */
					DebugPrintf(NORM_DBG,"秘钥已解锁，直接写入数据",0,0,DBG_VAR_TYPE);

					if(TRANS_LIST == trans_mod)
					{
						/* 发送文件列表读取申请 */
						can_data.id     = avm_id;
						can_data.len    = 8;
						memset(can_data.buf, 0x55, 8);
						can_data.buf[0] = 0x05;
						can_data.buf[1] = 0x38;
						can_data.buf[2] = 0x05;
						can_data.buf[3] = 0x00;
						can_data.buf[4] = 0x01;
						can_data.buf[5] = 0x00;
						can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
						DebugPrintf(NORM_DBG,"文件列表读取申请",0,0,DBG_VAR_TYPE);

						res				= 1;
						diag_wait_time	= GetTickCount();
						diag_transfer_step = TRANS_FROM_TBOX_REQUEST_READDIR;
					}

					if(TRANS_FILE == trans_mod)
					{
						/* 发送文件读取申请 */
						if(!m_dequeSelFile.empty())
						{
							can_data.id     = avm_id;
							can_data.len    = 16 - 2;
							memset(can_data.buf, 0x55, 14);
							can_data.buf[0] = 0x0c;
							can_data.buf[1] = 0x38;
							can_data.buf[2] = 0x04;
							can_data.buf[3] = 0x00;
							can_data.buf[4] = 0x08;
							memcpy(can_data.buf+5, m_dequeSelFile.at(file_cnt-1).file_buf, 8);
							can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
							str.Format("文件(%s) - 读取申请", m_dequeSelFile.at(file_cnt-1).file_name);
							DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

							res				= 1;
							diag_wait_time	= GetTickCount();
							diag_transfer_step = TRANS_FROM_TBOX_REQUEST_READFILE;
						}
						else
						{
							/* 文件读取申请错误 */
							res				= 1;
							str.Format("没有选中的文件，文件读取申请错误");
							WriteToTboxErr(str);
						}
					}

					if(DELETE_FILE == trans_mod)
					{
						/* 发送文件读取申请 */
						if(!m_dequeSelFile.empty())
						{
							can_data.id     = avm_id;
							can_data.len    = 16 - 2;
							memset(can_data.buf, 0x55, 14);
							can_data.buf[0] = 0x0c;
							can_data.buf[1] = 0x38;
							can_data.buf[2] = 0x02;
							can_data.buf[3] = 0x00;
							can_data.buf[4] = 0x08;
							memcpy(can_data.buf+5, m_dequeSelFile.at(file_cnt-1).file_buf, 8);
							can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
							str.Format("文件(%s) - 删除申请", m_dequeSelFile.at(file_cnt-1).file_name);
							DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

							res				= 1;
							diag_wait_time	= GetTickCount();
							diag_transfer_step = TRANS_FROM_TBOX_REQUEST_DELETE;
						}
						else
						{
							/* 文件读取申请错误 */
							res				= 1;
							str.Format("没有选中的文件，文件删除申请错误");
							WriteToTboxErr(str);
						}
					}
					break;
				}

				/* 生成password */
				diag_pas = safe.uds_calc_key(diag_seed,1);
					
				/* 发送密钥 */
				can_data.id     = avm_id;
				can_data.len    = 8;
				memset(can_data.buf, 0x55, 8);
				can_data.buf[0] = 0x06;
				can_data.buf[1] = 0x27;
				can_data.buf[2] = 0x02;
				can_data.buf[3] = GET_BYTE_24(diag_pas);
				can_data.buf[4] = GET_BYTE_16(diag_pas);
				can_data.buf[5] = GET_BYTE_8(diag_pas);
				can_data.buf[6] = GET_BYTE_0(diag_pas);
				can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);

				res				= 1;
				diag_wait_time	= GetTickCount();
				diag_transfer_step = TRANS_FROM_TBOX_WAIT_READ_PASSWORD;
				DebugPrintf(NORM_DBG,"发送秘钥",0,0,DBG_VAR_TYPE);
			}
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == 0x27 )
			{
				res = 1;
				WriteToTboxErr("无法获取TBOX种子信息，写入数据失败!");

				str.Format("get tbox seed failed: %d",can_data.buf[3]);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
			}
		}
		break;
	case TRANS_FROM_TBOX_WAIT_READ_PASSWORD:
		/* 等待密钥发送应答，开始向TBOX写入数据 */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			WriteToTboxErr("未能获取到TBOX密钥信息，写入数据失败!");
		}
		else
		{	/* 接收到数据 */
			if( can_sour.buf[1] == 0x67 && can_sour.buf[2] == 0x02 )
			{	/* 接收到秘钥验证应答，发送文件列表读取申请 */
				DebugPrintf(NORM_DBG,"秘钥验证成功，开始写入数据",0,0,DBG_VAR_TYPE);

				if(TRANS_LIST == trans_mod)
				{
					/* 发送文件列表读取申请 */
					can_data.id     = avm_id;
					can_data.len    = 8;
					memset(can_data.buf, 0x55, 8);
					can_data.buf[0] = 0x05;
					can_data.buf[1] = 0x38;
					can_data.buf[2] = 0x05;
					can_data.buf[3] = 0x00;
					can_data.buf[4] = 0x01;
					can_data.buf[5] = 0x00;
					can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
					DebugPrintf(NORM_DBG,"文件列表读取申请",0,0,DBG_VAR_TYPE);

					res				= 1;
					diag_wait_time	= GetTickCount();
					diag_transfer_step = TRANS_FROM_TBOX_REQUEST_READDIR;
				}

				if(TRANS_FILE == trans_mod)
				{
					/* 发送文件读取申请 */
					if(!m_dequeSelFile.empty())
					{
						can_data.id     = avm_id;
						can_data.len    = 16 - 2;
						memset(can_data.buf, 0x55, 14);
						can_data.buf[0] = 0x08;
						can_data.buf[1] = 0x38;
						can_data.buf[2] = 0x04;
						memcpy(can_data.buf+3, m_dequeSelFile.at(file_cnt-1).file_buf, 6);
						can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
						str.Format("文件(%s) - 读取申请", m_dequeSelFile.at(file_cnt-1).file_name);
						DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

						res				= 1;
						diag_wait_time	= GetTickCount();
						diag_transfer_step = TRANS_FROM_TBOX_REQUEST_READFILE;
					}
					else
					{
						/* 文件读取申请错误 */
						res				= 1;
						str.Format("没有选中的文件，文件读取申请错误");
						WriteToTboxErr(str);
					}
				}

				if(DELETE_FILE == trans_mod)
				{
					/* 发送文件读取申请 */
					if(!m_dequeSelFile.empty())
					{
						can_data.id     = avm_id;
						can_data.len    = 16 - 2;
						memset(can_data.buf, 0x55, 14);
						can_data.buf[0] = 0x0c;
						can_data.buf[1] = 0x38;
						can_data.buf[2] = 0x02;
						can_data.buf[3] = 0x00;
						can_data.buf[4] = 0x08;
						memcpy(can_data.buf+5, m_dequeSelFile.at(file_cnt-1).file_buf, 8);
						can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
						str.Format("文件(%s) - 删除申请", m_dequeSelFile.at(file_cnt-1).file_name);
						DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

						res				= 1;
						diag_wait_time	= GetTickCount();
						diag_transfer_step = TRANS_FROM_TBOX_REQUEST_DELETE;
					}
					else
					{
						/* 文件读取申请错误 */
						res				= 1;
						str.Format("没有选中的文件，文件删除申请错误");
						WriteToTboxErr(str);
					}
				}
			}
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == 0x27 )
			{
				res				= 1;
				WriteToTboxErr("密钥错误，写入数据失败!");

				str.Format("tbox password check failed: %d",can_sour.buf[3]);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
			}
		}
		break;
	case TRANS_FROM_TBOX_REQUEST_READDIR:
		/* 等待文件列表读取申请应答 */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			WriteToTboxErr("未能获取到文件列表读取申请响应，写入数据失败!");
		}
		else
		{
			/* 接收到数据 */
			if( can_sour.buf[2] == 0x78 && can_sour.buf[3] == 0x05 )
			{
				DebugPrintf(NORM_DBG,"文件列表读取申请成功",0,0,DBG_VAR_TYPE);

				unsigned int  max_day = 0;
				unsigned int  file_idx = 0;
				unsigned long temp_hour = 0;
				unsigned int  i,j;
				TBOX_FILE_LIST file_list = {0};

				max_day = (can_sour.buf[1] - 2) / 6;
				p_temp = &(can_sour.buf[4]);
				file_idx = 0;

				m_comb_file_list.ResetContent();
				m_comb_file_list.InsertString(0, "全部选中");
				m_comb_file_list.SetCurSel(0);
				for( i = 0 ; i < max_day ; i++ )
				{
					temp_hour = p_temp[3] + (p_temp[4]<<8) + (p_temp[5]<<16);
					for( j = 0 ; j < 24 ; j++ )
					{
						if( temp_hour & (1<<j) )
						{
							file_list.file_buf[0] = p_temp[0];
							file_list.file_buf[1] = p_temp[1];
							file_list.file_buf[2] = p_temp[2];
							file_list.file_buf[3] = (temp_hour & (1<<j)) & 0xff;
							file_list.file_buf[4] = ((temp_hour & (1<<j))>>8) & 0xff;
							file_list.file_buf[5] = ((temp_hour & (1<<j))>>16) & 0xff;
							file_list.file_name.Format("20%02d%02d%02d%02d.bin",p_temp[0],p_temp[1],p_temp[2],j);
							file_list.file_length = 0;
							file_list.file_serial = file_idx;
							m_dequeFileList.push_back(file_list);
							m_comb_file_list.InsertString(file_idx, file_list.file_name);
							file_idx++;
						}
					}
					p_temp+=6;
				}

				/* 退出传输文件列表流程 */
				res				= 1;
				diag_wait_time	= GetTickCount();
				diag_transfer_step = TRANS_FROM_TBOX_NULL;
				handle_flag		= 0;
				handle_type		= 0;
			}
			else if( can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x38 )
			{
				/* 文件列表读取申请失败 */
				res				= 1;
				str.Format("文件列表读取申请失败，");
				str += CanErrHandle(can_sour.buf[3]);  
				WriteToTboxErr(str);
			}
		}
		break;
	case TRANS_FROM_TBOX_REQUEST_READFILE:
		/* 文件读取申请 */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			str.Format("未能获取到文件读取申请响应，写入数据失败!");
			WriteToTboxErr(str);
		}
		else
		{
			/* 接收到数据 */
			if( can_sour.buf[1] == 0x78 && can_sour.buf[2] == 0x04 )
			{
				str.Format("文件(%s) - 读取申请成功", m_dequeSelFile.at(file_cnt-1).file_name);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				p_temp = &(can_sour.buf[3]);
				m_block_num = GET_WORD(p_temp);

				str.Format("文件(%s)长度：%d", m_dequeSelFile.at(file_cnt-1).file_name, m_block_num*320);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				str.Format("文件(%s)块数目：%d", m_dequeSelFile.at(file_cnt-1).file_name, m_block_num);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				/* 初始化进度条 */
				m_cProgress.SetRange32(0, m_block_num);
				m_cProgress.SetPos(0);

				/* 文件数据传输 */
				str.Format("文件(%s) - 开始传输", m_dequeSelFile.at(file_cnt-1).file_name);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				can_data.id     = avm_id;
				can_data.len    = 8;
				memset(can_data.buf, 0x55, 8);
				can_data.buf[0] = 0x03;
				can_data.buf[1] = 0x36;
				can_data.buf[2] = (block_cnt>>8)&0xff;
				can_data.buf[3] = block_cnt&0xff;

				can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
				str.Format("文件传输%d", block_cnt);
				//DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				ShowInfo(str, 2);

				res				= 1;
				diag_wait_time	= GetTickCount();
				diag_transfer_step = TRANS_FROM_TBOX_TRANSFER_FILEDATA;
			}
			else if( can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x38 )
			{
				/* 文件读取申请失败 */
				res				= 1;
				str.Format("文件(%s) - 读取申请失败，", m_dequeSelFile.at(file_cnt-1).file_name);
				str += CanErrHandle(can_sour.buf[3]);  
				WriteToTboxErr(str);
			}
		}
		break;
	case TRANS_FROM_TBOX_TRANSFER_FILEDATA:
		/* 文件传输 */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			str.Format("未能获取到文件数据传输响应，写入数据失败!");
			WriteToTboxErr(str);
		}
		else
		{
			/* 接收到数据 */
			if( can_sour.buf[2] == 0x76 /*&& can_sour.buf[3] == block_cnt*/ )
			{	/* 文件传输 */

				DWORD dw = ((can_sour.buf[0] & 0x0f) << 8) + can_sour.buf[1] - 3;
				//memcpy(m_file_data+f_index, can_sour.buf+4, dw);
				f_index += dw;
				str.Format("字节数：%d", f_index);
				ShowInfo(str, 2);

				/* 文件进度条 */
				m_cProgress.SetPos(f_index);

				str.Format("文件传输%d成功", block_cnt);
				//DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				ShowInfo(str, 2);

				if(block_cnt < m_block_num)
				{
					/* 文件传输 */
					block_cnt++;
					can_data.id     = avm_id;
					can_data.len    = 8;
					memset(can_data.buf, 0x55, 8);
					can_data.buf[0] = 0x03;
					can_data.buf[1] = 0x36;
					can_data.buf[2] = (block_cnt>>8)&0xff;
					can_data.buf[3] = block_cnt&0xff;
					can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
					str.Format("文件传输%d", block_cnt);
					//DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
					ShowInfo(str, 2);

					res				= 1;
					diag_wait_time	= GetTickCount();
					diag_transfer_step = TRANS_FROM_TBOX_TRANSFER_FILEDATA;
				}
				else if(block_cnt == m_block_num)
				{
					str.Format("文件(%s) - 传输完成", m_dequeSelFile.at(file_cnt-1).file_name);
					DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);


					/* 文件传输退出 */			
					can_data.id     = avm_id;
					can_data.len    = 8;
					memset(can_data.buf, 0x55, 8);
					can_data.buf[0] = 0x01;
					can_data.buf[1] = 0x37;
					can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
					str.Format("文件(%s) - 传输退出", m_dequeSelFile.at(file_cnt-1).file_name);
					DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

					res				= 1;
					diag_wait_time	= GetTickCount();
					diag_transfer_step = TRANS_FROM_TBOX_TRANSFER_FILEEXIT;
				}
			}
			else if(can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x36)
			{
				/* 文件传输失败 */
				res				= 1;
				str.Format("文件(%s) - 传输失败，", m_dequeSelFile.at(file_cnt-1).file_name);
				str += CanErrHandle(can_sour.buf[3]);  
				WriteToTboxErr(str);
			}
		}
		break;
	case TRANS_FROM_TBOX_TRANSFER_FILEEXIT:
		/* 文件传输退出 */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			str.Format("未能获取到文件传输退出响应，写入数据失败!");
			WriteToTboxErr(str);
		}
		else
		{
			/* 接收到数据 */
			if( can_sour.buf[1] == 0x77 )
			{	/* 文件传输退出 */
				str.Format("文件(%s) - 传输退出完成", m_dequeSelFile.at(file_cnt-1).file_name);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				/* 生成文件 */
				CString strFilePath;
				strFilePath.Format("%s\\%s", strDirName, m_dequeSelFile.at(file_cnt-1).file_name);
				HANDLE hFile = INVALID_HANDLE_VALUE;
				DWORD dwPos = 0 , dwLen = 0;
				//unsigned char ucHead[2];
				//ucHead[0] = 0xFF;
				//ucHead[1] = 0xFE;
				hFile = CreateFile(strFilePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
				if(hFile != INVALID_HANDLE_VALUE)
				{
					dwPos = GetFileSize(hFile, 0);
					SetFilePointer(hFile , dwPos , 0 , FILE_BEGIN);
					/*if(dwPos == 0)
						WriteFile(hFile, ucHead, 2, &dwLen, 0);*/
					//if(!WriteFile(hFile, m_file_data, m_file_size, &dwLen , 0))
					//{
					//	str.Format("写文件(%s)出错 - %d", m_dequeSelFile.at(file_cnt-1).file_name, GetLastError());
					//	DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
					//}
					CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;

					str.Format("======= 生成文件(%s) =======", m_dequeSelFile.at(file_cnt-1).file_name);
					DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				}
				else
				{
					str.Format("打开文件(%s)出错 - %d", m_dequeSelFile.at(file_cnt-1).file_name, GetLastError());
					DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				}

				if((size_t)file_cnt < m_dequeSelFile.size())
				{
					m_dwCRC	= 0;
					m_file_size = 0;
					//memset(m_file_data, 0, sizeof(m_file_data));
					m_block_size =0;
					m_block_num = 0;
					block_cnt = 1;
					f_index = 0;
					file_cnt++;

					//* 进入下一个文件读取申请流程 */
					res				= 1;
					diag_wait_time	= GetTickCount();
					diag_transfer_step = TRANS_FROM_TBOX_EXTERN_MODE;
					CAN_OBJ		can_buf  = {0};
					TransferFromTbox(can_buf);
				}
				else
				{
					/* 关闭传输流程 */
					res				= 1;
					diag_wait_time	= GetTickCount();
					diag_transfer_step = TRANS_FROM_TBOX_NULL;
					handle_flag		= 0;
					handle_type		= 0;
				}
			}
			else if(can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x37)
			{
				/* 文件传输退出失败 */
				res				= 1;
				str.Format("文件(%s) - 传输退出失败，", m_dequeSelFile.at(file_cnt-1).file_name);
				str += CanErrHandle(can_sour.buf[3]);  
				WriteToTboxErr(str);
			}
		}
		break;

/*********************************文件删除流程********************************************/
	case TRANS_FROM_TBOX_REQUEST_DELETE:
	/* 文件删除申请 */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* 等待超时 */
			str.Format("未能获取到文件删除申请响应，写入数据失败!");
			WriteToTboxErr(str);
		}
		else
		{
			/* 接收到数据 */
			if( can_sour.buf[1] == 0x78 && can_sour.buf[2] == 0x02 )
			{	/* 文件删除申请 */
				str.Format("文件(%s) - 删除申请成功", m_dequeSelFile.at(file_cnt-1).file_name);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				if((size_t)file_cnt < m_dequeSelFile.size())
				{
					file_cnt++;

					//* 进入下一个文件删除申请流程 */
					res				= 1;
					diag_wait_time	= GetTickCount();
					diag_transfer_step = TRANS_FROM_TBOX_EXTERN_MODE;
					CAN_OBJ		can_buf  = {0};
					TransferFromTbox(can_buf);
				}
				else
				{
					/* 删除队列中已删除的文件 */
					std::deque<TBOX_FILE_LIST>::iterator iter_list;
					std::deque<TBOX_FILE_LIST>::iterator iter_file;
					for(iter_file = m_dequeSelFile.begin(); iter_file != m_dequeSelFile.end(); iter_file++)
					{
						for(iter_list = m_dequeFileList.begin(); iter_list != m_dequeFileList.end(); iter_list++)
						{
							if( iter_file->file_serial  == iter_list->file_serial )
							{
								m_dequeFileList.erase(iter_list);
								m_comb_file_list.DeleteString(iter_list->file_serial + 1);
							}
						}
					}
					m_dequeSelFile.clear();


					/* 关闭传输流程 */
					res				= 1;
					diag_wait_time	= GetTickCount();
					diag_transfer_step = TRANS_FROM_TBOX_NULL;
					handle_flag		= 0;
					handle_type		= 0;
				}
			}
			else if(can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x38)
			{
				/* 文件删除申请失败 */
				res				= 1;
				str.Format("文件(%s) - 删除申请失败，", m_dequeSelFile.at(file_cnt-1).file_name);
				str += CanErrHandle(can_sour.buf[3]);  
				WriteToTboxErr(str);
			}
		}
		break;
	default:
		WriteToTboxErr("发现错误的文件传输步骤");
		break;
	}
	return res;
}