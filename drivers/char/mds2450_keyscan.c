/*
*	G2450_ADC.C - The s3c2450 adc module.
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <asm/delay.h>
#include <linux/io.h>
#include <plat/adc.h>
#include <plat/devs.h>
#include <linux/platform_device.h>
#include <mach/regs-gpio.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <asm/uaccess.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/sched.h>

static void kscan_timer_handler(unsigned long data); 

#define MDS2450_KSCAN_MAJOR 67
static char kscan_name[] = "mds2450-kscan";

static DECLARE_WAIT_QUEUE_HEAD(kscan_wq);

#define KSCAN_TIME	(1*HZ / 5)
static struct timer_list kscan_timer = TIMER_INITIALIZER(kscan_timer_handler, 0, 0);

static int key_value = 0;

static int key_scan( int row )
{
	int value = 0; 

	if( 0 == row )
	{	// 1 ~ 5
		gpio_set_value(S3C2410_GPG(0), 1);
		gpio_set_value(S3C2410_GPF(7), 0);
	}
	else
	{	// 6 ~ 10
		gpio_set_value(S3C2410_GPG(0), 0);
		gpio_set_value(S3C2410_GPF(7), 1);
	}

	if( 0 ==  gpio_get_value(S3C2410_GPF(2)))
	{
		value = 1 + (5 * row);
	}
	else if( 0 ==  gpio_get_value(S3C2410_GPF(3)))
	{
		value = 2 + (5 * row);
	}
	else if( 0 ==  gpio_get_value(S3C2410_GPF(4)))
	{
		value = 3 + (5 * row);
	}
	else if( 0 ==  gpio_get_value(S3C2410_GPF(5)))
	{
		value = 4 + (5 * row);
	}
	else if( 0 ==  gpio_get_value(S3C2410_GPF(6)))
	{
		value = 5 + (5 * row);
	}

	return value;
}

static int key_tact ( void )
{
	int value = 0;

	
	if( 0 ==  gpio_get_value(S3C2410_GPF(0)))
	{
		value = 11;
	}
	else if( 0 ==  gpio_get_value(S3C2410_GPF(1)))
	{
		value = 12;
	}

	return value;
}


static void kscan_timer_handler(unsigned long data)
{
	int lp;

	for( lp=0 ; lp<2 ; lp++ )
	{
		key_value = key_scan(lp);

		if( key_value > 0 )
		{
			wake_up_interruptible(&kscan_wq);
			break;
		}
	}

	mod_timer(&kscan_timer, jiffies + (KSCAN_TIME));
}


static ssize_t mds2450_kscan_read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
	int  ret;

	if( key_value == 0 )
		wait_event_interruptible(kscan_wq, key_value != 0);

	copy_to_user((void *)buff, (const void *)&key_value , sizeof( int ));
	ret = key_value;
	key_value = 0;
	
	return ret;
}

static irqreturn_t mds2450_kscan_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	key_value = key_tact();

	if( key_value > 0 )
		wake_up_interruptible(&kscan_wq);
	
	return IRQ_HANDLED;
}


static int mds2450_kscan_open(struct inode * inode, struct file * file)
{
	int ret = 0;

	printk(KERN_INFO "ready to scan key value\n");

	// GPIO Initial
	s3c_gpio_cfgpin(S3C2410_GPF(0), S3C_GPIO_SFN(2));
	s3c_gpio_cfgpin(S3C2410_GPF(1), S3C_GPIO_SFN(2));
	s3c_gpio_setpull(S3C2410_GPF(0), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S3C2410_GPF(1), S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(S3C2410_GPF(2), S3C_GPIO_SFN(0));
	s3c_gpio_cfgpin(S3C2410_GPF(3), S3C_GPIO_SFN(0));
	s3c_gpio_cfgpin(S3C2410_GPF(4), S3C_GPIO_SFN(0));
	s3c_gpio_cfgpin(S3C2410_GPF(5), S3C_GPIO_SFN(0));
	s3c_gpio_cfgpin(S3C2410_GPF(6), S3C_GPIO_SFN(0));
	s3c_gpio_setpull(S3C2410_GPF(2), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S3C2410_GPF(3), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S3C2410_GPF(4), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S3C2410_GPF(5), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S3C2410_GPF(6), S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(S3C2410_GPF(7), S3C_GPIO_SFN(1));
	s3c_gpio_cfgpin(S3C2410_GPG(0), S3C_GPIO_SFN(1));

	// Request IRQ
	if( request_irq(IRQ_EINT0, mds2450_kscan_irq, IRQF_DISABLED | IRQF_TRIGGER_FALLING , "MDS2450_KEY_0", NULL ) )     
	{
		printk("failed to request external interrupt. - EINT0\n");
		ret = -ENOENT;
		return ret;
	}

	if( request_irq(IRQ_EINT1, mds2450_kscan_irq, IRQF_DISABLED | IRQF_TRIGGER_FALLING , "MDS2450_KEY_1", NULL ) )     
	{
		printk("failed to request external interrupt. - EINT1\n");
		ret = -ENOENT;
		return ret;
	}

	// Scan timer
	mod_timer(&kscan_timer, jiffies + (KSCAN_TIME));

	return ret;
}

static void mds2450_kscan_release(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "end of the scanning\n");

	free_irq( IRQ_EINT0, NULL );
	free_irq( IRQ_EINT1, NULL );

	 del_timer_sync(&kscan_timer);
}

static struct file_operations mds2450_kscan_fops = {
	.owner 	= THIS_MODULE,
	.open 	= mds2450_kscan_open,
	.release= mds2450_kscan_release,
	.read 	= mds2450_kscan_read,
};

static int __devinit mds2450_kscan_probe(struct platform_device *pdev)
{
	int ret;


	ret = register_chrdev( MDS2450_KSCAN_MAJOR, kscan_name, &mds2450_kscan_fops );

    return ret;
}

static int __devexit mds2450_kscan_remove(struct platform_device *pdev)
{
	unregister_chrdev( MDS2450_KSCAN_MAJOR, kscan_name );

	return 0;
}

static struct platform_driver mds2450_kscan_device_driver = {
	.probe      = mds2450_kscan_probe,
	.remove     = __devexit_p(mds2450_kscan_remove),
	.driver     = {
		.name   = "mds2450-kscan",
		.owner  = THIS_MODULE,
	}
};

static int __init mds2450_kscan_init(void)
{
 	return platform_driver_register(&mds2450_kscan_device_driver);
}

static void __exit mds2450_kscan_exit(void)
{
	platform_driver_unregister(&mds2450_kscan_device_driver);
}

module_init(mds2450_kscan_init);
module_exit(mds2450_kscan_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("GIT <gemini525@nate.com>");
MODULE_DESCRIPTION("Key driver for MDS2450");
