#include "dev_uart.h"
#include "string.h"
#include "delay.h"
//#include "dev_gpio.h"
#include "drv_lcd.h"
#include "math.h"
#include "dev_lcd.h"
//#include "drv_lcd_font_data.h"

u8 lcd_lbuf[640];

u16 qr_pencolor = 0xFFFF;//前景色，画笔颜色白色
u16 qr_backcolor = 0x0;//背景色黑色

#if 0
const u8 wechat_logo[]=
{
	/*--  调入了一幅图像：wechat.bmp  --*/
	/*--  宽度x高度=32x32  --*/
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF8,0xFC,0xFE,0xEE,0xEE,0xFE,0xFE,0xFE,0xEE,
	0xEE,0xFC,0xF8,0xF0,0xC0,0xC0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x03,0x07,0x1F,0x0F,0x0F,0x0F,0x0F,0x1F,0x3F,
	0x7F,0x7F,0xFD,0xFF,0xFF,0xFF,0xFD,0xFF,0x3F,0x1E,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
const u8 alipay_logo[]=
{
	/*--  调入了一幅图像：alipay.bmp  --*/
	/*--  宽度x高度=32x32  --*/
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x84,0x84,0x84,0x84,0x96,0x1F,0x1F,
	0x1F,0x84,0xF4,0xF4,0x34,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x11,0x31,0x20,0x20,0x20,0x30,0x31,0x19,0x1F,
	0x0F,0x07,0x07,0x0E,0x0C,0x1C,0x1C,0x1C,0x38,0x38,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
#endif

const u8 no_font[]=
{
0x80,0xFE,0x82,0x82,0x82,0x82,0x82,0x82,0x80,0x80,0x80,0x80,0xFE,0x9C,0xB0,0xE0,
0x80,0x80,0x80,0xFE,0x80,0x80,0x80,0x82,0x82,0x82,0xFE,0x82,0x82,0x82,0x80,0x80,
0x00,0x7F,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,
0x03,0x0E,0x38,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x00,0x00,/*"d:\Users\oushaojun\Desktop\新建位图图像.bmp",0*/
};

//延迟程序
void delay1(long dt)               //us
{
    while(dt)
    {
        dt--;
    }
}
void lcd_qr_init_gpio(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);//使能GPIOD时钟
    //B10/11初始化设置
    GPIO_InitStructure.GPIO_Pin = CS_LCD_QR | AO_LCD_QR |RST_LCD_QR |BL_LCD_QR;//
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIO
  

    GPIO_SetBits(GPIOD,CS_LCD_QR | AO_LCD_QR | BL_LCD_QR);//设置高电平
    GPIO_ResetBits(GPIOD,RST_LCD_QR);//复位脚设置为低电平
    //复位脚没有设置
    SPI2_Init();//初始化spi2
    //dev_spimaster_open(SPI10, 0, 2);
}
void lcd_qr_send_command(u8 data1)   
{
	
	CS_LCD_QR_level(0);
	AO_LCD_QR_level(0);
	
	dev_spi_write(&data1, 1);
	
	CS_LCD_QR_level(1);
}

void lcd_qr_send_data(u8 *data, u16 len)
{
	
	CS_LCD_QR_level(0);
	AO_LCD_QR_level(1);
	
	dev_spi_write(data, len);

	CS_LCD_QR_level(1);
}

void lcd_qr_send_a_data(u8 data)
{
	
	CS_LCD_QR_level(0);
	AO_LCD_QR_level(1);
	
	dev_spi_write(&data, 1);

	CS_LCD_QR_level(1);
}

int st7735s_write( u8 cmd, u8 * pdat, int len )
{
	//int i;
	lcd_qr_send_command(cmd); 
	delayms(2);

	//for( i = 0; i < len; i++ )
	{
		lcd_qr_send_data(pdat, len);
	}
    return 0;
}

/*
rect (x1,y1), (x2, y2)
*/
void st7735s_set_disp_rect( u16 x1, u16 y1, u16 x2,u16 y2)
{
	u8 dat1[4] = {0x0, 0x02, 0x0, 0x81};
	u8 dat2[4] = {0x0, 0x01, 0x0, 0xa0};

	dat1[0] = (x1 >> 8);
	dat1[1] = (x1 & 0xff);
	dat1[2] = (x2 >> 8);
	dat1[3] = (x2 & 0xff);

	st7735s_write( 0x2A, dat1, 4 );
	
	dat2[0] = (y1 >> 8);
	dat2[1] = (y1 & 0xff);
	dat2[2] = (y2 >> 8);
	dat2[3] = (y2 & 0xff);

	st7735s_write( 0x2B, dat2, 4 );
	lcd_qr_send_command(0x2C); 
}

void dev_lcd_qr_fill_rectangle(u8 x1,u8 y1,u8 x2,u8 y2, u16 color)
{
	int i,j;
	if(y2>LCD_QR_HEIGHT-1)
		y2 = LCD_QR_HEIGHT-1 ;
	if(x2>LCD_QR_WIDTH-1)
		x2 = LCD_QR_WIDTH-1 ;

	//uart_printf("x2 = %d y2 = %d \r\n",x2, y2);
	st7735s_set_disp_rect( x1, y1, x2, y2);
	for (i = 0; i <= y2 ; i++ )
	{
		for( j = 0; j <=x2 ; j++ )
		{
			lcd_lbuf[j*2] = (u8)(color>>8) ;
			lcd_lbuf[j*2+1] = (u8)color ;
		}
		lcd_qr_send_data(lcd_lbuf, (x2 - x1 + 1) * 2);
	}
}


void lcd_qr_ascii_setPos(u8 x1, u8 y1, u8 x2, u8 y2)
{
	u8 ch = 0x00 ;
	lcd_qr_send_command(0x2A);//0x2a,0x2b,0x2c都是和lcd屏相关的命令
	lcd_qr_send_data(&ch, 1);
	lcd_qr_send_data(&x1, 1);
	lcd_qr_send_data(&ch, 1);
	lcd_qr_send_data(&x2, 1);
	//uart_printf("x1=%d x2=%d\r\n", x1, x2);
	lcd_qr_send_command(0x2B);
	lcd_qr_send_data(&ch, 1);
	lcd_qr_send_data(&y1, 1);
	lcd_qr_send_data(&ch, 1);
	lcd_qr_send_data(&y2, 1);
	//uart_printf("y1=%d y2=%d\r\n", y1, y2);
	lcd_qr_send_command(0x2c);
}

void lcd_qr_string_display(u8* pdata,u16 inlen)
{
    #define DISPLAY_MAX_CHAR_NUM    10 
	int i,j,count;
    u16 tmp[8*DISPLAY_MAX_CHAR_NUM];//最多一次显示10个字符或者5个汉字
    u16 len = inlen;

    while(len > 0)
    {
        if(len >= DISPLAY_MAX_CHAR_NUM)
        {
            count = DISPLAY_MAX_CHAR_NUM;
            len -= DISPLAY_MAX_CHAR_NUM;
        }
        else
        {
            count = len;
            len = 0;
        }
        for(j=0; j<count; j++)
        {
            for(i=0; i<8; i++)
            {
                if(*pdata<<i&0x80)
                {
                    tmp[i+j*8] = qr_backcolor;
                }
                else
                {
                    tmp[i+j*8] = qr_pencolor;
                }
            }
            pdata++;
        }  
        
        lcd_qr_send_data((u8*)(&tmp),count*16);
    }
    
    
}

//这个函数和前面一个函数重合了
#if 0 /*xqy 2018-1-29*/
void lcd_qr_ascii_Dis(u8 data)
{
	int i;
    u16 tmp[8];
	for(i=0; i<8; i++)
	{
		if(data<<i&0x80)
		{
			//lcd_qr_send_data((u8*)(&qr_backcolor), 2);
			tmp[i] = qr_backcolor;
		}
		else
		{
			//lcd_qr_send_data((u8*)(&qr_pencolor), 2);
			tmp[i] = qr_pencolor;
		}
	}
	lcd_qr_send_data((u8*)(&tmp), 16);
}
#endif

void st7735s_initial(void)
{
	u8 lcm_tmp[2];
	u8 lcd_fr[6] = {0x05, 0x3C, 0x3C,0x05, 0x3C, 0x3C};

	u8 lcd_pw_seq[6] = {0x28, 0x08, 0x04,0, 0, 0};

	u8 lcd_gamma[16] = {0x04, 0x22, 0x07, 0x0A, 0x2E, 
						0x30, 0x25, 0x2A, 0x28,  0x26, 
						0x2E, 0x3A, 0x00, 0x01, 0x03,
						0x13};

	u8 lcd_gamma1[16] = {	0x04, 0x16, 0x06, 0x0D, 0x2D, 
						0x26, 0x23, 0x27, 0x27,  0x25, 
						0x2D, 0x3B, 0x00, 0x01, 0x04,
						0x13};
	
	int i, j ;
	u8 redh, redl ;
	lcm_tmp[0] = 0x05;
	
	lcd_qr_init_gpio();
	
	RST_LCD_QR_level(1);
	//delay_ms(2); 
	RST_LCD_QR_level(0);
	//delay_ms(2); 
	RST_LCD_QR_level(1);
	//delay_ms(2);
	lcd_qr_send_command(0x11); //soft reset 开始进行软复位
	
	//------------------------------------ST7735S Frame Rate-----------------------------------------//
	st7735s_write( 0xB1, lcd_fr, 3 );
	st7735s_write( 0xB2, lcd_fr, 3 );
	st7735s_write( 0xB3, lcd_fr, 6 );
	
	lcd_fr[0] = 0x03;
	st7735s_write( 0xB4, lcd_fr, 1 );	//Dot inversion
	
	//------------------------------------ST7735S Power Sequence-----------------------------------------//
	st7735s_write( 0xC0, lcd_pw_seq, 3 );
	
	lcd_pw_seq[0] = 0xC0;
	st7735s_write( 0xC1, lcd_pw_seq, 1 );	//Dot inversion
	
	lcd_pw_seq[0] = 0x0D;
	lcd_pw_seq[1] = 0x00;
	st7735s_write( 0xC2, lcd_pw_seq, 2 );	//Dot inversion
	
	lcd_pw_seq[0] = 0x8D;
	//lcd_pw_seq[1] = 0x6A;
	lcd_pw_seq[1] = 0x2A;
	st7735s_write( 0xC3, lcd_pw_seq, 2 );	//Dot inversion
	
	lcd_pw_seq[0] = 0x8D;
	lcd_pw_seq[1] = 0xEE;
	st7735s_write( 0xC4, lcd_pw_seq, 2 );	//Dot inversion
	
	//---------------------------------End ST7735S Power Sequence-------------------------------------//
	//lcd_pw_seq[0] = 0x12;
	lcd_pw_seq[0] = 0x1A;
	st7735s_write( 0xC5, lcd_pw_seq, 1 );	//Dot inversion
	lcd_pw_seq[0] = 0x36;
	st7735s_write( 0xC8, lcd_pw_seq, 1 );	//Dot inversion
	delayms(2);
	lcd_qr_send_command(0x36);     //MX, MY, RGB mode
	lcd_qr_send_a_data(0x68);
	delayms(2);
	//------------------------------------ST7735S Gamma Sequence-----------------------------------------//
	st7735s_write( 0xE0, lcd_gamma, 16 );
	st7735s_write( 0xE1, lcd_gamma1, 16 );
	
	//------------------------------------End ST7735S Gamma Sequence-----------------------------------------//
	st7735s_write( 0x3A, lcm_tmp, 1 );
	
	lcd_qr_send_command(0x29); //Display on

	dev_lcd_qr_fill_rectangle(0, 0, LCD_QR_WIDTH - 1, LCD_QR_HEIGHT - 1, 0);//xqy填充黑色

}
