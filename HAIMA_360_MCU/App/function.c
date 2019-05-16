
#include <can_diag_protocol.h>
#include <usart.h>
#include <api.h>
#include <protocolapi.h>
#include <can.h>
#include "can_did.h"
#include "iflash.h"
#include "timer.h"
#include "can_net_layer.h"
#include "stm32f30x_rcc.h"


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
	0,//write_flash_time_flag
	0,
};


void  get_ecu_ver(device_info_t *dev)
{	
	u8 send_did_data[2]={0};
	STMFLASH_Read(ADDR_ECUSoftwareVersion,(u16*)send_did_data,1);	
	dbg_msgv(dev,"    	---ECU_Soft__Ver %x.%x---\r\n",send_did_data[0],send_did_data[1]);
}

void ipc_init(device_info_t * dev)
{
	dev->ipc = (ipc_info_t *)get_ipc_base();//存放在 SRAM boot和APP跳转 值有效，但需要分开初始化
	dev->ipc->in_boot = 0;
	dev->ipc->heart_count = 0;
	dev->ipc->can_wait_count= 0;
	dev->ipc->dtc_camera_state=0;
	dev->ipc->dtc_arm_com_sate=0;
	dev->ipc->work_state=0;
	dev->ipc->avm_disviewsts=0;
	dev->ipc->usart_normal=0;
	dev->ipc->miss_avm0=1;
	dev->ipc->miss_avm1=1;
	dev->ipc->miss_avm2=1;
	dev->ipc->miss_avm3=1;
	dev->ipc->miss_avm4=1;
	}

static void flag_init(device_info_t * dev)
{
	dev->diag_t.cf_time_flag= 0;
	dev->diag_t.cf_last_time= 0;
	dev->diag_t.cf_curret_time= 0;
}

static void judge_acc_sts(device_info_t *dev)
{
	dev->ci->state = HOST_BOOTING;
 
	start_timer(TIMER_DEV1,1000);
	
	set_flag(dev,FLAG_TIMEOUT_400MS);

	}

void device_init(device_info_t *dev, u32 vect_offset)
{
	u32 addr, i;

	SCB->VTOR = vect_offset;

	ipc_init(dev);

	flag_init(dev);
	
	dev->ci = &cinfo;
	//dev->ipc->app_version = APP_VER_BIN;

	for(i = 0; i <(u32 )ARRAY_SIZE(dev->flags); i++)
	{
		dev->flags[i] = 0;
	}

	addr = (u32)dev->flags;
	dev->bitbang_addr = (u32 *)BITBANG_ADDR(addr);
	
	stimer_init(dev);

	__disable_irq();

	Iwdg_Feed();

	app_gpio_init(dev); // for boot.bin reserve
	usart_device_init(dev);
	
	__enable_irq();
	
	dbg_uart_init(dev);
		
	comm_uart_init(dev);

	acc_line_gpio_init(dev);
	
	can_line_gpio_init(dev);

	car_can_wake_init(dev);

	car_can_init(dev);

		//start NM 
	start_timer(TIMER_DEV7,1);
		
    TalkNM(&our_netid);
    StartNM(&our_netid);

	ACC_EXTI_Init();
	
	adc1_init();
	
	init_net_layer_parameer();
	
	init_diag_protocol_parameer();//??????

	//mdelay(36);
	judge_acc_sts(dev);

}

void confirm_upgrading(device_info_t *dev, u8 type, u8 dir)
{
	u8 buf[2];

	buf[0] = type;
	buf[1] = dir;

	comm_message_send(dev, CM_ARM_UPGRADE, 0, buf, 2);

//	usart_tx_empty_poll(dev, dev->comm_uart); // wait for the first ARM11 sync ACK & upgrade cmdsend out
}


void device_deinit(device_info_t *dev)
{
	comm_uart_deinit(dev);
		
	adc1_deinit();

	car_can_deinit_simple(dev);

	dbg_uart_deinit(dev);

	internal_disableall(dev);

	dev->ci->host_ready=0;
}

#if 0
void device_reinit(device_info_t *dev)
{
		u32 addr, i;
	
		dev->ipc = (ipc_info_t *)get_ipc_base();
		dev->ipc->in_boot = 0;
		dev->ipc->heart_count = 0;
		dev->ipc->can_wait_count=0;
			
		dev->ci = &cinfo;

		for(i = 0; i <(u32 )ARRAY_SIZE(dev->flags); i++)
		{
			dev->flags[i] = 0;
		}

		addr = (u32)dev->flags;
	
		dev->bitbang_addr = (u32 *)BITBANG_ADDR(addr);

		clock_switch(dev,TRUE);
		//dev->ipc->app_version = APP_VER_BIN;
		app_gpio_init(dev); // for boot.bin reserve                	                 
}
#endif

