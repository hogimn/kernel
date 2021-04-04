/*
 * Base Board 7-Segment Device Driver
 *  Hanback Electronics Co.,ltd
 * File : baseseg.c
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
//#include <mach/gpio-bank.h>
#include <plat/gpio-cfg.h>



#define DRIVER_AUTHOR		"hanback"		
#define DRIVER_DESC		"Base Board 7-Segment program"

#define	SEGMENT_NAME		"baseseg"
#define SEGMENT_MODULE_VERSION	"BASESEG V1.0"

//Global variable
static unsigned int segment_usage = 0;
unsigned int ibuf[8];


// define functions...
int baseseg_open(struct inode *inode, struct file *filp)
{
	int err,i;

	gpio_free(S5PV310_GPC0(0));
	gpio_free(S5PV310_GPC0(1));
	
	for(i=0; i<8; i++)
		gpio_free(S5PV310_GPL0(i));

	err = gpio_request(S5PV310_GPC0(0), "GPC0(0)");
       	if (err)
                printk("#### baseled.c failed to request GPC0(0)\n");
	s3c_gpio_setpull(S5PV310_GPC0(0), S3C_GPIO_PULL_UP);
       	gpio_direction_output(S5PV310_GPC0(0), 1);
			
	err = gpio_request(S5PV310_GPC0(1), "GPC0(1)");
       	if (err)
                printk("#### baseled.c failed to request GPC0(1)\n");
	s3c_gpio_setpull(S5PV310_GPC0(1), S3C_GPIO_PULL_UP);
       	gpio_direction_output(S5PV310_GPC0(1), 1);

	for(i=0; i<8; i++)
	{
		err = gpio_request(S5PV310_GPL0(i), "Segment");
        	if (err)
	                printk("baseled.c failed to request GPL0(%d) \n",i);
		s3c_gpio_setpull(S5PV310_GPL0(i), S3C_GPIO_PULL_NONE);
	        gpio_direction_output(S5PV310_GPL0(i), 0);
	}
	
	return 0; 
}

int baseseg_release (struct inode *inode, struct file *filp)
{

	int i;
	
	gpio_free(S5PV310_GPC0(0));
	gpio_free(S5PV310_GPC0(1));
	
	for(i=0; i<8; i++)
		gpio_free(S5PV310_GPL0(i));
	
	return 0;
}
#if 1
int Getsegmentcode_base (int x)
{
	unsigned int i;

	for (i = 0; i < 8; i++)
		ibuf[i] = 0;

	switch (x) {
		case 0 : 
			for (i=0; i<6; i++) ibuf[i] = 1;
			break;
			
		case 1 : ibuf[1] = 1; ibuf[2] = 1; break;
		
		case 2 : 
			for (i=0; i<2; i++) ibuf[i] = 1;
			for (i=3; i<5; i++) ibuf[i] = 1;
			ibuf[6] = 1;
			break;
			
		case 3 : 
			for (i=0; i<4; i++) ibuf[i] = 1;
			ibuf[6] = 1;
			break;
			
		case 4 :
			for (i=1; i<3; i++) ibuf[i] = 1;
			for (i=5; i<7; i++) ibuf[i] = 1;
			break;
			
		case 5 : 
			ibuf[0] = 1;
			for (i=2; i<4; i++) ibuf[i] = 1;
			for (i=5; i<7; i++) ibuf[i] = 1;
			break;
			
		case 6 : 
			for (i=2; i<7; i++) ibuf[i] = 1;
			//for (i=5; i<7; i++) ibuf[i] = 1;
			break;
			
		case 7 : 
			for (i=0; i<3; i++) ibuf[i] = 1;
			ibuf[5] = 1;
			break;

		case 8 : 
			for (i=0; i<7; i++) ibuf[i] = 1;
			break;
			
		case 9 : 
			for (i=0; i<4; i++) ibuf[i] = 1;
			for (i=5; i<7; i++) ibuf[i] = 1;
			break;
		
		case 10 :
			for (i=0; i<3; i++) ibuf[i] = 1;
			for (i=4; i<8; i++) ibuf[i] = 1;
			break;
		case 11 :
			for (i=0; i<8; i++) ibuf[i] = 1;
			break;
		case 12 :
			for (i=3; i<6; i++) ibuf[i] = 1;
			ibuf[0] = 1;
			ibuf[7] = 1;
			//ibuf[4] = 1;
			//ibuf[6] = 1;
			//ibuf[7] = 1;
			break;
		case 13 :
			ibuf[7] = 1;
			for (i=0; i<6; i++) ibuf[i] = 1;
			break;
		case 14 :
			for (i=3; i<8; i++) ibuf[i] = 1;
			ibuf[0] = 1;
			//ibuf[6] = 1;
			//ibuf[7] = 1;
			break;
		case 15 :
			for (i=4; i<8; i++) ibuf[i] = 1;
			//ibuf[4] = 1;
			ibuf[0] = 1;
			break;
			
		default : 
			for (i=0; i<8; i++) ibuf[i] = 1;
			break;
	}
	return 0;
}

#else

int Getsegmentcode_base (int x)
{
	unsigned int i;

	for (i = 0; i < 8; i++)
		ibuf[i] = 0;

	switch (x) {
		case 0 : 
			ibuf[0] = 1;
			for (i=2; i<7; i++) ibuf[i] = 1;
			break;
			
		case 1 : ibuf[3] = 1; ibuf[5] = 1; break;
		
		case 2 : 
			for (i=0; i<2; i++) ibuf[i] = 1;
			for (i=3; i<5; i++) ibuf[i] = 1;
			ibuf[6] = 1;
			break;
			
		case 3 : 
			for (i=3; i<7; i++) ibuf[i] = 1;
			ibuf[1] = 1;
			break;
			
		case 4 :
			for (i=1; i<4; i++) ibuf[i] = 1;
			for (i=5; i<6; i++) ibuf[i] = 1;
			break;
			
		case 5 : 
			//ibuf[0] = 1;
			for (i=1; i<3; i++) ibuf[i] = 1;
			for (i=4; i<7; i++) ibuf[i] = 1;
			break;
			
		case 6 : 
			for (i=0; i<3; i++) ibuf[i] = 1;
			for (i=5; i<7; i++) ibuf[i] = 1;
			break;
			
		case 7 : 
			for (i=2; i<6; i++) ibuf[i] = 1;
			//ibuf[5] = 1;
			break;

		case 8 : 
			for (i=0; i<7; i++) ibuf[i] = 1;
			break;
			
		case 9 : 
			for (i=1; i<6; i++) ibuf[i] = 1;
			//for (i=5; i<7; i++) ibuf[i] = 1;
			break;
		
		case 10 :
			for (i=0; i<6; i++) ibuf[i] = 1;
			for (i=7; i<8; i++) ibuf[i] = 1;
			break;
		case 11 :
			for (i=0; i<8; i++) ibuf[i] = 1;
			break;
		case 12 :
			//for (i=3; i<6; i++) ibuf[i] = 1;
			ibuf[0] = 1;
			ibuf[2] = 1;
			ibuf[4] = 1;
			ibuf[6] = 1;
			ibuf[7] = 1;
			break;
		case 13 :
			ibuf[0] = 1;
			for (i=0; i<8; i++) ibuf[i] = 1;
			break;
		case 14 :
			for (i=0; i<3; i++) ibuf[i] = 1;
			ibuf[4] = 1;
			ibuf[6] = 1;
			ibuf[7] = 1;
			break;
		case 15 :
			for (i=0; i<3; i++) ibuf[i] = 1;
			ibuf[4] = 1;
			ibuf[7] = 1;
			break;
			
		default : 
			for (i=0; i<8; i++) ibuf[i] = 1;
			break;
	}
	return 0;
}
#endif

ssize_t baseseg_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what)
{
	unsigned int ret,i;
	unsigned int num[2];
	unsigned char data[2];
	ret=copy_from_user(data,gdata,2);

	if((data[0] == 0xff) && (data[1] == 0xff)) {
		gpio_direction_output(S5PV310_GPC0(0), 1);
		gpio_direction_output(S5PV310_GPC0(1), 1);
		return length;
	}
	Getsegmentcode_base((unsigned int)data[0]);
	
	gpio_direction_output(S5PV310_GPC0(0), 0);
	gpio_direction_output(S5PV310_GPC0(1), 1);
	for(i=0;i<8;i++) {
		gpio_direction_output(S5PV310_GPL0(i), (unsigned int) ibuf[i]);
	}
	mdelay(5);	

	Getsegmentcode_base((unsigned int)data[1]);

	gpio_direction_output(S5PV310_GPC0(0), 1);
	gpio_direction_output(S5PV310_GPC0(1), 0);
	for(i=0;i<8;i++) {
		gpio_direction_output(S5PV310_GPL0(i), (unsigned int) ibuf[i]);
	}
	mdelay(1);
	
	return length;
}

static int baseseg_ioctl(struct inode *inode, struct file *flip, unsigned int cmd, unsigned long arg)
{
	segment_usage = cmd;
	
    return 0;
}
							    

struct file_operations baseseg_fops = 
{
	.owner		= THIS_MODULE,
	.open		= baseseg_open,
	.write		= baseseg_write,
	.release	= baseseg_release,
	.ioctl		= baseseg_ioctl,
};

static struct miscdevice baseseg_driver = {
	.fops	= &baseseg_fops,
	.name = SEGMENT_NAME,
	.minor = MISC_DYNAMIC_MINOR,
};


int baseseg_init(void)
{
	printk("driver: %s DRIVER INIT\n",SEGMENT_NAME);
	return misc_register(&baseseg_driver);
}

void baseseg_exit(void)
{
	misc_deregister(&baseseg_driver);
        printk("driver: %s DRIVER EXIT\n",SEGMENT_NAME);
}

module_init(baseseg_init);
module_exit(baseseg_exit);	

MODULE_AUTHOR(DRIVER_AUTHOR);	
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("Dual BSD/GPL");	

