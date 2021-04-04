/*
 *  Copyright (C) 2009 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __MAX17040_BATTERY_H_
#define __MAX17040_BATTERY_H_
#define GPIO_CHARGER_ONLINE	S5PV310_GPX3(4) // Low
#define GPIO_CHARGER_STATUS	S5PV310_GPX3(3) // Low
#define GPIO_BATTERY_ONLINE	S5PV310_GPX2(6) // High
#define GPIO_M1_ADAPTER_ONLINE	S5PV310_GPX1(4) // High

struct max17040_platform_data {
	int (*battery_online)(void);
	int (*charger_online)(void);
//	int (*charger_enable)(void);
	int (*charger_done)(void);
	int (*adapter_online)(void);
//	void (*charger_disable)(void);
	int (*power_supply_register)(struct device *parent,
		struct power_supply *psy);
	void (*power_supply_unregister)(struct power_supply *psy);
	u16 rcomp_value;
};

#endif
