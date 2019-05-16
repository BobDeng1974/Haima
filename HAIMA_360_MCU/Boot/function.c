

#include <usart.h>
#include <api.h>
#include <protocolapi.h>
#include <can.h>
#include <diag_upgrade.h>
#include <can_diag_protocol.h>
#include <timer.h>
#include <iflash.h>

static ctrl_info_t cinfo =
{
	0, // host state, will change
	0, // host not ready at default
	0, // revers state, will change
	0, // upgrade type, will set before call
	0, // canwake_init
	0, // upgrade type, will set before call
	0, // canwake_init
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};

static void ipc_init(device_info_t *dev)
{
	ipc_info_t *ipc;
	u32 boot_base, app_base, endian = 1;

	ipc = (ipc_info_t *)get_ipc_base();

	dev->ipc = ipc;

	ipc->in_boot = 1;

	ipc->big_endian = ((u8 *)&endian)[0] ? 0 : 1;

	boot_base = get_boot_base();
	app_base = get_app_base();

	// these 3 may change after download[debug & Hot boot]
	ipc->boot_stack = READ_FLASH32(boot_base);
	ipc->boot_entry = READ_FLASH32(boot_base + 0x04);
	ipc->boot_version = get_boot_version();

	// these 4 may change after App upgrade or in debug mode
	// app stack & entry must update after App change
	ipc->app_stack = READ_FLASH32(app_base);
	ipc->app_entry = READ_FLASH32(app_base + 0x04);
	//ipc->app_version = 0; // always update this filed in App
	//ipc->rodata_base = get_rodata_base(ipc->app_entry);

	if((IPC_FLAG1 == ipc->flag1) && (IPC_FLAG2 == ipc->flag2) && (IPC_CHKSUM == ipc->checksum))
	{
		ipc->cold_boot = 0;
		return;
	}

	ipc->cold_boot = 1;
	ipc->flag1 = IPC_FLAG1;
	ipc->flag2 = IPC_FLAG2;
	ipc->checksum = IPC_CHKSUM;
	ipc->app_jump = 0;
	ipc->usart_normal = 0;
	ipc->autotest_flag=0;
	ipc->autotest_MCU_SLEEP_flag=0;
	ipc->usart_normal=0;
	ipc->stop_module_flag=0;
	ipc->upgrade_mcu_boot=0;
}

u8 get_stop_flag(device_info_t *dev)
{	
	return dev->ipc->stop_module_flag;
	}

u8 judge_iwdog_sts(device_info_t *dev)//返回1 开启  返回0 不开启
{		
	if(get_stop_flag(dev))
	{	
		dev->ipc->stop_module_flag=0;
		
		//dbg_msg(dev,"Ready to enter stop module \r\n");
		
		stop_mode_proc(dev);
		
		return 0;
		}
	else
	{
			//dbg_msg(dev,"Normal power up,open Iwdog and go \r\n");
	  //  Iwdg_Init(2,3072);
		  return 1;
	}	
	}

void device_init(device_info_t *dev, u32 vect_offset)
{
	u32 addr, i;
	dev=get_device_info();

	SCB->VTOR = vect_offset;

	// config SLEEPONEXIT for sleep mode(WFI/WFE)
	// not DEEPSLEEP
	SCB->SCR &= ~(__BIT(2) | __BIT(1));

	dev->ci = &cinfo;

	for(i = 0; i < ARRAY_SIZE(dev->flags); i++)
	{
		dev->flags[i] = 0;
	}

	addr = (u32)dev->flags;
	dev->bitbang_addr = (u32 *)BITBANG_ADDR(addr);

	ipc_init(dev);

	item_test_init();

	stimer_init(dev);

	usart_device_init(dev);

	internal_ableall(dev);

	boot_gpio_init(dev);

	// assume always booting stage
	dev->ci->state = HOST_BOOTING;

	__enable_irq();

	dbg_uart_init(dev); // debug UART initialization

	dbg_msgv(dev, "\r\n--- %s RESET detected ---\r\n", dev->ipc->cold_boot ? "Cold" : "Hot");

	dump_post_message(dev); // Power on self test

	comm_uart_init(dev);

	car_can_wake_init(dev);

	car_can_init(dev);
		
	judge_iwdog_sts(dev);//204ms 内喂狗, 判断是否开启看门狗
	
	iflash_unlock();

	iflash_flag_clear();
	
/* for empty mcu first ,write angin  必须先擦除
	iflash_page_erase(APP_FLAG_ADDR);

	iflash_word_program(APP_FLAG_ADDR, APP_FLAG_CODE);
	
	dbg_msgv(dev,"%x \r\n",READ_FLASH32(APP_FLAG_ADDR));*/
}

void confirm_upgrading(device_info_t *dev, u8 type, u8 dir)
{
	u8 buf[2];

	buf[0] = type;
	buf[1] = dir;

	comm_message_send(dev, CM_ARM_UPGRADE, 0, buf, 2);

//	usart_on_dma_end(dev, dev->comm_uart); // wait for the first ARM11 sync ACK & upgrade cmdsend out
}

void device_deinit(device_info_t *dev) 
{
	//adc1_deinit();	

	comm_uart_deinit(dev);

	car_can_deinit_simple(dev);

	stimer_stop_all(dev);

	dbg_uart_deinit(dev);
	
	

	//RCC_DeInit();

//	internal_disableall(dev);
}



