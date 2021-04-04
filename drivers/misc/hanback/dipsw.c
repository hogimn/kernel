/*
 * Dipsw Device Driver
 *  Hanback Electronics Co.,ltd
 * File : dipsw.c
 * Date : Dec 14, 2010
 */ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <asm/fcntl.h>
#include <linux/ioport.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/file.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define DRIVER_AUTHOR           "Hanback Electronics"	
#define DRIVER_DESC		"dipsw program" 

#define	DIP_NAME		"dipsw"	
#define DIP_MODULE_VERSION	"DIPSW V1.0"

#define ADDR_DIP_PHY		0x05000062
#define DIP_ADDRESS_RANGE	0x1000		
#define TIMER_INTERVAL		20

//Global variable
static unsigned int dipsw_usage = 0;
static unsigned long  *dipsw_ioremap;
static unsigned short *dipaddr;
static unsigned short value[4];

// define functions...
int dipsw_open (struct inode *inode, struct file *filp)
{
	if(dipsw_usage != 0) return -EBUSY;

	dipsw_ioremap = ioremap(ADDR_DIP_PHY, DIP_ADDRESS_RANGE);

	if(!check_mem_region((unsigned long)dipsw_ioremap,DIP_ADDRESS_RANGE)) {
		request_mem_region((unsigned long)dipsw_ioremap, DIP_ADDRESS_RANGE, DIP_NAME);
	}
	else	printk("driver : unable to register this!\n");
	
	dipaddr =(unsigned short*)dipsw_ioremap;
	
	dipsw_usage = 1;
	return 0;
}

int dipsw_release (struct inode *inode, struct file *filp)
{
	iounmap(dipsw_ioremap);

	release_mem_region((unsigned long)dipsw_ioremap, DIP_ADDRESS_RANGE);
	dipsw_usage = 0;
	return 0;
}

ssize_t dipsw_read(struct file *inode, char *gdata, size_t length, loff_t *off_what)
{
	int ret;

	value[0] = ((*dipaddr & 0x00f0)>>4) | ((*dipaddr & 0x000f)<<4);

	value[1]=((*dipaddr & 0xf000)>>8)>>4 | ((*dipaddr & 0x0f00)>>8)<<4;

	ret=copy_to_user(gdata,value,4);
	if(ret<0) return -1;

	return length;
}

struct file_operations dipsw_fops = 
{
	.owner		= THIS_MODULE,
	.open		= dipsw_open,
	.read		= dipsw_read,
	.release	= dipsw_release,
};

static struct miscdevice dipsw_driver = {
	.fops	= &dipsw_fops,
	.name	= &DIP_NAME,
	.minor	= MISC_DYNAMIC_MINOR,
};

int dipsw_init(void)
{
	
	return misc_register(&dipsw_driver);
}

void dipsw_exit(void)
{
	misc_deregister(&dipsw_driver);
	printk("driver: %s DRIVER EXIT\n", DIP_NAME);
}

module_init(dipsw_init);	
module_exit(dipsw_exit);	

MODULE_AUTHOR(DRIVER_AUTHOR);	
MODULE_DESCRIPTION(DRIVER_DESC); 
MODULE_LICENSE("Dual BSD/GPL");	 

