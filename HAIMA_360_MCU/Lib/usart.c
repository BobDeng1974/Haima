
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <config.h>
#include <miscdef.h>
#include <usart.h>
#include <api.h>
#include <protocolapi.h>
#include <can.h>
#include <iflash.h>

#define USART3_REMAP		1

#if ENABLE_DEBUG_UART
#define KEY_BACKSPACE		0x08
#define KEY_ENTER			0x0d
#define KEY_SPACE			0x20
#endif


// support 3 USARTs now
static const usart_ctrl_t uartc[] =
{
	{/*PA9 PA10*/
		USART1_IRQn,
		USART1,
		DMA1_Channel4_IRQn,
		DMA1,
		DMA1_Channel4
	},
	{/*PB10 PB11*/
		USART3_IRQn,
		USART3,
		DMA1_Channel2_IRQn,
		DMA1,
		DMA1_Channel2
	}
};



static usart_tx_block_t *uart_tx[USART_DEV_CNT];
static usart_rx_block_t *uart_rx[USART_DEV_CNT];

	/********************** For UART Tx managment ******************/
	
#if ENABLE_DEBUG_UART
static usart_tx_block_t t_debug;
#define tb_debug			&t_debug
#else
#define tb_debug			NULL
#endif

#if ENABLE_COMM_UART
static usart_tx_block_t t_comm;
#define tb_comm			&t_comm
#else
#define tb_comm			NULL
#endif
	/********************** For UART Rx managment ******************/
	
#if (ENABLE_DEBUG_UART)
static usart_rx_block_t r_debug;
#define rb_debug			&r_debug
#else
#define rb_debug			NULL
#endif

#if ENABLE_COMM_UART
static usart_rx_block_t r_comm;
#define rb_comm			&r_comm
#else
#define rb_comm			NULL
#endif




/********************** For DEBUG ******************/
#if ENABLE_DEBUG_UART
	u8 debug_buf[64]; // debug info TX
#if ENABLE_CMD_LINE
static message_t debug_msg; // debug cmd info RX
#endif
#endif


static void usart_irq(u32 which)
{
//	usart_tx_block_t *tb = uart_tx[which];
	usart_rx_block_t *rb = uart_rx[which];
	usart_ctrl_t *usart_dev = (usart_ctrl_t *)&uartc[which];
	device_info_t *dev=get_device_info();

	if(__BIT(6) & usart_dev->uart->ISR)
	{
		usart_dev->uart->ICR |= __BIT(6); //clear the flag TCCF
		usart_dev->uart->CR1 &= ~__BIT(6); // disable UART TC interrupt
		
//		tb->link_list[tb->dma_idx] = 0;

//		tb->dma_idx++;
//		tb->dma_idx &= (u16)(USART_TX_LINK_CNT - 1);
		//set_flag(get_device_info(), tb->msg_flag);
	}

	// USART Rx interrupt flags are cleared by following
	if(__BIT(5) & usart_dev->uart->ISR)
	{		
		rb->mem[rb->write] = usart_dev->uart->RDR;
		rb->write=(rb->write +1)%USART_RX_BUF_SIZE;
		}

//	if(__BIT(3)  & usart_dev->uart->ISR)//????,????   ????Err?????,???????? ????????????,????? 
	//{
	//	usart_dev->uart->ISR  &= (u16)~((u16)0x0008);//??????
		
	//	set_flag(dev,FLAG_USART_ORE);
	//	}
}
// USART1 ISR
void USART1_IRQHandler(void)
{
	usart_irq(USART_DEV0);
}

// USART3 ISR
void USART3_IRQHandler(void)
{
	usart_irq(USART_DEV1);
}


static void usart_dma_irq(u32 which, u32 pos)
{
	usart_ctrl_t *usart_dev = (usart_ctrl_t *)&uartc[which];
	usart_tx_block_t *tb = uart_tx[which];
	
	usart_dev->dma->IFCR = __BIT(pos) | __BIT(pos + 1) | __BIT(pos + 2);

	// DMA must be disabled before re-config
	usart_dev->dma_ch->CCR &= ~(__BIT(0)); // disable DMA channel
	
	tb->send_flag = 0;
//	usart_dev->uart->CR1 |= __BIT(6); // enable UART TC interrupt
}

// DMA1 CH4 ISR, for USART1 TX
void DMA1_Channel4_IRQHandler(void)
{
	usart_dma_irq(USART_DEV0, 12);
}

// DMA1 CH2 ISR, for USART3 TX
void DMA1_Channel2_IRQHandler(void)
{
	usart_dma_irq(USART_DEV1, 4);
}


void usart_device_init(device_info_t *dev)
{
	dev->comm_uart = USART_DEV0;//全局宏
	dev->debug_uart = USART_DEV1;

#if ENABLE_DEBUG_UART
	uart_tx[dev->debug_uart] = tb_debug;
	uart_rx[dev->debug_uart] = rb_debug;
	if(NULL != uart_tx[dev->debug_uart])
	{
		uart_tx[dev->debug_uart]->send_flag = 0;
	}
#endif

#if ENABLE_COMM_UART
	uart_tx[dev->comm_uart] = tb_comm;
	uart_rx[dev->comm_uart] = rb_comm;
	if(NULL != uart_tx[dev->comm_uart])
	{
		uart_tx[dev->comm_uart]->send_flag = 0;
	}
#endif

}

void usart_init(device_info_t *dev, u32 which, u32 br, u32 fmt)
{
	u16 reg;
	u32 reg32, clock;
	RCC_TypeDef *rcc = RCC;
	usart_rx_block_t *rb = uart_rx[which];
	usart_tx_block_t *tb = uart_tx[which];
	usart_ctrl_t *usart_dev = (usart_ctrl_t *)&uartc[which];
	USART_TypeDef *usart = usart_dev->uart;

	switch(which)
	{
		case USART_DEV0:

			clock = 0x044AA200;//USART1  

			// enable DMA1 clock, for UART1
			reg32 = rcc->AHBENR;
			reg32 |= __BIT(0);
			rcc->AHBENR = reg32;

			// MUX GPIO: PA9[TX], PA10[RX]
			// enable USART1
			reg32 = rcc->APB2ENR;
			reg32 |= __BIT(14);
			rcc->APB2ENR = reg32;
			// config GPA9, GPA10
			IO_INIT_AF(GPIOA, 9,GPIO_AF_7);
			IO_INIT_AF(GPIOA, 10,GPIO_AF_7);
			// reset the peripheral
			rcc->APB2RSTR |= __BIT(14);
			rcc->APB2RSTR &= ~__BIT(14);
			break;

		case USART_DEV1:

			clock = 0x02255100;//USART3 -

			// enable DMA1 clock, for UART3
			reg32 = rcc->AHBENR;
			reg32 |= __BIT(0);
			rcc->AHBENR = reg32;

			// enable USART3
			reg32 = rcc->APB1ENR;
			reg32 |= __BIT(18);
			rcc->APB1ENR = reg32;
			IO_INIT_AF(GPIOB, 10,GPIO_AF_7);
			IO_INIT_AF(GPIOB, 11,GPIO_AF_7);
		
			// reset the pheripheral
			rcc->APB1RSTR |= __BIT(18);
			rcc->APB1RSTR &= ~__BIT(18);
			break;
		
		default: // ERROR condition

			return;
	}

	// enable usart, word length , OVER8=0(UE=0时才能改)
	usart->CR1 = 0; // reset default value 默认16 过采样率
		// enable DMA transfer, no DMA receiving
	reg = usart->CR3;
	reg |= __BIT(7);//bit7 打开DMAT
	usart->CR3 = reg;
	usart->CR3 |= __BIT(12);///bit(12)不允许产生过载中断,Twhen the USART is disabled (UE=0)才能成功.
	
	reg = fmt;
	reg |= __BIT(0); // enable USART  UE位
	usart->CR1 = reg;

	// clear the TC flag fist[Default is 1]
	usart->ICR |=  __BIT(6);
	usart->RQR |=  __BIT(3);//RXFRQ

	// stop bit, 1bit
	reg = usart->CR2;
	reg &= ~(3 << 12);
	usart->CR2 = reg;

	// baudrate  此时OVER8=0
	reg = (clock >> 4) / br;
	reg = (reg << 4) + (clock % (br << 4)) / br;
	usart->BRR = reg;

	// set RE & TE bits, RXNEIE interrupt enabled, TXNE interrupt is not enabled(using DMA)
	reg = usart->CR1;
	if(NULL != rb)
	{
		reg |= __BIT(2) | __BIT(5); // enable RX, RXNEIE
	}

	if(NULL != tb)
	{
		reg |= __BIT(3); // enable TX
	}
	usart->CR1 = reg;

	// disable DMA channel first,	clear first
	usart_dev->dma_ch->CCR = 0;

	if(NULL != tb) // DMA ch for TX
	{
		// wait for TC flag for IDLE frame & clear it
	//	while(0 == (usart->ISR & __BIT(6)));
	//	usart->ISR &= ~(__BIT(5)|__BIT(6));//TCCF qingchu 

		// configure channel prio, Src/Dest Size, Inc mode
		reg32 = __BIT(12);
		reg32 |= __BIT(7);
		reg32 |= __BIT(4);
		reg32 |= __BIT(1); // transfer complete INTERRUPT enabled, not half-complete

		usart_dev->dma_ch->CCR = reg32;
	}

	if(NULL != rb)
	{
		rb->write = 0;
		rb->read = 0;
	}

	if(NULL != tb)
	{
		tb->send_flag = 0;
	}

	// enable USART Interrupt
	NVIC_SetPriority(usart_dev->uart_irq, 2);
	NVIC_EnableIRQ(usart_dev->uart_irq);
	//MY_NVIC_Init(1,1,usart_dev->uart_irq,2);
	
	if(NULL != tb)
	{
		// enable USART Tx-DMA Interrupt
		NVIC_SetPriority(usart_dev->dma_irq, 3);
		NVIC_EnableIRQ(usart_dev->dma_irq);
	
	}
}

static void usart_deinit(u32 which)
{
	u32 reg32;
	RCC_TypeDef *rcc = RCC;
	usart_ctrl_t *usart_dev = (usart_ctrl_t *)&uartc[which];
	device_info_t *dev = get_device_info();

	usart_on_dma_end(which);

	// disable interrupts
	NVIC_DisableIRQ(usart_dev->dma_irq);
	NVIC_DisableIRQ(usart_dev->uart_irq);

	// disable usart
	usart_dev->uart->CR1 = 0; // default value, disabel all

	// disable DMA channel
	usart_dev->dma_ch->CCR = 0;

	// clear usart interrupt flag
	usart_dev->uart->ISR = 0; //clear the flag

	// clear DMA interrupt flag
	switch(which)
	{
		case USART_DEV0:

			reg32 = rcc->APB2ENR;
			reg32 &= ~__BIT(14);
			rcc->APB2ENR = reg32;

			usart_dev->dma->IFCR = __BIT(12) | __BIT(13) | __BIT(15);//通道4
			break;

		case USART_DEV1:


			reg32 = rcc->APB1ENR;
			reg32 &= ~__BIT(18);
			rcc->APB1ENR = reg32;


			usart_dev->dma->IFCR = __BIT(4) | __BIT(5) | __BIT(7);//通道2 
			break;


		default:

			return;
	}
}

 usart_rx_block_t *usart_get_rx_block(u32 which)
{
	return uart_rx[which];
}

usart_tx_block_t *usart_get_tx_block(u32 which)
{
	return uart_tx[which];
}

bool usart_find_head(usart_rx_block_t *rb, u32 writep, u8 head)
{
	if(writep == rb->read)
	{
		return FALSE;
	}

	while(rb->mem[rb->read] ^ head)
	{
	//	rb->read++;
	//	rb->read &= USART_RX_BUF_MASK;
		rb->read = (rb->read +1)%USART_RX_BUF_SIZE;
	
		if(writep == rb->read)
		{
			return FALSE;
		}
	}

	return TRUE;
}

	bool usart_read_byte(usart_rx_block_t *rb, u8 *info, u32 writep, u32 *pos)
	{
	  if(writep == *pos)
	  {
	    return FALSE;
	  }
	
	  if(rb->mem[*pos] == 0xFE)
			{
				*pos =(*pos+1)%USART_RX_BUF_SIZE;
				if(writep == *pos)
				{
					return FALSE;
				}  
 
	    if(rb->mem[*pos] == 0x01)
			{
	      *info = 0xFE;
				*pos =(*pos+1)%USART_RX_BUF_SIZE;
	    }
	    else if(rb->mem[*pos] == 0x02)
			{
	      *info = 0xFF;
				*pos =(*pos+1)%USART_RX_BUF_SIZE;
	    }
	    else
			{
					*pos =(*pos+1)%USART_RX_BUF_SIZE;
	      return FALSE;
	    }
	  }
	  else{
	    *info = rb->mem[*pos];
			*pos =(*pos+1)%USART_RX_BUF_SIZE;
	  }
	  return TRUE;
	}
	
bool dbg_usart_read_byte(usart_rx_block_t *rb, u8 *info, u32 writep, u32 *pos)
{

	if(writep == *pos)
	{
		return FALSE;
	}

	*info = rb->mem[*pos];
	
	//*pos++;
	*pos = *pos + 1;
	*pos &= USART_RX_BUF_MASK;

	return TRUE;
}

void usart_data_post(u32 which, u8* dat, u32 len)
{
	u8 *buf;
	u32 i;
	usart_tx_block_t *tb = uart_tx[which];
	usart_ctrl_t *usart_dev = (usart_ctrl_t *)&uartc[which];
	device_info_t *dev;

	if((0 == len) || (NULL == dat))
	{
		return;
	}

	dev = get_device_info();
	
	if(len > USART_TX_ENTRY_SIZE)
	{
		while(len > USART_TX_ENTRY_SIZE)
		{
			watchdog_feed(dev);
			usart_data_post(which, dat, USART_TX_ENTRY_SIZE);
			dat += USART_TX_ENTRY_SIZE;
			len -= USART_TX_ENTRY_SIZE;
		}
	
		if(len)
		{
			usart_data_post(which, dat, len);
		}
		
		return;
	}
	


	
	usart_on_dma_end(which); //确保上一次发送完成
	
		
	buf = tb->buffer;
	
	for(i = 0; i < len; i++)
	{
		buf[i] = dat[i];
	}
	
	usart_dev->dma_ch->CPAR = (u32)&usart_dev->uart->TDR;	//DMA  外设地址 及USART发送寄存器 
	usart_dev->dma_ch->CMAR = (u32)tb->buffer;//DMA 存储器地址
	usart_dev->dma_ch->CNDTR =len;//DMA 传输数据量
	usart_dev->dma_ch->CCR |= __BIT(0); // start the DMA transfer
	
	tb->send_flag = 1;
}

void usart_on_dma_end(u32 which)
{
	usart_tx_block_t *tb = uart_tx[which];
	while(tb->send_flag){
		mdelay(1);
		Iwdg_Feed();
	}
	return;
}

/********************** For UART debug commands ******************/
#if ENABLE_CMD_LINE
static u32 cmd_reset(device_info_t *dev, u8 *par)
{

	device_deinit(dev);

	software_reset();

	return 0;
}

static u32 cmd_get_aver(device_info_t *dev, u8 *par)
{
	if(dev->ipc->in_boot)
	{
		dbg_msg(dev, "this cmd run in app only!");
	
		return 1;
	}

	dbg_msgv(dev, "APP version: %d.%d.%d", get_app_version() >> 16,
		(get_app_version() >> 8) & 0xff, get_app_version() & 0xff);

	return 0;
}

static u32 cmd_get_bver(device_info_t *dev, u8 *par)
{
	u8 *v = (u8 *)&dev->ipc->boot_version;
	
	dbg_msgv(dev, "BOOT version: %d.%d.%d", v[2], v[1], v[0]);

	return 0;
}

static u32 cmd_get_ipc(device_info_t *dev, u8 *par)
{
	ipc_info_t *ipc = dev->ipc;

	dbg_msg(dev, "----------- IPC INFO[HEX] ----------\r\n");

	dbg_msgv(dev, "FLAG1 = %08x\r\n", ipc->flag1);
	dbg_msgv(dev, "FLAG2 = %08x\r\n", ipc->flag2);
	dbg_msgv(dev, "CHKSM = %08x\r\n", ipc->checksum);
	dbg_msgv(dev, "INBOT = %08x\r\n", ipc->in_boot);
	dbg_msgv(dev, "BSTCK = %08x\r\n", ipc->boot_stack);
	dbg_msgv(dev, "BENTY = %08x\r\n", ipc->boot_entry);
	dbg_msgv(dev, "BTVER = %08x\r\n", ipc->boot_version);
	dbg_msgv(dev, "ASTCK = %08x\r\n", ipc->app_stack);
	dbg_msgv(dev, "AENTY = %08x\r\n", ipc->app_entry);
	dbg_msgv(dev, "CBOOT = %08x\r\n", ipc->cold_boot);
	dbg_msgv(dev, "AJUMP = %08x\r\n", ipc->app_jump);
	//dbg_msgv(dev, "RDBAS = %08x\r\n", ipc->rodata_base);

	dbg_msg(dev, "----------- End IPC INFO ----------");

	return 0;
}

static const char * const stat_info[] =
{
	"BOOTING",
	"RUNNING",
	"ACCOFF",
};



static u32 cmd_get_stat(device_info_t *dev, u8 *par)
{
	dbg_msgv(dev, "host is %s\r\n", stat_info[dev->ci->state]);

	return 0;
}

static u32 cmd_get_ciinfo(device_info_t *dev, u8 *par)
{
	ctrl_info_t *ci = dev->ci;

	dbg_msg(dev, "----------- CI INFO ----------\r\n");

	dbg_msgv(dev, "HOST STATE [%s]\r\n", stat_info[ci->state]);
	dbg_msgv(dev, "HOST READY [%c]\r\n", ci->host_ready ? 'Y' : 'N');
	dbg_msgv(dev, "REVERSE ON [%c]\r\n", ci->reverse_on ? 'Y' : 'N');
	dbg_msgv(dev, "UPGRAD TYPE [%s]\r\n", (UPD_TYPE_APP == ci->upgrade_type) ? "APP" : "BOOT");
	dbg_msgv(dev, "CAR CAN WAKE [%c]\r\n", ci->carcan_wake ? 'Y' : 'N');
	dbg_msg(dev, "----------- End CI INFO ----------");

	return 0;

}

u32 cmd_can_msg_proc(device_info_t *dev,u8 *par)
{

	static u8 buff[8]={0};
//	data_info_t di;
	u8 i;
	u16 speed;
	u8 temp;
	
	if(par[0] == 's')
	{
		for(i=1; i++;)
		{
			if(par[i] == ',')
			{
			  break;
			}
			if(i >= 6)
			{
				dbg_msg(dev, "the speed input is error \r\n");
				return 0;
			}
		}
		
		if(i==3)
		{
			speed = (par[1]&0x0f)*10 + (par[2]&0x0f);
		}
		else if(i==4)
		{
			speed =  (par[1]&0x0f)*100;
			speed += (par[2]&0x0f)*10;
			speed += par[3]&0x0f;
		}
		else if(i==5)
		{
			speed = (par[1]&0x0f)*1000;
			speed +=(par[2]&0x0f)*100;
			speed +=(par[3]&0x0f)*10;
			speed += par[4]&0x0f;
		}
		else
		{
			dbg_msg(dev, "the speed input is error \r\n");
			return 0;
		}

			
		speed &=0x0FFF; 
		buff[0] &= 0x0f;
		buff[0] |= (u8)(speed & 0x0f)<<4;
		buff[1] =  (u8) (speed >> 4);
		dbg_msgv(dev, "the speed = %4d \r\n",speed);


	}
	else if(par[0] == 'r')
	{		

		for(i=2; i++;)
		{
			if(par[i] == ',')
			 break;
			if(i >= 5)
			{
				dbg_msg(dev, "the angle input is error \r\n");
				return 0;
			}
		}
		
		if(i==2)
		{
			speed =  par[2]&0x0f;
		}
		else if(i==3)
		{
			speed = (par[2]&0x0f)*10;
			speed += (par[3]&0x0f);
		}
		else if(i==5)
		{
			speed = (par[2]&0x0f)*100;
			speed += (par[3]&0x0f)*10;
			speed += par[4]&0x0f;
		}
		else
		{
			dbg_msg(dev, "the angle input is error \r\n");
			return 0;
		}
		speed &=0x0FFF; 
		
		if(par[1] == 'l')  //左转
		{
			buff[2] = speed;
			buff[3] = 0;	

			dbg_msgv(dev, "the left = %4d \r\n",speed);

			
		}
		else if(par[1] == 'r') // 右转
		{			
			buff[2] = 0;	
			buff[3] = speed;	
			dbg_msgv(dev, "the right = %3d \r\n",speed);

		}

	}
	else if(par[0] == 'o')
	{
		buff[4] &= 0xCF;
		buff[4] |= (par[1]&0x0f)<< 4;  // 转向灯
		buff[5] &= 0xF8;
		buff[5] |= (par[2]&0x0f) ;     //档位
		
		dbg_msgv(dev, "the other1 = %x \r\n",buff[4]);	
		dbg_msgv(dev, "the other2 = %x \r\n",buff[5]);


	}
	else if(par[0] == 'd')
	{
		temp = (par[1]&0x0f);
		comm_message_send(dev, CM_SET_MODE, 0, &temp, 1);
		if(temp == 0x01)
			dbg_msg(dev, "the display mode is 2D\r\n");
		else if(temp == 0x02)
			dbg_msg(dev, "the display mode is 3D\r\n");
		else
			dbg_msg(dev, "the display mode input error\r\n");

		return 0;
	}
	else
	{
		dbg_msg(dev, "the can message input is error \r\n");
		return 0;
	}
	//di.data = buff;
	//di.len = 8;
	
	comm_message_send(dev, CM_SEND_VEHICLE_INFO, 0, buff, 8);


	return 1;

}


static u32 cmd_help(device_info_t *dev, u8 *par);

static const cmd_info_t commands[] =
{
	{DBG_CMD_RESET, "rese: software reset MCU", cmd_reset},
	{DBG_CMD_AVER, "aver: get app version, app only", cmd_get_aver},
	{DBG_CMD_BVER, "bver: get boot version", cmd_get_bver},
	{DBG_CMD_IPCINFO, "ipc:  get ipc info", cmd_get_ipc},
	{DBG_CMD_STATE, "stat: get code running state", cmd_get_stat},
	{DBG_CMD_CIINFO, "ci:   get ci info", cmd_get_ciinfo},

	{DBG_CMD_CAN, "can:   can message info", cmd_can_msg_proc},

	{DBG_CMD_LIST, "ls:   request help info", cmd_help},
	{DBG_CMD_ASK, "?:    request help info", cmd_help},
	{DBG_CMD_HELP, "help: get cmd info", cmd_help},
	{DBG_CMD_INVALID, NULL, NULL}
};

static u32 cmd_help(device_info_t *dev, u8 *par)
{
	cmd_info_t *cmd = (cmd_info_t *)commands;

#ifdef __GNUC__ /* emblocks v2.3 default: gcc -O2 */
#define COMPILER		"GNU GCC"
#elif defined(__CC_ARM) /* keil default: armcc -O0 */
#define COMPILER		"KEIL ARMC"
#elif defined(__ICCARM__) /* IAR compiler */
#error IAR compiler is not supported now!
#elif defined(__TASKING__) /* TASKING compiler */
#error TASKING compiler is not supported now!
#else
#error Unknown compiler!
#endif
	dbg_msgv(dev, "[Compiler: %s]\r\n", COMPILER);

	dbg_msg(dev, "----------- CMD INFO ----------\r\n");

	while(cmd->func)
	{
		dbg_msgv(dev, "%s\r\n", cmd->info);
		cmd++;
	}

	dbg_msgv(dev, "--------- Total %d cmd(s) --------", cmd - commands);

	return 0;
}


static bool cmdcheck_char_vaild(u8 ch_input)
{
  bool char_s = FALSE;
  	
  if((ch_input >= '0')&&(ch_input <= '9'))
     char_s =TRUE;
  else if((ch_input >= 'A')&&(ch_input <= 'Z'))
  	 char_s =TRUE;
  else if((ch_input >= 'a')&&(ch_input <= 'z'))
     char_s =TRUE;
  else if((ch_input == '_')||(ch_input == ';'))
     char_s =TRUE;
  else if((ch_input == '-')||(ch_input == '+'))
     char_s =TRUE;
  else if((ch_input == ',')||(ch_input == '?'))
     char_s =TRUE;
   else
   	 char_s =FALSE;
 
 return char_s;
}

static bool cmdline_message_poll(u32 which, message_t *dmsg)
{
	bool ret = FALSE;
	device_info_t *dev;
	usart_rx_block_t *rb = usart_get_rx_block(which);
	u8 ch, bs[4] = {KEY_BACKSPACE, KEY_SPACE, KEY_BACKSPACE};

	if(rb->write == rb->read)
	{
		goto exit0;
	}

	dev = get_device_info();

	ch = rb->mem[rb->read];

	if(KEY_ENTER == ch)
	{
		ch = dmsg->par_cnt;
		dmsg->par.b[ch++] = 0; // may a single char input with Enter
		dmsg->par.b[ch++] = 0;
		dmsg->par.b[ch++] = 0;

		ret = TRUE; // only when press ENTER
	}
	else if(KEY_BACKSPACE == ch)
	{
		if(dmsg->par_cnt)
		{
			dmsg->par_cnt--;
			usart_data_post(dev->debug_uart, bs, 3);
		}
	}
	else
	{
		if(ch < 0x7f) // acceptable char
		{
			if(dmsg->par_cnt < (USART_DBG_CMD_LEN - 3)) // single char input with Enter
			{
				if(cmdcheck_char_vaild(ch)) 
				{
					if(';'==ch)
					{
						for(;dmsg->par_cnt<4;)
						{
							dmsg->par.b[dmsg->par_cnt] = 0;
							dmsg->par_cnt++;
						}
					}
					dmsg->par.b[dmsg->par_cnt] = ch;
					dmsg->par_cnt++;

					usart_data_post(dev->debug_uart, &ch, 1);
				}
			}
		}
	}
	rb->read++;
	rb->read &= USART_RX_BUF_MASK;

exit0:
	return ret;
}

static void cmdline_message_process(device_info_t *dev, message_t *dmsg)
{
	u32 code;
	cmd_info_t *cmd = (cmd_info_t *)commands;

	if(!dmsg->par_cnt)
	{
		goto exit0;
	}

	code = dmsg->par.w[0];
	
	while((u32)cmd->func)
	{
		if(cmd->code == code)
		{
			dbg_newline_msg(dev);
			cmd->func(dev, &dmsg->par.b[5]);
			break;
		}

		cmd++;
	}

	if(NULL == cmd->func) // invalid command
	{

		if(strlen((const char *)dmsg->par.b) > 4)
		{
			dmsg->par.w[1] = C4_2_U32('.', '.', '.', 0);
		}
		
		dbg_msgv(dev, "\r\nunknown command: '%s'", (const char *)dmsg->par.b);
	}

	dmsg->par_cnt = 0;

exit0:
	dbg_prompt_msg(dev);
}
#endif

static void autotest_message_process(device_info_t *dev, message_t *dmsg)
{

	u32 ver,i,hid = dmsg->hid.w;
	u8 buff[2] = {0};
	u8 buff1[8] = {0};
	u16 voltage[1] = {0};
  u8 Device_ID[8] = {0};
  u8 SerialNumber[20] = {0};
	switch(dmsg->id)
		{
			case CM_PC_ARM_READY:
				
			    buff[0] = CM_PC_ARM_READY;
			    
			    buff[1] = (HOST_RUNNING == dev->ci->state)? 1:0;

                if(1 == buff[1])
				{

                  dev->ipc->autotest_flag = 1;
                	}	
			
				debug_message_send(dev, CM_PC_GENERAL_ACK, hid, buff, 2);

				break;

            case CM_PC_ARM_STOP:

                buff[0] = CM_PC_ARM_STOP;
			    
			    buff[1] = 0x01;
				
				if(1 == dev->ipc->autotest_flag)
				{
				  debug_message_send(dev, CM_PC_GENERAL_ACK, hid, buff, 2);
				}
				
                dev->ipc->autotest_flag = 0;

                break;

			case CM_PC_SEND_VEHICLE_INFO: 

                for(i=0;i<8;i++){
                    
                   buff1[i] = dmsg->par.b[i];
                }
			    
			    comm_message_send(dev, CM_SEND_VEHICLE_INFO, hid, buff1, 8);
			     
				break;
	
			case CM_PC_SET_AVM_ENABLE:
				
			    buff1[0] = dmsg->par.b[0];

			    comm_message_send(dev, CM_SEND_KEY, hid, buff1, 1); 
			
				break;
				
			case CM_PC_MCU_APP_VERSION: 
				
			    ver = get_app_version();	
				if(1 == dev->ipc->autotest_flag)
				{
			     debug_message_send(dev, CM_PC_MCU_APP_VERSION_ACK, hid,(u8 *)&ver , 4);
				}
		
				break;
	
			case CM_PC_MCU_BOOT_VERSION:
				
			    ver = get_boot_version();       
			    if(1 == dev->ipc->autotest_flag)
				{
			     debug_message_send(dev, CM_PC_MCU_BOOT_VERSION_ACK, hid,(u8 *)&ver , 4);
			    }
 
				break;

			case CM_PC_RESET_SYSTEM: 
		      
			    buff[0] = CM_PC_RESET_SYSTEM;
			    buff[1] = 01;
			    if(1 == dev->ipc->autotest_flag)
				{
			     debug_message_send(dev, CM_PC_GENERAL_ACK, hid, buff, 2);
			    }
			
				device_deinit(dev);

	            software_reset();
			
				break;
	
			case CM_PC_ARM_APP_VERSION:
                
			    comm_message_send(dev, CM_ARM_APP_VERSION, hid, buff1, 0);

				break;

			case CM_PC_ARM_ARI_VERSION:
         			
			    comm_message_send(dev, CM_ARM_ARI_VERSION, hid, buff1, 0);
			
			    break;

			case CM_PC_SET_DISP_TYPE: 
				
			    buff[0] = dmsg->par.b[0];
				
			    comm_message_send(dev, CM_SET_DISP_TYPE, hid, buff, 1);
		
				break;
	   
			case CM_PC_GET_DISP_TYPE:

			    comm_message_send(dev, CM_GET_DISP_TYPE, hid, buff, 0);
			
				break;
     
		    case CM_PC_VIDEO_CALIBRATION:		
				
				buff1[0] = dmsg->par.b[0];
				    
				comm_message_send(dev, CM_VIDEO_CALIBRATION, hid, buff1, 1);
				
				break;
	
			case CM_PC_SET_CAR_COLOUR:

			    buff1[0] = dmsg->par.b[0];
				
			    comm_message_send(dev, CM_SET_CAR_COLOUR, hid, buff1, 1);
				
			    break;

			case CM_PC_SET_AVM_ALL_SWITCH:
			
			    buff1[0] = dmsg->par.b[0];
				
			    comm_message_send(dev, CM_SET_AVM_ALL_SWITCH, hid, buff1, 1);
		
				break;

			case CM_PC_SET_CAR_SUBLINE: 
			
			    buff1[0] = dmsg->par.b[0];
				  
			    comm_message_send(dev, CM_SET_CAR_SUBLINE, hid, buff1, 1);		
				
				break;
	
			case CM_PC_SET_ADAS_FUNCTION_SWITCH:
				
			    buff1[0] = dmsg->par.b[0];
				
			    comm_message_send(dev, CM_SET_ADAS_FUNCTION_SWITCH, hid, buff1, 1);
					

				break;

			case CM_PC_SET_ADAS_FUNCTION_SPEED: 
		
			    buff1[0] = dmsg->par.b[0];
				
			    comm_message_send(dev, CM_SET_ADAS_FUNCTION_SPEED, hid, buff1, 1);		

				break;
	    
			case CM_PC_GET_AVM_INFO	:
				
			    comm_message_send(dev, CM_GET_AVM_INFO, hid, buff1, 0);

				break;
      
			
			case CM_PC_SET_TURN_ENABLE: 
				
			    buff[0] = dmsg->par.b[0];
			
			    comm_message_send(dev, CM_SET_TURN_ENABLE, hid, buff, 1);

				comm_message_send(dev, CM_GET_TURN_STATUS , hid, NULL, 0);

				break;

            case CM_PC_SET_SerialNumber:

						if(1 == dev->ipc->autotest_flag)
						{
							buff[0]=CM_PC_SET_SerialNumber;
							
							memcpy(SerialNumber,dmsg->par.b,SERIALNUMBER_LEN);
			 
							if(STMFLASH_Write(SERIALNUMBER_ADDR,(u16*)SerialNumber,(SERIALNUMBER_LEN+1)/2))
							{
								mdelay(2);
                buff[1]=0x01;//成功
                }
							else
							{
								mdelay(2);
								buff[1]=0x02;//写入失败
							}
									
								debug_message_send(dev, CM_PC_GENERAL_ACK, hid, buff, 2);
							}
						
                break;
							
				case CM_PC_GET_SerialNumber:
							
               if(1 == dev->ipc->autotest_flag)
							{
						//		 para_read_Nbyte(SERIALNUMBER_ADDR, SerialNumber,SERIALNUMBER_LEN);
									
								STMFLASH_Read(SERIALNUMBER_ADDR,(u16*)SerialNumber,(SERIALNUMBER_LEN+1)/2);  //读NumToRead（半字个数）
                 debug_message_send(dev, CM_PC_GET_SerialNumber_Ack, hid,SerialNumber,SERIALNUMBER_LEN);
								
                }
                break;

            case CM_PC_SET_DeviceID:
   
   						if(1 == dev->ipc->autotest_flag)
							{
								
								memcpy(Device_ID,dmsg->par.b,DEVICE_ID_LEN);
			 
							if(STMFLASH_Write(DEVICE_ID_ADDR,(u16*)Device_ID,DEVICE_ID_LEN/2))
							{
									mdelay(2);
									buff[1]=0x01;//成功
                }
							else
							{
									mdelay(2);
									buff[1]=0x02;//写入失败
									}
								debug_message_send(dev, CM_PC_GENERAL_ACK, hid, buff, 2);
							}

                break;
             
            case CM_PC_GET_DeviceID:
							
               if(1 == dev->ipc->autotest_flag)
							{						
								 STMFLASH_Read(DEVICE_ID_ADDR,(u16*)Device_ID,DEVICE_ID_LEN/2);

                 debug_message_send(dev, CM_PC_GET_DeviceID_Ack, hid,Device_ID,DEVICE_ID_LEN);								
                }
                break;

			case CM_PC_SET_MODE:

				buff[0] = dmsg->par.b[0];
			
			    comm_message_send(dev, CM_SET_MODE, hid, buff, 1);

				break;

			case CM_PC_TEST_SD_CARD:

				comm_message_send(dev, CM_TEST_SD_CARD, hid, NULL, 0);

				break;

			case CM_PC_MCU_SLEEP:

               	if(1 == dev->ipc->autotest_flag)
				{
                    
				 buff[0] = CM_PC_MCU_SLEEP;
			    
			     buff[1] = 0x01;
   
                 debug_message_send(dev, CM_PC_GENERAL_ACK, hid, buff, 2);  
                    
                 dev->ipc->autotest_MCU_SLEEP_flag = 1;
				 
               	}

				break;
               
             case CM_PC_MCU_VOLTAGE:

			    if(1 == dev->ipc->autotest_flag)
				{
                    
				 voltage[0] = dev->ipc->autotest_voltage;

				 //dbg_msgv(dev,"vlot = %x\r\n",voltage[0]);
                    
                 debug_message_send(dev, CM_PC_MCU_VOLTAGE_ACK, hid, (u8 *)voltage, 2); 
                    
               	}
				break;

			  case CM_PC_USB:

				comm_message_send(dev, CM_CHECK_USB, hid, NULL, 0);
				
				break;
     
     
            default:break;
		}

}

s8 autotest_message_poll(device_info_t *dev, message_t *dmsg)
{
#if ENABLE_DEBUG_UART
  u8 ch, par;
  usart_rx_block_t *rb;
  u32 i, len, writep, pos;

  rb = usart_get_rx_block(dev->debug_uart);
  
  writep = rb->write;
  pos = rb->read;
  
  if(FALSE == usart_find_head(rb, writep, USART_MSG_HEAD))
  {  
    rb->read = pos;
    goto exit0;
  }

  if(writep < rb->read)
  {
    len = USART_RX_BUF_SIZE - rb->read + writep;
  }
  else
  {
    len = writep - rb->read;
  }
  
  if(len < USART_MSG_LEN_MIN)
  {
    goto exit1;
  }
  
  pos = rb->read;

  // skip head
  dbg_usart_read_byte(rb, &ch, writep, &pos);

  // get msg id
  dbg_usart_read_byte(rb, &ch, writep, &pos);

  par = ch;
  dmsg->id = ch;

  // get hid
  for(i = 0; i < 4; i++)
  {
    dbg_usart_read_byte(rb, &ch, writep, &pos);
    dmsg->hid.b[i] = ch;
    par ^= ch;
  }
  
  // get len
  dbg_usart_read_byte(rb, &ch, writep, &pos);

  if(ch >= MSG_MAX_PARA)
  {
  //  dbg_msg(dev, "PC AND MCU para cnt error!\r\n");
	//dbg_msgv(dev, "invalid para cnt %d, skip!\n", ch);
    //rb->read = pos;
    goto exit1;
  }

  par ^= ch;
  dmsg->par_cnt = ch;

  for(i = 0; i < dmsg->par_cnt; i++)
  {
    if(FALSE == dbg_usart_read_byte(rb, &ch, writep, &pos))
    {
      goto exit1;
    }
    else
    {
      par ^= ch;
      dmsg->par.b[i] = ch;
    }
  }
  
  // read out checksum
  if(FALSE == dbg_usart_read_byte(rb, &ch, writep, &pos))
  {
    goto exit1;
  }

  // skip the used data
  rb->read = pos;

  // compare chksum  
  if(par ^ ch)
  {
 //   dbg_msg(dev, "PC AND MCU checknum error!\r\n");
   // dbg_msgv(dev, "checksum mismatch[%02x:%02x], skip!\r\n", par, ch);
   // dbg_msgv(dev, "R=%d,Rp=%d,W=%d,Wp=%d\r\n", rb->read, pos, rb->write, writep);
   // dbg_msgv(dev, "cmd=%02x:par num=%02x\r\n", dmsg->id, dmsg->par_cnt);
    for(i = 0; i < dmsg->par_cnt; i++)
    {
  //    dbg_msgv(dev, "par%d:%02x\r\n", i, dmsg->par.b[i]);
    }
		rb->read =rb->write= 0;//丢失就清零
	//	memset(rb->mem, 0, sizeof(rb->mem));
    goto exit1;
  }
  
//  comm_message_send(dev, msg->id, NULL, NULL, 0); // ACK for valid msg
    
  return 0;
exit1:
  return 1;
exit0:
#endif
  return -1;
}


void dbg_uart_init(device_info_t *dev)
{
#if ENABLE_DEBUG_UART
	u32 which;

	which = dev->debug_uart;
	
	usart_init(dev, which, 115200, USART_FMT_8N1);
#endif
}

void dbg_uart_deinit(device_info_t *dev)
{
#if ENABLE_DEBUG_UART
	usart_deinit(dev->debug_uart);
#endif
}

message_t *dbg_msg_alloc(device_info_t *dev) // for debug command line interface
{
#if ENABLE_CMD_LINE
	static message_t debug_msg; // debug cmd info RX
	return &debug_msg;
#else
	return NULL;
#endif
}

bool dbg_uart_msg_process(device_info_t *dev, message_t *dmsg)
{
  
#if ENABLE_DEBUG_UART
  s8 ret;
 // if(get_flag(dev, FLAG_UART_DEBUG_DMAT_END))
//  {
 //   usart_on_dma_end(dev->debug_uart);
 // }
  
  ret = autotest_message_poll(dev, dmsg);
  
  if(ret == 0)
  {
    
    autotest_message_process(dev, dmsg);
    
  }
  else if(ret == (-1))
  {
#if ENABLE_CMD_LINE
    if(dev->ipc->autotest_flag == 0)/*非自动化测试*/
    {
      if(TRUE == cmdline_message_poll(dev->debug_uart, dmsg))
      {
        cmdline_message_process(dev, dmsg);
      }
    }
    
#endif
  }
  else{
  	}
  
#endif

  return TRUE;
}

void comm_uart_init(device_info_t *dev)
{
#if ENABLE_COMM_UART
	u32 which;

	which = dev->comm_uart;
	  
	usart_init(dev, which, 115200, USART_FMT_8N1);

#endif
}


void debug_message_send(device_info_t *dev, u8 cmd, u32 hid, u8 *info, u32 len)
{

  /*一起发送发送数据有大小限制不超过32 cnt = 1*/
#if ENABLE_DEBUG_UART
	u8 *buf, sum;
	u32 i, j, mem[15]={0};

	buf = (u8 *)mem;
	buf += 2;

	buf[0] = USART_MSG_HEAD;
	buf[1] = cmd;
	mem[1] = hid; // buf[2:5]
	buf[6] += len;
	sum = 0;

	for(i = 1; i < 7; i++) // cmd id len
	{
		sum ^= buf[i];
	}
	buf = &buf[7];
	
	for(i = 0; i < len; i++)
	{
//	watchdog_feed(dev);
		buf[i] = info[i];
	}
	for(j = 0; j < len; j++)
	{
		sum ^= info[j];
	}
	buf[len] = sum;
	buf = (u8 *)mem;
	buf += 2;
	usart_data_post(dev->debug_uart, buf, (8 + len)); 
#endif
}


void comm_uart_deinit(device_info_t *dev)
{
#if ENABLE_COMM_UART
	usart_deinit(dev->comm_uart);
#endif
}
//轮询comm msg
bool comm_message_poll(device_info_t *dev, message_t *msg)
{
#if ENABLE_COMM_UART
	u8 ch, par;
	usart_rx_block_t *rb;
	u32 i, len, writep, pos;

	rb = usart_get_rx_block(dev->comm_uart);
	
	writep = rb->write;
	
	if(FALSE == usart_find_head(rb, writep, USART_MSG_HEAD))
	{
		goto exit0;
	}

	if(writep < rb->read)
	{
		len = USART_RX_BUF_SIZE - rb->read + writep;
	}
	else
	{
		len = writep - rb->read;
	}
	
	if(len < USART_MSG_LEN_MIN)
	{
		goto exit0;
	}
	
	pos = rb->read;

	// skip head
	usart_read_byte(rb, &ch, writep, &pos);

	// get msg id
	 if(FALSE == usart_read_byte(rb, &ch, writep, &pos))
	{
   // dbg_msgv(dev, "get msg id failed! r=w, skip!\n");
    rb->read = pos;
    goto exit0;
  }
	
//	dbg_msgv(dev, "id:%x hid",ch);
	
	par = ch;
	msg->id = ch;

	// get hid
	for(i = 0; i < 4; i++)
	{
	 if(FALSE == usart_read_byte(rb, &ch, writep, &pos))
		{
    //  dbg_msgv(dev, "get msg hid failed! r=w, skip!\n");
      rb->read = pos;
      goto exit0;
    }
		msg->hid.b[i] = ch;
		par ^= ch;
	}
	
	// get len
  if(FALSE == usart_read_byte(rb, &ch, writep, &pos))
	{
   // dbg_msgv(dev, "get msg len failed! r=w, skip!\n");
    rb->read = pos;
    goto exit0;
  }
//	dbg_msgv(dev, ":len:%x:data:",ch);
	if(ch >= MSG_MAX_PARA)
	{
		dbg_msgv(dev, "invalid para cnt %d, skip!\n", ch);
		rb->read = pos;
		rb->read=rb->write;
		goto exit0;
	}

	par ^= ch;
	msg->par_cnt = ch;

	for(i = 0; i < msg->par_cnt; i++)
	{
		if(FALSE == usart_read_byte(rb, &ch, writep, &pos))
		{
			goto exit0;
		}
		else
		{
			par ^= ch;
			msg->par.b[i] = ch;
		}
	}
	
	// read out checksum
	if(FALSE == usart_read_byte(rb, &ch, writep, &pos))
	{
		goto exit0;
	}

	// skip the used data
	rb->read = pos;

	// compare chksum	
	if(par ^ ch)
	{
	//	dbg_msgv(dev, "usart_sts=%x\r\n", USART1->SR);//? 3
		//dbg_msgv(dev, "checksum mismatch[self_checkNum=%02x:rec_checkNum=%02x], skip!\r\n", par, ch);
	//	dbg_msgv(dev, "R=%d,Rp=%d,(?????write)W=%d,(??msg??????)Wp=%d\r\n", rb->read, pos, rb->write, writep);//writep?rb->write??????????? ???????????
	//	dbg_msgv(dev, "cmd_id=%02x:par_cnt=%02x\r\n", msg->id, msg->par_cnt);

		rb->read =rb->write= 0;//?????
		
		goto exit0;
	}
	
//	comm_message_send(dev, msg->id, NULL, NULL, 0); // ACK for valid msg
		
	return TRUE;
	
exit0:
#endif
	return FALSE;
}



//??????
void  pkg_transfer(u8* src_buf, u32 src_n, u8* dst_buf, u32* dst_n)
{
  u32 i = 0;
  u32 j = 0;
  if(src_buf == NULL || src_n == 0)
  {
//   dbg_msg(dev,"src_buf is null\n");
    return;
  }
  for(i=0; i< src_n; i++)
  {
    if(src_buf[i] == 0xff)
    {
      dst_buf[j++] = 0xfe;
      dst_buf[j] = 0x02;
    }
    else if(src_buf[i] == 0xfe)
    {
      dst_buf[j++] = 0xfe;
      dst_buf[j] = 0x01;
    }
    else
    {
      dst_buf[j] = src_buf[i];
    }
    j++;
  }
  *dst_n = j;
}

#if 0 //ENABLE_COMM_UART
void comm_message_send(device_info_t *dev, u8 cmd, u32 hid, u8 *info, u32 len)
{

	u8 sum, *buf;
	u32 i, mem[3];  
	u32 len_trans=0;
	u8 trans_sum[2]={0};
		
	buf = (u8 *)mem;
	buf += 2;

	buf[0] = USART_MSG_HEAD;
	buf[1] = cmd;

	mem[1] = hid; // buf[2:5]

	buf[6] = len;

	usart_data_post(dev->comm_uart, buf, 7, 0); // send msg head fix part

	sum = 0;

	for(i = 1; i < 7; i++) // cmd id len
	{
		sum ^= buf[i];
	}

	for(i = 0; i < len; i++)
	{
		sum ^= info[i];
	}

 	for(i=0;i<len;i++)
 	{
		pkg_transfer(&info[i], 1, trans_sum, &len_trans);
		
		usart_data_post(dev->comm_uart,trans_sum, len_trans, 0); // send msg part
		
		}
	
	pkg_transfer(&sum, 1, trans_sum, &len_trans);
	
	usart_data_post(dev->comm_uart,trans_sum, len_trans, 1); // send msg tail part
	
}
#endif

void comm_message_send(device_info_t *dev, u8 cmd, u32 hid, u8 *info, u32 len)
{

   /*一起发送发送数据有大小限制不超过32 cnt = 1*/
#if ENABLE_COMM_UART
	u8 *buf, sum;
	u32 i, j, mem[3]={0};
	u32 len_trans,len_sum;//??????
	u8 trans_sum[64]={0};
	u16 bag_num=0,last_len=0;
	buf = (u8 *)mem;
	buf += 2;
	
	buf[0] = 0xff;
	buf[1] = cmd;
	mem[1] = hid; // buf[2:5]
	buf[6] = len;

	sum = 0;

	for(i = 1; i < 7; i++) // cmd id len
	{
		sum ^= buf[i];
	}	
	
	trans_sum[0]=buf[0];
	
	pkg_transfer(&buf[1], 6, &trans_sum[1], &len_trans);
	
	usart_data_post(dev->comm_uart,trans_sum, (len_trans+1)); // send msg part
	

	
	for(j = 0; j <len; j++)
	{
		sum ^= info[j];
	}

	bag_num = len/32;
	last_len= len%32;
 	i =0;
	while (bag_num--)
 	{
		pkg_transfer(&info[(i++)*32], 32,trans_sum, &len_trans);
	
		usart_data_post(dev->comm_uart,trans_sum, len_trans); // send msg part
	}
	if(len != 0)
	{
		pkg_transfer(&info[i*32], last_len, trans_sum, &len_trans);
		
		usart_data_post(dev->comm_uart,trans_sum, len_trans); // send msg part
	}

	pkg_transfer(&sum,1,trans_sum,&len_sum);
	
	usart_data_post(dev->comm_uart, trans_sum, len_sum); // send msg tail part
#endif
}
 
FlagStatus USART_GetFlagStatus(USART_TypeDef* USARTx, uint32_t USART_FLAG)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  
  if ((USARTx->ISR & USART_FLAG) != (uint16_t)RESET)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }
  return bitstatus;
}


/*重定向printf到usart3*/
int fputc(int ch, FILE* stream)
{
//    USART_SendData(USART3, ch);
	
	 /* Transmit Data */
		USART3->TDR = (ch & (uint16_t)0x01FF);

    while(USART_GetFlagStatus(USART3, USART_ISR_TC) == RESET) ;
    return(ch);
}
