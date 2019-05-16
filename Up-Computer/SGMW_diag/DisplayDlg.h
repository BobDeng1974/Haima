#pragma once
#include "afxwin.h"


// CDisplayDlg 对话框

class CDisplayDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDisplayDlg)

public:
	CDisplayDlg(CWnd* pParent = NULL);   // 标准构造函数
	CDisplayDlg(CString str, CWnd* pParent = NULL);
	virtual ~CDisplayDlg();

// 对话框数据
	enum { IDD = IDD_DISPLAY_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CString strDisplay;
	CEdit m_editDisplay;
};
