/*
 * nm.c
 *
 *  Created on: 2019年2月1日  
 *      Author: Administrator
 *
 *referenced from 江淮TOX osekNM（ ARCCORE open source）
 */
#include "stdio.h"
#include "nm.h"
#include "can.h"
#include "usart.h"
#include "api.h"

#define nmDebug(str)   {dbg_msg(get_device_info(), str);}
/* ================================ TYPEs     =============================== */

//NMPduType  nm_node_group[NODE_ID_MAX]={0};
NmStsCheckFunc_t NmStsCheck = NULL;//
checkAccFunc_t checkAccFunc = NULL;//

/************有些地方加大括号主要就是为了消除警告（ missing braces around initializer）**************************/
NM_ControlBlockType our_netid={

		4,
		8,
		0x100,
		0x1F,
		0,
		0,

		{
		   		0x17,
				100,
				260,
				1000,
				1500
		},
		{
				0,
				0,
				0,
				0
		},

		{
				0x17,
				0x17,
				{0}
		},

		{ {0} },
		{0},
        NM_stInit,

};


void InitDirectNMParams(NM_ControlBlockType* nm_blk,uint8 NodeId,TickType TimerTyp,TickType TimerMax, 	\
								TickType TimerError,TickType TimerWaitBusSleep,TickType TimerTx)
{
	nm_blk->nmDirectNMParams.NodeId = NodeId;
	nm_blk->nmDirectNMParams._TTyp = TimerTyp;
	nm_blk->nmDirectNMParams._TMax = TimerMax;
	nm_blk->nmDirectNMParams._TError = TimerError;
	nm_blk->nmDirectNMParams._TWbs = TimerWaitBusSleep;

	nm_blk->nmTxPdu.Source = NodeId;
}



void nmSendMessage(NM_ControlBlockType* nm_blk)
{
#if 0
    printf("ID=%x,DLC=%x   ",nm_blk->nmTxPdu.Source,8);
    printf("%x %x ",nm_blk->nmTxPdu.Destination,nm_blk->nmTxPdu.OpCode);
    for(i=0;i<6;i++)
    {

        printf(" %x",0);
    }

     printf("\r\n");

#endif // 0


	CAN_INFO_S avm_send ={0};

	avm_send.ulID = nm_blk->nmBaseAddr|nm_blk->nmTxPdu.Source;
	avm_send.ucLen = 0x08;
	avm_send.aucData[0] = nm_blk->nmTxPdu.Destination;
	avm_send.aucData[1] = nm_blk->nmTxPdu.OpCode.b;

//	CAN0_Write_nm_Msg(avm_send);can收发无缓存  
	//dbg_msgv(get_device_info(), "SEND_ID: 0x%x, DAT: %02x_%02x···\r\n", avm_send.ulID, avm_send.aucData[0], avm_send.aucData[1]);

	NM_msg_send(avm_send);
}

static void nmInitReset6(NM_ControlBlockType* nm_blk)
{
	nm_blk->nmState= NM_stInitReset;


	nm_blk->nmRxCount += 1;

	nm_blk->nmTxPdu.Destination = nm_blk->nmTxPdu.Source;
	// Initialise the NMPDU(Data,OpCode)
	nm_blk->nmTxPdu.OpCode.b = 0;
    nm_blk->nmMerker.w = 0;
	// Cancel all Alarm

	nmCancelAlarm(nm_blk,TTyp);
	nmCancelAlarm(nm_blk,TMax);
	nmCancelAlarm(nm_blk,TWbs);
	nmCancelAlarm(nm_blk,TError);
	if(nm_blk->NetworkStatus.W.NMactive)
	{
		nm_blk->nmTxCount += 1;
		// Send A alive message
		nm_blk->nmTxPdu.OpCode.b = NM_MaskAlive;
		nmSendMessage(nm_blk);
		NM_TxConformation(nm_blk);
		nmDebug("nmInitReset6:Send Alive,");
	}
	if((nm_blk->nmTxCount <= nm_blk->tx_limit)	&&
		(nm_blk->nmRxCount <= nm_blk->rx_limit))
	{
		nmSetAlarm(nm_blk,TTyp);
		nm_blk->nmState = NM_stNormal;
		nmDebug("Set TTyp, enter Normal state.\r\n");
	}
	else
	{
		nmSetAlarm(nm_blk,TError);
		nm_blk->nmState = NM_stLimphome;
		nmDebug("Set TError, Enter Limphome state.\r\n");
	}
}

static void nmInitReset5(NM_ControlBlockType* nm_blk)
{
	nmCancelAlarm(nm_blk,TError);

	nm_blk->nmRxCount = 0;
	nm_blk->nmTxCount = 0;

	nmInitReset6(nm_blk);
}


//before send nm ，config NM environment
void NM_TxConformation(NM_ControlBlockType* nm_blk)
 {
	nm_blk->nmTxCount = 0;
 	switch(nm_blk->nmState)
 	{
 		case NM_stNormal:
 		{
 			if(nm_blk->nmTxPdu.OpCode.B.Ring)
 			{
 				nmCancelAlarm(nm_blk,TTyp);
 				nmCancelAlarm(nm_blk,TMax);
 				nmSetAlarm(nm_blk,TMax);
 				if(nm_blk->nmTxPdu.OpCode.B.SleepInd)
 				{
 					if(nm_blk->NetworkStatus.W.bussleep)
 					{
 						nm_blk->nmTxPdu.OpCode.B.SleepAck = 1;
 						nm_blk->nmState = NM_stNormalPrepSleep;
 					}
 				}
 			}
 			break;
 		}
 		case NM_stNormalPrepSleep:
 		{	// 2 NMInitBusSleep
 			if(nm_blk->nmTxPdu.OpCode.B.Ring)
 			{
 //				D_Offline(NetId);
 				nmSetAlarm(nm_blk,TWbs);
 				nm_blk->nmState = NM_stTwbsNormal;
 			}
 			break;
 		}
 		case NM_stLimphome:
 		{
 			if(nm_blk->nmTxPdu.OpCode.B.Limphome)
 			{
 				nm_blk->nmMerker.W.limphome = 1;
                if(nm_blk->nmMerker.W.firstlimphome)
                    nmInitReset5(nm_blk);
 			}
 			break;
 		}
 		default:
 			break;
 	}
 }


// nmInit3 is special doesn't mean all the operation of "Leave NMBusSleep"
static void nmInit3(NM_ControlBlockType* nm_blk)
{

	// step 4:
	nm_blk->nmRxCount = 0;
	nm_blk->nmTxCount = 0;

	nmInitReset6(nm_blk);
}

// see nm253.pdf Figure 30 Start-up of the network
uint8 StartNM(NM_ControlBlockType* nm_blk)
{
	uint8_t ercd = E_OK;
	// Reset Control block, value 0 = invalid or initialize value
	//memset(nm_node_group,0,sizeof(NMPduType)*NODE_ID_MAX);

	// step 2: enter NMInit
	nm_blk->nmState= NM_stInit;

	nmInit3(nm_blk);
	return ercd;
}

uint8 StopNM(NM_ControlBlockType* nm_blk)
{
	nm_blk->nmState= NM_stOff;
	return E_OK;
}

 StatusType GotoMode(NM_ControlBlockType* nm_blk,NMModeName NewMode)
{
	if(NewMode == NM_BusSleep)
	{
		nm_blk->NetworkStatus.W.bussleep = 1;
		switch(nm_blk->nmState)
		{
			case NM_stNormal:
			{
				if(nm_blk->NetworkStatus.W.NMactive)
				{
					// NMNormal
				}
				else
				{
					nmDebug("GotoMode_BusSleep,Enter NormalPrepSleep.\r\n");
					nm_blk->nmState = NM_stNormalPrepSleep;
				}
				break;
			}
			default:
				break;
		}
	}
	else if(NewMode == NM_Awake)
	{
		nm_blk->NetworkStatus.W.bussleep = 0;
		switch(nm_blk->nmState)
		{
			case NM_stNormalPrepSleep:
			{
				nm_blk->nmState = NM_stNormal;
				break;
			}
			case NM_stTwbsNormal:
			{
				nmCancelAlarm(nm_blk,TWbs);
				nmInitReset6(nm_blk);
				break;
			}
			case NM_stBusSleep:
			{
				nmInit3(nm_blk);
				break;
			}
			case NM_stLimphomePrepSleep:
			{
				nm_blk->nmState = NM_stLimphome;
				nmSetAlarm(nm_blk,TError);
				break;
			}
			case NM_stTwbsLimphome:
			{
				nmCancelAlarm(nm_blk,TWbs);
				nm_blk->nmState = NM_stLimphome;
				nmSetAlarm(nm_blk,TError);
				break;
			}
			default:
				break;
		}
	}

	return E_OK;
}

/*节点不参与网络管理，但依旧监视网络活动NMActive→NMPassive*/
 StatusType SilentNM(NM_ControlBlockType* nm_blk)
{
	 nm_blk->NetworkStatus.W.NMactive = 0;
	return E_OK;
}
 /*节点参与网络管理，NMPassive→NMActive*/
 StatusType TalkNM(NM_ControlBlockType* nm_blk)
{
	 nm_blk->NetworkStatus.W.NMactive = 1;
	return E_OK;
}




 static NodeIdType nmDetermineLS(NodeIdType S,NodeIdType R,NodeIdType L)
  {
  	NodeIdType newL = L;
  	if(L==R)
  	{
  		newL = S;
  	}
  	else
  	{
  		if(L<R)
  		{
  			if(S<L)
  			{
  				newL = S; // SLR
  			}
  			else
  			{
  				if(S<R)
  				{
  					//LSR
  				}
  				else
  				{
  					newL = S; //LRS
  				}
  			}
  		}
  		else
  		{
  			if(S<L)
  			{
  				if(S<R)
  				{
  					//SRL
  				}
  				else
  				{
  					newL = S; // RSL
  				}
  			}
  			else
  			{
  				//RLS
  			}
  		}
  	}
  	return newL;
  }
 static  boolean nmIsMeSkipped(NodeIdType S,NodeIdType R,NodeIdType D)
  {
  	boolean isSkipped = FALSE;
  	if(D<R)
  	{
  		if(S<D)
  		{
  			// not skipped //SDR
  		}
  		else
  		{
  			if(S<R)
  			{
  				isSkipped = TRUE; // DRS
  			}
  			else
  			{
  				// not skipped //DSR
  			}
  		}
  	}
  	else
  	{
  		if(S<D)
  		{
  			if(S<R)
  			{
  				isSkipped = TRUE; //SRD
  			}
  			else
  			{
  				//RSD
  			}
  		}
  		else
  		{
  			isSkipped = TRUE; // RDS
  		}
  	}
  	return isSkipped;
  }

static  void nmNormalStandard(NM_ControlBlockType* nm_blk,NMPduType* NMPDU)
 {
	 nm_blk->nmRxCount = 0;
	// nm_node_group[NMPDU->Source].Destination = NMPDU->Destination;
	// nm_node_group[NMPDU->Source].OpCode = NMPDU->OpCode;
 	if(NMPDU->OpCode.B.Limphome)
 	{
 		// add sender to config.limphome
 		// nm_node_group[NMPDU->Source].Destination = NMPDU->Source;
 	}
 	else
 	{

 		// determine logical successor
 		nm_blk->nmTxPdu.Destination = nmDetermineLS(NMPDU->Source,	\
 											nm_blk->nmDirectNMParams.NodeId,	\
											nm_blk->nmTxPdu.Destination);

       // printf("dest=%x\r\n",nm_blk->nmTxPdu.Destination);
 		if(NMPDU->OpCode.B.Ring)
 		{
 			nmCancelAlarm(nm_blk,TTyp);
 			nmCancelAlarm(nm_blk,TMax);

 			if((NMPDU->Destination == nm_blk->nmDirectNMParams.NodeId) // to me
 				|| (NMPDU->Destination == NMPDU->Source)) // or D = S
 			{
 				nmSetAlarm(nm_blk,TTyp);
 				// Do Ring Data indication


 			}
 			else
 			{
 				nmSetAlarm(nm_blk,TMax);
 				//printf("NMPDU->Source=%x\r\n",NMPDU->Source);
 				if(nmIsMeSkipped(NMPDU->Source,nm_blk->nmDirectNMParams.NodeId,NMPDU->Destination))
 				{
 					if(nm_blk->NetworkStatus.W.NMactive)
 					{
 						nm_blk->nmTxPdu.OpCode.b= NM_MaskAlive;
 						nm_blk->nmTxPdu.Destination= nm_blk->nmDirectNMParams.NodeId;
 						// printf("dest=%x\r\n",nm_blk->nmTxPdu.Destination);
 						if(nm_blk->NetworkStatus.W.bussleep)

 						{
 							nm_blk->nmTxPdu.OpCode.B.SleepInd = 1;
 						}
						nmSendMessage(nm_blk);
						NM_TxConformation(nm_blk);
		//				nmDebug("nmNormalStandard deal\r\n");
 					}
 				}
 			}
 		}
 	}
 }



void NM_BusErrorIndication(NM_ControlBlockType* nm_blk)
 {

 	nmSetAlarm(nm_blk,TError);//等待TError结束发送Limphome报文，发送成功，开始NM 
 	nm_blk->nmState = NM_stLimphome;
 }

uint8 NM_Rx_Judge_ID_inrange(NM_ControlBlockType* nm_blk,uint32 s_id,uint32 d_id)
{
	if(((s_id)<nm_blk->nmBaseAddr)||\
			(s_id>(nm_blk->nmBaseAddr+nm_blk->nmAddrRange)))
		{
		 return 0;
		}
		if((d_id<nm_blk->nmBaseAddr)||\
				(d_id>(nm_blk->nmBaseAddr+nm_blk->nmAddrRange)))
			{
			 return 0;
			}

		return 1;
}

void NM_RxIndication(NM_ControlBlockType* nm_blk,NMPduType* NMPDU)
 {


	uint8 accSts;
 	switch(nm_blk->nmState)
 	{
 		case NM_stNormal:
 		case NM_stNormalPrepSleep:
 		{
 			nmNormalStandard(nm_blk,NMPDU);
 			if(NMPDU->OpCode.B.SleepAck)
 			{
 				if(nm_blk->NetworkStatus.W.bussleep)
 				{ // 2 NMInitBusSleep
// 					D_Offline(NetId);
 					nmSetAlarm(nm_blk,TWbs);
 					nm_blk->nmState = NM_stTwbsNormal;
 				}
 			}
 			// Special process for NM_stNormalPrepSleep only
 			if(NM_stNormalPrepSleep == nm_blk->nmState)
 			{
 				if(NMPDU->OpCode.B.SleepInd)
 				{
 				}
 				else
 				{
 					nm_blk->nmState = NM_stNormal;
 				}
 			}
 			break;
 		}
 		case NM_stTwbsNormal:
 		{
// 		    printf("sleepind=%d\r\n",NMPDU->OpCode);
 			if(NMPDU->OpCode.B.SleepInd)
 			{  	// = 1
 			}
 			else
 			{	// = 0
 				nmCancelAlarm(nm_blk,TWbs);
 				nmInitReset6(nm_blk);
 			}
 			break;
 		}
 		case NM_stLimphome:
 		{
 			if(nm_blk->NetworkStatus.W.NMactive)
 			{
 				if(nm_blk->nmMerker.W.limphome)
 				{
 					if(NMPDU->OpCode.B.SleepAck)  //nm_blk->NetworkStatus.W.bussleep
 					{
// 						D_Offline(NetId);
 						if(checkAccFunc != NULL)
 							accSts = checkAccFunc();
 						if(accSts == 0)
 						{
 							nmSetAlarm(nm_blk,TWbs);
 							nmCancelAlarm(nm_blk,TError);
 							nm_blk->nmState = NM_stTwbsLimphome;
 						}
 						else
 							nmInitReset5(nm_blk);
 					}
 					else
 					{
 						nmInitReset5(nm_blk);
 					}
 				}
 				else
 				{
 					if(NMPDU->OpCode.B.SleepAck)
 					{
 					    if((nm_blk->nmMerker.W.limphome == 0)&&(nm_blk->nmRxCount > nm_blk->rx_limit))
 					    {
 					        nm_blk->nmMerker.W.firstlimphome = 1;
 					    }
 					    else
 					    {
// 							D_Offline(NetId);
 					    	nmSetAlarm(nm_blk,TWbs);
 					    	nm_blk->nmState = NM_stTwbsLimphome;
 					    }
 					}
 					else
 					{
 					    if((nm_blk->nmMerker.W.limphome == 0)&&(nm_blk->nmRxCount > nm_blk->rx_limit))
 					    {
 					        nm_blk->nmMerker.W.firstlimphome = 1;
 					    }
                        else
 						    nmInitReset5(nm_blk);
 					}
 				}
 			}
 			else
 			{
 				if(nm_blk->nmMerker.W.limphome)
 				{
 					if( (nm_blk->NetworkStatus.W.bussleep) && (NMPDU->OpCode.B.SleepAck))
 					{
// 						D_Offline(NetId);
 						nmSetAlarm(nm_blk,TWbs);
 						nm_blk->nmState = NM_stTwbsLimphome;
 					}
 					else
 					{
 						nmInitReset5(nm_blk);
 					}
 				}
 				else
 				{
 					if(NMPDU->OpCode.B.SleepAck)
 					{
// 						D_Offline(NetId);
 						nmSetAlarm(nm_blk,TWbs);
 						nm_blk->nmState = NM_stTwbsLimphome;
 					}
 					else
 					{
 						nmInitReset5(nm_blk);
 					}
 				}
 			}
 			break;
 		}
 		case NM_stLimphomePrepSleep:
 		{
 			if(0 == NMPDU->OpCode.B.SleepAck)
 			{
 				nmInit3(nm_blk);
 			}
 			else
 			{
 				//nm_blk->nmState = NM_stLimphome;
 				nmSetAlarm(nm_blk,TWbs);
 				nmCancelAlarm(nm_blk,TError);
 				nm_blk->nmState = NM_stTwbsLimphome;
 			}
 			break;
 		}
 		case NM_stTwbsLimphome:
 		{
 			if(0 == NMPDU->OpCode.B.SleepInd)
 			{
 				nmCancelAlarm(nm_blk,TWbs);
 				nm_blk->nmState = NM_stLimphome;
 				nmSetAlarm(nm_blk,TError);
 			}
 			else
 			{
 			}
 			break;
 		}
 		default:
 			break;
 	}
 }
 void NM_WakeupIndication(NM_ControlBlockType* nm_blk)
 {
 	if(NM_stBusSleep == nm_blk->nmState)
 	{  // 3 NMInit
 		//OK Wakeup
// 		D_Init(NetId,BusAwake);
 		nmInit3(nm_blk);
 	}
 }



static void nmNormalMain(NM_ControlBlockType* nm_blk)
 {
 	if(nmIsAlarmStarted(nm_blk,TTyp))
 	{
 		nmSingalAlarm(nm_blk,TTyp);
 		if(nmIsAlarmTimeout(nm_blk,TTyp))
 		{
 			nmCancelAlarm(nm_blk,TTyp);
 			nmCancelAlarm(nm_blk,TMax);
 			nmSetAlarm(nm_blk,TMax);
 			if(nm_blk->NetworkStatus.W.NMactive)
 			{
 				nm_blk->nmTxPdu.OpCode.b = NM_MaskRing;
 				if(nm_blk->NetworkStatus.W.bussleep)
 				{
 					nm_blk->nmTxPdu.OpCode.B.SleepInd = 1;
 				}
 				nm_blk->nmTxCount ++;
				nmSendMessage(nm_blk);
				NM_TxConformation(nm_blk);
			//	nmDebug("nmNormalMain deal\r\n");
 			}
 			else
 			{
 			}
 			if(nm_blk->nmTxCount > nm_blk->tx_limit)
 			{
 				nm_blk->nmState = NM_stLimphome;
 				nmSetAlarm(nm_blk,TError);
 			}
 			else
 			{
 				if(nm_blk->nmMerker.W.stable)
 				{
 					nm_blk->NetworkStatus.W.configurationstable = 1;
 				}
 				else
 				{
 					nm_blk->nmMerker.W.stable = 1;
 				}
 			}
 		}
 	}

 	if(nmIsAlarmStarted(nm_blk,TMax))
 	{
 		nmSingalAlarm(nm_blk,TMax);
 		if(nmIsAlarmTimeout(nm_blk,TMax))
 		{
 			nmCancelAlarm(nm_blk,TMax);
 			nmInitReset6(nm_blk);
 		}
 	}
 }
static void nmLimphomeMain(NM_ControlBlockType* nm_blk)
 {
 	if(nmIsAlarmStarted(nm_blk,TError))
 	{
 		nmSingalAlarm(nm_blk,TError);
 		if(nmIsAlarmTimeout(nm_blk,TError))
 		{
 			nmCancelAlarm(nm_blk,TError);
// 			D_Online(NetId);
 			nm_blk->nmTxPdu.OpCode.b = NM_MaskLimphome;
 			if(nm_blk->NetworkStatus.W.bussleep)
 			{
 				nmSetAlarm(nm_blk,TMax);
 				nm_blk->nmTxPdu.OpCode.B.SleepInd =1;
 				nm_blk->nmState = NM_stLimphomePrepSleep;
 			}
 			else
 			{
 				nmSetAlarm(nm_blk,TError);
 			}
 			if(nm_blk->NetworkStatus.W.NMactive)
 			{
 				//NM_ControlBlock[NetId].nmTxPdu.Destination = NM_ControlBlock[NetId].nmDirectNMParams.NodeId;
 				nmSendMessage(nm_blk);
				NM_TxConformation(nm_blk);
			//	nmDebug("nmLimphomeMain deal\r\n");
 			}
 		}
 	}
 }
 static void nmNormalPrepSleepMain(NM_ControlBlockType* nm_blk)
 {
 	if(nmIsAlarmStarted(nm_blk,TTyp))
 	{
 		nmSingalAlarm(nm_blk,TTyp);
 		if(nmIsAlarmTimeout(nm_blk,TTyp))
 		{
 			nmCancelAlarm(nm_blk,TTyp);
 			if(nm_blk->NetworkStatus.W.NMactive)
 			{
 				// Send ring message with set sleep.ack bit
 				nm_blk->nmTxPdu.OpCode.b = NM_MaskRing|NM_MaskSI|NM_MaskSA;
				nmSendMessage(nm_blk);
				NM_TxConformation(nm_blk);
				//nmDebug("nmNormalPrepSleepMain deal\r\n");
 			}
 		}
 	}

 	if(nmIsAlarmStarted(nm_blk,TMax))
 	 	{
 	 		nmSingalAlarm(nm_blk,TMax);
 	 		if(nmIsAlarmTimeout(nm_blk,TMax))
 	 		{
 	 			nmCancelAlarm(nm_blk,TMax);
 	 			nmInitReset6(nm_blk);
 	 		}
 	 	}
 }
static void nmBusSleep(NM_ControlBlockType* nm_blk)
 {
// 	D_Init(NetId,BusSleep);
 	nmDebug("nmBusSleep \r\n");

	 nm_blk->nmState = NM_stBusSleep;
 }
static void nmNormalTwbsMain(NM_ControlBlockType* nm_blk)
 {
 	if(nmIsAlarmStarted(nm_blk,TWbs))
 	{
 		nmSingalAlarm(nm_blk,TWbs);
 		if(nmIsAlarmTimeout(nm_blk,TWbs))
 		{
 			nmCancelAlarm(nm_blk,TWbs);
 			nmBusSleep(nm_blk);
 		}
 	}
 }
static void nmLimphomePrepSleepMain(NM_ControlBlockType* nm_blk)
 {
 	if(nmIsAlarmStarted(nm_blk,TMax))
 	{
 		nmSingalAlarm(nm_blk,TMax);
 		if(nmIsAlarmTimeout(nm_blk,TMax))
 		{
 			nmCancelAlarm(nm_blk,TMax);
 			// 7 NMInitBusSleep
// 			D_Offline(NetId);
 			nmSetAlarm(nm_blk,TWbs);
 			nm_blk->nmState = NM_stTwbsLimphome;
		//	nmDebug("nmLimphomePrepSleepMain,set Twbs \r\n");
 		}
 	}
 }
 static void nmTwbsLimphomeMain(NM_ControlBlockType* nm_blk)
 {
	
 	if(nmIsAlarmStarted(nm_blk,TWbs))
 	{ 
 		nmSingalAlarm(nm_blk,TWbs);
 		if(nmIsAlarmTimeout(nm_blk,TWbs))
 		{
 			nmCancelAlarm(nm_blk,TWbs);
 			nmBusSleep(nm_blk);
 		}
 	}
 }
void NM_MainTask(NM_ControlBlockType* nm_blk)
 {



 		switch(nm_blk->nmState)
 		{
 			case NM_stNormal:
 				nmNormalMain(nm_blk);		
 				break;
 			case NM_stLimphome:
 				nmLimphomeMain(nm_blk);
 				break;
 			case NM_stNormalPrepSleep:
 				nmNormalPrepSleepMain(nm_blk);
 				break;
 			case NM_stTwbsNormal:
 				nmNormalTwbsMain(nm_blk);
 				break;
 			case NM_stLimphomePrepSleep:
 				nmLimphomePrepSleepMain(nm_blk);
 				break;
 			case NM_stTwbsLimphome:
 				nmTwbsLimphomeMain(nm_blk);
 				break;
 			default:
 				break;
 		}

 		/*{

 				if(NmStsCheck != NULL)
 					NmStsCheck();
 		}*/

 }

int get_nm_running_state(void)
{
	if((our_netid.nmState == NM_stTwbsNormal) ||(our_netid.nmState == NM_stTwbsLimphome)||(our_netid.nmState == NM_stBusSleep))
		return 1;
	else
		return 0;
}

NMStateType get_nm_sts(void)
{
	return our_netid.nmState;
}
