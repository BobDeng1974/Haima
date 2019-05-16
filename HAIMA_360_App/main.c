/******************************************************************************

                  版权所有 (C), 2018-2028, 赛格导航科技股份有限公司

 ******************************************************************************
  文 件 名   : main.c
  版 本 号   : 初稿
  作    者   : ZD
  生成日期   : 2018年6月15日
  最近修改   :
  功能描述   : 在360_APP的版版本上，做一个专门针对潍柴的版本
  函数列表   :
*
*       1.                com_thread
*       2.                main
*

  修改历史   :

  2.日    期   : 2018年7月17日
    作    者   : Liuh
    修改内容   : 根据潍柴需求，速度>20km/h时，关闭360系统。问题单号：liuh-0002
  1.日    期   : 2018年6月15日
    作    者   : Liuhuan
    修改内容   : 根据潍柴的需求做相应的修改，问题单号：liuh-0001

******************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "imx6_com.h"
#include "tsh_comm.h"
//#define VOS_Debug_Trace(s)	printf(s);

void* com_thread(void *arg)
{
	tty1_com_thread();
	return  NULL;
}


void* Receive_thread(void *arg)
{
	ReceiveSerialDataThread();
	return  NULL;
}


int main(void)
{
    int err;
    pthread_t tid1,tid2;
    while (1)
    {
        err = TshComm_Init(EN_APP_OUTER, msg_process_cb, NULL);
        if(0 == err)
            break;
        printf("TshCommt_Init failed. errNo=%u\n", TshComm_GetInnerErrorCode());
        usleep(100000);
    }
    while(!TshComm_IsReady())
    {
        usleep(1000);
        printf("not ready\n");
    }
    TshComm_SetInformFlag(EN_TSH_INFORM_VIEWING_ANGLE  | EN_TSH_INFORM_VIDEO_EXCEPTION
    | EN_TSH_INFORM_CALI_RESULT | EN_TSH_INFORM_CAMERAS_STATE| EN_TSH_INFORM_ADAS_STATE);
    err = pthread_create(&tid1,NULL,com_thread,NULL);
    if(err != 0)
    {
        printf("can not create thread 1:%s\n",strerror(err));
		TshComm_Exit();
    	return 0;
    }
    else
    {	
    	err = pthread_create(&tid2,NULL,Receive_thread,NULL);
		if(err != 0){
			printf("can not create thread 2:%s\n",strerror(err));
			TshComm_Exit();
	    	return 0;
		}

        SendHeartBeatPkg(); //主线程
    }
    err = pthread_join(tid1,NULL);
    if(err !=0)
    {
        printf("can not join thread 1:%s\n",strerror(err));/*等待子线程结束*/
    }
	err = pthread_join(tid2,NULL);
    if(err !=0)
    {
        printf("can not join thread 2:%s\n",strerror(err));
    }
    TshComm_Exit();
	printf("[liuh>>>>>]acc_off_flag = %d\n",acc_off_flag);
	if(acc_off_flag != 1)
	{
		execl_boot();
	}
    	
    return 0;
}
