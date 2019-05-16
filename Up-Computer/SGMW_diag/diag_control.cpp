#include "stdafx.h"
#include "SGMW_diag.h"
#include "SGMW_diagDlg.h"
#include "afxdialogex.h"
#include "diag.h"

const TBOX_READ_INFO avm_read_info_list[] = 
{	
	
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 1, 0xF186, 0,0,0,0,0,	"��ȡ��ǰ�Ựģʽ "},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 9, 3, 0xF187, 0,0,0,0,0,	"read/write �����㲿����"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 9, 3, 0xF188, 0,0,0,0,0,	"read/write ����������"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 2, 3, 0xF189, 0,0,0,0,0,	"read/write ECU����汾��"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 9, 3, 0xF191, 0,0,0,0,0,	"read/write ����Ӳ�����"},
	//{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 2, 3, 0xF189, 0,0,0,0,0,	"read/write ����Ӳ���汾��"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 8, 1, 0xF18A, 0,0,0,0,0,	"read ��Ӧ�̴���"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 4, 1, 0xF18B, 0,0,0,0,0,	"read ��������"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 10, 1, 0xF18C, 0,0,0,0,0,	"read ���к�"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 3, 0xF18D, 0,0,0,0,0,	"read/write ֧�ֵĹ��ܵ�λ"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			17, 3, 0xF190, 0,0,0,0,0,	"Read/Write VINDataIdentifier "},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 4, 3, 0xF197, 0,0,0,0,0,	"read/write ����������"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 4, 3, 0xF199, 0,0,0,0,0,	"read/write ����ˢд����"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 4, 3, 0xF19B, 0,0,0,0,0,	"read/write �궨����"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 4, 3, 0xF19D, 0,0,0,0,0,	"read/write װ������"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 2, 1, 0xF1b0, 0,0,0,0,0,	"read ��������ѹ"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 3, 1, 0xFD00, 0,0,0,0,0,	"��ȡMCU APP�汾��� "},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 3, 1, 0xFD01, 0,0,0,0,0,	"��ȡMCU BOOT�汾��� "},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 3, 1, 0xFD02, 0,0,0,0,0,	"��ȡARM APP�汾��� "},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 3, 1, 0xFD03, 0,0,0,0,0,	"��ȡARM SF�汾��� "},


	

	{DIAGNOSITIC_SESSION_CONTROL,		 	DIAGNOSITIC_SESSION_CONTROL,		 1, 2, 0x0000, 1,0,0,0,0,	"DiagnosticSessionControl (10 hex)"},
	{ECU_RESET, 							ECU_RESET,	 						 1, 2, 0x0000, 1,0,0,0,0,	"ECUReset (11 hex)"},
	{CLEAR_DIAGNOSTIC_INFORMATION, 			CLEAR_DIAGNOSTIC_INFORMATION,	 	 1, 2, 0x0000, 1,1,1,0,0,	"ClearDiagnosticInformation (14 hex)"},
	{READ_DTC_INFORMATION, 					READ_DTC_INFORMATION,	 			 1, 2, 0x0000, 1,1,1,1,1,	"ReadDTCInformation (19 hex)"},
	{COMMUNICATION_CONTROL, 				COMMUNICATION_CONTROL,				 1, 2, 0x0000, 1,1,0,0,0,	"CommunicationControl (28 hex)"},
	{TESTER_PRESENT, 						TESTER_PRESENT,						 1, 2, 0x0000, 1,0,0,0,0,	"TesterPresent (3E hex)"},
	{CONTROL_DTC_SETTING, 					CONTROL_DTC_SETTING,				 1, 2, 0x0000, 1,0,0,0,0,	"ControlDTCSetting (85 hex)"},
};

/* 
const TBOX_READ_INFO tbox_read_info_list[] = 
{
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			12, 1, 0xF195, 0,0,0,0,0,	"��ȡ�汾��Ϣ "},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 9, 1, 0xFD07, 0,0,0,0,0,	"��ȡ����Ϣ "},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 1, 0xFD08, 0,0,0,0,0,	"��ȡ������Ϣ"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			26, 1, 0xF18C, 0,0,0,0,0,	"��ȡTBOX����"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			10, 1, 0xFD05, 0,0,0,0,0,	"��ȡTBOX ICCID "},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			19, 1, 0xF18C, 0,0,0,0,0,	"Read ECUSerialNumber "},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			17, 3, 0xF190, 0,0,0,0,0,	"Read/Write VehicleIdentificationNumbe"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 8, 1, 0xF192, 0,0,0,0,0,	"Read SupplierECUHardwareNumber "},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 4, 1, 0xF193, 0,0,0,0,0,	"Read SupplierECUHardwareVersionNumber"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 8, 1, 0xF194, 0,0,0,0,0,	"Read Supplier ECU Software Number "},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 4, 1, 0xF195, 0,0,0,0,0,	"Read Supplier ECU Software Version Number"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 4, 3, 0xF1B2, 0,0,0,0,0,	"Read/Write Vehicle Manufacturing Date"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 4, 3, 0xF1B9, 0,0,0,0,0,	"Read/Write Subnet Configuration List on High Speed CAN"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 4, 1, 0xF1CB, 0,0,0,0,0,	"Read EndModelPartNumber (SGMW P/N)"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 1, 0x6000, 0,0,0,0,0,	"Read B+��ѹ"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 1, 0x6001, 0,0,0,0,0,	"Read B+����״̬"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 1, 0x6002, 0,0,0,0,0,	"Read GPS ģ�鹤��״̬"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 1, 0x6003, 0,0,0,0,0,	"Read GPS ���߹���״̬"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 1, 0x6004, 0,0,0,0,0,	"Read GPS ��λ״̬"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			10, 1, 0x6005, 0,0,0,0,0,	"Read GPS ��λ����"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 1, 0x6006, 0,0,0,0,0,	"Read GPRS ģ�鹤��״̬"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 1, 0x6007, 0,0,0,0,0,	"Read GPRS ���߹���״̬"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 1, 0x6008, 0,0,0,0,0,	"Read GPRS ����״̬"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 1, 0x6009, 0,0,0,0,0,	"Read CANBUS ͨ��״̬"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			14, 1, 0x600a, 0,0,0,0,0,	"Read �ƶ��豸ʶ����"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 1, 0x600b, 0,0,0,0,0,	"Read �����ź�ǿ��"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 4, 1, 0x600c, 0,0,0,0,0,	"Read ��ǰ�ϴ�Ƶ��"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 1, 0x600d, 0,0,0,0,0,	"Read �洢�ڴ�ʹ����"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 1, 0x600e, 0,0,0,0,0,	"Read �洢����״̬"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 1, 0x600f, 0,0,0,0,0,	"Read ���õ�ص���"},
	{READ_DATA_BY_IDENTIFIER, 				WRITE_DATA_BY_IDENTIFIER,			 1, 1, 0x6010, 0,0,0,0,0,	"Read ���õ������״̬"},

	{DIAGNOSITIC_SESSION_CONTROL,		 	DIAGNOSITIC_SESSION_CONTROL,		 1, 2, 0x0000, 1,0,0,0,0,	"DiagnosticSessionControl (10 hex)"},
	{ECU_RESET, 							ECU_RESET,	 						 1, 2, 0x0000, 1,0,0,0,0,	"ECUReset (11 hex)"},
	{CLEAR_DIAGNOSTIC_INFORMATION, 			CLEAR_DIAGNOSTIC_INFORMATION,	 	 1, 2, 0x0000, 1,1,1,0,0,	"ClearDiagnosticInformation (14 hex)"},
	{READ_DTC_INFORMATION, 					READ_DTC_INFORMATION,	 			 1, 2, 0x0000, 1,1,1,1,1,	"ReadDTCInformation (19 hex)"},
	{COMMUNICATION_CONTROL, 				COMMUNICATION_CONTROL,				 1, 2, 0x0000, 1,1,0,0,0,	"CommunicationControl (28 hex)"},
	{TESTER_PRESENT, 						TESTER_PRESENT,						 1, 2, 0x0000, 1,0,0,0,0,	"TesterPresent (3E hex)"},
	{CONTROL_DTC_SETTING, 					CONTROL_DTC_SETTING,				 1, 2, 0x0000, 1,0,0,0,0,	"ControlDTCSetting (85 hex)"},
};

*/

/* ��DTC��Ϣ */
const unsigned char tbox_dtc_param0[8]					=	{ 0x01, 0x02};	
const unsigned char tbox_dtc_param1_0[4]				=	{ 0x09 };
const DWORD tbox_dtc_param1_2[64]						=							//x_2,������param0�в�����λ��
{
	0x901131,	0x901331,	0x901531,	0x901631,	0x901714,	0x901814,	0x90191C,	0x901A15,
	0x901B42,	0xC07300,	0xD90087,	0xD90187,	0xD90287,	0xD90387,	0xD90687,	0xD98087,
	0xD98187,	0xD98287,	0xDA0087,	0xDA0187,	0xDA0287,	0xDA0487,	0xDA0587,	0xDA0687,	
	0xDA0787,	0xDA0887,	0xDA0987,	0xDA0A87,	0xDA0B87,	0xDA0C87,	0xDA0D87,	0xDA0E87,	
	0xDA0F87,	0xE60087,	0xD80087,	0xD80187,	0xD40087,	0xD40287,	0xD40587,	0xD40387,	
	0xD40487,	0xD60087,	0xD40787,	0xD40887,	0xDB8087,	0xDD0087,
};
const unsigned char tbox_dtc_param2_2[4]				=	{ 0xff };
const unsigned char tbox_dtc_param2_3[4]				=	{ 0xff };

/* ���DTC������ */
const DWORD tbox_dtc_clear_param0[4]					=	{ 0xffffff }; 

/* ���ĻỰģʽ */
const unsigned char tbox_diag_session_param0[4]			=	{ 0x01, 0x02, 0x03 };

/* ��λģʽ */
const unsigned char tbox_ecu_reset_param0[4]			=	{ 0x01, 0x03 };

/* ͨ�ſ��� */
const unsigned char tbox_control_type_param0[4]			=	{ 0x00, 0x03 };
const unsigned char tbox_communicate_type_param1[4]		=	{ 0x03 };

/* ����DTC���ܿ��� */
const unsigned char tbox_dtc_control_param0[4]			=	{ 0x01, 0x02 };

void CSGMW_diagDlg::OnCbnSelchangeComboSelDid()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	/* ��ʾ��ǰ��������ִ�е����� */
	disp_diag_info(m_comb_did.GetCurSel());

	CString strCombo;
	int iCurSel = m_comb_did.GetCurSel();
	m_comb_did.GetLBText(iCurSel, strCombo);
	if(!strCombo.IsEmpty())
	{
		int iListCount = m_list_batch_cmd.GetItemCount();
		if(into_batch_cmd_list(iCurSel, iListCount))
			m_list_batch_cmd.InsertItem(iListCount, strCombo);
	}

	/* ����д����ʱ�Ŀؼ� */
	if(m_combo_write_mode.GetCurSel())
	{
		TBOX_READ_INFO* p_info   = (TBOX_READ_INFO*)&avm_read_info_list[iCurSel];
		diag_write_id = p_info->write_id;
		m_combo_param0.ResetContent();
		m_combo_param1.ResetContent();
		m_combo_param2.ResetContent();
		m_combo_param3.ResetContent();
		switch(p_info->write_id)
		{
		case DIAGNOSITIC_SESSION_CONTROL:
			m_combo_param0.InsertString(0, "Ĭ�ϻỰģʽ(0x01)");
			m_combo_param0.InsertString(1, "��̻Ựģʽ(0x02)");
			m_combo_param0.InsertString(2, "��չ�Ựģʽ(0x03)");
			break;
		case ECU_RESET:
			m_combo_param0.InsertString(0, "Ӳ��λ(0x01)");
			m_combo_param0.InsertString(1, "��λ(0x03)");
			break;
		case CLEAR_DIAGNOSTIC_INFORMATION:
			m_combo_param0.InsertString(0, "���DTC������(0xffffff)");
			break;
		case COMMUNICATION_CONTROL:
			m_combo_param0.InsertString(0, "�����շ�(0x00)");
			m_combo_param0.InsertString(1, "��ֹ�շ�(0x03)");
			break;
		case ROUTINE_CONTROL:
			break;
		case TESTER_PRESENT:
			break;
		case CONTROL_DTC_SETTING:
			m_combo_param0.InsertString(0, "DTC���ܿ�(0x01)");
			m_combo_param0.InsertString(1, "DTC���ܹ�(0x02)");
			break;
		case READ_DTC_INFORMATION:
			m_combo_param0.InsertString(0, "��DTC������Ŀ(0x01)");
			m_combo_param0.InsertString(1, "��DTC����״̬(0x02)");
			//m_combo_param0.InsertString(2, "��DTC����(0x04)");
			//m_combo_param0.InsertString(3, "��DTC��չ����(0x06)");
			//m_combo_param0.InsertString(4, "������DTC_ID(0x0a)");
			break;
		case WRITE_DATA_BY_IDENTIFIER:
			break;
		default:
			break;
		}
	}
}


/*********************************************************************************/
void CSGMW_diagDlg::OnCbnSelchangeComboWriteMode()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	if(m_combo_write_mode.GetCurSel())
	{
		m_combo_param0.ShowWindow(true);
		m_combo_param1.ShowWindow(true);
		m_combo_param2.ShowWindow(true);
		m_combo_param3.ShowWindow(true);

		GetDlgItem(IDC_EDIT_WRITE)->ShowWindow(false);
		GetDlgItem(IDC_EDIT_PARAM0)->ShowWindow(false);
		GetDlgItem(IDC_EDIT_PARAM1)->ShowWindow(false);
		GetDlgItem(IDC_EDIT_PARAM2)->ShowWindow(false);
		GetDlgItem(IDC_EDIT_PARAM3)->ShowWindow(false);
		GetDlgItem(IDC_EDIT_PARAM4)->ShowWindow(false);

		/* ����д����ʱ�Ŀؼ� */
		m_combo_param0.ResetContent();
		m_combo_param1.ResetContent();
		m_combo_param2.ResetContent();
		m_combo_param3.ResetContent();
		TBOX_READ_INFO* p_info   = (TBOX_READ_INFO*)&avm_read_info_list[m_comb_did.GetCurSel()];
		switch(p_info->write_id)
		{
		case DIAGNOSITIC_SESSION_CONTROL:
			m_combo_param0.InsertString(0, "Ĭ�ϻỰģʽ(0x01)");
			m_combo_param0.InsertString(1, "��̻Ựģʽ(0x02)");
			m_combo_param0.InsertString(2, "��չ�Ựģʽ(0x03)");
			break;
		case ECU_RESET:
			m_combo_param0.InsertString(0, "Ӳ��λ(0x01)");
			m_combo_param0.InsertString(1, "��λ(0x03)");
			break;
		case CLEAR_DIAGNOSTIC_INFORMATION:
			m_combo_param0.InsertString(0, "���DTC������(0xffffff)");
			break;
		case COMMUNICATION_CONTROL:
			m_combo_param0.InsertString(0, "�����շ�(0x00)");
			m_combo_param0.InsertString(1, "��ֹ�շ�(0x03)");
			break;
		case ROUTINE_CONTROL:
			break;
		case TESTER_PRESENT:
			break;
		case CONTROL_DTC_SETTING:
			m_combo_param0.InsertString(0, "DTC���ܿ�(0x01)");
			m_combo_param0.InsertString(1, "DTC���ܹ�(0x02)");
			break;
		case READ_DTC_INFORMATION:
			m_combo_param0.InsertString(0, "��DTC������Ŀ(0x01)");
			m_combo_param0.InsertString(1, "��DTC����״̬(0x02)");
			//m_combo_param0.InsertString(2, "��DTC����(0x04)");
			//m_combo_param0.InsertString(3, "��DTC��չ����(0x06)");
			//m_combo_param0.InsertString(4, "������DTC_ID(0x0a)");
			break;
		case WRITE_DATA_BY_IDENTIFIER:
			break;
		default:
			break;
		}
	}
	else
	{
		m_combo_param0.ShowWindow(false);
		m_combo_param1.ShowWindow(false);
		m_combo_param2.ShowWindow(false);
		m_combo_param3.ShowWindow(false);

		GetDlgItem(IDC_EDIT_WRITE)->ShowWindow(true);
		GetDlgItem(IDC_EDIT_PARAM0)->ShowWindow(true);
		GetDlgItem(IDC_EDIT_PARAM1)->ShowWindow(true);
		GetDlgItem(IDC_EDIT_PARAM2)->ShowWindow(true);
		GetDlgItem(IDC_EDIT_PARAM3)->ShowWindow(true);
		GetDlgItem(IDC_EDIT_PARAM4)->ShowWindow(true);
	}
}

void CSGMW_diagDlg::OnCbnSelchangeComboParam0()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	m_combo_param1.ResetContent();
	m_combo_param2.ResetContent();
	m_combo_param3.ResetContent();

	TBOX_READ_INFO* p_info   = (TBOX_READ_INFO*)&avm_read_info_list[m_comb_did.GetCurSel()];
	switch(p_info->write_id)
	{
	case DIAGNOSITIC_SESSION_CONTROL:
		write_info.param0 = tbox_diag_session_param0[m_combo_param0.GetCurSel()];
		break;
	case ECU_RESET:
		write_info.param0 = tbox_ecu_reset_param0[m_combo_param0.GetCurSel()];
		break;
	case CLEAR_DIAGNOSTIC_INFORMATION:
		write_info.param0 = tbox_dtc_clear_param0[m_combo_param0.GetCurSel()];
		break;
	case COMMUNICATION_CONTROL:
		write_info.param0 = tbox_control_type_param0[m_combo_param0.GetCurSel()];
		m_combo_param1.InsertString(0, "ͨ������(0x03)");
		break;
	case ROUTINE_CONTROL:
		break;
	case TESTER_PRESENT:
		break;
	case CONTROL_DTC_SETTING:
		write_info.param0 = tbox_dtc_control_param0[m_combo_param0.GetCurSel()];
		break;
	case READ_DTC_INFORMATION:
		write_info.param0 = tbox_dtc_param0[m_combo_param0.GetCurSel()];
		switch(m_combo_param0.GetCurSel())
		{
		case 0:
		case 1:
			m_combo_param1.InsertString(0, "DTC Mask(0x09)");

			break;
		default:
			break;
		}
		break;
	case WRITE_DATA_BY_IDENTIFIER:
	
		break;
	default:
		break;
	}
}


void CSGMW_diagDlg::OnCbnSelchangeComboParam1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	m_combo_param2.ResetContent();
	m_combo_param3.ResetContent();

	TBOX_READ_INFO* p_info   = (TBOX_READ_INFO*)&avm_read_info_list[m_comb_did.GetCurSel()];
	switch(p_info->write_id)
	{
	case DIAGNOSITIC_SESSION_CONTROL:
		break;
	case ECU_RESET:
		break;
	case CLEAR_DIAGNOSTIC_INFORMATION:
		break;
	case COMMUNICATION_CONTROL:
		write_info.param1 = tbox_communicate_type_param1[m_combo_param1.GetCurSel()];
		break;
	case ROUTINE_CONTROL:
		break;
	case TESTER_PRESENT:
		break;
	case CONTROL_DTC_SETTING:
		break;
	case READ_DTC_INFORMATION:
		switch(m_combo_param0.GetCurSel())
		{
		case 0:
		case 1:
			write_info.param1 = tbox_dtc_param1_0[m_combo_param1.GetCurSel()];
			break;
		case 2:
			//write_info.param1 = tbox_dtc_param1_2[m_combo_param1.GetCurSel()];
		//	m_combo_param2.InsertString(0, "������Ŀ(0xff)");
			break;
		case 3:
			//write_info.param1 = tbox_dtc_param1_2[m_combo_param1.GetCurSel()];
			//m_combo_param2.InsertString(0, "��չ����(0xff)");
			break;
		case 4:
			break;
		default:
			break;
		}
		break;
	case WRITE_DATA_BY_IDENTIFIER:
		break;
	default:
		break;
	}
}


void CSGMW_diagDlg::OnCbnSelchangeComboParam2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	m_combo_param3.ResetContent();

	TBOX_READ_INFO* p_info   = (TBOX_READ_INFO*)&avm_read_info_list[m_comb_did.GetCurSel()];
	switch(p_info->write_id)
	{
	case DIAGNOSITIC_SESSION_CONTROL:
		break;
	case ECU_RESET:
		break;
	case CLEAR_DIAGNOSTIC_INFORMATION:
		break;
	case COMMUNICATION_CONTROL:
		break;
	case ROUTINE_CONTROL:
		break;
	case TESTER_PRESENT:
		break;
	case CONTROL_DTC_SETTING:
		break;
	case READ_DTC_INFORMATION:
		switch(m_combo_param0.GetCurSel())
		{
		case 0:
		case 1:
			break;
		case 2:
			write_info.param2 = tbox_dtc_param2_2[m_combo_param2.GetCurSel()];
			break;
		case 3:
			write_info.param2 = tbox_dtc_param2_3[m_combo_param2.GetCurSel()];
			break;
		case 4:
			break;
		default:
			break;
		}
		break;
	case WRITE_DATA_BY_IDENTIFIER:
		break;
	default:
		break;
	}
}


void CSGMW_diagDlg::OnCbnSelchangeComboParam3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}
/*********************************************************************************/


int CSGMW_diagDlg::disp_diag_info(int sel)
{
	TBOX_READ_INFO* p_info = (TBOX_READ_INFO*)&avm_read_info_list[sel];

	/* ��ȡ�����Ƿ���Ч */
	if( p_info->act_mode & 0x01 )
		((CButton*)GetDlgItem(IDC_BUTTON_READ_DID))->EnableWindow(true);
	else
		((CButton*)GetDlgItem(IDC_BUTTON_READ_DID))->EnableWindow(false);

	/* д�밴���Ƿ���Ч */
	if( p_info->act_mode & 0x02 )
	{
		((CButton*)GetDlgItem(IDC_EDIT_WRITE))->EnableWindow(true);
		((CButton*)GetDlgItem(IDC_BUTTON_WRITE_DID))->EnableWindow(true);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_EDIT_WRITE))->EnableWindow(false);
		((CButton*)GetDlgItem(IDC_BUTTON_WRITE_DID))->EnableWindow(false);
	}

	/* ����0�Ƿ���ʾ */
	if( p_info->param0 )
		((CButton*)GetDlgItem(IDC_EDIT_PARAM0))->EnableWindow(true);
	else
		((CButton*)GetDlgItem(IDC_EDIT_PARAM0))->EnableWindow(false);

	/* ����1�Ƿ���ʾ */
	if( p_info->param1 )
		((CButton*)GetDlgItem(IDC_EDIT_PARAM1))->EnableWindow(true);
	else
		((CButton*)GetDlgItem(IDC_EDIT_PARAM1))->EnableWindow(false);

	/* ����2�Ƿ���ʾ */
	if( p_info->param2 )
		((CButton*)GetDlgItem(IDC_EDIT_PARAM2))->EnableWindow(true);
	else
		((CButton*)GetDlgItem(IDC_EDIT_PARAM2))->EnableWindow(false);

	/* ����3�Ƿ���ʾ */
	if( p_info->param3 )
		((CButton*)GetDlgItem(IDC_EDIT_PARAM3))->EnableWindow(true);
	else
		((CButton*)GetDlgItem(IDC_EDIT_PARAM3))->EnableWindow(false);

	/* ����4�Ƿ���ʾ */
	if( p_info->param4 )
		((CButton*)GetDlgItem(IDC_EDIT_PARAM4))->EnableWindow(true);
	else
		((CButton*)GetDlgItem(IDC_EDIT_PARAM4))->EnableWindow(false);
	return 0;
}

/* ��ʼ��diag�б� */
int CSGMW_diagDlg::init_diag_info(void)
{
	int i;
	for( i = 0 ; i < sizeof(avm_read_info_list) / sizeof(TBOX_READ_INFO) ; i++ )
		m_comb_did.AddString(avm_read_info_list[i].str);

	m_comb_did.SetCurSel(0);
	return 0;
}
/* ����diag��д״̬ */
int CSGMW_diagDlg::get_diag_rw_status(int sel)
{
	TBOX_READ_INFO* p_info = (TBOX_READ_INFO*)&avm_read_info_list[sel];
	return p_info->act_mode;
}
/* ����diag����״̬ */
int CSGMW_diagDlg::get_diag_param_status(int sel)
{
	TBOX_READ_INFO* p_info = (TBOX_READ_INFO*)&avm_read_info_list[sel];
	int res = 0;
	if( p_info->param0 )
		res++;
	if( p_info->param1 )
		res++;
	if( p_info->param2 )
		res++;
	if( p_info->param3 )
		res++;
	if( p_info->param4 )
		res++;
	return res;
}
/* ������������ */
int CSGMW_diagDlg::write_norm_data_to_tbox(TBOX_READ_INFO* p_info,unsigned char id)
{
	unsigned char   idx = 1;
	char			string_buf[32] = {0};
	unsigned long	bin  = 0;
	CString			str;
	CAN_OBJ			can_data = {0};
	char			can_hex[64] = {0};
	unsigned int	hex_len  = 0;
	char*			pch = NULL;
	unsigned char buf[100] = { 0 };
	CString s;

	memset(diag_write.buf, 0 , sizeof(diag_write.buf));
	diag_write.buf[idx++] = p_info->write_id;
	diag_write.buf[idx++] = GET_BYTE_8(p_info->did);
	diag_write.buf[idx++] = GET_BYTE_0(p_info->did);

	UpdateData(true);
	switch( p_info->did )
	{
		case 0xF187:
		case 0xF190:  //VehicleIdentificationNumber
		case 0xF197:
			if (m_edit_write.GetLength() != p_info->len)
			{
				UpdateData(false);
				str.Format("������ַ�����������ӦΪ%d", p_info->len);
				DebugPrintf(NORM_DBG, str, (unsigned char*)&p_info->did, 2, DBG_VAR_TYPE);
				return 0;
			}

			str.Format("׼��д�������Ϊ�� %s", m_edit_write.GetBuffer(p_info->len));
			DebugPrintf(C3_DBG, str, 0, 0, DBG_VAR_TYPE);
			memcpy(&diag_write.buf[idx], m_edit_write.GetBuffer(p_info->len), p_info->len);
			idx += (p_info->len & 0xff);
			break;

		case 0xF189:   //BCD  HEX
		case 0xF18D:
		case 0xF19D:
			if (m_edit_write.GetLength() != (p_info->len*2))
			{
				UpdateData(false);
				str.Format("�����HEX/BCD�ַ�������������ASCLL�ַ���ʾһ���ֽڣ���ӦΪ%d", p_info->len*2);
				DebugPrintf(NORM_DBG, str, (unsigned char*)&p_info->did, 2, DBG_VAR_TYPE);
				return 0;
			}
			
			memcpy(buf, m_edit_write.GetBuffer(p_info->len*2), p_info->len*2);
			
			s.Format("%s", buf);

			for (int iSrc = 0; iSrc < p_info->len*2; iSrc += 2, idx++)
			{
				CString szSub = s.Mid(iSrc, 2);
				diag_write.buf[idx] = (BYTE)_tcstoul(szSub, NULL, 16);
				str.Format("׼��д�������Ϊ�� %x", diag_write.buf[idx]);
				DebugPrintf(C3_DBG, str, 0, 0, DBG_VAR_TYPE);
			}

			idx += (p_info->len & 0xff);
			break;
		
	default:
		UpdateData(false);
		DebugPrintf(NORM_DBG,"write data found unknow did: ",(unsigned char*)&p_info->did,2,DBG_VAR_TYPE);
		return 0;
	}
	UpdateData(false);

	diag_write.buf[0]       = idx - 1;
	diag_write.id			= avm_id;
	diag_write.len			= idx;

	/* ����״̬������ʼ��֤���������� */
	WriteToTboxStart();
	return 0;
}

/* ������������ָ�� */
int CSGMW_diagDlg::write_norm_cmd_to_tbox(TBOX_READ_INFO* p_info,unsigned char id)
{
	unsigned char   idx = 1;
	CAN_OBJ			can_data = {0};
	CString			str;

	can_data.buf[idx++] = p_info->read_id;
	can_data.buf[idx++] = (p_info->did >> 8) & 0xff;
	can_data.buf[idx++] = (p_info->did >> 0) & 0xff;
	can_data.buf[0]     = idx - 1;

	can_data.id				= avm_id;
	if (idx < 8) {
		idx = 8;
	}
	can_data.len			= idx;
	can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
	str.Format("==== read info: id (0x%x) - did (0x%x)",can_data.id,p_info->did);
	DebugPrintf(C4_DBG,str,(unsigned char*)can_data.buf,can_data.len,DBG_DATA_TYPE);
	return 1;
}

int CSGMW_diagDlg::read_norm_cmd_to_tbox(TBOX_READ_INFO* p_info,unsigned char mode)
{
	unsigned char   idx = 1;
	CAN_OBJ			can_data = {0};
	CString			str;


	UpdateData(true);

	if(m_combo_write_mode.GetCurSel())
	{
		can_data.buf[idx++] = p_info->read_id;

		int cursel_0 = m_combo_param0.GetCurSel();
		int cursel_1 = m_combo_param1.GetCurSel();
		int cursel_2 = m_combo_param2.GetCurSel();
		int cursel_3 = m_combo_param3.GetCurSel();
		if( p_info->param0 && cursel_0 != 0xffffffff )
		{
			if(GET_BYTE_24(write_info.param0))
				can_data.buf[idx++] = GET_BYTE_24(write_info.param0);
			if(GET_BYTE_16(write_info.param0) || GET_BYTE_24(write_info.param0))
				can_data.buf[idx++] = GET_BYTE_16(write_info.param0);
			if(GET_BYTE_16(write_info.param0) || GET_BYTE_24(write_info.param0) || GET_BYTE_8(write_info.param0))
				can_data.buf[idx++] = GET_BYTE_8(write_info.param0);
			if(GET_BYTE_16(write_info.param0) || GET_BYTE_24(write_info.param0) || GET_BYTE_8(write_info.param0) || GET_BYTE_0(write_info.param0))
				can_data.buf[idx++] = GET_BYTE_0(write_info.param0);
		}
		if( p_info->param1 && cursel_1 != 0xffffffff )
		{
			if(GET_BYTE_24(write_info.param1))
				can_data.buf[idx++] = GET_BYTE_24(write_info.param1);
			if(GET_BYTE_16(write_info.param1) || GET_BYTE_24(write_info.param1))
				can_data.buf[idx++] = GET_BYTE_16(write_info.param1);
			if(GET_BYTE_16(write_info.param1) || GET_BYTE_24(write_info.param1) || GET_BYTE_8(write_info.param1))
				can_data.buf[idx++] = GET_BYTE_8(write_info.param1);
			if(GET_BYTE_16(write_info.param1) || GET_BYTE_24(write_info.param1) || GET_BYTE_8(write_info.param1) || GET_BYTE_0(write_info.param1))
				can_data.buf[idx++] = GET_BYTE_0(write_info.param1);
		}
		if( p_info->param2 && cursel_2 != 0xffffffff )
		{
			if(GET_BYTE_24(write_info.param2))
				can_data.buf[idx++] = GET_BYTE_24(write_info.param2);
			if(GET_BYTE_16(write_info.param2) || GET_BYTE_24(write_info.param2))
				can_data.buf[idx++] = GET_BYTE_16(write_info.param2);
			if(GET_BYTE_16(write_info.param2) || GET_BYTE_24(write_info.param2) || GET_BYTE_8(write_info.param2))
				can_data.buf[idx++] = GET_BYTE_8(write_info.param2);
			if(GET_BYTE_16(write_info.param2) || GET_BYTE_24(write_info.param2) || GET_BYTE_8(write_info.param2) || GET_BYTE_0(write_info.param2))
				can_data.buf[idx++] = GET_BYTE_0(write_info.param2);
		}
		if( p_info->param3 && cursel_3 != 0xffffffff )
		{
			if(GET_BYTE_24(write_info.param3))
				can_data.buf[idx++] = GET_BYTE_24(write_info.param3);
			if(GET_BYTE_16(write_info.param3) || GET_BYTE_24(write_info.param3))
				can_data.buf[idx++] = GET_BYTE_16(write_info.param3);
			if(GET_BYTE_16(write_info.param3) || GET_BYTE_24(write_info.param3) || GET_BYTE_8(write_info.param3))
				can_data.buf[idx++] = GET_BYTE_8(write_info.param3);
			if(GET_BYTE_16(write_info.param3) || GET_BYTE_24(write_info.param3) || GET_BYTE_8(write_info.param3) || GET_BYTE_0(write_info.param3))
				can_data.buf[idx++] = GET_BYTE_0(write_info.param3);
		}

	}
	else
	{
		can_data.buf[idx++] = p_info->read_id;
		if( p_info->param0 && !m_edit_param0.IsEmpty() )
			can_data.buf[idx++] = comm.SwitchAscToHex(m_edit_param0);
		if( p_info->param1 && !m_edit_param1.IsEmpty() )
			can_data.buf[idx++] = comm.SwitchAscToHex(m_edit_param1);
		if( p_info->param2 && !m_edit_param2.IsEmpty() )
			can_data.buf[idx++] = comm.SwitchAscToHex(m_edit_param2);
		if( p_info->param3 && !m_edit_param3.IsEmpty() )
			can_data.buf[idx++] = comm.SwitchAscToHex(m_edit_param3);
		if( p_info->param4 && !m_edit_param4.IsEmpty() )
			can_data.buf[idx++] = comm.SwitchAscToHex(m_edit_param4);
	}
	
	UpdateData(false);
	can_data.buf[0]         = idx - 1;

	can_data.id			= avm_id;
	can_data.len		= idx + 1;
	can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
	str.Format("==== send cmd to avm: id( 0x%04x )",can_data.id);
	DebugPrintf(C4_DBG,str,can_data.buf,can_data.len,DBG_DATA_TYPE);
	return 1;
}

/* ����diagָ�� */
int CSGMW_diagDlg::send_diag_cmd(int sel,int mode)
{
	int				idx		 =  0;
	CAN_OBJ			can_data = {0};
	TBOX_READ_INFO* p_info   = (TBOX_READ_INFO*)&avm_read_info_list[sel];

	if( (mode & 0x03) == 0x03 )
	{   /* ���ܼ�����������Ҳ��д���� */
		return -1;
	}
	if( mode & 0x01 )
	{   /* ׼�����ж����� */
		if( !(p_info->act_mode & 0x01) )
			return -2;	//��ǰָ�֧�ֶ�����
		switch(p_info->read_id)
		{
		case READ_DATA_BY_IDENTIFIER:
			write_norm_cmd_to_tbox(p_info,p_info->read_id);
			break;
		default:
			DebugPrintf(NORM_DBG,"read found unknown CMD: ",(unsigned char*)&p_info->read_id,1,DBG_VAR_TYPE);
			break;
		}
	}
	else if( mode & 0x02 )
	{   /* ׼������д���� */
		if( !(p_info->act_mode & 0x02) )
			return -3;	//��ǰָ�֧��д����
		switch(p_info->write_id)
		{
		case DIAGNOSITIC_SESSION_CONTROL:
		case ECU_RESET:
		case CLEAR_DIAGNOSTIC_INFORMATION:
		case COMMUNICATION_CONTROL:
		case ROUTINE_CONTROL:
		case TESTER_PRESENT:
		case CONTROL_DTC_SETTING:
		case READ_DTC_INFORMATION:
			/* ����ָ�TBOX */
			read_norm_cmd_to_tbox(p_info,p_info->write_id);
			break;
		case WRITE_DATA_BY_IDENTIFIER:
			/* �������ݵ�TBOX */
			write_norm_data_to_tbox(p_info,p_info->write_id);
			break;
		default:
			DebugPrintf(NORM_DBG,"write found unknown CMD: ",(unsigned char*)&p_info->write_id,1,DBG_VAR_TYPE);
			break;
		}
	}
	return 0;
}


/* �����ֶ�д���CANָ�� */
void CSGMW_diagDlg::send_diag_cmd_manual(CString str)
{
	CAN_OBJ	can_data = {0};
	char buf[512]	= {0};
	char hex[256]	= {0};
	memcpy(buf, str.GetBuffer(), str.GetLength());
	unsigned char hex_len = comm.Ascii2Hex(buf, strlen(buf), hex);

	/* ׼������д���� */
	can_data.id			= avm_id;
	can_data.len		= hex_len;
	memcpy(can_data.buf, (unsigned char*)hex, hex_len);
	can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
	str.Format("==== send cmd to avm: id( 0x%04x )",can_data.id);
	DebugPrintf(C4_DBG,str,can_data.buf,can_data.len,DBG_DATA_TYPE);
}

BOOL CSGMW_diagDlg::into_batch_cmd_list(int sel, int count)
{
	if((avm_read_info_list[sel].act_mode & 0x01) && (count < 5))
	{
		m_dequeBatchCmd.push_back(avm_read_info_list[sel]);

		return TRUE;
	}
	
	return FALSE;
}