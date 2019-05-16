#pragma once

#include "ECanVci.h"
#include "RingBuf.h"
#pragma warning(disable:4430)
#include <map>
#include "afxcmn.h"
using namespace std;

#define	CAN_UNCONNECT							0	//未连接状态
#define	CAN_CONNECT_OK							1	//连接正常
#define CAN_CONNECT_CHECK_CODE_ERR				2	//验收码错误
#define CAN_CONNECT_SHEILD_CODE_FORMAT_ERR		3	//屏蔽码格式错误
#define	CAN_CONNECT_TIMER0_FORMAT_ERR			4	//定时器0格式不对
#define	CAN_CONNECT_TIMER1_FORMAT_ERR			5	//定时器1格式不对
#define	CAN_CONNECT_OPEN_DRIVER_ERR				6	//设备打开失败
#define	CAN_CONNECT_INIT_DRIVER_ERR				7	//设备初始化失败
#define CAN_CONNECT_START_ERR					8	//设备开始失败

#define CAN_RESET_OK							0	//can驱动复位成功
#define	CAN_RESET_FAILED						1	//can驱动复位失败
#define CAN_RESET_NOT_OPEN						2	//can驱动没有打开

#define	CAN_NO									0
#define	CAN_YES									1

#define	MAX_DEV_TYPE_NUM						16
#define MAX_LARGE_SEND_ID						32
#define MAX_LARGE_SIZE							4096

#define CAN_LARGE_NO							0
#define	CAN_LARGE_SEND							1
#define	CAN_LARGE_RECEIVE						2

typedef struct __CAN_OBJ
{
	int				id;
	int				len;
	unsigned char	buf[MAX_LARGE_SIZE];
}CAN_OBJ;

typedef struct __CAN_LARGE_DATA
{
	int				id;			//当前ID值
	int				ack;		//CAN_NO，表示当前大数据发送未收到许可应答
								//CAN_YES，表示当前大数据接收到了许可应答
	int				len;		//发送或者接受数据的长度
	int				flag;		//当前id是否正在使用
	int				start;		//当前数据开始位置
	int				rx_id;		//接受ID
	int				tx_id;		//发送ID
	int				serial;		//流水号
	unsigned char	data[MAX_LARGE_SIZE];	//数据存放
}CAN_LARGE_DATA;

class ECanDriver
{
public:
	ECanDriver(void);
	~ECanDriver(void);

	/* 设备类型名称及型号 */
	map<CString,int>			m_mapDevtype;
	int							m_devtype;



	/* 设备名称处理 */
	LPCTSTR						dev_type_str_list[MAX_DEV_TYPE_NUM];
	int							dev_type_str_num;
	int							dev_type_select;
	CString						dev_type_str;

	/* can通道处理 */
	CString						dev_can_channel_str_list[MAX_DEV_TYPE_NUM];
	int							dev_can_channel_str_num;
	int							dev_can_channel_select;

	/* 索引号 */
	CString						dev_can_idx_str_list[MAX_DEV_TYPE_NUM];
	int							dev_can_idx_str_num;
	int							dev_can_idx_select;

	/* 波特率 */
	CString						dev_can_bitrate_str_list[MAX_DEV_TYPE_NUM];
	int							dev_can_bitrate_str_num;
	int							dev_can_bitrate_select;

	/* 滤波 */
	CString						dev_can_fliter_str_list[MAX_DEV_TYPE_NUM];
	int							dev_can_fliter_str_num;
	int							dev_can_fliter_select;

	/* 模式 */
	CString						dev_can_mode_str_list[MAX_DEV_TYPE_NUM];
	int							dev_can_mode_str_num;
	int							dev_can_mode_select;

	/* 验收码 */
	CString						unmask_code;
	/* 屏蔽码 */
	CString						mask_code;
	/* can驱动互斥量句柄 */
	HANDLE						h_mutex;
	/* 数据接收ringbuf */
	RingBuf						can_receive_buf;

	/* 批量发送数据时使用，发送帧头信息成功后，等待终端接收应答 */
	int							send_head_ack;

	/* 大数量发送中标志 */
	int							large_data_sending;
	/* 大数据发送ID */
	int							large_data_send_id;
	/* 允许大数据量发送的ID列表 */
	CAN_LARGE_DATA				large_data_id_list[MAX_LARGE_SEND_ID];
	/* 最大允许大数据发送的ID个数 */
	int							large_data_id_num;
	/* can驱动状态 */
	int		m_can_link_status;

	/* CAN数据接收线程 */
	static UINT ReceiveThread(void *param);

	/* CAN心跳数据发送线程 */
	static UINT TransmitHeartBeat(void *param);

	/* 初始化can驱动变量 */
	int		CanDriverInit();
	/* 关闭can驱动 */
	int		CanDriverClose();
	/* can驱动连接 */
	int		CanDriverConnect();
	/* 断开can驱动 */
	int		CanDriverUnconnect();
	/* 复位can驱动 */
	int		CanDriverReset();
	/* 获得当前can驱动状态 */
	int		GetCanDriverStatus();
	/* 发送数据 */
	int		TransmitData(unsigned int id, unsigned char* p_data, int len);
	/* 增加允许大数据发送ID */
	int		AddLargeSendCanId(int tx_id,int rx_id);
	/* 清除允许大数据发送ID */
	int		ClearLargeSendCanId(void);
	/* 通过can id获取对应的同组ID */
	int		GetCanTxRxId(int tx_id, int rx_id);
protected:
	/* 发送N帧数据到CAN总线上 */
	int		TransmitDataSub(PVCI_CAN_OBJ pSend,int frame);
	/* 通过tx id或者rx id获取当前操作的大数据结构 */
	CAN_LARGE_DATA * GetCanLargePdata(int tx_id, int rx_id);
};

