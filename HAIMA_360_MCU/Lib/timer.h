#ifndef __TIMER_H__
#define __TIMER_H__

#include"miscdef.h"

////////////////// GPIO LINE Related ////////////////

typedef struct line_timer_info
{
	/* Fixed para */
	u32 flag; // bit-banging flag
	u32 gpio_bit;
	IRQn_Type gpio_irq;
	GPIO_TypeDef *gpio;
	const char *name;

	/* Variable para */
	u32 comp_state; // state for compare
	u32 prev_state;
	u32 count;
}line_timer_info_t;


//extern volatile unsigned int T1ms;
extern void TIMER0_IRQHandler(void);
extern void TIMER1_IRQHandler(void);
extern void TIMER2_IRQHandler(void);
extern void TIMER3_IRQHandler(void);
extern void TIMER4_IRQHandler(void);

void acc_line_gpio_init(device_info_t *dev);
void can_line_gpio_init(device_info_t *dev);
void start_timer(u8 TIMx,u16 time1ms);
void stop_timer(u8 TIMx);
void stimer_init(device_info_t *dev);
s32 stimer_start(device_info_t *dev, u32 flag, u32 count);
u32 stimer_stop(device_info_t *dev, u32 flag);
void systick_start(device_info_t *dev);
void systick_restart(void);
void systick_stop(void);
void stimer_stop_all(device_info_t *dev);
void Det_line_msg_pre_init(void);
u32 get_systick_value(void);

#endif 

