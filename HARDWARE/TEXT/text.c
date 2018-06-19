#include "sys.h" 
#include "fontupd.h"
#include "lcd.h"
#include "text.h"	
#include "string.h"												    
#include "usart.h"
#include "exfuns.h" 

//////////////////////////////////////////////////////////////////////////////////	 
 
//汉字显示 驱动代码	   
//STM32F4工程模板-库函数版本
//淘宝店铺：http://mcudev.taobao.com									  
////////////////////////////////////////////////////////////////////////////////// 	 

//xqy这些字库都是竖扫的，不是横扫
FIL* GBK_12 = NULL;
FIL* GBK_16 = NULL;
FIL* GBK_24 = NULL;

//code 字符指针开始
//从字库中查找出字模
//code 字符串的开始地址,GBK码
//mat  数据存放地址 (size/8+((size%8)?1:0))*(size) bytes大小	
//size:字体大小
void Get_HzMat(unsigned char *code,unsigned char *mat,u8 size)
{		    
	unsigned char qh,ql;
	unsigned char i;					  
	unsigned long foffset; 
	u32 br_xqy;
	u8 csize=(size/8+((size%8)?1:0))*(size);//得到字体一个字符对应点阵集所占的字节数	 
	qh=*code;
	ql=*(++code);
	if(qh<0x81||ql<0x40||ql==0xff||qh==0xff)//非 常用汉字
	{   		   
	    uart_printf("非常用汉字\n");
	    for(i=0;i<csize;i++)
	        *mat++=0x00;//填充满格
	    return; //结束访问
	}          
	if(ql<0x7f)
	    ql-=0x40;//注意!
	else
	    ql-=0x41;
	qh-=0x81;   
	foffset=((unsigned long)190*qh+ql)*csize;	//得到字库中的字节偏移量  		  
	switch(size)
	{
		case 12:
		    f_lseek(GBK_12,foffset);
		    f_read(GBK_12,mat,csize,&br_xqy);
			//W25QXX_Read(mat,foffset+ftinfo.f12addr,csize);
			break;
		case 16:
		    f_lseek(GBK_16,foffset);
		    f_read(GBK_16,mat,csize,&br_xqy);
			//W25QXX_Read(mat,foffset+ftinfo.f16addr,csize);
			break;
		case 24:
		    f_lseek(GBK_24,foffset);
		    f_read(GBK_24,mat,csize,&br_xqy);
			//W25QXX_Read(mat,foffset+ftinfo.f24addr,csize);
			break;
			
	}
	//uart_printf("\n--%d------%d-----%d-\n",size,csize,foffset);
	//printf_format(mat,csize);
	//printf_format_bit(mat,size,csize);
}  
//显示一个指定大小的汉字
//x,y :汉字的坐标
//font:汉字GBK码
//size:字体大小
//mode:0,正常显示,1,叠加显示	 

void Show_Font(u16 x,u16 y,u8 *font,u8 size,u8 mode)
{
	u8 temp,t,t1;
	u16 y0=y;
	u8 dzk[72];   
	u8 csize=(size/8+((size%8)?1:0))*(size);//得到字体一个字符对应点阵集所占的字节数	 
	if(size!=12&&size!=16&&size!=24)
	    return;	//不支持的size
	Get_HzMat(font,dzk,size);	//得到相应大小的点阵数据 
	for(t=0;t<csize;t++)
	{   												   
		temp=dzk[t];			//得到点阵数据                          
		for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)
			    DrawPixel_1(x,y,POINT_COLOR);
			else if(mode==0)
			    DrawPixel_1(x,y,BACK_COLOR); 
			temp<<=1;
			y++;
			if((y-y0)==size)
			{
				y=y0;
				x++;
				break;
			}
		}  	 
	}  
}
//在指定位置开始显示一个字符串	    
//支持自动换行
//(x,y):起始坐标
//width,height:区域
//str  :字符串
//size :字体大小
//mode:0,非叠加方式;1,叠加方式    	   		   
void Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode)
{					
	u16 x0=x;
	u16 y0=y;							  	  
    u8 bHz=0;     //字符或者中文  	    				    				  	  
    while(*str!=0)//数据未结束
    { 
        if(!bHz)
        {
	        if(*str>0x80)
	        {
	            bHz=1;//中文 
	        }
	        else              //字符
	        {      
                if(x>(x0+width-size/2))//换行
				{				   
					y+=size;
					x=x0;	   
				}							    
		        if(y>(y0+height-size))
		            break;//越界返回      
		        if(*str==13)//换行符号
		        {         
		            y+=size;
					x=x0;
		            str++; 
		        }  
		        else 
		        {
		            LCD_ShowChar(x,y,*str,size,mode);//有效部分写入 
		        }
				str++; 
		        x+=size/2; //字符,为全字的一半 
	        }
        }
        else//中文 
        {     
            bHz=0;//有汉字库    
            if(x>(x0+width-size))//换行
			{	    
				y+=size;
				x=x0;		  
			}
	        if(y>(y0+height-size))
	            break;//越界返回  						     
	        Show_Font(x,y,str,size,mode); //显示这个汉字,空心显示 
	        str+=2; 
	        x+=size;//下一个汉字偏移	    
        }						 
    }   
}  			 		 
//在指定宽度的中间显示字符串
//如果字符长度超过了len,则用Show_Str显示
//len:指定要显示的宽度			  
void Show_Str_Mid(u16 x,u16 y,u8*str,u8 size,u8 len)
{
	u16 strlenth=0;
   	strlenth=strlen((const char*)str);
	strlenth*=size/2;
	if(strlenth>len)
	{
	    Show_Str(x,y,lcddev.width,lcddev.height,str,size,1);
	}
	else
	{
		strlenth=(len-strlenth)/2;
	    Show_Str(strlenth+x,y,lcddev.width,lcddev.height,str,size,1);
	}
}   
//初始化字体
//返回值:0,字库完好.
//		 其他,字库丢失
s32 font_init(void)
{	
    s32 ret = 0;
    GBK_12 = (FIL*)mymalloc(sizeof(FIL));//申请内存
    GBK_16 = (FIL*)mymalloc(sizeof(FIL));//申请内存
    GBK_24 = (FIL*)mymalloc(sizeof(FIL));//申请内存
    if(!GBK_12 || !GBK_16 || !GBK_24)
    {
        uart_printf("font int malloc fail!\n");
        return -1;
    }
    
    if(f_open(GBK_12,GBK12_PATH,FA_READ))//打开文件
	{
	    ret = -1;
		uart_printf("打开%s文件错误\n",GBK12_PATH);
	}
	if(f_open(GBK_16,GBK16_PATH,FA_READ))//打开文件
	{
	    ret = -1;
		uart_printf("打开%s文件错误\n",GBK16_PATH);
	}
	if(f_open(GBK_24,GBK24_PATH,FA_READ))//打开文件
	{
	    ret = -1;
		uart_printf("打开%s文件错误\n",GBK24_PATH);
	}
	return ret;
}

void lcd_show_text(u16 x,u16 y,u8*str,u8 size,u8 mode)
{
    Show_Str(x,y,lcddev.width,lcddev.height,str,size,mode);
}

void lcd_show(u16 row,u16 line,u8*str,u8 size,u8 mode)
{
    u16 x,y;
    y = size*(row);
    x = size*(line);
    #if 0 /*xqy 2018-6-17*/
    if(x>(LCD_WIDTH-size) || y>(LCD_HIGH-size))
        return;
    #endif
    Show_Str(x,y,lcddev.width,lcddev.height,str,size,mode);
}
























		  






