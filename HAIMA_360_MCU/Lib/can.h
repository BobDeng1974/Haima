

#ifndef __CAN_H__
#define __CAN_H__
#include "miscdef.h"
// All CAN MSGs are STANDARD FRAME
// 1) AVN state info msg, DIR: OUTPUT

//#define CAN_ID_AVN_STAT			0x602  // no use

typedef union
{
	u32 w[2];

	uint64_t d;

	struct
	{
		struct
		{
			u32 resv0 : 6; // [6:0]
			u32 dat_req_fb : 2; // [8:7]
			u32 dat_chk_fb : 2; // [10:9]
			u32 ctrl_3g : 2; // [12:11]
			u32 avn_stat_fb : 2; // [14:13]
			u32 resv1 : 17; // [31:15]
		}bits;

		u32 resv0;
	}info;
}can_avn_stat_t;  //no  use

// 2) AVN stat info query msg
//#define CAN_ID_AVN_STAT_QUERY		0x603  //no use

#define CAN_Id_Standard			0x00000000  /*!< Standard Id */
#define CAN_Id_Extended			0x00000004  /*!< Extended Id */
#define CAN_RTR_Data			0x00000000  /*!< Data frame */
#define CAN_RTR_Remote			0x00000002  /*!< Remote frame */


/* 发送报文状态 */
#define CAN_TxStatus_Failed			0x00 /*!< CAN transmission failed */
#define CAN_TxStatus_Ok				0x01 /*!< CAN transmission succeeded */
#define CAN_TxStatus_Pending		0x02 /*!< CAN transmission pending */
#define CAN_TxStatus_NoMailBox		0x04 /*!< CAN cell did not provide an empty mailbox */


/*can 消息结构体 */
typedef struct can_msg
{
	u32 id; // Standard or Extend id
	u32 id_ext; // type: CAN_Id_Standard or CAN_Id_Extended
	u32 rtr; // RTR
	u32 dlc;
	union64_t dat;
}can_msg_t;

enum{
	EVENT=0,
	CYCLE,	
};


typedef struct body_can_send_block_t{
	char* name;
	u8 type; 
	can_msg_t can_msg;
	u16 cycle_time;
	u8 event_count;//事件型还需要发送的次数
	u8 send_flag;//立即发送标志  
	u16 cur_time;


}body_can_send_block;

/*can 发送消息结构体 */
typedef struct 
{
	u32 ulID; // Standard or Extend id
	u8 ucLen;
	u8 aucData[8];
}CAN_INFO_S;



#define AVM1_ID         		0x0B1//event
#define AVM2_ID             0x0B3//event
#define AVM3_ID             0x520//cycle

/** CAN Bus Status (CAN Controller is involed in bus activity or not*/
#define  CAN_ESR_BS        ((u32)__BIT(2))//20180415
#define  CAN_BUS_OFF_REG  0
#define  CAN_BUS_ON_REG   1



//test can  id   测试用的ID
#if ENABLE_TEST
#if EXTEND_ID  
#define GET_MCU_VER_ID               0x18FFFF01
#define GET_MCU_VER_ACK_ID       0x18FFFF02
#define GET_ARM_VER_ID               0x18FFFF03
#define GET_ARM_VER_ACK_ID       0x18FFFF04
#define SET_VIDEO_VIEW_ID          0x18FFFF05
#else
#define GET_MCU_VER_ID               0x7F1
#define GET_MCU_VER_ACK_ID       0x7F2
#define GET_ARM_VER_ID               0x7F3
#define GET_ARM_VER_ACK_ID       0x7F4
#define SET_VIDEO_VIEW_ID          0x7F5
          

#endif
#endif
void car_can_wake_init(device_info_t * dev);
void car_can_sleep(device_info_t * dev);
void car_can_init(device_info_t *dev);
void car_can_init1(device_info_t *dev);
void car_can_deinit(device_info_t *dev);
void car_can_deinit_simple(device_info_t * dev);
void car_can_send(device_info_t *dev, can_msg_t *msg);
can_msg_t *car_can_recv(device_info_t *dev);
void can_msg_Send_SWJ_Up(device_info_t *dev, u32 id,u32* buf);
u32 car_can_enabled(void);
void can_msg_download(device_info_t *dev, message_t *msg);
void car_can_wake_init(device_info_t * dev);
void car_can_wake(device_info_t * dev);
void can_msg_Send_SWJ(device_info_t *dev, u32 id,u32* buf);
void can_send_NRC(u32 SID,u32 NRC);
uint8_t CAN_ErrorStatusRegister(device_info_t *dev);
u8 get_busoff_sts(void);
u8 get_buserr_sts(void);
void car_can_reinit(device_info_t *dev);
void NM_msg_send(CAN_INFO_S stCanSendMsg);
void can_msg_send_handle(device_info_t *dev);
void abort_request_for_mailbox(void);

void body_can_send_fill(device_info_t *dev, u32 id,u32 *buf,u8 immediately,char* name);
void enable_err_retrans_mode(u8 is);
void reset_can(void);
u32 body_can_send_by_name(can_msg_t* msg,char* name);
u8 CAN_TransmitStatus(CAN_TypeDef* CANx, u8 TransmitMailbox);
void body_can_send_fill_event_count_by_name(char* name,u8 count);
void body_can_send_fill_cur_time_by_name(char* name,u16 cur_time);

#endif  /* __CAN_H__ */
