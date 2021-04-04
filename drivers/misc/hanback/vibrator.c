/* drivers/misc/vibrator.c
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

#define DRIVER_AUTHOR           "Hanback"
#define DRIVER_DESC             "HBE-SM5-S4210 Vibrator"
#define VIB_NAME                "vibdev"

// Global variable


static int vib_open(struct inode *minode, struct file *mfile)
{
        int err;

        err = gpio_request(S5PV310_GPC0(4), "GPC0(4)");
        if (err)
                printk("#### vibdev.c failed to request GPG2_2 ####\n");

        s3c_gpio_setpull(S5PV310_GPC0(4), S3C_GPIO_PULL_NONE);
        gpio_direction_output(S5PV310_GPC0(4), 0);

        return 0;
}

static int vib_release(struct inode *minode, struct file *mfile)
{
        gpio_free(S5PV310_GPC0(4));
        return 0;
}

static int vib_ioctl(struct inode *inode, struct file *flip, unsigned int cmd, unsigned long arg)
{
        if(cmd !=0 ) {
                gpio_direction_output(S5PV310_GPC0(4), 1);
                mdelay(cmd);
        }

        gpio_direction_output(S5PV310_GPC0(4), 0);

        return 0;
}

struct file_operations vib_fops = {
	.owner	= THIS_MODULE,
	.open = vib_open,
	.release = vib_release,
	.ioctl = vib_ioctl,
};


static struct miscdevice vib_driver = {
	.fops	= &vib_fops,
	.name = VIB_NAME,
	.minor = MISC_DYNAMIC_MINOR,
};


int vib_init(void)
{
        printk("driver: %s DRIVER INIT\n",VIB_NAME);
	return misc_register(&vib_driver);
}

void vib_exit(void)
{
	misc_deregister(&vib_driver);
        printk("driver: %s DRIVER EXIT\n",VIB_NAME);
}

module_init(vib_init);
module_exit(vib_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("Dual BSD/GPL");

