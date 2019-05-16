
#include <stdio.h>
#include <string.h>
#include <can.h>
#include <usart.h>
#include <api.h>
#include <can_net_layer.h>
#include <timer.h>
#include <version.h>
#include <can_diag_protocol.h>
static device_info_t device;
static RCC_ClocksTypeDef RCC_Clocks;
int main(void)
{	
	message_t msg, *dmsg;
	state_func_t *state_func;
	device_info_t *dev = &device;
	// vector base, uarts, gpio...
	
	device_init(dev, get_boot_base());

	dbg_msgv(dev, "[Bootloader- %d.%d.%d]\r\n", get_boot_version() >> 16,
		(get_boot_version() >> 8) & 0xff, get_boot_version() & 0xff);

	dbg_msgv(dev, "%s\r\n",MCU_BOOT_VER);  //ARM升级boot版本号对比，不能去掉	
	
	if(	dev->ipc->app_jump)
	{	
		init_diag_protocol_parameer_boot();
		dev->ipc->app_jump = 0;
		dev->ci->host_ready =0;//升级的时候强制置1
		dev->ci->state =HOST_RUNNING;
	}
	else
	{
		if(FALSE == app_code_verify(dev))//APP_FLAG标志位错误
		{
			dbg_msg(dev, "\r\nAPP_FLAG is wrong\r\n");
			dev->ci->state = HOST_RUNNING;//升级
			}
		else 	
		{
			//boot_debug_mode(dev);
			goto exit0; // normal boot up
		}
	}


	dmsg = dbg_msg_alloc(dev);
	
	state_func = get_stage_func_array();

	systick_start(dev);
	
	while(1)
	{
		(state_func[dev->ci->state])(dev, &msg, dmsg);
	}

exit0:
	dbg_msgv(dev, "\r\nStarting App from %x, stack top %x...\r\n", dev->ipc->app_entry, dev->ipc->app_stack);

	//systick_stop();

	device_deinit(dev);

	process_switch(dev->ipc->app_entry, dev->ipc->app_stack);

	while(1);
}

device_info_t *get_device_info(void)
{
	return &device;
}

RCC_ClocksTypeDef *get_clocks_info(void)
{
	return &RCC_Clocks;
}
