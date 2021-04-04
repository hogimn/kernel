/*
 *  linux/drivers/serial/8250_hanback.c
 *
 *  Written by Paul B Schroeder < pschroeder "at" uplogix "dot" com >
 *  Based on 8250_boca.
 *
 *  Copyright (C) 2005 Russell King.
 *  Data taken from include/asm-i386/serial.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/serial_8250.h>
#include <mach/map.h>
#include <asm/mach/irq.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/s5pv310.h>

#define PORT(_irq, _mapbase)				\
	{						\
		.irq		= _irq,			\
		.mapbase	= _mapbase,		\
		.uartclk	= 14745600,		\
		.iotype		= UPIO_MEM,		\
		.flags		= UPF_IOREMAP,		\
	}

static struct plat_serial8250_port exar_data[] = {
  PORT(IRQ_EINT(8), HANBACK_ST16C554_UART0),
	PORT(IRQ_EINT(9), HANBACK_ST16C554_UART1),
	PORT(IRQ_EINT(10), HANBACK_ST16C554_UART2),
	PORT(IRQ_EINT(11), HANBACK_ST16C554_UART3),
	{ },
};

static struct platform_device exar_device = {
	.name			= "serial8250",
	.id			= PLAT8250_DEV_EXAR_ST16C554,
	.dev			= {
		.platform_data	= exar_data,
	},
};

static int __init exar_init(void)
{
  u32 err;
  /* XEINT[8] Setting */
  err = gpio_request(S5PV310_GPX1(0), "QUART0");
  if (err) {
    printk(KERN_INFO "gpio request error : %d\n", err);
  } else {
    s3c_gpio_cfgpin(S5PV310_GPX1(0), S3C_GPIO_SFN(0xF));
    s3c_gpio_setpull(S5PV310_GPX1(0), S3C_GPIO_PULL_NONE);
  }
  set_irq_type(IRQ_EINT(8), IRQ_TYPE_EDGE_RISING);
  set_irq_wake(IRQ_EINT(8), 1);

  /* XEINT[9] Setting */
  err = gpio_request(S5PV310_GPX1(1), "QUART1");
  if (err) {
    printk(KERN_INFO "gpio request error : %d\n", err);
  } else {
    s3c_gpio_cfgpin(S5PV310_GPX1(1), S3C_GPIO_SFN(0xF));
    s3c_gpio_setpull(S5PV310_GPX1(1), S3C_GPIO_PULL_NONE);
  }
  set_irq_type(IRQ_EINT(9), IRQ_TYPE_EDGE_RISING);
  set_irq_wake(IRQ_EINT(9), 1);

  /* XEINT[10] Setting */
  err = gpio_request(S5PV310_GPX1(2), "QUART2");
  if (err) {
    printk(KERN_INFO "gpio request error : %d\n", err);
  } else {
    s3c_gpio_cfgpin(S5PV310_GPX1(2), S3C_GPIO_SFN(0xF));
    s3c_gpio_setpull(S5PV310_GPX1(2), S3C_GPIO_PULL_NONE);
  }
  set_irq_type(IRQ_EINT(10), IRQ_TYPE_EDGE_RISING);
  set_irq_wake(IRQ_EINT(10), 1);

  /* XEINT[11] Setting */
  err = gpio_request(S5PV310_GPX1(3), "QUART3");
  if (err) {
    printk(KERN_INFO "gpio request error : %d\n", err);
  } else {
    s3c_gpio_cfgpin(S5PV310_GPX1(3), S3C_GPIO_SFN(0xF));
    s3c_gpio_setpull(S5PV310_GPX1(3), S3C_GPIO_PULL_NONE);
  }
  set_irq_type(IRQ_EINT(11), IRQ_TYPE_EDGE_RISING);
  set_irq_wake(IRQ_EINT(11), 1);

	return platform_device_register(&exar_device);
}

module_init(exar_init);

MODULE_AUTHOR("Paul B Schroeder");
MODULE_DESCRIPTION("8250 serial probe module for Exar cards");
MODULE_LICENSE("GPL");
