/*
 * 8bit-LED Device Driver
 *  Hanback Electronics Co.,ltd
 * File : led.c
 * Date : September, 2011
 */ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/kdev_t.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <asm/ioctl.h>
#include <asm/io.h>

#define DRIVER_AUTHOR		"Hanback Electronics"
#define DRIVER_DESC		"led test program"	

#define LED_NAME    		"led"	
#define LED_MODULE_VERSION	"LED V1.0"	
#define LED_ADDRESS 		0x05000020	
#define LED_ADDRESS_RANGE 	0x1000		

//Global variable
static int led_usage = 0;	
static unsigned long *led_ioremap; 

// define functions...
int led_open(struct inode *minode, struct file *mfile) 
{

	if(led_usage != 0) return -EBUSY;
	
	led_ioremap= ioremap(LED_ADDRESS,LED_ADDRESS_RANGE);

	if(!check_mem_region((unsigned long)led_ioremap,LED_ADDRESS_RANGE)){
		request_mem_region((unsigned long)led_ioremap,LED_ADDRESS_RANGE,LED_NAME);
	} else printk(KERN_WARNING"Can't get IO Region 0x%x\n", (unsigned int)led_ioremap);

	led_usage = 1;
	return 0;
}

int led_release(struct inode *minode, struct file *mfile) 
{	
	iounmap(led_ioremap);

	release_mem_region((unsigned long)led_ioremap,LED_ADDRESS_RANGE);
	led_usage = 0;
	return 0;
}

ssize_t led_write_byte(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	unsigned short *addr;
	unsigned char  c;

	get_user(c,gdata); 
	
	addr = (unsigned short*)led_ioremap;
	*addr = c|0x100;
	
	return length;
}

static struct file_operations led_fops = {
	.owner		= THIS_MODULE, 
	.open		= led_open,
	.write		= led_write_byte,
	.release	= led_release,
};

static struct miscdevice led_driver = {
	.fops = &led_fops,
	.name = LED_NAME,
	.minor = MISC_DYNAMIC_MINOR,
};

int led_init(void) {
	
	misc_register(&led_driver);
	return 0;
}

void led_exit(void) 
{
	misc_deregister(&led_driver);
	printk(KERN_INFO"driver: %s DRIVER EXIT\n", LED_NAME);
}


module_init(led_init);	
module_exit(led_exit);	

MODULE_AUTHOR(DRIVER_AUTHOR);	
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("Dual BSD/GPL");	
