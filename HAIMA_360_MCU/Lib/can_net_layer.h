
#include "miscdef.h"
#include "can.h"
#define  CAN_NET_LAYER_SEND_OS_TIM  1  //1 表示用OS系统定时器来实现  0表示用硬件定时器0实现

/**********定义数据类型*******************/
#define DIAG_REV_BUFFER_MAX  (2080u)
#define CONSECUTIVE_FRAME_STATE_REVCEVING  0x01
#define CONSECUTIVE_FRAME_STATE_FINISH     0x00
typedef struct
{
	uint8_t  buf[DIAG_REV_BUFFER_MAX];  //数据内容
	uint8_t last_frame_sn;              //连续桢上一桢的序号 1X~21~2F 20~2F
	uint8_t consecutive_frame_state;    //连续桢 1 表示在接收连续桢，0 表示没有
	uint16_t num;                       //接收的节字数,用于接收累计长度
	uint16_t total_len;                 //桢内容长度 总长度	
	uint16_t consecutive_frame_sum;     //连续桢的总数
	uint32_t CF_last_frame_time;        //连续桢上一桢的接收时间  用于判断有没有超时
}DIAG_REC_TYPE;




/**********定义数据类型*******************/
#define OTHER_DEV_REV_BUFFER_MAX  256
#define CONSECUTIVE_FRAME_STATE_REVCEVING  0x01
#define CONSECUTIVE_FRAME_STATE_FINISH     0x00
typedef struct
{
	u8  buf[OTHER_DEV_REV_BUFFER_MAX];  //数据内容
	u8 last_frame_sn;//连续桢上一桢的序号 1X~21~2F 20~2F
	u8 consecutive_frame_state; //连续桢 1 表示在接收连续桢，0 表示没有
	u16 num;   //接收的节字数,用于接收累计长度
	u16 total_len;   //桢内容长度 总长度	
	u16 consecutive_frame_sum; //连续桢的总数
	u32 CF_last_frame_time; //连续桢上一桢的接收时间  用于判断有没有超时
}OTHER_DEV_REC_TYPE;


typedef struct 
{
    uint8_t FS;
    uint8_t BS;
    uint8_t STmin;  //单位为ms
}FLOW_FRAME_PARA_TYPE; 

/****************diag_send.step  start的值**************************/
typedef enum{
	STOP_STEP,//停止发送 STOP_STEP
	CONSECUTIVE_FRAME_SECOND_FRAME_STEP,   //要发第二桢状态
	CONSECUTIVE_FRAME_THIRD_AND_AFTER_FRAME_STEP,//要连续桢发第三桢（含它）以后的桢状态
}DIAG_SEND_STEP_TYPE;

/****************diag_send.step  end**************************/
#define  CAN_NET_LAYER_SEND_BUFF_MAX  280   //网络层发送buf 最大值？？
#define  NET_LAYER_CLIENT   0x01 //客户端
#define  NET_LAYER_SERVER   0x00 //　服务端
typedef struct 
{
    uint8_t   valid;  //是否有数据要发  
    uint8_t   type;   //NET_LAYER_SERVER  NET_LAYER_CLIENT 客户端 服务端
    uint16_t  len;    //要发数据内容的长度
    uint32_t  ID;     //要发送的ID
    uint32_t  dst_ID; //其它ECU要回id的,用于区别不同的接收
    uint32_t  time;   //发送桢的时间   
    DIAG_SEND_STEP_TYPE   step;//发送步骤  g_time0_state diag_send.step
    uint16_t had_send_len;     //已经发了字节数  unsigned int had_send_len;
    uint8_t  consecutive_frame_serial; // 发送时要填的多包序号  serial;    g_mul_frame.serial 
    uint8_t   buff[CAN_NET_LAYER_SEND_BUFF_MAX]; //要发数据内容
}CAN_NET_LAYER_SEND_TYPE;   
extern CAN_NET_LAYER_SEND_TYPE    can_net_layer_send;




/********网络层向应用层发送的指示原语 start N_USData.ind  N_USData.indication******************/
#define N_FC_RIGHT                 0x00 
#define N_BUFFER_OVFLW                  0x01 //连续桢第一桢的长度大于接收方的网络层缓存大小
#define N_INVALID_FS                    0x02
#define N_INVALID_STMIN                 0x03
#define N_CF_TIMEOUT                    0x04
#define N_FLOW_OVER                     0x05
#define N_WRONG_SN                      0x06 //连续桢不符合计数规则
#define N_WAIT_NEXT_FC                  0x07 

/********网络层向应用层发送的指示原语 end N_USData.ind  N_USData.indication******************/

/****************连续桢头 start************************/
#define CONSECUTIVE_FRAME_FIRST_HEAD_0X10         0x10
#define CONSECUTIVE_FRAME_SECOND_HEAD_0X20        0x20
/****************连续桢头 end************************/

/*******流控制 FS 参数值*********************/
#define FS_SEND   0x00
#define FS_WAIT   0x01
#define FS_OVLW   0x02
/*******流控制 BS 参数值*********************/
#define BS_0         0x00
#define BS_DEFAULT   8//
/*******流控制 STmin 参数值*********************/
#define STMIN_0MS         0x00
#define STMIN_DEFAULT_20MS  20//20ms


/******************网络层计时器 start**************************
	//多桢发送
	多桢发送时，启动发送第一桢，同时等待流控制桢，要在N_Bs（发送方接收流控帧等待时间)时间内，等到，如果
	对方回了流控制桢，其中FB为1，即等待下一条流控制桢，则重新计时N_Bs（发送方接收流控帧等待时间).后面发送
	是根据流控制回的时间来发送(rev_flow_frame_para.STmin, N_Cs(发送方发送连续桢的间隔时间) 这个时间没用，
	会流控制改变)
	//长数据接收
	N_Cr 接收方接收连续帧的等待时间
	**************************************************************/
#if CAN_NET_LAYER_SEND_OS_TIM //用OS定时器实现
	#define  OS_1MS          1
	#define N_As            (70)  //数据经数据链路层发送时间
	#define N_Ar            (70)  //数据经数据链路层发送时间
	//#define N_Bs            (10000*OS_1MS) //60S for test//发送方接收流控帧等待时间
	#define N_Bs            (150) //发送方接收流控帧等待时间
	#define N_Br            (70)  //接收方发送流控帧的间隔时间 要超快越快，满足小于这个时间，所以软件就不用判断；
	#define N_Cs            (70)  //发送方发送连续桢的间隔时间  这个时间没用，会流控制改变
	#define N_Cr            (150) //(150*OS_1MS) //T_150ms  //接收方接收连续帧的等待时间
	#define WAINT_FLOW_CONTROL_FRAME_TIM   N_Bs
	#define REV_CONSECUTIVE_FRAME_OVER_TIM N_Cr
#else  //用定时器
	#define TIMER0_F_DEVIDE      60      //T0定时器60分频 ，外设频率60*1000000
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
		
	#define N_As            T_70ms  //数据经数据链路层发送时间
	#define N_Ar            T_70ms  //数据经数据链路层发送时间
	#define N_Bs            T_1min //for test//发送方接收流控帧等待时间
		//#define N_Bs            T_150ms //发送方接收流控帧等待时间
	#define N_Br            T_70ms  //接收方发送流控帧的间隔时间 要超快越快，满足小于这个时间，所以软件就不用判断；
	#define N_Cs            T_70ms  //发送方发送连续桢的间隔时间  这个时间没用，会流控制改变
	#define N_Cr            T_150ms //T_150ms  //接收方接收连续帧的等待时间
	#define WAINT_FLOW_CONTROL_FRAME_TIM   N_Bs
	#define REV_CONSECUTIVE_FRAME_OVER_TIM N_Cr
#endif
/******************网络层计时器 end***********************/

 void init_net_layer_parameer(void);
 void join_can_send_queue(uint32_t send_can_id,uint32_t dst_can_id,uint8_t *buf,uint16_t len );
 void diag_send_data(device_info_t *dev, u8 *Data, u16 len );
 u32 Body_Can_proc(device_info_t *dev, can_msg_t *cmsg);
 void diag_receive_data(device_info_t *dev,can_msg_t  *p_msg);
void can_link_receive_data(device_info_t * dev);



