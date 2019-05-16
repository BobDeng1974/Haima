/*
 * board-power-56x.c
 *
 * Copyright (C) 2016 RICOH ELECTRONIC DEVICES COMPANY,LTD.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */
#include <linux/i2c.h>
#include <linux/pda_power.h>
#include <linux/platform_device.h>
#include <linux/resource.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/rn5t56x.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/rn5t56x-regulator.h>
#include <mach/irqs.h>

/* RN5T56x IRQs */
#define RN5T56x_IRQ_BASE	INT_PMIC_BASE
#define RN5T56x_GPIO_BASE	INT_GPIO_BASE

static struct regulator_consumer_supply rn5t56x_dc1_supply_0[] = {
	REGULATOR_SUPPLY("vdd_cpu", NULL),
};
static struct regulator_consumer_supply rn5t56x_dc2_supply_0[] = {
	REGULATOR_SUPPLY("vdd_core", NULL),
};
static struct regulator_consumer_supply rn5t56x_dc3_supply_0[] = {
	REGULATOR_SUPPLY("vdd_ddr", NULL),
};
static struct regulator_consumer_supply rn5t56x_dc4_supply_0[] = {
	REGULATOR_SUPPLY("vdd_ddr2", NULL),
};
static struct regulator_consumer_supply rn5t56x_ldo1_supply_0[] = {
	REGULATOR_SUPPLY("vdd_iod0", NULL),
};
static struct regulator_consumer_supply rn5t56x_ldo2_supply_0[] = {
	REGULATOR_SUPPLY("vdd_18d", NULL),
};
static struct regulator_consumer_supply rn5t56x_ldo3_supply_0[] = {
	REGULATOR_SUPPLY("vdd_cam", NULL),
};
static struct regulator_consumer_supply rn5t56x_ldo4_supply_0[] = {
	REGULATOR_SUPPLY("vdd_hdmi_pll", NULL),
};
static struct regulator_consumer_supply rn5t56x_ldo5_supply_0[] = {
	REGULATOR_SUPPLY("vdd_hdmi_osc", NULL),
};
#define RICOH_PDATA_INIT(_name, _sname, _minmv, _maxmv, _supply_reg, _always_on, \
	_boot_on, _apply_uV, _init_mV, _init_enable, _init_apply, _flags, \
	_ext_contol, _sleep_mV) \
	static struct rn5t56x_regulator_platform_data pdata_##_name##_##_sname = \
	{ \
		.regulator = { \
			.constraints = { \
				.min_uV = (_minmv)*1000, \
				.max_uV = (_maxmv)*1000, \
				.valid_modes_mask = (REGULATOR_MODE_NORMAL | \
				REGULATOR_MODE_STANDBY), \
				.valid_ops_mask = (REGULATOR_CHANGE_MODE | \
				REGULATOR_CHANGE_STATUS | \
				REGULATOR_CHANGE_VOLTAGE), \
				.always_on = _always_on, \
				.boot_on = _boot_on, \
				.apply_uV = _apply_uV, \
			}, \
			.num_consumer_supplies = \
			ARRAY_SIZE(rn5t56x_##_name##_supply_##_sname), \
			.consumer_supplies = rn5t56x_##_name##_supply_##_sname, \
			.supply_regulator = _supply_reg, \
		}, \
		.init_uV = (_init_mV)*1000, \
		.init_enable = _init_enable, \
		.init_apply = _init_apply, \
		.flags = _flags, \
		.ext_pwr_req = _ext_contol, \
		.sleep_uV = (_sleep_mV)*1000, \
	}
RICOH_PDATA_INIT(dc1, 0, 950, 1500, 0, 1, 1, 0, -1, 0, 0, 0, 0, 950);
RICOH_PDATA_INIT(dc2, 0, 600, 3500, 0, 1, 1, 0, -1, 0, 0, 0, 0, 0);
RICOH_PDATA_INIT(dc3, 0, 950, 1500, 0, 1, 1, 0, 1500, 0, 0, 0, 0, 950);
RICOH_PDATA_INIT(dc4, 0, 600, 3500, 0, 1, 1, 0, -1, 0, 0, 0, 0, 0);
RICOH_PDATA_INIT(ldo1, 0, 900, 3500, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0);
RICOH_PDATA_INIT(ldo2, 0, 900, 3500, 0, 1, 0, 1, -1, 0, 0, 0, 0, 0);
RICOH_PDATA_INIT(ldo3, 0, 600, 3500, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0);
RICOH_PDATA_INIT(ldo4, 0, 900, 3500, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0);
RICOH_PDATA_INIT(ldo5, 0, 900, 3500, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0);
#define RICOH_REG(_id, _name, _sname) \
	{ \
		.id = RN5T56x_ID_##_id, \
		.name = "rn5t56x-regulator", \
		.platform_data = &pdata_##_name##_##_sname, \
	}
#define RN5T56x_DEV_REG		\
	RICOH_REG(DC1, dc1, 0), \
	RICOH_REG(DC2, dc2, 0), \
	RICOH_REG(DC3, dc3, 0), \
	RICOH_REG(DC4, dc4, 0), \
	RICOH_REG(LDO1, ldo1, 0), \
	RICOH_REG(LDO2, ldo2, 0), \
	RICOH_REG(LDO3, ldo3, 0), \
	RICOH_REG(LDO4, ldo4, 0), \
	RICOH_REG(LDO5, ldo5, 0),

static struct rn5t56x_pwrkey_platform_data rn5t56x_pwrkey_data = {
	.irq = RN5T56x_IRQ_BASE,
	.delay_ms = 20,
};
#define RN5T56x_PWRKEY_REG \
	{ \
		.id = -1, \
		.name = "rn5t56x-pwrkey", \
		.platform_data = &rn5t56x_pwrkey_data, \
	}

static struct rn5t56x_subdev_info rn5t56x_subdev[] = {
	RN5T56x_DEV_REG,
	RN5T56x_PWRKEY_REG,
};

#define RICOH_GPIO_INIT(_output_mode, _output_val, _init_apply, _led_mode, _led_func) \
	{					\
		.output_mode_en = _output_mode,\
		.output_val = _output_val,	\
		.init_apply = _init_apply,	\
		.led_mode = _led_mode,		\
		.led_func = _led_func,		\
	}

struct rn5t56x_gpio_init_data ricoh_gpio_data[] = {
		RICOH_GPIO_INIT(false, false, false, false, 0),
		RICOH_GPIO_INIT(false, false, false, false, 0),
		RICOH_GPIO_INIT(false, false, false, false, 0),
		RICOH_GPIO_INIT(true,  false,  true, true, 1),
};

static struct rn5t56x_platform_data rn5t56x_info = {
	.num_subdevs		= ARRAY_SIZE(rn5t56x_subdev),
	.subdevs		= rn5t56x_subdev,
	.gpio_base		= RN5T56x_GPIO_BASE,
	.irq_base		= RN5T56x_IRQ_BASE,
	.gpio_init_data		= ricoh_gpio_data,
	.num_gpioinit_data	= ARRAY_SIZE(ricoh_gpio_data),
	.enable_shutdown_pin	= true,
};


static struct i2c_board_info __initdata rn5t56x_regulators[] = {
	{
		I2C_BOARD_INFO("rn5t56x", 0x32),
		.irq           = PMIC_IRQ,
		.platform_data = &rn5t56x_info,
	},
};

