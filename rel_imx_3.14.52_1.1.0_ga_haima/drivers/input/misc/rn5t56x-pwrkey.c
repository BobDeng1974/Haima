/*
* driver/input/misc/rn5t56x-pwrkey.c
*
* Power Key driver for RN5T56x power management chip.
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
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/mfd/rn5t56x.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>

#define RN5T56x_ONKEY_OFF_IRQ		0

struct rn5t56x_pwrkey {
	struct device *dev;
	struct input_dev *pwr;
	struct workqueue_struct *workqueue;
	struct work_struct work;
	struct rn5t56x_pwrkey_platform_data *pdata;
	unsigned long	delay;
	int		key_irq;
	bool		pressed_first;
	spinlock_t	lock;
};

struct rn5t56x_pwrkey *g_pwrkey;

static void rn5t56x_irq_work(struct work_struct *work)
{
	/* unsigned long flags; */
	uint8_t val;

	printk(KERN_DEBUG "PMU: %s:\n", __func__);
	/* spin_lock_irqsave(&g_pwrkey->lock, flags); */
#if IRQ_TYPE_IS_EDGE
	if (pwrkey_wakeup) {
		printk(KERN_DEBUG "PMU: %s: pwrkey_wakeup\n", __func__);
		pwrkey_wakeup = 0;
		input_event(g_pwrkey->pwr, EV_KEY, KEY_POWER, 1);
		input_event(g_pwrkey->pwr, EV_SYN, 0, 0);
		input_event(g_pwrkey->pwr, EV_KEY, KEY_POWER, 0);
		input_event(g_pwrkey->pwr, EV_SYN, 0, 0);

		return;
	}
#endif
	rn5t56x_read(g_pwrkey->dev->parent, RN5T56x_INT_MON_SYS, &val);

	val &= 0x1;
	if (val) {
		if (!g_pwrkey->pressed_first) {
			g_pwrkey->pressed_first = true;
			printk(KERN_DEBUG "PMU1: Push Power Key!!!\n");
			/* input_report_key(g_pwrkey->pwr, KEY_POWER, 1); */
			/* input_sync(g_pwrkey->pwr); */
			input_event(g_pwrkey->pwr, EV_KEY, KEY_POWER, 1);
			input_event(g_pwrkey->pwr, EV_SYN, 0, 0);
		}
	} else {
		if (g_pwrkey->pressed_first) {
			printk(KERN_DEBUG "PMU2: Release Power Key!!!\n");
			/* input_report_key(g_pwrkey->pwr, KEY_POWER, 0); */
			/* input_sync(g_pwrkey->pwr); */
			input_event(g_pwrkey->pwr, EV_KEY, KEY_POWER, 0);
			input_event(g_pwrkey->pwr, EV_SYN, 0, 0);
		}
		g_pwrkey->pressed_first = false;
	}

	/* spin_unlock_irqrestore(&g_pwrkey->lock, flags); */
}

static irqreturn_t pwrkey_irq(int irq, void *_pwrkey)
{
	printk(KERN_DEBUG "PMU: %s:\n", __func__);

	queue_work(g_pwrkey->workqueue, &g_pwrkey->work);

	return IRQ_HANDLED;
}

#if RN5T56x_ONKEY_OFF_IRQ
static irqreturn_t pwrkey_irq_off(int irq, void *_pwrkey)
{
	dev_warn(g_pwrkey->dev, "ONKEY is pressed long time!\n");
	return IRQ_HANDLED;
}
#endif

static int rn5t56x_pwrkey_probe(struct platform_device *pdev)
{
	struct input_dev *pwr;
	int key_irq;
	int err;
	struct rn5t56x_pwrkey *pwrkey;
	struct rn5t56x_pwrkey_platform_data *pdata = pdev->dev.platform_data;
	uint8_t val;

	printk(KERN_DEBUG "PMU: %s:\n", __func__);

	if (!pdata) {
		dev_err(&pdev->dev, "power key platform data not supplied\n");
		return -EINVAL;
	}
	key_irq = (pdata->irq + RN5T56x_IRQ_POWER_ON);
	printk(KERN_DEBUG "PMU1: %s: key_irq=%d\n", __func__, key_irq);
	pwrkey = kzalloc(sizeof(*pwrkey), GFP_KERNEL);
	if (!pwrkey)
		return -ENOMEM;

	pwrkey->dev = &pdev->dev;
	pwrkey->pdata = pdata;
	pwrkey->pressed_first = false;
	pwrkey_wakeup = 0;
	pwrkey->delay = HZ / 1000 * pdata->delay_ms;
	g_pwrkey = pwrkey;

	pwr = input_allocate_device();
	if (!pwr) {
		dev_dbg(&pdev->dev, "Can't allocate power button\n");
		err = -ENOMEM;
		goto free_pwrkey;
	}
	input_set_capability(pwr, EV_KEY, KEY_POWER);
	pwr->name = "rn5t56x_pwrkey";
	pwr->phys = "rn5t56x_pwrkey/input0";
	pwr->dev.parent = &pdev->dev;

	spin_lock_init(&pwrkey->lock);
	err = input_register_device(pwr);
	if (err) {
		dev_dbg(&pdev->dev, "Can't register power key: %d\n", err);
		goto free_input_dev;
	}
	pwrkey->key_irq = key_irq;
	pwrkey->pwr = pwr;
	platform_set_drvdata(pdev, pwrkey);

	/* Check if power-key is pressed at boot up */
	err = rn5t56x_read(pwrkey->dev->parent, RN5T56x_INT_MON_SYS, &val);
	if (err < 0) {
		dev_err(&pdev->dev, "Key-press status at boot failed rc=%d\n",
									 err);
		goto unreg_input_dev;
	}
	val &= 0x1;
	if (val) {
		input_report_key(pwrkey->pwr, KEY_POWER, 1);
		printk(KERN_DEBUG "******KEY_POWER:1\n");
		input_sync(pwrkey->pwr);
		pwrkey->pressed_first = true;
	}

	err = request_threaded_irq(key_irq, NULL, pwrkey_irq, IRQF_ONESHOT,
						"rn5t56x_pwrkey", pwrkey);
	if (err < 0) {
		dev_err(&pdev->dev, "Can't get %d IRQ for pwrkey: %d\n",
								key_irq, err);
		goto unreg_input_dev;
	}

	#if RN5T56x_ONKEY_OFF_IRQ
	err = request_threaded_irq(key_irq + RN5T56x_IRQ_ONKEY_OFF, NULL,
						pwrkey_irq_off, IRQF_ONESHOT,
						"rn5t56x_pwrkey_off", pwrkey);
	if (err < 0) {
		dev_err(&pdev->dev, "Can't get %d IRQ for pwrkey: %d\n",
			key_irq + RN5T56x_IRQ_ONKEY_OFF, err);
		free_irq(key_irq, pwrkey);
		goto unreg_input_dev;
	}
	#endif

	pwrkey->workqueue = create_singlethread_workqueue("rn5t56x_pwrkey");
	INIT_WORK(&pwrkey->work, rn5t56x_irq_work);

	/* Enable power key IRQ */
	/* trigger both edge */
	rn5t56x_set_bits(pwrkey->dev->parent, RN5T56x_PWR_IRSEL, 0x1);
	/* Enable system interrupt */
	rn5t56x_set_bits(pwrkey->dev->parent, RN5T56x_INTC_INTEN, 0x1);
	/* Enable power-on interrupt */
	rn5t56x_set_bits(pwrkey->dev->parent, RN5T56x_INT_EN_SYS, 0x1);
	printk(KERN_DEBUG "PMU: %s is OK!\n", __func__);
	return 0;

unreg_input_dev:
	input_unregister_device(pwr);
	pwr = NULL;

free_input_dev:
	input_free_device(pwr);
free_pwrkey:
	kfree(pwrkey);

	return err;
}

static int rn5t56x_pwrkey_remove(struct platform_device *pdev)
{
	struct rn5t56x_pwrkey *pwrkey = platform_get_drvdata(pdev);

	flush_workqueue(pwrkey->workqueue);
	destroy_workqueue(pwrkey->workqueue);
	free_irq(pwrkey->key_irq, pwrkey);
	input_unregister_device(pwrkey->pwr);
	kfree(pwrkey);

	return 0;
}

#ifdef CONFIG_PM
static int rn5t56x_pwrkey_suspend(struct device *dev)
{
	struct rn5t56x_pwrkey *info = dev_get_drvdata(dev);

	printk(KERN_DEBUG "PMU: %s\n", __func__);

	cancel_work_sync(&info->work);
	flush_workqueue(info->workqueue);

	return 0;
}

static int rn5t56x_pwrkey_resume(struct device *dev)
{
	printk(KERN_DEBUG "PMU: %s\n", __func__);

	return 0;
}

static const struct dev_pm_ops rn5t56x_pwrkey_pm_ops = {
	.suspend	= rn5t56x_pwrkey_suspend,
	.resume		= rn5t56x_pwrkey_resume,
};
#endif

static struct platform_driver rn5t56x_pwrkey_driver = {
	.probe = rn5t56x_pwrkey_probe,
	.remove = rn5t56x_pwrkey_remove,
	.driver = {
		.name = "rn5t56x-pwrkey",
		.owner = THIS_MODULE,
#ifdef CONFIG_PM
		.pm	= &rn5t56x_pwrkey_pm_ops,
#endif
	},
};

static int __init rn5t56x_pwrkey_init(void)
{
	return platform_driver_register(&rn5t56x_pwrkey_driver);
}
module_init(rn5t56x_pwrkey_init);

static void __exit rn5t56x_pwrkey_exit(void)
{
	platform_driver_unregister(&rn5t56x_pwrkey_driver);
}
module_exit(rn5t56x_pwrkey_exit);


MODULE_DESCRIPTION("RICOH RN5T56x Power Key driver");
MODULE_ALIAS("platform:rn5t56x-pwrkey");
MODULE_LICENSE("GPL v2");
