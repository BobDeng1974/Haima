
// SGMW_diagDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SGMW_diag.h"
#include "SGMW_diagDlg.h"
#include "afxdialogex.h"

#include "winTest.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static bool test_thread_exit_flag = false;	//测试线程退出标志

deque<TBOX_READ_INFO> m_dequeBatchCmd;
deque<TBOX_FILE_LIST> m_dequeFileList;
deque<TBOX_FILE_LIST> m_dequeSelFile;
deque<TBOX_CAN_ID_INFO>	m_dequeCanInfo;


CSGMW_diagDlg* p_dlg = NULL;


TBOX_CAN_ID_INFO can_id_info[]	=
{
	{0x0165,	10,		UNLIMIT},
	{0x0155,	10,		UNLIMIT},
	{0x04C5,	100,	UNLIMIT},
	{0x036A,	100,	UNLIMIT},
	{0x04C7,	100,	UNLIMIT},
	{0x0284,	100,	UNLIMIT},
	{0x0287,	50,		UNLIMIT},
	{0x0297,	50,		UNLIMIT},
	{0x028C,	50,		UNLIMIT},
	{0x028B,	50,		UNLIMIT},
	{0x029A,	50,		UNLIMIT},
	{0x029B,	100,	UNLIMIT},
	{0x0295,	100,	UNLIMIT},
	{0x0289,	100,	UNLIMIT},
	{0x029E,	100,	UNLIMIT},
	{0x05D6,	5000,	UNLIMIT},
	{0x05D7,	5000,	UNLIMIT},
	{0x05DD,	5000,	UNLIMIT},
	{0x05C6,	2000,	UNLIMIT},
	{0x05C7,	2000,	UNLIMIT},
	{0x016C,	10,		UNLIMIT},
	{0x016E,	10,		UNLIMIT},
	{0x02AE,	100,	UNLIMIT},
	{0x0120,	100,	UNLIMIT},
	{0x01E5,	10,		UNLIMIT},
	{0x0141,	100,	UNLIMIT},
	{0x00E1,	500,	UNLIMIT},
	{0x01E9,	20,		UNLIMIT},
	{0x017D,	100,	UNLIMIT},
	{0x01C6,	20,		UNLIMIT},
	{0x0348,	10,		UNLIMIT},
	{0x034A,	10,		UNLIMIT},
	{0x0128,	20,		UNLIMIT},
	{0x05E3,	20,		UNLIMIT},
	{0x014A,	10,		UNLIMIT},
	{0x0230,	50,		UNLIMIT},
	{0x04f6,	100,	UNLIMIT},
	{0x04d5,	100,	UNLIMIT},
	{0x0413,	100,	UNLIMIT},
	
};

/* 微秒级延时 */
void CSGMW_diagDlg::DelayUs(int uDelay)  
{  
  
    LARGE_INTEGER litmp;  
    LONGLONG QPart1,QPart2;  
  
    double dfMinus,dfFreq,dfTim;  
      
    /* 
        Pointer to a variable that the function sets, in counts per second, to the current performance-counter frequency.  
        If the installed hardware does not support a high-resolution performance counter,  
        the value passed back through this pointer can be zero.  
 
    */  
    QueryPerformanceFrequency(&litmp);  
  
    dfFreq = (double)litmp.QuadPart;  
  
    /* 
        Pointer to a variable that the function sets, in counts, to the current performance-counter value.  
    */  
    QueryPerformanceCounter(&litmp);  
  
    QPart1 = litmp.QuadPart;  
    do  
    {  
		QueryPerformanceCounter(&litmp);  
		QPart2 = litmp.QuadPart;  

		dfMinus = (double)(QPart2-QPart1);

		if(dfMinus < 0)
			break;

		dfTim = dfMinus/dfFreq;  
     }while(dfTim<0.000001 * uDelay);  
}  

#if 1
/* 单线程处理 */
UINT CSGMW_diagDlg::TestThreadProc(void *param)
{
	CSGMW_diagDlg		*p_diag			= (CSGMW_diagDlg*)param;
	//ECanDriver			*p_can			= (ECanDriver*)&p_diag->can_driver;
	//CAN_OBJ				can_sour_data	= {0};
	//CString				str				= "";
	DWORD				wait_time		= GetTickCount();
	DWORD				cnt				= 0;

	/* 初始化wait_time_list */
	DWORD* wait_time_list = new DWORD[m_dequeCanInfo.size()];
	for(size_t i = 0; i < m_dequeCanInfo.size(); ++i)
	{
		wait_time_list[i] = wait_time;
	}

	while(1)
	{
		if(test_thread_exit_flag)
			break;

		for(size_t i = 0; i < m_dequeCanInfo.size(); ++i)
		{
			if( 0xffffffff == m_dequeCanInfo[i].send_cnt )
			{
				if( (GetTickCount() - wait_time_list[i]) >= m_dequeCanInfo[i].send_period )
				{
					p_diag->SendTestCanInfo(p_diag, m_dequeCanInfo[i].can_id);
					wait_time_list[i] = GetTickCount();

				}
			}
			else
			{
				if( cnt < m_dequeCanInfo[i].send_cnt )
				{
					p_diag->SendTestCanInfo(p_diag, m_dequeCanInfo[i].can_id);
					wait_time_list[i] = GetTickCount();
					cnt++;

				}
				else
				{
					break;
				}
			}

		}

	}

	delete[] wait_time_list; 

	return 0;
}
#else 
/* 多线程处理 */
UINT CSGMW_diagDlg::TestThreadProc(void *param)
{
	ECanDriver			*p_can			= (ECanDriver*)&p_dlg->can_driver;
	CAN_OBJ				can_sour_data	= {0};
	CString				str				= "";
	int					wait_time		= GetTickCount();
	DWORD				cnt				= 0;

	//TBOX_CAN_ID_INFO* can_id_info = (TBOX_CAN_ID_INFO*)lpParam;
	TBOX_CAN_ID_INFO* can_id_info = (TBOX_CAN_ID_INFO*)param;
	if( 0xffffffff == can_id_info->send_cnt )	//不限次数循环
	{
		while(1)
		{
			if(test_thread_exit_flag)
				break;

			if( (GetTickCount() - wait_time) >= can_id_info->send_period )
			{
				if( 0x04c5 == can_id_info->can_id )
				{
					p_dlg->GetDlgItem(IDC_EDIT_VAR)->GetWindowTextA(str);
					can_sour_data.id  = 0x04c5;
					can_sour_data.len = 8;
					can_sour_data.buf[0] = 0x00;
					can_sour_data.buf[1] = 0x08;
					if(!str.IsEmpty())
						can_sour_data.buf[1] = p_dlg->comm.SwitchAscToHex(str);
					can_sour_data.buf[2] = 0x55;
					can_sour_data.buf[3] = 0x55;
					can_sour_data.buf[4] = 0x55;
					can_sour_data.buf[5] = 0x55;
					can_sour_data.buf[6] = 0x55;
					can_sour_data.buf[7] = 0x55;
				}
				else
				{
					can_sour_data.id  = can_id_info->can_id;
					can_sour_data.len = 8;
					can_sour_data.buf[0] = 0x00;
					can_sour_data.buf[1] = 0x00;
					can_sour_data.buf[2] = 0x55;
					can_sour_data.buf[3] = 0x55;
					can_sour_data.buf[4] = 0x55;
					can_sour_data.buf[5] = 0x55;
					can_sour_data.buf[6] = 0x55;
					can_sour_data.buf[7] = 0x55;
				}

				p_can->TransmitData(can_sour_data.id, can_sour_data.buf, can_sour_data.len);

				wait_time = GetTickCount();

				//Sleep(can_id_info->send_period);
				//DelayUs( can_id_info->send_period * 1000 );
			}
			else
			{
				Sleep(1);
			}
		}
	}
	else	//限制次数循环
	{
		while(cnt < can_id_info->send_cnt)
		{
			if(test_thread_exit_flag)
				break;

			if( (GetTickCount() - wait_time) >= can_id_info->send_period )
			{
				if( 0x04c5 == can_id_info->can_id )
				{
					p_dlg->GetDlgItem(IDC_EDIT_VAR)->GetWindowTextA(str);
					can_sour_data.id  = 0x04c5;
					can_sour_data.len = 8;
					can_sour_data.buf[0] = 0x00;
					can_sour_data.buf[1] = 0x08;
					if(!str.IsEmpty())
						can_sour_data.buf[1] = p_dlg->comm.SwitchAscToHex(str);
					can_sour_data.buf[2] = 0x55;
					can_sour_data.buf[3] = 0x55;
					can_sour_data.buf[4] = 0x55;
					can_sour_data.buf[5] = 0x55;
					can_sour_data.buf[6] = 0x55;
					can_sour_data.buf[7] = 0x55;
				}
				else
				{
					can_sour_data.id  = can_id_info->can_id;
					can_sour_data.len = 8;
					can_sour_data.buf[0] = 0x00;
					can_sour_data.buf[1] = 0x00;
					can_sour_data.buf[2] = 0x55;
					can_sour_data.buf[3] = 0x55;
					can_sour_data.buf[4] = 0x55;
					can_sour_data.buf[5] = 0x55;
					can_sour_data.buf[6] = 0x55;
					can_sour_data.buf[7] = 0x55;
				}

				p_can->TransmitData(can_sour_data.id, can_sour_data.buf, can_sour_data.len);
				cnt++;

				wait_time = GetTickCount();

				//Sleep(can_id_info->send_period);
				//DelayUs( can_id_info->send_period * 1000 );
			}
			else
			{
				Sleep(1);
			}

		}
	}


	return 0;
}
#endif

void CSGMW_diagDlg::SendTestCanInfo(CSGMW_diagDlg *p_diag, WORD can_id)
{
	ECanDriver			*p_can			= (ECanDriver*)&p_diag->can_driver;
	CAN_OBJ				can_sour_data	= {0};
	CString				str				= "";

	if( 0x04c5 == can_id )
	{
		p_diag->GetDlgItem(IDC_EDIT_VAR)->GetWindowTextA(str);
		can_sour_data.id  = 0x04c5;
		can_sour_data.len = 8;
		can_sour_data.buf[0] = 0x00;
		can_sour_data.buf[1] = 0x08;
		if(!str.IsEmpty())
			can_sour_data.buf[1] = p_diag->comm.SwitchAscToHex(str);
		can_sour_data.buf[2] = 0x55;
		can_sour_data.buf[3] = 0x55;
		can_sour_data.buf[4] = 0x55;
		can_sour_data.buf[5] = 0x55;
		can_sour_data.buf[6] = 0x55;
		can_sour_data.buf[7] = 0x55;
	}
	else
	{
		can_sour_data.id  = can_id;
		can_sour_data.len = 8;
		can_sour_data.buf[0] = 0x00;
		can_sour_data.buf[1] = 0x00;
		can_sour_data.buf[2] = 0x55;
		can_sour_data.buf[3] = 0x55;
		can_sour_data.buf[4] = 0x55;
		can_sour_data.buf[5] = 0x55;
		can_sour_data.buf[6] = 0x55;
		can_sour_data.buf[7] = 0x55;
	}

	p_can->TransmitData(can_sour_data.id, can_sour_data.buf, can_sour_data.len);

}



// CSGMW_diagDlg 对话框


CSGMW_diagDlg::CSGMW_diagDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSGMW_diagDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	p_dlg = this;

	InitializeCriticalSection(&m_csLog);//初始化临界区
}

void CSGMW_diagDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_SEL_DID	, m_comb_did		);		//通过DID读取信息列表
	DDX_Control(pDX, IDC_COMBO_DEBUG	, m_comb_debug		);		//打印信息列表
	DDX_Control(pDX, IDC_COMBO_DIFFUD	, m_comb_diffud		);		//差分升级
	DDX_Control(pDX, IDC_COMBO_FWTYPE	, m_comb_fwtype		);		//程序类型

	DDX_Control(pDX, IDC_BUTTON_START_UPDATE, m_btn_update	);

	DDX_Control(pDX, IDC_LIST_DID		, m_list_did		);		//通过DID读取信息列表
	DDX_Control(pDX, IDC_LIST_CMD		, m_list_cmd		);		//通过指令读取信息列表

	DDX_Text(pDX,	 IDC_EDIT_WRITE		, m_edit_write		);		//写入数据
	DDX_Text(pDX,	 IDC_EDIT_PARAM0	, m_edit_param0		);		//参数0
	DDX_Text(pDX,	 IDC_EDIT_PARAM1	, m_edit_param1		);		//参数1
	DDX_Text(pDX,	 IDC_EDIT_PARAM2	, m_edit_param2		);		//参数2
	DDX_Text(pDX,	 IDC_EDIT_PARAM3	, m_edit_param3		);		//参数3
	DDX_Text(pDX,	 IDC_EDIT_PARAM4	, m_edit_param4		);		//参数4

	DDX_Text(pDX,	 IDC_EDIT_PC_ID		,m_edit_pc_id		);
	DDX_Text(pDX,	 IDC_EDIT_AVM_ID	,m_edit_avm_id		);
	DDX_Control(pDX, IDC_LIST_BATCH_CMD, m_list_batch_cmd	);
	DDX_Control(pDX, IDC_EDIT_CANWRITE, m_edit_canwrite);
	DDX_Control(pDX, IDC_PROGRESS_UPDATE_FILE, m_cProgress);
	DDX_Control(pDX, IDC_COMBO_FILE_LIST, m_comb_file_list);
	DDX_Control(pDX, IDC_LIST_SEL_FILE_LIST, m_list_sel_file);
	DDX_Control(pDX, IDC_CHECK_PRINT, m_check_print);
	DDX_Control(pDX, IDC_COMBO_WRITE_MODE, m_combo_write_mode);
	DDX_Control(pDX, IDC_COMBO_PARAM0, m_combo_param0);
	DDX_Control(pDX, IDC_COMBO_PARAM1, m_combo_param1);
	DDX_Control(pDX, IDC_COMBO_PARAM2, m_combo_param2);
	DDX_Control(pDX, IDC_COMBO_PARAM3, m_combo_param3);
	DDX_Control(pDX, IDC_CHECK_VAR, m_check_var);
}

BEGIN_MESSAGE_MAP(CSGMW_diagDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CSGMW_diagDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CSGMW_diagDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_CAN_CONNECT, &CSGMW_diagDlg::OnBnClickedButtonCanConnect)
	ON_BN_CLICKED(IDC_BUTTON_READ_DID, &CSGMW_diagDlg::OnBnClickedButtonReadDid)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_DID, &CSGMW_diagDlg::OnBnClickedButtonWriteDid)
	ON_CBN_SELCHANGE(IDC_COMBO_SEL_DID, &CSGMW_diagDlg::OnCbnSelchangeComboSelDid)
	ON_BN_CLICKED(IDC_BUTTON_BATCH_READ_DID, &CSGMW_diagDlg::OnBnClickedButtonBatchReadDid)
	ON_COMMAND(ID_CMD_32771, &CSGMW_diagDlg::OnCmd32771)
	ON_COMMAND(ID_CMD_32772, &CSGMW_diagDlg::OnCmd32772)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_BATCH_CMD, &CSGMW_diagDlg::OnNMRClickListBatchCmd)
	ON_BN_CLICKED(IDC_BUTTON_CANWRITE, &CSGMW_diagDlg::OnBnClickedButtonCanwrite)
	ON_BN_CLICKED(IDC_BUTTON_SEND_FILE, &CSGMW_diagDlg::OnBnClickedButtonSendFile)
	ON_BN_CLICKED(IDC_BUTTON_START_UPDATE, &CSGMW_diagDlg::OnBnClickedButtonStartUpdate)
	ON_BN_CLICKED(IDC_BUTTON_CLS, &CSGMW_diagDlg::OnBnClickedButtonCls)
	ON_BN_CLICKED(IDC_BUTTON_GET_FILE_LIST, &CSGMW_diagDlg::OnBnClickedButtonGetFileList)
	ON_BN_CLICKED(IDC_BUTTON_FILE_TRANSFER, &CSGMW_diagDlg::OnBnClickedButtonFileTransfer)
	ON_CBN_SELCHANGE(IDC_COMBO_FILE_LIST, &CSGMW_diagDlg::OnCbnSelchangeComboFileList)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SEL_FILE_LIST, &CSGMW_diagDlg::OnNMRClickListSelFileList)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CSGMW_diagDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_FILE_DELETE, &CSGMW_diagDlg::OnBnClickedButtonFileDelete)
	ON_CBN_SELCHANGE(IDC_COMBO_PARAM0, &CSGMW_diagDlg::OnCbnSelchangeComboParam0)
	ON_CBN_SELCHANGE(IDC_COMBO_PARAM1, &CSGMW_diagDlg::OnCbnSelchangeComboParam1)
	ON_CBN_SELCHANGE(IDC_COMBO_PARAM2, &CSGMW_diagDlg::OnCbnSelchangeComboParam2)
	ON_CBN_SELCHANGE(IDC_COMBO_PARAM3, &CSGMW_diagDlg::OnCbnSelchangeComboParam3)
	ON_CBN_SELCHANGE(IDC_COMBO_WRITE_MODE, &CSGMW_diagDlg::OnCbnSelchangeComboWriteMode)
	ON_LBN_DBLCLK(IDC_LIST_CMD, &CSGMW_diagDlg::OnLbnDblclkListCmd)
	ON_LBN_DBLCLK(IDC_LIST_DID, &CSGMW_diagDlg::OnLbnDblclkListDid)
	ON_BN_CLICKED(IDC_CHECK_VAR, &CSGMW_diagDlg::OnBnClickedCheckVar)
	ON_BN_CLICKED(IDC_CHECK_SELECT_ALL, &CSGMW_diagDlg::OnBnClickedCheckSelectAll)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CSGMW_diagDlg::OnBnClickedButtonTest)
    ON_STN_CLICKED(IDC_STATIC_FILE_PATH, &CSGMW_diagDlg::OnStnClickedStaticFilePath)
    ON_BN_CLICKED(IDC_BUTTON2, &CSGMW_diagDlg::OnBnClickedButton2)
	ON_CBN_SELCHANGE(IDC_COMBO_FWTYPE, &CSGMW_diagDlg::OnCbnSelchangeComboFwtype)
	ON_CBN_SELCHANGE(IDC_COMBO_DIFFUD, &CSGMW_diagDlg::OnCbnSelchangeComboDiffud)
	ON_EN_CHANGE(IDC_EDIT1, &CSGMW_diagDlg::OnEnChangeEdit1)
END_MESSAGE_MAP()


// CSGMW_diagDlg 消息处理程序

BOOL CSGMW_diagDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	h_log_mutex = CreateMutex(NULL,FALSE,NULL);
	/* 初始化m_list_batch_cmd */
	m_list_batch_cmd.SetExtendedStyle(m_list_batch_cmd.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_list_batch_cmd.InsertColumn(0, "BATCH_CMD_ID", LVCFMT_LEFT, 600, 0);

	/* 初始化m_list_sel_file */
	m_list_sel_file.SetExtendedStyle(m_list_sel_file.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_list_sel_file.InsertColumn(0, "文件序号", LVCFMT_LEFT, 200, 0);
	m_list_sel_file.InsertColumn(1, "文件名", LVCFMT_LEFT, 200, 1);
	m_list_sel_file.InsertColumn(2, "文件长度", LVCFMT_LEFT, 200, 2);

	/* 初始化m_edit_table */
	CRect rect;
	((CListCtrl*)GetDlgItem(IDC_LIST_EDIT))->GetWindowRect(&rect);
	ScreenToClient(&rect);
	DWORD dwStyle = LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_BORDER |
		WS_CHILD | WS_GROUP | WS_TABSTOP /*| WS_VISIBLE*/;  
	if(m_edit_table.Create(dwStyle, rect, this, IDC_LIST_EDIT) == TRUE)
	{
		m_edit_table.SetExtendedStyle(m_edit_table.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES /*| LVS_EX_TWOCLICKACTIVATE*/);
		m_edit_table.InsertColumn(0, "序号", LVCFMT_LEFT, 60, 0);
		m_edit_table.InsertColumn(1, "CAN_ID", LVCFMT_LEFT, 180, 1);
		m_edit_table.InsertColumn(2, "发送周期", LVCFMT_LEFT, 180, 2);
		m_edit_table.InsertColumn(3, "发送次数", LVCFMT_LEFT, 180, 3);

		CString str;
		for(int i=0; i<CAN_INFO_NUM; ++i)
		{
			str.Format("%d", i);
			m_edit_table.InsertItem(i, str);
			str.Format("%04x", can_id_info[i].can_id);
			m_edit_table.SetItemText(i, 1, str);
			str.Format("%d", can_id_info[i].send_period);
			m_edit_table.SetItemText(i, 2, str);
			if(UNLIMIT == can_id_info[i].send_cnt)
			{
				str.Format("%s", "unlimit");
				m_edit_table.SetItemText(i, 3, str);
			}
			else
			{
				str.Format("%d", can_id_info[i].send_cnt);
				m_edit_table.SetItemText(i, 3, str);
			}
		}

	}


	//初始化DEBUG信息
	m_comb_debug.ResetContent();
	m_comb_debug.AddString("NORM_DBG");
	m_comb_debug.AddString("C1_DBG");
	m_comb_debug.AddString("C2_DBG");
	m_comb_debug.AddString("C3_DBG");
	m_comb_debug.AddString("C4_DBG");
	m_comb_debug.SetCurSel(4);
	DebugPrintf(NORM_DBG,"CAN总线处于断开状态",0,0,DBG_VAR_TYPE);

	m_comb_diffud.ResetContent();
	m_comb_diffud.AddString("NO");
	m_comb_diffud.AddString("YES");
	m_comb_diffud.SetCurSel(0);
	m_comb_diffud.EnableWindow(FALSE);

	m_comb_fwtype.ResetContent();
	m_comb_fwtype.AddString("MCU-app");
	m_comb_fwtype.AddString("imax6app");
	m_comb_fwtype.AddString("sf");
	m_comb_fwtype.AddString("kernel");
	m_comb_fwtype.AddString("dtb");
	m_comb_fwtype.AddString("uboot");
	m_comb_fwtype.AddString("MCU-boot");
	m_comb_fwtype.SetCurSel(0);

	/* 刷新can节点ID */
	UpdateData(true);
	/*m_edit_pc_id	= "076A";
	m_edit_avm_id	= "0762";*/
	m_edit_pc_id	= "0753";
	m_edit_avm_id	= "0752";
	//m_edit_write    = "0123456789abcdefg";

//	flush_can_id(&avm_id,&pc_id);

	//限定m_edit_canwrite的字符输入长度
	//m_edit_canwrite.LimitText(16);

	init_diag_info();

	/********************************/
	InitWriteOperate();
	/********************************/
	InitParam();
	ShowOrHideControl(handle_flag);
	/********************************/
	//sel_can_id_num = 0;
	memset(cwThread, NULL, CAN_INFO_NUM);

	((CEdit*)GetDlgItem(IDC_EDIT_WRITE))->SetLimitText(20);
	GetDlgItem(IDC_BUTTON_WRITE_DID)->EnableWindow(false);//
	GetDlgItem(IDC_STATIC_VAR)->ShowWindow(false);
	GetDlgItem(IDC_EDIT_VAR)->ShowWindow(false);
	((CEdit*)GetDlgItem(IDC_EDIT_VAR))->SetLimitText(2);


	/* 获取工程根目录 */
	char chModule[1024] = {0};
	::GetModuleFileName(NULL, chModule, 1024);
	char* pchr = strrchr(chModule, '\\');
	if(pchr)
	{
		pchr[0] = 0;
		strRootDir.Format("%s", chModule);
	}

	UpdateData(false);

	/* 建立数据接收线程 */
	AfxBeginThread(DiagReceiveThread, this);
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSGMW_diagDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSGMW_diagDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/* 信息显示函数 */
void CSGMW_diagDlg::DispPrintf(CString str, unsigned char* pData,int len,int type )
{
	int			   i     = 0;
	int	           sel   = 0;;
	int			   size  = (len+8) * 2;
	unsigned char *p_buf = NULL;
	unsigned char  temp  = 0;
	CString		   info;

	/* 打印缓冲区中的内容 */
	if( pData != NULL && len != 0 )
	{
		p_buf = new unsigned char[size];
		memset(p_buf,0,size);
		info.Empty();

		/* 将缓冲区中的内容全部转换成可以显示的字符串 */
		for( i = 0 ; i < len ; i++ )
		{
			/* 转换高四位数据 */
			if( type == DBG_VAR_TYPE )
				temp = (pData[len - i - 1] & 0xf0) >> 4;
			else
				temp = (pData[i] & 0xf0) >> 4;

			if( temp >= 0 && temp < 10 )
				p_buf[i*2+0] = temp + '0';
			else if( temp >= 10 )
				p_buf[i*2+0] = (temp-10) + 'a';

			/* 转换低四位数据 */
			if( type == DBG_VAR_TYPE )
				temp = pData[len - i - 1] & 0x0f;
			else
				temp = pData[i] & 0x0f;
			if( temp >= 0 && temp < 10 )
				p_buf[i*2+1] = temp + '0';
			else if( temp >= 10 )
				p_buf[i*2+1] = (temp-10) + 'a';
		}
		p_buf[i*2+0] = 0x00;
		p_buf[i*2+1] = 0x00;

		/* 将转换后的数据放入cstring中 */
		if( !str.IsEmpty() )
			info.Format("%s - %s\r\n",str.GetBuffer(str.GetLength()),p_buf);
		else
			info.Format("%s\r\n",p_buf);
		/* 将数据输出到缓冲区 */
		m_list_did.InsertString(m_list_did.GetCount(),info);
		m_list_did.SetCurSel(m_list_did.GetCount()-1);
		delete[] p_buf;
	}
	else
	{
		/* 打印字符串中的内容 */
		if( !str.IsEmpty() )
		{
			m_list_did.InsertString(m_list_did.GetCount(),str);
			m_list_did.SetCurSel(m_list_did.GetCount()-1);
			//m_list_tbox_receive.AddString(str);
		}
	}
	return ;
}

/* 打印缓冲区数据 */
void CSGMW_diagDlg::DebugPrintf(int mode, CString str, unsigned char* pData, int len, int type )
{
	int			   i     = 0;
	int	           sel   = 0;;
	int			   size  = (len+8) * 2;
	unsigned char *p_buf = NULL;
	unsigned char  temp  = 0;
	CString		   info;

	/* 获取当前打印级别 */
	sel = m_comb_debug.GetCurSel();
	if( sel < mode )
		return;

	/* 打印缓冲区中的内容 */
	if( pData != NULL && len != 0 )
	{
		p_buf = new unsigned char[size];
		memset(p_buf,0,size);
		info.Empty();

		/* 将缓冲区中的内容全部转换成可以显示的字符串 */
		for( i = 0 ; i < len ; i++ )
		{
			/* 转换高四位数据 */
			if( type == DBG_VAR_TYPE )
				temp = (pData[len - i - 1] & 0xf0) >> 4;
			else
				temp = (pData[i] & 0xf0) >> 4;

			if( temp >= 0 && temp < 10 )
				p_buf[i*2+0] = temp + '0';
			else if( temp >= 10 )
				p_buf[i*2+0] = (temp-10) + 'a';

			/* 转换低四位数据 */
			if( type == DBG_VAR_TYPE )
				temp = pData[len - i - 1] & 0x0f;
			else
				temp = pData[i] & 0x0f;
			if( temp >= 0 && temp < 10 )
				p_buf[i*2+1] = temp + '0';
			else if( temp >= 10 )
				p_buf[i*2+1] = (temp-10) + 'a';
		}
		p_buf[i*2+0] = 0x00;
		p_buf[i*2+1] = 0x00;

		/* 将转换后的数据放入cstring中 */
		if( !str.IsEmpty() )
			info.Format("%s - %s\r\n",str.GetBuffer(str.GetLength()),p_buf);
		else
			info.Format("%s\r\n",p_buf);
		/* 将数据输出到缓冲区 */
		m_list_cmd.InsertString(m_list_cmd.GetCount(),info);
		m_list_cmd.SetCurSel(m_list_cmd.GetCount()-1);
		delete[] p_buf;
	}
	else
	{
		/* 打印字符串中的内容 */
		if( !str.IsEmpty() )
		{
			m_list_cmd.InsertString(m_list_cmd.GetCount(),str);
			m_list_cmd.SetCurSel(m_list_cmd.GetCount()-1);
			//m_list_tbox_receive.AddString(str);
		}
	}
	return ;
}
/* 记录CAN ID处理程序 */
void CSGMW_diagDlg::flush_can_id(int *p_tx, int *p_rx)
{
	CString str;
	UpdateData(true);
	*p_rx		= comm.SwitchAscToHex(m_edit_pc_id);
	*p_tx		= comm.SwitchAscToHex(m_edit_avm_id);
	if( can_driver.AddLargeSendCanId(*p_tx,*p_rx) == STATUS_ERR )
	{
		DebugPrintf(NORM_DBG,"can mask buf is overflow",0,0,DBG_VAR_TYPE);
	}
	UpdateData(false);
}

/* DiagReceiveThread 消息处理程序 */
UINT CSGMW_diagDlg::DiagReceiveThread(void *param)
{
	CSGMW_diagDlg		*p_diag			= (CSGMW_diagDlg*)param;
	ECanDriver			*p_can			= (ECanDriver*)&p_diag->can_driver;
	CAN_OBJ				can_sour_data	= {0};
	CString				str				= "";
	int					res				=  0;
	int					index			=  0;
	int					wait_time		= GetTickCount();
	int					pre_block_cnt	=  0;
	int					pre_time_tick	=  0;
	int					byte_per_sec	=  0;
	int					remain_time		=  0;

	while(1)
	{
		/* 测试模式时，直接退出线程 */
		if( p_diag->m_check_var.GetCheck() )
			break;

		if(p_diag->handle_flag)
		{
			if(p_diag->diag_start_tm > 0)
			{
				CString info;
				
				if (GetTickCount() - pre_time_tick > 1000)
				{
					byte_per_sec = ((p_diag->block_cnt - pre_block_cnt) * p_diag->m_block_size) / 1024;
					pre_block_cnt = p_diag->block_cnt;
					pre_time_tick = GetTickCount();
				}

				if (byte_per_sec != 0)
					remain_time = ((p_diag->m_block_num - p_diag->block_cnt) * p_diag->m_block_size) / (byte_per_sec * 1024);

				info.Format("开始计时：%d秒    速度：%d kB/s    剩余时间：%d 分 %d 秒" , (GetTickCount() - p_diag->diag_start_tm)/1000, byte_per_sec, remain_time/60, remain_time%60);
				((CStatic*)p_diag->GetDlgItem(IDC_STATIC_TIMER))->SetWindowTextA(info);
			}
		}

		res = p_can->can_receive_buf.get_ringbuf((unsigned char*)&can_sour_data);
		if( res == RINGBUF_OK && can_sour_data.id == p_diag->pc_id )
		{	/* 获取到TBOX发送的can信息 */
			//if( !(can_sour_data.buf[0] == 0x02 && can_sour_data.buf[1] == 0x7E) )
			//{
				str.Format("receive can data id(0x%04x):",can_sour_data.id);
				p_diag->DebugPrintf(C4_DBG,str,can_sour_data.buf,can_sour_data.len,DBG_DATA_TYPE);
			//}

			/* 处理接收到的数据协议 */
			p_diag->diag_receive_proc(can_sour_data);
		}

#if 0
		if( (GetTickCount() - wait_time) >= 2000 )
		{
			/*if(p_dlg->handle_flag)
			{*/

			wait_time = GetTickCount();
			//can_sour_data.id  = p_diag->avm_id;
			/*	can_sour_data.id = 0x07df;
			can_sour_data.len = 8;
			can_sour_data.buf[0] = 0x02;
			can_sour_data.buf[1] = 0x3e;
			can_sour_data.buf[2] = 0x80;
			can_sour_data.buf[3] = 0x55;
			can_sour_data.buf[4] = 0x55;
			can_sour_data.buf[5] = 0x55;
			can_sour_data.buf[6] = 0x55;
			can_sour_data.buf[7] = 0x55;*/
			//p_can->TransmitData(can_sour_data.id,can_sour_data.buf,can_sour_data.len);
			//AfxBeginThread(p_can->TransmitHeartBeat, p_diag);


			/*	}
			else
			{
			wait_time = GetTickCount();
			can_sour_data.id  = p_diag->avm_id;
			can_sour_data.len = 8;
			can_sour_data.buf[0] = 0x02;
			can_sour_data.buf[1] = 0x3e;
			can_sour_data.buf[2] = 0x00;
			can_sour_data.buf[3] = 0x00;
			can_sour_data.buf[4] = 0x00;
			can_sour_data.buf[5] = 0x00;
			can_sour_data.buf[6] = 0x00;
			can_sour_data.buf[7] = 0x00;
			p_can->TransmitData(can_sour_data.id,can_sour_data.buf,can_sour_data.len);
			}*/
		}
#endif

		//Sleep(1);
	}

	return 0;
}

#if 1
/* 开启或者关闭测试线程，单线程处理 */
void CSGMW_diagDlg::OnBnClickedButtonTest()
{
	//// TODO: 在此添加控件通知处理程序代码

	//if( can_driver.m_can_link_status != CAN_CONNECT_OK )
	//{
	//	MessageBox("请先启动CanBus!", "警告", MB_OK|MB_ICONQUESTION);
	//	return;
	//}

	//CString str;
	//((CButton*)GetDlgItem(IDC_BUTTON_TEST))->GetWindowTextA(str);
	//if( "开始测试" == str )
	//{
	//	m_dequeCanInfo.clear();
	//	for(int i=0; i<m_edit_table.GetItemCount(); ++i)
	//	{
	//		if( m_edit_table.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED || m_edit_table.GetCheck(i) )
	//		{			
	//			m_dequeCanInfo.push_back(can_id_info[i]);
	//		}
	//	}

	//	if(!m_dequeCanInfo.empty())
	//	{
	//		((CButton*)GetDlgItem(IDC_BUTTON_TEST))->SetWindowTextA("结束测试");
	//		test_thread_exit_flag = false;

	//		AfxBeginThread(TestThreadProc, this);
	//	}
	//	else
	//	{
	//		MessageBox("没有选中的测试项，请重试", "提示");
	//		return;
	//	}
	//		
	//}
	//else
	//{
	//	((CButton*)GetDlgItem(IDC_BUTTON_TEST))->SetWindowTextA("开始测试");
	//	test_thread_exit_flag = true;

	//}
	//
}
#else
/* 开启或者关闭测试线程，多线程处理 */
void CSGMW_diagDlg::OnBnClickedButtonTest()
{
	// TODO: 在此添加控件通知处理程序代码

	if( can_driver.m_can_link_status != CAN_CONNECT_OK )
	{
		MessageBox("请先启动CanBus!", "警告", MB_OK|MB_ICONQUESTION);
		return;
	}

	CString str;
	((CButton*)GetDlgItem(IDC_BUTTON_TEST))->GetWindowTextA(str);
	if( "开始测试" == str )
	{
		m_dequeCanInfo.clear();
		test_thread_exit_flag = false;
		for(int i=0; i<m_edit_table.GetItemCount(); ++i)
		{
			if( m_edit_table.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED || m_edit_table.GetCheck(i) )
			{
				AfxBeginThread(TestThreadProc, &can_id_info[i]);

				m_dequeCanInfo.push_back(can_id_info[i]);
			}
		}

		if(!m_dequeCanInfo.empty())
		{
			((CButton*)GetDlgItem(IDC_BUTTON_TEST))->SetWindowTextA("结束测试");
		}
		else
		{
			MessageBox("没有选中的测试项，请重试", "提示");
			return;
		}

	}
	else
	{
		((CButton*)GetDlgItem(IDC_BUTTON_TEST))->SetWindowTextA("开始测试");
		test_thread_exit_flag = true;

		/*for(int i=0; i<sel_can_id_num; ++i)
		{
			TerminateThread(cwThread[i]->m_hThread, 0);	//强制结束线程
		}
		sel_can_id_num = 0;*/
	}
	
}
#endif

void CSGMW_diagDlg::OnBnClickedCheckSelectAll()
{
	// TODO: 在此添加控件通知处理程序代码

	if( ((CButton*)GetDlgItem(IDC_CHECK_SELECT_ALL))->GetCheck() )
	{
		for(int i=0; i<CAN_INFO_NUM; ++i)
		{
			m_edit_table.SetCheck(i, true);
		}
	}
	else
	{
		for(int i=0; i<CAN_INFO_NUM; ++i)
		{
			m_edit_table.SetCheck(i, false);
		}
	}
}


void CSGMW_diagDlg::OnBnClickedCheckVar()
{
	// TODO: 在此添加控件通知处理程序代码

	if(m_check_var.GetCheck())
	{
		GetDlgItem(IDC_STATIC_VAR)->ShowWindow(true);
		GetDlgItem(IDC_EDIT_VAR)->ShowWindow(true);

		ShowOrHideControl(true);
		m_edit_table.ShowWindow(true);
		GetDlgItem(IDC_CHECK_SELECT_ALL)->ShowWindow(true);
		GetDlgItem(IDC_BUTTON_TEST)->ShowWindow(true);

		m_comb_file_list.ShowWindow(false);
		m_list_sel_file.ShowWindow(false);
		(CStatic*)GetDlgItem(IDC_STATIC_FILE_LIST)->ShowWindow(false);
		(CButton*)GetDlgItem(IDC_BUTTON_FILE_TRANSFER)->ShowWindow(false);
		(CButton*)GetDlgItem(IDC_BUTTON_FILE_DELETE)->ShowWindow(false);

		((CButton*)GetDlgItem(IDC_BUTTON_TEST))->SetWindowTextA("开始测试");
		((CButton*)GetDlgItem(IDC_CHECK_SELECT_ALL))->SetCheck(false);
		for(int i=0; i<CAN_INFO_NUM; ++i)
		{
			m_edit_table.SetCheck(i, false);
		}
	}
	else
	{
		GetDlgItem(IDC_STATIC_VAR)->ShowWindow(false);
		GetDlgItem(IDC_EDIT_VAR)->ShowWindow(false);
		GetDlgItem(IDC_CHECK_SELECT_ALL)->ShowWindow(false);
		GetDlgItem(IDC_BUTTON_TEST)->ShowWindow(false);
		
		ShowOrHideControl(false);
		m_edit_table.ShowWindow(false);

		test_thread_exit_flag = true;	//关闭测试线程 
		/* 退出测试模式，重新启动线程 */
		AfxBeginThread(((ECanDriver*)&(this->can_driver))->ReceiveThread, (ECanDriver*)&(this->can_driver));	
		AfxBeginThread(DiagReceiveThread, this);
	}
}

void CSGMW_diagDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}


void CSGMW_diagDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}


void CSGMW_diagDlg::OnBnClickedButtonCanConnect()
{
	// TODO: 在此添加控件通知处理程序代码
	int res = 0;
	CString str;
	// TODO: 在此添加控件通知处理程序代码
	can_driver.dev_type_str				= "VCI_USBCAN1";
	can_driver.dev_type_select			= 0;
	can_driver.dev_can_channel_select	= 0;
	can_driver.dev_can_idx_select		= 0;
	can_driver.dev_can_bitrate_select	= 3;
	can_driver.dev_can_fliter_select	= 0;
	can_driver.dev_can_mode_select		= 0;

	/* 打开or断开can总线，同时更新按钮上的提示信息 */
	GetDlgItem(IDC_BUTTON_CAN_CONNECT)->GetWindowText(str);
	if( str.Compare("CAN连接") == 0 )
	{
		UpdateData(true);
		res = can_driver.CanDriverConnect();
		UpdateData(false);

		switch( res )
		{
		case CAN_CONNECT_CHECK_CODE_ERR:
			MessageBox("验收码错误!","警告",MB_OK|MB_ICONQUESTION);
			break;
		case CAN_CONNECT_SHEILD_CODE_FORMAT_ERR:
			MessageBox("屏蔽码错误!","警告",MB_OK|MB_ICONQUESTION);
			break;
		case CAN_CONNECT_OPEN_DRIVER_ERR:
			MessageBox("打开CAN设备失败!","警告",MB_OK|MB_ICONQUESTION);
			break;
		case CAN_CONNECT_INIT_DRIVER_ERR:
			MessageBox("初始化CAN失败!","警告",MB_OK|MB_ICONQUESTION);
			break;
		case CAN_CONNECT_START_ERR:
			MessageBox("CAN启动失败!","警告",MB_OK|MB_ICONQUESTION);
			break;
		case CAN_CONNECT_OK:
			//打开can驱动成功
			((CButton*)GetDlgItem(IDC_EDIT_PC_ID))->EnableWindow(false);
			((CButton*)GetDlgItem(IDC_EDIT_AVM_ID))->EnableWindow(false);

			/* 刷新can id */
			flush_can_id(&avm_id,&pc_id);
			
			GetDlgItem(IDC_BUTTON_CAN_CONNECT)->SetWindowText("CAN断开");
			DebugPrintf(NORM_DBG,"CAN总线已连接",0,0,DBG_VAR_TYPE);
			UpdateData(false);
			break;
		}
	}
	else
	{
		UpdateData(true);

		/* 设置传送模式 */
		((CButton*)GetDlgItem(IDC_EDIT_PC_ID))->EnableWindow(true);
		((CButton*)GetDlgItem(IDC_EDIT_AVM_ID))->EnableWindow(true);

		GetDlgItem(IDC_BUTTON_CAN_CONNECT)->SetWindowText("CAN连接");
		DebugPrintf(NORM_DBG,"CAN总线断开",0,0,DBG_VAR_TYPE);
		can_driver.CanDriverUnconnect();

		UpdateData(false);
	}
}


void CSGMW_diagDlg::OnBnClickedButtonReadDid()
{
	// TODO: 在此添加控件通知处理程序代码
	if( can_driver.m_can_link_status != CAN_CONNECT_OK )
	{
		MessageBox("请先启动CanBus!","警告",MB_OK|MB_ICONQUESTION);
		return;
	}
	/* 发送读命令 */
	send_diag_cmd(m_comb_did.GetCurSel(),0x01);
}


void CSGMW_diagDlg::OnBnClickedButtonWriteDid()
{
	//// TODO: 在此添加控件通知处理程序代码
	//if( can_driver.m_can_link_status != CAN_CONNECT_OK )
	//{
	//	MessageBox("请先启动CanBus!","警告",MB_OK|MB_ICONQUESTION);
	//	return;
	//}
	/* 发送写命令 */
	send_diag_cmd(m_comb_did.GetCurSel(),0x02);
}


void CSGMW_diagDlg::OnBnClickedButtonCanwrite()
{
	// TODO: 在此添加控件通知处理程序代码
	//if( can_driver.m_can_link_status != CAN_CONNECT_OK )
	//{
	//	MessageBox("请先启动CanBus!","警告",MB_OK|MB_ICONQUESTION);
	//	return;
	//}

	UpdateData(TRUE);
	CString strCanWrite;
	m_edit_canwrite.GetWindowTextA(strCanWrite);
	if(strCanWrite.IsEmpty())
	{
		MessageBox("请先输入CAN数据!","警告",MB_OK|MB_ICONQUESTION);
		return;
	}
	strCanWrite.Remove(' ');  // 去除空格
	if(strCanWrite.GetLength()%2 != 0)
	{
		MessageBox("输入的字符个数为奇数，请重新输入!","警告",MB_OK|MB_ICONQUESTION);
		return;
	}

	/* 发送手动写入的CAN指令 */
	send_diag_cmd_manual(strCanWrite);
}


void CSGMW_diagDlg::OnBnClickedButtonCls()
{
	// TODO: 在此添加控件通知处理程序代码

	m_list_did.ResetContent();
	m_list_cmd.ResetContent();
}


void CSGMW_diagDlg::OnBnClickedButtonBatchReadDid()
{
	// TODO: 在此添加控件通知处理程序代码

	if( can_driver.m_can_link_status != CAN_CONNECT_OK )
	{
		MessageBox("请先启动CanBus!","警告",MB_OK|MB_ICONQUESTION);
		return;
	}

	unsigned char   idx = 1;
	CAN_OBJ			can_data = {0};
	CString			str;

	if(!m_dequeBatchCmd.empty())
	{
		for(unsigned int i=0; i<m_dequeBatchCmd.size(); ++i)
		{
			can_data.buf[idx+1] = (m_dequeBatchCmd.at(i).did >> 8) & 0xff;
			can_data.buf[idx+2] = (m_dequeBatchCmd.at(i).did >> 0) & 0xff;
			idx += 2;
		}

		can_data.buf[0] = idx;
		can_data.buf[1] = 0x22;
		can_data.id = avm_id;
		can_data.len = idx + 1;
		can_driver.TransmitData(can_data.id,can_data.buf,can_data.len);
		str.Format("==== send cmd to avm: id( 0x%04x )",can_data.id);
		DebugPrintf(C4_DBG,str,can_data.buf,can_data.len,DBG_DATA_TYPE);	
	}
}

void CSGMW_diagDlg::OnBnClickedButtonStop()
{
	// TODO: 在此添加控件通知处理程序代码

	handle_flag = 0;
	handle_type = 0;
	diag_update_step = UPDATE_TO_TBOX_NULL;
	diag_transfer_step = TRANS_FROM_TBOX_NULL;
	DebugPrintf(NORM_DBG,"停止",0,0,DBG_VAR_TYPE);

	(CButton*)GetDlgItem(IDC_BUTTON_READ_DID)->EnableWindow(TRUE);
	(CButton*)GetDlgItem(IDC_BUTTON_BATCH_READ_DID)->EnableWindow(TRUE);
	(CButton*)GetDlgItem(IDC_BUTTON_WRITE_DID)->EnableWindow(TRUE);
	(CButton*)GetDlgItem(IDC_BUTTON_CANWRITE)->EnableWindow(TRUE);

    ((CButton*)GetDlgItem(IDC_EDIT_AVM_ID))->EnableWindow(true);

	GetDlgItem(IDC_BUTTON_CAN_CONNECT)->SetWindowText("CAN连接");

	pFile.Close(); // 关闭文件

	m_btn_update.EnableWindow(TRUE); // 恢复升级按钮
	InitParam();
}


/* 双击显示m_list_did的内容 */
void CSGMW_diagDlg::OnLbnDblclkListDid()
{
	// TODO: 在此添加控件通知处理程序代码

	CString str;
	m_list_did.GetText(m_list_did.GetCurSel(), str);
	//MessageBox(str, "内容显示");

	if (!m_DisplayDlg.GetSafeHwnd())
	{
		m_DisplayDlg.Create(IDD_DISPLAY_DIALOG, this);

		m_DisplayDlg.ShowWindow(SW_SHOW);
		m_DisplayDlg.CenterWindow();
		m_DisplayDlg.SetDlgItemText(IDC_EDIT_DISPLAY, str);
	}

}


/* 双击显示m_list_cmd的内容 */
void CSGMW_diagDlg::OnLbnDblclkListCmd()
{
	// TODO: 在此添加控件通知处理程序代码

	CString str;
	m_list_cmd.GetText(m_list_cmd.GetCurSel(), str);
	//MessageBox(str, "内容显示");

	if (!m_DisplayDlg.GetSafeHwnd())
	{
		m_DisplayDlg.Create(IDD_DISPLAY_DIALOG, this);
	}
	m_DisplayDlg.ShowWindow(SW_SHOW);
	m_DisplayDlg.CenterWindow();
	m_DisplayDlg.SetDlgItemText(IDC_EDIT_DISPLAY, str);
}


void CSGMW_diagDlg::OnNMRClickListBatchCmd(NMHDR *pNMHDR, LRESULT *pResult)		//右键弹出菜单
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if(pNMListView->iItem != -1)
	{
		DWORD dwPos = GetMessagePos();
		CPoint point( LOWORD(dwPos), HIWORD(dwPos) );
		CMenu menu;
		VERIFY( menu.LoadMenu( IDR_MENU1 ) );     
		CMenu* popup = menu.GetSubMenu(0);
		ASSERT( popup != NULL );
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );
	}
}


void CSGMW_diagDlg::OnNMRClickListSelFileList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if(pNMListView->iItem != -1)
	{
		DWORD dwPos = GetMessagePos();
		CPoint point( LOWORD(dwPos), HIWORD(dwPos) );
		CMenu menu;
		VERIFY( menu.LoadMenu( IDR_MENU1 ) );     
		CMenu* popup = menu.GetSubMenu(0);
		ASSERT( popup != NULL );
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );
	}
}


void CSGMW_diagDlg::OnCmd32771()		//右键删除
{
	// TODO: 在此添加命令处理程序代码

	if(m_list_sel_file.IsWindowEnabled())
	{
		int i, iState;
		int nItemSelected = m_list_sel_file.GetSelectedCount();//获取列表视图控件中被选择列表项的数量
		int nItemCount = m_list_sel_file.GetItemCount();//获取列表视图控件中列表项的数量
		if(nItemSelected < 1)
			return;

		std::deque<TBOX_FILE_LIST>::iterator iter;
		iter = m_dequeSelFile.begin();
		for(i = nItemCount-1; i >= 0; i--)
		{
			iState = m_list_sel_file.GetItemState(i, LVIS_SELECTED);
			if(iState != 0)
			{
				m_list_sel_file.DeleteItem(i);
				m_dequeSelFile.erase(iter+i);
			}
		}
	}

	if(m_list_batch_cmd.IsWindowEnabled())
	{
		int i, iState;
		int nItemSelected = m_list_batch_cmd.GetSelectedCount();//获取列表视图控件中被选择列表项的数量
		int nItemCount = m_list_batch_cmd.GetItemCount();//获取列表视图控件中列表项的数量
		if(nItemSelected < 1)
			return;

		std::deque<TBOX_READ_INFO>::iterator iter;
		iter = m_dequeBatchCmd.begin();
		for(i = nItemCount-1; i >= 0; i--)
		{
			iState = m_list_batch_cmd.GetItemState(i, LVIS_SELECTED);
			if(iState != 0)
			{
				m_list_batch_cmd.DeleteItem(i);
				m_dequeBatchCmd.erase(iter+i);
			}
		}
	}
}


void CSGMW_diagDlg::OnCmd32772()
{
	// TODO: 在此添加命令处理程序代码	//y右键清空
	
	if(m_list_sel_file.IsWindowEnabled())
	{
		m_list_sel_file.DeleteAllItems();
		m_dequeSelFile.clear();
	}

	if(m_list_batch_cmd.IsWindowEnabled())
	{
		m_list_batch_cmd.DeleteAllItems();
		m_dequeBatchCmd.clear();
	}
}

/* 初始化升级/传输相关参数 */
void CSGMW_diagDlg::InitParam()
{											
	handle_flag = 0;	
	handle_type = 0;
	m_dwCRC = 0;	
	m_file_size = 0;						
	//memset(m_file_data, 0, FILE_MAX_SIZE);
	diag_update_step = 0;			
	m_block_size = 0;		
	m_block_num = 0;								
	m_temp_len_before = 0;							
	m_temp_len_last = 0; 							
	m_remain_len = 0;															
	block_cnt = 1;
	module_cnt = 1;
	dwFilePos = 0;
	diag_start_tm = 0;
	//m_module_num = 0;
	//m_module_size = 0;
	//memset(m_module_data, 0, 64*KB_1);

	diag_transfer_step = 0;
	m_file_list_pack_num = 0;
	m_file_list_pack_size = 0;										
	m_file_list_size = 0;							
	memset(m_file_list_data, 0, 10*KB_1);
	m_file_num = 0;
	f_index = 0;
	file_cnt = 1;
	trans_mod = 0;
}

//打印	--> 1 - m_list_did		2 - m_list_cmd	
void CSGMW_diagDlg::ShowInfo(CString str, int sec)
{
	if(str.IsEmpty())
		return;

	//if(m_check_print.GetCheck())
	//{
	//	switch(sec)
	//	{
	//	case 1:
	//		m_list_did.InsertString(m_list_did.GetCount(), str);
	//		m_list_did.SetCurSel(m_list_did.GetCount()-1);
	//		break;
	//	case 2:
	//		m_list_cmd.InsertString(m_list_cmd.GetCount(), str);
	//		m_list_cmd.SetCurSel(m_list_cmd.GetCount()-1);
	//		break;
	//	default:
	//		break;
	//	}
	//}
	str.Format(str);
	DispPrintf(str,0,0,DBG_VAR_TYPE);

#if 0
	/* 写打印日志 */
	str += "\r\n";
	CString strFilePath;
	strFilePath.Format("%s\\%s", strRootDir, PRINT_LOG);
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwPos = 0 , dwLen = 0;
	unsigned char ucHead[2];
	ucHead[0] = 0xFF;
	ucHead[1] = 0xFE;
	EnterCriticalSection(&m_csLog);	//进入临界区
	hFile = CreateFile(strFilePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		dwPos = GetFileSize(hFile, 0);
		SetFilePointer(hFile , dwPos , 0 , FILE_BEGIN);
		if(dwPos == 0)
			WriteFile(hFile, ucHead, 2, &dwLen, 0);

		char *pStr = str.GetBuffer(str.GetLength()); //取得str对象的原始字符串
		int nBufferSize = MultiByteToWideChar(CP_ACP, 0, pStr, -1, NULL, 0); //取得所需缓存的多少
		wchar_t *pBuffer = (wchar_t*)malloc(nBufferSize * sizeof(wchar_t));//申请缓存空间
		MultiByteToWideChar(CP_ACP, 0, pStr, -1 , pBuffer, nBufferSize*sizeof(wchar_t));//转码
		
		if(!WriteFile(hFile, pBuffer, nBufferSize * sizeof(wchar_t), &dwLen , 0))
		{
			str.Format("写日志(%s)出错 - %d", PRINT_LOG, GetLastError());
			DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
		}
		free(pBuffer); //释放缓存
		str.ReleaseBuffer();
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	else
	{
		str.Format("打开日志(%s)出错 - %d", PRINT_LOG, GetLastError());
		DebugPrintf(NORM_DBG,str,0,0,DBG_VAR_TYPE);
	}
	LeaveCriticalSection(&m_csLog);	//退出临界区
#endif

}

/* 检查目录是否存在，不存在则创建 */
void CSGMW_diagDlg::CheckDirectory(char* FileName)
{
	CString strPath;
	char chModule[1024] = {0};
	::GetModuleFileName(NULL, chModule, 1024);
	char* pchr = strrchr(chModule, '\\');
	if(pchr)
	{
		pchr[0] = 0;
		strPath.Format("%s\\%s", chModule, FileName);
		strDirName = strPath;
	}

	if(!PathIsDirectory(strPath))
	{
		CreateDirectory(strPath, NULL);
	}
}


CString CSGMW_diagDlg::CanErrHandle(unsigned char func)
{
	CString str;
	switch(func)
	{
	case NO_ERR:
		str.Format("正确");
		break;
	case SID_ERR:
		str.Format("服务不支持");
		break;
	case SUB_ERR:
		str.Format("不支持子功能");
		break;
	case FOR_ERR:
		str.Format("不正确的消息长度和无效的格式");
	case CONDITION_ERR:
		str.Format("条件不正确");
		break;
	case SEQUENCE_ERROR:
		str.Format("请求序列错误");
		break;
	case OUTRANGE_ERR:
		str.Format("请求超出范围");
		break;
	case SECURITY_ERR:
		str.Format("安全访问拒绝");
		break;
	case INVALID_KEY_ERR:
		str.Format("密钥无效");
		break;
	case OVER_TRY_ERR:
		str.Format("超出尝试次数");
		break;
	case DELAY_NOT_ARRIVAL_ERR:
		str.Format("所需时间延迟未到");
		break;
	case UPLOAD_DOWNLOAD_NOT_ACCEPTED:
		str.Format("不允许上传下载");
		break;
	case TRANSFER_DATA_SUSPENDED:
		str.Format("数据传输暂停");
		break;
	case PROGRAMME_ERR:
		str.Format("一般编程失败");
		break;
	case WRONG_BLOCK_SEQUENCE_COUNTER:
		str.Format("错误的数据块序列计数器");
		break;
	case REQ_CORRECTLY_REC_RESP_PENDING:
		str.Format("正确接收请求消息 - 等待响应");
		break;
	case SUB_FUNC_NOT_SUPPORT_ACTIVE_SES:
		str.Format("激活会话不支持该子服务");
		break;
	case NOT_SUPPERT_SUB_IN_ACTIVE_SESSION:
		str.Format("激活会话不支持该服务");
		break;
	case BUFF_OVER_FLOW:
		str.Format("栈溢出");
		break;
	default:
		break;
	}
	return str;
}


void CSGMW_diagDlg::InitWriteOperate()
{
	m_combo_write_mode.InsertString(0, "写模式1");
	m_combo_write_mode.InsertString(1, "写模式2");
	m_combo_write_mode.SetCurSel(0);

	m_combo_param0.ShowWindow(false);
	m_combo_param1.ShowWindow(false);
	m_combo_param2.ShowWindow(false);
	m_combo_param3.ShowWindow(false);

	GetDlgItem(IDC_EDIT_WRITE)->ShowWindow(true);
	GetDlgItem(IDC_EDIT_PARAM0)->ShowWindow(true);
	GetDlgItem(IDC_EDIT_PARAM1)->ShowWindow(true);
	GetDlgItem(IDC_EDIT_PARAM2)->ShowWindow(true);
	GetDlgItem(IDC_EDIT_PARAM3)->ShowWindow(true);
	GetDlgItem(IDC_EDIT_PARAM4)->ShowWindow(true);
}



void CSGMW_diagDlg::OnStnClickedStaticFilePath()
{
    // TODO: 在此添加控件通知处理程序代码
}


void CSGMW_diagDlg::OnBnClickedButton2()
{
    // TODO: 在此添加控件通知处理程序代码
    OnBnClickedButtonStop();
    Sleep(500);
    can_driver.CanDriverUnconnect();
    can_driver.CanDriverReset();

    CString str;
    GetDlgItem(IDC_BUTTON_CAN_CONNECT)->GetWindowText(str);

	if( str.Compare("CAN连接") != 0 )
	{
        	GetDlgItem(IDC_BUTTON_CAN_CONNECT)->SetWindowText("CAN连接");
    }
    
    DebugPrintf(NORM_DBG,"CAN总线已复位，请重新连接",0,0,DBG_VAR_TYPE);
	UpdateData(false);
 }


void CSGMW_diagDlg::OnCbnSelchangeComboFwtype()
{
	// TODO: Add your control notification handler code here
	int type = m_comb_fwtype.GetCurSel();

	if ((type == 0)  || (type == 6))  
	{
		m_comb_diffud.SetCurSel(0);
		m_comb_diffud.EnableWindow(FALSE);  // MCU 无差分升级
	}
	else
	{
		m_comb_diffud.EnableWindow(TRUE);
	}

	CString str_path;
	str_path.Format("加载文件：");
	((CStatic*)GetDlgItem(IDC_STATIC_FILE_PATH))->SetWindowTextA(str_path);
	m_strPath = "";

}


void CSGMW_diagDlg::OnCbnSelchangeComboDiffud()
{
	// TODO: Add your control notification handler code here

	CString str_path;
	str_path.Format("加载文件：");
	((CStatic*)GetDlgItem(IDC_STATIC_FILE_PATH))->SetWindowTextA(str_path);
	m_strPath = "";
}








void CSGMW_diagDlg::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
