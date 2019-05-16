/******************************************************************************

                  ��Ȩ���� (C), 2018-2028, ���񵼺��Ƽ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : main.c
  �� �� ��   : ����
  ��    ��   : ZD
  ��������   : 2018��6��15��
  ����޸�   :
  ��������   : ��360_APP�İ�汾�ϣ���һ��ר�����Ϋ��İ汾
  �����б�   :
*
*       1.                com_thread
*       2.                main
*

  �޸���ʷ   :

  2.��    ��   : 2018��7��17��
    ��    ��   : Liuh
    �޸�����   : ����Ϋ�������ٶ�>20km/hʱ���ر�360ϵͳ�����ⵥ�ţ�liuh-0002
  1.��    ��   : 2018��6��15��
    ��    ��   : Liuhuan
    �޸�����   : ����Ϋ�����������Ӧ���޸ģ����ⵥ�ţ�liuh-0001

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

        SendHeartBeatPkg(); //���߳�
    }
    err = pthread_join(tid1,NULL);
    if(err !=0)
    {
        printf("can not join thread 1:%s\n",strerror(err));/*�ȴ����߳̽���*/
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
