#include "dev_uart.h"

#include "usart.h"	
#include "stdio.h"

#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos สนำร	  
#endif

extern int dev_com_printf(u8 ch);
int uart_printf(const char *fmt, ...)
{
    OS_ERR err;
     va_list ap;
     char string[256];
	 //char *string;
     char *pt;
     u8 i;
	//string = malloc(256);
	if(string == NULL)
	{
		return 0;
	}
     va_start(ap,fmt);
     vsprintf(string,fmt,ap);
     pt = &string[0];
     i=0;
     while(*pt!='\0')
     {
	     dev_com_printf(*pt);
         pt++;
         i++;
     }
     va_end(ap);
     //delayms(10);
     return i;
}
void printf_format(u8 *buf, u32 len)
{
	int i;
	OS_ERR err;
	u8 temp[2] = {0,'\0'};
	
	for(i=0; i<len; i++)
	{
		if(i>0 && (i%16)==0)
		{
			uart_printf("\r\n");
		}
		//dev_com_printf(buf[i]);
		temp[0] = buf[i];
		uart_printf("%02X ",temp[0]);
	}
	uart_printf("\r\n");
}
void printf_format_bit(u8 *buf,u32 bit_num,u32 len)
{
    u32 i;
    u8 temp;
    u8 t1;
    u8 bit_flag = 0;
    for(i=0;i<len;i++)
    {
        temp = buf[i];
        for(t1=0;t1<8;t1++)
		{
		    if(bit_flag>bit_num)
		    {
		        uart_printf("\n");
		        bit_flag = 0;
		    }
			if(temp&0x80)
			    uart_printf("%c",'*');
			else
			    uart_printf(" ");
			temp<<=1;
			bit_flag++;
		}  	 
    }
}


