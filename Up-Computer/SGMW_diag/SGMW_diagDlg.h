
// SGMW_diagDlg.h : ͷ�ļ�
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


// CSGMW_diagDlg �Ի���
class CSGMW_diagDlg : public CDialogEx
{
// ����
public:
	CSGMW_diagDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SGMW_DIAG_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox		m_comb_did;				//DID��ȡ�б�
	CListCtrl		m_list_batch_cmd;		//��ȡcmd�����б�
	CComboBox		m_comb_debug;			//��ӡ��Ϣ
	CComboBox		m_comb_diffud;			//�������
	CComboBox		m_comb_fwtype;			//��������
	CListBox		m_list_did;				//did������ʾ�б�
	CListBox		m_list_cmd;				//���ָ�������ʾ�б�
	CString			m_edit_pc_id;			//pc�ڵ�ID
	CString			m_edit_avm_id;			//avm�ڵ�ID
	CString			m_edit_write;			//��Ҫд�������
	CString			m_edit_param0;			//����0
	CString			m_edit_param1;			//����1
	CString			m_edit_param2;			//����2
	CString			m_edit_param3;			//����3
	CString			m_edit_param4;			//����4
	CEdit			m_edit_canwrite;		//CANָ��д��

	CButton			m_btn_update;			//������ť

	ECanDriver		can_driver;				//can����
	common			comm;					//ͨ�ú���
	saver			safe;					//��ȫ��֤�㷨
	logout			log;					//��־��Ϣ

	int				pc_id;					//pc�ڵ�ID
	int				avm_id;				//tbox�ڵ�ID

	CString			diag_tbox_info;			//�����ص��ַ���Ӧ��
	int				diag_did_info;			//did����
	int				diag_data_type;			//=0��д������ַ���
										//=1��д���������

	HANDLE			h_log_mutex;

	int				diag_write_step;		//diagд����״̬��
	int				diag_wait_time;			//д���ݼ�ʱ����
	CAN_OBJ			diag_write;				//��¼��Ҫд�������
	
	/* ��Ϣ��ʾ���� */
	void			DispPrintf(CString str, unsigned char* pData,int len,int type );
	/* ��Ϣ��ӡ���� */
	void			DebugPrintf(int mode, CString str, unsigned char* pData,int len,int type );
	/* can ID��ʼ������ */
	void			flush_can_id(int *p_tx, int *p_rx);
	/* ��ϴ��󣬴�ӡ������Ϣ */
	int				WriteToTboxErr(CString str);
	/* ������ݶ�ȡ�������� */
	int				GetTboxReadInfo(CAN_OBJ can_data);
	/* �����͸�PC��������ӦЭ�� */
	int				diag_ack_data(CAN_OBJ can_sour,int sid);
	/* �����͸�PC������Э�� */
	int				diag_request_cmd(CAN_OBJ can_sour,int sid,int sub);
	/* CAN���ݽ��մ������ */
	int				diag_receive_proc(CAN_OBJ can_sour);

	/* asc�ַ�ת����hex���� */
	unsigned char	asc_to_hex(unsigned char data);
	/* ���ʹ���Ӧ��֡ */
	int				SendDiagErrFrame(CAN_OBJ can_sour, unsigned char sid, unsigned char err);
	/* ��ʼ�������ݵ�TBOX������״̬�� */
	int				WriteToTboxStart(void);
	
	/* ��TBOXд���������̣���Ҫ��ȫ��֤ */
	int				WriteInfoToTbox(CAN_OBJ can_sour);
	/* ��ʼ��diag�б� */
	int				init_diag_info(void);
	/* ˢ�µ�ǰdiag��Ϣ */
	int				disp_diag_info(int sel);
	/* ����diag��д״̬ */
	int				get_diag_rw_status(int sel);
	/* ����diag����״̬ */
	int				get_diag_param_status(int sel);
	/* ����diagָ�� */
	int				send_diag_cmd(int sel,int mode);
	/* ����������� */
	int				read_diag_cmd_to_tbox(TBOX_READ_INFO* p_info);
	/* ������������ָ�� */
	int				write_norm_cmd_to_tbox(TBOX_READ_INFO* p_info,unsigned char id);
	/* ������������ */
	int				write_norm_data_to_tbox(TBOX_READ_INFO* p_info,unsigned char id);
	/* ���Ͷ�����ָ�� */
	int				read_norm_cmd_to_tbox(TBOX_READ_INFO* p_info,unsigned char mode);
	/* �����ֶ�д���CANָ�� */
	void			send_diag_cmd_manual(CString str);

	/* DiagReceiveThread ��Ϣ������� */
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
	CString strRootDir;								//���̸�Ŀ¼
	CRITICAL_SECTION m_csLog;						//д��־�ٽ�������
	CProgressCtrl m_cProgress;						//�ļ�������
	CButton m_check_print;							//��Ϣ��ӡ
	bool handle_flag;								//�����־ --> 0-������	 1-����
	int handle_type;								//�������� --> 1-����	 2-����
	int block_cnt;									//���ݿ����������1��ʼ������
	int module_cnt;									//ģ�����������1��ʼ������
	int file_cnt;									//�ļ�����������1��ʼ������

	DWORD m_dwCRC;									//�ļ�CRCУ����		
	CString m_strPath;								//�ļ�·��		
	DWORD m_file_size;								//�ļ���С
	//unsigned char m_file_data[FILE_MAX_SIZE];		//�ļ�����
	WORD m_block_size;								//���ݿ���󳤶�
	int m_block_num;								//���ݿ�����
	int trans_mod;									//��������
	DWORD diag_start_tm;							//�¼���ʼʱ��		

	void ShowInfo(CString str, int sec);
	CString CanErrHandle(unsigned char func);
	void CheckDirectory(TCHAR* FileName);
	void InitParam();
	void ShowOrHideControl(bool var);

public:
	int	diag_update_step;							//diag����״̬��
	int m_temp_len_before;							//ǰn-1���CAN���ݳ���
	int m_temp_len_last; 							//��n���CAN���ݳ���
	int m_remain_len;								//��n����ļ����ݳ���
	DWORD dwFilePos;								//�ļ��ֽڼ���
	//int m_module_num;								//�ļ�ģ��������ģ���С��64K�������һ�鲻��64Kʱ����ʵ�ʴ�С����
	//DWORD m_module_size;							//�ļ�ģ���С
	//unsigned char m_module_data[64*KB_1];			//�ļ�ģ������

	int GetFileInfo(CString str, DWORD &dwcrc, DWORD &size);
	void Start_Update();
	int UpdateToTbox(CAN_OBJ can_sour);

public:
	CComboBox m_comb_file_list;
	CListCtrl m_list_sel_file;
	int	diag_transfer_step;							//diag����״̬��
	int m_file_list_pack_num;						//�ļ��б����
	int m_file_num;									//�����ļ�����
	WORD m_file_list_pack_size;						//�ļ��б����С						
	DWORD m_file_list_size;							//�ļ��б��С
	unsigned char m_file_list_data[10*KB_1];		//�ļ��б�����
	DWORD f_index;									//�ļ��б����ݼ���
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

	CButton m_check_var;		//���Ʋ��Խ�����л�
	
	CEditTable m_edit_table;	//�ɱ༭��list_control

	//int sel_can_id_num;			//ѡ�еĲ���can_id��Ŀ
	
public:
	//HANDLE hThead[CAN_INFO_NUM];			//���ڴ洢�߳̾��  
	//DWORD dwThreadID[CAN_INFO_NUM];		//���ڴ洢�̵߳�ID  
	//DWORD dwExitCode[CAN_INFO_NUM];		//�߳��˳���
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
