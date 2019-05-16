#pragma once
#include "ItemEdit.h"

// CEditTable

class CEditTable : public CListCtrl
{
	DECLARE_DYNAMIC(CEditTable)

public:
	CEditTable();
	virtual ~CEditTable();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

public:
	bool BeginEdit();
	void EndEdit( BOOL bValidate );

public:
	CItemEdit  m_edit;
	int m_nItem;		//被编辑表项的行号
	int m_nSubItem;		//被编辑表项的列号
	bool m_bEditing;	//判断表项是否正在被编辑
	
	
};


