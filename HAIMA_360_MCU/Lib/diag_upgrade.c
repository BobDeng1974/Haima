/*
********************************************************************************
**                                  CHINA-GPS 
**
** Project  TB1704-17
**
** File     diag_upgrade.c
** 
** Terms:
**    
**         
** Version     Author     Date       changed     Description
** ========   =======    ========    ========    ===========   
**  V1.0       xiezb                 Create     
** 
**
********************************************************************************
*/
#include <string.h>
#include "diag_upgrade.h"
#include "debug.h"
#include "can_diag_protocol.h"
#include "miscdef.h"
#include "can.h"
#include "api.h"
#include "iflash.h"
#include "usart.h"
#include "protocolapi.h"
#include "can_net_layer.h"

u8 SendBuff[1032];

//static u8  Upflag=0;  //���������־λ
UPGRADE_T stUpgradeParam = {0XFF, 0, 0, 0, 0, 0};//��ʼ�������ṹ��
stUpdateInfo_t update ={0,{0}};
/***********************************************************************************************
* Description:
* Arugment   :
* Return     :
* Author     :
***********************************************************************************************/
static const u32 crc32tab[] = {
 0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
 0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
 0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
 0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
 0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
 0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
 0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
 0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
 0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
 0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
 0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
 0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
 0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d 
};

/*************************************************************************************************
* Description:
* Arugment   :
* Return     :
* Author     :
*************************************************************************************************/
unsigned int  GenerateCrc32_1(u32 *inivalue,u8  *Buffer,u32 Length)
{
    u32  crc;
    u32  i;

    crc = *inivalue ^ 0xffffffff;

    for (i = 0; i < Length; i++)
    {
        crc = crc32tab[(crc ^ Buffer[i]) & 0xff] ^ (crc >> 8);
    }

    *inivalue = (crc ^ 0xffffffff);
	return *inivalue;
}

/****************************************************************************************************
* Description:
* Arugment   :
* Return     :
* Author     :
****************************************************************************************************/
u8 FileCRC_Check(device_info_t *dev)
{	
	u16 len = 256;
	u32 read_len=0;
	u32 crc_value = 0;
	//dbg_msgv(dev, "�ļ����� %d\r\n",stUpgradeParam.fileTotal);
	while(read_len < stUpgradeParam.fileTotal)
	{
		if((read_len + 256) > stUpgradeParam.fileTotal)
			len = stUpgradeParam.fileTotal - read_len;
		
		if(len > 0)
		{
			if(dev->ipc->upgrade_mcu_boot==0x00)
			{
				STMFLASH_Read( APP_CODE_BASE+read_len, (u16 *)update.buff, len);
				}
			else
			{	
				STMFLASH_Read( BOOT_CODE_BASE+read_len, (u16 *)update.buff, len);
				}
			crc_value=GenerateCrc32_1(&crc_value,(u8 *) update.buff, len);	
			read_len += len;
		}
		watchdog_feed(dev);
	}
	if(crc_value == stUpgradeParam.fileCheck)
		return 1;
	
	return 0;	
}

void upgrade_mcu_boot_init(device_info_t *dev)
{
	u8 send_data[8]={0};
	dev=get_device_info();
	dev->ci->host_ready = 0 ;//��ʼ����36����
	diag.security_level1_state = DIAG_SECURITY_LEVEL_STATE_UNLOCK;
	diag.security_level3_state = DIAG_SECURITY_LEVEL_STATE_UNLOCK;
	diag.sesion_mode = DIAG_PR_MODE;
	stUpgradeParam.optStep = OPT_EREASE_MEMORY_OK;
	dev->ci->state = HOST_UPGRADING ;//��ʼ����36����
	//0x78 ��Ӧ�ȴ�
	send_data[0]=0x05;
	send_data[1]=0x71;
	send_data[2]=0x01;//Routine Control Type
	send_data[3]=0x01;
	send_data[4]=0x00;
	send_data[5]=0x01;//�ɹ�01 ʧ��02
	can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32*)send_data);
}


//$31 ff02�ӷ���
u8 checkProgrammingDependencies(device_info_t *dev)
{
	return 0;
}

void update_mcu_ver(device_info_t *dev)
{
	u8 buf[2]={0};
	STMFLASH_Read(ADDR_ECUSoftwareVersion, (u16 *)buf,1);
	buf[1]++;
	STMFLASH_Write(ADDR_ECUSoftwareVersion,(u16*)buf,1);
	mdelay(100);
}


/********************************************************************************************
* Description: step1: earseMemory
* Argument   :
* Return     :
* Author     :
********************************************************************************************/
u8  upgrade_earseMemory(device_info_t *dev)
{
	u32 i=0;
	stUpgradeParam.optStep = 0XFF;
//	printf("erase App flag \r\n");
	iflash_unlock();
	i = iflash_page_erase(APP_FLAG_ADDR); // clear flag sector
	if((i^FLASH_COMPLETE) !=0)
	{
		dbg_msg(dev,"erase falut\r\n");
		return 2;
		}
	iflash_lock();
	dbg_msg(dev,"Erase app\r\n");
	stUpgradeParam.optStep = OPT_EREASE_MEMORY_OK;

//	Upflag                 = 1;

	return 	1;
}

/********************************************************************************************
* Description: step2: ���������ļ�����
* Argument   :
* Return     :
* Author     : xz modify 20180604
********************************************************************************************/
void upgrade_requestFileDownload(device_info_t *dev,u8 *buf, u16 len)
{
	UINT8 response_data[8]= {0x04,0x74,0x20,0x08,0x04,0x00,0x00,0x00};//�̶���ʽ�Ļ�Ӧ��2K����һ����(stm32f302����2KΪ��ͣ���ΪflashһҳΪ2K)
	
	if(OPT_EREASE_MEMORY_OK == stUpgradeParam.optStep)
	{//4���ֽڵ�ַ�����4���ֽ��ļ�����(ʵ���������������ݣ�������SID�Ͱ����)
		stUpgradeParam.fileTotal = ((buf[6])<<24)|\
			                       ((buf[7])<<16)|\
			                       ((buf[8])<<8 )|\
			                       (buf[9]);
		stUpgradeParam.update_type =buf[10];
		dbg_msgv(dev, "file size:%d words\r\n",stUpgradeParam.fileTotal );
		can_msg_Send_SWJ(dev, AVM_Phy_Resp,(u32*) response_data);//�ȴ�$36 �������
		stUpgradeParam.optStep  = OPT_REQ_DOWNLOAD_OK;//0x01
		stUpgradeParam.bagSeq   = 1; // the first pack  36����ĵ�һ����
		stUpgradeParam.fileRecvCnt = 0; 
//		Upflag         = 1;
	}
}

/********************************************************************************************
* Description: step2: ���������ļ�����
* Argument   :
* Return     :
* Author     : xz modify 20180604
********************************************************************************************/
void upgrade_requestFileDownload_ARM(device_info_t *dev,u8 *buf, u16 len)
{
	if(OPT_EREASE_MEMORY_OK == stUpgradeParam.optStep)
	{//4���ֽڵ�ַ�����4���ֽ��ļ�����(ʵ���������������ݣ�������SID�Ͱ����)
		stUpgradeParam.fileTotal = ((buf[6])<<24)|\
			                       ((buf[7])<<16)|\
			                       ((buf[8])<<8 )|\
			                       (buf[9]);
		dbg_msgv(dev, "�����ļ���С��%d ���ֽ�\r\n",stUpgradeParam.fileTotal );
		stUpgradeParam.update_type = buf[10];
		stUpgradeParam.optStep  = OPT_REQ_DOWNLOAD_OK;//0x01
		stUpgradeParam.bagSeq   = 1; // the first pack  36����ĵ�һ����
		stUpgradeParam.fileRecvCnt = 0; 
//		Upflag         = 1;
	}
}

/********************************************************************************************
* Description: step3: �������ص��ļ�����
* Argument   :
* Return     :
* Author     : xzb modify 20170303
********************************************************************************************/
void upgrade_transferData(device_info_t *dev,u8 *buf, u16 len)
{
	u8 response_data[8] = {0x01,0x7F};
	u32 i=0;
	u32 curBagSeq;
	if(OPT_REQ_DOWNLOAD_OK != stUpgradeParam.optStep)
	{
			return;
		}
	//3�ֽڰ���Ÿ�Ϊ4�ֽڣ����ֽ���ǰ
	curBagSeq =0;
	curBagSeq |=buf[3]<<16;
	curBagSeq |=buf[2]<<8;
	curBagSeq |=buf[1];
	//����MCU
	if((stUpgradeParam.update_type==0x10)||(stUpgradeParam.update_type==0x20))
	{
	  if(stUpgradeParam.bagSeq != curBagSeq)
	  {
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)response_data);
	//	Upflag = 0;
		return; //���Ŵ���
	   }
	  if(0 == Update_WriteAndCheck(dev,(u16*)&buf[4],curBagSeq,len-4))//len��ȥ�����ռһ���ֽ�
	  {// write and check fail
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)response_data);
	//	Upflag = 0;
		return;
	    }
	
	//  Upflag = 1;
	  stUpgradeParam.fileRecvCnt += (len-4);
	  if(stUpgradeParam.fileRecvCnt <= stUpgradeParam.fileTotal)
	   {
		response_data[0] = 0x02;
		response_data[1] = 0x76;
		response_data[2] = (u8)stUpgradeParam.bagSeq;
		if(255 == stUpgradeParam.bagSeq)
			 stUpgradeParam.bagSeq = 1;
		else
			stUpgradeParam.bagSeq++;
		i = stUpgradeParam.fileRecvCnt *100/stUpgradeParam.fileTotal;
		dbg_msgv(dev,"��������: %d%%\r\n",i);
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)response_data);
	   }
	}
	else 
	{
	if(stUpgradeParam.bagSeq != curBagSeq)
	{
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)response_data);
	//	Upflag = 0;
		return; //���Ŵ���
		}
			//���͵�ǰ������
		buf[0]= (len>>8)&0xff ;
		buf[1]= (curBagSeq>>16)&0xff ;
		buf[2]= (curBagSeq>>8)&0xff ;
	    buf[3]=  curBagSeq&0xff ;
		comm_message_send(dev, CM_UPDATE_ARM_DATATRANS, 0, &buf[0],len);
	//	Upflag = 1;
		stUpgradeParam.fileRecvCnt += (len-4);
		//�ظ���λ��
		response_data[0] = 0x04;
		response_data[1] = 0x76;
		response_data[2] = (stUpgradeParam.bagSeq&0xff0000)>>24;
	 	response_data[3] = (stUpgradeParam.bagSeq&0xff00)>>8;
		response_data[4] = stUpgradeParam.bagSeq&0xff;
	    can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)response_data);
	 }		
}

/********************************************************************************************
* Description: �˳��ļ���������
* Argument   :
* Return     :
* Author     : xzb modify 20170303
********************************************************************************************/
void upgrade_requestTransferExit(device_info_t *dev,u8 *buf, u16 len)
{
	u8  response_data[8] = {0x01,0x77,0x00,0x00,0x00,0x00,0x00,0x00};

	if(OPT_REQ_DOWNLOAD_OK == stUpgradeParam.optStep)
	{
		can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,(u32 *)response_data);	   
		stUpgradeParam.optStep = OPT_EXIT_DOWNLOAD_OK;
//		Upflag                 = 0;
	}
}
/********************************************************************************************
* Description: �ļ�����У��1������Ӧ��
* Argument   :
* Return     :
* Author     :
********************************************************************************************/
void upgrade_loadFile_check1(device_info_t *dev,u8 *buf, u16 len)
{	
	u8 response_data[8]   = {0x05,0x71,0x01,0xff,0x01};
	u8 file_check_arm[4]={0};
	u8 i=0;
	//Upflag = 0;
	if(OPT_EXIT_DOWNLOAD_OK == stUpgradeParam.optStep)
	{
		stUpgradeParam.fileCheck = (buf[4]<<24)|\
			                       (buf[5]<<16)|\
			                       (buf[6]<< 8)|\
			                       (buf[7]);		
		if((stUpgradeParam.update_type==0x10)||(stUpgradeParam.update_type==0x20))
		{
		  if(1 == FileCRC_Check(dev))
		  {
			response_data[5]= 0x01;/*У��ɹ�*/
			can_msg_Send_SWJ_Up(dev,AVM_Phy_Resp,(u32*)response_data);
			stUpgradeParam.optStep = OPT_REQ_CHECK1_OK;	
			update_mcu_ver(dev);
			dbg_msg(dev, "����У��ɹ�,�������\r\n");
			
			/*дAPP_FLAG*/
				//i = item_test_operation(dev, NULL);
			if(dev->ipc->upgrade_mcu_boot==0x00)
			{
				iflash_unlock();
				i = iflash_page_erase(APP_FLAG_ADDR); // clear flag sector
				if((i^FLASH_COMPLETE) !=0)
				{
					dbg_msg(dev,"erase falut\r\n");
				} 
				i=iflash_word_program(APP_FLAG_ADDR, APP_FLAG_CODE);
				iflash_lock();
				iflash_operation_result(dev, i);
				dbg_msg(dev," write app flag success,ready to reset \r\n");
				}
			else
			{
				dev->ipc->upgrade_mcu_boot=0x00;
				}
		  	}
		 
		  else
		  {
			response_data[5]= 0x00;
			can_msg_Send_SWJ_Up(dev,AVM_Phy_Resp,(u32*)response_data);
			}
			}
		else
		{
			file_check_arm[3]=buf[7];
			file_check_arm[2]=buf[6];
			file_check_arm[1]=buf[5];
			file_check_arm[0]=buf[4];
			dbg_msgv(dev,"ARM crc32 = %x-%x-%x-%x\r\n", file_check_arm[0],file_check_arm[1],file_check_arm[2],file_check_arm[3]);
			comm_message_send(dev, CM_UPDATE_ARM_DATATRANS_END, 0,&file_check_arm[0], 4);
			}
	}

}

