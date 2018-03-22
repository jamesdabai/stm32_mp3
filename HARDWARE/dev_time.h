/********************************Copyright ( c ) ***************************
**                  Shenzhen Xinguodu Technology Co., LTD.
**                        http://www.xinguodu.com
**
**
**Created By:     hxj
**Created Date:       2014-3-6
**Version:            <version >
**Description:   时间管理头文件 
**
****************************************************************************/
#ifndef _FUN_TIME_H_
#define _FUN_TIME_H_

#define FUN_TIME_OVER 0x01
#define FUN_TIME_NOOVER 0X00

//#define delayms delay_ms

extern u32 dev_get_timeID(void);
extern s32 dev_querry_time(u32 Time, u32 Delay);
extern s32 ddi_sys_get_tick(u32 *nTick);
extern void delayms(int ms);
//extern s32 fun_delay_us(s32 us);
//extern void delay_ms(uint32_t ms);
//extern u32 dev_time_check(u32 times,u32 delay);

#endif


