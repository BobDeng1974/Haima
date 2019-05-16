
#ifndef __CONFIG_H__
#define __CONFIG_H__

///////////// HW config, fixed /////////////
#include <hw_config.h>


///////////// SW config ////////////////

// USART config
#define ENABLE_DEBUG_UART			CFG_DEBUG_UART
#define ENABLE_COMM_UART			1

#define ENABLE_WEICHAI				0

#define ENABLE_UPDATE_ARM_BY_CAN    1
// command line support(for debug)
#define ENABLE_CMD_LINE			0

// i2c bus support
#define ENABLE_I2C_BUS				0

// LED config
#define ENABLE_LED_CTRL			0

// ADC config, if 0 don't care ENABLE_REG_CHN
#define ENABLE_KEY_ADC				0
#define ENABLE_REG_CHN				0 /*1: regular channel, 0: injection channel*/
#define ENABLE_HOT_DET				0
#define ENABLE_REBOOT_KEY			0

// FM config
#define ENABLE_FM					0

//0:  for ymodem debug(no iflash program)
#define ENABLE_IFLASH_UPDATE		1

// ymodem upgrading msg ctrl
#define ENABLE_UPGRADE_MSG		1

// ACC GPIO detect
#define ENABLE_ACC_DET				1

// B+ GPIO detect
#define ENABLE_B_DET				0

// SOS key detect
#define ENABLE_SOSKEY_DET			0

// Other GPIO support(ILL/REVS/PARK/IPOD/BEEP)
#define ENABLE_OTHER_GPIO			0
#define ENABLE_BEEP_NOTIFY			0 /* depand on ENABLE_OTHER_GPIO */
#define ENABLE_ILL_DET_V0			0 /* IRQ mode, BOOT/RUNNING stages only */
#define ENABLE_ILL_DET_V1			0 /* Polling mode, All stages */
#define ENABLE_TBOX_WAKE			0 /* SLEEP stage TBOX GPIO wake, same as CAN-INH */

// DTV ctrl
#define ENABLE_DTV					0

// tw8825 support
#define ENABLE_TW8825				0

// sound effect chip config
#define ENABLE_SND_EFFECT			0

// CAN bus support
// stm32 CAN2 is slave(share filter), CAN1 must be enabled for CAN2
#define ENABLE_OBD_CAN				0
#define ENABLE_CAR_CAN				1 /* car can bus */

// Amplifier TEF8541, if 0 don't care ENABLE_TDF8541_I2CMODE
#define ENABLE_TDF8541				0
#define ENABLE_TDF8541_I2CMODE	0 /* 0: legacy no-i2c mode(as TDA7388), 1: support i2c config mode */

#define ENABLE_BOOT_REC   1
/////////////// post process, don't modify///////////////

#if (0 == ENABLE_ACC_DET)
#undef ENABLE_B_DET
#define ENABLE_B_DET				0
#endif

#if (0 == ENABLE_DEBUG_UART)
#undef ENABLE_CMD_LINE
#define ENABLE_CMD_LINE			0
#undef ENABLE_UPGRADE_MSG
#define ENABLE_UPGRADE_MSG		0
#endif

#define ENABLE_UART_LP				(ENABLE_DEBUG_UART | ENABLE_3G_UART | ENABLE_TBOX_UART)

#define ENABLE_CAN_LP				(ENABLE_OBD_CAN | ENABLE_CAR_CAN)

#endif

