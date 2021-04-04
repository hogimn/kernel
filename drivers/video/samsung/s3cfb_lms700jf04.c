/* linux/drivers/video/samsung/s3cfb_lms700jf04.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * HT101HD1-100 XWVGA Landscape LCD module driver for the SMDK
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include "s3cfb.h"

static struct s3cfb_lcd lms700jf04 = {
	.width = 1024,
	.height = 600,
	.bpp = 32,
	.freq = 60,

	.timing = {
		.h_fp = 36,
		.h_bp = 60,
		.h_sw = 30,
		.v_fp = 10,
		.v_fpe = 1,
		.v_bp = 11,
		.v_bpe = 1,
		.v_sw = 10,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

/* name should be fixed as 's3cfb_set_lcd_info' */
void s3cfb_set_lcd_info(struct s3cfb_global *ctrl)
{
	lms700jf04.init_ldi = NULL;
	ctrl->lcd = &lms700jf04;
}
