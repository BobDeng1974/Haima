// DisplayDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SGMW_diag.h"
#include "DisplayDlg.h"
#include "afxdialogex.h"


// CDisplayDlg �Ի���

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


// CDisplayDlg ��Ϣ�������


BOOL CDisplayDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	m_editDisplay.SetWindowText(strDisplay);

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}
