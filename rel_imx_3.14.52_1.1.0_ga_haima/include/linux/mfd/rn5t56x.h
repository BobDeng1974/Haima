/*
 * include/linux/mfd/rn5t56x.h
 *
 * Core driver interface to access RN5T56x power management chip.
 *
 * Copyright (C) 2016 RICOH ELECTRONIC DEVICES COMPANY,LTD.
 *
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef __LINUX_MFD_RN5T56x_H
#define __LINUX_MFD_RN5T56x_H

#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/i2c.h>

/* Set 1 if IRQ type is Falling Edge */
#define IRQ_TYPE_IS_EDGE		0

/* Maximum number of main interrupts */
#define MAX_INTERRUPT_MASKS	4
#define MAX_MAIN_INTERRUPT	4
#define MAX_GPEDGE_REG		1

/* Power control register */
#define RN5T56x_PWR_WD			0x0B
#define RN5T56x_PWR_WD_COUNT		0x0C
#define RN5T56x_PWR_FUNC		0x0D
#define RN5T56x_PWR_SLP_CNT		0x0E
#define RN5T56x_PWR_REP_CNT		0x0F
#define RN5T56x_PWR_ON_TIMSET		0x10
#define RN5T56x_PWR_NOE_TIMSET		0x11

/* Interrupt enable register */
#define RN5T56x_INT_EN_SYS		0x12
#define RN5T56x_INT_EN_DCDC		0x40
#define RN5T56x_INT_EN_GPIO		0x94
#define RN5T56x_INT_EN_GPIO2		0x94 /* dummy */

/* Interrupt select register */
#define RN5T56x_PWR_IRSEL		0x15

/* interrupt status registers (monitor regs)*/
#define RN5T56x_INTC_INTPOL		0x9C
#define RN5T56x_INTC_INTEN		0x9D
#define RN5T56x_INTC_INTMON		0x9E
#define RN5T56x_INT_MON_SYS		0x14
#define RN5T56x_INT_MON_DCDC		0x42

/* interrupt clearing registers */
#define RN5T56x_INT_IR_SYS		0x13
#define RN5T56x_INT_IR_DCDC		0x41
#define RN5T56x_INT_IR_GPIOR		0x95
#define RN5T56x_INT_IR_GPIOF		0x96

/* GPIO register base address */
#define RN5T56x_GPIO_IOSEL		0x90
#define RN5T56x_GPIO_IOOUT		0x91
#define RN5T56x_GPIO_GPEDGE1		0x92

/***
#define RN5T56x_GPIO_EN_GPIR		0x94
#define RN5T56x_GPIO_IR_GPR		0x95
#define RN5T56x_GPIO_IR_GPF		0x96
***/

#define RN5T56x_GPIO_MON_IOIN		0x97
#define RN5T56x_GPIO_LED_FUNC		0x98

#define RN5T56x_REG_BANKSEL		0xFF

/* RN5T56x IRQ definitions */
enum {
	RN5T56x_IRQ_POWER_ON,
	RN5T56x_IRQ_EXTIN,
	RN5T56x_IRQ_PRE_VINDT,
	RN5T56x_IRQ_PREOT,
	RN5T56x_IRQ_POWER_OFF,
	RN5T56x_IRQ_NOE_OFF,
	RN5T56x_IRQ_WD,
	RN5T56x_IRQ_CLK_STP,

	RN5T56x_IRQ_DC1LIM,
	RN5T56x_IRQ_DC2LIM,
	RN5T56x_IRQ_DC3LIM,
	RN5T56x_IRQ_DC4LIM,

	RN5T56x_IRQ_GPIO0,
	RN5T56x_IRQ_GPIO1,
	RN5T56x_IRQ_GPIO2,
	RN5T56x_IRQ_GPIO3,

	/* Should be last entry */
	RN5T56x_NR_IRQS,
};

/* RN5T56x gpio definitions */
enum {
	RN5T56x_GPIO0,
	RN5T56x_GPIO1,
	RN5T56x_GPIO2,
	RN5T56x_GPIO3,

	RN5T56x_NR_GPIO,
};

enum rn5t56x_sleep_control_id {
	RN5T56x_DS_DC1,
	RN5T56x_DS_DC2,
	RN5T56x_DS_DC3,
	RN5T56x_DS_DC4,
	RN5T56x_DS_LDO1,
	RN5T56x_DS_LDO2,
	RN5T56x_DS_LDO3,
	RN5T56x_DS_LDO4,
	RN5T56x_DS_LDO5,
	RN5T56x_DS_LDORTC1,
	RN5T56x_DS_LDORTC2,
	RN5T56x_DS_PSO0,
	RN5T56x_DS_PSO1,
	RN5T56x_DS_PSO2,
	RN5T56x_DS_PSO3,
};

struct rn5t56x_subdev_info {
	int		id;
	const char	*name;
	void		*platform_data;
};

struct rn5t56x_gpio_init_data {
	unsigned output_mode_en:1;	/* Enable output mode during init */
	unsigned output_val:1;	/* Output value if it is in output mode */
	unsigned init_apply:1;	/* Apply init data on configuring gpios*/
	unsigned led_mode:1;	/* Select LED mode during init */
	unsigned led_func:1;	/* Set LED function if LED mode is 1 */
};

struct rn5t56x {
	struct device		*dev;
	struct i2c_client	*client;
	struct mutex		io_lock;
	int			gpio_base;
	struct gpio_chip	gpio_chip;
	int			irq_base;
/*	struct irq_chip		irq_chip; */
	int			chip_irq;
	struct mutex		irq_lock;
	unsigned long		group_irq_en[MAX_MAIN_INTERRUPT];

	/* For main interrupt bits in INTC */
	u8			intc_inten_cache;
	u8			intc_inten_reg;

	/* For group interrupt bits and address */
	u8			irq_en_cache[MAX_INTERRUPT_MASKS];
	u8			irq_en_reg[MAX_INTERRUPT_MASKS];

	/* For gpio edge */
	u8			gpedge_cache[MAX_GPEDGE_REG];
	u8			gpedge_reg[MAX_GPEDGE_REG];

	int			bank_num;
};

struct rn5t56x_platform_data {
	int		num_subdevs;
	struct	rn5t56x_subdev_info *subdevs;
	int (*init_port)(int irq_num); /* Init GPIO for IRQ pin */
	int		gpio_base;
	int		irq_base;
	struct rn5t56x_gpio_init_data *gpio_init_data;
	int num_gpioinit_data;
	bool enable_shutdown_pin;
};

/* ====================================*/
/* RN5T56x Power_Key device data	*/
/* ====================================*/
struct rn5t56x_pwrkey_platform_data {
	int irq;
	unsigned long delay_ms;
};

/* ====================================*/
/* RN5T56x battery device data		*/
/* ====================================*/
extern int pwrkey_wakeup;

extern int rn5t56x_read(struct device *dev, uint8_t reg, uint8_t *val);
extern int rn5t56x_read_bank1(struct device *dev, uint8_t reg, uint8_t *val);
extern int rn5t56x_bulk_reads(struct device *dev, u8 reg, u8 count,
								uint8_t *val);
extern int rn5t56x_bulk_reads_bank1(struct device *dev, u8 reg, u8 count,
								uint8_t *val);
extern int rn5t56x_write(struct device *dev, u8 reg, uint8_t val);
extern int rn5t56x_write_bank1(struct device *dev, u8 reg, uint8_t val);
extern int rn5t56x_bulk_writes(struct device *dev, u8 reg, u8 count,
								uint8_t *val);
extern int rn5t56x_bulk_writes_bank1(struct device *dev, u8 reg, u8 count,
								uint8_t *val);
extern int rn5t56x_set_bits(struct device *dev, u8 reg, uint8_t bit_mask);
extern int rn5t56x_clr_bits(struct device *dev, u8 reg, uint8_t bit_mask);
extern int rn5t56x_update(struct device *dev, u8 reg, uint8_t val,
								uint8_t mask);
extern int rn5t56x_update_bank1(struct device *dev, u8 reg, uint8_t val,
								uint8_t mask);
extern int rn5t56x_power_off(void);
extern int rn5t56x_irq_init(struct rn5t56x *rn5t56x, int irq, int irq_base);
extern int rn5t56x_irq_exit(struct rn5t56x *rn5t56x);

#endif
