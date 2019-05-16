
#ifndef __PROTOCAL_API_H__
#define __PROTOCAL_API_H__

#include "protocolapi.h"
// uart message related
//#define USART_MSG_HEAD			0xff

/*================系统消息======================*/

///启动消息
//for ARM:none
//for MCU:{}
//ARM通知MCU启动完毕
#define CM_ARM_READY				0x81ul
/*================时间同步======================*/

///MCU 向ARM 上报时间,如果para[0] ~para[5] 全为0表示时间值是无效的
//for MCU:{para[0] = time>>7,para[1] = time>>6，para[2] =..，}
//for ARM:{}
//MCU 向ARM 上报时间读取时间
#define CM_GET_TIME				0x01ul//握手命令回应

#define CM_ACC_OFF				0x02ul//握手命令回应

#define	CM_GENERAL_ACK			0x00


#define	CM_SEND_KEY				0x03  // 按键状态消息

// 车辆消息
// for ARM: {para, 7字节= ( para[0]: 车辆信息}
// 
// for MCU: none
// MAU通知ARM当前车辆信息

#define	CM_SEND_VEHICLE_INFO				0x04	

/* FROM HMI ID:0x31  event  */
#define CM_SET_MODE				0x05  // 1:2D 2:3D显示切换
#define	CM_SET_ViewType			0x0E  //设置显示视图

//#define CM_SET_AVM_SW     		0x03  // AVM开关按键信号
#define	CM_GET_TURN_STAT	   	0x90
#define  CM_SET_CAR_COLOUR      0x11 //SET_CAR_COLOUR


//获取显示方式
// for ARM: {para, 1字节= ( para[0]: 显示方式}
// 1:2D 2:3D
// for MCU: none
// MAU通知ARM当前车辆信息

#define	CM_GET_MODE				0x06

//查询当前视图类型（保留）
#define	CM_GET_DISP_TYPE				0x07

//休眠与唤醒
#define	CM_SET_WAKE				0x08


//mcu请求升级
#define CM_ARM_UPGRADE				0x09

//mcu回复APP 版本
#define CM_ARM_APP_VER				0x0A

//mcu回复BOOT 版本
#define CM_ARM_BOOT_VER				0x0B
//get arm app ver 
#define	CM_ARM_APP_VERSION			0x0C
//获取算法版本
#define	CM_ARM_ARI_VERSION			0x0D	

//设置显示视图
#define	CM_SET_DISP_TYPE				0x0E

//ARM回应MCU当前显示视图
#define	CM_SET_DISP_TYPE_ACK	0x9A       
//一键标定
#define CM_VIDEO_CALIBRATION   0x0F

//设置车模颜色
#define  CM_SET_CAR_COLOUR   0x11

//设置AVM总开关
#define	CM_SET_AVM_ALL_SWITCH				0x12

//设置车辅线开关状态
#define	CM_SET_CAR_SUBLINE				0x13

//设置辅助驾驶功能开关
#define	CM_SET_ADAS_FUNCTION_SWITCH			0x14

//设置辅助驾驶功能开启速度
#define	CM_SET_ADAS_FUNCTION_SPEED				0x15

//获取AVM设置参数
#define	CM_GET_AVM_INFO				0x10

//获取AVM设置参数回应
#define	CM_GET_AVM_INFO_ACK				0x8E

//启动转向开关
//#define CM_SET_TURN_ENABLE					0x17

//启动转向开关回应
#define	CM_GET_TURN_STAT				0x90

//设置显示视图回应
//#define	CM_SET_DISP_TYPE_ACK				0x8C

//通用应答
#define	CM_GENERAL_ACK				0x00

//SD卡测试
#define	CM_TEST_SD_CARD				0x18

//SD卡测试应答
#define CM_TEST_SD_CARD_ACK			   0x91


//此命令通过MCU转发给显示设备进行切换
#define	CM_UNVTRANS_TO_ARM				0x16
#define	CM_UNVTRANS_TO_MCU				0x110
#define	CM_GET_MODE_ACK				0x82
/*================升级消息======================*/


#define	CM_SET_AVM_DISP					0x84//arm切换AVM 发给MCU

#define CM_AVM_CalibrationStatus       0x40 
#define CM_AVM_CamCaliFault       		0x41
#define CM_VIDEO_CALIBRATION            0x0F
#define CM_VIDEO_CALIBRATION_ACK          0x8C
#define  CM_SEND_CAMERA_FAULT            0x99


///升级消息
//for ARM:{para[0] = (0:升级boot, 1:升级App),para[1] = (0:arm 发起的升级，1:MCU发起的升级)}
//for MCU:{para[0] = (0:升级boot, 1:升级App),para[1] = (0:arm 发起的升级，1:MCU发起的升级)}
//通知对方MCU升级模式
#define CM_MCU_UPGRADE				0x85
#define	CM_MCU_TO_ARM_UPGRADE	    		0x09

///ARM读取MCU BOOT版本信息（ 01 00 03 00，表示版本号：3.0.1）
//for ARM:{para[0] = ..,para[1] = ..，para[2] =..，para[3] =00}
//for MCU:{}
//获取MCU Boot 版本号
#define CM_MCU_BOOT_VERSION			0x87

///ARM读取MCU APP版本信息（ 01 00 03 00，表示版本号：3.0.1）
//for ARM:{para[0] = ..,para[1] = ..，para[2] =..，para[3] =00}
//for MCU:{}
//获取MCU App 版本号
#define CM_MCU_APP_VERSION			0x86

//arm 请求复位
#define CM_RESET_SYSTEM            0x88
//获取ARM软件APP版本信息
#define	CM_ARM_APP_VERSION_ACK							0x89	
// 获取ARM软件算法版本信息
#define	CM_ARM_ARI_VERSION_ACK					0x8A	
#define	CM_HEARTBEAT				0x8B
/*================硬件模块测试消息======================*/
///测试项消息
///测试项数据格式: 共16字节，其中第0字节表示项目编号[0x00 ~ 0x10]
//for MCU: 读测试项: para[0] = 项目编号, 写测试项: para[0:15]
//for ARM: 上报测试项: para[0:15]
#define ITEM_TEST_ID_MAX			0x10ul
#define ITEM_TEST_ITEM_SIZE		0x10ul
#define CM_ITEM_TEST				0x98ul

#define CM_HOST_ACK_MSG			0x90ul


/************UDS   诊断升级  **************/
#define    CM_UPDATE_ARM_READY 					   0x19ul
#define    CM_UPDATE_ARM_READY_ACK 				   0x92ul

#define    CM_UPDATE_ARM_START 					    0x1Aul
#define    CM_UPDATE_ARM_START_ACK 					   0x93ul

#define    CM_UPDATE_ARM_DATATRANS 					   0x1Bul
#define    CM_UPDATE_ARM_DATATRANS_ACK 			   0x94ul

#define    CM_UPDATE_ARM_DATATRANS_END 			   0x1Cul
#define    CM_UPDATE_ARM_DATATRANS_END_ACK 			   0x95ul

#define    CM_UPDATE_ARM_FINISH							 0x96ul
#define    CM_UPDATE_ARM_ABORT 							 0x1Dul
/************End UDS 诊断升级  **************/
#define	CM_SET_TURN_ENABLE				                   0x1E
#define	CM_GET_TURN_STATUS       				          0x1F
#define	CM_GET_TURN_STATUS_ACK      				            0x97
#define CM_PRESENT_TIME										   0x20
/**cycle from ARM显示视图**/
#define	CM_SEND_DISP_TYPE				                    0x98

#define	CM_GET_DISP_TYPE_ACK			  	0x83

#define	CM_CYCLE_SEND_HAIMA_AVM_INFO			                  0x9C

//用于转发USB CHECK
#define    CM_CHECK_USB 								  0x21
#define    CM_CHECK_USB_ACK 								 0x9B


/*================PC自动化测试消息======================*/

//退出自动化测试
#define	CM_PC_ARM_STOP				0x21	

//通用应答，确认MUC or PC是否收到指令
#define	CM_PC_GENERAL_ACK				0x00

//PC检测MUC是否在线
#define	CM_PC_ARM_READY				0x01

//PC向MUC发送车辆信息
#define	CM_PC_SEND_VEHICLE_INFO				0x02

//PC向MCU请求打开/关闭AVM视频输出
#define	CM_PC_SET_AVM_ENABLE				0x03

//PC请求获取MUC软件APP版本信息
#define	CM_PC_MCU_APP_VERSION								0x04

//MCU向PC发送MCU_APP版本信息
#define	CM_PC_MCU_APP_VERSION_ACK							0x05

//PC请求获取MUC软件boot版本信息
#define	CM_PC_MCU_BOOT_VERSION						0x06	

//MCU向PC发送MCU_boot版本信息
#define	CM_PC_MCU_BOOT_VERSION_ACK					0x07

//PC请求重启系统
#define	CM_PC_RESET_SYSTEM				0x08

//PC请求获取ARM_APP版本信息
#define	CM_PC_ARM_APP_VERSION								0x09

//MCU向PC发送ARM_APP版本信息
#define	CM_PC_ARM_APP_VERSION_ACK							0x0A

//PC请求获取ARM算法版本信息
#define	CM_PC_ARM_ARI_VERSION						0x0B	

//MCU向PC发送ARM算法版本信息
#define	CM_PC_ARM_ARI_VERSION_ACK					0x0C	

//PC请求设置显示视图
#define	CM_PC_SET_DISP_TYPE				0x0D

//MCU向PC发送设置显示视图结果
#define	CM_PC_SET_DISP_TYPE_ACK				0x0E

//PC请求查询当前视图显示类型
#define	CM_PC_GET_DISP_TYPE				0x0F

//MCU向PC发送当前视图显示类型
#define	CM_PC_GET_DISP_TYPE_ACK				0x10

//PC请求一键标定
#define	CM_PC_VIDEO_CALIBRATION				0x11

//PC请求设置车模颜色
#define	CM_PC_SET_CAR_COLOUR				0x12

//PC请求设置AVM开关
#define	CM_PC_SET_AVM_ALL_SWITCH				0x13

//PC请求设置车辅线开关状态
#define	CM_PC_SET_CAR_SUBLINE				0x14

//PC请求设置辅助驾驶功能开关
#define	CM_PC_SET_ADAS_FUNCTION_SWITCH			0x15

//PC请求设置辅助驾驶功能开启速度
#define	CM_PC_SET_ADAS_FUNCTION_SPEED				0x16

//PC请求获取AVM设置参数
#define	CM_PC_GET_AVM_INFO				0x17

//MCU向PC发送AVM设置参数
#define	CM_PC_GET_AVM_INFO_ACK				0x18

//PC请求启动转向开关（潍柴）
#define	CM_PC_SET_TURN_ENABLE					0x19

//MCU向PC发送转向开关启动结果
#define	CM_PC_GET_TURN_STAT				0x1A

//上位机写入主机条码
#define	CM_PC_SET_SerialNumber				0x1B 

//上位机读出主机条码
#define	CM_PC_GET_SerialNumber				0x1C   

//MCU回复上位机主机条码
#define	CM_PC_GET_SerialNumber_Ack				0x1D    

//上位机写入设备ID
#define	CM_PC_SET_DeviceID				    0x1E    

//上位机读取设备ID
#define	CM_PC_GET_DeviceID				    0x1F    

//MCU应答读取设备ID
#define	CM_PC_GET_DeviceID_Ack				    0x20   

//2D/3D显示切换
#define CM_PC_SET_MODE                        0x22

//SD卡测试
#define    CM_PC_TEST_SD_CARD			   0x23

//SD卡测试应答
#define	CM_PC_TEST_SD_CARD_ACK				0x24

//MCU立即休眠指令
#define    CM_PC_MCU_SLEEP			   0x25

//电压检测指令
#define    CM_PC_MCU_VOLTAGE           0X26

//电压检测指令应答
#define    CM_PC_MCU_VOLTAGE_ACK           0X27

//检测USB
#define	CM_PC_USB				                           0x28u
#define	CM_PC_USB_ACK				                  0x29u

//#define	CM_CRT_YMODEM_YPDATE				            0x88u

/**********End auto test define*************/

#endif /* end of __PROTOCAL_API_H__ */

