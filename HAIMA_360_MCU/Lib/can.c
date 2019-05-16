

#include <usart.h>
#include <api.h>
#include <can.h>
#include <protocolapi.h>
#include <can_diag_protocol.h>
#include <iflash.h>
#include	<can_dtc.h>
#if  ENABLE_CAR_CAN//使能车身can通讯

/* TIR发送邮箱标识符寄存器*/
#define TMIDxR_TXRQ	 0x00000001 /*TIR第0位TERQ软件置1请求邮箱发送data，硬件置0 */

/*FMR为CAN 过滤器主控寄存器,其第一位FINIT控制过滤器组。
				0: 过滤器组工作在正常模式；
				1: 过滤器组工作在初始化模式。 */
#define FMR_FINIT    0x00000001 /* Filter init mode */

/* CAN主状态寄存器 (CAN_MSR)第一位INAK 初始化确认 */
#define INAK_TIMEOUT				0x0000FFFF//初始化确认

/* Mailboxes definition  3个发送邮箱 */
#define CAN_TXMAILBOX_0				0x00
#define CAN_TXMAILBOX_1				0x01
#define CAN_TXMAILBOX_2		   	0x02

// CAN mode definitions can工作模式
#define CAN_Mode_Normal			    0x00  /*!< normal mode */
#define CAN_Mode_LoopBack			0x01  /*!< loopback mode */
#define CAN_Mode_Silent				0x02  /*!< silent mode */
#define CAN_Mode_Silent_LoopBack	0x03  /*!< loopback combined with silent mode */

//CAN_SJW                 //1-4, Reg:0 ~ 3
//CAN_BS1                 //1-16, Reg:0 ~ 15
//CAN_BS2                 //1-8, Reg:0 ~ 7
//CAN_Prescaler         // 1-1024, Reg:0 ~ 1023
//波特率为 36/(Prescaler*(SJW+BS1+BS2))
// 36MHz is APB1 bus fixed freq, max 36MHz

//波特率设置
#define __MODE_REG(m, sjw, bs1, bs2, pres)		(((m) << 30) | (((sjw) - 1) << 24) | (((bs1) - 1) << 16) | (((bs2) - 1) << 20) | ((pres) - 1))
#define CAN_FMT_NOR_500K			__MODE_REG(CAN_Mode_Normal, 1, 14, 3, 4) // 36M/(4 * 18)
#define CAN_FMT_NOR_250K			__MODE_REG(CAN_Mode_Normal, 1, 3, 2, 24) // 36M/(24 * 6) /* UNDER TEST */
#define CAN_FMT_NOR_200K			__MODE_REG(CAN_Mode_Normal, 4, 10, 6, 9) // 36M/(9 * 20) /* UNDER TEST */
#define CAN_FMT_NOR_125K			__MODE_REG(CAN_Mode_Normal, 1, 14, 3, 16) // 36M/(16 * 18)
#define CAN_FMT_NOR_100K			__MODE_REG(CAN_Mode_Normal, 1, 3, 2, 60) // 36M/(60 * 6)

// for low speed(LOW POWER) mode, 3MHz PCLK1 低频
#define CAN_FMT_NOR_500K_LP		__MODE_REG(CAN_Mode_Normal, 1, 1, 1, 2) // 3M/(2 * 3)

// for test  回环模式下
#define CAN_FMT_LOP_500K			__MODE_REG(CAN_Mode_LoopBack, 1, 14, 3, 4) // 36M/(4 * 18)

/*    过滤器组模式 标识符和屏蔽模式       */
#define CAN_FilterMode_IdMask		0x00  /*!< identifier/mask mode */
#define CAN_FilterMode_IdList		0x01  /*!< identifier list mode */

/*过滤器组分组模式设置*/
#define CAN_FilterScale_16bit			0x00 /*!< Two 16-bit filters */
#define CAN_FilterScale_32bit			0x01 /*!< One 32-bit filter */

/*过滤器组关联接受邮箱FIFO0或FIFO1*/
#define CAN_Filter_FIFO0				0x00  /*!< Filter FIFO 0 assignment for filter x */
#define CAN_Filter_FIFO1				0x01  /*!< Filter FIFO 1 assignment for filter x */

/* Receive FIFO number */
#define CAN_FIFO0					0
#define CAN_FIFO1					1

/* Receive Interrupts 接受中断触发事件 */
#define CAN_IT_FMP0					0x00000002 /*!< FIFO 0 message pending Interrupt*/
#define CAN_IT_FF0					0x00000004 /*!< FIFO 0 full Interrupt*/
#define CAN_IT_FOV0					0x00000008 /*!< FIFO 0 overrun Interrupt*/
#define CAN_IT_FMP1					0x00000010 /*!< FIFO 1 message pending Interrupt*/
#define CAN_IT_FF1					0x00000020 /*!< FIFO 1 full Interrupt*/
#define CAN_IT_FOV1					0x00000040 /*!< FIFO 1 overrun Interrupt*/

#define CAN_IT_ERRIE				0x00008000 /*!< 错误中断使能 Error interrupt*/
#define CAN_IT_LECIE				0x00000800 /*!< 上次错误号中断使能 (Last error code interrupt enable)*/
#define CAN_IT_BOFIE				0x00000400 /*!< 离线中断使能 (Bus-off interrupt enable)*/


/* Operating Mode Interrupts */
#define CAN_IT_WKU					0x00010000 /*!<唤醒中断屏蔽Wake-up Interrupt*/
#define CAN_IT_SLK					0x00020000 /*!<睡眠标志位中断屏蔽 Sleep acknowledge Interrupt*/


// IDEXT[17:15 -> 2:0] not included, Standard form
#define CAN_ID_TO_REG16(id)			((id) << 5) | (CAN_Id_Standard << 3) | (CAN_RTR_Data << 4)

// IDEXT[17:0 -> 20:3] not included, Standard id form
#define CAN_ID_TO_REG32H(id)			((((u32)(id) << 21) | (CAN_Id_Standard << 2) | (CAN_RTR_Data << 1))>> 16)
#define CAN_ID_TO_REG32L(id)			(u16)(((u32)(id) << 21) | (CAN_Id_Standard << 2) | (CAN_RTR_Data << 1))

//
#define CAN_EID_TO_REG32H(id)			((((u32)(id) << 3) | (CAN_Id_Extended ) | (CAN_RTR_Data << 1))>> 16)
#define CAN_EID_TO_REG32L(id)			(u16)(((u32)(id) << 3) | (CAN_Id_Extended ) | CAN_RTR_Data << 1)

//16位宽度 屏蔽模式  fltn过滤器组标号  fifn 接收邮箱标号 
#define CAN_MASK_FILT16(fltn, fifn, id1, id2, msk1, msk2)				\
	{							\
		fltn,						\
		CAN_FilterMode_IdMask,	\
		CAN_FilterScale_16bit,	\
		CAN_ID_TO_REG16(id1),	\
		CAN_ID_TO_REG16(id2),	\
		CAN_ID_TO_REG16(msk1),	\
		CAN_ID_TO_REG16(msk2),	\
		fifn						\
	}

//16位宽度 标识符模式
#define CAN_LIST_FILT16(fltn, fifn, id1, id2, id3, id4)	\
	{							\
		fltn,						\
		CAN_FilterMode_IdList,	\
		CAN_FilterScale_16bit,	\
		CAN_ID_TO_REG16(id1),	\
		CAN_ID_TO_REG16(id2),	\
		CAN_ID_TO_REG16(id3),	\
		CAN_ID_TO_REG16(id4),	\
		fifn						\
	}
//32位宽度 屏蔽模式 
#define CAN_MASK_FILT32(fltn, fifn, id1, msk1)		\
	{							\
		fltn,						\
		CAN_FilterMode_IdMask,	\
		CAN_FilterScale_32bit,	\
		CAN_ID_TO_REG32H(id1),	\
		CAN_ID_TO_REG32L(id1),	\
		CAN_ID_TO_REG32H(msk1),	\
		CAN_ID_TO_REG32L(msk1),	\
		fifn						\
	}

//326位宽度 标识符列表模式
#define CAN_LIST_FILT32(fltn, fifn, id1, id2)			\
	{							\
		fltn,						\
		CAN_FilterMode_IdList,	\
		CAN_FilterScale_32bit,	\
		CAN_ID_TO_REG32H(id1), 	\
		CAN_ID_TO_REG32L(id1),	\
		CAN_ID_TO_REG32H(id2),	\
		CAN_ID_TO_REG32L(id2), 	\
		fifn						\
	}

//扩展ID  32位宽度 标识符列表
#define CAN_ELIST_FILT32(fltn, fifn, id1, id2)			\
	{							\
		fltn,						\
		CAN_FilterMode_IdList,	\
		CAN_FilterScale_32bit,	\
		CAN_EID_TO_REG32H(id1), 	\
		CAN_EID_TO_REG32L(id1),	\
		CAN_EID_TO_REG32H(id2),	\
		CAN_EID_TO_REG32L(id2), 	\
		fifn						\
	}
#endif


// PRE-DEFINE
#if (ENABLE_CAR_CAN)

#define CAN_RX_MSG_CNT				8
/*can过滤器组结构体*/
typedef struct can_filt
{
	u32 filt_num;//过滤器组号
	u32 mode;//过滤器模式CAN_FilterMode_IdMask or CAN_FilterMode_IdMask
	u32 scale;//过滤器分组设置CAN_FilterScale_16bit or CAN_FilterScale_32bit
	u32 id_hi;//列表标识符  高
	u32 id_lo;//列表标识符  低
	u32 id_hi_mask;//屏蔽标识符  高
	u32 id_lo_mask;//屏蔽标识符  低
	u32 fifo_num;//邮箱号
}can_filt_t;  //过滤器config 结构体

/* can接受发送状态结构体*/
typedef struct can_rxtx
{
	u32 configured; // 0: not configured, 1: configured
	u32 wake_level;

	u32 w_index;
	u32 r_index;
	can_msg_t *msg;

	const u32 irq_enabled; // depand on hw
	IRQn_Type rx_irq0;//接受中断类型
	CAN_TypeDef *dev;
}can_rxtx_t;//can接受发送 状态结构体
static can_rxtx_t can_info =
{
		0, // default: not configured
		LOW, // WAKE pin default state

		0,
		0,
		NULL,//can消息

		1, // interrupt disabled 初始化
 		USB_LP_CAN1_RX0_IRQn,//接受中断入口函数
		CAN1  //can1 使能
};//初始化can  一些配置

//ID
static body_can_send_block g_body_can_send_block[]=
{
	{
		"SVM_591",
		CYCLE,
		{0},
		400,
		0,
		0,
		0,
	},
};




#endif

#if ( ENABLE_CAR_CAN)
//can1接受中断服务函数
static void can_recv_isr(u32 fifo_num)
{
	u32 fmi, index;//fmi 过滤器组匹配号  index  索引
	can_msg_t *msg;
	can_rxtx_t *can_rtx = &can_info;//初始化can_info内容赋给指针 *can_rtx
	CAN_TypeDef *can = can_rtx->dev;//dev  can1
	CAN_FIFOMailBox_TypeDef *fmb;//定义接受邮箱指针（4个寄存器的配置）

	index = can_rtx->w_index;//0
	msg = &can_rtx->msg[index];//
	index++;//1
	index &= (CAN_RX_MSG_CNT - 1);//1
	can_rtx->w_index = index;//1
	fmb = &can->sFIFOMailBox[fifo_num];//接受FIFO

	/* Get the Id ，RIR标识符寄存器  */
	msg->id_ext = 0x04 & fmb->RIR;//接收FIFO邮箱标识符寄存器, 获取接受报文标识符状态：标准or扩展

	if(CAN_Id_Standard == msg->id_ext)//标准ID
	{
		msg->id = fmb->RIR >> 21;//右移21位 取出11位标准ID
	}
	else
	{
		msg->id = fmb->RIR >> 3;//扩展ID模式  右移3位 取出29位扩展ID
	}

	msg->rtr = 0x02 & fmb->RIR;//取出RTR位， 数据帧or远程帧

	/* Get the DLC ，RDTR 接收FIFO邮箱数据长度和时间戳寄存器*/
	msg->dlc = 0x0F & fmb->RDTR;//RDTR 0~3位   保存DLC

	/* Get the FMI ，RDTR 接收FIFO邮箱数据长度和时间戳寄存器*/
	fmi = 0xFF & (fmb->RDTR >> 8);//RDTR 8~15位   保存fmi过滤器匹配序号
	fmi = fmi; // avoid warning ？？？？？

	/* Get the data field */
	msg->dat.w[0] = fmb->RDLR;//接受FIFO邮箱低位数据寄存器  4个低字节
	msg->dat.w[1] = fmb->RDHR;//接受FIFO邮箱低高数据寄存器  4个高字节

	/* Release FIFOx 清除标志位，
	  通过对CAN_RF0R寄存器RFOM0置1 来释放接受FIFO邮箱*/
	if(CAN_FIFO0 == fifo_num)
	{
		can->RF0R |= CAN_RF0R_RFOM0;
	}
	else
	{
		can->RF1R |= CAN_RF1R_RFOM1;
	}
}

/*can1 接受中断服务函数*/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	can_recv_isr(CAN_FIFO0);//使用FIFO0为接受邮箱
}



void abort_request_for_mailbox(void){

	CAN1->TSR |= __BIT(7) |  __BIT(15) | __BIT(23);
	return;
}



u8 CAN_TransmitStatus(CAN_TypeDef* CANx, u8 TransmitMailbox)
{
  u32 state = 0;

  switch (TransmitMailbox)
  {
    case (CAN_TXMAILBOX_0): 
      state =   CANx->TSR &  (CAN_TSR_RQCP0 | CAN_TSR_TXOK0 | CAN_TSR_TME0);
      break;
    case (CAN_TXMAILBOX_1): 
      state =   CANx->TSR &  (CAN_TSR_RQCP1 | CAN_TSR_TXOK1 | CAN_TSR_TME1);
      break;
    case (CAN_TXMAILBOX_2): 
      state =   CANx->TSR &  (CAN_TSR_RQCP2 | CAN_TSR_TXOK2 | CAN_TSR_TME2);
      break;
    default:
      state = CAN_TxStatus_Failed;
      break;
  }
  switch (state)
  {
      /* transmit pending  */
    case (0x0): state = CAN_TxStatus_Pending;
		//dbg_msg(get_device_info(), "CAN_TxStatus_Pending\r\n");
      break;
      /* transmit failed  */
     case (CAN_TSR_RQCP0 | CAN_TSR_TME0): state = CAN_TxStatus_Failed;
      break;
     case (CAN_TSR_RQCP1 | CAN_TSR_TME1): state = CAN_TxStatus_Failed;
      break;
     case (CAN_TSR_RQCP2 | CAN_TSR_TME2): state = CAN_TxStatus_Failed;
      break;
      /* transmit succeeded  */
    case (CAN_TSR_RQCP0 | CAN_TSR_TXOK0 | CAN_TSR_TME0):state = CAN_TxStatus_Ok;
      break;
    case (CAN_TSR_RQCP1 | CAN_TSR_TXOK1 | CAN_TSR_TME1):state = CAN_TxStatus_Ok;
      break;
    case (CAN_TSR_RQCP2 | CAN_TSR_TXOK2 | CAN_TSR_TME2):state = CAN_TxStatus_Ok;
      break;
    default: state = CAN_TxStatus_Failed;
      break;
  }
  return (u8) state;
}





void enable_err_retrans_mode(u8 is){
	u32 ack = 0;
	can_rxtx_t *can_rtx = &can_info;
	CAN_TypeDef *can = can_rtx->dev;
	// exit sleep mode
	can->MCR &= ~(uint32_t)CAN_MCR_SLEEP;

	// start initialization
	can->MCR |= CAN_MCR_INRQ;

	// wait hw ack
	while (((can->MSR & CAN_MSR_INAK) != CAN_MSR_INAK) && (ack != INAK_TIMEOUT))
	{
		Iwdg_Feed();
		ack++;
	}

	if(is){
		can->MCR &= ~(uint32_t)CAN_MCR_NART; 
		
	}	
	else{
		can->MCR |= CAN_MCR_NART; // 
	}
	

	/* Request leave initialisation */
	can->MCR &= ~(uint32_t)CAN_MCR_INRQ;
	/* Wait the acknowledge */
	ack = 0;
	while (((can->MSR & CAN_MSR_INAK) == CAN_MSR_INAK) && (ack != INAK_TIMEOUT))//
	{	
		Iwdg_Feed();
		ack++;
	}
}


void reset_can(void){
	u32 ack = 0;
	can_rxtx_t *can_rtx = &can_info;
	CAN_TypeDef *can = can_rtx->dev;
	can->MCR |= CAN_MCR_RESET;
	while (((can->MSR & CAN_MCR_RESET) == CAN_MCR_RESET) && (ack != INAK_TIMEOUT))//
	{	
		Iwdg_Feed();
		ack++;
	}
	return;
}

/*
*@brief：初始化can1
*输入：fmt 波特率设置（RTR寄存器）  *filters 过滤器    
*/
static void can_init(device_info_t *dev,u32 fmt, const can_filt_t *filters, u32 filter_num, can_msg_t *msg_buf)
{
	u32 filt = 0;
	u32 ack = 0, fn;
	const can_filt_t *filtp;
	IRQn_Type rx0_irq;
	can_rxtx_t *can_rtx = &can_info;
	CAN_TypeDef *can = can_rtx->dev;

	rx0_irq = can_rtx->rx_irq0;

	can_rtx->msg = msg_buf;

	dbg_msgv(dev, "CAN%d: %d filter(s) configured\r\n", CAN_DEV0, filter_num);

	RCC->AHBENR |= __BIT(17); // enable GPIOA
	// PA11: CAN_RX, PA12: CAN_TX
	IO_INIT_AF(GPIOA, 11,GPIO_AF_9);
	IO_INIT_AF(GPIOA, 12,GPIO_AF_9);

	RCC->APB1ENR |= __BIT(25); // eanble CAN2, Bit25=CAN1, Bit26=CAN2


	if(can_rtx->configured)
	{
	//	goto exit0;
	}

	can_rtx->configured = 1;

	// Basic config

	// exit sleep mode
	can->MCR &= ~(uint32_t)CAN_MCR_SLEEP;

	/* Request initialisation */
	can->MCR |= CAN_MCR_INRQ;

	// wait hw ack
	while (((can->MSR & CAN_MSR_INAK) != CAN_MSR_INAK) && (ack != INAK_TIMEOUT))
	{
		Iwdg_Feed();
		ack++;
	}

	/* Set the time triggered communication mode */
//	can->MCR |= CAN_MCR_TTCM; // enable
	can->MCR &= ~(uint32_t)CAN_MCR_TTCM; // disable

	/* Set the automatic bus-off management */
	can->MCR |= CAN_MCR_ABOM; // enable
//	can->MCR &= ~(uint32_t)CAN_MCR_ABOM; // disable

	/* Set the automatic wake-up mode */
//	can->MCR |= CAN_MCR_AWUM; // enable
	can->MCR &= ~(uint32_t)CAN_MCR_AWUM; // disable

	/* Set the no automatic retransmission */
//	can->MCR |= CAN_MCR_NART; // 错误不自动重传
	can->MCR &= ~(uint32_t)CAN_MCR_NART; // Cause TX-HALT after 3 mailbox full!!!!!!!!!!!!!!!!

	/* Set the receive FIFO locked mode */
	can->MCR &= ~(uint32_t)CAN_MCR_RFLM; // Disable

	/* Set the transmit FIFO priority */
	can->MCR &= ~(uint32_t)CAN_MCR_TXFP; // Disable

	/* Set the bit timing register  CAN位时序寄存器,波特率选择  */
	can->BTR = fmt;

	/* Request leave initialisation */
	can->MCR &= ~(uint32_t)CAN_MCR_INRQ;

	/* Wait the acknowledge */
	ack = 0;

	while (((can->MSR & CAN_MSR_INAK) == CAN_MSR_INAK) && (ack != INAK_TIMEOUT))
	{
		Iwdg_Feed();
		ack++;
	}

	// Filter config, CAN1 & CAN2 share filter resouce

	/* Initialisation mode for the filter */
	if(filters)
	{
		CAN1->FMR |= FMR_FINIT;//CAN 过滤器主控寄存器 1为初始化模式

		while(filt < filter_num)//过滤器标号
		{
			filtp = &filters[filt];
			fn = filtp->filt_num;

			/* Filter Deactivation */
			CAN1->FA1R &= ~__BIT(fn);

			if(CAN_FilterScale_16bit == filtp->scale)
			{
				/* 16-bit scale for the filter */
				CAN1->FS1R &= ~__BIT(fn);

				 /* First 16-bit identifier and First 16-bit mask */
				 /* Or First 16-bit identifier and Second 16-bit identifier */
				 CAN1->sFilterRegister[fn].FR1 = ((0x0000FFFF & filtp->id_lo_mask) << 16) | (0x0000FFFF & filtp->id_lo);

				 /* Second 16-bit identifier and Second 16-bit mask */
				 /* Or Third 16-bit identifier and Fourth 16-bit identifier */
				 CAN1->sFilterRegister[fn].FR2 = ((0x0000FFFF & filtp->id_hi_mask) << 16) | (0x0000FFFF & filtp->id_hi);
			}
			else
			{
				/* 32-bit scale for the filter */
				CAN1->FS1R |= __BIT(fn);
				/* 32-bit identifier or First 32-bit identifier */
				dbg_msgv(dev, "CAN%d: %x filter(s) configured\r\n", CAN_DEV0,  ((0x0000FFFF & filtp->id_hi) << 16) | (0x0000FFFF & filtp->id_lo));
				CAN1->sFilterRegister[fn].FR1 = ((0x0000FFFF & filtp->id_hi) << 16) | (0x0000FFFF & filtp->id_lo);
				/* 32-bit mask or Second 32-bit identifier */
				CAN1->sFilterRegister[fn].FR2 = ((0x0000FFFF & filtp->id_hi_mask) << 16) | (0x0000FFFF & filtp->id_lo_mask);
			}

			/* Filter Mode */
			if(CAN_FilterMode_IdMask == filtp->mode)
			{
				/*Id/Mask mode for the filter*/
				CAN1->FM1R &= ~__BIT(fn);
			}
			else /* CAN_FilterInitStruct->CAN_FilterMode == CAN_FilterMode_IdList */
			{
				/*Identifier list mode for the filter*/
				CAN1->FM1R |= __BIT(fn);
			}

			/* Filter FIFO assignment  FIFO0关联过滤器组*/
			if(CAN_Filter_FIFO0 == filtp->fifo_num)
			{
				/* FIFO 0 assignation for the filter */
				CAN1->FFA1R &= ~__BIT(fn);
			}
			else if(CAN_Filter_FIFO1 == filtp->fifo_num)
			{
				/* FIFO 1 assignation for the filter */
				CAN1->FFA1R |= __BIT(fn);
			}

			/* Filter activation */
			CAN1->FA1R |= __BIT(fn);

			filt++;
		}

		/* Leave the initialisation mode for the filter */
		CAN1->FMR &= ~FMR_FINIT;
	}

	can_rtx->r_index = 0;
	can_rtx->w_index = 0;

	if(!can_rtx->irq_enabled) // not support msg rx
	{
		return;
	}

	// IRQ config

	// enable FMP0 interrupt
	can->IER |= CAN_IT_FMP0 | CAN_IT_FMP1;

	// enable IRQ
	NVIC_SetPriority(rx0_irq, 1); // call CAN device using same prio, for FIFO RX interrupt sync
	NVIC_EnableIRQ(rx0_irq);
	//MY_NVIC_Init(1,0,rx0_irq,2);

}


/*唤醒初始化  CAN_STB high到low， */
static void can_wake_init(void)
{
	// transceiver enters standby mode first (EN = 1 & STB = 1)
	IO_INIT_O_OUT(IO_CARCAN_STB, IOBIT_CARCAN_STB, GPIO_OType_PP,HIGH); // standby pin
	mdelay(3);
	// tansceiver enters normal mode
	IO_SET(IO_CARCAN_STB, IOBIT_CARCAN_STB, LOW);
	mdelay(1);
}

/*can   */
static void can_wake(void)
{
	IO_SET(IO_CARCAN_STB, IOBIT_CARCAN_STB, LOW);
	mdelay(1);
}

/*can 收发器供电电压  置高  standby 待机*/
static void can_sleep(void)
{
	///IO_SET(IO_CARCAN_EN, IOBIT_CARCAN_EN, LOW);
	IO_SET(IO_CARCAN_STB, IOBIT_CARCAN_STB, HIGH); // STB --> 1
	udelay(50);
}


/*停止can使用*/
static void can_deinit_simple(void)
{
	IRQn_Type rx0_irq;
	can_rxtx_t *can_rtx = &can_info;
	CAN_TypeDef *can = can_rtx->dev;

	if(can_rtx->irq_enabled)
	{
		rx0_irq = can_rtx->rx_irq0;

		/* Flush transmit mailboxes */
		if(can->TSR & CAN_TSR_TME)
		{
			mdelay(5);
		}
		// disable IRQ
		NVIC_DisableIRQ(rx0_irq);
		can->IER &= ~(CAN_IT_FMP0 | CAN_IT_FMP1);
	}
	//can TX RX 引脚设置为悬空状态
	IO_INIT_I(GPIOA, 11,GPIO_PuPd_NOPULL);
	IO_INIT_I(GPIOA, 12,GPIO_PuPd_NOPULL);

	// disable can module
	RCC->APB1ENR &= ~__BIT(25);
}


/* can 消息发送  */
static u32 can_msg_send(device_info_t *dev, can_msg_t *msg)
{
	u32 mailbox = ~0, c = 0;
	can_rxtx_t *can_rtx = &can_info;
	CAN_TypeDef *can = can_rtx->dev;
	CAN_TxMailBox_TypeDef *mb;

	while(1)
	{
		/* Select one empty transmit mailbox */
		if(CAN_TSR_TME0 == (can->TSR & CAN_TSR_TME0))
		{
			mailbox = 0;
			break;
		}
		else if(CAN_TSR_TME1 == (can->TSR & CAN_TSR_TME1))
		{
			mailbox = 1;
			break;
		}
		else if(CAN_TSR_TME2 == (can->TSR & CAN_TSR_TME2))
		{
			mailbox = 2;
			break;
		}
		else if(c >= 2)
		{
		//	dbg_msgv(dev, "CAN%d msg send fail, no transmit mailbox\r\n",  CAN_DEV0);
			return ~0;
		}
		Iwdg_Feed();
		c++;
		mdelay(2);
	}

	/* Set up the Id */
	//can->sTxMailBox[mailbox].TIR &= TMIDxR_TXRQ;
	mb = &can->sTxMailBox[mailbox];
	mb->TIR = 0;

	if(CAN_Id_Standard == msg->id_ext)
	{
		mb->TIR |= ((msg->id << 21) | msg->rtr);
	}
	else
	{
		mb->TIR |= ((msg->id << 3) | msg->id_ext | msg->rtr);
	}

	/* Set up the DLC */
	mb->TDTR &= 0xFFFFFFF0;
	mb->TDTR |= msg->dlc;

	/* Set up the data field */
	mb->TDLR = msg->dat.w[0];
	mb->TDHR = msg->dat.w[1];

	/* Request transmission */
	mb->TIR |= TMIDxR_TXRQ;

	return mailbox;
}


/******************************
**@brief:指针函数，指向can接受
**
**
*****************************/
static can_msg_t *can_msg_recv(device_info_t *dev)
{
	u32 rd, wr;
	can_msg_t *msg;
	can_rxtx_t *can_rtx = &can_info;
	rd = can_rtx->r_index;
	wr = can_rtx->w_index;
	if(rd == wr)
	{
		return NULL;
	}
	msg = &can_rtx->msg[rd];
	rd++;
	rd &= (CAN_RX_MSG_CNT - 1);
	can_rtx->r_index = rd;
	return msg;
}


/**@brief:车身can唤醒
**    
**    会更新 ipc cancan_wake状态位
**
***/
void car_can_wake_init(device_info_t *dev)
{
	if(dev->ci->carcan_wake) // has been waken up，已经唤醒
	{
		return;
	}
	can_wake_init();
	dev->ci->carcan_wake = 1;
	//dbg_msg(dev, "CAR CAN wakeup manually\r\n");
}

void car_can_sleep(device_info_t *dev)
{

#if ENABLE_CAR_CAN
	if(dev->ci->carcan_wake==0) // has been waken up
	{
		return;
	}

	can_sleep();

	dev->ci->carcan_wake = 0;
#endif

}

void car_can_wake(device_info_t *dev)
{
#if ENABLE_CAR_CAN
	if(dev->ci->carcan_wake) // has been waken up
	{
		return;
	}
	can_wake();
	dev->ci->carcan_wake = 1;
#endif
}



/*配置过滤器组标识符*/
#if ENABLE_CAR_CAN
// CAN1's filter num starts from 0, max 13
static const can_filt_t car_can_filters[] =
{	//海马RX ID：0x22C、0X3D4、0x539、0x6C2、0X1F7  DIAG：0x752
	CAN_LIST_FILT16(0, CAN_Filter_FIFO0, 0X1F7, 0x22C, 0X3D4, 0x539),	
	CAN_LIST_FILT16(1, CAN_Filter_FIFO0, 0x6C2, 0x752, 0x7DF, 0x652),
	CAN_MASK_FILT32(2, CAN_Filter_FIFO0, 0X100,0x780),//如果0x100-0x11f， mask=0x7e0,如果0x100~0x17f,mask=0x780

#if ENABLE_autotest
	CAN_LIST_FILT16(3, CAN_Filter_FIFO0, 0x652, 0x00, 0x00, 0x00),
#endif
};

static can_msg_t car_can_rx_msg[CAN_RX_MSG_CNT];//can消息接受结构体
#endif

#endif

// !!! CAN2 is slave CAN, CAN1 must be configured first !!!
void car_can_init(device_info_t *dev)
{
#if ENABLE_CAR_CAN
	can_init(dev,
	#if ENABLE_DIANDONGCHE
		CAN_FMT_NOR_250K,
	#else
		CAN_FMT_NOR_500K,
	#endif
		car_can_filters,
		ARRAY_SIZE(car_can_filters),
		car_can_rx_msg);//ARRAY_SIZE(car_can_filters) 数组大小代表使用的过滤器组数量
#endif
}

void car_can_reinit(device_info_t *dev)
{
#if ENABLE_CAR_CAN
	can_init(dev,
	#if ENABLE_DIANDONGCHE
		CAN_FMT_NOR_250K,
	#else
		CAN_FMT_NOR_500K,
	#endif
		car_can_filters,
		ARRAY_SIZE(car_can_filters),
		car_can_rx_msg);//ARRAY_SIZE(car_can_filters) 数组大小代表使用的过滤器组数量
#endif
}

u8 get_busoff_sts(void)
{
	return (CAN1->ESR>>2)&0x01;
	}

u8 get_buserr_sts(void)
{
	return (CAN1->ESR>>4)&0x0f;
	}
  



void car_can_send(device_info_t *dev, can_msg_t *msg)
{
#if ENABLE_CAR_CAN
	if(dev->ci->can_no_tx==0x00)
	{
		if(get_buserr_sts())
		{	
			dbg_msgv(dev, "bus err,CAN1_LEC= 0x%x\r\n",get_buserr_sts());	
		}
		can_msg_send(dev,msg); // returns 'mailbox'
	}
	return;
#endif
}

can_msg_t *car_can_recv(device_info_t *dev)
{
	can_msg_t *msg = NULL;
#if ENABLE_CAR_CAN
	msg = can_msg_recv(dev);
#endif
	return msg;
}

void car_can_deinit_simple(device_info_t *dev)
{
#if ENABLE_CAR_CAN
	can_deinit_simple();
#endif
}


u32 car_can_enabled(void)
{
	return ENABLE_CAR_CAN;
}


void can_msg_download(device_info_t *dev, message_t *msg)
{
	can_msg_t can_msg;
	can_msg.id = msg->par.w[0];//??
	can_msg.dat.w[0] = msg->par.w[1];//??
	can_msg.dat.w[1] = msg->par.w[2];
	can_msg.dlc = 8;
	can_msg.id_ext = CAN_Id_Standard;
	can_msg.rtr = CAN_RTR_Data;
	car_can_send(dev, &can_msg);

	//dbg_msgv(dev, "[CAR-CAN-TX] ID: 0x%x, DAT: %08x_%08x\r\n", can_msg.id, can_msg.dat.w[1], can_msg.dat.w[0]);
}

/**
**@brief：can消息下载》？？？
**输入：一般消息结构
**              
**/

void can_msg_Send_SWJ(device_info_t *dev, u32 id,u32* buf)
{
	can_msg_t can_msg;
	can_msg.id =id;
	can_msg.dat.w[0] = buf[0];
	can_msg.dat.w[1] = buf[1];
	can_msg.dlc = 8;
#if EXTEND_ID  
	can_msg.id_ext = CAN_Id_Extended;
#else
	can_msg.id_ext = CAN_Id_Standard;
#endif
	can_msg.rtr = CAN_RTR_Data;
	car_can_send(dev, &can_msg);
	//dbg_msgv(dev, "[CAR-CAN-TX] ID: 0x%x, DAT: %08x_%08x\r\n", can_msg.id, can_msg.dat.w[1], can_msg.dat.w[0]);
}


#if ENABLE_UPDATE_ARM_BY_CAN
void can_msg_Send_SWJ_Up(device_info_t *dev, u32 id,u32* buf)
{
	can_msg_t can_msg;

	can_msg.id =id;
	can_msg.dat.w[0] = buf[0];
	can_msg.dat.w[1] = buf[1];
	can_msg.dlc = 8;
#if EXTEND_ID  
	can_msg.id_ext = CAN_Id_Extended;
#else
	can_msg.id_ext = CAN_Id_Standard;
#endif
	can_msg.rtr = CAN_RTR_Data;
	car_can_send(dev, &can_msg);

	//dbg_msgv(dev, "[CAR-CAN-TX] ID: 0x%x, DAT: %08x_%08x\r\n", can_msg.id, can_msg.dat.w[1], can_msg.dat.w[0]);
}
#endif



static s8  find_pos_by_name(char* name){
	u8  i;
	for(i=0; i< (sizeof(g_body_can_send_block)/sizeof(body_can_send_block));i++){
		if(strcmp(name,g_body_can_send_block[i].name) == 0){
			return i;
		}	
	}
	return -1;

}

void body_can_send_fill_event_count_by_name(char* name,u8 count){
	
	body_can_send_block* p= g_body_can_send_block;
	int pos = find_pos_by_name(name);
	if(pos < 0){
		dbg_msg(get_device_info(), "no find body\n");
		return;
	}
	p[pos].event_count = count;
	return;
}
void body_can_send_fill_cur_time_by_name(char* name,u16 cur_time){
	
	body_can_send_block* p= g_body_can_send_block;
	int pos = find_pos_by_name(name);
	if(pos < 0){
		dbg_msg(get_device_info(), "no find body\n");
		return;
	}
	p[pos].cur_time = cur_time;
	return;
}
void body_can_send_fill(device_info_t *dev, u32 id,u32 *buf,u8 immediately,char* name) 
{
	can_msg_t can_msg;
	body_can_send_block* p= g_body_can_send_block;
	int pos = find_pos_by_name(name);
	if(pos < 0){
		dbg_msg(dev, "fill body can err\n");
		return;
	}
	can_msg.id =id;
	if(buf != NULL){
		can_msg.dat.w[0] = buf[0];
		can_msg.dat.w[1] = buf[1];
	}
	else{// 
		can_msg.dat.w[0] = p[pos].can_msg.dat.w[0];
		can_msg.dat.w[1] = p[pos].can_msg.dat.w[1];
	}
	can_msg.dlc = 8;
#if EXTEND_ID  
	can_msg.id_ext = CAN_Id_Extended;
#else
	can_msg.id_ext = CAN_Id_Standard;
#endif
	can_msg.rtr = CAN_RTR_Data;
	p[pos].can_msg = can_msg;
	p[pos].send_flag = immediately;
	p[pos].event_count = 5;
	if(p[pos].type == EVENT){
		p[pos].cur_time = p[pos].cycle_time;
		dev->ci->write_flash_time_flag = 0;
	}
	else{//CYCLE
		if(immediately == 1){
			p[pos].cur_time = 0;
			dev->ci->write_flash_time_flag = 0;
		}

		if(immediately == 0){
			p[pos].cur_time = p[pos].cycle_time;
		}
	}
	
	return;
	
}


//μ±msg ==null  
u32 body_can_send_by_name(can_msg_t* msg,char* name){
	can_msg_t can_msg;
	body_can_send_block* p= g_body_can_send_block;
	int pos = find_pos_by_name(name);
	if(pos < 0){
		dbg_msg(get_device_info(), "no find body\n");
		return ~0;
	}

	if(msg == NULL){
		can_msg = p[pos].can_msg;
	}
	else{
		can_msg = *msg;
	}

	return can_msg_send(get_device_info(), &can_msg);
}
void can_msg_send_handle(device_info_t *dev)
{
	u8  i,time_flag = 0;
	body_can_send_block* pdata= g_body_can_send_block;
	if(get_flag(dev,FLAG_TIMEOUT_SEND_BODY_CAN_10MS)){
		time_flag = 1; //10ms
	}else{
		time_flag = 0;
	}

	for(i=0; i< (sizeof(g_body_can_send_block)/sizeof(body_can_send_block));i++){
		if(pdata[i].send_flag > 0){ //
			
			if(pdata[i].type == EVENT){
				pdata[i].send_flag = 0;
				pdata[i].cur_time = 0;
				if(pdata[i].event_count != 0){
				pdata[i].event_count--;
				car_can_send(dev, &pdata[i].can_msg);
				}

			}else{
				if(pdata[i].send_flag == 1){
					pdata[i].cur_time = 0;
					pdata[i].send_flag = 0;
					car_can_send(dev, &pdata[i].can_msg);	
				}
				else{// send_flag=2 
					pdata[i].send_flag = 0;
					if(time_flag == 1){
						pdata[i].cur_time+=10;
					}
					if((pdata[i].cur_time >= 490) || (pdata[i].cur_time == 0)){ 
					}
					car_can_send(dev, &pdata[i].can_msg);	
				}
			}
		}
		else{
			if(time_flag){
			pdata[i].cur_time+=10;
			if((50<pdata[i].cur_time)&&(pdata[i].cur_time<300))
				{
				
					dev->ci->write_did_flag=1;//可以写did
	
					if(dev->did_t.flag)
					{
						dbg_msg(dev, "relay to write did,keep BC400 cycle normal\r\n");
						dev->did_t.flag=0;
						STMFLASH_Write(dev->did_t.ADDR,(u16*)dev->did_t.buf,dev->did_t.LEN);
						dev->ci->write_flash_time_flag = 1;
						}
	
					
					if(dev->ci->write_dtc_flag==0x01)
					{
						dev->ci->write_flash_time_flag = 1;
						dev->ci->write_dtc_flag=0;
						//dbg_msg(dev,"save dtc\r\n");
						save_dtc();
						}
					}
				else
				{
					dev->ci->write_did_flag=0;
					}
				
				if(dev->ci->write_flash_time_flag){
					if(pdata[i].type == CYCLE){
						dev->ci->write_flash_time_flag = 0;
						pdata[i].cur_time+=40;
					}
					
				}
				if(pdata[i].cur_time >= pdata[i].cycle_time){
					pdata[i].cur_time = 0;
					if(pdata[i].type == EVENT){
					if(pdata[i].event_count != 0){
						pdata[i].event_count--;
						car_can_send(dev, &pdata[i].can_msg);
					}
					}else{
						car_can_send(dev, &pdata[i].can_msg);	
					}
				}
	
				}
			}
		}
		return;
	}



/*****************************************************************************
 函 数 名  : 诊断负响应回复
 功能描述  : 诊断负响应回复
 输入参数  : 无
      输出参数  : 无
 返 回 值  : 无
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年7月20日
    作    者   : xz
    修改内容   : 新生成函数

*****************************************************************************/
void can_send_NRC(u32 SID,u32 NRC)
{	
	device_info_t *dev=get_device_info();
	
	u32 buff[2]={0};
//负响应 长度 4个字节（单帧：by0为长度  by1为7F by2为SID by3 NRC）
	buff[0] = 0x03;
	buff[0] |=0x7F<<8;
	buff[0] |=SID<<16;
	buff[0] |=NRC<<24;
    can_msg_Send_SWJ_Up(dev, AVM_Phy_Resp,buff);
}

// DESCRIPTION:  CAN0 获取总线状态
u8 CAN_ErrorStatusRegister(device_info_t *dev)
{
	if(CAN1->ESR & CAN_ESR_BS)
	{
		return	CAN_BUS_OFF_REG;
	}
	else
	{
		return	CAN_BUS_ON_REG;
	}	
}

// DESCRIPTION: network management报文发送状态
// ARGUMENT   :
// RETURN     :
// AUTHOR     : lfq
//*************************************************************************************************
uint8_t get_nm_msg_en_state(void)
{
//	 get_event(g_can_flag,(CAN_NM_MSG));
	return 0;
}

//*************************************************************************************************
// DESCRIPTION:  CAN 发送network management报文
// ARGUMENT   :
// RETURN     :
// AUTHOR     :  lfq
//*************************************************************************************************
void NM_msg_send(CAN_INFO_S stCanSendMsg)
{
	device_info_t *dev=get_device_info();
	if(get_nm_msg_en_state())
	{

	}
	else
	{
		can_msg_Send_SWJ_Up(dev,stCanSendMsg.ulID,(u32*)&stCanSendMsg.aucData[0]);//
	}

    return;
}


