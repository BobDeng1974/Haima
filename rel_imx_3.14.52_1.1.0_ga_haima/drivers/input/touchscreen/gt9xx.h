#ifndef TPD_CUSTOM_GT9XX_H__
#define TPD_CUSTOM_GT9XX_H__

/* Pre-defined definition */

#define TPD_KEY_COUNT   4
#define key_1           60,850              //auto define
#define key_2           180,850
#define key_3           300,850
#define key_4           420,850

#define TPD_KEYS        {KEY_HOME,KEY_MENU,KEY_BACK,KEY_SEARCH}
#define TPD_KEYS_DIM    {{key_1,50,30},{key_2,50,30},{key_3,50,30},{key_4,50,30}}

//***************************PART1:ON/OFF define*******************************
#define GTP_CUSTOM_CFG              1
#define GTP_DRIVER_SEND_CFG         1       // driver send config to TP in intilization
#define GTP_HAVE_TOUCH_KEY          0
#define GTP_POWER_CTRL_SLEEP        0       // turn off/on power on suspend/resume

#define GTP_AUTO_UPDATE             0       // auto updated fw by .bin file
#define GTP_HEADER_FW_UPDATE        0       // auto updated fw by gtp_default_FW in gt9xx_firmware.h, function together with GTP_AUTO_UDPATE
#define GTP_AUTO_UPDATE_CFG         0       // auto update config by .cfg file, function together with GTP_AUTO_UPDATE

#define GTP_SUPPORT_I2C_DMA         0       // if gt9xxf, better enable it if hardware platform supported
#define GTP_COMPATIBLE_MODE         0       // compatible with GT9XXF

#define GTP_CREATE_WR_NODE          1
#define GTP_ESD_PROTECT             1       // esd protection with a cycle of 2 seconds
#define GTP_CHARGER_SWITCH          0       // charger plugin & plugout detect
#define GTP_WITH_PEN                0
#define GTP_SLIDE_WAKEUP            0
#define GTP_DBL_CLK_WAKEUP          0       // double-click wakup, function together with GTP_SLIDE_WAKEUP
//#define TPD_PROXIMITY
//#define TPD_HAVE_BUTTON                   // report key as coordinate,Vibration feedback
// #define TPD_WARP_X                        // mirrored x coordinate
// #define TPD_WARP_Y                        // mirrored y coordinate
#define GTP_DEBUG_ON                1
#define GTP_DEBUG_ARRAY_ON          0
#define GTP_DEBUG_FUNC_ON           0

//***************************PART2:TODO define**********************************
//STEP_1(REQUIRED):Change config table.
// Sensor_ID Map:
/* sensor_opt1 sensor_opt2 Sensor_ID 
    GND         GND         0 
    VDDIO       GND         1 
    NC          GND         2 
    GND         NC/300K     3 
    VDDIO       NC/300K     4 
    NC          NC/300K     5 
*/
// TODO: define your own default or for Sensor_ID == 0 config here. 
// The predefined one is just a sample config, which is not suitable for your tp in most cases.
#define CTP_CFG_GROUP1 {\
    0x44,0x00,0x04,0x58,0x02,0x05,0x4D,0x00,0x02,0x2A,\
    0x28,0x0F,0x5F,0x46,0x03,0x05,0x00,0x00,0x00,0x00,\
    0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x8C,0x2E,0x0E,\
    0x28,0x23,0xBB,0x0B,0x00,0x00,0x00,0x9A,0x02,0x2D,\
    0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
    0x00,0x19,0x5A,0x94,0xC5,0x02,0x07,0x00,0x00,0x04,\
    0x9B,0x1C,0x00,0x7A,0x25,0x00,0x64,0x2F,0x00,0x51,\
    0x3D,0x00,0x43,0x4F,0x00,0x43,0x00,0x00,0x00,0x00,\
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
    0x00,0x00,0x1C,0x1A,0x18,0x16,0x14,0x12,0x10,0x0E,\
    0x0C,0x0A,0x08,0x06,0x04,0x02,0x00,0x00,0x00,0x00,\
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
    0x00,0x00,0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0F,\
    0x10,0x12,0x13,0x14,0x16,0x18,0x1C,0x1D,0x1E,0x1F,\
    0x20,0x21,0x22,0x24,0x26,0x28,0x29,0x2A,0x00,0x00,\
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
    0x00,0x00,0x00,0x00,0x10,0x01\
    }

// TODO: define your config for Sensor_ID == 1 here, if needed
#define CTP_CFG_GROUP2 {\
    }

// TODO: define your config for Sensor_ID == 2 here, if needed
#define CTP_CFG_GROUP3 {\
    }

// TODO: define your config for Sensor_ID == 3 here, if needed
#define CTP_CFG_GROUP4 {\
    }

// TODO: define your config for Sensor_ID == 4 here, if needed
#define CTP_CFG_GROUP5 {\
    }

// TODO: define your config for Sensor_ID == 5 here, if needed
#define CTP_CFG_GROUP6 {\
    }

// STEP_2(REQUIRED): Customize your I/O ports & I/O operations here


#define GTP_GPIO_AS_INPUT(pin) \
	do{ \
		gpio_direction_input(pin); \
	}while(0)

#define GTP_GPIO_AS_INT(pin) \
	do{ \
		gpio_direction_input(pin); \
	} while(0)

#define GTP_GPIO_GET_VALUE(pin) \
	gpio_get_value(pin)

#define GTP_GPIO_OUTPUT(pin,level) \
	do{\
		gpio_direction_output(pin, level); \
	}while(0)

#define GTP_GPIO_REQUEST(pin, label) \
	gpio_request(pin, label)

#define GTP_GPIO_FREE(pin) \
	gpio_free(pin)

#define GTP_IRQ_TAB		{IRQ_TYPE_EDGE_RISING, IRQ_TYPE_EDGE_FALLING, IRQ_TYPE_LEVEL_LOW, IRQ_TYPE_LEVEL_HIGH}

// STEP_3(optional):Custom set some config by themself,if need.
#if GTP_CUSTOM_CFG
 #define GTP_MAX_HEIGHT   720
 #define GTP_MAX_WIDTH    1280
 #define GTP_INT_TRIGGER  1    //0:Rising 1:Falling
#else 
 #define GTP_MAX_HEIGHT   4096
 #define GTP_MAX_WIDTH    4096
 #define GTP_INT_TRIGGER  1
#endif
#define GTP_MAX_TOUCH      5
#define VELOCITY_CUSTOM
#define TPD_VELOCITY_CUSTOM_X 15
#define TPD_VELOCITY_CUSTOM_Y 15

//STEP_4(optional):If this project have touch key,Set touch key config.
#if GTP_HAVE_TOUCH_KEY
	#define GTP_KEY_TAB  {KEY_HOME, KEY_MENU, KEY_BACK, KEY_SEARCH}
#endif

//***************************PART3:OTHER define*********************************
#define GTP_DRIVER_VERSION          "V2.0<2013/08/28>"
#define GTP_I2C_NAME                "Goodix-TS"
#define GT91XX_CONFIG_PROC_FILE     "gt9xx_config"
#define GTP_POLL_TIME               10
#define GTP_ADDR_LENGTH             2
#define GTP_CONFIG_MIN_LENGTH       186
#define GTP_CONFIG_MAX_LENGTH       240
#define FAIL                        0
#define SUCCESS                     1
#define SWITCH_OFF                  0
#define SWITCH_ON                   1

#define CFG_GROUP_LEN(p_cfg_grp)  (sizeof(p_cfg_grp) / sizeof(p_cfg_grp[0]))

//******************** For GT9XXF Start **********************//
#if GTP_COMPATIBLE_MODE
typedef enum
{
    CHIP_TYPE_GT9  = 0,
    CHIP_TYPE_GT9F = 1,
} CHIP_TYPE_T;
#endif

#define GTP_REG_MATRIX_DRVNUM           0x8069
#define GTP_REG_MATRIX_SENNUM           0x806A
#define GTP_REG_RQST                    0x8043
#define GTP_REG_BAK_REF                 0x99D0
#define GTP_REG_MAIN_CLK                0x8020
#define GTP_REG_CHIP_TYPE               0x8000
#define GTP_REG_HAVE_KEY                0x804E

#define GTP_FL_FW_BURN              0x00
#define GTP_FL_ESD_RECOVERY         0x01
#define GTP_FL_READ_REPAIR          0x02

#define GTP_BAK_REF_SEND                0
#define GTP_BAK_REF_STORE               1
#define CFG_LOC_DRVA_NUM                29
#define CFG_LOC_DRVB_NUM                30
#define CFG_LOC_SENS_NUM                31

#define GTP_CHK_FW_MAX                  1000
#define GTP_CHK_FS_MNT_MAX              300
#define GTP_BAK_REF_PATH                "/data/gtp_ref.bin"
#define GTP_MAIN_CLK_PATH               "/data/gtp_clk.bin"
#define GTP_RQST_CONFIG                 0x01
#define GTP_RQST_BAK_REF                0x02
#define GTP_RQST_RESET                  0x03
#define GTP_RQST_MAIN_CLOCK             0x04
#define GTP_RQST_RESPONDED              0x00
#define GTP_RQST_IDLE                   0xFF

//******************** For GT9XXF End **********************//

//Register define
#define GTP_READ_COOR_ADDR          0x814E
#define GTP_REG_SLEEP               0x8040
#define GTP_REG_SENSOR_ID           0x814A
#define GTP_REG_CONFIG_DATA         0x8047
#define GTP_REG_VERSION             0x8140
#define GTP_REG_HW_INFO             0x4220

#define RESOLUTION_LOC              3
#define TRIGGER_LOC                 8

#define I2C_MASTER_CLOCK            300
#define I2C_BUS_NUMBER                  1     // I2C Bus for TP, mt6572
#define GTP_DMA_MAX_TRANSACTION_LENGTH  255   // for DMA mode
#define GTP_DMA_MAX_I2C_TRANSFER_SIZE   (GTP_DMA_MAX_TRANSACTION_LENGTH - GTP_ADDR_LENGTH)
#define MAX_TRANSACTION_LENGTH          8
#define MAX_I2C_TRANSFER_SIZE           (MAX_TRANSACTION_LENGTH - GTP_ADDR_LENGTH)
#define TPD_MAX_RESET_COUNT             3
#define TPD_CALIBRATION_MATRIX          {962,0,0,0,1600,0,0,0};


#define TPD_RESET_ISSUE_WORKAROUND
#define TPD_HAVE_CALIBRATION
#define TPD_NO_GPIO
#define TPD_RESET_ISSUE_WORKAROUND

#define TPD_WARP_X(x_max, x) ( x_max - 1 - x )
#define TPD_WARP_Y(y_max, y) ( y_max - 1 - y )

//Log define
#define GTP_INFO(fmt,arg...)            printk("<<-GTP-INFO->> "fmt"\n",##arg)
#define GTP_ERROR(fmt,arg...)           printk("<<-GTP-ERROR->> "fmt"\n",##arg)
#define GTP_DEBUG(fmt,arg...)           do{\
                                            if(GTP_DEBUG_ON)\
                                                printk("<<-GTP-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg);\
                                        }while(0)
#define GTP_DEBUG_ARRAY(array, num)     do{\
                                            s32 i;\
                                            u8* a = array;\
                                            if(GTP_DEBUG_ARRAY_ON)\
                                            {\
                                                printk("<<-GTP-DEBUG-ARRAY->>\n");\
                                                for (i = 0; i < (num); i++)\
                                                {\
                                                    printk("%02x   ", (a)[i]);\
                                                    if ((i + 1 ) %10 == 0)\
                                                    {\
                                                        printk("\n");\
                                                    }\
                                                }\
                                                printk("\n");\
                                            }\
                                        }while(0)
#define GTP_DEBUG_FUNC()                do{\
                                            if(GTP_DEBUG_FUNC_ON)\
                                                printk("<<-GTP-FUNC->> Func:%s@Line:%d\n",__func__,__LINE__);\
                                        }while(0)
#define GTP_SWAP(x, y)                    do{\
                                            typeof(x) z = x;\
                                            x = y;\
                                            y = z;\
                                            }while (0)


//*****************************End of Part III********************************

#endif /* TPD_CUSTOM_GT9XX_H__ */
