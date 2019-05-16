
#include <stdio.h>
#include <string.h>
#include <iflash.h>
#include <config.h>
#include <miscdef.h>
#include <usart.h>
#include <api.h>
#include <stm32f30x.h>

/* Flash Access Control Register bits */
#define ACR_LATENCY_Mask		0x00000038u
#define ACR_HLFCYA_Mask			0xFFFFFFF7u
#define ACR_PRFTBE_Mask			0xFFFFFFEFu

/* Flash Access Control Register bits */
#define ACR_PRFTBS_Mask			0x00000020u

/* Flash Control Register bits */
#define CR_PG_Set					((uint32_t)0x00000001)  
#define CR_PG_Reset					((uint32_t)0x00000001) 
#define CR_PER_Set					((uint32_t)0x00000002)
#define CR_PER_Reset				((uint32_t)0x00000002)
#define CR_MER_Set					0x00000004u
#define CR_MER_Reset				0x00001FFBu
#define CR_OPTPG_Set				0x00000010u
#define CR_OPTPG_Reset				0x00001FEFu
#define CR_OPTER_Set				0x00000020u
#define CR_OPTER_Reset				0x00001FDFu
#define CR_STRT_Set					((uint32_t)0x00000040)
#define CR_LOCK_Set				  ((uint32_t)0x00000080)

/* FLASH Mask */
#define RDPRT_Mask					0x00000002u
#define WRP0_Mask					0x000000FFu
#define WRP1_Mask					0x0000FF00u
#define WRP2_Mask					0x00FF0000u
#define WRP3_Mask					0xFF000000u
#define OB_USER_BFB2				0x0008

/* FLASH Keys 
#define RDP_Key						0x00A5
#define FLASH_KEY1					0x45670123u
#define FLASH_KEY2					0xCDEF89ABu*/

/* FLASH BANK address */
#define FLASH_BANK1_END_ADDRESS	0x807FFFFu

/* Delay definition */   
#define EraseTimeout					((uint32_t)0x000B0000)
#define ProgramTimeout				((uint32_t)0x000B0000)


#define FLASH_FLAG_BSY					((uint32_t)0x00000001)  
#define FLASH_FLAG_EOP					0x00000020u
#define FLASH_FLAG_PGERR				((uint32_t)0x00000004)    
#define FLASH_FLAG_WRPRTERR			((uint32_t)0x00000010)
#define FLASH_FLAG_OPTERR				 0x00000001u

#define FLASH_FLAG_BANK1_BSY			FLASH_FLAG_BSY
#define FLASH_FLAG_BANK1_EOP			FLASH_FLAG_EOP
#define FLASH_FLAG_BANK1_PGERR		FLASH_FLAG_PGERR
#define FLASH_FLAG_BANK1_WRPRTERR	FLASH_FLAG_WRPRTERR



/**
  * @brief  Returns the FLASH Bank1 Status.
  * @note   This function can be used for all STM32F10x devices, it is equivalent
  *         to FLASH_GetStatus function.
  * @param  None
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP or FLASH_COMPLETE
  */
static u32 FLASH_GetBank1Status(void)
{
	u32 flashstatus = FLASH_COMPLETE;
  
	if((FLASH->SR & FLASH_FLAG_BANK1_BSY) == FLASH_FLAG_BSY) 
	{
		flashstatus = FLASH_BUSY;
	}
	else 
	{  
		if((FLASH->SR & FLASH_FLAG_BANK1_PGERR) != (u32)0ul)
		{ 
			flashstatus = FLASH_ERROR_WRP;
		}
		else 
		{
			if((FLASH->SR & FLASH_FLAG_BANK1_WRPRTERR) != (u32)0ul )
			{
				flashstatus = FLASH_ERROR_PG;
			}
			else
			{
				flashstatus = FLASH_COMPLETE;
			}
		}
	}

	return flashstatus;
}

#ifdef STM32F10X_XL
/**
  * @brief  Returns the FLASH Bank2 Status.
  * @note   This function can be used for STM32F10x_XL density devices.
  * @param  None
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PG,
  *        FLASH_ERROR_WRP or FLASH_COMPLETE
  */
static u32 FLASH_GetBank2Status(void)
{
	u32 flashstatus = FLASH_COMPLETE;
  
	if((FLASH->SR2 & (FLASH_FLAG_BANK2_BSY & 0x7FFFFFFF)) == (FLASH_FLAG_BANK2_BSY & 0x7FFFFFFF)) 
	{
		flashstatus = FLASH_BUSY;
	}
	else 
	{  
		if((FLASH->SR2 & (FLASH_FLAG_BANK2_PGERR & 0x7FFFFFFF)) != 0)
		{ 
			flashstatus = FLASH_ERROR_PG;
		}
		else 
		{
			if((FLASH->SR2 & (FLASH_FLAG_BANK2_WRPRTERR & 0x7FFFFFFF)) != 0 )
			{
				flashstatus = FLASH_ERROR_WRP;
			}
			else
			{
				flashstatus = FLASH_COMPLETE;
			}
		}
	}

	return flashstatus;
}
#endif /* STM32F10X_XL */

#ifdef STM32F10X_XL
/**
  * @brief  Waits for a Flash operation on Bank2 to complete or a TIMEOUT to occur.
  * @note   This function can be used only for STM32F10x_XL density devices.
  * @param  Timeout: FLASH programming Timeout
  * @retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
static u32 FLASH_WaitForLastBank2Operation(u32 Timeout)
{ 
	u32 status = FLASH_COMPLETE;
   
	/* Check for the Flash Status */
	status = FLASH_GetBank2Status();
	/* Wait for a Flash operation to complete or a TIMEOUT to occur */
	while((status == (FLASH_FLAG_BANK2_BSY & 0x7FFFFFFF)) && (Timeout != 0x00))
	{
		status = FLASH_GetBank2Status();
		Timeout--;
	}
	if(Timeout == 0x00 )
	{
		status = FLASH_TIMEOUT;
	}
	/* Return the operation status */
	return status;
}
#endif /* STM32F10X_XL */

/**
  * @brief  Waits for a Flash operation to complete or a TIMEOUT to occur.
  * @note   This function can be used for all STM32F10x devices, 
  *         it is equivalent to FLASH_WaitForLastBank1Operation.
  *         - For STM32F10X_XL devices this function waits for a Bank1 Flash operation
  *           to complete or a TIMEOUT to occur.
  *         - For all other devices it waits for a Flash operation to complete 
  *           or a TIMEOUT to occur.
  * @param  Timeout: FLASH programming Timeout
  * @retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
static u32 FLASH_WaitForLastOperation(u32 Timeout)
{ 
	u32 status = FLASH_COMPLETE;
   
	/* Check for the Flash Status */
	status = FLASH_GetBank1Status();
	/* Wait for a Flash operation to complete or a TIMEOUT to occur */
	while((status == FLASH_BUSY) && (Timeout != 0x00))
	{
		status = FLASH_GetBank1Status();
		Timeout--;
	}
	
	if(Timeout == 0x00 )
	{
		status = FLASH_TIMEOUT;
	}

	return status;
}

#ifdef STM32F10X_XL
/**
  * @brief  Waits for a Flash operation on Bank1 to complete or a TIMEOUT to occur.
  * @note   This function can be used for all STM32F10x devices, 
  *         it is equivalent to FLASH_WaitForLastOperation.
  * @param  Timeout: FLASH programming Timeout
  * @retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
static u32 FLASH_WaitForLastBank1Operation(u32 Timeout)
{ 
	u32 status = FLASH_COMPLETE;
   
	/* Check for the Flash Status */
	status = FLASH_GetBank1Status();
	/* Wait for a Flash operation to complete or a TIMEOUT to occur */
	while((status == FLASH_FLAG_BANK1_BSY) && (Timeout != 0x00))
	{
		status = FLASH_GetBank1Status();
		Timeout--;
	}
	
	if(Timeout == 0x00 )
	{
		status = FLASH_TIMEOUT;
	}

	return status;
}
#endif

/**
  * @brief  Unlocks the FLASH Program Erase Controller.
  * @note   This function can be used for all STM32F10x devices.
  *         - For STM32F10X_XL devices this function unlocks Bank1 and Bank2.
  *         - For all other devices it unlocks Bank1 and it is equivalent 
  *           to FLASH_UnlockBank1 function.. 
  * @param  None
  * @retval None
  */
void iflash_unlock(void)
{
	/* Authorize the FPEC of Bank1 Access */
  if((FLASH->CR & FLASH_CR_LOCK) != RESET)
  {
    /* Authorize the FLASH Registers access */
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
  } 
}

/**
  * @brief  Locks the FLASH Program Erase Controller.
  * @note   This function can be used for all STM32F10x devices.
  *         - For STM32F10X_XL devices this function Locks Bank1 and Bank2.
  *         - For all other devices it Locks Bank1 and it is equivalent 
  *           to FLASH_LockBank1 function.
  * @param  None
  * @retval None
  */
void iflash_lock(void)
{
	/* Set the Lock Bit to lock the FPEC and the CR of  Bank1 */
	FLASH->CR |= CR_LOCK_Set;
}

/**
  * @brief  Erases a specified FLASH page.
  * @note   This function can be used for all STM32F10x devices.
  * @param  Page_Address: The page address to be erased.
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
u32 iflash_page_erase(u32 page_addr)
{
	u32 status = FLASH_COMPLETE;

#ifdef STM32F30X

	/* Wait for last operation to be completed */
	status = FLASH_WaitForLastOperation(EraseTimeout);
  
	if(status == FLASH_COMPLETE)
	{ 
		/* if the previous operation is completed, proceed to erase the page */
		FLASH->CR|= CR_PER_Set;
		FLASH->AR = page_addr; 
		FLASH->CR|= CR_STRT_Set;
    
		/* Wait for last operation to be completed */
		status = FLASH_WaitForLastOperation(EraseTimeout);
    
		/* Disable the PER Bit */
		FLASH->CR &= ~CR_PER_Reset;
	}
#endif 

  return status;
}

/**
  * @brief  Programs a word at a specified address.
  * @note   This function can be used for all STM32F10x devices.
  * @param  Address: specifies the address to be programmed.
  * @param  Data: specifies the data to be programmed.
  * @retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT. 
  */
u32 iflash_word_program(u32 addr, u32 dat)
{
	u32 status = FLASH_COMPLETE;
	__IO u32 tmp = 0;

#ifdef STM32F30X
	/* Wait for last operation to be completed */
	status = FLASH_WaitForLastOperation(ProgramTimeout);
  
	if(status == FLASH_COMPLETE)
	{
		/* if the previous operation is completed, proceed to program the new first 
		half word */
		FLASH->CR |= CR_PG_Set;
  
		*(__IO u16*)addr = (u16)dat;
		/* Wait for last operation to be completed */
		status = FLASH_WaitForLastOperation(ProgramTimeout);
 
		if(status == FLASH_COMPLETE)
		{
			/* if the previous operation is completed, proceed to program the new second 
			half word */
			tmp = addr + 2;

			*(__IO u16*) tmp = dat >> 16;
    
			/* Wait for last operation to be completed */
			status = FLASH_WaitForLastOperation(ProgramTimeout);
        
			/* Disable the PG Bit */
			FLASH->CR &= ~CR_PG_Reset;
		}
		else
		{
			/* Disable the PG Bit */
			FLASH->CR &= ~CR_PG_Reset;
		}
	}         
#endif 
	return status;
}

/**
  * @brief  Programs a half word at a specified address.
  * @note   This function can be used for all STM32F10x devices.
  * @param  Address: specifies the address to be programmed.
  * @param  Data: specifies the data to be programmed.
  * @retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT. 
  */
u32 iflash_halfword_program(u32 addr, u16 dat)
{
	u32 status = FLASH_COMPLETE;

#ifdef STM32F30X
	
	/* Wait for last operation to be completed */
	status = FLASH_WaitForLastOperation(ProgramTimeout);
  
	if(status == FLASH_COMPLETE)
	{
		/* if the previous operation is completed, proceed to program the new data */
		FLASH->CR |= CR_PG_Set;
  
		*(__IO u16*)addr = dat;
		/* Wait for last operation to be completed */
		status = FLASH_WaitForLastOperation(ProgramTimeout);
    
		/* Disable the PG Bit */
		FLASH->CR &= ~CR_PG_Set;
	} 
#endif 
  
  /* Return the Program Status */
  return status;
}

/**
  * @brief  Clears the FLASH's pending flags.
  * @note   This function can be used for all STM32F10x devices.
  *         - For STM32F10X_XL devices, this function clears Bank1 or Bank2�s pending flags
  *         - For other devices, it clears Bank1�s pending flags.
  * @param  FLASH_FLAG: specifies the FLASH flags to clear.
  *   This parameter can be any combination of the following values:         
  *     @arg FLASH_FLAG_PGERR: FLASH Program error flag       
  *     @arg FLASH_FLAG_WRPRTERR: FLASH Write protected error flag      
  *     @arg FLASH_FLAG_EOP: FLASH End of Operation flag           
  * @retval None
  */
static void __iflash_flag_clear(u32 flag)
{
#ifdef STM32F30X
 	FLASH->SR = flag;
#endif /* STM32F10X_XL */
}

void iflash_flag_clear(void)
{
	__iflash_flag_clear(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
}

#if ENABLE_DEBUG_UART
void iflash_operation_result(device_info_t *dev, u32 ret)
{
	switch(ret)
	{
		case FLASH_BUSY:

			dbg_msg(dev, "FLASH BUSY\r\n");
			break;

		case FLASH_ERROR_PG:

			dbg_msg(dev, "FLASH ERROR PG\r\n");
			break;

		case FLASH_ERROR_WRP:
	
			dbg_msg(dev, "FLASH ERROR WRP\r\n");
			break;

		case FLASH_COMPLETE:
	
			dbg_msg(dev, "FLASH COMPLETE >>>> success\r\n");
			break;

		case FLASH_TIMEOUT:
	
			dbg_msg(dev, "FLASH TIMEOUT\r\n");
			break;
	}
}
#endif
//������д��
//WriteAddr:��ʼ��ַ ����Ϊż����ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)����   
u8 STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		if(FLASH_COMPLETE^iflash_halfword_program(WriteAddr,pBuffer[i]))
		 {
		 	return 0;
		}
	   else  WriteAddr+=2;//��ַ����2.
	} 
	return 1;
} 


//��NumToRead�����ָ�����
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=READ_REG16(ReadAddr);//���ֵ�ַ��
		ReadAddr+=2;//���ֲ���2	
	}
}

/*****************************************************************************
 �� �� ��  : STMFLASH_BUF
 ��������  : flashģ��EEPROM д����  ,��ָ����ַ��ʼд��ָ�����ȵ����ݡ��ⲿдDID  дrecord��
 �������  : 
		WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
		Buffer:����ָ��
		NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
 �������  : 
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��7��25��
    ��    ��   : xz
    �޸�����   : �����ɺ���

*****************************************************************************/
u16 STMFLASH_BUF[1024]={0};//1ҳ����ȡ�������ڴ档
u8 STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{

	device_info_t *dev;
	u32 secpos;    //������ַ
	u16 secoff;    //������ƫ�Ƶ�ַ��16λ��
	u16 secremain; // ������ʣ���ַ��16λ��	
	u16 i;	  
	u32 offaddr;   //ȥ������ַ�� ��ַ
	if(WriteAddr<BOOT_CODE_BASE||(WriteAddr>=(BOOT_CODE_BASE+FLASH_SIZE)))
		return 0;//�Ƿ���	
	iflash_unlock();						//����
	offaddr=WriteAddr-BOOT_CODE_BASE;		//ʵ��ƫ�Ƶ�ַ
	secpos=offaddr/2048;			//����ƫ�Ƶ�ַ���ڼ���������
	secoff=(offaddr%2048)/2;		//�����ڵ�ƫ�Ƶ�ַ��16λ,2���ֽ�-����Ϊ��λ��
	secremain=1024-secoff;		//����ʣ��ռ��С(����Ϊ��λ ʣ�����) 
	if(NumToWrite<=secremain)
		secremain=NumToWrite;//�����ڸ�����ʣ������
	while(1) 
	{	
		STMFLASH_Read(secpos*2048u+BOOT_CODE_BASE,STMFLASH_BUF,2048/2);//���������������ݵ��ڴ�
		for(i=0;i<secremain;i++)//У������
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)
				break;//��Ҫ����������for�������	  
		}
		if(i<secremain)//��Ҫ������ i��λ�ڲ�Ϊ0xFFFF���Ǹ���ַ�ط�	
		{
			iflash_page_erase(secpos*2048+BOOT_CODE_BASE);//������������	
			for(i=0;i<secremain;i++)//����
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}

			if(!STMFLASH_Write_NoCheck(secpos*2048+BOOT_CODE_BASE,STMFLASH_BUF ,2048/2))//д������
				{
					dbg_msg(dev, "ERROR: Flash addr prog failed \r\n");
					return 0;
				}
		}
		else
		{
			if(!STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain))//д�Ѿ������ģ�ֱ��д������ʣ��ռ�				   
				{
					dbg_msg(dev, "ERROR: Flash addr prog failed \r\n");
					return 0;
				}
			}
		if(NumToWrite==secremain)
			break;//д�����
		else//δ����
		{
			secpos++;				//��ַ+1
			secoff=0;				//ƫ��λ��Ϊ0	 
			pBuffer+=secremain; 	//ָ��ƫ��
			WriteAddr+=secremain*2; //д��ַƫ�ƣ�16λ��	   
			NumToWrite-=secremain;	//��??��(16??)��y��Y??
			if(NumToWrite>(2048/2))secremain=2048/2;//�¸�������д����
			else
				secremain=NumToWrite;//�¸�����д����
		}	 
	}	
	iflash_lock();//����
	return 1;
}


//*****************************************************************************************
// DESCRIPTON : д���洢�������Ҷ�����д����Ƿ�һ��
// ARGUMENT   :
// RETURN     : 0 - д�����ݺͶ������ݲ�һ��
//              1 - д�����ݺͶ�������һ��
// AUTHOR     :
//*****************************************************************************************/
u8 Update_WriteAndCheck(device_info_t *dev,u16 *buf,u32 bagnum ,u32 len)
{
	u32 addr,i,NumToWrite;
	//dbg_msgv(dev,"len =%d\r\n",len);
	if(len%2==0)
	{
		NumToWrite =(len)/2;
		}
	else
		{
		NumToWrite =(len)/2+1;
	}
	
	if(dev->ipc->upgrade_mcu_boot!=0x00)
	{
		addr = BOOT_CODE_BASE+(bagnum-1)*FLASH_PAGE_SIZE; 
		}
	else
	{
		addr = APP_CODE_BASE+(bagnum-1)*FLASH_PAGE_SIZE;
		}	
	iflash_unlock();	
	if((iflash_page_erase(addr)^FLASH_COMPLETE) !=0)		
	{		
//		printf("erase falut\r\n");		
		return 2;                      		
	}                                   		
	for(i=0;i<NumToWrite;i++)
	{
		if(FLASH_COMPLETE^iflash_halfword_program(addr+i*2,buf[i]))
		{
		 	return 0;
		}
	} 
	iflash_lock();
	return 1;
}

/*****************************************************************************
 �� �� ��  : read_dtc_from_eeprom
 ��������  : ��flash��ȡ��ȡ��DTC��
 �������  : unsigned int offset  
             unsigned char *buf   
             uint16_t len         
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��7��26��
    ��    ��   : xz
    �޸�����   : �����ɺ���

*****************************************************************************/
void read_dtc_from_flash(u32 addr,u8 *buf,u16 len)
{
	
	STMFLASH_Read(addr,(u16*)buf,len/2);
}


