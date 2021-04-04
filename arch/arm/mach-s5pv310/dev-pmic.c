/* linux/arch/arm/mach-s5pv310/dev-pmic.c
 *
 * MAX8997 PMIC platform data.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/max8997.h>

#include <mach/gpio.h>


//static struct regulator_consumer_supply ldo1_consumer[] = {
//	REGULATOR_SUPPLY("vdd_adc", NULL),
//};
//static struct regulator_consumer_supply ldo2_consumer[] = {
//	REGULATOR_SUPPLY("vdd_alive", NULL),
//};
//static struct regulator_consumer_supply ldo3_consumer[] = {
//	REGULATOR_SUPPLY("vdd_otgd", NULL),
//};
static struct regulator_consumer_supply ldo4_consumer[] = {
	REGULATOR_SUPPLY("vdd_mipi", NULL),
};
//static struct regulator_consumer_supply ldo5_consumer[] = {
//	REGULATOR_SUPPLY("vdd_mif", NULL),
//};
static struct regulator_consumer_supply ldo6_consumer[] = {
	REGULATOR_SUPPLY("vdd_cam", NULL),
};
//static struct regulator_consumer_supply ldo7_consumer[] = {
//	REGULATOR_SUPPLY("vdd_gps", NULL),
//};
//static struct regulator_consumer_supply ldo8_consumer[] = {
//	REGULATOR_SUPPLY("vdd_otga", NULL),
//};
static struct regulator_consumer_supply ldo9_consumer[] = {
	REGULATOR_SUPPLY("vdd_lcd", NULL),
};
//static struct regulator_consumer_supply ldo10_consumer[] = {
//	REGULATOR_SUPPLY("vdd_pll", NULL),
//};

static struct regulator_consumer_supply ldo13_consumer[] = {
	REGULATOR_SUPPLY("vdd_cam1_30", NULL),
};
static struct regulator_consumer_supply ldo14_consumer[] = {
	REGULATOR_SUPPLY("vdd_cam0_18", NULL),
};
static struct regulator_consumer_supply ldo15_consumer[] = {
	REGULATOR_SUPPLY("vdd_cam0_28", NULL),
};
static struct regulator_consumer_supply ldo21_consumer[] = {
	REGULATOR_SUPPLY("vdd_cam1_15", NULL),
};

static struct regulator_consumer_supply buck1_consumer[] = {
	REGULATOR_SUPPLY("vdd_arm", NULL),
};

static struct regulator_consumer_supply buck2_consumer[] = {
	REGULATOR_SUPPLY("vdd_int", NULL),
};

static struct regulator_consumer_supply buck3_consumer[] = {
	REGULATOR_SUPPLY("vdd_g3d", NULL),
};

//static struct regulator_consumer_supply buck5_consumer[] = {
//	REGULATOR_SUPPLY("vdd_mem", NULL),
//};

static struct regulator_init_data max8997_ldo1_data = {
	.constraints	= {
		.name		= "VDD_ADC_3.3V",
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 3300000,
			.enabled = 1,
		},
	},
//	.num_consumer_supplies  = 1,
//	.consumer_supplies  = &ldo1_consumer[0],
};

static struct regulator_init_data max8997_ldo2_data = {
	.constraints	= {
		.name		= "VALIVE_1.1V",
		.min_uV		= 1100000,
		.max_uV		= 1100000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1100000,
			.enabled = 1,
		},
	},
//	.num_consumer_supplies  = 1,
//	.consumer_supplies  = &ldo2_consumer[0],
};

static struct regulator_init_data max8997_ldo3_data = {
	.constraints	= {
		.name		= "VUOTG_D_1.1V/VUHOST_D_1.1V",
		.min_uV		= 1100000,
		.max_uV		= 1100000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1100000,
			.disabled = 1,
		},
	},
//	.num_consumer_supplies  = 1,
//	.consumer_supplies  = &ldo3_consumer[0],
};

static struct regulator_init_data max8997_ldo4_data = {
	.constraints	= {
		.name		= "V_MIPI_1.8V",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1800000,
			.disabled = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo4_consumer[0],
};

static struct regulator_init_data max8997_ldo5_data = {
	.constraints	= {
		.name		= "VDD_MIF_1.8V",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1800000,
			.disabled = 1,
		},
	},
//	.num_consumer_supplies  = 1,
//	.consumer_supplies  = &ldo5_consumer[0],
};

static struct regulator_init_data max8997_ldo6_data = {
	.constraints	= {
		.name		= "VDD_CAM_1.8V",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	 = {
			.uV		= 1800000,
			.disabled = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo6_consumer[0],
};

static struct regulator_init_data max8997_ldo7_data = {
	.constraints	= {
		.name		= "VDD_GPS_1.8V",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1800000,
			.disabled = 1,
		},
	},
//	.num_consumer_supplies  = 1,
//	.consumer_supplies  = &ldo7_consumer[0],
};

static struct regulator_init_data max8997_ldo8_data = {
	.constraints	= {
		.name		= "VUOTG_A_3.3V/VUHOST_A_3.3V",
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 3300000,
			.enabled = 1,
		},
	},
//	.num_consumer_supplies  = 1,
//	.consumer_supplies  = &ldo8_consumer[0],
};

static struct regulator_init_data max8997_ldo9_data = {
	.constraints	= {
#if defined(CONFIG_FB_S3C_NS070L2C)
		.name		= "VDD_LCD_3.3V",
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE,
		.state_mem	= {
			.uV		= 3300000,
			.enabled = 1,
		},
#else
		.name		= "VDD_LCD_2.8V",
		.min_uV		= 3000000,
		.max_uV		= 3000000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE,
		.state_mem	= {
			.uV		= 3000000,
			.enabled = 1,
		},
#endif
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo9_consumer[0],
};

static struct regulator_init_data max8997_ldo10_data = {
	.constraints	= {
		.name		= "VDD_PLL_1.1V",
		.min_uV		= 1100000,
		.max_uV		= 1100000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE,
		.state_mem	= {
			.uV		= 1100000,
			.enabled = 1,
		},
	},
//	.num_consumer_supplies  = 1,
//	.consumer_supplies  = &ldo10_consumer[0],
};


static struct regulator_init_data max8997_ldo13_data = {
	.constraints	= {
		.name		= "VDD_LDO13",
		.min_uV		= 3000000,
		.max_uV		= 3000000,
		.apply_uV	= 1,
		.always_on	= 0,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 3000000,
			.enabled = 0,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo13_consumer[0],
};

static struct regulator_init_data max8997_ldo14_data = {
	.constraints	= {
		.name		= "VDD_LDO14",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1800000,
			.enabled = 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &ldo14_consumer[0],

};

static struct regulator_init_data max8997_ldo15_data = {
	.constraints	= {
		.name		= "VDD_LDO15",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.always_on	= 0,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 2800000,
			.enabled = 0,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo15_consumer[0],	
};


static struct regulator_init_data max8997_ldo21_data = {
	.constraints	= {
		.name		= "VDD_LDO21",
		.min_uV		= 1500000,
		.max_uV		= 1500000,
		.apply_uV	= 1,
		.always_on	= 0,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1500000,
			.enabled = 0,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &ldo21_consumer[0],	
};

static struct regulator_init_data max8997_buck1_data = {
	.constraints	= {
		.name		= "vdd_arm range",
		.min_uV		= 770000,
		.max_uV		= 1400000,
		.always_on	= 1,
		.boot_on	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE,
		.state_mem	= {
			.uV		= 1200000,
			.disabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &buck1_consumer[0],
};

static struct regulator_init_data max8997_buck2_data = {
	.constraints	= {
		.name		= "vdd_int range",
		.min_uV		= 750000,
		.max_uV		= 1380000,
		.always_on	= 1,
		.boot_on	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE,
		.state_mem	= {
			.uV		= 1100000,
			.disabled	= 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &buck2_consumer[0],
};

static struct regulator_init_data max8997_buck3_data = {
	.constraints	= {
		.name		= "vdd_g3d range",
		.min_uV		= 900000,
		.max_uV		= 1200000,
		.always_on	= 1,
		.boot_on	= 0,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
						  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 1100000,
			.disabled	= 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies  = &buck3_consumer[0],
};


static struct regulator_init_data max8997_buck5_data = {
	.constraints	= {
		.name		= "VDD_MEM",
		.min_uV		= 1200000,
		.max_uV		= 1200000,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.uV	= 1200000,
			.mode	= REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
	},
//	.num_consumer_supplies  = 1,
//	.consumer_supplies  = &buck5_consumer[0],
};

static struct regulator_init_data max8997_buck7_data = {
	.constraints	= {
		.name		= "VDD_IO_1.8V",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.always_on	= 1,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 1800000,
			.mode	= REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
	},
};


static struct max8997_regulator_data max8997_regulators[] = {
	{ MAX8997_LDO1,			&max8997_ldo1_data },  		
	{ MAX8997_LDO2,     	&max8997_ldo2_data },  
	{ MAX8997_LDO3,     	&max8997_ldo3_data },  
	{ MAX8997_LDO4,     	&max8997_ldo4_data },  
	{ MAX8997_LDO5,     	&max8997_ldo5_data },  
	{ MAX8997_LDO6,     	&max8997_ldo6_data },  
	{ MAX8997_LDO7,     	&max8997_ldo7_data },
	{ MAX8997_LDO8,     	&max8997_ldo8_data },
	{ MAX8997_LDO9,     	&max8997_ldo9_data },
	{ MAX8997_LDO10,    	&max8997_ldo10_data },

//	{ MAX8997_LDO11,    	&max8997_ldo11_data }, 
//	{ MAX8997_LDO12,    	&max8997_ldo12_data }, 
	{ MAX8997_LDO13,		&max8997_ldo13_data },  
	{ MAX8997_LDO14,    	&max8997_ldo14_data },  
	{ MAX8997_LDO15,    	&max8997_ldo15_data },  
//	{ MAX8997_LDO16,    	&max8997_ldo16_data },  
//	{ MAX8997_LDO17,    	&max8997_ldo17_data },  
//	{ MAX8997_LDO18,    	&max8997_ldo18_data },  
	{ MAX8997_LDO21,    	&max8997_ldo21_data },  

	{ MAX8997_BUCK1,    	&max8997_buck1_data },  
	{ MAX8997_BUCK2,    	&max8997_buck2_data }, 
	{ MAX8997_BUCK3,    	&max8997_buck3_data }, 
//	{ MAX8997_BUCK4,    	&max8997_buck4_data }, 
	{ MAX8997_BUCK5,    	&max8997_buck5_data }, 
//	{ MAX8997_BUCK6,		&max8997_buck6_data },  
	{ MAX8997_BUCK7,        &max8997_buck7_data },  

//	{ MAX8997_EN32KHZ_AP,   NULL},//&max8997_en32khz_ap_data },  
//	{ MAX8997_EN32KHZ_CP,   NULL},//&max8997_en32khz_cp_data },  
//	{ MAX8997_ENVICHG,      NULL},//&max8997_envichg_data },  
//	{ MAX8997_ESAFEOUT1,    NULL},//&max8997_esafeout1_data },  
//	{ MAX8997_ESAFEOUT2,    NULL},//&max8997_esafeout2_data },  
//	{ MAX8997_CHARGER_CV,   NULL},//&max8997_charger_cv_data },   /* control MBCCV of MBCCTRL }, 3 */
//	{ MAX8997_CHARGER,      NULL},//&max8997_charger_data },  /* charger current, MBCCTRL4 */
//	{ MAX8997_CHARGER_TOPOFF,NULL},//&max8997_charger_topoff_data },  /* MBCCTRL5 */
};


struct max8997_platform_data max8997_pdata = {
	.num_regulators	= ARRAY_SIZE(max8997_regulators),
	.regulators	= max8997_regulators,

	.wakeup = true,
	.buck1_gpiodvs 	= false,
	.buck2_gpiodvs	= false,
	.buck5_gpiodvs 	= false,

	.ignore_gpiodvs_side_effect=true,

	.buck125_default_idx = 0x0,

	.buck125_gpios[0]	= S5PV310_GPX1(6),
	.buck125_gpios[1]	= S5PV310_GPX1(7),
	.buck125_gpios[2]	= S5PV310_GPX0(4),

	.buck1_voltage[0] 	= 1250000,
	.buck1_voltage[1] 	= 1200000,
	.buck1_voltage[2] 	= 1150000,
	.buck1_voltage[3] 	= 1100000,
	.buck1_voltage[4] 	= 1050000,
	.buck1_voltage[5] 	= 1000000,
	.buck1_voltage[6] 	= 950000,
	.buck1_voltage[7] 	= 950000,

	.buck2_voltage[0] 	= 1100000,
	.buck2_voltage[1] 	= 1100000,
	.buck2_voltage[2] 	= 1100000,
	.buck2_voltage[3] 	= 1100000,
	.buck2_voltage[4] 	= 1000000,
	.buck2_voltage[5] 	= 1000000,
	.buck2_voltage[6] 	= 1000000,
	.buck2_voltage[7] 	= 1000000,

	.buck5_voltage[0] 	= 1200000,
	.buck5_voltage[1] 	= 1200000,
	.buck5_voltage[2] 	= 1200000,
	.buck5_voltage[3] 	= 1200000,
	.buck5_voltage[4] 	= 1200000,
	.buck5_voltage[5] 	= 1200000,
	.buck5_voltage[6] 	= 1200000,
	.buck5_voltage[7] 	= 1200000,
};

