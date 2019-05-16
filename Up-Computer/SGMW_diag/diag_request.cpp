#include "stdafx.h"
#include "SGMW_diag.h"
#include "SGMW_diagDlg.h"
#include "afxdialogex.h"
#include "diag.h"

/* �����͸�PC������Э�� */
int CSGMW_diagDlg::diag_request_cmd(CAN_OBJ can_sour,int sid,int sub)
{
	CString	str;

	switch( sid )
	{
	case DIAGNOSITIC_SESSION_CONTROL:
		/* ��Ͻ��̿��Ʒ��� - diagnositic session control */
		SendDiagErrFrame(can_sour,sid,0x22);
		str.Format("diagnositic session control: ");
		DebugPrintf(C3_DBG,str,can_sour.buf,can_sour.len,DBG_DATA_TYPE);
		break;
	case ECU_RESET:
		/* ECU��λ���� -- ECU reset */
		SendDiagErrFrame(can_sour,sid,0x22);
		str.Format("ecu reset: ");
		DebugPrintf(C2_DBG,str,can_sour.buf,can_sour.len,DBG_DATA_TYPE);
		break;
	case CLEAR_DIAGNOSTIC_INFORMATION:			
		/* ������������ -- clear diagnostic information */
		SendDiagErrFrame(can_sour,sid,0x22);
		str.Format("clear diagnostic information");
		DebugPrintf(C2_DBG,str,0,0,DBG_DATA_TYPE);
		break;
	case READ_DTC_INFORMATION:					
		/* ����������� -- read DTC information */
		SendDiagErrFrame(can_sour,sid,0x22);
		str.Format("read dtc informaton");
		DebugPrintf(C2_DBG,str,0,0,0);
		break;
	case READ_DATA_BY_IDENTIFIER:
		/* ������������ -- read data by identifier */
		SendDiagErrFrame(can_sour,sid,0x22);
		str.Format("read data by idenifler");
		DebugPrintf(C1_DBG,str,0,0,DBG_DATA_TYPE);
		break;
	case SECURITY_ACCESS:
		/* ��ȫģʽ���� -- security access */
		SendDiagErrFrame(can_sour,sid,0x22);
		str.Format("security access");
		DebugPrintf(C2_DBG,str,0,0,DBG_DATA_TYPE);
		break;
	case COMMUNICATION_CONTROL:
		/* ͨѶ���Ʒ��� -- communication control */
		SendDiagErrFrame(can_sour,sid,0x22);
		str.Format("communication control");
		DebugPrintf(C2_DBG,str,0,0,DBG_DATA_TYPE);
		break;
	case WRITE_DATA_BY_IDENTIFIER:
		/* д���ݷ��� -- write data by identifier */
		SendDiagErrFrame(can_sour,sid,0x22);
		str.Format("write data by identifier");
		DebugPrintf(C2_DBG,str,0,0,DBG_DATA_TYPE);
		break;
	case INPUT_OUTPUT_CONTROL_BY_IDENTIFIER:
		/* �������Է��� -- input output control by identifier */
		SendDiagErrFrame(can_sour,sid,0x22);
		str.Format("input output control by identifier");
		DebugPrintf(C2_DBG,str,0,0,DBG_DATA_TYPE);
		break;
	case ROUTINE_CONTROL:
		/* ���̿��Ʒ��� -- routine control */
		SendDiagErrFrame(can_sour,sid,0x22);
		str.Format("routine control");
		DebugPrintf(C2_DBG,str,0,0,DBG_DATA_TYPE);
		break;
	case TESTER_PRESENT:
		/* �������ַ��� -- tester present */
		can_sour.buf[0] = 0x02;
		can_sour.buf[1] = 0x7e;
		can_sour.buf[2] = 0x00;
		can_sour.buf[3] = 0x00;
		can_sour.buf[4] = 0x00;
		can_sour.buf[5] = 0x00;
		can_sour.buf[6] = 0x00;
		can_sour.buf[7] = 0x00;
		can_sour.len    = 8;
		can_sour.id		= can_driver.GetCanTxRxId(0,can_sour.id);
		can_driver.TransmitData(can_sour.id,can_sour.buf,can_sour.len);
		str.Format("test present");
		DebugPrintf(C2_DBG,str,can_sour.buf,can_sour.len,DBG_DATA_TYPE);
		break;
	case CONTROL_DTC_SETTING:					/* control DTC setting */
		SendDiagErrFrame(can_sour,sid,0x22);
		str.Format("control dtc setting");
		DebugPrintf(C2_DBG,str,0,0,DBG_VAR_TYPE);
		break;
	case ERR_DTC_ID:
		str.Format("found error ack: 0x%04x - ",can_sour.id);
		DispPrintf(str,(unsigned char*)can_sour.buf,can_sour.len,DBG_DATA_TYPE);
		break;
	default:
		SendDiagErrFrame(can_sour,sid,0x22);
		str.Format("foune unknow sid: %d",sid);
		DebugPrintf(C1_DBG,str,0,0,DBG_VAR_TYPE);
		return 0;
	}
	return 1;
}

