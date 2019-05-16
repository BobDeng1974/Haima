#include "stdafx.h"
#include "SGMW_diag.h"
#include "SGMW_diagDlg.h"
#include "afxdialogex.h"
#include "diag.h"

/* asc字符转换成hex变量 */
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
/* 发送错误应答帧 */
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

/* 诊断错误，打印错误信息 */
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

	/* 触发写CAN数据流程 */
	CAN_OBJ		can_buf  = {0};
	WriteInfoToTbox(can_buf);

	return 0;
}

/* CAN数据接收处理程序 */
int CSGMW_diagDlg::diag_receive_proc(CAN_OBJ can_sour)
{
	unsigned char		sid  = 0;
	unsigned char		len  = 0;
	unsigned char		sub  = 0;
	int					temp = 0;
	int					i    = 0;
	CString				str  = "";


	if( can_sour.id == pc_id )
	{	/* 接收到TBOX数据 */
		len = can_sour.buf[0];		/* 数据长度 */
		sid = can_sour.buf[1];		/* 命令字 */
		sub = can_sour.buf[2];		/* 子命令 */

		//判断是否进入升级或传输流程
		if(handle_flag)
		{
			if(handle_type == UPDATE_TYPE)
			{
				/* 处理向TBOX升级流程 */
				if( UpdateToTbox(can_sour) )
					return 1;
			}

			if(handle_type == TRANSFER_TYPE)
			{
				/* 处理从TBOX传输流程 */
				if( TransferFromTbox(can_sour) )
					return 1;
			}
		}
		else
		{
			/* 处理向TBOX写入数据流程 */
			if( WriteInfoToTbox(can_sour) )
				return 1;

			if( sid & 0x40 )
			{	/* 接收到应答数据 */
				CString avmLogStr;
				//avmLogStr.Format("[AVM Debug Log | %s:%d] 接收到应答数据...", __FUNCTION__, __LINE__);
				//p_dlg->ShowInfo(avmLogStr, 1);

				diag_ack_data(can_sour,sid);
			}
			else
			{	/* 接收到请求数据,目前未做任何处理 */
				diag_request_cmd(can_sour,sid,sub);
			}
		}
	}
	else
	{	/* 发现未知can id */
		DebugPrintf(C2_DBG,"found unknow id: ",(unsigned char*)&can_sour.id,2,DBG_VAR_TYPE);
	}
	return 0;
}

