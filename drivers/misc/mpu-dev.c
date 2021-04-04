/*
 $License:
    Copyright (C) 2010 InvenSense Corporation, All Rights Reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
  $
 */
 
#include <linux/platform_device.h> //KSJ
#include <linux/input.h> //KSJ
#include <linux/types.h> //KSJ
#include <linux/delay.h> //KSJ
#include <linux/workqueue.h> //KSJ
#include <asm/fcntl.h>
#include <linux/ioport.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>
#include <linux/pwm.h>
// KSJ


#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#if 1
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/signal.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/pm.h>
#include <linux/mutex.h>
#include <linux/suspend.h>
#include <linux/poll.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#endif

#define MPU3050_CHIP_ID_REG     0x00
#define MPU3050_CHIP_ID         0x69
#define MPU3050_CHIP_ID1        0x68
#define MPU3050_XOUT_H          0x1D
#define MPU3050_X_OFFS_USRH     0x0C
#define MPU3050_X_OFFS_USRL     0x0D
#define MPU3050_Y_OFFS_USRH     0x0E
#define MPU3050_Y_OFFS_USRL     0x0F
#define MPU3050_Z_OFFS_USRH     0x10
#define MPU3050_Z_OFFS_USRL     0x11
#define MPU3050_SMPLRT_DIV		0x15
#define MPU3050_DLPF_FS_SYNC	0x16
#define MPU3050_INT_CFG	        0x17
#define MPU3050_PWR_MGM         0x3E
#define MPU3050_PWR_MGM_POS     6
#define MPU3050_PWR_MGM_MASK    0x40

#define MPU3050_AUTO_DELAY      1000

#define MPU3050_MIN_VALUE       -32768
#define MPU3050_MAX_VALUE       32767


struct axis_data {
        s16 x;
        s16 y;
        s16 z;
};

struct mpu3050_sensor {
        struct i2c_client *client;
        struct device *dev;
        struct input_dev *idev;
		atomic_t enable; //KSJ
		atomic_t delay;  //KSJ
		struct mutex enable_mutex; //KSJ
		struct mutex mutex; //KSJ
		struct input_dev *input; //KSJ
		struct delayed_work work; //KSJ
		//struct completion completion; //KSJ
		
};

static struct mpu3050_sensor *mpu_ptr; 


static unsigned int mpu3050_poll(struct file *file, struct poll_table_struct *poll)
{
	return 0;
}

/* ioctl - I/O control */
static long mpu3050_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return 0;
}

/* close function - called when the "file" /dev/mpu is closed in userspace   */
static int mpu3050_release(struct inode *inode, struct file *file)
{

	return 0;
}


static int mpu3050_xyz_read_reg(struct i2c_client *client,
                               u8 *buffer, int length)
{
        /*
         * Annoying we can't make this const because the i2c layer doesn't
         * declare input buffers const.
         */
        char cmd = MPU3050_XOUT_H;
        struct i2c_msg msg[] = {
                {
                        .addr = MPU3050_CHIP_ID1,
                        .flags = 0,
                        .len = 1,
                        .buf = &cmd,
                },
                {
                        .addr = MPU3050_CHIP_ID1,
                        .flags = I2C_M_RD,
                        .len = length,
                        .buf = buffer,
                },
        };

        return i2c_transfer(client->adapter, msg, 2);
}

static int mpu3050_i2c_write(struct i2c_adapter *i2c_adap,
		     unsigned char address,
		     unsigned int len, unsigned char const *data)
{
        struct i2c_msg msgs[1];
		int res;

		if (NULL == data || NULL == i2c_adap)
			return -EINVAL;

		msgs[0].addr = address;
		msgs[0].flags = 0;	/* write */
		msgs[0].buf = (unsigned char *) data;
		msgs[0].len = len;

		return i2c_transfer(i2c_adap, msgs, 1);
}

static int mpu3050_i2c_write_reg(struct i2c_adapter *i2c_adap,
			      unsigned char address,
			      unsigned char reg, unsigned char value)
{
        unsigned char data[2];

		data[0] = reg;
		data[1] = value;
		return mpu3050_i2c_write(i2c_adap, address, 2, data);
}


static void mpu3050_read_xyz(struct i2c_client *client,
                             struct axis_data *coords)
{
        u16 buffer[3];
		
        mpu3050_xyz_read_reg(client, (u8 *)buffer, 6);
        coords->x = be16_to_cpu(buffer[0]);
        coords->y = be16_to_cpu(buffer[1]);
        coords->z = be16_to_cpu(buffer[2]);
        dev_dbg(&client->dev, "%s: x %d, y %d, z %d\n", __func__,
                                       coords->x, coords->y, coords->z);
		//printk("//KSJ mpu3050_read_xyz\n");
}

static void mpu3050_set_power_mode(struct i2c_client *client, u8 val)
{
        u8 value;
		//printk("//KSJ mpu3050_set_power_mode\n");
        value = i2c_smbus_read_byte_data(client, MPU3050_PWR_MGM);
        value = (value & ~MPU3050_PWR_MGM_MASK) |
                (((val << MPU3050_PWR_MGM_POS) & MPU3050_PWR_MGM_MASK) ^
                 MPU3050_PWR_MGM_MASK);
        i2c_smbus_write_byte_data(client, MPU3050_PWR_MGM, value);
}

static int mpu3050_input_open(struct input_dev *input)
{
        struct mpu3050_sensor *sensor = input_get_drvdata(input);
		//printk("//KSJ mpu3050_input_open\n");
        pm_runtime_get(sensor->dev);

        return 0;
}

static void mpu3050_input_close(struct input_dev *input)
{
        struct mpu3050_sensor *sensor = input_get_drvdata(input);
		//printk("//KSJ mpu3050_input_close\n");
        pm_runtime_put(sensor->dev);
}

static irqreturn_t mpu3050_interrupt_thread(int irq, void *data)
{
        struct mpu3050_sensor *sensor = data;
        struct axis_data axis;

        mpu3050_read_xyz(sensor->client, &axis);

		//printk("//KSJ %d %d %d\n", axis.x, axis.y, axis.z);
		
        input_report_abs(sensor->idev, REL_RX, axis.x);
        input_report_abs(sensor->idev, REL_RY, axis.y);
        input_report_abs(sensor->idev, REL_RZ, axis.z);
        input_sync(sensor->idev);

        return IRQ_HANDLED;
}

static void mpu3050_read(int irq, void *data)
{
        struct mpu3050_sensor *sensor = data;
        struct axis_data axis;

        mpu3050_read_xyz(sensor->client, &axis);

		//printk("//KSJ %d %d %d\n", axis.x, axis.y, axis.z);
		
        input_report_abs(sensor->idev, REL_RX, axis.x);
        input_report_abs(sensor->idev, REL_RY, axis.y);
        input_report_abs(sensor->idev, REL_RZ, axis.z);
        input_sync(sensor->idev);
}


// ~add KSJ driver_selftest

//add KSJ(mpu_set_enable)
#define delay_to_jiffies(d) ((d)?msecs_to_jiffies(d):1)
#define actual_delay(d)     (jiffies_to_msecs(delay_to_jiffies(d)))

static void mpu_work_func(struct work_struct *work)
{
	//printk("//KSJ mpu_work_func\n");

        struct mpu3050_sensor *mpu = container_of((struct delayed_work *)work, struct mpu3050_sensor, work);
        unsigned long delay = delay_to_jiffies(atomic_read(&mpu->delay));
		struct axis_data axis;
		unsigned char sensor_data[64];
		
		mpu3050_read_xyz(mpu->client, &axis);
		//sensor_i2c_read(mpu->client->adapter,
		//input_report_rel(mpu->idev, REL_RX, 0);
		//input_report_rel(mpu->idev, REL_RY, 0);
		//input_report_rel(mpu->idev, REL_RZ, 0);
		input_report_rel(mpu->idev, REL_RX, axis.x);
        input_report_rel(mpu->idev, REL_RY, axis.y);
        input_report_rel(mpu->idev, REL_RZ, axis.z);
		input_sync(mpu->idev);

        mutex_lock(&mpu->mutex);
        //mpu->distance = distance;
        mutex_unlock(&mpu->mutex);

        schedule_delayed_work(&mpu->work, delay);

}

/*
 * Input device interface
 */

static int mpu_input_init(struct mpu3050_sensor *mpu)
{
        struct input_dev *dev;
        int err;
		int ret;
//		printk("//KSJ mpu_input_init\n");

        dev = input_allocate_device();
        if (!dev)
                return -ENOMEM;

        dev->name = "mpu3050";
//JNJ        dev->id.bustype = 0;
		dev->open = mpu3050_input_open;
        dev->close = mpu3050_input_close;
		
        set_bit(EV_REL, dev->evbit);
        set_bit(EV_SYN, dev->evbit);

		/* X */
		input_set_capability(dev, EV_REL, REL_RX);
		input_set_abs_params(dev, REL_RX, MPU3050_MIN_VALUE, MPU3050_MAX_VALUE, 0, 0);
		/* Y */
		input_set_capability(dev, EV_REL, REL_RY);
		input_set_abs_params(dev, REL_RY, MPU3050_MIN_VALUE, MPU3050_MAX_VALUE, 0, 0);
		/* Z */
		input_set_capability(dev, EV_REL, REL_RZ);
		input_set_abs_params(dev, REL_RZ, MPU3050_MIN_VALUE, MPU3050_MAX_VALUE, 0, 0);	
        input_set_drvdata(dev, mpu);

        err = input_register_device(dev);
        if (err < 0) {
                input_free_device(dev);
                return err;
        }
        mpu->idev = dev;
		
        return 0;
}

static void mpu_input_fini(struct mpu3050_sensor *mpu)
{
        struct input_dev *dev = mpu->idev;

        input_unregister_device(dev);
        input_free_device(dev);
}

static void mpu_register_set(struct mpu3050_sensor *mpu)
{

		mpu3050_i2c_write_reg(mpu->client->adapter, MPU3050_CHIP_ID1, MPU3050_SMPLRT_DIV, 0x4);
		mdelay(10);
		mpu3050_i2c_write_reg(mpu->client->adapter, MPU3050_CHIP_ID1, MPU3050_DLPF_FS_SYNC, 0x1b);
		mdelay(10);
		mpu3050_i2c_write_reg(mpu->client->adapter, MPU3050_CHIP_ID1, MPU3050_INT_CFG, 0x12);
		mdelay(10);
}


static void mpu_set_enable(int enable)
{
        struct mpu3050_sensor *mpu = mpu_ptr;
        int delay = atomic_read(&mpu->delay);
//		printk("//KSJ mpu_set_enable\n");


        mutex_lock(&mpu->enable_mutex);

        if (enable)
        {                   /* enable if state will be changed */
                if (!atomic_cmpxchg(&mpu->enable, 0, 1))
                {
                        schedule_delayed_work(&mpu->work,
                                              delay_to_jiffies(delay) + 1);
                }

				mpu_register_set(mpu);

        }
        else
        {                        /* disable if state will be changed */
                if (atomic_cmpxchg(&mpu->enable, 1, 0))
                {
                        cancel_delayed_work_sync(&mpu->work);
                }
        }
        atomic_set(&mpu->enable, enable);

        mutex_unlock(&mpu->enable_mutex);
}

static int mpu_get_enable()
{
        struct mpu3050_sensor *mpu = mpu_ptr;
//		printk("//KSJ mpu_get_enable\n");
        return 1;
}

static int mpu_get_delay()
{
        struct mpu3050_sensor *mpu = mpu_ptr;

        return atomic_read(&mpu->delay);
}

static void mpu_set_delay(int delay)
{
        int delay_ms;
        struct mpu3050_sensor *mpu = mpu_ptr;

        atomic_set(&mpu->delay, delay);

        mutex_lock(&mpu->enable_mutex);

        if (mpu_get_enable())
        {
                delay_ms = delay;
                cancel_delayed_work_sync(&mpu->work);
                schedule_delayed_work(&mpu->work,
                                      delay_to_jiffies(delay_ms) + 1);
        }

        mutex_unlock(&mpu->enable_mutex);
}

/*
 * sysfs device attributes
 */
static ssize_t mpu_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
        return sprintf(buf, "1\n");
}

static ssize_t mpu_enable_store(struct device *dev,
                                   struct device_attribute *attr,
                                   const char *buf, size_t count)
{
        unsigned long enable = simple_strtoul(buf, NULL, 10);
//		printk("//KSJ mpu_enable_store %d\n", enable);
        if ((enable == 0) || (enable == 1)){
           mpu_set_enable(enable);
        }

        return count;
}

static ssize_t mpu_delay_show(struct device *dev,
                                 struct device_attribute *attr, char *buf)
{
        return sprintf(buf, "%d\n", mpu_get_delay());
}

static ssize_t mpu_delay_store(struct device *dev,
                                  struct device_attribute *attr,
                                  const char *buf, size_t count)
{
        unsigned long delay = simple_strtoul(buf, NULL, 10);

        mpu_set_delay(delay);

        return count;
}

static ssize_t mpu_wake_store(struct device *dev,
                                 struct device_attribute *attr,
                                 const char *buf, size_t count)
{
        struct input_dev *input = to_input_dev(dev);
        static atomic_t serial = ATOMIC_INIT(0);
		//printk("//KSJ mpu_wake_store\n");

        input_report_abs(input, ABS_MISC, atomic_inc_return(&serial));

        return count;
}

static ssize_t mpu_data_show(struct device *dev,
                                struct device_attribute *attr, char *buf)
{
        struct mpu3050_sensor *mpu = mpu_ptr;
		int distance;
		//printk("//KSJ mpu_data_show\n");

        mutex_lock(&mpu->mutex);
        mutex_unlock(&mpu->mutex);

        return sprintf(buf, "%d \n", distance);
}

static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP,
                   mpu_enable_show, mpu_enable_store);
static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP,
                   mpu_delay_show, mpu_delay_store);
static DEVICE_ATTR(wake, S_IWUSR|S_IWGRP,
                   NULL, mpu_wake_store);
static DEVICE_ATTR(data, S_IRUGO,
                   mpu_data_show, NULL);


static struct attribute *mpu_attributes[] = {
        &dev_attr_enable.attr,
        &dev_attr_delay.attr,
        &dev_attr_wake.attr,
        &dev_attr_data.attr,
        NULL
};

static struct attribute_group mpu_attribute_group = {
        .attrs = mpu_attributes
};

/* define which file operations are supported */
static const struct file_operations mpu_fops = {
	.owner = THIS_MODULE,
	.read = mpu3050_read_xyz,
	.poll = mpu3050_poll,

#if HAVE_COMPAT_IOCTL
	.compat_ioctl = mpu3050_ioctl,
#endif
#if HAVE_UNLOCKED_IOCTL
	.unlocked_ioctl = mpu3050_ioctl,
#endif
	.open = mpu3050_input_open,
	.release = mpu3050_release,
};

static struct miscdevice i2c_mpu_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mpu3050", /* Same for both 3050 and 6000 */
	.fops = &mpu_fops,
};

// ~add KSJ
static int __devinit mpu3050_probe(struct i2c_client *client,
                                   const struct i2c_device_id *id)
{
        struct mpu3050_sensor *sensor;
        struct input_dev *idev;
        int ret;
        int error;

//		printk("//KSJ mpu3050_probe\n");
        sensor = kzalloc(sizeof(struct mpu3050_sensor), GFP_KERNEL);
        idev = input_allocate_device();
        if (!sensor || !idev) {
                dev_err(&client->dev, "failed to allocate driver data\n");
                error = -ENOMEM;
                goto err_free_mem;
        }

        sensor->client = client;
        sensor->dev = &client->dev;
        sensor->idev = idev;

        mpu3050_set_power_mode(client, 1);
        msleep(10);

        ret = i2c_smbus_read_byte_data(client, MPU3050_CHIP_ID_REG);
        if (ret < 0) {
                dev_err(&client->dev, "failed to detect device\n");
                error = -ENXIO;
                goto err_free_mem;
        }

        if (ret != MPU3050_CHIP_ID) {
                dev_err(&client->dev, "unsupported chip id\n");
                error = -ENXIO;
                goto err_free_mem;
        }

		mutex_init(&sensor->mutex);	//KSJ
		mutex_init(&sensor->enable_mutex); //KSJ
	
		INIT_DELAYED_WORK(&sensor->work, mpu_work_func); //KSJ
		ret = mpu_input_init(sensor);//KSJ
		sysfs_create_group(&sensor->idev->dev.kobj, &mpu_attribute_group);//KSJ
		
		mpu_ptr = sensor;	//KSJ
	
    	mpu_set_delay(100);	//KSJ
        
		error = misc_register(&i2c_mpu_device); //KSJ

		if (error) {
//				printk("//KSJ device error\n");
                dev_err(&client->dev, "failed to register input device\n");
                goto err_mpuirq_failed;
        }

        return 0;
		
err_mpuirq_failed:
		misc_deregister(&i2c_mpu_device);
err_free_irq:
        free_irq(client->irq, sensor);
err_pm_set_suspended:
        pm_runtime_set_suspended(&client->dev);
err_free_mem:
        input_free_device(idev);
        kfree(sensor);
        return error;
}

static int __devexit mpu3050_remove(struct i2c_client *client)
{
		struct mpu3050_sensor *sensor = i2c_get_clientdata(client);

        pm_runtime_disable(&client->dev);
        pm_runtime_set_suspended(&client->dev);

        free_irq(client->irq, sensor);
        input_unregister_device(sensor->idev);
		misc_deregister(&i2c_mpu_device);
        kfree(sensor);

        return 0;
}

#ifdef CONFIG_PM

static int mpu3050_suspend(struct device *dev)
{
        struct i2c_client *client = to_i2c_client(dev);

        mpu3050_set_power_mode(client, 0);

        return 0;
}

static int mpu3050_resume(struct device *dev)
{
        struct i2c_client *client = to_i2c_client(dev);

        mpu3050_set_power_mode(client, 1);
        msleep(100);  /* wait for gyro chip resume */

        return 0;
}
#endif

static UNIVERSAL_DEV_PM_OPS(mpu3050_pm, mpu3050_suspend, mpu3050_resume, NULL);

static const struct i2c_device_id mpu3050_ids[] = {
       { "mpu3050", 0 },
       { }
};
MODULE_DEVICE_TABLE(i2c, mpu3050_ids);

static struct i2c_driver mpu3050_i2c_driver = {
        .driver = {
                .name   = "mpu3050",
                .owner  = THIS_MODULE,
                .pm     = &mpu3050_pm,
        },
        .probe          = mpu3050_probe,
        .remove         = __devexit_p(mpu3050_remove),
        .id_table       = mpu3050_ids,
};

static int __init mpu3050_init(void)
{
        return i2c_add_driver(&mpu3050_i2c_driver);
}
module_init(mpu3050_init);

static void __exit mpu3050_exit(void)
{
        i2c_del_driver(&mpu3050_i2c_driver);
}
module_exit(mpu3050_exit);

MODULE_AUTHOR("Wistron Corp.");
MODULE_DESCRIPTION("MPU3050 Tri-axis gyroscope driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("mpu3050");

