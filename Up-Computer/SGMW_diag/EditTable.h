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
	int m_nItem;		//���༭������к�
	int m_nSubItem;		//���༭������к�
	bool m_bEditing;	//�жϱ����Ƿ����ڱ��༭
	
	
};


