/*
 * linux/regulator/rn5t56x-regulator.h
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

#ifndef __LINUX_REGULATOR_RN5T56x_H
#define __LINUX_REGULATOR_RN5T56x_H

#include <linux/regulator/machine.h>
#include <linux/regulator/driver.h>

#define rn5t56x_rails(_name) "RN5T56x_"#_name

/* RICOH Regulator IDs */
enum regulator_id {
	RN5T56x_ID_DC1,
	RN5T56x_ID_DC2,
	RN5T56x_ID_DC3,
	RN5T56x_ID_DC4,
	RN5T56x_ID_LDO1,
	RN5T56x_ID_LDO2,
	RN5T56x_ID_LDO3,
	RN5T56x_ID_LDO4,
	RN5T56x_ID_LDO5,
	RN5T56x_ID_LDORTC1,
	RN5T56x_ID_LDORTC2,
};

struct rn5t56x_regulator_platform_data {
		struct regulator_init_data regulator;
		int init_uV;
		unsigned init_enable:1;
		unsigned init_apply:1;
		int sleep_uV;
		int sleep_slots;
		unsigned long ext_pwr_req;
		unsigned long flags;
};

#endif
