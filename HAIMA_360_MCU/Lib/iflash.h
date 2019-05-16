/******************************************************************************

                  ��Ȩ���� (C), 2017-2018, ���񵼺��Ƽ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : iflash.h
  �� �� ��   : ����
  ��    ��   : xz
  ��������   : 2018��8��2��
  ����޸�   :
  ��������   : DID��DTC ��дflash
  �����б�   :
*
*

  �޸���ʷ   :
  1.��    ��   : 2018��8��2��
    ��    ��   : xz
    �޸�����   : �����ļ�

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
/****DTC��ʼ��ַ*******/		
#define ADD_DTC_START     			     (SERIALNUMBER_ADDR+19+1)//ż����ַ



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


