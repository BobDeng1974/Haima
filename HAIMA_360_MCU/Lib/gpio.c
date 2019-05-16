
#include <stdio.h>
#include <string.h>
#include <config.h>
#include <miscdef.h>
#include <usart.h>
#include <api.h>
#include <timer.h>
#include <stm32f30x.h>

/**************** IO init*****************************/
/*配置成模拟输入 FOR ADC DAC */
void IO_INIT_I_AN(GPIO_TypeDef* GPIOx, u8 bit)
{
	GPIOx->MODER &= ~(GPIO_clear_2bit<<(2*bit));/*mode */
	GPIOx->MODER |= (uint32_t)(GPIO_Mode_AN << (bit * 2));
	
	GPIOx->PUPDR &= ~(GPIO_clear_2bit<<(2*bit));/*floating */
	GPIOx->PUPDR |= (uint32_t)(GPIO_PuPd_NOPULL << (bit * 2));
	}	

	
/*配置成输入模式 */

void IO_INIT_I(GPIO_TypeDef* GPIOx, u8 bit, u8 io_config)
{
	GPIOx->MODER &= ~(GPIO_clear_2bit<<(2*bit));/*mode */
	GPIOx->MODER |= (uint32_t)(GPIO_Mode_IN << (bit * 2));
	
	GPIOx->PUPDR &= ~(GPIO_clear_2bit<<(2*bit));/* */
	GPIOx->PUPDR |= (uint32_t)(io_config << (bit * 2));
	}	

		/*配置成通用输出模式,默认50MHZ*/

void IO_INIT_O_OUT(GPIO_TypeDef* GPIOx, u16 bit, u8 io_config,u8 state)
{
	GPIOx->MODER &= ~(GPIO_clear_2bit << (2*bit));/*mode */
	GPIOx->MODER |= (uint32_t)(GPIO_Mode_OUT << (bit * 2));
	/*PP OD*/
  GPIOx->OTYPER &= ~(GPIO_clear_1bit << bit);
  GPIOx->OTYPER |= (uint16_t)(((uint16_t)io_config) << bit);
	/*speed*/
  GPIOx->OSPEEDR &= ~(GPIO_clear_2bit << (bit * 2));
  GPIOx->OSPEEDR |= ((uint32_t)(GPIO_Speed_Max) << (bit * 2);

	IO_SET(GPIOx,bit,state);
	}	

/*配置为AF, 复用功能, 推挽输出, 上拉输入，for  usart 、can*/

void  IO_INIT_AF(GPIO_TypeDef* GPIOx, u16 bit, u32 GPIO_AF)
{	
	uint32_t temp = 0x00;
  uint32_t temp_2 = 0x00;
	
	GPIOx->MODER &= ~(GPIO_clear_2bit<<(2*bit));/*mode */
	GPIOx->MODER |= (uint32_t)(GPIO_Mode_AF << (bit * 2));
	/*speed*/
  GPIOx->OSPEEDR &= ~(GPIO_clear_2bit << (bit * 2));
  GPIOx->OSPEEDR |= (uint32_t)((GPIO_Speed_Max << (bit * 2));
	/*PP推挽输出 */
  GPIOx->OTYPER &= ~(GPIO_clear_1bit << bit);
  GPIOx->OTYPER |= (uint16_t)(((uint16_t)GPIO_OType_PP) << bit);
	/*上拉输入模式 */
	GPIOx->PUPDR &= ~(GPIO_clear_2bit<<(2*bit));/*floating */
	GPIOx->PUPDR |= (uint32_t)(GPIO_PuPd_UP << (bit * 2));
	
	/* Connect CAN pins to AF7 */
	temp = ((uint32_t)(GPIO_AF) << ((uint32_t)((uint32_t)bit & (uint32_t)0x07) * 4));
  GPIOx->AFR[bit >> 0x03] &= ~((uint32_t)0xF << ((uint32_t)((uint32_t)bit & (uint32_t)0x07) * 4));
  temp_2 = GPIOx->AFR[bit >> 0x03] | temp;
  GPIOx->AFR[bit >> 0x03] = temp_2;
	}
/************END IO config******************/
	

void EXTI9_5_IRQHandler(void)
{
	EXTI_TypeDef *exti = EXTI;
	
	if(exti->PR & __BIT(8)) // 
	{
		exti->PR |= __BIT(8);
	}
}

void EXTI15_10_IRQHandler(void)
{
	EXTI_TypeDef *exti = EXTI;


	if(exti->PR & __BIT(11))  
	{
		exti->PR |= __BIT(11);
	}

}

void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group)	 
{ 
//	SCB_Type *scb =SCB; 
	u32 temp,temp1;	  
	temp1=(~NVIC_Group)&0x07;//è?oóèy??
	temp1<<=8;
	temp=SCB->AIRCR;  //?áè??è?°μ?éè??
	temp&=0X0000F8FF; //?????è?°·?×é
	temp|=0X05FA0000; //D′è???3×
	temp|=temp1;	   
	SCB->AIRCR=temp;  //éè??·?×é	    	  				   
}

void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group)	 
{ 
	u32 temp;	
	MY_NVIC_PriorityGroupConfig(NVIC_Group);//éè??·?×é
	temp=NVIC_PreemptionPriority<<(4-NVIC_Group);	  
	temp|=NVIC_SubPriority&(0x0f>>NVIC_Group);
	temp&=0xf;								//è?μí????  
	NVIC->ISER[NVIC_Channel/32]|=(1<<NVIC_Channel%32);//ê1?ü?D????(òa??3yμ??°,?à·′2ù×÷?íOK) 
	NVIC->IP[NVIC_Channel]|=temp<<4;		//éè???ìó|ó??è??oí?à??ó??è??   	    	  				   
}

void Ex_NVIC_Config(u8 GPIOx,u8 BITx,u8 TRIM) 
{
//	u8 EXTADDR;
//	u8 EXTOFFSET;
//	EXTADDR=BITx/4;//μ?μ??D????′??÷×éμ?±ào?
//	EXTOFFSET=(BITx%4)*4; 
	RCC->APB2ENR|=0x01;//ê1?üio?′ó?ê±?ó			 
//	AFIO->EXTICR[EXTADDR]&=~(0x000F<<EXTOFFSET);//??3y?-à′éè??￡?￡?￡?
//	AFIO->EXTICR[EXTADDR]|=GPIOx<<EXTOFFSET;//EXTI.BITxó3é?μ?GPIOx.BITx 
	//×??ˉéè??
	EXTI->IMR|=1<<BITx;//  ?a??line BITxé?μ??D??
	//EXTI->EMR|=1<<BITx;//2??á±?line BITxé?μ?ê??t (è?1?2??á±??a??,?úó2?té?ê??éò?μ?,μ?ê??úèí?t·???μ?ê±oò?T·¨??è??D??!)
 	if(TRIM&0x01)EXTI->FTSR|=1<<BITx;//line BITxé?ê??t???μ??′￥·￠
	if(TRIM&0x02)EXTI->RTSR|=1<<BITx;//line BITxé?ê??té?éy?μ??′￥·￠
} 

void ACC_EXTI_Init(void)
{
//	RCC->APB2ENR|=1<<2;     //A时钟使能
//	IO_INIT_I(GPIOA, 8, GPIO_PuPd_UP);
	IO_INIT_I( GPIOA, 8,GPIO_PuPd_NOPULL);
	
	Ex_NVIC_Config(0,8,1); 	
	//MY_NVIC_Init(3,1,EXTI9_5_IRQn,2);  	//?à??2￡?×óó??è??1￡?×é2
	NVIC_SetPriority(EXTI9_5_IRQn, 8);
	NVIC_EnableIRQ(EXTI9_5_IRQn);

}
void CAN_RX_EXTI_Init(void)
{
//	RCC->APB2ENR|=1<<2;     //A时钟使能
//	IO_INIT_I(GPIOA, 11, GPIO_PuPd_UP);
		IO_INIT_I( GPIOA, 11,GPIO_PuPd_NOPULL);
	Ex_NVIC_Config(0,11,1); 	
//	MY_NVIC_Init(3,0,EXTI15_10_IRQn,2);  	//?à??2￡?×óó??è??1￡?×é2
	NVIC_SetPriority(EXTI15_10_IRQn, 7);
	NVIC_EnableIRQ(EXTI15_10_IRQn);
}

// for boot code only
void boot_gpio_init(device_info_t *dev)
{
	if(dev->ipc->app_jump)
	{
		return;
	}
	/*enable GPIOA/B/C/D/E peripherals*/
	RCC->AHBENR |= __BIT(17)|__BIT(18) | __BIT(19) | __BIT(20) | __BIT(21);
	
	IO_INIT_O_OUT(GPIOB, 0, GPIO_OType_PP,HIGH); //

	IO_INIT_O_OUT(GPIOA, 3,GPIO_OType_PP ,HIGH); //

	IO_INIT_O_OUT(GPIOB, 8, GPIO_OType_PP,HIGH); //
	
	//摄像头
	IO_INIT_O_OUT(GPIOB, 12, GPIO_OType_PP,LOW); //
	IO_INIT_O_OUT(GPIOB, 13, GPIO_OType_PP,LOW); //
	IO_INIT_O_OUT(GPIOB, 14, GPIO_OType_PP,LOW); //
	IO_INIT_O_OUT(GPIOB, 15, GPIO_OType_PP,LOW); //
}


void app_gpio_init(device_info_t *dev)
{
	IO_INIT_I(GPIOA, 8, GPIO_PuPd_UP); // ACC in  PULL
	
	//IO_INIT_O_OUT(GPIOA, 3, GPIO_OType_PP,LOW); //
	
	IO_INIT_O_OUT(GPIOB, 0, GPIO_OType_PP,HIGH); //

	IO_INIT_O_OUT(GPIOB, 8, GPIO_OType_PP,LOW); //
		
//摄像头
	IO_INIT_O_OUT(GPIOB, 12, GPIO_OType_PP,HIGH); //
	IO_INIT_O_OUT(GPIOB, 13, GPIO_OType_PP,HIGH); //
	IO_INIT_O_OUT(GPIOB, 14, GPIO_OType_PP,HIGH); //
	IO_INIT_O_OUT(GPIOB, 15, GPIO_OType_PP,HIGH);    //
}
void power_on_gpio_ctrl(void)
{
	//dev = get_device_info();
	//IO_SET(GPIOB, 5, HIGH); //
	//mdelay(10);
	//IO_INIT_O_OUT(GPIOB, 0, GPIO_OType_PP,HIGH); //
//	mdelay(10);
	//IO_INIT_O_OUT(GPIOA, 3, GPIO_OType_PP,HIGH); //
//	mdelay(500);//等待PMIC IMX6 复位脚 输出高 
	IO_INIT_O_OUT(GPIOB, 0, GPIO_OType_PP,LOW); //
	
//	IO_SET(GPIOB, 1, HIGH); //  NO  use
//	IO_SET(GPIOB, 8, LOW); //CAN  WAKE
}
//

//1-ACCON    0-ACCOFF 
u8 GetACCStatus(void)
{
	if(IO_GET(GPIOA, 8)==0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}




