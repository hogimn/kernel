/*
 * OLED Device Driver
 *  Hanback Electronics Co.,ltd
 * File : oled.h
 * Date : April,2009
 */

#define DRIVER_AUTHOR		"hanback"	// 모듈의 저작자
#define DRIVER_DESC		"oled test program" // 모듈에 대한 설명

//#define OLED_MAJOR		247		 // 디바이스 주번호
#define OLED_NAME		"oled"	 // 디바이스 이름
#define OLED_MODULE_VERSION	"OLED PORT V0.1" // 디바이스 버전
#define OLED_ADDRESS		0x050000c0	 // Physical Address
#define OLED_ADDRESS_RANGE	0x1000		 // I/O 영역의 크기

// OLED Interface
#define CS_OLED         	0x1000          // OLED CS
#define RESETPIN_OLED   	0x800
#define D_C             	0x400
#define WN_OLED         	0x200           // OLED WN : R/W#(WR#)
#define RN_OLED         	0x100           // OLED RN : E(RD#)

//--------------------------------------------------------------------------//
// SSD1355 Registers
//--------------------------------------------------------------------------//
#define	SET_COLUMN_ADDRESS		0x2A
#define	SET_ROW_ADDRESS			0x2B
#define	WRITE_GRAM			0x2C
#define MEMORY_ACCSEE_CNTL		0x36
#define	INTERFACE_PIXEL_FORMAT		0x3A
#define EN_T_EFFECT			0x35
#define	SET_MUX_RATIO			0xCA
#define	SET_DISPLAY_OFFSET		0xC8
#define FUNC_SEL			0xB3
#define	CLOCK_FREQUENCY			0xD2

#define	CONTRAST_RGB			0xBA
#define WRITE_LUMINANCE			0x51

#define	DISPLAY_ALL_ONOFF		0x29
#define	DISPLAY_INVERSE_OFF		0x20
#define	DISPLAY_NORMAL			0x13

#define	GAMMA_LUT			0xBE

#define	SET_PHASE_LENGTH		0xCD
#define	FIRST_PRECHARGE			0xBD
#define	SECOND_PRECHARGE_PERIOD		0xCE

#define	SET_2TH_PRECHARGE_SPEED		0xCE
#define	SET_VCOMH			0xD3

#define	SLEEP_MODE_ON			0x10
#define	SLEEP_MODE_OFF			0x11

#define COMMAD_LOCK			0xFD
#define SOFT_RESET			0x01

#define	_65K_COLOURS			0x05
#define	_262K_COLOURS			0x06

#define RGB565(r,g,b) ( (((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3) )

//--------------------------------------------------------------------------//
// Screen Related definitions
//--------------------------------------------------------------------------//
#define X_RES				128
#define Y_RES				128

#define MAXCOLOUR			0xFFFF
#define	MAX_TEXTROWS			25

//ioctl cmd
enum {
	OLED_INIT=0x10,  MENU_PRINT, WIN_CLEAR, IMAGE_DRAW, 
	RECT_DRAW, LINE_DRAW, CIRCLE_DRAW, TEXT_PRINT
} IOCTL_CMD ;

#define BLACK                   	0x0000
#define WHITE                   	0xFFFF
#define RED                     	0xF800
#define GREEN                   	0x07E0
#define BLUE                    	0x001F
#define YELLOW                  	RED | GREEN

//Global variable
static int oled_usage = 0;
static unsigned short *oled_ioremap;

// cmd struct
struct cmd_clear_value {
	unsigned short sx; 	//start x, y
	unsigned short sy;
	unsigned short ex;	//end x, y
	unsigned short ey;
};

struct cmd_rect_value {
	unsigned short x; 	// start x, y
	unsigned short y;
	unsigned short w;	// wight, height
	unsigned short h;
	unsigned short inC;	// in Color
	unsigned short outC;	// out Color
};

struct cmd_line_value {
	unsigned short sx;	// start x, y
	unsigned short sy;
	unsigned short ex;	// end x, y
	unsigned short ey;
	unsigned short color;	// line color
};

struct cmd_circle_value {
	unsigned short x;
	unsigned short y;
	unsigned short rad;
	unsigned short inC;	// in Color
	unsigned short outC;	// out Color
};

struct cmd_text_value {
	unsigned short x;
	unsigned short y;
	char 	 buf[21];
	unsigned short color;
};

const unsigned char gamma0[] = {
	0x01,0x04,0x07,0x09,
	0x0B,0x0C,0x0D,0x0E,
	0x0F,0x10,0x11,0x12,
	0x13,0x16,0x1d,0x21,
	0x23,0x24,0x2E,0x31,
	0x32,0x37,0x41,0x43,
	0x4c,0x51,0x54,0x61,
	0x68,0x6f,0x75,0x7f,//

	0x01,0x08,0x0E,0x13,
	0x17,0x1a,0x1c,0x1e,
	0x1f,0x20,0x21,0x22,
	0x23,0x25,0x2b,0x30,
	0x31,0x33,0x37,0x39,
	0x41,0x43,0x47,0x50,
	0x52,0x55,0x61,0x63,
	0x64,0x70,0x74,0x7f,//

	0x01,0x04,0x07,0x09,
	0x0B,0x0C,0x0D,0x0E,
	0x0F,0x10,0x11,0x12,
	0x13,0x16,0x1E,0x21,
	0x22,0x25,0x2d,0x30,
	0x32,0x37,0x41,0x43,
	0x4c,0x51,0x56,0x62,
	0x69,0x70,0x77,0x7f
};

// Functions...
void write_cmd(unsigned char);
void write_data(unsigned char);
void SetAddr(unsigned char x, unsigned char y);
void PutPixel(unsigned char x, unsigned char y, unsigned short color);
void Reset_SSD1355(void);
void initOLED(void);
void Print_text(unsigned char x,unsigned char y, char *text,unsigned short color);
void print_Menu(void);

