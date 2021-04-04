/*
 * Dotmatrix Device Driver
 *  Hanback Electronics Co.,ltd
 * File : dotmatrix.c
 * Date : June 3, 2010
 */ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define DRIVER_AUTHOR		"Hanback Electronics"
#define DRIVER_DESC		"dotmatrix program" 

#define DOT_NAME 		"dotmatrix"	
#define DOT_MODULE_VERSION 	"DOTMATRIX V1.0"
#define DOT_PHY_ADDR		0x05000000	
#define DOT_ADDR_RANGE 		0x1000		

#define NUMSIZE			4
#define DELAY			2

//Global variable
static int dot_usage = 0;
static unsigned long  dot_ioremap;
static unsigned short *dot_row_addr,*dot_col_addr;

// Delay func.
void m_delay(int num) 
{
	volatile int i,j;
	for(i=0;i<num;i++)
		for(j=0;j<16384;j++);
}

// define functions...
int dot_open(struct inode *minode, struct file *mfile) 
{
	if(dot_usage != 0) return -EBUSY;
	
	dot_ioremap=(unsigned long)ioremap(DOT_PHY_ADDR,DOT_ADDR_RANGE);
	
	dot_row_addr =(unsigned short *)(dot_ioremap+0x40);
	dot_col_addr =(unsigned short *)(dot_ioremap+0x42);
	*dot_row_addr =0;
	*dot_col_addr =0;
	
	if(!check_mem_region(dot_ioremap, DOT_ADDR_RANGE)) {
		request_mem_region(dot_ioremap, DOT_ADDR_RANGE, DOT_NAME);
	}
	else	printk("driver: unable to register this!\n");
	
	dot_usage = 1;
	return 0;
}

int dot_release(struct inode *minode, struct file *mfile) 
{
	iounmap((unsigned long*)dot_ioremap);

	release_mem_region(dot_ioremap, DOT_ADDR_RANGE);
	dot_usage = 0;
	return 0;
}

int htoi(const char hexa)
{
	int ch = 0;
	if('0' <= hexa && hexa <= '9')
		ch = hexa - '0';
	if('A' <= hexa && hexa <= 'F')
		ch = hexa - 'A' + 10;
	if('a' <= hexa && hexa <= 'f')
		ch = hexa - 'a' + 10;
	return ch;
} 

ssize_t dot_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what)
{
	int ret=0, i;
	char buf[20];
	unsigned short result[10] = { 0 };
	unsigned int init=0x001; //Scan value
	unsigned int n1, n2;

	ret = copy_from_user(&buf, gdata, length);
	if(ret<0) return -1;
	
	for (i=0; i < 10; i++) {
		n1 = htoi( buf[2*i] );
		n2 = htoi( buf[2*i+1] );
		result[i] = n1*16+n2;
		*dot_row_addr = init << i;
		*dot_col_addr = 0x8000 | result[ i ];
		m_delay(3);
	}
	return length;
}

struct file_operations dot_fops = {
	.owner		= THIS_MODULE,
	.write		= dot_write,
	.open		= dot_open,
	.release	= dot_release,
};

static struct miscdevice dot_driver = {
	.fops	= &dot_fops,
	.name	= DOT_NAME,
	.minor	= MISC_DYNAMIC_MINOR,
};

int dot_init(void) 
{
	misc_register(&dot_driver);
	return 0;
}

void dot_exit(void) 
{
	misc_deregister(&dot_driver);
	printk("driver: %s DRIVER EXIT\n", DOT_NAME);
}

module_init(dot_init);		
module_exit(dot_exit);		

MODULE_AUTHOR(DRIVER_AUTHOR); 	
MODULE_DESCRIPTION(DRIVER_DESC); 
MODULE_LICENSE("Dual BSD/GPL");	 
