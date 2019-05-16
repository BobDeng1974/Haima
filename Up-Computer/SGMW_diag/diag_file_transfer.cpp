#include "stdafx.h"
#include "SGMW_diag.h"
#include "SGMW_diagDlg.h"
#include "afxdialogex.h"
#include "diag.h"
#include "crc32.h"
#include "time.h"



/* 获取文件列表 */
void CSGMW_diagDlg::OnBnClickedButtonGetFileList()
{
	// TODO: 在此添加控件通知处理程序代码

	InitParam();

	if( can_driver.m_can_link_status != CAN_CONNECT_OK )
	{
		MessageBox("请先启动CanBus!", "警告", MB_OK|MB_ICONQUESTION);
		return;
	}

	CString info;
	info.Format("开始计时：0秒");
	((CStatic*)GetDlgItem(IDC_STATIC_TIMER))->SetWindowTextA(info);

	trans_mod = TRANS_LIST;
	Start_Transfer(TRANS_FROM_TBOX_EXTERN_MODE);
}

/* 获取文件 */
void CSGMW_diagDlg::OnBnClickedButtonFileTransfer()
{
	// TODO: 在此添加控件通知处理程序代码
	
	InitParam();

	if( can_driver.m_can_link_status != CAN_CONNECT_OK )
	{
		MessageBox("请先启动CanBus!", "警告", MB_OK|MB_ICONQUESTION);
		return;
	}

	if(m_dequeSelFile.empty())
	{
		MessageBox("请先选中需传输的文件!", "警告", MB_OK|MB_ICONQUESTION);
		return;
	}

	CString info;
	info.Format("开始计时：0秒");
	((CStatic*)GetDlgItem(IDC_STATIC_TIMER))->SetWindowTextA(info);

	CheckDirectory(GET_FILE_DIRECTORY);

	trans_mod = TRANS_FILE;
	Start_Transfer(TRANS_FROM_TBOX_EXTERN_MODE);
}

/* 删除文件 */
void CSGMW_diagDlg::OnBnClickedButtonFileDelete()
{
	// TODO: 在此添加控件通知处理程序代码

	InitParam();

	if( can_driver.m_can_link_status != CAN_CONNECT_OK )
	{
		MessageBox("请先启动CanBus!", "警告", MB_OK|MB_ICONQUESTION);
		return;
	}

	if(m_dequeSelFile.empty())
	{
		MessageBox("请先选中需删除的文件!", "警告", MB_OK|MB_ICONQUESTION);
		m_list_sel_file.DeleteAllItems();
		return;
	}

	trans_mod = DELETE_FILE;
	Start_Transfer(TRANS_FROM_TBOX_EXTERN_MODE);
}


void CSGMW_diagDlg::Start_Transfer(int step)
{
	handle_flag				= 1;
	handle_type = TRANSFER_TYPE;
	diag_wait_time			= GetTickCount();
	diag_transfer_step		= step;//TRANS_FROM_TBOX_EXTERN_MODE;
	switch(trans_mod)
	{
	case TRANS_NULL:
		break;
	case TRANS_LIST:
		diag_start_tm		= GetTickCount();
		DebugPrintf(NORM_DBG,"开始传输文件列表",0,0,DBG_VAR_TYPE);
		break;
	case TRANS_FILE:
		diag_start_tm		= GetTickCount();
		DebugPrintf(NORM_DBG,"开始传输文件",0,0,DBG_VAR_TYPE);
		break;
	case DELETE_FILE:
		DebugPrintf(NORM_DBG,"开始删除文件",0,0,DBG_VAR_TYPE);
		break;
	default:
		DebugPrintf(NORM_DBG,"未知的传输模式",0,0,DBG_VAR_TYPE);
		break;
	}
	
	diag_start_tm		  = GetTickCount();
	DebugPrintf(NORM_DBG,"开始传输",0,0,DBG_VAR_TYPE);

	ShowOrHideControl(handle_flag);

	CAN_OBJ		can_buf  = {0};
	TransferFromTbox(can_buf);
}


/* 显示或隐藏控件 */
void CSGMW_diagDlg::ShowOrHideControl(bool var)
{
	bool flag = var;

	m_comb_did.ShowWindow(!flag);
	m_list_batch_cmd.ShowWindow(!flag);
	(CButton*)GetDlgItem(IDC_BUTTON_READ_DID)->ShowWindow(!flag);
	(CButton*)GetDlgItem(IDC_BUTTON_BATCH_READ_DID)->ShowWindow(!flag);
	(CButton*)GetDlgItem(IDC_BUTTON_WRITE_DID)->ShowWindow(!flag);
	(CButton*)GetDlgItem(IDC_BUTTON_CANWRITE)->ShowWindow(!flag);
	m_edit_canwrite.ShowWindow(!flag);

	m_combo_write_mode.ShowWindow(!flag);
	if(m_combo_write_mode.GetCurSel())
	{
		m_combo_param0.ShowWindow(!flag);
		m_combo_param1.ShowWindow(!flag);
		m_combo_param2.ShowWindow(!flag);
		m_combo_param3.ShowWindow(!flag);
	}
	else
	{
		(CEdit*)GetDlgItem(IDC_EDIT_WRITE)->ShowWindow(!flag);
		(CEdit*)GetDlgItem(IDC_EDIT_PARAM0)->ShowWindow(!flag);
		(CEdit*)GetDlgItem(IDC_EDIT_PARAM1)->ShowWindow(!flag);
		(CEdit*)GetDlgItem(IDC_EDIT_PARAM2)->ShowWindow(!flag);
		(CEdit*)GetDlgItem(IDC_EDIT_PARAM3)->ShowWindow(!flag);
		(CEdit*)GetDlgItem(IDC_EDIT_PARAM4)->ShowWindow(!flag);
	}
	
	m_comb_file_list.ShowWindow(flag);
	m_list_sel_file.ShowWindow(flag);
	(CStatic*)GetDlgItem(IDC_STATIC_FILE_LIST)->ShowWindow(flag);
	(CButton*)GetDlgItem(IDC_BUTTON_FILE_TRANSFER)->ShowWindow(flag);
	(CButton*)GetDlgItem(IDC_BUTTON_FILE_DELETE)->ShowWindow(flag);
}

void CSGMW_diagDlg::OnCbnSelchangeComboFileList()
{
	// TODO: 在此添加控件通知处理程序代码
	
	CString str;
	int iCurSel = m_comb_file_list.GetCurSel();
	if(iCurSel)
	{
		if(!m_dequeSelFile.empty())
		{
			for(size_t i=0; i<m_dequeSelFile.size(); ++i)
			{
				if(m_dequeSelFile.at(i).file_serial == m_dequeFileList.at(iCurSel-1).file_serial)
				{
					DebugPrintf(NORM_DBG,"文件已被选中，无需重复操作",0,0,DBG_VAR_TYPE);
					return;
				}
			}
		}
	
		m_dequeSelFile.push_back(m_dequeFileList.at(iCurSel-1));
		int iListCount = m_list_sel_file.GetItemCount();
		str.Format("%d", m_dequeFileList.at(iCurSel-1).file_serial);
		m_list_sel_file.InsertItem(iListCount, str);
		str.Format("%s", m_dequeFileList.at(iCurSel-1).file_name);
		m_list_sel_file.SetItemText(iListCount, 1, str);
		str.Format("%d", m_dequeFileList.at(iCurSel-1).file_length);
		m_list_sel_file.SetItemText(iListCount, 2, str);
	}
	else
	{
		if(!m_dequeSelFile.empty())
		{
			m_dequeSelFile.clear();
			m_list_sel_file.DeleteAllItems();
		}

		for(size_t i=0; i<m_dequeFileList.size(); ++i)
		{
			m_dequeSelFile.push_back(m_dequeFileList.at(i));
			
			str.Format("%d", m_dequeFileList.at(i).file_serial);
			m_list_sel_file.InsertItem(i, str);
			str.Format("%s", m_dequeFileList.at(i).file_name);
			m_list_sel_file.SetItemText(i, 1, str);
			str.Format("%d", m_dequeFileList.at(i).file_length);
			m_list_sel_file.SetItemText(i, 2, str);
		}
	}
}