#pragma once
#include "afxwin.h"


// CDisplayDlg �Ի���

class CDisplayDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDisplayDlg)

public:
	CDisplayDlg(CWnd* pParent = NULL);   // ��׼���캯��
	CDisplayDlg(CString str, CWnd* pParent = NULL);
	virtual ~CDisplayDlg();

// �Ի�������
	enum { IDD = IDD_DISPLAY_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CString strDisplay;
	CEdit m_editDisplay;
};
