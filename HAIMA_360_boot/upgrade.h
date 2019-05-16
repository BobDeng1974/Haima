#ifndef UPGRADE_H
#define UPGRADE_H

#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include "types.h"
#include "queue.h"

#define MAX_REC_SIZE 2060
#define MAX_SEND_SIZE 1024
#define MSG_HEAD             0xFFu
#define MSG_DATA_OFFSET      7
#define MSG_LEN_OFFSET       6
#define MSG_CMD_OFFSET       1
#define MSG_HID_OFFSET       2
#define MSG_HEAD_SIZE        7
#define MSG_FRAME_OTHER_SIZE        8

#define COMMUNICATION_FRAMES  0x01  
#define DATA_FRAMES           0x02 

#define START_UPDATE_CMD      0x01
#define SEND_PACKAGE_NUM_CMD      0x02
#define SEND_PACKAGE_DONE_CMD         0x03

#define  ARM_READY_CMD            0x01
#define SEND_PACKAGE_NUM_ACK_CMD      0x02
#define SEND_PACKAGE_DONE_ACK_CMD         0x03
#define ARM_TIMEOUT_CMD                 0x04          

#define	CM_UNVTRANS_TO_ARM				0x16
#define	CM_UNVTRANS_TO_MCU				0x8F
#define	CM_GENERAL_ACK		    0x00
#define	CM_ARM_READY			    0x81
#define	CM_ARM_READY_ACK		    0x01
#define	CM_ACC_OFF				    0x02	//关ACC
#define	CM_SEND_KEY			    0x03
#define	CM_SEND_VEHICLE_INFO      0x04
#define	CM_SET_MODE			     0x05
#define	CM_SET_WAKE			     0x08
#define	CM_SET_CVBS_DISP		     0x84
#define   CM_ARM_TO_MCU_UPGRADE                0x85
#define	CM_MCU_TO_ARM_UPGRADE	    		0x09

#define	CM_MCU_APP_VERSION				0x86
#define	CM_MCU_APP_VERSION_ACK			0x0A

#define	CM_MCU_BOOT_VERSION				0x87	
#define	CM_MCU_BOOT_VERSION_ACK		0x0B	


#define	CM_RESET_SYSTEM				       0x88


#define	CM_ARM_APP_VERSION				0x0C
#define	CM_ARM_APP_VERSION_ACK			0x89	

#define	CM_ARM_ARI_VERSION				0x0D	
#define	CM_ARM_ARI_VERSION_ACK			0x8A	
#define	CM_HEARTBEAT				    0x8B
#define	CM_SET_DISP_TYPE				       0x0E
#define	CM_SET_DISP_TYPE_ACK				0x8C

#define CM_VIDEO_CALIBRATION        0x0F

#define CM_GET_AVM_INFO             0x10
#define	CM_GET_AVM_INFO_ACK				0x8E

#define CM_UPDATE_ARM_READY                0x19
#define CM_UPDATE_ARM_READY_ACK            0x92
#define CM_UPDATE_ARM_START                0x1A
#define CM_UPDATE_ARM_START_ACK            0x93
#define CM_UPDATE_ARM_DATATRANS            0x1B
#define CM_UPDATE_ARM_DATATRANS_ACK        0x94
#define CM_UPDATE_ARM_DATATRANS_END        0x1C
#define CM_UPDATE_ARM_DATATRANS_END_ACK    0x95
#define CM_UPDATE_ARM_FINISH               0x96
#define CM_UPDATE_ARM_ABORT                0x1D


#define SUCCEED_ACK          0x01u
#define FAILED_ACK           0x00u
#define VALID_FLAG_PATH              "/home/chinagps/app_valid_flag.ini"
#define UPDATE_APP_PATH              "/home/chinagps/update/t6a_app"
#define UPDATE_KERNEL_PATH           "/home/chinagps/update/zImage"
#define UPDATE_DTB_PATH              "/home/chinagps/update/imx6q-sabresd.dtb"
#define UPDATE_SUANFA_PATH           "/home/chinagps/update/suanfa.tar.bz2"
#define UPDATE_UBOOT_PATH            "/home/chinagps/update/u-boot.imx"

#define UPDATE_APP_PATCH             "/home/chinagps/update/t6a_app.patch"
#define UPDATE_KERNEL_PATCH          "/home/chinagps/update/zImage.patch"
#define UPDATE_DTB_PATCH             "/home/chinagps/update/imx6q-sabresd.dtb.patch"
#define UPDATE_SUANFA_PATCH          "/home/chinagps/update/suanfa.tar.bz2.patch" 
#define UPDATE_UBOOT_PATCH           "/home/chinagps/update/u-boot.imx.patch"



typedef enum cCodeType{
	TImx6App=0,
    TArithmetic,
    Tkernel,
    TDtb,
    TUboot,
}CodeType;

typedef  enum {
    T_START,
    T_CMD,
    T_HID,
    T_LEN,
    T_DATA,
    T_LRC,
}RecType;


typedef enum cInterval{
	T100=0,
    T1000,
    T6000,
    T10000,
}AckInterval;

typedef struct cUpdateInfo{
   uint8_t init;      
   uint8_t diff_update;       //1 差分升级    0 整包升级
   CodeType code_type;      
   uint32_t packageNum;       //包序号
   uint32_t  filesize;          //文件总长度
   uint32_t packageCount;         //包总个数
   uint32_t packageSize;        //包大小
   uint32_t crc32;
   uint32_t alreadyRecPkg;
   WriteFileInfo writeFileInfo;
   Queue queue;
}UpdateInfo;


uint32_t fwriten(const int32_t fd, void *const vptr, const uint32_t n);
void WriteDataFull(void);
void WriteDataByInterval(void);
uint32_t calc_crc(const int32_t fd, uint32_t count);
int32_t WriteAppFlag(const uint8_t u);
int32_t ProcessSendDone(const uint32_t Hid, const uint8_t* const pMsgData);
int32_t setCom(const char *const Dev);
uint32_t WriteCom(char* const buf,const uint32_t n);
uint32_t ReadCom(char* buf);
int32_t ReadOneChar(char* const c);
uint32_t ReadOneFrame(char* const buf);
void flush_buf(const int32_t type);
uint8_t* arrchr(uint8_t* pBuf,const uint8_t ch,uint32_t size);
void ParseGeneralAckCmd(const uint8_t* const msg);
char* memstr(char* const full_data, const int32_t full_data_len, const char* const substr);
void SendPkgStruct(const uint8_t cmd, const uint32_t Hid, const signed char IsAck, const uint8_t* const msg,const uint8_t msgLen);
void SendGeneralAck(const uint8_t cmd,const uint32_t Hid,const uint8_t isSucc);
void ParseProtocol(uint8_t* const msgData, const uint8_t cmd);
void ProcessComHandle(char* const buf ,const uint32_t size);
int32_t Filed_Filename(void);
int32_t UpdateARM_Start(const uint32_t Hid, uint8_t* const msg);
int32_t UpdateARM_DataTrans(const uint32_t Hid, uint8_t* const pMsgData);
int32_t UpdateARM_DataTrans_End(const uint32_t Hid,const uint8_t* const pMsgData);
int32_t UpdateARM_Abort(void);
void tty1_com_thread(int* arg);

#endif
