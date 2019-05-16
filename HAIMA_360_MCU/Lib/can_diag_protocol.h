
#ifndef _CAN_DIAG_PROTOCOL_H
#define _CAN_DIAG_PROTOCOL_H
#include "stdint.h"
#include "miscdef.h"



#ifdef	CAN_DIAG_GLOBALS
#define	CAN_DIAG_EXT     //当CAN_DIAG_GLOBALS有定义的.c 模块时，编译器使CAN_DIAG_EXT强制为空
#else
#define	CAN_DIAG_EXT	extern//编译器处理未定义CAN_DIAG_GLOBALS的.c时，定义CAN_DIAG_EXT为extern(外部全部变量) 可供其它模块调用
#endif

//海马ID分配
#define AVM_Phy_Req           (0x752) //诊断设备到控制器的物理请求CAN标识符
#define FUNCATIONAL_REQ_ADDR  (0x7DF) //诊断设备功能寻址请求CAN标识符	
#define AVM_Phy_Resp          (0x753) //控制器到诊断设备的物理响应CAN标识符

#define FIX_NO_USE_DATA    0x00  //单桢和最后一桢连续有效数据如果不足8字节，未使用字节用0x00填充 

  /*****************address_type  start**********************/
  #define DIAG_P_ADDRESS                       0x01
#define DIAG_F_ADDRESS                       0x02
/*****************address_type  start*end*****************/

	
/*****************sid类型  start**********************/
#define DIAG_SESSION_SID                0x10 //会话模式
#define DIAG_ECU_RESET_SID              0x11 //复位
#define DIAG_CLR_DTC_SID                0x14 //清除故障　
#define DIAG_SECURITY_ACCESS_SID        0x27 //安全访问
#define DIAG_COMM_CONTROL_SID           0x28 //通信控制
#define DIAG_R_DIR_DID                  0x38 //读取文件目录列表
#define DIAG_TESTER_PRESENT_SID         0x3e //依断仪在线
#define DIAG_W_DID_SID                  0x2e //写标识符
#define DIAG_R_DID_SID                  0x22 //读标识符
#define DIAG_R_DTC_SID                  0x19 //读DTC
#define DIAG_DTC_CONTTOL_SID            0x85 //DTC控制
#define DIAG_ROUTE_CONTROL_SID          0x31 //例程控制

#define DIAG_REQUEST_DOWNLOAD_SID      (0X34)   // add by xzb 20170303
#define DIAG_TRANSFER_DATA_SID         (0X36)   // add by xzb 20170303
#define DIAG_REQUEST_TRANSFER_EXIT_SIG (0X37)   // add by xzb 20170303

/* 会话ack  sid(正响应应答值为sid+0x40)   add by taoqh 20170427 */
#define DIAG_SESSION_ACK_SID                (0x10|0x40) //会话模式
#define DIAG_ECU_RESET_ACK_SID              (0x11|0x40) //复位
#define DIAG_CLR_DTC_ACK_SID                (0x14|0x40) //清除故障　
#define DIAG_SECURITY_ACCESS_ACK_SID        (0x27|0x40) //安全访问
#define DIAG_COMM_CONTROL_ACK_SID           (0x28|0x40) //通信控制
#define DIAG_TESTER_PRESENT_ACK_SID         (0x3e|0x40) //依断仪在线
#define DIAG_W_DID_ACK_SID                  (0x2e|0x40) //写标识符
#define DIAG_R_DID_ACK_SID                  (0x22|0x40) //读标识符
#define DIAG_R_DTC_ACK_SID                  (0x19|0x40) //读DTC
#define DIAG_DTC_CONTTOL_ACK_SID            (0x85|0x40) //DTC控制
#define DIAG_ROUTE_CONTROL_ACK_SID          (0x31|0x40) //例程控制
#define DIAG_REQUEST_DOWNLOAD_ACK_SID       (0X34|0x40) //开始传送数据
#define DIAG_TRANSFER_DATA_ACK_SID          (0X36|0x40) //传送数据
#define DIAG_REQUEST_TRANSFER_EXIT_ACK_SIG  (0X37|0x40) //数据传送完毕
/*****************sid类型  end**********************/

/**************sesion_mode 会话模式 start******************/
#define DIAG_DE_MODE       0x01
#define DIAG_PR_MODE       0x02
#define DIAG_EX_MODE       0x03
/**************sesion_mode 会话模式 end******************/

/*****************安全级别状态 start***********************/
#define	DIAG_SECURITY_LEVEL_STATE_UNLOCK    	0  //要加前缀，防止其它也定义相同
#define	DIAG_SECURITY_LEVEL_STATE_LOCK		    1
/*****************安全级别状态 end***********************/

/*************dtc_function_enabled start*********************/
#define DIAG_DTC_FUNCTION_ON    0x01
#define DIAG_DTC_FUNCTIONOFF    0x02
/*************dtc_function_enabled end*********************/

/***************communication_enable start*******************/
#define DIAG_COMM_RX_AND_TX_ENABLE   0x00
#define DIAG_ENABLE_RX_AND_TX_DISABLE  0x01
#define DIAG_COMM_RX_AND_TX_DISABLE  0x03
/***************communication_enable end*******************/

/*************要保存的参数 start******************************/
//#define SAFC_1_add1   0x00
//#define SAFC_3_add1  (0x00+1)   
/*************要保存的参数 end******************************/

/**********************负响应 start**************************/
#define NO_ERR                             0x00    //正确
#define SID_ERR                            0x11    //服务不支持
#define SUB_ERR                            0x12    //不支持子功能
#define FOR_ERR                            0x13    //不正确的消息长度和无效的格式
#define CONDITION_ERR                      0x22    //条件不正确
#define SEQUENCE_ERROR                     0x24    //请求序列错误
#define OUTRANGE_ERR                       0x31    //请求超出范围
#define SECURITY_ERR                       0x33    //安全访问拒绝
#define INVALID_KEY_ERR                    0x35    //密钥无效
#define OVER_TRY_ERR                       0x36    //超出尝试次数
#define DELAY_NOT_ARRIVAL_ERR              0x37    //所需时间延迟未到
#define UPLOAD_DOWNLOAD_NOT_ACCEPTED       0x70    //不允许上传下载
#define TRANSFER_DATA_SUSPENDED            0x71    //数据传输暂停
#define PROGRAMME_ERR                      0x72    //一般编程失败
#define WRONG_BLOCK_SEQUENCE_COUNTER       0x73    //错误的数据块序列计数器
#define REQ_CORRECTLY_REC_RESP_PENDING     0x78    //正确接收请求消息-等待响应
#define SUB_FUNC_NOT_SUPPORT_ACTIVE_SES    0x7E    //激活会话不支持该子服务
#define NOT_SUPPERT_SUB_IN_ACTIVE_SESSION  0x7F    //激活会话不支持该服务
#define BUFF_OVER_FLOW                     0xFF    //the buff is overflow 
/**********************负响应 end**************************/



/**************SAFC_reach_3num_mcu_resetbit SAFC 达到三次复位标志 start*************/
#define DIAG_ECU_HAD_RESET     0x01
#define DIAG_ECU_HAD_NOT_RESET 0x00
/**************SAFC_reach_3num_mcu_resetbit SAFC 达到三次复位标志 end*************/

/***********security_level3_access_step  security_level1_access_step start***********/
#define DIAG_HAD_REQ_SEED   0x01
#define DIAG_NOT_REQ_SEED   0x02

/***********security_level3_access_step security_level1_access_step end***********/
//
#define ECU_SUPPORT_MSAK       				0x09 


// 安全访问种子 和ECU_MASK

#define  ECU_Mask_Boot  	(0x6a7c52e7) 
#define  ECU_Mask_APP      	(0x52c36b19)


typedef struct{
	
	uint8_t 	address_type;//地址类型　g_address_type　分功能寻址(广播)　物理寻址(单独对某一个ECU)
	uint8_t 	sesion_mode; //会话模式     g_session_model
	uint8_t 	security_level1_state; //一般安全状态 对应 server_lock_state_1
	//uint8_t 	security_level2_state; //防盗安全状态  no use
	uint8_t 	security_level3_state; //编程安全状态  server_lock_state_2
	uint8_t 	dtc_function_enable;  //DTC功能使能　dtc_setting_type	  别的地方用到disable_send_can_data用dtc_function_enable替代
	uint8_t 	communication_enable; //通信控制 Type  comm_control_type
	uint32_t    security_seed; //种子　　分Seed_App   Seed_Bootloader
	uint32_t    security_ECU_Mask;//种子 ECU_Mask_App   SECU_Mask_Bootloader    
	                    //失败数达到3次后，延迟三分钟，三分钟不能访问，三分钟延迟完成SAFC清掉,如果达到3次，重启，种子可以出，但密码时会提示访问次数过来，返回失败
	uint8_t 	SAFC_1; //一般安全 安全访问失败计数  SAFC_1 
	uint32_t 	time_SAFC_1; //一般安全 安全访问失败计数达到三次时间点 timing_SAFC_1
	uint8_t     security_level1_access_step; //安全访问步骤　req_seed_state_1
	uint8_t     security_level3_access_step; //安全访问步骤　req_seed_state_2
	uint8_t 	SAFC_3;  //编程安全状态 安全访问失败计数  SAFC_2
	uint32_t 	time_SAFC_3; //编程安全 安全访问失败计数达到三次时间点 timing_SAFC_2
	uint8_t     SAFC_reach_3num_mcu_resetbit:1; //SAFC 达到三次复位标志 g_ecu_had_reset_just_now  #define ECU_HAD_RESET     0x01 #define ECU_HAD_NOT_RESET 0x00
	uint8_t     file_trans_mode; //04 文件数据传输 05 文件列表传输
	uint32_t    file_len;        //文件或文件列表长度
	uint32_t 	rev_frame_tim; //接收到任何桢时间
	//u8          update_type;//0升级MCU_APP  非0 升级ARM
}CAN_DIAG_TYPE;
extern CAN_DIAG_TYPE diag; 

//应用层时间参数
#define OS_1MS     1
typedef struct
{
	uint16_t P2can;//诊断仪（PC）诊断仪在成功发送完诊断请求报文后，等待ECU发送诊断响应的超时时间ISO 15765-3中该参数名为：P2CAN_Client
	uint16_t P2_asterisk_can; //ms P2*can;当服务器在发送否定响应码为0x78 的否定响应后，到服务器发出响应消息时间的性能要求
	uint16_t S3_sever; //暂时服务端不用做ms 服务器的定时参数：仅用于非默认会话模式，在S3Server时间内，如果服务器没有接收到任何诊断请求报文，则退出非默认会话模式，返回默认会话模式 
}SERVER_APPLICATION_LAYER_TIM_TYPE; //服务器 时间参数


typedef struct
{
	uint16_t P2can;//AVM接收到请求消息后发送响应消时间的性能要求 ms
	uint16_t P2_asterisk_can; //ms P2*can;当服务器在发送否定响应码为0x78 的否定响应后，到服务器发出响应消息时间的性能要求
	uint16_t P3_can_phy;/*客户端成功发送物理寻址请求消息，并且该请求消息不需要服务器进行响应后，客户端再次发送
                        下一个物理寻址请求消息的最小间隔时间 */
	uint16_t P3_can_func; /*客户端成功发送完功能寻址请求消息，并再次发送下一个功能寻址请求消息时的最小间隔时间，
                        分两种情况：一种为客户端发送的请求不需要服务器响应，一种为仅由部分支持该请求的服务器
                        响应，即存在部分服务器不给予响应。 */
	uint16_t S3_client; //ms 服务器的定时参数：仅用于非默认会话模式，在S3Server时间内，如果服务器没有接收到任何诊断请求报文，则退出非默认会话模式，返回默认会话模式 
	uint16_t S3_Tester;//诊断仪发送两帧诊断仪在线(3E hex)请求报文使ECU保持在非默认会话模式下的最大时间间隔
}CLIENT_APPLICATION_LAYER_TIM_TYPE; //应用层会话管理计时器


//车身报文信息
typedef struct body_car_rec_info
{	//from 0x1f7
	u8 GearSelDisp;//档位选择
	u16 VehSpd;//车速
	u8 VehSpdVd;//车速是否有效
	//0x22C
	u8 DirLampLeftSwitchSts;//左转向
	u8 DirLampRightSwitchSts; //右转向
	u8 RsvGearSwitchSts;//手动变速器倒档信号
	//0x3D4
	s16 StrWhlAngle; //当前方向盘转向角
	u8 StrWhlAngleSign;//方向盘转动方向
	u16 StrWhlSpd;//方向盘转动速度

	//0x539
	u8 AvmGuidesSet; //全景辅助线设置
	u8 AVM_LDW_SET;//360 车道偏离功能状态设置
	u8 SET_CAR_color; //颜色
	u8 TouScrPreKey2;//触摸按键位置2
	u8 Avm2D3DSwitch;//
	//0x6C2
	u32 AVMOneKeyCaltion;//设置一键标定 用于车辆下线标定
	u8 AVMOneKeyCaltionVd;//一键标定标志位
}body_car_rec_info_t;

//发送报文
typedef struct body_car_send_info
{	
	u8	AVM_SET_AVM_DISP;
	u8	AVM_CalibrationStatus;
	u8  AVM_CamCaliFault;

	u8  AVM_ActivationStatus;
	u8	AVM_DispViewStatus;
	
}body_car_send_info_t;


CAN_DIAG_EXT u32 app_diagnose(	device_info_t *dev,uint8_t *buffer,uint16_t len);
CAN_DIAG_EXT uint8_t get_diag_sesion_mode(void);
CAN_DIAG_EXT uint8_t get_diag_address_type(void);
CAN_DIAG_EXT uint8_t get_diag_communication_state(void);
CAN_DIAG_EXT uint8_t get_diag_dtc_function_state(void);

CAN_DIAG_EXT void set_diag_address_type(uint8_t type);
CAN_DIAG_EXT void init_diag_protocol_parameer(void);
CAN_DIAG_EXT void init_diag_protocol_parameer_boot(void);
CAN_DIAG_EXT void diag_send_data(device_info_t *dev, u8 *Data, u16 len );
CAN_DIAG_EXT void check_s3_client(device_info_t *dev);


//#define ROUTINE_RESULT_SUCESS  0x01  //：成功
//#define ROUTINE_RESULT_DOING   0x02  //：正在进行
//#define ROUTINE_RESULT_FAIL    0x03  //：失败
#endif
