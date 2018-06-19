/********************************Copyright ( c ) ***************************
**                  Shenzhen Xinguodu Technology Co., LTD.
**                        http://www.xinguodu.com
**
**
**Created By:         xqy
**Created Date:       2017-12-13
**Version:            <version >
**Description:        
**
****************************************************************************/

#ifndef _DRV_KEY_H_
#define _DRV_KEY_H_

#include "sys.h"

#define KEY_CHG_PRESS  1//按下
#define KEY_CHG_RELEASE  2//松开

#define LINE_TO_LINE_SCAN 1



#define KEY_ROW_MASK 0X0F//4 ROW
#define KEY_ROW_NUM 4//4 ROW
#define KEY_ROW_STA_INIT 0xffff0000//状态目前16个键xqy


#ifndef LINE_TO_LINE_SCAN

struct _key_scan_sta
{
    u8 type;//变化类型 按下1,松开2
    u16 col;//变化后的列状态
    u16 row;//变化后的行状态
};
#else
struct _key_scan_sta
{
    u32 chg;//改变的按键
    u32 sta;//当前按键状态
};
#endif

struct _key_fun_tab
{
    u8 name[20];
};

struct _key_value
{
    u8 name[10];
};

//extern void drv_key_sleep(void);
//extern void drv_key_wakeup(void);
#endif


/********************************Copyright ( c ) ***************************
**                  Shenzhen Xinguodu Technology Co., LTD.
**                        http://www.xinguodu.com
**
**
**Created By:     hxj
**Created Date:       2014-3-13
**Version:            <version >
**Description:    
**
****************************************************************************/
#ifndef _DEV_KEY_H_
#define _DEV_KEY_H_

#define KEYNULL     0x00    //无效键

#define DIGITAL0   '0'
#define DIGITAL1   '1'
#define DIGITAL2   '2'
#define DIGITAL3   '3'
#define DIGITAL4   '4'
#define DIGITAL5   '5'
#define DIGITAL6   '6'
#define DIGITAL7   '7'
#define DIGITAL8   '8'
#define DIGITAL9   '9'
#define UP         '*'
#define DOWN       '#'


#define F1          0x11
#define F2          0x12
#define F3          0x13     
#define FUNCTION    0x14    // 功能
#define ESC         0x1B
#define ALPHA       0x16    // 输入法/字母切换
#define CLR         0x17
#define ENTER       0X18
#define PWRONOFF    0x19

#define KEY_DEL     0xff//不支持

#define KEY_ONOFF_HOLD  0xEF//开关机长按2S后，上报一个0XEF给APP

#define DDI_KEY_CTL_LONGPRESS   0//设置长按键
#define DDI_KEY_CTL_BKLIGHT     1//控制按键背光
#define DDI_KEY_CTL_BKLIGHTTIME 2//设置背光时间
#define DDI_KEY_CTL_BKLIGHT_CTRL 3//直接控制背光 
//xiaqiyun
#define cancel 10 
#define clr    11 
#define up     12 
#define down   13 
#define enter  14 
#define pwroff 15 


extern s32 ddi_key_open (void);
extern s32 ddi_key_close (void);
extern s32 ddi_key_clear (void);
extern s32 ddi_key_read (u32 *lpKey);
extern s32 ddi_key_ioctl(u32 nCmd, u32 lParam, u32 wParam);

extern s32 dev_key_check_onoff(void);
extern s32 dev_key_clear(void);

extern void dev_key_task(void);

extern s32 dev_key_fun_read(u32 *fun_key);
extern void disp_keyvalue(u32 key);


#if 1 /*xqy 2018-6-7*/
//////////////////////////////暂时用开发板上的3个按键--代码//////////////////////
#include "sys.h" 
/*下面的方式是通过直接操作库函数方式读取IO*/
#define key_right   1 
#define key_down	2
#define key_ok      3
#define key_up	    4
#define key_exit  	5
#define key_left    6
void KEY_Init(void);	//IO初始化
u8 KEY_Scan(u8);  		//按键扫描函数	
#endif




#endif


