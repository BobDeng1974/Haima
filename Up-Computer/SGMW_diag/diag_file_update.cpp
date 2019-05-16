#include "stdafx.h"
#include "SGMW_diag.h"
#include "SGMW_diagDlg.h"
#include "afxdialogex.h"
#include "diag.h"

#include "winTest.h"


void CSGMW_diagDlg::OnBnClickedButtonSendFile()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int m_isDiff, m_codetype;
	CString m_strname, filtStr;

	m_isDiff = m_comb_diffud.GetCurSel();
	m_codetype = m_comb_fwtype.GetCurSel();

	if (m_codetype == 0)
		filtStr = "BIN Files (*.bin)|*.bin|HEX Files (*.hex)|*.hex|S19 Files (*.S19)|*.S19|All Files(*.*)|*.*||";
	else
		filtStr = "All Files(*.*)|*.*||";

	CFileDialog cFileDlg(TRUE,"","",OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,filtStr);

	if(IDOK == cFileDlg.DoModal())
	{
		m_strname = cFileDlg.GetFileName();

		if(m_isDiff == 0){
			if(m_codetype == 0){
				if(m_strname != "Extend_360_Pro.bin"){
					MessageBox("����ļ����������������Ƿ���ȷ","����",MB_OK|MB_ICONQUESTION);
					return;
				}
			}else if(m_codetype == 1){
				if(m_strname != "t6a_app"){
					MessageBox("����ļ����������������Ƿ���ȷ","����",MB_OK|MB_ICONQUESTION);
					return;
				}
			}else if(m_codetype == 2){
				if(m_strname != "suanfa.tar.bz2"){
					MessageBox("����ļ����������������Ƿ���ȷ","����",MB_OK|MB_ICONQUESTION);
					return;
				}
			}else if(m_codetype ==3){
				if(m_strname != "zImage"){
					MessageBox("����ļ����������������Ƿ���ȷ","����",MB_OK|MB_ICONQUESTION);
					return;
				}
			}else if(m_codetype ==4){
				if(m_strname != "imx6q-sabresd.dtb"){
					MessageBox("����ļ����������������Ƿ���ȷ","����",MB_OK|MB_ICONQUESTION);
					return;
				}
			}else if(m_codetype ==5){
				if(m_strname != "u-boot.imx"){
					MessageBox("����ļ����������������Ƿ���ȷ","����",MB_OK|MB_ICONQUESTION);
					return;
				}
			}
			else if (m_codetype == 6) {
				if (m_strname != "Extend_360_Pro_Boot.bin") {
					MessageBox("����ļ����������������Ƿ���ȷ", "����", MB_OK | MB_ICONQUESTION);
					return;
				}
			}
			
		}else if(m_isDiff == 1){
			if(m_codetype == 1){
				if(m_strname != "t6a_app.patch"){
					MessageBox("����ļ����������������Ƿ���ȷ","����",MB_OK|MB_ICONQUESTION);
					return;
				}
			}else if(m_codetype == 2){
				if(m_strname != "suanfa.tar.bz2.patch"){
					MessageBox("����ļ����������������Ƿ���ȷ","����",MB_OK|MB_ICONQUESTION);
					return;
				}
			}else if(m_codetype ==3){
				if(m_strname != "zImage.patch"){
					MessageBox("����ļ����������������Ƿ���ȷ","����",MB_OK|MB_ICONQUESTION);
					return;
				}
			}else if(m_codetype ==4){
				if(m_strname != "imx6q-sabresd.dtb.patch"){
					MessageBox("����ļ����������������Ƿ���ȷ","����",MB_OK|MB_ICONQUESTION);
					return;
				}
			}else if(m_codetype ==5){
				if(m_strname != "u-boot.imx.patch"){
					MessageBox("����ļ����������������Ƿ���ȷ","����",MB_OK|MB_ICONQUESTION);
					return;
				}
			}
		}



		CString str;
		CString str_path;

		str = cFileDlg.GetPathName();
		str_path.Format("�����ļ���%s", str.GetBuffer());
		((CStatic*)GetDlgItem(IDC_STATIC_FILE_PATH))->SetWindowTextA(str_path);
		m_strPath = str;
	}


}


void CSGMW_diagDlg::OnBnClickedButtonStartUpdate()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	InitParam();

#ifndef WIN_TEST
	if( can_driver.m_can_link_status != CAN_CONNECT_OK )
	{
		MessageBox("��������CanBus!", "����", MB_OK|MB_ICONQUESTION);
		return;
	}
#endif


	if(m_strPath == "")
	{
		MessageBox("���ȼ����ļ�", "��ʾ", MB_OK|MB_ICONQUESTION);	
	}

	if(comm.MyIsFileExists(m_strPath))
	{
		if(GetFileInfo(m_strPath, m_dwCRC, m_file_size))
		{
			WriteToTboxErr("�����ļ�ʧ��");
			return;
		}

		//if( m_file_size % (64*1024))
		//	m_module_num = m_file_size/(64*1024) + 1;
		//else
		//	m_module_num = m_file_size/(64*1024);

		m_btn_update.EnableWindow(FALSE);  //����������ť����ֹ�ļ��ظ���

		CString info;
		CString log_str;
		info.Format("��ʼ��ʱ��0��");
		((CStatic*)GetDlgItem(IDC_STATIC_TIMER))->SetWindowTextA(info);

		m_cProgress.SetRange32(0, m_file_size);
		m_cProgress.SetPos(0);

		log_str.Format("======== begin update ========");
		WaitForSingleObject(p_dlg->h_log_mutex,INFINITE);	//��ȡ�ź���
		p_dlg->log.FlushLog(log_str);
		ReleaseMutex(p_dlg->h_log_mutex);					//�ͷ��ź���

		Start_Update();
	}
	else
	{
		WriteToTboxErr("�����ļ�������");
	}
}

int CSGMW_diagDlg::GetFileInfo(CString str , DWORD &dwcrc , DWORD &size)
{
	if(!comm.MyIsFileExists(str)){
		CString str;
		str.Format("�ļ�·������%s",str.GetBuffer());
		return -1;
	}

	CFile pFile(str , CFile::modeRead);
	ULONGLONG ull = pFile.GetLength();
	if(ull > FILE_MAX_SIZE){
		WriteToTboxErr("�ļ���С��������!!!");
		pFile.Close();
		return -1;
	}

	size = (DWORD)ull;

	pFile.Close();
	get_OBDupgradefile_CRC32(&dwcrc, str);

	//CString check;
	//check.Format("check code: %u", dwcrc);
	//MessageBox(check,"����",MB_OK|MB_ICONQUESTION);

	return 0;
}


void CSGMW_diagDlg::Start_Update() 
{
	handle_flag = 1;
	handle_type = UPDATE_TYPE;
	diag_wait_time		  = GetTickCount();
	diag_start_tm		  = GetTickCount();  

	diag_update_step	  = UPDATA_TO_AVM_ENTER_EXTMODE;

	DebugPrintf(NORM_DBG,"��ʼ����",0,0,DBG_VAR_TYPE);

	//(CButton*)GetDlgItem(IDC_BUTTON_READ_DID)->EnableWindow(FALSE);
	//(CButton*)GetDlgItem(IDC_BUTTON_BATCH_READ_DID)->EnableWindow(FALSE);
	//(CButton*)GetDlgItem(IDC_BUTTON_WRITE_DID)->EnableWindow(FALSE);
	//(CButton*)GetDlgItem(IDC_BUTTON_CANWRITE)->EnableWindow(FALSE);
	/*ShowOrHideControl(handle_flag);*/

	CAN_OBJ		can_buf  = {0};
	pFile.Open(m_strPath , CFile::modeRead);
	UpdateToTbox(can_buf);
}