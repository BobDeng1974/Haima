#ifndef __DEBUG_H
#define __DEBUG_H

#include  "serial.h"


// 调试开关偏移位
enum{
	ON_OFF_NORM = 0,
	ON_OFF_PARAM, // 1 
	ON_OFF_REMCTRL, // 2
	ON_OFF_MSGQ_DBG,// 3
	ON_OFF_COM_DBG, // 4
	ON_OFF_SMS_DBG, // 5
	ON_OFF_GPRS_DBG, // 6
	ON_OFF_GPS,// 7
	ON_OFF_CAN, // 8
	ON_OFF_MODEM,// 9
	ON_OFF_SLEEP,// 10
	ON_OFF_CMCC, // 11
	ON_OFF_ALARM, // 12
	ON_OFF_BATCH,// 13
	ON_OFF_AVN,// 14
	ON_OFF_LOGIN,// 15
	ON_OFF_UPD,// 16
	ON_OFF_POWER,// 17
	ON_OFF_RS485,// 18
	ON_OFF_TRANS,// 19
	ON_OFF_DIAG, // 20
	ON_OFF_CAN_DATA, // 21
	ON_OFF_ROLLOVER, // 22
	ON_OFF_CTRL,// 23
	ON_OFF_FTP,// 24
	ON_OFF_PTL,// 25
	ON_OFF_OTHER,// 26
    ON_OFF_RING,//27

	ON_OFF_ERR, // 28   - 打印错误情况
	ON_ALL_DEBUG,// 29
	OFF_ALL_DEBUG // 30
};
#define	NORM_DBG	(1<<ON_OFF_NORM)
#define PARAM_DBG	(1<<ON_OFF_PARAM)
#define MSGQ_DBG    (1<<ON_OFF_MSGQ_DBG)		
#define COM_DBG		(1<<ON_OFF_COM_DBG)
#define SMS_DBG		(1<<ON_OFF_SMS_DBG)
#define GPRS_DBG	(1<<ON_OFF_GPRS_DBG)
#define GPS_DBG     (1<<ON_OFF_GPS)
#define CAN_DBG	    (1<<ON_OFF_CAN)
#define MODEM_DBG	(1<<ON_OFF_MODEM)
#define SLEEP_DBG	(1<<ON_OFF_SLEEP)
#define CMCC_DBG	(1<<ON_OFF_CMCC)
#define ALARM_DBG	(1<<ON_OFF_ALARM)
#define BATCH_DBG   (1<<ON_OFF_BATCH)
#define AVN_DBG		(1<<ON_OFF_AVN)
#define LOGIN_DBG	(1<<ON_OFF_LOGIN)
#define UPD_DBG		(1<<ON_OFF_UPD)
#define POWER_DBG   (1<<ON_OFF_POWER)//功率检测打印开关
#define SAVE_DBG       (1<<ON_OFF_RS485)//rs485串口设置
#define TRANS_DBG       (1<<ON_OFF_TRANS)
#define DIAG_DBG        (1<<ON_OFF_DIAG)
#define CAN_DATA_DBG    (1<<ON_OFF_CAN_DATA)
#define	ROLLOVER_DBG	(1<<ON_OFF_ROLLOVER)
#define	CTRL_DBG		(1<<ON_OFF_CTRL)
#define FTP_DBG         (1<<ON_OFF_FTP)  // ftp upgrade
#define PTL_DBG         (1<<ON_OFF_PTL)  // protocol 
#define RING_DBG        (1<<ON_OFF_RING) //CAN????
#define  ERR_DBG        (1<<ON_OFF_ERR)
#define OTHER_DBG	    (1<<ON_OFF_OTHER)



extern unsigned int gDebugFlag ; //调试打印开关
//extern void InfoPrintf (char *fmt, ...);


#define DebugPrintf(mode,fmt,...) do{\
	if(mode & gDebugFlag) \
//		InfoPrintf(fmt,##__VA_ARGS__ );\
		}while(0)

#define DebugPut(mode,buff,len) do{\
	if(mode & gDebugFlag) \
		UART_Write(UART0_DEBUG,buff,len);\
		}while(0)	
extern void debug_printf (unsigned long mode,unsigned char* p_data, unsigned short len);
#endif
