/*
 * Keypad Device Driver
 *  Hanback Electronics Co.,ltd
 * File : keypad_fpga.c
 * Date : October,2010
 */ 

#include <linux/init.h>
#include <linux/module.h>
#include <mach/hardware.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <asm/ioctl.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/input.h>

#define DRIVER_AUTHOR 		  "Hanback Electronics"
#define DRIVER_DESC	 	      "KEYPAD program"

#define KEYPAD_NAME 		  "fpga-keypad"	
#define KEYPAD_PHY_ADDR		  0x05000000
#define KEYPAD_ADDR_RANGE 	0x1000
#define TIMER_INTERVAL		  35		

static struct timer_list mytimer;
static unsigned short value;
static unsigned long  keypad_ioremap;
static unsigned char *keypad_row_addr,*keypad_col_addr;
static unsigned short *keypad_check_addr;

#if defined(CONFIG_ANDROID)
char keypad_fpga_keycode[16] = {
  KEY_1,KEY_2,KEY_3,61,
  KEY_4,KEY_5,KEY_6,62,
  KEY_7,KEY_8,KEY_9,KEY_CAMERA,
  227,KEY_0,228,KEY_BACKSPACE
};
#else
char keypad_fpga_keycode[16] = {
  1,2,3,4,
  5,6,7,8,
  9,10,11,12,
  13,14,15,16
};
#endif

static struct input_dev *idev;
void mypollingfunction(unsigned long data);

int keypad_open(struct inode *minode, struct file *mfile)
{
  keypad_col_addr = (unsigned char *)(keypad_ioremap+0x70);
  keypad_row_addr = (unsigned char *)(keypad_ioremap+0x72);

  init_timer(&mytimer);

  mytimer.expires = get_jiffies_64() + TIMER_INTERVAL;
  mytimer.function = &mypollingfunction;

  add_timer(&mytimer);

  return 0;
}

int keypad_release(struct inode *minode, struct file *mfile)
{
	del_timer(&mytimer);
	
	return 0;
}

void mypollingfunction(unsigned long data)
{
    int j=1,k,i;
    int funtion_key = 0;

    unsigned char tmp[4] = {0x01, 0x02, 0x04, 0x08};

    value =0;
    for(i=0;i<4;i++) {
      *keypad_row_addr = tmp[i];
      value = *keypad_col_addr & 0x0f;
      if(value > 0) {
        for(k=0;k<4;k++) {
          if(value == tmp[k])  {
            value = j+(i*4);
            funtion_key =   keypad_fpga_keycode[value-1];
            if(value != 0x00) goto stop_poll;
          }
          j++;
        }
      }
    }

stop_poll:
    if(value > 0) {
      if(*keypad_check_addr == 0x2a) {
        input_report_key(idev, funtion_key ,1);
        input_report_key(idev, funtion_key ,0);
        input_sync(idev);
        //      kill_pid(id,SIGUSR1,1);
      }
    }
    else
      *keypad_row_addr = 0x00;

    mytimer.expires = get_jiffies_64() + TIMER_INTERVAL;
    add_timer(&mytimer);
}

ssize_t keypad_read(struct file *inode, char *gdata, size_t length, loff_t *off_what)
{
	int ret;

	ret=copy_to_user(gdata, &value, 1);
	if(ret<0) return -1;
	
	return length;
}

struct file_operations keypad_fops = {
  .owner		= THIS_MODULE,
  .open		  = keypad_open,
  .read		  = keypad_read,
  .release	= keypad_release,
};

int keypad_init(void) 
{
  int result;
  int i = 0;

	keypad_ioremap=(unsigned long)ioremap(KEYPAD_PHY_ADDR,KEYPAD_ADDR_RANGE);

	if(!check_mem_region(keypad_ioremap, KEYPAD_ADDR_RANGE)) {
		request_mem_region(keypad_ioremap, KEYPAD_ADDR_RANGE, KEYPAD_NAME);
	}
	else {
		printk("FPGA KEYPAD Memory Alloc Faild!\n");
    return -1;
  }

  keypad_check_addr = (unsigned short *)(keypad_ioremap+0x92);

  if(*keypad_check_addr != 0x2a) {
    printk("HBE-SM5-S4210 M3 Board is not exist...,0x%x\r\n",*keypad_check_addr);
	  iounmap((unsigned long*)keypad_ioremap);
	  release_region(keypad_ioremap, KEYPAD_ADDR_RANGE);
    return -1;
  }

  idev = input_allocate_device();

  set_bit(EV_KEY,idev->evbit);
  set_bit(EV_KEY,idev->keybit);

  for(i = 0; i < 30; i++)
    set_bit(i & KEY_MAX,idev->keybit);

  idev->name              = "fpga-keypad";
  idev->id.vendor         = 0x1002;
  idev->id.product        = 0x1002;
  idev->id.bustype        = BUS_HOST;
  idev->phys              = "keypad/input1";
  idev->open              = keypad_open;
  idev->close             = keypad_release;

  result = input_register_device(idev);

  if(result < 0) {
    printk(KERN_WARNING"FPGA KEYPAD Register Faild... \n");
    return result;
  }

	return 0;
}

void keypad_exit(void) 
{
	iounmap((unsigned long*)keypad_ioremap);
	release_region(keypad_ioremap, KEYPAD_ADDR_RANGE);

  input_unregister_device(idev);
}

module_init(keypad_init);
module_exit(keypad_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("Dual BSD/GPL");
