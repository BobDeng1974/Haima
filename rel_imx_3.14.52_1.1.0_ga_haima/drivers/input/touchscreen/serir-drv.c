/*
 *  Copyright (c) 2016-2018 Lenver Shan <lenver520@163.com>
 */

/*
 * Serial IR Virtual Keyboard Driver for Linux
 */

/*
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to <vojtech@ucw.cz>, or by paper mail:
 * Vojtech Pavlik, Simunkova 1594, Prague 8, 182 00 Czech Republic
 */
#include <linux/types.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/serio.h>
#include <linux/tty.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/syscalls.h>
#include <linux/compat.h>
#include <linux/workqueue.h>


#define SVEV_BUSY	1
#define SVEV_ACTIVE	2
#define SVEV_DEAD	3

#define DRIVER_DESC		"Serial IR Virtual Keyboard Driver"
//#define N_IRKBD			(NR_LDISCS - 2)
#define N_IRKBD  (NR_LDISCS - 3)
/*
 * Per-keyboard data.
 */

static int x = 0,y=0,pen_down=0;

static unsigned char irkbd_ircode[] = {
	0x1C, 0x08, 
	0x56, 0x57, 0x1F, 0x5B, 
	0x55, 0x5E, 0x51, 0x59, 
	0x5A, 0x52, 0x5D, 0x5C, 
	0x01, 0x5F, 0x19, 0x58, 
	0x18, 0x17, 
	0x1A, 0x47, 0x06, 0x07, 0x48, 
	0x49, 0x0A, 
	0x4B, 0x4F, 
	0x03, 0x42, 
	0x09, 0x05, 
	0x54, 0x16, 0x15, 
	0x50, 0x12, 0x11, 
	0x4C, 0x0E, 0x0D, 
	0x41, 0x0C, 0x10,
};

static int irkbd_keymap[] = {
	KEY_POWER, KEY_MUTE,
	KEY_RED, KEY_GREEN, KEY_YELLOW, KEY_BLUE,
	KEY_F1, KEY_F2, KEY_F3, KEY_F4,
	KEY_F5, KEY_F6, KEY_F7, KEY_F8,
	KEY_F9, KEY_F10, KEY_F11, KEY_F12,
	KEY_MENU, KEY_EXIT,
	KEY_UP, KEY_LEFT, KEY_ENTER, KEY_RIGHT, KEY_DOWN,
	KEY_HOME, KEY_STOP,
	KEY_REWIND, KEY_FASTFORWARD,
	KEY_PLAY, KEY_PAUSE,
	KEY_VOLUMEUP, KEY_VOLUMEDOWN,
	KEY_KP1, KEY_KP2, KEY_KP3, 
	KEY_KP4, KEY_KP5, KEY_KP6, 
	KEY_KP7, KEY_KP8, KEY_KP9, 
	KEY_KPASTERISK, KEY_KP0, KEY_BACKSPACE,
};

struct svev {
	struct tty_struct *tty;
	struct input_dev *dev;
	struct serio *serio;
	struct serio_device_id id;

	struct timer_list timer;	// proto timeout timer
	spinlock_t lock;
	unsigned long flags;
	wait_queue_head_t wait;
	int keycode[ARRAY_SIZE(irkbd_keymap)];

	int act_gpio;	// GPIO3_IO26(EIM_D26) IR act pin
	int irq;
	bool pressed;
	
	unsigned int ss;
	unsigned char key;
	int kc;
	spinlock_t vlock;
	char name[64];
	char phys[32];
};

static int get_keycode(unsigned char ircode)
{
	int i = 0;
	int sz = sizeof(irkbd_ircode);
	while(i < sz) {
		if(irkbd_ircode[i] == ircode)
			return irkbd_keymap[i];
		++i;
	}
	return -1;
}


static s32 atoi(char *psz_buf)
{
	char *pch = psz_buf;
	s32 base = 0;

	while (*pch== ' ')
		pch++;

	if (*pch == '-' || *pch == '+') {
		base = 10;
		pch++;
	} 
	base = 10;
	return simple_strtoul(pch, NULL, base);
}


static int parse_ircode_proto(struct svev *sv, unsigned char data)
{
	int res = -1;
#if 0
	if(sv->pressed) {
		sv->ss = 0;
		return res;
	}
	
	if(sv->ss == 0) {
		sv->key = data;
		sv->ss++;
	} else if(sv->ss == 1) {
		if(sv->key == (unsigned char)(~data)) {
			sv->ss++;
		} else {
			sv->ss = 0;
		}
	} else if(sv->ss == 2) {
		sv->key = data;
		sv->ss = 0;
		res = 0;
	}

	if(sv->ss > 0) {
		mod_timer(&sv->timer, jiffies + HZ / 80);	// 12.5ms
		return -1;
	} else if(sv->ss == 0) {
		del_timer(&sv->timer);
	}
#else

typedef  enum {
    F_S,
    F_E,
    F_G,
    F_3,
    F_6,
    F_0,
    F_ID,
    F_LEN,
    F_DATA,
    F_END,
    F_VERIFY
}RecType;

//注意down是1，up是2，move是3   seg360 	255,285,1
static char rec_data[256];
static RecType f_level = F_S;
static char len = 0, index = 0;
	switch(f_level)
	{
		case F_S:
			if(data == 's'){
			rec_data[index++] = data;
			f_level = F_E;
			}
			break;

		case F_E:
			if(data == 'e'){
	                    rec_data[index++] = data;
	                    f_level = F_G;
	                } else {
	                    index = 0;
	                    f_level = F_S;
	                }
			break;

		case F_G:
			if(data == 'g'){
	                    rec_data[index++] = data;
	                    f_level = F_3;
	                } else {
	                    index = 0;
	                    f_level = F_S;
	                }
				
			break;

		case F_3:

			if(data == '3'){
	                    rec_data[index++] = data;
	                    f_level = F_6;
	                } else {
	                    index = 0;
	                    f_level = F_S;
	                }
		break;


		case F_6:
			if(data == '6'){
	                    rec_data[index++] = data;
	                    f_level = F_0;
	                } else {
	                    index = 0;
	                    f_level = F_S;
	                }
		break;


		case F_0:
			if(data == '0'){
	                    rec_data[index++] = data;
	                    f_level = F_ID;
	                } else {
	                    index = 0;
	                    f_level = F_S;
	                }
		break;


		case F_ID:
			if(data == 0x01){
	                    rec_data[index++] = data;
	                    f_level = F_LEN;
	                } else {
	                    index = 0;
	                    f_level = F_S;
	                }
				
			
		break;


		case F_LEN:
			if(data <= 15){
	                    rec_data[index++] = data;
				len = data;		
	                    f_level = F_DATA;
	                } else {
	                    index = 0;
	                    f_level = F_S;
	                }
		break;

		case F_DATA:
			if(len--){
			 	rec_data[index++] = data;
			}

			if(len == 0){
				char lenth = rec_data[7];
				char buf_x[6] = {0};
				char buf_y[6] = {0};
				char *pData = &rec_data[8];
				int i = 0,j=0;
				//printk("REC:%s\r\n",rec_data);
				while(lenth--){
					if(*pData != ',')
					{
						switch(j)
						{
							case 0: 
							
							buf_x[i++] = *pData;
							break;

							case 1:
							buf_y[i++] = *pData;
							break;

							case 2:
								pen_down = *pData - '0';
							break;
							default:break;
						}
							
					}
					else
					{
						j++;
						i =0;
					}
						pData++;
				}

					x = atoi(buf_x);
					y = atoi(buf_y);
					x = (x*1280) /1024;
					y = (y*720) /600;
					printk("x=%d,y=%d,z=%d\n",x,y,pen_down);
					 index = 0;
	                   		 f_level = F_S;
					 res = 2;
				}

				
		break;

		default:
			index = 0;
                    f_level = F_S;
		break;
	}


#endif 
	return res;
}

/*
 * irkbd_interrupt() is called by the low level driver when a character
 * is received.
 */

static irqreturn_t irkbd_interrupt(struct serio *serio,
		unsigned char data, unsigned int flags)
{
	struct svev *sv = serio_get_drvdata(serio);
	int parse = -1;
#if 0
	spin_lock(&sv->vlock);
	parse = parse_ircode_proto(sv, data);
	if(!parse) {
		sv->kc = get_keycode(sv->key);
		if(sv->kc > 0) {
			sv->pressed = true;
			input_report_key(sv->dev, sv->kc, 1);
			input_sync(sv->dev);
		}
	}
	spin_unlock(&sv->vlock);
#else
	spin_lock(&sv->vlock);
	parse = parse_ircode_proto(sv, data);

	if(parse == 2){
		if((pen_down == 1) ||(pen_down == 3)){
		input_report_abs(sv->dev, ABS_X, x);
		input_report_abs(sv->dev, ABS_Y, y);
		input_report_abs(sv->dev, ABS_PRESSURE, pen_down);
		input_report_key(sv->dev, BTN_TOUCH, 1);
		}
		else{
			input_report_abs(sv->dev, ABS_PRESSURE, 0);
			input_report_key(sv->dev, BTN_TOUCH, 0);
		}
		input_sync(sv->dev);

	}
	spin_unlock(&sv->vlock);
#endif
	return IRQ_HANDLED;
}
#if 0
static irqreturn_t iract_handler(int irq, void *dev_id)
{
	struct svev *sv = (struct svev*)dev_id;
	if(sv->pressed) {
		del_timer(&sv->timer);
		sv->pressed = false;
		input_report_key(sv->dev, sv->kc, 0);
		input_sync(sv->dev);
	}
	
	return IRQ_HANDLED;
}
#endif
static void timer_handler(unsigned long arg)
{
#if 0
	struct svev *sv = (struct svev*)arg;
	unsigned long flags;
	if(!sv->pressed) {
		spin_lock_irqsave(&sv->vlock, flags);
		sv->ss = 0;
		sv->key = 0;
		spin_unlock_irqrestore(&sv->vlock, flags);
	}
#endif
}

/*
 * irkbd_connect() probes for a Sun keyboard and fills the necessary
 * structures.
 */

static int irkbd_connect(struct serio *serio, struct serio_driver *drv)
{
	// struct svev *sv = container_of(serio, struct svev *, serio);
	struct svev *sv = (struct svev *)serio->port_data;
	struct input_dev *input_dev;
	int err = -ENOMEM;
	int i;
	printk(" irkbd_connect>>>>>>>>>>>>>>\n");
	input_dev = input_allocate_device();
	if (!input_dev)
		goto fail1;

	sv->dev = input_dev;
	snprintf(sv->phys, sizeof(sv->phys), "%s/input0", serio->phys);

	serio_set_drvdata(serio, sv);

	err = serio_open(serio, drv);
	if (err)
		goto fail2;

	snprintf(sv->name, sizeof(sv->name), "IR virtual keyboard");
	memcpy(sv->keycode, irkbd_keymap, sizeof(irkbd_keymap));

	input_dev->name = sv->name;
	input_dev->phys = sv->phys;
	input_dev->id.bustype = BUS_RS232;
	input_dev->id.vendor  = SERIO_UNKNOWN;
	input_dev->id.product = SERIO_UNKNOWN;
	input_dev->id.version = 0x0100;
	input_dev->dev.parent = &serio->dev;

	
	

	
	input_set_drvdata(input_dev, sv);
#if 0
	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);

	input_dev->keycode = sv->keycode;
	input_dev->keycodesize = sizeof(int);
	input_dev->keycodemax = ARRAY_SIZE(irkbd_keymap);
	for (i = 0; i < ARRAY_SIZE(irkbd_keymap); i++)
		__set_bit(irkbd_keymap[i], input_dev->keybit);
	__clear_bit(KEY_RESERVED, input_dev->keybit);

#else
	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(ABS_X, input_dev->absbit);
	__set_bit(ABS_Y, input_dev->absbit);
	__set_bit(ABS_PRESSURE, input_dev->absbit);
	__set_bit(EV_SYN, input_dev->evbit);
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(BTN_TOUCH, input_dev->keybit);

	input_set_abs_params(input_dev, ABS_X, 0, 1280, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, 720, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, 720,0, 0);	
	printk("set params>>>>>>>>>>>>>>\n");
#endif
	err = input_register_device(sv->dev);
	if (err)
		goto fail3;

	init_timer(&sv->timer);
	sv->timer.function = &timer_handler;
	sv->timer.data = (unsigned long)sv;
	
	input_dev->rep[REP_DELAY] = 400;
	input_dev->rep[REP_PERIOD] = 200;

	return 0;

 fail3:	serio_close(serio);
 fail2:	serio_set_drvdata(serio, NULL);
 fail1:	input_free_device(input_dev);
	return err;
}

/*
 * irkbd_disconnect() unregisters and closes behind us.
 */

static void irkbd_disconnect(struct serio *serio)
{
	struct svev *sv = serio_get_drvdata(serio);
	del_timer_sync(&sv->timer);
	input_unregister_device(sv->dev);
	serio_close(serio);
	serio_set_drvdata(serio, NULL);
}

static struct serio_device_id irkbd_serio_ids[] = {
	{
		.type	= SERIO_RS232,
		.proto	= SERIO_UNKNOWN,
		.id	= 'I',
		.extra	= 'R',
	},
	{ 0 }
};

MODULE_DEVICE_TABLE(serio, irkbd_serio_ids);

static struct serio_driver irkbd_drv = {
	.driver		= {
		.name	= "irkbd",
	},
	.description	= DRIVER_DESC,
	.id_table	= irkbd_serio_ids,
	.interrupt	= irkbd_interrupt,
	.connect	= irkbd_connect,
	.disconnect	= irkbd_disconnect,
};

static void svev_set_type(struct tty_struct *tty)
{
	struct svev *svev = tty->disc_data;
	svev->id.proto = SERIO_UNKNOWN;
	svev->id.id    = 'I';
	svev->id.extra = 'R';
}

/*
 * Callback functions from the serio code.
 */

static int svev_serio_write(struct serio *serio, unsigned char data)
{
	struct svev *sv = serio->port_data;
	return -(sv->tty->ops->write(sv->tty, &data, 1) != 1);
}

static int svev_serio_open(struct serio *serio)
{
	struct svev *sv = serio->port_data;
	unsigned long flags;

	spin_lock_irqsave(&sv->lock, flags);
	set_bit(SVEV_ACTIVE, &sv->flags);
	spin_unlock_irqrestore(&sv->lock, flags);

	return 0;
}

static void svev_serio_close(struct serio *serio)
{
	struct svev *sv = serio->port_data;
	unsigned long flags;

	spin_lock_irqsave(&sv->lock, flags);
	clear_bit(SVEV_ACTIVE, &sv->flags);
	set_bit(SVEV_DEAD, &sv->flags);
	spin_unlock_irqrestore(&sv->lock, flags);
}

/*
 * svev_ldisc_open() is the routine that is called upon setting our line
 * discipline on a tty. It prepares the serio struct.
 */

static int svev_ldisc_open(struct tty_struct *tty)
{
	struct svev *sv;
	struct serio *serio;
	char name[64];
	int ret = 0;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	sv = kzalloc(sizeof(struct svev), GFP_KERNEL);
	if (!sv)
		return -ENOMEM;

	sv->tty = tty;
	spin_lock_init(&sv->lock);
	spin_lock_init(&sv->vlock);
	init_waitqueue_head(&sv->wait);

	tty->disc_data = sv;
	tty->receive_room = 256;
	set_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);

	svev_set_type(tty);

	if (test_and_set_bit(SVEV_BUSY, &sv->flags)) {
		ret = -EBUSY;
		goto err1;
	}
	
	sv->serio = serio = kzalloc(sizeof(struct serio), GFP_KERNEL);
	if (!serio) {
		ret = -ENOMEM;
		goto err1;
	}

	strlcpy(serio->name, "Serial port", sizeof(serio->name));
	snprintf(serio->phys, sizeof(serio->phys), "%s/serio0", tty_name(tty, name));
	serio->id = sv->id;
	serio->id.type = SERIO_RS232;
	serio->write = svev_serio_write;
	serio->open = svev_serio_open;
	serio->close = svev_serio_close;
	serio->port_data = sv;
	serio->dev.parent = tty->dev;

	serio_register_port(sv->serio);
	printk(KERN_INFO "serio: Serial port %s\n", tty_name(tty, name));
	
	sv->pressed = false;
	//sv->act_gpio = (3 - 1) * 32 + 26;	// gpio3_26
	//gpio_request(sv->act_gpio, "IR_ACT");
	//gpio_direction_input(sv->act_gpio);
	//sv->irq = gpio_to_irq(sv->act_gpio);
	//ret = request_irq(sv->irq, iract_handler, IRQF_TRIGGER_RISING, "IR_IRQ", sv);
	//if(ret) {
	//	printk(KERN_ERR "Request ir irq(%d) error : %d\n", sv->irq, ret);
	//}
	
	return 0;
err1:
	serio_unregister_port(sv->serio);
	sv->serio = NULL;
	clear_bit(SVEV_DEAD, &sv->flags);
	clear_bit(SVEV_BUSY, &sv->flags);
	kfree(sv);
	return ret;
}

/*
 * svev_ldisc_close() is the opposite of svev_ldisc_open()
 */

static void svev_ldisc_close(struct tty_struct *tty)
{
	struct svev *sv = (struct svev *) tty->disc_data;
	serio_unregister_port(sv->serio);
	free_irq(sv->irq, sv);
	sv->serio = NULL;
	clear_bit(SVEV_DEAD, &sv->flags);
	clear_bit(SVEV_BUSY, &sv->flags);
	kfree(sv);
}

/*
 * svev_ldisc_ioctl() allows to get and set some useful data
 */

static int svev_ldisc_ioctl(struct tty_struct *tty, struct file *file,
			       unsigned int cmd, unsigned long arg)
{
	if (cmd == SPIOCSTYPE) {
		svev_set_type(tty);
		return 0;
	}

	return -EINVAL;
}

#ifdef CONFIG_COMPAT
#define COMPAT_SPIOCSTYPE	_IOW('q', 0x01, compat_ulong_t)
static long svev_ldisc_compat_ioctl(struct tty_struct *tty,
				       struct file *file,
				       unsigned int cmd, unsigned long arg)
{
	if (cmd == COMPAT_SPIOCSTYPE) {
		svev_set_type(tty);
		return 0;
	}
	return -EINVAL;
}
#endif

/*
 * svev_ldisc_receive() is called by the low level tty driver when characters
 * are ready for us. We forward the characters and flags, one by one to the
 * 'interrupt' routine.
 */

static void svev_ldisc_receive(struct tty_struct *tty, const unsigned char *cp, char *fp, int count)
{
	struct svev *sv = (struct svev*) tty->disc_data;
	unsigned long flags;
	unsigned int ch_flags = 0;
	int i;

	spin_lock_irqsave(&sv->lock, flags);

	if (!test_bit(SVEV_ACTIVE, &sv->flags))
		goto out;

	for (i = 0; i < count; i++) {
		if (fp) {
			switch (fp[i]) {
			case TTY_FRAME:
				ch_flags = SERIO_FRAME;
				break;

			case TTY_PARITY:
				ch_flags = SERIO_PARITY;
				break;

			default:
				ch_flags = 0;
				break;
			}
		}

		serio_interrupt(sv->serio, cp[i], ch_flags);
	}

out:
	spin_unlock_irqrestore(&sv->lock, flags);
}

/*
 * svev_ldisc_read() just waits indefinitely if everything goes well.
 * However, when the serio driver closes the serio port, it finishes,
 * returning 0 characters.
 */

static ssize_t svev_ldisc_read(struct tty_struct * tty, struct file * file, unsigned char __user * buf, size_t nr)
{
	struct svev *sv = (struct svev*) tty->disc_data;
	if(nr <= 0) return -ENOMEM;
	wait_event_interruptible(sv->wait, test_bit(SVEV_DEAD, &sv->flags));
	if(signal_pending(current)) return -EINTR;
	return 0;
}

static ssize_t svev_ldisc_write(struct tty_struct *tty, struct file *file, const unsigned char *buf, size_t nr)
{
	return -EINVAL;
}

static unsigned int svev_ldisc_poll(struct tty_struct *tty, struct file *filp, struct poll_table_struct *wait)
{
	return 0;
}

static void svev_ldisc_write_wakeup(struct tty_struct * tty)
{
	struct svev *sv = (struct svev *) tty->disc_data;
	unsigned long flags;

	spin_lock_irqsave(&sv->lock, flags);
	if (test_bit(SVEV_ACTIVE, &sv->flags))
		serio_drv_write_wakeup(sv->serio);
	spin_unlock_irqrestore(&sv->lock, flags);
}

/*
 * The line discipline structure.
 */

static struct tty_ldisc_ops svev_ldisc = {
	.owner =	THIS_MODULE,
	.name =		"input",
	.open =		svev_ldisc_open,
	.close =	svev_ldisc_close,
	.read =		svev_ldisc_read,
	.write =	svev_ldisc_write,
	.poll =		svev_ldisc_poll,
	.ioctl =	svev_ldisc_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl =	svev_ldisc_compat_ioctl,
#endif
	.receive_buf =	svev_ldisc_receive,
	.write_wakeup =	svev_ldisc_write_wakeup
};

/*
 * The functions for insering/removing us as a module.
 */

static int __init svev_init(void)
{
	int retval;
	retval = tty_register_ldisc(N_IRKBD, &svev_ldisc);
	if (retval) {
		printk(KERN_ERR "Error registering line discipline.\n");
		return retval;
	}

	retval = serio_register_driver(&irkbd_drv);
	if (retval) {
		printk(KERN_ERR "Error registering serio input.\n");
		goto err;
	}
	return  retval;
err:
	tty_unregister_ldisc(N_IRKBD);
	return  retval;
}

static void __exit svev_exit(void)
{
	serio_unregister_driver(&irkbd_drv);
	tty_unregister_ldisc(N_IRKBD);
}

module_init(svev_init);
module_exit(svev_exit);

MODULE_AUTHOR("Lenver Shan <lenver520@163.com>");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
MODULE_ALIAS_LDISC(N_IRKBD);
