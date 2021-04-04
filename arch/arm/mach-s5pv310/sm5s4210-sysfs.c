//
//
// 
//  SM5S4210 Board : SM5S4210 sysfs driver 
// 
//
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/sysfs.h>
#include <linux/input.h>
#include <linux/gpio.h>

#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>

#define	DEBUG_PM_MSG

// Sleep disable flage
#define	SLEEP_DISABLE_FLAG

#if defined(SLEEP_DISABLE_FLAG)
	#ifdef CONFIG_HAS_WAKELOCK
		#include <linux/wakelock.h>
		static struct wake_lock 	sleep_wake_lock;
	#endif
#endif

//   Control GPIO Define
// GPIO Index Define
enum	{
	// Power Control
//	SYSTEM_POWER_3V3,		// BUCK6 Enable Control
	SYSTEM_POWER_5V0,		// USB HOST Power
	SYSTEM_POWER_12V0,		// VLED Control (Backlight)
	
	GPIO_INDEX_END
};

static struct {
	int		gpio_index;		// Control Index
	int 	gpio;			// GPIO Number
	char	*name;			// GPIO Name == sysfs attr name (must)
	bool 	output;			// 1 = Output, 0 = Input
	int 	value;			// Default Value(only for output)
	int		pud;			// Pull up/down register setting : S3C_GPIO_PULL_DOWN, UP, NONE
} sControlGpios[] = {
	// SYSTEM POWER CONTROL
//	{	SYSTEM_POWER_3V3,		S5PV310_GPX3(5), "power_3v3",			1, 1, S3C_GPIO_PULL_DOWN	},
	{	SYSTEM_POWER_5V0,		S5PV310_GPC0(0), "power_5v0",			1, 1, S3C_GPIO_PULL_DOWN	},
	{	SYSTEM_POWER_12V0,		S5PV310_GPC0(3), "power_12v0",			1, 0, S3C_GPIO_PULL_DOWN	},
};

//   function prototype define
static 	int	sm5s4210_sysfs_resume(struct platform_device *dev);
static 	int	sm5s4210_sysfs_suspend(struct platform_device *dev, pm_message_t state);
static	int	sm5s4210_sysfs_probe(struct platform_device *pdev);
static	int	sm5s4210_sysfs_remove(struct platform_device *pdev);	

static 	int 	__init sm5s4210_sysfs_init(void);
static 	void 	__exit sm5s4210_sysfs_exit(void);

static struct platform_driver sm5s4210_sysfs_driver = {
	.driver = {
		.name = "sm5s4210-sysfs",
		.owner = THIS_MODULE,
	},
	.probe 		= sm5s4210_sysfs_probe,
	.remove 	= sm5s4210_sysfs_remove,
	.suspend	= sm5s4210_sysfs_suspend,
	.resume		= sm5s4210_sysfs_resume,
};

module_init(sm5s4210_sysfs_init);
module_exit(sm5s4210_sysfs_exit);


MODULE_DESCRIPTION("SYSFS driver for sm5s4210-Dev board");
MODULE_AUTHOR("Hanback Electronics");
MODULE_LICENSE("GPL");

extern	bool s5p_hpd_get_status(void);
//
//   sysfs function prototype define
//
static	ssize_t show_gpio(struct device *dev, struct device_attribute *attr, char *buf);
static 	ssize_t set_gpio(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static	ssize_t show_hdmi(struct device *dev, struct device_attribute *attr, char *buf);
static	DEVICE_ATTR(power_3v3,	S_IRWXUGO, show_gpio, set_gpio);
static	DEVICE_ATTR(power_5v0,	S_IRWXUGO, show_gpio, set_gpio);
static	DEVICE_ATTR(power_12v0,	S_IRWXUGO, show_gpio, set_gpio);
static	DEVICE_ATTR(hdmi_state,	S_IRWXUGO, show_hdmi, NULL);
static struct attribute *sm5s4210_sysfs_entries[] = {
	&dev_attr_power_3v3.attr,
	&dev_attr_power_5v0.attr,
	&dev_attr_power_12v0.attr,
	&dev_attr_hdmi_state,
	NULL
};

static struct attribute_group sm5s4210_sysfs_attr_group = {
	.name   = NULL,
	.attrs  = sm5s4210_sysfs_entries,
};

static 	ssize_t show_gpio(struct device *dev, struct device_attribute *attr, char *buf)
{
	int	i;

	for (i = 0; i < ARRAY_SIZE(sControlGpios); i++) {
		if(!strcmp(sControlGpios[i].name, attr->attr.name))
			return	sprintf(buf, "%d\n", (gpio_get_value(sControlGpios[i].gpio) ? 1 : 0));
	}
	
	return	sprintf(buf, "ERROR! : Not found gpio!\n");
}

static 	ssize_t set_gpio(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    unsigned int	val, i;

    if(!(sscanf(buf, "%d\n", &val))) 	return	-EINVAL;

	for (i = 0; i < ARRAY_SIZE(sControlGpios); i++) {
		if(!strcmp(sControlGpios[i].name, attr->attr.name))	{
			if(sControlGpios[i].output)
				gpio_set_value(sControlGpios[i].gpio, ((val != 0) ? 1 : 0));
			else
				printk("This GPIO Configuration is INPUT!!\n");
		    return count;
		}
	}

	printk("ERROR! : Not found gpio!\n");
    return 0;
}

static	ssize_t show_hdmi(struct device *dev, struct device_attribute *attr, char *buf)
{
	int status = s5p_hpd_get_status();
	
	if(status)	return	sprintf(buf, "%s\n", "on");
	else		return	sprintf(buf, "%s\n", "off");
}

void 	SYSTEM_POWER_CONTROL	(int power, int val)
{
	int	index;
	
	switch(power)	{
//		case	0:	index = SYSTEM_POWER_3V3;		break;
		case	1:	index = SYSTEM_POWER_5V0;		break;
		case	2:	index = SYSTEM_POWER_12V0;		break;
		default	:									return;
	}
	
	gpio_set_value(sControlGpios[index].gpio, ((val != 0) ? 1 : 0));
}

EXPORT_SYMBOL(SYSTEM_POWER_CONTROL);
static int	sm5s4210_sysfs_resume(struct platform_device *dev)
{
	#if	defined(DEBUG_PM_MSG)
		printk("%s\n", __FUNCTION__);
	#endif

    return  0;
}

static int	sm5s4210_sysfs_suspend(struct platform_device *dev, pm_message_t state)
{
	#if	defined(DEBUG_PM_MSG)
		printk("%s\n", __FUNCTION__);
	#endif
	
    return  0;
}

static	int sm5s4210_sysfs_probe(struct platform_device *pdev)	
{
	int	i;

	// Control GPIO Init
	for (i = 0; i < ARRAY_SIZE(sControlGpios); i++) {
		if(gpio_request(sControlGpios[i].gpio, sControlGpios[i].name))	{
			printk("%s : %s gpio reqest err!\n", __FUNCTION__, sControlGpios[i].name);
		}
		else	{
			if(sControlGpios[i].output)		gpio_direction_output	(sControlGpios[i].gpio, sControlGpios[i].value);
			else							gpio_direction_input	(sControlGpios[i].gpio);

			s3c_gpio_setpull		(sControlGpios[i].gpio, sControlGpios[i].pud);
		}
	}

#if defined(SLEEP_DISABLE_FLAG)
	#ifdef CONFIG_HAS_WAKELOCK
		wake_lock(&sleep_wake_lock);
	#endif
#endif

	return	sysfs_create_group(&pdev->dev.kobj, &sm5s4210_sysfs_attr_group);
}

static	int		sm5s4210_sysfs_remove		(struct platform_device *pdev)	
{
	int	i;
	
	for (i = 0; i < ARRAY_SIZE(sControlGpios); i++) 	gpio_free(sControlGpios[i].gpio);

#if defined(SLEEP_DISABLE_FLAG)
	#ifdef CONFIG_HAS_WAKELOCK
		wake_unlock(&sleep_wake_lock);
	#endif
#endif

    sysfs_remove_group(&pdev->dev.kobj, &sm5s4210_sysfs_attr_group);
    
    return	0;
}
//[*]--------------------------------------------------------------------------------------------------[*]
static int __init sm5s4210_sysfs_init(void)
{	
#if defined(SLEEP_DISABLE_FLAG)
#ifdef CONFIG_HAS_WAKELOCK
	printk("--------------------------------------------------------\n");
	printk("\n%s(%d) : Sleep Disable Flag SET!!(Wake_lock_init)\n\n", __FUNCTION__, __LINE__);
	printk("--------------------------------------------------------\n");

    	wake_lock_init(&sleep_wake_lock, WAKE_LOCK_SUSPEND, "sleep_wake_lock");
#endif
#endif
    return platform_driver_register(&sm5s4210_sysfs_driver);
}

static void __exit sm5s4210_sysfs_exit(void)
{
#if defined(SLEEP_DISABLE_FLAG)
	#ifdef CONFIG_HAS_WAKELOCK
	    wake_lock_destroy(&sleep_wake_lock);
	#endif
#endif
    platform_driver_unregister(&sm5s4210_sysfs_driver);
}

