
#ifndef __MISCDEF_H__
#define __MISCDEF_H__

#include <stm32f30x.h>
#include <config.h>
#include <stm32f30x_rcc.h>
#include <nm.h>
typedef u32 bool;

typedef u8 BYTE;
typedef unsigned short  WORD16;
typedef unsigned long INT32U;
typedef unsigned int UINT;

#define C4_2_U32(a, b, c, d)		((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))


enum
{
	// event-flag, bit position for bit-bang access
	FLAG_RESERVED = 0, // reserved flag
	FLAG_TIMEOUT_KEYADC, // used by ADC key scan, drive by systick, systick base interval
	FLAG_TIMEOUT_ENCADC, // used by encoder adc scan, drive by hw timer
	FLAG_TIMEOUT_SEND_BODY_CAN_10MS,
	FLAG_TIMEOUT_10MS,
	FLAG_TIMEOUT_20MS, //
	FLAG_TIMEOUT_WD, // used by watchdog, 50ms interval
	FLAG_TIMEOUT_100MS, // used by req-pwroff mode time delay
	FLAG_TIMEOUT_NO_ACK_10MS,
	FLAG_TIMEOUT_400MS, //
	FLAG_TIMEOUT_500MS,
	FLAG_TIMEOUT_1S,
	FLAG_HOST_TIMEOUT, // host feed mcu failed

	FLAG_UART_DEBUG_DMAT_END , // UART for debug Tx completed
	FLAG_UART_COMM_DMAT_END, // UART for comm with host dma Tx completed

	FLAG_STIMER_START = 15,
	FLAG_STIMER0 = FLAG_STIMER_START,
	FLAG_STIMER1 ,
	FLAG_STIMER2,
	FLAG_STIMER3,
	FLAG_STIMER4,
	FLAG_STIMER5,
	FLAG_STIMER6,
	FLAG_STIMER7,
	FLAG_STIMER8,
	FLAG_STIMER9,
	FLAG_STIMER_END,
	FLAG_MAX=FLAG_STIMER_END+1,
	// !!!!!!!!!!flag count depands on device_info.flags bits!!!!!!!!!!!
	// current is 128

	// not affect FLAG_MAX
	//// BOOTING stage
	// STIMER flag share must take care, stage-switch may cause conflict
	FLAG_HOST_REST = FLAG_STIMER0, // [BOOT/WAKEUP]
	FLAG_HOST_ACK, // [BOOT/WAKEUP]
	FLAG_INA_TIMEOUT, // [WAKEUP]
	FLAG_SHUTDN_TIMEOUT,
	FLAG_BODYCAN_MISS0,
	FLAG_BODYCAN_MISS1,
	FLAG_BODYCAN_MISS2,
	FLAG_BODYCAN_MISS3,
	FLAG_BODYCAN_MISS4,
	FLAG_S3_CLIENT,
	FLAG_SECURITY_DELAY,
	
	FLAG_FCP_LINE=FLAG_MAX+1,   //used by ISR, no flag gap
	FLAG_FCP_LINE_OFF,
	FLAG_FCP_LINE_ON,

	FLAG_BCP_LINE,  	// used by ISR, no flag gap
	FLAG_BCP_LINE_OFF,  //
	FLAG_BCP_LINE_ON,   //

	FLAG_LCP_LINE,  	//used by ISR, no flag gap
	FLAG_LCP_LINE_OFF,
	FLAG_LCP_LINE_ON,

	FLAG_RCP_LINE, 		// used by ISR, no flag gap
	FLAG_RCP_LINE_OFF,  //
	FLAG_RCP_LINE_ON,  // 


	FLAG_ACC_LINE, //used by ISR, no flag gap
	FLAG_ACC_LINE_ON,
	FLAG_ACC_LINE_OFF,

	FLAG_B_LINE, // used by ISR, no flag gap
	FLAG_B_LINE_OFF, // B+ is L
	FLAG_B_LINE_ON, // B+ is H

	FLAG_CARCAN, // used by ISR, no flag gap
	FLAG_CARCAN_ON, // car can wake event
	FLAG_CARCAN_OFF, // no car can wake event, not use


	
	FLAG_RCP_POWERON ,
	FLAG_LCP_POWERON ,
	FLAG_BCP_POWERON ,
	FLAG_FCP_POWERON ,

	FLAG_USART_ORE,


};
enum
{
	OFF = 0,
	ON,
	TRISTATE, // usually INPUT mode

	//level
	LOW = 0,
	HIGH,

	// bool
	FALSE = 0,
	TRUE = 1,
	
	ADC_DEV0 = 0,
	CAN_REMOTE_WAKEUP = 1, // wake up by remote can bus message(by ECU)

	// return value
	ERR_MSG_HEAD = 0x10,
	ERR_MSG_FORMAT,
	ERR_MSG_ID_RANGE,
	ERR_MSG_CHKSUM,
	ERR_MSG_TAIL,

	// iflash status
	FLASH_BUSY = 1,
	FLASH_ERROR_PG,
	FLASH_ERROR_WRP,
	FLASH_COMPLETE,
	FLASH_TIMEOUT,

	USART_DEV0 = 0,
	USART_DEV1,
	USART_DEV_CNT,


	TIMER_DEV0 = 0,
	TIMER_DEV1,//TIM2
	TIMER_DEV2,//TIM3
	TIMER_DEV3,//TIM4
	TIMER_DEV7,//TIM4
	TIMER_DEV_CNT,
	
//	SYSTICK_DEV,

	EXTI_DEV0 = 0, /* dvdroom ctrl */
	EXTI_DEV1, /* GPD2, 101, VOL Encoder Up/Down */
	EXTI_DEV2, /* GPC9, ill det */
	EXTI_DEV3, /* GPC8, ACC det */
	EXTI_DEV4, /* gpio encoder: GPC7=VOL+, GPB2=VOL- */
	EXTI_DEV5, /* gpio encoder: GPB5=FREQ+, GPB8=FREQ- */
	EXTI_DEV6, /* GPA4/PB8, reverse det */
	EXTI_DEV7, /* GPC10, Parking det */
	EXTI_DEV8, /* GPC12, IPod det */
	EXTI_DEV9, /* GPA5, B+ det */
	EXTI_DEV10, /* GPE2, SOS key det */
	EXTI_DEV11, /* GPC12, CAR can transceiver INH interrupt */
	EXTI_DEV12, /* GPE0, host beep nofity */
	EXTI_DEV13, /* GPA11, tbox-can wake gpio */
	EXTI_DEV_CNT,
	CAN_DEV0 = 0,

	// key value
	KEY_NULL = 0,


	// host state
	HOST_BOOTING = 0, /* host power on */
	HOST_UPGRADING, /* mcu upgrading */
	HOST_RUNNING, /* host normal stage */
	//HOST_WAKEUP, /* mainboard power on when ACC off & B+ is ON, for CAN/SOS wakeup */
	HOST_ACCOFF,
	//HOST_SLEEP, /* position fixed(Systick), mainboard power off when ACC off & B+ is ON */
	//HOST_DEEP_SLEEP, /* position fixed(Systick), extra low power state, when B+ if OFF */
	
	/* postion fixed(Systick), no more stage(s) */

	// sub state for HOST_RUNNING stage
	SSTATE_RUN = 0,
	SSTATE_READY,


	// debug cmd set
	DBG_CMD_RESET = C4_2_U32('r', 'e', 's', 'e'), // software reset chip
	DBG_CMD_AVER = C4_2_U32('a', 'v', 'e', 'r'), // get app version
	DBG_CMD_BVER = C4_2_U32('b', 'v', 'e', 'r'), // get boot version
	DBG_CMD_IPCINFO = C4_2_U32('i', 'p', 'c', 0), // get ipc info
	DBG_CMD_STATE = C4_2_U32('s', 't', 'a', 't'), // get curr state
	DBG_CMD_CIINFO = C4_2_U32('c', 'i', 0, 0), // get ci info
	DBG_CMD_CAN = C4_2_U32('c', 'a', 'n', 0), // for help info

	DBG_CMD_LIST = C4_2_U32('l', 's', 0, 0), // for help info
	DBG_CMD_ASK = C4_2_U32('?', 0, 0, 0), // for help info
	DBG_CMD_HELP = C4_2_U32('h', 'e', 'l', 'p'), // get cmd info
	
	DBG_CMD_INVALID = ~0
};





/**/ 
enum
{
  GPIO_Mode_IN   = 0x00, /*!< GPIO Input Mode */
  GPIO_Mode_OUT  = 0x01, /*!< GPIO Output Mode */
  GPIO_Mode_AF   = 0x02, /*!< GPIO Alternate function Mode */
  GPIO_Mode_AN   = 0x03,  /*!< GPIO Analog In/Out Mode      */
	
	/*pull dowm*/
	GPIO_PuPd_NOPULL = 0x00,
  GPIO_PuPd_UP     = 0x01,
  GPIO_PuPd_DOWN   = 0x02,
	
	/*输出*/
	GPIO_OType_PP = 0x00,
  GPIO_OType_OD = 0x01
};

#define GPIO_AF_0            ((uint8_t)0x00) /* JTCK-SWCLK, JTDI, JTDO/TRACESW0, JTMS-SWDAT,  */
#define GPIO_AF_1            ((uint8_t)0x01) /*  OUT, TIM2, TIM15, TIM16, TIM17 */
#define GPIO_AF_2            ((uint8_t)0x02) /* COMP1_OUT, TIM1, TIM2, TIM3, TIM4, TIM8, TIM15, TIM16 */
#define GPIO_AF_3            ((uint8_t)0x03) /* COMP7_OUT, TIM8, TIM15, Touch, HRTIM1 */
#define GPIO_AF_4            ((uint8_t)0x04) /* I2C1, I2C2, TIM1, TIM8, TIM16, TIM17 */ 
#define GPIO_AF_5            ((uint8_t)0x05) /* IR_OUT, I2S2, I2S3, SPI1, SPI2, TIM8, USART4, USART5 */ 
#define GPIO_AF_6            ((uint8_t)0x06) /*  IR_OUT, I2S2, I2S3, SPI2, SPI3, TIM1, TIM8 */
#define GPIO_AF_7            ((uint8_t)0x07) /* AOP2_OUT,  COMP3_OUT, COMP5_OUT, COMP6_OUT, 
                                                USART1, USART2, USART3 */ 
#define GPIO_AF_8            ((uint8_t)0x08) /* COMP1_OUT, COMP2_OUT, COMP3_OUT, COMP4_OUT, 
                                                COMP5_OUT, COMP6_OUT */
#define GPIO_AF_9            ((uint8_t)0x09) /* AOP4_OUT, CAN, TIM1, TIM8, TIM15 */
#define GPIO_AF_10            ((uint8_t)0x0A) /* AOP1_OUT, AOP3_OUT, TIM2, TIM3, TIM4, TIM8, TIM17 */
#define GPIO_AF_11            ((uint8_t)0x0B) /* TIM1, TIM8 */
#define GPIO_AF_12            ((uint8_t)0x0C) /* TIM1, HRTIM1 */
#define GPIO_AF_13            ((uint8_t)0x0D) /* HRTIM1, AOP2_OUT */ 
#define GPIO_AF_14            ((uint8_t)0x0E) /* USBDM, USBDP */
#define GPIO_AF_15            ((uint8_t)0x0F) /* OUT */


#define GPIO_clear_2bit 	 ((uint32_t)0x00000003)
#define GPIO_clear_1bit	 	 ((uint32_t)0x00000001)
#define GPIO_Speed_Max      (uint32_t)0x00000003)

#define IO_SET(gpio, bit, stat)			(gpio)->BSRR = __BIT((bit) + (!(stat) << 4))//置高置低
#define IO_GET(gpio, bit)				((gpio)->IDR >> (bit) & 1)//读输入IO 值

#define READ_FLASH32(addr)		(*(__I u32 *)(addr))
#define READ_FLASH16(addr)		(*(__I u16 *)(addr))
#define READ_REG32(addr)			(*(__I u32 *)(addr))
#define READ_REG16(addr)			(*(__I u16 *)(addr))
#define READ_REG8(addr)				(*(__I u8 *)(addr))

#define DIFF(a, b)				(((a) >= (b)) ? ((a) - (b)) : ((b) - (a)))

#define is_alpha(c)				((((c) >= 'a') && ((c) <= 'z')) || (((c) >= 'A') && ((c) <= 'Z')))
#define is_digit(c)				(((c) >= '0') && ((c) <= '9'))


// Bit-bang area conversion
#define BITBANG_ADDR(ram)		(0x22000000ul + (((ram) & SRAM_SIZE_MASK) << 5))

// bit-bang addr 
#define PWR_CR_BB_ADDR			(PERIPH_BB_BASE + ((PWR_BASE - PERIPH_BASE) << 5) + 32) // bit 8 of CR reg bitbang addr

// Bit definition
#define __BIT(n)					(1ul << (n))

// NULL pointer
#ifndef NULL
#define NULL						((void *)0)
#endif

// Array size
#define ARRAY_SIZE(arr)			(sizeof(arr) / sizeof((arr)[0]))

#define IPC_FLAG1				0x4e494843 /*NIHC*/
#define IPC_FLAG2				0x53504741 /*SPGA*/
#define IPC_CHKSUM				(IPC_FLAG1 ^ IPC_FLAG2)

// max msg para count
#define MSG_MAX_PARA			0xff//0x80原始值 通讯规则规定len只能一个字节表示  建议勿修改

// upgrading type
#define UPD_TYPE_BOOT			0x00
#define UPD_TYPE_APP			0x01
// upgrading dir
#define UPD_FROM_ARM			0x00
#define UPD_FROM_MCU			0x01

// for delay poweroff
#define DELAY_PWRONFF_CNT		201 /* unit: 100ms */

// for systick timer, if changed, SysTick_Handler() need modify
#define SYSTICK_DIV				100 // 10ms interval
#define SYSTICK_CNT_MAX			12000 // local_counter max

enum
{
	RVS_SCREEN_NO_IMAGE = 0, //不镜像
	RVS_SCREEN_IMAGE_LR, //左右镜像
	RVS_SCREEN_IMAGE_UD, //上下镜像
	RVS_SCREEN_IMAGE_LRUD //左右上下镜像
};

enum
{
	RVS_LANGUAGE_ENGLISH = 0,  //英语
	RVS_LANGUAGE_CHINESE_SIMPLIFIED, //中文简体
	RVS_LANGUAGE_CHINESE_TRADITIONAL //中文繁体
};

typedef union union16
{
	u8 b[2];
	u16 h;
}union16_t;

typedef union union32
{
	u8 b[4];
	u16 h[2];
	u32 w;
}union32_t;

typedef union union64
{
	u8 b[8];
	u16 h[4];
	u32 w[2];
	uint64_t d;
}union64_t; 
	
typedef struct clock_info
{
	u16 pll_on; // OFF / ON
	u16 hsi_on; // OFF / ON
	u16 hse_on; // OFF / ON
	u16 pad; // for word align
	u32 sys_clk; // system clock, SYSCLK, in Hz
	u32 hclk; // for AHB bus/CORE/MEMORY/DMA, 72MHz max
	u32 pclk1; // for APB1 peripherals, 36MHz max
	u32 timxclk1; // for timer2/3/4/5/6/7/12/13/14, PCLK1 & APB1 prescaler
	u32 pclk2; // for APB2 peripherals, 72MHz max
	u32 timxclk2; // for timer1/8/9/10/11, 72MHz max, PCLK2 & APB2 prescaler
	u32 adcclk; // ADC clock, 14MHz max
}clock_info_t;

typedef struct ipc_info
{
	// 3 words, fixed data for cold-boot checking
	u32 flag1;
	u32 flag2;
	u32 checksum; // flag1 ^ flag2

	u32 in_boot; // 1: running in boot, 0: running in app

	u32 big_endian; // 1: big endian, 0: little endian
	u32 boot_stack;
	u32 boot_entry;
	u32 boot_version;
	u32 app_stack;
	u32 app_entry;
	//u32 app_version;
	u32 cold_boot;
	u32 upgrade_mcu_boot; // 
	u32 app_jump; // 1: jump from App(request upgrade), 0: normal
	//u32 rodata_base; // read only data base addr
	u32 arm_app_ver;
	u32 arm_ari_ver;
	u32 heart_count;
	u32 stopmode_awake_type_state;//0初值  1 ACC  2  CAN
	u32 work_state;//1 休眠  2工作 3 500ms无心跳
	u32 avm_disviewsts;
	u32 dtc_arm_com_sate;//
	u32 dtc_camera_state;
	u32 usart_normal;
	u32 miss_avm0;//
	u32 miss_avm1;//missing  0x316 bcm 40ms*10
	u32 miss_avm2;//missing  0x278 bcm 10ms*10
	u32 miss_avm3;//missing  0x116 bcm 10ms*10
	u32 miss_avm4;//missing  0x1f0 bcm 10ms*10
	u32 can_wait_count;
	u16 stop_module_flag;//进入停机模式标志位 1  
	
	u32 autotest_flag;
  u16 autotest_voltage;
  u16 autotest_MCU_SLEEP_flag;   //0 正常  1 MCU立即进入睡眠状态
}ipc_info_t;

typedef struct ctrl_info
{

	u8 state; // system state
	u8 acc_line; // system state
	u8 b_line; // system state
	u8 host_ready; // default: 0, for WAKEUP stage back to mode usage
	u8 reverse_on; // 0/1, select color[x]
	u8 upgrade_type; // upgrade type
	u8 carcan_wake;
	u8 wait_canMsg_time;  //state=ACCOFf 开启10s计时，等于1时到达 等于2  开启3s等待状态
	u8 wait_arm_keep_data;//3s  等待arm keep data  0 正常 1 开启3s定时  2 3s定时到
	u8 wait_can_ack_normal;//150ms rx_ack
	u8 wait_can_revovery;//150ms rx_recovery
	u8 can_no_tx;//禁止发送报文 标志位 1禁止发送 
	u8 BusOffQuick;//busoffquick计时标志位
	u8 BusOffSlow;//busoffslow计时标志位
	u8 arm_sleep;//1 表示ARM与MCU  15s未通讯上，
	u8 write_flash_time_flag;// 写flash记录标志
	u8 flame_miss_dtc;//可以记录dtc，1不记录报文超时dtc
	u8 write_dtc_flag;
	u8 write_did_flag;
}ctrl_info_t;

//ADC struct
typedef struct adc_info
{
//	u16 adc_times;
	uint16_t low_keepCount ;	
  	uint16_t hight_keepCount ;
	float main_pwr_volt;//100次求一个均值（1s内）
}adc_info_t;


//diag par struct
typedef struct diag_info
{
	u8  diag_rec_sts;
	u8  cf_time_flag;
	u32 cf_last_time;
	u32 cf_curret_time;
}diag_info_t;

//diag par struct
typedef struct diag_dtc
{
	u8  record_bus_dtc_flag;//can_bus_dtc switch
	u8  votlage_dtc_flag;//标志KL15 ON >2s 可以检测电压dtc
	u8  busoff_dtc_flag;//连续3次未恢复就记录busoff dtc
}diag_dtc_t;

//diag par struct
typedef struct diag_did
{
	u8	flag;//标志KL15 ON >2s 可以检测电压dtc
	u32  ADDR;//
	u8  LEN;//
	u8  buf[20];
}diag_did_t;


typedef struct device_info
{
	u32 flags[4]; //each bit represent a flag, max 4 * 32 = 128 flags
	u32 *bitbang_addr; // bit-bang addr of member 'flags'
	u32 debug_uart; // enum, the debug UART device
	u32 comm_uart; // enum, the UART comm with host
	clock_info_t clock;
	ctrl_info_t *ci;
	ipc_info_t *ipc;
	adc_info_t adc_t;
	diag_info_t diag_t;
	diag_dtc_t  dtc_t;
	diag_did_t  did_t;
}device_info_t;

// generic message struct
typedef struct message
{
	u8 dir;
	u8 id; // message id, cmd or req
	union32_t hid; // host msg id, android app id in T8
	u8 par_cnt;
	union
	{
		u8 b[MSG_MAX_PARA];
		u16 h[MSG_MAX_PARA >> 1];
		u32 w[MSG_MAX_PARA >> 2];
		uint64_t d[MSG_MAX_PARA >> 3];
	}par;
}message_t;

// i2c struct
typedef union i2c_access
{
	u16 combine;
	struct
	{
		u8 addr;
		u8 value;
	}info;
}i2c_access_t;

// for upgrading
typedef struct blk_info
{
	u32 base;
	u8 *dat;
}blk_info_t;

// for GPIO state save
typedef struct gpio_info
{
	u32 MODER;
	u32 OTYPER;
	u32 PUPDR;
}gpio_info_t;

// for multi-data-segment msg
typedef struct data_info
{
	u8 *data;
	u32 len;
}data_info_t;

/******************* Timer based on Systick[unit: 10ms] *******************/
#define STIMER_CNT				(FLAG_STIMER_END - FLAG_STIMER_START)//4

#define stimer_restart(dev, flag, count)		stimer_start(dev, flag, count)

typedef struct soft_timer
{
	u32 flag;
	u32 timeout;
}soft_timer_t;

#define _TO_CNT_10MS		((u32)1) // base unit
#define _TO_CNT_1S			(100 * _TO_CNT_10MS)
#define _TO_CNT_1MIN		(60 * _TO_CNT_1S)
#define _TO_CNT_1HOUR		(60 * _TO_CNT_1MIN)

#define TO_CNT_20MS		(2 * _TO_CNT_10MS)
#define TO_CNT_30MS		(3 * _TO_CNT_10MS)
#define TO_CNT_40MS		(4 * _TO_CNT_10MS)
#define TO_CNT_50MS		(5 * _TO_CNT_10MS)
#define TO_CNT_80MS		(8 * _TO_CNT_10MS)
#define TO_CNT_100MS		(10 * _TO_CNT_10MS)
#define TO_CNT_120MS		(12 * _TO_CNT_10MS)
#define TO_CNT_150MS		(15 * _TO_CNT_10MS)
#define TO_CNT_200MS		(20 * _TO_CNT_10MS)
#define TO_CNT_280MS		(28 * _TO_CNT_10MS)
#define TO_CNT_300MS		(30 * _TO_CNT_10MS)
#define TO_CNT_400MS		(40 * _TO_CNT_10MS)
#define TO_CNT_500MS		(50 * _TO_CNT_10MS)

#define TO_CNT_1S			_TO_CNT_1S
#define TO_CNT_1500MS		(150 * _TO_CNT_10MS)
#define TO_CNT_2S			(2 * TO_CNT_1S)
#define TO_CNT_3S			(3 * TO_CNT_1S)
#define TO_CNT_4S			(4 * TO_CNT_1S)
#define TO_CNT_5S			(5 * TO_CNT_1S)
#define TO_CNT_6S			(6 * TO_CNT_1S)
#define TO_CNT_7S			(7 * TO_CNT_1S)
#define TO_CNT_10S			(10 * _TO_CNT_1S)
#define TO_CNT_20S			(20 * _TO_CNT_1S)
#define TO_CNT_30S			(30 * _TO_CNT_1S)
#define TO_CNT_40S			(40 * _TO_CNT_1S)
#define TO_CNT_50S			(50 * _TO_CNT_1S)

#define TO_CNT_1MIN			_TO_CNT_1MIN
#define TO_CNT_9MIN			(9 * _TO_CNT_1MIN)
#define TO_CNT_10MIN		(10 * _TO_CNT_1MIN)
#define TO_CNT_48HR			(48 * _TO_CNT_1HOUR)
#define TO_CNT_72HR			(72 * _TO_CNT_1HOUR)

#define TO_CNT_HREST		TO_CNT_50MS

#define TO_CNT_LCD			TO_CNT_1500MS

#define TO_CNT_SOS			TO_CNT_2S

#define TO_CNT_ACC			TO_CNT_10S
#define TO_CNT_BLINE		TO_CNT_10S

#define TO_CNT_HOST_ACK	TO_CNT_4S
#define TO_CNT_PMIC_ON		TO_CNT_6S

#define TO_CNT_RVSMON		TO_CNT_300MS // RVS ON MSG -> NOTIFY HOST
#define TO_CNT_RVS2LCD		(_TO_CNT_10MS * 150) // NOTIFY HOST -> LCD ON

/*************************************************************/
extern NM_ControlBlockType our_netid;

typedef void (*state_func_t)(device_info_t *dev, message_t *msg, message_t *dmsg);
#endif /* end of __DEV_DEFINE_H__ */

