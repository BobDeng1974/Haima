#pragma once


// CItemEdit

class CItemEdit : public CEdit
{
	DECLARE_DYNAMIC(CItemEdit)

public:
	CItemEdit();
	virtual ~CItemEdit();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnKillFocus(CWnd* pNewWnd);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

public:
	bool m_bInputValid;		//�жϱ༭����Ƿ���Ч
};


