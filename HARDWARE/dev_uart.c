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
	 //free(string);
	 //OSTimeDlyHMSM(0u, 0u, 0u, 10u,
     //                     OS_OPT_TIME_HMSM_STRICT,
     //                     &err);
     delayms(10);
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
		OSTimeDlyHMSM(0u, 0u, 0u, 4u,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
	}
	uart_printf("\r\n");
}


