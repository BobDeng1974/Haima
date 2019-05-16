
#include "miscdef.h"
#include "can.h"
#define  CAN_NET_LAYER_SEND_OS_TIM  1  //1 ��ʾ��OSϵͳ��ʱ����ʵ��  0��ʾ��Ӳ����ʱ��0ʵ��

/**********������������*******************/
#define DIAG_REV_BUFFER_MAX  (2080u)
#define CONSECUTIVE_FRAME_STATE_REVCEVING  0x01
#define CONSECUTIVE_FRAME_STATE_FINISH     0x00
typedef struct
{
	uint8_t  buf[DIAG_REV_BUFFER_MAX];  //��������
	uint8_t last_frame_sn;              //��������һ������ 1X~21~2F 20~2F
	uint8_t consecutive_frame_state;    //������ 1 ��ʾ�ڽ��������壬0 ��ʾû��
	uint16_t num;                       //���յĽ�����,���ڽ����ۼƳ���
	uint16_t total_len;                 //�����ݳ��� �ܳ���	
	uint16_t consecutive_frame_sum;     //�����������
	uint32_t CF_last_frame_time;        //��������һ��Ľ���ʱ��  �����ж���û�г�ʱ
}DIAG_REC_TYPE;




/**********������������*******************/
#define OTHER_DEV_REV_BUFFER_MAX  256
#define CONSECUTIVE_FRAME_STATE_REVCEVING  0x01
#define CONSECUTIVE_FRAME_STATE_FINISH     0x00
typedef struct
{
	u8  buf[OTHER_DEV_REV_BUFFER_MAX];  //��������
	u8 last_frame_sn;//��������һ������ 1X~21~2F 20~2F
	u8 consecutive_frame_state; //������ 1 ��ʾ�ڽ��������壬0 ��ʾû��
	u16 num;   //���յĽ�����,���ڽ����ۼƳ���
	u16 total_len;   //�����ݳ��� �ܳ���	
	u16 consecutive_frame_sum; //�����������
	u32 CF_last_frame_time; //��������һ��Ľ���ʱ��  �����ж���û�г�ʱ
}OTHER_DEV_REC_TYPE;


typedef struct 
{
    uint8_t FS;
    uint8_t BS;
    uint8_t STmin;  //��λΪms
}FLOW_FRAME_PARA_TYPE; 

/****************diag_send.step  start��ֵ**************************/
typedef enum{
	STOP_STEP,//ֹͣ���� STOP_STEP
	CONSECUTIVE_FRAME_SECOND_FRAME_STEP,   //Ҫ���ڶ���״̬
	CONSECUTIVE_FRAME_THIRD_AND_AFTER_FRAME_STEP,//Ҫ�����巢�����壨�������Ժ����״̬
}DIAG_SEND_STEP_TYPE;

/****************diag_send.step  end**************************/
#define  CAN_NET_LAYER_SEND_BUFF_MAX  280   //����㷢��buf ���ֵ����
#define  NET_LAYER_CLIENT   0x01 //�ͻ���
#define  NET_LAYER_SERVER   0x00 //�������
typedef struct 
{
    uint8_t   valid;  //�Ƿ�������Ҫ��  
    uint8_t   type;   //NET_LAYER_SERVER  NET_LAYER_CLIENT �ͻ��� �����
    uint16_t  len;    //Ҫ���������ݵĳ���
    uint32_t  ID;     //Ҫ���͵�ID
    uint32_t  dst_ID; //����ECUҪ��id��,��������ͬ�Ľ���
    uint32_t  time;   //�������ʱ��   
    DIAG_SEND_STEP_TYPE   step;//���Ͳ���  g_time0_state diag_send.step
    uint16_t had_send_len;     //�Ѿ������ֽ���  unsigned int had_send_len;
    uint8_t  consecutive_frame_serial; // ����ʱҪ��Ķ�����  serial;    g_mul_frame.serial 
    uint8_t   buff[CAN_NET_LAYER_SEND_BUFF_MAX]; //Ҫ����������
}CAN_NET_LAYER_SEND_TYPE;   
extern CAN_NET_LAYER_SEND_TYPE    can_net_layer_send;




/********�������Ӧ�ò㷢�͵�ָʾԭ�� start N_USData.ind  N_USData.indication******************/
#define N_FC_RIGHT                 0x00 
#define N_BUFFER_OVFLW                  0x01 //�������һ��ĳ��ȴ��ڽ��շ�������㻺���С
#define N_INVALID_FS                    0x02
#define N_INVALID_STMIN                 0x03
#define N_CF_TIMEOUT                    0x04
#define N_FLOW_OVER                     0x05
#define N_WRONG_SN                      0x06 //�����岻���ϼ�������
#define N_WAIT_NEXT_FC                  0x07 

/********�������Ӧ�ò㷢�͵�ָʾԭ�� end N_USData.ind  N_USData.indication******************/

/****************������ͷ start************************/
#define CONSECUTIVE_FRAME_FIRST_HEAD_0X10         0x10
#define CONSECUTIVE_FRAME_SECOND_HEAD_0X20        0x20
/****************������ͷ end************************/

/*******������ FS ����ֵ*********************/
#define FS_SEND   0x00
#define FS_WAIT   0x01
#define FS_OVLW   0x02
/*******������ BS ����ֵ*********************/
#define BS_0         0x00
#define BS_DEFAULT   8//
/*******������ STmin ����ֵ*********************/
#define STMIN_0MS         0x00
#define STMIN_DEFAULT_20MS  20//20ms


/******************������ʱ�� start**************************
	//���巢��
	���巢��ʱ���������͵�һ�壬ͬʱ�ȴ��������壬Ҫ��N_Bs�����ͷ���������֡�ȴ�ʱ��)ʱ���ڣ��ȵ������
	�Է������������壬����FBΪ1�����ȴ���һ���������壬�����¼�ʱN_Bs�����ͷ���������֡�ȴ�ʱ��).���淢��
	�Ǹ��������ƻص�ʱ��������(rev_flow_frame_para.STmin, N_Cs(���ͷ�����������ļ��ʱ��) ���ʱ��û�ã�
	�������Ƹı�)
	//�����ݽ���
	N_Cr ���շ���������֡�ĵȴ�ʱ��
	**************************************************************/
#if CAN_NET_LAYER_SEND_OS_TIM //��OS��ʱ��ʵ��
	#define  OS_1MS          1
	#define N_As            (70)  //���ݾ�������·�㷢��ʱ��
	#define N_Ar            (70)  //���ݾ�������·�㷢��ʱ��
	//#define N_Bs            (10000*OS_1MS) //60S for test//���ͷ���������֡�ȴ�ʱ��
	#define N_Bs            (150) //���ͷ���������֡�ȴ�ʱ��
	#define N_Br            (70)  //���շ���������֡�ļ��ʱ�� Ҫ����Խ�죬����С�����ʱ�䣬��������Ͳ����жϣ�
	#define N_Cs            (70)  //���ͷ�����������ļ��ʱ��  ���ʱ��û�ã��������Ƹı�
	#define N_Cr            (150) //(150*OS_1MS) //T_150ms  //���շ���������֡�ĵȴ�ʱ��
	#define WAINT_FLOW_CONTROL_FRAME_TIM   N_Bs
	#define REV_CONSECUTIVE_FRAME_OVER_TIM N_Cr
#else  //�ö�ʱ��
	#define TIMER0_F_DEVIDE      60      //T0��ʱ��60��Ƶ ������Ƶ��60*1000000
	#define T_1us            1
	#define T_100us         (T_1us   *100)
	#define T_1ms           (T_1us   *1000)
	#define T_10ms          (T_1ms   *10)
	#define T_100ms         (T_10ms  *10)
	#define T_250ms         (T_10ms  *25)
	#define T_1s            (T_1ms   *1000)
	#define T_5s            (T_1s    *5)
	#define T_1min          (T_1s    *60)
	#define T_150ms         (T_10ms  *15)
	#define T_70ms          (T_10ms  *7)
		
	#define N_As            T_70ms  //���ݾ�������·�㷢��ʱ��
	#define N_Ar            T_70ms  //���ݾ�������·�㷢��ʱ��
	#define N_Bs            T_1min //for test//���ͷ���������֡�ȴ�ʱ��
		//#define N_Bs            T_150ms //���ͷ���������֡�ȴ�ʱ��
	#define N_Br            T_70ms  //���շ���������֡�ļ��ʱ�� Ҫ����Խ�죬����С�����ʱ�䣬��������Ͳ����жϣ�
	#define N_Cs            T_70ms  //���ͷ�����������ļ��ʱ��  ���ʱ��û�ã��������Ƹı�
	#define N_Cr            T_150ms //T_150ms  //���շ���������֡�ĵȴ�ʱ��
	#define WAINT_FLOW_CONTROL_FRAME_TIM   N_Bs
	#define REV_CONSECUTIVE_FRAME_OVER_TIM N_Cr
#endif
/******************������ʱ�� end***********************/

 void init_net_layer_parameer(void);
 void join_can_send_queue(uint32_t send_can_id,uint32_t dst_can_id,uint8_t *buf,uint16_t len );
 void diag_send_data(device_info_t *dev, u8 *Data, u16 len );
 u32 Body_Can_proc(device_info_t *dev, can_msg_t *cmsg);
 void diag_receive_data(device_info_t *dev,can_msg_t  *p_msg);
void can_link_receive_data(device_info_t * dev);



