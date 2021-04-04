/* 
 *
 * Zinitix touch driver
 *
 * Copyright (C) 2009 Zinitix, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
 
#ifndef ZINITIX_REG_HEADER
#define ZINITIX_REG_HEADER

// select touch mode	// 1 is recommended
#define	TOUCH_MODE				1

// max 10
#define	SUPPORTED_FINGER_NUM			10

// max 8
#define	SUPPORTED_BUTTON_NUM			4

// Thread or Workqueue : workqueue is recommended
#define	USE_THREAD_METHOD	0	// 1 = thread, 0 = workqueue

// Upgrade Method
#define	TOUCH_ONESHOT_UPGRADE	0
#define	TOUCH_USING_ISP_METHOD	0	// if you use isp mode, you must add i2c device : name = "zinitix_isp" , addr 0x50

// Power Control
#define	RESET_CONTROL			1
#define	DELAY_FOR_SIGNAL_DELAY	10	//us
#define	CHIP_POWER_OFF_DELAY	500	//ms
#define	CHIP_ON_DELAY			500	//ms

#if (RESET_CONTROL==0)
#undef	TOUCH_USING_ISP_METHOD
#define	TOUCH_USING_ISP_METHOD	0
#endif

typedef	enum
{	
	POWER_OFF,
	POWER_ON,
	RESET_LOW,
	RESET_HIGH,
}_zinitix_power_control;

// Button Enum
typedef	enum
{	
	ICON_BUTTON_UNCHANGE,
	ICON_BUTTON_DOWN,
	ICON_BUTTON_UP,
}_zinitix_button_event;


// ESD Protection
#define	ZINITIX_ESD_TIMER_INTERVAL	0	//second : if 0, no use. if you have to use, 3 is recommended
#define	ZINITIX_SCAN_RATE_HZ	60

// Use sysfs : for show info & upgrade
#define	USE_UPDATE_SYSFS	1
#define	INPUT_BINARY_DATA	0


// Other Things
#define	ZINITIX_INIT_RETRY_CNT	10
#define	I2C_SUCCESS				0

// Register Map
#define ZINITIX_REST_CMD			0x00
#define ZINITIX_WAKEUP_CMD			0x01

#define ZINITIX_IDLE_CMD			0x04
#define ZINITIX_SLEEP_CMD			0x05

#define	ZINITIX_CLEAR_INT_STATUS_CMD	0x03	
#define	ZINITIX_CALIBRATE_CMD		0x06
#define	ZINITIX_SAVE_STATUS_CMD		0x07
#define	ZINITIX_RECALL_FACTORY_CMD		0x0f

// 0x10~12
#define ZINITIX_TOUCH_MODE			0x10
#define ZINITIX_CHIP_REVISION		0x13
#define ZINITIX_EEPROM_INFO		0x14

// 0x20~21
#define ZINITIX_TOTAL_NUMBER_OF_X		0x20
#define ZINITIX_TOTAL_NUMBER_OF_Y		0x21
#define ZINITIX_SUPPORTED_FINGER_NUM	0x22

#define ZINITIX_AFE_FREQUENCY		0x23

#define	ZINITIX_X_RESOLUTION		0x28
#define	ZINITIX_Y_RESOLUTION		0x29

// 0x30~33
#define ZINITIX_CALIBRATION_REF 		0x30
#define ZINITIX_CALIBRATION_DEFAULT_N 	0x31
#define ZINITIX_NUMBER_OF_CALIBRATION 	0x32
#define ZINITIX_CALIBRATION_ACCURACY	0x33

#define	ZINITIX_PERIODICAL_INTERRUPT_INTERVAL	0x35

#define	ZINITIX_POINT_STATUS_REG		0x80
#define	ZINITIX_ICON_STATUS_REG		0x9a	//icon event - four icon

#define	ZINITIX_RAWDATA_REG		0x9F	//raw data 320byte

#define	ZINITIX_EEPROM_INFO_REG		0xaa
#define	ZINITIX_DATA_VERSION_REG		0xab

#define ZINITIX_FIRMWARE_VERSION		0xc9

#define	ZINITIX_ERASE_FLASH		0xc9
#define	ZINITIX_WRITE_FLASH		0xc8
#define	ZINITIX_READ_FLASH			0xca


//0xF0
#define	ZINITIX_INT_ENABLE_FLAG		0xf0
//---------------------------------------------------------------------

// Interrupt & status register flag bit
//-------------------------------------------------
#define	BIT_PT_CNT_CHANGE			0
#define	BIT_DOWN				1
#define	BIT_MOVE				2
#define	BIT_UP					3
#define	BIT_HOLD				4
#define	BIT_LONG_HOLD				5
#define	RESERVED_0				6
#define	RESERVED_1				7
#define	BIT_WEIGHT_CHANGE			8
#define	BIT_PT_NO_CHANGE			9
#define	BIT_REJECT				10
#define	BIT_PT_EXIST				11		// status register only
//-------------------------------------------------
#define	RESERVED_2				12
#define	RESERVED_3				13
#define	RESERVED_4				14
#define	BIT_ICON_EVENT				15

// 4 icon
#define	BIT_O_ICON0_DOWN			0
#define	BIT_O_ICON1_DOWN			1
#define	BIT_O_ICON2_DOWN			2
#define	BIT_O_ICON3_DOWN			3
#define	BIT_O_ICON4_DOWN			4
#define	BIT_O_ICON5_DOWN			5
#define	BIT_O_ICON6_DOWN			6
#define	BIT_O_ICON7_DOWN			7

#define	BIT_O_ICON0_UP				8
#define	BIT_O_ICON1_UP				9
#define	BIT_O_ICON2_UP				10
#define	BIT_O_ICON3_UP				11
#define	BIT_O_ICON4_UP				12
#define	BIT_O_ICON5_UP				13
#define	BIT_O_ICON6_UP				14
#define	BIT_O_ICON7_UP				15


#define	SUB_BIT_EXIST			0		// status register only
#define	SUB_BIT_DOWN			1
#define	SUB_BIT_MOVE			2
#define	SUB_BIT_UP			3
#define	SUB_BIT_UPDATE			4
#define	SUB_BIT_WAIT			5


#define	zinitix_bit_set(val,n)		((val) &=~(1<<(n)), (val) |=(1<<(n)))
#define	zinitix_bit_clr(val,n)		((val) &=~(1<<(n)))
#define	zinitix_bit_test(val,n)		((val) & (1<<(n)))

extern void smdkc210_zinitix_set(void);
extern void smdkc210_zinitix_reset(void);
#endif //ZINITIX_REG_HEADER


