#include <stdio.h>
#include <string.h>
#include <Version.h>
#include <miscdef.h>
#include <usart.h>
#include <api.h>
#include <iflash.h>
#include "timer.h"
#include "diag_upgrade.h"
/*
ʹ�ø�������ʱ��Ӧע���ж����ȼ����䡣TIM��EXIT��can��usart��dma
����Ŀʹ��Ĭ�Ϸ��飨δ�����ж����ȼ����飩����ռ���ȼ�16��������Ӧ���ȼ�������ж�Ƕ��	
*/

static device_info_t device;
static RCC_ClocksTypeDef RCC_Clocks;

u32  GetLockCode(void)
{
	u32 CpuID[3];
	CpuID[0]=*(vu32*)(0x1ffff7e8);
	CpuID[1]=*(vu32*)(0x1ffff7ec);
	CpuID[2]=*(vu32*)(0x1ffff7f0);
	return (CpuID[0]>>1)+(CpuID[1]>>2)+(CpuID[2]>>3);
}


int main(void)
{
	message_t msg, *dmsg;
		/*�鿴ϵͳʱ��	*/	
	state_func_t *state_func;
	device_info_t *dev = &device;
	RCC_ClocksTypeDef *sys_clock = &RCC_Clocks;
	RCC_GetClocksFreq(sys_clock);/*ʱ������,��device_initǰ����ܳ�ʼ��ʱ�������������*/
	// vector base, uarts, gpio...
	device_init(dev, get_app_base());

	if(dev->ipc->upgrade_mcu_boot)
	{		
		stop_timer(TIMER_DEV7);
		upgrade_mcu_boot_init(dev);

		}
	
	dbg_msg(dev, "\r\n------------------------------------------------------\r\n");

	dbg_msg(dev, "       Welcome to MCU Command Line Interface \r\n");

  	get_ecu_ver(dev);

	dbg_msgv(dev, "[MCU_APP_VER-%d.%d.%d]\r\n", get_app_version() >> 16,
		(get_app_version() >> 8) & 0xff, get_app_version() & 0xff);

	dbg_msg(dev, "------------------------------------------------------\r\n");
	//dbg_msgv(dev, "LockCode=%x\r\n",GetLockCode());
	dbg_prompt_msg(dev);
	
	dmsg = dbg_msg_alloc(dev);

	state_func = get_stage_func_array();
	
	systick_start(dev);
	
	while(1)
	{
		(state_func[dev->ci->state])(dev, &msg, dmsg);
	}
}

device_info_t *get_device_info(void)
{
	return &device;
}

RCC_ClocksTypeDef *get_clocks_info(void)
{
	return &RCC_Clocks;
}
