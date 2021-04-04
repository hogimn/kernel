/*
 * TextLCD Device Driver
 *  Hanback Electronics Co.,ltd
 * File : textlcd.h
 * Date : June, 2010
 */ 

#define DRIVER_AUTHOR           "Hanback"
#define DRIVER_DESC             "textlcd test program"	

#define TEXTLCD_NAME   		"textlcd"		
#define TEXTLCD_MODULE_VERSION  "TEXT LCD PORT V1.0"	
#define TEXTLCD_ADDRESS 	0x05000010  		
#define TEXTLCD_ADDRESS_RANGE	0x1000		

#define TEXTLCD_BASE		0xbc
#define TEXTLCD_COMMAND_SET     _IOW(TEXTLCD_BASE,0,int)
#define TEXTLCD_FUNCTION_SET    _IOW(TEXTLCD_BASE,1,int)
#define TEXTLCD_DISPLAY_CONTROL _IOW(TEXTLCD_BASE,2,int)
#define TEXTLCD_CURSOR_SHIFT    _IOW(TEXTLCD_BASE,3,int)
#define TEXTLCD_ENTRY_MODE_SET  _IOW(TEXTLCD_BASE,4,int)
#define TEXTLCD_RETURN_HOME     _IOW(TEXTLCD_BASE,5,int)
#define TEXTLCD_CLEAR           _IOW(TEXTLCD_BASE,6,int)
#define TEXTLCD_DD_ADDRESS	_IOW(TEXTLCD_BASE,7,int)
#define TEXTLCD_WRITE_BYTE      _IOW(TEXTLCD_BASE,8,int)

//Global variable
static int textlcd_usage = 0;
static unsigned short *textlcd_ioremap;

struct strcommand_varible {
	char rows;
	char nfonts;
	char display_enable;
	char cursor_enable;
	
	char nblink;
	char set_screen;
	char set_rightshit;
	char increase;
	char nshift;
	char pos;
	char command;
	char strlength;
	char buf[16];
};

// functions...
void setcommand(unsigned short command);
void writebyte(char ch);
void initialize_textlcd(void);
int function_set(int rows, int nfonts);
int display_control(int display_enable, int cursor_enable, int nblink);
int cusrsor_shit(int set_screen, int set_rightshit);
int entry_mode_set(int increase, int nshift);
int return_home(void);
int clear_display(void);
int set_ddram_address(int pos);
