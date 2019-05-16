// ItemEdit.cpp : 实现文件
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



// CItemEdit 消息处理程序

BOOL CItemEdit::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	//拦截ESC键和RETURN键按下的消息，解释为WM_KILLFOCUS消息，这里也可以根据需要设置其它键作为输入结束或取消输入的标志
	if( pMsg->message==WM_KEYDOWN )
	{
		if( pMsg->wParam == 13 )        //回车键（ASCII值为13）
			pMsg->message = WM_KILLFOCUS;
		else if( pMsg->wParam == 27 )    //ESC键（ASCII值为27）
		{
			m_bInputValid = FALSE;    //此时的编辑结果无效
			pMsg->message = WM_KILLFOCUS;
		}
	}

	return CEdit::PreTranslateMessage(pMsg);
}


void CItemEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);

	// TODO: 在此处添加消息处理程序代码

	CEditTable *parent = (CEditTable*)GetParent();//
	if( parent )
		parent->EndEdit( m_bInputValid );
	m_bInputValid = TRUE;

}
