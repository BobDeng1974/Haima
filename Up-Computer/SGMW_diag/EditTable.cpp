// EditTable.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SGMW_diag.h"
#include "EditTable.h"
#include "SGMW_diagDlg.h"

// CEditTable

IMPLEMENT_DYNAMIC(CEditTable, CListCtrl)

CEditTable::CEditTable()
{
	m_nItem = 0;		
	m_nSubItem = 0;	
	m_bEditing = false;
}

CEditTable::~CEditTable()
{
}


BEGIN_MESSAGE_MAP(CEditTable, CListCtrl)
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()



// CEditTable ��Ϣ�������

void CEditTable::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	POSITION pos;
	BOOL bSelected = FALSE;
	// ����Ƿ���Item�����༭
	if( m_bEditing == TRUE )
		goto defalt_session;
	// ����Ƿ���Item��ѡ�У�û��ʱ������༭
	pos = GetFirstSelectedItemPosition();
	if( pos )
	{
		// �õ��������Item
		LVHITTESTINFO testinfo;
		testinfo.pt.x = point.x;
		testinfo.pt.y = point.y;            //���ʱ�����λ��
		testinfo.flags = LVHT_ONITEMLABEL;    //����ı����Ǳ���
		if( SubItemHitTest(&testinfo)<0 )
			goto defalt_session;            //û�е�����Ч���򣬲�����༭
		m_nItem = testinfo.iItem;            //�����������к�
		m_nSubItem = testinfo.iSubItem;    //�����������к�
		//if(m_nSubItem == 0)
		//{
		//goto defalt_session;            //ѡ�е�һ�У����༭
		//}
		// ���ñ����Ƿ�ѡ�У�û��ѡ�в�����༭
		while( pos )
		{
			if( m_nItem == GetNextSelectedItem(pos) )
			{
				bSelected = TRUE;
				break;
			}
		}
	
		if( bSelected == FALSE )
			goto defalt_session;            //û�е�����Ч���򣬲��༭

		// ��ʼ�༭
		m_bEditing = BeginEdit();
		return;
	}

defalt_session:
	CListCtrl::OnLButtonDblClk(nFlags, point);
}


//����½�CEdit���󡢻�ȡEdit�������ֵȹ���
bool CEditTable::BeginEdit()
{
    // �õ����༭���������
    CRect rect;
    if( GetSubItemRect(m_nItem, m_nSubItem, LVIR_LABEL, rect) == FALSE )
        return FALSE;
    // �����༭�ؼ�
    int style =     WS_CHILD |
                    WS_CLIPSIBLINGS |
                    WS_EX_TOOLWINDOW |
                    WS_BORDER;
    if( m_edit.Create(style, rect, this, IDC_EDIT_MY) == FALSE )
        return FALSE;
    // ȡ���༭���������
    CString txtItem = GetItemText( m_nItem, m_nSubItem );
    // ȡ����������д���༭�ؼ�
    m_edit.SetWindowText( txtItem );
    m_edit.SetFocus();
    m_edit.SetSel( 0, -1 );
    m_edit.ShowWindow( SW_SHOW );
    return TRUE;
}

//����滻Item���ݡ����ٱ༭��ȹ��ܡ�
void CEditTable::EndEdit( BOOL bValidate )
{
    // �༭�������Ч�ģ����豻�༭���������
    if( bValidate )
    {
        CString txtItem;
        m_edit.GetWindowText( txtItem );
        SetItemText(m_nItem, m_nSubItem, txtItem);

		/* �޸Ķ�Ӧ�����е�ֵ */
		switch(m_nSubItem)
		{
		case 0:
			break;
		case 1:
			can_id_info[m_nItem].can_id = p_dlg->comm.SwitchAscToHex(txtItem);
			break;
		case 2:
			can_id_info[m_nItem].send_period = p_dlg->comm.SwitchAscToInt(txtItem);
			break;
		case 3:
			can_id_info[m_nItem].send_cnt = p_dlg->comm.SwitchAscToInt(txtItem);
			break;
		default:
			break;
		}
    }

    // ���ٱ༭����
    m_edit.DestroyWindow();
    m_bEditing = FALSE;
}
