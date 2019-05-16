/******************************************************************************

                  版权所有 (C), 2017-2018, 赛格导航科技股份有限公司

 ******************************************************************************
  文 件 名   : iflash.h
  版 本 号   : 初稿
  作    者   : xz
  生成日期   : 2018年8月2日
  最近修改   :
  功能描述   : DID，DTC 读写flash
  函数列表   :
*
*

  修改历史   :
  1.日    期   : 2018年8月2日
    作    者   : xz
    修改内容   : 创建文件

******************************************************************************/
#ifndef	__IFLASH_H__
#define  __IFLASH_H__
#include "stm32f30x.h"
#include "hw_config.h"
#include "miscdef.h"
#include "can_did.h"

//auto tes
#define DEVICE_ID_LEN        	 					8ul
#define SERIALNUMBER_LEN        	 			19ul
#define DEVICE_ID_ADDR        	 			(ADDR_Supplier_ID+7+1)
#define SERIALNUMBER_ADDR        			(DEVICE_ID_ADDR + 8)
/****DTC起始地址*******/		
#define ADD_DTC_START     			     (SERIALNUMBER_ADDR+19+1)//偶数地址



 u8 Update_WriteAndCheck(device_info_t *dev,u16 *buf,u32 bagnum ,u32 len);
 void read_dtc_from_flash(u32 addr,u8 *buf,u16 len);
 u8 STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);
 void iflash_unlock(void);
void iflash_lock(void);
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);
u32 iflash_word_program(u32 addr, u32 dat);
 u32 iflash_halfword_program(u32 addr, u16 dat);
 void iflash_lock(void);
 void iflash_unlock(void);
void iflash_flag_clear(void);
void iflash_operation_result(device_info_t *dev, u32 ret);
#endif/*__IFLASH_H__*/


