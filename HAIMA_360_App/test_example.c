/*
 * 外部App  例子
 * 本demo中共有两个线程，一个是主线程，一个是回调函数所在的子线程，子线程是负责接收内部消息的。
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "tshcommunication.h"

static int running = 1;

//回调函数
void msg_process_cb(void* param, void* data, uint32_t len)
{ // 此为子线程
   (void)param;
   stTlvPacket_t decodePkt;
   memset(&decodePkt, 0, sizeof(decodePkt));
   TlvPacket_Init(&decodePkt, data, len);
   uint16_t type = (uint16_t)-1;
   if (!TlvPacket_ReadHead(&decodePkt, &type))
   {
       printf("inner msg read head error.\n");
       return;
   }
   
   uint16_t tag;
   uint16_t length;
   char* data_temp = NULL;
   if (type != EN_TSH_MSG_REQUEST)
       return;
   
   while ((data_temp = TlvPacket_ReadBody(&decodePkt, &tag, &length)))
   {
       switch (tag)
       {
       case EN_TSH_FIELD_INFORM_CURRAPP:
       {
           if (length != sizeof(uint32_t))
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
           if (length != sizeof(stViewingMode_t))
               return;
           
           stViewingMode_t* vm = (stViewingMode_t*)data_temp;
           // ...
           // ...
           break;
       }
       case EN_TSH_FIELD_INFORM_RECORDINGSTATE:
       {
           if (length != sizeof(uint32_t))
               return;
           
           
           break;
       }
       case EN_TSH_FIELD_INFORM_VIDEOEXCEPTION:
       {
           if (length != sizeof(uint32_t))
               return;

           
           break;
       }
       case EN_TSH_FIELD_INFORM_CALIRESULT:
       {
           if (length != sizeof(uint32_t))
               return;  

           
           break;
       }
       }
   }
}

static void signalCatcher(int signo)
{
    printf("catch a signal no=%d\n", signo);
    running = 0;
}

int main()
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, signalCatcher);
    // 初始化
    int res;
    while (running)
    {
        res = TshComm_Init(EN_APP_OUTER, msg_process_cb, NULL);
        if (0 == res)
            break;
        printf("TshComm_Init failed. errNo=%u\n", TshComm_GetInnerErrorCode());
        usleep(1000000);
    }
    if (!TshComm_IsReady())
        return 1;
    
    // 初始化成功
    //  设置通知功能
    uint32_t nFlag = 0;
    //nFlag |= EN_TSH_INFORM_VIEWING_ANGLE;
    //nFlag |= EN_TSH_INFORM_RECORDING_STATE;
    TshComm_SetInformFlag(nFlag);
    
    // 设置车速
    float speed = 20.f;
    TshComm_SetVehicleSpeed(speed);
    
    // 设置方向盘转角
    float wheel_angle = -20.f;
    TshComm_SetVehicleWheelAngle(wheel_angle);
    
    // 设置时间
    TshComm_SetSysTime(2017, 1, 16, 23, 59, 0);
    
    // 设置语言
    TshComm_SetLangInfo(EN_TSH_LANG_SIMPLIFIED_CHINESE);
    
    // 设置车辆信号
    TshComm_SetVehicleSignal(EN_TSH_VEHICLE_SIGNAL_DRIVER_DOOR, TRUE);
    TshComm_SetVehicleSignal(EN_TSH_VEHICLE_SIGNAL_TRUNK_DOOR, FALSE);
    
    //  设置标定
    TshComm_SetCaliFieldOfView(EN_TSH_CALI_FIELD_WIDE); // 执行标定
    
    // 设置切换界面
    TshComm_SetDirectEnterApp(EN_TSH_360APP_TSHCONF);
    
    // 设置显示视角
    TshComm_SetViewingMode(EN_TSH_VIEW_2D_FRONT_AND_SPLICING, 0.f); // 2D
    TshComm_SetViewingMode(EN_TSH_VIEW_3D_SPECIFIC_ANGLE_AND_SPLICING, 40.f); // 3D  自定义角度
    
    // 设置录像功能
    TshComm_SetRecording(EN_TSH_RECORDING_COMMON, TRUE);
    
    // 设置视频
    TshComm_SetVideoMode(EN_TSH_VIDEO_CLOSE);
    
    // 设置恢复默认配置
    TshComm_SetRecoverConfig(TRUE); // 谨慎使用
    
    // 获取版本信息
    stVersion_t vers;
    TShComm_GetVersionInfo(&vers);
    printf("highV=%u lowV=%u year=%d month=%d mday=%d\n", vers.nHighV, vers.nLowV, vers.nYear, vers.nMonth, vers.nMday);
    
    // 获取四个摄像头状态
    uint32_t nCameraState = 0;
    nCameraState = TshComm_GetCamerasState();
    if (nCameraState & EN_TSH_CAMERA_1)
        printf("cam 1 ok.\n");
    else
        printf("cam 1 not ok\n");
    if (nCameraState & EN_TSH_CAMERA_2)
        printf("cam 2 ok.\n");
    else
        printf("cam 2 not ok\n");
    if (nCameraState & EN_TSH_CAMERA_3)
        printf("cam 3 ok.\n");
    else
        printf("cam 3 not ok\n");
    if (nCameraState & EN_TSH_CAMERA_4)
        printf("cam 4 ok.\n");
    else
        printf("cam 4 not ok\n");
        
    
    // 释放
    TshComm_Exit();
}


