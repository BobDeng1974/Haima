#pragma once

#define FILE_MAX_SIZE						0x100000000			//文件空间上限 -> 4GB
//#define FILE_MAX_SIZE						0x400000			//文件空间上限 -> 4MB
#define KB_1								0x400				//1kb	
#define UNLIMIT								0xffffffff			//当发送次数值为0xffffffff时，发送次数不受限	
#define CAN_INFO_NUM						0x27				//设置CAN信息数目为36+3
#define GET_FILE_DIRECTORY					"SGMW_GetFiles"		//tbc文件目录
#define PRINT_LOG							"print.log"			//打印日志


/* 错误帧消息处理 */
#define NO_ERR								0x00	//正确
#define SID_ERR								0x11	//服务不支持
#define SUB_ERR								0x12	//不支持子功能
#define FOR_ERR								0x13	//不正确的消息长度和无效的格式
#define CONDITION_ERR						0x22	//条件不正确
#define SEQUENCE_ERROR						0x24	//请求序列错误
#define OUTRANGE_ERR						0x31	//请求超出范围
#define SECURITY_ERR						0x33	//安全访问拒绝
#define INVALID_KEY_ERR						0x35   	//密钥无效
#define OVER_TRY_ERR          				0x36    //超出尝试次数
#define DELAY_NOT_ARRIVAL_ERR 				0x37    //所需时间延迟未到
#define UPLOAD_DOWNLOAD_NOT_ACCEPTED        0x70	//不允许上传下载
#define TRANSFER_DATA_SUSPENDED				0x71	//数据传输暂停
#define PROGRAMME_ERR         				0x72	//一般编程失败
#define WRONG_BLOCK_SEQUENCE_COUNTER		0x73	//错误的数据块序列计数器
#define REQ_CORRECTLY_REC_RESP_PENDING  	0x78 	//正确接收请求消息-等待响应
#define SUB_FUNC_NOT_SUPPORT_ACTIVE_SES 	0x7E 	//激活会话不支持该子服务
#define NOT_SUPPERT_SUB_IN_ACTIVE_SESSION   0x7F	//激活会话不支持该服务
#define BUFF_OVER_FLOW						0xFF	//the buff is overflow
#define PROGRAMME_ERR         				0x72	//一般编程失败


/* ISO14429协议命令字 */
#define DIAGNOSITIC_SESSION_CONTROL			0x10
#define ECU_RESET							0x11
#define CLEAR_DIAGNOSTIC_INFORMATION		0x14
#define READ_DTC_INFORMATION				0x19
#define READ_DATA_BY_IDENTIFIER				0x22
#define SECURITY_ACCESS						0x27
#define COMMUNICATION_CONTROL				0x28
#define WRITE_DATA_BY_IDENTIFIER			0x2E
#define INPUT_OUTPUT_CONTROL_BY_IDENTIFIER	0x2F
#define ROUTINE_CONTROL						0x31
#define TESTER_PRESENT						0x3E
#define CONTROL_DTC_SETTING					0x85
#define ERR_DTC_ID							0x7F


/* 数据写入状态机 */
#define WRITE_TO_TBOX_NULL					0		//无动作
#define WRITE_TO_TBOX_EXTERN_MODE			1		//进入扩展模式
#define WRITE_TO_TBOX_WAIT_EXTERN_MODE		2		//等待进入扩展模式
#define WRITE_TO_TBOX_READ_SN				3		//读SN
#define WRITE_TO_TBOX_WAIT_READ_SN			4		//等待读取SN
#define WRITE_TO_TBOX_READ_SEED				5		//读种子
#define WRITE_TO_TBOX_WAIT_READ_SEED		6		//等待读取种子
#define WRITE_TO_TBOX_READ_PASSWORD			7		//读密钥
#define WRITE_TO_TBOX_WAIT_READ_PASSWORD	8		//等待读取密钥
#define WRITE_TO_TBOX_WRITE_DATA			9		//写入数据
#define WRITE_TO_TBOX_WAIT_WRITE_DATA		10		//等待数据写入完成



/* 文件传输状态机 */
#define TRANS_FROM_TBOX_NULL				0		//无动作
#define TRANS_FROM_TBOX_EXTERN_MODE			1		//进入扩展模式
#define TRANS_FROM_TBOX_WAIT_EXTERN_MODE	2		//等待进入扩展模式
#define TRANS_FROM_TBOX_READ_SN				3		//读SN
#define TRANS_FROM_TBOX_WAIT_READ_SN		4		//等待读取SN
#define TRANS_FROM_TBOX_READ_SEED			5		//读种子
#define TRANS_FROM_TBOX_WAIT_READ_SEED		6		//等待读取种子
#define TRANS_FROM_TBOX_READ_PASSWORD		7		//读密钥
#define TRANS_FROM_TBOX_WAIT_READ_PASSWORD	8		//等待读取密钥
#define TRANS_FROM_TBOX_REQUEST_READDIR		9		//文件列表读取申请
#define TRANS_FROM_TBOX_TRANSFER_DIRDATA	10		//文件列表传输
#define TRANS_FROM_TBOX_TRANSFER_DIREXIT	11		//文件列表传输退出
#define TRANS_FROM_TBOX_REQUEST_READFILE	12		//文件读取申请
#define TRANS_FROM_TBOX_TRANSFER_FILEDATA	13		//文件数据传输
#define TRANS_FROM_TBOX_TRANSFER_FILEEXIT   14		//文件传输退出
#define TRANS_FROM_TBOX_TRANSFER_CHECK	    15		//文件CRC校验
#define TRANS_FROM_TBOX_REQUEST_DELETE      16		//文件删除申请
#define TRANS_FROM_TBOX_TRANSFER_END		17		//传输完成


/* 处理类型 */
#define NULL_TYPE							0		//空类型
#define UPDATE_TYPE							1		//升级类型
#define TRANSFER_TYPE						2		//传输类型

/* 传输类型 */
#define TRANS_NULL							0		//无传输
#define TRANS_LIST							1		//文件列表传输
#define TRANS_FILE							2		//文件传输
#define DELETE_FILE							3		//删除文件


#define GET_BYTE_24(n)						((unsigned char)((n>>24) & 0xff))
#define GET_BYTE_16(n)						((unsigned char)((n>>16) & 0xff))
#define GET_BYTE_8(n)						((unsigned char)((n>>8) & 0xff))
#define GET_BYTE_0(n)						((unsigned char)((n>>0) & 0xff))


#define GET_5WORD(p_temp) ((p_temp[0]<<32) + (p_temp[1]<<24) + (p_temp[2]<<16) + (p_temp[3]<<8) + (p_temp[4]))
#define GET_DWORD(p_temp) ((p_temp[0]<<24) + (p_temp[1]<<16) + (p_temp[2]<<8) + (p_temp[3]))
#define GET_MWORD(p_temp) ((p_temp[0]<<16) + (p_temp[1]<<8) + (p_temp[2]))
#define GET_WORD(p_temp)  ((p_temp[0]<<8) + (p_temp[1]))


typedef struct __tbox_read_info
{
	unsigned char		read_id;		//读操作ID
	unsigned char		write_id;		//写操作ID

	unsigned long		len;			//数据长度
	unsigned long		act_mode;		//读写模式
	unsigned long		did;			//didi

	unsigned char		param0;			//参数0
	unsigned char		param1;			//参数1
	unsigned char		param2;			//参数2
	unsigned char		param3;			//参数3
	unsigned char		param4;			//参数4

	CString				str;			//描述数据
}TBOX_READ_INFO;


typedef struct __tbox_file_list
{
	CString file_name;
	DWORD file_serial;
	DWORD file_length;
	unsigned char file_buf[8];
}TBOX_FILE_LIST;


typedef struct __tbox_write_info
{
	DWORD param0;
	DWORD param1;
	DWORD param2;
	DWORD param3;
}TBOX_WRITE_INFO;


typedef struct __tbox_can_id_info
{
	WORD can_id;
	DWORD send_period;
	DWORD send_cnt;

}TBOX_CAN_ID_INFO;


typedef struct __tbox_wait_time
{
	DWORD time;		//等待时间
	bool status;	//等待状态

}TBOX_WAIT_TIME;


#define AVM_FUN_ID              0x7DF          //潍柴功能寻址地址

typedef enum {

	 /* 文件升级状态机 */
	 UPDATE_TO_TBOX_NULL = 0,				    //无动作
	 UPDATA_TO_AVM_FUN_PREPROGRAM,                  //功能寻址预编程阶段
	 UPDATA_TO_AVM_FUN_AFTER_PROGRAM,               //功能寻址编程后阶段
	 UPDATA_TO_AVM_FUN_NEGATIVE_RESPONSE,           //功能寻址负响应处理阶段

	 UPDATA_TO_AVM_ENTER_EXTMODE,               //进入扩展模式
	 UPDATA_TO_AVM_WAIT_ENTER_EXTMODE,          //等待进入扩展模式
	 UPDATA_TO_AVM_STOP_SET_DTC,                //停止设定DTC
	 UPDATE_TO_AVM_COMMUNICATE_CONTROL,		    //通信控制
	 UPDATE_TO_AVM_READ_SN,                    //读序列号
	 UPDATE_TO_AVM_PROGRAM_MODE,				//进入编程模式
	 UPDATE_TO_AVM_WAIT_PROGRAM_MODE,			//等待进入编程模式
	 UPDATE_TO_AVM_READ_SEED,					//读种子
	 UPDATE_TO_AVM_WAIT_READ_SEED,				//等待读取种子
	 UPDATE_TO_AVM_READ_PASSWORD,				//读密钥
	 UPDATE_TO_AVM_WAIT_READ_PASSWORD,			//等待读取密钥
	 UPDATE_TO_AVM_WRITE_FINGERPRINT,           //写指纹
	 UPDATE_TO_AVM_EARSE_MEMORY,				//内存擦除
	 UPDATE_TO_AVM_DOWNLOAD_FLASH_DRV,			//下载flash驱动
	 UPDATE_TO_AVM_REQUEST_DOWNLOAD,			//请求下载
	 UPDATE_TO_AVM_TRANSFER_DATA,				//块数据传输
	 UPDATE_TO_AVM_TRANSFER_DATA_EXIT,		    //退出数据传输
	 UPDATE_TO_AVM_UPDATE_CHECK,				//文件下载校验1
	 UPDATE_TO_AVM_WRITE_DATA,                  //写日期
	 UPDATE_TO_AVM_WRITE_VER,                  //写软件版本
	 UPDATE_TO_AVM_UPDATE_END,					//升级完成（终端复位）
	 UPDATE_TO_AVM_RESET,					    //终端复位
	 UPDATA_TO_AVM_ENTER_EXTMODE_AGAIN,         //再次进入扩展模式
	 UPDATE_TO_AVM_RECOVER_COMMUNICATE_CONTROL, //恢复通行控制
	 UPDATA_TO_AVM_START_SET_DTC,               //开启设定DTC
	 UPDATA_TO_AVM_ENTER_DEFMODE,               //进入默认会话模式
	 UPDATA_TO_AVM_CLEAR_DTC,                   //清除DTC
	 UPDATA_TO_AVM_UPDATE_END,                  //升级完成
}update_stat;