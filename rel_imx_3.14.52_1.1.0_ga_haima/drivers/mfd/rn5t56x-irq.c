/*
 * driver/mfd/rn5t56x-irq.c
 *
 * Interrupt driver for RN5T56x power management chip.
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
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/mfd/rn5t56x.h>


enum int_type {
	SYS_INT  = 0x1,
	DCDC_INT = 0x2,
	GPIO_INT = 0x10,
};

static int gpedge_add[] = {
	RN5T56x_GPIO_GPEDGE1,
};

static int irq_en_add[] = {
	RN5T56x_INT_EN_SYS,
	RN5T56x_INT_EN_DCDC,
	RN5T56x_INT_EN_GPIO,
	RN5T56x_INT_EN_GPIO2,
};

static int irq_mon_add[] = {
	RN5T56x_INT_IR_SYS,		/* RN5T56x_INT_MON_SYS, */
	RN5T56x_INT_IR_DCDC,		/* RN5T56x_INT_MON_DCDC, */
	RN5T56x_INT_IR_GPIOR,
	RN5T56x_INT_IR_GPIOF,
};

static int irq_clr_add[] = {
	RN5T56x_INT_IR_SYS,
	RN5T56x_INT_IR_DCDC,
	RN5T56x_INT_IR_GPIOR,
	RN5T56x_INT_IR_GPIOF,
};

static int main_int_type[] = {
	SYS_INT,
	DCDC_INT,
	GPIO_INT,
	GPIO_INT,
};

struct rn5t56x_irq_data {
	u8	int_type;
	u8	master_bit;
	u8	int_en_bit;
	u8	mask_reg_index;
	int	grp_index;
};

#define RN5T56x_IRQ(_int_type, _master_bit, _grp_index, _int_bit, _mask_ind) \
	{						\
		.int_type	= _int_type,		\
		.master_bit	= _master_bit,		\
		.grp_index	= _grp_index,		\
		.int_en_bit	= _int_bit,		\
		.mask_reg_index	= _mask_ind,		\
	}

static const struct rn5t56x_irq_data rn5t56x_irqs[RN5T56x_NR_IRQS] = {
	[RN5T56x_IRQ_POWER_ON]	= RN5T56x_IRQ(SYS_INT,  0, 0, 0, 0),
	[RN5T56x_IRQ_EXTIN]	= RN5T56x_IRQ(SYS_INT,  0, 1, 1, 0),
	[RN5T56x_IRQ_PRE_VINDT]	= RN5T56x_IRQ(SYS_INT,  0, 2, 2, 0),
	[RN5T56x_IRQ_PREOT]	= RN5T56x_IRQ(SYS_INT,  0, 3, 3, 0),
	[RN5T56x_IRQ_POWER_OFF]	= RN5T56x_IRQ(SYS_INT,  0, 4, 4, 0),
	[RN5T56x_IRQ_NOE_OFF]	= RN5T56x_IRQ(SYS_INT,  0, 5, 5, 0),
	[RN5T56x_IRQ_WD]	= RN5T56x_IRQ(SYS_INT,  0, 6, 6, 0),

	[RN5T56x_IRQ_DC1LIM]	= RN5T56x_IRQ(DCDC_INT, 1, 0, 0, 1),
	[RN5T56x_IRQ_DC2LIM]	= RN5T56x_IRQ(DCDC_INT, 1, 1, 1, 1),
	[RN5T56x_IRQ_DC3LIM]	= RN5T56x_IRQ(DCDC_INT, 1, 2, 2, 1),
	[RN5T56x_IRQ_DC4LIM]	= RN5T56x_IRQ(DCDC_INT, 1, 3, 3, 1),

	[RN5T56x_IRQ_GPIO0]	= RN5T56x_IRQ(GPIO_INT, 4, 0, 0, 2),
	[RN5T56x_IRQ_GPIO1]	= RN5T56x_IRQ(GPIO_INT, 4, 1, 1, 2),
	[RN5T56x_IRQ_GPIO2]	= RN5T56x_IRQ(GPIO_INT, 4, 2, 2, 2),
	[RN5T56x_IRQ_GPIO3]	= RN5T56x_IRQ(GPIO_INT, 4, 3, 3, 2),
};

static void rn5t56x_irq_lock(struct irq_data *irq_data)
{
	struct rn5t56x *rn5t56x = irq_data_get_irq_chip_data(irq_data);

	mutex_lock(&rn5t56x->irq_lock);
}

static void rn5t56x_irq_unmask(struct irq_data *irq_data)
{
	struct rn5t56x *rn5t56x = irq_data_get_irq_chip_data(irq_data);
	unsigned int __irq = irq_data->irq - rn5t56x->irq_base;
	const struct rn5t56x_irq_data *data = &rn5t56x_irqs[__irq];

	rn5t56x->group_irq_en[data->master_bit] |= (1 << data->grp_index);
	if (rn5t56x->group_irq_en[data->master_bit])
		rn5t56x->intc_inten_reg |= 1 << data->master_bit;

	rn5t56x->irq_en_reg[data->mask_reg_index] |= 1 << data->int_en_bit;
}

static void rn5t56x_irq_mask(struct irq_data *irq_data)
{
	struct rn5t56x *rn5t56x = irq_data_get_irq_chip_data(irq_data);
	unsigned int __irq = irq_data->irq - rn5t56x->irq_base;
	const struct rn5t56x_irq_data *data = &rn5t56x_irqs[__irq];

	rn5t56x->group_irq_en[data->master_bit] &= ~(1 << data->grp_index);
	if (!rn5t56x->group_irq_en[data->master_bit])
		rn5t56x->intc_inten_reg &= ~(1 << data->master_bit);

	rn5t56x->irq_en_reg[data->mask_reg_index] &= ~(1 << data->int_en_bit);
}

static void rn5t56x_irq_sync_unlock(struct irq_data *irq_data)
{
	struct rn5t56x *rn5t56x = irq_data_get_irq_chip_data(irq_data);
	int i;

	for (i = 0; i < ARRAY_SIZE(rn5t56x->gpedge_reg); i++) {
		if (rn5t56x->gpedge_reg[i] != rn5t56x->gpedge_cache[i] &&
			!WARN_ON(rn5t56x_write(rn5t56x->dev,
				gpedge_add[i], rn5t56x->gpedge_reg[i])))
			rn5t56x->gpedge_cache[i] = rn5t56x->gpedge_reg[i];
	}

	for (i = 0; i < ARRAY_SIZE(rn5t56x->irq_en_reg); i++) {
		if (rn5t56x->irq_en_reg[i] != rn5t56x->irq_en_cache[i] &&
			!WARN_ON(rn5t56x_write(rn5t56x->dev,
				irq_en_add[i], rn5t56x->irq_en_reg[i])))
			rn5t56x->irq_en_cache[i] = rn5t56x->irq_en_reg[i];
	}

	if (rn5t56x->intc_inten_reg != rn5t56x->intc_inten_cache) {
		if (!WARN_ON(rn5t56x_write(rn5t56x->dev,
				RN5T56x_INTC_INTEN, rn5t56x->intc_inten_reg)))
			rn5t56x->intc_inten_cache = rn5t56x->intc_inten_reg;
	}

	mutex_unlock(&rn5t56x->irq_lock);
}

static int rn5t56x_irq_set_type(struct irq_data *irq_data, unsigned int type)
{
	struct rn5t56x *rn5t56x = irq_data_get_irq_chip_data(irq_data);
	unsigned int __irq = irq_data->irq - rn5t56x->irq_base;
	const struct rn5t56x_irq_data *data = &rn5t56x_irqs[__irq];
	int val = 0;
	int gpedge_index;
	int gpedge_bit_pos;

	if (data->int_type & GPIO_INT) {
		gpedge_index = data->int_en_bit / 4;
		gpedge_bit_pos = data->int_en_bit % 4;

		if (type & IRQ_TYPE_EDGE_FALLING)
			val |= 0x2;

		if (type & IRQ_TYPE_EDGE_RISING)
			val |= 0x1;

		rn5t56x->gpedge_reg[gpedge_index] &= ~(3 << gpedge_bit_pos);
		rn5t56x->gpedge_reg[gpedge_index] |= (val << gpedge_bit_pos);
		rn5t56x_irq_unmask(irq_data);
	}
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int rn5t56x_irq_set_wake(struct irq_data *irq_data, unsigned int on)
{
	struct rn5t56x *rn5t56x = irq_data_get_irq_chip_data(irq_data);
	return irq_set_irq_wake(rn5t56x->chip_irq, on);	/* i2c->irq */
}
#else
#define rn5t56x_irq_set_wake NULL
#endif

static irqreturn_t rn5t56x_irq(int irq, void *data)
{
	struct rn5t56x *rn5t56x = data;
	u8 int_sts[MAX_INTERRUPT_MASKS];
	u8 master_int;
	int i;
	int ret;

	/* printk(KERN_DEBUG "PMU: %s: irq=%d\n", __func__, irq); */
	/* disable_irq_nosync(irq); */
	/* Clear the status */
	for (i = 0; i < MAX_INTERRUPT_MASKS; i++)
		int_sts[i] = 0;

	ret = rn5t56x_read(rn5t56x->dev, RN5T56x_INTC_INTMON,
						&master_int);
	printk(KERN_DEBUG "PMU1: %s: master_int=0x%x\n", __func__, master_int);
	if (ret < 0) {
		dev_err(rn5t56x->dev, "Error in reading reg 0x%02x "
			"error: %d\n", RN5T56x_INTC_INTMON, ret);
		return IRQ_HANDLED;
	}

	for (i = 0; i < MAX_INTERRUPT_MASKS; ++i) {

		if (!(master_int & main_int_type[i]))
			continue;

		ret = rn5t56x_read(rn5t56x->dev,
				irq_mon_add[i], &int_sts[i]);
		printk(KERN_DEBUG "PMU2: %s: int_sts[%d]=0x%x\n",
						 __func__, i, int_sts[i]);
		if (ret < 0) {
			dev_err(rn5t56x->dev, "Error in reading reg 0x%02x "
				"error: %d\n", irq_mon_add[i], ret);
			int_sts[i] = 0;
			continue;
		}
		if (!int_sts[i])
			continue;

		ret = rn5t56x_write(rn5t56x->dev, irq_clr_add[i], ~int_sts[i]);
		if (ret < 0) {
			dev_err(rn5t56x->dev, "Error in writing reg 0x%02x "
			"error: %d\n", irq_clr_add[i], ret);
		}

	}

	/* Merge gpio interrupts  for rising and falling case*/
	int_sts[2] |= int_sts[3];

	/* Call interrupt handler if enabled */
	for (i = 0; i < RN5T56x_NR_IRQS; ++i) {
		const struct rn5t56x_irq_data *data = &rn5t56x_irqs[i];
		if ((int_sts[data->mask_reg_index] & (1 << data->int_en_bit))
			&& (rn5t56x->group_irq_en[data->master_bit]
			& (1 << data->grp_index)))
				handle_nested_irq(rn5t56x->irq_base + i);
	}

	printk(KERN_DEBUG "PMU: %s: out\n", __func__);
	return IRQ_HANDLED;
}

static struct irq_chip rn5t56x_irq_chip = {
	.name			= "rn5t56x",
	.irq_mask		= rn5t56x_irq_mask,
	.irq_unmask		= rn5t56x_irq_unmask,
	.irq_bus_lock		= rn5t56x_irq_lock,
	.irq_bus_sync_unlock	= rn5t56x_irq_sync_unlock,
	.irq_set_type		= rn5t56x_irq_set_type,
	.irq_set_wake		= rn5t56x_irq_set_wake,
};

int rn5t56x_irq_init(struct rn5t56x *rn5t56x, int irq,
				int irq_base)
{
	int i, ret;

	if (!irq_base) {
		dev_warn(rn5t56x->dev, "No interrupt support on IRQ base\n");
		return -EINVAL;
	}

	mutex_init(&rn5t56x->irq_lock);

	/* Initialize all locals to 0 */
	for (i = 0; i < 4; i++) {
		rn5t56x->irq_en_cache[i] = 0;
		rn5t56x->irq_en_reg[i] = 0;
	}

	rn5t56x->intc_inten_cache = 0;
	rn5t56x->intc_inten_reg = 0;
	for (i = 0; i < MAX_GPEDGE_REG; i++) {
		rn5t56x->gpedge_cache[i] = 0;
		rn5t56x->gpedge_reg[i] = 0;
	}

	/* Initailize all int register to 0 */
	for (i = 0; i < MAX_INTERRUPT_MASKS; i++) {
		ret = rn5t56x_write(rn5t56x->dev, irq_en_add[i],
				rn5t56x->irq_en_reg[i]);
		if (ret < 0)
			dev_err(rn5t56x->dev, "Error in writing reg 0x%02x "
				"error: %d\n", irq_en_add[i], ret);
	}

	for (i = 0; i < MAX_GPEDGE_REG; i++) {
		ret = rn5t56x_write(rn5t56x->dev, gpedge_add[i],
						rn5t56x->gpedge_reg[i]);
		if (ret < 0)
			dev_err(rn5t56x->dev, "Error in writing reg 0x%02x "
				"error: %d\n", gpedge_add[i], ret);
	}

	ret = rn5t56x_write(rn5t56x->dev, RN5T56x_INTC_INTEN, 0x0);
	if (ret < 0)
		dev_err(rn5t56x->dev, "Error in writing reg 0x%02x "
				"error: %d\n", RN5T56x_INTC_INTEN, ret);

	/* Clear all interrupts in case they woke up active. */
	for (i = 0; i < MAX_INTERRUPT_MASKS; i++) {
		ret = rn5t56x_write(rn5t56x->dev, irq_clr_add[i], 0);
		if (ret < 0)
			dev_err(rn5t56x->dev, "Error in writing reg 0x%02x "
				"error: %d\n", irq_clr_add[i], ret);
	}

	rn5t56x->irq_base = irq_base;
	rn5t56x->chip_irq = irq;

	for (i = 0; i < RN5T56x_NR_IRQS; i++) {
		int __irq = i + rn5t56x->irq_base;
		irq_set_chip_data(__irq, rn5t56x);
		irq_set_chip_and_handler(__irq, &rn5t56x_irq_chip,
						handle_simple_irq);
		irq_set_nested_thread(__irq, 1);
#ifdef CONFIG_ARM
		set_irq_flags(__irq, IRQF_VALID);
#endif
	}

	ret = request_threaded_irq(irq, NULL, rn5t56x_irq,
#if IRQ_TYPE_IS_EDGE
			IRQ_TYPE_EDGE_FALLING | IRQF_DISABLED | IRQF_ONESHOT,
#else
			IRQF_TRIGGER_LOW | IRQF_ONESHOT,
#endif
						"rn5t56x", rn5t56x);
	if (ret < 0)
		dev_err(rn5t56x->dev, "Error in registering interrupt "
				"error: %d\n", ret);
	if (!ret) {
		device_init_wakeup(rn5t56x->dev, 1);
		enable_irq_wake(irq);
	}

	return ret;
}

int rn5t56x_irq_exit(struct rn5t56x *rn5t56x)
{
	if (rn5t56x->chip_irq)
		free_irq(rn5t56x->chip_irq, rn5t56x);
	return 0;
}

