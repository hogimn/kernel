/*
 * ECT23H 2D to 3D Real time converter
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#include "ect223h.h"

#define ECT223H_VERSION "1.0.0"

#define ECT223H_NAME "ect223h"

/* for debugging */
#define DEBUG 1

#define REG8		0
#define REG16		1

#define E223H_I2C_DEV_ADDR	0xE8  // or 0xD8

#define TRUE			1
#define FALSE		0

#define ON 		   	1
#define OFF   		0

#define REG_SET		1 // Register Set : value | reg
#define REG_CLR		0 // Register Clear : ~value & reg

#define REGBIT0		(1<<0)
#define REGBIT1		(1<<1)
#define REGBIT2		(1<<2)
#define REGBIT3		(1<<3)
#define REGBIT4		(1<<4)
#define REGBIT5		(1<<5)
#define REGBIT6		(1<<6)
#define REGBIT7		(1<<7)

#define LVDS_PINMAP_VESA		0
#define LVDS_PINMAP_JEIDA		1
#define LVDS_PINMAP_8BIT		0
#define LVDS_PINMAP_10BIT		1

#define IN_SbyS				0 //  Side by Side image input, real 3D
#define IN_2D3D				2 // 2D to 3D converting

#define OUT_BYPASS			0		
#define OUT_PbyP			1 // LR for barrier
#define OUT_LbyL			2 // LR line
#define OUT_FS				3 // Frame Switch
#define OUT_RB				4 // Red/Blue
#define OUT_LR_CHECKER		5 // LR checkboard
#define OUT_SbyS			6 // LLRR demux ( Side by Side )
#define OUT_SubPixel_Stripe			7 // subpixel stripe
#define OUT_SubPixel_Step			8 // subpixel step

#define INPUT_NORMAL		0 
#define INPUT_BYPASS		1
// 3d level :  0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
const u8 PIXEL_DELAY1[21]={0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
const u8 PIXEL_DELAY2[21]={6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
const u8 SHIFT_IMG1[21] = {6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
const u8 SHIFT_IMG2[21] = {0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

u8 g2d3d_mode;// Bypass(0), LR_line(1), SbyS(2)
u8 g2d3d_ob_level;
u8 g2d3d_fb_level;
u8 g2d3d_slope_level;
u8 g2d3d_LR_swap;// 0:R first, 1:L first
u8 g2d3d_fb;
u8 depth_mix_cont_level;

struct ect223h_data {
	struct i2c_client *client;
	struct ect223h_platform_data *pdata;
	struct miscdevice ect223h_device;
};

static struct file_operations ect223h_device_fops =
{
	.owner = THIS_MODULE,
	.open = ect223h_device_open,
	.ioctl = ect223h_device_ioctl,
	.release = ect223h_device_release,
	.read = ect223h_device_read,
	.write = ect223h_device_write,
};

static struct miscdevice ect223h_misc =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = ECT223H_NAME,
	.fops = &ect223h_device_fops,
};

struct ect223h_data *ect223h_ptr;


#define ECT223H                        0x50
#define SET_2D3D_MODE_BYPASS_MODE _IO(ECT223H, 0x00)
#define SET_2D3D_MODE_PIXEL_MODE_ON	_IO(ECT223H, 0x01)
#define SET_2D3D_MODE_PIXEL_MODE_OFF	_IO(ECT223H, 0x02)
#define SET_2D3D_MODE_SIDE_MODE	_IO(ECT223H, 0x03)
#define SET_MIX_3D_LEVEL_DOWN	_IO(ECT223H, 0x04)
#define SET_MIX_3D_LEVEL_UP	_IO(ECT223H, 0x05)
#define SET_FRONT_BACK_LEVEL_DOWN	_IO(ECT223H, 0x06)
#define SET_FRONT_BACK_LEVEL_UP		_IO(ECT223H, 0x07)

extern void ns070_reset(int);
extern void ect223h_reset(int);

// utility
// LCD 3D filter on/off
// GPIO control
void lcd_3d_on_off(int on)
{
	ns070_reset(on);	
}

/*
 * Device dependant operations
 */

int ect223h_i2c_write(unsigned short reg, unsigned short data)
{	
	int res;
	unsigned short addr;

        if(ect223h_ptr->client == NULL) 
                return -ENODEV;

// Bank Select
	if(reg & 0x100) { // back 1, 16bit
		res = i2c_smbus_write_byte_data(ect223h_ptr->client, 0x00, 0x01);
	} else { // bank 0 , 8bit
		res = i2c_smbus_write_byte_data(ect223h_ptr->client, 0x00, 0x00);
	}
        if(res < 0)
                return res;

	addr = reg & 0xff;
// write data
	if(reg & 0x100) { // back 1, 16bit
		res = i2c_smbus_write_block_data(ect223h_ptr->client, addr, 2, (char *)&data);
	} else {
		res = i2c_smbus_write_byte_data(ect223h_ptr->client, addr, (char)data);
	}
        if(res < 0)
                return res;

	return 0;
}

int ect223h_i2c_read(unsigned short reg)
{
	int res;
	unsigned short addr;
	unsigned short data;
	

        if (ect223h_ptr->client == NULL) 
                return -ENODEV;

// Bank Select
	if(reg & 0x100) { // back 1, 16bit
		res = i2c_smbus_write_byte_data(ect223h_ptr->client, 0x00, 0x01);
	} else { // bank 0 , 8bit
		res = i2c_smbus_write_byte_data(ect223h_ptr->client, 0x00, 0x00);
	}
        if(res < 0)
                return res;

	addr = reg & 0xff;
// read data
	if(reg & 0x100) { // back 1, 16bit
		res = i2c_smbus_read_i2c_block_data(ect223h_ptr->client, addr, 2, (u8 *)&data);
	} else {
		data = i2c_smbus_read_byte_data(ect223h_ptr->client, addr);
	}
	return data;
}


/* ect223h file operation */
int ect223h_device_open( struct inode* inode, struct file* file)
{
	return 0;
}

int ect223h_device_release(struct inode* inode, struct file* file)
{
	return 0;
}

int ect223h_device_ioctl(struct inode* inode, struct file* file, unsigned int cmd, unsigned long arg)
{
	switch( cmd )
	{
	case SET_2D3D_MODE_BYPASS_BARRIER_OFF : // off
		lcd_3d_on_off(0); //off
		ect223h_i2c_write(E223H_LVDS_TX_CH_SEL, 0x8B);
		ect223h_reg_bit_control(0x10, REGBIT0, g2d3d_LR_swap);
		break;
	case SET_2D3D_MODE_PIXEL_BARRIER_ON:
		lcd_3d_on_off(1); // on
		ect223h_i2c_write(E223H_OUT_3D_FORMAT, 0x1);// 
		ect223h_i2c_write(E223H_LVDS_TX_CH_SEL, 0x0B);// 
		g2d3d_LR_swap = 0;
		ect223h_reg_bit_control(0x10, REGBIT0, g2d3d_LR_swap);
		break;
	case SET_2D3D_MODE_PIXEL_BARRIER_OFF:
		lcd_3d_on_off(0); //off
		ect223h_i2c_write(E223H_OUT_3D_FORMAT, 0x1);// 
		ect223h_i2c_write(E223H_LVDS_TX_CH_SEL, 0x0B);// 
		g2d3d_LR_swap = 0;
		ect223h_reg_bit_control(0x10, REGBIT0, g2d3d_LR_swap);
		break;
	case SET_2D3D_MODE_SIDE_BARRIER_OFF:
		lcd_3d_on_off(0); //off
		ect223h_i2c_write(E223H_OUT_3D_FORMAT, 0x6);// 
		g2d3d_LR_swap = 1;
		ect223h_reg_bit_control(0x10, REGBIT0, g2d3d_LR_swap);
		break;
	case SET_MIX_3D_LEVEL_DOWN:
		if(depth_mix_cont_level > 1) 
			depth_mix_cont_level--;
		ect223h_2d3d_depth_control_mix(depth_mix_cont_level);
		break;
	case SET_MIX_3D_LEVEL_UP:
		if(depth_mix_cont_level < 20) 
			depth_mix_cont_level++;
		ect223h_2d3d_depth_control_mix(depth_mix_cont_level);
		break;
	case SET_FRONT_BACK_LEVEL_DOWN:
		if(g2d3d_fb) {
			g2d3d_fb--;
			ect223h_2d3d_fb(g2d3d_fb);
		}
		break;
	case SET_FRONT_BACK_LEVEL_UP:
		if(g2d3d_fb < 15) {
			g2d3d_fb++;
			ect223h_2d3d_fb(g2d3d_fb);
		}
		break;
	default:
		return -ENOTTY;
	}

	return 0;
}

ssize_t ect223h_device_read(struct file *filp, char *buf, size_t count, loff_t *ofs )
{
	return 0;
}

ssize_t ect223h_device_write(struct file *filp, const char *buf, size_t count, loff_t *ofs )
{
	return 0;
}


void ect223h_reg_bit_control(u8 reg_addr, u8 reg_bit, u8 mask_mode)
{
	u8 r_data;

	r_data=ect223h_i2c_read(reg_addr);

	if(mask_mode==REG_SET)// set bit 1
		ect223h_i2c_write(reg_addr, r_data | reg_bit);
	else// MASK_AND, set bit 0
		ect223h_i2c_write(reg_addr, r_data & ~reg_bit);
}

void ect223h_input_pinmap_sel(u8 mode)
{
	ect223h_reg_bit_control(E223H_FIL_PIN_MAP_CONT, REGBIT4, mode);// IPM ->  0 :VESA, 1:JEIDA
}

void ect223h_output_pinmap_sel(u8 mode)
{
	ect223h_reg_bit_control(E223H_FIL_PIN_MAP_CONT, REGBIT5, mode);// OPM ->  0 :VESA, 1:JEIDA	
}

void ect223h_input_pinmap_res(u8 mode)
{
	ect223h_reg_bit_control(E223H_FIL_PIN_MAP_CONT, REGBIT6, mode);// IP10M ->  0 :8bit, 1:10bit
}

void ect223h_output_pinmap_res(u8 mode)
{
	ect223h_reg_bit_control(E223H_FIL_PIN_MAP_CONT, REGBIT7, mode);// OP10M ->  0 :8bit, 1:10bit	
}

void ect223h_input_mode(u8 mode)
{
	u8 r_data, w_data;

	r_data=ect223h_i2c_read(E223H_RGB_IF_CONT) & 0x1f;
	w_data = (mode<<5) | r_data;
		
	ect223h_i2c_write(E223H_RGB_IF_CONT, w_data);
}

void ect223h_output_mux_mode(u8 mode)
{
	ect223h_i2c_write(E223H_OUT_3D_FORMAT, mode);
}

void ect223h_bypass_mode(u8 en)
{
	ect223h_reg_bit_control(E223H_RGB_IN_CONT, REGBIT4, en);
	
	if(en==INPUT_BYPASS){
		ect223h_i2c_write(E223H_CLIP_OUT_LEFT_START, 0);// L start masking
		ect223h_i2c_write(E223H_CLIP_OUT_RIGHT_END, 0);// R end
		ect223h_i2c_write(E223H_CLIP_OUT_LEFT_END, 0);// L end masking
		ect223h_i2c_write(E223H_CLIP_OUT_RIGHT_START, 0);// R start masking
	}	
}

void ect223h_wire_bypass_mode(u8 enable)
{
	if(enable == INPUT_BYPASS) {
		ect223h_i2c_write(E223H_LVDS_TX_CH_SEL, 0xEB);
		ect223h_i2c_write(E223H_LVDS_RX_CH_SEL, 0x12);
		ect223h_i2c_write(E223H_INPUT_PLL_REG, 0x02);
	} else {
		ect223h_i2c_write(E223H_LVDS_RX_CH_SEL, 0x10);
		ect223h_i2c_write(E223H_INPUT_PLL_REG, 0x03);
		ect223h_i2c_write(E223H_LVDS_TX_CH_SEL, 0xE4);
	}
}

int ect223h_detect(void)
{
	unsigned short id;
	int res;

	res = i2c_smbus_write_byte_data(ect223h_ptr->client, 0x00, 0x00);
	if(res < 0) return -ENODEV;

	res = i2c_smbus_read_i2c_block_data(ect223h_ptr->client, E223H_READ_CHIP_VERSION,2,(char *)&id);
	if(res < 0) return -ENODEV;
	printk("ECT223H Chip Version, %d\r\n",id);
	return id;
}

void ect223h_initialize(void)
{
	
	ect223h_reset(1); // defined in machine init
	mdelay(100);
	ect223h_reset(0); // defined in machine init
	
	ect223h_wire_bypass_mode(INPUT_BYPASS);

	ect223h_i2c_write(E223H_SYSTEM_CONT, 0xE2);// osc disable, lvds even/odd channel
	ect223h_i2c_write(E223H_RGB_IF_CONT, 0x55);// in/out oddeven swap, merge enable

	// lvds pinmap
	ect223h_i2c_write(E223H_SSCG1_0, 0xB0);// SSCG1 pll powerdown
	ect223h_i2c_write(E223H_SSCG2_0, 0xB0);// SSCG2 pll powerdown
	// panel setting
	ect223h_input_pinmap_sel(LVDS_PINMAP_JEIDA);
	ect223h_output_pinmap_sel(LVDS_PINMAP_JEIDA);
	ect223h_input_pinmap_res(LVDS_PINMAP_8BIT);
	ect223h_output_pinmap_res(LVDS_PINMAP_8BIT);
//	ect223h_input_pinmap_sel(LVDS_PINMAP_VESA);
//	ect223h_output_pinmap_sel(LVDS_PINMAP_VESA);
//	ect223h_input_pinmap_res(LVDS_PINMAP_10BIT);
//	ect223h_output_pinmap_res(LVDS_PINMAP_10BIT);

	// 2d3d
	ect223h_i2c_write(E223H_DEPTH_AUTO_CONT, 0x80);// 2d3d auto variable range 0 : 0~3, enable
	ect223h_input_mode(IN_2D3D);
	ect223h_output_mux_mode(OUT_LbyL);
//	ect223h_output_mux_mode(OUT_SbyS);	
	ect223h_i2c_write(E223H_2D3D_INIT_COMPENSATION_LEVEL, 0x04);// depth level average up

	// mask
	ect223h_i2c_write(E223H_CLIP_OUT_LEFT_START, 18);// L start masking
	ect223h_i2c_write(E223H_CLIP_OUT_RIGHT_END, 18); //R end

	ect223h_i2c_write(E223H_SYSTEM_CONT, 0xEA);// FOR RGB
	ect223h_i2c_write(E223H_RGB_IN_CONT, 0);// FOR RGB
	ect223h_i2c_write(E223H_RGB_OUT_CONT, 0);// FOR RGB
	ect223h_i2c_write(E223H_FIL_PIN_MAP_CONT, 0xC0);// FOR RGB
	ect223h_i2c_write(E223H_LVDS_RX_CH_SEL, 0x2A);// FOR RGB
	ect223h_i2c_write(E223H_LVDS_TX_CH_SEL, 0x0B);// FOR RGB
	ect223h_i2c_write(E223H_OUT_3D_FORMAT, 0x01);// FOR RGB
	ect223h_i2c_write(E223H_RGB_DIR_0, 0xFF);// FOR RGB
	ect223h_i2c_write(E223H_RGB_DIR_1, 0xFF);// FOR RGB
	ect223h_i2c_write(E223H_RGB_DIR_2, 0xFF);// FOR RGB
	ect223h_i2c_write(E223H_RGB_DIR_3, 0xFF);// FOR RGB
	ect223h_i2c_write(E223H_RGB_DIR_4, 0x03);// FOR RGB
	ect223h_i2c_write(E223H_RGB_DIR_5, 0x00);// FOR RGB
	ect223h_i2c_write(E223H_RGB_DIR_6, 0x00);// FOR RGB
	ect223h_i2c_write(E223H_RGB_DIR_7, 0x00);// FOR RGB
	ect223h_i2c_write(E223H_RGB_DIR_8, 0x00);// FOR RGB
	ect223h_i2c_write(E223H_RGB_DIR_9, 0x00);// FOR RGB

	depth_mix_cont_level = 12;
	g2d3d_ob_level =11;
	g2d3d_fb_level =7;
	g2d3d_slope_level=2;
	g2d3d_LR_swap =0;// R first, HYIT
	g2d3d_mode =0;

	ect223h_2d3d_object_depth(g2d3d_ob_level);
	ect223h_2d3d_fb(g2d3d_fb_level);
	ect223h_2d3d_slope(g2d3d_slope_level);
	ect223h_reg_bit_control(0x10, REGBIT0, g2d3d_LR_swap);
}

void ect223h_2d3d_object_depth(u8 val)
{
	u8 real_val;

	real_val = 10 + (val<<1);
	ect223h_i2c_write(0xB6, real_val); // level 1
	ect223h_i2c_write(0xB7, real_val + 8); // level 2
}

void ect223h_2d3d_fb(u8 val)
{
	ect223h_i2c_write(0x4E, PIXEL_DELAY1[val]); //pixel delay 1
	ect223h_i2c_write(0x4F, PIXEL_DELAY2[val]);// pixel delay 2
	ect223h_i2c_write(0xBA, SHIFT_IMG1[val]);// shift img 1
	ect223h_i2c_write(0xBB, SHIFT_IMG2[val]);// shift img 2
}

void ect223h_2d3d_slope(u8 val)
{
	ect223h_i2c_write(0xBC, (val<<1));// level * 2 
}

void ect223h_2d3d_depth_control_mix(u8 level)
{
	u8 ob_level_1=0,ob_level_2=0;
	u8 slop_level=0;

// 3d prameter value range : 
// 3d depth control range : 0 ~ 20, 21step
// object level       : 0 ~ 60
// front/back level : 0 ~ 15
// slope level 		: 0 ~ 4

	if(level){
		ob_level_2 = level*3;// level * 3 	
		if(ob_level_2 >=8 )	
			ob_level_1 = ob_level_2 -8;
	}	

	if(level < 6){
		slop_level = 0;		
	}else if(level < 12){
		slop_level = 1;		
	}else if(level < 17){
		slop_level = 2;		
	}else if(level < 19){
		slop_level = 3;		
	}else{
		slop_level = 4;		
	}	

	if(level==0){
		ob_level_1 = 0;
		ob_level_2 = 0;
		slop_level = 0;
	}	

	// Object level
	ect223h_i2c_write(0xB6, ob_level_1); // level 1
	ect223h_i2c_write(0xB7, ob_level_2); // level 2

	// fb level
	ect223h_i2c_write(0x4E, PIXEL_DELAY1[level]);// pixel delay 1
	ect223h_i2c_write(0x4F, PIXEL_DELAY2[level]);// pixel delay 2
	ect223h_i2c_write(0xBA, SHIFT_IMG1[level]);// shift img 1
	ect223h_i2c_write(0xBB, SHIFT_IMG2[level]);// shift img 2

	// slope level
	ect223h_i2c_write(0xBC, slop_level);// 
}

static int ect223h_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ect223h_data *ect223h;
	int err = 0;

	/* setup private data */
	ect223h = kzalloc(sizeof(struct ect223h_data), GFP_KERNEL);
	if (!ect223h)
	{
		err = -ENOMEM;
		goto error_0;
	}

	/* setup i2c client */
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
		err = -ENODEV;
		goto error_1;
	}
	i2c_set_clientdata(client, ect223h);
	ect223h->client = client;
	ect223h_ptr = ect223h;

	/* detect and init hardware */
	if (ect223h_detect() != 0x28AE)
		goto error_1;

	ect223h_initialize();

	dev_info(&client->dev, "%s found\n", id->name);

	err = misc_register(&ect223h_misc);
	if (err < 0)
		goto error_1;

	printk("******* ECT223H proved *********\r\n");
	return 0;

error_1:
	kfree(ect223h);
error_0:
	return err;
}

static int ect223h_remove(struct i2c_client *client)
{
	struct ect223h_data *ect223h = i2c_get_clientdata(client);

	misc_deregister(&ect223h_misc);
	kfree(ect223h);

	return 0;
}

static int ect223h_suspend(struct i2c_client *client, pm_message_t mesg)
{
	return 0;
}

static int ect223h_resume(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id ect223h_id[] =
{
	{ECT223H_NAME, 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, ect223h_id);

struct i2c_driver ect223h_driver =
{
	.driver = {
		.name = "ect223h",
		.owner = THIS_MODULE,
	},
	.probe = ect223h_probe,
	.remove = ect223h_remove,
	.suspend = ect223h_suspend,
	.resume = ect223h_resume,
	.id_table = ect223h_id,
};

/*
 * Module init and exit
 */
static int __init ect223h_init(void)
{
	return i2c_add_driver(&ect223h_driver);
}
module_init(ect223h_init);

static void __exit ect223h_exit(void)
{
	i2c_del_driver(&ect223h_driver);
}
module_exit(ect223h_exit);

MODULE_DESCRIPTION("ECT223H 2D to 3D Real Time Converter");
MODULE_LICENSE("GPL");
MODULE_VERSION(ECT223H_VERSION);

