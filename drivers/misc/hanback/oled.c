/****************************************************************************
 * Copyright (c) Hanback Electronics. All right reserved.
 *
 * FileName : oled.c
 *
 * Abstract :
 * 	This file is a device driver for OLED.
 *
 * Modified :
 * 	Created : July 21, 2009 by Park, Hyosang
****************************************************************************/

// ����� ������� ����
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/list.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/ioport.h>
#include <linux/delay.h>


#include "oled.h"
#include "oledbg.h"

static unsigned short ImageData[128*128];

// ������ �Լ�
unsigned int isqrt(unsigned int i) 
{
	int max;
	int j;
	unsigned long a, c, d, s;

	a=0; c=0;

	if (i>=0x10000)
		max=15;

	else max=7;

	s = 1<<(max*2);

	j=max;

	do {
		d = c + (a<<(j+1)) + s;
		if (d<=i) {
			c=d; a |= 1<<j;
		}

		if (d!=i) {
			s>>=2; j--;
		}
	} while ((j>=0)&&(d!=i));

	return a;
}


// ���ۿ� ���� ��� �Է� �Լ�
void write_cmd(unsigned char cmd)
{
	// outl() : ���μ����� I/O ������ 32��Ʈ ������(4����Ʈ)�� ����.
	outl((inl((unsigned long)oled_ioremap)&~0xFF), (unsigned long)oled_ioremap);
	
	outl((inl((unsigned long)oled_ioremap)&~D_C), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)&~CS_OLED), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)&~WN_OLED), (unsigned long)oled_ioremap);

	outl((inl((unsigned long)oled_ioremap)|cmd), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|cmd), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|cmd), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|cmd), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|cmd), (unsigned long)oled_ioremap);

	udelay(10); //KSJ
	
	outl((inl((unsigned long)oled_ioremap)|WN_OLED), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|D_C), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|CS_OLED), (unsigned long)oled_ioremap);
}

// ���ۿ� ���� data �Է� �Լ�
void write_data(unsigned char data)
{
	outl((inl((unsigned long)oled_ioremap)&~0xFF), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|D_C), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)&~CS_OLED), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)&~WN_OLED), (unsigned long)oled_ioremap);

	outl((inl((unsigned long)oled_ioremap)|data), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|data), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|data), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|data), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|data), (unsigned long)oled_ioremap);
	
	udelay(10); //KSJ

	outl((inl((unsigned long)oled_ioremap)|WN_OLED), (unsigned long)oled_ioremap);

	outl((inl((unsigned long)oled_ioremap)&~D_C), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|CS_OLED), (unsigned long)oled_ioremap);

}

//==================================================
//==================================================
void SetAddr(unsigned char x, unsigned char y)
{
	write_cmd(SET_COLUMN_ADDRESS); //colum �ּ� 

	write_data(x); 
	write_data(0x7f);
	
	write_cmd(SET_ROW_ADDRESS);
	write_data(y);
	write_data(0x7f);
	write_cmd(WRITE_GRAM);
}


//==================================================
// Plot one point
// at x,y with pixel color
//==================================================
void PutPixel(unsigned char x, unsigned char y, unsigned short color)
{
	SetAddr(x,y);
	write_data(color >> 8);
	write_data(color);
}

//======================================================================//
//======================================================================//
void Reset_SSD1355(void)
{
	outl((inl((unsigned long)oled_ioremap)&~RESETPIN_OLED), (unsigned long)oled_ioremap);
	udelay(2);
	udelay(2);
	outl((inl((unsigned long)oled_ioremap)|RESETPIN_OLED), (unsigned long)oled_ioremap);
	udelay(2);
	udelay(2);
}

//======================================================================//
void initOLED(void)
{		
	unsigned char j;

	outl((inl((unsigned long)oled_ioremap)&~0xFF),  (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|RN_OLED), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|RESETPIN_OLED), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|CS_OLED), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|WN_OLED), (unsigned long)oled_ioremap);
	outl((inl((unsigned long)oled_ioremap)|D_C), (unsigned long)oled_ioremap);
	
	Reset_SSD1355();
		
	write_cmd(SOFT_RESET); 
	
	write_cmd(COMMAD_LOCK); 
	write_data(0x12); 

	write_cmd(COMMAD_LOCK); 
	write_data(0xB3); 
	
	write_cmd(SLEEP_MODE_ON);

	write_cmd(CLOCK_FREQUENCY); 
	write_data(0x10);
	
	write_cmd(SET_MUX_RATIO); 
	write_data(0x7f);  //127

	write_cmd(SET_DISPLAY_OFFSET); 
	write_data(0x00); 

	write_cmd(MEMORY_ACCSEE_CNTL); 
	write_data(0x88); 
	write_data(0x01); 

	write_cmd(INTERFACE_PIXEL_FORMAT); 
	write_data(_65K_COLOURS); 
	
	write_cmd(EN_T_EFFECT); 
	write_data(0x00); 

	write_cmd(FUNC_SEL); 
	write_data(0x03); 

	write_cmd(CONTRAST_RGB); 
	write_data(0xc3); 
	write_data(0x55); 
	write_data(0x87); 
	
	write_cmd(WRITE_LUMINANCE); 
	write_data(0xF0); 
	
	write_cmd(GAMMA_LUT); 		

	for(j=0;j<96;j++)
		write_data(gamma0[j]);		
	
	write_cmd(SET_PHASE_LENGTH); 
	write_data(0x32); 

	write_cmd(FIRST_PRECHARGE); 
	write_data(0x09); 

	write_cmd(SECOND_PRECHARGE_PERIOD); 
	write_data(0x0b); 

	write_cmd(SET_2TH_PRECHARGE_SPEED); 
	write_data(0x03); 

	write_cmd(SET_VCOMH); 
	write_data(0x04); 

	write_cmd(DISPLAY_ALL_ONOFF); 

	write_cmd(DISPLAY_INVERSE_OFF); 
	
	write_cmd(DISPLAY_NORMAL); 

	write_cmd(SLEEP_MODE_OFF); 
}

void Draw_Line(unsigned char sx, unsigned char sy, 
	unsigned char ex, unsigned char ey, unsigned short color)
{
	
	unsigned char x, first_x, first_y, end_x, end_y;
	int gradient;
	int current_y = 0;
	
	if(sx==ex && sy == ey ) return;

	first_x	= (sx<ex?sx:ex)&0x7F;
	first_y	= (sx<=ex?sy:ey)&0x7F;

	end_x	= (sx>ex?sx:ex)&0x7F;
	end_y	= (sx>ex?sy:ey)&0x7F;
	
	if(sx==ex)
	{
		first_x = sy;
		end_x =   ey; 

		for(x=first_x;x<=end_x;x++)
		{			
			PutPixel(sx,x,color);

		}
	}
	else
	{
		gradient =  ((end_y-first_y)/(end_x-first_x));
		for(x=first_x;x<=end_x;x++)
		{
			current_y = (gradient*(x-first_x)+first_y);
			PutPixel(x,(unsigned char)current_y,color);
		}
	}
}

void Draw_Rectanle(unsigned char sx,unsigned char sy,unsigned char ex, 
	unsigned char ey,unsigned short inColor,unsigned short outColor)
{	
	unsigned char x,y;
	unsigned char first_x, first_y, end_x, end_y;
	
	if(sx==ex && sy == ey ) return;
	
	first_x = (sx<ex?sx:ex)&0x7F;
	first_y = (sx<ex?sy:ey)&0x7F;
	end_x	= (sx>ex?sx:ex)&0x7F;
	end_y	= (sx>ex?sy:ey)&0x7F;

	for(y=first_y;y<=end_y;y++) //���� �簢�� �׸���
	{
		SetAddr(first_x,y);

		for(x=first_x;x<=end_x;x++)
		{
			write_data(inColor>>8);
			write_data(inColor&0xFF);			
		}	
	}	

	Draw_Line(first_x,first_y,end_x,first_y,outColor);//-
	Draw_Line(end_x,first_y,end_x,end_y,outColor); //   |
	Draw_Line(first_x,first_y,first_x,end_y,outColor); // | 
	Draw_Line(first_x,end_y,end_x,end_y,outColor); // _
}

void Draw_Circle(unsigned char x,unsigned char y, unsigned char rad,
	unsigned short inColor)
{
	unsigned char i=1;
	int j=0;
	char sx, sy;
	int radius;
	
	if((x+rad > 127 ) || (y+rad > 127 ) || (x-rad < 0 ) ||(y-rad < 0 ))
		return ;
	
	sx = x&0x7F;
	sy = y&0x7F;	
	radius = rad * rad;	

	for(i=0;i<=rad;i++)
	{
		//j=isqrt(radius-i*i)+0.5;
		j = isqrt(radius-i*i);
		
	
		Draw_Line(sx+i,sy-j,sx+i,sy+j,inColor);
		Draw_Line(sx-i,sy-j,sx-i,sy+j,inColor);
	
		//PutPixel(x+i,y+j,outColor);//1
		//PutPixel(x+i,y-j,outColor);//4
		//PutPixel(x-i,y+j,outColor);//2
		//PutPixel(x-i,y-j,outColor);//3
	} 

}

void Print_text(unsigned char x,unsigned char y, char *text,unsigned short color)
{
	unsigned char a_index,font_index;
	unsigned char i=0,j=0,k=0,l=0;
	
	unsigned char ystep=0x01;
	a_index=0;font_index=0;
	i=y; j=x;

	while(1){

		if(text[k]=='\0') //���ڿ� ���ΰ� ?
			break;			
		l=text[k]-32;	//��Ʈ���� �ش� ���ڰ� �ִ� ��ġ�� �Ǻ�  	
		k++;			//���ڿ� ���� 
		
#if 0
	//FONT8X15[][];
	while(1){	
		if((FONT8X15[l][a_index][font_index] & ystep)) //����� ������ ��Ʈ�� 1 �ΰ� ?
			PutPixel(j,i++,color); //red				// ������ ����� �ȼ��� ���� ��´�.
		else
			i++;

		ystep<<=1;

		if(ystep==0x00) { //��Ʈ������ �����÷ο� �Ȱ�� ,�� ��� �񱳰� �����ٸ�,
			ystep=0x01;	 //�ʱ�ȭ 		
			
			if(font_index==1){
				a_index++;j++;font_index=0;i=y; 		
			}
			else 
				font_index++;
						
			if(a_index==8){  
				j+=5;i=y;font_index=0;a_index=0;	//�ش� ���ڿ��� ����� ������� ���� ���ڷ� 		
				break;
			}
		}//end if
	}//while
#endif

	//FONT5X7[][];
	while(1){	
		if(((FONT5X7[l][a_index]) & ystep)) //����� ������ ��Ʈ�� 1 �ΰ� ?
			PutPixel(j,i++,color);  // ������ ����� �ȼ��� ���� ��´�.
		else
			i++;

		ystep<<=1;

		if(ystep==0x00) {       //��Ʈ������ �����÷ο� �Ȱ�� ,�� ��� �񱳰� �����ٸ�,
			ystep=0x01;     //�ʱ�ȭ

			a_index++; j++; i=y;

			if(a_index==5){
				j+=1; i=y;
				a_index = 0; l=0;     //�ش� ������ ����� ������� ���� ���ڷ�
				break;
			}
		}//end if
	}//while
	}//while

}

void OLED_DrawImage(unsigned short *Image0)
{
	unsigned short i,j,w_count,h_count,k=16384;

	w_count=128;
	h_count=128;

	for(i=0; i<h_count;i++)
		for(j=w_count; j>0; j--)
			PutPixel(j,i,Image0[k--]); 

}

// �޴� �ؽ�Ʈ�� ���
void print_Menu(void)
{
	Print_text(5, 10, "<OLED Draw Example>", WHITE);
	Print_text(10, 25, " 1. MENU print", WHITE);
	Print_text(10, 35, " 2. CLEAR Window", WHITE);
	Print_text(10, 45, " 3. Image draw", WHITE);
	Print_text(10, 55, " 4. Rect draw", WHITE);
	Print_text(10, 65, " 5. Line draw", WHITE);
	Print_text(10, 75, " 6. Circle draw", WHITE);
	Print_text(10, 85, " 7. Text Print", WHITE);
}

// ����̽� ����̹��� ���⸦ �����ϴ� �Լ�
static ssize_t oled_write(struct file *inode, const char*gdata, size_t length, loff_t *off_what) 
{

	int i,j,ret; // add (a) KSJ
	unsigned short w_count,h_count,k=16384;

	//initOLED();  // KSJ
	
	// ����� �޸� gdata�� Ŀ�� �޸� buf�� length��ŭ ����
	ret=copy_from_user((char*)ImageData,(char *)gdata,length);

	if(ret < 0) return -1;

	w_count=128;
	h_count=128;


	for(i=h_count; i>0;i--)
		for(j=w_count; j>0; j--)
		{
			//PutPixel(j,i,ImageData[k]);
			//PutPixel(j,i,ImageData[k]);
			//PutPixel(j,i,ImageData[k]);
			//PutPixel(j,i,ImageData[k]);
			PutPixel(j,i,ImageData[k--]);
		}
		
				//PutPixel(j,i,0xffff);
	

	return length;
}


//read()�� write()�� �����ϱ� ����� ����̽� ����̹��� ����� ó���� �����ϴ� �Լ�
static int oled_ioctl(struct inode *inode, struct file *flip, unsigned int cmd, unsigned long arg)
{
	struct cmd_clear_value	cmd_clear;
	struct cmd_rect_value 	cmd_rect;
	struct cmd_line_value 	cmd_line;
	struct cmd_circle_value cmd_circle;
	struct cmd_text_value	cmd_text;
	int ret=0;

	switch(cmd) {	// cmd�� ���� ���� ����
		case OLED_INIT:
			initOLED();
			Draw_Rectanle(0,0,127,127, BLACK,BLACK);
			break;

		case MENU_PRINT:
			print_Menu();
			break;

		case WIN_CLEAR:	
			// ����� �޸� arg�� Ŀ�� �޸� cmd�� 8����Ʈ ����
			ret=copy_from_user(&cmd_clear,(char *)arg,8);
			Draw_Rectanle(cmd_clear.sx, cmd_clear.sy, cmd_clear.ex, cmd_clear.ey, BLACK,BLACK);
			break;
	
		case IMAGE_DRAW:
			OLED_DrawImage(oledlogo);
			break;
			
		case RECT_DRAW:
			ret=copy_from_user(&cmd_rect,(char *)arg,12);
			Draw_Rectanle(cmd_rect.x, cmd_rect.y, cmd_rect.w, cmd_rect.h, cmd_rect.inC, cmd_rect.outC); 
			break;

		case LINE_DRAW:
			ret=copy_from_user(&cmd_line,(char *)arg,10);
			Draw_Line(cmd_line.sx, cmd_line.sy, cmd_line.ex, cmd_line.ey, cmd_line.color);
			break;
			
		case CIRCLE_DRAW:
			ret=copy_from_user(&cmd_circle,(char *)arg,10);
			Draw_Circle(cmd_circle.x, cmd_circle.y, cmd_circle.rad, cmd_circle.inC);
			break;
			
		case TEXT_PRINT:
			ret=copy_from_user(&cmd_text,(char *)arg,28);
			Print_text(cmd_text.x, cmd_text.y, cmd_text.buf, cmd_text.color);
			break;

		default:
			return -EINVAL;
	}
	if(ret<0) return -1;

	return 0;
}

// open() �Լ��� �̿��Ͽ� ����̽� ����̹��� ���� ��� ȣ��Ǵ� �Լ�
static int oled_open(struct inode *minode, struct file *mfile)
{
	// ����̽��� ���� �ִ��� Ȯ��.
	if(oled_usage !=0) return -EBUSY;

	// �����ּ� ����
	oled_ioremap = (unsigned short*)ioremap(OLED_ADDRESS, OLED_ADDRESS_RANGE);

	// ����� �� �ִ� I/O �������� Ȯ��
	if(!check_mem_region((unsigned long)oled_ioremap, OLED_ADDRESS_RANGE)) 
	{
		// I/O �޸� ������ ���
		request_mem_region((unsigned long)oled_ioremap, OLED_ADDRESS_RANGE, OLED_NAME);
	}
	else printk(KERN_WARNING"Can't get IO Region 0x%x\n", OLED_ADDRESS);
	
	oled_usage = 1;

	initOLED();
	Draw_Rectanle(0,0,127,127, BLACK,BLACK);

	return 0;
}

// ���� ���α׷����� ����̽��� ���̻� ������� �ʾƼ� �ݱ⸦ �����ϴ� �Լ�
static int oled_release(struct inode *minode, struct file *mfile)
{
	// ���ε� �����ּҸ� ����
	iounmap(oled_ioremap);

	// ��ϵ� I/O �޸� ������ ����
	release_mem_region((unsigned long)oled_ioremap, OLED_ADDRESS_RANGE);

	oled_usage = 0;
	return 0;
}

// ���� ���۷��̼� ����ü
// ������ ���� open()�� ����Ѵ�. open()�� �ý��� ���� ȣ���Ͽ� Ŀ�� ���η� ����.
// �ش� �ý��� �ݰ� ���õ� ���� ������ ����ü ������ open�� �ش��ϴ� �ʵ尡 ����̹� ������
// oled_open()���� ���ǵǾ� �����Ƿ� oled_open()�� ȣ��ȴ�.
// ioctl�� release�� ���������� �����Ѵ�. ���� ��ϵ��� ���� ���ۿ� ���ؼ��� Ŀ�ο��� ������
// ���� default ������ �ϵ��� �Ǿ� �ִ�.
static struct file_operations oled_fops = {
	.owner		= THIS_MODULE,
	.open		= oled_open,
	.write		= oled_write,
	.ioctl		= oled_ioctl,
	.release	= oled_release,
};

static struct miscdevice oled_driver = {
	.fops	= &oled_fops,
	.name = OLED_NAME,
	.minor = MISC_DYNAMIC_MINOR,
};

// ����� Ŀ�� ���η� ����
// ��� ���α׷��� �ٽ����� ������ Ŀ�� ���η� ���� ���񽺸� �����޴� ���̹Ƿ�
// Ŀ�� ���η� ���� init()�� ���� �����Ѵ�.
// ���� ���α׷��� �ҽ� ���ο��� ���ǵ��� ���� ���� �Լ��� ����Ѵ�. �װ��� �ܺ�
// ���̺귯���� ������ �������� ��ũ�Ǿ� ���Ǳ� �����̴�. ��� ���α׷��� Ŀ��
// �����ϰ� ��ũ�Ǳ� ������ Ŀ�ο��� �����ϰ� ����ϴ� �Լ����� ����� �� �ִ�.
int oled_init(void)
{
        printk("driver: %s DRIVER INIT\n",OLED_NAME);
	return misc_register(&oled_driver);
}

// ����� Ŀ�ο��� ����
void oled_exit(void) 
{
	misc_deregister(&oled_driver);
        printk("driver: %s DRIVER EXIT\n",OLED_NAME);
}

module_init(oled_init);		// ��� ���� �� ȣ��Ǵ� �Լ�
module_exit(oled_exit);		// ��� ���� �� ȣ��Ǵ� �Լ�

MODULE_AUTHOR(DRIVER_AUTHOR); 	 // ����� ������
MODULE_DESCRIPTION(DRIVER_DESC); // ��⿡ ���� ����
MODULE_LICENSE("Dual BSD/GPL");	 // ����� ���̼��� ���
