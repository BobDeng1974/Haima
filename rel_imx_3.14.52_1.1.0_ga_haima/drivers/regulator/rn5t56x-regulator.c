/*
 * drivers/regulator/rn5t56x-regulator.c
 *
 * Regulator driver for RN5T56x power management chip.
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

/*#define DEBUG			1*/
/*#define VERBOSE_DEBUG		1*/
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/rn5t56x.h>
#include <linux/regulator/rn5t56x-regulator.h>

/*#define KERNELRELEASE_IS_V310	1 */

struct rn5t56x_regulator {
	int		id;
	int		sleep_id;
	/* Regulator register address.*/
	u8		reg_en_reg;
	u8		en_bit;
	u8		reg_disc_reg;
	u8		disc_bit;
	u8		vout_reg;
	u8		vout_mask;
	u8		vout_reg_cache;
	u8		sleep_reg;

	/* chip constraints on regulator behavior */
	int			min_uV;
	int			max_uV;
	int			step_uV;
	int			nsteps;

	/* regulator specific turn-on delay */
	u16			delay;

	/* used by regulator core */
	struct regulator_desc	desc;

	/* Device */
	struct device		*dev;
};

static unsigned int rn5t56x_suspend_status;

static inline struct device *to_rn5t56x_dev(struct regulator_dev *rdev)
{
	return rdev_get_dev(rdev)->parent->parent;
}

static int rn5t56x_regulator_enable_time(struct regulator_dev *rdev)
{
	struct rn5t56x_regulator *ri = rdev_get_drvdata(rdev);

	return ri->delay;
}

static int rn5t56x_reg_is_enabled(struct regulator_dev *rdev)
{
	struct rn5t56x_regulator *ri = rdev_get_drvdata(rdev);
	struct device *parent = to_rn5t56x_dev(rdev);
	uint8_t control;
	int ret;

	ret = rn5t56x_read(parent, ri->reg_en_reg, &control);
	if (ret < 0) {
		dev_err(&rdev->dev, "Error in reading the control register\n");
		return ret;
	}
	return (((control >> ri->en_bit) & 1) == 1);
}

static int rn5t56x_reg_enable(struct regulator_dev *rdev)
{
	struct rn5t56x_regulator *ri = rdev_get_drvdata(rdev);
	struct device *parent = to_rn5t56x_dev(rdev);
	int ret;

	ret = rn5t56x_set_bits(parent, ri->reg_en_reg, (1 << ri->en_bit));
	if (ret < 0) {
		dev_err(&rdev->dev, "Error in updating the STATE register\n");
		return ret;
	}
	udelay(ri->delay);
	return ret;
}

static int rn5t56x_reg_disable(struct regulator_dev *rdev)
{
	struct rn5t56x_regulator *ri = rdev_get_drvdata(rdev);
	struct device *parent = to_rn5t56x_dev(rdev);
	int ret;

	ret = rn5t56x_clr_bits(parent, ri->reg_en_reg, (1 << ri->en_bit));
	if (ret < 0)
		dev_err(&rdev->dev, "Error in updating the STATE register\n");

	return ret;
}

static int rn5t56x_list_voltage(struct regulator_dev *rdev, unsigned index)
{
	struct rn5t56x_regulator *ri = rdev_get_drvdata(rdev);

	return ri->min_uV + (ri->step_uV * index);
}

static int __rn5t56x_set_s_voltage(struct device *parent,
		struct rn5t56x_regulator *ri, int min_uV, int max_uV)
{
	int vsel;
	int ret;

	if ((min_uV < ri->min_uV) || (max_uV > ri->max_uV))
		return -EDOM;

	vsel = (min_uV - ri->min_uV + ri->step_uV - 1)/ri->step_uV;
	if (vsel > ri->nsteps)
		return -EDOM;

	ret = rn5t56x_update(parent, ri->sleep_reg, vsel, ri->vout_mask);
	if (ret < 0)
		dev_err(ri->dev, "Error in writing the sleep register\n");
	return ret;
}
static int rn5t56x_set_s_voltage(struct regulator_dev *rdev, int uV)
{
	struct rn5t56x_regulator *ri = rdev_get_drvdata(rdev);
	struct device *parent = to_rn5t56x_dev(rdev);

	if (rn5t56x_suspend_status)
		return -EBUSY;

	return __rn5t56x_set_s_voltage(parent, ri, uV, uV);
}

static int __rn5t56x_set_voltage(struct device *parent,
		struct rn5t56x_regulator *ri, int min_uV, int max_uV,
		unsigned *selector)
{
	int vsel;
	int ret;
	uint8_t vout_val;

	if ((min_uV < ri->min_uV) || (max_uV > ri->max_uV))
		return -EDOM;

	vsel = (min_uV - ri->min_uV + ri->step_uV - 1)/ri->step_uV;
	if (vsel > ri->nsteps)
		return -EDOM;

	if (selector)
		*selector = vsel;

	vout_val = (ri->vout_reg_cache & ~ri->vout_mask) |
				(vsel & ri->vout_mask);
	ret = rn5t56x_write(parent, ri->vout_reg, vout_val);
	if (ret < 0)
		dev_err(ri->dev, "Error in writing the Voltage register\n");
	else
		ri->vout_reg_cache = vout_val;

	return ret;
}

static int rn5t56x_set_voltage(struct regulator_dev *rdev,
		int min_uV, int max_uV, unsigned *selector)
{
	struct rn5t56x_regulator *ri = rdev_get_drvdata(rdev);
	struct device *parent = to_rn5t56x_dev(rdev);

	if (rn5t56x_suspend_status)
		return -EBUSY;

	return __rn5t56x_set_voltage(parent, ri, min_uV, max_uV, selector);
}

static int rn5t56x_get_voltage(struct regulator_dev *rdev)
{
	struct rn5t56x_regulator *ri = rdev_get_drvdata(rdev);
	uint8_t vsel;

	vsel = ri->vout_reg_cache & ri->vout_mask;
	return ri->min_uV + vsel * ri->step_uV;
}

static struct regulator_ops rn5t56x_ops = {
	.list_voltage	= rn5t56x_list_voltage,
	.set_voltage	= rn5t56x_set_voltage,
	.get_voltage	= rn5t56x_get_voltage,
	.enable		= rn5t56x_reg_enable,
	.disable	= rn5t56x_reg_disable,
	.is_enabled	= rn5t56x_reg_is_enabled,
	.enable_time	= rn5t56x_regulator_enable_time,
	.set_suspend_voltage = rn5t56x_set_s_voltage,
};

#define RN5T56x_REG(_id, _en_reg, _en_bit, _disc_reg, _disc_bit, _vout_reg, \
		_vout_mask, _ds_reg, _min_mv, _max_mv, _step_uV, _nsteps,    \
		_ops, _delay)					\
{								\
	.reg_en_reg	= _en_reg,				\
	.en_bit		= _en_bit,				\
	.reg_disc_reg	= _disc_reg,				\
	.disc_bit	= _disc_bit,				\
	.vout_reg	= _vout_reg,				\
	.vout_mask	= _vout_mask,				\
	.sleep_reg	= _ds_reg,				\
	.min_uV		= _min_mv * 1000,			\
	.max_uV		= _max_mv * 1000,			\
	.step_uV	= _step_uV,				\
	.nsteps		= _nsteps,				\
	.delay		= _delay,				\
	.id		= RN5T56x_ID_##_id,			\
	.sleep_id	= RN5T56x_DS_##_id,			\
	.desc = {						\
		.name = rn5t56x_rails(_id),			\
		.id = RN5T56x_ID_##_id,				\
		.n_voltages = _nsteps,				\
		.ops = &_ops,					\
		.type = REGULATOR_VOLTAGE,			\
		.owner = THIS_MODULE,				\
	},							\
}

static struct rn5t56x_regulator rn5t56x_regulator[] = {
	RN5T56x_REG(DC1, 0x2C, 0, 0x2C, 1, 0x36, 0xFF, 0x3B,
			600, 3500, 12500, 0xE8, rn5t56x_ops, 500),

	RN5T56x_REG(DC2, 0x2E, 0, 0x2E, 1, 0x37, 0xFF, 0x3C,
			600, 3500, 12500, 0xE8, rn5t56x_ops, 500),

	RN5T56x_REG(DC3, 0x30, 0, 0x30, 1, 0x38, 0xFF, 0x3D,
			600, 3500, 12500, 0xE8, rn5t56x_ops, 500),

	RN5T56x_REG(DC4, 0x32, 0, 0x32, 1, 0x39, 0xFF, 0x3E,
			600, 3500, 12500, 0xE8, rn5t56x_ops, 500),

	RN5T56x_REG(LDO1, 0x44, 0, 0x46, 0, 0x4C, 0x7E, 0x58,
			900, 3500, 25000, 0x68, rn5t56x_ops, 500),

	RN5T56x_REG(LDO2, 0x44, 1, 0x46, 1, 0x4D, 0x7E, 0x59,
			900, 3500, 25000, 0x68, rn5t56x_ops, 500),

	RN5T56x_REG(LDO3, 0x44, 2, 0x46, 2, 0x4E, 0x7E, 0x5A,
			600, 3500, 25000, 0x74, rn5t56x_ops, 500),

	RN5T56x_REG(LDO4, 0x44, 3, 0x46, 3, 0x4F, 0x7E, 0x5B,
			900, 3500, 25000, 0x68, rn5t56x_ops, 500),

	RN5T56x_REG(LDO5, 0x44, 4, 0x46, 4, 0x50, 0x7E, 0x5C,
			900, 3500, 25000, 0x68, rn5t56x_ops, 500),

	RN5T56x_REG(LDORTC1, 0x45, 4, 0x00, 0, 0x56, 0x7E, 0x00,
			1200, 3500, 25000, 0x5C, rn5t56x_ops, 500),

	RN5T56x_REG(LDORTC2, 0x45, 5, 0x00, 0, 0x57, 0x7E, 0x00,
			900, 3500, 25000, 0x68, rn5t56x_ops, 500),
};
static inline struct rn5t56x_regulator *find_regulator_info(int id)
{
	struct rn5t56x_regulator *ri;
	int i;

	for (i = 0; i < ARRAY_SIZE(rn5t56x_regulator); i++) {
		ri = &rn5t56x_regulator[i];
		if (ri->desc.id == id)
			return ri;
	}
	return NULL;
}

static int rn5t56x_regulator_preinit(struct device *parent,
		struct rn5t56x_regulator *ri,
		struct rn5t56x_regulator_platform_data *rn5t56x_pdata)
{
	int ret = 0;

	if (!rn5t56x_pdata->init_apply)
		return 0;

	if (rn5t56x_pdata->init_uV >= 0) {
		ret = __rn5t56x_set_voltage(parent, ri,
				rn5t56x_pdata->init_uV,
				rn5t56x_pdata->init_uV, 0);
		if (ret < 0) {
			dev_err(ri->dev, "Not able to initialize voltage %d "
				"for rail %d err %d\n", rn5t56x_pdata->init_uV,
				ri->desc.id, ret);
			return ret;
		}
	}
	
	if (rn5t56x_pdata->sleep_uV > 0) {
		ret = __rn5t56x_set_s_voltage(parent, ri,
				rn5t56x_pdata->sleep_uV,
				rn5t56x_pdata->sleep_uV);
		if (ret < 0) {
			dev_err(ri->dev, "Not able to initialize sleep vol. %d "
				"for rail %d err %d\n", rn5t56x_pdata->sleep_uV,
	   			ri->desc.id, ret);
			return ret;
		}
	}

	if (rn5t56x_pdata->init_enable)
		ret = rn5t56x_set_bits(parent, ri->reg_en_reg,
							(1 << ri->en_bit));
	else
		ret = rn5t56x_clr_bits(parent, ri->reg_en_reg,
							(1 << ri->en_bit));
	if (ret < 0)
		dev_err(ri->dev, "Not able to %s rail %d err %d\n",
			(rn5t56x_pdata->init_enable) ? "enable" : "disable",
			ri->desc.id, ret);

	return ret;
}

static inline int rn5t56x_cache_regulator_register(struct device *parent,
	struct rn5t56x_regulator *ri)
{
	ri->vout_reg_cache = 0;
	return rn5t56x_read(parent, ri->vout_reg, &ri->vout_reg_cache);
}

static int rn5t56x_regulator_probe(struct platform_device *pdev)
{
	struct rn5t56x_regulator *ri = NULL;
	struct regulator_dev *rdev;
	struct rn5t56x_regulator_platform_data *pdata;
	int id = pdev->id;
	int err;

	printk(KERN_DEBUG "PMU: %s\n", __func__);
	ri = find_regulator_info(id);
	if (ri == NULL) {
		dev_err(&pdev->dev, "invalid regulator ID specified\n");
		return -EINVAL;
	}
	pdata = pdev->dev.platform_data;
	ri->dev = &pdev->dev;
	rn5t56x_suspend_status = 0;

	err = rn5t56x_cache_regulator_register(pdev->dev.parent, ri);
	if (err) {
		dev_err(&pdev->dev, "Fail in caching register\n");
		return err;
	}

	err = rn5t56x_regulator_preinit(pdev->dev.parent, ri, pdata);
	if (err) {
		dev_err(&pdev->dev, "Fail in pre-initialisation\n");
		return err;
	}
#ifdef KERNELRELEASE_IS_V310
	{
		struct regulator_config config = { };
		config.dev = &pdev->dev;
		config.init_data = &pdata->regulator;
		config.driver_data = ri;

		rdev = regulator_register(&ri->desc, &config);
	}
#else
	rdev = regulator_register(&ri->desc, &pdev->dev,
				&pdata->regulator, ri);
#endif
	if (IS_ERR_OR_NULL(rdev)) {
		dev_err(&pdev->dev, "failed to register regulator %s\n",
				ri->desc.name);
		return PTR_ERR(rdev);
	}

	platform_set_drvdata(pdev, rdev);
	return 0;
}

static int rn5t56x_regulator_remove(struct platform_device *pdev)
{
	struct regulator_dev *rdev = platform_get_drvdata(pdev);

	regulator_unregister(rdev);
	return 0;
}

#ifdef CONFIG_PM
static int rn5t56x_regulator_suspend(struct device *dev)
{
	printk(KERN_DEBUG "PMU: %s\n", __func__);

	rn5t56x_suspend_status = 1;

	return 0;
}

static int rn5t56x_regulator_resume(struct device *dev)
{
	printk(KERN_DEBUG "PMU: %s\n", __func__);

	rn5t56x_suspend_status = 0;

	return 0;
}

static const struct dev_pm_ops rn5t56x_regulator_pm_ops = {
	.suspend	= rn5t56x_regulator_suspend,
	.resume		= rn5t56x_regulator_resume,
};
#endif

static struct platform_driver rn5t56x_regulator_driver = {
	.driver	= {
		.name	= "rn5t56x-regulator",
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM
		.pm	= &rn5t56x_regulator_pm_ops,
#endif
	},
	.probe		= rn5t56x_regulator_probe,
	.remove		= rn5t56x_regulator_remove,
};

static int __init rn5t56x_regulator_init(void)
{
	printk(KERN_DEBUG "PMU: %s\n", __func__);
	return platform_driver_register(&rn5t56x_regulator_driver);
}
subsys_initcall(rn5t56x_regulator_init);

static void __exit rn5t56x_regulator_exit(void)
{
	platform_driver_unregister(&rn5t56x_regulator_driver);
}
module_exit(rn5t56x_regulator_exit);

MODULE_DESCRIPTION("RICOH RN5T56x regulator driver");
MODULE_ALIAS("platform:rn5t56x-regulator");
MODULE_LICENSE("GPL");
