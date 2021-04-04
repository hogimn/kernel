//
// 
//  SM5S4210 Board : SM5S4210 Keypad Interface driver 
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

#include <mach/regs-gpio.h>
#include <mach/regs-clock.h>

#include <asm/gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-keypad.h>

#include "sm5s4210_keypad.h"

//   function prototype define
int	sm5s4210_keypad_sysfs_create(struct platform_device *pdev);
void	sm5s4210_keypad_sysfs_remove(struct platform_device *pdev);	
		
//
//   sysfs function prototype define
//
// 	force system power off sysfs
static	ssize_t show_system_off(struct device *dev, struct device_attribute *attr, char *buf);
static	ssize_t set_system_off(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static	DEVICE_ATTR(system_off, S_IRWXUGO, show_system_off, set_system_off);

//   touch sampling rate control (5, 10, 20 : unit msec)
static	ssize_t show_sampling_rate(struct device *dev, struct device_attribute *attr, char *buf);
static	ssize_t set_sampling_rate(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static	DEVICE_ATTR(sampling_rate, S_IRWXUGO, show_sampling_rate, set_sampling_rate);

//   long power off control (set to 1 than touch controller reset)
static	ssize_t show_poweroff_control(struct device *dev, struct device_attribute *attr, char *buf);
static 	ssize_t set_poweroff_control(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static	DEVICE_ATTR(poweroff_control, S_IRWXUGO, show_poweroff_control, set_poweroff_control);

static struct attribute *sm5s4210_keypad_sysfs_entries[] = {
	&dev_attr_system_off.attr,
	&dev_attr_sampling_rate.attr,
	&dev_attr_poweroff_control.attr,

	NULL
};

static struct attribute_group sm5s4210_keypad_attr_group = {
	.name   = NULL,
	.attrs  = sm5s4210_keypad_sysfs_entries,
};

static 	ssize_t show_system_off(struct device *dev, struct device_attribute *attr, char *buf)
{
	return	sprintf(buf, "%s : unsupport this function!\n", __FUNCTION__);
}

static 	ssize_t set_system_off(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    unsigned long flags;

    local_irq_save(flags);

    if(!strcmp(buf, "on\n")){
		// PS_HOLD Enable (LOW-> HIGH)
	printk(KERN_EMERG "%s : setting GPIO_PDA_PS_HOLD low.\n", __func__);
	(*(unsigned long *)(S5PV310_VA_PMU + 0x330C)) = 0x5200;
    }

    local_irq_restore(flags);

    return count;
}

static 	ssize_t show_sampling_rate(struct device *dev, struct device_attribute *attr, char *buf)
{
	switch(sm5s4210_keypad.sampling_rate)	{
		default	:
			sm5s4210_keypad.sampling_rate = 0;
		case	0:
			return	sprintf(buf, "10 msec\n");
		case	1:
			return	sprintf(buf, "20 msec\n");
		case	2:
			return	sprintf(buf, "50 msec\n");
	}
}

static 	ssize_t set_sampling_rate(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    unsigned long 	flags;
    unsigned int	val;

    if(!(sscanf(buf, "%u\n", &val))) 	return	-EINVAL;
    
    local_irq_save(flags);
    
    if(val > 20)	sm5s4210_keypad.sampling_rate = 2;
    else if(val > 10)	sm5s4210_keypad.sampling_rate = 1;
    else		sm5s4210_keypad.sampling_rate = 0;

    local_irq_restore(flags);

    return count;
}

static 	ssize_t show_poweroff_control	(struct device *dev, struct device_attribute *attr, char *buf)
{
	switch(sm5s4210_keypad.poweroff_time)	{
		default	:
			sm5s4210_keypad.poweroff_time = 0;
		case	0:
			return	sprintf(buf, "1 sec\n");
		case	1:
			return	sprintf(buf, "3 sec\n");
		case	2:
			return	sprintf(buf, "5 sec\n");
	}
}

static 	ssize_t set_poweroff_control(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    unsigned long 	flags;
    unsigned int	val;

    if(!(sscanf(buf, "%u\n", &val)))	return	-EINVAL;

    local_irq_save(flags);
    
    if(val > 3)	sm5s4210_keypad.sampling_rate = 2;
    else if(val > 1)	sm5s4210_keypad.sampling_rate = 1;
    else		sm5s4210_keypad.sampling_rate = 0;

    local_irq_restore(flags);

    return count;
}

int  sm5s4210_keypad_sysfs_create(struct platform_device *pdev)	
{
	// variable init
	sm5s4210_keypad.sampling_rate	= 0;	// 10 msec sampling
	sm5s4210_keypad.poweroff_time	= 1;	// 3 sec

	return sysfs_create_group(&pdev->dev.kobj, &sm5s4210_keypad_attr_group);
}

void	sm5s4210_keypad_sysfs_remove(struct platform_device *pdev)	
{
    sysfs_remove_group(&pdev->dev.kobj, &sm5s4210_keypad_attr_group);
}

