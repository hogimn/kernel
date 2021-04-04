	/* linux/drivers/media/video/s5k4ea.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 * 		http://www.samsung.com/
 *
 * Driver for S5K4EA (UXGA camera) from Samsung Electronics
 * 1/4" 2.0Mp CMOS Image Sensor SoC with an Embedded Image Processor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-i2c-drv.h>
#include <media/s5k4ea_platform.h>
#include <linux/slab.h>

#ifdef CONFIG_VIDEO_SAMSUNG_V4L2
#include <linux/videodev2_samsung.h>
#endif

struct s5k4ea_jpeg_param {
	u32 enable;
	u32 quality;
	u32 main_size;		/* Main JPEG file size */
	u32 thumb_size;		/* Thumbnail file size */
	u32 main_offset;
	u32 thumb_offset;
	u32 postview_offset;
};


#define SENSOR_JPEG_SNAPSHOT_MEMSIZE	0x410580

#include "s5k4ea.h"

#define S5K4EA_DRIVER_NAME	"S5K4EA"

/* Default resolution & pixelformat. plz ref s5k4ea_platform.h */
#define DEFAULT_RES		WVGA	/* Index of resoultion */
#define DEFAUT_FPS_INDEX	S5K4EA_15FPS
#define DEFAULT_FMT		V4L2_PIX_FMT_UYVY	/* YUV422 */

static int read_device_id(struct i2c_client *client);

/*
 * Specification
 * Parallel : ITU-R. 656/601 YUV422, RGB565, RGB888 (Up to VGA), RAW10
 * Serial : MIPI CSI2 (single lane) YUV422, RGB565, RGB888 (Up to VGA), RAW10
 * Resolution : 1280 (H) x 1024 (V)
 * Image control : Brightness, Contrast, Saturation, Sharpness, Glamour
 * Effect : Mono, Negative, Sepia, Aqua, Sketch
 * FPS : 15fps @full resolution, 30fps @VGA, 24fps @720p
 * Max. pixel clock frequency : 48MHz(upto)
 * Internal PLL (6MHz to 27MHz input frequency)
 */

/* Camera functional setting values configured by user concept */
struct s5k4ea_userset {
	signed int exposure_bias;	/* V4L2_CID_EXPOSURE */
	unsigned int ae_lock;
	unsigned int awb_lock;
	unsigned int auto_wb;	/* V4L2_CID_AUTO_WHITE_BALANCE */
	unsigned int manual_wb;	/* V4L2_CID_WHITE_BALANCE_PRESET */
	unsigned int wb_temp;	/* V4L2_CID_WHITE_BALANCE_TEMPERATURE */
	unsigned int effect;	/* Color FX (AKA Color tone) */
	unsigned int contrast;	/* V4L2_CID_CONTRAST */
	unsigned int saturation;	/* V4L2_CID_SATURATION */
	unsigned int sharpness;		/* V4L2_CID_SHARPNESS */
	unsigned int glamour;
};

struct s5k4ea_state {
	struct s5k4ea_platform_data *pdata;
	struct v4l2_subdev sd;
	struct v4l2_pix_format pix;
	struct v4l2_fract timeperframe;
	struct s5k4ea_userset userset;
	struct s5k4ea_jpeg_param jpeg;
	int framesize_index;//KSJ
	int freq;	/* MCLK in KHz */
	int is_mipi;
	int isize;
	int ver;
	int fps;
	int check_previewdata;//KSJ
};
int g_alive3=0;
int g_s5k4ea_width=640;

static inline struct s5k4ea_state *to_state(struct v4l2_subdev *sd)
{
	printk("//KSJ to_state\n");
	return container_of(sd, struct s5k4ea_state, sd);
}

static unsigned short i2c_read_reg(struct i2c_client *client, unsigned short reg_h, unsigned short reg)
{
	int ret;
	unsigned char i2c_data[10];
	printk("//KSJ to_state\n");
	//{0x00, 0x28, 0x70, 0x00},  
	i2c_data[0]= 0x00;
	i2c_data[1]= 0x2c;
	i2c_data[2]= reg_h >>8;		//0x70;
	i2c_data[3]= reg_h & 0xff;	//0x00;

	i2c_master_send(client,i2c_data,4);
	

	i2c_data[0]= 0x00;
	i2c_data[1]= 0x2e;
	i2c_data[2]= (unsigned char)((reg>>8) & 0xff);
	i2c_data[3]= (unsigned char)(reg & 0xff);	
	

	i2c_master_send(client,i2c_data,4);

	i2c_data[0]=0x0f;
	i2c_data[1]=0x12;
	i2c_master_send(client,i2c_data,2);			
	

	ret = i2c_master_recv(client,i2c_data,2);

#if 0
	for(i=0;i<2;i++)
	printk("retdata %d => %x \n",i,i2c_data[i]);
#endif

#if 0
		if (ret < 0)
			printk( "%s: err %d\n", __func__, ret);
#endif
	
		return i2c_data[0]<<8 | i2c_data[1];
}

/*
 * S5K4EA register structure : 2bytes address, 2bytes value
 * retry on write failure up-to 5 times
 */
static inline int s5k4ea_write(struct v4l2_subdev *sd, u8 addr, u8 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct i2c_msg msg[1];
	unsigned char reg[2];
	int err = 0;
	int retry = 0;


	if (!client->adapter)
		return -ENODEV;

again:
	msg->addr = client->addr;
	msg->flags = 0;
	msg->len = 2;
	msg->buf = reg;

	reg[0] = addr & 0xff;
	reg[1] = val & 0xff;

	err = i2c_transfer(client->adapter, msg, 1);
	if (err >= 0)
		return err;	/* Returns here on success */

	/* abnormal case: retry 5 times */
	if (retry < 5) {
		dev_err(&client->dev, "%s: address: 0x%02x%02x, " \
			"value: 0x%02x%02x\n", __func__, \
			reg[0], reg[1], reg[2], reg[3]);
		retry++;
		goto again;
	}

	return err;
}

static int s5k4ea_i2c_write(struct v4l2_subdev *sd, unsigned char i2c_data[],
				unsigned char length)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned char buf[length], i;
	struct i2c_msg msg = {client->addr, 0, length, buf};
	
	for (i = 0; i < length; i++)
		buf[i] = i2c_data[i];

	return i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
}

static int s5k4ea_write_regs(struct v4l2_subdev *sd, unsigned char regs[],
				int size)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int i, err;

	for (i = 0; i < size; i++) {
		err = s5k4ea_i2c_write(sd, &regs[i], sizeof(regs[i]));
		if (err < 0)
			v4l_info(client, "%s: register set failed\n", \
			__func__);
	}

	return 0;	/* FIXME */
}


static int s5k4ea_write_array(struct v4l2_subdev *sd,unsigned  short * reg , int size)
{	
	unsigned char _tmp[4];
	int i,err=0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	for (i = 0; i < size ; i=i+2) {
		if(reg[i] == 0xffff){ //delay
			mdelay(reg[i+1]);
		}
		else {
//			printk("%x %x. ",reg[i],reg[i+1]);
//			printk(".");

			_tmp[0] =(unsigned char)( reg[i] >> 8) ;
			_tmp[1] =(unsigned char)( reg[i] & 0xff);

			_tmp[2] =(unsigned char)( reg[i+1] >> 8) ;
			_tmp[3] =(unsigned char)( reg[i+1] & 0xff);
			err = s5k4ea_i2c_write(sd,_tmp , 4);
			if (err < 0){
				v4l_info(client, "%s: register set failed\n", \
					__func__);
				v4l_info(client,"err i=%d %02x %02x %02x %02x \n",\
					i, _tmp[0],_tmp[1],_tmp[2],_tmp[3]); 
				return -1;
				
				}
		}
//	if(i%50 == 0) 	printk("\n");
	}
//	printk(" %d\n",i);
	
	return err;
}

#if 0

static const char *s5k4ea_querymenu_wb_preset[] = {
	"WB Tungsten", "WB Fluorescent", "WB sunny", "WB cloudy", NULL
};

static const char *s5k4ea_querymenu_effect_mode[] = {
	"Effect Sepia", "Effect Aqua", "Effect Monochrome",
	"Effect Negative", "Effect Sketch", NULL
};

static const char *s5k4ea_querymenu_ev_bias_mode[] = {
	"-3EV",	"-2,1/2EV", "-2EV", "-1,1/2EV",
	"-1EV", "-1/2EV", "0", "1/2EV",
	"1EV", "1,1/2EV", "2EV", "2,1/2EV",
	"3EV", NULL
};


static struct v4l2_queryctrl s5k4ea_controls[] = {
#if 1

	{
		/*
		 * For now, we just support in preset type
		 * to be close to generic WB system,
		 * we define color temp range for each preset
		 */
		.id = V4L2_CID_WHITE_BALANCE_TEMPERATURE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "White balance in kelvin",
		.minimum = 0,
		.maximum = 10000,
		.step = 1,
		.default_value = 0,	/* FIXME */
	},
	{
		.id = V4L2_CID_WHITE_BALANCE_PRESET,
		.type = V4L2_CTRL_TYPE_MENU,
		.name = "White balance preset",
		.minimum = 0,
		.maximum = ARRAY_SIZE(s5k4ea_querymenu_wb_preset) - 2,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_AUTO_WHITE_BALANCE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.name = "Auto white balance",
		.minimum = 0,
		.maximum = 1,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_EXPOSURE,
		.type = V4L2_CTRL_TYPE_MENU,
		.name = "Exposure bias",
		.minimum = 0,
		.maximum = ARRAY_SIZE(s5k4ea_querymenu_ev_bias_mode) - 2,
		.step = 1,
		.default_value = \
			(ARRAY_SIZE(s5k4ea_querymenu_ev_bias_mode) - 2) / 2,
			/* 0 EV */
	},
	{
		.id = V4L2_CID_COLORFX,
		.type = V4L2_CTRL_TYPE_MENU,
		.name = "Image Effect",
		.minimum = 0,
		.maximum = ARRAY_SIZE(s5k4ea_querymenu_effect_mode) - 2,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_CONTRAST,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Contrast",
		.minimum = 0,
		.maximum = 4,
		.step = 1,
		.default_value = 2,
	},
	{
		.id = V4L2_CID_SATURATION,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Saturation",
		.minimum = 0,
		.maximum = 4,
		.step = 1,
		.default_value = 2,
	},
	{
		.id = V4L2_CID_SHARPNESS,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Sharpness",
		.minimum = 0,
		.maximum = 4,
		.step = 1,
		.default_value = 2,
	},
	#endif
};

const char **s5k4ea_ctrl_get_menu(u32 id)
{
	printk("//KSJ s5k4ea_ctrl_get_menu\n");
	switch (id) {
	case V4L2_CID_WHITE_BALANCE_PRESET:
		return s5k4ea_querymenu_wb_preset;

	case V4L2_CID_COLORFX:
		return s5k4ea_querymenu_effect_mode;

	case V4L2_CID_EXPOSURE:
		return s5k4ea_querymenu_ev_bias_mode;

	default:
		return v4l2_ctrl_get_menu(id);
	}
}

#else
static int s5k4ea_init(struct v4l2_subdev *sd, u32 val); // KSJ


static const char *s5k4ea_querymenu_wb_preset[] = {
	"WB sunny",
	"WB cloudy",
	"WB Tungsten",
	"WB Fluorescent",
	NULL
};

static const char *s5k4ea_querymenu_effect_mode[] = {
	"Effect Normal",
	"Effect Monochrome",
	"Effect Sepia",
	"Effect Negative",
	"Effect Aqua",
	"Effect Sketch",
	NULL
};

static const char *s5k4ea_querymenu_ev_bias_mode[] = {
	"-3EV",	"-2,1/2EV", "-2EV", "-1,1/2EV",
	"-1EV", "-1/2EV", "0", "1/2EV",
	"1EV", "1,1/2EV", "2EV", "2,1/2EV",
	"3EV", NULL
};

static struct v4l2_queryctrl s5k4ea_controls[] = {
	{
		/*
		 * For now, we just support in preset type
		 * to be close to generic WB system,
		 * we define color temp range for each preset
		 */
		.id = V4L2_CID_WHITE_BALANCE_TEMPERATURE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "White balance in kelvin",
		.minimum = 0,
		.maximum = 10000,
		.step = 1,
		.default_value = 0,	/* FIXME */
	},
	{
		.id = V4L2_CID_WHITE_BALANCE_PRESET,
		.type = V4L2_CTRL_TYPE_MENU,
		.name = "White balance preset",
		.minimum = 0,
		.maximum = ARRAY_SIZE(s5k4ea_querymenu_wb_preset) - 2,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_CAMERA_WHITE_BALANCE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.name = "Auto white balance",
		.minimum = 0,
		.maximum = 1,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_EXPOSURE,
		.type = V4L2_CTRL_TYPE_MENU,
		.name = "Exposure bias",
		.minimum = 0,
		.maximum = ARRAY_SIZE(s5k4ea_querymenu_ev_bias_mode) - 2,
		.step = 1,
		.default_value = (ARRAY_SIZE(s5k4ea_querymenu_ev_bias_mode)
				- 2) / 2,	/* 0 EV */
	},
	{
		.id = V4L2_CID_CAMERA_EFFECT,
		.type = V4L2_CTRL_TYPE_MENU,
		.name = "Image Effect",
		.minimum = 0,
		.maximum = ARRAY_SIZE(s5k4ea_querymenu_effect_mode) - 2,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_CAMERA_CONTRAST,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Contrast",
		.minimum = 0,
		.maximum = 4,
		.step = 1,
		.default_value = 2,
	},
	{
		.id = V4L2_CID_CAMERA_SATURATION,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Saturation",
		.minimum = 0,
		.maximum = 4,
		.step = 1,
		.default_value = 2,
	},
	{
		.id = V4L2_CID_CAMERA_SHARPNESS,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Sharpness",
		.minimum = 0,
		.maximum = 4,
		.step = 1,
		.default_value = 2,
	},
};
/*
static int s5k4ea_reset(struct v4l2_subdev *sd)
{
	return s5k4ea_init(sd, 0);
}
*/
const char **s5k4ea_ctrl_get_menu(u32 id)
{
	switch (id) {
	case V4L2_CID_WHITE_BALANCE_PRESET:
		return s5k4ea_querymenu_wb_preset;

	case V4L2_CID_CAMERA_EFFECT:
		return s5k4ea_querymenu_effect_mode;

	case V4L2_CID_EXPOSURE:
		return s5k4ea_querymenu_ev_bias_mode;

	default:
		return v4l2_ctrl_get_menu(id);
	}
}

#endif

static inline struct v4l2_queryctrl const *s5k4ea_find_qctrl(int id)
{
	int i;
	printk("//KSJ v4l2_queryctrl\n");
	for (i = 0; i < ARRAY_SIZE(s5k4ea_controls); i++)
		if (s5k4ea_controls[i].id == id)
			return &s5k4ea_controls[i];

	return NULL;
}

static int s5k4ea_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	int i;
	printk("//KSJ s5k4ea_queryctrl\n");
	for (i = 0; i < ARRAY_SIZE(s5k4ea_controls); i++) {
		if (s5k4ea_controls[i].id == qc->id) {
			memcpy(qc, &s5k4ea_controls[i], \
				sizeof(struct v4l2_queryctrl));
			return 0;
		}
	}

	return -EINVAL;
}

static int s5k4ea_querymenu(struct v4l2_subdev *sd, struct v4l2_querymenu *qm)
{
	struct v4l2_queryctrl qctrl;
	printk("//KSJ s5k4ea_querymenu\n");
	qctrl.id = qm->id;
	s5k4ea_queryctrl(sd, &qctrl);

	return v4l2_ctrl_query_menu(qm, &qctrl, s5k4ea_ctrl_get_menu(qm->id));
}

/*
 * Clock configuration
 * Configure expected MCLK from host and return EINVAL if not supported clock
 * frequency is expected
 * 	freq : in Hz
 * 	flag : not supported for now
 */
static int s5k4ea_s_crystal_freq(struct v4l2_subdev *sd, u32 freq, u32 flags)
{
	int err = -EINVAL;
	printk("//KSJ s5k4ea_s_crystal_freq\n");
	return err;
}

static int s5k4ea_g_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;
	printk("##KSJ s5k4ea_g_fmt \n");
	
	return err;
}

static int s5k4ea_set_frame_size(struct v4l2_subdev *sd,int width)
{
	int err = 0;
	printk("//KSJ s5k4ea_set_frame_size = %d\n",width);
	/*
	if(width == 640)
	{
	 		//err=s5k4ea_write_array(sd,s5k4ea_preview_preset_0,S5K4EA_PREVIEW_PRESET_0);
	}
	else if(width == 720)
	{
	 		err=s5k4ea_write_array(sd,s5k4ea_movie_preset_0,S5K4EA_MOVIE_PRESET_0);
	}
	
	else if(width == 800)
	{
	 		err=s5k4ea_write_array(sd,s5k4ea_preview_preset_800x480,S5K4EA_PREVIEW_PRESET_800x480);
	}

	 else if(width == 2048)
 	{
	 		err=s5k4ea_write_array(sd,s5k4ea_capture_preset_0,S5K4EA_CAPTURE_PRESET_0);
			mdelay(100);
 	}
 	*/
	return err;
}


static int s5k4ea_s_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;
	
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ea_state, sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	printk("//KSJ s5k4ea_s_fmt\n");
	dev_dbg(&client->dev, "%s: pixelformat = 0x%x (%c%c%c%c),"
		" colorspace = 0x%x, width = %d, height = %d\n",
		__func__, fmt->fmt.pix.pixelformat,
	fmt->fmt.pix.pixelformat,
	fmt->fmt.pix.pixelformat >> 8,
	fmt->fmt.pix.pixelformat >> 16,
	fmt->fmt.pix.pixelformat >> 24,
	fmt->fmt.pix.colorspace,
	fmt->fmt.pix.width, fmt->fmt.pix.height);

	if (fmt->fmt.pix.pixelformat == V4L2_PIX_FMT_JPEG &&
	fmt->fmt.pix.colorspace != V4L2_COLORSPACE_JPEG) {
	dev_err(&client->dev,
		"%s: mismatch in pixelformat and colorspace\n",
		__func__);
	return -EINVAL;
	}

	if (fmt->fmt.pix.colorspace == V4L2_COLORSPACE_JPEG) {

	} else {
	}
		
	g_s5k4ea_width=fmt->fmt.pix.width;
	
	printk("## s5k4ea_s_fmt :M%d %d %d \n",g_alive3,fmt->fmt.pix.width,fmt->fmt.pix.height);
	if( g_alive3 != 3 ) return 0;

	err = s5k4ea_set_frame_size(sd,g_s5k4ea_width);
		
	return 0;
}
static int s5k4ea_enum_framesizes(struct v4l2_subdev *sd, \
					struct v4l2_frmsizeenum *fsize)
{
	int err = 0;
	printk("//KSJ s5k4ea_enum_framesizes\n");
	return err;
}

static int s5k4ea_enum_frameintervals(struct v4l2_subdev *sd,
					struct v4l2_frmivalenum *fival)
{
	int err = 0;
	printk("//KSJ s5k4ea_enum_frameintervals\n");
	return err;
}

static int s5k4ea_enum_fmt(struct v4l2_subdev *sd, struct v4l2_fmtdesc *fmtdesc)
{
	int err = 0;
	printk("##KSJ s5k4ea_enum_fmt \n");

	return err;
}

static int s5k4ea_try_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;
	printk("//KSJ s5k4ea_try_fmt\n");
	return err;
}

static int s5k4ea_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;
	printk("//KSJ s5k4ea_g_parm\n");
	dev_dbg(&client->dev, "%s\n", __func__);

	return err;
}

static int s5k4ea_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;
	printk("//KSJ s5k4ea_s_parm\n");
	dev_dbg(&client->dev, "%s: numerator %d, denominator: %d\n", \
		__func__, param->parm.capture.timeperframe.numerator, \
		param->parm.capture.timeperframe.denominator);

	return err;
}
static int s5k4ea_set_jpeg_quality(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ea_state *state =
		container_of(sd, struct s5k4ea_state, sd);
	printk("//KSJ s5k4ea_set_jpeg_quality\n");
	dev_dbg(&client->dev,
		"%s: jpeg.quality %d\n", __func__, state->jpeg.quality);
	if (state->jpeg.quality < 0)
		state->jpeg.quality = 0;
	if (state->jpeg.quality > 100)
		state->jpeg.quality = 100;

	switch (state->jpeg.quality) {
	case 90 ... 100:
		dev_dbg(&client->dev,
			"%s: setting to high jpeg quality\n", __func__);
		break;
	case 80 ... 89:
		dev_dbg(&client->dev,
			"%s: setting to normal jpeg quality\n", __func__);
		break;
	default:
		dev_dbg(&client->dev,
			"%s: setting to low jpeg quality\n", __func__);
		break;
	}
	return 0;
}

static int s5k4ea_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ea_state *state = to_state(sd);
	struct s5k4ea_userset userset = state->userset;
	int err = 0;
	printk("//KSJ s5k4ea_g_ctrl\n");
	#if 0
	switch (ctrl->id) {
	case V4L2_CID_EXPOSURE:
		printk("//KSJ V4L2_CID_EXPOSURE\n");
		ctrl->value = userset.exposure_bias;
		break;

	case V4L2_CID_AUTO_WHITE_BALANCE:
		printk("//KSJ V4L2_CID_AUTO_WHITE_BALANCE\n");
		ctrl->value = userset.auto_wb;
		break;

	case V4L2_CID_WHITE_BALANCE_PRESET:
		printk("//KSJ V4L2_CID_WHITE_BALANCE_PRESET\n");
		ctrl->value = userset.manual_wb;
		break;

	case V4L2_CID_COLORFX:
		printk("//KSJ V4L2_CID_COLORFX\n");
		ctrl->value = userset.effect;
		break;

	case V4L2_CID_CONTRAST:
		printk("//KSJ V4L2_CID_CONTRAST\n");
		ctrl->value = userset.contrast;
		break;

	case V4L2_CID_SATURATION:
		printk("//KSJ V4L2_CID_SATURATION\n");
		ctrl->value = userset.saturation;
		break;

	case V4L2_CID_SHARPNESS:
		printk("//KSJ V4L2_CID_SHARPNESS\n");
		ctrl->value = userset.saturation;
		break;
	case V4L2_CID_CAM_JPEG_MEMSIZE:
		printk("//KSJ V4L2_CID_CAM_JPEG_MEMSIZE\n");
		ctrl->value = SENSOR_JPEG_SNAPSHOT_MEMSIZE;
		break;
	case V4L2_CID_CAM_JPEG_MAIN_OFFSET:
		printk("//KSJ V4L2_CID_CAM_JPEG_MAIN_OFFSET\n");
		ctrl->value = state->jpeg.main_offset;
		break;			
	case V4L2_CID_CAM_JPEG_POSTVIEW_OFFSET:
		printk("//KSJ V4L2_CID_CAM_JPEG_POSTVIEW_OFFSET\n");
			ctrl->value = state->jpeg.postview_offset;
		break;
	case V4L2_CID_CAM_JPEG_MAIN_SIZE:
		printk("//KSJ V4L2_CID_CAM_JPEG_MAIN_SIZE\n");
			ctrl->value =SENSOR_JPEG_SNAPSHOT_MEMSIZE;// state->jpeg.main_size;
		break;
	case V4L2_CID_CAM_JPEG_QUALITY:
		printk("//KSJ V4L2_CID_CAM_JPEG_QUALITY\n");
			ctrl->value = state->jpeg.quality;
		break;

	default:
		err= -ENOIOCTLCMD;
		dev_err(&client->dev, "%s: no such ctrl\n", __func__);
		break;
	}
#endif
	return err;
}
static int s5k4ea_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	unsigned char _tmp[4];
	int i,value;
	struct s5k4ea_state *state =
			container_of(sd, struct s5k4ea_state, sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = -EINVAL;
	printk("//KSJ s5k4ea_s_ctrl\n");
	#if 0
	switch (ctrl->id) {
		
	case V4L2_CID_FOCUS_AUTO:
		printk("//KSJ V4L2_CID_FOCUS_AUTO\n");
		err=s5k4ea_write_array(sd,s5k4ea_regs_focus[ctrl->value],S5K4EA_FOCUS);

		for(i=0;i<30;i++){
			//err=i2c_read_reg(client,0x7000, 0x26fe); //read AF status reg.

			if(err == 0x0000){
				printk("Idle AF search \n");
				mdelay(100);
			}
			else if(err == 0x0001){
				printk("AF search in progress\n");
				mdelay(100);
			}
			else if(err == 0x0002){
				printk("AF search success\n");
				return 1;
			}
			else if(err == 0x0003){
				printk("Low confidence position\n");
				return 0;
			}
			else if(err == 0x0004){
				printk("AF search is cancelled\n");
				return 1;
			}
			else {
				return 1;
			}
		}
#if 0		
		printk("---V4L2_CID_FOCUS_AUTOl ----------\n");
		printk("s5k4ea_s_ctrl V4L2_CID_FOCUS_AUTO %d \n",ctrl->value);
		printk("af reg %d \n", i2c_read_reg(client,0x7000, 0x0252));	
		printk("REG_TC_AF_AfCmdErro %d \n", i2c_read_reg(client,0x7000, 0x0256));	
		printk("---------------------------------\n");			
#endif		

		return 0;
		break;

		
	case V4L2_CID_CAMERA_SET_AUTO_FOCUS:
		printk("//KSJ V4L2_CID_CAMERA_SET_AUTO_FOCUS\n");
		if (value == AUTO_FOCUS_ON){}
			//err = s5k4ecgx_start_auto_focus(sd);
		else if (value == AUTO_FOCUS_OFF){}
			//err = s5k4ecgx_stop_auto_focus(sd);
		else {
			err = -EINVAL;
			dev_err(&client->dev,
				"%s: bad focus value requestion %d\n",
					__func__, value);
			}
			break;

	case V4L2_CID_EXPOSURE:
		printk("//KSJ V4L2_CID_EXPOSURE\n");
		dev_dbg(&client->dev, "%s: V4L2_CID_EXPOSURE\n", __func__);
		err = s5k4ea_write_regs(sd, \
		(unsigned char *) s5k4ea_regs_ev_bias[ctrl->value], \
			sizeof(s5k4ea_regs_ev_bias[ctrl->value]));
		break;

	case V4L2_CID_AUTO_WHITE_BALANCE:
		printk("//KSJ V4L2_CID_AUTO_WHITE_BALANCE\n");
		dev_dbg(&client->dev, "%s: V4L2_CID_AUTO_WHITE_BALANCE\n", \
			__func__);
		err = s5k4ea_write_regs(sd, \
		(unsigned char *) s5k4ea_regs_awb_enable[ctrl->value], \
			sizeof(s5k4ea_regs_awb_enable[ctrl->value]));
		break;

	case V4L2_CID_WHITE_BALANCE_PRESET:
		printk("//KSJ V4L2_CID_WHITE_BALANCE_PRESET\n");
		dev_dbg(&client->dev, "%s: V4L2_CID_WHITE_BALANCE_PRESET\n", \
			__func__);
			printk( "%s: V4L2_CID_WHITE_BALANCE_PRESET %d\n", \
			__func__,ctrl->value);
				err=s5k4ea_write_array(sd,s5k4ea_regs_wb_preset[ctrl->value],\
					S5K4EA_WB_PRESET);

		break;

	case V4L2_CID_COLORFX:
		printk("//KSJ V4L2_CID_COLORFX\n");
		dev_dbg(&client->dev, "%s: V4L2_CID_COLORFX\n", __func__);
		err=s5k4ea_write_array(sd,s5k4ea_regs_color_effect[ctrl->value]
			,S5K4EA_COLOR_FX);
		break;

	case V4L2_CID_CONTRAST:
		printk("//KSJ V4L2_CID_CONTRAST\n");
		dev_dbg(&client->dev, "%s: V4L2_CID_CONTRAST\n", __func__);
		err = s5k4ea_write_regs(sd, \
		(unsigned char *) s5k4ea_regs_contrast_bias[ctrl->value], \
			sizeof(s5k4ea_regs_contrast_bias[ctrl->value]));
		break;

	case V4L2_CID_SATURATION:
		printk("//KSJ V4L2_CID_SATURATION\n");
		dev_dbg(&client->dev, "%s: V4L2_CID_SATURATION\n", __func__);
		err = s5k4ea_write_regs(sd, \
		(unsigned char *) s5k4ea_regs_saturation_bias[ctrl->value], \
			sizeof(s5k4ea_regs_saturation_bias[ctrl->value]));
		break;

	case V4L2_CID_SHARPNESS:
		printk("//KSJ V4L2_CID_SHARPNESS\n");
		dev_dbg(&client->dev, "%s: V4L2_CID_SHARPNESS\n", __func__);
		err = s5k4ea_write_regs(sd, \
		(unsigned char *) s5k4ea_regs_sharpness_bias[ctrl->value], \
			sizeof(s5k4ea_regs_sharpness_bias[ctrl->value]));
		break;
	case V4L2_CID_CAMERA_CAPTURE :
		printk("//KSJ V4L2_CID_CAMERA_CAPTURE\n");
		printk("s5k4ea_capture_jpeg_2084x1536\n");
		err=s5k4ea_write_array(sd,s5k4ea_capture_jpeg_2048x1536, \
			S5K4EA_CAPTURE_JPEG_2048x1536);
		break;
	case V4L2_CID_CAM_JPEG_QUALITY:
		printk("//KSJ V4L2_CID_CAM_JPEG_QUALITY\n");
		state->jpeg.quality = ctrl->value;
			err = s5k4ea_set_jpeg_quality(sd);
		break;
	default:
		dev_err(&client->dev, "%s: no such control\n", __func__);
		break;
	}
#endif

	if (err < 0)
		goto out;
	else
		return 0;

out:
	dev_dbg(&client->dev, "%s: vidioc_s_ctrl failed\n", __func__);
	return err;
}

static int s5k4ea_connected_check(struct i2c_client *client)
{
	int id;
// id check
	printk("//KSJ s5k4ea_connected_check\n");
	id=read_device_id(client);
	if(id != 0x4ea) return -1;
	return 0;
}

static int s5k4ea_init(struct v4l2_subdev *sd, u32 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = -EINVAL, i;
	unsigned char _tmp[4];;

	//init parameters
	struct s5k4ea_state *state =
		container_of(sd, struct s5k4ea_state, sd);
	printk("//KSJ s5k4ea_init\n");
	state->jpeg.enable = 0;
	state->jpeg.quality = 100;
	state->jpeg.main_offset = 0;
	state->jpeg.main_size = 0;
	state->jpeg.thumb_offset = 0;
	state->jpeg.thumb_size = 0;
	state->jpeg.postview_offset = 0;

	if(s5k4ea_connected_check(client)<0) {
		printk("Not Connected\n");
		v4l_err(client, "%s: camera initialization failed\n", __func__);
		g_alive3 = 0;
		return -1;
	}

	
	v4l_info(client, "%s: camera initialization start\n", __func__);

	err = s5k4ea_write_array(sd, s5k4ea_init_reg_total, S5K4EA_INIT_REGS_TOTAL);
	
v4l_info(client, "%s: err = %d\n", __func__, err);
#if 0
	err=s5k4ea_write_array(sd,s5k4ea_init_reg_short0,S5K4EA_INIT_REGS0);
	mdelay(10);
	err=s5k4ea_write_array(sd,s5k4ea_init_reg_short1,S5K4EA_INIT_REGS1);
	err=s5k4ea_write_array(sd,s5k4ea_init_reg_short2,S5K4EA_INIT_REGS2);
	err=s5k4ea_write_array(sd,s5k4ea_init_reg_short3,S5K4EA_INIT_REGS3);
	err=s5k4ea_write_array(sd,s5k4ea_init_reg_short4,S5K4EA_INIT_REGS4);
	err=s5k4ea_write_array(sd,s5k4ea_init_reg_short4_2,S5K4EA_INIT_REGS4_2);
	err=s5k4ea_write_array(sd,s5k4ea_init_reg_short5,S5K4EA_INIT_REGS5);
	err=s5k4ea_write_array(sd,s5k4ea_init_reg_short6,S5K4EA_INIT_REGS6);
	err=s5k4ea_write_array(sd,s5k4ea_init_reg_short7,S5K4EA_INIT_REGS7);
	err=s5k4ea_write_array(sd,s5k4ea_init_reg_short8,S5K4EA_INIT_REGS8);
	//

	err = s5k4ea_set_frame_size(sd,g_s5k4ea_width);
#endif

	if (err < 0) {
		v4l_err(client, "%s: camera initialization failed\n", \
			__func__);
		return -EIO;	/* FIXME */
	}
	v4l_info(client, "%s: camera initialization done\n", __func__);

	g_alive3 = 3;
	return 3;
}

/*
 * s_config subdev ops
 * With camera device, we need to re-initialize every single opening time
 * therefor, it is not necessary to be initialized on probe time.
 * except for version checking.
 * NOTE: version checking is optional
 */
static int s5k4ea_s_config(struct v4l2_subdev *sd, int irq, void *platform_data)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ea_state *state = to_state(sd);
	struct s5k4ea_platform_data *pdata;

	dev_info(&client->dev, "fetching platform data\n");
	printk("//KSJ s5k4ea_s_config\n");
	pdata = client->dev.platform_data;

	if (!pdata) {
		dev_err(&client->dev, "%s: no platform data\n", __func__);
		return -ENODEV;
	}

	/*
	 * Assign default format and resolution
	 * Use configured default information in platform data
	 * or without them, use default information in driver
	 */
	if (!(pdata->default_width && pdata->default_height)) {
		/* TODO: assign driver default resolution */
	} else {
		state->pix.width = pdata->default_width;
		state->pix.height = pdata->default_height;
	}

	if (!pdata->pixelformat)
		state->pix.pixelformat = DEFAULT_FMT;
	else
		state->pix.pixelformat = pdata->pixelformat;

	if (!pdata->freq)
		state->freq = 48000000;	/* 48MHz default */
	else
		state->freq = pdata->freq;

	if (!pdata->is_mipi) {
		state->is_mipi = 0;
		dev_info(&client->dev, "parallel mode\n");
	} else
		state->is_mipi = pdata->is_mipi;

	return 0;
}

static const struct v4l2_subdev_core_ops s5k4ea_core_ops = {
	.init = s5k4ea_init,	/* initializing API */
	.s_config = s5k4ea_s_config,	/* Fetch platform data */
	.queryctrl = s5k4ea_queryctrl,
	.querymenu = s5k4ea_querymenu,
	.g_ctrl = s5k4ea_g_ctrl,
	.s_ctrl = s5k4ea_s_ctrl,
};

static const struct v4l2_subdev_video_ops s5k4ea_video_ops = {
	.s_crystal_freq = s5k4ea_s_crystal_freq,
	.g_fmt = s5k4ea_g_fmt,
	.s_fmt = s5k4ea_s_fmt,
	.enum_framesizes = s5k4ea_enum_framesizes,
	.enum_frameintervals = s5k4ea_enum_frameintervals,
	.enum_fmt = s5k4ea_enum_fmt,
	.try_fmt = s5k4ea_try_fmt,
	.g_parm = s5k4ea_g_parm,
	.s_parm = s5k4ea_s_parm,
};

static const struct v4l2_subdev_ops s5k4ea_ops = {
	.core = &s5k4ea_core_ops,
	.video = &s5k4ea_video_ops,
};


/*
 * s5k4ea_probe
 * Fetching platform data is being done with s_config subdev call.
 * In probe routine, we just register subdev device
 */
static int s5k4ea_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct s5k4ea_state *state;
	struct v4l2_subdev *sd;
	printk("//KSJ s5k4ea_probe\n");
	state = kzalloc(sizeof(struct s5k4ea_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;
	g_alive3=0;

	sd = &state->sd;
	strcpy(sd->name, S5K4EA_DRIVER_NAME);

	/* Registering subdev */
	v4l2_i2c_subdev_init(sd, client, &s5k4ea_ops);

	dev_info(&client->dev, "s5k4ea has been probed\n");


	return 0;
}


static int s5k4ea_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	printk("//KSJ s5k4ea_remove\n");
	v4l2_device_unregister_subdev(sd);
	kfree(to_state(sd));
	g_alive3=0;
	return 0;
}

static const struct i2c_device_id s5k4ea_id[] = {
	{ S5K4EA_DRIVER_NAME, 0 },
	{ },
};


static int read_device_id(struct i2c_client *client)
{
	int id;
	id= i2c_read_reg(client,0x0, 0x40);	
	printk("//KSJ read_device_id\n");
	v4l_info(client,"Chip ID 0x00000040 :0x%x \n", id);
	v4l_info(client,"Chip Revision  0x00000042 :0x%x \n",i2c_read_reg(client,0x0, 0x42));
	v4l_info(client,"FW version control revision  0x00000048 :%d \n",i2c_read_reg(client,0x0, 0x48));
	v4l_info(client,"FW compilation date(0xYMDD) 0x0000004e :0x%x \n",i2c_read_reg(client,0x0, 0x4e));

return id;
}


MODULE_DEVICE_TABLE(i2c, s5k4ea_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = S5K4EA_DRIVER_NAME,
	.probe = s5k4ea_probe,
	.remove = s5k4ea_remove,
	.id_table = s5k4ea_id,
};

MODULE_DESCRIPTION("Samsung Electronics S5K4EA UXGA camera driver");
MODULE_AUTHOR("Jinsung Yang <jsgood.yang@samsung.com>");
MODULE_LICENSE("GPL");

