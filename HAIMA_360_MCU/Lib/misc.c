	
#include <usart.h>
#include <api.h>
#include <protocolapi.h>
#include <version.h>
#include <iflash.h>
/* following called by API in misc.c only */
void asm_delay(u32 tm, u32 div);
void asm_process_switch(u32 entry, u32 stack);

#ifdef CORE_36MHZ
/* All for STM32F101 */
#if (HSE_HZ == 12000000ul)
#define HSE_DIVID				0u /* 0: not divid, 1: divid by 2 */
#define PLL_MULTI				0 /* debug in STM32F101, 36MHz max=12*3 */
#elif (HSE_HZ == 8000000ul)
#define HSE_DIVID				1 /* 0: not divid, 1: divid by 2 */
#define PLL_MULTI				6 /* debug in STM32F101, 36MHz max=4*9 */
#else
#error HSE_HZ define error!
#endif
#define ADC_DIVID				1 /* 0: /2, 1: /4 */
#define APB1_DIVD				0 /* no need divid for 36MHz SYSCLK, divided by 2 for 72MHz SYSCLK */
#define PREF_CNT				1 /* 0=zero wait(0~24MHz), 1=one wait(24~48MHz), 2=2 wait(48~72MHz) */
#elif defined(CORE_72MHZ)
/* All for STM32F103 */
#if (HSE_HZ == 12000000ul)
#define HSE_DIVID				0 /* 0: not divid, 1: divid by 2 */
#define PLL_MULTI				3 /* debug in STM32F101, 72MHz max=12*6 */
#elif (HSE_HZ == 8000000ul)
#define HSE_DIVID				0 /* 0: not divid, 1: divid by 2 */
#define PLL_MULTI				6 /* debug in STM32F101, 72MHz max=8*9 */
#else
#error HSE_HZ define error!
#endif
#define ADC_DIVID				2 /* 0: /2, 1: /4, 2: /6 */
#define APB1_DIVD				4 /* no need divid for 36MHz SYSCLK, divided by 2 for 72MHz SYSCLK */
#define PREF_CNT				2 /* 0=zero wait(0~24MHz), 1=one wait(24~48MHz), 2=2 wait(48~72MHz) */
#else
#error chip select error!
#endif

// low speed mode, AHB clock source using HSE, not HSI, for uart/can applications in sleep mode
// [0 ~ 7]: not divid, [8/9/10/11/12/13/14/15]: div by [2/4/8/16/64/128/256/512]
#if (0 == ENABLE_DEBUG_UART) //((0 == ENABLE_UART_LP) && (0 == ENABLE_CAN_LP))
#if (HSE_HZ == 12000000ul)
#define AHB_DIVD				10 /* divid by 8 */
#define AHB_DIVD_SHIFT			3  /* according to AHB_DIVD */
#elif (HSE_HZ == 8000000ul)  /* !!!!!! NOT TEST !!!!!!!! */
#define AHB_DIVD				10 /* divid by 8 */
#define AHB_DIVD_SHIFT			3  /* according to AHB_DIVD */
#else
#error HSE_HZ define error!
#endif
#else // for uart(debug OR 3G), too low AHB may cause uart abnormal
#if (HSE_HZ == 12000000ul)
#define AHB_DIVD				9 /* divid by 4 */
#define AHB_DIVD_SHIFT			2  /* according to AHB_DIVD */
#elif (HSE_HZ == 8000000ul)  /* !!!!!! NOT TEST !!!!!!!! */
#define AHB_DIVD				8 /* divid by 2 */
#define AHB_DIVD_SHIFT			1  /* according to AHB_DIVD */
#else
#error HSE_HZ define error!
#endif		
#endif

/* USB clk div keeps 1.5, for low-speed mode it's not used, for STM32F101, no USB */
void clock_switch(device_info_t *dev, bool high_speed)
{
	clock_info_t *clk;
	u32 cr, cfg, ir;
	RCC_TypeDef *rcc = RCC;

	if(TRUE == high_speed)
	{
		cfg = rcc->CFGR & 0x0c;
		
		if(0x08 ^ cfg) // check if clock PLL has been used as SYSCLK
		{
			// HSI ON
			cr = rcc->CR;
			cr |= (1 << 0);
			rcc->CR = cr;

			// wait for HSI clock stable
			while(!((1 << 1) & rcc->CR));
		
			// select HSI output as SYSCLK, not PLL
			cfg = rcc->CFGR;
			cfg &= ~0x03;
			rcc->CFGR = cfg;
			while(0x00 ^ (0x0c & rcc->CFGR)); // wait for clock switch stable
	
			/* Enable Prefetch Buffer, 1 or 2 wait cycle(wait state), disable half-cycle */
			cr = FLASH->ACR;
			cr &= ~0x0f;
			cr |= (PREF_CNT | __BIT(4));
			FLASH->ACR = cr;

			// turn off HSE & PLL
			cr = rcc->CR;
			cr &= ~((1 << 16) | (1 << 24));
			rcc->CR = cr;

			// wait for HSE clock off
			while(((1 << 17) & rcc->CR));

			// make sure PLL is OFF for the following REG setting
			cfg = rcc->CFGR;
			cfg &= ~(1 << 17); // HSE clk is not divided
			cfg = (HSE_DIVID << 17);
			cfg &= ~(0x0f << 18);
			cfg |= ((1 + PLL_MULTI) << 18); // pll multiply: x6, 12M HSE osc * 6 = 72M max for STM32F103
							// pll multiply: x3, 12M HSE osc * 3 = 36M max for STM32F101

			cfg |= (1 << 16); // clk from PREDIV1 as PLL in
			cfg &= ~(0x0f << 4); // AHB prescaler = 1, AHB clk is not divided, HCLK = SYSCLK
			cfg &= ~(7 << 8);
			cfg |= (APB1_DIVD << 8); // PCLK1 = HCLK / x; APB1 pre-scaler
			cfg &= ~(7 << 11); // PCLK2 = HCLK, APB2 pre-scaler
			cfg &= ~(3 << 14);
			cfg |= (ADC_DIVID << 14); // ADC CLK = PCLK2 / x, 14MHz max
			rcc->CFGR = cfg;

			// HSE ON
			cr = rcc->CR;
			cr |= (1 << 16);
			rcc->CR = cr;

			// wait for HSE clock stable
			while(!((1 << 17) & rcc->CR));

			// PLL ON
			cr = rcc->CR;
			cr |= (1 << 24);
			rcc->CR = cr;

			// wait for PLL locked
			while(!((1 << 25) & rcc->CR));

			// select PLL output as SYSCLK
			cfg = rcc->CFGR;
			cfg &= ~3;
			cfg |= 0x02;
			rcc->CFGR = cfg;

			while(0x08 ^ (0x0c & rcc->CFGR)); // wait for clock switch stable

			#if 0 // flash programming use HSI
			// disable HSI
			cr = rcc->CR;
			cr &= ~(1 << 0);
			rcc->CR = cr;

			// wait until HSI is OFF
			while((1 << 1) & rcc->CR);
			#endif
		
			ir = rcc->CIR;
			ir |= (3 << 18); // clear HSE & HSI flag
			rcc->CIR = ir;
		}
		
		// fill the global clock structure
		clk = &dev->clock;
		
		clk->pll_on = ON;
		clk->hse_on = ON;
		clk->hsi_on = ON;
		clk->sys_clk = (HSE_HZ >> HSE_DIVID) * (3 + PLL_MULTI);
		clk->hclk = clk->sys_clk;
		clk->pclk1 = clk->hclk >> (APB1_DIVD >> 2); // PCLK1 = HCKL / x, 36MHz max
		clk->timxclk1 = clk->pclk1 << (APB1_DIVD >> 2); // if APB1 prescaler = 2, then 'x 2', else 'x1'
		clk->pclk2 = clk->hclk; // PCLK2 = HCLK, 72MHz max
		clk->timxclk2 = clk->pclk2; // APB2 prescaler = 1, so 'x1'
		clk->adcclk = clk->pclk2 / ((ADC_DIVID + 1) << 1); // 14MHz max
	}
	else
	{
		cfg = rcc->CFGR & 0x0c;

		if(0x04 ^ cfg) // check if clock HSE has been selected as SYSCLK
		{
			// HSI ON
			cr = rcc->CR;
			cr |= (1 << 0);
			rcc->CR = cr;

			// wait for HSI clock stable
			while(!((1 << 1) & rcc->CR));
		
			// select HSI output as SYSCLK, not PLL
			cfg = rcc->CFGR;
			cfg &= ~0x03;
			rcc->CFGR = cfg;
			while(0x00 ^ (0x0c & rcc->CFGR)); // wait for clock switch stable

			cr = FLASH->ACR;
			/* Use latency 0, Enable Prefetch Buffer, Enable Half cycle(as CLK from HSE) , Page 59 */
			cr &= ~0x0f;
			cr |= (FLASH_ACR_HLFCYA | __BIT(4));
			FLASH->ACR = cr;

			// turn off HSE & PLL
			cr = rcc->CR;
			cr &= ~((1 << 16) | (1 << 24));
			rcc->CR = cr;

			// wait for HSE clock off
			while(((1 << 17) & rcc->CR));

			// after HSI is SYSCLK and PLL is off, then config following
			cfg = rcc->CFGR;
			cfg &= ~(0x0f << 4);
			cfg |= (AHB_DIVD << 4); // AHB divid by x, SYSCLK = HSE, HCLK = HSE / y
			cfg &= ~(0x07 << 11); // APB2 is not divided
			cfg &= ~(3 << 14); // ADC CLK  div by 2, max
			cfg &= ~(7 << 8); // PCLK1 = HCLK; APB1 pre-scaler, not divided
			rcc->CFGR = cfg;

			// HSE ON
			cr = rcc->CR;
			cr |= (1 << 16);
			rcc->CR = cr;

			// wait for HSE clock stable
			while(!((1 << 17) & rcc->CR));

			// select HSE output as SYSCLK
			cfg = rcc->CFGR;
			cfg &= ~0x03;
			cfg |= 0x01;
			rcc->CFGR = cfg;
		
			while(0x04 ^ (0x0c & rcc->CFGR)); // wait for clock switch stable

			// disable HSI
			cr = rcc->CR;
			cr &= ~(1 << 0);
			rcc->CR = cr;

			// wait until HSI is OFF
			while((1 << 1) & rcc->CR);
		
			ir = rcc->CIR;
			ir |= (3 << 18); // clear HSE & HSI flag
			rcc->CIR = ir;
		}

		// fill the global clock structure
		clk = &dev->clock;

		clk->pll_on = OFF;
		clk->hse_on = ON;
		clk->hsi_on = OFF;
		clk->sys_clk = (u32)HSE_HZ;
		clk->hclk = clk->sys_clk >> AHB_DIVD_SHIFT; // AHB divider is x
		clk->pclk1 = clk->hclk; // PCLK1 = HCKL, 36MHz max
		clk->timxclk1 = clk->pclk1; // APB1 prescaler = 1, so 'x 1'
		clk->pclk2 = clk->hclk; // PCLK2 = HCLK, 72MHz max
		clk->timxclk2 = clk->pclk2; // APB2 prescaler = 1, so 'x1'
		clk->adcclk = clk->pclk2 >> 1; // 14MHz max
	}
}

#if 0
void led_init(void)
{
#if ENABLE_LED_CTRL
	u32 reg;
	RCC_TypeDef *rcc = RCC;

	// enable GPE, led use GPE6
	reg = rcc->APB2ENR;
	reg |= __BIT(6);// | __BIT(0);
	rcc->APB2ENR = reg;

	IO_INIT_O_PP(GPIOE, 6, HIGH); // LED OFF == HIGH
#endif
}

void led_ctrl(u32 on)
{
#if ENABLE_LED_CTRL	
	IO_SET(GPIOE, 6, !on);
#endif
}
#endif

void process_switch(u32 entry, u32 stack)
{
	__disable_irq();

	asm_process_switch(entry, stack);
}

void software_reset(void)
{
	__disable_irq();

	__set_FAULTMASK(1);
	__DSB();
	SCB->AIRCR = (0x05FAul << 16) | __BIT(2);
	//SCB->AIRCR = ((0x5FA << SCB_AIRCR_VECTKEY_Pos) |(SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk)|SCB_AIRCR_SYSRESETREQ_Msk);

	__DSB();

	while(1);
}


void HardFault_Handler(void)
{
	//	software_reset();
  while (1);
}
#if 0
void software_reexecute(u32 type)
{
	u32 base, entry, stack;

	if(UPD_TYPE_APP == type)
	{
		base = get_app_base();
	}
	else
	{
		base = get_boot_base();
	}

	entry = READ_FLASH32(base);
	stack = READ_FLASH32(base + 0x04);

	process_switch(entry, stack);

	__DSB();

	while(1);
}
#endif

/////////// Watchdog part //////////////////

#if ENABLE_EXTERNAL_WDG
static u32 wdg_state = 0;
#endif

void __watchdog_feed(void)
{
#if ENABLE_INTERNAL_IWDG
	IWDG_TypeDef *iwdg = IWDG;
#endif

#if ENABLE_EXTERNAL_WDG
	wdg_state = !wdg_state;
	IO_SET(GPIOE, 4, wdg_state);
#endif

#if ENABLE_INTERNAL_IWDG
	iwdg->KR = 0xaaaau; // disabel & reload the wdg first

	iwdg->KR = 0x5555u; // config PR & RL
	while(iwdg->SR & __BIT(0)); // PVU bit
	iwdg->PR = 1; // div by 8, 0.2ms/clk @ 40KHz
	while(iwdg->SR & __BIT(1)); // RVU bit
	iwdg->RLR = 4095; // 12-bit counter,Tout = (4*2^prv) / 40 * rlv (s)=8*4096/4=819ms
	
	while(iwdg->SR & 0x03); // updating ongoing, not necessary for sw waiting
	iwdg->KR = 0xcccc; // start the watchdog
#endif

#if ENABLE_INTERNAL_WWDG
	WWDG->CR = __BIT(7) | 126; // interval: 126 - 64[0x40] = 62, 62 * 0.91 = 56.4ms
#endif
}

void watchdog_gpio_init(void)
{
#if ENABLE_EXTERNAL_WDG
	// use GPE4, push-pull output
	IO_INIT_O_PP(GPIOE, 4, LOW);
	wdg_state = LOW; // according to IO_INIT_O_PP()
#endif
}

void watchdog_start(device_info_t *dev)
{
#if ENABLE_EXTERNAL_WDG
	// enable GPIOE clk
	RCC->APB2ENR |= __BIT(6);

	watchdog_gpio_init();
	udelay(5000); // wait IO stable
#endif

#if ENABLE_INTERNAL_IWDG
	RCC->CIR &= ~__BIT(8); // disable LSI interrupt
	RCC->CSR |= __BIT(0); // enable LSI clock, 40KHz, range: 30~60KHz
	while(!(RCC->CSR & __BIT(1))); // wait LSI clock stable
#endif

#if ENABLE_INTERNAL_WWDG
	// enable clock & reset
	RCC->APB1ENR |= __BIT(11);
	RCC->APB1RSTR |= __BIT(11);
	RCC->APB1RSTR &= ~__BIT(11);

	WWDG->CFR = (3u << 7) | 0x7f; // ((PCLK1 / 4096) / 8) = 0.91ms/(dec 1), high window limit is 127(max)
#endif

	__watchdog_feed();
}

u32 get_ipc_base(void)
{
	return RAM_IPC_BASE;
}

//////////// Code base //////////////
u32 get_boot_base(void)
{
	return BOOT_CODE_BASE;
}

u32 get_app_base(void)
{
	return APP_CODE_BASE;
}

u32 get_boot_version(void)
{
	return BOOT_VER_BIN;
}
u32 get_app_version(void)
{
	return APP_VER_BIN;
}

// Flag page config
// Byte  0          4             8              12            16                  -> 31
//         A5A5     0000        mtype[0...7]             uuid[0...15]
void get_mach_type(u8 *buf)
{
	u32 *p = (u32 *)buf;
	
	*p++ = READ_FLASH32(APP_FLAG_ADDR + 8);
	*p++ = READ_FLASH32(APP_FLAG_ADDR + 12);
}

void get_uuid(u8 *buf)
{
	u32 *p = (u32 *)buf, i = 0;

	for(i = 0; i < UUID_BYTES; i += 4)
	{
		*p++ = READ_FLASH32(APP_FLAG_ADDR + 16 + i);
	}
}

void update_uuid(u8 *uuid)
{
	u32 buf[10], i, *p = (u32 *)buf;

	for(i = 0; i < (UUID_BYTES + 16); i += 4)
	{
		*p++ = READ_FLASH32(APP_FLAG_ADDR + i);
	}

	p = (u32 *)buf;

	if(!memcmp(buf + 4, uuid, UUID_BYTES))
	{
		dbg_msg(get_device_info(), "uuid is the same, flash not update\r\n");
		
		return;
	}

	memcpy(buf + 4, uuid, UUID_BYTES);

	iflash_page_erase(APP_FLAG_ADDR);

	for(i = 0; i < (UUID_BYTES + 16); i += 4)
	{
		iflash_word_program(APP_FLAG_ADDR + i, *p++);
	}
}

void app_code_invalidate(device_info_t * dev, u8 *mt, u8 *uuid)
{
	u32 i, *p;

	p = (u32 *)mt;
	*p++ = READ_FLASH32(APP_FLAG_ADDR + 8);
	*p++ = READ_FLASH32(APP_FLAG_ADDR + 12);

	p = (u32 *)uuid;
	for(i = 0; i < UUID_BYTES; i += 4)
	{
		*p++ = READ_FLASH32(APP_FLAG_ADDR + 16 + i);
	}

	iflash_page_erase(APP_FLAG_ADDR);
}

bool app_code_verify(device_info_t *dev)
{
	u32 v[2] = { 0 };
	
	v[0] = READ_FLASH32(APP_FLAG_ADDR);/**/

	dbg_msgv(dev, "\r\nFlag code = 0x%08x(%s)\r\n", v[0], (char *)v);

	if(v[0] ^ APP_FLAG_CODE)
	{
		return FALSE;
	}

	return TRUE;
}

bool app_code_mark(device_info_t *dev, u8 *mt, u8 *uuid)
{
	u32 i = C4_2_U32('0', '0', '0', '0'), *v = (u32 *)mt, *p = (u32 *)uuid;

	iflash_word_program(APP_FLAG_ADDR, APP_FLAG_CODE);
	iflash_word_program(APP_FLAG_ADDR + 4, i);
	iflash_word_program(APP_FLAG_ADDR + 8, *v++);
	iflash_word_program(APP_FLAG_ADDR + 12, *v++);

	for(i = 0; i < UUID_BYTES; i += 4)
	{
		iflash_word_program(APP_FLAG_ADDR + 16 + i, *p++);
	}

	return TRUE;
}

void mdelay(u32 ms)
{
	u32 tm, unit;
	device_info_t *dev = get_device_info();
	unit = 72000000 / 9003;/*精确1ms*/

	while(ms)
	{
		tm = (ms >= 50) ? 50 : ms;
		asm_delay(tm, unit);
		Iwdg_Feed();
		ms -= tm;
	}
}

void udelay(u32 us)
{
	u32 unit;
	device_info_t *dev = get_device_info();

	unit = 72000000 / 6000000;

	asm_delay(us, unit);
}

void dump_post_message(device_info_t *dev)
{
#if ENABLE_DEBUG_UART
	u32 reg, base, core;

	/////// CPUID //////

	core = 0x044AA200ul;
	
	base = HSE_HZ; //(TRUE == dev->clock.hse_on) ? HSE_HZ : HSI_HZ;	
	
	reg = READ_REG32(0xe000ed00); // CPUID, CM3 User Guide

	dbg_msgv(dev, "\r\nCPU = ARM Cortex-M4, r%dp%d @ %d MHz",
		(reg >> 20) & 0x0f, reg & 0x0f, core / 1000 / 1000);

	dbg_msgv(dev, "(%d x %d.%d)\r\n",
		base / 1000 / 1000, core / base,
		((core % base) * 10) / base);
		//////// Flash info ////////

	dbg_msg(dev, "Flash = 128KB, on-chip\r\n");

	//////// SRAM info /////////

	dbg_msgv(dev, "SRAM = %dKB, on-chip\r\n", (int)SRAM_SIZE >> 10);



	//dbg_msgv(dev, dev->ipc->rodata_base ?
	//	"RO DATA found @ 0x%08x\r\n" : "RO DATA NOT FOUND[0x%08x]\r\n", dev->ipc->rodata_base);
#endif
}

void boot_debug_mode(device_info_t *dev)
{
#if ENABLE_CMD_LINE
	u32 i = 0;
	bool recv = FALSE;
	message_t msg = {0};
	
	// check Enter key hit
	while(i++ < 70)
	{		
		mdelay(5);
				
		recv = dbg_uart_msg_process(dev, &msg);
					
		if(TRUE == recv)
		{
			break;
		}
	}
				
	if(recv ^ TRUE) // timeout, boot up
	{
		return;
	}

	systick_start(dev);

	// debug mode, never return
	while(1)
	{
		watchdog_feed(dev);
				
		dbg_uart_msg_process(dev, &msg);
	}
#endif
}

void internal_disableall(device_info_t *dev)
{
#if (1 == ENABLE_DEBUG_UART)
	RCC->AHBENR &= ~__BIT(0); // disable DMA1 clk for UART1/2/3
#endif

}

void internal_ableall(device_info_t *dev)
{
#if (1 == ENABLE_DEBUG_UART)
	RCC->AHBENR &= __BIT(0); // disable DMA1 clk for UART1/2/3
#endif

}


static gpio_info_t gpio[5];

// all gpio set to input, except 3G usart, 3G related port config reserve
void device_low_power_enter(void)
{
	//u32 reg;
	/*模拟输入*/	
	GPIOA->MODER |= 0xFFFFFFFF;
	
	GPIOB->MODER |= 0xFFFFFFFF;

	
	GPIOC->MODER |= 0xFFFFFFFF;

	GPIOD->MODER |= 0xFFFFFFFF;

	GPIOE->MODER |= 0xFFFFFFFF;


	GPIOF->MODER |= 0xFFFFFFFF;
}

void device_low_power_leave(void)
{
GPIOA->MODER=	gpio[0].MODER;
GPIOA->OTYPER=	gpio[0].OTYPER;
GPIOA->PUPDR=gpio[0].PUPDR;
	
GPIOB->MODER=	gpio[0].MODER;
GPIOB->OTYPER=	gpio[0].OTYPER;
GPIOB->PUPDR=	gpio[0].PUPDR;
	
GPIOC->MODER=   gpio[0].MODER;
GPIOC->OTYPER=	gpio[0].OTYPER;
GPIOC->PUPDR=	gpio[0].PUPDR;
	
GPIOD->MODER=	gpio[0].MODER;
GPIOD->OTYPER=	gpio[0].OTYPER;
GPIOD->PUPDR=	gpio[0].PUPDR;
	
GPIOE->MODER=	gpio[0].MODER;
GPIOE->OTYPER=	gpio[0].OTYPER;
GPIOE->PUPDR= 	gpio[0].PUPDR;
}



#define ENDIAN_EXCHG(p)		p[1] ^= p[0]; p[0] ^= p[1]; p[1] ^= p[0] 

void endian_exchg16(union16_t *un)
{
	u8 *b = un->b;

	ENDIAN_EXCHG(b);
}

void endian_exchg32(union32_t *un)
{
	u8 *b = un->b;
	u16 *h = un->h;

	ENDIAN_EXCHG(b);
	b += 2;
	ENDIAN_EXCHG(b);

	ENDIAN_EXCHG(h);
}

void endian_exchg64(union64_t *un)
{
	u8 *b = un->b;
	u16 *h = un->h;
	u32 *w = un->w;

	ENDIAN_EXCHG(b);
	b += 2;
	ENDIAN_EXCHG(b);
	b += 2;
	ENDIAN_EXCHG(b);
	b += 2;
	ENDIAN_EXCHG(b);

	ENDIAN_EXCHG(h);
	h += 2;
	ENDIAN_EXCHG(h);

	ENDIAN_EXCHG(w);
}

///////////// HW ITEM TEST ///////////////

typedef struct
{
	u32 valid; // item test info area valid

	u32 flag[8]; // original APP flag area
	u32 data[ITEM_TEST_ID_MAX + 1][ITEM_TEST_ITEM_SIZE >> 2];
}item_test_t;

static item_test_t it_data;

void item_test_init(void)
{
	u32 *w = it_data.flag;
	
	it_data.valid = (C4_2_U32('V', 'A', 'L', 'D') == READ_FLASH32(APP_FLAG_ADDR + 4));

	memcpy(it_data.flag, (void *)APP_FLAG_ADDR, sizeof(it_data.flag)); // read out APP flag area

	if(APP_FLAG_CODE ^ it_data.flag[0]) // init flag area, for APP upgrading process shutdown problem
	{
		w[0] = APP_FLAG_CODE;
		w[1] = C4_2_U32('0', '0', '0', '0');
		w[2] = C4_2_U32('0', '0', '0', '0');
		w[3] = C4_2_U32('0', '0', '0', '1');
		w[4] = C4_2_U32('0', '0', '0', '0');
		w[5] = C4_2_U32('0', '0', '0', '0');
		w[6] = C4_2_U32('0', '0', '0', '0');
		w[7] = C4_2_U32('0', '0', '0', '1');
	}

	if(it_data.valid)
	{
		memcpy(it_data.data, (void *)(APP_FLAG_ADDR + sizeof(it_data.flag)), sizeof(it_data.data)); // skip APP flag area
	}
	else
	{
		memset(it_data.data, 0, sizeof(it_data.data));
	}


// debug info is not allowed now, dbg_uart_init() is not called!!!
//	dbg_msgv(get_device_info(), "after item_test_init(): %02x%02x%02x%02x\r\n",
//		it_data.flag[0], it_data.flag[1], it_data.flag[2], it_data.flag[3]);
}

u32 item_test_operation(device_info_t *dev, message_t *msg)
{
	u32 i, *w, ret = 0;

	if(NULL == msg) // for App FLAG page restore
	{
		w = it_data.flag;

		//dbg_msgv(dev, "[READ1: %08x]\r\n", w[0]);

		for(i = 0; i < sizeof(it_data.flag); i += 4)
		{
			watchdog_feed(dev);
		
			ret = iflash_word_program(APP_FLAG_ADDR + i, *w++);

			if(FLASH_COMPLETE ^ ret)
			{
				goto exit0;
			}
		}

		w = (u32 *)it_data.data;
	
		for(i = 0; i < sizeof(it_data.data); i += 4)
		{
			watchdog_feed(dev);
		
			ret = iflash_word_program(APP_FLAG_ADDR + sizeof(it_data.flag) + i, *w++);

			if(FLASH_COMPLETE ^ ret)
			{
				goto exit0;
			}
		}

		//dbg_msgv(dev, "[READ2: %08x]\r\n",	READ_FLASH32(APP_FLAG_ADDR));
	}
	else if(msg->par_cnt > 1) // WR
	{
		if(msg->par.b[0] > ITEM_TEST_ID_MAX)
		{
			dbg_msgv(dev, "invalid item test id 0x%x for write, skip\r\n", msg->par.b[0]);

			goto exit0;
		}
	
		memcpy(it_data.data[msg->par.b[0]], msg->par.b, ITEM_TEST_ITEM_SIZE);

		ret = iflash_page_erase(APP_FLAG_ADDR);

		if(FLASH_COMPLETE ^ ret)
		{
			goto exit0;
		}

		w = it_data.flag;
		w[1] = C4_2_U32('V', 'A', 'L', 'D'); // update the mark for item_test_init()

		for(i = 0; i < sizeof(it_data.flag); i += 4)
		{
			watchdog_feed(dev);
			
			ret = iflash_word_program(APP_FLAG_ADDR + i, *w++);

			if(FLASH_COMPLETE ^ ret)
			{
				goto exit0;
			}
		}

		w = (u32 *)it_data.data;
	
		for(i = 0; i < sizeof(it_data.data); i += 4)
		{
			watchdog_feed(dev);
			
			ret = iflash_word_program(APP_FLAG_ADDR + sizeof(it_data.flag) + i, *w++);

			if(FLASH_COMPLETE ^ ret)
			{
				goto exit0;
			}
		}

		dbg_msgv(dev, "item 0x%02x in flash updated!\r\n", msg->par.b[0]);
	}
	else if(1 == msg->par_cnt) // RD
	{
		if(msg->par.b[0] > ITEM_TEST_ID_MAX)
		{
			dbg_msgv(dev, "invalid item test id 0x%x for read, skip\r\n", msg->par.b[0]);

			goto exit0;
		}

		dbg_msgv(dev, "item 0x%02x in flash read out!\r\n", msg->par.b[0]);

		memcpy(msg->par.b, it_data.data[msg->par.b[0]], ITEM_TEST_ITEM_SIZE);

		comm_message_send(dev, CM_ITEM_TEST, msg->hid.w, msg->par.b, ITEM_TEST_ITEM_SIZE);
	}
	else
	{
		dbg_msgv(dev, "invalid item test cmd with par count %d, skip!\r\n", msg->par_cnt);
	}

exit0:
	return ret;
}

//Tout=((4*2^prer)*rlr)/40 ms  prer 为预分频系数
void Iwdg_Init(u8 pre,u16 rlr)//1,4096  819ms

{
	IWDG_TypeDef *iwdg = IWDG;

	iwdg->KR =  0xCCCCu; 

    iwdg->KR = 0x5555u;      //使能对PR RLR寄存器的写操作

    iwdg->PR = pre;     //设置分频数 

    iwdg->RLR = rlr;        //设定重装值

	while(iwdg->SR!=0)

    iwdg->KR =  0xAAAAu;    //装载RLR值到看门狗计数器，即喂狗

    iwdg->KR =  0xCCCCu;    //启动看门狗

}

void Iwdg_Feed(void)
{
	IWDG_TypeDef *iwdg = IWDG;

    iwdg->KR = 0xAAAAu;  //喂狗
}

void watchdog_feed(device_info_t *dev)
{
#if 1
		Iwdg_Feed();
	//}
#endif
}
//保存WWDG计数器的设置值,默认为最大. 

//初始化窗口看门狗 	
//tr   :T[6:0],计数器值 
//wr   :W[6:0],窗口值 
//fprer:分频系数（WDGTB）,仅最低2位有效 
//Fwwdg=PCLK1/(4096*2^fprer). Twwg(窗口时间)4096 *(2^WDGTB)*(WWDG_CFR[6:0] - 0x3F)/36 (us)
	u8 WWDG_CNT=0x7f; 
void WWDG_Init(u8 tr,u8 wr,u8 fprer) 
{   
	u8 WWDG_CNT;
	RCC->APB1ENR|=1<<11; 	//使能wwdg时钟 
	WWDG_CNT=tr&WWDG_CNT;   //初始化WWDG_CNT.     
	WWDG->CFR|=fprer<<7;    //PCLK1/4096再除2^fprer 
	WWDG->CFR&=0XFF80;      
	WWDG->CFR|=wr;     		//设定窗口值      
	WWDG->CR|=WWDG_CNT; 	//设定计数器值 
	WWDG->CR|=1<<7;  		//开启看门狗      
//	MY_NVIC_Init(2,3,WWDG_IRQn,2);//抢占2，子优先级3，组2     
	WWDG->SR=0X00; 			//清除提前唤醒中断标志位 
	WWDG->CFR|=1<<9;        //使能提前唤醒中断 
} 
//重设置WWDG计数器的值 
void WWDG_Set_Counter(u8 cnt) 
{ 
	WWDG->CR =(cnt&WWDG_CNT);//重设置7位计数器 
} 
//窗口看门狗中断服务程序 
void WWDG_IRQHandler(void) 
{      
	WWDG_Set_Counter(WWDG_CNT);//重设窗口看门狗的值!         
	WWDG->SR=0X00;//清除提前唤醒中断标志位  
}

#if 0
void sys_halt(device_info_t *dev, u32 ms)
{
	__disable_irq();

	while(1)
	{
		led_ctrl(ON);
		mdelay(ms);
		led_ctrl(OFF);
		mdelay(ms);
	}
}

const char *get_car_type_string(void)
{
	return "TBOX communicates with UART(Qirui Kaiyi)!";
}
#endif


