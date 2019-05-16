/*
 * driver/mfd/rn5t56x.c
 *
 * Core driver implementation to access RN5T56x power management chip.
 *
 * Copyright (C) 2016 RICOH ELECTRONIC DEVICES COMPANY,LTD.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
/*#define DEBUG			1 */
/*#define VERBOSE_DEBUG		1 */
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/mfd/core.h>
#include <linux/mfd/rn5t56x.h>


struct sleep_control_data {
	u8 reg_add;
};

#define SLEEP_INIT(_id, _reg)		\
	[RN5T56x_DS_##_id] = {.reg_add = _reg}


static inline int __rn5t56x_read(struct i2c_client *client,
				  u8 reg, uint8_t *val)
{
	int ret;

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		dev_err(&client->dev, "failed reading at 0x%02x\n", reg);
		return ret;
	}

	*val = (uint8_t)ret;
	dev_dbg(&client->dev, "rn5t56x: reg read  reg=%x, val=%x\n",
				reg, *val);
	return 0;
}

static inline int __rn5t56x_bulk_reads(struct i2c_client *client, u8 reg,
				int len, uint8_t *val)
{
	int ret;
	int i;

	ret = i2c_smbus_read_i2c_block_data(client, reg, len, val);
	if (ret < 0) {
		dev_err(&client->dev, "failed reading from 0x%02x\n", reg);
		return ret;
	}
	for (i = 0; i < len; ++i) {
		dev_dbg(&client->dev, "rn5t56x: reg read  reg=%x, val=%x\n",
				reg + i, *(val + i));
	}
	return 0;
}

static inline int __rn5t56x_write(struct i2c_client *client,
				 u8 reg, uint8_t val)
{
	int ret;

	dev_dbg(&client->dev, "rn5t56x: reg write  reg=%x, val=%x\n",
				reg, val);
	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret < 0) {
		dev_err(&client->dev, "failed writing 0x%02x to 0x%02x\n",
				val, reg);
		return ret;
	}

	return 0;
}

static inline int __rn5t56x_bulk_writes(struct i2c_client *client, u8 reg,
				  int len, uint8_t *val)
{
	int ret;
	int i;

	for (i = 0; i < len; ++i) {
		dev_dbg(&client->dev, "rn5t56x: reg write  reg=%x, val=%x\n",
				reg + i, *(val + i));
	}

	ret = i2c_smbus_write_i2c_block_data(client, reg, len, val);
	if (ret < 0) {
		dev_err(&client->dev, "failed writings to 0x%02x\n", reg);
		return ret;
	}

	return 0;
}

static inline int set_bank_rn5t56x(struct device *dev, int bank)
{
	struct rn5t56x *rn5t56x = dev_get_drvdata(dev);
	int ret;

	if (bank != (bank & 1))
		return -EINVAL;
	if (bank == rn5t56x->bank_num)
		return 0;
	ret = __rn5t56x_write(to_i2c_client(dev), RN5T56x_REG_BANKSEL, bank);
	if (!ret)
		rn5t56x->bank_num = bank;

	return ret;
}

int rn5t56x_write(struct device *dev, u8 reg, uint8_t val)
{
	struct rn5t56x *rn5t56x = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&rn5t56x->io_lock);
	ret = set_bank_rn5t56x(dev, 0);
	if (!ret)
		ret = __rn5t56x_write(to_i2c_client(dev), reg, val);
	mutex_unlock(&rn5t56x->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(rn5t56x_write);

int rn5t56x_write_bank1(struct device *dev, u8 reg, uint8_t val)
{
	struct rn5t56x *rn5t56x = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&rn5t56x->io_lock);
	ret = set_bank_rn5t56x(dev, 1);
	if (!ret)
		ret = __rn5t56x_write(to_i2c_client(dev), reg, val);
	mutex_unlock(&rn5t56x->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(rn5t56x_write_bank1);

int rn5t56x_bulk_writes(struct device *dev, u8 reg, u8 len, uint8_t *val)
{
	struct rn5t56x *rn5t56x = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&rn5t56x->io_lock);
	ret = set_bank_rn5t56x(dev, 0);
	if (!ret)
		ret = __rn5t56x_bulk_writes(to_i2c_client(dev), reg, len, val);
	mutex_unlock(&rn5t56x->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(rn5t56x_bulk_writes);

int rn5t56x_bulk_writes_bank1(struct device *dev, u8 reg, u8 len, uint8_t *val)
{
	struct rn5t56x *rn5t56x = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&rn5t56x->io_lock);
	ret = set_bank_rn5t56x(dev, 1);
	if (!ret)
		ret = __rn5t56x_bulk_writes(to_i2c_client(dev), reg, len, val);
	mutex_unlock(&rn5t56x->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(rn5t56x_bulk_writes_bank1);

int rn5t56x_read(struct device *dev, u8 reg, uint8_t *val)
{
	struct rn5t56x *rn5t56x = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&rn5t56x->io_lock);
	ret = set_bank_rn5t56x(dev, 0);
	if (!ret)
		ret = __rn5t56x_read(to_i2c_client(dev), reg, val);
	mutex_unlock(&rn5t56x->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(rn5t56x_read);

int rn5t56x_read_bank1(struct device *dev, u8 reg, uint8_t *val)
{
	struct rn5t56x *rn5t56x = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&rn5t56x->io_lock);
	ret = set_bank_rn5t56x(dev, 1);
	if (!ret)
		ret =  __rn5t56x_read(to_i2c_client(dev), reg, val);
	mutex_unlock(&rn5t56x->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(rn5t56x_read_bank1);

int rn5t56x_bulk_reads(struct device *dev, u8 reg, u8 len, uint8_t *val)
{
	struct rn5t56x *rn5t56x = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&rn5t56x->io_lock);
	ret = set_bank_rn5t56x(dev, 0);
	if (!ret)
		ret = __rn5t56x_bulk_reads(to_i2c_client(dev), reg, len, val);
	mutex_unlock(&rn5t56x->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(rn5t56x_bulk_reads);

int rn5t56x_bulk_reads_bank1(struct device *dev, u8 reg, u8 len, uint8_t *val)
{
	struct rn5t56x *rn5t56x = dev_get_drvdata(dev);
	int ret = 0;

	mutex_lock(&rn5t56x->io_lock);
	ret = set_bank_rn5t56x(dev, 1);
	if (!ret)
		ret = __rn5t56x_bulk_reads(to_i2c_client(dev), reg, len, val);
	mutex_unlock(&rn5t56x->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(rn5t56x_bulk_reads_bank1);

int rn5t56x_set_bits(struct device *dev, u8 reg, uint8_t bit_mask)
{
	struct rn5t56x *rn5t56x = dev_get_drvdata(dev);
	uint8_t reg_val;
	int ret = 0;

	mutex_lock(&rn5t56x->io_lock);
	ret = set_bank_rn5t56x(dev, 0);
	if (!ret) {
		ret = __rn5t56x_read(to_i2c_client(dev), reg, &reg_val);
		if (ret)
			goto out;

		if ((reg_val & bit_mask) != bit_mask) {
			reg_val |= bit_mask;
			ret = __rn5t56x_write(to_i2c_client(dev), reg,
								 reg_val);
		}
	}
out:
	mutex_unlock(&rn5t56x->io_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(rn5t56x_set_bits);

int rn5t56x_clr_bits(struct device *dev, u8 reg, uint8_t bit_mask)
{
	struct rn5t56x *rn5t56x = dev_get_drvdata(dev);
	uint8_t reg_val;
	int ret = 0;

	mutex_lock(&rn5t56x->io_lock);
	ret = set_bank_rn5t56x(dev, 0);
	if (!ret) {
		ret = __rn5t56x_read(to_i2c_client(dev), reg, &reg_val);
		if (ret)
			goto out;

		if (reg_val & bit_mask) {
			reg_val &= ~bit_mask;
			ret = __rn5t56x_write(to_i2c_client(dev), reg,
								 reg_val);
		}
	}
out:
	mutex_unlock(&rn5t56x->io_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(rn5t56x_clr_bits);

int rn5t56x_update(struct device *dev, u8 reg, uint8_t val, uint8_t mask)
{
	struct rn5t56x *rn5t56x = dev_get_drvdata(dev);
	uint8_t reg_val;
	int ret = 0;

	mutex_lock(&rn5t56x->io_lock);
	ret = set_bank_rn5t56x(dev, 0);
	if (!ret) {
		ret = __rn5t56x_read(rn5t56x->client, reg, &reg_val);
		if (ret)
			goto out;

		if ((reg_val & mask) != val) {
			reg_val = (reg_val & ~mask) | (val & mask);
			ret = __rn5t56x_write(rn5t56x->client, reg, reg_val);
		}
	}
out:
	mutex_unlock(&rn5t56x->io_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(rn5t56x_update);

int rn5t56x_update_bank1(struct device *dev, u8 reg, uint8_t val, uint8_t mask)
{
	struct rn5t56x *rn5t56x = dev_get_drvdata(dev);
	uint8_t reg_val;
	int ret = 0;

	mutex_lock(&rn5t56x->io_lock);
	ret = set_bank_rn5t56x(dev, 1);
	if (!ret) {
		ret = __rn5t56x_read(rn5t56x->client, reg, &reg_val);
		if (ret)
			goto out;

		if ((reg_val & mask) != val) {
			reg_val = (reg_val & ~mask) | (val & mask);
			ret = __rn5t56x_write(rn5t56x->client, reg, reg_val);
		}
	}
out:
	mutex_unlock(&rn5t56x->io_lock);
	return ret;
}

static struct i2c_client *rn5t56x_i2c_client;
int rn5t56x_power_off(void)
{
	int ret;

	if (!rn5t56x_i2c_client)
		return -EINVAL;

	/* Disable all Interrupt */
	ret = __rn5t56x_write(rn5t56x_i2c_client, RN5T56x_INTC_INTEN, 0);
	if (ret)
		goto out;

	/* Not repeat power ON after power off(Power Off/N_OE) */
	__rn5t56x_write(rn5t56x_i2c_client, RN5T56x_PWR_REP_CNT, 0x0);
	if (ret)
		goto out;

	/* Power OFF */
	__rn5t56x_write(rn5t56x_i2c_client, RN5T56x_PWR_SLP_CNT, 0x1);
	if (ret)
		goto out;

	return 0;
out:
	return ret;
}

static int rn5t56x_gpio_get(struct gpio_chip *gc, unsigned offset)
{
	struct rn5t56x *rn5t56x = container_of(gc, struct rn5t56x, gpio_chip);
	uint8_t val;
	int ret;

	ret = rn5t56x_read(rn5t56x->dev, RN5T56x_GPIO_MON_IOIN, &val);
	if (ret < 0)
		return ret;

	return ((val & (0x1 << offset)) != 0);
}

static void rn5t56x_gpio_set(struct gpio_chip *gc, unsigned offset,
			int value)
{
	struct rn5t56x *rn5t56x = container_of(gc, struct rn5t56x, gpio_chip);
	if (value)
		rn5t56x_set_bits(rn5t56x->dev, RN5T56x_GPIO_IOOUT,
						1 << offset);
	else
		rn5t56x_clr_bits(rn5t56x->dev, RN5T56x_GPIO_IOOUT,
						1 << offset);
}

static int rn5t56x_gpio_input(struct gpio_chip *gc, unsigned offset)
{
	struct rn5t56x *rn5t56x = container_of(gc, struct rn5t56x, gpio_chip);

	return rn5t56x_clr_bits(rn5t56x->dev, RN5T56x_GPIO_IOSEL,
						1 << offset);
}

static int rn5t56x_gpio_output(struct gpio_chip *gc, unsigned offset,
				int value)
{
	struct rn5t56x *rn5t56x = container_of(gc, struct rn5t56x, gpio_chip);

	rn5t56x_gpio_set(gc, offset, value);
	return rn5t56x_set_bits(rn5t56x->dev, RN5T56x_GPIO_IOSEL,
						1 << offset);
}

static int rn5t56x_gpio_to_irq(struct gpio_chip *gc, unsigned off)
{
	struct rn5t56x *rn5t56x = container_of(gc, struct rn5t56x, gpio_chip);

	if ((off >= 0) && (off < 8))
		return rn5t56x->irq_base + RN5T56x_IRQ_GPIO0 + off;

	return -EIO;
}


static void rn5t56x_gpio_init(struct rn5t56x *rn5t56x,
	struct rn5t56x_platform_data *pdata)
{
	int ret;
	int i;
	struct rn5t56x_gpio_init_data *ginit;

	if (pdata->gpio_base  <= 0)
		return;

	for (i = 0; i < pdata->num_gpioinit_data; ++i) {
		ginit = &pdata->gpio_init_data[i];

		if (!ginit->init_apply)
			continue;

		if (ginit->output_mode_en) {
			/* GPIO output mode */
			if (ginit->output_val)
				/* output H */
				ret = rn5t56x_set_bits(rn5t56x->dev,
					RN5T56x_GPIO_IOOUT, 1 << i);
			else
				/* output L */
				ret = rn5t56x_clr_bits(rn5t56x->dev,
					RN5T56x_GPIO_IOOUT, 1 << i);
			if (!ret)
				ret = rn5t56x_set_bits(rn5t56x->dev,
					RN5T56x_GPIO_IOSEL, 1 << i);
		} else
			/* GPIO input mode */
			ret = rn5t56x_clr_bits(rn5t56x->dev,
					RN5T56x_GPIO_IOSEL, 1 << i);

		/* if LED function enabled in OTP */
		if (ginit->led_mode) {
			/* LED Mode 1 */
			if (i == 0)	/* GP0 */
				ret = rn5t56x_set_bits(rn5t56x->dev,
					 RN5T56x_GPIO_LED_FUNC,
					 0x04 | (ginit->led_func & 0x03));
			if (i == 1)	/* GP1 */
				ret = rn5t56x_set_bits(rn5t56x->dev,
					 RN5T56x_GPIO_LED_FUNC,
					 0x40 | (ginit->led_func & 0x03) << 4);

		}


		if (ret < 0)
			dev_err(rn5t56x->dev, "Gpio %d init "
				"dir configuration failed: %d\n", i, ret);

	}

	rn5t56x->gpio_chip.owner		= THIS_MODULE;
	rn5t56x->gpio_chip.label		= rn5t56x->client->name;
	rn5t56x->gpio_chip.dev			= rn5t56x->dev;
	rn5t56x->gpio_chip.base			= pdata->gpio_base;
	rn5t56x->gpio_chip.ngpio		= RN5T56x_NR_GPIO;
	rn5t56x->gpio_chip.can_sleep		= 1;

	rn5t56x->gpio_chip.direction_input	= rn5t56x_gpio_input;
	rn5t56x->gpio_chip.direction_output	= rn5t56x_gpio_output;
	rn5t56x->gpio_chip.set			= rn5t56x_gpio_set;
	rn5t56x->gpio_chip.get			= rn5t56x_gpio_get;
	rn5t56x->gpio_chip.to_irq		= rn5t56x_gpio_to_irq;

	ret = gpiochip_add(&rn5t56x->gpio_chip);
	if (ret)
		dev_warn(rn5t56x->dev, "GPIO registration failed: %d\n", ret);
}

static int rn5t56x_remove_subdev(struct device *dev, void *unused)
{
	platform_device_unregister(to_platform_device(dev));
	return 0;
}

static int rn5t56x_remove_subdevs(struct rn5t56x *rn5t56x)
{
	return device_for_each_child(rn5t56x->dev, NULL,
				     rn5t56x_remove_subdev);
}

static int rn5t56x_add_subdevs(struct rn5t56x *rn5t56x,
				struct rn5t56x_platform_data *pdata)
{
	struct rn5t56x_subdev_info *subdev;
	struct platform_device *pdev;
	int i, ret = 0;

	for (i = 0; i < pdata->num_subdevs; i++) {
		subdev = &pdata->subdevs[i];

		pdev = platform_device_alloc(subdev->name, subdev->id);

		pdev->dev.parent = rn5t56x->dev;
		pdev->dev.platform_data = subdev->platform_data;

		ret = platform_device_add(pdev);
		if (ret)
			goto failed;
	}
	return 0;

failed:
	rn5t56x_remove_subdevs(rn5t56x);
	return ret;
}

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
static void print_regs(const char *header, struct seq_file *s,
		struct i2c_client *client, int start_offset,
		int end_offset)
{
	uint8_t reg_val;
	int i;
	int ret;

	seq_printf(s, "%s\n", header);
	for (i = start_offset; i <= end_offset; ++i) {
		ret = __rn5t56x_read(client, i, &reg_val);
		if (ret >= 0)
			seq_printf(s, "Reg 0x%02x Value 0x%02x\n", i, reg_val);
	}
	seq_printf(s, "------------------\n");
}

static int dbg_ricoh_show(struct seq_file *s, void *unused)
{
	struct rn5t56x *ricoh = s->private;
	struct i2c_client *client = ricoh->client;

	seq_printf(s, "rn5t56x Registers\n");
	seq_printf(s, "------------------\n");

	print_regs("System Regs",		s, client, 0x0, 0x05);
	print_regs("Power Control Regs",	s, client, 0x07, 0x2B);
	print_regs("DCDC  Regs",		s, client, 0x2C, 0x43);
	print_regs("LDO   Regs",		s, client, 0x44, 0x61);
	print_regs("GPIO  Regs",		s, client, 0x90, 0x98);
	print_regs("INTC  Regs",		s, client, 0x9C, 0x9E);
	return 0;
}

static int dbg_ricoh_open(struct inode *inode, struct file *file)
{
	return single_open(file, dbg_ricoh_show, inode->i_private);
}

static const struct file_operations debug_fops = {
	.open		= dbg_ricoh_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
static void __init rn5t56x_debuginit(struct rn5t56x *ricoh)
{
	(void)debugfs_create_file("rn5t56x", S_IRUGO, NULL,
			ricoh, &debug_fops);
}
#else
static void print_regs(const char *header, struct i2c_client *client,
		int start_offset, int end_offset)
{
	uint8_t reg_val;
	int i;
	int ret;

	printk(KERN_DEBUG "%s\n", header);
	for (i = start_offset; i <= end_offset; ++i) {
		ret = __rn5t56x_read(client, i, &reg_val);
		if (ret >= 0)
			printk(KERN_DEBUG "Reg 0x%02x Value 0x%02x\n",
							 i, reg_val);
	}
	printk(KERN_DEBUG "------------------\n");
}
static void __init rn5t56x_debuginit(struct rn5t56x *ricoh)
{
	struct i2c_client *client = ricoh->client;

	printk(KERN_DEBUG "rn5t56x Registers\n");
	printk(KERN_DEBUG "------------------\n");

	print_regs("System Regs",		client, 0x0, 0x05);
	print_regs("Power Control Regs",	client, 0x07, 0x2B);
	print_regs("DCDC  Regs",		client, 0x2C, 0x43);
	print_regs("LDO   Regs",		client, 0x44, 0x5C);
	print_regs("GPIO  Regs",		client, 0x90, 0x9B);
	print_regs("INTC  Regs",		client, 0x9C, 0x9E);

	return 0;
}
#endif

static void rn5t56x_noe_init(struct rn5t56x *ricoh)
{
	struct i2c_client *client = ricoh->client;

	/* N_OE timer setting to 128mS */
	__rn5t56x_write(client, RN5T56x_PWR_NOE_TIMSET, 0x0);
	/* Repeat power ON after reset (Power Off/N_OE) */
	__rn5t56x_write(client, RN5T56x_PWR_REP_CNT, 0x1);
}

static int rn5t56x_i2c_probe(struct i2c_client *client,
			      const struct i2c_device_id *id)
{
	struct rn5t56x *rn5t56x;
	struct rn5t56x_platform_data *pdata = client->dev.platform_data;
	int ret;
	printk(KERN_INFO "PMU: %s:\n", __func__);

	rn5t56x = kzalloc(sizeof(struct rn5t56x), GFP_KERNEL);
	if (rn5t56x == NULL)
		return -ENOMEM;

	rn5t56x->client = client;
	rn5t56x->dev = &client->dev;
	i2c_set_clientdata(client, rn5t56x);
	mutex_init(&rn5t56x->io_lock);

	rn5t56x->bank_num = 0;

	/* For init PMIC_IRQ port */
	ret = pdata->init_port(client->irq);

	if (client->irq) {
		ret = rn5t56x_irq_init(rn5t56x, client->irq, pdata->irq_base);
		if (ret) {
			dev_err(&client->dev, "IRQ init failed: %d\n", ret);
			goto err_irq_init;
		}
	}

	ret = rn5t56x_add_subdevs(rn5t56x, pdata);
	if (ret) {
		dev_err(&client->dev, "add devices failed: %d\n", ret);
		goto err_add_devs;
	}

	rn5t56x_noe_init(rn5t56x);

	rn5t56x_gpio_init(rn5t56x, pdata);

	rn5t56x_debuginit(rn5t56x);

	rn5t56x_i2c_client = client;
	return 0;

err_add_devs:
	if (client->irq)
		rn5t56x_irq_exit(rn5t56x);
err_irq_init:
	kfree(rn5t56x);
	return ret;
}

static int rn5t56x_i2c_remove(struct i2c_client *client)
{
	struct rn5t56x *rn5t56x = i2c_get_clientdata(client);

	if (client->irq)
		rn5t56x_irq_exit(rn5t56x);

	rn5t56x_remove_subdevs(rn5t56x);
	kfree(rn5t56x);
	return 0;
}

#ifdef CONFIG_PM
static int rn5t56x_i2c_suspend(struct i2c_client *client, pm_message_t state)
{
	printk(KERN_INFO "PMU: %s:\n", __func__);

	return 0;
}

int pwrkey_wakeup;
#if IRQ_TYPE_IS_EDGE
static void resume_irq_edge_cont(struct i2c_client *client)
{
	uint8_t reg_val = 0;
	uint8_t reg_inten = 0x13;
	int ret;

	/* Save master Interrupt */
	ret = __rn5t56x_read(client, RN5T56x_INTC_INTEN, &reg_inten);
	if (ret < 0) {
		dev_err(&client->dev, "Err in reading INTC_INTEN %d\n", ret);
		reg_inten = 0x13;
		goto out;
	}
	/* Disable master Interrupt */
	ret = __rn5t56x_write(client, RN5T56x_INTC_INTEN, 0x0);
	if (ret < 0) {
		dev_err(&client->dev, "Err in writing INTC_INTEN %d\n", ret);
		goto out;
	}
	/* Check PWR KEY wakeup ? */
	ret = __rn5t56x_read(client, RN5T56x_INT_IR_SYS, &reg_val);
	if (ret < 0) {
		dev_err(&client->dev, "Err in reading INT_IR_SYS %d\n", ret);
		goto out;
	} else if (reg_val & 0x01) { /* If PWR_KEY wakeup */
		dev_dbg(&client->dev, "--- PWR_KEY Wakeup\n");
		pwrkey_wakeup = 1;
	}
	/* Restore master Interrupt */
	ret = __rn5t56x_write(client, RN5T56x_INTC_INTEN, reg_inten);
	if (ret < 0) {
		dev_err(&client->dev, "Err in writing INTC_INTEN %d\n", ret);
		goto out;
	}
out:
	return;
}
#endif

static int rn5t56x_i2c_resume(struct i2c_client *client)
{
	printk(KERN_INFO "PMU: %s:\n", __func__);
#if IRQ_TYPE_IS_EDGE
	resume_irq_edge_cont(client);
#endif
	return 0;
}

#endif

static const struct i2c_device_id rn5t56x_i2c_id[] = {
	{"rn5t56x", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, rn5t56x_i2c_id);

static struct i2c_driver rn5t56x_i2c_driver = {
	.driver = {
		   .name = "rn5t56x",
		   .owner = THIS_MODULE,
		   },
	.probe = rn5t56x_i2c_probe,
	.remove = rn5t56x_i2c_remove,
#ifdef CONFIG_PM
	.suspend = rn5t56x_i2c_suspend,
	.resume = rn5t56x_i2c_resume,
#endif
	.id_table = rn5t56x_i2c_id,
};


static int __init rn5t56x_i2c_init(void)
{
	int ret = -ENODEV;

	printk(KERN_INFO "PMU: %s:\n", __func__);

	ret = i2c_add_driver(&rn5t56x_i2c_driver);
	if (ret != 0)
		pr_err("Failed to register I2C driver: %d\n", ret);

	return ret;
}

subsys_initcall(rn5t56x_i2c_init);

static void __exit rn5t56x_i2c_exit(void)
{
	i2c_del_driver(&rn5t56x_i2c_driver);
}

module_exit(rn5t56x_i2c_exit);

MODULE_DESCRIPTION("RICOH RN5T56x PMU multi-function core driver");
MODULE_LICENSE("GPL");
