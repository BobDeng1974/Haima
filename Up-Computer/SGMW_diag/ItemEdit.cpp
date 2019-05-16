// ItemEdit.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SGMW_diag.h"
#include "ItemEdit.h"
#include "EditTable.h"

// CItemEdit

IMPLEMENT_DYNAMIC(CItemEdit, CEdit)

CItemEdit::CItemEdit()
{
	m_bInputValid = false;
}

CItemEdit::~CItemEdit()
{
}


BEGIN_MESSAGE_MAP(CItemEdit, CEdit)
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()



// CItemEdit ��Ϣ�������

BOOL CItemEdit::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���

	//����ESC����RETURN�����µ���Ϣ������ΪWM_KILLFOCUS��Ϣ������Ҳ���Ը�����Ҫ������������Ϊ���������ȡ������ı�־
	if( pMsg->message==WM_KEYDOWN )
	{
		if( pMsg->wParam == 13 )        //�س�����ASCIIֵΪ13��
			pMsg->message = WM_KILLFOCUS;
		else if( pMsg->wParam == 27 )    //ESC����ASCIIֵΪ27��
		{
			m_bInputValid = FALSE;    //��ʱ�ı༭�����Ч
			pMsg->message = WM_KILLFOCUS;
		}
	}

	return CEdit::PreTranslateMessage(pMsg);
}


void CItemEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);

	// TODO: �ڴ˴������Ϣ����������

	CEditTable *parent = (CEditTable*)GetParent();//
	if( parent )
		parent->EndEdit( m_bInputValid );
	m_bInputValid = TRUE;

}
