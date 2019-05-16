/*
 * Copyright (C) 2011-2015 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regulator/of_regulator.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/pfuze100.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/regmap.h>

#define PFUZE_NUMREGS		95
#define PFUZE100_VOL_OFFSET	0
#define PFUZE100_STANDBY_OFFSET	1
/*
 * below regs will lost after exit from LPSR mode(PFUZE3000), need to be saved
 * and restored:
 * 0x20~0x40: 33
 * 0x66~0x71: 12
 * 0x7f: 1
 * total 46 registers.
 */
#define PFUZE100_REG_SAVED_NUM (33 + 12 + 1)

#define PFUZE100_DEVICEID	0x0
#define PFUZE100_REVID		0x1
#define PFUZE100_FABID		0x1

#define PFUZE100_SW1ABVOL	0x36
#define PFUZE100_SW1CVOL	0x37
#define PFUZE100_SW2VOL		0x38
#define PFUZE100_SW3AVOL	0x39
//#define PFUZE100_SW3BVOL	0x39


//#define PFUZE100_SW4VOL		0x00
//#define PFUZE100_SWBSTCON1	0x00
//#define PFUZE100_VREFDDRCON	0x00
//#define PFUZE100_VSNVSVOL	0x00


#define PFUZE100_VGEN1VOL	0x4c  
#define PFUZE100_VGEN2VOL	0x4d
#define PFUZE100_VGEN3VOL	0x4e
#define PFUZE100_VGEN4VOL	0x4f
#define PFUZE100_VGEN5VOL	0x50   
//#define PFUZE100_VGEN6VOL	0x51

enum chips { PFUZE100, PFUZE200, PFUZE3000 = 3, PFUZE3001 = 0x31, };

struct pfuze_regulator {
	struct regulator_desc desc;
	unsigned char stby_reg;
	unsigned char stby_mask;
};

struct pfuze_chip {
	int	chip_id;
	struct regmap *regmap;
	struct device *dev;
	bool need_restore;
	unsigned int reg_save_array[PFUZE100_REG_SAVED_NUM];
	struct pfuze_regulator regulator_descs[PFUZE100_MAX_REGULATOR];
	struct regulator_dev *regulators[PFUZE100_MAX_REGULATOR];
};


static const struct i2c_device_id pfuze_device_id[] = {
	{.name = "pfuze100", .driver_data = PFUZE100},
	{.name = "pfuze200", .driver_data = PFUZE200},
	{.name = "pfuze3000", .driver_data = PFUZE3000},
	{.name = "pfuze3001", .driver_data = PFUZE3001},
	{ }
};
MODULE_DEVICE_TABLE(i2c, pfuze_device_id);

static const struct of_device_id pfuze_dt_ids[] = {
	{ .compatible = "fsl,pfuze100", .data = (void *)PFUZE100},
	{ .compatible = "fsl,pfuze200", .data = (void *)PFUZE200},
	{ .compatible = "fsl,pfuze3000", .data = (void *)PFUZE3000},
	{ .compatible = "fsl,pfuze3001", .data = (void *)PFUZE3001},
	{ }
};
MODULE_DEVICE_TABLE(of, pfuze_dt_ids);

static int pfuze100_set_ramp_delay(struct regulator_dev *rdev, int ramp_delay)
{
	return 0;
}

static struct regulator_ops pfuze100_ldo_regulator_ops = {
	.enable = regulator_enable_regmap,
	.disable = regulator_disable_regmap,
	.is_enabled = regulator_is_enabled_regmap,
	.list_voltage = regulator_list_voltage_linear,
	.set_voltage_sel =  regulator_set_voltage_sel_regmap,
	.get_voltage_sel = regulator_get_voltage_sel_regmap,
};

static struct regulator_ops pfuze100_fixed_regulator_ops = {
	.list_voltage = regulator_list_voltage_linear,
};

static struct regulator_ops pfuze100_sw_regulator_ops = {
	.list_voltage = regulator_list_voltage_linear,
	.set_voltage_sel = regulator_set_voltage_sel_regmap,
	.get_voltage_sel = regulator_get_voltage_sel_regmap,
	.set_voltage_time_sel = regulator_set_voltage_time_sel,
	.set_ramp_delay = pfuze100_set_ramp_delay,
};

static struct regulator_ops pfuze100_swb_regulator_ops = {
	.list_voltage = regulator_list_voltage_table,
	.map_voltage = regulator_map_voltage_ascend,
	.set_voltage_sel = regulator_set_voltage_sel_regmap,
	.get_voltage_sel = regulator_get_voltage_sel_regmap,

};

#define PFUZE100_FIXED_REG(_chip, _name, base, voltage)	\
	[_chip ## _ ## _name] = {	\
		.desc = {	\
			.name = #_name,	\
			.n_voltages = 1,	\
			.ops = &pfuze100_fixed_regulator_ops,	\
			.type = REGULATOR_VOLTAGE,	\
			.id = _chip ## _ ## _name,	\
			.owner = THIS_MODULE,	\
			.min_uV = (voltage),	\
			.enable_reg = (base),	\
			.enable_mask = 0x10,	\
		},	\
	}

#define PFUZE100_SW_REG(_chip, _name, base, min, max, step)	\
	[_chip ## _ ## _name] = {	\
		.desc = {	\
			.name = #_name,\
			.n_voltages = ((max) - (min)) / (step) + 1,	\
			.ops = &pfuze100_sw_regulator_ops,	\
			.type = REGULATOR_VOLTAGE,	\
			.id = _chip ## _ ## _name,	\
			.owner = THIS_MODULE,	\
			.min_uV = (min),	\
			.uV_step = (step),	\
			.vsel_reg = (base) + PFUZE100_VOL_OFFSET,	\
			.vsel_mask = 0xff,	\
		},	\
		.stby_reg = (base) + PFUZE100_VOL_OFFSET,	\
		.stby_mask = 0xff,	\
	}

#define PFUZE100_SWB_REG(_chip, _name, base, mask, voltages)	\
	[_chip ## _ ##  _name] = {	\
		.desc = {	\
			.name = #_name,	\
			.n_voltages = ARRAY_SIZE(voltages),	\
			.ops = &pfuze100_swb_regulator_ops,	\
			.type = REGULATOR_VOLTAGE,	\
			.id = _chip ## _ ## _name,	\
			.owner = THIS_MODULE,	\
			.volt_table = voltages,	\
			.vsel_reg = (base),	\
			.vsel_mask = (mask),	\
		},	\
	}

#define PFUZE100_VGEN_REG(_chip, _name, base, min, max, step)	\
	[_chip ## _ ## _name] = {	\
		.desc = {	\
			.name = #_name,	\
			.n_voltages = ((max) - (min)) / (step) + 1,	\
			.ops = &pfuze100_ldo_regulator_ops,	\
			.type = REGULATOR_VOLTAGE,	\
			.id = _chip ## _ ## _name,	\
			.owner = THIS_MODULE,	\
			.min_uV = (min),	\
			.uV_step = (step),	\
			.vsel_reg = (base),	\
			.vsel_mask = 0x7F,	\
			.enable_reg = (0x44),	\
			.enable_mask = (0x01 <<(base-0x4C)),	\
		},	\
		.stby_reg = (base),	\
		.stby_mask = 0x00,	\
	}

#define PFUZE3000_VCC_REG(_chip, _name, base, min, max, step)	{	\
	.desc = {	\
		.name = #_name,	\
		.n_voltages = ((max) - (min)) / (step) + 1,	\
		.ops = &pfuze100_ldo_regulator_ops,	\
		.type = REGULATOR_VOLTAGE,	\
		.id = _chip ## _ ## _name,	\
		.owner = THIS_MODULE,	\
		.min_uV = (min),	\
		.uV_step = (step),	\
		.vsel_reg = (base),	\
		.vsel_mask = 0x3,	\
		.enable_reg = (base),	\
		.enable_mask = 0x10,	\
	},	\
	.stby_reg = (base),	\
	.stby_mask = 0x20,	\
}


#define PFUZE3000_SW2_REG(_chip, _name, base, min, max, step)	{	\
	.desc = {	\
		.name = #_name,\
		.n_voltages = ((max) - (min)) / (step) + 1,	\
		.ops = &pfuze100_sw_regulator_ops,	\
		.type = REGULATOR_VOLTAGE,	\
		.id = _chip ## _ ## _name,	\
		.owner = THIS_MODULE,	\
		.min_uV = (min),	\
		.uV_step = (step),	\
		.vsel_reg = (base) + PFUZE100_VOL_OFFSET,	\
		.vsel_mask = 0x7,	\
	},	\
	.stby_reg = (base) + PFUZE100_STANDBY_OFFSET,	\
	.stby_mask = 0x7,	\
}

#define PFUZE3000_SW3_REG(_chip, _name, base, min, max, step)	{	\
	.desc = {	\
		.name = #_name,\
		.n_voltages = ((max) - (min)) / (step) + 1,	\
		.ops = &pfuze100_sw_regulator_ops,	\
		.type = REGULATOR_VOLTAGE,	\
		.id = _chip ## _ ## _name,	\
		.owner = THIS_MODULE,	\
		.min_uV = (min),	\
		.uV_step = (step),	\
		.vsel_reg = (base) + PFUZE100_VOL_OFFSET,	\
		.vsel_mask = 0xf,	\
	},	\
	.stby_reg = (base) + PFUZE100_STANDBY_OFFSET,	\
	.stby_mask = 0xf,	\
}

/* PFUZE100 */
static struct pfuze_regulator pfuze100_regulators[] = {
	PFUZE100_SW_REG(PFUZE100, SW1AB, PFUZE100_SW1ABVOL, 600000, 1875000, 12500),
	PFUZE100_SW_REG(PFUZE100, SW1C, PFUZE100_SW1CVOL, 600000, 1875000, 12500),
	PFUZE100_SW_REG(PFUZE100, SW2, PFUZE100_SW2VOL, 600000, 3300000, 12500),
	PFUZE100_SW_REG(PFUZE100, SW3A, PFUZE100_SW3AVOL, 600000, 1975000, 12500),
	//PFUZE100_SW_REG(PFUZE100, SW3B, PFUZE100_SW3BVOL, 400000, 1975000, 25000),
	//PFUZE100_SW_REG(PFUZE100, SW4, PFUZE100_SW4VOL, 400000, 1975000, 25000),
	//PFUZE100_SWB_REG(PFUZE100, SWBST, PFUZE100_SWBSTCON1, 0x3 , pfuze100_swbst),
	//PFUZE100_SWB_REG(PFUZE100, VSNVS, PFUZE100_VSNVSVOL, 0x7, pfuze100_vsnvs),
	//PFUZE100_FIXED_REG(PFUZE100, VREFDDR, PFUZE100_VREFDDRCON, 750000),
	PFUZE100_VGEN_REG(PFUZE100, VGEN1, PFUZE100_VGEN1VOL, 900000, 3500000, 25000),
	PFUZE100_VGEN_REG(PFUZE100, VGEN2, PFUZE100_VGEN2VOL, 900000, 3500000, 25000),
	PFUZE100_VGEN_REG(PFUZE100, VGEN3, PFUZE100_VGEN3VOL, 600000, 3500000, 25000),
	PFUZE100_VGEN_REG(PFUZE100, VGEN4, PFUZE100_VGEN4VOL, 900000, 3500000, 25000),
	PFUZE100_VGEN_REG(PFUZE100, VGEN5, PFUZE100_VGEN5VOL, 900000, 3500000, 25000),
	//PFUZE100_VGEN_REG(PFUZE100, VGEN6, PFUZE100_VGEN6VOL, 1800000, 3300000, 100000),
};


static struct pfuze_regulator *pfuze_regulators;

#ifdef CONFIG_OF
/* PFUZE100 */
static struct of_regulator_match pfuze100_matches[] = {
	{ .name = "sw1ab",	},
	{ .name = "sw1c",	},
	{ .name = "sw2",	},
	{ .name = "sw3a",	},
	//{ .name = "sw3b",	},
	//{ .name = "sw4",	},
	//{ .name = "swbst",	},
	//{ .name = "vsnvs",	},
	//{ .name = "vrefddr",	},
	{ .name = "vgen1",	},
	{ .name = "vgen2",	},
	{ .name = "vgen3",	},
	{ .name = "vgen4",	},
	{ .name = "vgen5",	},
	//{ .name = "vgen6",	},
};

/* PFUZE200 */
static struct of_regulator_match pfuze200_matches[] = {

	{ .name = "sw1ab",	},
	{ .name = "sw2",	},
	{ .name = "sw3a",	},
	{ .name = "sw3b",	},
	{ .name = "swbst",	},
	{ .name = "vsnvs",	},
	{ .name = "vrefddr",	},
	{ .name = "vgen1",	},
	{ .name = "vgen2",	},
	{ .name = "vgen3",	},
	{ .name = "vgen4",	},
	{ .name = "vgen5",	},
	{ .name = "vgen6",	},
};

/* PFUZE3000 */
static struct of_regulator_match pfuze3000_matches[] = {

	{ .name = "sw1a",	},
	{ .name = "sw1b",	},
	{ .name = "sw2",	},
	{ .name = "sw3",	},
	{ .name = "swbst",	},
	{ .name = "vsnvs",	},
	{ .name = "vrefddr",	},
	{ .name = "vldo1",	},
	{ .name = "vldo2",	},
	{ .name = "vccsd",	},
	{ .name = "v33",	},
	{ .name = "vldo3",	},
	{ .name = "vldo4",	},
};

/* PFUZE3001 */
static struct of_regulator_match pfuze3001_matches[] = {

	{ .name = "sw1",	},
	{ .name = "sw2",	},
	{ .name = "sw3",	},
	{ .name = "vsnvs",	},
	{ .name = "vrefddr",	},
	{ .name = "vldo1",	},
	{ .name = "vldo2",	},
	{ .name = "vccsd",	},
	{ .name = "v33",	},
	{ .name = "vldo3",	},
	{ .name = "vldo4",	},
};

static struct of_regulator_match *pfuze_matches;

static int pfuze_parse_regulators_dt(struct pfuze_chip *chip)
{
	struct device *dev = chip->dev;
	struct device_node *np, *parent;
	int ret;

	np = of_node_get(dev->of_node);
	if (!np)
		return -EINVAL;

	parent = of_get_child_by_name(np, "regulators");
	if (!parent) {
		dev_err(dev, "regulators node not found\n");
		return -EINVAL;
	}

	switch (chip->chip_id) {
	case PFUZE3001:
		pfuze_matches = pfuze3001_matches;
		ret = of_regulator_match(dev, parent, pfuze3001_matches,
					 ARRAY_SIZE(pfuze3001_matches));
		break;
	case PFUZE3000:
		pfuze_matches = pfuze3000_matches;
		ret = of_regulator_match(dev, parent, pfuze3000_matches,
					 ARRAY_SIZE(pfuze3000_matches));
		break;
	case PFUZE200:
		pfuze_matches = pfuze200_matches;
		ret = of_regulator_match(dev, parent, pfuze200_matches,
					 ARRAY_SIZE(pfuze200_matches));
		break;

	case PFUZE100:
	default:
		pfuze_matches = pfuze100_matches;
		ret = of_regulator_match(dev, parent, pfuze100_matches,
					 ARRAY_SIZE(pfuze100_matches));
		break;
	}

	of_node_put(parent);
	if (ret < 0) {
		dev_err(dev, "Error parsing regulator init data: %d\n",
			ret);
		return ret;
	}

	return 0;
}

static inline struct regulator_init_data *match_init_data(int index)
{
	return pfuze_matches[index].init_data;
}

static inline struct device_node *match_of_node(int index)
{
	return pfuze_matches[index].of_node;
}
#else
static int pfuze_parse_regulators_dt(struct pfuze_chip *chip)
{
	return 0;
}

static inline struct regulator_init_data *match_init_data(int index)
{
	return NULL;
}

static inline struct device_node *match_of_node(int index)
{
	return NULL;
}
#endif

static int pfuze_identify(struct pfuze_chip *pfuze_chip)
{
	unsigned int value;
	int ret;

	ret = regmap_read(pfuze_chip->regmap, PFUZE100_DEVICEID, &value);
	if (ret)
		return ret;
	return 0;
}

static const struct regmap_config pfuze_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = PFUZE_NUMREGS - 1,
	.cache_type = REGCACHE_RBTREE,
};

static int pfuze100_regulator_probe(struct i2c_client *client,
				    const struct i2c_device_id *id)
{
	struct pfuze_chip *pfuze_chip;
	struct pfuze_regulator_platform_data *pdata =
	    dev_get_platdata(&client->dev);
	struct regulator_config config = { };
	int i, ret;
	const struct of_device_id *match;
	u32 regulator_num;
	u32 sw_check_start, sw_check_end, sw_hi = 0x40;

	pfuze_chip = devm_kzalloc(&client->dev, sizeof(*pfuze_chip),
			GFP_KERNEL);
	if (!pfuze_chip)
		return -ENOMEM;

	if (client->dev.of_node) {
		match = of_match_device(of_match_ptr(pfuze_dt_ids),
				&client->dev);
		if (!match) {
			dev_err(&client->dev, "Error: No device match found\n");
			return -ENODEV;
		}
		pfuze_chip->chip_id = (int)(long)match->data;
	} else if (id) {
		pfuze_chip->chip_id = id->driver_data;
	} else {
		dev_err(&client->dev, "No dts match or id table match found\n");
		return -ENODEV;
	}

	i2c_set_clientdata(client, pfuze_chip);
	pfuze_chip->dev = &client->dev;

	pfuze_chip->regmap = devm_regmap_init_i2c(client, &pfuze_regmap_config);
	if (IS_ERR(pfuze_chip->regmap)) {
		ret = PTR_ERR(pfuze_chip->regmap);
		dev_err(&client->dev,
			"regmap allocation failed with err %d\n", ret);
		return ret;
	}

	ret = pfuze_identify(pfuze_chip);
	if (ret) {
		dev_err(&client->dev, "unrecognized pfuze chip ID!\n");
		return ret;
	}

	/* use the right regulators after identify the right device */
	switch (pfuze_chip->chip_id) {
	case PFUZE100:
	default:
		pfuze_regulators = pfuze100_regulators;
		regulator_num = ARRAY_SIZE(pfuze100_regulators);
		sw_check_start = PFUZE100_SW2;
		sw_check_end = PFUZE100_SW3A;
		break;
	}
	dev_info(&client->dev, "pfuze%s found.\n",
		(pfuze_chip->chip_id == PFUZE100) ? "100" :
		(((pfuze_chip->chip_id == PFUZE200) ? "200" :
		((pfuze_chip->chip_id == PFUZE3000) ? "3000" : "3001"))));

	memcpy(pfuze_chip->regulator_descs, pfuze_regulators,
		sizeof(pfuze_chip->regulator_descs));

	ret = pfuze_parse_regulators_dt(pfuze_chip);
	if (ret)
		return ret;

	for (i = 0; i < regulator_num; i++) {
		struct regulator_init_data *init_data;
		struct regulator_desc *desc;
		int val;

		desc = &pfuze_chip->regulator_descs[i].desc;

		if (pdata)
			init_data = pdata->init_data[i];
		else
			init_data = match_init_data(i);

	

		config.dev = &client->dev;
		config.init_data = init_data;
		config.driver_data = pfuze_chip;
		config.of_node = match_of_node(i);
		if(desc->vsel_reg == PFUZE100_VGEN2VOL){
			regmap_update_bits(pfuze_chip->regmap, desc->enable_reg,
				  desc->enable_mask, desc->enable_mask);
		}
		if(desc->vsel_reg == PFUZE100_VGEN3VOL){ //set 3.3
			regmap_update_bits(pfuze_chip->regmap, desc->vsel_reg,
				  desc->vsel_mask, 0x6C);
			//regmap_read(pfuze_chip->regmap, desc->vsel_reg, &val);
			//printk(KERN_INFO"----name reg:%x,val:%x----\n",desc->vsel_reg, val);
		}
		
		
		pfuze_chip->regulators[i] =
			devm_regulator_register(&client->dev, desc, &config);
		if (IS_ERR(pfuze_chip->regulators[i])) {
			dev_err(&client->dev, "register regulator%s failed\n",
				pfuze_regulators[i].desc.name);
			return PTR_ERR(pfuze_chip->regulators[i]);
		}
	}


	if (of_get_property(client->dev.of_node, "fsl,lpsr-mode", NULL))
		pfuze_chip->need_restore = true;

	return 0;
	
}

static int pfuze_reg_save_restore(struct pfuze_chip *pfuze_chip, int start,
				  int end, int index, bool save)
{
	int i, ret;

	for (i = 0; i < end - start + 1; i++) {
		if (save)
			ret = regmap_read(pfuze_chip->regmap, start + i,
					&pfuze_chip->reg_save_array[index + i]);
		else
			ret = regmap_write(pfuze_chip->regmap, start + i,
					pfuze_chip->reg_save_array[index + i]);

		if (ret)
			return ret;
	}

	return index + i;
}

static int pfuze_suspend(struct device *dev)
{

	return 0;
}

static int pfuze_resume(struct device *dev)
{
	return 0;

}

static const struct dev_pm_ops pfuze_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(pfuze_suspend, pfuze_resume)
};

static struct i2c_driver pfuze_driver = {
	.id_table = pfuze_device_id,
	.driver = {
		.name = "pfuze100-regulator",
		.owner = THIS_MODULE,
		.of_match_table = pfuze_dt_ids,
		.pm = &pfuze_pm_ops,
	},
	.probe = pfuze100_regulator_probe,
};
module_i2c_driver(pfuze_driver);

MODULE_AUTHOR("Robin Gong <b38343@freescale.com>");
MODULE_DESCRIPTION("Regulator Driver for Freescale PFUZE100/PFUZE200 PMIC");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("i2c:pfuze100-regulator");
