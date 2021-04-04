/*
 *  max17040_battery.c
 *  fuel-gauge systems for lithium-ion (Li+) batteries
 *
 *  Copyright (C) 2009 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/power_supply.h>
#include <linux/max17040_battery.h>
#include <linux/slab.h>

#include <asm/mach/irq.h>
#include <asm/io.h>
#include <mach/gpio.h>
#include <mach/regs-gpio.h>

#include <plat/gpio-cfg.h>


#define MAX17040_VCELL_MSB	0x02
#define MAX17040_VCELL_LSB	0x03
#define MAX17040_SOC_MSB	0x04
#define MAX17040_SOC_LSB	0x05
#define MAX17040_MODE_MSB	0x06
#define MAX17040_MODE_LSB	0x07
#define MAX17040_VER_MSB	0x08
#define MAX17040_VER_LSB	0x09
#define MAX17040_RCOMP_MSB	0x0C
#define MAX17040_RCOMP_LSB	0x0D
#define MAX17040_CMD_MSB	0xFE
#define MAX17040_CMD_LSB	0xFF

#define MAX17040_DELAY		msecs_to_jiffies(1000)
#define MAX17040_BATTERY_FULL	99

struct max17040_chip {
	struct i2c_client		*client;
	struct delayed_work             work;
	struct power_supply		battery;
#if defined(CONFIG_BOARD_SM5S4210)
	struct power_supply		ac;
#endif
	struct max17040_platform_data	*pdata;
	//struct timespec			next_update_time;

	/* State Of Connect */
	int online;
	/* battery voltage */
	int vcell;
	/* battery capacity */
	int soc;
	/* State Of Charge */
	int status;
};


//static void max17040_update_values(struct max17040_chip *chip);

static int max17040_get_property(struct power_supply *psy,
			    enum power_supply_property psp,
			    union power_supply_propval *val)
{
	struct max17040_chip *chip = container_of(psy,
				struct max17040_chip, battery);


	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = chip->status;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = chip->online;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = chip->vcell;
		if(psp  == POWER_SUPPLY_PROP_PRESENT)
			val->intval = 1; /* You must never run Odrioid1 without Battery. */
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = chip->soc;
		break;
	 case POWER_SUPPLY_PROP_TECHNOLOGY:
		//val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		val->intval = POWER_SUPPLY_TECHNOLOGY_LIPO;
		break;
	 case POWER_SUPPLY_PROP_HEALTH:
//	 	if(chip->vcell  < 2850)
	 	if(chip->vcell  < 3500000)
			val->intval = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;
		else
		 	 val->intval = POWER_SUPPLY_HEALTH_GOOD;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = 365;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

#if defined(CONFIG_BOARD_SM5S4210)
static int adapter_get_property(struct power_supply *psy,
			enum power_supply_property psp,
			union power_supply_propval *val)
{
	struct max17040_chip *chip = container_of(psy,
				struct max17040_chip, ac);
	int ret = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_PRESENT:
	case POWER_SUPPLY_PROP_ONLINE:
		//TODO:
		if (chip->pdata->charger_online)
			val->intval = chip->pdata->charger_online();
		else val->intval = 0;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}
#endif

static int max17040_write_reg(struct i2c_client *client, int reg, u8 value)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client, reg, value);

	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static int max17040_read_reg(struct i2c_client *client, int reg)
{
	int ret;

	ret = i2c_smbus_read_byte_data(client, reg);

	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static void max17040_get_vcell(struct i2c_client *client)
{
	struct max17040_chip *chip = i2c_get_clientdata(client);
	u8 msb;
	u8 lsb;

	msb = max17040_read_reg(client, MAX17040_VCELL_MSB);
	lsb = max17040_read_reg(client, MAX17040_VCELL_LSB);

	chip->vcell = (msb << 4) + (lsb >> 4);
	chip->vcell = (chip->vcell * 125) * 10;
}

static void max17040_get_soc(struct i2c_client *client)
{
	struct max17040_chip *chip = i2c_get_clientdata(client);
#if 0 //JNJ-org
	u8 msb;
	u8 lsb;

	msb = max17040_read_reg(client, MAX17040_SOC_MSB);
	lsb = max17040_read_reg(client, MAX17040_SOC_LSB);

	//chip->soc = (msb > 100) ? 100 : msb;
	chip->soc = min(msb, (u8)100);
#else
	u8 msb;

	msb = max17040_read_reg(client, MAX17040_SOC_MSB);
	chip->soc = min(msb, (u8)100);
	if(chip->soc == 100) return;

	if(chip->vcell > 4000000) chip->soc = 99;
	else if(chip->vcell < 3550000) chip->soc = 0;
	else {
		chip->soc = (chip->vcell - 3550000)*100/450000;
		chip->soc = min(chip->soc, (u8)99);
	}
//printk("MAX17040 : chip->vcell = %d,soc=%d \r\n",chip->vcell,chip->soc);
#endif
}

static void max17040_get_version(struct i2c_client *client)
{
	u8 msb;
	u8 lsb;

	msb = max17040_read_reg(client, MAX17040_VER_MSB);
	lsb = max17040_read_reg(client, MAX17040_VER_LSB);

	dev_info(&client->dev, "MAX17040 Fuel-Gauge Ver %d%d\n", msb, lsb);
}

static void max17040_get_online(struct i2c_client *client)
{
	struct max17040_chip *chip = i2c_get_clientdata(client);

//JNJ
#if defined(CONFIG_BOARD_SM5S4210) 
	if (chip->pdata->charger_online)
		chip->online = chip->pdata->charger_online() && !chip->pdata->adapter_online();
	else
		chip->online = 0;
#else
	if (chip->pdata && chip->pdata->battery_online)
		chip->online = chip->pdata->battery_online();
	else
		chip->online = 1;
#endif
}

static void max17040_get_status(struct i2c_client *client)
{
	struct max17040_chip *chip = i2c_get_clientdata(client);

	if (!chip->pdata || !chip->pdata->charger_online ||
		!chip->pdata->battery_online) {
		chip->status = POWER_SUPPLY_STATUS_UNKNOWN;
		return;
	}

#if defined(CONFIG_BOARD_SM5S4210)
	if(chip->pdata->adapter_online()) {
		chip->status = POWER_SUPPLY_STATUS_FULL;
		chip->vcell = 4200000;
		chip->soc = 100;
	} else {
#endif
	if (chip->pdata->charger_online()) {
//		if (chip->soc > MAX17040_BATTERY_FULL)
		if(chip->pdata->charger_done()){
			chip->status = POWER_SUPPLY_STATUS_FULL;
			chip->soc = 100;
		} else {
			if(chip->soc == 100) chip->soc = 99;
			chip->status = POWER_SUPPLY_STATUS_CHARGING;
		}
	} else if(chip->pdata->battery_online()){ // battery_online()
		if (chip->soc > MAX17040_BATTERY_FULL) {
			chip->status = POWER_SUPPLY_STATUS_FULL;
			chip->soc = 100;
		} else {
#if 0
			if (chip->pdata->charger_enable()) 
				chip->status = POWER_SUPPLY_STATUS_CHARGING;
			else 
				chip->status = POWER_SUPPLY_STATUS_DISCHARGING;
#else
			chip->status = POWER_SUPPLY_STATUS_DISCHARGING;
#endif
		}
	}
	else { // Base Board power
		chip->status = POWER_SUPPLY_STATUS_FULL;
		chip->vcell = 4200000;
		chip->soc = 100;
	}
#if defined(CONFIG_BOARD_SM5S4210)
	}
#endif
//printk("MAX17040 : status = %d\r\n",chip->status);

#if 0
	if (chip->pdata->charger_online()) {
		if (chip->pdata->charger_enable())
			chip->status = POWER_SUPPLY_STATUS_CHARGING;
		else
			chip->status = POWER_SUPPLY_STATUS_NOT_CHARGING;
	} else {
		chip->status = POWER_SUPPLY_STATUS_DISCHARGING;
	}

	if (chip->soc > MAX17040_BATTERY_FULL)
		chip->status = POWER_SUPPLY_STATUS_FULL;
#endif
}

static void max17040_work(struct work_struct *work)
{
	struct max17040_chip *chip;
	int old_online, old_vcell, old_soc;

	chip = container_of(work, struct max17040_chip, work.work);

	old_online = chip->online;
	old_vcell = chip->vcell;
	old_soc = chip->soc;
	max17040_get_online(chip->client);
#if defined(CONFIG_BOARD_SM5S4210)
	if(!chip->pdata->adapter_online() && chip->pdata->battery_online())
	{
#endif
	max17040_get_vcell(chip->client);
	max17040_get_soc(chip->client);
#if defined(CONFIG_BOARD_SM5S4210)
	}
#endif
	max17040_get_status(chip->client);

//printk("MAX17040 : vcell=%d, soc=%d\r\n",chip->vcell, chip->soc);
	if((old_vcell != chip->vcell) || (old_soc != chip->soc))
	{
		power_supply_changed(&chip->battery);
//		printk("MAX17040 Fuel-Gauge vcell:%d soc:%d\n", chip->vcell, chip->soc);
	}
	if(old_online != chip->online){
		power_supply_changed(&chip->ac);
	}
	schedule_delayed_work(&chip->work, MAX17040_DELAY);
}


static enum power_supply_property max17040_battery_props[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_TEMP,
};

static enum power_supply_property adapter_get_props[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
};

static int __devinit max17040_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct max17040_chip *chip;
	int ret;

printk("MAX17040 probe + \r\n");
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
		return -EIO;

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->client = client;
	chip->pdata = client->dev.platform_data;

	i2c_set_clientdata(client, chip);

//	chip->battery.name		= "battery";
	chip->battery.name		= "Battery";
	chip->battery.type		= POWER_SUPPLY_TYPE_BATTERY;
	chip->battery.get_property	= max17040_get_property;
	chip->battery.properties	= max17040_battery_props;
	chip->battery.num_properties	= ARRAY_SIZE(max17040_battery_props);

#if defined(CONFIG_BOARD_SM5S4210)
//	chip->ac.name		= "ac";
	chip->ac.name		= "Mains";
	chip->ac.type		= POWER_SUPPLY_TYPE_MAINS;
	chip->ac.get_property	= adapter_get_property;
	chip->ac.properties	= adapter_get_props;
	chip->ac.num_properties	= ARRAY_SIZE(adapter_get_props);
	chip->ac.external_power_changed = NULL;
#endif

	//max17040_update_values(chip);

	if (chip->pdata && chip->pdata->power_supply_register)
		ret = chip->pdata->power_supply_register(&client->dev, &chip->battery);
	else
		ret = power_supply_register(&client->dev, &chip->battery);
	if (ret) {
		dev_err(&client->dev, "failed: battery, power supply register\n");
		kfree(chip);
		return ret;
	}

#if defined(CONFIG_BOARD_SM5S4210)
	ret = power_supply_register(&client->dev, &chip->ac);
	if(ret) {
		dev_err(&client->dev, "failed: ac, power supply register\n");
		kfree(chip);
		return ret;
	}
#endif

#if defined(CONFIG_BOARD_SM5S4210)
	if(!chip->pdata->adapter_online() && chip->pdata->battery_online())
#endif
	max17040_get_version(client);

	INIT_DELAYED_WORK_DEFERRABLE(&chip->work, max17040_work);
	schedule_delayed_work(&chip->work, MAX17040_DELAY);

#if defined(CONFIG_BOARD_SM5S4210)
	if(!chip->pdata->adapter_online() && chip->pdata->battery_online())
#endif
	if (chip->pdata)
		i2c_smbus_write_word_data(client, MAX17040_RCOMP_MSB,
			swab16(chip->pdata->rcomp_value));

printk("MAX17040 probed.. \r\n");
	return 0;
}

static int __devexit max17040_remove(struct i2c_client *client)
{
	struct max17040_chip *chip = i2c_get_clientdata(client);

	if (chip->pdata && chip->pdata->power_supply_unregister)
		chip->pdata->power_supply_unregister(&chip->battery);
	else
		power_supply_unregister(&chip->battery);

#if defined(CONFIG_BOARD_SM5S4210)
	power_supply_unregister(&chip->ac);
#endif
	kfree(chip);
	return 0;
}

#ifdef CONFIG_PM

static int max17040_suspend(struct i2c_client *client,
		pm_message_t state)
{
	struct max17040_chip *chip = i2c_get_clientdata(client);

#if defined(CONFIG_BOARD_SM5S4210)
	if(!chip->pdata->adapter_online() && chip->pdata->battery_online())
#endif
	cancel_delayed_work(&chip->work);
	return 0;
}

static int max17040_resume(struct i2c_client *client)
{
	struct max17040_chip *chip = i2c_get_clientdata(client);

#if defined(CONFIG_BOARD_SM5S4210)
	if(!chip->pdata->adapter_online() && chip->pdata->battery_online())
#endif
	schedule_delayed_work(&chip->work, MAX17040_DELAY);
	return 0;
}

#else

#define max17040_suspend NULL
#define max17040_resume NULL

#endif /* CONFIG_PM */


static const struct i2c_device_id max17040_id[] = {
	{ "max17040", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, max17040_id);

static struct i2c_driver max17040_i2c_driver = {
	.driver	= {
		.name	= "max17040",
	},
	.probe		= max17040_probe,
	.remove		= __devexit_p(max17040_remove),
	.suspend	= max17040_suspend,
	.resume		= max17040_resume,
	.id_table	= max17040_id,
};

static int __init max17040_init(void)
{
	return i2c_add_driver(&max17040_i2c_driver);
}
module_init(max17040_init);

static void __exit max17040_exit(void)
{
	i2c_del_driver(&max17040_i2c_driver);
}
module_exit(max17040_exit);

MODULE_AUTHOR("Minkyu Kang <mk7.kang@samsung.com>");
MODULE_DESCRIPTION("MAX17040 Fuel Gauge");
MODULE_LICENSE("GPL");
