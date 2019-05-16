#define CAN_DID_GLOBALS
#include <stdio.h>
#include <string.h>
#include "iflash.h"
#include "can_did.h"
#include "usart.h"
//#include "usart.h"
//支持2E 服务的did：F187、F189、F18D、F190 、F197、F19D、FD00、FD01、FD02、FD03 默认值
const u8 ECU_Part_Number[LEN_Part_Number ]={'B','A','0','1','6','6','4','2','X'}; //ASCII
const u8 ECU_SoftwareVersion[LEN_ECUSoftwareVersion]={01,00};														
const u8 ECU_Supported_units[LEN_Supported_units]={0x11};//7~4温度类型，3~0语言类型  默认0x11
const u8 ECU_VINDataIdentifier[LEN_VINDataIdentifier]="LAPRJHLPB1D205205" ;//车辆VIN号  
const u8 ECU_System_name [LEN_System_name]="AVM ";//System Supplier Identifier 
const u8 ECU_Install_date [LEN_Install_date]={0x20,0x19,0x03,0x22};//ECUSerialNumberDataIdentifier 修正为BCD码 年-年-月-日
const u8 ECU_Fingerprint[LEN_Fingerprint]={0x20,0x19,0x03,0x22};//修正为BCD码 年-年-月-日
const u8 ECU_MCU_APP_VER[LEN_MCU_APP_VER]={1,0,0};//默认值 no use
const u8 ECU_MCU_BOOT_VER[LEN_MCU_BOOT_VER]={1,0,0};//默认值 no use
const u8 ECU_ARM_APP_VER[LEN_ARM_APP_VER]={2,4,9};//默认值 no use
const u8 ECU_ARM_SF_VER[LEN_ARM_SF_VER]={2,4,9};//默认值 no use
const u8 ECU_Supplier_ID[LEN_Supplier_ID]="123-456L";
const u8 ECU_Manufacture_Date[LEN_Manufacture_Date]={0x20,0x19,0x03,0x22};//修正为BCD码 年-年-月-日
const u8 ECU_SerialNumber[LEN_SerialNumber]={0,1,2,3,4,5,6,7,8,9};

void init_did_data(	device_info_t *dev)
{
	typedef union union1024
	{
		u8 b[1024];
		u16 h[512];
	}union1024_t;
	union1024_t w_buffer={{0}};/*强制将2个8位数据转换为1个16位数据*/

	//读取did是否初始化标志位   0 ascii字符对应是空
	STMFLASH_Read(ADDR_DID_FLAG,&w_buffer.h[0], 1);
	if(w_buffer.h[0]!=0x25)
	{		
		dbg_msg(dev," the first time to write did data\r\n");
		w_buffer.h[0]=0x25;
		memcpy(&w_buffer.b[2],&ECU_Part_Number[0],LEN_Part_Number);
		memcpy(&w_buffer.b[2+10],&ECU_SoftwareVersion[0],LEN_ECUSoftwareVersion);
		memcpy(&w_buffer.b[2+12],&ECU_Supported_units[0],LEN_Supported_units);
		memcpy(&w_buffer.b[2+14],&ECU_VINDataIdentifier[0],LEN_VINDataIdentifier);
		memcpy(&w_buffer.b[2+32],&ECU_System_name[0],LEN_System_name);
		memcpy(&w_buffer.b[2+36],&ECU_Install_date[0],LEN_Install_date);
		memcpy(&w_buffer.b[2+40],&ECU_Fingerprint[0],LEN_Fingerprint);
		memcpy(&w_buffer.b[2+44],&ECU_MCU_APP_VER[0],LEN_MCU_APP_VER);
		memcpy(&w_buffer.b[2+48],&ECU_MCU_BOOT_VER[0],LEN_MCU_BOOT_VER);
		memcpy(&w_buffer.b[2+52],&ECU_ARM_APP_VER[0],LEN_ARM_APP_VER);
		memcpy(&w_buffer.b[2+56],&ECU_ARM_SF_VER[0],LEN_ARM_SF_VER);
		memcpy(&w_buffer.b[2+60],&ECU_Supplier_ID[0],LEN_Supplier_ID);
		memcpy(&w_buffer.b[2+68],&ECU_Manufacture_Date[0],LEN_Manufacture_Date);
		memcpy(&w_buffer.b[2+72],&ECU_SerialNumber[0],LEN_SerialNumber);
		STMFLASH_Write(ADDR_DID_FLAG,w_buffer.h ,512);
		}
	else{
		dbg_msg(dev," did data has been written \r\n");
	}
}	
