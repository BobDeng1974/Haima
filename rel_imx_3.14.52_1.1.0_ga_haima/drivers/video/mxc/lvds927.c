#include <linux/module.h>
#include <linux/init.h>
#include <linux/ctype.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/of_device.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/regulator/consumer.h>
#include <linux/fsl_devices.h>
struct lvds927_data {
	int gpio_pdb;
	int gpio_intb;
	int irq;
	int use_irq;
	struct i2c_client *client;
};
static inline int lvds927_read_reg(struct lvds927_data *ts, u8 reg)
{
	int val;
	int retry, timeout = 3;

	//ts->client->addr = ;
	for (retry = 0; retry < timeout; retry ++) {
		val = i2c_smbus_read_byte_data(ts->client, reg);
		if (val < 0)
			msleep(5);
		else
			break;
	}

	if (retry >= timeout) {
		dev_info(&ts->client->dev,
			"%s:read reg error: reg=%2x\n", __func__, reg);
		return -1;
	}

	return val;
}

static int lvds927_write_reg(struct lvds927_data *ts, u8 reg, u8 val)
{
	s32 ret;
	int retry, timeout = 3;

	for (retry = 0; retry < timeout; retry ++) {
		ret = i2c_smbus_write_byte_data(ts->client, reg, val);
		if (ret < 0)
			msleep(5);
		else
			break;
	}

	if (retry >= timeout) {
		dev_info(&ts->client->dev,
			"%s:write reg error:reg=%2x,val=%2x\n", __func__,
			reg, val);
		return -1;
	}

	return 0;
}

static int lvds927_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    s32 ret = -1;
	struct lvds927_data *ts;
	struct device_node *np = client->dev.of_node;
    printk("LVDS927 I2C Address:0x%02x\n", client->addr);
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        printk("I2C check functionality failed.");
        return -ENODEV;
    }
	
    ts = devm_kzalloc(&client->dev, sizeof(*ts), GFP_KERNEL);
    if (!ts) {
        printk("Alloc 927_KERNEL memory failed.");
        return -ENOMEM;
    }
   
    memset(ts, 0, sizeof(*ts));
    ts->client = client;
    i2c_set_clientdata(client, ts);
	printk("LVDS927 read Address:0x%x\n",lvds927_read_reg(ts,0x00));
	lvds927_write_reg(ts, 0x13, 0x70); //·´×ªÏßÐò
	ts->gpio_pdb = of_get_named_gpio(np, "gpio_pdb", 0);
	if (!gpio_is_valid(ts->gpio_pdb))
		return -ENODEV;
	
	ts->gpio_intb = of_get_named_gpio(np, "gpio_intb", 0);
	if (!gpio_is_valid(ts->gpio_intb))
		return -ENODEV;

	if(gpio_request(ts->gpio_pdb, "pdb")< 0 ){
		printk("Failed to request GPIO:%d", (s32)ts->gpio_pdb);
        ret = -ENODEV;
	}
	if(gpio_request(ts->gpio_intb, "intb")< 0 ){
		printk("Failed to request GPIO:%d", (s32)ts->gpio_intb);
		gpio_free(ts->gpio_pdb);
        ret = -ENODEV;
	}
	gpio_direction_input(ts->gpio_intb);
	gpio_direction_output(ts->gpio_pdb, 1);
	return 0;
}



static int lvds927_remove(struct i2c_client *client)
{
    struct lvds927_data *ts = i2c_get_clientdata(client);

    if (ts){

           gpio_free(ts->gpio_intb);
		   gpio_free(ts->gpio_pdb);
    }
    i2c_set_clientdata(client, NULL);
    return 0;
}

static const struct i2c_device_id lvds927_id[] = {
	{"lvds_927", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, lvds927_id);
static struct of_device_id lvds927_dt_ids[] = {
	{ .compatible = "Texas,lvds927" },
	{ /* sentinel */ }
};
static struct i2c_driver lvds927_i2c_driver = {
	.driver = {
		  .owner = THIS_MODULE,
		  .name  = "lvds_927",
		  .of_match_table	= lvds927_dt_ids,
		  },
	.probe  = lvds927_probe,
	.remove = lvds927_remove,
	.id_table = lvds927_id,
};


static __init int lvds927_init(void)
{
	u8 err;

	err = i2c_add_driver(&lvds927_i2c_driver);
	if (err != 0)
		pr_err("%s:driver registration failed, error=%d\n",
			__func__, err);

	return err;
}

static void __exit lvds927_clean(void)
{
	i2c_del_driver(&lvds927_i2c_driver);
}

module_init(lvds927_init);
module_exit(lvds927_clean);

MODULE_AUTHOR("Freescale Semiconductor, Inc.");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");