#include"stm32f30x.h"
#include"stm32f30x_adc.h"
#include"stm32f30x_rcc.h"
#include"miscdef.h"
#include"can_dtc.h"
#include"api.h"
#include"usart.h"
#include"can.h"
void adc1_init(void)
{
	ADC_InitTypeDef tADC;
	ADC_CommonInitTypeDef tADC_Comm;
	//configure gpio PA4
	//RCC->AHBENR |= __BIT(17);
	IO_INIT_I_AN(GPIOA, 4);
	/* Configure the ADC clock */
	RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div2);

	/* Enable ADC clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);
	
	ADC_StructInit(&tADC);
	
	/* Calibration procedure */
	ADC_VoltageRegulatorCmd(ADC2, ENABLE);

	/* Insert delay (> 10 μs) */
	udelay(20);
	
	ADC_SelectCalibrationMode(ADC2, ADC_CalibrationMode_Single);
	ADC_StartCalibration(ADC2);

	while(ADC_GetCalibrationStatus(ADC2));

	tADC_Comm.ADC_Mode = ADC_Mode_Independent;                                                                    
	tADC_Comm.ADC_Clock = ADC_Clock_AsynClkMode;                    
	tADC_Comm.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;             
	tADC_Comm.ADC_DMAMode = ADC_DMAMode_OneShot;                  
	tADC_Comm.ADC_TwoSamplingDelay = 2;          
	ADC_CommonInit(ADC2, &tADC_Comm);

	tADC.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Enable;
	tADC.ADC_Resolution = ADC_Resolution_12b; 
	tADC.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;         
	tADC.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
	tADC.ADC_DataAlign = ADC_DataAlign_Right;
	tADC.ADC_OverrunMode = ADC_OverrunMode_Disable;   
	tADC.ADC_AutoInjMode = ADC_AutoInjec_Disable;  
	tADC.ADC_NbrOfRegChannel = 1;
	ADC_Init(ADC2, &tADC);

	/* ADCx regular channel11 configuration */ 
	ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 1, ADC_SampleTime_19Cycles5);

	/* Enable the ADC */
	ADC_Cmd(ADC2, ENABLE);

	/* wait for ADRDY */
	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_RDY));

	/* Start ADC Software Conversion */ 
	ADC_StartConversion(ADC2); 	
}

uint16_t adc_get_value(ADC_TypeDef* ADCx, uint8_t chChannel)	 
{
	uint8_t chTimeout = 0;

	while(!ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC) && (++ chTimeout));

	return ADC_GetConversionValue(ADCx);	
}

uint16_t adc_get_average(ADC_TypeDef* ADCx, uint8_t chChannel, uint8_t chTimes)
{
	uint16_t i;
	uint32_t chTemp = 0;
	
	for(i = 0; i < chTimes; i ++) {
		chTemp += adc_get_value(ADCx, chChannel);
		udelay(10);
	}
	chTemp /= chTimes;
	
	return chTemp;
} 

void adc1_deinit(void)
{	

	ADC_DeInit(ADC2);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, DISABLE);
}   


void Batt_adc_message_process(device_info_t * dev)
{
#if ENABLE_KEY_ADC
	float mainPwrVol=0;
	dev =get_device_info();	
	if(get_flag(dev, FLAG_TIMEOUT_ENCADC))/*10ms读一次值*/
	{	
		dev->adc_t.main_pwr_volt = adc_get_average( ADC2,  ADC_Channel_1, 20)*3.3*11.34/4095+1.2;//转换成实际电压值
		mainPwrVol =dev->adc_t.main_pwr_volt;
		mainPwrVol = 10*mainPwrVol;
		//dbg_msgv(dev, "batt = %6.1f \r\n",mainPwrVol);
		if(mainPwrVol<(MAIN_PWR_TOO_LOW_VALUE-3))
		{//低于6.5V  can sleep 不需要CAN 通信
			//dev->ci->can_no_tx=0x01;
		//	can_deinit_simple(dev);
			car_can_sleep(dev);
			mdelay(1);
			}
		else 
		{
			car_can_wake_init(dev);
			}

		if((mainPwrVol>MAIN_PWR_HIGHT_ALARM_VALUE)||(mainPwrVol<MAIN_PWR_LOW_ALARM_VALUE))
		{
			record_can_bus_dtc(dev,DISABLE);
			}
		else
		{
			record_can_bus_dtc(dev,ENABLE);
		}
		check_main_volt_abnormal_pwr(dev,mainPwrVol);
		}
#endif
}

