

#define CORE_72MHZ // For 72MHz HCLK config
// 2) HSE clock, external high-speed oscilator, depand on board
#define HSE_HZ					12000000ul
//#define HSE_HZ					8000000ul
// 3) HSI clock, fixed 8MHz
#define HSI_HZ					8000000ul
// 4) SRAM size config
//#define SRAM_SIZE				(0x400ul * 10) /* STM32F101R8 has 10KB */
//#define SRAM_SIZE_MASK		(0x400ul << 4) - 1)
#define SRAM_SIZE				(0x400ul * 24) /* STM32F302RB has 32KB */
#define SRAM_SIZE_MASK			((0x400ul << 5) - 1)
//#define SRAM_SIZE				(0x10000ul) /* STM32F105(7)XX has 64KB */
//#define SRAM_SIZE_MASK			(SRAM_SIZE - 1)
// 5)  Flash related config
//#if (defined STM32F10X_LD) || (defined STM32F10X_MD)
#define FLASH_PAGE_SIZE		0x800ul  
//#else
//#define FLASH_PAGE_SIZE		0x800ul
//#endif
#define FLASH_SIZE				(128ul*1024) /* STM32F103C8 has 64KB flash */
#define BOOT_CODE_BASE			FLASH_BASE
#define APP_FLAG_CODE			C4_2_U32('A', 'R', 'M', 'C') // app valid flag, 1.x.x version not compatible
#define APP_CODE_BASE			(BOOT_CODE_BASE + 0x7800ul) // Boot code size: 30KB(include flag page area)
#define APP_FLAG_ADDR			(APP_CODE_BASE-FLASH_PAGE_SIZE) // update flag addr, 1.x.x version not compatible





// 6) Watchdog config, select one
// IWDG & WWDG can't stop by software
// except by hot/cold resets
#define ENABLE_EXTERNAL_WDG			0 // external watchdog
#define ENABLE_INTERNAL_IWDG		0
#define ENABLE_INTERNAL_WWDG		0 // don't use, for interval too short
// 7) Mach-type & uuid
#define UUID_BYTES					16
// 8) IPC RAM base
#define RAM_IPC_BASE				SRAM_BASE


//GPIO
#define IO_LEFT						GPIOA
#define IOBIT_LEFT					1

#define IO_RIGHT					GPIOA
#define IOBIT_RIGHT					2

#define IO_ACC						GPIOA
#define IOBIT_ACC					8

#define  IO_CARCAN_STB          GPIOB
#define  IOBIT_CARCAN_STB        8

#define  IO_CARCAN_WKAEUP          GPIOA
#define  IOBIT_CARCAN_WKAEUP         11

#define  IO_CP_CONTROL			GPIOB
#define  FCP_S					15u
#define  BCP_S					14u
#define  LCP_S					13u
#define  RCP_S					12u

// 10) Machine type define, select one
// 11) Debug info support
#define CFG_DEBUG_UART				1
#define CFG_MACH_EXTEND_360			1 /* Íâ¹Ò360 */

