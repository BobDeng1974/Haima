#ifndef __SERIAL_H
#define __SERIAL_H
#include "stdint.h"

//================================================================
//                       
//================================================================
/*typedef enum
{
	UART0_DEBUG = 0,
	UART1_3G,          // 3G模块 RAM最大为2048BYTES
	//UART2_GPS,
	UART_MAX
}ENUART_T;*/
#define UART0_DEBUG   0
#define	UART1_3G      1  // 3G模块 RAM最大为2048BYTES
#define UART2_GPS     2  //qdhai add 移出外面，接收单独处理，不生产额外变量
#define UART_MAX      3
//================================================================
//                         发送状态
//================================================================
#define SEND_IDLE		0
#define SENDING			1
#define SEND_OVER		2

#define UART_READ		0
#define UART_WRITE		1



typedef	struct {
	int Wr;
	int Rd;
	int State;
	unsigned int reg; 
	unsigned int BuffSize;
	unsigned char *buf;
}UART;

typedef struct{
	UART recv;
	UART send;
}SERIAL_DEV;

extern void InitialiseUART0(unsigned int bps);
extern void InitialiseUART1(unsigned int bps);
extern void InitialiseUART2(unsigned int bps);
extern void InitialiseUART3(unsigned int bps);
extern void UART0_IRQHandler(void);
extern void UART0_IRQHandler(void);
extern void UART0_IRQHandler(void);
extern void UART0_IRQHandler(void);
extern void UART_Close(uint8_t enUartX);

extern int UART_Open( uint8_t enUartX, unsigned int baudrate);
extern int UART_Read( uint8_t enUartX, unsigned char *buf, int size);
extern int UART_Write( uint8_t enUartX, const unsigned char *buf, int size);
extern void UART_Flush( uint8_t enUartX, int WrRd );
extern uint8_t get_gps_rev_q_state(void);
extern void  clr_gps_rev_q_state(void);
extern void check_gsm_rev(void);
extern uint8_t get_gprs_rev_q_state(void);
extern void  clr_gprs_rev_q_state(void);
	
#endif
