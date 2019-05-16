/*
版权所有 (c) 2016, 深圳市天双科技有限公司
保留所有权利.

V 1.0
作者: tianshuang

描述: CommonlibAPI

历史: 1. 2016/12/29, 初稿
      2. 2017/1/15，优化
      3. 2017/1/19，添加枚举EN_TSH_ADAS_TYPE的声音提示和显示提示
      4. 2017/3/22，给stVersion_t的成员添加注释。
      5. 2017/4/25，添加自定义值通知字段类型，枚举EN_TSH_INFORM_MODE中添加了EN_TSH_INFORM_XXXFLAG成员。
      6. 2017/5/5，在枚举EN_TSH_VEHICLE_EVENT中添加了刹车事件；
                    添加了字段“外部命令事件”（EN_TSH_FIELD_COMMAND_EVENT）；
                    添加了枚举EN_TSH_COMMAND_EVENT；
                    添加了枚举EN_TSH_STRAIGHT_CALI_PARAM；
                    添加了直线标定参数的数据结构stStraightCaliParams_t
      7. 2017/5/11，添加了结构体stRadarInfo_t；
                    添加了枚举EN_TSH_RADAR_POSITION和EN_TSH_RADAR_STATE；
      8. 2017/5/12，添加了字段“棋盘参数”（EN_TSH_FIELD_CHESS_PARAMS）；
                    添加了结构体stChessParams_t；
                    废弃字段EN_TSH_FIELD_CALI_FIELD_OF_VIEW；
                    添加了枚举标定类型EN_TSH_CALIBRATION_TYPE和视野类型EN_TSH_CALI_VISION；
                    添加了结构体stCalibration_t;
                    添加了字段“标定”（EN_TSH_FIELD_CALIBRATION）
      9. 2017/5/27，添加了字段“电源”（EN_TSH_FIELD_POWER）；
                    添加了枚举类型EN_TSH_POWER_STATE和EN_TSH_POWER_STATE；
                    添加了结构体stPowerState_t；
      10. 2017/6/2，添加了字段“通知屏幕亮度值”（EN_TSH_FIELD_RSP_SCREEN_BRIGHTNESS）；
      11. 2017/6/23，在枚举EN_TSH_VIEWING_TYPE中添加了：
                        EN_TSH_VIEW_SWITCH_TO_2D；
                        EN_TSH_VIEW_SWITCH_TO_3D；
                        EN_TSH_VIEW_SWITCH_DIMENSION;
      12. 2017/7/14， 在枚举EN_TSH_VIEWING_TYPE中添加了：
                        EN_TSH_VIEW_2D_LR_F_AND_SPLICING；
                        EN_TSH_VIEW_2D_LR_B_AND_SPLICING；
                        EN_TSH_VIEW_2D_SPLICING_HORIZON_DISPLAY；
      13. 2017/7/15，添加了枚举EN_TSH_RADAR_NUMBER；
                        更新了枚举EN_TSH_RADAR_POSITION和结构体stRadarInfo_t；
      14. 2017/7/29，添加了枚举EN_ADAS_STATE；
                        添加了字段EN_TSH_FIELD_INFORM_ADAS_STATE；
                        添加了主动通知标志位EN_TSH_INFORM_ADAS_STATE；
      15. 2017/9/22，枚举EN_TSH_VEHICLE_EVENT中添加了新的成员，详情请查看枚举定义。
      16. 2017/11/10， 字段“EN_TSH_FIELD_LANGUAGE”不使用。语言存储在共享内存中。                
      17. 2017/11/29, 枚举EN_TSH_KEY_TYPE添加新成员，详情查看枚举定义。
      18. 2017/12/21，增加了枚举EN_LIGHT_POSITION和EN_LIGHT_STATE。
      19. 2017/12/28，增加了字段“EN_TSH_FIELD_TRACKLINE”和字段“EN_TSH_FIELD_CARMODELANDCOLOR”；
                        增加了枚举EN_TRACKLINE_TYPE和N_TRACKLINE_STATE；
                        增加了数据结构 stTrackLineData_t和stCarModelAndColor_t。
      20. 2018/2/1，增加了字段“EN_TSH_FIELD_CUSTOM_EVENTS”
                    添加了枚举EN_TSH_VEHICLE_EVENT_WIDTH_LIGHT_ON和EN_TSH_VEHICLE_EVENT_WIDTH_LIGHT_OFF。
      21. 2018/3/29，枚举EN_TRACKLINE_TYPE增加成员EN_TRACKLINE_TYPE_FRONT和EN_TRACKLINE_TYPE_REAR；
                    增加了字段“EN_TSH_FIELD_TONING”；
                    增加了枚举类型EN_TONING_TYPE；
                    增加了数据结构stToningData_t。
      22. 2018/4/23，枚举EN_TSH_RADAR_STATE添加不可用状态“EN_TSH_RADAR_STATE_DISABLE”。
      23. 2018/8/30，增加了字段“EN_TSH_FIELD_INFORM_CAMERAS_STATE”，枚举EN_TSH_LANG_TYPE增加新成员，
	                 枚举EN_TSH_INFORM_MODE增加新成员，枚举EN_TSH_RADAR_NUMBER增加新成员，枚举EN_TSH_RADAR_POSITION增加新成员
					 枚举EN_TRACKLINE_TYPE增加新成员
	                            
*/
#ifndef __TSHINNERDEF_H__
#define __TSHINNERDEF_H__


#include <stdint.h>
#include <string.h>

#ifndef BOOL
typedef int BOOL;
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define MSG_QUEUE_KEY_VAL           0x1234  // 消息队列的key
#define MESSAGE_MAX_SIZE            819200  // bytes
#define QUEUE_MAX_SIZE              1638400  // bytes

#define SEMAPHORE_KEY1_VAL          0x1236   // 信号量的key
#define SEMAPHORE_KEY2_VAL          0x1237
#define SHARED_MEMORY_KEY_VAL       0x1235  // 共享内存的key

#define MSG_MAX_LEN                 2048

#define PID_BUFFER_MAX_NUM  128

//  错误码
typedef enum {
    EN_CREATE_MSGQUE_ERROR          = 1,
    EN_CREATE_SEMPHORE_ERROR        = 2,
    EN_CREATE_SHAREDMEMORY_ERROR    = 3,
    EN_CREATE_RECVMSGTHREAD_ERROR   = 4
}EN_ERROR_CODE;

// APP类型(uint32_t)
typedef enum {
    EN_APP_NONE         = 0,
    EN_APP_INNER        = 0x00000001, // 内部app，比如360系app
    EN_APP_OUTER        = 0x00000002, // 外部app，比如读取串口的app
    EN_APP_SERVICE      = 0x00000004, // 守护进程service
}EN_APP_TYPE;

// 版本号信息
typedef struct Version {
    uint32_t nHighV; // 主版本号，十进制
    uint32_t nLowV; // 次版本号，十进制
    int32_t nYear; // 版本发布年
    int32_t nMonth; // 版本发布月
    int32_t nMday; // 版本发布日
}__attribute__((packed)) stVersion_t;

// app的标记位(uint32_t)
typedef enum {
    EN_TSH_360APP_NONE     = 0,
    EN_TSH_360APP_TSHMAIN  = 0x00000001, // 主界面
    EN_TSH_360APP_TSHCALI  = 0x00000002, // 标定界面
    EN_TSH_360APP_TSHCONF  = 0x00000004, // 配置界面
    EN_TSH_360APP_TSHPLAY  = 0x00000008, // 播放界面
    EN_TSH_360APP_SWITCH   = 0x00000010,
}EN_TSH_360APP_TYPE;

// 消息类型(uint16_t) 0-0xffff
typedef enum {
    EN_TSH_MSG_NONE             = 0x0000,
    EN_TSH_MSG_SWITCHAPP        = 0x0001, // 切换app，只能由tshmain,tshconf,tshcali,tshplay发送给switch
    EN_TSH_MSG_EXITAPP          = 0x0002, // 由switch发送给tshconf或tshcali或tshplay；由tshconf直接发给tshmain
    EN_TSH_MSG_SHOWAPP          = 0x0003, // 只能由switch发送给tshconf或tshcali或tshplay
    EN_TSH_MSG_HIDEAPP          = 0x0004, // 只能由switch发送给tshconf或tshcali或tshplay
    EN_TSH_MSG_HIDEAPP_RESP     = 0x0005, // 只由tshmain发送给switch
        
    EN_TSH_MSG_REQUEST          = 0x0006, // 请求消息（外部App-->内部App）
    EN_TSH_MSG_RESPONE          = 0x0007, // 应答消息（内部App-->外部App）
    EN_TSH_MSG_INFORM           = 0x0008, // 通知消息（内部App-->外部App）
    EN_TSH_MSG_INVALID          = 0xffff
}EN_TSH_MSG_TYPE;

// 字段类型(uint16_t) 0-0xffff
enum {
    EN_TSH_FIELD_NONE                           = 0x0000,
    // switch 内部App使用       
    EN_TSH_FIELD_SRCAPP                         = 0x0001, // 源app(uint32_t)
    EN_TSH_FIELD_DSTAPP                         = 0x0002, // 目的app(uint32_t)
    EN_TSH_FIELD_OPTAPP                         = 0x0003, //  要操作的app(uint32_t)
    
    // business  内部App使用        
    EN_TSH_FIELD_TOUCHEVENT                     = 0x03e9, // 触摸事件(stPoint_t)
    EN_TSH_FIELD_KEYEVENT                       = 0x03ea, // 键盘事件(uint32_t)
    EN_TSH_FIELD_VEHICLE_SIGNAL                 = 0x03eb, // 车辆信号(uint32_t)
    EN_TSH_FIELD_VEHICLE_EVENT                  = 0x03ec, // 车辆事件(uint32_t)
    EN_TSH_FIELD_ADAS_FEATURES                  = 0x03ed, // adas功能
    EN_TSH_FIELD_CALI_FIELD_OF_VIEW             = 0x03ee, // 标定视野类型（已废弃）
    EN_TSH_FIELD_SYSTIME                        = 0x03ef, // 系统时间(stSysTime_t)
    EN_TSH_FIELD_SWITCH_APP                     = 0x03f0, // 360app切换命令
    EN_TSH_FIELD_VIEWMODE                       = 0x03f1, // 视角模式(stViewingMode_t)
    EN_TSH_FIELD_RECORDING                      = 0x03f2, // 录像
    EN_TSH_FIELD_VIDEOMODE                      = 0x03f3, // 视频
    EN_TSH_FIELD_RESET_DEFAULTCFG               = 0x03f4, // 恢复默认设置  (int)
    // EN_TSH_FIELD_LANGUAGE                       = 0x03f5, // 语言信息
    EN_TSH_FIELD_COMMAND_EVENT                  = 0x03f6, // 外部命令事件
    EN_TSH_FIELD_CHESS_PARAMS                   = 0x03f7, // 棋盘参数 (stChessParams_t)
    EN_TSH_FIELD_CALIBRATION                    = 0x03f8, // 标定(stCalibration_t)
    EN_TSH_FIELD_POWER                          = 0x03f9, // 电源(stPowerState_t)
    EN_TSH_FIELD_TRACKLINE                      = 0x03fa, // 轨迹线(stTrackLineData_t)
    EN_TSH_FIELD_CARMODELANDCOLOR               = 0x03fb, // 车模型号和颜色(stCarModelAndColor_t)
    EN_TSH_FIELD_CUSTOM_EVENTS                  = 0x03fc, // 自定义事件(uint32_t)
    EN_TSH_FIELD_TONING                         = 0x03fd, // 调色(stToningData_t)
    
    // inform  外部App使用
    // 需要在回调函数中进行解包
    EN_TSH_FIELD_INFORM_CURRAPP                 = 0xfffe, // 通知外部当前app(uint32_t)  值参考EN_TSH_360APP_TYPE
    EN_TSH_FIELD_INFORM_CURRVIEWINGANGLE        = 0xfffd, // 通知当前视角(stViewingMode_t)  值参考EN_TSH_VIEWING_TYPE
    EN_TSH_FIELD_INFORM_RECORDINGSTATE          = 0xfffc, // 通知录像状态(uint32_t) 值参考EN_TSH_RECORDING_STATE
    EN_TSH_FIELD_INFORM_VIDEOEXCEPTION          = 0xfffb, // 通知视频异常(uint32_t)  值为1
    EN_TSH_FIELD_INFORM_CALIRESULT              = 0xfffa, // 通知标定结果(uint32_t)  值为标定的错误码，0表示成功，>0 表示失败
    
    EN_TSH_FIELD_INFORM_XXXFLAG                 = 0xfff9, // 自定义通知（uint32_t） 值后定
    EN_TSH_FIELD_RSP_SCREEN_BRIGHTNESS          = 0xfff8, // 应答屏幕亮度值（int32_t）
    EN_TSH_FIELD_INFORM_ADAS_STATE              = 0xfff7, // 通知adas状态（uint32_t）
    EN_TSH_FIELD_INFORM_IMAGE_STATE             = 0xfff6, // 通知打开图像（uint32_t）
    EN_TSH_FIELD_INFORM_CAMERAS_STATE             = 0xfff5, // 通知摄像头状态（uint32_t）
    
    EN_TSH_FIELD_INVALID                        = 0xffff
};

// 按键事件类型
typedef enum {
    EN_TSH_KEYEVENT_AUTO    = 1, // 一次性触发down和up
    EN_TSH_KEYEVENT_UP      = 2, // 触发弹起
    EN_TSH_KEYEVENT_DOWN    = 3  // 触发按下
}EN_TSH_KEYEVENT_TYPE;

// 触摸事件类型
typedef enum {
    EN_TSH_TOUCHEVENT_AUTO  = 1, // 一次性触发down和up
    EN_TSH_TOUCHEVENT_UP    = 2, // 触发弹起
    EN_TSH_TOUCHEVENT_DOWN  = 3, // 触发按下
    EN_TSH_TOUCHEVENT_MOVE  = 4  // 触发移动
}EN_TSH_TOUCHEVENT_TYPE;

// 按键类型
typedef enum {
    EN_TSH_KEY_NONE         = 0,
    EN_TSH_KEY_UP           = 1, // 按键上
    EN_TSH_KEY_DOWN         = 2, // 按键下
    EN_TSH_KEY_LEFT         = 3, // 按键左
    EN_TSH_KEY_RIGHT        = 4, // 按键右
    EN_TSH_KEY_RETURN       = 5, // 按键返回/菜单
    EN_TSH_KEY_CONFIRM      = 6, // 按键确定
    EN_TSH_KEY_1            = 7, // 按键数字1
    EN_TSH_KEY_2            = 8,  // 按键数字2
    EN_TSH_KEY_3            = 9, // 按键数字3
    EN_TSH_KEY_4            = 10, // 按键数字4
    EN_TSH_KEY_5            = 11, // 按键数字5
    EN_TSH_KEY_6            = 12, // 按键数字6
    EN_TSH_KEY_7            = 13, // 按键数字7
    EN_TSH_KEY_8            = 14, // 按键数字8
    EN_TSH_KEY_9            = 15, // 按键数字9
    EN_TSH_KEY_0            = 16, // 按键数字0
}EN_TSH_KEY_TYPE;

typedef struct TouchActor {
    uint32_t nEventType;
    float x;
    float y;
}stTouchActor_t;

typedef struct KeyActor {
    uint32_t nEventType;
    uint32_t nKey;
}stKeyActor_t;

// 视角模式(uint32_t)
// 新版：车朝向：0前  1右前  2右后  3后  4左后  5左前  6倒车
typedef enum {
    EN_TSH_VIEW_NONE                                    = 0,
    EN_TSH_VIEW_2D_FRONT_AND_SPLICING                   = 1, // 显示2D前视视角+拼接
    EN_TSH_VIEW_2D_BACK_AND_SPLICING                    = 2, // 显示2D后视视角+拼接
    EN_TSH_VIEW_2D_LEFT_AND_SPLICING                    = 3, // 显示2D左视视角+拼接
    EN_TSH_VIEW_2D_RIGHT_AND_SPLICING                   = 4, // 显示2D右视视角+拼接
    EN_TSH_VIEW_3D_FRONT_AND_SPLICING                   = 5, // 显示3D前视视角+拼接
    EN_TSH_VIEW_3D_RF_AND_SPLICING                      = 6, // 显示3D右前视角+拼接
    EN_TSH_VIEW_3D_RB_AND_SPLICING                      = 7, // 显示3D右后视角+拼接
    EN_TSH_VIEW_3D_BACK_AND_SPLICING                    = 8, // 显示3D后视视角+拼接
    EN_TSH_VIEW_3D_LB_AND_SPLICING                      = 9, // 显示3D左后视角+拼接
    EN_TSH_VIEW_3D_LF_AND_SPLICING                      = 10, // 显示3D左前视角+拼接
    EN_TSH_VIEW_3D_REVERSE_AND_SPLICING                 = 11, // 显示3D倒车视角+拼接
    EN_TSH_VIEW_3D_SPECIFIC_ANGLE_AND_SPLICING          = 12, // 显示3D自定义角度视角+拼接
    EN_TSH_VIEW_WIDE_DEFAULT                            = 13, // 显示宽视角（默认）
    EN_TSH_VIEW_WIDE_FRONT                              = 14, // 显示宽视角（前）
    EN_TSH_VIEW_WIDE_BACK                               = 15, // 显示宽视角（后）
    
    EN_TSH_VIEW_SWITCH_TO_2D                            = 16, // 切换到2D视角
    EN_TSH_VIEW_SWITCH_TO_3D                            = 17, // 切换到3D视角
    EN_TSH_VIEW_SWITCH_DIMENSION                        = 18, // 2D/3D切换
    EN_TSH_VIEW_2D_LR_F_AND_SPLICING                    = 19, // 显示左+右（前） + 拼接
    EN_TSH_VIEW_2D_LR_B_AND_SPLICING                    = 20, // 显示左+右（后） + 拼接
    EN_TSH_VIEW_2D_SPLICING_HORIZON_DISPLAY             = 21, // 2D拼接横屏显示
    
    // for HS
    EN_TSH_VIEW_3D_LEFT_AND_SPLICING                    = 22, // 显示3D正左视角+拼接
    EN_TSH_VIEW_3D_RIGHT_AND_SPLICING                   = 23, // 显示3D正右视角+拼接
    EN_TSH_VIEW_3D_LEFT_BACK_SIDE                       = 24, // 显示3D左后侧视角+拼接
    EN_TSH_VIEW_3D_RIGHT_BACK_SIDE                      = 25, // 显示3D右后侧视角+拼接
    EN_TSH_VIEW_3D_LEFT_FRONT_SIDE                      = 26, // 显示3D左前侧视角+拼接
    EN_TSH_VIEW_3D_RIGHT_FRONT_SIDE                     = 27, // 显示3D右前侧视角+拼接
	EN_TSH_VIEW_ORIGIN_FRONT                            = 28, // 原图前
    EN_TSH_VIEW_ORIGIN_REAR                             = 29, // 原图后
    EN_TSH_VIEW_ORIGIN_LEFT                             = 30, // 原图左
    EN_TSH_VIEW_ORIGIN_RIGHT                            = 31, // 原图右
    EN_TSH_VIEW_ORIGIN_ALL                              = 32, // 原图四个
    EN_TSH_VIEW_INVALID                                 = 0xffffffff // 非法
}EN_TSH_VIEWING_TYPE;

typedef struct ViewingMode {
    uint32_t nMode;
    float fAngle;
}__attribute__((packed)) stViewingMode_t;

// 视频
typedef enum {
    EN_TSH_VIDEO_NONE       = 0,
    EN_TSH_VIDEO_OPEN       = 1, // 视频打开
    EN_TSH_VIDEO_CLOSE      = 2  // 视频关闭
}EN_TSH_VIDEO_STATE;

// 语言信息
typedef enum {
    EN_TSH_LANG_NONE                    = 0,
    EN_TSH_LANG_SIMPLIFIED_CHINESE      = 1, // 简体中文
    EN_TSH_LANG_TRADITIONAL_CHINESE     = 2, // 繁体中文
    EN_TSH_LANG_ENGLISH                 = 3,  // 英语
    EN_TSH_LANG_KOREAN                  = 4  // 韩语
}EN_TSH_LANG_TYPE;

// 车辆信号
typedef enum {
    EN_TSH_VEHICLE_SIGNAL_NONE                  = 0,
    EN_TSH_VEHICLE_SIGNAL_DRIVER_DOOR           = 0x00000001, // 驾驶员门信号
    EN_TSH_VEHICLE_SIGNAL_CODRIVER_DOOR         = 0x00000002, // 副驾驶员门信号
    EN_TSH_VEHICLE_SIGNAL_TRUNK_DOOR            = 0x00000004,  // 后备箱门信号
}EN_TSH_SIGNAL_TYPE;

typedef struct VehicleSignal {
    uint32_t nSignal;
    BOOL bOpened;
}__attribute__((packed)) stVehicleSignal_t;

// 车辆事件
typedef enum {
    EN_TSH_VEHICLE_EVENT_NONE                               = 0,
    EN_TSH_VEHICLE_EVENT_RFGEAR_CHANGE                      = 1, // 进入倒车状态
    EN_TSH_VEHICLE_EVENT_FRGEAR_CHANGE                      = 2, // 退出倒车状态
    EN_TSH_VEHICLE_EVENT_LLIGHT_ON                          = 3, // 左转向灯开
    EN_TSH_VEHICLE_EVENT_LLIGHT_OFF                         = 4, // 左转向灯关
    EN_TSH_VEHICLE_EVENT_RLIGHT_ON                          = 5, // 右转向灯开
    EN_TSH_VEHICLE_EVENT_RLIGHT_OFF                         = 6, // 右转向灯关
    EN_TSH_VEHICLE_EVENT_ELIGHT_ON                          = 7, // 紧急灯开
    EN_TSH_VEHICLE_EVENT_ELIGHT_OFF                         = 8, // 紧急灯关
    EN_TSH_VEHICLE_EVENT_FLIGHT_ON                          = 9, // front light is turned on
    EN_TSH_VEHICLE_EVENT_FLIGHT_OFF                         = 10, // front light is turned off
    // Brake
    EN_TSH_VEHICLE_EVENT_FOOT_BRAKE_ON                      = 11, // 脚刹开
    EN_TSH_VEHICLE_EVENT_FOOT_BRAKE_OFF                     = 12, // 脚刹关
    EN_TSH_VEHICLE_EVENT_HAND_BRAKE_ON                      = 13, // 手刹开
    EN_TSH_VEHICLE_EVENT_HAND_BRAKE_OFF                     = 14, // 手刹关
    
    EN_TSH_VEHICLE_EVENT_DOUBLE_FLASHING_LIGHTS_ON          = 15, // 双闪灯开
    EN_TSH_VEHICLE_EVENT_DOUBLE_FLASHING_LIGHTS_OFF         = 16, // 双闪灯关
    EN_TSH_VEHICLE_EVENT_ILL_ON                             = 17, // 车内小灯开启
    EN_TSH_VEHICLE_EVENT_ILL_OFF                            = 18, // 车内小灯关闭
    EN_TSH_VEHICLE_EVENT_HIGH_BEAM_ON                       = 19, // 远光灯开
    EN_TSH_VEHICLE_EVENT_HIGH_BEAM_OFF                      = 20, // 远光灯关
    // ACC
    EN_TSH_VEHICLE_EVENT_ACC_KEY_PULLOUTED                  = 21, // 钥匙拔出
    EN_TSH_VEHICLE_EVENT_ACC_OFF                            = 22, // ACC OFF
    EN_TSH_VEHICLE_EVENT_ACC                                = 23, // ACC
    EN_TSH_VEHICLE_EVENT_ACC_ON                             = 24, // ACC ON
    // Door
    EN_TSH_VEHICLE_EVENT_LF_DOOR_ON                         = 25, // 左前门开
    EN_TSH_VEHICLE_EVENT_LF_DOOR_OFF                        = 26, // 左前门关
    EN_TSH_VEHICLE_EVENT_RF_DOOR_ON                         = 27, // 右前门开
    EN_TSH_VEHICLE_EVENT_RF_DOOR_OFF                        = 28, // 右前门关
    EN_TSH_VEHICLE_EVENT_LB_DOOR_ON                         = 29, // 左后门开
    EN_TSH_VEHICLE_EVENT_LB_DOOR_OFF                        = 30, // 左后门关
    EN_TSH_VEHICLE_EVENT_RBDOOR_ON                          = 31, // 右后门开
    EN_TSH_VEHICLE_EVENT_RBDOOR_OFF                         = 32, // 右后门关
    EN_TSH_VEHICLE_EVENT_REAR_BOX_ON                        = 33, // 后尾箱开
    EN_TSH_VEHICLE_EVENT_REAR_BOX_OFF                       = 34, // 后尾箱关
    EN_TSH_VEHICLE_EVENT_HOOD_ON                            = 35, // 引擎盖开
    EN_TSH_VEHICLE_EVENT_HOOD_OFF                           = 36, // 引擎盖关
    
    EN_TSH_VEHICLE_EVENT_P_KEY_ON                           = 37, // P键开
    EN_TSH_VEHICLE_EVENT_P_KEY_OFF                          = 38, // P键关
    EN_TSH_VEHICLE_EVENT_WIDTH_LIGHT_ON                     = 39, // 示宽灯开（小灯）
    EN_TSH_VEHICLE_EVENT_WIDTH_LIGHT_OFF                    = 40, // 示宽灯关（小灯）

}EN_TSH_VEHICLE_EVENT;

// adas功能
typedef enum {
    EN_TSH_ADAS_NONE    = 0,
    EN_TSH_ADAS_BSD     = 0x00000001, // 盲区检测
    EN_TSH_ADAS_LDW     = 0x00000002, // 车道偏离警示
    EN_TSH_ADAS_FCW     = 0x00000004, // 前方碰撞预警
    EN_TSH_ADAS_MOD     = 0x00000008, // 移动物体检测
    EN_TSH_ADAS_TRACE   = 0x00000010, // 轨迹线
    EN_TSH_ADAS_SOUND   = 0x00000020, // 声音提示
    EN_TSH_ADAS_DISPLAY = 0x00000040  // 显示提示
}EN_TSH_ADAS_TYPE;

typedef struct AdasFeatures {
    uint32_t nAdasFlag;
    BOOL bOpened;
}__attribute__((packed)) stAdasFeatures_t;

// 备注：此枚举已废弃
// 标定视野类型
typedef enum {
    EN_TSH_CALI_FIELD_NONE          = 0,     // 
    EN_TSH_CALI_FIELD_WIDE          = 1, // 大视野标定
    EN_TSH_CALI_FIELD_MIDDLE        = 2, // 中视野标定
    EN_TSH_CALI_FIELD_SMALL         = 3, // 小视野标定
    EN_TSH_CALI_FIELD_HIGH          = 4  // 超视野标定
}EN_TSH_CALI_TYPE;

// 录像类型
typedef enum {
    EN_TSH_RECORDING_NONE       = 0,
    EN_TSH_RECORDING_COMMON     = 0x00000001,   // 普通录像
    EN_TSH_RECORDING_URGENT     = 0x00000002    // 紧急录像
}EN_TSH_RECORDING_TYPE;

typedef struct Recording {
    uint32_t nRecordingFlag;
    BOOL bOpened;
}__attribute__((packed)) stRecording_t;

// 摄像头类型
typedef enum {
    EN_TSH_CAMERA_NONE      = 0,
    EN_TSH_CAMERA_1         = 0x00000001,   // 摄像头1
    EN_TSH_CAMERA_2         = 0x00000002,   // 摄像头2
    EN_TSH_CAMERA_3         = 0x00000004,   // 摄像头3
    EN_TSH_CAMERA_4         = 0x00000008    // 摄像头4
}EN_TSH_CAMERA_STATE;

// 主机状态
typedef enum {
    EN_TSH_HOST_STATE_NONE      = 0,
    EN_TSH_HOST_STATE_WORKING   = 1,    // 主机正在工作
    EN_TSH_HOST_STATE_EXCEPTION = 2     // 主机异常
}EN_TSH_HOST_STATE;

// USB电源状态
typedef enum {
    EN_TSH_USBPOWER_STATE_NONE      = 0,
    EN_TSH_USBPOWER_STATE_OPEN      = 1, // usb电源打开
    EN_TSH_USBPOWER_STATE_CLOSE     = 2  // usb电源关闭
}EN_TSH_USBPOWER_STATE;

typedef struct Tlv {
    uint16_t tag;
    uint16_t len;
    uint8_t val[0];
}__attribute__((packed)) stTlv_t;


typedef struct TlvPacket {
    char* mData;
    char* mTempData;
    uint32_t mLen;
    uint32_t mTempLen;
    uint32_t mRealLen;
    BOOL mHeadOpted;
}stTlvPacket_t;


// tsh程序主动通知功能
typedef enum {
    EN_TSH_INFORM_NONE                  = 0,
    EN_TSH_INFORM_VIEWING_ANGLE         = 0x00000001, // 视角变化时通知当前的视角
    EN_TSH_INFORM_RECORDING_STATE       = 0x00000002, // 录像开始或停止时通知录像的状态
    EN_TSH_INFORM_CURRENT_MENU          = 0x00000004, // 菜单变化时通知
    EN_TSH_INFORM_VIDEO_EXCEPTION       = 0x00000008,  // 视频异常时通知
    EN_TSH_INFORM_CALI_RESULT           = 0x00000010,    // 标定结果通知
    EN_TSH_INFORM_XXXFLAG               = 0x00000020,    // 自定义通知
    EN_TSH_INFORM_ADAS_STATE            = 0x00000040,  // 通知Adas状态
    EN_TSH_INFORM_IMAGE_STATE           = 0x00000080,  // 打开图像通知
    EN_TSH_INFORM_CAMERAS_STATE         = 0x00000100,  // 摄像头状态通知
}EN_TSH_INFORM_MODE;

// tsh程序录像状态变化时通知
typedef enum {
    EN_TSH_RECORDING_COMMON_OPEN        = 1,  // 普通录像状态打开
    EN_TSH_RECORDING_COMMON_CLOSE       = 2,  // 普通录像状态关闭
    EN_TSH_RECORDING_URGENT_OPEN        = 3,  // 紧急录像状态打开
    EN_TSH_RECORDING_URGENT_CLOSE       = 4   // 紧急录像状态关闭
}EN_TSH_RECORDING_STATE;

// 时间
typedef struct SysTime {
    int32_t year;
    int32_t month;
    int32_t mday;
    int32_t hour;
    int32_t minute;
    int32_t second;
}__attribute__((packed)) stSysTime_t;

// 坐标
typedef struct Point {
    int32_t x;
    int32_t y;
}__attribute__((packed)) stPoint_t;

// 标定的状态
typedef enum {
    EN_TSH_CALI_NONE        = 0, // 未进行标定
    EN_TSH_CALI_RUNNING     = 1, // 正在标定
    EN_TSH_CALI_SUCCESS     = 2, // 标定成功
    EN_TSH_CALI_FAIL        = 3 // 标定失败
}EN_TSH_CALI_STATE;

// 车辆信息
typedef struct {
    int32_t nCarPhysicalLength; // 车长
    int32_t nCarPhysicalWidth; // 车宽
    int32_t nLRPhysicalDist; // 前距
    int32_t nWheelBase; // 轴距
    int32_t nWheel2Rear; // 尾距
    int32_t nCaliMode; // 模式
}__attribute__((packed)) stCarInfo_t;

// 车辆各个参数设置类型
typedef enum {
    EN_TSH_CARINFO_NONE         = 0,
    EN_TSH_CARINFO_LENGTH       = 1, // 车长
    EN_TSH_CARINFO_WIDTH        = 2, // 车宽
    EN_TSH_CARINFO_DIST         = 3, // 前距
    EN_TSH_CARINFO_WHEELBASE    = 4, // 轴距
    EN_TSH_CARINFO_WHEEL2REAR   = 5, // 尾距
    EN_TSH_CARINFO_CALIMODE     = 6 // 模式
}EN_TSH_CARINFO_TYPE;

// 外部命令事件
typedef enum {
    EN_TSH_CMD_EVENT_NONE               = 0,
    EN_TSH_CMD_EVENT_OPTIMIZATION       = 1, // 优化
}EN_TSH_COMMAND_EVENT;

// 直线标定参数类型
typedef enum {
    EN_TSH_STRAIGHT_CALI_NONE           = 0, 
    EN_TSH_STRAIGHT_CALI_LANEWIDTH      = 1, // 线距 单位mm
    EN_TSH_STRAIGHT_CALI_LEFTDIST       = 2, // 左距 单位mm
    EN_TSH_STRAIGHT_CALI_RIGHTDIST      = 3  // 右距 单位mm
}EN_TSH_STRAIGHT_CALI_PARAM;

// 直线标定参数
typedef struct StraightCaliParams {
    int nLaneWidth;      // 线距 单位mm
    int nLeftDist;       // 左距 单位mm
    int nRightDist;      // 右距 单位mm
}__attribute__((packed)) stStraightCaliParams_t;

typedef enum {
    EN_TSH_RADAR_NUMBER_FRONT  = 0, // 车身前雷达，最大数为16
    EN_TSH_RADAR_NUMBER_REAR   = 1, // 车身后雷达，最大数为16
    EN_TSH_RADAR_NUMBER_LEFT   = 2, // 车身左雷达，最大数为16
    EN_TSH_RADAR_NUMBER_RIGHT  = 3, // 车身右雷达，最大数为16
    EN_TSH_RADAR_NUMBER_MAX,
}EN_TSH_RADAR_NUMBER;

// 雷达提示位置
typedef enum {
    // 前雷达，从左到右算起
    EN_TSH_RADAR_POS_F1     = 0,
    EN_TSH_RADAR_POS_F2,
    EN_TSH_RADAR_POS_F3,
    EN_TSH_RADAR_POS_F4,
    EN_TSH_RADAR_POS_F5,
    EN_TSH_RADAR_POS_F6,
    EN_TSH_RADAR_POS_F7,
    EN_TSH_RADAR_POS_F8,
    EN_TSH_RADAR_POS_F9,
    EN_TSH_RADAR_POS_F10,
    EN_TSH_RADAR_POS_F11,
    EN_TSH_RADAR_POS_F12,
    EN_TSH_RADAR_POS_F13,
    EN_TSH_RADAR_POS_F14,
    EN_TSH_RADAR_POS_F15,
    EN_TSH_RADAR_POS_F16,
    // 后雷达，从左到右算起
    EN_TSH_RADAR_POS_B1     = 16,
    EN_TSH_RADAR_POS_B2,
    EN_TSH_RADAR_POS_B3,
    EN_TSH_RADAR_POS_B4,
    EN_TSH_RADAR_POS_B5,
    EN_TSH_RADAR_POS_B6,
    EN_TSH_RADAR_POS_B7,
    EN_TSH_RADAR_POS_B8,
    EN_TSH_RADAR_POS_B9,
    EN_TSH_RADAR_POS_B10,
    EN_TSH_RADAR_POS_B11,
    EN_TSH_RADAR_POS_B12,
    EN_TSH_RADAR_POS_B13,
    EN_TSH_RADAR_POS_B14,
    EN_TSH_RADAR_POS_B15,
    EN_TSH_RADAR_POS_B16,

	 // 左雷达，从头到尾算起
    EN_TSH_RADAR_POS_L1     = 32,
    EN_TSH_RADAR_POS_L2,
    EN_TSH_RADAR_POS_L3,
    EN_TSH_RADAR_POS_L4,
    EN_TSH_RADAR_POS_L5,
    EN_TSH_RADAR_POS_L6,
    EN_TSH_RADAR_POS_L7,
    EN_TSH_RADAR_POS_L8,
    EN_TSH_RADAR_POS_L9,
    EN_TSH_RADAR_POS_L10,
    EN_TSH_RADAR_POS_L11,
    EN_TSH_RADAR_POS_L12,
    EN_TSH_RADAR_POS_L13,
    EN_TSH_RADAR_POS_L14,
    EN_TSH_RADAR_POS_L15,
    EN_TSH_RADAR_POS_L16,

	 // 右雷达，从头到尾算起
    EN_TSH_RADAR_POS_R1     = 48,
    EN_TSH_RADAR_POS_R2,
    EN_TSH_RADAR_POS_R3,
    EN_TSH_RADAR_POS_R4,
    EN_TSH_RADAR_POS_R5,
    EN_TSH_RADAR_POS_R6,
    EN_TSH_RADAR_POS_R7,
    EN_TSH_RADAR_POS_R8,
    EN_TSH_RADAR_POS_R9,
    EN_TSH_RADAR_POS_R10,
    EN_TSH_RADAR_POS_R11,
    EN_TSH_RADAR_POS_R12,
    EN_TSH_RADAR_POS_R13,
    EN_TSH_RADAR_POS_R14,
    EN_TSH_RADAR_POS_R15,
    EN_TSH_RADAR_POS_R16,
    
    EN_TSH_RADAR_POS_MAX    = 64,
}EN_TSH_RADAR_POSITION;

// 雷达提示状态
typedef enum {
    EN_TSH_RADAR_STATE_NONE         = 0, // 关闭
    EN_TSH_RADAR_STATE_NORMAL       = 1, // 正常
    EN_TSH_RADAR_STATE_ATTENTION    = 2, // 注意
    EN_TSH_RADAR_STATE_WARNING      = 3, // 警告
    EN_TSH_RADAR_STATE_DISABLE      = 4, // 不可用
    EN_TSH_RADAR_STATE_MALFUNCTION  = 5, // 故障
}EN_TSH_RADAR_STATE;

// 雷达信息
typedef struct RadarInfo {
    uint8_t arrRadarNums[EN_TSH_RADAR_NUMBER_MAX];
    uint8_t arrPositions[EN_TSH_RADAR_POS_MAX];
}__attribute__((packed)) stRadarInfo_t;

// 棋盘参数
typedef struct ChessParams {
    int32_t nChessMode;                     // 棋盘格模式 1 后装 2 sd 3 sf
    int32_t nLRChessSpaceDist;              // 左右棋盘格之间的距离，<=0则表示不修改值
    int32_t nFBChessSpaceDist;              // 前后棋盘格之间的距离，<=0则表示不修改值
    int32_t nFrontLRDist;                   // 前与左右棋盘格之间的距离，<=0则表示不修改值
    int32_t nFrontBorderSize;               // 前棋盘格与车距离，<0则表示不修改值
    int32_t nBackBorderSize;                // 后棋盘格与车距离，<0则表示不修改值
    int32_t nLeftBorderSize;                // 左棋盘格与车距离，<0则表示不修改值
    int32_t nRightBorderSize;               // 右棋盘格与车距离，<0则表示不修改值
}__attribute__((packed)) stChessParams_t;

// 标定类型
typedef enum {
    EN_TSH_CALIBRATION_NONE    = 0,
    EN_TSH_CALIBRATION_CHESSBOARD      = 1, // 棋盘格标定
    EN_TSH_CALIBRATION_BEELINE         = 2  // 直线标定
}EN_TSH_CALIBRATION_TYPE;

// 视野类型
typedef enum {
    EN_TSH_CALI_VISION_NONE         = 0,
    EN_TSH_CALI_VISION_BIG          = 1, // 大视野
    EN_TSH_CALI_VISION_MIDDLE       = 2, // 中视野
    EN_TSH_CALI_VISION_SMALL        = 3, // 小视野
    EN_TSH_CALI_VISION_HIGH         = 4  // 超视野
}EN_TSH_CALI_VISION;

typedef struct Calibration {
    uint32_t nCaliType;     // 标定类型
    uint32_t nVisionMode;   // 标定视野
}__attribute__((packed)) stCalibration_t;

// 电源类型
typedef enum {
    EN_TSH_POWER_TYP_NONE       = 0,
    EN_TSH_POWER_TYP_CAMERAS    = 1     // 四个摄像头
}EN_TSH_POWER_TYPE;

// 电源状态
typedef enum {
    EN_TSH_POWER_STAT_NONE  = 0,
    EN_TSH_POWER_STAT_ON    = 1,    // 开
    EN_TSH_POWER_STAT_OFF   = 2     // 关
}EN_TSH_POWER_STATE;

typedef struct PowerState{
    uint32_t nType;
    uint32_t nState;
}__attribute__((packed)) stPowerState_t;

// 通知adas状态(uint32_t)
typedef enum {
    EN_ADAS_STATE_NONE              = 0,
    EN_ADAS_STATE_LEFT_BSDS         = 0x00000001,
    EN_ADAS_STATE_RIGHT_BSDS        = 0x00000002,
    EN_ADAS_STATE_LEFT_LDWS         = 0x00000004,
    EN_ADAS_STATE_RIGHT_LDWS        = 0x00000008,   
}EN_ADAS_STATE;

// 通知图像状态(uint32_t)
typedef enum {
    EN_IMAGE_STATE_NONE              = 0,
    EN_IMAGE_STATE_OPEN              = 0x00000001,  
}EN_IMAGE_STATE;


// 灯的位置
typedef enum {
    EN_LIGHT_POSITION_NONE          = 0,
    EN_LIGHT_POSITION_LEFT_TURN     = 1, // 左转向灯源
    EN_LIGHT_POSITION_RIGHT_TURN    = 2, // 右转向灯源
}EN_LIGHT_POSITION;

// 灯的状态
typedef enum {
    EN_LIGHT_STATE_NONE             = 0,
    EN_LIGHT_STATE_ON               = 1,
    EN_LIGHT_STATE_OFF              = 2,
}EN_LIGHT_STATE;

// 轨迹线类型
typedef enum {
    EN_TRACKLINE_TYPE_NONE              = 0,
    EN_TRACKLINE_TYPE_FRONT_DYNAMIC     = 1, // 前动态轨迹线
    EN_TRACKLINE_TYPE_REAR_DYNAMIC      = 2, // 后动态轨迹线
    EN_TRACKLINE_TYPE_FRONT_STATIC      = 3, // 前静态轨迹线
    EN_TRACKLINE_TYPE_REAR_STATIC       = 4, // 后静态轨迹线
    EN_TRACKLINE_TYPE_FRONT             = 5, // 前动态轨迹线+前静态轨迹线
    EN_TRACKLINE_TYPE_REAR              = 6, // 后动态轨迹线+后静态轨迹线
    EN_TRACKLINE_TYPE_LEFT              = 7, // 左辅助线
    EN_TRACKLINE_TYPE_RIGHT             = 8, // 右辅助线
}EN_TRACKLINE_TYPE;

// 轨迹线状态
typedef enum {
    EN_TRACKLINE_STATE_NONE         = 0,
    EN_TRACKLINE_STATE_ON           = 1,
    EN_TRACKLINE_STATE_OFF          = 2,
}EN_TRACKLINE_STATE;

// 设置轨迹线状态的数据结构
typedef struct TrackLineData {
    uint32_t nType;
    uint32_t nState;
}__attribute__((packed)) stTrackLineData_t;

// 车模
typedef struct CarModelAndColor {
    uint32_t nModel;
    uint32_t nColor;
}__attribute__((packed)) stCarModelAndColor_t;

// 调色类型
typedef enum {
    EN_TONING_LUMINANCE     = 1, // 亮度
    EN_TONING_CONTRAST      = 2, // 对比度
    EN_TONING_SATURATION    = 3, // 饱和度
    EN_TONING_HUE           = 4, // 色度
}EN_TONING_TYPE;

// 调色
typedef struct ToningData {
    uint32_t nType; // 值参考枚举类型EN_TONING_TYPE
    uint32_t nValue; // 一般是1~99
}__attribute__((packed)) stToningData_t;

#endif // __TSHINNERDEF_H__

