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

struct platform_device; /* don't need the contents */

#if defined(CONFIG_FB_S3C_LTP700WV) || defined(CONFIG_FB_S3C_LMS700JF04) || defined(CONFIG_FB_S3C_NS070L2C)
void s3cfb_cfg_gpio(struct platform_device *pdev)
{
	int i;
	u32 reg;

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV310_GPF0(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV310_GPF0(i), S3C_GPIO_PULL_NONE);
		s5p_gpio_set_drvstr(S5PV310_GPF0(i), S5P_GPIO_DRVSTR_LV4);
	}

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV310_GPF1(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV310_GPF1(i), S3C_GPIO_PULL_NONE);
		s5p_gpio_set_drvstr(S5PV310_GPF1(i), S5P_GPIO_DRVSTR_LV4);
	}

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV310_GPF2(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV310_GPF2(i), S3C_GPIO_PULL_NONE);
		s5p_gpio_set_drvstr(S5PV310_GPF2(i), S5P_GPIO_DRVSTR_LV4);
	}

	for (i = 0; i < 4; i++) {
		s3c_gpio_cfgpin(S5PV310_GPF3(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV310_GPF3(i), S3C_GPIO_PULL_NONE);
		s5p_gpio_set_drvstr(S5PV310_GPF3(i), S5P_GPIO_DRVSTR_LV4);
	}

	/* Set FIMD0 bypass */
	reg = __raw_readl(S3C_VA_SYS + 0x0210);
	reg |= (1<<1);
	__raw_writel(reg, S3C_VA_SYS + 0x0210);
}
#elif defined(CONFIG_FB_S3C_AMS369FG06)
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
#elif defined(CONFIG_FB_S3C_HT101HD1)
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

	for (i = 0; i < 6; i++) {
		s3c_gpio_cfgpin(S5PV310_GPF3(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV310_GPF3(i), S3C_GPIO_PULL_NONE);
		s5p_gpio_set_drvstr(S5PV310_GPF3(i), S5P_GPIO_DRVSTR_LV1);
	}

	/* Set FIMD0 bypass */
	reg = __raw_readl(S3C_VA_SYS + 0x0210);
	reg |= (1<<1);
	__raw_writel(reg, S3C_VA_SYS + 0x0210);
}
#endif

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
#if 0
	rate = clk_round_rate(sclk, 133400000);
	dev_dbg(&pdev->dev, "set fimd sclk rate to %d\n", rate);

	if (!rate)
		rate = 133400000;

	clk_set_rate(sclk, rate);
#else
//#if defined(CONFIG_FB_S3C_NS070L2C)
//	clk_set_rate(sclk,66700000); 
//#else
	clk_set_rate(sclk,133400000); 
//#endif
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

#define S5PV210_GPD_0_0_TOUT_0  (0x2)
#define S5PV210_GPD_0_1_TOUT_1  (0x2 << 4)
#define S5PV210_GPD_0_2_TOUT_2  (0x2 << 8)
#define S5PV210_GPD_0_3_TOUT_3  (0x2 << 12)

#if defined(CONFIG_FB_S3C_WA101S)
int s3cfb_backlight_on(struct platform_device *pdev)
{
	int err;

	err = gpio_request(S5PV310_GPD0(1), "GPD0");
	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
			"lcd backlight control\n");
		return err;
	}
#if !defined(CONFIG_BACKLIGHT_PWM)
	gpio_direction_output(S5PV310_GPD0(1), 1);
	gpio_free(S5PV310_GPD0(1));
#else
	err = gpio_request(S5PV310_GPD0(1), "GPD0");

	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
			"lcd backlight control\n");
		return err;
	}

	gpio_direction_output(S5PV310_GPD0(1), 0);

	s3c_gpio_cfgpin(S5PV310_GPD0(1), S5PV210_GPD_0_1_TOUT_1);

	gpio_free(S5PV310_GPD0(1));
#endif

	return 0;
}

int s3cfb_backlight_off(struct platform_device *pdev)
{
	int err;

	err = gpio_request(S5PV310_GPD0(1), "GPD0");
	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
			"lcd backlight control\n");
		return err;
	}

	gpio_direction_output(S5PV310_GPD0(1), 0);
	gpio_free(S5PV310_GPD0(1));
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

#elif defined(CONFIG_FB_S3C_LTP700WV) || defined(CONFIG_FB_S3C_LMS700JF04) || defined(CONFIG_FB_S3C_NS070L2C)
int s3cfb_backlight_on(struct platform_device *pdev)
{
#if !defined(CONFIG_BACKLIGHT_PWM)
	int err;

	err = gpio_request(S5PV310_GPD0(1), "GPD0");
	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
			"lcd backlight control\n");
		return err;
	}

	gpio_direction_output(S5PV310_GPD0(1), 1);
	gpio_free(S5PV310_GPD0(1));
#endif
	return 0;
}

int s3cfb_backlight_off(struct platform_device *pdev)
{
#if !defined(CONFIG_BACKLIGHT_PWM)
	int err;

	err = gpio_request(S5PV310_GPD0(1), "GPD0");
	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
			"lcd backlight control\n");
		return err;
	}

	gpio_direction_output(S5PV310_GPD0(1), 0);
	gpio_free(S5PV310_GPD0(1));
#endif
	return 0;
}

int s3cfb_lcd_on(struct platform_device *pdev)
{
	int err;

	err = gpio_request(S5PV310_GPX0(6), "GPX0");
	if (err) {
		printk(KERN_ERR "failed to request GPX0 for "
			"lcd reset control\n");
		return err;
	}

	gpio_direction_output(S5PV310_GPX0(6), 1);
	mdelay(100);

	gpio_set_value(S5PV310_GPX0(6), 0);
	mdelay(10);

	gpio_set_value(S5PV310_GPX0(6), 1);
	mdelay(10);

	gpio_free(S5PV310_GPX0(6));

	return 0;
}

int s3cfb_lcd_off(struct platform_device *pdev)
{
	return 0;
}
#elif defined(CONFIG_FB_S3C_HT101HD1)
int s3cfb_backlight_on(struct platform_device *pdev)
{
#if !defined(CONFIG_BACKLIGHT_PWM)
	int err;

	err = gpio_request(S5PV310_GPD0(0), "GPD0");
	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
			"lcd backlight control\n");
		return err;
	}

	err = gpio_request(S5PV310_GPB(2), "GPB");
	if (err) {
		printk(KERN_ERR "failed to request GPB for "
			"lcd LED_EN control\n");
		return err;
	}

	gpio_direction_output(S5PV310_GPD0(0), 1); /* BL pwm High */
	gpio_direction_output(S5PV310_GPB(2), 1); /* LED_EN (SPI1_MOSI) */

	gpio_free(S5PV310_GPD0(0));
	gpio_free(S5PV310_GPB(2));
#endif
	return 0;
}

int s3cfb_backlight_off(struct platform_device *pdev)
{
#if !defined(CONFIG_BACKLIGHT_PWM)
	int err;

	err = gpio_request(S5PV310_GPD0(0), "GPD0");
	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
			"lcd backlight control\n");
		return err;
	}

	err = gpio_request(S5PV310_GPB(2), "GPB");
	if (err) {
		printk(KERN_ERR "failed to request GPB for "
			"lcd LED_EN control\n");
		return err;
	}

	gpio_direction_output(S5PV310_GPD0(3), 0);
	gpio_direction_output(S5PV310_GPB(2), 0);

	gpio_free(S5PV310_GPD0(0));
	gpio_free(S5PV310_GPB(2));
#endif
	return 0;
}

int s3cfb_lcd_on(struct platform_device *pdev)
{
	int err;

	err = gpio_request(S5PV310_GPH0(1), "GPH0");
	if (err) {
		printk(KERN_ERR "failed to request GPH0 for "
			"lcd reset control\n");
		return err;
	}

	gpio_direction_output(S5PV310_GPH0(1), 1);

	gpio_set_value(S5PV310_GPH0(1), 0);
	gpio_set_value(S5PV310_GPH0(1), 1);

	gpio_free(S5PV310_GPH0(1));

	return 0;
}

int s3cfb_lcd_off(struct platform_device *pdev)
{
	return 0;
}
#elif defined(CONFIG_FB_S3C_AMS369FG06)
int s3cfb_backlight_on(struct platform_device *pdev)
{
#if !defined(CONFIG_BACKLIGHT_PWM)
	int err;

	err = gpio_request(S5PV310_GPD0(1), "GPD0");
	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
			"lcd backlight control\n");
		return err;
	}

	gpio_direction_output(S5PV310_GPD0(1), 1);
	gpio_free(S5PV310_GPD0(1));
#endif
	return 0;
}

int s3cfb_backlight_off(struct platform_device *pdev)
{
	int err;

	err = gpio_request(S5PV310_GPD0(1), "GPD0");
	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
			"lcd backlight control\n");
		return err;
	}

	gpio_direction_output(S5PV310_GPD0(1), 0);
	gpio_free(S5PV310_GPD0(1));
	return 0;
}

int s3cfb_lcd_on(struct platform_device *pdev)
{
	int err;

	err = gpio_request(S5PV310_GPX0(6), "GPX0");
	if (err) {
		printk(KERN_ERR "failed to request GPX0 for "
			"lcd reset control\n");
		return err;
	}

	gpio_direction_output(S5PV310_GPX0(6), 1);
	mdelay(100);

	gpio_set_value(S5PV310_GPX0(6), 0);
	mdelay(100);

	gpio_set_value(S5PV310_GPX0(6), 1);
	mdelay(100);

	gpio_free(S5PV310_GPX0(6));

	return 0;
}

int s3cfb_lcd_off(struct platform_device *pdev)
{
	return 0;
}

#else
void s3cfb_cfg_gpio(struct platform_device *pdev)
{
	return;
}
int s3cfb_backlight_on(struct platform_device *pdev)
{
	return 0;
}

int s3cfb_backlight_off(struct platform_device *pdev)
{
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
