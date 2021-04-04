/*
 * Full Color LED Device Driver
 *  Hanback Electronics Co.,ltd
 * File : fullcolorled.c
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

#define DRIVER_AUTHOR			"hanback"		
#define DRIVER_DESC			"full color led test program"	

#define FULLCOLORLED_NAME    		"fullcolorled"	
#define FULLCOLORLED_MODULE_VERSION	"FULL COLOR LED V0.1"	
#define FULLCOLORLED_ADDRESS 		0x05000000	
#define FULLCOLORLED_ADDRESS_RANGE 	0x1000		

#define FULL_LED1	9
#define FULL_LED2	8
#define FULL_LED3	7
#define FULL_LED4	6
#define ALL_LED		5

static int fullled_usage = 0;
static int led_num=0;
static unsigned long  *fullled_ioremap;
static unsigned short *full_led_addr[4][3];

int fullled_open(struct inode *minode, struct file *mfile) 
{
	if(fullled_usage != 0) return -EBUSY;
	
	fullled_ioremap=ioremap(FULLCOLORLED_ADDRESS,FULLCOLORLED_ADDRESS_RANGE);
	
	if(!check_mem_region((unsigned long)fullled_ioremap, FULLCOLORLED_ADDRESS_RANGE)) {
		request_mem_region((unsigned long)fullled_ioremap, FULLCOLORLED_ADDRESS_RANGE, FULLCOLORLED_NAME);
	}
	else	printk("driver: unable to register this!\n");


	full_led_addr[0][0] =(unsigned short *)((unsigned long)fullled_ioremap+0xa0);
	full_led_addr[0][1] =(unsigned short *)((unsigned long)fullled_ioremap+0xa2);
	full_led_addr[0][2] =(unsigned short *)((unsigned long)fullled_ioremap+0xa4);

	full_led_addr[1][0] =(unsigned short *)((unsigned long)fullled_ioremap+0xa6);
	full_led_addr[1][1] =(unsigned short *)((unsigned long)fullled_ioremap+0xa8);
	full_led_addr[1][2] =(unsigned short *)((unsigned long)fullled_ioremap+0xaa);

	full_led_addr[2][0] =(unsigned short *)((unsigned long)fullled_ioremap+0xb0);
	full_led_addr[2][1] =(unsigned short *)((unsigned long)fullled_ioremap+0xb2);
	full_led_addr[2][2] =(unsigned short *)((unsigned long)fullled_ioremap+0xb4);

	full_led_addr[3][0] =(unsigned short *)((unsigned long)fullled_ioremap+0xb6);
	full_led_addr[3][1] =(unsigned short *)((unsigned long)fullled_ioremap+0xb8);
	full_led_addr[3][2] =(unsigned short *)((unsigned long)fullled_ioremap+0xba);

	fullled_usage = 1;
	return 0;
}

int fullled_release(struct inode *minode, struct file *mfile) 
{
	iounmap(fullled_ioremap);

	release_mem_region((unsigned long)fullled_ioremap, FULLCOLORLED_ADDRESS_RANGE);
	fullled_usage = 0;
	return 0;
}

static int fullled_ioctl(struct inode *inode, struct file *file, unsigned int cmd,unsigned long arg)
{
	switch(cmd)
	{
		case FULL_LED1:
			led_num = 0;
			break;
		case FULL_LED2:
			led_num = 1;
			break;
		case FULL_LED3:
			led_num = 2;
			break;
		case FULL_LED4:
			led_num = 3;
			break;
		case ALL_LED:
			led_num = 4;
			break;
	}
	return 0;
}

ssize_t fullled_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what)
{
	int ret,i=0,j=0;
	unsigned char buf[3];
	ret = copy_from_user(buf,gdata,length);
	for(j=0;j<3;j++)
	{
		if(buf[j]%0x10 >= 0xA)
			buf[j]+=0x06;
		if(buf[j]%0x100 >= 0xA0)
			buf[j]+=0x60;		
	}

	if(led_num != 4)
	{
		for(j=0;j<3;j++)
		{
			*full_led_addr[led_num][j] = buf[j];
		}
	}
	else
	{
		for(i=0;i<4;i++)
		{
			for(j=0;j<3;j++)
			{
				*full_led_addr[i][j] = buf[j];
			}
		}
	}
	return length;
}

struct file_operations fullled_fops = {
	.owner		= THIS_MODULE,
	.write		= fullled_write,
	.open		= fullled_open,
	.release	= fullled_release,
	.ioctl		= fullled_ioctl,
};

static struct miscdevice fullled_driver = {
	.fops	= &fullled_fops,
	.name = FULLCOLORLED_NAME,
	.minor = MISC_DYNAMIC_MINOR,
};

int fullled_init(void)
{
	printk("driver: %s DRIVER INIT\n",FULLCOLORLED_NAME);
	
	return misc_register(&fullled_driver);
}

void fullled_exit(void)
{
	misc_deregister(&fullled_driver);
	printk("driver: %s DRIVER EXIT\n",FULLCOLORLED_NAME);

}

module_init(fullled_init);		
module_exit(fullled_exit);		

MODULE_AUTHOR(DRIVER_AUTHOR); 	 
MODULE_DESCRIPTION(DRIVER_DESC); 
MODULE_LICENSE("Dual BSD/GPL");	
