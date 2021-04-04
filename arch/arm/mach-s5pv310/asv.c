/* linux/arch/arm/mach-s5pv310/asv.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * S5PV310 - ASV(Adaptive Supply Voltage) Test driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <mach/map.h>
#include <mach/regs-iem.h>
#include <mach/regs-pmu.h>
#include <mach/regs-clock.h>
#include <mach/asv.h>

#define LOOP_CNT	50

#define IDS_OFFSET	24
#define IDS_MASK	0xFF

#define IDS_SS		4
#define IDS_A1		8
#define IDS_A2		12
#define IDS_B1		17
#define IDS_B2		27
#define IDS_C1		45
#define IDS_C2		55
#define IDS_D1		56

#define HPM_SS		8
#define HPM_A1		11
#define HPM_A2		14
#define HPM_B1		18
#define HPM_B2		21
#define HPM_C1		23
#define HPM_C2		25
#define HPM_D1		26

static int __init iem_clock_init(void)
{
	struct clk *clk_hpm;
	struct clk *clk_copy;
	struct clk *clk_parent;

	/* PWI clock setting */
	clk_copy = clk_get(NULL, "sclk_pwi");
	if (IS_ERR(clk_copy)) {
		printk(KERN_ERR"ASV : SCLK_PWI clock get error\n");
		return -EINVAL;
	} else {
		clk_parent = clk_get(NULL, "xusbxti");
		if (IS_ERR(clk_parent)) {
			printk(KERN_ERR"ASV : MOUT_APLL clock get error\n");
			return -EINVAL;
		}
		clk_set_parent(clk_copy, clk_parent);

		clk_put(clk_parent);
	}
	clk_set_rate(clk_copy, 4800000);

	clk_put(clk_copy);

	/* HPM clock setting */
	clk_copy = clk_get(NULL, "dout_copy");
	if (IS_ERR(clk_copy)) {
		printk(KERN_ERR"ASV : DOUT_COPY clock get error\n");
		return -EINVAL;
	} else {
		clk_parent = clk_get(NULL, "mout_mpll");
		if (IS_ERR(clk_parent)) {
			printk(KERN_ERR"ASV : MOUT_APLL clock get error\n");
			return -EINVAL;
		}
		clk_set_parent(clk_copy, clk_parent);


		clk_put(clk_parent);
	}

	clk_set_rate(clk_copy, (400 * 1000 * 1000));

	clk_put(clk_copy);

	clk_hpm = clk_get(NULL, "sclk_hpm");
	if (IS_ERR(clk_hpm))
		return -EINVAL;

	clk_set_rate(clk_hpm, (200 * 1000 * 1000));

	clk_put(clk_hpm);

	return 0;
}

void __init iem_clock_set(void)
{
	/* APLL_CON0 level register */
	__raw_writel(0x80FA0601, S5P_APLL_CON0L8);
	__raw_writel(0x80C80601, S5P_APLL_CON0L7);
	__raw_writel(0x80C80602, S5P_APLL_CON0L6);
	__raw_writel(0x80C80604, S5P_APLL_CON0L5);
	__raw_writel(0x80C80601, S5P_APLL_CON0L4);
	__raw_writel(0x80C80601, S5P_APLL_CON0L3);
	__raw_writel(0x80C80601, S5P_APLL_CON0L2);
	__raw_writel(0x80C80601, S5P_APLL_CON0L1);

	/* IEM Divider register */
	__raw_writel(0x00500000, S5P_CLKDIV_IEM_L8);
	__raw_writel(0x00500000, S5P_CLKDIV_IEM_L7);
	__raw_writel(0x00500000, S5P_CLKDIV_IEM_L6);
	__raw_writel(0x00500000, S5P_CLKDIV_IEM_L5);
	__raw_writel(0x00500000, S5P_CLKDIV_IEM_L4);
	__raw_writel(0x00500000, S5P_CLKDIV_IEM_L3);
	__raw_writel(0x00500000, S5P_CLKDIV_IEM_L2);
	__raw_writel(0x00500000, S5P_CLKDIV_IEM_L1);
}


enum find_asv {
	HPM_GROUP,
	IDS_GROUP,
};

static int __init s5pv310_find_group(unsigned int value, enum find_asv grp)
{
	unsigned int ret=0;

	if (grp == HPM_GROUP) {
	    /* hpm grouping */
	    if ((value > 0) && (value <= HPM_SS))
		    ret = 0;
	    else if ((value > HPM_SS) && (value <= HPM_A1))
		    ret = 1;
	    else if ((value > HPM_A1) && (value <= HPM_A2))
		    ret = 2;
	    else if ((value > HPM_A2) && (value <= HPM_B1))
		    ret = 3;
	    else if ((value > HPM_B1) && (value <= HPM_B2))
		    ret = 4;
	    else if ((value > HPM_B2) && (value <= HPM_C1))
		    ret = 5;
	    else if ((value > HPM_C1) && (value <= HPM_C2))
		    ret = 6;
	    else if (value >= HPM_D1)
		    ret = 7;
	} else {
	    /* ids grouping */
	    if ((value > 0) && (value <= IDS_SS))
		    ret = 0;
	    else if ((value > IDS_SS) && (value <= IDS_A1))
		    ret = 1;
	    else if ((value > IDS_A1) && (value <= IDS_A2))
		    ret = 2;
	    else if ((value > IDS_A2) && (value <= IDS_B1))
		    ret = 3;
	    else if ((value > IDS_B1) && (value <= IDS_B2))
		    ret = 4;
	    else if ((value > IDS_B2) && (value <= IDS_C1))
		    ret = 5;
	    else if ((value > IDS_C1) && (value <= IDS_C2))
		    ret = 6;
	    else if (value >= IDS_D1)
		    ret = 7;
	}

	return ret;
}

static int __init s5pv310_asv_init(void)
{
	unsigned int i;
	unsigned long hpm_delay = 0;
	unsigned int tmp;
	unsigned int ids_arm;
	unsigned int hpm_group, ids_group;
	unsigned int asv_group = 0;
	static void __iomem * iem_base;
	struct clk *clk_iec;
	struct clk *clk_apc;
	struct clk *clk_hpm;	
    struct clk *clk_chipid;

    if((tmp >> ASV_ID_SHIFT) == ASV_ID)
        return 0;
    
	__raw_writel(asv_group|(ASV_ID<<ASV_ID_SHIFT), S5P_INFORM2);
	
	/* chip id clock gate enable*/
	clk_chipid = clk_get(NULL, "chipid");
	if (IS_ERR(clk_chipid)) {
		printk(KERN_ERR "ASV : chipid clock get error\n");
		return -EINVAL;
	}
	clk_enable(clk_chipid);
	
	tmp = __raw_readl(S5P_VA_CHIPID + 0x4);

	ids_arm = ((tmp >> IDS_OFFSET) & IDS_MASK);

	iem_base = ioremap(S5PV310_PA_IEM, (128 * 1024));

	if (iem_base == NULL) {
		printk(KERN_ERR "faile to ioremap\n");
		goto out;
	}

    /* IEC clock gate enable */
	clk_iec = clk_get(NULL, "iem-iec");
	if (IS_ERR(clk_iec)) {
		printk(KERN_ERR"ASV : IEM IEC clock get error\n");
		return -EINVAL;
	}
	clk_enable(clk_iec);

	/* APC clock gate enable */
	clk_apc = clk_get(NULL, "iem-apc");
	if (IS_ERR(clk_apc)) {
		printk(KERN_ERR"ASV : IEM APC clock get error\n");
		return -EINVAL;
	}
	clk_enable(clk_apc);

	/* hpm clock gate enalbe */
	clk_hpm = clk_get(NULL, "hpm");
	if (IS_ERR(clk_hpm)) {
		printk(KERN_ERR"ASV : HPM clock get error\n");
		return -EINVAL;
	}
	clk_enable(clk_hpm);

	if (iem_clock_init()) {
		printk(KERN_ERR "ASV driver clock_init fail\n");
		goto out;
	} else {
		/* HPM enable  */
		tmp = __raw_readl(iem_base + S5PV310_APC_CONTROL);
		tmp |= APC_HPM_EN;
		__raw_writel(tmp, (iem_base + S5PV310_APC_CONTROL));

		iem_clock_set();

		/* IEM enable */
		tmp = __raw_readl(iem_base + S5PV310_IECDPCCR);
		tmp |= IEC_EN;
		__raw_writel(tmp, (iem_base + S5PV310_IECDPCCR));
	}

	for ( i=0 ; i < LOOP_CNT ; i++ ){
		tmp = __raw_readb(iem_base + S5PV310_APC_DBG_DLYCODE);
		hpm_delay += tmp;
	}

	hpm_delay /= LOOP_CNT;
    hpm_delay -= 1;
    
	/* hpm clock gate disable */
	clk_disable(clk_hpm);
	clk_put(clk_hpm);

	/* IEC clock gate disable */
	clk_disable(clk_iec);
	clk_put(clk_iec);

	/* APC clock gate disable */
	clk_disable(clk_apc);
	clk_put(clk_apc);

	/* Disable chipid clock */
//	clk_disable(clk_chipid);
	
	iounmap(iem_base);

	/* HPM SCLKAPLL */
	tmp = __raw_readl(S5P_CLKSRC_CPU);
	tmp &= ~(0x1 << S5P_CLKSRC_CPU_MUXHPM_SHIFT);
	tmp |= 0x0 << S5P_CLKSRC_CPU_MUXHPM_SHIFT;
	__raw_writel(tmp, S5P_CLKSRC_CPU);

	hpm_group = s5pv310_find_group(hpm_delay, HPM_GROUP);
	ids_group = s5pv310_find_group(ids_arm,IDS_GROUP);
	

	/* decide asv group */
	if (ids_group > hpm_group) {
		if (ids_group - hpm_group >= 3)
			asv_group = ids_group - 3;
        else
		    asv_group = hpm_group;
	} else {
		if (hpm_group - ids_group >= 3)
			asv_group = hpm_group - 3;
        else
		    asv_group = ids_group;
	}
	
	printk(KERN_INFO "********************** ASV *********************\n");
	printk(KERN_INFO "ASV ids_arm = %d hpm_delay = %d\n", ids_arm, hpm_delay);
    printk(KERN_INFO "ASV ids_group = %d hpm_group = %d asv_group=%d\n", ids_group, hpm_group,asv_group);
    
	__raw_writel(asv_group|(ASV_ID<<ASV_ID_SHIFT), S5P_INFORM2);

	return 0;

out:
	return -EINVAL;

}
core_initcall(s5pv310_asv_init);
