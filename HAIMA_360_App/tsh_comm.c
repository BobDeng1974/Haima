#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "tsh_comm.h"
#include "imx6_com.h"
static volatile uint32_t CurrViewMode = 0x01;

uint32_t GetCurrViewMode(void)
{
	return CurrViewMode;
}


/*---------------------------------------------------------------
* ����ԭ�ͣ�void SG_SetViewingMode(uint32_t viewMode, float angle)
* �������ܣ�������ʾģʽ 
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/	
void SG_SetViewingMode(uint32_t viewMode, float angle)
{
	if(viewMode == CurrViewMode){
		return;
	}
    /* CurrViewMode = viewMode; */    
	printf("set view:%d\n",viewMode);
	TshComm_SetViewingMode(viewMode, angle);
	return;
}



/*---------------------------------------------------------------
* ����ԭ�ͣ�void msg_process_cb(void* param, void* data, uint32_t len)
* �������ܣ��ص�����
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/	
void msg_process_cb(void* param, void* data, uint32_t len)
{ // ��Ϊ���߳�
   // (void)param;
   stTlvPacket_t decodePkt;
   Msg msg;
   //static unsigned char mode = 0,mode_last = 0;
   memset(&decodePkt, 0, (size_t)sizeof(decodePkt));
   TlvPacket_Init(&decodePkt, data, len);
   uint16_t type = 0xFFFF;  //(uint16_t)-1;
   if (!TlvPacket_ReadHead(&decodePkt, &type))
   {
       printf("inner msg read head error.\n");
       return;
   }
   uint16_t tag;
   uint16_t length;
   char* data_temp = NULL;
   if (type != EN_TSH_MSG_INFORM)
   {
   		printf("skip=%d!\n",type);
        return;
   }
   while ((data_temp = TlvPacket_ReadBody(&decodePkt, &tag, &length)))
   {
       switch (tag)
       {
       case EN_TSH_FIELD_INFORM_CURRAPP:
       {
           if (length != (uint16_t)sizeof(uint32_t))
               return;
           uint32_t nApp = *((uint32_t*)data_temp);
           if (EN_TSH_360APP_TSHMAIN == nApp)
           {
               // to do
           }
           else if (EN_TSH_360APP_TSHCALI == nApp)
           {
               // to do
           }
           else if (EN_TSH_360APP_TSHCONF == nApp)
           {
               // to do
           }
           else if (EN_TSH_360APP_TSHPLAY == nApp)
           {
               // to do
           }
           else
           {
               // do nothing
           }
           
           break;
       }
       case EN_TSH_FIELD_INFORM_CURRVIEWINGANGLE:
       {
           if (length != (uint16_t)sizeof(stViewingMode_t))
               return;
           
           stViewingMode_t* vm = (stViewingMode_t*)data_temp;
     	    CurrViewMode = vm->nMode;
			printf("tsh back function CurrViewMode=%d\n",CurrViewMode);	
            break;
       }
       case EN_TSH_FIELD_INFORM_RECORDINGSTATE:
       {
           if (length != (uint16_t)sizeof(uint32_t))
               return;
           
           
           break;
       }
       case EN_TSH_FIELD_INFORM_VIDEOEXCEPTION:
       {
           if (length != (uint16_t)sizeof(uint32_t))
               return;

           
           break;
       }
       case EN_TSH_FIELD_INFORM_CALIRESULT:
       {
       #if 0
           if (length != (uint16_t)sizeof(uint32_t))
               return;  
           uint32_t camera_status = *((uint32_t*)data_temp);
           printf("camera_status ==> %d\n", camera_status);
           avmCalibrationPara.caliFlag = 1;
           switch (camera_status)
           {
           case CALI_ERR_0:
            avmCalibrationPara.caliResult = CAMERA_CALI_NONE; 
                break;
           case CALI_ERR_17:
           case CALI_ERR_33:
           case CALI_ERR_49:
           case CALI_ERR_65:
            avmCalibrationPara.caliResult = CAMERA_CALI_F; 
               break;
           case CALI_ERR_18:
           case CALI_ERR_34:
           case CALI_ERR_50:
           case CALI_ERR_66:
            avmCalibrationPara.caliResult = CAMERA_CALI_B; 
               break;
           case CALI_ERR_19:
           case CALI_ERR_35:
           case CALI_ERR_51:
           case CALI_ERR_67:
            avmCalibrationPara.caliResult = CAMERA_CALI_L; 
               break;
           case CALI_ERR_20:
           case CALI_ERR_36:
           case CALI_ERR_52:
           case CALI_ERR_68:
            avmCalibrationPara.caliResult = CAMERA_CALI_R; 
               break;
            default:
               printf("calibration fail!\n");
               break;
           }
		   #else
		   if (length != sizeof(uint32_t))
               return;  
		   msg.MsgType = EN_TSH_FIELD_INFORM_CALIRESULT;
		   msg.MsgData = *(uint32_t*)data_temp;
		   EnQueue(&MsgQueue,&msg);
		   #endif
           
           break;
       }
	   case EN_TSH_FIELD_INFORM_ADAS_STATE:
       case EN_TSH_FIELD_INFORM_CAMERAS_STATE:
            if(length != (uint16_t)sizeof(uint32_t))
                return;
            msg.MsgType = tag;
			msg.MsgData = *(uint32_t*)data_temp;
			EnQueue(&MsgQueue,&msg);
        break;

		
		
        /* case EN_TSH_FIELD_CARMODELANDCOLOR: */
        /* { */
        /*     printf("length ==> %d\n", length); */
        /*     if (length != sizeof(stCarModelAndColor_t)) */
        /*         return; */
        /*     stCarModelAndColor_t* carModeAndColor = (stCarModelAndColor_t*)data_temp; */
        /*     avmParaSetInfo.carColour = carModeAndColor->nColor; */
        /*     printf("In call back function, the color : %d\n", avmParaSetInfo.carColour); */
        /*     break; */
        /* } */
        default:
            break;
       }
   }
}

