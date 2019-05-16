

#include <miscdef.h>
#include <usart.h>
#include <api.h>
#include <can.h>
#include <can_net_layer.h>
#include <can_diag_protocol.h>
#include "timer.h"
#include <stm32f30x_rcc.h>
#include <stm32f30x.h>
/*
TIM1_BRK_IRQHandler
TIM1_UP_IRQHandler
TIM1_TRG_COM_IRQHandler
TIM1_CC_IRQHandler
TIM2_IRQHandler
TIM3_IRQHandler
TIM4_IRQHandler
*/

#define TIMER_INTERVAL			10 /* ms */
#define RECHECK_COUNT			4u
#define SOS_RECHECK_COUNT		300 /* sos key use long press det, 3s */
#define BLINE_RECHECK_COUNT		500 /* B+ uses 5s checking */
#define GPIO_TIMER				TIM2
#define GPIO_TIMER_DEV			TIMER_DEV1



#if 0
static line_timer_info_t FCP_ti =
{
	FLAG_FCP_LINE,
	0,
	EXTI0_IRQn,
	GPIOA,
	"FCP_S",
	0
};

static line_timer_info_t BCP_ti =
{
	FLAG_BCP_LINE,
	1,
	EXTI1_IRQn,
	GPIOA,
	"BCP_S",
	0
};
static line_timer_info_t LCP_ti =
{
	FLAG_LCP_LINE,
	2,
	EXTI2_IRQn,
	GPIOA,
	"LCP_S",
	0
};
static line_timer_info_t RCP_ti =
{
	FLAG_RCP_LINE,
	3,
	EXTI3_IRQn,
	GPIOA,
	"RCP_S",
	0
};


static void xxx_line_msg_init(line_timer_info_t *lti)
{
	lti->count = RECHECK_COUNT;

	set_flag(get_device_info(), lti->flag);

	//NVIC_DisableIRQ(lti->gpio_irq);
	EXTI->IMR &= ~__BIT(lti->gpio_bit);

	if(!GPIO_TIMER->CR1)
	{
		timer_start(GPIO_TIMER_DEV);
	}
}


void FCP_S_init(void)
{
	xxx_line_msg_init(&FCP_ti);
}
void BCP_S_init(void)
{
	xxx_line_msg_init(&BCP_ti);
}
void LCP_S_init(void)
{
	xxx_line_msg_init(&LCP_ti);
}
void RCP_S_init(void)
{
	xxx_line_msg_init(&RCP_ti);
}
static line_timer_info_t *line_ti[] =
{
	&FCP_ti,
	&BCP_ti,
	&LCP_ti,
	&RCP_ti,
	NULL
};





void other_line_msg_pre_init(device_info_t *dev) // can add other GPIOs init code
{
	u32 stat[4], i, v;
	line_timer_info_t *p;

	for(i = 0; (u32)line_ti[i]; i++)
	{
		p = line_ti[i];

		stat[i] = p->gpio->IDR & __BIT(p->gpio_bit);
	}

	mdelay(10);

	for(i = 0; (u32)line_ti[i]; i++)
	{
		p = line_ti[i];

		v = p->gpio->IDR & __BIT(p->gpio_bit);

		if(stat[i] ^ v)
		{
			i = 0;

			stat[i] = v;

			mdelay(10);
		}
	}

	for(i = 0; (u32)line_ti[i]; i++)
	{
		p = line_ti[i];

		p->comp_state = stat[i];
		p->prev_state = stat[i];
		get_flag(dev, p->flag);
		get_flag(dev, p->flag + 1);
		get_flag(dev, p->flag + 2);

		switch(p->flag)
		{

			case FLAG_FCP_LINE:
				if(stat[i] >> p->gpio_bit)
					CAM_Control(FCP_S, LOW);
				break;

			case FLAG_BCP_LINE:
				if(stat[i] >> p->gpio_bit)
					CAM_Control(BCP_S, LOW);
				break;

			case FLAG_LCP_LINE:
				if(stat[i] >> p->gpio_bit)
					CAM_Control(LCP_S, LOW);
				break;

			case FLAG_RCP_LINE:
				if(stat[i] >> p->gpio_bit)
					CAM_Control(RCP_S, LOW);
				break;

			default:
				dbg_msgf(dev, "FLAG mismatch, need check!\r\n");
				while(1);
		}
	}

}

static line_timer_info_t FCP_ti =
{
	FLAG_FCP_LINE,
	0,
	EXTI0_IRQn,
	GPIOA,
	"FCP_S",
	0
};

static line_timer_info_t BCP_ti =
{
	FLAG_BCP_LINE,
	1,
	EXTI1_IRQn,
	GPIOA,
	"BCP_S",
	0
};
static line_timer_info_t LCP_ti =
{
	FLAG_LCP_LINE,
	2,
	EXTI2_TS_IRQn,
	GPIOA,
	"LCP_S",
	0
};
static line_timer_info_t RCP_ti =
{
	FLAG_RCP_LINE,
	3,
	EXTI3_IRQn,
	GPIOA,
	"RCP_S",
	0
};
static line_timer_info_t *line_ti[] =
{
	&FCP_ti,
	&BCP_ti,
	&LCP_ti,
	&RCP_ti,
	NULL
}
	void Det_line_msg_pre_init(void)
{

	u8 i=0;
	while(line_ti[i])
	{
		Det_line_msg_pre_init_one(line_ti[i++]);
	}
}
#endif

static line_timer_info_t acc_line_ti =
{
	FLAG_ACC_LINE,
	IOBIT_ACC, /* 8 -> 12 */
	EXTI9_5_IRQn, /* not use */
	IO_ACC, /* GPIOC -> GPIOA */
	"ACC",
	0
};

static line_timer_info_t can_line_ti =
{
	FLAG_CARCAN,
	IOBIT_CARCAN_WKAEUP, /* 8 -> 12 */
	EXTI15_10_IRQn, /* not use */
	IO_CARCAN_WKAEUP, /* GPIOC -> GPIOA */
	"CAN",
	0
};

void Det_line_msg_pre_init_one(line_timer_info_t *lti)
{
	device_info_t *dev = get_device_info();
	lti->comp_state = __BIT(lti->gpio_bit);
	lti->prev_state = __BIT(lti->gpio_bit);
	get_flag(dev, lti->flag);
	get_flag(dev, lti->flag + 1);
	get_flag(dev, lti->flag + 2);

	// ACC detect method change
	lti->count = RECHECK_COUNT << 1;
 }



//ACC detect init
static void acc_line_msg_pre_init(device_info_t *dev)
{
	ctrl_info_t *ci = dev->ci;
	line_timer_info_t *lti = &acc_line_ti;

	lti->comp_state = __BIT(lti->gpio_bit);
	lti->prev_state = __BIT(lti->gpio_bit);
	get_flag(dev, lti->flag);
	get_flag(dev, lti->flag + 1);
	get_flag(dev, lti->flag + 2);

	// ACC detect method change
	lti->count = RECHECK_COUNT << 1;

	ci->acc_line = OFF; // according to lti->prev_state
}
void acc_line_gpio_init(device_info_t *dev)
{
	acc_line_msg_pre_init(dev);
}

static void can_line_msg_pre_init(device_info_t *dev)
{
	ctrl_info_t *ci = dev->ci;
	line_timer_info_t *lti = &can_line_ti;

	lti->comp_state = __BIT(lti->gpio_bit);
	lti->prev_state = __BIT(lti->gpio_bit);
	get_flag(dev, lti->flag);
	get_flag(dev, lti->flag + 1);
	get_flag(dev, lti->flag + 2);

	// CAN detect method change
	lti->count = RECHECK_COUNT << 1;

	ci->carcan_wake = OFF; // according to lti->prev_state
}
void can_line_gpio_init(device_info_t *dev)
{
    IO_INIT_I(GPIOA,11,GPIO_PuPd_UP);
	//IO_INIT_I_PU(GPIOA, 11);//��������
		can_line_msg_pre_init(dev);

}


///////////////// End of GPIO LINE Related //////////



static soft_timer_t stimer[STIMER_CNT];
 
 /***********************
 **@brief: ͨ�ö�ʱ����ʼ��
 ** which ��ʾTIMx��1ΪTIM2(32bit) 2 Ϊ TIM3��   msΪ��ʱʱ�� 
 ***
 *****************************************/
 void timer_init(device_info_t *dev, u32 which, u16 ms)
 {
	 u32 prio, reg,clock;
	 RCC_ClocksTypeDef *sys_clock=get_clocks_info();
	 IRQn_Type irq;
	 TIM_TypeDef *tm;
	 RCC_TypeDef *rcc = RCC;
	 
	 // config base clock divid, all 100us, for 16-bit reg can't contain 72000
	 switch(which)
	 {
		 case TIMER_DEV0: // PWM usage
			 
			 return;
 
		 case TIMER_DEV1: // gpio timer
 
			 // enable TIM2 clock  32bit register
			 reg = rcc->APB1ENR;
			 reg |= __BIT(0);
			 rcc->APB1ENR = reg;
 
			 // reset peripheral ����ʱ��
			 rcc->APB1RSTR |= __BIT(0);
			 rcc->APB1RSTR &= ~__BIT(0);
 
			 prio = 6;//����
			 tm = TIM2;
			 clock = sys_clock->PCLK1_Frequency*2;
			 irq = TIM2_IRQn;
			 break;
 
			 
		 case TIMER_DEV2:
 
			 // enable TIM3 clock
			 reg = rcc->APB1ENR;
			 reg |= __BIT(1);
			 rcc->APB1ENR = reg;
 
			 // reset peripheral
			 rcc->APB1RSTR |= __BIT(1);
			 rcc->APB1RSTR &= ~__BIT(1);
 
			 prio = 5;
			 tm = TIM3;
			 clock = sys_clock->PCLK1_Frequency*2;
			 irq = TIM3_IRQn;
			 break;
 
		 case TIMER_DEV3:
			 // enable TIM4 clock
			 reg = rcc->APB1ENR;
			 reg |= __BIT(2);
			 rcc->APB1ENR = reg;
 
			 // reset peripheral
			 rcc->APB1RSTR |= __BIT(2);
			 rcc->APB1RSTR &= ~__BIT(2);
 
			 prio = 4;
			 tm = TIM4;
			 clock = sys_clock->PCLK1_Frequency*2;
			 irq = TIM4_IRQn;
			 break;

		case TIMER_DEV7://basic timer6
			 // enable TIM6 clock
			 reg = rcc->APB1ENR;
			 reg |= __BIT(5);
			 rcc->APB1ENR = reg;
 
			 // reset peripheral
			 rcc->APB1RSTR |= __BIT(5);
			 rcc->APB1RSTR &= ~__BIT(5);
 
			 prio = 7;
			 tm = TIM7;
			 clock = sys_clock->PCLK1_Frequency*2;
			 irq = TIM7_IRQn;
			 break;
 
		 default:
 
			 return;
	 }
 
	 NVIC_SetPriority(irq, prio);//�������ȼ����ж�
	 NVIC_DisableIRQ(irq);//ʧ��
	// MY_NVIC_Init(2,prio,irq,2);
	 // clear update pending flag,Ϊ�˷�ֹһ��UIE�ͽ����жϺ��������룺URSΪ1 ͬʱ SR�������ʼλ��
	 tm->SR = 0;//��� �жϱ�־λ
	 
	 // configs, up-counting
	 tm->CR1 = __BIT(2);//ʹ��,tmΪTIMx�Ĵ�����ַ
 
	 // auto-reload value
	 tm->ARR = ms*10-1; //�Զ���װֵ�� ����õ�TIM   1ms�ж�һ��
 
	 // prescaler
	 tm->PSC = clock /10000 - 1;//��Ƶϵ����Ԥ��Ƶ��7200,�õ�10Khz �ļ���ʱ�ӣ�,100usһ��
 
	 // software UPDATE event, flush config to shadow registers
	 tm->EGR = __BIT(0);//��λ��������ã���Ӳ���Զ���������³�ʼ����ʱ���ļ������������ԼĴ����ĸ���
 
	 // enable UPDATE interrupt
	 tm->DIER = __BIT(0);//ʹ�� UIE
	 
	 // enable this counter, can be used to start/stop this timer
	 //tm->CR1 = __BIT(0);
 }
 
 /***********************
 **@brief: ͨ�ö�ʱ�� ʹ��
 **  �����ж�
 ***
 *****************************************/
  void timer_start(u32 which)
 {
	 IRQn_Type irq;
	 TIM_TypeDef *tm;
 
	 switch(which)
	 {
		 case TIMER_DEV0:
 
			 return;
 
		 case TIMER_DEV1:
 
			 tm = TIM2;
			 irq = TIM2_IRQn;
			 break;
 
		 case TIMER_DEV2:
 
			 tm = TIM3;
			 irq = TIM3_IRQn;
			 break;
 
		 case TIMER_DEV3:
			 tm = TIM4;
			 irq = TIM4_IRQn;
		 break;  
 
		  case TIMER_DEV7:
			 tm = TIM7;
			 irq = TIM7_IRQn;
		 break;
		 default:
 
			 return;
	 }
 
	 // start this counter
	 tm->SR = 0;
	 tm->CR1 = __BIT(0);
	 NVIC_EnableIRQ(irq);
 }

 static void timer_stop(u32 which)
{
	IRQn_Type irq;
	TIM_TypeDef *tm;

	switch(which)
	{
		case TIMER_DEV0:

			return;

		case TIMER_DEV1:

			tm = TIM2;
			irq = TIM2_IRQn;
			break;

		case TIMER_DEV2:

			tm = TIM3;
			irq = TIM3_IRQn;
			break;

		case TIMER_DEV3:

			tm = TIM4;
			irq = TIM4_IRQn;
			break;

		case TIMER_DEV7:

			tm = TIM7;
			irq = TIM7_IRQn;
			break;

		default:

			return;
	}

	tm->CR1 = 0;
	tm->SR = 0;
	NVIC_DisableIRQ(irq);
}

 static void timer_deinit(device_info_t *dev, u32 which)
{
	u32 reg;
	IRQn_Type irq;
	RCC_TypeDef *rcc = RCC;

	// STM32F10x medium density devices only support TIM1~4

	timer_stop(which); // stop first, this api is always not called explicitly

	switch(which)
	{
		case TIMER_DEV0: // PWM

			return;

		case TIMER_DEV1: // gpio timer

			irq = TIM2_IRQn;

			reg = rcc->APB1ENR;
			reg &= ~__BIT(0);
			rcc->APB1ENR = reg;
			break;

		case TIMER_DEV2:

			irq = TIM3_IRQn;

			reg = rcc->APB1ENR;
			reg &= ~__BIT(1);
			rcc->APB1ENR = reg;
			break;

		case TIMER_DEV3:
			irq = TIM4_IRQn;

			reg = rcc->APB1ENR;
			reg &= ~__BIT(2);
			rcc->APB1ENR = reg;
			break;

		case TIMER_DEV7:
			irq = TIM7_IRQn;

			reg = rcc->APB1ENR;
			reg &= ~__BIT(5);
			rcc->APB1ENR = reg;
			break;
		default:

			return;
	}

	NVIC_DisableIRQ(irq);
}

//20180607
/*
*----------------------------------------------------------------------------------------------------------------------------------------
* ����ԭ�ͣ�void start_timer0(uint32_t time1us) //��λΪ1us
* �������ܣ�������ʱ������λΪ1ms �����can��ʱ������
  ����˵��������ʱ��ֵ
  
* �����������
* �� �� ֵ����
  ���ߣ�    
*----------------------------------------------------------------------------------------------------------------------------------------
*/
 void start_timer(u8 TIMx,u16 time1ms)
{
	timer_init(get_device_info(),TIMx,time1ms);
	timer_start(TIMx);
}


/*
*----------------------------------------------------------------------------------------------------------------------------------------
* ����ԭ�ͣ�void stop_timer0(void)
* �������ܣ�ֹͣ��ʱ��  TIM3   
  ����˵����
  
* �����������
* �� �� ֵ����
  ���ߣ�    
*----------------------------------------------------------------------------------------------------------------------------------------
*/
void stop_timer(u8 TIMx)
{
	timer_stop(TIMx);
	timer_deinit(get_device_info(), TIMx);
}


/***********************
**@brief: �ⲿ�жϴ����������� detection
** 
***
*****************************************/
/*
static u32 line_isr_process(line_timer_info_t *lti)
{
	u32 state;
	device_info_t *dev = get_device_info();

	if(!chk_flag(dev, lti->flag))
	{
		goto exit0;
	}

	// Enable IO INTERRUPT first [EX]
	EXTI->PR |= __BIT(lti->gpio_bit);
	EXTI->IMR |= __BIT(lti->gpio_bit);

	state = lti->gpio->IDR & __BIT(lti->gpio_bit);

	if(state == lti->prev_state)
	{
		lti->count--;
	}
	else
	{
		lti->prev_state = state;
		lti->count = RECHECK_COUNT;
	}

	if(!lti->count)
	{
		if(state ^ lti->comp_state) // valid
		{
			lti->comp_state = state; // prepare for the next time

		 // send out line ON/OFF msg
			//dbg_msgv(dev, "[%s->%c]\r\n", (char *)lti->name, state ? 'H' : 'L');
			set_flag(dev, lti->flag + (state >> lti->gpio_bit) + 1);
		}
		else
		{
			dbg_msgv(dev, "[%s noise]\r\n", (char *)lti->name);
		}

		goto exit1;
	}
	else
	{
		// Disable IO INTERRUPT(restore) [EX]
		EXTI->IMR &= ~__BIT(lti->gpio_bit);
	}

	return 1;

exit1:
	get_flag(dev, lti->flag);
	// move ahead [EX]
	//EXTI->PR |= __BIT(lti->gpio_bit);
	//EXTI->IMR |= __BIT(lti->gpio_bit);

exit0:
	return 0;
}*/



 /*******************************
 **@brief: ��ʱ��2�жϷ�����
 **  for   can   ����
 ***************************************/
 static u8 ARM_ERR_DTC_TIM=0;
 void TIM2_IRQHandler(void)
 {
	device_info_t *dev=get_device_info();
	TIM2->SR = 0;//״̬�Ĵ�����Ӳ����1 �������.
	ARM_ERR_DTC_TIM++;
	//dbg_msg(dev,"ENTER tim2\r\n");
	if(ARM_ERR_DTC_TIM>=0x14)
	{	
		ARM_ERR_DTC_TIM=0;

		dev->ipc->dtc_arm_com_sate=(dev->ci->host_ready)?0:1;

		if(dev->ipc->usart_normal == 0)
		{
	 		dev->ci->arm_sleep=1;
	 	}
	
		stop_timer(TIMER_DEV1);
		TIM2->SR = 0;//Ӳ����1 �����	
	}
	else if(ARM_ERR_DTC_TIM==0x01){
		power_on_gpio_ctrl();
	}//kl15 on>2s��־
	else if(ARM_ERR_DTC_TIM==0x02){
		dev->dtc_t.votlage_dtc_flag=1;
	}
 }


/*****************************************************************************
 �� �� ��  : TIM3_IRQHandler
 ��������  : TIM3��ʱ��    ����20ms �ж� �Ƿ���������־
 �������  : void  
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��7��18��
    ��    ��   : xz
    �޸�����   : �����ɺ���

*****************************************************************************/
static u8 BusOffQuick_count=0;
void TIM3_IRQHandler(void)/*��ʱ�����ʱ6500ms*/
{
	 device_info_t *dev=get_device_info();
 
	TIM3->SR = 0;//״̬�Ĵ�����Ӳ����1 ������� 
	if(dev->ci->BusOffQuick==0x01)
	{
		stop_timer(TIMER_DEV2);
		BusOffQuick_count++;
		if(BusOffQuick_count==0x05)
		{
			BusOffQuick_count=0;
			dev->ci->BusOffQuick=0x03;
			}
		else
			dev->ci->BusOffQuick=0x02;
		}
	 
	if(dev->ci->BusOffSlow==0x02)
	{
		dev->ci->BusOffSlow=0x03;//��ʱ��
		stop_timer(TIMER_DEV2);
			}
	
	 TIM3->SR = 0;//״̬�Ĵ�����Ӳ����1 �������
}


#if  (ENABLE_WULING || ENABLE_DIANDONGCHE)
static void left_led_detection(device_info_t *dev)
{
	static u32 v=0,last=0,count = 0;
	v = IO_LEFT->IDR & __BIT(IOBIT_LEFT);
	if(v != last)
	{
		count = 0;
	}
	else
	{
		count++;
		if(count >=5){
			if(v > 0){
			dev->ipc->left_led = 0;
			}
			else{
			dev->ipc->left_led = 1;
			}
		}
	}
	last = v;	
}


static void right_led_detection(device_info_t *dev)
{
	static u32 v=0,last=0,count = 0;
	v = IO_RIGHT->IDR & __BIT(IOBIT_RIGHT);
	if(v != last)
	{
		count = 0;
	}
	else
	{
		count++;
		if(count >=5){
			if(v > 0){
			dev->ipc->right_led = 0;
			}
			else{
			dev->ipc->right_led = 1;
			}
		}
	}
	last = v;	
}
#endif

/***********************
**@brief: �ⲿ����ж�����
**  
***
*****************************************/
static void xxx_detection(device_info_t *dev, line_timer_info_t *lti, u32 count)
{
	u32 v;


	v = lti->gpio->IDR & __BIT(lti->gpio_bit);//����ǰIO�ڵ�ֵ����or�ͣ�
	
	
	if(v ^ lti->prev_state)//����һ��һ�� 
	{
		lti->count = 0;
		lti->prev_state = v;
	}
	else //���ϴβ�һ��
	{
		lti->count++;

		if(count == lti->count)//�ж�count(4��)��4�β�һ����set ��־λ
		{
			set_flag(dev, lti->flag + (v >> lti->gpio_bit)+1);//flag=FLAG_CARCAN,  ���v=0,����FLAG_CARCAN_ON,v=1����FLAG_CARCAN_OFF
		}

		// avoid flag re-send
		if(lti->count == (count << 2))
		{
			lti->count >>= 1;
		}
	}
}



////////////////// Soft timer ///////////////////////
static soft_timer_t stimer[STIMER_CNT];//4  ��ʱ����

/***********************
**@brief: ��ʼ������
***
*****************************************/
void stimer_init(device_info_t *dev)
{
	memset(stimer, 0, sizeof(stimer));
}

/***********************
**@brief: ������ʱ��
***flag = FLAG_HOST_ACK�� count =  ��ʱʱ��
*****************************************/
s32 stimer_start(device_info_t *dev, u32 flag, u32 count)
{
	soft_timer_t *stm;//ָ������

	stm = stimer + (flag - FLAG_STIMER0);

	stm->flag = 0; // for Systick issue
	get_flag(dev, flag); // remove the flag first

	stm->timeout = count; // must place before 'stm->flag = flag', ref stimer_update()
	stm->flag = flag;

	return 0;
}



/********************************************************************************
*@brief:����ʱ��
*
*
***********************************************/
static void stimer_update(device_info_t *dev)
{
	u32 i;
	soft_timer_t *stm;

	for(i = 0; i < STIMER_CNT; i++)
	{
		stm = stimer + i;//ָ������ƫ��i

		if(stm->flag)//��0--������ʼ����stimer_start������ʱ��
		{
			stm->timeout--;

			if(!stm->timeout)//timeout ����0ʱ
			{
				set_flag(dev, stm->flag);

				stm->flag = 0;
			}
		}
	}
}



u32 stimer_stop(device_info_t *dev, u32 flag)
{
	soft_timer_t *stm;

	stm = stimer + (flag - FLAG_STIMER0);

	if((flag == stm->flag) || get_flag(dev, flag))
	{
		stm->flag = 0; // for Systick issue
		stm->timeout = 0;
		get_flag(dev, flag); // remove the flag [boundary issue]

		return 1;
	}

	return 0;
}


void stimer_stop_all(device_info_t *dev)
{
	u32 i;
	soft_timer_t *stm;

	for(i = 0; i < STIMER_CNT; i++)
	{
		stm = stimer + i;

		stm->flag = 0; // for Systick issue
		stm->timeout= 0;
		get_flag(dev, FLAG_STIMER0 + i); // remove the flag [boundary issue]
	}
}

u32 get_systick_value(void)
{
	return SysTick->VAL;
	}

////////////////End Soft timer //////////////////////

/*******************
*
*@brief:����
*
*
*****************************************/
void SysTick_Handler(void)//���ñ�־λΪ1
{
	static u32 lc = 0;   

	device_info_t *dev = get_device_info();//����device��ַ
	#if ENABLE_ACC_DET
	xxx_detection(dev, &acc_line_ti, RECHECK_COUNT);
	#endif

	#if ENABLE_CAN_WEKAUP
	xxx_detection(dev, &can_line_ti, RECHECK_COUNT);//100�� ���ñ�־λ
	if((dev->ci->state == HOST_ACCOFF)&&(dev->ci->wait_canMsg_time==0x00))
	{	
	    dev->ipc->can_wait_count++;
		if(dev->ipc->can_wait_count>=1000)
		{
			dev->ci->wait_canMsg_time =1;
			dev->ipc->can_wait_count=0;
			}

		}
	#endif

//	dbg_msgv(dev, "dev->ipc->heart_count=%d\r\n",dev->ipc->heart_count);//����
	if(dev->ci->host_ready)//host����׼����
	{
		dev->ipc->heart_count++;//�����ж�һ�μ�1һ��
		
		 if(dev->ipc->heart_count ==500)//5S
		{
			dev->ipc->work_state = 0x03;
		}
		
		 if(dev->ipc->heart_count >=500)//�ﵽ500��
		{
			dev->ipc->dtc_arm_com_sate=1;
		}
		
	 if(dev->ipc->heart_count >=2000)//20s
		{
			dbg_msgf(dev, "do whole reset here \r\n");//����
			//dev->ipc->heart_count =0;
					
			IO_SET(GPIOA, 3, LOW);
	
			mdelay(10);
				
			software_reset(); // never return 
			}
	}
	if(dev->diag_t.cf_time_flag)
	{
		dev->diag_t.cf_curret_time+=10;
		
		}
	
	// SLEEP & DEEP-SLEEP stages, no stimer
	stimer_update(dev);//��ʱ������
	
	lc++;
	
	#if ENABLE_KEY_ADC
	set_flag(dev, FLAG_TIMEOUT_ENCADC); // adc key scan use 10ms interval
	#endif
//	set_flag(dev,FLAG_TIMEOUT_10MS);

//	set_flag(dev, FLAG_TIMEOUT_SEND_BODY_CAN_10MS);

//	set_flag(dev,FLAG_TIMEOUT_NO_ACK_10MS);
    if(lc%2 == 0){//20ms
		set_flag(dev, FLAG_TIMEOUT_20MS);
	}

	if(lc%5 == 0){//50ms
		set_flag(dev, FLAG_TIMEOUT_WD);
	}

	if(lc%10 == 0){//100ms
		set_flag(dev, FLAG_TIMEOUT_100MS);
	}

	
	if(lc%40 == 0){//400ms
		set_flag(dev, FLAG_TIMEOUT_400MS);
	}
	
	if(lc%50 == 0){//500ms
		set_flag(dev, FLAG_TIMEOUT_500MS);
	}

	if(lc%100 == 0){//1s
		set_flag(dev, FLAG_TIMEOUT_1S);
	}
}

void systick_start(device_info_t *dev)
{
	// Systick priority is configed inside SysTick_Config()
	SysTick_Config(72000000ul/ SYSTICK_DIV);//10ms�ж�һ��
}


void systick_stop(void)
{
	SysTick->CTRL = 0;
}


void systick_restart(void)
{
	SysTick->CTRL = 0x07u;
}


u32 systick_debug(device_info_t *dev)
{
	return 0;
}







