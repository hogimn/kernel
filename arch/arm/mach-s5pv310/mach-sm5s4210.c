/* linux/arch/arm/mach-s5pv310/mach-smdkc210.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/platform_device.h>
#include <linux/serial_core.h>
#include <linux/delay.h>
#include <linux/usb/ch9.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>
#include <linux/spi/spi_gpio.h>
#include <linux/mmc/host.h>
#include <linux/pn544.h>
#include <linux/io.h>
#include <linux/clk.h>
#if defined(CONFIG_S5P_MEM_CMA)
#include <linux/cma.h>
#endif
#include <linux/regulator/machine.h>
#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#endif

#include <asm/pmu.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <plat/regs-serial.h>
#include <plat/s5pv310.h>
#include <plat/cpu.h>
#include <plat/fb.h>
#include <plat/iic.h>
#include <plat/devs.h>
#include <plat/adc.h>
#include <plat/ts.h>
#include <plat/fimg2d.h>
#include <plat/sdhci.h>
#include <plat/mshci.h>
#include <plat/regs-otg.h>
#include <plat/pm.h>
#include <plat/tvout.h>

#include <plat/csis.h>
#include <plat/fimc.h>
#include <plat/media.h>

#include <plat/s3c64xx-spi.h>
#include <plat/gpio-cfg.h>
#include <plat/pd.h>

//#include <media/s5k3ba_platform.h>
//#include <media/s5k4ba_platform.h>
#include <media/s5k4ea_platform.h>
//#include <media/s5k6aa_platform.h>
//#include <media/s5k5cagx_platform.h>
//#include <media/s5k5aafa_platform.h>


#include <mach/regs-gpio.h>

#include <mach/map.h>
#include <mach/regs-mem.h>
#include <mach/regs-srom.h>
#include <mach/regs-clock.h>
#include <mach/media.h>
#include <mach/gpio.h>
#include <mach/gpio-s4210.h>

#include <mach/spi-clocks.h>


#if defined(CONFIG_REGULATOR_MAX8997)
extern struct max8997_platform_data max8997_pdata;
#endif

#if defined(CONFIG_MPU_SENSORS_MPU3050) 
#include <linux/mpu.h>
#endif

#if defined(CONFIG_BACKLIGHT_PWM)	// 
	#include <linux/pwm_backlight.h>
#endif

#if defined(CONFIG_DEV_THERMAL)
#include <plat/s5p-tmu.h>
#include <mach/regs-tmu.h>
#endif


#if defined(CONFIG_SPI_S5PV310)
	#include <plat/spi.h>
#endif


/* Following are default values for UCON, ULCON and UFCON UART registers */
#define SMDKC210_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define SMDKC210_ULCON_DEFAULT	S3C2410_LCON_CS8

#define SMDKC210_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)

static struct s3c2410_uartcfg smdkc210_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= SMDKC210_UCON_DEFAULT,
		.ulcon		= SMDKC210_ULCON_DEFAULT,
		.ufcon		= SMDKC210_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= SMDKC210_UCON_DEFAULT,
		.ulcon		= SMDKC210_ULCON_DEFAULT,
		.ufcon		= SMDKC210_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= SMDKC210_UCON_DEFAULT,
		.ulcon		= SMDKC210_ULCON_DEFAULT,
		.ufcon		= SMDKC210_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= SMDKC210_UCON_DEFAULT,
		.ulcon		= SMDKC210_ULCON_DEFAULT,
		.ufcon		= SMDKC210_UFCON_DEFAULT,
	},
	[4] = {
		.hwport		= 4,
		.flags		= 0,
		.ucon		= SMDKC210_UCON_DEFAULT,
		.ulcon		= SMDKC210_ULCON_DEFAULT,
		.ufcon		= SMDKC210_UFCON_DEFAULT,
	},
};

#ifdef CONFIG_SERIAL_8250_HANBACK
static struct platform_device st16c554_uart_device = {
  .name           = "st16c554-uart",
  .id             = 0,
};
static struct platform_device st16c554_uart_device1 = {
  .name           = "st16c554-uart",
  .id             = 1,
};

static struct platform_device st16c554_uart_device2 = {
  .name           = "st16c554-uart",
  .id             = 2,
};
static struct platform_device st16c554_uart_device3 = {
  .name           = "st16c554-uart",
  .id             = 3,
};
#endif

#undef WRITEBACK_ENABLED

#ifdef CONFIG_VIDEO_FIMC
/*
 * External camera reset
 * Because the most of cameras take i2c bus signal, so that
 * you have to reset at the boot time for other i2c slave devices.
 * This function also called at fimc_init_camera()
 * Do optimization for cameras on your platform.
*/
#define CONFIG_ITU_A
#ifdef CONFIG_ITU_A
static int smdkv310_cam0_reset(int dummy)
{
	int err;
	/* Camera A */
//Standby
	err = gpio_request(S5PV310_GPX2(0), "GPX2(0)");
	if (err)
		printk(KERN_ERR "#### failed to request GPX2_0 ####\n");

	s3c_gpio_setpull(S5PV310_GPX2(0), S3C_GPIO_PULL_NONE);
//	gpio_direction_output(S5PV310_GPX2(0), 0);
	gpio_direction_output(S5PV310_GPX2(0), 1);
	gpio_free(S5PV310_GPX2(0));

//Reset
	err = gpio_request(S5PV310_GPJ1(4), "GPJ1(4)");
	if (err)
		printk(KERN_ERR "#### failed to request GPJ1(4) for S5K4EA Camera Reset\n");

	s3c_gpio_setpull(S5PV310_GPJ1(4), S3C_GPIO_PULL_NONE);
	gpio_direction_output(S5PV310_GPJ1(4), 0);
	udelay(1000);
	gpio_direction_output(S5PV310_GPJ1(4), 1);
	gpio_free(S5PV310_GPJ1(4));

	return 0;
}

static int smdkv310_cam0_power(int onoff)
{
        static struct regulator *cam0_18_regulator;
        static struct regulator *cam0_28_regulator;
        int ret=0;

        cam0_18_regulator = regulator_get(NULL, "vdd_cam0_18");
        if (IS_ERR(cam0_18_regulator)) {
                printk(KERN_ERR "failed to get resource %s\n", "vdd_cam0_18");
                return -1;
        }

        cam0_28_regulator = regulator_get(NULL, "vdd_cam0_28");
        if (IS_ERR(cam0_28_regulator)) {
                printk(KERN_ERR "failed to get resource %s\n", "vdd_cam0_28");
                return -1;
        }

        //
        if(onoff){
                ret=regulator_enable(cam0_18_regulator);
                ret=regulator_enable(cam0_28_regulator);

                mdelay(50);
                printk( KERN_INFO "cam0 power: ON\n");
                smdkv310_cam0_reset(1);

        }
        else{
                smdkv310_cam0_reset(0);
                printk( KERN_INFO "cam0 power: OFF\n");
                ret=regulator_disable(cam0_18_regulator);
                ret=regulator_disable(cam0_28_regulator);
        }
        return ret;
}

#endif

#ifdef CONFIG_VIDEO_S5K4EA
static struct s5k4ea_platform_data s5k4ea_plat = {
	.default_width = 640,
	.default_height = 480,
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.freq = 24000000,
	.is_mipi = 0,
};

static struct i2c_board_info  s5k4ea_i2c_info = {
	I2C_BOARD_INFO("S5K4EA", 0x3c),
	.platform_data = &s5k4ea_plat,
};

static struct s3c_platform_camera s5k4ea = {
	.id		= CAMERA_PAR_A,
	.type		= CAM_TYPE_ITU,
	.fmt		= ITU_601_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_YCBYCR,
	.srclk_name	= "xusbxti",
	.clk_name	= "sclk_cam0",
	.i2c_busnum	= 1,
	.cam_power	= smdkv310_cam0_reset,
	.info		= &s5k4ea_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_UYVY,
//JNJ	.clk_rate	= 48000000,
	.clk_rate	= 24000000,
	.line_length	= 1920,
//	.width		= 640,
//	.height		= 480,
	.width		= 2560,
	.height		= 1920,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 640,
		.height	= 480,
	},

	/* Polarity */
	.inv_pclk	= 0,
	.inv_vsync	= 1,
	.inv_href	= 0,
	.inv_hsync	= 0,

	.initialized	= 0,
	.cam_power	= smdkv310_cam0_power,
};
#endif

#ifdef WRITEBACK_ENABLED
static struct i2c_board_info  writeback_i2c_info = {
	I2C_BOARD_INFO("WriteBack", 0x0),
};

static struct s3c_platform_camera writeback = {
	.id		= CAMERA_WB,
	.fmt		= ITU_601_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_CBYCRY,
	.i2c_busnum	= 0,
	.info		= &writeback_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_YUV444,
	.line_length	= 800,
	.width		= 480,
	.height		= 800,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 480,
		.height	= 800,
	},

	.initialized	= 0,
};
#endif

/* Interface setting */
static struct s3c_platform_fimc fimc_plat = {
#ifdef CONFIG_ITU_A
	.default_cam	= CAMERA_PAR_A,
#endif
#ifdef WRITEBACK_ENABLED
	.default_cam	= CAMERA_WB,
#endif
	.camera		= {
#ifdef CONFIG_VIDEO_S5K4EA
		&s5k4ea,
#endif
#ifdef WRITEBACK_ENABLED
		&writeback,
#endif
	},
	.hw_ver		= 0x51,
};
#endif /* CONFIG_VIDEO_FIMC */

#ifdef CONFIG_SMC911X
#include <linux/smc911x.h>
static struct resource smdkc210_smc911x_resources[] = {
	[0] = {
		.start = S5PV310_PA_SMC9115,
		.end   = S5PV310_PA_SMC9115 + 0x100,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_EINT(7),
		.end   = IRQ_EINT(7),
		.flags = IORESOURCE_IRQ,
	},
};

static struct smc911x_platdata smc9115 = {
	.flags = SMC911X_USE_16BIT,
};

static struct platform_device smdkc210_smc911x = {
	.name          = "smc911x",
	.id            = -1,
	.num_resources = ARRAY_SIZE(smdkc210_smc911x_resources),
	.resource      = smdkc210_smc911x_resources,
	.dev = {
		.platform_data = &smc9115,
	},
};
#endif

#if	defined(CONFIG_S3C64XX_DEV_SPI) || defined(CONFIG_SPI_S5PV310)
static struct s3c64xx_spi_csinfo spi1_csi[] = {
	[0] = {
		.line = S5PV310_GPB(5),
		.set_level = gpio_set_value,
	},
};

#if defined(CONFIG_SPI_S5PV310)
		static void s3c_cs_suspend(int pin, pm_message_t pm)
		{
				/* Whatever need to be done */
		}
		
		static void s3c_cs_resume(int pin)
		{
				/* Whatever need to be done */
		}
		
		static void s3c_cs_set(int pin, int lvl)
		{
				if(lvl == CS_HIGH)
				   s3c_gpio_setpin(pin, 1);
				else
				   s3c_gpio_setpin(pin, 0);
		}
		static void s3c_cs_config(int pin, int mode, int pull)
		{
				s3c_gpio_cfgpin(pin, mode);
		
				if(pull == CS_HIGH){
				   s3c_gpio_setpull(pin, S3C_GPIO_PULL_UP);
				   s3c_gpio_setpin(pin, 0);
				}
				else{
				   s3c_gpio_setpull(pin, S3C_GPIO_PULL_DOWN);
				   s3c_gpio_setpin(pin, 1);
				}
		}
		
		#define S5PV210_GPB_OUTPUT(__gpio)	(0x1 << ((__gpio) * 4))

		static struct s3c_spi_pdata s3c_slv_pdata_0[] __initdata = {
				[0] = { /* Slave-0 */
						.cs_level	  = CS_FLOAT,
						.cs_pin 	  = S5PV310_GPB(1),
						.cs_mode	  = S5PV210_GPB_OUTPUT(1),
						.cs_set 	  = s3c_cs_set,
						.cs_config	  = s3c_cs_config,
						.cs_suspend   = s3c_cs_suspend,
						.cs_resume	  = s3c_cs_resume,
				},
		};
		
		static struct s3c_spi_pdata s3c_slv_pdata_1[] __initdata = {
				[0] = { /* Slave-0 */
						.cs_level	  = CS_FLOAT,
						.cs_pin 	  = S5PV310_GPB(5),
						.cs_mode	  = S5PV210_GPB_OUTPUT(5),
						.cs_set 	  = s3c_cs_set,
						.cs_config	  = s3c_cs_config,
						.cs_suspend   = s3c_cs_suspend,
						.cs_resume	  = s3c_cs_resume,
				},
		};


		static struct spi_board_info s3c_spi_devs[] __initdata = {
		[0] = {
		#if defined(CONFIG_TEST_SPI)
				.modalias = "spi_test",
		#else
				.modalias = "spidev",
		#endif
				.mode			 = SPI_MODE_0,	/* CPOL=0, CPHA=0 */
				.max_speed_hz	 = 6000000,
				/* Connected to SPI-1 as 1st Slave */
				.bus_num		 = 1,
				.irq			 = IRQ_SPI1,
				.chip_select	 = 0,
//					.controller_data = &spi1_csi[0],
			},

#if 0
	        [1] = {
	                .modalias        = "spidev", /* Test Interface */
	                .mode            = SPI_MODE_0,  /* CPOL=0, CPHA=0 */
	                .max_speed_hz    = 100000,
	                /* Connected to SPI-0 as 2nd Slave */
	                .bus_num         = 0,
	                .irq             = IRQ_SPI0,
	                .chip_select     = 1,
	        },		
#endif	        

	};

#else
static struct spi_board_info spi1_board_info[] __initdata = {
	{
	#if defined(CONFIG_TEST_SPI)
			.modalias = "spi_test",
	#else
			.modalias = "spidev",
	#endif
		.platform_data = NULL,
		.max_speed_hz = 1*1000*1000,
		.bus_num = 1,
		.chip_select = 0,
		.mode = SPI_MODE_0,
		.controller_data = &spi1_csi[0],
	}
};
#endif

#else
#define SPI1_CS S5PV310_GPB(5)
#define SPI1_CLK S5PV310_GPB(4)
#define SPI1_SI S5PV310_GPB(6)
#define SPI1_SO S5PV310_GPB(3)

static struct s3c64xx_spi_csinfo spi1_csi[] = {
	[0] = {
		.line = S5PV310_GPB(5),
		.set_level = gpio_set_value,
	},
};

static struct spi_board_info spi1_board_info[] __initdata = {
	{
		.platform_data = NULL,
		.max_speed_hz = 6*1000*1000,
		.bus_num = 1,
		.chip_select = 0,
		.mode = SPI_MODE_0,
		//.controller_data = &spi1_csi[0],
		.controller_data = SPI1_CS,
	}	
};

static struct spi_gpio_platform_data mtv350_spi_gpio_data = {
    .sck    = SPI1_CLK,
    .mosi   = SPI1_SI,
    .miso   = SPI1_SO,
    .num_chipselect = 1,
};

static struct platform_device s3c_device_spi_gpio = {
    .name   = "spi_gpio",
    .id = 1,
    .dev    = {
//	.parent = &s3c_device_fb.dev
        .platform_data  = &mtv350_spi_gpio_data,
    },
};
#endif	//#if	defined(CONFIG_S3C64XX_DEV_SPI)

#ifdef CONFIG_HAVE_PWM
#define S5PV310_GPD_0_0_TOUT_0  (0x2)
#define S5PV310_GPD_0_1_TOUT_1  (0x2 << 4)
#define S5PV310_GPD_0_2_TOUT_2  (0x2 << 8)
#define S5PV310_GPD_0_3_TOUT_3  (0x2 << 12)

struct s3c_pwm_data {
        /* PWM output port */
        unsigned int gpio_no;
        const char  *gpio_name;
        unsigned int gpio_set_value;
};

struct s3c_pwm_data pwm_data[] = {
        { // LCD backlight
                .gpio_no    = S5PV310_GPD0(0),
                .gpio_name  = "GPD",
                .gpio_set_value = S5PV310_GPD_0_0_TOUT_0,
        }, { // Proximity
                .gpio_no    = S5PV310_GPD0(2),
                .gpio_name  = "GPD",
                .gpio_set_value = S5PV310_GPD_0_2_TOUT_2,
        }
};
#endif



#if defined(CONFIG_BOARD_SM5S4210)	// 

extern void 	SYSTEM_POWER_CONTROL(int power, int val);// sm5s4210-sysfs.c


static struct platform_device sm5s4210_sysfs = {
    .name = "sm5s4210-sysfs",
    .id = -1,
};

static void sm5s4210_power_off(void)
{
	/* PS_HOLD --> Output Low */
	printk(KERN_EMERG "%s : setting GPIO_PDA_PS_HOLD low.\n", __func__);

	SYSTEM_POWER_CONTROL(0, 0);	// BUCK6 Disable
	SYSTEM_POWER_CONTROL(1, 0);	// P5V Disable
	SYSTEM_POWER_CONTROL(2, 1);	// P12V(VLED) Disable
	
	/* PS_HOLD output High --> Low  PS_HOLD_CONTROL, R/W, 0x1002_330C */
	(*(unsigned long *)(S5PV310_VA_PMU + 0x330C)) = 0x5200;	// Power OFF

	while(1);

	printk(KERN_EMERG "%s : should not reach here!\n", __func__);
}

#if defined(CONFIG_BACKLIGHT_PWM)
static struct platform_pwm_backlight_data sm5s4210_backlight_data = {
	.pwm_id  = 0,
	.max_brightness = 255,
	.dft_brightness = 255,
	.pwm_period_ns  = 78770,
};

static struct platform_device sm5s4210_backlight_device = {
	.name      = "pwm-backlight",
	.id        = -1,
	.dev        = {
		.parent = &s3c_device_timer[0].dev,
		.platform_data = &sm5s4210_backlight_data,
	},
};
static void __init sm5s4210_backlight_register(void)
{
	int ret;
#ifdef CONFIG_HAVE_PWM
	int i, ntimer;

	/* Timer GPIO Set */
	ntimer = ARRAY_SIZE(pwm_data);
	for (i = 0; i < ntimer; i++) {
		if (gpio_is_valid(pwm_data[i].gpio_no)) {
			ret = gpio_request(pwm_data[i].gpio_no,
				pwm_data[i].gpio_name);
			if (ret) {
				printk(KERN_ERR "failed to get GPIO for PWM\n");
				return;
			}
			s3c_gpio_cfgpin(pwm_data[i].gpio_no,
				pwm_data[i].gpio_set_value);

			gpio_free(pwm_data[i].gpio_no);
		}
	}
#endif

	ret = platform_device_register(&sm5s4210_backlight_device);
	if (ret)
		printk(KERN_ERR "sm5s4210: failed to register backlight device: %d\n", ret);
}
#endif	// defined(CONFIG_BACKLIGHT_PWM)

#ifdef CONFIG_S5P_ADC
static struct s3c_adc_mach_info s3c_adc_platform __initdata = {
        /* s5pc210 support 12-bit resolution */
        .delay  = 10000,
        .presc  = 49,
        .resolution = 12,
};
#endif

#ifdef CONFIG_PROXIMITY_SENSOR
static struct resource proximity_resource[] = {
};

struct platform_device proximity_device = {
	.name	= "proximity",
	.id	= -1,
	.num_resources = ARRAY_SIZE(proximity_resource),
	.resource = proximity_resource,
};
EXPORT_SYMBOL(proximity_device);
#endif

#if defined(CONFIG_FB_S3C_LTP700WV) || defined(CONFIG_FB_S3C_LMS700JF04) || defined(CONFIG_FB_S3C_NS070L2C)
static struct s3c_platform_fb lcd_data __initdata = {
	.hw_ver = 0x70,
	.clk_name = "sclk_lcd",
	.nr_wins = 5,
	.default_win = CONFIG_FB_S3C_DEFAULT_WINDOW,
	.swap = FB_SWAP_HWORD | FB_SWAP_WORD,
};

int reset_lcd(void)
{
	int err;

	err = gpio_request(S5PV310_GPX2(1), "GPX2(1)");
	if (err) {
		printk(KERN_ERR "failed to request GPX2(1) for "
				"lcd reset control\n");
		return err;
	}

	gpio_direction_output(S5PV310_GPX2(1), 1);
	mdelay(100);

	gpio_set_value(S5PV310_GPX2(1), 0);
	mdelay(10);

	gpio_set_value(S5PV310_GPX2(1), 1);
	mdelay(10);

	gpio_free(S5PV310_GPX2(1));

	return 0;
}
#endif
#if defined(CONFIG_FB_S3C_HDMI_UI_FB)
static struct s3c_platform_fb hdmi_ui_fb_data __initdata = {
	.hw_ver = 0x70,
	.clk_name = "sclk_lcd",
	.nr_wins = 5,
	.default_win = CONFIG_FB_S3C_DEFAULT_WINDOW,
	.swap = FB_SWAP_HWORD | FB_SWAP_WORD,
};
#endif

#if defined (CONFIG_BATTERY_MAX17040)
#include <linux/max17040_battery.h>
static void charger_gpio_init(void)
{
	u32 err;

	// M1 b'd Adapter Detect Pin : High active
	err = gpio_request(GPIO_M1_ADAPTER_ONLINE, "GPX1(4)");
	if (err) {
		printk(KERN_INFO "gpio request error : %d\n", err);
	} else {
		s3c_gpio_cfgpin(GPIO_M1_ADAPTER_ONLINE, S3C_GPIO_INPUT );
		s3c_gpio_setpull(GPIO_M1_ADAPTER_ONLINE, S3C_GPIO_PULL_DOWN);
		gpio_free(GPIO_M1_ADAPTER_ONLINE);
	}

	// Battery b'd Detect Pin : High active
	err = gpio_request(GPIO_BATTERY_ONLINE, "GPX2(6)");
	if (err) {
		printk(KERN_INFO "gpio request error : %d\n", err);
	} else {
		s3c_gpio_cfgpin(GPIO_BATTERY_ONLINE, S3C_GPIO_INPUT );
		s3c_gpio_setpull(GPIO_BATTERY_ONLINE, S3C_GPIO_PULL_DOWN);
		gpio_free(GPIO_BATTERY_ONLINE);
	}

	// Charge status : Low active
	err = gpio_request(GPIO_CHARGER_STATUS, "GPX3(3)");
	if (err) {
		printk(KERN_INFO "gpio request error : %d\n", err);
	} else {
		s3c_gpio_cfgpin(GPIO_CHARGER_STATUS, S3C_GPIO_INPUT );
		s3c_gpio_setpull(GPIO_CHARGER_STATUS, S3C_GPIO_PULL_UP);
		gpio_free(GPIO_CHARGER_STATUS);
	}

	// Charger Detect Pin : Low active
	err = gpio_request(GPIO_CHARGER_ONLINE, "GPX3(4)");
	if (err) {
		printk(KERN_INFO "gpio request error : %d\n", err);
	} else {
		s3c_gpio_cfgpin(GPIO_CHARGER_ONLINE, S3C_GPIO_INPUT );
		s3c_gpio_setpull(GPIO_CHARGER_ONLINE, S3C_GPIO_PULL_UP);
		gpio_free(GPIO_CHARGER_ONLINE);
	}
}

static int adapter_online(void)
{
	int ret;

	gpio_request(GPIO_M1_ADAPTER_ONLINE, "GPX1(4)");
	gpio_direction_input(GPIO_M1_ADAPTER_ONLINE);
	ret = gpio_get_value(GPIO_M1_ADAPTER_ONLINE); 
	gpio_free(GPIO_M1_ADAPTER_ONLINE);

//printk("Adapter_online, ret=%d\r\n",ret);
		
	return ret;
}

static int battery_online(void)
{
	int ret;

	gpio_request(GPIO_BATTERY_ONLINE, "GPX2(6)");
	gpio_direction_input(GPIO_BATTERY_ONLINE);
	ret = gpio_get_value(GPIO_BATTERY_ONLINE);
	gpio_free(GPIO_BATTERY_ONLINE);
		
//printk("battery_online,ret=%d\r\n",ret);
	return ret;
}

static int charger_online(void)
{
	int ret;

	gpio_request(GPIO_CHARGER_ONLINE, "GPX3(4)");
	gpio_direction_input(GPIO_CHARGER_ONLINE);
	ret = gpio_get_value(GPIO_CHARGER_ONLINE) ? 0 : 1;
	gpio_free(GPIO_CHARGER_ONLINE);


//printk("charger_online,ret=%d\r\n",ret);
	return ret;
}


static int charger_done(void)
{
	int ret;

	gpio_request(GPIO_CHARGER_STATUS, "GPX3(3)");
	gpio_direction_input(GPIO_CHARGER_STATUS);
	ret = gpio_get_value(GPIO_CHARGER_STATUS);
	gpio_free(GPIO_CHARGER_STATUS);
		
//printk("charger_done,ret=%d\r\n",ret);
	return ret;
}

static struct max17040_platform_data max17040_platform_data = {
	.charger_online = charger_online,
	.charger_done = charger_done,
	.battery_online = battery_online,
	.adapter_online = adapter_online,
};
#endif	//

#endif	// defined(CONFIG_BOARD_SM5S4210)

#ifdef CONFIG_TOUCHSCREEN_ZINITIX_BT4X2
#define GPIO_TOUCH_PIN_NUM      S5PV310_GPX3(5)
#define GPIO_TOUCH_RESET_PIN_NUM      S5PV310_GPB(0)

void smdkc210_zinitix_set(void)
{
	u32 err;
	printk("################################ smdkc210_zinitix_set \n"); 
	gpio_free(GPIO_TOUCH_PIN_NUM);
	err = gpio_request(GPIO_TOUCH_PIN_NUM, "zinitix touch int");
	if (err) {
		printk(KERN_INFO "gpio request error : %d\n", err);
	} else {
		s3c_gpio_cfgpin(GPIO_TOUCH_PIN_NUM, S3C_GPIO_SFN(0xF));
		s3c_gpio_setpull(GPIO_TOUCH_PIN_NUM, S3C_GPIO_PULL_UP);
		gpio_free(GPIO_TOUCH_PIN_NUM);
	}

}
EXPORT_SYMBOL(smdkc210_zinitix_set);

void smdkc210_zinitix_reset(void)
{
        u32 err;

        printk("################################ smdkc210_zinitix_reset \n");
	gpio_free(GPIO_TOUCH_RESET_PIN_NUM);
        err = gpio_request(GPIO_TOUCH_RESET_PIN_NUM, "zinitix touch reset");
        if (err) {
                printk(KERN_INFO "gpio request error : %d\n", err);
        } else {
                s3c_gpio_cfgpin(GPIO_TOUCH_RESET_PIN_NUM, S3C_GPIO_OUTPUT);
                s3c_gpio_setpull(GPIO_TOUCH_RESET_PIN_NUM, S3C_GPIO_PULL_UP);
                gpio_set_value(GPIO_TOUCH_RESET_PIN_NUM, 0);
                mdelay(50);
                gpio_set_value(GPIO_TOUCH_RESET_PIN_NUM, 1);
                mdelay(500);
                gpio_free(GPIO_TOUCH_RESET_PIN_NUM);
        }
}
EXPORT_SYMBOL(smdkc210_zinitix_reset);


/* Capacitive Sensing interface */
static struct resource s3c_zinitix_resources[] = {
	[0] = {
		.start = IRQ_EINT(29),
		.end   = IRQ_EINT(29),
		.flags = IORESOURCE_IRQ,
	},
};


struct platform_device s3c_device_zinitix = {
	.name           = "zinitix",
	.id             =  -1,
	.num_resources  = ARRAY_SIZE(s3c_zinitix_resources),
	.resource       = s3c_zinitix_resources,
};
#endif

#ifdef CONFIG_I2C_S3C2410
/* I2C0 */
static struct i2c_board_info i2c_devs0[] __initdata = {
#if defined(CONFIG_REGULATOR_MAX8997)
	{
		/* The address is 0xCC used since SRAD = 0 */
		I2C_BOARD_INFO("max8997", (0xCC >> 1)),
		.platform_data = &max8997_pdata,
	},
#endif
};
#endif

#ifdef CONFIG_S3C_DEV_I2C1

#if defined (CONFIG_BATTERY_MAX17040)
#include	<linux/i2c-gpio.h>

#define		GPIO_I2C1_SDA	S5PV310_GPD1(2)
#define		GPIO_I2C1_SCL	S5PV310_GPD1(3)

static struct 	i2c_gpio_platform_data 	i2c1_gpio_platdata = {
	.sda_pin = GPIO_I2C1_SDA, // gpio number
	.scl_pin = GPIO_I2C1_SCL,
	.udelay  = 15,
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.scl_is_output_only = 0
};

static struct 	platform_device 	i2c1_gpio_device = {
	.name 	= "i2c-gpio",
	.id  	= 1, // adopter number
	.dev.platform_data = &i2c1_gpio_platdata,
};
#endif	//#if defined (CONFIG_BATTERY_MAX17040)

/* I2C1 */
static struct i2c_board_info i2c_devs1[] __initdata = {
	#ifdef CONFIG_VIDEO_TVOUT
		{
			I2C_BOARD_INFO("s5p_ddc", (0x74 >> 1)),
		},
	#endif

	#if defined (CONFIG_BATTERY_MAX17040)
		{
			I2C_BOARD_INFO("max17040", 0x36), 
			.platform_data = &max17040_platform_data,
		},
	#endif
};
#endif

#ifdef CONFIG_S3C_DEV_I2C2

//JNJ #define	GPIO_I2C2_CONTROL
#undef GPIO_I2C2_CONTROL

#if defined(GPIO_I2C2_CONTROL)

#include	<linux/i2c-gpio.h>

#define		GPIO_I2C2_SDA	S5PV310_GPA0(6)
#define		GPIO_I2C2_SCL	S5PV310_GPA0(7)

static struct 	i2c_gpio_platform_data 	i2c2_gpio_platdata = {
	.sda_pin = GPIO_I2C2_SDA, // gpio number
	.scl_pin = GPIO_I2C2_SCL,
	.udelay  = 2,
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.scl_is_output_only = 0
};

static struct 	platform_device 	i2c2_gpio_device = {
	.name 	= "i2c-gpio",
	.id  	= 2, // adepter number
	.dev.platform_data = &i2c2_gpio_platdata,
};
#endif	//#if defined(GPIO_I2C2_CONTROL)

//JNJ #define	GPIO_I2C2_CONTROL

#if defined(CONFIG_MPU_SENSORS_MPU3050) 

#define SENSOR_MPU_NAME "mpu3050"

static struct mpu3050_platform_data mpu_data = {
        .int_config  = 0x10,
        .orientation = {  -1,  0,  0,
                           0,  1,  0,
                           0,  0, -1 },
//	.level_shifter	= 0,

};
#endif

/* I2C2 */
static struct i2c_board_info i2c_devs2[] __initdata = {
// Sensors
#ifdef CONFIG_INPUT_IM3640_I2C
        {
                I2C_BOARD_INFO("im3640", (0x50 >> 1)),
        },
#endif
// Gyroscope
	#if defined(CONFIG_MPU_SENSORS_MPU3050) 
	{
		I2C_BOARD_INFO(SENSOR_MPU_NAME, 0x68),
		.irq = S5PV310_GPX3(2),
		.platform_data = &mpu_data,
	},
	#endif //#ifdef CONFIG_MPU_SENSORS_MPU3050                    		
};

#if defined(CONFIG_PN544)
static unsigned int nfc_gpio_table[][4] = {
        {GPIO_NFC_IRQ, S3C_GPIO_INPUT, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN},
        {GPIO_NFC_EN, S3C_GPIO_OUTPUT, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE},
        {GPIO_NFC_FIRM, S3C_GPIO_OUTPUT, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE},
};

void nfc_setup_gpio(void)
{
        int array_size = ARRAY_SIZE(nfc_gpio_table);
        u32 i, gpio;
        for (i = 0; i < array_size; i++) {
                gpio = nfc_gpio_table[i][0];
                s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(nfc_gpio_table[i][1]));
                s3c_gpio_setpull(gpio, nfc_gpio_table[i][3]);
                if (nfc_gpio_table[i][2] != GPIO_LEVEL_NONE)
                        gpio_set_value(gpio, nfc_gpio_table[i][2]);
        }
}


static struct pn544_i2c_platform_data pn544_pdata = {
        .irq_gpio = GPIO_NFC_IRQ,
        .ven_gpio = GPIO_NFC_EN,
        .firm_gpio = GPIO_NFC_FIRM,
};

#endif


#ifdef CONFIG_S3C_DEV_I2C3
/* I2C3 */
static struct i2c_board_info i2c_devs3[] __initdata = {
        {
                I2C_BOARD_INFO("wm8580", 0x1b),
        },
#if defined(CONFIG_PN544)
        {
                I2C_BOARD_INFO("pn544", 0x2b),
		.irq = IRQ_EINT(24),
		.platform_data = &pn544_pdata,
        },
#endif

#if defined(CONFIG_ECT223H_3D_CONVERTER)
        {
                I2C_BOARD_INFO("ect223h", 0xe8 >> 1),
        },
#endif
};
#endif

#if defined(CONFIG_FB_S3C_NS070L2C)
static unsigned int ns070_gpio_table[4] = {
        GPIO_3DLCD_RESET, S3C_GPIO_OUTPUT, GPIO_LEVEL_LOW, S3C_GPIO_PULL_DOWN
};

void ns070_gpio_setup(void)
{
	u32 gpio;

        gpio = ns070_gpio_table[0];
        s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(ns070_gpio_table[1]));
        s3c_gpio_setpull(gpio, ns070_gpio_table[3]);
        if (ns070_gpio_table[2] != GPIO_LEVEL_NONE)
        	gpio_set_value(gpio, ns070_gpio_table[2]);
}

void ns070_reset(int on)
{
        gpio_set_value(ns070_gpio_table[0], on ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
}

EXPORT_SYMBOL(ns070_reset);
#endif

#if defined(CONFIG_ECT223H_3D_CONVERTER)
static unsigned int ect223h_gpio_table[4] = {
        GPIO_3DCHIP_RESET, S3C_GPIO_OUTPUT, GPIO_LEVEL_LOW, S3C_GPIO_PULL_DOWN
};

void ect223h_gpio_setup(void)
{
	u32 gpio;

        gpio = ect223h_gpio_table[0];
        s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(ect223h_gpio_table[1]));
        s3c_gpio_setpull(gpio, ect223h_gpio_table[3]);
        if (ect223h_gpio_table[2] != GPIO_LEVEL_NONE)
        	gpio_set_value(gpio, ect223h_gpio_table[2]);
}

void ect223h_reset(int on)
{
        gpio_set_value(ect223h_gpio_table[0], on ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
}

EXPORT_SYMBOL(ect223h_reset);
#endif

#ifdef CONFIG_S3C_DEV_I2C4
/* I2C4 */
static struct i2c_board_info i2c_devs4[] __initdata = {
#ifdef CONFIG_TOUCHSCREEN_ZINITIX_BT4X2
        {
                I2C_BOARD_INFO("zinitix_touch", (0x40>>1)),
        },
        {
                I2C_BOARD_INFO("zinitix_isp", (0xA0>>1)),
        },
#endif
};
#endif

#endif	// #ifdef CONFIG_I2C_S3C2410


#ifdef CONFIG_S3C_DEV_HSMMC
static struct s3c_sdhci_platdata smdkc210_hsmmc0_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
#if defined(CONFIG_S5PV310_SD_CH0_8BIT)
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA,
#endif
};
#endif
#ifdef CONFIG_S3C_DEV_HSMMC1
static struct s3c_sdhci_platdata smdkc210_hsmmc1_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
};
#endif
#ifdef CONFIG_S3C_DEV_HSMMC2
static struct s3c_sdhci_platdata smdkc210_hsmmc2_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
#if defined(CONFIG_S5PV310_SD_CH2_8BIT)
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA,
#endif
};
#endif
#ifdef CONFIG_S3C_DEV_HSMMC3
static struct s3c_sdhci_platdata smdkc210_hsmmc3_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
	.wp_gpio		= S5PV310_GPX0(5),
	.has_wp_gpio		= true,
};
#endif
#ifdef CONFIG_S5P_DEV_MSHC
static struct s3c_mshci_platdata smdkc210_mshc_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
#if defined(CONFIG_S5PV310_MSHC_CH0_8BIT) && \
defined(CONFIG_S5PV310_MSHC_CH0_DDR)
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA | MMC_CAP_DDR,
#elif defined(CONFIG_S5PV310_MSHC_CH0_8BIT)
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA,
#elif defined(CONFIG_S5PV310_MSHC_CH0_DDR)
	.host_caps				= MMC_CAP_DDR,
#endif
};
#endif

#ifdef CONFIG_VIDEO_FIMG2D
static struct fimg2d_platdata fimg2d_data __initdata = {
	.hw_ver = 30,
	.parent_clkname = "mout_mpll",
	.clkname = "sclk_fimg2d",
	.gate_clkname = "fimg2d",
	.clkrate = 250 * 1000000,
};
#endif

#ifdef CONFIG_ANDROID_PMEM
static struct android_pmem_platform_data pmem_pdata = {
	.name = "pmem",
	.no_allocator = 1,
	.cached = 0,
	.start = 0, // will be set during proving pmem driver.
	.size = 0 // will be set during proving pmem driver.
};

static struct android_pmem_platform_data pmem_gpu1_pdata = {
	.name = "pmem_gpu1",
	.no_allocator = 1,
	.cached = 0,
	/* .buffered = 1, */
	.start = 0,
	.size = 0,
};

static struct platform_device pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &pmem_pdata },
};

static struct platform_device pmem_gpu1_device = {
	.name = "android_pmem",
	.id = 1,
	.dev = { .platform_data = &pmem_gpu1_pdata },
};

static void __init android_pmem_set_platdata(void)
{
#if defined(CONFIG_S5P_MEM_CMA)
	pmem_pdata.size = CONFIG_ANDROID_PMEM_MEMSIZE_PMEM * SZ_1K;
	pmem_gpu1_pdata.size = CONFIG_ANDROID_PMEM_MEMSIZE_PMEM_GPU1 * SZ_1K;
#elif defined(CONFIG_S5P_MEM_BOOTMEM)
	pmem_pdata.start = (u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM, 0);
	pmem_pdata.size = (u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM, 0);

	pmem_gpu1_pdata.start = (u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM_GPU1, 0);
	pmem_gpu1_pdata.size = (u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM_GPU1, 0);
#endif
}
#endif

#ifdef CONFIG_BATTERY_S3C
struct platform_device sec_device_battery = {
	.name	= "sec-fake-battery",
	.id	= -1,
};
#endif

static struct resource pmu_resource[] = {
	[0] = {
		.start = IRQ_PMU_0,
		.end   = IRQ_PMU_0,
		.flags = IORESOURCE_IRQ,
	},
	[1] = {
		.start = IRQ_PMU_1,
		.end   = IRQ_PMU_1,
		.flags = IORESOURCE_IRQ,
	}
};

static struct platform_device pmu_device = {
	.name 		= "arm-pmu",
	.id 		= ARM_PMU_DEVICE_CPU,
	.resource 	= pmu_resource,
	.num_resources 	= 2,
};

static struct platform_device *smdkc210_devices[] __initdata = {

#ifdef CONFIG_BOARD_SM5S4210	// 
	&sm5s4210_sysfs,
#endif

#ifdef CONFIG_BACKLIGHT_PWM			// 
	&s3c_device_timer[0],
	&s3c_device_timer[2],
#endif

#ifdef CONFIG_S5PV310_DEV_PD
	&s5pv310_device_pd[PD_MFC],
	&s5pv310_device_pd[PD_G3D],
	&s5pv310_device_pd[PD_LCD0],
	&s5pv310_device_pd[PD_LCD1],
	&s5pv310_device_pd[PD_CAM],
	&s5pv310_device_pd[PD_GPS],
	&s5pv310_device_pd[PD_TV],
	/* &s5pv310_device_pd[PD_MAUDIO], */
#endif

#ifdef CONFIG_BATTERY_S3C
	&sec_device_battery,
#endif

#ifdef CONFIG_FB_S3C
	&s3c_device_fb,
#endif

#ifdef CONFIG_S5P_ADC
        &s3c_device_adc,
#endif

#ifdef CONFIG_I2C_S3C2410
	&s3c_device_i2c0,
#if defined(CONFIG_S3C_DEV_I2C1)
//	#if defined (CONFIG_BATTERY_MAX17040)
//		&i2c1_gpio_device,
//	#else
		&s3c_device_i2c1,
//	#endif
#endif
#if defined(CONFIG_S3C_DEV_I2C2)
	#if defined(GPIO_I2C2_CONTROL)
		&i2c2_gpio_device,
	#else
		&s3c_device_i2c2,
	#endif
#endif

	#if defined(CONFIG_S3C_DEV_I2C3)
		&s3c_device_i2c3,
	#endif
	#if defined(CONFIG_S3C_DEV_I2C4)
		&s3c_device_i2c4,
	#endif
#endif	// #ifdef CONFIG_I2C_S3C2410

#ifdef CONFIG_SND_S3C64XX_SOC_I2S_V4
	&s5pv310_device_iis0,
#endif
#ifdef CONFIG_SND_S3C_SOC_PCM
	&s5pv310_device_pcm1,
#endif

#ifdef CONFIG_SND_SAMSUNG_SOC_SPDIF
	&s5pv310_device_spdif,
#endif

#ifdef CONFIG_SND_S5P_RP
	&s5pv310_device_rp,
#endif

#ifdef CONFIG_S3C_DEV_HSMMC
	&s3c_device_hsmmc0,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC1
	&s3c_device_hsmmc1,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC2
	&s3c_device_hsmmc2,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC3
	&s3c_device_hsmmc3,
#endif
#ifdef CONFIG_S5P_DEV_MSHC
	&s3c_device_mshci,
#endif

#ifdef CONFIG_VIDEO_TVOUT
	&s5p_device_tvout,
	&s5p_device_cec,
	&s5p_device_hpd,
#endif

#ifdef CONFIG_S3C2410_WATCHDOG
	&s3c_device_wdt,
#endif

#ifdef CONFIG_SMC911X
	&smdkc210_smc911x,
#endif

#ifdef CONFIG_USB
	&s3c_device_usb_ehci,
	&s3c_device_usb_ohci,
#endif

#ifdef CONFIG_ANDROID_PMEM
	&pmem_device,
	&pmem_gpu1_device,
#endif

#ifdef CONFIG_VIDEO_FIMC
	&s3c_device_fimc0,
	&s3c_device_fimc1,
	&s3c_device_fimc2,
	&s3c_device_fimc3,
#ifdef CONFIG_VIDEO_FIMC_MIPI
	&s3c_device_csis0,
	&s3c_device_csis1,
#endif
#endif
#ifdef CONFIG_VIDEO_JPEG
	&s5p_device_jpeg,
#endif
#if defined(CONFIG_VIDEO_MFC50) || defined(CONFIG_VIDEO_MFC5X)
	&s5p_device_mfc,
#endif

#ifdef CONFIG_VIDEO_FIMG2D
	&s5p_device_fimg2d,
#endif

#ifdef CONFIG_USB_GADGET
	&s3c_device_usbgadget,
#endif
#ifdef CONFIG_USB_ANDROID_RNDIS
	&s3c_device_rndis,
#endif
#ifdef CONFIG_USB_ANDROID
	&s3c_device_android_usb,
	&s3c_device_usb_mass_storage,
#endif

#if defined(CONFIG_S3C64XX_DEV_SPI)
	&s5pv310_device_spi1,

#elif defined(CONFIG_SPI_S5PV310)
	&s3c_device_spi1,
	
#else //mtv350 
	&s3c_device_spi_gpio,
#endif

#ifdef CONFIG_S5P_SYSMMU
	&s5p_device_sysmmu,
#endif

#ifdef CONFIG_S3C_DEV_GIB
	&s3c_device_gib,
#endif

#ifdef CONFIG_S3C_DEV_RTC
	&s3c_device_rtc,
#endif

	&s5p_device_ace,

#ifdef CONFIG_DEV_THERMAL
	&s5p_device_tmu,
#endif

	&pmu_device,


#ifdef CONFIG_SERIAL_8250_HANBACK
  &st16c554_uart_device,
  &st16c554_uart_device1,
  &st16c554_uart_device2,
  &st16c554_uart_device3,
#endif

#ifdef CONFIG_PROXIMITY_SENSOR
  &proximity_device,
#endif
};

#if defined(CONFIG_VIDEO_TVOUT)
static struct s5p_platform_hpd hdmi_hpd_data __initdata = {

};
static struct s5p_platform_cec hdmi_cec_data __initdata = {

};
#endif

#ifdef CONFIG_SMC911X
static void __init smdkc210_smc911x_init(void)
{
	u32 err;

	err = gpio_request(S5PV310_GPX0(7), "GPX0(7)");
	if (err) {
		printk(KERN_INFO "gpio request error : %d\n", err);
	} else {
		s3c_gpio_cfgpin(S5PV310_GPX0(7), S3C_GPIO_SFN(0xF));
		s3c_gpio_setpull(S5PV310_GPX0(7), S3C_GPIO_PULL_UP);
		gpio_free(S5PV310_GPX0(7));
	}
}
#endif

#if defined(CONFIG_S5P_MEM_CMA)
static void __init s5pv310_reserve(void);
#endif
static void __init smdkc210_map_io(void)
{
	s5p_init_io(NULL, 0, S5P_VA_CHIPID);
	s3c24xx_init_clocks(24000000);
	s3c24xx_init_uarts(smdkc210_uartcfgs, ARRAY_SIZE(smdkc210_uartcfgs));

#if defined(CONFIG_S5P_MEM_CMA)
	s5pv310_reserve();
#elif defined(CONFIG_S5P_MEM_BOOTMEM)
	s5p_reserve_bootmem();
#endif
}

static void __init smdkc210_machine_init(void)
{
#if defined(CONFIG_SPI_S5PV310)
	struct clk *sclk = NULL;
	struct clk *prnt = NULL;
	struct device *spi1_dev = &s3c_device_spi1.dev;

#elif defined(CONFIG_S3C64XX_DEV_SPI)
	struct clk *sclk = NULL;
	struct clk *prnt = NULL;
	struct device *spi1_dev = &s5pv310_device_spi1.dev;
#endif	//#if defined(CONFIG_S3C64XX_DEV_SPI)

#ifdef CONFIG_ANDROID_PMEM
	android_pmem_set_platdata();
#endif
	s3c_pm_init();

#if defined(CONFIG_S5PV310_DEV_PD) && !defined(CONFIG_PM_RUNTIME)
	/*
	 * These power domains should be always on
	 * without runtime pm support.
	 */
	s5pv310_pd_enable(&s5pv310_device_pd[PD_MFC].dev);
	s5pv310_pd_enable(&s5pv310_device_pd[PD_G3D].dev);
	s5pv310_pd_enable(&s5pv310_device_pd[PD_LCD0].dev);
	s5pv310_pd_enable(&s5pv310_device_pd[PD_LCD1].dev);
	s5pv310_pd_enable(&s5pv310_device_pd[PD_CAM].dev);
	s5pv310_pd_enable(&s5pv310_device_pd[PD_TV].dev);
	s5pv310_pd_enable(&s5pv310_device_pd[PD_GPS].dev);
#endif

#ifdef CONFIG_SMC911X
	smdkc210_smc911x_init();
#endif

#if defined(CONFIG_S5P_ADC)
        s3c_adc_set_platdata(&s3c_adc_platform);
#endif

#ifdef CONFIG_I2C_S3C2410
	s3c_i2c0_set_platdata(NULL);
	i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));

#ifdef CONFIG_S3C_DEV_I2C1
	s3c_i2c1_set_platdata(NULL);
	i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));
#endif

#ifdef CONFIG_S3C_DEV_I2C2
	// Sensor MPU3050 INT
	if(gpio_request(S5PV310_GPX3(2), "MPU3050 INT"))	printk("MPU3050 INT(GPX3(2) Port request error!!!\n");
	else	{
//		s3c_gpio_cfgpin(S5PV310_GPX3(2), S3C_GPIO_SFN(0xF));
		s3c_gpio_setpull(S5PV310_GPX3(2), S3C_GPIO_PULL_DOWN);
		gpio_direction_input(S5PV310_GPX3(2));
		gpio_free(S5PV310_GPX3(2));
	}

	s3c_i2c2_set_platdata(NULL);
	i2c_register_board_info(2, i2c_devs2, ARRAY_SIZE(i2c_devs2));
#endif
	#ifdef CONFIG_S3C_DEV_I2C3
#if defined(CONFIG_PN544)
		nfc_setup_gpio();
#endif
#if defined(CONFIG_FB_S3C_NS070L2C)
	ns070_gpio_setup();
#endif
#if defined(CONFIG_ECT223H_3D_CONVERTER)
	ect223h_gpio_setup();
#endif
		s3c_i2c3_set_platdata(NULL);
		i2c_register_board_info(3, i2c_devs3, ARRAY_SIZE(i2c_devs3));
	#endif
	#ifdef CONFIG_S3C_DEV_I2C4
		s3c_i2c4_set_platdata(NULL);
		i2c_register_board_info(4, i2c_devs4, ARRAY_SIZE(i2c_devs4));
	#endif
#endif	// #ifdef CONFIG_I2C_S3C2410

#if 0
//open the rtc external clock for GPS
	static void __iomem *s3c_rtc_base;
	s3c_rtc_base = ioremap(0x10070000, 0xff + 1);

	if (s3c_rtc_base == NULL) {
		printk("Error: no memory mapping for rtc\n");
	}
	__raw_writel(0x200, s3c_rtc_base + 0x40);
	iounmap(s3c_rtc_base);
#endif

#ifdef CONFIG_VIDEO_FIMC
	/* fimc */
	s3c_fimc0_set_platdata(&fimc_plat);
	s3c_fimc1_set_platdata(&fimc_plat);
	s3c_fimc2_set_platdata(&fimc_plat);
	s3c_fimc3_set_platdata(&fimc_plat);
#ifdef CONFIG_ITU_A
	smdkv310_cam0_reset(1);
#endif
#endif

#ifdef CONFIG_VIDEO_MFC5X
#ifdef CONFIG_S5PV310_DEV_PD
	s5p_device_mfc.dev.parent = &s5pv310_device_pd[PD_MFC].dev;
#endif
#endif

#ifdef CONFIG_VIDEO_FIMG2D
	s5p_fimg2d_set_platdata(&fimg2d_data);
#ifdef CONFIG_S5PV310_DEV_PD
	s5p_device_fimg2d.dev.parent = &s5pv310_device_pd[PD_LCD0].dev;
#endif
#endif
#ifdef CONFIG_S3C_DEV_HSMMC
	s3c_sdhci0_set_platdata(&smdkc210_hsmmc0_pdata);
#endif
#ifdef CONFIG_S3C_DEV_HSMMC1
	s3c_sdhci1_set_platdata(&smdkc210_hsmmc1_pdata);
#endif
#ifdef CONFIG_S3C_DEV_HSMMC2
	s3c_sdhci2_set_platdata(&smdkc210_hsmmc2_pdata);
#endif
#ifdef CONFIG_S3C_DEV_HSMMC3
	s3c_sdhci3_set_platdata(&smdkc210_hsmmc3_pdata);
#endif
#ifdef CONFIG_S5P_DEV_MSHC
	s3c_mshci_set_platdata(&smdkc210_mshc_pdata);
#endif

#ifdef CONFIG_DEV_THERMAL
	s5p_tmu_set_platdata(NULL);
#endif

#if defined(CONFIG_VIDEO_TVOUT)
	s5p_hdmi_hpd_set_platdata(&hdmi_hpd_data);
	s5p_hdmi_cec_set_platdata(&hdmi_cec_data);

#ifdef CONFIG_S5PV310_DEV_PD
	s5p_device_tvout.dev.parent = &s5pv310_device_pd[PD_TV].dev;
#endif
#endif

#ifdef CONFIG_S5PV310_DEV_PD
#ifdef CONFIG_FB_S3C
	s3c_device_fb.dev.parent = &s5pv310_device_pd[PD_LCD0].dev;
#endif
#endif

#ifdef CONFIG_S5PV310_DEV_PD
#ifdef CONFIG_VIDEO_FIMC
	s3c_device_fimc0.dev.parent = &s5pv310_device_pd[PD_CAM].dev;
	s3c_device_fimc1.dev.parent = &s5pv310_device_pd[PD_CAM].dev;
	s3c_device_fimc2.dev.parent = &s5pv310_device_pd[PD_CAM].dev;
	s3c_device_fimc3.dev.parent = &s5pv310_device_pd[PD_CAM].dev;
#endif
#endif

	platform_add_devices(smdkc210_devices, ARRAY_SIZE(smdkc210_devices));

#if defined(CONFIG_S3C64XX_DEV_SPI) || defined(CONFIG_SPI_S5PV310)
	sclk = clk_get(spi1_dev, "sclk_spi");
	if (IS_ERR(sclk))	{
		dev_err(spi1_dev, "failed to get sclk for SPI-1\n");
	}

	prnt = clk_get(spi1_dev, "mout_mpll");
	if (IS_ERR(prnt))	{
		dev_err(spi1_dev, "failed to get prnt\n");
	}
	clk_set_parent(sclk, prnt);
	clk_put(prnt);

	if (!gpio_request(S5PV310_GPB(5), "SPI_CS1")) {
		gpio_direction_output(S5PV310_GPB(5), 1);
		s3c_gpio_cfgpin(S5PV310_GPB(5), S3C_GPIO_SFN(1));
		s3c_gpio_setpull(S5PV310_GPB(5), S3C_GPIO_PULL_UP);

	#if defined(CONFIG_SPI_S5PV310)
		s3cspi_set_slaves(BUSNUM(1), ARRAY_SIZE(s3c_slv_pdata_1), s3c_slv_pdata_1);
	#else
		s5pv310_spi_set_info(1, S5PV310_SPI_SRCCLK_SCLK, ARRAY_SIZE(spi1_csi));
	#endif
	}
  #if defined(CONFIG_SPI_S5PV310)
  	spi_register_board_info(s3c_spi_devs, ARRAY_SIZE(s3c_spi_devs));
  #else
	spi_register_board_info(spi1_board_info, ARRAY_SIZE(spi1_board_info));
  #endif

#endif	//#if defined(CONFIG_S3C64XX_DEV_SPI)

#if defined(CONFIG_BOARD_SM5S4210)
	pm_power_off = sm5s4210_power_off;

	// PS_HOLD Enable (LOW-> HIGH)
	(*(unsigned long *)(S5PV310_VA_PMU + 0x330C)) = 0x5300;

#if 0 //JNJ
	// LCD Disable
	if(gpio_request(S5PV310_GPD0(1), "LCD_EN"))		printk("LCD_EN(GPD0.1) Port request error!!!\n");
	else	{
		gpio_direction_output(S5PV310_GPD0(1), 1); /* LCD_EN High(Invert) : Lcd Disable */
		gpio_free(S5PV310_GPD0(1));
	}
#endif

	#if defined(CONFIG_BACKLIGHT_PWM)
#if 0
		// PWM Port Init
		if(gpio_request(S5PV310_GPD0(0), "PWM0"))	printk("PWM0(GPD0.0) Port request error!!!\n");
		else	{
			s3c_gpio_setpull(S5PV310_GPD0(0), S3C_GPIO_PULL_NONE);
			s3c_gpio_cfgpin(S5PV310_GPD0(0), S3C_GPIO_SFN(2));	
			gpio_free(S5PV310_GPD0(0));
		}
#endif
	
		sm5s4210_backlight_register();
	#endif
		
		s3cfb_set_platdata(NULL);
	#if defined(CONFIG_FB_S3C_LTP700WV) || defined(CONFIG_FB_S3C_LMS700JF04) || defined(CONFIG_FB_S3C_NS070L2C)
		reset_lcd();
		s3cfb_set_platdata(&lcd_data);
	#endif

	#if defined(CONFIG_FB_S3C_HDMI_UI_FB)
		s3cfb_set_platdata(NULL);
		s3cfb_set_platdata(&hdmi_ui_fb_data);
	#endif

#if defined (CONFIG_BATTERY_MAX17040)
	charger_gpio_init();
#endif

#ifdef CONFIG_TOUCHSCREEN_ZINITIX_BT4X2
//JNJ        smdkc210_zinitix_reset();
#endif

#endif	//#if defined(CONFIG_BOARD_SM5S4210)

}

#if defined(CONFIG_S5P_MEM_CMA)
static void __init s5pv310_reserve(void)
{
	static struct cma_region regions[] = {
#ifdef CONFIG_ANDROID_PMEM_MEMSIZE_PMEM
		{
			.name = "pmem",
			.size = CONFIG_ANDROID_PMEM_MEMSIZE_PMEM * SZ_1K,
			.start = 0,
		},
#endif
#ifdef CONFIG_ANDROID_PMEM_MEMSIZE_PMEM_GPU1
		{
			.name = "pmem_gpu1",
			.size = CONFIG_ANDROID_PMEM_MEMSIZE_PMEM_GPU1 * SZ_1K,
			.start = 0,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMD
		{
			.name = "fimd",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMD * SZ_1K,
		.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC
		{
			.name = "mfc",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC * SZ_1K,
			.start = 0,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC0
		{
			.name = "mfc0",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC0 * SZ_1K,
			{
				.alignment = 1 << 17,
			},
			.start = 0,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC1
		{
			.name = "mfc1",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC1 * SZ_1K,
			{
				.alignment = 1 << 17,
			},
			.start = 0,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC0
		{
			.name = "fimc0",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC0 * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC1
		{
			.name = "fimc1",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC1 * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC2
		{
			.name = "fimc2",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC2 * SZ_1K,
			.start = 0,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC3
		{
			.name = "fimc3",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC3 * SZ_1K,
			.start = 0,
		},
#endif
#ifdef CONFIG_AUDIO_SAMSUNG_MEMSIZE_SRP
		{
			.name = "srp",
			.size = CONFIG_AUDIO_SAMSUNG_MEMSIZE_SRP * SZ_1K,
			.start = 0,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_JPEG
		{
			.name = "jpeg",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_JPEG * SZ_1K,
			.start = 0,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMG2D
       {
           .name = "fimg2d",
           .size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMG2D * SZ_1K,
           .start = 0,
       },
#endif
		{}
	};

	static const char map[] __initconst =
		"android_pmem.0=pmem;android_pmem.1=pmem_gpu1;s3cfb.0=fimd;"
		"s3c-fimc.0=fimc0;s3c-fimc.1=fimc1;s3c-fimc.2=fimc2;"
		"s3c-fimc.3=fimc3;s3c-mfc=mfc,mfc0,mfc1;s5p-rp=srp;"
		"s5p-jpeg=jpeg;"
		"s5p-fimg2d=fimg2d";
	int i = 0;
	unsigned int bank0_end = meminfo.bank[4].start +
					meminfo.bank[4].size;
//	unsigned int bank1_end = meminfo.bank[1].start +
//					meminfo.bank[1].size;

	printk(KERN_ERR "mem infor: bank4 start-> 0x%x, bank4 size-> 0x%x\n",\
//			\nbank1 start-> 0x%x, bank1 size-> 0x%x\n",
//			(int)meminfo.bank[0].start, (int)meminfo.bank[0].size,
			(int)meminfo.bank[4].start, (int)meminfo.bank[4].size);
	for (i = ARRAY_SIZE(regions) - 1; i >= 0; i--) {
		if (!regions[i].name)
			continue;
		if (regions[i].start == 0) {
			regions[i].start = bank0_end - regions[i].size;
			bank0_end = regions[i].start;
		}
//		else if (regions[i].start == 1) {
//			regions[i].start = bank1_end - regions[i].size;
//			bank1_end = regions[i].start;
//		}
		printk(KERN_ERR "CMA reserve : %s, addr is 0x%x, size is 0x%x\n",
			regions[i].name, regions[i].start, regions[i].size);
	}

	cma_set_defaults(regions, map);
	cma_early_regions_reserve(NULL);
}
#endif

//MACHINE_START(SMDKC210, "SMDKC210")
MACHINE_START(SM5S4210, "SM5S4210")
	/* Maintainer: Kukjin Kim <kgene.kim@samsung.com> */
	.phys_io	= S3C_PA_UART & 0xfff00000,
	.io_pg_offst	= (((u32)S3C_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S5P_PA_SDRAM + 0x100,
	.init_irq	= s5pv310_init_irq,
	.map_io		= smdkc210_map_io,
	.init_machine	= smdkc210_machine_init,
	.timer		= &s5pv310_timer,
MACHINE_END
