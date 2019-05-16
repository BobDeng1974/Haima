
// SGMW_diagDlg.h : 头文件
//

#pragma once
#include "ECanDriver.h"
#include "common.h"
#include "saver.h"
#include "diag.h"
#include <deque>
#include "afxwin.h"
#include "afxcmn.h"
#include "crc32.h"
#include "DisplayDlg.h"
#include "EditTable.h"
#include "logout.h"
using namespace std;


// CSGMW_diagDlg 对话框
class CSGMW_diagDlg : public CDialogEx
{
// 构造
public:
	CSGMW_diagDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SGMW_DIAG_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox		m_comb_did;				//DID读取列表
	CListCtrl		m_list_batch_cmd;		//获取cmd批量列表
	CComboBox		m_comb_debug;			//打印信息
	CComboBox		m_comb_diffud;			//差分升级
	CComboBox		m_comb_fwtype;			//程序类型
	CListBox		m_list_did;				//did操作显示列表
	CListBox		m_list_cmd;				//诊断指令操作显示列表
	CString			m_edit_pc_id;			//pc节点ID
	CString			m_edit_avm_id;			//avm节点ID
	CString			m_edit_write;			//需要写入的数据
	CString			m_edit_param0;			//参数0
	CString			m_edit_param1;			//参数1
	CString			m_edit_param2;			//参数2
	CString			m_edit_param3;			//参数3
	CString			m_edit_param4;			//参数4
	CEdit			m_edit_canwrite;		//CAN指令写入

	CButton			m_btn_update;			//升级按钮

	ECanDriver		can_driver;				//can驱动
	common			comm;					//通用函数
	saver			safe;					//安全验证算法
	logout			log;					//日志信息

	int				pc_id;					//pc节点ID
	int				avm_id;				//tbox节点ID

	CString			diag_tbox_info;			//诊断相关的字符串应用
	int				diag_did_info;			//did索引
	int				diag_data_type;			//=0，写入的是字符串
										//=1，写入的是数据

	HANDLE			h_log_mutex;

	int				diag_write_step;		//diag写数据状态机
	int				diag_wait_time;			//写数据计时变量
	CAN_OBJ			diag_write;				//记录需要写入的数据
	
	/* 信息显示函数 */
	void			DispPrintf(CString str, unsigned char* pData,int len,int type );
	/* 信息打印函数 */
	void			DebugPrintf(int mode, CString str, unsigned char* pData,int len,int type );
	/* can ID初始化函数 */
	void			flush_can_id(int *p_tx, int *p_rx);
	/* 诊断错误，打印错误信息 */
	int				WriteToTboxErr(CString str);
	/* 诊断数据读取分析函数 */
	int				GetTboxReadInfo(CAN_OBJ can_data);
	/* 处理发送给PC的数据响应协议 */
	int				diag_ack_data(CAN_OBJ can_sour,int sid);
	/* 处理发送给PC的请求协议 */
	int				diag_request_cmd(CAN_OBJ can_sour,int sid,int sub);
	/* CAN数据接收处理程序 */
	int				diag_receive_proc(CAN_OBJ can_sour);

	/* asc字符转换成hex变量 */
	unsigned char	asc_to_hex(unsigned char data);
	/* 发送错误应答帧 */
	int				SendDiagErrFrame(CAN_OBJ can_sour, unsigned char sid, unsigned char err);
	/* 开始发送数据到TBOX，触发状态机 */
	int				WriteToTboxStart(void);
	
	/* 向TBOX写入数据流程，需要安全认证 */
	int				WriteInfoToTbox(CAN_OBJ can_sour);
	/* 初始化diag列表 */
	int				init_diag_info(void);
	/* 刷新当前diag信息 */
	int				disp_diag_info(int sel);
	/* 发送diag读写状态 */
	int				get_diag_rw_status(int sel);
	/* 发送diag参数状态 */
	int				get_diag_param_status(int sel);
	/* 发送diag指令 */
	int				send_diag_cmd(int sel,int mode);
	/* 发送诊断请求 */
	int				read_diag_cmd_to_tbox(TBOX_READ_INFO* p_info);
	/* 发送正常输入指令 */
	int				write_norm_cmd_to_tbox(TBOX_READ_INFO* p_info,unsigned char id);
	/* 发送正常数据 */
	int				write_norm_data_to_tbox(TBOX_READ_INFO* p_info,unsigned char id);
	/* 发送读数据指令 */
	int				read_norm_cmd_to_tbox(TBOX_READ_INFO* p_info,unsigned char mode);
	/* 发送手动写入的CAN指令 */
	void			send_diag_cmd_manual(CString str);

	/* DiagReceiveThread 消息处理程序 */
	static UINT		DiagReceiveThread(void *param);

	static UINT		TestThreadProc(void *param);
	static void		DelayUs(int uDelay);  
	void			SendTestCanInfo(CSGMW_diagDlg *p_diag, WORD can_id);

	BOOL			into_batch_cmd_list(int sel, int count);

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonCanConnect();
	afx_msg void OnBnClickedButtonReadDid();
	afx_msg void OnBnClickedButtonWriteDid();
	afx_msg void OnCbnSelchangeComboSelDid();
	afx_msg void OnBnClickedButtonBatchReadDid();
	afx_msg void OnNMRClickListBatchCmd(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCmd32771();
	afx_msg void OnCmd32772();
	afx_msg void OnBnClickedButtonCanwrite();
	afx_msg void OnBnClickedButtonSendFile();
	afx_msg void OnBnClickedButtonStartUpdate();
	afx_msg void OnBnClickedButtonCls();
	afx_msg void OnBnClickedButtonGetFileList();
	afx_msg void OnBnClickedButtonFileTransfer();
	afx_msg void OnNMRClickListSelFileList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnCbnSelchangeComboFileList();
	afx_msg void OnBnClickedButtonFileDelete();
	afx_msg void OnCbnSelchangeComboParam0();
	afx_msg void OnCbnSelchangeComboParam1();
	afx_msg void OnCbnSelchangeComboParam2();
	afx_msg void OnCbnSelchangeComboParam3();
	afx_msg void OnCbnSelchangeComboWriteMode();
	afx_msg void OnLbnDblclkListCmd();
	afx_msg void OnLbnDblclkListDid();
	afx_msg void OnBnClickedCheckVar();
	afx_msg void OnBnClickedCheckSelectAll();
	afx_msg void OnBnClickedButtonTest();

public:
	CString strRootDir;								//工程根目录
	CRITICAL_SECTION m_csLog;						//写日志临界区变量
	CProgressCtrl m_cProgress;						//文件进度条
	CButton m_check_print;							//信息打印
	bool handle_flag;								//处理标志 --> 0-不处理	 1-处理
	int handle_type;								//处理类型 --> 1-升级	 2-传输
	int block_cnt;									//数据块计数器（从1开始计数）
	int module_cnt;									//模块计数器（从1开始计数）
	int file_cnt;									//文件计数器（从1开始计数）

	DWORD m_dwCRC;									//文件CRC校验码		
	CString m_strPath;								//文件路径		
	DWORD m_file_size;								//文件大小
	//unsigned char m_file_data[FILE_MAX_SIZE];		//文件数据
	WORD m_block_size;								//数据块最大长度
	int m_block_num;								//数据块数量
	int trans_mod;									//传输类型
	DWORD diag_start_tm;							//事件开始时间		

	void ShowInfo(CString str, int sec);
	CString CanErrHandle(unsigned char func);
	void CheckDirectory(TCHAR* FileName);
	void InitParam();
	void ShowOrHideControl(bool var);

public:
	int	diag_update_step;							//diag升级状态机
	int m_temp_len_before;							//前n-1块的CAN数据长度
	int m_temp_len_last; 							//第n块的CAN数据长度
	int m_remain_len;								//第n块的文件数据长度
	DWORD dwFilePos;								//文件字节计数
	//int m_module_num;								//文件模块数量（模块大小按64K处理，最后一块不足64K时，按实际大小处理）
	//DWORD m_module_size;							//文件模块大小
	//unsigned char m_module_data[64*KB_1];			//文件模块数据

	int GetFileInfo(CString str, DWORD &dwcrc, DWORD &size);
	void Start_Update();
	int UpdateToTbox(CAN_OBJ can_sour);

public:
	CComboBox m_comb_file_list;
	CListCtrl m_list_sel_file;
	int	diag_transfer_step;							//diag升级状态机
	int m_file_list_pack_num;						//文件列表包数
	int m_file_num;									//传输文件个数
	WORD m_file_list_pack_size;						//文件列表包大小						
	DWORD m_file_list_size;							//文件列表大小
	unsigned char m_file_list_data[10*KB_1];		//文件列表数据
	DWORD f_index;									//文件列表数据计数
	CString strDirName;
	CFile pFile;

	void Start_Transfer(int step);
	int TransferFromTbox(CAN_OBJ can_sour);

public:
	void InitWriteOperate();

	unsigned char diag_write_id;//
	CComboBox m_combo_write_mode;
	CComboBox m_combo_param0;
	CComboBox m_combo_param1;
	CComboBox m_combo_param2;
	CComboBox m_combo_param3;
	TBOX_WRITE_INFO write_info;
	CDisplayDlg m_DisplayDlg;

	CButton m_check_var;		//控制测试界面的切换
	
	CEditTable m_edit_table;	//可编辑的list_control

	//int sel_can_id_num;			//选中的测试can_id数目
	
public:
	//HANDLE hThead[CAN_INFO_NUM];			//用于存储线程句柄  
	//DWORD dwThreadID[CAN_INFO_NUM];		//用于存储线程的ID  
	//DWORD dwExitCode[CAN_INFO_NUM];		//线程退出码
	CWinThread* cwThread[CAN_INFO_NUM];
    afx_msg void OnStnClickedStaticFilePath();
    afx_msg void OnBnClickedButton2();
	afx_msg void OnCbnSelchangeComboFwtype();
	afx_msg void OnCbnSelchangeComboDiffud();
	afx_msg void OnEnChangeEdit1();
};


extern deque<TBOX_READ_INFO> m_dequeBatchCmd;
extern deque<TBOX_FILE_LIST> m_dequeFileList;
extern deque<TBOX_FILE_LIST> m_dequeSelFile;
extern deque<TBOX_CAN_ID_INFO> m_dequeCanInfo;

extern TBOX_CAN_ID_INFO can_id_info[];

extern CSGMW_diagDlg* p_dlg;
