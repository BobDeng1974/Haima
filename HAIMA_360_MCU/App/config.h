
#ifndef __CONFIG_H__
#define __CONFIG_H__

///////////// HW config, fixed /////////////
#include <hw_config.h>


///////////// SW config ////////////////

// USART config
#define ENABLE_DEBUG_UART			CFG_DEBUG_UART
#define ENABLE_COMM_UART			1

// command line support(for debug)
#define ENABLE_CMD_LINE			1



//0:  for ymodem debug(no iflash program)
#define ENABLE_IFLASH_UPDATE		1

// ymodem upgrading msg ctrl
#define ENABLE_UPGRADE_MSG		1

#define ENABLE_ACC_DET				1
#define ENABLE_KEY_ADC				1

#define ENABLE_CAN_WEKAUP			1//«–ªªACC ªΩ–— ∫Õ ºÊ»› can ªΩ–— 

#define ENABLE_WEICHAI				0

#define ENABLE_HAIMA                1 //∫£¬Ì—› æ 


#define ENABLE_autotest             1

#define EXTEND_ID                   0
#define ENABLE_TEST                 0   //≤ª≤‚ ‘–Ëπÿ±’
#define ENABLE_UPDATE_ARM_BY_CAN    1


//i.mx6 debug 
#define IMX6_NOT_RESET             0


// CAN bus support
// stm32 CAN2 is slave(share filter), CAN1 must be enabled for CAN2
#define ENABLE_CAR_CAN				1

/////////////// post process, don't modify///////////////


#if (0 == ENABLE_DEBUG_UART)
#undef ENABLE_CMD_LINE
#define ENABLE_CMD_LINE			0
#undef ENABLE_UPGRADE_MSG
#define ENABLE_UPGRADE_MSG		0
#define wait_for_interrupt()			__WFI()
#else
#define wait_for_interrupt()
#endif

#endif

