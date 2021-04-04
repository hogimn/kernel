/* linux/arch/arm/mach-s5pv310/dev-spi.c
 *
 * Copyright (C) 2010 Samsung Electronics Co. Ltd.
 *	Jaswinder Singh <jassi.brar@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>

#include <mach/irqs.h>
#include <mach/dma.h>
#include <mach/map.h>
#include <mach/gpio.h>
#include <mach/spi-clocks.h> 
#include <mach/regs-clock.h>

#include <plat/s3c64xx-spi.h> 
#include <plat/gpio-cfg.h>
#include <plat/irqs.h>



///
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/err.h>

#include <mach/map.h>

#include <plat/spi.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <mach/irqs.h>
#include <linux/slab.h>



static char *spi_src_clks[] = {
	[S5PV310_SPI_SRCCLK_SCLK] = "sclk_spi",
};

/* SPI Controller platform_devices */

/* Since we emulate multi-cs capability, we do not touch the CS.
 * The emulated CS is toggled by board specific mechanism, as it can
 * be either some immediate GPIO or some signal out of some other
 * chip in between ... or some yet another way.
 * We simply do not assume anything about CS.
 */
static int s5pv310_spi_cfg_gpio(struct platform_device *pdev)
{
	switch (pdev->id) {
	case 0:
		s3c_gpio_cfgpin(S5PV310_GPB(0), S3C_GPIO_SFN(2));
		s3c_gpio_cfgpin(S5PV310_GPB(1), S3C_GPIO_SFN(2));
		s3c_gpio_cfgpin(S5PV310_GPB(2), S3C_GPIO_SFN(2));
		s3c_gpio_cfgpin(S5PV310_GPB(3), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV310_GPB(0), S3C_GPIO_PULL_UP);
		s3c_gpio_setpull(S5PV310_GPB(2), S3C_GPIO_PULL_UP);
		s3c_gpio_setpull(S5PV310_GPB(3), S3C_GPIO_PULL_UP);
		break;

	case 1:
		s3c_gpio_cfgpin(S5PV310_GPB(4), S3C_GPIO_SFN(2));
		s3c_gpio_cfgpin(S5PV310_GPB(6), S3C_GPIO_SFN(2));
		s3c_gpio_cfgpin(S5PV310_GPB(7), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV310_GPB(4), S3C_GPIO_PULL_UP);
		s3c_gpio_setpull(S5PV310_GPB(6), S3C_GPIO_PULL_UP);
		s3c_gpio_setpull(S5PV310_GPB(7), S3C_GPIO_PULL_UP);
		break;

	case 2:
		s3c_gpio_cfgpin(S5PV310_GPC1(1), S3C_GPIO_SFN(5));
		s3c_gpio_cfgpin(S5PV310_GPC1(3), S3C_GPIO_SFN(5));
		s3c_gpio_cfgpin(S5PV310_GPC1(4), S3C_GPIO_SFN(5));
		s3c_gpio_setpull(S5PV310_GPC1(1), S3C_GPIO_PULL_UP);
		s3c_gpio_setpull(S5PV310_GPC1(3), S3C_GPIO_PULL_UP);
		s3c_gpio_setpull(S5PV310_GPC1(4), S3C_GPIO_PULL_UP);
		break;

	default:
		dev_err(&pdev->dev, "Invalid SPI Controller number!");
		return -EINVAL;
	}

	return 0;
}

static struct resource s5pv310_spi0_resource[] = {
	[0] = {
		.start = S5PV310_PA_SPI0,
		.end   = S5PV310_PA_SPI0 + 0x100 - 1,
		.flags = IORESOURCE_MEM,
	},
#if defined(CONFIG_SPI_S5PV310)
	[1] = {
		.start = IRQ_SPI0,
		.end   = IRQ_SPI0,
		.flags = IORESOURCE_IRQ,
	},	
#else
	[1] = {
		.start = DMACH_SPI0_TX,
		.end   = DMACH_SPI0_TX,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.start = DMACH_SPI0_RX,
		.end   = DMACH_SPI0_RX,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.start = IRQ_SPI0,
		.end   = IRQ_SPI0,
		.flags = IORESOURCE_IRQ,
	},
#endif	
};

static struct s3c64xx_spi_info s5pv310_spi0_pdata = {
	.cfg_gpio = s5pv310_spi_cfg_gpio,
	.fifo_lvl_mask = 0x1ff,
	.rx_lvl_offset = 15,
	.high_speed = 1,
	.clk_from_cmu = true,
};

static u64 spi_dmamask = DMA_BIT_MASK(32);

struct platform_device s5pv310_device_spi0 = {
	.name		  = "s3c64xx-spi",
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(s5pv310_spi0_resource),
	.resource	  = s5pv310_spi0_resource,
	.dev = {
		.dma_mask		= &spi_dmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
		.platform_data = &s5pv310_spi0_pdata,
	},
};

static struct resource s5pv310_spi1_resource[] = {
	[0] = {
		.start = S5PV310_PA_SPI1,
		.end   = S5PV310_PA_SPI1 + 0x100 - 1,
		.flags = IORESOURCE_MEM,
	},
#if defined(CONFIG_SPI_S5PV310)
	[1] = {
		.start = IRQ_SPI1,
		.end   = IRQ_SPI1,
		.flags = IORESOURCE_IRQ,
	},
#else
	[1] = {
		.start = DMACH_SPI1_TX,
		.end   = DMACH_SPI1_TX,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.start = DMACH_SPI1_RX,
		.end   = DMACH_SPI1_RX,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.start = IRQ_SPI1,
		.end   = IRQ_SPI1,
		.flags = IORESOURCE_IRQ,
	},
#endif
};

static struct s3c64xx_spi_info s5pv310_spi1_pdata = {
	.cfg_gpio = s5pv310_spi_cfg_gpio,
	.fifo_lvl_mask = 0x7f,
	.rx_lvl_offset = 15,
	.high_speed = 1,
	.clk_from_cmu = true,
};

struct platform_device s5pv310_device_spi1 = {
	.name		  = "s3c64xx-spi",
	.id		  = 1,
	.num_resources	  = ARRAY_SIZE(s5pv310_spi1_resource),
	.resource	  = s5pv310_spi1_resource,
	.dev = {
		.dma_mask		= &spi_dmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
		.platform_data = &s5pv310_spi1_pdata,
	},
};

static struct resource s5pv310_spi2_resource[] = {
	[0] = {
		.start = S5PV310_PA_SPI2,
		.end   = S5PV310_PA_SPI2 + 0x100 - 1,
		.flags = IORESOURCE_MEM,
	},
#if defined(CONFIG_SPI_S5PV310)
	[1] = {
		.start = IRQ_SPI2,
		.end   = IRQ_SPI2,
		.flags = IORESOURCE_IRQ,
	},
#else
	[1] = {
		.start = DMACH_SPI2_TX,
		.end   = DMACH_SPI2_TX,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.start = DMACH_SPI2_RX,
		.end   = DMACH_SPI2_RX,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.start = IRQ_SPI2,
		.end   = IRQ_SPI2,
		.flags = IORESOURCE_IRQ,
	},
#endif	
};


#if defined(CONFIG_SPI_S5PV310)
#include <plat/spi.h>


#if 1
#define dbg_printk(x...)	printk(x)
#else
#define dbg_printk(x...)	do{}while(0)
#endif

#if defined(CONFIG_SPICLK_EXT_MOUT_EPLL)
#define SPICLK_SRC "mout_epll"

#elif defined(CONFIG_SPICLK_EXT_MOUT_MPLL)
#define SPICLK_SRC "mout_mpll"

#elif defined(CONFIG_SPICLK_EXT_MOUT_VPLL)
#define SPICLK_SRC "mout_vpll"
#endif

static int smi_getclcks(struct s3c_spi_mstr_info *smi)
{
	struct clk *cspi, *cp, *cm, *cf;

	cp = NULL;
	cm = NULL;
	cf = NULL;
	cspi = smi->clk;

#if 1 // RAONTECH
//if(cspi == NULL)
{
	cspi = clk_get(&smi->pdev->dev, "sclk_spi");
	if(IS_ERR(cspi)){
		printk("Unable to get sclk_spi!\n");
		return -EBUSY;
	}
}

#else
	if(cspi == NULL){
		cspi = clk_get(&smi->pdev->dev, "spi");
		if(IS_ERR(cspi)){
			printk("Unable to get spi!\n");
			return -EBUSY;
		}
	}
#endif	
	
	dbg_printk("%s:%d Got clk=spi\n", __func__, __LINE__);

#if defined(CONFIG_SPICLK_EXT)
	cp = clk_get(&smi->pdev->dev, "spi-bus");
	if(IS_ERR(cp)){
		printk("Unable to get parent clock(%s)!\n", "spi-bus");
		if(smi->clk == NULL){
			clk_disable(cspi);
			clk_put(cspi);
		}
		return -EBUSY;
	}
	dbg_printk("%s:%d Got clk=%s\n", __func__, __LINE__, "spi-bus");

	cm = clk_get(&smi->pdev->dev, SPICLK_SRC);
	if(IS_ERR(cm)){
		printk("Unable to get %s\n", SPICLK_SRC);
		clk_put(cp);
		return -EBUSY;
	}
	dbg_printk("%s:%d Got clk=%s\n", __func__, __LINE__, SPICLK_SRC);
	if(clk_set_parent(cp, cm)){
		printk("failed to set %s as the parent of %s\n", SPICLK_SRC, "spi-bus");
		clk_put(cm);
		clk_put(cp);
		return -EBUSY;
	}
	dbg_printk("Set %s as the parent of %s\n", SPICLK_SRC, "spi-bus");
	
#if defined(CONFIG_SPICLK_EXT_MOUT_EPLL) /* MOUTepll through EPLL */
	cf = clk_get(&smi->pdev->dev, "fout_epll");
	if(IS_ERR(cf)){
		printk("Unable to get fout_epll\n");
		clk_put(cm);
		clk_put(cp);
		return -EBUSY;
	}
	dbg_printk("Got fout_epll\n");
	dbg_printk("clk-rate of fout_epll is %lu \n",clk_get_rate(cf));
	if(clk_set_parent(cm, cf)){
		printk("failed to set FOUTepll as parent of %s\n", SPICLK_SRC);
		clk_put(cf);
		clk_put(cm);
		clk_put(cp);
		return -EBUSY;
	}
	dbg_printk("Set FOUTepll as parent of %s\n", SPICLK_SRC);
	clk_put(cf);
#endif
	clk_put(cm);

	smi->prnt_clk = cp;
	
#endif
	smi->clk = cspi;
	return 0;
}

static void smi_putclcks(struct s3c_spi_mstr_info *smi)
{
//dbg_printk("[smi_putclcks] smi->prnt_clk: %p\n", smi->prnt_clk);
	if(smi->prnt_clk != NULL)
		clk_put(smi->prnt_clk);

	clk_put(smi->clk);
}

static int smi_enclcks(struct s3c_spi_mstr_info *smi)
{
//dbg_printk("[smi_enclcks] smi->prnt_clk: %p\n", smi->prnt_clk);

#if 1 // RAONTECH
	struct clk *src_clk;

#if 1
	src_clk = clk_get(&smi->pdev->dev, "spi");
	if(IS_ERR(src_clk)){
		printk("Unable to get spi!\n");
	//	return -EBUSY;
	}
	clk_enable(src_clk);
#endif	

	src_clk = clk_get(&smi->pdev->dev, "sclk_spi");
	if(IS_ERR(src_clk)){
		printk("Unable to get sclk_spi!\n");
		return -EBUSY;
	}
	clk_enable(src_clk);

	return 0;

#else
	if(smi->prnt_clk != NULL)
		clk_enable(smi->prnt_clk);

	return clk_enable(smi->clk);
#endif	
}

static void smi_disclcks(struct s3c_spi_mstr_info *smi)
{
	if(smi->prnt_clk != NULL)
		clk_disable(smi->prnt_clk);

	clk_disable(smi->clk);
}

static u32 smi_getrate(struct s3c_spi_mstr_info *smi)
{
	u32 ret;

//dbg_printk("[smi_getrate] smi->prnt_clk: %p, smi->clk: %p\n", smi->prnt_clk, smi->clk);

	if(smi->prnt_clk != NULL){
		ret = clk_get_rate(smi->prnt_clk);		
		return ret;
	}
	else{
		ret = clk_get_rate(smi->clk);

//dbg_printk("[smi_getrate] ret: %u\n", ret);		
		return ret;
	}
}

static int smi_setrate(struct s3c_spi_mstr_info *smi, u32 r)
{
 /* We don't take charge of the Src Clock, yet */
	return 0;
}

/* SPI (0) */
static struct resource s3c_spi0_resource[] = {
	[0] = {
		.start = S5PV310_PA_SPI0,
		.end   = S5PV310_PA_SPI0 + 0x100 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SPI0,
		.end   = IRQ_SPI0,
		.flags = IORESOURCE_IRQ,
	}
};

static struct s3c_spi_mstr_info sspi0_mstr_info = {
	.pdev = NULL,
	.clk = NULL,
	.prnt_clk = NULL,
	.num_slaves = 0,
	.spiclck_get = smi_getclcks,
	.spiclck_put = smi_putclcks,
	.spiclck_en = smi_enclcks,
	.spiclck_dis = smi_disclcks,
	.spiclck_setrate = smi_setrate,
	.spiclck_getrate = smi_getrate,
};

static u64 s3c_device_spi0_dmamask = 0xffffffffUL;

struct platform_device s3c_device_spi0 = {
	.name		= "s3c-spi",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(s3c_spi0_resource),
	.resource	= s3c_spi0_resource,
	.dev		= {
		.dma_mask = &s3c_device_spi0_dmamask,
		.coherent_dma_mask = 0xffffffffUL,
		.platform_data = &sspi0_mstr_info,
	}
};
EXPORT_SYMBOL(s3c_device_spi0);

/* SPI (1) */
static struct resource s3c_spi1_resource[] = {
	[0] = {
		.start = S5PV310_PA_SPI1,
		.end   = S5PV310_PA_SPI1 + 0x100 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SPI1,
		.end   = IRQ_SPI1,
		.flags = IORESOURCE_IRQ,
	}
};

static struct s3c_spi_mstr_info sspi1_mstr_info = {
	.pdev = NULL,
	.clk = NULL,
	.prnt_clk = NULL,
	.num_slaves = 0,
	.spiclck_get = smi_getclcks,
	.spiclck_put = smi_putclcks,
	.spiclck_en = smi_enclcks,
	.spiclck_dis = smi_disclcks,
	.spiclck_setrate = smi_setrate,
	.spiclck_getrate = smi_getrate,
};

static u64 s3c_device_spi1_dmamask = 0xffffffffUL;

struct platform_device s3c_device_spi1 = {
	.name		= "s3c-spi",
	.id		= 1,
	.num_resources	= ARRAY_SIZE(s3c_spi1_resource),
	.resource	= s3c_spi1_resource,
	.dev		= {
		.dma_mask = &s3c_device_spi1_dmamask,
		.coherent_dma_mask = 0xffffffffUL,
		.platform_data = &sspi1_mstr_info,
	}
};
EXPORT_SYMBOL(s3c_device_spi1);

/* SPI (2) */
static struct resource s3c_spi2_resource[] = {
	[0] = {
		.start = S5PV310_PA_SPI2,
		.end   = S5PV310_PA_SPI2 + 0x100 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SPI2,
		.end   = IRQ_SPI2,
		.flags = IORESOURCE_IRQ,
	}
};

static struct s3c_spi_mstr_info sspi2_mstr_info = {
	.pdev = NULL,
	.clk = NULL,
	.prnt_clk = NULL,
	.num_slaves = 0,
	.spiclck_get = smi_getclcks,
	.spiclck_put = smi_putclcks,
	.spiclck_en = smi_enclcks,
	.spiclck_dis = smi_disclcks,
	.spiclck_setrate = smi_setrate,
	.spiclck_getrate = smi_getrate,
};

static u64 s3c_device_spi2_dmamask = 0xffffffffUL;

struct platform_device s3c_device_spi2 = {
	.name		= "s3c-spi",
	.id		= 2,
	.num_resources	= ARRAY_SIZE(s3c_spi2_resource),
	.resource	= s3c_spi2_resource,
	.dev		= {
		.dma_mask = &s3c_device_spi2_dmamask,
		.coherent_dma_mask = 0xffffffffUL,
		.platform_data = &sspi2_mstr_info,
	}
};
EXPORT_SYMBOL(s3c_device_spi2);

void __init s3cspi_set_slaves(unsigned id, int n, struct s3c_spi_pdata const *dat)
{
	struct s3c_spi_mstr_info *pinfo;

	if(id == 0)
	   pinfo = (struct s3c_spi_mstr_info *)s3c_device_spi0.dev.platform_data;
	else if(id == 1)
	   pinfo = (struct s3c_spi_mstr_info *)s3c_device_spi1.dev.platform_data;
	else if(id == 2)
	   pinfo = (struct s3c_spi_mstr_info *)s3c_device_spi2.dev.platform_data;
	else
	   return;

	pinfo->spd = kmalloc(n * sizeof (*dat), GFP_KERNEL);
	if(!pinfo->spd)
	   return;
	memcpy(pinfo->spd, dat, n * sizeof(*dat));

	pinfo->num_slaves = n;
}

#else /////////////////////////////////////////////////////////////
static struct s3c64xx_spi_info s5pv310_spi2_pdata = {
	.cfg_gpio = s5pv310_spi_cfg_gpio,
	.fifo_lvl_mask = 0x7f,
	.rx_lvl_offset = 15,
	.high_speed = 1,
	.clk_from_cmu = true,
};

struct platform_device s5pv310_device_spi2 = {
	.name		  = "s3c64xx-spi",
	.id		  = 2,
	.num_resources	  = ARRAY_SIZE(s5pv310_spi2_resource),
	.resource	  = s5pv310_spi2_resource,
	.dev = {
		.dma_mask		= &spi_dmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
		.platform_data = &s5pv310_spi2_pdata,
	},
};

void __init s5pv310_spi_set_info(int cntrlr, int src_clk_nr, int num_cs)
{
	struct s3c64xx_spi_info *pd;

	/* Reject invalid configuration */
	if (!num_cs || src_clk_nr < 0
			|| src_clk_nr > S5PV310_SPI_SRCCLK_SCLK) {
		printk(KERN_ERR "%s: Invalid SPI configuration\n", __func__);
		return;
	}

	switch (cntrlr) {
	case 0:
		pd = &s5pv310_spi0_pdata;
		break;
	case 1:
		pd = &s5pv310_spi1_pdata;
		break;
	case 2:
		pd = &s5pv310_spi2_pdata;
		break;
	default:
		printk(KERN_ERR "%s: Invalid SPI controller(%d)\n",
							__func__, cntrlr);
		return;
	}

	pd->num_cs = num_cs;
	pd->src_clk_nr = src_clk_nr;
	pd->src_clk_name = spi_src_clks[src_clk_nr];
}
#endif

