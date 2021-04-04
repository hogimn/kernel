/* linux/arch/arm/plat-s5pc1xx/setup-sdhci-gpio.c
 *
 * Copyright (c) 2009-2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * S5PV310 - Helper functions for setting up SDHCI device(s) GPIO (HSMMC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>

#include <mach/gpio.h>
#include <mach/map.h>
#include <plat/gpio-cfg.h>

#define GPK0DRV	(S5PV310_VA_GPIO2 + 0x4C)
#define GPK1DRV	(S5PV310_VA_GPIO2 + 0x6C)
#define GPK2DRV	(S5PV310_VA_GPIO2 + 0x8C)
#define GPK3DRV	(S5PV310_VA_GPIO2 + 0xAC)

#if defined (CONFIG_S5PV310_MSHC_VPLL_46MHZ) || \
	defined (CONFIG_S5PV310_MSHC_EPLL_45MHZ)
#define DIV_FSYS3	(S5PV310_VA_CMU + 0x0C54C)
#endif


void s5pv310_setup_mshci_cfg_gpio(struct platform_device *dev, int width)
{
	unsigned int gpio;

	early_printk("v310_setup_mshci_cfg_gpio\n");

	/* Set all the necessary GPG0/GPG1 pins to special-function 2 */
	for (gpio = S5PV310_GPK0(0); gpio < S5PV310_GPK0(3); gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		if ( gpio == S5PV310_GPK0(0) )
			s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		else
			s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
	}
	
	switch (width) {
	case 8:
		for (gpio = S5PV310_GPK1(3); gpio <= S5PV310_GPK1(6); gpio++) {
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(4));
			s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
		}
		__raw_writel(0x2AAA, GPK1DRV);
	case 4:
		/* GPK[3:6] special-funtion 2 */
		for (gpio = S5PV310_GPK0(3); gpio <= S5PV310_GPK0(6); gpio++) {
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
			s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
		}
		__raw_writel(0x2AAA, GPK0DRV);
		break;
	case 1:
		/* GPK[3] special-funtion 2 */
		for (gpio = S5PV310_GPK0(3); gpio < S5PV310_GPK0(4); gpio++) {
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
			s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
		}
		__raw_writel(0xAA, GPK0DRV);
	default:
		break;
	}
}

#if defined (CONFIG_S5PV310_MSHC_VPLL_46MHZ) || \
	defined (CONFIG_S5PV310_MSHC_EPLL_45MHZ)
void s5pv310_setup_mshci_cfg_ddr(struct platform_device *dev, int ddr)
{
		if (ddr) {
#ifdef CONFIG_S5PV310_MSHC_EPLL_45MHZ
			__raw_writel(0x00, DIV_FSYS3);
#else
			__raw_writel(0x01, DIV_FSYS3);
#endif
		} else {
#ifdef CONFIG_S5PV310_MSHC_EPLL_45MHZ
			__raw_writel(0x01, DIV_FSYS3);
#else
			__raw_writel(0x03, DIV_FSYS3);
#endif
		}
}
#endif
