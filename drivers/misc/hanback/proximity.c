/*
 * Proximity Device Driver
 *  Hanback Electronics Co.,ltd
 * File : proximity.c
 * Date : August,2011
 */ 

#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <asm/fcntl.h>
#include <linux/ioport.h>

#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>

#include <linux/pwm.h>

/* Prototypes */
extern int s3c_adc_get_adc_data(int channel);
#define ADC_CHANNEL  6 /* index for S5PC210 channel adc */

#define PROXIMITY_BASE          0xbc
#define IOCTL_PROXIMITY_READ   _IOR(PROXIMITY_BASE,0,int)

#define PROXIMITY_PWM_CH 2
#define PERIOD_NS 35714        /* 1000000000 / 35714 => 28 Khz */
#define DUTY_NS 1000*18       /* 1000(1 us) * 18 => 18 us */

static struct pwm_device *proximity_pwm;
static unsigned int proximity_period_ns;
static unsigned int proximity_duty_ns;

#define DRIVER_AUTHOR	"hanback"	      
#define DRIVER_DESC	"proximity program" 

#define	PROXIMITY_NAME	"proximity"	

#define TIMER_INTERVAL		  20

//Global variable
static unsigned int proximity_usage = 0;

struct proximity_data {
        atomic_t enable;                /* attribute value */
        atomic_t delay;                /* attribute value */
        int 	distance;  	/* last measured data */
        struct mutex enable_mutex;
        struct mutex data_mutex;
        struct input_dev *input;
        struct delayed_work work;
};

static struct proximity_data *proximity_ptr;

#define PROXIMITYIO                      0x51
#define PROXIMITY_IOCTL_ENABLE             _IO(PROXIMITYIO, 0x31)
#define PROXIMITY_IOCTL_DISABLE            _IO(PROXIMITYIO, 0x32)
#define PROXIMITY_IOCTL_IS_ENABLE          _IOR(PROXIMITYIO, 0x33, int)
#define PROXIMITY_IOCTL_DELAY_GET          _IOR(PROXIMITYIO, 0x34, int)
#define PROXIMITY_IOCTL_DELAY_SET          _IOW(PROXIMITYIO, 0x35, int)
#define PROXIMITY_IOCTL_DATA               _IOR(PROXIMITYIO, 0x36, int)


static int proximity_device_open( struct inode*, struct file* );
static int proximity_device_release( struct inode*, struct file* );
static int proximity_device_ioctl( struct inode*, struct file*, unsigned int, unsigned long );
static ssize_t proximity_device_read( struct file *filp, char *buf, size_t count, loff_t *ofs );
static ssize_t proximity_device_write( struct file *filp, const char *buf, size_t count, loff_t *ofs );
static unsigned int proximity_device_poll( struct file *filp, struct poll_table_struct *pwait );

static struct file_operations proximity_device_fops =
{
        .owner = THIS_MODULE,
        .open = proximity_device_open,
        .ioctl = proximity_device_ioctl,
        .release = proximity_device_release,
        .read = proximity_device_read,
        .write = proximity_device_write,
        .poll = proximity_device_poll,
};

static struct miscdevice proximity_misc =
{
        .minor = MISC_DYNAMIC_MINOR,
        .name = PROXIMITY_NAME,
        .fops = &proximity_device_fops,
};

#define delay_to_jiffies(d) ((d)?msecs_to_jiffies(d):1)
#define actual_delay(d)     (jiffies_to_msecs(delay_to_jiffies(d)))

static void proximity_work_func(struct work_struct *work)
{
        struct proximity_data *proximity = container_of((struct delayed_work *)work, struct proximity_data, work);
        unsigned long delay = delay_to_jiffies(atomic_read(&proximity->delay));
		int	distance;

// read proximity data
		distance = s3c_adc_get_adc_data(ADC_CHANNEL);
// 

		if (distance > 500)
			distance = 0;
		else
			distance = 1;
		
        input_report_abs(proximity->input, ABS_DISTANCE, distance);
        input_sync(proximity->input);

        mutex_lock(&proximity->data_mutex);
        proximity->distance = distance;
        mutex_unlock(&proximity->data_mutex);

        schedule_delayed_work(&proximity->work, delay);
}

/*
 * Input device interface
 */
static int proximity_input_init(struct proximity_data *proximity)
{
        struct input_dev *dev;
        int err;
//		int ret;

        dev = input_allocate_device();
        if (!dev)
                return -ENOMEM;

        dev->name = "proximity";
//JNJ        dev->id.bustype = 0;

        set_bit(EV_ABS, dev->evbit);
        set_bit(EV_SYN, dev->evbit);

        input_set_abs_params(dev, ABS_DISTANCE, 0, 100, 0, 0);

        input_set_drvdata(dev, proximity);

        err = input_register_device(dev);
        if (err < 0) {
                input_free_device(dev);
                return err;
        }
        proximity->input = dev;

#if 0
// add KSJ
		proximity_period_ns = PERIOD_NS;
  		proximity_duty_ns = DUTY_NS;
  		proximity_pwm = pwm_request(PROXIMITY_PWM_CH, "proximity");

  		if (IS_ERR(proximity_pwm)) {
  		  printk("unable to request PWM for proximity\n");
  		  ret = PTR_ERR(proximity_pwm);
  		}
		//printk("got pwm for proximity\n");

 		 pwm_config(proximity_pwm, proximity_duty_ns, proximity_period_ns);
 		 pwm_enable(proximity_pwm);
// ~add KSJ
#endif
        return 0;
}

static void proximity_input_fini(struct proximity_data *proximity)
{
        struct input_dev *dev = proximity->input;

        input_unregister_device(dev);
        input_free_device(dev);
}

static void proximity_set_enable(int enable)
{
        struct proximity_data *proximity = proximity_ptr;
        int delay = atomic_read(&proximity->delay);
	int ret;

//////////// PWM Enable
	if(enable) {
		proximity_period_ns = PERIOD_NS;
  		proximity_duty_ns = DUTY_NS;
  		proximity_pwm = pwm_request(PROXIMITY_PWM_CH, "proximity");

  		if (IS_ERR(proximity_pwm)) {
  		  printk("unable to request PWM for proximity\n");
  		  ret = PTR_ERR(proximity_pwm);
  		}
		//printk("got pwm for proximity\n");

 		 pwm_config(proximity_pwm, proximity_duty_ns, proximity_period_ns);
 		 pwm_enable(proximity_pwm);
	} else {
		pwm_config(proximity_pwm, 0, proximity_period_ns);
		pwm_disable(proximity_pwm);
		pwm_free(proximity_pwm);
	}
////////////////////////

        mutex_lock(&proximity->enable_mutex);

        if (enable)
        {                   /* enable if state will be changed */
                if (!atomic_cmpxchg(&proximity->enable, 0, 1))
                {
                        schedule_delayed_work(&proximity->work,
                                              delay_to_jiffies(delay) + 1);
                }
        }
        else
        {                        /* disable if state will be changed */
                if (atomic_cmpxchg(&proximity->enable, 1, 0))
                {
                        cancel_delayed_work_sync(&proximity->work);
                }
        }
        atomic_set(&proximity->enable, enable);

        mutex_unlock(&proximity->enable_mutex);
}

static int proximity_get_enable()
{
        struct proximity_data *proximity = proximity_ptr;

        return atomic_read(&proximity->enable);
}

static int proximity_get_delay()
{
        struct proximity_data *proximity = proximity_ptr;

        return atomic_read(&proximity->delay);
}

static void proximity_set_delay(int delay)
{
        int delay_ms;
        struct proximity_data *proximity = proximity_ptr;

        atomic_set(&proximity->delay, delay);

        mutex_lock(&proximity->enable_mutex);

        if (proximity_get_enable())
        {
                delay_ms = delay;
                cancel_delayed_work_sync(&proximity->work);
                schedule_delayed_work(&proximity->work,
                                      delay_to_jiffies(delay_ms) + 1);
        }

        mutex_unlock(&proximity->enable_mutex);
}

/*
 * sysfs device attributes
 */
static ssize_t proximity_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
        return sprintf(buf, "1\n");
}

static ssize_t proximity_enable_store(struct device *dev,
                                   struct device_attribute *attr,
                                   const char *buf, size_t count)
{
        unsigned long enable = simple_strtoul(buf, NULL, 10);

        if ((enable == 0) || (enable == 1)){
           proximity_set_enable(enable);
        }

        return count;
}

static ssize_t proximity_delay_show(struct device *dev,
                                 struct device_attribute *attr, char *buf)
{
        return sprintf(buf, "%d\n", proximity_get_delay());
}

static ssize_t proximity_delay_store(struct device *dev,
                                  struct device_attribute *attr,
                                  const char *buf, size_t count)
{
        unsigned long delay = simple_strtoul(buf, NULL, 10);

        proximity_set_delay(delay);

        return count;
}

static ssize_t proximity_wake_store(struct device *dev,
                                 struct device_attribute *attr,
                                 const char *buf, size_t count)
{
        struct input_dev *input = to_input_dev(dev);
        static atomic_t serial = ATOMIC_INIT(0);

        input_report_abs(input, ABS_MISC, atomic_inc_return(&serial));

        return count;
}

static ssize_t proximity_data_show(struct device *dev,
                                struct device_attribute *attr, char *buf)
{
        struct proximity_data *proximity = proximity_ptr;
	int distance;

        mutex_lock(&proximity->data_mutex);
        distance = proximity->distance;
        mutex_unlock(&proximity->data_mutex);

        return sprintf(buf, "%d \n", distance);
}


static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP,
                   proximity_enable_show, proximity_enable_store);
static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP,
                   proximity_delay_show, proximity_delay_store);
static DEVICE_ATTR(wake, S_IWUSR|S_IWGRP,
                   NULL, proximity_wake_store);
static DEVICE_ATTR(data, S_IRUGO,
                   proximity_data_show, NULL);

static struct attribute *proximity_attributes[] = {
        &dev_attr_enable.attr,
        &dev_attr_delay.attr,
        &dev_attr_wake.attr,
        &dev_attr_data.attr,
        NULL
};

static struct attribute_group proximity_attribute_group = {
        .attrs = proximity_attributes
};

/* proximity file operation */
static int proximity_device_open( struct inode* inode, struct file* file)
{
#if 0
	int ret;
	// add KSJ
			proximity_period_ns = PERIOD_NS;
			proximity_duty_ns = DUTY_NS;
			proximity_pwm = pwm_request(PROXIMITY_PWM_CH, "proximity");
	
			if (IS_ERR(proximity_pwm)) {
			  printk("unable to request PWM for proximity\n");
			  ret = PTR_ERR(proximity_pwm);
			}
			//printk("got pwm for proximity\n");
	
			 pwm_config(proximity_pwm, proximity_duty_ns, proximity_period_ns);
			 pwm_enable(proximity_pwm);
			 proximity_usage = 1;
	// ~add KSJ
#endif
		
    return 0;
}

static int proximity_device_release( struct inode* inode, struct file* file)
{
#if 0
	pwm_config(proximity_pwm, 0, proximity_period_ns);
		pwm_disable(proximity_pwm);
		pwm_free(proximity_pwm);
		proximity_usage = 0;
#endif

        return 0;
}

static int proximity_device_ioctl( struct inode* inode, struct file* file, unsigned int cmd, unsigned long arg)
{
        void __user *argp = (void __user *)arg;
        int ret=0;
        int kbuf=0;
        int distance;

        switch( cmd )
        {
        case PROXIMITY_IOCTL_ENABLE:
                proximity_set_enable(1);
                break;
        case PROXIMITY_IOCTL_DISABLE:
		proximity_set_enable(0);
                break;
        case PROXIMITY_IOCTL_IS_ENABLE:
                kbuf = proximity_get_enable();
                if(copy_to_user(argp, &kbuf, sizeof(kbuf)))
                        return -EFAULT;
                break;
        case PROXIMITY_IOCTL_DELAY_GET:
                kbuf = proximity_get_delay();
                if(copy_to_user(argp, &kbuf, sizeof(kbuf)))
                        return -EFAULT;
                break;
        case PROXIMITY_IOCTL_DELAY_SET:
                if(copy_from_user(&kbuf, argp, sizeof(kbuf)))
                        return -EFAULT;
                proximity_set_delay(kbuf);
                break;
        case PROXIMITY_IOCTL_DATA:
                distance = proximity_ptr->distance;
                if(copy_to_user(argp, &distance, sizeof(distance)))
                        return -EFAULT;
                break;
        default:
                return -ENOTTY;
        }

        return ret;
}

//static ssize_t proximity_device_read( struct file *filp, char *buf, size_t count, loff_t *ofs )
static ssize_t proximity_device_read( struct file *inode, char *gdata, size_t length, loff_t *off_what )
{
	int ret, value;

	value = s3c_adc_get_adc_data(ADC_CHANNEL);

	ret=copy_to_user(gdata,&value,4);

	if(ret<0) return -1;

    return 0;
}

static ssize_t proximity_device_write( struct file *filp, const char *buf, size_t count, loff_t *ofs )
{
        return 0;
}

static unsigned int proximity_device_poll( struct file *filp, struct poll_table_struct *pwait )
{
        return 0;
}

static int proximity_probe(struct platform_device *pdev)
{
	struct proximity_data *proximity;
        int err = 0;

	proximity = kzalloc(sizeof(struct proximity_data), GFP_KERNEL);
	if(!proximity) {
		return -ENOMEM;
	}
        mutex_init(&proximity->enable_mutex);
        mutex_init(&proximity->data_mutex);


        /* setup driver interfaces */
        INIT_DELAYED_WORK(&proximity->work, proximity_work_func);

        err = proximity_input_init(proximity);
        if (err < 0)
                goto error_1;

        err = sysfs_create_group(&proximity->input->dev.kobj, &proximity_attribute_group);
        if (err < 0)
                goto error_2;

        err = misc_register(&proximity_misc);
        if (err < 0)
                goto error_3;

        proximity_ptr = proximity;

        proximity_set_delay(100);

        return 0;

error_3:
	sysfs_remove_group(&proximity->input->dev.kobj, &proximity_attribute_group);
error_2:
	proximity_input_fini(proximity);
error_1:
	kfree(proximity);
	return err;
}

static int proximity_remove(struct platform_device *pdev)
{
        struct proximity_data *proximity = platform_get_drvdata(pdev);

        misc_deregister(&proximity_misc);
	sysfs_remove_group(&proximity->input->dev.kobj, &proximity_attribute_group);
        proximity_input_fini(proximity);
#if 0
// add KSJ
		pwm_config(proximity_pwm, 0, proximity_period_ns);
  		pwm_disable(proximity_pwm);
 		pwm_free(proximity_pwm);
// ~add KSJ
#endif
        return 0;
}

static int proximity_suspend(struct platform_device *pdev, pm_message_t mesg)
{
        return 0;
}

static int proximity_resume(struct platform_device *pdev)
{
        return 0;
}

static struct platform_driver proximity_driver = {
	.probe		= proximity_probe,
	.remove		= proximity_remove,
	.suspend	= proximity_suspend,
	.resume		= proximity_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= PROXIMITY_NAME,
	},

};


static int __init proximity_init(void)
{
	printk("HBE-SM5-S4210 PROXIMITY Device Driver initialized Ver!! 1.0 \n");
	return platform_driver_register(&proximity_driver);
}

static void __exit proximity_exit(void)
{
	platform_driver_unregister(&proximity_driver);
	printk("driver: %s DRIVER EXIT\n", PROXIMITY_NAME);
}

module_init(proximity_init);	
module_exit(proximity_exit);	

MODULE_AUTHOR(DRIVER_AUTHOR);	
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("Dual BSD/GPL");	

