//##########################################################
// H/W model 		: ECT223H 
// File name		: ect223h.h
//##########################################################
#ifndef _ECT223H_H_
#define _ECT223H_H_

#define E223H_BANK_SEL				0x0000
/* BANK 0 REGISTER */
#define E223H_SYSTEM_CONT			0x0001
#define E223H_RGB_IN_CONT			0x0002
#define E223H_RGB_OUT_CONT			0x0003
#define E223H_RGB_IF_CONT			0x0004
#define E223H_FIL_PIN_MAP_CONT			0x0005
#define E223H_RGB_CLK_OUT_SEL			0x0006
#define E223H_DEPTH_AUTO_CONT			0x0007
#define E223H_EXT_PIN_CONT			0x0008
#define E223H_LVDS_RX_CH_SEL			0x0009
#define E223H_LVDS_TX_CH_SEL			0x000A
#define E223H_2D3D_INIT_COMPENSATION_LEVEL	0x000B
#define E223H_PC_DET_PIXEL_THRESHOLD		0x000C // 16BIT
#define E223H_PC_DET_THRESHOLD			0x000E // 16BIT
#define E223H_MUX_MODE_CONT			0x0010
#define E223H_OUT_3D_FORMAT			0x0011
#define E223H_IN_FIFO_RD_START			0x0012
#define E223H_OUT_FIFO_RD_START			0x0013
#define E223H_OUT_HSIZE_OFFSET			0x0014
#define E223H_LLRR_HALF_OFFSET			0x0015
#define E223H_DEBUG_REG				0x0016
#define E223H_2D3D_PC_DET_DEFAULT_LEVEL		0x0017
#define E223H_DET_CONT				0x0018
#define E223H_IN_DET_VSIZE			0x001A // 16BIT
#define E223H_IN_DET_HSIZE			0x001C // 16BIT
#define E223H_IN_DET_HBLANK_SIZE		0x001E // 16BIT
#define E223H_IN_DET_VBACKPORCH_SIZE		0x0020 // 16BIT
#define E223H_IN_DET_HBACKPORCH_SIZE		0x0022 // 16BIT
#define E223H_IN_DET_SYNC_POL			0x0024
#define E223H_IN_HFIL_RED_COEFFICIENT_0		0x0030 // 16BIT
#define E223H_IN_HFIL_RED_COEFFICIENT_1		0x0032 // 16BIT
#define E223H_IN_HFIL_RED_COEFFICIENT_2		0x0034 // 16BIT
#define E223H_IN_HFIL_RED_COEFFICIENT_3		0x0036 // 16BIT
#define E223H_IN_HFIL_RED_COEFFICIENT_4		0x0038 // 16BIT
#define E223H_IN_HFIL_GRN_COEFFICIENT_0		0x003A // 16BIT
#define E223H_IN_HFIL_GRN_COEFFICIENT_1		0x003C // 16BIT
#define E223H_IN_HFIL_GRN_COEFFICIENT_2		0x003E // 16BIT
#define E223H_IN_HFIL_GRN_COEFFICIENT_3		0x0040 // 16BIT
#define E223H_IN_HFIL_GRN_COEFFICIENT_4		0x0042 // 16BIT
#define E223H_IN_HFIL_BLU_COEFFICIENT_0		0x0044 // 16BIT
#define E223H_IN_HFIL_BLU_COEFFICIENT_1		0x0046 // 16BIT
#define E223H_IN_HFIL_BLU_COEFFICIENT_2		0x0048 // 16BIT
#define E223H_IN_HFIL_BLU_COEFFICIENT_3		0x004A // 16BIT
#define E223H_IN_HFIL_BLU_COEFFICIENT_4		0x004C // 16BIT
#define E223H_2D3D_DELAY_IMG_1			0x004E 
#define E223H_2D3D_DELAY_IMG_2			0x004F 
#define E223H_OUT_HFIL_RED_COEFFICIENT_0	0x0050 // 16BIT
#define E223H_OUT_HFIL_RED_COEFFICIENT_1	0x0052 // 16BIT
#define E223H_OUT_HFIL_RED_COEFFICIENT_2	0x0054 // 16BIT
#define E223H_OUT_HFIL_RED_COEFFICIENT_3	0x0056 // 16BIT
#define E223H_OUT_HFIL_RED_COEFFICIENT_4	0x0058 // 16BIT
#define E223H_OUT_HFIL_GRN_COEFFICIENT_0	0x005A // 16BIT
#define E223H_OUT_HFIL_GRN_COEFFICIENT_1	0x005C // 16BIT
#define E223H_OUT_HFIL_GRN_COEFFICIENT_2	0x005E // 16BIT
#define E223H_OUT_HFIL_GRN_COEFFICIENT_3	0x0060 // 16BIT
#define E223H_OUT_HFIL_GRN_COEFFICIENT_4	0x0062 // 16BIT
#define E223H_OUT_HFIL_BLU_COEFFICIENT_0	0x0064 // 16BIT
#define E223H_OUT_HFIL_BLU_COEFFICIENT_1	0x0066 // 16BIT
#define E223H_OUT_HFIL_BLU_COEFFICIENT_2	0x0068 // 16BIT
#define E223H_OUT_HFIL_BLU_COEFFICIENT_3	0x006A // 16BIT
#define E223H_OUT_HFIL_BLU_COEFFICIENT_4	0x006C // 16BIT
#define E223H_LR_COLOR_PATTERN_CONT		0x0070
#define E223H_LEFT_PATTERN_RED_VALUE		0x0071
#define E223H_LEFT_PATTERN_GRN_VALUE		0x0072
#define E223H_LEFT_PATTERN_BLU_VALUE		0x0073
#define E223H_RIGHT_PATTERN_RED_VALUE		0x0074
#define E223H_RIGHT_PATTERN_GRN_VALUE		0x0075
#define E223H_RIGHT_PATTERN_BLU_VALUE		0x0076
#define E223H_DE_MODE_THRESHOLD			0x0079
#define E223H_2D3D_HSIZE			0x00B0 // 16BIT
#define E223H_2D3D_VSIZE			0x00B2 // 16BIT
#define E223H_2D3D_CONT				0x00B4
#define E223H_2D3D_OCCLUSION_START_POINT	0x00B5
#define E223H_2D3D_DEPTH_STRONG_LEVEL_1		0x00B6
#define E223H_2D3D_DEPTH_STRONG_LEVEL_2		0x00B7
#define E223H_2D3D_TYPE				0x00B8
#define E223H_2D3D_DEPTH_GAMMA			0x00B9
#define E223H_2D3D_SHIFT_IMG_1			0x00BA
#define E223H_2D3D_SHIFT_IMG_2			0x00BB
#define E223H_2D3D_SLOP				0x00BC
#define E223H_OCCLUSION_SYNC_DELAY		0x00BD
#define E223H_OCCLUSION_DATA_DELAY		0x00BE
#define E223H_MODIFIED_DEPTH_STRONG_LEVEL	0x00BF // READ ONLY
#define E223H_GPIO_INPUT_DATA			0x00E0 // READ ONLY
#define E223H_GPIO_OUTPUT_DATA			0x00E1
#define E223H_GPIO_DIR				0x00E2
#define E223H_GPIO_MODE				0x00E3
#define E223H_GPIO_DEBUG			0x00E4
#define E223H_READ_CHIP_VERSION			0x00EE // READ ONLY, 0x28AE
#define E223H_CLIP_CONT				0x00F0
#define E223H_CLIP_IN_HSTART			0x00F1
#define E223H_CLIP_IN_HSIZE			0x00F2 // 16BIT
#define E223H_CLIP_OUT_LEFT_START		0x00F4
#define E223H_CLIP_OUT_LEFT_END			0x00F5
#define E223H_CLIP_OUT_RIGHT_START		0x00F6
#define E223H_CLIP_OUT_RIGHT_END		0x00F7
/* BANK 1 REGISTER */
#define E223H_SSCG1_0				0x0101
#define E223H_SSCG1_1				0x0102
#define E223H_SSCG1_2				0x0103
#define E223H_SSCG1_3				0x0104
#define E223H_SSCG1_4				0x0105
#define E223H_SSCG2_0				0x0106
#define E223H_SSCG2_1				0x0107
#define E223H_SSCG2_2				0x0108
#define E223H_SSCG2_3				0x0109
#define E223H_SSCG2_4				0x010A
#define E223H_RX_1E_0				0x010B
#define E223H_RX_1E_1				0x010C
#define E223H_RX_1E_2				0x010D
#define E223H_RX_1E_3				0x010E
#define E223H_RX_1O_0				0x0110
#define E223H_RX_1O_1				0x0111
#define E223H_RX_1O_2				0x0112
#define E223H_RX_1O_3				0x0113
#define E223H_TX_1E_0				0x0115
#define E223H_TX_1E_1				0x0116
#define E223H_TX_1E_2				0x0117
#define E223H_TX_1E_3				0x0118
#define E223H_TX_1O_0				0x0119 
#define E223H_TX_1O_1				0x011A 
#define E223H_TX_1O_2				0x011B 
#define E223H_TX_1O_3				0x011C 
#define E223H_INPUT_PLL_REG			0x011D
#define E223H_RGB_DIR_0				0x0120
#define E223H_RGB_DIR_1				0x0121
#define E223H_RGB_DIR_2				0x0122
#define E223H_RGB_DIR_3				0x0123
#define E223H_RGB_DIR_4				0x0124
#define E223H_RGB_DIR_5				0x0125
#define E223H_RGB_DIR_6				0x0126
#define E223H_RGB_DIR_7				0x0127
#define E223H_RGB_DIR_8				0x0128
#define E223H_RGB_DIR_9				0x0129
#define E223H_GPIO_PULL_UP			0x0134
#define E223H_GPIO_DRV_PWR_SEL			0x013F

int ect223h_device_open( struct inode* inode, struct file* file);
int ect223h_device_release( struct inode* inode, struct file* file);
int ect223h_device_ioctl( struct inode* inode, struct file* file, unsigned int cmd, unsigned long arg);
ssize_t ect223h_device_read( struct file *filp, char *buf, size_t count, loff_t *ofs );
ssize_t ect223h_device_write( struct file *filp, const char *buf, size_t count, loff_t *ofs );


int ect223h_i2c_write(unsigned short reg, unsigned short data);
int ect223h_i2c_read(unsigned short reg);
void ect223h_reg_bit_control(u8 reg_addr, u8 reg_bit, u8 mask_mode);
void ect223h_input_pinmap_sel(u8 mode);
void ect223h_output_pinmap_sel(u8 mode);
void ect223h_input_pinmap_res(u8 mode);
void ect223h_output_pinmap_res(u8 mode);
void ect223h_input_mode(u8 mode);
void ect223h_output_mux_mode(u8 mode);
void ect223h_bypass_mode(u8 en);
void ect223h_wire_bypass_mode(u8 enable);
int ect223h_detect(void);
void ect223h_initialize(void);
void ect223h_2d3d_object_depth(u8 val);
void ect223h_2d3d_fb(u8 val);
void ect223h_2d3d_slope(u8 val);
void ect223h_2d3d_depth_control_mix(u8 level);

#endif 

