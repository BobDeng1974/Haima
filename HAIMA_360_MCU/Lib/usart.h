
#ifndef __USART_H__
#define __USART_H__

#include <stm32f30x.h>
#include <string.h>
#include <stdio.h>
#include <config.h> /* For dbg_msg() API */
#include <miscdef.h>
// UART define
#define USART_RX_BUF_SIZE		512 /* bytes, must be 2^n */
#define USART_RX_BUF_MASK		(USART_RX_BUF_SIZE - 1)
#define USART_TX_ENTRY_SIZE	    32

// message related
#define USART_MSG_HEAD		   0xff
#define USART_MSG_LEN_MIN		8 /* HEAD + CMD + ID32 + CNT + PARs + SUM */
#define USART_MSG_MAX_PARA	    MSG_MAX_PARA /* max para count */
#define USART_DBG_CMD_LEN		MSG_MAX_PARA /* fit for 32bit word switch-case compare */

// usart packet config
#define USART_FMT_8N1			0x00ul /* 1 start, 8 data, no parity, 1 stop */
#define USART_FMT_7E1			(0x02ul << 9) /* 1 start, 7 data, 1 even parity, 1 stop */
#define USART_FMT_7O1			(0x03ul << 9) /* 1 start, 7 data, 1 odd parity, 1 stop */
#define USART_FMT_9N1			(0x08ul << 9) /* 1 start, 9 data, no parity, 1 stop */
#define USART_FMT_8E1			(0x0aul << 9) /* 1 start, 8 data, 1 even parity, 1 stop */
#define USART_FMT_8O1			(0x0bul << 9) /* 1 start, 8 data, 1 odd parity, 1 stop */

#if ENABLE_DEBUG_UART
extern u8 debug_buf[];
#define dbg_msg(dev, str)			usart_data_post((dev)->debug_uart, (u8 *)str, strlen((const char *)(str)))
#define dbg_msgf(dev, str)		    dbg_msg(dev, str); \
								usart_on_dma_end((dev)->debug_uart)
#define dbg_msgv(dev, fmt, arg...)	sprintf((char *)debug_buf, fmt, ##arg);\
								usart_data_post((dev)->debug_uart, debug_buf, strlen((const char *)debug_buf))
#define dbg_msgfv(dev, fmt, arg...)	dbg_msgv(dev, fmt, ##arg);	\
									usart_on_dma_end((dev)->debug_uart)
#else

#define dbg_msg(dev, str)	
#define dbg_msgf(dev, str)	
#define dbg_msgv(dev, fmt, arg...)
#define dbg_msgfv(dev, fmt, arg...)
#endif
#define dbg_newline_msg(dev)	dbg_msg(dev, "\r\n")
#define dbg_prompt_msg(dev)	dbg_msg(dev, dev->ipc->in_boot ? (u8 *)"\r\nboot>" : (u8 *)"\r\nroot@usr:~#")

typedef struct usart_rx_block
{
	u8  mem[USART_RX_BUF_SIZE];
	u32 write; // write pointer, point to the 1st empty place
	u32 read; // read pointer, point to the 1st readabble place
}usart_rx_block_t;

typedef struct usart_tx_block
{
	u32 send_flag; // 
	u8 buffer[USART_TX_ENTRY_SIZE];
}usart_tx_block_t;

typedef struct usart_ctrl
{
	IRQn_Type uart_irq;
	USART_TypeDef *uart;
	IRQn_Type dma_irq;
	DMA_TypeDef *dma;
	DMA_Channel_TypeDef *dma_ch;
}usart_ctrl_t;

typedef struct cmd_info
{
	u32 code; // command code
	const char *info;
	u32 (*func)(device_info_t *dev, u8 *par);
}cmd_info_t;

void usart_device_init(device_info_t * dev);
usart_rx_block_t *usart_get_rx_block(u32 which);
usart_tx_block_t *usart_get_tx_block(u32 which);
bool usart_find_head(usart_rx_block_t *rb, u32 writep, u8 head);
bool usart_read_byte(usart_rx_block_t *rb, u8 *info, u32 writep, u32 *pos);
void usart_data_post(u32 which, u8* dat, u32 len);
void usart_on_dma_end(u32 which);
void dbg_uart_init(device_info_t * dev);
void dbg_uart_deinit(device_info_t *dev);
message_t *dbg_msg_alloc(device_info_t *dev);
bool dbg_uart_msg_process(device_info_t * dev, message_t *dmsg);
void debug_message_send(device_info_t *dev, u8 cmd, u32 hid, u8 *info, u32 len);
void comm_uart_init(device_info_t *dev);
void comm_uart_deinit(device_info_t *dev);

bool comm_message_poll(device_info_t *dev, message_t *msg);
void comm_message_send(device_info_t *dev, u8 cmd, u32 hid, u8 *info, u32 len);
bool dbg_usart_read_byte(usart_rx_block_t *rb, u8 *info, u32 writep, u32 *pos);
int fputc(int ch, FILE* stream);

#endif

