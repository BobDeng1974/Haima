#ifndef __CAN_DID_H__
#define __CAN_DID_H__
#ifdef CAN_DID_GLOBALS
#define CAN_DID_EXT 
#else
#define	CAN_DID_EXT	extern
#endif

#include "stm32f30x.h"
#include "miscdef.h"
//æ½æŸ´  added 20180723 xz
#define LEN_DID_FLAG   			 	1
#define LEN_CURRENT_DIAG_SESSION    1
#define LEN_Part_Number          	9
#define LEN_ECUSoftwareVersion   	2
#define LEN_Supplier_ID       	    8
#define LEN_Manufacture_Date 		4
#define LEN_SerialNumber     		10
#define LEN_Supported_units        	1
#define LEN_VINDataIdentifier    	17
#define LEN_System_name         	4
#define LEN_Install_date       	  	4
#define LEN_BAT_Vol        			2
#define LEN_MCU_APP_VER        		3
#define LEN_MCU_BOOT_VER       		3
#define LEN_ARM_APP_VER       		3
#define LEN_ARM_SF_VER       		3
#define LEN_Fingerprint          	4
#define LEN_DID_MAX         		30

#define  DID_CURRENT_DIAG_SESSION					 	0xF186
#define  DID_Part_Number         					 	0XF187
#define  DID_ECUSoftwareVersion         				0XF189 
#define  DID_Supplier_ID        						0XF18A
#define  DID_Manufacture_Date 							0xF18B
#define  DID_ECUSerialNumber        					0XF18C 
#define  DID_Supported_units        					0XF18D
#define  DID_VINDataIdentifier         					0XF190
#define  DID_System_name         						0XF197	
#define  DID_Install_date         						0XF19D
#define  DID_BAT_Vol        							0XF1B0
#define  DID_Fingerprint     						    0XF199
#define  DID_MCU_APP_VER        						0XFD00
#define  DID_MCU_BOOT_VER        						0XFD01
#define  DID_ARM_APP_VER        						0XFD02
#define  DID_ARM_SF_VER        							0XFD03


#define DID_FlASH_ADDR_BASE    				(FLASH_BASE+0x1FA00ul)
#define ADDR_DID_FLAG					  	(DID_FlASH_ADDR_BASE)
#define ADDR_Part_Number        			(ADDR_DID_FLAG+2)
#define ADDR_ECUSoftwareVersion   			(ADDR_Part_Number+LEN_Part_Number+1)  	
#define ADDR_Supported_units     			(ADDR_ECUSoftwareVersion+LEN_ECUSoftwareVersion)
#define ADDR_VINDataIdentifier 				(ADDR_Supported_units+LEN_Supported_units+1)
#define ADDR_System_name       				(ADDR_VINDataIdentifier+LEN_VINDataIdentifier+1)
#define ADDR_Install_date         			(ADDR_System_name+LEN_System_name) 
#define ADDR_Fingerprint    		        (ADDR_Install_date+LEN_Install_date)
#define ADDR_MCU_APP_VER        			(ADDR_Fingerprint+LEN_Fingerprint)
#define ADDR_MCU_BOOT_VER       			(ADDR_MCU_APP_VER+LEN_MCU_APP_VER+1)
#define ADDR_ARM_APP_VER          			(ADDR_MCU_BOOT_VER+LEN_MCU_BOOT_VER+1)  
#define ADDR_ARM_SF_VER      				(ADDR_ARM_APP_VER+LEN_ARM_APP_VER+1)  		
#define ADDR_Supplier_ID					(ADDR_ARM_SF_VER+LEN_ARM_SF_VER+1)
#define ADDR_Manufacture_Date				(ADDR_Supplier_ID+LEN_Supplier_ID)
#define ADDR_ECUSerialNumber 				(ADDR_Manufacture_Date+LEN_Manufacture_Date)



CAN_DID_EXT void init_did_data(	device_info_t *dev);
#endif
