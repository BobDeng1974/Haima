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
	avmLogStr.Format("[AVM Debug Log | %s:%d] ����DID����...", __FUNCTION__, __LINE__);
	p_dlg->ShowInfo(avmLogStr, 1);
	while(index < total_len)
	{
		p_temp = &can_data.buf[index];
		did    = GET_WORD(p_temp);
		index += 2;

		switch( did )
		{
		case 0xF189:	// ECU����汾���
			memcpy(p_data, &can_data.buf[index], SIZE_F189);
			index += SIZE_F189;
			//str.Format("0xF189-��������汾���: %d",p_data);
			avmLogStr.Format("0xF189-ECU����汾���: %d .%d",(p_data[0]>>4)*10+(p_data[0] & 0x0F ), (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0 , sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;

		case 0xF186:	// ��ǰ�Ựģʽ
			memcpy(p_data, &can_data.buf[index], 1);
			index += 1;
			//str.Format("0xF189-��������汾���: %d",p_data);
			avmLogStr.Format("0xF186��ǰ�Ựģʽ: %d", p_data[0]);
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;


		case 0xF187:	// �����㲿����
			memcpy(p_data, &can_data.buf[index], 9);
			index += 9;
			//str.Format("0xF189-��������汾���: %d",p_data);
			avmLogStr.Format("0xF187-�����㲿����: %s", p_data);
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;

		case 0xF18A:	// ��Ӧ�̴���
			memcpy(p_data, &can_data.buf[index], 8);
			index += 8;
			//str.Format("0xF189-��������汾���: %d",p_data);
			avmLogStr.Format("0xF18A-��Ӧ�̴���: %s", p_data);
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;
	
		case 0xF188:	// ����������
			memcpy(p_data, &can_data.buf[index], 9);
			index += 9;
			avmLogStr.Format("0xF188-����������: %s", p_data);
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;



		case 0xF18B:	// ��������������
			memcpy(p_data, &can_data.buf[index], 4);
			index += 4;
			//str.Format("0xF189-��������汾���: %d",p_data);
			avmLogStr.Format("0xF18B-ECU��������������:  %d��-%d��-%d��", ((p_data[0] >> 4) * 10 + (p_data[0] & 0x0F))*100 + (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F),
				(p_data[2] >> 4) * 10 + (p_data[2] & 0x0F) , (p_data[3] >> 4) * 10 + (p_data[3] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;

		
		case 0xF18C:	// ���к�
			memcpy(p_data, &can_data.buf[index], 10);
			index += 10;
			//str.Format("0xF189-��������汾���: %d",p_data);
			avmLogStr.Format("0xF18C-ECU���к�: %x,%x, %x,%x, %x,%x, %x,%x, %x,%x", p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7], p_data[8], p_data[9]);
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;


		case 0xF18D:	// ֧�ֵĹ��ܵ�λ
			memcpy(p_data, &can_data.buf[index], SIZE_F189);
			index += SIZE_F189;
			//str.Format("0xF189-��������汾���: %d",p_data);
			avmLogStr.Format("0xF18D-ECU֧�ֵĹ��ܵ�λ: %x", p_data[0]);
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

		case 0xF197:	// ����������
			memcpy(p_data, &can_data.buf[index], 4);
			index += 4;
			//str.Format("0xF189-��������汾���: %d",p_data);
			avmLogStr.Format("0xF197-ECU����������: %s", p_data);
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;
	


		
		case 0xF19D:	// װ������
			memcpy(p_data, &can_data.buf[index], 4);
			index += 4;
			//str.Format("0xF187-Part Number: %s",p_data);

			avmLogStr.Format("0xF19D-ECUװ������:  %d��-%d��-%d��", ((p_data[0] >> 4) * 10 + (p_data[0] & 0x0F)) * 100 + (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F),
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
		
		case 0xF199:	// ����ˢд����
			memcpy(p_data, &can_data.buf[index], 4);
			index += 4;
			//str.Format("0xF187-Part Number: %s",p_data);

			avmLogStr.Format("0xF199-ECU����ˢд����: %d��-%d��-%d��", ((p_data[0] >> 4) * 10 + (p_data[0] & 0x0F)) * 100 + (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F),
				(p_data[2] >> 4) * 10 + (p_data[2] & 0x0F), (p_data[3] >> 4) * 10 + (p_data[3] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);
			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			break;

		case 0xF19B:	// �궨����
			memcpy(p_data, &can_data.buf[index], 4);
			index += 4;
			//str.Format("0xF187-Part Number: %s",p_data);

			avmLogStr.Format("0xF19B-ECU�궨����:  %d��-%d��-%d��", ((p_data[0] >> 4) * 10 + (p_data[0] & 0x0F)) * 100 + (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F),
				(p_data[2] >> 4) * 10 + (p_data[2] & 0x0F), (p_data[3] >> 4) * 10 + (p_data[3] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);
			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			break;



		case 0xF1B0:	// ��������ѹ
			memcpy(p_data, &can_data.buf[index], 2);
			index += 2;
			//str.Format("0xF18A-System Supplier Identifier: %s",p_data);

			avmLogStr.Format("0xF1B0-��������ѹ: %d.%d", (p_data[0] >> 4) * 10 + (p_data[0] & 0x0F), (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0 , sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;
		case 0xFD00:	// MCU_APP�汾��
			memcpy(p_data, &can_data.buf[index], 3);
			index += 3;
			//str.Format("0xF18A-System Supplier Identifier: %s",p_data);

			avmLogStr.Format("0xFD00-MCU_APP�汾��: %d.%d.%d", ((p_data[0] >> 4) * 10 + (p_data[0] & 0x0F)), (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F), (p_data[2] >> 4) * 10 + (p_data[2] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;
		

		case 0xFD01:	// MCU_BOOT�汾�� 
			memcpy(p_data, &can_data.buf[index], 3);
			index += 3;
			//str.Format("0xF18A-System Supplier Identifier: %s",p_data);

			avmLogStr.Format("0xFD01-MCU_BOOT�汾��: %d.%d.%d", (p_data[0] >> 4) * 10 + (p_data[0] & 0x0F), (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F), (p_data[2] >> 4) * 10 + (p_data[2] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;


		case 0xFD02:	// ARM_APP�汾�� 
			memcpy(p_data, &can_data.buf[index], 3);
			index += 3;
			//str.Format("0xF18A-System Supplier Identifier: %s",p_data);

			avmLogStr.Format("0xFD02-ARM_APP�汾��: %d.%d.%d", (p_data[0] >> 4) * 10 + (p_data[0] & 0x0F), (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F), (p_data[2] >> 4) * 10 + (p_data[2] & 0x0F));
			p_dlg->ShowInfo(avmLogStr, 1);

			memset(p_data, 0, sizeof(p_data));
			strBatch += str;
			strBatch += "; ";
			//DispPrintf(str,0,0,DBG_VAR_TYPE);
			break;


		case 0xFD03:	// ARM_SF�汾�� 
			memcpy(p_data, &can_data.buf[index], 3);
			index += 3;
			//str.Format("0xF18A-System Supplier Identifier: %s",p_data);

			avmLogStr.Format("0xFD03-ARM_SF�汾�� : %d.%d.%d", (p_data[0] >> 4) * 10 + (p_data[0] & 0x0F), (p_data[1] >> 4) * 10 + (p_data[1] & 0x0F), (p_data[2] >> 4) * 10 + (p_data[2] & 0x0F));
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

/* �����͸�PC��������ӦЭ�� */
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



	/* �����ȡ������Ӧ������ */
	switch( sid & (~0x40) )
	{
	case DIAGNOSITIC_SESSION_CONTROL:
		/* ��Ͻ��̿��Ʒ��� - diagnositic session control */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: ��Ͻ��̿��Ʒ���...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		DebugPrintf(NORM_DBG,"found diagnositic session control ack:",(unsigned char*)can_sour.buf,can_sour.len,DBG_DATA_TYPE);

		p_temp = &can_sour.buf[3];
		p2_can_ecu_max = GET_WORD(p_temp);
		p_temp = &can_sour.buf[5];
		p2__can_ecu_max = GET_WORD(p_temp);

		switch( can_sour.buf[2])
		{
		case 0x01:
			str.Format("DiagnosticSessionControl (10 hex):DefaultSession/Ĭ�ϻỰģʽ: P2 CAN_ECU_max = 0x%04x ; P2* CAN_ECU_max = 0x%04x",p2_can_ecu_max,p2__can_ecu_max);
			break;
		case 0x02:
			str.Format("DiagnosticSessionControl (10 hex):ProgrammingSession/��̻Ựģʽ: P2 CAN_ECU_max = 0x%04x ; P2* CAN_ECU_max = 0x%04x",p2_can_ecu_max,p2__can_ecu_max);
			break;
		case 0x03:
			str.Format("DiagnosticSessionControl (10 hex):ExtendedDiagnosticSession/��չ��ϻỰģʽ: P2 CAN_ECU_max = 0x%04x ; P2* CAN_ECU_max = 0x%04x",p2_can_ecu_max,p2__can_ecu_max);
			break;
		default:
			str.Format("error mode for  DiagnosticSessionControl (10 hex): 0x%x",can_sour.buf[2]);
			break;
		}
		DispPrintf(str,0,0,DBG_VAR_TYPE);
		break;
	case ECU_RESET:
		/* ECU��λ���� -- ECU reset */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: ECU��λ����...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);
		switch( can_sour.buf[2])
		{
		case 0x01:
			DispPrintf("ECUReset (11 hex) /ECU: hardReset/ Ӳ��λ �ɹ�",0,0,DBG_VAR_TYPE);
			break;
		case 0x02:
			DispPrintf("ECUReset (11 hex) /ECU: keyOffOnReset/ Կ��OFF ON  �ɹ�",0,0,DBG_VAR_TYPE);
			break;
		default:
			DispPrintf("error mode for ECUReset (11 hex) /ECU ",(unsigned char*)&can_sour.buf[2],1,DBG_VAR_TYPE);
			break;
		}
		break;
	case CLEAR_DIAGNOSTIC_INFORMATION:			
		/* ������������ -- clear diagnostic information */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: ������������...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);
		DispPrintf("ClearDiagnosticInformation (14 hex)/ ���������Ϣ: success",0,0,DBG_VAR_TYPE);
		break;
	case READ_DTC_INFORMATION:					
		/* ����������� -- read DTC information */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: �����������...", __FUNCTION__, __LINE__);
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
		/* ������������ -- read data by identifier */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: ������������...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		GetTboxReadInfo(can_sour);
		break;
	case SECURITY_ACCESS:
		/* ��ȫģʽ���� -- security access */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: ��ȫģʽ����...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		DispPrintf("========  not support SecurityAccess (27 hex)/��ȫ���� ========",0,0,DBG_VAR_TYPE);
		break;
	case COMMUNICATION_CONTROL:
		/* ͨѶ���Ʒ��� -- communication control */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: ͨѶ���Ʒ���...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		DispPrintf("CommunicationControl (28 hex) / ͨ�ſ��� :  ",(unsigned char*)&can_sour.buf[2],1,DBG_VAR_TYPE);
		break;
	case WRITE_DATA_BY_IDENTIFIER:
		/* д���ݷ��� -- write data by identifier */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: д���ݷ���...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		//p_temp = &can_sour.buf[2];
		//did    = GET_WORD(p_temp);

		//avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: д���ݷ���...", __FUNCTION__, __LINE__);
		//p_dlg->ShowInfo(avmLogStr, 1);

		DispPrintf("========  not support WriteDataByIdentifier (2E hex)/ ͨ����ʶ��д���� ========",0,0,DBG_VAR_TYPE);
		break;
	case INPUT_OUTPUT_CONTROL_BY_IDENTIFIER:
		/* �������Է��� -- input output control by identifier */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: �������Է���...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		DispPrintf("========  not support  InputOutputControlByIdentifier (2F hex)/ͨ����ʶ����������������� ========",0,0,DBG_VAR_TYPE);
		break;
	case ROUTINE_CONTROL:
		/* ���̿��Ʒ��� -- routine control */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: ���̿��Ʒ���...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		DispPrintf("========  not support RoutineControl (31 hex)/ ���̿��� ========",0,0,DBG_VAR_TYPE);
		break;
	case TESTER_PRESENT:
		/* �������ַ��� -- tester present */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: �������ַ���...", __FUNCTION__, __LINE__);
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
		/* ������洢���Ʒ��� -- control DTC setting */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: ������洢���Ʒ���...", __FUNCTION__, __LINE__);
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
		/* δָ֪�� */
		avmLogStr.Format("[AVM Debug Log | %s:%d] SID case: δָ֪��...", __FUNCTION__, __LINE__);
		p_dlg->ShowInfo(avmLogStr, 1);

		str.Format("foune unknow ack sid: %d",sid);
		DebugPrintf(C1_DBG,str,0,0,DBG_VAR_TYPE);
		return 0;
	}
	return 1;
}
