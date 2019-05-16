#ifndef __DIAG_UPGRADE_H__
#define __DIAG_UPGRADE_H__

#include "miscdef.h"

#ifndef UINT8
typedef unsigned char UINT8;
#endif
#ifndef UINT16
typedef unsigned short UINT16;
#endif
#ifndef UINT32
typedef unsigned int UINT32;
#endif


#define OPT_EREASE_MEMORY_OK    (0X00)
#define OPT_REQ_DOWNLOAD_OK     (0X01)
#define OPT_EXIT_DOWNLOAD_OK    (0X04)
#define OPT_REQ_CHECK1_OK       (0X05)
#define OPT_REQ_CHECK2_OK       (0X06)
#define OPT_REQ_RESETMCU_OK     (0X07)


extern u8  Upflag;

typedef struct{
	int offset;
	u8 buff[256];
}stUpdateInfo_t;
extern stUpdateInfo_t update;

typedef struct
{
	u32  fileTotal;   // 文件大小
	u32  fileCheck;   // 文件校验值
	u32  fileRecvCnt; // 接收文件长度
	u32  bagSeq;      // 包序号	
	u8   optStep;      // 升级操作流程	
	u8   update_type;
}UPGRADE_T;
extern UPGRADE_T stUpgradeParam;




extern int can_update_send(unsigned char *buf,int len);
extern u8  upgrade_earseMemory(device_info_t *dev);
extern void upgrade_requestFileDownload(device_info_t *dev,u8 *buf, u16 len);
extern void upgrade_requestFileDownload_ARM(device_info_t *dev,u8 *buf, u16 len);
extern void  upgrade_transferData(device_info_t *dev,u8 *buf, u16 len);
extern void  upgrade_requestTransferExit(device_info_t *dev,u8 *buf, u16 len);
extern void upgrade_loadFile_check1(device_info_t *dev,u8 *buf, u16 len);
extern UINT8  upgrade_loadFile_check2(u8 *buf, u16 len);
extern void upgrade_mcu_boot_init(device_info_t *dev);


#endif  /* __DIAG_UPGRADE_H__ */
