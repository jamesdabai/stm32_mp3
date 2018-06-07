#include "sys.h"
#include "ddi.h"
#include "dev_time.h"

extern u32 OSTickCtr;
u32 dev_get_timeID(void)
{
    u32 timer_tmp;
    
    timer_tmp = OSTickCtr;//用ucosiii中的时基值
    
    return timer_tmp;
}
/****************************************************************************
**Description:    查询从Time时间到当前系统时间是否经过了Delay时间
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:            黄学佳(2013-4-12)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
s32 dev_querry_time(u32 Time, u32 Delay)
{
    u32 curtime;//当前系统时间
    u32 passtime;
    
    curtime = dev_get_timeID();

    //uart_printf("id:%d, ti:%d, d:%d\r\n", curtime, Time, Delay);
    if(curtime >= Time)//系统时间无溢出
    {
        passtime = curtime - Time;
    }
    else
    {
        passtime = (u32)(~0) - (Time - curtime);
    }
    
    if(passtime >= Delay)
    {
        return FUN_TIME_OVER;//已经过去指定时间
    }
    else
    {
        return FUN_TIME_NOOVER;//经过的时间没有Delay这么长   
    }
}

s32 ddi_sys_get_tick(u32 *nTick)  //系统滴答
{
    
    *nTick = dev_get_timeID();
    return DDI_OK;
}

void delay_ms(int ms)//延时函数
{
	int us;

    s32 curtime;
	curtime = dev_get_timeID();
	
	while(1)
	{
		if(dev_querry_time(curtime, ms)==FUN_TIME_OVER)
		{
			break;
		}
	}
}



