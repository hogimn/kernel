/*
 * 8bit-LED Device Driver
 *  Hanback Electronics Co.,ltd
 * File : baseled.c
 * Date : October,2010
 */ 


#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/list.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <mach/regs-gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/gpio-cfg.h>


#define DRIVER_AUTHOR			"hanback"		
#define DRIVER_DESC			"BaseLed program"	

#define BASELED_NAME    		"baseled"	
#define BASELED_MODULE_VERSION		"BASE LED V1.0"	


//Global variable
static int baseled_usage = 0;		

// define functions...
int baseled_open(struct inode *minode, struct file *mfile) 
{
	int err,i;
	char cNum[8];

	for (i=0; i<7; i++)
	{
		err = gpio_request(S5PV310_GPK0(i), "BaseLed");
			
        	if (err)
			printk("baseled.c failed to request GPK0(%d) \n",i);

		s3c_gpio_setpull(S5PV310_GPK0(i), S3C_GPIO_PULL_NONE);
        	gpio_direction_output(S5PV310_GPK0(i), 0);
	}

	err = gpio_request(S5PV310_GPC1(2), "GPC1(2)");
       	if (err)
                printk("#### baseled.c failed to request GPC1(2) ####\n");
	s3c_gpio_setpull(S5PV310_GPC1(2), S3C_GPIO_PULL_NONE);
       	gpio_direction_output(S5PV310_GPC1(2), 0);

	return 0;
}

int baseled_release(struct inode *minode, struct file *mfile) 
{	
	int i;
	for (i=0; i<7; i++)
		gpio_free(S5PV310_GPK0(i));

	gpio_free(S5PV310_GPC1(2));

	return 0;
}

static int baseled_ioctl(struct inode *inode, unsigned char cmd[], size_t length, loff_t *off_what)
{
	gpio_direction_output(S5PV310_GPK0(0), (unsigned int) cmd[0]);
	gpio_direction_output(S5PV310_GPK0(1), (unsigned int) cmd[1]);
	gpio_direction_output(S5PV310_GPK0(2), (unsigned int) cmd[2]);
	gpio_direction_output(S5PV310_GPK0(3), (unsigned int) cmd[3]);
	gpio_direction_output(S5PV310_GPK0(4), (unsigned int) cmd[4]);
	gpio_direction_output(S5PV310_GPK0(5), (unsigned int) cmd[5]);
	gpio_direction_output(S5PV310_GPK0(6), (unsigned int) cmd[6]);
	gpio_direction_output(S5PV310_GPC1(2), (unsigned int) cmd[7]);

	return 0;
}

ssize_t baseled_write_byte(struct file *inode, unsigned char *gdata, size_t length, loff_t *off_what) 
{
	int ret,i=0,j=0;
	unsigned char buf[8];
	ret = copy_from_user(buf,gdata,length);
	
	gpio_direction_output(S5PV310_GPK0(0), (unsigned int) buf[0]);
	gpio_direction_output(S5PV310_GPK0(1), (unsigned int) buf[1]);
	gpio_direction_output(S5PV310_GPK0(2), (unsigned int) buf[2]);
	gpio_direction_output(S5PV310_GPK0(3), (unsigned int) buf[3]);
	gpio_direction_output(S5PV310_GPK0(4), (unsigned int) buf[4]);
	gpio_direction_output(S5PV310_GPK0(5), (unsigned int) buf[5]);
	gpio_direction_output(S5PV310_GPK0(6), (unsigned int) buf[6]);
	gpio_direction_output(S5PV310_GPC1(2), (unsigned int) buf[7]);
	
	return length;
}

static struct file_operations baseled_fops = {
	.owner		= THIS_MODULE, 
	.open		= baseled_open,
	.ioctl		= baseled_ioctl,
	.write		= baseled_write_byte,
	.release	= baseled_release,
};

static struct miscdevice baseled_driver = {
	.fops	= &baseled_fops,
	.name = BASELED_NAME,
	.minor = MISC_DYNAMIC_MINOR,
};


int baseled_init(void) {
	
	printk("driver: %s DRIVER INIT\n",BASELED_NAME);
	return misc_register(&baseled_driver);
}

void baseled_exit(void) 
{
	misc_deregister(&baseled_driver);
        printk("driver: %s DRIVER EXIT\n",BASELED_NAME);
}


module_init(baseled_init);	
module_exit(baseled_exit);	

MODULE_AUTHOR(DRIVER_AUTHOR);	
MODULE_DESCRIPTION(DRIVER_DESC); 
MODULE_LICENSE("Dual BSD/GPL");	 
