/* linux/arch/arm/mach-s5pv310/dev-pd.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * S5PV310 - Power Domain support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <plat/pd.h>

#include <mach/regs-pmu.h>
#include <mach/regs-clock.h>

static int s5pv310_pd_init(struct device *dev)
{
#if 0	
	struct samsung_pd_info *pdata =  dev->platform_data;
	struct s5pv310_pd_data *data = (struct s5pv310_pd_data *) pdata->data;

	if (data->read_phy_addr) {
		data->read_base = ioremap(data->read_phy_addr, SZ_4K);
		if (!data->read_base)
			return -ENOMEM;
	}
	pdata->enabled = 0;
#endif

//printk("CHARLES DEBUG ===================> Power domain %s init.\n", dev_name(dev));

	return 0;
}

int s5pv310_pd_enable(struct device *dev)
{
	struct samsung_pd_info *pdata =  dev->platform_data;
	struct s5pv310_pd_data *data = (struct s5pv310_pd_data *) pdata->data;
	u32 timeout;
	u32 tmp = 0;

#if 0
	if (data->blk_clk_base) {
		tmp = __raw_readl(data->blk_clk_base);
		tmp |= data->control;
		__raw_writel(tmp, data->blk_clk_base);
	}

	if (data->read_base)
		/*  save IP clock gating register */
		tmp = __raw_readl(data->clk_base);

	/*  enable all the clocks of IPs in the power domain */
	__raw_writel(0xffffffff, data->clk_base);

	__raw_writel(S5P_INT_LOCAL_PWR_EN, pdata->base);

	/* Wait max 1ms */
	timeout = 1000;
	while ((__raw_readl(pdata->base + 0x4) & S5P_INT_LOCAL_PWR_EN)
		!= S5P_INT_LOCAL_PWR_EN) {
		if (timeout == 0) {
			printk(KERN_ERR "Power domain %s enable failed.\n",
				dev_name(dev));
			return -ETIMEDOUT;
		}
		timeout--;
		udelay(1);
	}

	if (data->read_base) {
		/* dummy read to check the completion of power-on sequence */
		__raw_readl(data->read_base);

		/* restore IP clock gating register */
		__raw_writel(tmp, data->clk_base);
	}
	pdata->enabled = 1;
#endif

//printk("CHARLES DEBUG ===================> Power domain %s enable.\n", dev_name(dev));

	return 0;
}

static int s5pv310_pd_disable(struct device *dev)
{
#if 0
	struct samsung_pd_info *pdata =  dev->platform_data;
	struct s5pv310_pd_data *data = (struct s5pv310_pd_data *) pdata->data;
	u32 timeout;
	u32 tmp = 0;

	__raw_writel(0, pdata->base);

	/* Wait max 1ms */
	timeout = 1000;
	while (__raw_readl(pdata->base + 0x4) & S5P_INT_LOCAL_PWR_EN) {
		if (timeout == 0) {
			printk(KERN_ERR "Power domain %s disable failed.\n",
				dev_name(dev));
			return -ETIMEDOUT;
		}
		timeout--;
		udelay(1);
	}
	pdata->enabled = 0;

	if (data->blk_clk_base) {
		tmp = __raw_readl(data->blk_clk_base);
		tmp &= ~(data->control);
		__raw_writel(tmp, data->blk_clk_base);
	}
#endif
//printk("CHARLES DEBUG ===================> Power domain %s disable.\n", dev_name(dev));

	return 0;
}

struct platform_device s5pv310_device_pd[] = {
	{
		.name		= "samsung-pd",
		.id		= 0,
		.dev = {
			.platform_data = &(struct samsung_pd_info) {
				.init		= s5pv310_pd_init,
				.enable		= s5pv310_pd_enable,
				.disable	= s5pv310_pd_disable,
				.base		= S5P_PMU_MFC_CONF,
				.data		= &(struct s5pv310_pd_data) {
					.clk_base	= S5P_CLKGATE_IP_MFC,
					.read_phy_addr	= S5PV310_PA_MFC,
					.blk_clk_base   = S5P_CLKGATE_BLOCK,
					.control        = (0x1 << 2),
				},
			},
		},
	}, {
		.name		= "samsung-pd",
		.id		= 1,
		.dev = {
			.platform_data = &(struct samsung_pd_info) {
				.init		= s5pv310_pd_init,
				.enable		= s5pv310_pd_enable,
				.disable	= s5pv310_pd_disable,
				.base		= S5P_PMU_G3D_CONF,
				.data		= &(struct s5pv310_pd_data) {
					.clk_base	= S5P_CLKGATE_IP_G3D,
					.read_phy_addr	= S5PV310_PA_G3D,
					.blk_clk_base   = S5P_CLKGATE_BLOCK,
					.control        = (0x1 << 3),
				},
			},
		},
	}, {
		.name		= "samsung-pd",
		.id		= 2,
		.dev = {
			.platform_data = &(struct samsung_pd_info) {
				.init		= s5pv310_pd_init,
				.enable		= s5pv310_pd_enable,
				.disable	= s5pv310_pd_disable,
				.base		= S5P_PMU_LCD0_CONF,
				.data		= &(struct s5pv310_pd_data) {
					.clk_base	= S5P_CLKGATE_IP_LCD0,
					.read_phy_addr	= S5PV310_PA_LCD0,
					.blk_clk_base   = S5P_CLKGATE_BLOCK,
					.control        = (0x1 << 4),
				},
			},
		},
	}, {
		.name		= "samsung-pd",
		.id		= 3,
		.dev = {
			.platform_data = &(struct samsung_pd_info) {
				.init		= s5pv310_pd_init,
				.enable		= s5pv310_pd_enable,
				.disable	= s5pv310_pd_disable,
				.base		= S5P_PMU_LCD1_CONF,
				.data		= &(struct s5pv310_pd_data) {
					.clk_base	= S5P_CLKGATE_IP_LCD1,
					.read_phy_addr	= S5PV310_PA_LCD1,
					.blk_clk_base   = S5P_CLKGATE_BLOCK,
					.control        = (0x1 << 5),
				},
			},
		},
	}, {
		.name		= "samsung-pd",
		.id		= 4,
		.dev = {
			.platform_data = &(struct samsung_pd_info) {
				.init		= s5pv310_pd_init,
				.enable		= s5pv310_pd_enable,
				.disable	= s5pv310_pd_disable,
				.base		= S5P_PMU_TV_CONF,
				.data		= &(struct s5pv310_pd_data) {
					.clk_base	= S5P_CLKGATE_IP_TV,
					.read_phy_addr	= S5PV310_PA_VP,
					.blk_clk_base   = S5P_CLKGATE_BLOCK,
					.control        = (0x1 << 1),
				},
			},
		},
	}, {
		.name		= "samsung-pd",
		.id		= 5,
		.dev = {
			.platform_data = &(struct samsung_pd_info) {
				.init		= s5pv310_pd_init,
				.enable		= s5pv310_pd_enable,
				.disable	= s5pv310_pd_disable,
				.base		= S5P_PMU_CAM_CONF,
				.data		= &(struct s5pv310_pd_data) {
					.clk_base	= S5P_CLKGATE_IP_CAM,
					.read_phy_addr	= S5PV310_PA_FIMC0,
					.blk_clk_base   = S5P_CLKGATE_BLOCK,
					.control        = (0x1 << 0),
				},
			},
		},
	}, {
		.name		= "samsung-pd",
		.id		= 6,
		.dev = {
			.platform_data = &(struct samsung_pd_info) {
				.init		= s5pv310_pd_init,
				.enable		= s5pv310_pd_enable,
				.disable	= s5pv310_pd_disable,
				.base		= S5P_PMU_GPS_CONF,
				.data		= &(struct s5pv310_pd_data) {
					.clk_base	= S5P_CLKGATE_IP_GPS,
					.blk_clk_base   = S5P_CLKGATE_BLOCK,
					.control        = (0x1 << 7),
				},
			},
		},
	},
};
EXPORT_SYMBOL(s5pv310_device_pd);
