/* Copyright (c) 2011 Samsung Electronics Co, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h> 
#include <linux/delay.h>
#include <linux/platform_device.h> 
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <asm/irq.h>

#include <mach/regs-tmu.h>
#include <plat/s5p-tmu.h>
#include <plat/watchdog-reset.h>

//#define TRIGGER_LEV0 5	//90c
#define TRIGGER_LEV0 14	//99c


#define    TRIGGER_LEV1 45  //130c --> reset
#define    TRIGGER_LEV2 0xFF   
#define    TRIGGER_LEV3 0xFF

#define INIT_THRESHOLD 80

#define VREF_SLOPE     0x07000F02
#define TMU_EN         0x1

static struct resource *s5p_tmu_mem;

static int trigger0no = NO_IRQ;
static int trigger1no = NO_IRQ;
static int trigger2no = NO_IRQ;
	
struct timer_list  tmu_timer;

void  s5p_tmu_timmer_monitoring(unsigned long arg)
{
   struct tmu_platform_device *dev = arg;
   char cur_temp;
   cur_temp = __raw_readb(dev->tmu_base + CURRENT_TEMP) - dev->data.dc_temp;

	if(cur_temp > 95)
	   printk(KERN_DEBUG "Current Temperature : %dc!!\n", cur_temp);

	if(cur_temp < 100) dev->dvfs_flag = 0;
	else  dev->dvfs_flag = 1;
	
	//
 	tmu_timer.expires = get_jiffies_64() + 2*HZ;
	add_timer(&tmu_timer);
}

int tmu_timmer_start(struct platform_device *pdev)
{
	struct tmu_platform_device *tmu_dev = platform_get_drvdata(pdev);
   printk(KERN_INFO "tmu_timer_start.\n");

	init_timer(&tmu_timer);
	tmu_timer.expires = get_jiffies_64() + 2*HZ;
	tmu_timer.data = tmu_dev;
	tmu_timer.function = s5p_tmu_timmer_monitoring;

	add_timer(&tmu_timer);

    return 0;
}
void tmu_timmer_stop(void)
{
	   printk(KERN_INFO "tmu_timer_stop.\n");
	del_timer(&tmu_timer);
}

static void tmu_initialize(struct platform_device *pdev)
{
   struct tmu_platform_device *tmu_dev = platform_get_drvdata(pdev);
   unsigned int en;
   unsigned int te_temp;
   char te1,te2;
   char t_new;

   tmu_dev->dvfs_flag = 0;

   en = (__raw_readb(tmu_dev->tmu_base + TMU_STATUS) && 0x1);

   while(!en)
       printk(KERN_INFO "TMU is busy..!! wating......\n");

   /* Temperature compensation */
   te_temp = __raw_readl(tmu_dev->tmu_base + TRIMINFO);
   te1 = te_temp & TRIM_TEMP_MASK;
   te2 = ((te_temp >> 8) & TRIM_TEMP_MASK);

   if(tmu_dev->data.mode)
       t_new = ((INIT_THRESHOLD - 25) * (te2 - te1) / ((85 - 25) + te1));
   else
       t_new = INIT_THRESHOLD + (te1 - 25);

   __raw_writeb(t_new, tmu_dev->tmu_base + THRESHOLD_TEMP);

   /* Need to initail regsiter setting after getting parameter info */
   
   /* [28:23] vref [11:8] slope - Tunning parameter */
   __raw_writel(VREF_SLOPE, tmu_dev->tmu_base + TMU_CON0);

   __raw_writeb(TRIGGER_LEV0, tmu_dev->tmu_base + TRG_LEV0);
   __raw_writeb(TRIGGER_LEV1, tmu_dev->tmu_base + TRG_LEV1);
   __raw_writeb(TRIGGER_LEV2, tmu_dev->tmu_base + TRG_LEV2);
   __raw_writeb(TRIGGER_LEV3, tmu_dev->tmu_base + TRG_LEV3);

   printk(KERN_INFO "Threshold temp: %dc   Thortting temp: %dc   Trimming temp : %dc   Cooling temp: %d\n",	\
       t_new, t_new + __raw_readb(tmu_dev->tmu_base + TRG_LEV0),           \
       t_new + __raw_readb(tmu_dev->tmu_base + TRG_LEV1),tmu_dev->data.t1);  // ??
}  

static void tmu_start(struct platform_device *pdev)
{
   struct tmu_platform_device *tmu_dev = platform_get_drvdata(pdev);
   unsigned int con;
   char cur_temp;

   __raw_writel(0x1111, tmu_dev->tmu_base + INTCLEAR); 
   
   /* TMU core enable */
   con = __raw_readl(tmu_dev->tmu_base + TMU_CON0);
   con |= TMU_EN;
   
   __raw_writel(con, tmu_dev->tmu_base + TMU_CON0); 

   /*LEV0 LEV1 interrupt enable */
   __raw_writel(INTEN0 | INTEN1, tmu_dev->tmu_base + INTEN);

   /* It needs to delay after enable for over 100us */
   mdelay(5);
   
   cur_temp = __raw_readb(tmu_dev->tmu_base + CURRENT_TEMP) - tmu_dev->data.dc_temp;
   
   printk(KERN_INFO "Current Temperature : %dc\n", cur_temp);
}

static irqreturn_t s5p_tmu_throtting_irq(int irq, void *id)
{
   struct  tmu_platform_device *dev = id;
   char cur_temp;

   printk(KERN_INFO "Throtting interrupt occured!!!!\n");
   __raw_writel(INTCLEAR0, dev->tmu_base + INTCLEAR);
   
   cur_temp = __raw_readb(dev->tmu_base + CURRENT_TEMP) - dev->data.dc_temp;
   printk(KERN_INFO "Current Temperature : %dc\n", cur_temp);
  
   dev->dvfs_flag = 1;

   disable_irq_nosync(irq);
   return IRQ_HANDLED;
}

static irqreturn_t s5p_tmu_trimming_irq(int irq, void *id)
{
   struct tmu_platform_device *dev = id;
   char cur_temp;

   printk(KERN_INFO "Trimming interrupt occured!!!!\n");
   __raw_writel(INTCLEAR1, dev->tmu_base + INTCLEAR);

   cur_temp = __raw_readb(dev->tmu_base + CURRENT_TEMP) - dev->data.dc_temp;
  printk(KERN_INFO "Current Temperature : %dc\n", cur_temp);

  disable_irq_nosync(irq);

  printk( "Temperature is too High: %dc It will be reset!!!!!!!\n", cur_temp);
 mdelay(500);

   /* Temperature is higher than T2, Start watchdog */
//   arch_wdt_reset();

   return IRQ_HANDLED;
}


static int __devinit s5p_tmu_probe(struct platform_device *pdev)
{
   struct tmu_platform_device *tmu_dev = platform_get_drvdata(pdev);
   struct resource *res;
   int ret;

   pr_debug("%s: probe=%p\n", __func__, pdev);

   trigger0no = platform_get_irq(pdev, 0);
   if (trigger0no < 0) {
       dev_err(&pdev->dev, "no irq for thermal throtting \n");
       return -ENOENT;
   }
                                   
   trigger1no = platform_get_irq(pdev, 1);
   if (trigger1no < 0) {
   dev_err(&pdev->dev, "no irq for thermal tripping\n");
   return -ENOENT;
   }

   res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
   if (res == NULL) {
   dev_err(&pdev->dev, "failed to get memory region resource\n");
   return -ENOENT;
    }

   s5p_tmu_mem = request_mem_region(res->start,
                   res->end-res->start+1,
                   pdev->name);
   if (s5p_tmu_mem == NULL) {
       dev_err(&pdev->dev, "failed to reserve memory region\n");
       ret = -ENOENT;
       goto err_nores;
   }
   
   tmu_dev->tmu_base = ioremap(res->start, (res->end - res->start) + 1);
   if (tmu_dev->tmu_base == NULL) {
       dev_err(&pdev->dev, "failed ioremap()\n");
       ret = -EINVAL;
       goto err_nomap;
   }

   ret = request_irq(trigger0no, s5p_tmu_throtting_irq,
           IRQF_DISABLED,  "s5p-tmu throtting", tmu_dev);
   if (ret) {
       dev_err(&pdev->dev, "IRQ%d error %d\n", trigger0no, ret);
       goto err_throtting;
   }

   ret = request_irq(trigger1no, s5p_tmu_trimming_irq,
           IRQF_DISABLED,  "s5p-tmu trimming", tmu_dev);

   if (ret) {
       dev_err(&pdev->dev, "IRQ%d error %d\n", trigger1no, ret);
       goto err_trimming;
   }

   tmu_initialize(pdev);
   tmu_start(pdev);
   tmu_timmer_start(pdev);

   return 0;

err_nomap:
    release_resource(s5p_tmu_mem);

err_throtting:
   free_irq(trigger0no, tmu_dev);

err_trimming:
   free_irq(trigger1no, tmu_dev);

err_nores:
   return ret;
}

#ifdef CONFIG_PM
static int s5p_tmu_suspend(struct platform_device *pdev, pm_message_t state)
{
	tmu_timmer_stop();

   return 0;
}

static int s5p_tmu_resume(struct platform_device *pdev)
{
	 tmu_timmer_start(pdev);
   return 0;
}

#else
#define s5p_tmu_suspend    NULL
#define s5p_tmu_resume NULL
#endif
 
static struct platform_driver s5p_tmu_driver = {
   .probe      = s5p_tmu_probe,
   .suspend    = s5p_tmu_suspend,  
   .resume     = s5p_tmu_resume,
   .driver     = {
       .name   =   "s5p-tmu",
       .owner  =   THIS_MODULE,
       },
};

static int __init s5p_tmu_driver_init(void)
{
   printk(KERN_INFO "TMU driver is loaded....!!\n");
   return platform_driver_register(&s5p_tmu_driver);
}

static void __exit s5p_tmu_driver_exit(void)
{  
	tmu_timmer_stop();
   platform_driver_unregister(&s5p_tmu_driver);
}

module_init(s5p_tmu_driver_init);
module_exit(s5p_tmu_driver_exit);

