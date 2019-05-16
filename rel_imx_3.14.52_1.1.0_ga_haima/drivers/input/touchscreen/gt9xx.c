#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <asm/uaccess.h>
#include <linux/hrtimer.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/of_gpio.h>
#include <linux/irq.h>
#include <linux/byteorder/generic.h>

#include <linux/input/mt.h>

#include "gt9xx.h"

struct gt9xx_data {
	int gpio_rst;
	int gpio_irq;
	int irq;
	int irq_is_disable;
	int use_irq;
	int int_trigger_type;
	int enter_update;
	int gtp_rawdiff_mode;
	int gtp_cfg_len;
	int abs_x_max;
	int abs_y_max;
	bool swap_xy;
	bool wrap_x;
	bool wrap_y;
	spinlock_t irq_lock;
	struct hrtimer timer;
	struct input_dev *input_dev;
	struct work_struct work;
	struct workqueue_struct *goodix_wq;
	struct i2c_client *client;
};

static const char *goodix_ts_name = "Goodix Capacitive TouchScreen";

static u8 config[GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH]
	= {GTP_REG_CONFIG_DATA >> 8, GTP_REG_CONFIG_DATA & 0xff};

#if GTP_HAVE_TOUCH_KEY
static const u16 touch_key_array[] = GTP_KEY_TAB;
#define GTP_MAX_KEY_NUM	 ARRAY_SIZE(touch_key_array)
#endif

static s8 gtp_i2c_test(struct gt9xx_data *ts);
void gtp_reset_guitar(struct gt9xx_data *ts, s32 ms);
void gtp_int_sync(struct gt9xx_data* ts, s32 ms);

s32 gtp_i2c_read(struct gt9xx_data *ts, u8 *buf, s32 len)
{
    struct i2c_msg msgs[2];
    s32 ret=-1;
    s32 retries = 0;

    GTP_DEBUG_FUNC();

    msgs[0].flags = !I2C_M_RD;
    msgs[0].addr  = ts->client->addr;
    msgs[0].len   = GTP_ADDR_LENGTH;
    msgs[0].buf   = &buf[0];

    msgs[1].flags = I2C_M_RD;
    msgs[1].addr  = ts->client->addr;
    msgs[1].len   = len - GTP_ADDR_LENGTH;
    msgs[1].buf   = &buf[GTP_ADDR_LENGTH];

    while(retries < 5) {
        ret = i2c_transfer(ts->client->adapter, msgs, 2);
        if(ret == 2) break;
			retries++;
    }
	
    if(retries >= 5) {
        GTP_DEBUG("I2C retry timeout, reset chip.");
        gtp_reset_guitar(ts, 10);
    }
    return ret;
}

s32 gtp_i2c_write(struct gt9xx_data *ts, u8 *buf,s32 len)
{
    struct i2c_msg msg;
    s32 ret=-1;
    s32 retries = 0;

    GTP_DEBUG_FUNC();

    msg.flags = !I2C_M_RD;
    msg.addr  = ts->client->addr;
    msg.len   = len;
    msg.buf   = buf;

    while(retries < 5) {
        ret = i2c_transfer(ts->client->adapter, &msg, 1);
        if (ret == 1)break;
        retries++;
    }
    if(retries >= 5) {
        GTP_DEBUG("I2C retry timeout, reset chip.");
        gtp_reset_guitar(ts, 10);
    }
    return ret;
}

s32 gtp_send_cfg(struct gt9xx_data *ts)
{
    s32 ret = 0;
#if GTP_DRIVER_SEND_CFG
    s32 retry = 0;

    for (retry = 0; retry < 5; retry++) {
        ret = gtp_i2c_write(ts, config , GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH);
        if (ret > 0)
            return ret;
    }
#endif

    return ret;
}

void gtp_irq_disable(struct gt9xx_data *ts)
{
    unsigned long irqflags;

    GTP_DEBUG_FUNC();

    spin_lock_irqsave(&ts->irq_lock, irqflags);
    if (!ts->irq_is_disable)
    {
        ts->irq_is_disable = 1; 
        disable_irq_nosync(ts->client->irq);
    }
    spin_unlock_irqrestore(&ts->irq_lock, irqflags);
}

void gtp_irq_enable(struct gt9xx_data *ts)
{
    unsigned long irqflags = 0;
    GTP_DEBUG_FUNC();

    spin_lock_irqsave(&ts->irq_lock, irqflags);
    if (ts->irq_is_disable) {
        enable_irq(ts->client->irq);
        ts->irq_is_disable = 0;	
    }
    spin_unlock_irqrestore(&ts->irq_lock, irqflags);
}

/*******************************************************
Function:
	Touch down report function.

Input:
	ts:private data.
	id:tracking id.
	x:input x.
	y:input y.
	w:input weight.
	
Output:
	None.
*******************************************************/
static void gtp_touch_down(struct gt9xx_data* ts,s32 id,s32 x,s32 y,s32 w)
{
	if(ts->swap_xy)
		GTP_SWAP(x, y);

	if(ts->wrap_x)
		x = TPD_WARP_X(ts->abs_x_max, x);
	if(ts->wrap_y)
		y = TPD_WARP_Y(ts->abs_y_max, y);
#ifdef GTP_ICS_SLOT_REPORT
    input_mt_slot(ts->input_dev, id);
    input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, id);
    input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
    input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);
    input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, w);
    input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, w);
#else
    input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
    input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);
    input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, w);
    input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, w);
    input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, id);
    input_mt_sync(ts->input_dev);
#endif

    // GTP_DEBUG("ID:%d, X:%d, Y:%d, W:%d", id, x, y, w);
}

/*******************************************************
Function:
	Touch up report function.

Input:
	ts:private data.
	
Output:
	None.
*******************************************************/
static void gtp_touch_up(struct gt9xx_data* ts, s32 id)
{
#ifdef GTP_ICS_SLOT_REPORT
    input_mt_slot(ts->input_dev, id);
    input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, -1);
    // GTP_DEBUG("Touch id[%2d] release!", id);
#else
    input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
    input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
    input_mt_sync(ts->input_dev);
#endif
}

/*******************************************************
Function:
	Goodix touchscreen work function.

Input:
	work:	work_struct of goodix_wq.
	
Output:
	None.
*******************************************************/
static void goodix_ts_work_func(struct work_struct *work)
{
    u8  end_cmd[3] = {GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF, 0};
    u8  point_data[2 + 1 + 8 * GTP_MAX_TOUCH + 1]={GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF};
    u8  touch_num = 0;
    u8  finger = 0;
    static u16 pre_touch = 0;
    static u8 pre_key = 0;
    u8  key_value = 0;
    u8* coor_data = NULL;
    s32 input_x = 0;
    s32 input_y = 0;
    s32 input_w = 0;
    s32 id = 0;
    s32 i  = 0;
    s32 ret = -1;
    struct gt9xx_data *ts = NULL;

    GTP_DEBUG_FUNC();

    ts = container_of(work, struct gt9xx_data, work);
    if (ts->enter_update) {
        return;
    }

    ret = gtp_i2c_read(ts, point_data, 12);
    if (ret < 0) {
        GTP_ERROR("I2C transfer error. errno:%d\n ", ret);
        goto exit_work_func;
    }

    finger = point_data[GTP_ADDR_LENGTH];    
    if((finger & 0x80) == 0) {
        goto exit_work_func;
    }

    touch_num = finger & 0x0f;
    if (touch_num > GTP_MAX_TOUCH) {
        goto exit_work_func;
    }

    if (touch_num > 1) {
        u8 buf[8 * GTP_MAX_TOUCH] = {(GTP_READ_COOR_ADDR + 10) >> 8, (GTP_READ_COOR_ADDR + 10) & 0xff};

        ret = gtp_i2c_read(ts, buf, 2 + 8 * (touch_num - 1)); 
        memcpy(&point_data[12], &buf[2], 8 * (touch_num - 1));
    }

#if GTP_HAVE_TOUCH_KEY
    key_value = point_data[3 + 8 * touch_num];
    
    if(key_value || pre_key) {
        for (i = 0; i < GTP_MAX_KEY_NUM; i++) {
            input_report_key(ts->input_dev, touch_key_array[i], key_value & (0x01<<i));   
        }
        touch_num = 0;
        pre_touch = 0;
    }
#endif
    pre_key = key_value;

    // GTP_DEBUG("pre_touch:%02x, finger:%02x.", pre_touch, finger);

#ifdef GTP_ICS_SLOT_REPORT
    if (pre_touch || touch_num) {
        s32 pos = 0;
        u16 touch_index = 0;

        coor_data = &point_data[3];
        if(touch_num) {
            id = coor_data[pos] & 0x0F;
            touch_index |= (0x01<<id);
        }

        GTP_DEBUG("id=%d,touch_index=0x%x,pre_touch=0x%x\n",id, touch_index,pre_touch);
        for (i = 0; i < GTP_MAX_TOUCH; i++) {
            if (touch_index & (0x01<<i)) {
                input_x  = coor_data[pos + 1] | coor_data[pos + 2] << 8;
                input_y  = coor_data[pos + 3] | coor_data[pos + 4] << 8;
                input_w  = coor_data[pos + 5] | coor_data[pos + 6] << 8;

                gtp_touch_down(ts, id, input_x, input_y, input_w);
                pre_touch |= 0x01 << i;

                pos += 8;
                id = coor_data[pos] & 0x0F;
                touch_index |= (0x01<<id);
            } else {
                gtp_touch_up(ts, i);
                pre_touch &= ~(0x01 << i);
            }
        }
    }

#else
    if (touch_num )
    {
        for (i = 0; i < touch_num; i++)
        {
            coor_data = &point_data[i * 8 + 3];

            id = coor_data[0] & 0x0F;
            input_x  = coor_data[1] | coor_data[2] << 8;
            input_y  = coor_data[3] | coor_data[4] << 8;
            input_w  = coor_data[5] | coor_data[6] << 8;

            gtp_touch_down(ts, id, input_x, input_y, input_w);
        }
    }
    else if (pre_touch)
    {
        // GTP_DEBUG("Touch Release!");
        gtp_touch_up(ts, 0);
    }

    pre_touch = touch_num;
    input_report_key(ts->input_dev, BTN_TOUCH, (touch_num || key_value));
#endif

    input_sync(ts->input_dev);

exit_work_func:
    if(!ts->gtp_rawdiff_mode)
    {
        ret = gtp_i2c_write(ts, end_cmd, 3);
        if (ret < 0)
        {
            GTP_INFO("I2C write end_cmd  error!"); 
        }
    }

    if (ts->use_irq)
    {
        gtp_irq_enable(ts);
    }
}

/*******************************************************
Function:
	Timer interrupt service routine.

Input:
	timer:	timer struct pointer.
	
Output:
	Timer work mode. HRTIMER_NORESTART---not restart mode
*******************************************************/
static enum hrtimer_restart goodix_ts_timer_handler(struct hrtimer *timer)
{
    struct gt9xx_data *ts = container_of(timer, struct gt9xx_data, timer);

    GTP_DEBUG_FUNC();

    queue_work(ts->goodix_wq, &ts->work);
    hrtimer_start(&ts->timer, ktime_set(0, (GTP_POLL_TIME+6)*1000000), HRTIMER_MODE_REL);
    return HRTIMER_NORESTART;
}

/*******************************************************
Function:
	External interrupt service routine.

Input:
	irq:	interrupt number.
	dev_id: private data pointer.
	
Output:
	irq execute status.
*******************************************************/
static irqreturn_t goodix_ts_irq_handler(int irq, void *dev_id)
{
    struct gt9xx_data *ts = dev_id;

    GTP_DEBUG_FUNC();

    gtp_irq_disable(ts);
    queue_work(ts->goodix_wq, &ts->work);

    return IRQ_HANDLED;
}
/*******************************************************
Function:
	Int sync Function.

Input:
	ms:sync time.
	
Output:
	None.
*******************************************************/
void gtp_int_sync(struct gt9xx_data* ts, s32 ms)
{
    GTP_GPIO_OUTPUT(ts->gpio_irq, 0);
    msleep(ms);
    GTP_GPIO_AS_INT(ts->gpio_irq);
}

/*******************************************************
Function:
	Reset chip Function.

Input:
	ms:reset time.
	
Output:
	None.
*******************************************************/
void gtp_reset_guitar(struct gt9xx_data *ts, s32 ms)
{
    GTP_DEBUG_FUNC();

    GTP_GPIO_OUTPUT(ts->gpio_rst, 0);   //begin select I2C slave addr
    msleep(ms);
    GTP_GPIO_OUTPUT(ts->gpio_irq, ts->client->addr == 0x14);

    msleep(2);
    GTP_GPIO_OUTPUT(ts->gpio_rst, 1);
    
    msleep(6);                          //must > 3ms
    GTP_GPIO_AS_INPUT(ts->gpio_rst);    //end select I2C slave addr
    
    gtp_int_sync(ts, 50);
}

/*******************************************************
Function:
	Eter sleep function.

Input:
	ts:private data.
	
Output:
	Executive outcomes.0--success,non-0--fail.
*******************************************************/
static s8 gtp_enter_sleep(struct gt9xx_data* ts)
{
    s8 ret = -1;
    s8 retry = 0;
    u8 i2c_control_buf[3] = {(u8)(GTP_REG_SLEEP >> 8), (u8)GTP_REG_SLEEP, 5};

    GTP_DEBUG_FUNC();

    GTP_GPIO_OUTPUT(ts->gpio_irq, 0);
    msleep(5);
    while(retry++ < 5) {
        ret = gtp_i2c_write(ts, i2c_control_buf, 3);
        if (ret > 0) {
            GTP_DEBUG("GTP enter sleep!");
            return ret;
        }
        msleep(10);
    }
    GTP_ERROR("GTP send sleep cmd failed.");
    return ret;
}

/*******************************************************
Function:
	Wakeup from sleep mode Function.

Input:
	ts:	private data.
	
Output:
	Executive outcomes.0--success,non-0--fail.
*******************************************************/
static s8 gtp_wakeup_sleep(struct gt9xx_data* ts)
{
    u8 retry = 0;
    s8 ret = -1;

    GTP_DEBUG_FUNC();

#if GTP_POWER_CTRL_SLEEP
    while(retry++ < 5) {
        gtp_reset_guitar(ts, 20);
        ret = gtp_send_cfg(ts);
        if (ret > 0) {
            GTP_DEBUG("Wakeup sleep send config success.");
            return ret;
        }
    }
#else
    while(retry++ < 10)
    {
        GTP_GPIO_OUTPUT(ts->gpio_irq, 1);
        msleep(5);
        
        ret = gtp_i2c_test(ts);
        if (ret > 0)
        {
            GTP_DEBUG("GTP wakeup sleep.");
            
            gtp_int_sync(ts, 25);
            return ret;
        }
        gtp_reset_guitar(ts, 20);
    }
#endif

    GTP_ERROR("GTP wakeup sleep failed.");
    return ret;
}

/*******************************************************
Function:
	GTP initialize function.

Input:
	ts:	i2c client private struct.
	
Output:
	Executive outcomes.0---succeed.
*******************************************************/
static s32 gtp_init_panel(struct gt9xx_data *ts)
{
    s32 ret = -1;
	struct device_node *np = ts->client->dev.of_node;

	ret = of_property_read_u32(np, "abs_max_x", &ts->abs_x_max);
	if (ret) {
		GTP_INFO("abs_max_x missing and default to 4096\n");
		ts->abs_x_max = 4096;
	}

	ret = of_property_read_u32(np, "abs_max_y", &ts->abs_y_max);
	if (ret) {
		GTP_INFO("abs_max_y missing and default to 4096\n");
		ts->abs_y_max = 4096;
	}

	if (of_property_read_bool(np, "enable-swap-xy"))
		ts->swap_xy = true;

	if (of_property_read_bool(np, "enable-warp-x"))
		ts->wrap_x = true;

	if (of_property_read_bool(np, "enable-warp-y"))
		ts->wrap_y = true;

#if GTP_DRIVER_SEND_CFG
    s32 i;
    u8 check_sum = 0;
    u8 rd_cfg_buf[16];

    u8 cfg_info_group1[] = CTP_CFG_GROUP1;
    u8 cfg_info_group2[] = CTP_CFG_GROUP2;
    u8 cfg_info_group3[] = CTP_CFG_GROUP3;
    u8 *send_cfg_buf[3] = {cfg_info_group1, cfg_info_group2, cfg_info_group3};
    u8 cfg_info_len[3] = {sizeof(cfg_info_group1)/sizeof(cfg_info_group1[0]), 
                          sizeof(cfg_info_group2)/sizeof(cfg_info_group2[0]),
                          sizeof(cfg_info_group3)/sizeof(cfg_info_group3[0])};
    for(i=0; i<3; i++)
    {
        if(cfg_info_len[i] > ts->gtp_cfg_len)
        {
            ts->gtp_cfg_len = cfg_info_len[i];
        }
    }
    GTP_DEBUG("len1=%d,len2=%d,len3=%d,send_len:%d",cfg_info_len[0],cfg_info_len[1],cfg_info_len[2],ts->gtp_cfg_len);
    if ((!cfg_info_len[1]) && (!cfg_info_len[2]))
    {
        rd_cfg_buf[GTP_ADDR_LENGTH] = 0; 
    }
    else
    {
        rd_cfg_buf[0] = GTP_REG_SENSOR_ID >> 8;
        rd_cfg_buf[1] = GTP_REG_SENSOR_ID & 0xff;
        ret = gtp_i2c_read(ts, rd_cfg_buf, 3);
        if (ret < 0)
        {
            GTP_ERROR("Read SENSOR ID failed,default use group1 config!");
            rd_cfg_buf[GTP_ADDR_LENGTH] = 0;
        }
        rd_cfg_buf[GTP_ADDR_LENGTH] &= 0x07;
    }
    GTP_DEBUG("SENSOR ID:%d", rd_cfg_buf[GTP_ADDR_LENGTH]);
    memset(&config[GTP_ADDR_LENGTH], 0, GTP_CONFIG_MAX_LENGTH);
    memcpy(&config[GTP_ADDR_LENGTH], send_cfg_buf[rd_cfg_buf[GTP_ADDR_LENGTH]], ts->gtp_cfg_len);

#if GTP_CUSTOM_CFG
    config[RESOLUTION_LOC]     = (u8)ts->abs_x_max;
    config[RESOLUTION_LOC + 1] = (u8)(ts->abs_x_max >> 8);
    config[RESOLUTION_LOC + 2] = (u8)ts->abs_y_max;
    config[RESOLUTION_LOC + 3] = (u8)(ts->abs_y_max >> 8);
    
    if (GTP_INT_TRIGGER == 0)  //RISING
    {
        config[TRIGGER_LOC] &= 0xfe; 
    }
    else if (GTP_INT_TRIGGER == 1)  //FALLING
    {
        config[TRIGGER_LOC] |= 0x01;
    }
#endif  //endif GTP_CUSTOM_CFG
    
    check_sum = 0;
    for (i = GTP_ADDR_LENGTH; i < ts->gtp_cfg_len; i++)
    {
        check_sum += config[i];
    }
    config[ts->gtp_cfg_len] = (~check_sum) + 1;
    
#else //else DRIVER NEED NOT SEND CONFIG

    if(ts->gtp_cfg_len == 0)
    {
        ts->gtp_cfg_len = GTP_CONFIG_MAX_LENGTH;
    }
    ret = gtp_i2c_read(ts, config, ts->gtp_cfg_len + GTP_ADDR_LENGTH);
    if (ret < 0)
    {
        GTP_ERROR("GTP read resolution & max_touch_num failed, use default value!");
        ts->int_trigger_type = GTP_INT_TRIGGER;
    }
#endif //endif GTP_DRIVER_SEND_CFG

    GTP_DEBUG_FUNC();

    ts->abs_x_max = (config[RESOLUTION_LOC + 1] << 8) + config[RESOLUTION_LOC];
    ts->abs_y_max = (config[RESOLUTION_LOC + 3] << 8) + config[RESOLUTION_LOC + 2];
    ts->int_trigger_type = (config[TRIGGER_LOC]) & 0x03;
    if ((!ts->abs_x_max)||(!ts->abs_y_max))
    {
        GTP_ERROR("GTP resolution & max_touch_num invalid, use default value!");
    }

    ret = gtp_send_cfg(ts);
    if (ret < 0)
    {
        GTP_ERROR("Send config error.");
    }

    GTP_DEBUG("X_MAX = %d,Y_MAX = %d,TRIGGER = 0x%02x", ts->abs_x_max, 
		ts->abs_y_max, ts->int_trigger_type);

    msleep(10);

    return 0;
}

/*******************************************************
Function:
	Read goodix touchscreen version function.

Input:
	client:	i2c client struct.
	version:address to store version info
	
Output:
	Executive outcomes.0---succeed.
*******************************************************/
s32 gtp_read_version(struct gt9xx_data *ts, u16* version)
{
    s32 ret = -1;
    s32 i = 0;
    u8 buf[8] = {GTP_REG_VERSION >> 8, GTP_REG_VERSION & 0xff};

    GTP_DEBUG_FUNC();

    ret = gtp_i2c_read(ts, buf, sizeof(buf));
    if (ret < 0)
    {
        GTP_ERROR("GTP read version failed");
        return ret;
    }

    if (version)
    {
        *version = (buf[7] << 8) | buf[6];
    }

    for(i = 2; i < 6; i++) {
        if(!buf[i]) {
            buf[i] = 0x30;
        }
    }
    GTP_INFO("IC VERSION:%c%c%c%c_%02x%02x", 
              buf[2], buf[3], buf[4], buf[5], buf[7], buf[6]);

    return 0;
}

/*******************************************************
Function:
	I2c test Function.

Input:
	client:i2c client.
	
Output:
	Executive outcomes.0--success,non-0--fail.
*******************************************************/
static s8 gtp_i2c_test(struct gt9xx_data *ts)
{
    u8 test[3] = {GTP_REG_CONFIG_DATA >> 8, GTP_REG_CONFIG_DATA & 0xff};
    u8 retry = 0;
    s8 ret = -1;
  
    GTP_DEBUG_FUNC();
  
    while(retry++ < 5) {
        ret = gtp_i2c_read(ts, test, 3);
        if (ret > 0)
            return ret;
        GTP_ERROR("GTP i2c test failed time %d.",retry);
        msleep(10);
    }
    return ret;
}

/*******************************************************
Function:
	Request gpio Function.

Input:
	ts:private data.
	
Output:
	Executive outcomes.0--success,non-0--fail.
*******************************************************/
static s8 gtp_request_io_port(struct gt9xx_data *ts)
{
    s32 ret = 0;
	struct device_node *np = ts->client->dev.of_node;
	
	ts->gpio_irq = of_get_named_gpio(np, "gpio_gt_intr", 0);
	if (!gpio_is_valid(ts->gpio_irq))
		return -ENODEV;
	
	ts->gpio_rst = of_get_named_gpio(np, "gpio_gt_rst", 0);
	if (!gpio_is_valid(ts->gpio_rst))
		return -ENODEV;

    ret = GTP_GPIO_REQUEST(ts->gpio_irq, "ts_irq");
    if (ret < 0) {
        GTP_ERROR("Failed to request GPIO:%d, ERRNO:%d", (s32)ts->gpio_irq, ret);
        ret = -ENODEV;
    } else {
        GTP_GPIO_AS_INT(ts->gpio_irq);	
        ts->client->irq = ts->client->irq;
    }

    ret = GTP_GPIO_REQUEST(ts->gpio_rst, "GTP_RST_PORT");
    if (ret < 0) {
        GTP_ERROR("Failed to request GPIO:%d, ERRNO:%d",(s32)ts->gpio_rst,ret);
        GTP_GPIO_FREE(ts->gpio_irq);
		ret = -ENODEV;
    }

    GTP_GPIO_AS_INPUT(ts->gpio_rst);
    gtp_reset_guitar(ts, 20);

    return ret;
}

/*******************************************************
Function:
	Request irq Function.

Input:
	ts:private data.
	
Output:
	Executive outcomes.0--success,non-0--fail.
*******************************************************/
static s8 gtp_request_irq(struct gt9xx_data *ts)
{
    s32 ret = -1;
    const u8 irq_table[] = GTP_IRQ_TAB;

    GTP_DEBUG("INT trigger type:%x", ts->int_trigger_type);

    ret = devm_request_irq(&ts->client->dev, ts->client->irq, 
			goodix_ts_irq_handler, irq_table[ts->int_trigger_type],
			ts->client->name, ts);
    if (ret) {
        GTP_ERROR("Request IRQ failed!ERRNO:%d.", ret);
        GTP_GPIO_AS_INPUT(ts->gpio_irq);
        GTP_GPIO_FREE(ts->gpio_irq);

        hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        ts->timer.function = goodix_ts_timer_handler;
        hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
        return -1;
    } else {
        gtp_irq_disable(ts);
        ts->use_irq = 1;
        return 0;
    }
}

static s8 gtp_request_input_dev(struct gt9xx_data *ts)
{
    s8 ret = -1;
    s8 phys[32];
#if GTP_HAVE_TOUCH_KEY
    u8 index = 0;
#endif
  
    GTP_DEBUG_FUNC();
  
    ts->input_dev = devm_input_allocate_device(&ts->client->dev);
    if (ts->input_dev == NULL) {
        GTP_ERROR("Failed to allocate input device.");
        return -ENOMEM;
    }

    ts->input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) ;
#ifdef GTP_ICS_SLOT_REPORT
    __set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);
    input_mt_init_slots(ts->input_dev, 255);
#else
    ts->input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
#endif

#if GTP_HAVE_TOUCH_KEY
    for (index = 0; index < GTP_MAX_KEY_NUM; index++) {
        input_set_capability(ts->input_dev,EV_KEY,touch_key_array[index]);	
    }
#endif


	if(ts->swap_xy)
		GTP_SWAP(ts->abs_x_max, ts->abs_y_max);

	input_set_abs_params(ts->input_dev, ABS_X, 0, ts->abs_x_max, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_Y, 0, ts->abs_y_max, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, ts->abs_x_max, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, ts->abs_y_max, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);	
    input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, 255, 0, 0);

    sprintf(phys, "input/ts");
    ts->input_dev->name = goodix_ts_name;
    ts->input_dev->phys = phys;
    ts->input_dev->id.bustype = BUS_I2C;
    ts->input_dev->id.vendor = 0xDEAD;
    ts->input_dev->id.product = 0xBEEF;
    ts->input_dev->id.version = 10427;
	
	input_set_drvdata(ts->input_dev, ts);
	
    ret = input_register_device(ts->input_dev);
    if (ret) {
        GTP_ERROR("Register %s input device failed", ts->input_dev->name);
        return -ENODEV;
    }

    return 0;
}

/*******************************************************
Function:
	Goodix touchscreen probe function.

Input:
	client:	i2c device struct.
	id:device id.
	
Output:
	Executive outcomes. 0---succeed.
*******************************************************/
static int goodix_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    s32 ret = -1;
    u16 version_info;
	struct gt9xx_data *ts;
    GTP_DEBUG_FUNC();
    
    GTP_INFO("GTP Driver Version:%s", GTP_DRIVER_VERSION);
    // GTP_INFO("GTP Driver build@%s,%s", __TIME__, __DATE__);
    GTP_INFO("GTP I2C Address:0x%02x", client->addr);

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        GTP_ERROR("I2C check functionality failed.");
        return -ENODEV;
    }
	
    ts = devm_kzalloc(&client->dev, sizeof(*ts), GFP_KERNEL);
    if (!ts) {
        GTP_ERROR("Alloc GFP_KERNEL memory failed.");
        return -ENOMEM;
    }
   
    memset(ts, 0, sizeof(*ts));
    INIT_WORK(&ts->work, goodix_ts_work_func);
    ts->client = client;
    i2c_set_clientdata(client, ts);
    ts->gtp_rawdiff_mode = 0;
	
	ts->goodix_wq = create_singlethread_workqueue("goodix_wq");
    if (!ts->goodix_wq) {
        GTP_ERROR("Creat workqueue failed.");
        return -ENOMEM;
    }

    ret = gtp_request_io_port(ts);
    if (ret < 0) {
        GTP_ERROR("GTP request IO port failed.");
        return ret;
    }

    ret = gtp_i2c_test(ts);
    if (ret < 0) {
        GTP_ERROR("I2C communication ERROR!");
		goto err1;
    }
    
    ret = gtp_init_panel(ts);
    if (ret < 0) {
        GTP_ERROR("GTP init panel failed.");
    }

    ret = gtp_request_input_dev(ts);
    if (ret < 0) {
        GTP_ERROR("GTP request input dev failed");
    }
    
    ret = gtp_request_irq(ts); 
    if (ret < 0) {
        GTP_INFO("GTP works in polling mode.");
    } else {
        GTP_INFO("GTP works in interrupt mode.");
    }

    ret = gtp_read_version(ts, &version_info);
    if (ret < 0) {
        GTP_ERROR("Read version failed.");
    }
    spin_lock_init(&ts->irq_lock);

    gtp_irq_enable(ts);
	return ret;
err1:
	GTP_GPIO_FREE(ts->gpio_irq);
	GTP_GPIO_FREE(ts->gpio_rst);
	destroy_workqueue(ts->goodix_wq);
    return ret;
}


/*******************************************************
Function:
	Goodix touchscreen driver release function.

Input:
	client:	i2c device struct.
	
Output:
	Executive outcomes. 0---succeed.
*******************************************************/
static int goodix_ts_remove(struct i2c_client *client)
{
    struct gt9xx_data *ts = i2c_get_clientdata(client);
	
    GTP_DEBUG_FUNC();

    if (ts) {
        if (ts->use_irq) {
            GTP_GPIO_AS_INPUT(ts->gpio_irq);
            GTP_GPIO_FREE(ts->gpio_irq);
        } else {
            hrtimer_cancel(&ts->timer);
        }
    }
	
    GTP_INFO("GTP driver is removing...");
    i2c_set_clientdata(client, NULL);

    input_unregister_device(ts->input_dev);
	
	if (ts->goodix_wq)
        destroy_workqueue(ts->goodix_wq);

    return 0;
}

static const struct i2c_device_id goodix_ts_id[] = {
	{ "Goodix-TS", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, goodix_ts_id);

static struct of_device_id goodix_ts_dt_ids[] = {
	{ .compatible = "goodix,gt9xx" },
	{ /* sentinel */ }
};

static struct i2c_driver goodix_ts_driver = {
    .probe      = goodix_ts_probe,
    .remove     = goodix_ts_remove,
    .id_table   = goodix_ts_id,
    .driver = {
        .name     = "Goodix-TS",
        .owner    = THIS_MODULE,
		.of_match_table	= goodix_ts_dt_ids,
    },
};

module_i2c_driver(goodix_ts_driver);

MODULE_DESCRIPTION("Goodix GT9xx Touch Driver");
MODULE_LICENSE("GPL");
