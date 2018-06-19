/********************************Copyright ( c ) ***************************
**                  Shenzhen Xinguodu Technology Co., LTD.
**                        http://www.xinguodu.com
**
**
**Created By:         xqy
**Created Date:       2017-12-13
**Version:            <version >
**Description:        移植键盘驱动到stm32中
**
****************************************************************************/



#include <stdarg.h>
#include <stdio.h>

#include "dev_key.h"
#include "stm32f4xx_gpio.h"
#include "dev_time.h"
#include "ddi.h"
#include "core_cm4.h"

#define Test_DRV_KEY
#ifdef Test_DRV_KEY
#define key_debug uart_printf
#else
#define key_debug(a,...)
#endif

struct _key_scan_ctr
{
    u32 old_sta;//键盘上一次稳定状态
    u32 new_sta;
    u8 col;//记录扫描的COL
    u8 deb;//防抖
};

static struct _key_scan_ctr KeyScnCtr;

//按键扫描的状态缓冲，
#define KEY_SCAN_STA_BUF_MAX 10
struct _key_scan_sta_buf
{
    struct _key_scan_sta chg[KEY_SCAN_STA_BUF_MAX];  
    u8 head;
    u8 end;
};

static struct _key_scan_sta_buf KeyChgBuf;

enum KEY_COL
{
    KEY_COL_0 = 0,
    KEY_COL_1,
    KEY_COL_2,
    KEY_COL_3,
    //KEY_COL_PWR,//专门用于扫描PWR键
    KEY_COL_MAX,
};

struct _key_value key_value[6] = 
{
    "取消",
    "清除",
    "上翻",
    "下翻",
    "确定",
    "开关机",
};


void key_init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOE时钟

  //GPIOE7~E10初始化设置行输出
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;//行输出
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO

  //GPIOE11~E14初始化设置列输入
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;//列输入
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO
	
  GPIO_SetBits(GPIOE,GPIO_Pin_7 | GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10);//

}



/****************************************************************************
**Description:      对指定 col输出0，其他输出1     
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:            hxj(2014-3-20)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
static s32 drv_key_col_output(u8 col)
{
    u16 pin_data;
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;//行输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    
	//set col output 0
	pin_data = GPIO_Pin_7 |GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;
	GPIO_SetBits(GPIOE,pin_data);//列输出1高电平
	
	
    #if 0 /*xqy 2017-12-13*/
    GPIO_WritePinOutput(KEY_COL_GPIO, PORTB0, HIGH_LEVEL);
	GPIO_WritePinOutput(KEY_COL_GPIO, PORTB1, HIGH_LEVEL);
	GPIO_WritePinOutput(KEY_COL_GPIO, PORTB2, HIGH_LEVEL);
	GPIO_WritePinOutput(KEY_COL_GPIO, PORTB3, HIGH_LEVEL);
    #endif


    if(col != KEY_COL_0)
    {
        #if 0 /*xqy 2017-12-13*/
        //DrvKeyDebug("COL 0 input\r\n");
		PORT_SetPinMux(KEY_COL_PORT, PORTB0, kPORT_MuxAsGpio);    // set as gpio
		GPIO_PinInit(KEY_COL_GPIO, PORTB0, &input_config);        // input
		pin_pull_Config.pullSelect = kPORT_PullUp;	
        pin_pull_Config.mux = kPORT_MuxAsGpio;
        PORT_SetPinConfig(PORTB, PORTB0, &pin_pull_Config);       // enable pull up
        #endif
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
        GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO
		
    }

    if(col != KEY_COL_1)
    {
        #if 0 /*xqy 2017-12-13*/
        //DrvKeyDebug("COL 1 input\r\n");
		PORT_SetPinMux(KEY_COL_PORT, PORTB1, kPORT_MuxAsGpio);    // set as gpio
		GPIO_PinInit(KEY_COL_GPIO, PORTB1, &input_config);        // input
		pin_pull_Config.pullSelect = kPORT_PullUp;	
        pin_pull_Config.mux = kPORT_MuxAsGpio;
        PORT_SetPinConfig(PORTB, PORTB1, &pin_pull_Config);       // enable pull up	
        #endif
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
        GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO
	
    }

    if(col != KEY_COL_2)
    {
        #if 0 /*xqy 2017-12-13*/
        //DrvKeyDebug("COL 2 input\r\n");
		PORT_SetPinMux(KEY_COL_PORT, PORTB2, kPORT_MuxAsGpio);    // set as gpio
		GPIO_PinInit(KEY_COL_GPIO, PORTB2, &input_config);        // input
		pin_pull_Config.pullSelect = kPORT_PullUp;	
        pin_pull_Config.mux = kPORT_MuxAsGpio;
        PORT_SetPinConfig(PORTB, PORTB2, &pin_pull_Config);       // enable pull up	
        #endif
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO

    }

    if(col != KEY_COL_3)
    {
        #if 0 /*xqy 2017-12-13*/
        //DrvKeyDebug("COL 3 input\r\n");
		PORT_SetPinMux(KEY_COL_PORT, PORTB3, kPORT_MuxAsGpio);    // set as gpio
		GPIO_PinInit(KEY_COL_GPIO, PORTB3, &input_config);        //set as  input
		pin_pull_Config.pullSelect = kPORT_PullUp;	
        pin_pull_Config.mux = kPORT_MuxAsGpio;
        PORT_SetPinConfig(PORTB, PORTB3, &pin_pull_Config);       // enable pull up	
        #endif
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO

    }

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    //DrvKeyDebug("key scan, next col output low:%d \r\n", col);
    switch(col)
    {
        case KEY_COL_0:
			
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
            GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO
		    GPIO_ResetBits(GPIOE,GPIO_Pin_7);
            break;

        case KEY_COL_1:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
            GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO
		    GPIO_ResetBits(GPIOE,GPIO_Pin_8);
            break;

        case KEY_COL_2:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
            GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO
		    GPIO_ResetBits(GPIOE,GPIO_Pin_9);
            break;

        case KEY_COL_3:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
            GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO
		    GPIO_ResetBits(GPIOE,GPIO_Pin_10);
            break;
        
        default:
            key_debug("key scan err col\r\n");
            break;
    }
    return 0;
}

/****************************************************************************
**Description:        读ROW的值，结果右对齐     
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:            hxj(2014-3-20)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
static s32 drv_key_row_read(u32 *status)
{
    u16 row_sta;

	row_sta = GPIO_ReadInputData(GPIOE);

	*status = (row_sta >> 11) & 0x0F;//取11到14位的数据
    return 0;
}
/****************************************************************************
**Description:       初始化按键扫描硬件     
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:            hxj(2014-3-20)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
s32 drv_key_init(void)
{
    key_init();//初始化列和行
    drv_key_col_output(KEY_COL_0);
    KeyScnCtr.col = KEY_COL_0;
    KeyScnCtr.old_sta = 0XFFFFFFFF;//初始化为没有按键按下
    KeyScnCtr.new_sta = 0XFFFFFFFF;
    KeyScnCtr.deb = 0;
    KeyChgBuf.end = 0;
    KeyChgBuf.head = 0;
    return 0;
}

/****************************************************************************
**Description:      逐行扫描按键状态  仅仅上报状态，至于按键处理，由按键任务处理   
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:            hxj(2014-3-20)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
void drv_key_scan(void)
{
    u32 row_sta_tmp;
    
    static u32 row_sta = KEY_ROW_STA_INIT;//最多保存32键
    
    //读ROW
    drv_key_row_read(&row_sta_tmp);
    //key_debug("row sta:%d, %01x\r\n", KeyScnCtr.col, row_sta_tmp);
    row_sta += (row_sta_tmp & KEY_ROW_MASK)<<(KEY_ROW_NUM * KeyScnCtr.col);
    //下一COL输出0
    KeyScnCtr.col++;
    if(KeyScnCtr.col >= KEY_COL_MAX)
    {   
        row_sta_tmp = row_sta&0xFFFF;
        //key_debug("%s,%04x\r\n",(u8 *)"一轮" ,row_sta_tmp);
        KeyScnCtr.col = KEY_COL_0;
    }

    drv_key_col_output(KeyScnCtr.col);//处理COL输出
    
    if(KeyScnCtr.col == KEY_COL_0)//键盘所有COL都扫描了一遍
    {
        if(row_sta != KeyScnCtr.new_sta)
        {
            //DrvKeyDebug("不等\r\n");
            KeyScnCtr.new_sta = row_sta;  
            KeyScnCtr.deb = 0;
        }
        else if(row_sta != KeyScnCtr.old_sta)
        {
            KeyScnCtr.deb++;
            if(KeyScnCtr.deb>=2)
            {
                //将变化的键跟当前按键状态上报
                KeyChgBuf.chg[KeyChgBuf.head].chg = KeyScnCtr.old_sta^row_sta;//求出变化按键
                key_debug("chg:%04x, sta:%04x\r\n", KeyChgBuf.chg[KeyChgBuf.head].chg, row_sta);
                KeyChgBuf.chg[KeyChgBuf.head].sta = row_sta;
                KeyChgBuf.head++;
                if(KeyChgBuf.head >= KEY_SCAN_STA_BUF_MAX)
                    KeyChgBuf.head = 0;
                
                KeyScnCtr.old_sta = row_sta;
                KeyScnCtr.deb = 0;
            }
        }
        else
        {
            KeyScnCtr.deb = 0;    
        }

        row_sta = KEY_ROW_STA_INIT;
    }
}


/****************************************************************************
**Description:      读按键改变事件     
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:            hxj(2014-3-11)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
s32 drv_key_chgsta_read(struct _key_scan_sta* chg)
{
    if(KeyChgBuf.head != KeyChgBuf.end)
    {
        chg->chg = KeyChgBuf.chg[KeyChgBuf.end].chg;
        chg->sta = KeyChgBuf.chg[KeyChgBuf.end].sta;
        KeyChgBuf.end++;
        if(KeyChgBuf.end >= KEY_SCAN_STA_BUF_MAX)
            KeyChgBuf.end = 0;
        return 1;
    }
    return 0;
}

/****************************************************************************
**Description:      打印按下的键     
**Input parameters:      
**Output parameters:     
**                       
**Returned value:        
**                       
**Created by:            xqy(2017-12-14)
**--------------------------------------------------------------------------
**Modified by:          
**Modified by:          
****************************************************************************/
void disp_keyvalue(u32 key)
{
    switch (key)
    {   
       case 0:
       case 1:
       case 2:
       case 3:
       case 4:
       case 5:
       case 6:
       case 7:
       case 8:
       case 9:
           //uart_printf("按下:%d\n",key);
           break;
       case cancel:
       case clr   :
       case up    :
       case down  :
       case enter :
       case pwroff:
           uart_printf("按下:%s\n",(u8 *)key_value[key-10].name);
           break;
        default:
            break;
    }
}
#if 0 /*xqy 2017-12-13*/
/****************************************************************************
**Description:      进入低功耗前需要设置按键，以便按键唤醒     
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:            hxj(2014-4-28)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
void drv_key_sleep(void)
{

    gpio_pin_config_t input_config = 
	{
        kGPIO_DigitalInput, 0,
    };

    gpio_pin_config_t output_config = 
	{
        kGPIO_DigitalOutput, LOW_LEVEL,
    };

    port_pin_config_t pin_pull_Config = {0};  //PULL UP
	pin_pull_Config.pullSelect = kPORT_PullUp;	
	pin_pull_Config.mux = kPORT_MuxAsGpio; 

    // row output 0
	PORT_SetPinMux(KEY_ROW_PORT, PORTB10, kPORT_MuxAsGpio);	  // set as gpio
	GPIO_PinInit(KEY_ROW_GPIO, PORTB10, &output_config);	  // set as  output
	//GPIO_WritePinOutput(KEY_ROW_GPIO, PORTB10, LOW_LEVEL);    // output 0	
	
	PORT_SetPinMux(KEY_ROW_PORT, PORTB11, kPORT_MuxAsGpio);	  // set as gpio
	GPIO_PinInit(KEY_ROW_GPIO, PORTB11, &output_config);	  // set as  output
	//GPIO_WritePinOutput(KEY_ROW_GPIO, PORTB11, LOW_LEVEL);    // output 0	

	PORT_SetPinMux(KEY_ROW_PORT, PORTB16, kPORT_MuxAsGpio);	  // set as gpio
	GPIO_PinInit(KEY_ROW_GPIO, PORTB16, &output_config);	  // set as  output
	//GPIO_WritePinOutput(KEY_ROW_GPIO, PORTB16, LOW_LEVEL);    // output 0	

	PORT_SetPinMux(KEY_ROW_PORT, PORTB17, kPORT_MuxAsGpio);	  // set as gpio
	GPIO_PinInit(KEY_ROW_GPIO, PORTB17, &output_config);	  // set as  output
	//GPIO_WritePinOutput(KEY_ROW_GPIO, PORTB17, LOW_LEVEL);    // output 0	

	PORT_SetPinMux(KEY_ROW_PORT, PORTB18, kPORT_MuxAsGpio);	  // set as gpio
	GPIO_PinInit(KEY_ROW_GPIO, PORTB18, &output_config);	  // set as  output
	//GPIO_WritePinOutput(KEY_ROW_GPIO, PORTB18, LOW_LEVEL);    // output 0	

    //col input
    PORT_SetPinInterruptConfig(KEY_COL_PORT, PORTB0, kPORT_InterruptFallingEdge);
	PORT_SetPinMux(KEY_COL_PORT, PORTB0, kPORT_MuxAsGpio);	  // set as gpio
	GPIO_PinInit(KEY_COL_GPIO, PORTB0, &input_config);		  // input
	PORT_SetPinConfig(KEY_COL_PORT, PORTB0, &pin_pull_Config); 	  // enable pull up

    PORT_SetPinInterruptConfig(KEY_COL_PORT, PORTB1, kPORT_InterruptFallingEdge);
	PORT_SetPinMux(KEY_COL_PORT, PORTB1, kPORT_MuxAsGpio);	  // set as gpio
	GPIO_PinInit(KEY_COL_GPIO, PORTB1, &input_config);		  // input
	PORT_SetPinConfig(KEY_COL_PORT, PORTB1, &pin_pull_Config); 	  // enable pull up	

    PORT_SetPinInterruptConfig(KEY_COL_PORT, PORTB2, kPORT_InterruptFallingEdge);
	PORT_SetPinMux(KEY_COL_PORT, PORTB2, kPORT_MuxAsGpio);	  // set as gpio
	GPIO_PinInit(KEY_COL_GPIO, PORTB2, &input_config);		  // input
	PORT_SetPinConfig(KEY_COL_PORT, PORTB2, &pin_pull_Config); 	  // enable pull up

    PORT_SetPinInterruptConfig(KEY_COL_PORT, PORTB3, kPORT_InterruptFallingEdge);
	PORT_SetPinMux(KEY_COL_PORT, PORTB3, kPORT_MuxAsGpio);	  // set as gpio
	GPIO_PinInit(KEY_COL_GPIO, PORTB3, &input_config);		  // input
	PORT_SetPinConfig(KEY_COL_PORT, PORTB3, &pin_pull_Config); 	  // enable pull up	

   

	EnableIRQ(PORTB_IRQn);
}

/****************************************************************************
**Description:      关闭按键中断功能     
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:            hxj(2014-4-28)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
void drv_key_wakeup(void)
{
    gpio_pin_config_t output_config = {
        kGPIO_DigitalOutput, HIGH_LEVEL,
    };
    DisableIRQ(PORTB_IRQn);
    // col output 1
	PORT_SetPinMux(KEY_COL_PORT, PORTB0, kPORT_MuxAsGpio);	  // set as gpio
	GPIO_PinInit(KEY_COL_GPIO, PORTB0, &output_config);	      // set as  output
	//GPIO_WritePinOutput(KEY_COL_GPIO, PORTB0, HIGH_LEVEL);    // output 1	
	
	PORT_SetPinMux(KEY_COL_PORT, PORTB1, kPORT_MuxAsGpio);	  // set as gpio
	GPIO_PinInit(KEY_COL_GPIO, PORTB1, &output_config);	      // set as  output
	//GPIO_WritePinOutput(KEY_COL_GPIO, PORTB1, HIGH_LEVEL);    // output 1	

	PORT_SetPinMux(KEY_COL_PORT, PORTB2, kPORT_MuxAsGpio);	  // set as gpio
	GPIO_PinInit(KEY_COL_GPIO, PORTB2, &output_config);	      // set as  output
	//GPIO_WritePinOutput(KEY_COL_GPIO, PORTB2, HIGH_LEVEL);    // output 1	

	PORT_SetPinMux(KEY_COL_PORT, PORTB3, kPORT_MuxAsGpio);	  // set as gpio
	GPIO_PinInit(KEY_COL_GPIO, PORTB3, &output_config);	      // set as  output
	//GPIO_WritePinOutput(KEY_COL_GPIO, PORTB3, HIGH_LEVEL);    // output 1	

	

	drv_key_init();
}
#endif


/********************************Copyright ( c ) ***************************
**                  Shenzhen Xinguodu Technology Co., LTD.
**                        http://www.xinguodu.com
**
**
**Created By:     hxj
**Created Date:       2014-3-13
**Version:            <version >
**Description:    按键接口
**
****************************************************************************/


//逐行扫描时，结构按键在扫描得到的按键状态的位置

struct _key_pra
{
    u8 jiegou;//结构按键值(物理按键值)，左上右上左下右下，从1开始
    u8 sch; //线路图按键值 左上右上左下右下，从1开始
    u32 line_mask;//逐行扫描的状态掩码
};


//这个是K200P的
const struct _key_pra KeyPra[16]=
{
    18, 14, 10,//0
    5,  1,  12,//1
    6,  2,  8,//2
    7,  3,  4,//3
    9,  5,  0,//4
    10, 6,  13,//5
    11, 7,  9,//6
    13, 9,  5,//7
    14, 10, 1,//8
    15, 11, 14,//9
    12, 13, 2,//取消
    16, 17, 6,//清除
    17, 18, 15,//上
    19, 12, 7,//下
    20, 15, 3,//确认
    20, 15, 11,//关机
};

const struct _key_fun_tab key_fun_tab[21]=
{
    "null\0",
    "on/off\0",
    "f\0",
    "f1\0",
    "f2\0",
    "ch1\0",
    "ch2\0",
    "ch3\0",
    "ch\0",
    "ch4\0",
    "ch5\0",
    "ch6\0",
    "cancel\0",
    "ch7\0",
    "ch8\0",
    "ch9\0",
    "clr\0",
    "up\0",
    "ch0\0",
    "down\0",
    "enter\0"
};//20个按键

#define key_buf_max 8//64  //sxlneicun

struct _key_buf
{
    u32 value[key_buf_max];
    u8 head;
    u8 end;
};

static struct _key_buf key_buf;

struct _key_ctrl
{

    u32 lastkey;//上一次上报的键值
    u32 timeid;//上次上报按键的时间
    u32 holdtime;//按键按下时间点，用于报卡键
    u32 delay;//过多长时间再报一次
    u32 last_key_sta;
    u8 keysta;//1 有键按下，0无键按下
    u8 holdflag;//卡键标识
    u8 longpress;

};

static struct _key_ctrl KeyCtrl;
static u8 KeyGd = 0;

#define KEY_LONGPRESS_OTHER_DELAY (200)//MS 长按键上报间隔

#define KEY_LONGPRESS_FRIST_DELAY (2000)//MS//长按键时间
#define KEY_PWRONOFF_DELAY      (3000)//ms//开关机响应时间
#define KEY_HOLD_ERR_TIME       (10*1000)//10s算卡键
/****************************************************************************
**Description:
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:            hxj(2014-3-11)
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
s32 dev_key_init()
{
    key_buf.end = 0;
    key_buf.head = 0;

    KeyCtrl.longpress = 0;//默认不开启长按键
    KeyCtrl.keysta = 0;
    KeyCtrl.holdflag = 0;
    (KeyCtrl.last_key_sta) = 0xffffffff;
    drv_key_init();
    return 0;
}
/****************************************************************************
**Description:      打开按键
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:            hxj(2014-3-11)
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
s32 dev_key_open(void)
{
    KeyGd = 1;
    return 0;
}

/****************************************************************************
**Description:      关闭按键
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:            hxj(2014-3-11)
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
s32 dev_key_close(void)
{
    KeyGd = 0;
    return 0;
}

/****************************************************************************
**Description:      清物理键值缓冲
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:            hxj(2014-3-11)
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
s32 dev_key_clear(void)
{
    key_buf.end = key_buf.head;
    return 0;
}

/****************************************************************************
**Description:      读物理键值
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:            hxj(2014-3-11)
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
s32 dev_key_read(u32 *lpKey)
{
    if(key_buf.head != key_buf.end)
    {
        *lpKey = key_buf.value[key_buf.end++];
        if(key_buf.end >= key_buf_max)
            key_buf.end = 0;
    }
    else
    {
        *lpKey = 0;
    }

    return 0;
}

/****************************************************************************
**Description:      偷窥一下，看是否有按键     
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:            hxj(2014-7-17)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
s32 dev_key_peek(u32 *lpKey)
{

    if(key_buf.head != key_buf.end)
    {
        *lpKey = key_buf.value[key_buf.end];
    }
    else
    {
        *lpKey = 0;
    }

    return 0;
    
}

/****************************************************************************
**Description:    写键值到缓冲       
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:          
**--------------------------------------------------------------------------
**Modified by: 2014.07.28 增加点亮按键与LCD背光功能。
**Modified by:      
****************************************************************************/
static s32 dev_key_write(u32 key)
{
    //dev_key_bl_on();
    //dev_lcd_bl_on();
        
    key_buf.value[key_buf.head++] = key;
    if(key_buf.head >= key_buf_max)
        key_buf.head = 0;
    return 0;
}


/****************************************************************************
**Description:       写一个开关机长按键值到缓冲    
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:      
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
s32 dev_key_wirte_power(void)
{
    dev_key_write(KEY_ONOFF_HOLD);   
    return 0;
}
/****************************************************************************
**Description:      控制按键
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:            hxj(2014-3-11)
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
s32 dev_key_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    s32 ret = DDI_OK;
#if 0
    switch(nCmd)
    {
    case DDI_KEY_CTL_LONGPRESS://设置长按键
        if(lParam == 0)//禁止长按键
        {
            KeyCtrl.longpress = 0;
        }
        else if(lParam == 1)//允许长按键
        {
            KeyCtrl.longpress = 1;
        }
        else
        {
            //参数错误
            ret = DDI_EINVAL;
        }
        break;

    case DDI_KEY_CTL_BKLIGHT://控制按键背光
        if((SysPwr.machine == MACHINE_G2)||(SysPwr.machine == MACHINE_G2_PT))
        {
            if(lParam == 0)//关
            {
                DevKeyDebug("dis 按键背光\r\n");
                dev_key_bl_dis();
            }
            else if(lParam == 1)//开
            {
                DevKeyDebug("en 按键背光\r\n");
                dev_key_bl_en();
            }
            else
            {
                //参数错误
                ret = DDI_EINVAL;
            }
        }
        break;

    case DDI_KEY_CTL_BKLIGHTTIME: //设置背光时间
        if((SysPwr.machine == MACHINE_G2)||(SysPwr.machine == MACHINE_G2_PT))
        {
            if(lParam > 255)
            {
                DevKeyDebug("设置背光时间参数错误\r\n");
                ret = DDI_EINVAL;
            }
            else
            {
                dev_key_bl_settime(lParam);
            }
        }
        break;

    case DDI_KEY_CTL_BKLIGHT_CTRL:
        if((SysPwr.machine == MACHINE_G2)||(SysPwr.machine == MACHINE_G2_PT))
        {
            if(lParam == 0)//熄灭
            {
                dev_key_bl_off(); 
                ret = DDI_OK;
            }
            else if(lParam == 1)//点亮
            {
                dev_key_bl_core_on();
                ret = DDI_OK;
            }
            else
            {
                ret = DDI_EINVAL;   
            }
        }
        break;
        
    default:
        ret = DDI_EINVAL;
        break;
    }
#endif
    return ret;
}


/****************************************************************************
**Description:      逐行扫描时，根据状态求键值
**Input parameters:  sta 按键状态
**Output parameters:
**
**Returned value: 键值
**
**Created by:            hxj(2014-3-20)
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static u32 dev_key_get_keyvalue(u32 sta)
{
    u32 key_value = 0;
    u32 key_sta;

    s8 i;
    u8 cnt;

    i = 15;//按键最大
    cnt = 0;
    while(i >= 0)
    {
        //key_debug("i=%d\n",i);
        key_sta = sta & (0x01<<(KeyPra[i].line_mask));
        if( key_sta == 0)
        {
            key_value = (key_value << 8)+i;
            cnt++;
            if(cnt >4)//最多4个键
            {
                key_value = 0;
                //key_debug("键值过多，跳出\n");
                break;
            }
        }
        i--;
    }
    //key_debug("返回值%08x\n",key_value);
    return key_value;
}
/****************************************************************************
**Description:      按键任务，用来处理长按键等任务
                    在CORE任务中调用
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:            hxj(2014-3-11)
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
void dev_key_task(void)
{
    u32 key_value;
    s32 res;
    u8 i;
    u8 cnt;
    u32 lParam;
    struct _key_scan_sta key_chg_sta;

    static u8 onoff_sta = 0;//保证只执行一次
    static s32 timeid = 0;

    if(KeyGd == 0)//按键关闭状态，不扫描
        return;

    //处理按键变化
    res = drv_key_chgsta_read(&key_chg_sta);
    if(1 == res)
    {
        key_debug("chg: %04x, %04x\r\n", key_chg_sta.chg, key_chg_sta.sta);
    
        //变化
        //求按键
        if(((KeyCtrl.last_key_sta) | key_chg_sta.sta) == 0xffffffff)
        {

            //对于卡键的判断，要细化 任意一个按键一直按住，都认为是卡键
            //只有跟上次的按键状态完全不一样时，才更新卡键时间
            KeyCtrl.holdtime = dev_get_timeID();//每次按键状态改变都更新
            key_debug("当前时基为%d\r\n", KeyCtrl.holdtime);
        }
        //求出键值
        (KeyCtrl.last_key_sta) = key_chg_sta.sta;
        key_value = dev_key_get_keyvalue(key_chg_sta.sta);
        KeyCtrl.lastkey = key_value;
        key_debug("按下 %d\r\n", key_value);
        if((key_chg_sta.chg & key_chg_sta.sta) == 0)
        {
            //按下
            key_debug("按下 %08x\r\n", key_value);
            if(KeyCtrl.holdflag == 0)//没有卡键
            {
                key_debug("写一个键值%d\r\n", key_value);
                dev_key_write(key_value);
            }
            KeyCtrl.keysta = 1;

        }
        else
        {
            //其他变化，不报松开
            key_debug("其他变化\r\n");
            if(key_chg_sta.sta == 0XFFFFFFFF)
            {
                //当前没键按下
                key_debug("无键\r\n");
                KeyCtrl.keysta = 0;
            }
        }

        KeyCtrl.delay = KEY_LONGPRESS_FRIST_DELAY;//2S  按规范定2S
        KeyCtrl.timeid = dev_get_timeID();//更新按键按下时间

    }

    //---处理卡键---  //卡键，不能响应其他按键，按键扫描跟处理还要继续修改 FIX
    if(KeyCtrl.keysta == 1)//有键按下
    {
        if(FUN_TIME_OVER == dev_querry_time(KeyCtrl.holdtime, KEY_HOLD_ERR_TIME))
        {
            if(KeyCtrl.holdflag==0)
            //dev_lcd_keywarm();
            KeyCtrl.holdflag = 1;//有键卡主
        }
    }
    else if(KeyCtrl.keysta == 0
            && KeyCtrl.holdflag == 1)
    {
        KeyCtrl.holdflag = 0;
    }

    //----处理长按键，
    if(KeyCtrl.longpress == 1//允许长按键
            &&KeyCtrl.holdflag == 0//没有卡键
      )
    {
        if(KeyCtrl.keysta == 1)//有键按下
        {
            //时间到
            if(FUN_TIME_OVER == dev_querry_time(KeyCtrl.timeid, KeyCtrl.delay))
            {
                if(KeyCtrl.lastkey != 0x01)//如果只有关机键按下，不报长按
                {
                    dev_key_write(KeyCtrl.lastkey);
                }

                KeyCtrl.timeid = dev_get_timeID();
                KeyCtrl.delay = KEY_LONGPRESS_OTHER_DELAY;
            }
        }
    }
    #if 0 /*xqy 2017-12-13*/
    //处理开关机键
    #ifndef COER_DEAL_ONOFF
    //处理开关机情况
    if(1 == dev_key_check_onoff())//on off 按下,必须保持一直按下才会执行开关机
    {
        if(onoff_sta == 0)//说明是第一次变化
        {
            key_debug("开关机按键按下\r\n");    
            onoff_sta = 1;
            timeid = dev_get_timeID();
        }
        else if(onoff_sta == 1)
        {
            //判断开挂机按键按住时间
            if(FUN_TIME_OVER == dev_querry_time(timeid, SYS_POWER_HOLD))//暂定2S开关机
            {
                key_debug("写入一个关机键\r\n");
                dev_key_wirte_power();
                onoff_sta = 3;
            }
        }
        else
        {
        
        }
    }
    else
    {
        onoff_sta = 0;    
    }
    #endif
    #endif


}

/****************************************************************************
**Description:      按键进入低功耗
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:            hxj(2014-4-28)
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
void dev_key_sleep(void)
{
    //停止按键扫描
    dev_key_close();
    //配置按键中断唤醒
    //drv_key_sleep();
    
}

void dev_key_wakeup(void)
{
    dev_key_init();     //add by hecaiwen,2014.11.07,解决进入低功耗后，蓝牙唤醒出现"哔哔"叫的缺陷
   // drv_key_wakeup();
    dev_key_open();
}


/****************************************************************************
**Description:      检测开关键是否按下//直接检测，不用通过按键扫描?
**Input parameters:
**Output parameters:
**
**Returned value: 1 按下
**
**Created by:            hxj(2014-3-14)
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
s32 dev_key_check_onoff(void)
{

    if((KeyCtrl.last_key_sta & (1 << (KeyPra[1].line_mask))) == 0)
    {
        return 1;
    }
    else
        return 0;

}

/****************************************************************************
**Description:      测试按键
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:            hxj(2014-3-10)
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
s32 dev_key_test(void)
{
    u32 key;

    while(1)
    {
        //uart_printf("read\r\n");
        drv_key_scan();     //按键扫描
        dev_key_task();     //处理按键变化，

        
        dev_key_read(&key);
        //uart_printf("key:%08x\r\n", key);
        if(key !=0)
        {

            key_debug("key: %s ", &key_fun_tab[key&0x000000ff]);

            key = key>>8;
            if(key !=0)
            {
                key_debug("%s ", &key_fun_tab[key&0x000000ff]);
            }
            key = key>>8;
            if(key !=0)
            {
                key_debug("%s ", &key_fun_tab[key&0x000000ff]);
            }
            key = key>>8;
            if(key !=0)
            {
                key_debug("%s ", &key_fun_tab[key&0x000000ff]);
            }

            key_debug("--\r\n");
        }
    }
}

//----------------------DDI--------------------
#include "ddi.h"


s32 DdiKeyGd = -1;


s32 ddi_key_open (void)
{

    if(DdiKeyGd == -1)
    {
        dev_key_open();
        DdiKeyGd = 0;
    }
    return DDI_OK;
}

s32 ddi_key_close (void)
{
    ;

    if(DdiKeyGd == 0)
    {
        dev_key_close();
        DdiKeyGd = -1;
    }
    return DDI_OK;
}

s32 ddi_key_clear (void)
{
    ;

    if(DdiKeyGd == 0)
    {
        dev_key_clear();
        return DDI_OK;
    }
    else
    {
        return DDI_EIO;
    }

}

/****************************************************************************
**Description:
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:            hxj(2014-5-7)
**--------------------------------------------------------------------------
**Modified by:     2014.05.07 单键测试OK
**Modified by:
****************************************************************************/
s32 ddi_key_read (u32 *lpKey)
{
    u32 key, key_tmp;
    s32 ret = 0;

    //DEBUG_SHOW_FUN_NAME;

    *lpKey = 0;

    dev_key_read(&key);

    key_tmp = key;

    if(DdiKeyGd == -1)//设备未打开
    {
        //DevKeyDebug("设备未打开\r\n");
        return DDI_EIO;
    }
    if(key == 0)//无按键
    {
        return 0;
    }
    //统计按键个数
    if((key & 0xff) != 0)
    {
        ret++;
    }
    key = (key >> 8);

    if((key & 0xff) != 0)
    {
        ret++;
    }
    key = (key >> 8);

    if((key & 0xff) != 0)
    {
        ret++;
    }
    key = (key >> 8);

    if((key & 0xff) != 0)
    {
        ret++;
    }

    *lpKey = key_tmp;

    return ret;

}

/****************************************************************************
**Description:
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:            hxj(2014-5-7)
**--------------------------------------------------------------------------
**Modified by:      2014.05.07 测试OK
**Modified by:
****************************************************************************/
s32 ddi_key_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    ;

    if(DdiKeyGd == -1)//设备为打开
    {
        return DDI_EIO;
    }

    return dev_key_ioctl(nCmd, lParam, wParam);
}




static const u32 keyfun[21]=
{
    0,
    0,    FUNCTION,    F1,    F2,
    '1', '2',  '3',   ALPHA,
    '4', '5',  '6',   ESC,
    '7', '8',  '9',   CLR,
    '*', '0',  '#',   ENTER,
};


/*
    在应用任务中调用，不需要扫描按键
    乱，，，，hxj 2015.06.13
*/
s32 dev_key_fun_read(u32 *fun_key)
{
    u32 key;

    dev_key_read(&key);

    if(
        ((key & 0x000000ff) != 0)
        && ((key & 0xffffff00) == 0)
    )//测试程序不处理多键
    {
        if(key == KEY_ONOFF_HOLD)
        {
            *fun_key = key;
            return 0;
        }

        //dev_buzzer_open(100);

        *fun_key = keyfun[key&0x000000ff];

    }
    else
        *fun_key = 0;

    return 0;
}


#if 1 /*xqy 2018-6-7*/
//////////////////////////////暂时用开发板上的3个按键--代码//////////////////////
static struct _key_buf KEY_BUFF;

//按键初始化函数
void KEY_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOA,GPIOE时钟
 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5; //KEY2 KEY3对应引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIOE2,3,4
    KEY_BUFF.end = 0;
    KEY_BUFF.head = 0;
 
} 

static s32 key_write(u32 key)
{
    uart_printf("写入一个键值=%d\n",key);
    if(key==key_exit*key_ok)//如果同时按下
    {
        NVIC_SystemReset();
    }
    KEY_BUFF.value[KEY_BUFF.head++] = key;//最大储存8个按键值
    if(KEY_BUFF.head >= key_buf_max)
        KEY_BUFF.head = 0;
    return 0;
}
s32 key_read(u32 *lpKey)
{
    if(KEY_BUFF.head != KEY_BUFF.end)
    {
        *lpKey = KEY_BUFF.value[KEY_BUFF.end++];
        if(KEY_BUFF.end >= key_buf_max)
            KEY_BUFF.end = 0;
    }
    else
    {
        *lpKey = 0;
    }

    return 0;
}
s32 key_clear(void)
{
    KEY_BUFF.end = KEY_BUFF.head;
    return 0;
}

//mode:0,不支持连续按;1,支持连续按;
//0，没有任何按键按下
//1，KEY0按下
//2，KEY1按下
//4，WKUP按下 WK_UP
//注意此函数有响应优先级,KEY0>KEY1>KEY2>WK_UP!!
u8 KEY_Scan(u8 mode)
{	 
    u16 key_PE;
    static u16 key_PE_bak=0;
    u32 key_value_x = 1;
    u8 i;
	static u8 key_up_flag=1;//按键按松开标志
	
	if(mode)
	    key_up_flag=1;  //支持连按		
	key_PE = GPIO_ReadInputData(GPIOE);
	key_PE = ~(key_PE);//取反
	key_PE &= 0x3f;//只取低6位数据
	if(key_up_flag && key_PE)//
	{
	    key_PE_bak = key_PE;
		delay_os(50);//delayms(10);//去抖动 
		key_up_flag=0;
        for(i=0;i<6;i++)
        {
            if(key_PE&0x01)
            {
                key_value_x *= (i+1);//组合按键就是按键相乘的值，很好操作，取值时相乘比较是否是该组合键按下
            }
			key_PE>>=1;
        }
        key_write(key_value_x);
	}
	else if(key_PE==0)//无按键按下
	{
	    key_up_flag=1; 	
	}
	else if(key_PE!=key_PE_bak)//无按键按下
	{
	    key_PE_bak = key_PE;
	    key_up_flag=1; 	
	}
 	return 0;// 无按键按下
}
#endif






