#include "stdafx.h"
#include "SGMW_diag.h"
#include "SGMW_diagDlg.h"
#include "afxdialogex.h"
#include "diag.h"

#define		SIZE_F189			2
#define		SIZE_F089			8
#define		SIZE_F187			11
#define		SIZE_F18A			7
#define		SIZE_F18C			13
#define		SIZE_F190			17

int CSGMW_diagDlg::GetTboxReadInfo(CAN_OBJ can_data)
{
	int	sub,did;
	unsigned char* p_temp = NULL;
	//unsigned char* p_data;
	unsigned char p_data[2048] = {0};
	unsigned char temp_buf[6] = {0};
	CString str;
	CString avmLogStr;
	CString strBatch;
	unsigned int total_len = can_data.len;
	unsigned int index = 0;
	sub    = can_data.buf[1];
	index += 2;

	//sub    = can_data.buf[1];
	//p_temp = &can_data.buf[2];
	//did    = GET_WORD(p_temp);
	//p_data = &can_data.buf[4];
	avmLogStr.Format("[AVM Debug Log | %s:%d] 解析DID数据...", __FUNCTION__, __LINE__);
	p_dlg->ShowInfo(avmLogStr, 1);
	while(index < total_len)
	{
		p_temp = &can_data.buf[index];
		did    = GET_WORD(p_temp);
		index += 2;

		switch( did )
		{
		case 0xF189:	// ECU软件版本编号
			memcpy(p_data, &can_data.buf[index], SIZE_F189);
			index += SIZE_F189;
			//str.Format("0xF189-海马软件版本编号: %d",p_data);
			avmLogStr.Format("0xF189-ECU软件版本编号: %d .%d",(p_data[0]>>4)*10+(p_data[0] & 0x0F ), (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0 , sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;

		case 0xF186:	// 当前会话模式
			memcpy(p_data, &can_data.buf[index], 1);
			index += 1;
			//str.Format("0xF189-海马软件版本编号: %d",p_data);
			avmLogStr.Format("0xF186当前会话模式: %d", p_data[0]);
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;


		case 0xF187:	// 海马零部件号
			memcpy(p_data, &can_data.buf[index], 9);
			index += 9;
			//str.Format("0xF189-海马软件版本编号: %d",p_data);
			avmLogStr.Format("0xF187-海马零部件号: %s", p_data);
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;

		case 0xF18A:	// 供应商代码
			memcpy(p_data, &can_data.buf[index], 8);
			index += 8;
			//str.Format("0xF189-海马软件版本编号: %d",p_data);
			avmLogStr.Format("0xF18A-供应商代码: %s", p_data);
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;
	
		case 0xF188:	// 海马软件编号
			memcpy(p_data, &can_data.buf[index], 9);
			index += 9;
			avmLogStr.Format("0xF188-海马软件编号: %s", p_data);
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;



		case 0xF18B:	// 控制器生产日期
			memcpy(p_data, &can_data.buf[index], 4);
			index += 4;
			//str.Format("0xF189-海马软件版本编号: %d",p_data);
			avmLogStr.Format("0xF18B-ECU控制器生产日期:  %d年-%d月-%d日", ((p_data[0] >> 4) * 10 + (p_data[0] & 0x0F))*100 + (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F),
				(p_data[2] >> 4) * 10 + (p_data[2] & 0x0F) , (p_data[3] >> 4) * 10 + (p_data[3] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;

		
		case 0xF18C:	// 序列号
			memcpy(p_data, &can_data.buf[index], 10);
			index += 10;
			//str.Format("0xF189-海马软件版本编号: %d",p_data);
			avmLogStr.Format("0xF18C-ECU序列号: %x,%x, %x,%x, %x,%x, %x,%x, %x,%x", p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7], p_data[8], p_data[9]);
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;


		case 0xF18D:	// 支持的功能单位
			memcpy(p_data, &can_data.buf[index], SIZE_F189);
			index += SIZE_F189;
			//str.Format("0xF189-海马软件版本编号: %d",p_data);
			avmLogStr.Format("0xF18D-ECU支持的功能单位: %x", p_data[0]);
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;

		case 0xF190:	// VINDataIdentifier
			memcpy(p_data, &can_data.buf[index], SIZE_F190);
			index += SIZE_F190;
			//str.Format("0xF190-VINDataIdentifier: %s",p_data);

			avmLogStr.Format("0xF190-VINDataIdentifier: %s", p_data);
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;

		case 0xF197:	// 控制器名称
			memcpy(p_data, &can_data.buf[index], 4);
			index += 4;
			//str.Format("0xF189-海马软件版本编号: %d",p_data);
			avmLogStr.Format("0xF197-ECU控制器名称: %s", p_data);
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;
	


		
		case 0xF19D:	// 装车日期
			memcpy(p_data, &can_data.buf[index], 4);
			index += 4;
			//str.Format("0xF187-Part Number: %s",p_data);

			avmLogStr.Format("0xF19D-ECU装车日期:  %d年-%d月-%d日", ((p_data[0] >> 4) * 10 + (p_data[0] & 0x0F)) * 100 + (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F),
				(p_data[2] >> 4) * 10 + (p_data[2] & 0x0F), (p_data[3] >> 4) * 10 + (p_data[3] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);

			//avmLogStr.Format("[DID Data Value] %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", p_data[0], p_data[1], 
			//				p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7], p_data[8], p_data[9], p_data[10]);
			//p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0 , sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;
		
		case 0xF199:	// 程序刷写日期
			memcpy(p_data, &can_data.buf[index], 4);
			index += 4;
			//str.Format("0xF187-Part Number: %s",p_data);

			avmLogStr.Format("0xF199-ECU程序刷写日期: %d年-%d月-%d日", ((p_data[0] >> 4) * 10 + (p_data[0] & 0x0F)) * 100 + (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F),
				(p_data[2] >> 4) * 10 + (p_data[2] & 0x0F), (p_data[3] >> 4) * 10 + (p_data[3] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);
			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			break;

		case 0xF19B:	// 标定日期
			memcpy(p_data, &can_data.buf[index], 4);
			index += 4;
			//str.Format("0xF187-Part Number: %s",p_data);

			avmLogStr.Format("0xF19B-ECU标定日期:  %d年-%d月-%d日", ((p_data[0] >> 4) * 10 + (p_data[0] & 0x0F)) * 100 + (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F),
				(p_data[2] >> 4) * 10 + (p_data[2] & 0x0F), (p_data[3] >> 4) * 10 + (p_data[3] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);
			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			break;



		case 0xF1B0:	// 控制器电压
			memcpy(p_data, &can_data.buf[index], 2);
			index += 2;
			//str.Format("0xF18A-System Supplier Identifier: %s",p_data);

			avmLogStr.Format("0xF1B0-控制器电压: %d.%d", (p_data[0] >> 4) * 10 + (p_data[0] & 0x0F), (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0 , sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;
		case 0xFD00:	// MCU_APP版本号
			memcpy(p_data, &can_data.buf[index], 3);
			index += 3;
			//str.Format("0xF18A-System Supplier Identifier: %s",p_data);

			avmLogStr.Format("0xFD00-MCU_APP版本号: %d.%d.%d", ((p_data[0] >> 4) * 10 + (p_data[0] & 0x0F)), (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F), (p_data[2] >> 4) * 10 + (p_data[2] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;
		

		case 0xFD01:	// MCU_BOOT版本号 
			memcpy(p_data, &can_data.buf[index], 3);
			index += 3;
			//str.Format("0xF18A-System Supplier Identifier: %s",p_data);

			avmLogStr.Format("0xFD01-MCU_BOOT版本号: %d.%d.%d", (p_data[0] >> 4) * 10 + (p_data[0] & 0x0F), (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F), (p_data[2] >> 4) * 10 + (p_data[2] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;


		case 0xFD02:	// ARM_APP版本号 
			memcpy(p_data, &can_data.buf[index], 3);
			index += 3;
			//str.Format("0xF18A-System Supplier Identifier: %s",p_data);

			avmLogStr.Format("0xFD02-ARM_APP版本号: %d.%d.%d", (p_data[0] >> 4) * 10 + (p_data[0] & 0x0F), (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F), (p_data[2] >> 4) * 10 + (p_data[2] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;


		case 0xFD03:	// ARM_SF版本号 
			memcpy(p_data, &can_data.buf[index], 3);
			index += 3;
			//str.Format("0xF18A-System Supplier Identifier: %s",p_data);

			avmLogStr.Format("0xFD03-ARM_SF版本号 : %d.%d.%d", (p_data[0] >> 4) * 10 + (p_data[0] & 0x0F), (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F), (p_data[2] >> 4) * 10 + (p_data[2] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;



		default:
			//str.Format("read unknow did: %d",did);
			//DebugPrintf(C1_DBG,str,0,0,DBG_VAR_TYPE);

			avmLogStr.Format("read unknow did: %d",did);
			p_dlg->ShowInfo(avmLogStr, 1);

			break;
		}
	}

	/*if(!strBatch.IsEmpty())
		DispPrintf(strBatch,0,0,DBG_VAR_TYPE);*/

	return 0;
}

/* 处理发送给PC的数据响应协议 */
int CSGMW_diagDlg::diag_ack_data(CAN_OBJ can_sour,int sid)
{
	CString	str;
	unsigned char* p_temp = NULL;
	unsigned long  num;
	unsigned long  temp;
	unsigned short p2_can_ecu_max  = 0;
	unsigned short p2__can_ecu_max = 0;
	unsigned char cnt;
	unsigned char status;
	CString avmLogStr;



	/* 处理获取其他的应答数据 */
	switch( sid & (~0x40) )
	{
	case DIAGNOSITIC_SESSION_CONTROL:
		/* 诊断进程控制服务 - diagnositic session control */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: 诊断进程控制服务...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		DebugPrintf(NORM_DBG,"found diagnositic session control ack:",(unsigned char*)can_sour.buf,can_sour.len,DBG_DATA_TYPE);

		p_temp = &can_sour.buf[3];
		p2_can_ecu_max = GET_WORD(p_temp);
		p_temp = &can_sour.buf[5];
		p2__can_ecu_max = GET_WORD(p_temp);

		switch( can_sour.buf[2])
		{
		case 0x01:
			str.Format("DiagnosticSessionControl (10 hex):DefaultSession/默认会话模式: P2 CAN_ECU_max = 0x%04x ; P2* CAN_ECU_max = 0x%04x",p2_can_ecu_max,p2__can_ecu_max);
			break;
		case 0x02:
			str.Format("DiagnosticSessionControl (10 hex):ProgrammingSession/编程会话模式: P2 CAN_ECU_max = 0x%04x ; P2* CAN_ECU_max = 0x%04x",p2_can_ecu_max,p2__can_ecu_max);
			break;
		case 0x03:
			str.Format("DiagnosticSessionControl (10 hex):ExtendedDiagnosticSession/扩展诊断会话模式: P2 CAN_ECU_max = 0x%04x ; P2* CAN_ECU_max = 0x%04x",p2_can_ecu_max,p2__can_ecu_max);
			break;
		default:
			str.Format("error mode for  DiagnosticSessionControl (10 hex): 0x%x",can_sour.buf[2]);
			break;
		}
		DispPrintf(str,0,0,DBG_VAR_TYPE);
		break;
	case ECU_RESET:
		/* ECU复位服务 -- ECU reset */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: ECU复位服务...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);
		switch( can_sour.buf[2])
		{
		case 0x01:
			DispPrintf("ECUReset (11 hex) /ECU: hardReset/ 硬复位 成功",0,0,DBG_VAR_TYPE);
			break;
		case 0x02:
			DispPrintf("ECUReset (11 hex) /ECU: keyOffOnReset/ 钥匙OFF ON  成功",0,0,DBG_VAR_TYPE);
			break;
		default:
			DispPrintf("error mode for ECUReset (11 hex) /ECU ",(unsigned char*)&can_sour.buf[2],1,DBG_VAR_TYPE);
			break;
		}
		break;
	case CLEAR_DIAGNOSTIC_INFORMATION:			
		/* 清除故障码服务 -- clear diagnostic information */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: 清除故障码服务...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);
		DispPrintf("ClearDiagnosticInformation (14 hex)/ 清除故障信息: success",0,0,DBG_VAR_TYPE);
		break;
	case READ_DTC_INFORMATION:					
		/* 读故障码服务 -- read DTC information */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: 读故障码服务...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);
		str.Format("========  ReadDTCInformation (19 hex) : 0x%02x ========",can_sour.buf[2]);
		DispPrintf(str,0,0,DBG_VAR_TYPE);
		switch( can_sour.buf[2])
		{
		case 0x01:
			DispPrintf("reportType: ",(unsigned char*)&can_sour.buf[2],1,DBG_VAR_TYPE);
			DispPrintf("DTCStatusAvailabilityMask: ",(unsigned char*)&can_sour.buf[3],1,DBG_VAR_TYPE);
			DispPrintf("DTCFormatIdentifier: ",(unsigned char*)&can_sour.buf[4],1,DBG_VAR_TYPE);
			p_temp = (unsigned char*)&can_sour.buf[5];
			temp   = GET_WORD(p_temp);
			DispPrintf("DTCCount: ",(unsigned char*)&temp,2,DBG_VAR_TYPE);
			break;
		case 0x02:
		case 0x0a:
			DispPrintf("reportType: ",(unsigned char*)&can_sour.buf[2],1,DBG_VAR_TYPE);
			DispPrintf("DTCStatusAvailabilityMask: ",(unsigned char*)&can_sour.buf[3],1,DBG_VAR_TYPE);

			cnt = (can_sour.buf[0] - 3) / 4;

			str.Format("dtc number: len = %d, cnt = %d",can_sour.buf[0],cnt);
			DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
			DispPrintf("DTCAndStatusRecord:",(unsigned char*)&cnt,1,DBG_VAR_TYPE);
			for( num = 0 ;  num < cnt ; num++)
			{
				p_temp = (unsigned char*)&can_sour.buf[4+num*4];
				temp   = GET_MWORD(p_temp);
				status = can_sour.buf[4+num*4 + 3];
				str.Format("DTC: 0x%06x, status of dtc: 0x%02x",temp,status);
				DispPrintf(str,0,0,DBG_VAR_TYPE);
			}
			break;
		case 0x03:
			DispPrintf("reportType: ",(unsigned char*)&can_sour.buf[2],1,DBG_VAR_TYPE);

			cnt = (can_sour.buf[0] - 2) / 4;

			str.Format("dtc number: len = %d, cnt = %d",can_sour.buf[0],cnt);
			DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
			DispPrintf("DTCAndStatusRecord:",(unsigned char*)&cnt,1,DBG_VAR_TYPE);
			for( num = 0 ;  num < cnt ; num++)
			{
				p_temp = (unsigned char*)&can_sour.buf[3+num*4];
				temp   = GET_MWORD(p_temp);
				status = can_sour.buf[3+num*4 + 3];
				str.Format("DTCRecord: 0x%06x, DTCSnapshotRecordNumber: 0x%02x",temp,status);
				DispPrintf(str,0,0,DBG_VAR_TYPE);
			}
			break;
		case 0x04:
			DispPrintf("reportType: ",(unsigned char*)&can_sour.buf[2],1,DBG_VAR_TYPE);
			p_temp = (unsigned char*)&can_sour.buf[3];
			temp   = GET_MWORD(p_temp);
			DispPrintf("DTCAndStatusRecord: DTC ",(unsigned char*)&temp,3,DBG_VAR_TYPE);
			DispPrintf("DTCAndStatusRecord: status of DTC ",(unsigned char*)&can_sour.buf[6],1,DBG_VAR_TYPE);

			cnt = (can_sour.buf[0] - 6) / 19;

			str.Format("dtc number: len = %d, cnt = %d",can_sour.buf[0],cnt);
			DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
			for( num = 0 ; num < cnt ; num++ )
			{
				str.Format("DTCSnapshotRecordNumber%d: 0x%x",num,can_sour.buf[7 + num*19 + 0 ]);
				DispPrintf(str,0,0,DBG_VAR_TYPE);
				str.Format("DTCSnapshotRecordNumberOfIdentifiers%d: 0x%x",num,can_sour.buf[7 + num*19 + 1 ]);
				DispPrintf(str,0,0,DBG_VAR_TYPE);
				p_temp = (unsigned char*)&can_sour.buf[7 + num*19 + 2 ];
				temp   = GET_WORD(p_temp);
				str.Format("ECU Voltage DataIdentifier: 0x%x",temp);
				DispPrintf(str,0,0,DBG_VAR_TYPE);
				str.Format("ECU Voltage: 0x%x", can_sour.buf[7 + num*19 + 4]);
				DispPrintf(str,0,0,DBG_VAR_TYPE);
				p_temp = (unsigned char*)&can_sour.buf[7 + num*19 + 5 ];
				temp   = GET_WORD(p_temp);
				str.Format("Engine SpeedDataIdentifier: 0x%x",temp);
				DispPrintf(str,0,0,DBG_VAR_TYPE);
				p_temp = (unsigned char*)&can_sour.buf[7 + num*19 + 7 ];
				temp   = GET_WORD(p_temp);
				str.Format("Engine Speed: 0x%x",temp);
				DispPrintf(str,0,0,DBG_VAR_TYPE);
				p_temp = (unsigned char*)&can_sour.buf[7 + num*19 + 9 ];
				temp   = GET_WORD(p_temp);
				str.Format("Vehicle Speed DataIdentifier: 0x%x",temp);
				DispPrintf(str,0,0,DBG_VAR_TYPE);
				p_temp = (unsigned char*)&can_sour.buf[7 + num*19 + 11 ];
				temp   = GET_WORD(p_temp);
				str.Format("Vehicle Speed: 0x%x",temp);
				DispPrintf(str,0,0,DBG_VAR_TYPE);
				p_temp = (unsigned char*)&can_sour.buf[7 + num*19 + 13 ];
				temp   = GET_WORD(p_temp);
				str.Format("Vehicle Odometer DataIdentifier: 0x%x",temp);
				DispPrintf(str,0,0,DBG_VAR_TYPE);
				p_temp = (unsigned char*)&can_sour.buf[7 + num*19 + 15 ];
				temp   = GET_DWORD(p_temp);
				str.Format("Vehicle Odometer: 0x%x",temp);
				DispPrintf(str,0,0,DBG_VAR_TYPE);
			}
			break;
		case 0x06:
			DispPrintf("reportType: ",(unsigned char*)&can_sour.buf[2],1,DBG_VAR_TYPE);
			p_temp = (unsigned char*)&can_sour.buf[3];
			temp   = GET_MWORD(p_temp);
			DispPrintf("DTCAndStatusRecord: DTC ",(unsigned char*)&temp,3,DBG_VAR_TYPE);
			DispPrintf("DTCAndStatusRecord: status of DTC ",(unsigned char*)&can_sour.buf[6],1,DBG_VAR_TYPE);
			DispPrintf("DTCExtendedDataRecordNumber #1 ",(unsigned char*)&can_sour.buf[7],1,DBG_VAR_TYPE);
			DispPrintf("Fault occurrence counter   ",(unsigned char*)&can_sour.buf[8],1,DBG_VAR_TYPE);
			DispPrintf("DTCExtendedDataRecordNumber #2 ",(unsigned char*)&can_sour.buf[9],1,DBG_VAR_TYPE);
			DispPrintf("Fault occurrence counter   ",(unsigned char*)&can_sour.buf[10],1,DBG_VAR_TYPE);
			DispPrintf("DTCExtendedDataRecordNumber #3 ",(unsigned char*)&can_sour.buf[11],1,DBG_VAR_TYPE);
			DispPrintf("Fault occurrence counter   ",(unsigned char*)&can_sour.buf[12],1,DBG_VAR_TYPE);
			DispPrintf("DTCExtendedDataRecordNumber #4 ",(unsigned char*)&can_sour.buf[13],1,DBG_VAR_TYPE);
			DispPrintf("Fault occurrence counter   ",(unsigned char*)&can_sour.buf[14],1,DBG_VAR_TYPE);
			break;
		}
		break;
	case READ_DATA_BY_IDENTIFIER:
		/* 读数据流服务 -- read data by identifier */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: 读数据流服务...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		GetTboxReadInfo(can_sour);
		break;
	case SECURITY_ACCESS:
		/* 安全模式服务 -- security access */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: 安全模式服务...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		DispPrintf("========  not support SecurityAccess (27 hex)/安全访问 ========",0,0,DBG_VAR_TYPE);
		break;
	case COMMUNICATION_CONTROL:
		/* 通讯控制服务 -- communication control */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: 通讯控制服务...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		DispPrintf("CommunicationControl (28 hex) / 通信控制 :  ",(unsigned char*)&can_sour.buf[2],1,DBG_VAR_TYPE);
		break;
	case WRITE_DATA_BY_IDENTIFIER:
		/* 写数据服务 -- write data by identifier */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: 写数据服务...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		//p_temp = &can_sour.buf[2];
		//did    = GET_WORD(p_temp);

		//avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: 写数据服务...", __FUNCTION__, __LINE__);
		//p_dlg->ShowInfo(avmLogStr, 1);

		DispPrintf("========  not support WriteDataByIdentifier (2E hex)/ 通过标识符写数据 ========",0,0,DBG_VAR_TYPE);
		break;
	case INPUT_OUTPUT_CONTROL_BY_IDENTIFIER:
		/* 动作测试服务 -- input output control by identifier */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: 动作测试服务...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		DispPrintf("========  not support  InputOutputControlByIdentifier (2F hex)/通过标识符进行输入输出控制 ========",0,0,DBG_VAR_TYPE);
		break;
	case ROUTINE_CONTROL:
		/* 例程控制服务 -- routine control */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: 例程控制服务...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		DispPrintf("========  not support RoutineControl (31 hex)/ 例程控制 ========",0,0,DBG_VAR_TYPE);
		break;
	case TESTER_PRESENT:
		/* 待机握手服务 -- tester present */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: 待机握手服务...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

#if 0
		switch( can_sour.buf[2])
		{
		case 0x00:
			DispPrintf("TesterPresent (3E hex): zeroSubFunction (suppressPosRspMsgIndicationBit =0)",0,0,DBG_VAR_TYPE);
			break;
		case 0x80:
			DispPrintf("TesterPresent (3E hex): zeroSubFunction (suppressPosRspMsgIndicationBit =1)",0,0,DBG_VAR_TYPE);
			break;
		default:
			DispPrintf("error mode for  TesterPresent (3E hex) ",(unsigned char*)&can_sour.buf[2],1,DBG_VAR_TYPE);
			break;
		}
#endif
		break;
	case CONTROL_DTC_SETTING:
		/* 故障码存储控制服务 -- control DTC setting */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: 故障码存储控制服务...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		switch( can_sour.buf[2] )
		{
		case 0x01:
		case 0x02:
			DispPrintf("ControlDTCSetting (85 hex) /DTC: DTCSettingType = ",(unsigned char*)&can_sour.buf[2],1,DBG_VAR_TYPE);
			break;
		default:
			DispPrintf("error mode for  ControlDTCSetting (85 hex) /DTC  ",(unsigned char*)&can_sour.buf[2],1,DBG_VAR_TYPE);
			break;
		}
		break;
	default:
		/* 未知指令 */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: 未知指令...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		str.Format("foune unknow ack sid: %d",sid);
		DebugPrintf(C1_DBG,str,0,0,DBG_VAR_TYPE);
		return 0;
	}
	return 1;
}
