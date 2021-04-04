/* Copyright (c) 2011 Samsung Electronics Co, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/serial_core.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/usb/ch9.h>

#include <plat/regs-otg.h>

#include <asm/mach/arch.h>
#include <asm/mach-types.h>

#include <mach/regs-clock.h>

#define ETC6PUD		(S5PV310_VA_GPIO2 + 0x228)

/* Initializes OTG Phy. */
void otg_phy_init(void)
{
	__raw_writel(__raw_readl(S5P_USBOTG_PHY_CONTROL)
		|(0x1<<0), S5P_USBOTG_PHY_CONTROL);
	__raw_writel((__raw_readl(S3C_USBOTG_PHYPWR)
		&~(0x7<<3)&~(0x1<<0)), S3C_USBOTG_PHYPWR);
	__raw_writel((__raw_readl(S3C_USBOTG_PHYCLK)
		&~(0x5<<2))|(0x3<<0), S3C_USBOTG_PHYCLK); /* PLL 24Mhz */
	__raw_writel((__raw_readl(S3C_USBOTG_RSTCON)
		&~(0x3<<1))|(0x1<<0), S3C_USBOTG_RSTCON);
	udelay(10);
	__raw_writel(__raw_readl(S3C_USBOTG_RSTCON)
		&~(0x7<<0), S3C_USBOTG_RSTCON);
	udelay(10);
}
EXPORT_SYMBOL(otg_phy_init);

/* USB Control request data struct must be located here for DMA transfer */
//struct usb_ctrlrequest usb_ctrl __cacheline_aligned;
//EXPORT_SYMBOL(usb_ctrl);

/* OTG PHY Power Off */
void otg_phy_off(void)
{
	__raw_writel(__raw_readl(S3C_USBOTG_PHYPWR)
		|(0x3<<3), S3C_USBOTG_PHYPWR);
	__raw_writel(__raw_readl(S5P_USBOTG_PHY_CONTROL)
		&~(1<<0), S5P_USBOTG_PHY_CONTROL);
}
EXPORT_SYMBOL(otg_phy_off);

void usb_host_phy_init(void)
{
	struct clk *usb_clk;

	/*  Must be enable usbhost & usbotg clk  */
	usb_clk = clk_get(NULL, "usbotg");

	if (IS_ERR(usb_clk)) {
		printk(KERN_ERR"cannot get usb-otg clock\n");
		return ;
	}

	clk_enable(usb_clk);

	if (__raw_readl(S5P_USBHOST_PHY_CONTROL) & (0x1<<0)) {
		printk(KERN_ERR"[usb_host_phy_init]Already power on PHY\n");
		return;
	}

#ifdef CONFIG_CPU_S5PV310_EVT1
	 /*
	 // set XuhostOVERCUR to in-active by controlling ET6PUD[15:14]
	 //  0x0 : pull-up/down disabled
	 //  0x1 : pull-down enabled
	 //  0x2 : reserved
	 //  0x3 : pull-up enabled
	 */
	__raw_writel((__raw_readl(ETC6PUD) & ~(0x3 << 14)) | (0x3 << 14), ETC6PUD); //pull-up
#else
	__raw_writel((__raw_readl(ETC6PUD) & ~(0x3 << 14)) | (0x1 << 14), ETC6PUD); // pull-down
#endif

	__raw_writel(__raw_readl(S5P_USBHOST_PHY_CONTROL)
		|(0x1<<0), S5P_USBHOST_PHY_CONTROL);

	/* floating prevention logic : disable */
	__raw_writel((__raw_readl(S3C_USBOTG_PHY1CON) | (0x1<<0)),
		S3C_USBOTG_PHY1CON);

	/* set hsic phy0,1 to normal */
	__raw_writel((__raw_readl(S3C_USBOTG_PHYPWR) & ~(0xf<<9)),
		S3C_USBOTG_PHYPWR);

	/* phy-power on */
	__raw_writel((__raw_readl(S3C_USBOTG_PHYPWR) & ~(0x7<<6)),
		S3C_USBOTG_PHYPWR);

	/* set clock source for PLL (24MHz) */
	__raw_writel((__raw_readl(S3C_USBOTG_PHYCLK) | (0x1<<7) | (0x3<<0)),
		S3C_USBOTG_PHYCLK);

	/* reset all ports of both PHY and Link */
	__raw_writel((__raw_readl(S3C_USBOTG_RSTCON) | (0x1<<6) | (0x7<<3)),
		S3C_USBOTG_RSTCON);
	udelay(10);
	__raw_writel((__raw_readl(S3C_USBOTG_RSTCON) & ~(0x1<<6) & ~(0x7<<3)),
		S3C_USBOTG_RSTCON);
	udelay(50);
}
EXPORT_SYMBOL(usb_host_phy_init);

void usb_host_phy_off(void)
{
	__raw_writel(__raw_readl(S3C_USBOTG_PHYPWR)
		|(0x1<<7)|(0x1<<6), S3C_USBOTG_PHYPWR);
	__raw_writel(__raw_readl(S5P_USBHOST_PHY_CONTROL)
		&~(1<<0), S5P_USBHOST_PHY_CONTROL);
}
EXPORT_SYMBOL(usb_host_phy_off);

/* For L2 suspend */
void usb_host_phy_suspend(void)
{
	unsigned int reg = __raw_readl(S3C_USBOTG_PHYPWR);
	
	/* if OHCI isn't used, 7 bit clear */
	__raw_writel((__raw_readl(S3C_USBOTG_PHYCLK) & ~(0x1<<7)),
		S3C_USBOTG_PHYCLK);

	reg |= (0x1<<6);  /* phy port0 suspend */
	reg |= (0x1<<9);  /* phy hsic port0 suspend */
	reg |= (0x1<<11); /* phy hsic port1 suspend */
	__raw_writel(reg, S3C_USBOTG_PHYPWR);
}
EXPORT_SYMBOL(usb_host_phy_suspend);

void usb_host_phy_resume(void)
{
	printk("LSI Host USB : Resume\n");
	__raw_writel(__raw_readl(S3C_USBOTG_PHYPWR)
		& ~((0x1<<6)|(0x1<<9)|(0x1<<11)), S3C_USBOTG_PHYPWR);
	/* if OHCI 48mhz, 7 bit set */
	__raw_writel((__raw_readl(S3C_USBOTG_PHYCLK) | (0x1<<7)),
		S3C_USBOTG_PHYCLK);
}
EXPORT_SYMBOL(usb_host_phy_resume);

/* For LPM(L1) suspend */
void usb_host_phy_sleep(void)
{
	unsigned int reg = __raw_readl(S3C_USBOTG_PHYPWR);
	/* if OHCI isn't used, 7 bit clear */
	__raw_writel((__raw_readl(S3C_USBOTG_PHYCLK) & ~(0x1<<7)),
		S3C_USBOTG_PHYCLK);

	reg |= (0x1<<8);  /* phy port0 sleep */
	reg |= (0x1<<10); /* phy hsic port0 sleep */
	reg |= (0x1<<12); /* phy hsic port1 sleep */
	__raw_writel(reg, S3C_USBOTG_PHYPWR);
}
EXPORT_SYMBOL(usb_host_phy_sleep);

void usb_host_phy_wakeup(void)
{
	__raw_writel(__raw_readl(S3C_USBOTG_PHYPWR)
		& ~((0x1<<8)|(0x1<<10)|(0x1<<12)), S3C_USBOTG_PHYPWR);
	/* if OHCI 48mhz, 7 bit set */
	__raw_writel((__raw_readl(S3C_USBOTG_PHYCLK) | (0x1<<7)),
		S3C_USBOTG_PHYCLK);
}
EXPORT_SYMBOL(usb_host_phy_wakeup);
