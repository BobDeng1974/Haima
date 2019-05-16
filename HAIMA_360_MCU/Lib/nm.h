/*
 * nm.h
 *
 *  Created on: 2018年7月1日
 *      Author: Administrator
 *
 */

#ifndef NM_H_
#define NM_H_
typedef unsigned int       uint32;
typedef unsigned char       uint8;
typedef unsigned int			TickType;

typedef uint8 StatusType;
typedef uint8 NetIdType;
typedef uint8 NodeIdType;
typedef unsigned char       boolean;


/* Values for Std_ReturnType */
#define E_OK            0
#define E_NOT_OK        1

// Macros for OpCode
#define NM_MaskAlive   	 	0x01
#define NM_MaskRing   	 	0x02
#define NM_MaskLimphome 	0x04
#define NM_MaskSI        		0x10   // sleep ind
#define NM_MaskSA        		0x20   // sleep ack

/* ================================ MACROs    =============================== */


/* @ nm253.pdf 4.4.3.1 P98 */
typedef enum
{
	NM_BusSleep = 1,
	NM_Awake
}NMModeName;


#pragma anon_unions
typedef struct
{
		union
		{
			uint32 w;
			struct{
				unsigned NMactive:1;
				unsigned bussleep:1;
				unsigned configurationstable:1;
			}W;
		};
}NetworkStatusType;

typedef enum
{
	NM_stOff = 0,
	NM_stInit,
	NM_stInitReset,
	NM_stNormal,
	NM_stNormalPrepSleep,
	NM_stTwbsNormal,
	NM_stBusSleep,
	NM_stLimphome,
	NM_stLimphomePrepSleep,
	NM_stTwbsLimphome,
	NM_stOn // in fact if not Off then ON.
	// ...  and so on
}NMStateType;

typedef struct
{
	uint8 Source;
	uint8 Destination;
	union
	{
		uint8 b;
		struct {
			uint8 Alive     :1;
			uint8 Ring      :1;
			uint8 Limphome  :1;
			uint8 reserved1 :1;
			uint8 SleepInd  :1;
			uint8 SleepAck  :1;
			uint8 reserved2 :2;
		}B;
	}OpCode;

}NMPduType;


typedef struct
{
    uint8 rx_limit;
	uint8 tx_limit;
	uint32 nmBaseAddr;
	uint8  nmAddrRange;//最大节点ID
	uint8 nmRxCount;
	uint8 nmTxCount;
	
	struct{
		uint8 NodeId;
		TickType _TTyp;
		TickType _TMax;
		TickType _TError;
		TickType _TWbs;
	}nmDirectNMParams;

	struct{
		TickType _TTyp;
		TickType _TMax;
		TickType _TError;
		TickType _TWbs;
	}Alarm;

	NMPduType nmTxPdu;
	NetworkStatusType NetworkStatus;
	
	union
		{
		uint32 w;
		struct
		{
			unsigned stable : 1;
			unsigned limphome : 1;
      		unsigned firstlimphome : 1;
			}W;
		}nmMerker; // TODO: what's the meaning?

		NMStateType nmState;
}NM_ControlBlockType;

// Alarm Management
#define nmSetAlarm(nm_blk,Timer)				\
do										\
{										\
	nm_blk->Alarm._##Timer = 1+nm_blk->nmDirectNMParams._##Timer;	\
}while(0)

// signal the alarm to process one step/tick forward 配合中断1ms一次，达到计时效果
#define nmSingalAlarm(nm_blk,Timer)								\
do															\
{															\
	 if(nm_blk->Alarm._##Timer > 1)			\
	 {														\
		(nm_blk->Alarm._##Timer)--;			\
	 }														\
}while(0)

#define nmIsAlarmTimeout(nm_blk,Timer)  (1 == nm_blk->Alarm._##Timer)

#define nmIsAlarmStarted(nm_blk,Timer)	 (0 !=nm_blk->Alarm._##Timer)

#define nmCancelAlarm(nm_blk,Timer)					\
do												\
{												\
	nm_blk->Alarm._##Timer = 0;	\
}while(0)


/*****************相关函数回调**************************/
typedef void (*NmStsCheckFunc_t)(void);

typedef uint8 (*checkAccFunc_t)(void);//定义函数指针指向一个输入参数为void返回值为uint8的函数

extern NmStsCheckFunc_t NmStsCheck;
extern checkAccFunc_t checkAccFunc;
/**********************************************
*一般来说结构体定义已经决定参数
该函数其实一般不会调用
*********************************************/
void InitDirectNMParams(NM_ControlBlockType* nm_blk,uint8 NodeId,TickType TimerTyp,TickType TimerMax, 	\
								TickType TimerError,TickType TimerWaitBusSleep,TickType TimerTx);

/************不往can总线上发送网络报文*****************************/
StatusType SilentNM(NM_ControlBlockType* nm_blk);
/************往can总线上发送网络报文*****************************/
StatusType TalkNM(NM_ControlBlockType* nm_blk);

/************nm ON*****************************/
 uint8 StartNM(NM_ControlBlockType* nm_blk);
 /************nm OFF*****************************/
 uint8 StopNM(NM_ControlBlockType* nm_blk);

 /************can休眠或者唤醒状态调用该函数*******************/
StatusType GotoMode(NM_ControlBlockType* nm_blk,NMModeName NewMode);

/************can从normal到busoff状态转换时调用该函数*******************/
void NM_BusErrorIndication(NM_ControlBlockType* nm_blk);
/************网络管理报文发送成功调用函数*******************/
void NM_TxConformation(NM_ControlBlockType* nm_blk);
/************判断接收到的ID是否在是网络管理ID范围*******************/
uint8 NM_Rx_Judge_ID_inrange(NM_ControlBlockType* nm_blk,uint32 s_id,uint32 d_id);
/************网络管理报文接收处理*******************/
void NM_RxIndication(NM_ControlBlockType* nm_blk,NMPduType* NMPDU);
/************can网络唤醒调用函数*******************/
 void NM_WakeupIndication(NM_ControlBlockType* nm_blk);
 /***************主处理函数 1ms运行一次****************************************/
void NM_MainTask(NM_ControlBlockType* nm_blk);
/******************获取网络管理是否处于准备休眠和休眠状态****************************************/
int get_nm_running_state(void);
/******************获取网络管理当前状态****************************************/
NMStateType get_nm_sts(void);
#endif /* NM_H_ */
