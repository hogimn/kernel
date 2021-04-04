/* linux/drivers/video/samsung/s3cfb_hdmi.c
 *
 * Samsung LSi HDMI Display Support
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include "s3cfb.h"

static struct s3cfb_lcd hdmi_ui_fb = {
	.width = 1920,
	.height = 1080,
	.bpp = 32,
	.freq = 60,

	.timing = {
		.h_fp = 1,
		.h_bp = 1,
		.h_sw = 1,
		.v_fp = 1,
		.v_fpe = 1,
		.v_bp = 1,
		.v_bpe = 1,
		.v_sw = 1,
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
	hdmi_ui_fb.init_ldi	= NULL;
	
	ctrl->lcd = &hdmi_ui_fb;
}

