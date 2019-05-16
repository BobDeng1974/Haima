#include "imx6_com.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <signal.h>
#include <strings.h>
#include <sys/time.h>
#include "tsh_comm.h"
#include "types.h"
#include "upgrade.h"
#include "queue.h"

static const char* app_ver = "T6A-APP-VER-1.2.2";
static volatile int stopped = 0;
int acc_off_flag = 0;		//1:acc off	0:acc on
static volatile int comfd = -1;
static volatile int avmEnableStat = 0;//��ǰavmʹ��״̬0 �ر� 1 ʹ��
static uint avmAllSwitch = 0; //  0��  1�ر�
static uchar Is3D = 0; //0 2D  1 3D
static volatile RecCarInfo carInfo = {0};
static volatile RecCarInfo lastInfo={0};
//static int reverse_flag = 0;
static int g_key_enter = 0; //����360��������ʱ 1       ����360�˳�����ʱ 0
/* static uint32_t view_mode_bak = 0; */
/* static  AvmParaSetInfo avmParaSetInfo = {0}; */
//AvmParaSetInfo avmParaSetInfo = {0};
//AvmCalibrationPara avmCalibrationPara = {0, 0};
static pthread_mutex_t read_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t com_lock = PTHREAD_MUTEX_INITIALIZER;
//static int jumpflag = 0;
static AvmCycleSendInfo avmCycleSendInfo={0};
static AvmParaSetInfo avmParaSetInfo = {0};
static volatile uchar CommReady = 0; /*1:��MCU�������� 0���쳣*/
Queue  MsgQueue;  /*��˫�ص���Ϣ����*/
Queue  RecMsgQueue;  /*���ڽ�����Ϣ����*/




#define   MAKE_DWORD(a,b,c,d)  ((a<<24)|(b<<16)|(c<<8)|(d))
#define   MAKE_WORD(a,b)       ((a<<8)|(b))

#define EXIT_R_GEARS         ((lastInfo.IsR == 1) && (carInfo.IsR == 0))
#define EXIT_TURN_LIGHT      ((lastInfo.Light != 0) && (carInfo.Light == 0))
#define SPEED_SLOW_DOWN      (lastInfo.CarSpeed > carInfo.CarSpeed)
#define SPEED_UP   			 (lastInfo.CarSpeed < carInfo.CarSpeed)
#define SPEED_IS_20          (20)
#define SPEED_IS_50          (50)
#define SPEED_IS_15          (15)
#define PRINT_LEVEL          (1)

#define DBG(fmt, args...) do{ \
	if(PRINT_LEVEL > 0){ \
		printf(fmt, ## args); \
	} \
}while(0)

#define PRINTFBUF(size,buf)   do{ \
		DBG("[REC_BUF]"); \
		int s=(size); \
		char* b = (buf); \
		while(s--){ \
			DBG("%x ",*b++); \
		} \
		DBG("\n");\
		}while(0)



#if 0
/*---------------------------------------------------------------
* ����ԭ�ͣ�void DelayFun(void)
* �������ܣ���ʱ�ص����� 
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
void DelayFun(int sig)
{

    printf("sig = %d\n",sig);
	ProcessAutoHandle();
	return;
}

/*---------------------------------------------------------------
* ����ԭ�ͣ�int InitTimer(void)
* �������ܣ�װ�ض�ʱ�� setitimerֻ��ʵ��һ����ʱ����
* ����˵���������ε���setitimer����ֵ���ᱻ���ǵ�
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/

int InitTimer(void)
{
	struct itimerval tick;  
    memset(&tick, 0, (size_t)sizeof(tick));  
    // Timeout to run function first time  
    tick.it_value.tv_sec = 3;  // sec  
    tick.it_value.tv_usec = 0; // micro sec.  
    return setitimer(ITIMER_REAL, &tick, NULL);  
}

#endif

/*---------------------------------------------------------------
* ����ԭ�ͣ�int setCom(const char *Dev)
* �������ܣ����ô���ͨ������
* ����˵����
  
* ����������豸���� 
* �� �� ֵ��fd
* ���ߣ�    zd
*---------------------------------------------------------------*/

int setCom(const char *const Dev)
{
    int fd;
    struct termios termios_new;
    fd = open(Dev, O_RDWR|O_NDELAY|O_NOCTTY);
    if (-1 == fd)
    {
        printf("open com dev filed!\n");
        return -1;
    }

	//��������
    bzero(&termios_new, (size_t)sizeof(termios_new));
    cfmakeraw(&termios_new);
#if PROJECT_SEL == 1
    termios_new.c_cflag = (B115200);
#else
    termios_new.c_cflag = (B115200);//��������
#endif
    termios_new.c_cflag |= CLOCAL | CREAD;
    termios_new.c_cflag &= ~CSIZE;
    termios_new.c_cflag |= CS8;
    termios_new.c_cflag &= ~PARENB;
    termios_new.c_cflag &= ~CSTOPB;
#if REC_DATA_ONE_BY_ONE		 
    termios_new.c_cc[VTIME] = 0;
    termios_new.c_cc[VMIN] = 0;
#else	
    termios_new.c_cc[VTIME] = 1;
    termios_new.c_cc[VMIN] = 0;
#endif
    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd, TCSANOW, &termios_new);
    return fd;
}



static char save_data_to_file(const uchar pos,const uchar value,const uchar size)
{
	int Parafd = -1;
	Parafd = open(T7A_APP_INI_PATH,O_RDWR);
    if(Parafd < 0)
    {
        close(Parafd);
        printf("open config file err!\n");
		return -1;
    } else{
		lseek(Parafd,pos,SEEK_SET);
	  	write(Parafd,&value,size);
    	close(Parafd);
 	}
	return 0;
}




/*---------------------------------------------------------------
* ����ԭ�ͣ�int WriteCom(char* buf,int n)
* �������ܣ�����д����
* ����˵����
  
* ���������
* �� �� ֵ����
* ���ߣ�    zd
*---------------------------------------------------------------*/
int WriteCom(char* const buf,const uint n)
{
    uint nleft;
    int nwrite;
    char  *ptr;
    ptr = buf;
    nleft = n;
	static pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;

	// **************�Դ����豸������Դ����***************
    pthread_mutex_lock(&write_lock);
    while (nleft > 0u) {
        if ( (nwrite = write(comfd, ptr, nleft)) <= 0) {
	     pthread_mutex_unlock(&write_lock);
            perror("write: ");
            return (-1);			/* error */
        }

        nleft -= (uint)nwrite;
        ptr   += nwrite;
    }
    pthread_mutex_unlock(&write_lock);
    return (n);
}

/*---------------------------------------------------------------
* ����ԭ�ͣ�int ReadCom(char* buf)
* �������ܣ����ڶ�����
* ����˵����
  
* ���������
* �� �� ֵ�����Ĵ�С
* ���ߣ�    zd
*---------------------------------------------------------------*/
int ReadCom(char* buf)
{
    int size = 0;
	pthread_mutex_lock(&read_lock);
    while(read(comfd,buf,1) > 0)
    {
        buf++;
        size++;
        if(size == MAX_REC_SIZE)
        {
            break;
        }
    }
	pthread_mutex_unlock(&read_lock);
    return size;
}

/*---------------------------------------------------------------
* ����ԭ�ͣ�unsigned char ReadOneChar(void)
* �������ܣ���ȡһ���ֽ�
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
unsigned char ReadOneChar(void)
{
	unsigned char c = 0;
	//����
	pthread_mutex_lock(&read_lock);
	while(1){
		if(read(comfd, (void*)&c,1) == 1 ){
			break;
		}
	}
	pthread_mutex_unlock(&read_lock);
	return c;
}

int  ReadOneCharNonBlocking(char* c)
{
	int size;
	pthread_mutex_lock(&read_lock);
	size = read(comfd, (void*)c,1);
	pthread_mutex_unlock(&read_lock);
	return size;
}

/*---------------------------------------------------------------
* ����ԭ�ͣ�void flush_buf(int type)
* �������ܣ���ϴ����
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
void flush_buf(const int type)
{
	tcflush(comfd, type);	
}





/*---------------------------------------------------------------
* ����ԭ�ͣ�uchar* arrchr(uchar* pBuf,uchar ch,int size)
* �������ܣ������ض��ַ�
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
uchar* arrchr(uchar* pBuf, const uchar ch,uint size)
{

    if(pBuf == NULL)
    {
        return NULL;
    }
    else if(*pBuf == ch)
    {
        return pBuf;
    }
    size--;
    while(size--)
    {
        if(*(++pBuf) == ch)
        {
            return pBuf;
        }
    }
    return NULL;
}

/*---------------------------------------------------------------
* ����ԭ�ͣ�void ParseGeneralAckCmd(uchar* msg)
* �������ܣ�������Ӧ�����Ƿ�ɹ�
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
void ParseGeneralAckCmd(const uchar* const msg)
{
    uchar cmdId = msg[0];
    uchar result = msg[1];
    if(result == (uchar)FAILED_ACK)
    {
        printf("failed ID:%d\n",cmdId);
    }
    return;

}


/*---------------------------------------------------------------
* ����ԭ�ͣ�void ParseReadyAckCmd(uchar* msg)
* �������ܣ�����ready����
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
void ParseReadyAckCmd(const uchar* const msg)
{
    struct tm time_tm;
    struct timeval time_tv;
    time_t timep;

    time_tm.tm_year = (msg[0]>>4)*10 + (msg[0] & 0x0F) + 2000 - 1900;
    time_tm.tm_mon = (msg[1]>>4)*10 + (msg[1] & 0x0F) -1;
    time_tm.tm_mday = (msg[2]>>4)*10 + (msg[2] & 0x0F);
    time_tm.tm_hour = (msg[3]>>4)*10 + (msg[3] & 0x0F);
    time_tm.tm_min = (msg[4]>>4)*10 + (msg[4] & 0x0F);
    time_tm.tm_sec = (msg[5]>>4)*10 + (msg[5] & 0x0F);
    time_tm.tm_wday = 0;
    time_tm.tm_yday = 0;
    time_tm.tm_isdst = 0;
    timep = mktime(&time_tm);
    time_tv.tv_sec = timep;
    time_tv.tv_usec = 0;
    if(settimeofday(&time_tv, NULL) != 0)
    {
       printf("settimeofday failed!\n");
    }
	CommReady = 1;
    return;
}



/*---------------------------------------------------------------
* ����ԭ�ͣ�void ParsePresentTimeCmd(uchar* msg)
* �������ܣ�������ǰʱ�����
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    Liuh
*---------------------------------------------------------------*/
void ParsePresentTimeCmd(uchar* msg)
{
    struct tm *time_tm;
    struct timeval time_tv;
    struct timezone time_tz;
    time_t timep;

    gettimeofday(&time_tv, &time_tz);
    timep = time_tv.tv_sec;
    time_tm = localtime(&timep);

    time_tm->tm_hour = msg[0];
    time_tm->tm_min = msg[1];
    time_tm->tm_sec = msg[2];

    /* time_tm.tm_year = (msg[0]>>4)*10 + (msg[0] & 0x0F) + 2000 - 1900; */
    /* time_tm.tm_mon = (msg[1]>>4)*10 + (msg[1] & 0x0F) -1; */
    /* time_tm.tm_mday = (msg[2]>>4)*10 + (msg[2] & 0x0F); */
    /* time_tm.tm_hour = (msg[3]>>4)*10 + (msg[3] & 0x0F); */
    /* time_tm.tm_min = (msg[4]>>4)*10 + (msg[4] & 0x0F); */
    /* time_tm.tm_sec = (msg[5]>>4)*10 + (msg[5] & 0x0F); */
    /* time_tm.tm_wday = 0; */
    /* time_tm.tm_yday = 0; */
    /* time_tm.tm_isdst = 0; */
    timep = mktime(time_tm);
    time_tv.tv_sec = timep;
    time_tv.tv_usec = 0;
    if(settimeofday(&time_tv, &time_tz) != 0)
    {
       printf("settimeofday failed!\n");
    }
    return;
}


/*---------------------------------------------------------------
* ����ԭ�ͣ�void ParseVehicleInfo(uchar* msg)
* �������ܣ�����������Ϣ
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
void ParseVehicleInfo(uchar* msg)
{
	// ����������Ϣ
	if(carInfo.CarSpeed != ((((unsigned short)msg[1]<<4)&0x0FF0) |(((unsigned short)msg[0]>>4)&0x0F)))
    {
		carInfo.CarSpeed = ((((unsigned short)msg[1]<<4)&0x0FF0) |(((unsigned short)msg[0]>>4)&0x0F));
        
		TshComm_SetVehicleSpeed(carInfo.CarSpeed);
	}

	// ���������̽Ƕ�
	if(carInfo.CarSteeringWheel != MAKE_WORD(msg[3],msg[2]))
    {
		carInfo.CarSteeringWheel = MAKE_WORD(msg[3],msg[2]);
		
		TshComm_SetVehicleWheelAngle((float)(carInfo.CarSteeringWheel));
	}
#if 0
	// ��ȡ������λ�����ҡ�ת��ơ�Կ�׿׵�λ��Ϣ
	if(carInfo.CarStat !=MAKE_DWORD(msg[7],msg[6],msg[5],msg[4]))
    {
		carInfo.CarStat = MAKE_DWORD(msg[7],msg[6],msg[5],msg[4]);
		carInfo.Light = (carInfo.CarStat & (0x0003<<4))>>4;
		if(carInfo.Light == 3u)
        { //˫��
			carInfo.Light = 0;
			carInfo.alarm = 1;
			TshComm_SetVehicleEvent(EN_TSH_VEHICLE_EVENT_DOUBLE_FLASHING_LIGHTS_ON);
		}
		else{
			carInfo.alarm = 0;
			TshComm_SetVehicleEvent(EN_TSH_VEHICLE_EVENT_DOUBLE_FLASHING_LIGHTS_OFF);
		}

		if(carInfo.Light == 1){
			TshComm_SetVehicleEvent(EN_TSH_VEHICLE_EVENT_LLIGHT_ON);
			TshComm_SetVehicleEvent(EN_TSH_VEHICLE_EVENT_RLIGHT_OFF);
		
			TshComm_SetLightState(EN_LIGHT_POSITION_LEFT_TURN, EN_LIGHT_STATE_ON);
			TshComm_SetLightState(EN_LIGHT_POSITION_RIGHT_TURN, EN_LIGHT_STATE_OFF);
		}

		if(carInfo.Light == 2){
			TshComm_SetVehicleEvent(EN_TSH_VEHICLE_EVENT_RLIGHT_ON);
			TshComm_SetVehicleEvent(EN_TSH_VEHICLE_EVENT_LLIGHT_OFF);

			TshComm_SetLightState(EN_LIGHT_POSITION_RIGHT_TURN, EN_LIGHT_STATE_ON);
			TshComm_SetLightState(EN_LIGHT_POSITION_LEFT_TURN, EN_LIGHT_STATE_OFF);
		}
		else{
			TshComm_SetVehicleEvent(EN_TSH_VEHICLE_EVENT_RLIGHT_OFF);
			TshComm_SetVehicleEvent(EN_TSH_VEHICLE_EVENT_LLIGHT_OFF);

			TshComm_SetLightState(EN_LIGHT_POSITION_LEFT_TURN, EN_LIGHT_STATE_OFF);
			TshComm_SetLightState(EN_LIGHT_POSITION_RIGHT_TURN, EN_LIGHT_STATE_OFF);
			
		}
		
		carInfo.IsR= (carInfo.CarStat & (0x0007<<8))>>8;
		if(carInfo.IsR != 1)
        {
			carInfo.IsR = 0;
		}
	}
	#else
	carInfo.CarStat = MAKE_DWORD(msg[7],msg[6],msg[5],msg[4]);
	carInfo.Light = (unsigned char)((carInfo.CarStat & (0x0003<<4))>>4);
	carInfo.IsR= (unsigned char)((carInfo.CarStat & (0x0007<<8))>>8);
	if(carInfo.IsR != 1)
    {
		carInfo.IsR = 0;
	}
	if(carInfo.Light == 3u)
    { //˫��
		carInfo.Light = 0;
		carInfo.alarm = 1;
		//TshComm_SetVehicleEvent(EN_TSH_VEHICLE_EVENT_DOUBLE_FLASHING_LIGHTS_ON);
	}
	else{
		carInfo.alarm = 0;
		//TshComm_SetVehicleEvent(EN_TSH_VEHICLE_EVENT_DOUBLE_FLASHING_LIGHTS_OFF);
	}
	#endif
    //printf("carStat ==> %d\n", carInfo.CarStat);
	DBG("light=%d,R=%d,speed=%d,whell=%d\n",carInfo.Light,carInfo.IsR,carInfo.CarSpeed,(float)carInfo.CarSteeringWheel );
}


/*---------------------------------------------------------------
* ����ԭ�ͣ�void ParseVideoCalibration(uchar pMsgData)
* �������ܣ���ȡAVM��Ϣ
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    liuh
*---------------------------------------------------------------*/
void ParseVideoCalibration(uchar *pMsgData)
{
    EN_TSH_CALI_STATE cali_state = EN_TSH_CALI_NONE;

    cali_state = TshComm_GetCaliState();
	avmCycleSendInfo.calibrateStat = cali_state;
    if(cali_state == EN_TSH_CALI_RUNNING)
    {
        printf("calibration is running....\n");
    }
    else
    {
        printf("calibration start....\n");
        TshComm_SetCarInfomation(EN_TSH_CARINFO_LENGTH, 5250);
        TshComm_SetCarInfomation(EN_TSH_CARINFO_WIDTH, 1850);
        TshComm_SetCarInfomation(EN_TSH_CARINFO_DIST, 0);
        if(pMsgData[0] == 0u)
        {
            TshComm_SetVideoMode(EN_TSH_VIDEO_OPEN);
        }
        else
        {
            TshComm_SetVideoMode(EN_TSH_VIDEO_CLOSE);
        }
        printf("calibration Start...\n");
        TshComm_SetCalibration(EN_TSH_CALIBRATION_CHESSBOARD,EN_TSH_CALI_VISION_BIG);
        if(TShComm_GetCaliErrorCode() == 0u)
        {
            printf("calibration is ok...\n");
        }
        else
        {
            printf("calibration is fail!\n");
        }
        printf("calibration End...\n");

    }

}


/*---------------------------------------------------------------
* ����ԭ�ͣ�void GetAvmInfo(uchar* sendMsg)
* �������ܣ���ȡAVM��Ϣ
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    liuh
*---------------------------------------------------------------*/
void GetAvmInfo(uchar* SendMsgBuf)
{
    FILE *fp;

    //��ȡ�ܿ���״̬
    SendMsgBuf[0] = 0;
    //��ȡ��ǰ������ɫ
    SendMsgBuf[1] = avmParaSetInfo.carColour;
    //��ȡ�����߿���״̬
    SendMsgBuf[2] = 0;
    //��ȡADAS����״̬
    SendMsgBuf[3] = 0;
    //��ȡADAS�����ٶ�
    SendMsgBuf[4] = 0;
    //��ȡARM�������¸���ʱ��
    fp = fopen(UPDATE_TIME_PATH, "rb"); 
    if(fp == NULL)
    {
    	printf("open update_time.txt failed!\n");
        return ;
    }
    fseek(fp,2,SEEK_SET);
    fread(&SendMsgBuf[5] , 1, 1, fp);
    fseek(fp,3,SEEK_SET);
    fread(&SendMsgBuf[6] , 1, 1, fp);
    SendMsgBuf[5] = (SendMsgBuf[5] - 48)*10 + (SendMsgBuf[6] - 48);  //��
    
    fseek(fp,5,SEEK_SET);
    fread(&SendMsgBuf[6] , 1, 1, fp);
    fseek(fp,6,SEEK_SET);
    fread(&SendMsgBuf[7] , 1, 1, fp);
    SendMsgBuf[6] = (SendMsgBuf[6] - 48)*10 + (SendMsgBuf[7] - 48);  //��
    

    fseek(fp,8,SEEK_SET);
    fread(&SendMsgBuf[7] , 1, 1, fp);
    fseek(fp,9,SEEK_SET);
    fread(&SendMsgBuf[8] , 1, 1, fp);
    SendMsgBuf[7] = (SendMsgBuf[7] - 48)*10 + (SendMsgBuf[8] - 48);  //��

    fseek(fp,11,SEEK_SET);
    fread(&SendMsgBuf[8] , 1, 1, fp);
    fseek(fp,12,SEEK_SET);
    fread(&SendMsgBuf[9] , 1, 1, fp);
    SendMsgBuf[8] = (SendMsgBuf[8] - 48)*10 + (SendMsgBuf[9] - 48);  //ʱ
    
    fseek(fp,14,SEEK_SET);
    fread(&SendMsgBuf[9] , 1, 1, fp);
    fseek(fp,15,SEEK_SET);
    fread(&SendMsgBuf[10] , 1, 1, fp);
    SendMsgBuf[9] = (SendMsgBuf[9] - 48)*10 + (SendMsgBuf[10] - 48);  //��
    

    fseek(fp,17,SEEK_SET);
    fread(&SendMsgBuf[11] , 1, 1, fp);
    fseek(fp,18,SEEK_SET);
    fread(&SendMsgBuf[12] , 1, 1, fp);
    SendMsgBuf[10] = (SendMsgBuf[11] - 48)*10 + (SendMsgBuf[12] - 48);  //��
    fclose(fp);
}


/*---------------------------------------------------------------
* ����ԭ�ͣ�void ParseCarColor(uchar* msg)
* �������ܣ����������ó�ģ��ɫ��Ϣ
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    liuh
*---------------------------------------------------------------*/
void ParseCarColor(uchar* msg)
{
	if(msg[0] == 0x00u)
    {
		printf("red\n");
    }
	if(msg[0] == 0x01u)
    {
		printf("white\n");
    }
	if(msg[0] == 0x02u)
    {
		printf("silver\n");
    }

	if(msg[0] == 0x03u)
    {
		printf("grey\n");	
    }
	if(msg[0] == 0x04u)
    {
		printf("green\n");
    }
	if(msg[0] == 0x05u)
    {
		printf("purple\n");	
    }
    TshComm_SetCarModelAndColor(1, (uint32_t)msg[0]);
}

/*---------------------------------------------------------------
* ����ԭ�ͣ�void ParseCarSubline(uchar *msg)
* �������ܣ����������ó����߿���״̬
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    liuh
*---------------------------------------------------------------*/
void ParseCarSubline(uchar *msg)
{
	//avmParaSetInfo
	if((msg[0]>>0)&0x01 == 0u)
	{
		printf("ǰ��̬�켣�߹�\n");
		TshComm_SetTrackLineState(EN_TRACKLINE_TYPE_FRONT_STATIC,EN_TRACKLINE_STATE_OFF);
	}
	else{
		printf("ǰ��̬�켣�߹�\n");//ǰ����
		TshComm_SetTrackLineState(EN_TRACKLINE_TYPE_FRONT_STATIC,EN_TRACKLINE_STATE_OFF);
	}
	if((msg[0]>>1)&0x01 == 0u)
	{
		printf("��̬�켣�߹�\n");
		TshComm_SetTrackLineState(EN_TRACKLINE_TYPE_REAR_STATIC,EN_TRACKLINE_STATE_OFF);
	}
	else{
		TshComm_SetTrackLineState(EN_TRACKLINE_TYPE_REAR_STATIC,EN_TRACKLINE_STATE_ON);

	}
	if((msg[0]>>2)&0x01 == 1u)
	{
		printf("��̬�켣�߹�\n");
		TshComm_SetTrackLineState(EN_TRACKLINE_TYPE_REAR_DYNAMIC,EN_TRACKLINE_STATE_OFF);
	}
	else{
		TshComm_SetTrackLineState(EN_TRACKLINE_TYPE_REAR_DYNAMIC,EN_TRACKLINE_STATE_ON);
	}
	if((msg[0]>>3)&0x01 == 1u)
	{
		printf("����ָ���߹�\n");
	}

	if(msg[0] == 0x0F){ //�� 
		if(avmCycleSendInfo.GuidesStat != 1){
			avmCycleSendInfo.GuidesStat = 1;
			save_data_to_file(OFFSET_SUBLINE_POS, 1, 1);
		}
	}

	if(msg[0] == 0x00){ //�� 
		if(avmCycleSendInfo.GuidesStat != 0){
			avmCycleSendInfo.GuidesStat = 0;
			save_data_to_file(OFFSET_SUBLINE_POS, 0, 1);
		}
	}
	
	
}

/*---------------------------------------------------------------
* ����ԭ�ͣ�void ParseAdasSwitch(uchar *msg)
* �������ܣ����������ø�����ʻ���ܿ���
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    liuh
*---------------------------------------------------------------*/
void ParseAdasSwitch(uchar *msg)
{
	if((msg[0]>>0)&0x01 == 0u)
	{
		printf("LDW��\n");

		if(avmCycleSendInfo.ldwStat == 0){
			avmCycleSendInfo.ldwStat = 1; 
			TshComm_SetAdasFeatures(EN_TSH_ADAS_LDW,TRUE);
			save_data_to_file(OFFSET_LDW_STAT_POS,avmCycleSendInfo.ldwStat,1);
		}
		
	}
	else{
		/*LDW�ر�*/
		if(avmCycleSendInfo.ldwStat != 0){
			avmCycleSendInfo.ldwStat = 0;
			TshComm_SetAdasFeatures(EN_TSH_ADAS_LDW,FALSE);
			save_data_to_file(OFFSET_LDW_STAT_POS,avmCycleSendInfo.ldwStat,1);
		}
	}
	if((msg[0]>>1)&0x01 == 0)
	{
		printf("BSD��\n");
	}
	if((msg[0]>>2)&0x01 == 0u)
	{
		printf("DOW��\n");
	}
	if((msg[0]>>3)&0x01 == 0u)
	{
		printf("CTA��\n");
	}
}

/*---------------------------------------------------------------
* ����ԭ�ͣ�void ParseAdasSpeed(uchar *msg)
* �������ܣ����������ø�����ʻ���ܿ����ٶ�
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    liuh
*---------------------------------------------------------------*/
void ParseAdasSpeed(uchar *msg)
{
	if((msg[0]>>0)&0x02 == 0u)
	{
		printf("ldw0km/h\n");
	}
	if((msg[0]>>2)&0x02 == 0u)
	{
		printf("bsd0km/h\n");
	}
	if((msg[0]>>4)&0x02 == 0u)
	{
		printf("DOW0km/h\n");
	}
	if((msg[0]>>6)&0x02 == 0u)
	{
		printf("CTA0km/h\n");
	}
	//ȱ�����ø�����ʻ���ܿ����ٶȵ�API
}


/*---------------------------------------------------------------
* ����ԭ�ͣ�int TestSDCard(void)
* �������ܣ�����SD���Ƿ����
* ����˵����
  
* ���������
* �� �� ֵ��0-���ڣ�1-������
* ���ߣ�    liuh
*---------------------------------------------------------------*/
int TestSDCard(void)
{
	if(access(SD_DEV_PATH, F_OK) == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/*---------------------------------------------------------------
* ����ԭ�ͣ�int Test_USB_Card(void)
* �������ܣ�����SD���Ƿ����
* ����˵����
  
* ���������
* �� �� ֵ��0-���ڣ�1-������
* ���ߣ�    liuh
*---------------------------------------------------------------*/
int Test_USB_Card(void)
{
	if(access(USB_DEV_PATH, F_OK) >= 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


//return NULL if not found  
/*---------------------------------------------------------------
* ����ԭ�ͣ�char* memstr(char* full_data, int full_data_len, const char* substr) 
* �������ܣ������ַ���
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
char* memstr(char* full_data, uint full_data_len, const char* substr)  
{  
    if ((full_data == NULL) || (full_data_len == 0) || (substr == NULL)) {  
        return NULL;  
    }  
  
    if (*substr == '\0') {  
        return NULL;  
    }  
  
    uint sublen = strlen(substr);  
  
    uint i;  
    char* cur = full_data;  
    uint last_possible = full_data_len - sublen + 1u;  
    for (i = 0; i < last_possible; i++) {  
        if (*cur == *substr) {  
            //assert(full_data_len - i >= sublen);  
            if (memcmp(cur, substr, sublen) == 0) {  
                //found  
                return cur;  
            }  
        }  
        cur++;  
    }  
  
    return NULL;  
}  

/*---------------------------------------------------------------
* ����ԭ�ͣ�void ParseVersionInfo(uchar* msg)
* �������ܣ������汾��Ϣ
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
void ParseVersionInfo(uchar* msg){
    FILE *fp;
	uchar *pbuffer = NULL;
	uint nFileLen = 0;
	char* ptr = NULL;
	fp = fopen(MCU_SD_PATH, "rb"); 
	if(fp != NULL)
    {
	    fseek(fp,0,SEEK_END); 
		nFileLen = ftell(fp);
		fseek(fp,0,SEEK_SET);
		pbuffer = (uchar *)malloc(nFileLen);
		if(pbuffer == NULL)
        {
			printf("Not Enough Memory!/n");
			return;
		}
		if(fread(pbuffer, 1, nFileLen, fp) == nFileLen)
        {
			fclose(fp);
			ptr = memstr((char*)pbuffer, nFileLen, "AVM-MCU-VER-");
			printf("sd card mcu ver is:%s\n",ptr);
			if(ptr != NULL)
            {
				if((msg[3] == (ptr[12]-48)) &&(msg[2] == (ptr[14]-48))&&(msg[1] == (ptr[16]-48))&&(msg[0] == (ptr[18]-48)))
                {
					printf("version the same!\n");
				}
				else
                {
					uchar SendMsgBuf[2];
					SendMsgBuf[0] = 0;//boot
					SendMsgBuf[1] = 0;
					printf("send 0x85 cmd!\n");
					SendPkgStruct(CM_ARM_TO_MCU_UPGRADE,0,0,SendMsgBuf,2);
				}
			}
            else
            {
				printf("not found ver info!\n");
			}	
		}
		else
        {
			printf("fread count err!\n");
			fclose(fp);
		}
		free(pbuffer);
	}
	return;
}

#if 0
/*---------------------------------------------------------------
* ����ԭ�ͣ�void ParseBootVersionInfo(uchar *msg)
* �������ܣ�����MCU_Boot�汾��Ϣ
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    chenjr
*---------------------------------------------------------------*/
void ParseBootVersionInfo(uchar *msg)
{
	printf("MCU_BOOT version information get failed\n");


#ifdef MCU_BOOT
	FILE *fp;
	uchar *pbuffer = NULL;
	int nFileLen = 0;
	char* ptr = NULL;

	fp = fopen(MCU_BOOT_PATH, "rb"); 
	if(fp != NULL)
	{
		fseek(fp,0,SEEK_END);
		nFileLen = ftell(fp);
		fseek(fp,0,SEEK_SET);
		pbuffer = (uchar *)malloc(nFileLen);
		if(pbuffer == NULL)
		{
			printf("Not Enough Memory!/n");
			return;
		}

		if(fread(pbuffer, 1, nFileLen, fp) == nFileLen)
		{
			fclose(fp);
			ptr = memstr((char*)pbuffer, nFileLen, "T7A-MCU-VER-");//������T7A��ʾ��MCU������ַ���Ӧ
			printf("sd card mcu ver is:%s\n",ptr);

			if(ptr != NULL)
			{
				if((msg[3] == (ptr[12]-48)) &&(msg[2] == (ptr[14]-48))&&(msg[1] == (ptr[16]-48))&&(msg[0] == (ptr[18]-48)))
				{
					printf("version the same!\n");
				}
				else
				{
					uchar SendMsgBuf[2];
					SendMsgBuf[0] = 1;//app
					SendMsgBuf[1] = 0;
					printf("send 0x85 cmd!\n");
					SendPkgStruct(CM_ARM_TO_MCU_UPGRADE,0,0,SendMsgBuf,2);
				}
			}
			else
			{
				printf("not found ver info!\n");
			}	
		}
		else
		{
			printf("fread count err!\n");
			fclose(fp);
		}
		free(pbuffer);
	}
#endif

	return;
}
#endif


/*---------------------------------------------------------------
* ����ԭ�ͣ�int pkg_transfer(const uchar* src_buf, uint src_n, uchar* dst_buf, uchar* dst_n)
* �������ܣ���Ҫ���͵����ݽ���ת��
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    liuh
*---------------------------------------------------------------*/

int pkg_transfer(const uchar* src_buf, uint src_n, uchar* dst_buf, uchar* dst_n)
{
	int i = 0;
	int j = 1;
	
	if(src_buf == NULL || src_n == 0)
	{
		printf("src_buf is null\n");
		return -1;
	}
	dst_buf[0] = src_buf[0];
	for(i=1; i< src_n; i++)
	{
		if(src_buf[i] == 0xff)
		{
			dst_buf[j++] = 0xfe;
			dst_buf[j] = 0x02;
		}
		else if(src_buf[i] == 0xfe)
		{
			dst_buf[j++] = 0xfe;
			dst_buf[j] = 0x01;
		}
		else
		{
			dst_buf[j] = src_buf[i];
		}
		j++;
	}
	*dst_n = j;
	return 0;
}




/*---------------------------------------------------------------
* ����ԭ�ͣ�int SendPkgStruct(uchar cmd,uint Hid,char IsAck, uchar* msg,uchar msgLen)
* �������ܣ��������Ͱ�
* ����˵����cmd-�����ţ�hid-�����������ˮ�ţ�IsAck-����Ӧ���Ƿ�ɹ���msg-ָ���������ݵ�ָ�룬msgLen-�����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
int SendPkgStruct(uchar cmd,uint Hid,char IsAck, uchar* msg,uchar msgLen)
{
    static unsigned int SendCount = 1;
    int ret;
	uint i;
    uchar crc;
    uchar SendBuf[MAX_SEND_SIZE];
	uchar TransferBuf[MAX_SEND_SIZE];
	uchar TransferLen = 0;
	// ********************���ݰ����***********************
    SendBuf[0] = MSG_HEAD;									// ���ݰ���head��־λ����
    SendBuf[MSG_CMD_OFFSET] = cmd;							// ���ݰ���������cmdλ����
    if(IsAck)
    {
        SendBuf[MSG_HID_OFFSET] = (uchar)((Hid>>24)&0xFF);			// ���ݰ���ˮ��hidλ����
        SendBuf[MSG_HID_OFFSET+1] = (uchar)((Hid>>16)&0xFF);
        SendBuf[MSG_HID_OFFSET+2] = (uchar)((Hid>>8)&0xFF);
        SendBuf[MSG_HID_OFFSET+3] = (uchar)(Hid&0xFF);
    }
    else
    {
        SendBuf[MSG_HID_OFFSET] = (uchar)((SendCount>>24)&0xFF);
        SendBuf[MSG_HID_OFFSET+1] = (uchar)((SendCount>>16)&0xFF);
        SendBuf[MSG_HID_OFFSET+2] = (uchar)((SendCount>>8)&0xFF);
        SendBuf[MSG_HID_OFFSET+3] = (uchar)(SendCount&0xFF);
    }
	
    SendBuf[MSG_LEN_OFFSET] = msgLen;						// ���ݰ���������data�ĳ���len����
    if(msgLen > 0u)
    {
        memcpy(&SendBuf[MSG_DATA_OFFSET],msg,msgLen);		// �������ݵ�������data����������
    }
	
    crc = 0;												// ����CRCλ
    for(i = 1;i<(msgLen + MSG_HEAD_SIZE);i++)				// �����ݳ���Ϊ0����ֻ�������ݰ�ǰ��6���ֽ�(��ȥhead:��ʶ��)�����У��
    {
        crc ^= SendBuf[i];									// У�鷽ʽ:��ǰ�������(��headλ)�໥������򣬵õ�У����
    }
    SendBuf[msgLen+MSG_HEAD_SIZE] = crc;

	pkg_transfer(SendBuf, MSG_HEAD_SIZE+msgLen+1, TransferBuf, &TransferLen);
	//PRINTFBUF(TransferLen, TransferBuf);
    //ret = WriteCom((char*)SendBuf,MSG_HEAD_SIZE+msgLen+1);
    ret = WriteCom((char*)TransferBuf,TransferLen);
    if(ret < 0)
    {
        return -1;
    }
    if(!IsAck)
    {
        SendCount++;										// ���͵��������
    }
    return 0;
}




void TransmitCycleData(void){
	uchar SendMsg[8] = {0};
	 avmCycleSendInfo.CurView  = GetCurrViewMode();
	SendMsg[4] |= avmCycleSendInfo.CurView & 0x1F;
	SendMsg[4] |= (avmCycleSendInfo.CarColor & 0x07) << 5;
	SendMsg[5] |= avmCycleSendInfo.ldwStat & 0x07;
	SendMsg[6] |= avmCycleSendInfo.AvmDisp & 0x03;
	SendMsg[6] |= (avmCycleSendInfo.RightCamFail & 0x01) << 2;
	SendMsg[6] |= (avmCycleSendInfo.LeftCamFail & 0x01) << 3;
	SendMsg[6] |= (avmCycleSendInfo.RearCamFail & 0x01) << 4;
	SendMsg[6] |= (avmCycleSendInfo.FrontCamFail & 0x01) << 5;
	SendMsg[6] |= (avmCycleSendInfo.GuidesStat & 0x01) << 6;
	SendMsg[7] |= avmCycleSendInfo.oldView & 0x0F;
	SendMsg[7] |= (avmCycleSendInfo.calibrateStat & 0x07) << 4;
	SendMsg[7] |= (avmCycleSendInfo.AvmWorkstat & 0x01) << 7;
	SendPkgStruct(CM_CYCLE_SEND_HAIMA_AVM_INFO,0,0,SendMsg,8);
}



/*---------------------------------------------------------------
* ����ԭ�ͣ�void SetCvbsDispCmd(uchar isOpen)
* �������ܣ�����CVBS��ʾ
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
void SetCvbsDispCmd(uchar isOpen)//isOpen 1��   2�ر�
{
    //uchar SendMsgBuf[MAX_SEND_SIZE];
    //SendMsgBuf[0] = isOpen ;
	if(isOpen ==1u)
    {
        DBG("cvbs display is open!\n");
		//system("echo 0 > /sys/class/graphics/fb0/blank");
		/* if(avmAllSwitch == 1) // */
		/* { */
		/* 	return; */
		/* } */
		/* SendPkgStruct(CM_SET_CVBS_DISP,0,0,SendMsgBuf,1);		// CM_SET_CVBS_DISP == CM_SET_AVM_DISP */
		avmCycleSendInfo.AvmDisp = 1;
		avmEnableStat = 1;
	}
	else if(isOpen == 2u)
    {
        DBG("cvbs display is close!\n");
		//system("echo 1 > /sys/class/graphics/fb0/blank");
		/* SendPkgStruct(CM_SET_CVBS_DISP,0,0,SendMsgBuf,1); */
		avmCycleSendInfo.AvmDisp = 2;
		avmEnableStat = 0;
	}
    TransmitCycleData();
    return;
}





/*---------------------------------------------------------------
* ����ԭ�ͣ�void SendGeneralAck(uchar cmd,uint Hid,uchar isSucc)
* �������ܣ�����ͨ��Ӧ��
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
void SendGeneralAck(uchar cmd,uint Hid,uchar isSucc)
{
    uchar SendMsgBuf[MAX_SEND_SIZE];
    SendMsgBuf[0] = cmd;
    SendMsgBuf[1] = isSucc;
    SendPkgStruct(CM_GENERAL_ACK,Hid,1,SendMsgBuf,2);
    return;
}

/*---------------------------------------------------------------
* ����ԭ�ͣ�void SendModeAck(uchar cmd,uint Hid,uchar isSucc)
* �������ܣ���Ӧ��ǰ��ʾģʽ��2D/3D��
* ����˵����

* ���������
* �� �� ֵ��
* ���ߣ� liuh
*---------------------------------------------------------------*/
void SendModeAck(uchar cmd,uint Hid,uchar mode)
{
    uchar SendMsgBuf[8];
    SendMsgBuf[0] = mode+1;
    SendPkgStruct(CM_GET_MODE_ACK,Hid,1,SendMsgBuf,1);
    return;
}

/*---------------------------------------------------------------
* ����ԭ�ͣ�void ParseSendKey(uchar* pMsgData,uint Hid)
* �������ܣ�������������
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
void ParseSendKey(uchar* pMsgData,uint Hid)
{
	
	KeyType  keytype = TNot;
	if((carInfo.IsR == 1) || (carInfo.Light != 0u) || (carInfo.CarSpeed > SPEED_IS_20))
	{
		return;
	}
	if((pMsgData[0] & (0x01<<0)) != 0u)
	{
		keytype = TEnter;
	}
	else if((pMsgData[0] & (0x01<<1)) != 0u)
	{
		keytype = TBack;
	}
	else if((pMsgData[0] & (0x01<<2)) != 0u)
	{
		keytype = TFront;
	}
	else if((pMsgData[0] & (0x01<<3)) != 0u)
	{
		keytype = TRear;
	}
	else if((pMsgData[0] & (0x01<<4)) != 0u)
	{
		keytype = TLeft;
	}
	else if((pMsgData[0] & (0x01<<5)) != 0u)
	{
		keytype = TRight;
	}
	else
	{
		keytype = TNot;
	}
	printf("keytype=%d\n",keytype);
	switch(keytype)
	{
		case TEnter: 
            g_key_enter = 1;
            if(Is3D)
            {
                SG_SetViewingMode(EN_TSH_VIEW_3D_FRONT_AND_SPLICING,0);//Ĭ��ǰ��ͼ
            }
            else
            {
                SG_SetViewingMode(EN_TSH_VIEW_2D_FRONT_AND_SPLICING,0);//Ĭ��ǰ��ͼ
            }
			if(avmEnableStat == 0){
				SetCvbsDispCmd(1);
			}

		break;
		
		case TBack: 
            g_key_enter = 0;
			if(avmEnableStat == 1){
				SetCvbsDispCmd(2);
			}
			
		break;
		
		case TFront: 
			if(Is3D){
				SG_SetViewingMode(EN_TSH_VIEW_3D_FRONT_AND_SPLICING,0.0f);
			}else{
				SG_SetViewingMode(EN_TSH_VIEW_2D_FRONT_AND_SPLICING,0.0f);
			}
		break;
		case TRear: 
			if(Is3D){
				SG_SetViewingMode(EN_TSH_VIEW_3D_BACK_AND_SPLICING,0.0f);
			}else{
				SG_SetViewingMode(EN_TSH_VIEW_2D_BACK_AND_SPLICING,0.0f);
			}
		break;
		case TLeft: 
			if(Is3D){
				SG_SetViewingMode(EN_TSH_VIEW_3D_LF_AND_SPLICING,0.0f);
			}else{
				SG_SetViewingMode(EN_TSH_VIEW_2D_LEFT_AND_SPLICING,0.0f);
			}
		break;
		case TRight: 
			if(Is3D){
				SG_SetViewingMode(EN_TSH_VIEW_3D_RF_AND_SPLICING,0.0f);
			}else{
				SG_SetViewingMode(EN_TSH_VIEW_2D_RIGHT_AND_SPLICING,0.0f);
			}
		break;
		default:break;
	}
				
	return;
}
void SendCamerasFaultState(void){
	uint state;
	uchar SendMsgBuf = 0;
	state = TshComm_GetCamerasState();
	if((state & EN_TSH_CAMERA_1) == 0){ /*������*/
		SendMsgBuf |= (uchar)EN_TSH_CAMERA_1;
	}
	if((state & EN_TSH_CAMERA_2) == 0){ /*������*/
		SendMsgBuf |= (uchar)EN_TSH_CAMERA_2;
	}
	if((state & EN_TSH_CAMERA_3) == 0){ /*������*/
		SendMsgBuf |= (uchar)EN_TSH_CAMERA_3;
	}
	if((state & EN_TSH_CAMERA_4) == 0){ /*������*/
		SendMsgBuf |= (uchar)EN_TSH_CAMERA_4;
	}
	
	SendPkgStruct(CM_SEND_CAMERA_FAULT,0,0,&SendMsgBuf,1);
}




/*--------------------------------------------------------------------------
 *  Function:
 *  MainWindow.ParseProtocol
 *
 *  Parameters:
 *
 *
 *  Returns value:
 *
 *
 *  Description:
 * 		��������Э��
 *
 *--------------------------------------------------------------------------*/
void ParseProtocol(uchar* msgData,uchar cmd)
{
    uint Hid = (msgData[0]<<24)|(msgData[1]<<16)|(msgData[2]<<8)| msgData[3];
    uchar* pMsgData = &msgData[5];
	FILE *fp;
	char res = 0;
    uchar SD_Exist = 1;
	uchar USB_Exist = 1;
	uchar SendMsgBuf[15];
	int app_flag = MAKE_DWORD('F','A','I','L');//ʧЧ

    switch(cmd)
    {
    case CM_GENERAL_ACK:
        ParseGeneralAckCmd(pMsgData);
        break;
    
    case CM_ARM_READY_ACK:                                                        
        ParseReadyAckCmd(pMsgData);
	    SendGeneralAck(cmd,Hid,SUCCEED_ACK);//ͨ��Ӧ��
	    SetCvbsDispCmd(2);//avmEnableStat=0  ��ʼ״̬AVMʼ�չر�
		//SendCamerasFaultState();/*��һ�η�������ͷ����״̬*/
		avmCycleSendInfo.calibrateStat = TshComm_GetCaliState();
		if(access(MCU_SD_PATH,F_OK) >= 0){
       		 SendPkgStruct(CM_MCU_BOOT_VERSION,0,0,NULL,0);
		}
        break;

    case CM_PRESENT_TIME:
        ParsePresentTimeCmd(pMsgData);
        break;
    
    case CM_ACC_OFF:
		SetCvbsDispCmd(2);
		stopped = 1;	
		acc_off_flag = 1;
        break;     
	
    case CM_SEND_KEY:
	    SendGeneralAck(cmd,Hid,SUCCEED_ACK);//������ͨ��Ӧ��
	 	ParseSendKey(pMsgData,Hid);
	    break;
    
    case CM_SEND_VEHICLE_INFO:
	 	ParseVehicleInfo(pMsgData);
	    SendGeneralAck(cmd,Hid,SUCCEED_ACK);//������ͨ��Ӧ��
	    break;

#if 0
	   case	CM_UNVTRANS_TO_ARM:
	   		if(pMsgData[0] != 0x01){
				break;
			}
			stopped = 1; //jump to boot
			jumpflag = UPGRADE_APP;
			if(((pMsgData[2] >> 1) & 0x07) == 0){//����APP ������־λ
				Parafd = open(VALID_FLAG_PATH,O_RDWR);
			    if(Parafd < 0)
			    {
			        printf("open app flag err!\n");
			    }else{
				  	lseek(Parafd,0,SEEK_SET);
				    write(Parafd,&app_flag,4);	
					close(Parafd);
			    }
			}
			else{
				jumpflag = UPGRADE_OTHER;
			}
		break;
#endif
    case CM_UPDATE_ARM_READY:
		
		printf("[liuh>>>]Rcv cmd: CM_UPDATE_ARM_READY\n");
		stopped = 1; //jump to boot

	    break;

    case CM_SET_MODE:
	 	if(pMsgData[0] == 1u)
        {
        	if(Is3D != 0){
				Is3D = 0;
				ViewChange2D();
				res = save_data_to_file(OFFSET_3D_MODE_POS,Is3D,1);
			}
			
			
		}
        else if(pMsgData[0] == 2u)
        {
        	if(Is3D != 1){
				Is3D = 1;
				ViewChange3D();
				res = save_data_to_file(OFFSET_3D_MODE_POS,Is3D,1);
			}
			
		}
		SendGeneralAck(cmd,Hid,(res == 0)?SUCCEED_ACK:FAILED_ACK);
		
        break;

	case CM_GET_MODE:
        printf("[liuh>>>]Rcv cmd: CM_GET_MODE\n");
	   	SendModeAck(cmd,Hid,Is3D);
        break;

    case CM_GET_DISP_TYPE:                          // ��ѯ��ǰ��ͼ����:ǰ��������ͼ
        SendMsgBuf[0] = (uchar)GetCurrViewMode();
        SendPkgStruct(CM_GET_DISP_TYPE_ACK,Hid,1,SendMsgBuf,1);
        break;

    case CM_SET_WAKE:
	    SendGeneralAck(cmd,Hid,SUCCEED_ACK);
        break;

    case CM_MCU_TO_ARM_UPGRADE:
	    if(pMsgData[0] == 0u) //0����boot   1->app
        {
		    if(access(MCU_SD_PATH,F_OK) >= 0)
            {
               pthread_mutex_lock(&com_lock);
			   UpgradeHander();
			   pthread_mutex_unlock(&com_lock);
			   SendGeneralAck(cmd,Hid,SUCCEED_ACK);
		    }
            else
            {
		        printf("not found sd card!\n");
		    }	
	    }
		else{
			printf("not support upgrade mcu app!!\r\n");
		}
	    break;

    case CM_MCU_APP_VERSION_ACK:
		printf("not support upgrade mcu app!\r\n");
	    //ParseVersionInfo(pMsgData);
	    break;

	case CM_MCU_BOOT_VERSION_ACK:
		ParseVersionInfo(pMsgData);
	    break;

	case CM_ARM_APP_VERSION:
		SendMsgBuf[0] = 0;
		SendMsgBuf[1] = (uchar)(app_ver[12]  -48);
		SendMsgBuf[2] = (uchar)(app_ver[14]  -48);
		SendMsgBuf[3] = (uchar)(app_ver[16]  -48);
		SendPkgStruct(CM_ARM_APP_VERSION_ACK,Hid,1,SendMsgBuf,4);
		break;
		
	case CM_ARM_ARI_VERSION:
		fp = fopen(TSH_VER_PATH, "rb"); 
		if(fp == NULL)
        {
			printf("open version.txt failed!\n");
			break;
		}
		fseek(fp,1,SEEK_SET);
		fread(&SendMsgBuf[1] , 1, 1, fp);
		fseek(fp,3,SEEK_SET);
		fread(&SendMsgBuf[2] , 1, 1, fp);
		fseek(fp,4,SEEK_SET);
		fread(&SendMsgBuf[3] , 1, 1, fp);
		fclose(fp);
		SendMsgBuf[0] = 0;
		SendMsgBuf[1] -= 48;
		SendMsgBuf[2] -= 48;
		SendMsgBuf[3] -= 48;
		SendPkgStruct(CM_ARM_ARI_VERSION_ACK,Hid,1,SendMsgBuf,4);
		break;
	
    case CM_SET_DISP_TYPE:  //�û�ͨ���˻��������ⰴ��������ͼ����
        if((carInfo.IsR == 1u) || (carInfo.Light != 0)){
            return;
        }
		SG_SetViewingMode((uint32_t)pMsgData[0],0);
		usleep(100000);//�ȴ��л��ɹ�
		SendMsgBuf[0] = pMsgData[0];    //GetCurrViewMode();
        SendPkgStruct(CM_SET_DISP_TYPE_ACK,Hid,1,SendMsgBuf,1);

		break;
	
    case CM_VIDEO_CALIBRATION:
        ParseVideoCalibration(pMsgData);
		break;

	case CM_GET_AVM_INFO:
        GetAvmInfo(SendMsgBuf);
		SendPkgStruct(CM_GET_AVM_INFO_ACK,Hid,1,SendMsgBuf,11);
		break;

    case CM_SET_CAR_COLOUR:                         // ���ó�ģ��ɫ
        ParseCarColor(pMsgData);
        SendGeneralAck(cmd,Hid,SUCCEED_ACK);
        break;

    case CM_SET_AVM_ALL_SWITCH:                     // ����AVM�ܿ��أ����ȼ���ߣ��ص�֮��ʹ����Ҳ���Ὺ��AVM
        avmAllSwitch = pMsgData[0];
        if(avmAllSwitch == 1u)
        {
            SetCvbsDispCmd(2);    
        }
        SendGeneralAck(cmd,Hid,SUCCEED_ACK);
        break;

	case CM_SET_CAR_SUBLINE:						// ���ó����߿���ת̬
		ParseCarSubline(pMsgData);
		SendGeneralAck(cmd,Hid,SUCCEED_ACK);
		break;

	case CM_SET_ADAS_FUNCTION_SWITCH:				// ���ø�����ʻ���ܿ���
		ParseAdasSwitch(pMsgData);	
		SendGeneralAck(cmd,Hid,SUCCEED_ACK);
		break;

	case CM_SET_ADAS_FUNCTION_SPEED:				// ���ø�����ʻ���ܿ����ٶ�
		ParseAdasSpeed(pMsgData);
		SendGeneralAck(cmd,Hid,SUCCEED_ACK);
		break;

	case CM_SET_TURN_ENABLE:						// ����ת�򿪹�
 	    SendGeneralAck(cmd,Hid,SUCCEED_ACK);//������ͨ��Ӧ��
		break;

    case CM_GET_TURN_STATUS:
        SendPkgStruct(CM_GET_TURN_STATUS_ACK, Hid, 1, SendMsgBuf, 1);
        break;

	case CM_TEST_SD_CARD:							// ����SD���Ƿ����
		SD_Exist = TestSDCard();
		SendMsgBuf[0] = SD_Exist+1;
		SendPkgStruct(CM_TEST_SD_CARD_ACK, Hid, 1, SendMsgBuf, 1);
		break;

	case CM_CHECK_USB:							// ����USB���Ƿ����
		USB_Exist = Test_USB_Card();
		SendMsgBuf[0] = USB_Exist+1;
		SendPkgStruct(CM_CHECK_USB_ACK, Hid, 1, SendMsgBuf, 1);
		break;
			
    default: 
		printf("parse cmd default!\n");
    	break;
    }
    return;
}

/*---------------------------------------------------------------
* ����ԭ�ͣ�void ProcessComHandle(char* buf ,int size)
* �������ܣ���������Э��
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
void ProcessComHandle(char* const buf , const uint size)
{
    uchar cmd;
    uchar* pbuf;
    uchar* Srcbuf;
    uchar crc;
    uint CalCnt,DataLen;
    uint i;

    PRINTFBUF(size,buf);

    if(size > (uint)MSG_HEAD_SIZE)
    {
        Srcbuf =(uchar*)buf;

        while((pbuf= arrchr(Srcbuf,MSG_HEAD,(size- (Srcbuf - (uchar*)buf)))) != NULL) //ճ������
        {

            cmd = pbuf[MSG_CMD_OFFSET];
            if(cmd == (uchar)CM_UPDATE_ARM_DATATRANS)
            {
                DataLen = ((uint)pbuf[MSG_LEN_OFFSET+1] << 8) | (uint)pbuf[MSG_LEN_OFFSET];
            }
            else
            {
                DataLen = (uint)pbuf[MSG_LEN_OFFSET];                    
            }
            if((pbuf - (uchar*)buf + DataLen + (uint)MSG_HEAD_SIZE +1u ) > size )
            {
                break;
            }
            else
            {

            }

            crc = 0;
            CalCnt = DataLen + (uint)MSG_HEAD_SIZE - 1u;
            i = 1;
            while(CalCnt--)
            {
                crc ^= pbuf[i++];
            }

            if(pbuf[DataLen + (uint)MSG_HEAD_SIZE] != crc)
            {
                printf("check ecc err!\n");
                break;
            }
            else
            {

            }
            ParseProtocol(&pbuf[MSG_HID_OFFSET], pbuf[MSG_CMD_OFFSET]);
            Srcbuf = pbuf + DataLen + MSG_HEAD_SIZE +1;
            if(Srcbuf > ((uchar*)&buf[size - 1u] - MSG_HEAD_SIZE))
            {
                break;
            }
            else
            {

            }
        }

    }
    else
    {
        printf("the size is litter than MSG_HEAD_SIZE!\n");
        return;
    }

    return;
}

/*---------------------------------------------------------------
* ����ԭ�ͣ�void ViewChange3D(void)
* �������ܣ��Զ�3D�л�
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
void ViewChange3D(void)
{
    if(carInfo.IsR == 0u)
    {
		if((carInfo.Light ==0u))
        {
			SG_SetViewingMode(EN_TSH_VIEW_3D_FRONT_AND_SPLICING,0);	
		}
		else if(carInfo.Light ==1u)
        {
			SG_SetViewingMode(EN_TSH_VIEW_3D_LF_AND_SPLICING,0);
		}
		else if(carInfo.Light ==2u)
        {
			SG_SetViewingMode(EN_TSH_VIEW_3D_RF_AND_SPLICING,0);
		}
	}
	else
    {
		if((carInfo.Light ==0u))
        {
			SG_SetViewingMode(EN_TSH_VIEW_3D_REVERSE_AND_SPLICING,0);	
		}
		else if(carInfo.Light ==1u)
        {
			SG_SetViewingMode(EN_TSH_VIEW_3D_LB_AND_SPLICING,0);
		}
		else if(carInfo.Light ==2u)
        {
			SG_SetViewingMode(EN_TSH_VIEW_3D_RB_AND_SPLICING,0);
		}
	}

}

/*---------------------------------------------------------------
* ����ԭ�ͣ�void ViewChange2D(void){
* �������ܣ��Զ�2D�л�
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
void ViewChange2D(void)
{
	if(carInfo.IsR == 0u)
    {
		if((carInfo.Light ==0u))
        {
			SG_SetViewingMode(EN_TSH_VIEW_2D_FRONT_AND_SPLICING,0);	
		}
		else if(carInfo.Light ==1u)
        {
			SG_SetViewingMode(EN_TSH_VIEW_2D_LEFT_AND_SPLICING,0);
		}
		else if(carInfo.Light ==2u)
        {
			SG_SetViewingMode(EN_TSH_VIEW_2D_RIGHT_AND_SPLICING,0);
		}
	}
	else
    {
		SG_SetViewingMode(EN_TSH_VIEW_2D_BACK_AND_SPLICING,0);	
	}

}

/*---------------------------------------------------------------
* ����ԭ�ͣ�void ProcessAutoHandle(void)
* �������ܣ��Զ��л��߼�����
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
void ProcessAutoHandle(void){


	if((lastInfo.IsR != carInfo.IsR) || ( lastInfo.Light != carInfo.Light)){
		if(Is3D)
		{
			ViewChange3D();
		}
		else
		{
			ViewChange2D();
		}
	}	
		
	

	
	if(carInfo.CarSpeed > SPEED_IS_20 ){
		if(carInfo.IsR == 0){  //�ǵ���
			if(avmEnableStat == 1){
				SetCvbsDispCmd(2);
			}
		}
	}
	else{
		if((carInfo.IsR == 1) || (carInfo.Light != 0)){
			if(carInfo.CarSpeed <= SPEED_IS_15){
				if(avmEnableStat == 0){
					SetCvbsDispCmd(1);
				}
			}
			else{  //15 ~20
				if(SPEED_UP){
					if(avmEnableStat == 0){
					SetCvbsDispCmd(1);
				}
					
			}

			}

		}
		
	}



	


	
	return;
}



/*---------------------------------------------------------------
* ����ԭ�ͣ�void InitSavePara(void)
* �������ܣ�������ʼ��
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
void InitSavePara(void)
{
    int Parafd = -1;
    if(access(T7A_APP_INI_PATH,F_OK) >= 0){
		 Parafd = open(T7A_APP_INI_PATH,O_RDWR);
	        if(Parafd < 0)
	        {
	            printf("open para err!\n");
	            return;
	        }
		  lseek(Parafd,OFFSET_3D_MODE_POS,SEEK_SET);
          read(Parafd,&Is3D,1);	
		  lseek(Parafd,OFFSET_LDW_STAT_POS,SEEK_SET);
          read(Parafd,&avmCycleSendInfo.ldwStat,1);	
		  lseek(Parafd,OFFSET_SUBLINE_POS,SEEK_SET);
          read(Parafd,&avmCycleSendInfo.GuidesStat,1);	
		  
	}else{
		 Parafd = open(T7A_APP_INI_PATH,O_CREAT|O_RDWR,777);
	        if(Parafd < 0)
	        {
	            printf("open para err!\n");
	            return;
	        }
	        lseek(Parafd,OFFSET_3D_MODE_POS,SEEK_SET);
            write(Parafd,&Is3D,1);
			lseek(Parafd,OFFSET_LDW_STAT_POS,SEEK_SET);
            write(Parafd,&avmCycleSendInfo.ldwStat,1);//Ĭ�Ϲر�ldw
			lseek(Parafd,OFFSET_SUBLINE_POS,SEEK_SET);
			avmCycleSendInfo.GuidesStat = 1;//Ĭ�ϴ򿪸�����
            write(Parafd,&avmCycleSendInfo.GuidesStat,1);
	}
	close(Parafd);
#if 0
	int fd;
	uchar vel = 0,temp;
	char cmd[100] = {0};	
	if(access("/home/chinagps/logidx.ini",F_OK) >= 0){
		 fd = open("/home/chinagps/logidx.ini",O_RDWR);
	        if(fd < 0)
	        {
	            printf("open para err!\n");
	            return;
	        }
		  lseek(fd,0,SEEK_SET);
    	  read(fd,&vel,1);
		  temp = (vel + 1);
		  if(temp > 255){
			temp = 0;
		  }
		  lseek(fd,0,SEEK_SET);
          write(fd,&temp,1);
	}else{
		 fd = open("/home/chinagps/logidx.ini",O_CREAT|O_RDWR,777);
	        if(fd < 0)
	        {
	            printf("open para err!\n");
	            return;
	        }
	        lseek(fd,0,SEEK_SET);
               write(fd,&vel,1);

	}
	sprintf(cmd,"cat /dev/kmsg > /home/chinagps/%d.log &",vel);
	system(cmd);
#endif
    return;
}





void ProcessCbMsg(void)
{
	Msg msg;
	while(DeQueue(&MsgQueue, &msg))
	{
		switch(msg.MsgType)
		{
			case EN_TSH_FIELD_INFORM_ADAS_STATE:   /*֪ͨADAS״̬*/
				if( msg.MsgData  & EN_ADAS_STATE_NONE )
				{
					avmCycleSendInfo.ldwStat = 1; //δƫ��
				} 
				else if(msg.MsgData  & EN_ADAS_STATE_LEFT_LDWS )
				{
					avmCycleSendInfo.ldwStat = 2;
				}
				else if(msg.MsgData  & EN_ADAS_STATE_RIGHT_LDWS ){
					avmCycleSendInfo.ldwStat = 3;
				}
				break;
				
			case EN_TSH_FIELD_INFORM_CAMERAS_STATE:
				if((msg.MsgData & EN_TSH_CAMERA_1) == 0){ /*������*/
					avmCycleSendInfo.FrontCamFail = 1;
				}
				else{
					avmCycleSendInfo.FrontCamFail = 0;
				}
				if((msg.MsgData & EN_TSH_CAMERA_2) == 0){ /*������*/
					avmCycleSendInfo.RearCamFail = 1;
				}
				else{
					avmCycleSendInfo.RearCamFail = 0;
				}
				if((msg.MsgData & EN_TSH_CAMERA_3) == 0){ /*������*/
					avmCycleSendInfo.LeftCamFail = 1;
				}
				else{
					avmCycleSendInfo.LeftCamFail = 0;
				}
				if((msg.MsgData & EN_TSH_CAMERA_4) == 0){ /*������*/
					avmCycleSendInfo.RightCamFail = 1;
				}
				else{
					avmCycleSendInfo.RightCamFail = 0;
				}

			 	break;
				
			 case EN_TSH_FIELD_INFORM_CALIRESULT:
			 	if(msg.MsgData == CALI_ERR_0)
				{
					avmCycleSendInfo.calibrateStat =2; 
				}
				else
				{
					avmCycleSendInfo.calibrateStat =3;
				}
			 	break;
			 default: 
			 	break;
		}
	}
	return;
}

void InitSendDate(void){

	uint state = TshComm_GetCamerasState();
	if((state & EN_TSH_CAMERA_1) == 0){ /*������*/
		avmCycleSendInfo.FrontCamFail = 1;
	}
	else{
		avmCycleSendInfo.FrontCamFail = 0;
	}
	if((state & EN_TSH_CAMERA_2) == 0){ /*������*/
		avmCycleSendInfo.RearCamFail = 1;
	}
	else{
		avmCycleSendInfo.RearCamFail = 0;
	}
	if((state & EN_TSH_CAMERA_3) == 0){ /*������*/
		avmCycleSendInfo.LeftCamFail = 1;
	}
	else{
		avmCycleSendInfo.LeftCamFail = 0;
	}
	if((state & EN_TSH_CAMERA_4) == 0){ /*������*/
		avmCycleSendInfo.RightCamFail = 1;
	}
	else{
		avmCycleSendInfo.RightCamFail = 0;
	}
	
	avmCycleSendInfo.calibrateStat  = TshComm_GetCaliState();

	avmCycleSendInfo.AvmDisp = 2;//�ر�

	avmCycleSendInfo.CurView = GetCurrViewMode();
	avmCycleSendInfo.AvmWorkstat = 1;

	avmCycleSendInfo.CarColor = 0; 


	if(avmCycleSendInfo.ldwStat == 0){
		TshComm_SetAdasFeatures(EN_TSH_ADAS_LDW,FALSE);
	}
	else{
		TshComm_SetAdasFeatures(EN_TSH_ADAS_LDW,TRUE);
	}

	if(avmCycleSendInfo.GuidesStat == 1){
		TshComm_SetTrackLineState(EN_TRACKLINE_TYPE_FRONT_STATIC,EN_TRACKLINE_STATE_OFF);
		TshComm_SetTrackLineState(EN_TRACKLINE_TYPE_REAR_STATIC,EN_TRACKLINE_STATE_ON);
		TshComm_SetTrackLineState(EN_TRACKLINE_TYPE_REAR_DYNAMIC,EN_TRACKLINE_STATE_ON);
	}
	else{
		TshComm_SetTrackLineState(EN_TRACKLINE_TYPE_FRONT_STATIC,EN_TRACKLINE_STATE_OFF);
		TshComm_SetTrackLineState(EN_TRACKLINE_TYPE_REAR_STATIC,EN_TRACKLINE_STATE_OFF);
		TshComm_SetTrackLineState(EN_TRACKLINE_TYPE_REAR_DYNAMIC,EN_TRACKLINE_STATE_OFF);
	}
}




/*---------------------------------------------------------------
* ����ԭ�ͣ�void InitSavePara(void)
* �������ܣ�tty1�߳�ѭ������ 
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
void tty1_com_thread(void)
{
	int size = 0;
	char buf[MAX_REC_SIZE];
	static uint times = 0;
	//uchar SendMsgBuf[15];
	Msg msg;
	printf("%s\n",app_ver);

	CQueueInit(&MsgQueue);
	CQueueInit(&RecMsgQueue);
	InitSavePara();
	InitSendDate();
	comfd = setCom(COM_DEV_PATH);
	if(comfd < 0)
    {
	    return;
	}
	
	while(!stopped)
    {
		if(CommReady == 0)
		{
			SendPkgStruct(CM_ARM_READY,0,0,NULL,0);
			usleep(30000);
		}

#if REC_DATA_ONE_BY_ONE
		if(DeQueue(&RecMsgQueue, &msg))
		{
			size = msg.SerialData.Len;
			memcpy(buf,msg.SerialData.data,msg.SerialData.Len);
		}
		else
		{
			size = 0;
		}
#else
		size = ReadCom(buf);
#endif
		if(size > 0)
        {
			ProcessComHandle(buf,size);
		}
	
		ProcessAutoHandle();
      

		/* ����ص�֪ͨ��Ϣ���� */
		ProcessCbMsg();		

		if((times++) >= 400)
		{
			times=0;
			TransmitCycleData();
		}
		
        lastInfo.IsR = carInfo.IsR;
        lastInfo.Light = carInfo.Light;
		lastInfo.CarSpeed = carInfo.CarSpeed;		
		usleep(1000);
	}
	close(comfd);
	comfd= -1;
 	printf("[liuh>>>]Jump to boot.....\n");
	return;
}

void execl_boot(void)
{
#if 0
	if(jumpflag == UPGRADE_APP)
    {
    	printf("[liuh>>>>>>>>>>>>]%s:%d\n",__func__, __LINE__);
		system("/home/chinagps/360_boot &"); 
		//execl("/bin/sh", "/home/chinagps/360_boot &", NULL);
		printf("[liuh>>>>>>>>>>>>]%s:%d\n",__func__, __LINE__);
	}
	else if(jumpflag == UPGRADE_OTHER)
    {
    	printf("[liuh>>>>>>>>>>>>]%s:%d\n",__func__, __LINE__);
		system("/home/chinagps/360_boot 1 &");
		//execl("/bin/sh", "/home/chinagps/360_boot &", "1", NULL);
		printf("[liuh>>>>>>>>>>>>]%s:%d\n",__func__, __LINE__);
	}
	#else
	//system("/home/chinagps/360_boot 1 &");
	execl("/home/chinagps/360_boot", "/home/chinagps/360_boot &", "1", NULL);
	#endif
}

void SendHeartBeatPkg(void)
{
	while(!stopped)
	{
		if((comfd > 0 ) && (CommReady == 1))
		{	
			pthread_mutex_lock(&com_lock);
			SendPkgStruct(CM_HEARTBEAT,0,0,NULL,0);
			pthread_mutex_unlock(&com_lock);
			
		}

		usleep(500000);
	}
	return;
}


/*---------------------------------------------------------------
* ����ԭ�ͣ�int pkg_rev_transfer(char* data)
* �������ܣ������յ������ݷ�ת��
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    liuh
*---------------------------------------------------------------*/

int pkg_rev_transfer(char* data)
{
	static char data_bak = 0;
	int ret;
	if(*data == 0xFE)
	{
		data_bak = *data;
		ret = -1;
	}
	else if((data_bak == 0xFE) && (*data == 0x02))
	{
		ret = 0x01;
		*data = 0xFF;
		data_bak = 0x00;
	}
	else if((data_bak == 0xFE) && (*data == 0x01))
	{
		ret = 0x01;
		*data = 0xFE;
		data_bak = 0x00;
	}
	else
	{
		if(data_bak == 0xFE){
			printf("transfer err!\n");
		}
		data_bak = 0x00;
		ret = 1;
	}
	
	return ret;
	
}


uint ReadOneFrame(char* const buf)
{
    RecType f_level = T_START;
    uint  index = 0,hid_size = 0,len_size = 0,ex_len_flag = 0;  
    uint len = 0, len_bak = 0;
    char one_data = 0;
    int size = 0;
	int rev_transfer_flag = 0;
    uint ret = 0;
	while(!stopped){
		pthread_mutex_lock(&com_lock);	/* �������߳���YMODE������ʱ�� ���������̴߳���*/
		size = ReadOneCharNonBlocking(&one_data);
		pthread_mutex_unlock(&com_lock);
	    if(size > 0)
	    {	
			//printf("data=%x\n",one_data);

			if(f_level != T_START){
			rev_transfer_flag = pkg_rev_transfer(&one_data);
			if(rev_transfer_flag < 0){
				continue;
			}
			}
		
	        switch(f_level)
	        {
	            case T_START:
	                if((uchar)one_data == MSG_HEAD)
	                {
	                    buf[index++] = one_data;
	                    f_level = T_CMD;
	                }
	                else
	                {
	                    DBG("Finding the msg head...\n");
	                }
	                break;
	            case T_CMD:
	                buf[index++] = one_data;
	                if((uchar)one_data == CM_UPDATE_ARM_DATATRANS)
	                {
	                    len_size = 2;
	                    ex_len_flag = 1;
	                }
	                else
	                {
	                    len_size = 1;
	                    ex_len_flag = 0;
	                }
	                hid_size = 4;
	                f_level = T_HID;
	                break;
	            case T_HID:
	                buf[index++] = one_data;
	                hid_size--;
	                if(hid_size == 0u)
	                {
	                    f_level = T_LEN;
	                }
	                else
	                {

	                }
	                break;
	            case T_LEN:
                    buf[index++] = one_data;
                    len_size--;
                    if(len_size == 0u)
                    {
                        f_level = T_DATA;
                        if(ex_len_flag)
                        {
                            len = ((uint)one_data << 8) | ((uint)buf[index-2u]);  //����one_dataʱ��Ҫ��һ ��Ϊ�Ѿ����յ�һ����
                            len_bak = len;
							if(len_bak <= 1u){
								index = 0;
                   				f_level = T_START;
							}
                        }
                        else
                        {
                            len = (uint)one_data;
                            len_bak = len;
							if(len_bak == 0){
							f_level = T_LRC;
							}
							
                        }
						if(len_bak > (COM_DATA_SIZE - MSG_FRAME_OTHER_SIZE)){
							index = 0;
                   			f_level = T_START;
						}
						
						
                    }
                    else
                    {

                    }

	                break;
	            case T_DATA:
	                buf[index++] = one_data;
	                len--;
	                if(ex_len_flag)
	                {
	                    if(len == 1u)
	                    { //�ٽ���һ������
	                        f_level = T_LRC;
	                    }
	                    else
	                    {

	                    }
	                }
	                else
	                {
	                    if(len == 0u)
	                    {
	                        f_level = T_LRC;
	                    }
	                    else
	                    {

	                    }
	                }
	                break;
	            case T_LRC:
	                buf[index++] = one_data;
	                index = 0;
	                f_level = T_START;  
	                ret = (len_bak + (uint)MSG_FRAME_OTHER_SIZE);
					return ret;
	                break;
	            default: 
	                index = 0;
	                f_level = T_START;
	                break;
	        }
	    }
		else{
			usleep(100);
		}

	}

    return ret;
}





void ReceiveSerialDataThread(void)
{
#if REC_DATA_ONE_BY_ONE
	uint size;
	char buf[COM_DATA_SIZE];
	while(!stopped)
	{
		if(comfd > 0)
		{
			size = ReadOneFrame(buf);
			//PRINTFBUF(size, buf);
			Msg msg;
			if(size > COM_DATA_SIZE)
			{
				size = COM_DATA_SIZE;
			}
			msg.SerialData.Len = size;
			memcpy(msg.SerialData.data,buf,size);
        	if(!EnQueue(&RecMsgQueue,&msg))
			{
				printf("enqueue fail too fast!\n");
			}
		}
	}
#endif
}


