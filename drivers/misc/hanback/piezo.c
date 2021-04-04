/*
 * Piezo Device Driver
 *  Hanback Electronics Co.,ltd
 * File : piezo.c
 * Date : August,2011
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

#define DRIVER_AUTHOR		"Hanback"
#define DRIVER_DESC		"piezo test program"	

#define PIEZO_NAME    		"piezo"	
#define PIEZO_MODULE_VERSION	"PIEZO V1.0"
#define PIEZO_ADDRESS 		0x05000050	
#define PIEZO_ADDRESS_RANGE 	0x1000	

//Global variable
static int piezo_usage = 0;
static unsigned long  *piezo_ioremap;
static unsigned char *piezo_addr;

// define functions...
int piezo_open(struct inode *minode, struct file *mfile) 
{

	if(piezo_usage != 0) return -EBUSY;
	
	piezo_ioremap=ioremap(PIEZO_ADDRESS,PIEZO_ADDRESS_RANGE);
	
	if(!check_mem_region((unsigned long)piezo_ioremap, PIEZO_ADDRESS_RANGE)) {
		request_mem_region((unsigned long)piezo_ioremap, PIEZO_ADDRESS_RANGE, PIEZO_NAME);
	}
	else	printk("driver: unable to register this!\n");
	
	piezo_addr = (unsigned char *)piezo_ioremap;
	piezo_usage = 1;
	return 0;
}

int piezo_release(struct inode *minode, struct file *mfile) 
{
	iounmap(piezo_ioremap);

	release_mem_region((unsigned long)piezo_ioremap, PIEZO_ADDRESS_RANGE);
	piezo_usage = 0;
	return 0;
}

ssize_t piezo_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what)
{
	unsigned char  c;

	get_user(c,gdata); 
	
	*piezo_addr = c;
	
	return length;
}

struct file_operations piezo_fops = {
	.owner		= THIS_MODULE,
	.write		= piezo_write,
	.open		= piezo_open,
	.release	= piezo_release,
};

static struct miscdevice piezo_driver = {
	.fops	= &piezo_fops,
	.name = PIEZO_NAME,
	.minor = MISC_DYNAMIC_MINOR,
};


int piezo_init(void)
{
	printk("driver: %s DRIVER INIT\n",PIEZO_NAME);
	
	return misc_register(&piezo_driver);
}

void piezo_exit(void)
{
	misc_deregister(&piezo_driver);
	printk("driver: %s DRIVER EXIT\n",PIEZO_NAME);

}

module_init(piezo_init);	
module_exit(piezo_exit);

MODULE_AUTHOR(DRIVER_AUTHOR); 	
MODULE_DESCRIPTION(DRIVER_DESC); 
MODULE_LICENSE("Dual BSD/GPL");	 
