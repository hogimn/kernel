/* linux/arch/arm/mach-s5pv310/setup-fb.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Base FIMD controller configuration
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <plat/clock.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-clock.h>
#include <mach/regs-gpio.h>
#include <linux/io.h>
#include <mach/map.h>
#include <mach/gpio.h>
/* #include <mach/pd.h> */

// 
//	CONFIG_FB_S3C_LMS700JF04
//
extern void 	SYSTEM_POWER_CONTROL(int power, int val); // sm5s4210-sysfs.c

#if defined(CONFIG_FB_S3C_LTP700WV) || defined(CONFIG_FB_S3C_LMS700JF04) || defined(CONFIG_FB_S3C_NS070L2C) || defined(CONFIG_FB_S3C_HDMI_UI_FB)


struct platform_device; /* don't need the contents */

void s3cfb_cfg_gpio(struct platform_device *pdev)
{
	int i;
	u32 reg;

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV310_GPF0(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV310_GPF0(i), S3C_GPIO_PULL_NONE);
		s5p_gpio_set_drvstr(S5PV310_GPF0(i), S5P_GPIO_DRVSTR_LV1);
	}

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV310_GPF1(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV310_GPF1(i), S3C_GPIO_PULL_NONE);
		s5p_gpio_set_drvstr(S5PV310_GPF1(i), S5P_GPIO_DRVSTR_LV1);
	}

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV310_GPF2(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV310_GPF2(i), S3C_GPIO_PULL_NONE);
		s5p_gpio_set_drvstr(S5PV310_GPF2(i), S5P_GPIO_DRVSTR_LV1);
	}

	for (i = 0; i < 4; i++) {
		s3c_gpio_cfgpin(S5PV310_GPF3(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV310_GPF3(i), S3C_GPIO_PULL_NONE);
		s5p_gpio_set_drvstr(S5PV310_GPF3(i), S5P_GPIO_DRVSTR_LV1);
	}

	/* Set FIMD0 bypass */
	reg = __raw_readl(S3C_VA_SYS + 0x0210);
	reg |= (1<<1);
	__raw_writel(reg, S3C_VA_SYS + 0x0210);
}

int s3cfb_clk_on(struct platform_device *pdev, struct clk **s3cfb_clk)
{
	struct clk *sclk = NULL;
	struct clk *mout_mpll = NULL;
	struct clk *lcd_clk = NULL;

	u32 rate = 0;

	lcd_clk = clk_get(NULL, "fimd0"); /*  CLOCK GATE IP ENABLE */
	if (IS_ERR(lcd_clk)) {
		dev_err(&pdev->dev, "failed to get ip clk for fimd0\n");
		goto err_clk0;
	}
	clk_enable(lcd_clk);
	clk_put(lcd_clk);

	sclk = clk_get(&pdev->dev, "sclk_fimd");
	if (IS_ERR(sclk)) {
		dev_err(&pdev->dev, "failed to get sclk for fimd\n");
		goto err_clk1;
	}

	mout_mpll = clk_get(&pdev->dev, "mout_mpll");
	if (IS_ERR(mout_mpll)) {
		dev_err(&pdev->dev, "failed to get mout_mpll\n");
		goto err_clk2;
	}

	clk_set_parent(sclk, mout_mpll);

#if 1
	rate = clk_round_rate(sclk, 133400000);
	dev_dbg(&pdev->dev, "set fimd sclk rate to %d\n", rate);

	if (!rate)
		rate = 133400000;

	clk_set_rate(sclk, rate);
#else
	clk_set_rate(sclk,133400000); 
//	clk_set_rate(sclk,800000000); 
#endif
	dev_dbg(&pdev->dev, "set fimd sclk rate to %d\n", rate);

	clk_put(mout_mpll);

	clk_enable(sclk);

	*s3cfb_clk = sclk;

	return 0;

err_clk2:
	clk_put(mout_mpll);
err_clk1:
	clk_put(sclk);
err_clk0:
	clk_put(lcd_clk);

	return -EINVAL;
}

int s3cfb_clk_off(struct platform_device *pdev, struct clk **clk)
{
	struct clk *lcd_clk = NULL;

	lcd_clk = clk_get(NULL, "fimd0"); /*  CLOCK GATE IP ENABLE */
	if (IS_ERR(lcd_clk)) {
		printk(KERN_ERR "failed to get ip clk for fimd0\n");
		goto err_clk0;
	}
	clk_disable(lcd_clk);
	clk_put(lcd_clk);

	clk_disable(*clk);
	clk_put(*clk);

	*clk = NULL;

	return 0;
err_clk0:
	clk_put(lcd_clk);

	return -EINVAL;
}

void s3cfb_get_clk_name(char *clk_name)
{
	strcpy(clk_name, "sclk_fimd");
}

int s3cfb_backlight_on(struct platform_device *pdev)
{
	int err;

	err = gpio_request(S5PV310_GPD0(0), "PWM0");
	if (err) {
		printk(KERN_ERR "LCD Backlight PWM Control(GPD0.0)\n");
		return err;
	}
	s3c_gpio_cfgpin(S5PV310_GPD0(0), S3C_GPIO_SFN(2));
	gpio_free(S5PV310_GPD0(0));


#if 0 //JNJ
	err = gpio_request(S5PV310_GPD0(1), "LCD_EN");
	if (err) {
		printk(KERN_ERR "LCD Enable Control(GPD0.1)\n");
		return err;
	}
	gpio_direction_output(S5PV310_GPD0(1), 0); /* LCD_EN Low(Invert) */
	gpio_free(S5PV310_GPD0(1));
#else
	err = gpio_request(S5PV310_GPX2(1), "LCD_EN");
	if (err) {
		printk(KERN_ERR "LCD Enable Control(GPX2.1)\n");
		return err;
	}
	gpio_direction_output(S5PV310_GPX2(1), 1); /* LCD_EN High */
	gpio_free(S5PV310_GPX2(1));
#endif

	SYSTEM_POWER_CONTROL(2, 0);	// VLED 12V ON

	return 0;
}

int s3cfb_backlight_off(struct platform_device *pdev)
{
	int err;

	err = gpio_request(S5PV310_GPD0(0), "PWM0");
	if (err) {
		printk(KERN_ERR "LCD Backlight PWM Control(GPD0.0)\n");
		return err;
	}

//JNJ	gpio_direction_output(S5PV310_GPD0(0), 1); /* BL pwm Low(Invert) */
	gpio_direction_output(S5PV310_GPD0(0), 0); /* BL pwm High */
	gpio_free(S5PV310_GPD0(0));

#if 0 //JNJ
	err = gpio_request(S5PV310_GPD0(1), "LCD_EN");
	if (err) {
		printk(KERN_ERR "LCD Enable Control(GPD0.1)\n");
		return err;
	}
	gpio_direction_output(S5PV310_GPD0(1), 1); /* LCD_EN High(Invert) */
	gpio_free(S5PV310_GPD0(1));
#else
	err = gpio_request(S5PV310_GPX2(1), "LCD_EN");
	if (err) {
		printk(KERN_ERR "LCD Enable Control(GPX2.1)\n");
		return err;
	}
	gpio_direction_output(S5PV310_GPX2(1), 0); /* LCD_EN Low */
	gpio_free(S5PV310_GPX2(1));
#endif

	SYSTEM_POWER_CONTROL(2, 1);	// VLED 12V OFF

	return 0;
}

int s3cfb_lcd_on(struct platform_device *pdev)
{
	return 0;
}

int s3cfb_lcd_off(struct platform_device *pdev)
{
	return 0;
}

#endif
