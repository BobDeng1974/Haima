#ifndef __types_H__
#define __types_H__

typedef unsigned int   uint;
typedef unsigned char  uchar;
typedef unsigned short u16;

#ifndef BOOL
typedef int BOOL;
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif






typedef enum cKeyType{
	TNot = 0,
	TEnter =1,
    TBack =2,
    TFront =3,
    TRear =4,
    TLeft = 5,
    TRight = 6,   
}KeyType;


typedef  enum {
    T_START,
    T_CMD,
    T_HID,
    T_LEN,
    T_DATA,
    T_LRC,
}RecType;
	


typedef enum cCarColour{
	TRed,
    Tblue,
    TGreen,
}CarColour;

typedef struct cRecCarInfo{
    /* float CarSpeed; */
    unsigned short CarSpeed;
    unsigned short CarSpeed_flag;   //�����ٳ���20Km/hʱ���˱�־λΪ1
    short CarSteeringWheel;
    unsigned int CarStat;
    unsigned char Light;//0����1 �� 2��
    unsigned char IsR;	// 0 ���� 1����
    unsigned char alarm; //0���� 1˫��
    unsigned char AvmEnableByLamp;  //���ڿ���ת���ʱ�Ƿ���Ҫ��ʾ360����   1��ʹ��    0���ر�
}RecCarInfo;


typedef struct cAvmParaSetInfo{
	unsigned char allSwitch;//0:open  1:close
	CarColour carColour;
	unsigned char carSubLineStat;
	unsigned char AdasStat;
	unsigned char AdasSpeed;
}AvmParaSetInfo;

typedef struct cAvmCalibrationPara
{
    unsigned char caliFlag;     //�궨�ı�־λ  
    unsigned char caliResult;   //�궨�����0 �궨�ɹ�  33 ǰ�궨ʧ��   34 ��궨ʧ��   35 ��궨ʧ��   36 �ұ궨ʧ��
}AvmCalibrationPara;

typedef enum
{
    CALI_ERR_0  = 0,       /*�ɹ���ƴ�����*/
    CALI_ERR_1  = 1,       /*���������������*/
    CALI_ERR_2  = 2,       /*���󣺴�ԭʼ�ļ�ʧ��*/
    CALI_ERR_17 = 17,     /*����ǰ�ǵ���ȡʧ��*/
    CALI_ERR_18 = 18,     /*���󣺺�ǵ���ȡʧ��*/
    CALI_ERR_19 = 19,     /*������ǵ���ȡʧ��*/
    CALI_ERR_20 = 20,     /*�����ҽǵ���ȡʧ��*/
    CALI_ERR_33 = 33,     /*����ǰ�궨ʧ��*/
    CALI_ERR_34 = 34,     /*���󣺺�궨ʧ��*/
    CALI_ERR_35 = 35,     /*������궨ʧ��*/
    CALI_ERR_36 = 36,     /*�����ұ궨ʧ��*/
    CALI_ERR_49 = 49,     /*����ǰͼ������*/
    CALI_ERR_50 = 50,     /*���󣺺�ͼ������*/
    CALI_ERR_51 = 51,     /*������ͼ������*/
    CALI_ERR_52 = 52,     /*������ͼ������*/
    CALI_ERR_65 = 65,     /*������ǰ����ͼ������*/
    CALI_ERR_66 = 66,     /*������ǰ����ͼ������*/
    CALI_ERR_67 = 67,     /*������󽻽�ͼ������*/
    CALI_ERR_68 = 68,     /*�����Һ󽻽�ͼ������*/
}CaliErro;

typedef enum
{
    CAMERA_CALI_NONE    = 0,             /*����*/
    CAMERA_CALI_F       = 0x00000001,    /*ǰ����ͷ�궨����*/
    CAMERA_CALI_B       = 0x00000002,    /*������ͷ�궨����*/
    CAMERA_CALI_L       = 0x00000004,    /*������ͷ�궨����*/
    CAMERA_CALI_R       = 0x00000008,    /*������ͷ�궨����*/
}CameraCali;




typedef struct cAvmCycleSendInfo{
	unsigned char CurView;
	unsigned char CarColor;
	unsigned char ldwStat;
	unsigned char AvmDisp;
	unsigned char RightCamFail;
	unsigned char LeftCamFail;
	unsigned char RearCamFail; //��
	unsigned char FrontCamFail;
	unsigned char GuidesStat;
	unsigned char oldView;
	unsigned char calibrateStat;
	unsigned char AvmWorkstat;
}AvmCycleSendInfo;




#endif
