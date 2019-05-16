#ifndef IMX6_COM_H
#define IMX6_COM_H

#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include "types.h"
#include "queue.h"
typedef unsigned int  uint;
typedef unsigned char uchar;



#define MAX_REC_SIZE 					2048
#define MAX_SEND_SIZE 					2048u
#define MSG_HEAD             			0xFFu
#define MSG_DATA_OFFSET      			7u
#define MSG_LEN_OFFSET       			6u
#define MSG_CMD_OFFSET       			1u
#define MSG_HID_OFFSET       			2u
#define MSG_HEAD_SIZE        			7u
#define MSG_FRAME_OTHER_SIZE            8u


#define	CM_GENERAL_ACK		   			0x00u
#define	CM_ARM_READY			    	0x81u
#define	CM_GET_MODE_ACK					0X82u
#define	CM_ARM_READY_ACK		    	0x01u
#define CM_PRESENT_TIME             	0x20u
#define	CM_ACC_OFF				    	0x02u	//关ACC
#define	CM_SEND_KEY			    		0x03u
#define	CM_SEND_VEHICLE_INFO      		0x04u
#define	CM_SET_MODE			     		0x05u
#define	CM_GET_MODE			     		0x06u
#define	CM_GET_DISP_TYPE			    0x07u
#define	CM_SET_WAKE			    		0x08u
#define CM_GET_DISP_TYPE_ACK            0x83u
#define	CM_SET_CVBS_DISP		     	0x84u
#define CM_ARM_TO_MCU_UPGRADE         	0x85u
#define	CM_MCU_TO_ARM_UPGRADE	    	0x09u

#define CM_GET_TURN_STATUS          	0x1Fu
#define CM_GET_TURN_STATUS_ACK      	0x97u
#define CM_SET_TURN_ENABLE    			0x1Eu

#define	CM_MCU_APP_VERSION				0x86u
#define	CM_MCU_APP_VERSION_ACK			0x0Au

#define	CM_MCU_BOOT_VERSION				0x87u	
#define	CM_MCU_BOOT_VERSION_ACK		    0x0Bu	


#define	CM_RESET_SYSTEM				    0x88u


#define	CM_ARM_APP_VERSION				0x0Cu
#define	CM_ARM_APP_VERSION_ACK			0x89u	

#define	CM_ARM_ARI_VERSION				0x0Du	
#define	CM_ARM_ARI_VERSION_ACK			0x8Au	
#define	CM_HEARTBEAT				    0x8Bu
#define	CM_SET_DISP_TYPE				0x0Eu
#define CM_SET_DISP_TYPE_ACK            0x9Au
#define	CM_SEND_DISP_TYPE				0x98u

#define CM_VIDEO_CALIBRATION            0x0Fu
#define CM_VIDEO_CALIBRATION_ACK        0x8Cu

#define CM_GET_AVM_INFO                 0x10u
#define	CM_GET_AVM_INFO_ACK				0x8Eu

#define	CM_SET_CAR_COLOUR				0x11u
#define	CM_SET_AVM_ALL_SWITCH			0x12u
#define	CM_SET_CAR_SUBLINE				0x13u
#define	CM_SET_ADAS_FUNCTION_SWITCH		0x14u
#define	CM_SET_ADAS_FUNCTION_SPEED		0x15u
#define	CM_UNVTRANS_TO_ARM				0x16u
#define CM_TEST_SD_CARD					0x18u
#define	CM_TEST_SD_CARD_ACK				0x91u
#define CM_UPDATE_ARM_READY             0x19u
#define	CM_SEND_CAMERA_FAULT			0x99u
#define CM_UPDATE_ARM_DATATRANS         0x1Bu
#define CM_CHECK_USB					0x21u
#define	CM_CHECK_USB_ACK				0x9Bu
#define	CM_CYCLE_SEND_HAIMA_AVM_INFO	0x9Cu  /*海马发送帧*/




#define UPGRADE_APP                     0x01
#define UPGRADE_OTHER                   0x02

#define SUCCEED_ACK          			0x01u
#define FAILED_ACK           			0x00u


#define REC_DATA_ONE_BY_ONE		        1u

#define PROJECT_SEL 					0u			// 0是I.MX6的SD和串口开关，1是T3的SD和串口开关

#define OFFSET_3D_MODE_POS             0
#define OFFSET_LDW_STAT_POS            1
#define OFFSET_SUBLINE_POS             2



#if (PROJECT_SEL == 1)
// T3串口和SD设备路径
#define SD_DEV_PATH				"/dev/mmcblk1p1"
#define COM_DEV_PATH			"/dev/ttyS5"
#else
// I.MX6串口和SD设备路径
#define SD_DEV_PATH				"/dev/mmcblk0p1" 		//"/run/media/mmcblk0p1"
#define USB_DEV_PATH			"/dev/sda" 
#define COM_DEV_PATH			"/dev/ttymxc1"	
#endif

#define MCU_SD_PATH             "/run/media/sda1/UPDATE/Extend_360_Pro_Boot.bin"
#define TSH_VER_PATH            "/home/tsh/1_tianshuang/version.txt"
#define T7A_APP_INI_PATH      	"/home/chinagps/config.ini"
#define UPDATE_TIME_PATH      	"/home/chinagps/update_time.txt"
#define VALID_FLAG_PATH         "/home/chinagps/app_valid_flag.ini"
#if 0
extern static int stopped;
extern static const char* app_ver;
#endif
extern void tty1_com_thread(void);
extern void ReceiveSerialDataThread(void);
extern int WriteCom(char* const buf,const uint n);
extern unsigned char ReadOneChar(void);
extern int ReadCom(char* buf);
extern void flush_buf(const int type);
extern void SendGeneralAck(uchar cmd,uint Hid,uchar isSucc);
extern void SendModeAck(uchar cmd,uint Hid,uchar mode);
extern int SendPkgStruct(uchar cmd,uint Hid,char IsAck, uchar* msg,uchar msgLen);
extern void execl_boot(void);
extern void SendHeartBeatPkg(void);
extern Queue  MsgQueue;
void ViewChange3D(void);
void ViewChange2D(void);

void ProcessAutoHandle(void);


extern int acc_off_flag;
//extern AvmParaSetInfo avmParaSetInfo ;
//extern AvmCalibrationPara avmCalibrationPara;

#endif
