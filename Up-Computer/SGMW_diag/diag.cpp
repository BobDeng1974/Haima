#include "stdafx.h"
#include "SGMW_diag.h"
#include "SGMW_diagDlg.h"
#include "afxdialogex.h"
#include "diag.h"

/* asc�ַ�ת����hex���� */
unsigned char CSGMW_diagDlg::asc_to_hex(unsigned char data)
{
	unsigned int temp1 = 0;
	unsigned int temp2 = 0;
	unsigned int var   = 0;

	if( data >= '0' && data <= '9')
		var = data - '0';
	else if( data >= 'A' && data <= 'F')
		var = data - 'A';
	else if( data >= 'a' && data <= 'f' )
		var = data - 'a';
	else
		return data;
	return var;
}
/* ���ʹ���Ӧ��֡ */
int CSGMW_diagDlg::SendDiagErrFrame(CAN_OBJ can_sour, unsigned char sid, unsigned char err)
{
	can_sour.buf[0] = 0x03;
	can_sour.buf[1] = 0x7f;
	can_sour.buf[2] = sid;
	can_sour.buf[3] = err;
	can_sour.buf[4] = 0;
	can_sour.buf[5] = 0;
	can_sour.buf[6] = 0;
	can_sour.buf[7] = 0;
	can_sour.id  = can_driver.GetCanTxRxId(0,can_sour.id);;
	can_sour.len = 8;
	can_driver.TransmitData(can_sour.id,can_sour.buf,can_sour.len);
	return 0;
}

/* ��ϴ��󣬴�ӡ������Ϣ */
int CSGMW_diagDlg::WriteToTboxErr(CString str)
{
	diag_wait_time		= 0;
	diag_write_step		= WRITE_TO_TBOX_NULL;

	if(handle_flag)
	{
		diag_update_step = UPDATE_TO_TBOX_NULL;
		diag_transfer_step = TRANS_FROM_TBOX_NULL;
		handle_flag = 0;
	}

	DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
	return 0;
}

int CSGMW_diagDlg::WriteToTboxStart(void)
{
	diag_wait_time	= GetTickCount();
	diag_write_step = WRITE_TO_TBOX_EXTERN_MODE;
	DebugPrintf(NORM_DBG,"start write data to avm",0,0,DBG_VAR_TYPE);

	/* ����дCAN�������� */
	CAN_OBJ		can_buf  = {0};
	WriteInfoToTbox(can_buf);

	return 0;
}

/* CAN���ݽ��մ������ */
int CSGMW_diagDlg::diag_receive_proc(CAN_OBJ can_sour)
{
	unsigned char		sid  = 0;
	unsigned char		len  = 0;
	unsigned char		sub  = 0;
	int					temp = 0;
	int					i    = 0;
	CString				str  = "";


	if( can_sour.id == pc_id )
	{	/* ���յ�TBOX���� */
		len = can_sour.buf[0];		/* ���ݳ��� */
		sid = can_sour.buf[1];		/* ������ */
		sub = can_sour.buf[2];		/* ������ */

		//�ж��Ƿ����������������
		if(handle_flag)
		{
			if(handle_type == UPDATE_TYPE)
			{
				/* ������TBOX�������� */
				if( UpdateToTbox(can_sour) )
					return 1;
			}

			if(handle_type == TRANSFER_TYPE)
			{
				/* �����TBOX�������� */
				if( TransferFromTbox(can_sour) )
					return 1;
			}
		}
		else
		{
			/* ������TBOXд���������� */
			if( WriteInfoToTbox(can_sour) )
				return 1;

			if( sid & 0x40 )
			{	/* ���յ�Ӧ������ */
				CString avmLogStr;
				//avmLogStr.Format("[AVM Debug Log | %s:%d] ���յ�Ӧ������...", __FUNCTION__, __LINE__);
				//p_dlg->ShowInfo(avmLogStr, 1);

				diag_ack_data(can_sour,sid);
			}
			else
			{	/* ���յ���������,Ŀǰδ���κδ��� */
				diag_request_cmd(can_sour,sid,sub);
			}
		}
	}
	else
	{	/* ����δ֪can id */
		DebugPrintf(C2_DBG,"found unknow id: ",(unsigned char*)&can_sour.id,2,DBG_VAR_TYPE);
	}
	return 0;
}

