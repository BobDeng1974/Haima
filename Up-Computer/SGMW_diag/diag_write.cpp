#include "stdafx.h"
#include "SGMW_diag.h"
#include "SGMW_diagDlg.h"
#include "afxdialogex.h"
#include "diag.h"

#include "winTest.h"

#define TIMEOUT_WAIT_TIME	5000//1000


/* ��TBOXд���������̣���Ҫ��ȫ��֤ */
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
		/* �޲��� */
		break;
	case WRITE_TO_TBOX_EXTERN_MODE:
		/* ���ͽ�����չģʽָ�� */
		can_data.id     = avm_id;
		can_data.len    = 8;
		can_data.buf[0] = 0x02;
		can_data.buf[1] = 0x10;
		//can_data.buf[2] = 0x03;		//��չģʽ0x03	���ģʽ0x02
		can_data.buf[2] = mod_key;
		can_data.buf[3] = 0x00;
		can_data.buf[4] = 0x00;
		can_data.buf[5] = 0x00;
		can_data.buf[6] = 0x00;
		can_data.buf[7] = 0x00;

		can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
		//DebugPrintf(C2_DBG,"���ͽ�����չģʽָ��",0,0,DBG_VAR_TYPE);
		if(!flag)
			DebugPrintf(C2_DBG,"���ͽ�����չģʽָ��",0,0,DBG_VAR_TYPE);
		else
			DebugPrintf(C2_DBG,"���ͽ�����ģʽָ��",0,0,DBG_VAR_TYPE);

		res				= 1;
		diag_wait_time	= GetTickCount();
		diag_write_step = WRITE_TO_TBOX_WAIT_EXTERN_MODE;
		break;

	case WRITE_TO_TBOX_WAIT_EXTERN_MODE:
		/* �ȴ�������չģʽӦ�𣬻�ȡ��Ӧ��󣬷��Ͷ�TBOX���к�ָ�� */
		if( (GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			//WriteToTboxErr("������չģʽָ��δ��Ӧ��д������ʧ��!");
			if(!flag)
				WriteToTboxErr("������չģʽָ��δ��Ӧ��д������ʧ��!");
			else
				WriteToTboxErr("������ģʽָ��δ��Ӧ��д������ʧ��!");
		}
		else
		{	/* ���յ����� */
			//if( can_sour.buf[1] == 0x50 && can_sour.buf[2] == 0x03 )
			if( can_sour.buf[1] == 0x50 && can_sour.buf[2] == mod_key )
			{	/* ���Ͷ����к�ָ�� */
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
				DebugPrintf(C2_DBG,"����������Ϣ",0,0,DBG_VAR_TYPE);
			}
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == mod_key)
			{
				res = 1;
				WriteToTboxErr("������չģʽָ���Ӧ��д������ʧ��!");
			}
		}
		break;


	case WRITE_TO_TBOX_WAIT_READ_SN:
		if ((GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME)
		{	/* �ȴ���ʱ */
			WriteToTboxErr("��SN�ų�ʱ��д������ʧ��!");
		}
		else {
			/* ���յ����� */
			if (can_sour.buf[1] == 0x62 && can_sour.buf[2] == 0xF1) {
				DebugPrintf(NORM_DBG, "��SN�ųɹ�", 0, 0, DBG_VAR_TYPE);
				p_temp = &can_sour.buf[4];
				SN = GET_DWORD(p_temp);
				/* ���յ�Ӧ��,���Ͷ����кŲ��� */
				/* ������������ */
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
				DebugPrintf(C2_DBG, "����������Ϣ", 0, 0, DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x22) {
				str.Format("��SN��ʧ�ܸ���Ӧ");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
			}

		}
		break;




	case WRITE_TO_TBOX_WAIT_READ_SEED:
		/* �ȴ���ȡ��������Ϣ,���ҷ�����Կ */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			WriteToTboxErr("δ�ܻ�ȡ��AVM�������ӣ�д������ʧ��!");
		}
		else
		{	/* ����TBOX�ϱ������� */
			//if( can_sour.buf[1] == 0x67 && can_sour.buf[2] == 0x01 )
			if( can_sour.buf[1] == 0x67 && can_sour.buf[2] == req_key )
			{	/* ���յ�Ӧ��,������Կ */
				/* ������������ */
				p_temp = &can_sour.buf[3];
				diag_seed = GET_DWORD(p_temp);
				DebugPrintf(C2_DBG,"found avm seed: 0x",(unsigned char*)&diag_seed,4,DBG_VAR_TYPE);

				if( diag_seed == 0  )
				{   /* ��Կ�Ѿ�������ֱ��д������ */
					DebugPrintf(C2_DBG,"��Կ�ѽ�����ֱ��д������",0,0,DBG_VAR_TYPE);
					can_driver.TransmitData(diag_write.id,diag_write.buf,diag_write.len);

					res				= 1;
					diag_wait_time	= GetTickCount();
					diag_write_step = WRITE_TO_TBOX_WAIT_WRITE_DATA;
					break;
				}
				/* ����password */
				//diag_pas = safe.uds_calc_key(diag_seed,1);
				//if((diag_write.buf[2] == 0xF1) && (diag_write.buf[3] == 0x84))
				if(flag)
					diag_pas = safe.uds_calc_key(diag_seed,3,SN);
				else
					diag_pas = safe.uds_calc_key(diag_seed,2,SN);
					
				/* ������Կ */
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
				DebugPrintf(C2_DBG,"������Կ",0,0,DBG_VAR_TYPE);
			}
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == 0x27 )
			{
				res = 1;
				WriteToTboxErr("�޷���ȡAVM������Ϣ��д������ʧ��!");
				str.Format("get tbox seed failed: %d",can_data.buf[3]);
				DebugPrintf(C2_DBG,str,0,0,DBG_VAR_TYPE);
			}
		}
		break;
	case WRITE_TO_TBOX_WAIT_READ_PASSWORD:
		/* �ȴ���Կ����Ӧ�𣬿�ʼ��TBOXд������ */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			WriteToTboxErr("δ�ܻ�ȡ��AVM��Կ��Ϣ��д������ʧ��!");
		}
		else
		{	/* ���յ����� */
			//if( can_sour.buf[1] == 0x67 && can_sour.buf[2] == 0x02 )
			if( can_sour.buf[1] == 0x67 && can_sour.buf[2] == (req_key+1) )
			{	/* ���յ�Ӧ��,����д����Ϣ��TBOX */
				can_driver.TransmitData(diag_write.id,diag_write.buf,diag_write.len);

				res				= 1;
				diag_wait_time	= GetTickCount();
				diag_write_step = WRITE_TO_TBOX_WAIT_WRITE_DATA;
				DebugPrintf(C2_DBG,"��Կ��֤�ɹ�����ʼд������",0,0,DBG_VAR_TYPE);
			}
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == 0x27 )
			{
				res = 1;
				WriteToTboxErr("��Կ����д������ʧ��!");

				str.Format("tbox password check failed: %d",can_sour.buf[3]);
				DebugPrintf(C1_DBG,str,0,0,DBG_VAR_TYPE);
			}
		}
		break;
	case WRITE_TO_TBOX_WAIT_WRITE_DATA:
		/* �ȴ���TBOXд�����ݵ�Ӧ�� */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			WriteToTboxErr("д��AVM���ݳ�ʱ!");
		}
		else
		{	/* ���յ����� */
			if( can_sour.buf[1] == 0x6e && can_sour.buf[2] ==  diag_write.buf[2] && can_sour.buf[3] == diag_write.buf[3])
			{
				res = 1;
				WriteToTboxErr("����д��ɹ�");
			} 
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == 0x2e )
			{
				res = 1;
				WriteToTboxErr("д��AVM������֤����");
				str.Format("write to avm data failed: %d",can_sour.buf[3]);
				DebugPrintf(C1_DBG,str,0,0,DBG_VAR_TYPE);
			}
		}
		break;
	default:
		WriteToTboxErr("���ִ����д�벽��");
		break;
	}
	return res;
}

	

/* �����������̣���TBOXд�����ݣ���Ҫ��ȫ��֤ */
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

	//��ӡ
	if(can_sour.len)
	{
		CString strTime = p_dlg->comm.GetTime();
		str.Format("��ӡ -- [%s] --> recv:", strTime);
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
		/* �޲��� ֹͣ����*/
		OnBnClickedButtonStop();
		break;
	case UPDATA_TO_AVM_ENTER_EXTMODE:   //����ui�̵߳���
		can_data.id = avm_id; 
		can_data.len = 8;
		memset(can_data.buf, 0x00, 8);
		can_data.buf[0] = 0x02;
		can_data.buf[1] = 0x10;
		can_data.buf[2] = 0x03;
		can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);
		//diag_update_step = UPDATA_TO_AVM_FUN_NEGATIVE_RESPONSE;
		DebugPrintf(NORM_DBG, "���ͽ�����չģʽָ��", 0, 0, DBG_VAR_TYPE);
		diag_update_step = UPDATA_TO_AVM_STOP_SET_DTC;
		diag_wait_time = GetTickCount();
		
	break;
	case UPDATA_TO_AVM_STOP_SET_DTC:
		if ((GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME)
		{	/* �ȴ���ʱ */
			WriteToTboxErr("������չģʽָ��δ��Ӧ��д������ʧ��!");
			OnBnClickedButtonStop();
		}
		else {
			/* ���յ����� */
			if (can_sour.buf[1] == 0x50 && can_sour.buf[2] == 0x03) {
				DebugPrintf(NORM_DBG, "������չģʽ�ɹ�", 0, 0, DBG_VAR_TYPE);

				/* ֹͣDTC */
				can_data.id = avm_id;
				can_data.len = 8;
				memset(can_data.buf, 0x00, 8);
				can_data.buf[0] = 0x02;
				can_data.buf[1] = 0x85;
				can_data.buf[2] = 0x02;
				can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);

				diag_wait_time = GetTickCount();
				diag_update_step = UPDATE_TO_AVM_COMMUNICATE_CONTROL;
				DebugPrintf(NORM_DBG, "ֹͣ�趨DTC", 0, 0, DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x10) {
				str.Format("������չģʽʧ�ܣ�");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();
			}
		
		}
		break;
		
	case UPDATE_TO_AVM_COMMUNICATE_CONTROL:
		if ((GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME)
		{	/* �ȴ���ʱ */
			WriteToTboxErr("ֹͣDTCָ��δ��Ӧ��д������ʧ��!");
			OnBnClickedButtonStop();
		}
		else {
			/* ���յ����� */
			if (can_sour.buf[1] == 0xc5 && can_sour.buf[2] == 0x02) {
				DebugPrintf(NORM_DBG, "ֹͣDTC���óɹ�", 0, 0, DBG_VAR_TYPE);

				/* ����ͨ�ſ���ָ�� */
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
				DebugPrintf(NORM_DBG, "���ͽ�ֹͨ�ſ���", 0, 0, DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x85) {
				str.Format("ֹͣDTC����ʧ�ܣ�");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();
			}

		}
		break;

	case UPDATE_TO_AVM_READ_SN:
		if ((GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME)
		{	/* �ȴ���ʱ */
			WriteToTboxErr("���ͽ�ֹͨ�ſ��Ƴ�ʱ��д������ʧ��!");
			OnBnClickedButtonStop();
		}
		else {
			/* ���յ����� */
			if (can_sour.buf[1] == 0x68 && can_sour.buf[2] == 0x01) {
				DebugPrintf(NORM_DBG, "���ͽ�ֹͨ�ſ��Ƴɹ�", 0, 0, DBG_VAR_TYPE);

				/* ���Ͷ����к�ָ�� */
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
				DebugPrintf(NORM_DBG, "���Ͷ����к�ָ��", 0, 0, DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x28) {
				str.Format("���ͽ�ֹͨ�ſ���ʧ�ܣ�");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();
			}

		}
		break;
	case UPDATE_TO_AVM_PROGRAM_MODE:
		if ((GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME)
		{	/* �ȴ���ʱ */
			WriteToTboxErr("���Ͷ����к�ָ�ʱ��д������ʧ��!");
			OnBnClickedButtonStop();
		}
		else {
			/* ���յ����� */
			if (can_sour.buf[2] == 0x62 && can_sour.buf[3] == 0xF1 && can_sour.buf[4] == 0x8C) {
				DebugPrintf(NORM_DBG, "�����кųɹ�", 0, 0, DBG_VAR_TYPE);
				p_temp = &can_sour.buf[11];
			    sN = GET_DWORD(p_temp);
				/* ���ͽ�����ģʽָ�� */
				can_data.id = avm_id;
				can_data.len = 8;
				memset(can_data.buf, 0x00, 8);
				can_data.buf[0] = 0x02;
				can_data.buf[1] = 0x10;
				can_data.buf[2] = 0x02;
				can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);

				diag_wait_time = GetTickCount();
				diag_update_step = UPDATE_TO_AVM_WAIT_PROGRAM_MODE;
				DebugPrintf(NORM_DBG, "���ͽ�����ģʽָ��", 0, 0, DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x22) {
				str.Format("���Ͷ����к�ָ��ʧ�ܣ�");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();
			}

		}

		break;

	case UPDATE_TO_AVM_WAIT_PROGRAM_MODE:
		/* �ȴ�������ģʽӦ�𣬻�ȡ��Ӧ��󣬷��Ͷ�TBOX���к�ָ�� */
		if( (GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			WriteToTboxErr("���ͽ�����ģʽָ�ʱ!");
			OnBnClickedButtonStop();
		}
		else
		{	/* ���յ����� */
			if( can_sour.buf[1] == 0x50 && can_sour.buf[2] == 0x02)
			{	/* ���յ�Ӧ��, */
				DebugPrintf(NORM_DBG,"���ͽ�����ģʽָ��ɹ�",0,0,DBG_VAR_TYPE);
				Sleep(1500);//�ȴ���λ����ת
				/* ������������ */
				can_data.id  = avm_id;
				can_data.len = 8;
				memset(can_data.buf, 0x00, 8);
				can_data.buf[0] = 0x02;
				can_data.buf[1] = 0x27;
				can_data.buf[2] = 0x03;
				can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);

				diag_wait_time	= GetTickCount();
				diag_update_step = UPDATE_TO_AVM_WAIT_READ_SEED;
				DebugPrintf(NORM_DBG,"����������Ϣ",0,0,DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x10) {
				if (can_sour.buf[3] == 0x78) {
					DebugPrintf(NORM_DBG, "NRC-78 �ȴ�...", 0, 0, DBG_VAR_TYPE);
					break;
				}

				str.Format("���ͽ�����ģʽָ��ʧ�ܣ�");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();

			}

		}
		break;
	case UPDATE_TO_AVM_WAIT_READ_SEED:
		/* �ȴ���ȡ��������Ϣ,���ҷ�����Կ */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			WriteToTboxErr("δ�ܻ�ȡ��AVM�������ӣ�д������ʧ��!");
			OnBnClickedButtonStop();
		}
		else
		{	/* ����TBOX�ϱ������� */
			if( can_sour.buf[1] == 0x67 && can_sour.buf[2] == 0x03 )
			{	/* ���յ�Ӧ��,������Կ */
				/* ������������ */

				p_temp = &can_sour.buf[3];
				diag_seed = GET_DWORD(p_temp);

				DebugPrintf(NORM_DBG,"found tbox seed: 0x",(unsigned char*)&diag_seed,4,DBG_VAR_TYPE);

				if( diag_seed == 0  )
				{   /* ��Կ�Ѿ����� */
					DebugPrintf(NORM_DBG,"��Կ�ѽ�����ֱ��д������",0,0,DBG_VAR_TYPE);

					if((m_codetype == 0) || (m_codetype == 6))  // MCU 
					{
						/* �ڴ���� */
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
						DebugPrintf(NORM_DBG, "�ڴ����..", 0, 0, DBG_VAR_TYPE);
						diag_wait_time = GetTickCount();
						diag_update_step = UPDATE_TO_AVM_EARSE_MEMORY;
					}
					else {
						/* ������������ ARM������дָ�� ����� */
						DebugPrintf(NORM_DBG, "��������", 0, 0, DBG_VAR_TYPE);
						/* �����ļ����� */
						can_data.id = avm_id;
						can_data.len = 12 + 1;// ���ͳ���+ һ���ֽڵĳ��ȣ�256> 2���ֽڣ�
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
						str.Format("�����ļ�����");
						DebugPrintf(NORM_DBG, str, 0, 0, DBG_VAR_TYPE);
						str.Format("update data type�� %02x", can_data.buf[12]);
						DebugPrintf(NORM_DBG, str, 0, 0, DBG_VAR_TYPE);

						diag_wait_time = GetTickCount();
						diag_update_step = UPDATE_TO_AVM_REQUEST_DOWNLOAD;

					}
					
				}
				/* ����password */
				diag_pas = safe.uds_calc_key(diag_seed,2,sN);
				DebugPrintf(NORM_DBG,"calculate key: 0x",(unsigned char*)&diag_pas,4,DBG_VAR_TYPE);
					
				/* ������Կ */
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
				DebugPrintf(NORM_DBG,"������Կ",0,0,DBG_VAR_TYPE);
			}
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == 0x27 )
			{
				if (can_sour.buf[3] == 0x78) {
					DebugPrintf(NORM_DBG, "NRC-78 �ȴ�...", 0, 0, DBG_VAR_TYPE);
					break;
				}
				WriteToTboxErr("�޷���ȡTBOX������Ϣ��д������ʧ��!");
				str.Format("get tbox seed failed: %d", can_sour.buf[3]);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				OnBnClickedButtonStop();
			}
		}
		break;



	case UPDATE_TO_AVM_WAIT_READ_PASSWORD:
		/* �ȴ���Կ����Ӧ�𣬿�ʼ��TBOXд������ */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			WriteToTboxErr("δ�ܻ�ȡ��TBOX��Կ��Ϣ��д������ʧ��!");
			OnBnClickedButtonStop();
		}
		else
		{	/* ���յ����� */	
#ifdef WIN_TEST
			if(1)
#else
			if( can_sour.buf[1] == 0x67 && can_sour.buf[2] == 0x04 )
#endif
			{	
				if ((m_codetype == 0) || (m_codetype == 6))   // MCU 
				{
					/* ���յ���Կ��֤Ӧ�𣬷��Ͳ���ָ�� */
					DebugPrintf(NORM_DBG, "��Կ��֤�ɹ������Ͳ���ָ��", 0, 0, DBG_VAR_TYPE);

					/* �ڴ���� */
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
					DebugPrintf(NORM_DBG, "�ڴ����..", 0, 0, DBG_VAR_TYPE);
					diag_wait_time = GetTickCount();
					diag_update_step = UPDATE_TO_AVM_EARSE_MEMORY;
				}
				else {
					/* ���յ���Կ��֤Ӧ�𣬽����������� ARM�����ò��� */
					DebugPrintf(NORM_DBG, "��Կ��֤�ɹ�����������", 0, 0, DBG_VAR_TYPE);
					/* �����ļ����� */
					can_data.id = avm_id;
					can_data.len = 12 + 1;// ���ͳ���+ һ���ֽڵĳ��ȣ�256> 2���ֽڣ�
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
					str.Format("�����ļ�����");
					DebugPrintf(NORM_DBG, str, 0, 0, DBG_VAR_TYPE);
					str.Format("update data type�� %02x", can_data.buf[12]);
					DebugPrintf(NORM_DBG, str, 0, 0, DBG_VAR_TYPE);

					diag_wait_time = GetTickCount();
					diag_update_step = UPDATE_TO_AVM_REQUEST_DOWNLOAD;
				
				}
				



			}
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == 0x27 )
			{

				WriteToTboxErr("��Կ����д������ʧ��!");

				str.Format("tbox password check failed: %d",can_sour.buf[3]);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				OnBnClickedButtonStop();
			}
		}
		break;


	case UPDATE_TO_AVM_EARSE_MEMORY:
		/* �ȴ��ڴ����Ӧ�� */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
		
			WriteToTboxErr("δ�ܻ�ȡ���ڴ������Ӧ��д������ʧ��!");
			
			OnBnClickedButtonStop();
		}
		else
		{
			/* ���յ����� */
			if(( can_sour.buf[1] == 0x71 ) && (can_sour.buf[2] ==  0x01))
			{	/* ���յ�Ӧ��*/
				DebugPrintf(NORM_DBG,"�ڴ�����ɹ�",0,0,DBG_VAR_TYPE);
			

				/* �����ļ����� */	
				can_data.id     = avm_id;
				can_data.len    = 12 + 1 ;// ���ͳ���+ һ���ֽڵĳ��ȣ�256> 2���ֽڣ�
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
				str.Format("�����ļ�����");
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				str.Format("update data type�� %02x", can_data.buf[12]);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				diag_wait_time	= GetTickCount();
				diag_update_step = UPDATE_TO_AVM_REQUEST_DOWNLOAD;
			}
			else if( (can_sour.buf[1] == 0x7f) && (can_sour.buf[2] == 0x31))
			{
				if (can_sour.buf[3] == 0x78) {
					DebugPrintf(NORM_DBG, "NRC-78 �ȴ�...", 0, 0, DBG_VAR_TYPE);
					break;
				}
				/* �ȴ��ڴ����ʧ�ܸ���Ӧ */
				DebugPrintf(NORM_DBG,"�ȴ��ڴ����ʧ��",0,0,DBG_VAR_TYPE);
				OnBnClickedButtonStop();
				
			}
		}
		break;
	case UPDATE_TO_AVM_REQUEST_DOWNLOAD:
		/* �ȴ���������Ӧ�� */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			str.Format("δ�ܻ�ȡ�������ļ�������Ӧ��д������ʧ��!");
			WriteToTboxErr(str);
			OnBnClickedButtonStop();
		}
		else
		{
			/* ���յ����� */
			if( can_sour.buf[1] == 0x74 && can_sour.buf[2] == 0x20)
			{	/* ���յ�Ӧ���������سɹ� */
				str.Format("�����ļ����سɹ�");
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				p_temp = &(can_sour.buf[3]);
				m_block_size = GET_WORD(p_temp);

				if(m_file_size % (m_block_size-4))
					m_block_num = m_file_size/(m_block_size-4) + 1;
				else
					m_block_num = m_file_size/(m_block_size-4);
			
				//ǰn-1���CAN���ݳ���
				if((m_block_size + 1) % 7)
				{
					m_temp_len_before = ((m_block_size+1)/7 + 1) * 7;
				}
				else
				{
					m_temp_len_before = m_block_size + 1;
				}

				//��n���CAN���ݳ���   ���һ�����ݳ���
				m_remain_len = m_file_size - (m_block_num - 1)*(m_block_size - 4);
				if((m_remain_len + 4 + 1) % 7)
				{
					m_temp_len_last = ((m_remain_len+4+1)/7 + 1) * 7;
				}
				else
				{
					m_temp_len_last = m_remain_len + 4 + 1;
				}

				/* ���Ϳ����� */
				if(block_cnt < m_block_num)	//ǰn-1��
				{
					can_data.id			= avm_id;
					if(m_temp_len_before <= 256)
					{
						can_data.len    = m_temp_len_before;
						memset(can_data.buf, 0x00, m_temp_len_before);
						can_data.buf[0] = (unsigned char)m_block_size;
						can_data.buf[1] = 0x36;

						// �������ţ��ȵͺ��
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

						// �������ţ��ȵͺ��
						can_data.buf[3] = block_cnt%0x100;
						can_data.buf[4] = block_cnt/0x100;
						can_data.buf[5] = block_cnt/0x10000;

						pFile.Seek((block_cnt-1)*(m_block_size-4), CFile::begin);
						pFile.Read(can_data.buf+6,m_block_size-4);

						//memcpy(can_data.buf+6, m_file_data+(block_cnt-1)*(m_block_size-4), m_block_size-4);  
					}
				}
				else if(block_cnt == m_block_num)			//��n��
				{
					can_data.id			= avm_id;
					if(m_temp_len_last <= 256)
					{
						can_data.len    = m_temp_len_last;
						memset(can_data.buf, 0x00, m_temp_len_last);
						can_data.buf[0] = m_remain_len + 4;
						can_data.buf[1] = 0x36;
						
						// �������ţ��ȵͺ��
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
						
						// �������ţ��ȵͺ��
						can_data.buf[3] = block_cnt%0x100;
						can_data.buf[4] = block_cnt/0x100;
						can_data.buf[5] = block_cnt/0x10000;

						pFile.Seek((block_cnt-1)*(m_block_size-4), CFile::begin);
						pFile.Read(can_data.buf+6,m_remain_len);

						//memcpy(can_data.buf+6, m_file_data+(block_cnt-1)*(m_block_size-4), m_remain_len);  
					}
				}

				can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
				str.Format("���͵�%d������", block_cnt);
				//DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				ShowInfo(str, 2);

				diag_wait_time	= GetTickCount();
				diag_update_step = UPDATE_TO_AVM_TRANSFER_DATA;
			}
			else if( can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x34)
			{
				str.Format("�����ļ�����ʧ�ܣ�");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();
			}
		}
		break;
	case UPDATE_TO_AVM_TRANSFER_DATA:
		/* ���Ϳ�������Ӧ */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			str.Format("δ�ܻ�ȡ����%d�����ݷ�����Ӧ��д������ʧ��!", block_cnt);
			WriteToTboxErr(str);
			OnBnClickedButtonStop();
		}
		else
		{
			/* ���յ����� */
			if( can_sour.buf[1] == 0x76 /*&& can_sour.buf[2] == block_cnt*/ )
			{	/* ���յ���i�����ݷ���Ӧ���˳���i�����ݴ��� */     
				str.Format("���͵�%d�����ݳɹ�", block_cnt);
				//DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				ShowInfo(str, 2);

				/* �ļ������� */
				if(block_cnt < m_block_num)
					dwFilePos += m_block_size - 4;
				else
					dwFilePos += m_remain_len;
				m_cProgress.SetPos(dwFilePos);

				if(block_cnt < m_block_num)
				{
					block_cnt++;

					/* ���Ϳ����� */
					if(block_cnt < m_block_num)	//ǰn-1��
					{
						can_data.id			= avm_id;
						if(m_temp_len_before <= 256)
						{
							can_data.len    = m_temp_len_before;
							memset(can_data.buf, 0x00, m_temp_len_before);
							can_data.buf[0] = (unsigned char)m_block_size;
							can_data.buf[1] = 0x36;

							// �������ţ��ȵͺ��
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

							// �������ţ��ȵͺ��
							can_data.buf[3] = block_cnt%0x100;
							can_data.buf[4] = block_cnt/0x100;
							can_data.buf[5] = block_cnt/0x10000;

							pFile.Seek((block_cnt-1)*(m_block_size-4), CFile::begin);
							pFile.Read(can_data.buf+6,m_block_size-4);

							//memcpy(can_data.buf+6, m_file_data+(block_cnt-1)*(m_block_size-4), m_block_size-4);  
						}
					}
					else if(block_cnt == m_block_num)			//��n��
					{
						can_data.id			= avm_id;
						if(m_temp_len_last <= 256)
						{
							can_data.len    = m_temp_len_last;
							memset(can_data.buf, 0x00, m_temp_len_last);
							can_data.buf[0] = m_remain_len + 4;
							can_data.buf[1] = 0x36;
						
							// �������ţ��ȵͺ��
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
						
							// �������ţ��ȵͺ��
							can_data.buf[3] = block_cnt%0x100;
							can_data.buf[4] = block_cnt/0x100;
							can_data.buf[5] = block_cnt/0x10000;

							pFile.Seek((block_cnt-1)*(m_block_size-4), CFile::begin);
							pFile.Read(can_data.buf+6,m_remain_len);

							//memcpy(can_data.buf+6, m_file_data+(block_cnt-1)*(m_block_size-4), m_remain_len);  
						}
					}

					//str.Format("��ʼ���͵�%d������", block_cnt);
					//ShowInfo(str, 2);
					can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
					str.Format("���͵�%d������", block_cnt);
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
					str.Format("�����˳��ļ����ݴ�������");
					DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				}
			}
			else if( can_sour.buf[1] == 0x7F)
			{

			if (can_sour.buf[3] == 0x78) {
				DebugPrintf(NORM_DBG, "NRC-78 �ȴ�...", 0, 0, DBG_VAR_TYPE);
				break;
			}
				/* ���Ϳ�����ʧ�� */
				str.Format("���͵�%d������ʧ�ܣ�", block_cnt);
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();
			}
		}
		break;
	case UPDATE_TO_AVM_TRANSFER_DATA_EXIT:
		/* �˳����ݴ��� */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			str.Format("δ�ܻ�ȡ���˳����ݴ���������Ӧ��д������ʧ��!");
			WriteToTboxErr(str);
			OnBnClickedButtonStop();
		}
		else
		{
			/* ���յ����� */

			if( can_sour.buf[1] == 0x77)
			{	/* �˳����ݴ��䣬�����ļ�����У�� */
				str.Format("�˳��ļ����ݴ���ɹ�");
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				pFile.Close(); // �ر��ļ�

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
				DebugPrintf(NORM_DBG,"�ļ�����У��",0,0,DBG_VAR_TYPE);
			
			}
			else if( can_sour.buf[1] == 0x7F)
			{
				/* �˳����ݴ���ʧ�� */
				str.Format("�˳��ļ����ݴ���ʧ�ܣ�");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();
			}
		}
		break;
	case UPDATE_TO_AVM_UPDATE_CHECK:
		/* �ļ�����У��2 */
		/*if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )*/
		if (0)
		{	/* �ȴ���ʱ */
			str.Format("δ�ܻ�ȡ���ļ�����У����Ӧ��д������ʧ��!");
			WriteToTboxErr(str);
			OnBnClickedButtonStop();
		}
		else
		{
			/* ���յ����� */
			if( can_sour.buf[1] == 0x71 && can_sour.buf[5] == 0x01)
			{	
				DebugPrintf(NORM_DBG,"�ļ�����У��ɹ�",0,0,DBG_VAR_TYPE);
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
					DebugPrintf(NORM_DBG, "д����", 0, 0, DBG_VAR_TYPE);


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
					DebugPrintf(NORM_DBG, "�ն˸�λ", 0, 0, DBG_VAR_TYPE);
					
				}

			}
			else if (can_sour.buf[1] == 0x71 && can_sour.buf[5] == 0x00) {
				DebugPrintf(NORM_DBG, "�ļ�У��ʧ��", 0, 0, DBG_VAR_TYPE);
				OnBnClickedButtonStop();
			}
			else if(can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x31)
			{	
				if (can_sour.buf[3] == 0x78) {
					DebugPrintf(NORM_DBG, "NRC-78 �ȴ�...", 0, 0, DBG_VAR_TYPE);
					break;
				}
				DebugPrintf(NORM_DBG,"�ļ�У�鸺��Ӧ",0,0,DBG_VAR_TYPE);
				OnBnClickedButtonStop();
			}
		}
		break;

	case UPDATE_TO_AVM_WRITE_DATA:
		if ((GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME)
		{	/* �ȴ���ʱ */
			WriteToTboxErr("����д����ָ�ʱ!");
			OnBnClickedButtonStop();
		}
		else
		{	/* ���յ����� */
			if (can_sour.buf[1] == 0x6E && can_sour.buf[2] == 0xF1 && can_sour.buf[3] == 0x99)
			{	/* ���յ�Ӧ��, */
				DebugPrintf(NORM_DBG, "����д����ָ��ɹ�", 0, 0, DBG_VAR_TYPE);

	
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
				//BCD��洢  
				can_data.buf[4] = (can_data.buf[4]/10 <<4 )  | (can_data.buf[4]%10 & 0x0F);
				can_data.buf[5] = (can_data.buf[5]/10 << 4) | (can_data.buf[5] % 10 & 0x0F);

				can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);

				diag_wait_time = GetTickCount();
				diag_update_step = UPDATE_TO_AVM_WRITE_VER;
				DebugPrintf(NORM_DBG, "д����汾", 0, 0, DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x2E) {

				str.Format("����д����ָ��ʧ�ܣ�");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();

			}

		}
		break;

	case UPDATE_TO_AVM_WRITE_VER:
		if ((GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME)
		{	/* �ȴ���ʱ */
			WriteToTboxErr("д����汾ָ�ʱ!");
			OnBnClickedButtonStop();
		}
		else
		{	/* ���յ����� */
			if (can_sour.buf[1] == 0x6E && can_sour.buf[2] == 0xF1 && can_sour.buf[3] == 0x89)
			{	/* ���յ�Ӧ��, */
				DebugPrintf(NORM_DBG, "д����汾ָ��ɹ�", 0, 0, DBG_VAR_TYPE);


				can_data.id = avm_id;
				can_data.len = 8;
				memset(can_data.buf, 0x00, 8);
				can_data.buf[0] = 0x02;
				can_data.buf[1] = 0x11;
				can_data.buf[2] = 0x01;
				can_driver.TransmitData(can_data.id, can_data.buf, can_data.len);

				diag_wait_time = GetTickCount();
				diag_update_step = UPDATE_TO_AVM_RESET;
				DebugPrintf(NORM_DBG, "�ն˸�λ", 0, 0, DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x2E) {

				str.Format("д����汾ָ��ʧ�ܣ�");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();

			}

		}
		break;

	case UPDATE_TO_AVM_RESET:
		if ((GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME)
		{	/* �ȴ���ʱ */
			WriteToTboxErr("�ն˸�λ��ʱ!");
			OnBnClickedButtonStop();
		}
		else
		{	/* ���յ����� */
			if (can_sour.buf[1] == 0x51 && can_sour.buf[2] == 0x01)
			{	/* ���յ�Ӧ��, */
				DebugPrintf(NORM_DBG, "�ն˸�λָ��ɹ�", 0, 0, DBG_VAR_TYPE);

				Sleep(1500);//�ȴ���λ�ɹ�
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
				DebugPrintf(NORM_DBG, "���DTC", 0, 0, DBG_VAR_TYPE);
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x11) {

				str.Format("�ն˸�λʧ�ܣ�");
				str += CanErrHandle(can_sour.buf[3]);
				WriteToTboxErr(str);
				OnBnClickedButtonStop();

			}

		}
		break;


	case UPDATA_TO_AVM_CLEAR_DTC:
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			str.Format("δ�ܻ�ȡ�����DTC��Ӧ!");
			WriteToTboxErr(str);
			OnBnClickedButtonStop();
		}
		else {
			if (can_sour.buf[1] == 0x54) {
				DebugPrintf(NORM_DBG, "DTC ����ɹ�", 0, 0, DBG_VAR_TYPE);
				DebugPrintf(NORM_DBG, "�����������", 0, 0, DBG_VAR_TYPE);
				OnBnClickedButtonStop();
			}
			else if (can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x14) {
				str.Format("DTC���ʧ��!");
				WriteToTboxErr(str);
				OnBnClickedButtonStop();
			}
		
		}

		break;
	default:
		WriteToTboxErr("���ִ������������");
		OnBnClickedButtonStop();
		break;
	}
	return res;
}








/* ���봫�����̣���TBOX�������ݣ���Ҫ��ȫ��֤ */
int CSGMW_diagDlg::TransferFromTbox(CAN_OBJ can_sour)
{
	int				res       =  0;
	unsigned char*	p_temp    = NULL;
	unsigned long   diag_seed =  0;
	unsigned long	diag_pas  =  0;
	CAN_OBJ			can_data  = {0};
	CString			str;

	//��ӡ
	if(can_sour.len)
	{
		CString strTime = p_dlg->comm.GetTime();
		str.Format("��ӡ -- [%s] --> recv:", strTime);
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
		/* �޲��� */
		break;
	case TRANS_FROM_TBOX_EXTERN_MODE:
		/* ���ͽ�����չģʽָ�� */
		can_data.id     = avm_id;
		can_data.len    = 8;
		memset(can_data.buf, 0x55, 8);
		can_data.buf[0] = 0x02;
		can_data.buf[1] = 0x10;
		can_data.buf[2] = 0x03;

		can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
		DebugPrintf(NORM_DBG,"���ͽ�����չģʽָ��",0,0,DBG_VAR_TYPE);

		res				= 1;
		diag_wait_time	= GetTickCount();
		diag_transfer_step = TRANS_FROM_TBOX_WAIT_EXTERN_MODE;
		break;
	case TRANS_FROM_TBOX_WAIT_EXTERN_MODE:
		/* �ȴ�������չģʽӦ�𣬻�ȡ��Ӧ��󣬷��Ͷ�TBOX���к�ָ�� */
		if( (GetTickCount() - diag_wait_time) >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			WriteToTboxErr("������չģʽָ��δ��Ӧ��д������ʧ��!");
		}
		else
		{	/* ���յ����� */
			if( can_sour.buf[1] == 0x50 && can_sour.buf[2] == 0x03 )
			{	/* ���յ�Ӧ��,���Ͷ����кŲ��� */
				DebugPrintf(NORM_DBG,"������չģʽ�ɹ�",0,0,DBG_VAR_TYPE);

				/* ������������ */
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
				DebugPrintf(NORM_DBG,"����������Ϣ",0,0,DBG_VAR_TYPE);
			}
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == 0x10 )
			{
				res = 1;
				str.Format("������չģʽʧ�ܣ�");
				str += CanErrHandle(can_sour.buf[3]);  
				WriteToTboxErr(str);
			}
		}
		break;
	case TRANS_FROM_TBOX_WAIT_READ_SEED:
		/* �ȴ���ȡ��������Ϣ,���ҷ�����Կ */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			WriteToTboxErr("δ�ܻ�ȡ��TBOX�������ӣ�д������ʧ��!");
		}
		else
		{	/* ����TBOX�ϱ������� */
			if( can_sour.buf[1] == 0x67 && can_sour.buf[2] == 0x01 )
			{	/* ���յ�Ӧ��,������Կ */
				/* ������������ */
				p_temp = &can_sour.buf[3];
				diag_seed = GET_DWORD(p_temp);
				DebugPrintf(NORM_DBG,"found tbox sedd: 0x",(unsigned char*)&diag_seed,4,DBG_VAR_TYPE);

				if( diag_seed == 0  )
				{   /* ��Կ�Ѿ�������ֱ��д������ */
					DebugPrintf(NORM_DBG,"��Կ�ѽ�����ֱ��д������",0,0,DBG_VAR_TYPE);

					if(TRANS_LIST == trans_mod)
					{
						/* �����ļ��б��ȡ���� */
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
						DebugPrintf(NORM_DBG,"�ļ��б��ȡ����",0,0,DBG_VAR_TYPE);

						res				= 1;
						diag_wait_time	= GetTickCount();
						diag_transfer_step = TRANS_FROM_TBOX_REQUEST_READDIR;
					}

					if(TRANS_FILE == trans_mod)
					{
						/* �����ļ���ȡ���� */
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
							str.Format("�ļ�(%s) - ��ȡ����", m_dequeSelFile.at(file_cnt-1).file_name);
							DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

							res				= 1;
							diag_wait_time	= GetTickCount();
							diag_transfer_step = TRANS_FROM_TBOX_REQUEST_READFILE;
						}
						else
						{
							/* �ļ���ȡ������� */
							res				= 1;
							str.Format("û��ѡ�е��ļ����ļ���ȡ�������");
							WriteToTboxErr(str);
						}
					}

					if(DELETE_FILE == trans_mod)
					{
						/* �����ļ���ȡ���� */
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
							str.Format("�ļ�(%s) - ɾ������", m_dequeSelFile.at(file_cnt-1).file_name);
							DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

							res				= 1;
							diag_wait_time	= GetTickCount();
							diag_transfer_step = TRANS_FROM_TBOX_REQUEST_DELETE;
						}
						else
						{
							/* �ļ���ȡ������� */
							res				= 1;
							str.Format("û��ѡ�е��ļ����ļ�ɾ���������");
							WriteToTboxErr(str);
						}
					}
					break;
				}

				/* ����password */
				diag_pas = safe.uds_calc_key(diag_seed,1);
					
				/* ������Կ */
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
				DebugPrintf(NORM_DBG,"������Կ",0,0,DBG_VAR_TYPE);
			}
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == 0x27 )
			{
				res = 1;
				WriteToTboxErr("�޷���ȡTBOX������Ϣ��д������ʧ��!");

				str.Format("get tbox seed failed: %d",can_data.buf[3]);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
			}
		}
		break;
	case TRANS_FROM_TBOX_WAIT_READ_PASSWORD:
		/* �ȴ���Կ����Ӧ�𣬿�ʼ��TBOXд������ */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			WriteToTboxErr("δ�ܻ�ȡ��TBOX��Կ��Ϣ��д������ʧ��!");
		}
		else
		{	/* ���յ����� */
			if( can_sour.buf[1] == 0x67 && can_sour.buf[2] == 0x02 )
			{	/* ���յ���Կ��֤Ӧ�𣬷����ļ��б��ȡ���� */
				DebugPrintf(NORM_DBG,"��Կ��֤�ɹ�����ʼд������",0,0,DBG_VAR_TYPE);

				if(TRANS_LIST == trans_mod)
				{
					/* �����ļ��б��ȡ���� */
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
					DebugPrintf(NORM_DBG,"�ļ��б��ȡ����",0,0,DBG_VAR_TYPE);

					res				= 1;
					diag_wait_time	= GetTickCount();
					diag_transfer_step = TRANS_FROM_TBOX_REQUEST_READDIR;
				}

				if(TRANS_FILE == trans_mod)
				{
					/* �����ļ���ȡ���� */
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
						str.Format("�ļ�(%s) - ��ȡ����", m_dequeSelFile.at(file_cnt-1).file_name);
						DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

						res				= 1;
						diag_wait_time	= GetTickCount();
						diag_transfer_step = TRANS_FROM_TBOX_REQUEST_READFILE;
					}
					else
					{
						/* �ļ���ȡ������� */
						res				= 1;
						str.Format("û��ѡ�е��ļ����ļ���ȡ�������");
						WriteToTboxErr(str);
					}
				}

				if(DELETE_FILE == trans_mod)
				{
					/* �����ļ���ȡ���� */
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
						str.Format("�ļ�(%s) - ɾ������", m_dequeSelFile.at(file_cnt-1).file_name);
						DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

						res				= 1;
						diag_wait_time	= GetTickCount();
						diag_transfer_step = TRANS_FROM_TBOX_REQUEST_DELETE;
					}
					else
					{
						/* �ļ���ȡ������� */
						res				= 1;
						str.Format("û��ѡ�е��ļ����ļ�ɾ���������");
						WriteToTboxErr(str);
					}
				}
			}
			else if( can_sour.buf[1] == 0x7f && can_sour.buf[2] == 0x27 )
			{
				res				= 1;
				WriteToTboxErr("��Կ����д������ʧ��!");

				str.Format("tbox password check failed: %d",can_sour.buf[3]);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
			}
		}
		break;
	case TRANS_FROM_TBOX_REQUEST_READDIR:
		/* �ȴ��ļ��б��ȡ����Ӧ�� */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			WriteToTboxErr("δ�ܻ�ȡ���ļ��б��ȡ������Ӧ��д������ʧ��!");
		}
		else
		{
			/* ���յ����� */
			if( can_sour.buf[2] == 0x78 && can_sour.buf[3] == 0x05 )
			{
				DebugPrintf(NORM_DBG,"�ļ��б��ȡ����ɹ�",0,0,DBG_VAR_TYPE);

				unsigned int  max_day = 0;
				unsigned int  file_idx = 0;
				unsigned long temp_hour = 0;
				unsigned int  i,j;
				TBOX_FILE_LIST file_list = {0};

				max_day = (can_sour.buf[1] - 2) / 6;
				p_temp = &(can_sour.buf[4]);
				file_idx = 0;

				m_comb_file_list.ResetContent();
				m_comb_file_list.InsertString(0, "ȫ��ѡ��");
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

				/* �˳������ļ��б����� */
				res				= 1;
				diag_wait_time	= GetTickCount();
				diag_transfer_step = TRANS_FROM_TBOX_NULL;
				handle_flag		= 0;
				handle_type		= 0;
			}
			else if( can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x38 )
			{
				/* �ļ��б��ȡ����ʧ�� */
				res				= 1;
				str.Format("�ļ��б��ȡ����ʧ�ܣ�");
				str += CanErrHandle(can_sour.buf[3]);  
				WriteToTboxErr(str);
			}
		}
		break;
	case TRANS_FROM_TBOX_REQUEST_READFILE:
		/* �ļ���ȡ���� */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			str.Format("δ�ܻ�ȡ���ļ���ȡ������Ӧ��д������ʧ��!");
			WriteToTboxErr(str);
		}
		else
		{
			/* ���յ����� */
			if( can_sour.buf[1] == 0x78 && can_sour.buf[2] == 0x04 )
			{
				str.Format("�ļ�(%s) - ��ȡ����ɹ�", m_dequeSelFile.at(file_cnt-1).file_name);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				p_temp = &(can_sour.buf[3]);
				m_block_num = GET_WORD(p_temp);

				str.Format("�ļ�(%s)���ȣ�%d", m_dequeSelFile.at(file_cnt-1).file_name, m_block_num*320);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				str.Format("�ļ�(%s)����Ŀ��%d", m_dequeSelFile.at(file_cnt-1).file_name, m_block_num);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				/* ��ʼ�������� */
				m_cProgress.SetRange32(0, m_block_num);
				m_cProgress.SetPos(0);

				/* �ļ����ݴ��� */
				str.Format("�ļ�(%s) - ��ʼ����", m_dequeSelFile.at(file_cnt-1).file_name);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				can_data.id     = avm_id;
				can_data.len    = 8;
				memset(can_data.buf, 0x55, 8);
				can_data.buf[0] = 0x03;
				can_data.buf[1] = 0x36;
				can_data.buf[2] = (block_cnt>>8)&0xff;
				can_data.buf[3] = block_cnt&0xff;

				can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
				str.Format("�ļ�����%d", block_cnt);
				//DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				ShowInfo(str, 2);

				res				= 1;
				diag_wait_time	= GetTickCount();
				diag_transfer_step = TRANS_FROM_TBOX_TRANSFER_FILEDATA;
			}
			else if( can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x38 )
			{
				/* �ļ���ȡ����ʧ�� */
				res				= 1;
				str.Format("�ļ�(%s) - ��ȡ����ʧ�ܣ�", m_dequeSelFile.at(file_cnt-1).file_name);
				str += CanErrHandle(can_sour.buf[3]);  
				WriteToTboxErr(str);
			}
		}
		break;
	case TRANS_FROM_TBOX_TRANSFER_FILEDATA:
		/* �ļ����� */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			str.Format("δ�ܻ�ȡ���ļ����ݴ�����Ӧ��д������ʧ��!");
			WriteToTboxErr(str);
		}
		else
		{
			/* ���յ����� */
			if( can_sour.buf[2] == 0x76 /*&& can_sour.buf[3] == block_cnt*/ )
			{	/* �ļ����� */

				DWORD dw = ((can_sour.buf[0] & 0x0f) << 8) + can_sour.buf[1] - 3;
				//memcpy(m_file_data+f_index, can_sour.buf+4, dw);
				f_index += dw;
				str.Format("�ֽ�����%d", f_index);
				ShowInfo(str, 2);

				/* �ļ������� */
				m_cProgress.SetPos(f_index);

				str.Format("�ļ�����%d�ɹ�", block_cnt);
				//DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				ShowInfo(str, 2);

				if(block_cnt < m_block_num)
				{
					/* �ļ����� */
					block_cnt++;
					can_data.id     = avm_id;
					can_data.len    = 8;
					memset(can_data.buf, 0x55, 8);
					can_data.buf[0] = 0x03;
					can_data.buf[1] = 0x36;
					can_data.buf[2] = (block_cnt>>8)&0xff;
					can_data.buf[3] = block_cnt&0xff;
					can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
					str.Format("�ļ�����%d", block_cnt);
					//DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
					ShowInfo(str, 2);

					res				= 1;
					diag_wait_time	= GetTickCount();
					diag_transfer_step = TRANS_FROM_TBOX_TRANSFER_FILEDATA;
				}
				else if(block_cnt == m_block_num)
				{
					str.Format("�ļ�(%s) - �������", m_dequeSelFile.at(file_cnt-1).file_name);
					DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);


					/* �ļ������˳� */			
					can_data.id     = avm_id;
					can_data.len    = 8;
					memset(can_data.buf, 0x55, 8);
					can_data.buf[0] = 0x01;
					can_data.buf[1] = 0x37;
					can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
					str.Format("�ļ�(%s) - �����˳�", m_dequeSelFile.at(file_cnt-1).file_name);
					DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

					res				= 1;
					diag_wait_time	= GetTickCount();
					diag_transfer_step = TRANS_FROM_TBOX_TRANSFER_FILEEXIT;
				}
			}
			else if(can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x36)
			{
				/* �ļ�����ʧ�� */
				res				= 1;
				str.Format("�ļ�(%s) - ����ʧ�ܣ�", m_dequeSelFile.at(file_cnt-1).file_name);
				str += CanErrHandle(can_sour.buf[3]);  
				WriteToTboxErr(str);
			}
		}
		break;
	case TRANS_FROM_TBOX_TRANSFER_FILEEXIT:
		/* �ļ������˳� */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			str.Format("δ�ܻ�ȡ���ļ������˳���Ӧ��д������ʧ��!");
			WriteToTboxErr(str);
		}
		else
		{
			/* ���յ����� */
			if( can_sour.buf[1] == 0x77 )
			{	/* �ļ������˳� */
				str.Format("�ļ�(%s) - �����˳����", m_dequeSelFile.at(file_cnt-1).file_name);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				/* �����ļ� */
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
					//	str.Format("д�ļ�(%s)���� - %d", m_dequeSelFile.at(file_cnt-1).file_name, GetLastError());
					//	DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
					//}
					CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;

					str.Format("======= �����ļ�(%s) =======", m_dequeSelFile.at(file_cnt-1).file_name);
					DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
				}
				else
				{
					str.Format("���ļ�(%s)���� - %d", m_dequeSelFile.at(file_cnt-1).file_name, GetLastError());
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

					//* ������һ���ļ���ȡ�������� */
					res				= 1;
					diag_wait_time	= GetTickCount();
					diag_transfer_step = TRANS_FROM_TBOX_EXTERN_MODE;
					CAN_OBJ		can_buf  = {0};
					TransferFromTbox(can_buf);
				}
				else
				{
					/* �رմ������� */
					res				= 1;
					diag_wait_time	= GetTickCount();
					diag_transfer_step = TRANS_FROM_TBOX_NULL;
					handle_flag		= 0;
					handle_type		= 0;
				}
			}
			else if(can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x37)
			{
				/* �ļ������˳�ʧ�� */
				res				= 1;
				str.Format("�ļ�(%s) - �����˳�ʧ�ܣ�", m_dequeSelFile.at(file_cnt-1).file_name);
				str += CanErrHandle(can_sour.buf[3]);  
				WriteToTboxErr(str);
			}
		}
		break;

/*********************************�ļ�ɾ������********************************************/
	case TRANS_FROM_TBOX_REQUEST_DELETE:
	/* �ļ�ɾ������ */
		if( GetTickCount() - diag_wait_time >= TIMEOUT_WAIT_TIME )
		{	/* �ȴ���ʱ */
			str.Format("δ�ܻ�ȡ���ļ�ɾ��������Ӧ��д������ʧ��!");
			WriteToTboxErr(str);
		}
		else
		{
			/* ���յ����� */
			if( can_sour.buf[1] == 0x78 && can_sour.buf[2] == 0x02 )
			{	/* �ļ�ɾ������ */
				str.Format("�ļ�(%s) - ɾ������ɹ�", m_dequeSelFile.at(file_cnt-1).file_name);
				DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);

				if((size_t)file_cnt < m_dequeSelFile.size())
				{
					file_cnt++;

					//* ������һ���ļ�ɾ���������� */
					res				= 1;
					diag_wait_time	= GetTickCount();
					diag_transfer_step = TRANS_FROM_TBOX_EXTERN_MODE;
					CAN_OBJ		can_buf  = {0};
					TransferFromTbox(can_buf);
				}
				else
				{
					/* ɾ����������ɾ�����ļ� */
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


					/* �رմ������� */
					res				= 1;
					diag_wait_time	= GetTickCount();
					diag_transfer_step = TRANS_FROM_TBOX_NULL;
					handle_flag		= 0;
					handle_type		= 0;
				}
			}
			else if(can_sour.buf[1] == 0x7F && can_sour.buf[2] == 0x38)
			{
				/* �ļ�ɾ������ʧ�� */
				res				= 1;
				str.Format("�ļ�(%s) - ɾ������ʧ�ܣ�", m_dequeSelFile.at(file_cnt-1).file_name);
				str += CanErrHandle(can_sour.buf[3]);  
				WriteToTboxErr(str);
			}
		}
		break;
	default:
		WriteToTboxErr("���ִ�����ļ����䲽��");
		break;
	}
	return res;
}