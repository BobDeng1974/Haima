// DisplayDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SGMW_diag.h"
#include "DisplayDlg.h"
#include "afxdialogex.h"


// CDisplayDlg 对话框

IMPLEMENT_DYNAMIC(CDisplayDlg, CDialogEx)

CDisplayDlg::CDisplayDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDisplayDlg::IDD, pParent)
{

}

CDisplayDlg::CDisplayDlg(CString str, CWnd* pParent)
	: CDialogEx(CDisplayDlg::IDD, pParent)
{
	strDisplay = str;
}

CDisplayDlg::~CDisplayDlg()
{
}

void CDisplayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_DISPLAY, m_editDisplay);
}


BEGIN_MESSAGE_MAP(CDisplayDlg, CDialogEx)
END_MESSAGE_MAP()


// CDisplayDlg 消息处理程序


BOOL CDisplayDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_editDisplay.SetWindowText(strDisplay);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
