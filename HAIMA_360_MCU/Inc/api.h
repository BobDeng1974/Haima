
#ifndef __API_H__
#define __API_H__
#include "miscdef.h"
static u32 __INLINE get_flag(device_info_t *dev, u32 flag) // return 0 OR 1
{
	u32 tmp;

	tmp = dev->bitbang_addr[flag];

	if(tmp)
	{
		dev->bitbang_addr[flag] = 0;
	}

	return tmp;
}

static u32 __INLINE chk_flag(device_info_t *dev, u32 flag)
{
	return dev->bitbang_addr[flag];
}

static void __INLINE set_flag(device_info_t *dev, u32 flag)
{
	dev->bitbang_addr[flag] = 1;
}

static void __INLINE clear_flag(device_info_t *dev, u32 flag)
{
	dev->bitbang_addr[flag] = 0;
}

device_info_t *get_device_info(void);
RCC_ClocksTypeDef *get_clocks_info(void);


void device_init(device_info_t *dev, u32 vect_offset);
void device_init_boot(device_info_t *dev, u32 vect_offset);
void device_deinit(device_info_t *dev);
void device_reinit(device_info_t *dev);

void upgrading_process(device_info_t *dev, message_t *dmsg);

state_func_t *get_stage_func_array(void);

void adc1_init(void);
void adc1_deinit(void);
void systick_start(device_info_t *dev);
void systick_stop(void);
void systick_restart(void);

void stimer_init(device_info_t *dev);
s32 stimer_start(device_info_t *dev, u32 flag, u32 count);
u32 stimer_stop(device_info_t *dev, u32 flag);
void stimer_stop_all(device_info_t *dev);
void stop_mode_proc(device_info_t *dev);
void boot_gpio_init(device_info_t *dev);
void app_gpio_init(device_info_t * dev);
u8 GetACCStatus(void);

void iflash_lock(void);
void iflash_unlock(void);
void iflash_flag_clear(void);
u32 iflash_page_erase(u32 page_addr);
u32 iflash_word_program(u32 addr,u32 dat);
u32 iflash_halfword_program(u32 addr,u16 dat);
void iflash_operation_result(device_info_t * dev,u32 ret);
void app_code_invalidate(device_info_t * dev, u8 *mt, u8 *uuid);
bool app_code_verify(device_info_t * dev);
bool app_code_mark(device_info_t *dev, u8 *mt, u8 *uuid);

void Enter_stopmode(void);
void ACC_EXTI_Init(void);
void CAN_RX_EXTI_Init(void);

void peripheral_power_ctrl(u32 on);
void boot_gpio_init(device_info_t *dev);
void app_gpio_init(device_info_t * dev);


void watchdog_start(device_info_t * dev);
void watchdog_feed(device_info_t * dev);
u32 get_ipc_base(void);
u32 get_boot_base(void);
u32 get_app_base(void);
u32 get_boot_version(void);
u32 get_app_version(void);
void get_mach_type(u8 *buf);
void get_uuid(u8 *buf);
void update_uuid(u8 *uuid);
void mdelay(u32 ms);
void udelay(u32 us);
void software_reset(void);
void process_switch(u32 entry, u32 stack);
void dump_post_message(device_info_t *dev);
void boot_debug_mode(device_info_t *dev);
void Iwdg_Feed(void);
void Iwdg_Init(u8 pre,u16 rlr);//1,4096  819ms
void clock_switch(device_info_t *dev, bool high_speed);

void device_low_power_enter(void);
void device_low_power_leave(void);
void confirm_upgrading(device_info_t * dev,u8 type,u8 dir);
//void upgrading_process(device_info_t *dev, message_t *dmsg);


//void sys_halt(device_info_t *dev, u32 ms);

//const char *get_car_type_string(void);

u32 systick_debug(device_info_t * dev);
void device_low_power_enter(void);
void endian_exchg16(union16_t * un);
void endian_exchg32(union32_t * un);
void endian_exchg64(union64_t * un);
void power_on_gpio_ctrl(void);
void item_test_init(void);
u32 item_test_operation(device_info_t *dev, message_t *msg);
 void stop_mode_proc(device_info_t *dev);
void Iwdg_Feed(void);
void Iwdg_Init(u8 pre,u16 rlr);//1,4096  819ms
//adc
void Batt_adc_message_process(device_info_t * dev);
void batt_adc_init(device_info_t *dev);
void batt_adc_deinit(device_info_t *dev);
void internal_disableall(device_info_t *dev);
void internal_ableall(device_info_t *dev);
void  get_mcu_ver(device_info_t *dev);
 void Iwdg_Init(u8 pre,u16 rlr);
 void Iwdg_Feed(void);
 void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group);
 void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group);
 void  get_mcu_ver(device_info_t *dev);
 void  get_ecu_ver(device_info_t *dev);


void IO_INIT_I_AN(GPIO_TypeDef* GPIOx, u8 bit);

void IO_INIT_I(GPIO_TypeDef* GPIOx, u8 bit, u8 io_config);

void IO_INIT_O_OUT(GPIO_TypeDef* GPIOx, u16 bit, u8 io_config,u8 state);

void  IO_INIT_AF(GPIO_TypeDef* GPIOx, u16 bit, u32 GPIO_AF);
void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group)	 ;
u8 get_main_pwr_volt(void);

#endif/*end of __API_H__ */

